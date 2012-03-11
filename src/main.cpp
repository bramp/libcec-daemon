/**
 * libcec-daemon
 * A simple daemon to connect libcec to uinput.
 * by Andrew Brampton
 *
 * TODO
 *
 */
#include "uinput.h"
#include "libcec.h"

#include <cstdio>
#include <iostream>
#include <stdint.h>
#include <cstddef>
#include <csignal>
#include <vector>

using namespace CEC;

using std::cerr;
using std::endl;
using std::vector;

class Main : public CecCallback {

	private:

		Cec cec;
		UInput uinput;

		bool running; // TODO Change this to be threadsafe!. Voiatile or better

		Main();
		virtual ~Main();

		// Not implemented to avoid copying the singleton
		Main(Main const&);
		void operator=(Main const&);

		static void signalHandler(int sigNum);

		static const std::vector<__u16> & setupUinputMap();

	public:

		static const std::vector<__u16> uinputCecMap;

		int onCecLogMessage(const cec_log_message &message);
		int onCecKeyPress(const cec_keypress &key);
		int onCecCommand(const cec_command &command);
		int onCecConfigurationChanged(const libcec_configuration & configuration);

		static Main & instance();

		void loop();
		void stop();

		void listDevices();

};

const vector<__u16> Main::uinputCecMap = Main::setupUinputMap();

Main & Main::instance() {
	static Main main;
	return main;
}

Main::Main() : cec("Linux PC", this), uinput("libcec-daemon", uinputCecMap), running(true) {

	std::cerr << "Main::Main()" << std::endl;

	signal (SIGINT,  &Main::signalHandler);
	signal (SIGTERM, &Main::signalHandler);
}

Main::~Main() {
	stop();
}

void Main::loop() {
	cec.open();
	while (running) {
		cerr << "Loop" << endl;
		sleep(1);
	}
	cec.close();
}

void Main::stop() {
	running = false;
}

void Main::listDevices() {
	cec.listDevices();
}

void Main::signalHandler(int sigNum) {
	cerr << "SignalHanlder(" << sigNum << ")" << endl;

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
		uinputCecMap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION         ] = 0;
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
	cerr << message;

	return 1;
}

int Main::onCecKeyPress(const cec_keypress &key) {
	cerr << key;

	int uinputKey = 0;

	// Check bounds and find uinput code for this cec keypress
	if (key.keycode >= 0 && key.keycode <= CEC_USER_CONTROL_CODE_MAX)
		uinputKey = uinputCecMap[key.keycode];

	if (uinputKey != 0) {
		cerr << " sent " << uinputKey;

		uinput.send_event(EV_KEY, uinputKey, key.duration == 0 ? 1 : 0);
		uinput.sync();
	}

	cerr << endl;
	return 1;
}

int Main::onCecCommand(const cec_command & command) {
	//cerr << command;
	return 1;
}


int Main::onCecConfigurationChanged(const libcec_configuration & configuration) {
	//cerr << configuration;
	return 1;
}

int main (int argc, char *argv[]) {

	// TODO Parse the config

	//int daemon(int nochdir, int noclose);

	// Create the main
	Main & main = Main::instance();
	main.loop();

	/*
	cec_keypress test = {CEC_USER_CONTROL_CODE_NUMBER0, 1};
	main.onCecKeyPress(test);
	main.onCecKeyPress(test);
	main.onCecKeyPress(test);
	main.onCecKeyPress(test);
	main.onCecKeyPress(test);
	*/

	//main.listDevices();

	return 0;
}

