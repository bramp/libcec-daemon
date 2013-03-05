/**
 * libcec-daemon
 * A simple daemon to connect libcec to uinput. That is, using your TV to control your PC! 
 * by Andrew Brampton
 *
 * TODO
 *
 */
#include "main.h"

#define VERSION     "libcec-daemon v0.9"

#define CEC_NAME    "linux PC"
#define UINPUT_NAME "libcec-daemon"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <csignal>
#include <vector>

#include <boost/program_options.hpp>
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

static Logger logger = Logger::getInstance("main");

const vector<__u16> Main::uinputCecMap = Main::setupUinputMap();

Main & Main::instance() {
	// Singleton pattern so we can use main from a sighandle
	static Main main;
	return main;
}

Main::Main() : cec(CEC_NAME, this), uinput(UINPUT_NAME, uinputCecMap), running(true) {
	LOG4CPLUS_TRACE_STR(logger, "Main::Main()");

	signal (SIGINT,  &Main::signalHandler);
	signal (SIGTERM, &Main::signalHandler);
}

Main::~Main() {
	LOG4CPLUS_TRACE_STR(logger, "Main::~Main()");
	stop();
}

void Main::loop() {
	LOG4CPLUS_TRACE_STR(logger, "Main::loop()");

	cec.open();
	while (running) {
		LOG4CPLUS_TRACE_STR(logger, "Loop");
		sleep(1);
	}
	cec.close();
}

void Main::stop() {
	LOG4CPLUS_TRACE_STR(logger, "Main::stop()");
	running = false;
}

void Main::listDevices() {
	LOG4CPLUS_TRACE_STR(logger, "Main::listDevices()");
	cec.listDevices(cout);
}

void Main::signalHandler(int sigNum) {
	LOG4CPLUS_DEBUG_STR(logger, "Main::signalHandler()");

	Main::instance().stop();
}

