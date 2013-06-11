/**
 * libcec.cpp
 * By Andrew Brampton
 *
 * Notes:
 *   Turn off:
 *     SetInactiveView() // We don't want to be in control anymore
 *     StandbyDevices()  // We now turn off all devices
 *
 *   Turn on:
 *     PowerOnDevices()  // Turn on all devices
 *     SetActiveSource(m_configuration.deviceTypes[0]) // Enable us as Active source
 *
 *     SetOSDString(CECDEVICE_TV, CEC_DISPLAY_CONTROL_DISPLAY_FOR_DEFAULT_TIME, g_localizeStrings.Get(36016).c_str());
 *
 */
#include "libcec.h"

#include <cstdio>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <cassert>
#include <map>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

using namespace CEC;
using namespace log4cplus;

using std::endl;
using std::hex;
using std::map;
using std::ostream;

// cecloader has to be after some #includes and using namespaces :(
using std::cout;
using std::cerr;
#include <libcec/cecloader.h>

static Logger logger = Logger::getInstance("libcec");

// Map of control codes to Strings
const map<enum cec_user_control_code, const char *> Cec::cecUserControlCodeName = Cec::setupUserControlCodeName();

// We store a global handle, so we can use g_cec->ToString(..) in certain cases. This is a bit of a HACK :(
ICECAdapter * g_cec = NULL;

int cecLogMessage(void *cbParam, const cec_log_message message) {
	try {
		return ((CecCallback*) cbParam)->onCecLogMessage(message);
	} catch (...) {}
	return 0;
}

int cecKeyPress(void *cbParam, const cec_keypress key) {
	try {
		return ((CecCallback*) cbParam)->onCecKeyPress(key);
	} catch (...) {}
	return 0;
}

int cecCommand(void *cbParam, const cec_command command) {
	try {
		return ((CecCallback*) cbParam)->onCecCommand(command);
	} catch (...) {}
	return 0;
}

int cecConfigurationChanged(void *cbParam, const libcec_configuration configuration) {
	try {
		return ((CecCallback*) cbParam)->onCecConfigurationChanged(configuration);
	} catch (...) {}
	return 0;
}

struct ICECAdapterDeleter : std::default_delete<ICECAdapter> {
	//ICECAdapterDeleter() = default;

	void operator()(ICECAdapter* ptr) const {
		if (ptr) {
			UnloadLibCec(ptr);
			g_cec = NULL;
		}
	}
};

/**
 * Redirects the stream buffer for a stream for the lifetime of this object
 */
class RedirectStreamBuffer {
private:
	std::ios & stream;
	std::streambuf * orig_sb;

public:
	RedirectStreamBuffer(std::ios &stream, std::streambuf * new_sb) : stream(stream) {
		orig_sb = stream.rdbuf( new_sb );
	}

	~RedirectStreamBuffer() {
		restore();
	}

	void restore() {
		stream.rdbuf( orig_sb );
	}
};

ICECAdapter * Cec::CecInit(const char * name, CecCallback * callback) {
	assert(name != NULL);
	assert(callback != NULL);

	config.Clear();

	config.deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
	strncpy(config.strDeviceName, name, sizeof(config.strDeviceName));

	callbacks.CBCecLogMessage           = &::cecLogMessage;
	callbacks.CBCecKeyPress             = &::cecKeyPress;
	callbacks.CBCecCommand              = &::cecCommand;
	callbacks.CBCecConfigurationChanged = &::cecConfigurationChanged;
	config.callbackParam                = callback;
	config.callbacks                    = &callbacks;

	// LibCecInitialise is noisy, so we redirect cout to nowhere
	RedirectStreamBuffer redirect(cout, 0);
	return g_cec = (ICECAdapter *)LibCecInitialise(&config);
}

Cec::Cec(const char * name, CecCallback * callback) :
	cec(CecInit(name, callback), ICECAdapterDeleter())
{
	if (cec == NULL) {
		throw std::runtime_error("Failed to initialise libCEC");
	}
	cec->InitVideoStandalone();
}

Cec::~Cec() {}


