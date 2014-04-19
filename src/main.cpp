/**
 * libcec-daemon
 * A simple daemon to connect libcec to uinput. That is, using your TV to control your PC! 
 * by Andrew Brampton
 *
 * TODO
 *
 */
#include "main.h"
#include "config.h"
#include "hdmi.h"

#define CEC_NAME    "linux PC"
#define UINPUT_NAME "libcec-daemon"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <csignal>
#include <cstdlib>
#include <vector>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include "accumulator.hpp"

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

using namespace CEC;
using namespace log4cplus;

using std::cout;
using std::cerr;
using std::endl;
using std::min;
using std::string;
using std::vector;
using std::queue;
using std::list;

static Logger logger = Logger::getInstance("main");
static boost::mutex libcec_sync;
static boost::condition_variable libcec_cond;

const vector<list<__u16>> Main::uinputCecMap = Main::setupUinputMap();

enum
{
	COMMAND_STANDBY,
	COMMAND_ACTIVE,
	COMMAND_INACTIVE,
	COMMAND_RESTART,
	COMMAND_KEYPRESS,
	COMMAND_KEYRELEASE,
	COMMAND_EXIT,
};

Main & Main::instance() {
	// Singleton pattern so we can use main from a sighandle
	static Main main;
	return main;
}

Main::Main() : cec(getCecName(), this), uinput(UINPUT_NAME, uinputCecMap),
	makeActive(true), running(false), lastUInputKeys({ }), logicalAddress(CECDEVICE_UNKNOWN)
{
	LOG4CPLUS_TRACE_STR(logger, "Main::Main()");

}

Main::~Main() {
	LOG4CPLUS_TRACE_STR(logger, "Main::~Main()");
	stop();
}

void Main::loop(const string & device) {
	LOG4CPLUS_TRACE_STR(logger, "Main::loop()");

	struct sigaction action;

	action.sa_handler = &Main::signalHandler;
	action.sa_flags = SA_RESETHAND;
	sigemptyset(&action.sa_mask);

	int restart = false;

	do
	{
		cec.open(device);

		running = true;

		/* install signals */
		sigaction (SIGHUP,  &action, NULL);
		sigaction (SIGINT,  &action, NULL);
		sigaction (SIGTERM, &action, NULL);

		if (makeActive) {
			cec.makeActive();
		}

		do
		{
			boost::unique_lock<boost::mutex> libcec_lock(libcec_sync);

			while( running && !commands.empty() )
			{
				Command cmd = commands.front();
				switch( cmd.command )
				{
					case COMMAND_STANDBY:
						if( ! onStandbyCommand.empty() )
						{
							LOG4CPLUS_DEBUG(logger, "Standby: Running \"" << onStandbyCommand << "\"");
							int ret = system(onStandbyCommand.c_str());
                            if( ret )
                                LOG4CPLUS_ERROR(logger, "Standby command failed: " << ret);
							
						}
						else
						{
							onCecKeyPress( CEC_USER_CONTROL_CODE_POWER );
						}
						break;
					case COMMAND_ACTIVE:
						makeActive = true;
						if( ! onActivateCommand.empty() )
						{
							LOG4CPLUS_DEBUG(logger, "Activated: Running \"" << onActivateCommand << "\"");
							int ret = system(onActivateCommand.c_str());
                            if( ret )
                                LOG4CPLUS_ERROR(logger, "Activate command failed: " << ret);
						}
						break;
					case COMMAND_INACTIVE:
						makeActive = false;
						if( ! onDeactivateCommand.empty() )
						{
							LOG4CPLUS_DEBUG(logger, "Deactivated: Running \"" << onDeactivateCommand << "\"");
							int ret = system(onDeactivateCommand.c_str());
                            if( ret )
                                LOG4CPLUS_ERROR(logger, "Deactivate command failed: " << ret);
						}
						break;
					case COMMAND_KEYPRESS:
						onCecKeyPress( cmd.keycode );
						break;
					case COMMAND_RESTART:
						running = false;
						restart = true;
						break;
					case COMMAND_EXIT:
						running = false;
						break;
				}
				commands.pop();
			}
			while( running && !libcec_cond.timed_wait(libcec_lock, boost::posix_time::seconds(43)) )
			{
				running = cec.ping();
			}
		}
		while( running );

		/* reset signals */
		signal (SIGHUP,  SIG_DFL);
		signal (SIGINT,  SIG_DFL);
		signal (SIGTERM, SIG_DFL);

		cec.close(!restart);
	}
	while( restart );
}