const std::vector<__u16> & Main::setupUinputMap() {
	static std::vector<__u16> uinputCecMap;

	if (uinputCecMap.empty()) {
		uinputCecMap.resize(CEC_USER_CONTROL_CODE_MAX + 1, 0);
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT                      ] = KEY_ENTER;
		uinputCecMap[CEC_USER_CONTROL_CODE_UP                          ] = KEY_UP;
		uinputCecMap[CEC_USER_CONTROL_CODE_DOWN                        ] = KEY_DOWN;
		uinputCecMap[CEC_USER_CONTROL_CODE_LEFT                        ] = KEY_LEFT;
		uinputCecMap[CEC_USER_CONTROL_CODE_RIGHT                       ] = KEY_RIGHT;
		uinputCecMap[CEC_USER_CONTROL_CODE_RIGHT_UP                    ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN                  ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_LEFT_UP                     ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_LEFT_DOWN                   ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_ROOT_MENU                   ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_SETUP_MENU                  ] = KEY_SETUP;
		uinputCecMap[CEC_USER_CONTROL_CODE_CONTENTS_MENU               ] = KEY_MENU;
		uinputCecMap[CEC_USER_CONTROL_CODE_FAVORITE_MENU               ] = KEY_FAVORITES;
		uinputCecMap[CEC_USER_CONTROL_CODE_EXIT                        ] = KEY_BACKSPACE;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER0                     ] = KEY_0;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER1                     ] = KEY_1;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER2                     ] = KEY_2;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER3                     ] = KEY_3;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER4                     ] = KEY_4;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER5                     ] = KEY_5;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER6                     ] = KEY_6;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER7                     ] = KEY_7;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER8                     ] = KEY_8;
		uinputCecMap[CEC_USER_CONTROL_CODE_NUMBER9                     ] = KEY_9;
		uinputCecMap[CEC_USER_CONTROL_CODE_DOT                         ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_ENTER                       ] = KEY_ENTER;
		uinputCecMap[CEC_USER_CONTROL_CODE_CLEAR                       ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_NEXT_FAVORITE               ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_CHANNEL_UP                  ] = KEY_CHANNELUP;
		uinputCecMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN                ] = KEY_CHANNELDOWN;
		uinputCecMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL            ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_SOUND_SELECT                ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_INPUT_SELECT                ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION         ] = KEY_INFO;
		uinputCecMap[CEC_USER_CONTROL_CODE_HELP                        ] = KEY_HELP;
		uinputCecMap[CEC_USER_CONTROL_CODE_PAGE_UP                     ] = KEY_PAGEUP;
		uinputCecMap[CEC_USER_CONTROL_CODE_PAGE_DOWN                   ] = KEY_PAGEDOWN;
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER                       ] = KEY_POWER;
		uinputCecMap[CEC_USER_CONTROL_CODE_VOLUME_UP                   ] = KEY_VOLUMEUP;
		uinputCecMap[CEC_USER_CONTROL_CODE_VOLUME_DOWN                 ] = KEY_VOLUMEDOWN;
		uinputCecMap[CEC_USER_CONTROL_CODE_MUTE                        ] = KEY_MUTE;
		uinputCecMap[CEC_USER_CONTROL_CODE_PLAY                        ] = KEY_PLAY;
		uinputCecMap[CEC_USER_CONTROL_CODE_STOP                        ] = KEY_STOP;
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE                       ] = KEY_PAUSE;
		uinputCecMap[CEC_USER_CONTROL_CODE_RECORD                      ] = KEY_RECORD;
		uinputCecMap[CEC_USER_CONTROL_CODE_REWIND                      ] = KEY_REWIND;
		uinputCecMap[CEC_USER_CONTROL_CODE_FAST_FORWARD                ] = KEY_FASTFORWARD;
		uinputCecMap[CEC_USER_CONTROL_CODE_EJECT                       ] = KEY_EJECTCD;
		uinputCecMap[CEC_USER_CONTROL_CODE_FORWARD                     ] = KEY_NEXTSONG;
		uinputCecMap[CEC_USER_CONTROL_CODE_BACKWARD                    ] = KEY_PREVIOUSSONG;
		uinputCecMap[CEC_USER_CONTROL_CODE_STOP_RECORD                 ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD                ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_ANGLE                       ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_SUB_PICTURE                 ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND             ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING           ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION       ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_PLAY_FUNCTION               ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_RECORD_FUNCTION             ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_STOP_FUNCTION               ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_MUTE_FUNCTION               ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_TUNE_FUNCTION               ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION    ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION          ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION           ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_F1_BLUE                     ] = KEY_BLUE;
		uinputCecMap[CEC_USER_CONTROL_CODE_F2_RED                      ] = KEY_RED;
		uinputCecMap[CEC_USER_CONTROL_CODE_F3_GREEN                    ] = KEY_GREEN;
		uinputCecMap[CEC_USER_CONTROL_CODE_F4_YELLOW                   ] = KEY_YELLOW;
		uinputCecMap[CEC_USER_CONTROL_CODE_F5                          ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_DATA                        ] = 0;
		uinputCecMap[CEC_USER_CONTROL_CODE_AN_RETURN                   ] = 0;
	}

	return uinputCecMap;
}

int Main::onCecLogMessage(const cec_log_message &message) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecLogMessage(" << message << ")");
	return 1;
}

int Main::onCecKeyPress(const cec_keypress &key) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecKeyPress(" << key << ")");

	int uinputKey = 0;

	// Check bounds and find uinput code for this cec keypress
	if (key.keycode >= 0 && key.keycode <= CEC_USER_CONTROL_CODE_MAX)
		uinputKey = uinputCecMap[key.keycode];

	if (uinputKey != 0) {
		LOG4CPLUS_DEBUG(logger, "sent " << uinputKey);

		uinput.send_event(EV_KEY, uinputKey, key.duration == 0 ? 2 : 0);
		uinput.sync();
	}

	return 1;
}

int Main::onCecCommand(const cec_command & command) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecCommand(" << command << ")");
	return 1;
}


int Main::onCecConfigurationChanged(const libcec_configuration & configuration) {
	LOG4CPLUS_DEBUG(logger, "Main::onCecConfigurationChanged(" << configuration << ")");
	return 1;
}

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
	    ("usb", po::value<std::string>(), "USB adapter path (as shown by --list)")
	;

	po::positional_options_description p;
	p.add("usb", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << "Usage: " << argv[0] << " [-h] [-v] [-d] [usb]" << endl << endl;
	    cout << desc << endl;
	    return 0;
	}

	if (vm.count("version")) {
		cout << VERSION << endl;
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

		if (vm.count("list")) {
			main.listDevices();
			return 0;
		}

		if (vm.count("usb")) {
			cout << vm["usb"].as< string >() << endl;
		}

		if (vm.count("daemon")) {
			daemon(0, 0);
		}

		main.loop();

	} catch (std::exception & e) {
		cerr << e.what() << endl;
		return -1;
	}

	return 0;
}