void Cec::open() {
	LOG4CPLUS_TRACE_STR(logger, "Cec::open()");

	assert(cec != NULL);

	// Search for adapters
	cec_adapter devices[10];
	uint8_t ret = cec->FindAdapters(devices, 10, NULL);
	if (ret < 0) {
		throw std::runtime_error("Error occurred searching for adapters");
	}

	if (ret == 0) {
		throw std::runtime_error("No adapters found");
	}

	// Just use the first found
	if (!cec->Open(devices[0].comm)) {
		throw std::runtime_error("Failed to open adapter");
	}

	LOG4CPLUS_INFO(logger, "Opened " << devices[0].path);
}

void Cec::close() {
	cec->SetInactiveView();
	cec->Close();
}

void Cec::makeActive() {
	// and made active
	if (!cec->SetActiveSource(config.deviceTypes[0])) {
		throw std::runtime_error("Failed to become active");
	}
}



/**
 * Prints the name of all found adapters
 * This will close any open device!
 */
ostream & Cec::listDevices(ostream & out) {
	cec_adapter devices[10];
	int8_t ret = cec->FindAdapters(devices, 10, NULL);
	if (ret < 0) {
		LOG4CPLUS_ERROR(logger, "Error occurred searching for adapters");
		return out;
	}

	if (ret == 0) {
		LOG4CPLUS_ERROR(logger, "No adapters found");
	}

	for (int8_t i = 0; i < ret; i++) {
		out << "[" << (int) i << "] port:" << devices[i].comm << " path:" << devices[i].path << endl;

		if (!cec->Open(devices[i].comm)) {
			out << "\tFailed to open" << endl;
		}

		cec_logical_addresses devices = cec->GetActiveDevices();
		for (int j = 0; j < 16; j++) {
			if (devices[j]) {
				cec_logical_address logical_addres = (cec_logical_address) j;

				uint16_t physical_address = cec->GetDevicePhysicalAddress(logical_addres);
				cec_osd_name name = cec->GetDeviceOSDName(logical_addres);
				cec_vendor_id vendor = (cec_vendor_id) cec->GetDeviceVendorId(logical_addres);

				out << "\t"  << cec->ToString(logical_addres)
				    << "@0x" << hex << physical_address
				    << " "   << name.name << " (" << cec->ToString(vendor) << ")"
				    << endl;
			}
		}
	}
	return out;
}

map<cec_user_control_code, const char *> & Cec::setupUserControlCodeName() {
	static map<cec_user_control_code, const char *> cecUserControlCodeName;

	if (cecUserControlCodeName.empty()) {
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SELECT]="SELECT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_UP]="UP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_DOWN]="DOWN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_LEFT]="LEFT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_RIGHT]="RIGHT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_RIGHT_UP]="RIGHT_UP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_RIGHT_DOWN]="RIGHT_DOWN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_LEFT_UP]="LEFT_UP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_LEFT_DOWN]="LEFT_DOWN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_ROOT_MENU]="ROOT_MENU";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SETUP_MENU]="SETUP_MENU";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_CONTENTS_MENU]="CONTENTS_MENU";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_FAVORITE_MENU]="FAVORITE_MENU";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_EXIT]="EXIT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER0]="NUMBER0";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER1]="NUMBER1";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER2]="NUMBER2";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER3]="NUMBER3";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER4]="NUMBER4";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER5]="NUMBER5";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER6]="NUMBER6";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER7]="NUMBER7";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER8]="NUMBER8";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NUMBER9]="NUMBER9";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_DOT]="DOT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_ENTER]="ENTER";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_CLEAR]="CLEAR";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_NEXT_FAVORITE]="NEXT_FAVORITE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_CHANNEL_UP]="CHANNEL_UP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_CHANNEL_DOWN]="CHANNEL_DOWN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL]="PREVIOUS_CHANNEL";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SOUND_SELECT]="SOUND_SELECT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_INPUT_SELECT]="INPUT_SELECT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION]="DISPLAY_INFORMATION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_HELP]="HELP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PAGE_UP]="PAGE_UP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PAGE_DOWN]="PAGE_DOWN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_POWER]="POWER";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_VOLUME_UP]="VOLUME_UP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_VOLUME_DOWN]="VOLUME_DOWN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_MUTE]="MUTE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PLAY]="PLAY";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_STOP]="STOP";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PAUSE]="PAUSE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_RECORD]="RECORD";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_REWIND]="REWIND";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_FAST_FORWARD]="FAST_FORWARD";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_EJECT]="EJECT";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_FORWARD]="FORWARD";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_BACKWARD]="BACKWARD";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_STOP_RECORD]="STOP_RECORD";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PAUSE_RECORD]="PAUSE_RECORD";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_ANGLE]="ANGLE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SUB_PICTURE]="SUB_PICTURE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND]="VIDEO_ON_DEMAND";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE]="ELECTRONIC_PROGRAM_GUIDE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING]="TIMER_PROGRAMMING";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION]="INITIAL_CONFIGURATION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PLAY_FUNCTION]="PLAY_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION]="PAUSE_PLAY_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_RECORD_FUNCTION]="RECORD_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION]="PAUSE_RECORD_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_STOP_FUNCTION]="STOP_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_MUTE_FUNCTION]="MUTE_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION]="RESTORE_VOLUME_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_TUNE_FUNCTION]="TUNE_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION]="SELECT_MEDIA_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION]="SELECT_AV_INPUT_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION]="SELECT_AUDIO_INPUT_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION]="POWER_TOGGLE_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION]="POWER_OFF_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION]="POWER_ON_FUNCTION";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_F1_BLUE]="F1_BLUE";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_F2_RED]="F2_RED";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_F3_GREEN]="F3_GREEN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_F4_YELLOW]="F4_YELLOW";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_F5]="F5";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_DATA]="DATA";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_AN_RETURN]="AN_RETURN";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST]="AN_CHANNELS_LIST";
		cecUserControlCodeName[CEC_USER_CONTROL_CODE_UNKNOWN]="UNKNOWN";
	}

	return cecUserControlCodeName;
}