void Main::push(Command cmd) {
	boost::lock_guard<boost::mutex> lock(libcec_sync);
	if( running )
	{
		commands.push(cmd);
		libcec_cond.notify_one();
	}
}

void Main::stop() {
	LOG4CPLUS_TRACE_STR(logger, "Main::stop()");
	push(Command(COMMAND_EXIT));
}

void Main::restart() {
	LOG4CPLUS_TRACE_STR(logger, "Main::restart()");
	push(Command(COMMAND_RESTART));
}

void Main::listDevices() {
	LOG4CPLUS_TRACE_STR(logger, "Main::listDevices()");
	cec.listDevices(cout);
}

void Main::signalHandler(int sigNum) {
	LOG4CPLUS_DEBUG_STR(logger, "Main::signalHandler()");
	switch( sigNum )
	{
		case SIGHUP:
			Main::instance().restart();
			break;
		default:
			Main::instance().stop();
			break;
	}
}

char *Main::getCecName() {
	LOG4CPLUS_TRACE_STR(logger, "Main::getCecName()");
	if (gethostname(cec_name,HOST_NAME_MAX) < 0 ) {
		LOG4CPLUS_TRACE_STR(logger, "Main::getCecName()");
		strncpy(cec_name, CEC_NAME, sizeof(HOST_NAME_MAX));
	}

	return cec_name;
}

