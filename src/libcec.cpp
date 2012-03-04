#include "libcec.h"

#include <iostream>
#include <cstddef>
#include <stdexcept>
#include <cassert>

using namespace CEC;

using std::cout;
using std::cerr;
using std::endl;
using std::hex;

// cecloader has to be after some #includes and using namespaces :(
#include <cecloader.h>

static int cecLogMessage(void *cbParam, const cec_log_message &message) {
	try {
		return ((CecCallback*) cbParam)->onCecLogMessage(message);
	} catch (...) {}
	return 0;
}

static int cecKeyPress(void *cbParam, const cec_keypress &key) {
	try {
		return ((CecCallback*) cbParam)->onCecKeyPress(key);
	} catch (...) {}
	return 0;
}

static int cecCommand(void *cbParam, const cec_command &command) {
	try {
		return ((CecCallback*) cbParam)->onCecCommand(command);
	} catch (...) {}
	return 0;
}

static int cecConfigurationChanged(void *cbParam, const libcec_configuration & configuration) {
	try {
		return ((CecCallback*) cbParam)->onCecConfigurationChanged(configuration);
	} catch (...) {}
	return 0;
}

struct ICECAdapterDeleter : std::default_delete<ICECAdapter> {
	//ICECAdapterDeleter() = default;

	void operator()(ICECAdapter* ptr) const {
		if (ptr)
			UnloadLibCec(ptr);
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

	return (ICECAdapter *)LibCecInitialise(&config);
}

Cec::Cec(const char * name, CecCallback * callback) :
	cec(CecInit(name, callback), ICECAdapterDeleter())
{

}

Cec::~Cec() {}


void Cec::open() {

	cerr << "Cec::open()" << endl;

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

	cerr << "Opened " << devices[0].path << endl;
}

void Cec::close() {
	cec->Close();
}

/**
 * Prints the name of all found adapters
 * This will close any open device!
 */
void Cec::listDevices() {
	cec_adapter devices[10];
	int8_t ret = cec->FindAdapters(devices, 10, NULL);
	if (ret < 0) {
		cout << "Error occurred searching for adapters" << endl;
		return;
	}

	if (ret == 0) {
		cout << "No adapters found" << endl;
	}

	for (int8_t i = 0; i < ret; i++) {
		cout << "[" << (int) i << "] port:" << devices[i].comm << " path:" << devices[i].path << endl;

		if (!cec->Open(devices[i].comm)) {
			cout << "\tFailed to open" << endl;
		}

		cec_logical_addresses devices = cec->GetActiveDevices();
		for (int j = 0; j < 16; j++) {
			if (devices[j]) {
				cec_logical_address logical_addres = (cec_logical_address) j;

				uint16_t physical_address = cec->GetDevicePhysicalAddress(logical_addres);
				cec_osd_name name = cec->GetDeviceOSDName(logical_addres);
				cec_vendor_id vendor = (cec_vendor_id) cec->GetDeviceVendorId(logical_addres);

				cout << "\t"  << cec->ToString(logical_addres)
				     << "@0x" << hex << physical_address
				     << " "   << name.name << " (" << cec->ToString(vendor) << ")"
				     << endl;
			}
		}
	}
}