std::ostream& operator<<(std::ostream &out, const cec_user_control_code code) {
	map<cec_user_control_code, const char *>::const_iterator it;

	it = Cec::cecUserControlCodeName.find(code);
	if (it == Cec::cecUserControlCodeName.end()) {
		it = Cec::cecUserControlCodeName.find(CEC_USER_CONTROL_CODE_UNKNOWN);
		assert(it != Cec::cecUserControlCodeName.end());
	}

	return out << it->second;
}

std::ostream& operator<<(std::ostream &out, const cec_log_level & log) {
	if (log & CEC_LOG_ERROR)
		out << "E";
	if (log & CEC_LOG_WARNING)
		out << "W";
	if (log & CEC_LOG_NOTICE)
		out << "N";
	if (log & CEC_LOG_TRAFFIC)
		out << "T";
	if (log & CEC_LOG_DEBUG)
		out << "D";
	return out;
}

std::ostream& operator<<(std::ostream &out, const cec_log_message & message) {
	return out << message.time << " [" << message.level << "]" << message.message;
}

std::ostream& operator<<(std::ostream &out, const cec_keypress & key) {
	return out << "Key press: " << key.keycode << " for " << key.duration << "ms";
}

std::ostream& operator<<(std::ostream &out, const cec_command & cmd) {
//  cec_logical_address initiator;        /**< the logical address of the initiator of this message */
//  cec_logical_address destination;      /**< the logical address of the destination of this message */
//  int8_t              ack;              /**< 1 when the ACK bit is set, 0 otherwise */
//  int8_t              eom;              /**< 1 when the EOM bit is set, 0 otherwise */
//  cec_opcode          opcode;           /**< the opcode of this message */
//  cec_datapacket      parameters;       /**< the parameters attached to this message */
//  int8_t              opcode_set;       /**< 1 when an opcode is set, 0 otherwise (POLL message) */
//  int32_t             transmit_timeout; /**< the timeout to use in ms */

	return out << "Command " << cmd.initiator << " to " << cmd.destination << " " << cmd.opcode;
}

std::ostream& operator<<(std::ostream &out, const cec_opcode & opcode) {
	if (g_cec)
		return out << g_cec->ToString(opcode);
	return out << "UNKNOWN";
}

std::ostream& operator<<(std::ostream &out, const libcec_configuration & configuration) {
	throw std::runtime_error("unsupported operator<<(std::ostream &out, const libcec_configuration & configuration)");
}