const std::vector<list<__u16>> & Main::setupUinputMap() {
	static std::vector<list<__u16>> uinputCecMap;

	if (uinputCecMap.empty()) {
		uinputCecMap.resize(CEC_USER_CONTROL_CODE_MAX + 1, {});
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT                      ] = { KEY_OK };
		uinputCecMap[CEC_USER_CONTROL_CODE_UP                          ] = { KEY_UP };
		uinputCecMap[CEC_USER_CONTROL_CODE_DOWN                        ] = { KEY_DOWN };
		uinputCecMap[CEC_USER_CONTROL_CODE_LEFT                        ] = { KEY_LEFT };
		uinputCecMap[CEC_USER_CONTROL_CODE_RIGHT                       ] = { KEY_RIGHT };
		uinputCecMap[CEC_USER_CONTROL_CODE_RIGHT_UP                    ] = { KEY_RIGHT, KEY_UP };
		uinputCecMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN                  ] = { KEY_RIGHT, KEY_DOWN };
		uinputCecMap[CEC_USER_CONTROL_CODE_LEFT_UP                     ] = { KEY_LEFT, KEY_UP };
		uinputCecMap[CEC_USER_CONTROL_CODE_LEFT_DOWN                   ] = { KEY_RIGHT, KEY_UP };
		uinputCecMap[CEC_USER_CONTROL_CODE_ROOT_MENU                   ] = { KEY_HOME };
		uinputCecMap[CEC_USER_CONTROL_CODE_SETUP_MENU                  ] = { KEY_SETUP };
		uinputCecMap[CEC_USER_CONTROL_CODE_CONTENTS_MENU               ] = { KEY_MENU };
		uinputCecMap[CEC_USER_CONTROL_CODE_FAVORITE_MENU               ] = { KEY_FAVORITES };
		uinputCecMap[CEC_USER_CONTROL_CODE_EXIT                        ] = { KEY_EXIT };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER0                     ] = { KEY_0 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER1                     ] = { KEY_1 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER2                     ] = { KEY_2 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER3                     ] = { KEY_3 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER4                     ] = { KEY_4 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER5                     ] = { KEY_5 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER6                     ] = { KEY_6 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER7                     ] = { KEY_7 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER8                     ] = { KEY_8 };
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER9                     ] = { KEY_9 };
		uinputCecMap[CEC_USER_CONTROL_CODE_DOT                         ] = { KEY_DOT };
		uinputCecMap[CEC_USER_CONTROL_CODE_ENTER                       ] = { KEY_ENTER };
		uinputCecMap[CEC_USER_CONTROL_CODE_CLEAR                       ] = { KEY_BACKSPACE };
		uinputCecMap[CEC_USER_CONTROL_CODE_NEXT_FAVORITE               ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_CHANNEL_UP                  ] = { KEY_CHANNELUP };
		uinputCecMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN                ] = { KEY_CHANNELDOWN };
		uinputCecMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL            ] = { KEY_PREVIOUS };
		uinputCecMap[CEC_USER_CONTROL_CODE_SOUND_SELECT                ] = { KEY_SOUND };
		uinputCecMap[CEC_USER_CONTROL_CODE_INPUT_SELECT                ] = { KEY_TUNER };
		uinputCecMap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION         ] = { KEY_INFO };
		uinputCecMap[CEC_USER_CONTROL_CODE_HELP                        ] = { KEY_HELP };
		uinputCecMap[CEC_USER_CONTROL_CODE_PAGE_UP                     ] = { KEY_PAGEUP };
		uinputCecMap[CEC_USER_CONTROL_CODE_PAGE_DOWN                   ] = { KEY_PAGEDOWN };
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER                       ] = { KEY_POWER };
		uinputCecMap[CEC_USER_CONTROL_CODE_VOLUME_UP                   ] = { KEY_VOLUMEUP };
		uinputCecMap[CEC_USER_CONTROL_CODE_VOLUME_DOWN                 ] = { KEY_VOLUMEDOWN };
		uinputCecMap[CEC_USER_CONTROL_CODE_MUTE                        ] = { KEY_MUTE };
		uinputCecMap[CEC_USER_CONTROL_CODE_PLAY                        ] = { KEY_PLAY };
		uinputCecMap[CEC_USER_CONTROL_CODE_STOP                        ] = { KEY_STOP };
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE                       ] = { KEY_PAUSE };
		uinputCecMap[CEC_USER_CONTROL_CODE_RECORD                      ] = { KEY_RECORD };
		uinputCecMap[CEC_USER_CONTROL_CODE_REWIND                      ] = { KEY_REWIND };
		uinputCecMap[CEC_USER_CONTROL_CODE_FAST_FORWARD                ] = { KEY_FASTFORWARD };
		uinputCecMap[CEC_USER_CONTROL_CODE_EJECT                       ] = { KEY_EJECTCD };
		uinputCecMap[CEC_USER_CONTROL_CODE_FORWARD                     ] = { KEY_FORWARD };
		uinputCecMap[CEC_USER_CONTROL_CODE_BACKWARD                    ] = { KEY_BACK };
		uinputCecMap[CEC_USER_CONTROL_CODE_STOP_RECORD                 ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD                ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_ANGLE                       ] = { KEY_SCREEN };
		uinputCecMap[CEC_USER_CONTROL_CODE_SUB_PICTURE                 ] = { KEY_SUBTITLE };
		uinputCecMap[CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND             ] = { KEY_VIDEO };
		uinputCecMap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    ] = { KEY_EPG };
		uinputCecMap[CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING           ] = { KEY_TIME };
		uinputCecMap[CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION       ] = { KEY_CONFIG };
		uinputCecMap[CEC_USER_CONTROL_CODE_PLAY_FUNCTION               ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_RECORD_FUNCTION             ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_STOP_FUNCTION               ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_MUTE_FUNCTION               ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_TUNE_FUNCTION               ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       ] = { KEY_MEDIA };
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION    ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION          ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION           ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_F1_BLUE                     ] = { KEY_BLUE };
		uinputCecMap[CEC_USER_CONTROL_CODE_F2_RED                      ] = { KEY_RED };
		uinputCecMap[CEC_USER_CONTROL_CODE_F3_GREEN                    ] = { KEY_GREEN };
		uinputCecMap[CEC_USER_CONTROL_CODE_F4_YELLOW                   ] = { KEY_YELLOW };
		uinputCecMap[CEC_USER_CONTROL_CODE_F5                          ] = { };
		uinputCecMap[CEC_USER_CONTROL_CODE_DATA                        ] = { KEY_TEXT };
		uinputCecMap[CEC_USER_CONTROL_CODE_AN_RETURN                   ] = { KEY_ESC };
		uinputCecMap[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST            ] = { KEY_LIST };
	}

	return uinputCecMap;
}

int Main::onCecLogMessage(const cec_log_message &message) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecLogMessage(" << message << ")");
	return 1;
}

int Main::onCecKeyPress(const cec_keypress &key) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecKeyPress(" << key << ")");

	// Check bounds and find uinput code for this cec keypress
	if (key.keycode >= 0 && key.keycode <= CEC_USER_CONTROL_CODE_MAX) {
		const list<__u16> & uinputKeys = uinputCecMap[key.keycode];

		if ( !uinputKeys.empty() ) {
			if( key.duration == 0 ) {
				if( uinputKeys == lastUInputKeys )
				{
					/*
					** KEY REPEAT
					*/
					for (std::list<__u16>::const_iterator ukeys = uinputKeys.begin(); ukeys != uinputKeys.end(); ++ukeys) {
						__u16 ukey = *ukeys;

						LOG4CPLUS_DEBUG(logger, "repeat " << ukey);

						uinput.send_event(EV_KEY, ukey, EV_KEY_REPEAT);
					}
				}
				else
				{
					/*
					** KEY PRESSED
					*/
					if( ! lastUInputKeys.empty() )
					{
						/* what happened with the last key release ? */
						for (std::list<__u16>::const_iterator ukeys = lastUInputKeys.begin(); ukeys != lastUInputKeys.end(); ++ukeys) {
							__u16 ukey = *ukeys;

							LOG4CPLUS_DEBUG(logger, "release " << ukey);

							uinput.send_event(EV_KEY, ukey, EV_KEY_RELEASED);
						}
					}
					for (std::list<__u16>::const_iterator ukeys = uinputKeys.begin(); ukeys != uinputKeys.end(); ++ukeys) {
						__u16 ukey = *ukeys;

						LOG4CPLUS_DEBUG(logger, "send " << ukey);

						uinput.send_event(EV_KEY, ukey, EV_KEY_PRESSED);
					}
					lastUInputKeys = uinputKeys;
				}
			}
			else {
				if( lastUInputKeys != uinputKeys ) {
					if( ! lastUInputKeys.empty() ) {
						/* what happened with the last key release ? */
						for (std::list<__u16>::const_iterator ukeys = lastUInputKeys.begin(); ukeys != lastUInputKeys.end(); ++ukeys) {
							__u16 ukey = *ukeys;

							LOG4CPLUS_DEBUG(logger, "release " << ukey);

							uinput.send_event(EV_KEY, ukey, EV_KEY_RELEASED);
						}
					}
					for (std::list<__u16>::const_iterator ukeys = uinputKeys.begin(); ukeys != uinputKeys.end(); ++ukeys) {
						__u16 ukey = *ukeys;

						LOG4CPLUS_DEBUG(logger, "send " << ukey);

						uinput.send_event(EV_KEY, ukey, EV_KEY_PRESSED);
					}
					boost::this_thread::sleep(boost::posix_time::milliseconds(100));
				}
				/*
				** KEY RELEASED
				*/
				for (std::list<__u16>::const_iterator ukeys = uinputKeys.begin(); ukeys != uinputKeys.end(); ++ukeys) {
					__u16 ukey = *ukeys;

					LOG4CPLUS_DEBUG(logger, "release " << ukey);

					uinput.send_event(EV_KEY, ukey, EV_KEY_RELEASED);

				}
				lastUInputKeys.clear();
			}
			uinput.sync();
		}
	}

	return 1;
}

int Main::onCecKeyPress(const cec_user_control_code & keycode) {
	cec_keypress key = { .keycode=keycode };

	/* PUSH KEY */
	key.duration = 0;
	onCecKeyPress( key );

	/* simulate delay */
	key.duration = 100;
	boost::this_thread::sleep(boost::posix_time::milliseconds(key.duration));

	/* RELEASE KEY */
	onCecKeyPress( key );

	return 1;
}

int Main::onCecCommand(const cec_command & command) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecCommand(" << command << ")");
	switch( command.opcode )
	{
		case CEC_OPCODE_STANDBY:
			if( (command.initiator == CECDEVICE_TV)
                         && ( (command.destination == CECDEVICE_BROADCAST) || (command.destination == logicalAddress))  )
			{
				push(Command(COMMAND_STANDBY));
			}
			break;
		case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
			if( (command.initiator == CECDEVICE_TV)
                         && ( (command.destination == CECDEVICE_BROADCAST) || (command.destination == logicalAddress))  )
			{
                if( makeActive )
                {
                    /* remind TV we are active */
                    push(Command(COMMAND_ACTIVE));
                }
			}
		case CEC_OPCODE_SET_MENU_LANGUAGE:
			if( (command.initiator == CECDEVICE_TV) && (command.parameters.size == 3)
                         && ( (command.destination == CECDEVICE_BROADCAST) || (command.destination == logicalAddress))  )
			{
				/* TODO */
			}
			break;
		case CEC_OPCODE_DECK_CONTROL:
			if( (command.initiator == CECDEVICE_TV) && (command.parameters.size == 1)
                         && ( (command.destination == CECDEVICE_BROADCAST) || (command.destination == logicalAddress))  )
			{
				if( command.parameters[0] == CEC_DECK_CONTROL_MODE_STOP ) {
					push(Command(COMMAND_KEYPRESS, CEC_USER_CONTROL_CODE_STOP));
				}
			}
			break;
		case CEC_OPCODE_PLAY:
			if( (command.initiator == CECDEVICE_TV) && (command.parameters.size == 1)
                         && ( (command.destination == CECDEVICE_BROADCAST) || (command.destination == logicalAddress))  )
			{
				if( command.parameters[0] == CEC_PLAY_MODE_PLAY_FORWARD ) {
					push(Command(COMMAND_KEYPRESS, CEC_USER_CONTROL_CODE_PLAY));
				}
				else if( command.parameters[0] == CEC_PLAY_MODE_PLAY_STILL ) {
					push(Command(COMMAND_KEYPRESS, CEC_USER_CONTROL_CODE_PAUSE));
				}
			}
			break;
		default:
			break;
	}
	return 1;
}

int Main::onCecAlert(const CEC::libcec_alert alert, const CEC::libcec_parameter & param) {
	LOG4CPLUS_ERROR(logger, "Main::onCecAlert(alert=" << alert << ")");
	switch( alert )
	{
		case CEC_ALERT_SERVICE_DEVICE:
			break;
		case CEC_ALERT_CONNECTION_LOST:
		case CEC_ALERT_PERMISSION_ERROR:
		case CEC_ALERT_PORT_BUSY:
		case CEC_ALERT_PHYSICAL_ADDRESS_ERROR:
		case CEC_ALERT_TV_POLL_FAILED:
			Main::instance().restart();
			break;
		default:
			break;
	}
	return 1;
}

int Main::onCecConfigurationChanged(const libcec_configuration & configuration) {
	//LOG4CPLUS_DEBUG(logger, "Main::onCecConfigurationChanged(" << configuration << ")");
	LOG4CPLUS_DEBUG(logger, "Main::onCecConfigurationChanged(logicalAddress=" << configuration.logicalAddresses.primary << ")");
	logicalAddress = configuration.logicalAddresses.primary;
	return 1;
}


int Main::onCecMenuStateChanged(const cec_menu_state & menu_state) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecMenuStateChanged(" << menu_state << ")");

	return onCecKeyPress(CEC_USER_CONTROL_CODE_CONTENTS_MENU);
}

void Main::onCecSourceActivated(const cec_logical_address & address, bool bActivated) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecSourceActivated(logicalAddress " << address << " = " << bActivated << ")");
	if( logicalAddress == address )
	{
		if( bActivated )
		{
			push(Command(COMMAND_ACTIVE));
		}
		else
		{	
			push(Command(COMMAND_INACTIVE));
		}
	}
}

#if defined(HAVE_BOOST_PO_TYPED_VALUE_NAME)

/*
** boost versions 1.50 and newer support value_name() program option value
*/

using boost::program_options::value;

#else

/*
** provide a value_name() replacement for older versions
*/

template<class T> class typed_value_name : public boost::program_options::typed_value<T>
{
    public:
        typed_value_name(T *my_value = new T()) : boost::program_options::typed_value<T>(my_value), val_name("arg") {};
        boost::program_options::typed_value<T> *value_name(const char *name) { val_name = name;; return this; };
        std::string name() const { return std::string(val_name); };

    protected:
        const char *val_name;
};

template<class T> typed_value_name<T> *value(T *my_value = new T()) { return new typed_value_name<T>(my_value); };

#endif

int main (int argc, char *argv[]) {

    BasicConfigurator config;
    config.configure();

    int loglevel = 0;

	namespace po = boost::program_options;

	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help,h",    "show help message")
	    ("version,V", "show version (and exit)")
	    ("daemon,d",  "daemon mode, run in background")
	    ("list,l",    "list available CEC adapters and devices")
	    ("verbose,v", accumulator<int>(&loglevel)->implicit_value(1), "verbose output (use -vv for more)")
	    ("quiet,q",   "quiet output (print almost nothing)")
	    ("donotactivate,a", "do not activate device on startup")
	    ("onstandby", value<string>()->value_name("<path>"),  "command to run on standby")
	    ("onactivate", value<string>()->value_name("<path>"),  "command to run on activation")
	    ("ondeactivate", value<string>()->value_name("<path>"),  "command to run on deactivation")
	    ("port,p", value<HDMI::address>()->value_name("[a[.b.c.d]>"),  "HDMI port A or address A.B.C.D (overrides autodetected value)")
	    ("usb", value<string>()->value_name("<path>"), "USB adapter path (as shown by --list)")
	;

	po::positional_options_description p;
	p.add("usb", 1);

    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    }
    catch( po::error &e )
    {
        cerr << argv[0] << ": " << e.what() << endl;
        cerr << "Type \"" << argv[0] << " --help\" for more information." << endl;
        return 1;
    }
    po::notify(vm);

	if (vm.count("help")) {
		cout << "Usage: " << argv[0] << " [options] [usb]" << endl << endl;
	    cout << desc << endl;
	    return 0;
	}

	if (vm.count("version")) {
		cout << PACKAGE_STRING << endl;
		return 0;
	}

	if(vm.count("quiet")) {
		loglevel = -1;
	} else {
		loglevel = min(loglevel, 2);
	}

	Logger root = Logger::getRoot();
	switch (loglevel) {
		case 2:  root.setLogLevel(TRACE_LOG_LEVEL); break;
		case 1:  root.setLogLevel(DEBUG_LOG_LEVEL); break;
		default: root.setLogLevel(INFO_LOG_LEVEL); break;
		case -1: root.setLogLevel(FATAL_LOG_LEVEL); break;
	}

	try {
		// Create the main
		Main & main = Main::instance();
        string device = "";

		if (vm.count("list")) {
			main.listDevices();
			return 0;
		}

		if (vm.count("donotactivate")) {
			main.setMakeActive(false);
		}

		if (vm.count("usb")) {
			device = vm["usb"].as< string >();
		}

		if (vm.count("onstandby")) {
			main.setOnStandbyCommand(vm["onstandby"].as< string >());
		}

		if (vm.count("onactivate")) {
			main.setOnActivateCommand(vm["onactivate"].as< string >());
		}

		if (vm.count("ondeactivate")) {
			main.setOnDeactivateCommand(vm["ondeactivate"].as< string >());
		}

		if (vm.count("port")) {
            main.setTargetAddress(vm["port"].as< HDMI::address >());
        }

        if (vm.count("daemon")) {
            if( daemon(0, 0) )
                return -1;
        }

		main.loop(device);

	} catch (std::exception & e) {
		cerr << e.what() << endl;
		return -1;
	}

	return 0;
}

