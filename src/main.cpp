/**
 * libcec-daemon
 * A simple daemon to connect libcec to uinput.
 * by Andrew Brampton
 */
#include "uinput.h"
#include "libcec.h"

#include <cstdio>
#include <iostream>
#include <stdint.h>
#include <cstddef>
#include <csignal>

using namespace CEC;

static int uinputCecMap[CEC_USER_CONTROL_CODE_MAX + 1];

class Main : Cec, UInput {

	private:

		static Main * main;
		static void signalHandler(int sigNum);

		bool running = true;

		Main();
		virtual ~Main();

		void setupUinputMap();

		int onCecLogMessage(const cec_log_message &message);
		int onCecKeyPress(const cec_keypress &key);
		int onCecCommand(const cec_command command);
		int onCecConfigurationChanged(const libcec_configuration & configuration);

	public:

		static Main * instance();

		void loop();
		void stop();

};

Main * Main::main = NULL;

Main * Main::instance() {
	if (Main::main == NULL)
		Main::main = new Main();
	return Main::main;
}

Main::Main() : Cec("Linux PC"), UInput("libcec-daemon") {

	signal (SIGINT,  &Main::signalHandler);
	signal (SIGTERM, &Main::signalHandler);
	signal (SIGKILL, &Main::signalHandler);

	setupUinputMap();
	Cec::open();
}

Main::~Main() {
	stop();
}

void Main::loop() {
	while (running) {
		sleep(1);
	}
}

void Main::stop() {
	running = false;
}


void Main::signalHandler(int sigNum) {
	Main * main = Main::instance();
	main->stop();
}

void Main::setupUinputMap() {
	memset(uinputCecMap, 0, sizeof(uinputCecMap));

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
	uinputCecMap[CEC_USER_CONTROL_CODE_SETUP_MENU                  ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_CONTENTS_MENU               ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_FAVORITE_MENU               ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_EXIT                        ] = 0;
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
	uinputCecMap[CEC_USER_CONTROL_CODE_POWER                       ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_VOLUME_UP                   ] = KEY_VOLUMEUP;
	uinputCecMap[CEC_USER_CONTROL_CODE_VOLUME_DOWN                 ] = KEY_VOLUMEDOWN;
	uinputCecMap[CEC_USER_CONTROL_CODE_MUTE                        ] = KEY_MUTE;
	uinputCecMap[CEC_USER_CONTROL_CODE_PLAY                        ] = KEY_PLAY;
	uinputCecMap[CEC_USER_CONTROL_CODE_STOP                        ] = KEY_STOP;
	uinputCecMap[CEC_USER_CONTROL_CODE_PAUSE                       ] = KEY_PAUSE;
	uinputCecMap[CEC_USER_CONTROL_CODE_RECORD                      ] = KEY_RECORD;
	uinputCecMap[CEC_USER_CONTROL_CODE_REWIND                      ] = KEY_REWIND;
	uinputCecMap[CEC_USER_CONTROL_CODE_FAST_FORWARD                ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_EJECT                       ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_FORWARD                     ] = 0;
	uinputCecMap[CEC_USER_CONTROL_CODE_BACKWARD                    ] = 0;
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
	uinputCecMap[CEC_USER_CONTROL_CODE_MAX                         ] = 0;
}

int Main::onCecLogMessage(const cec_log_message &message) {
	return 1;
}

int Main::onCecKeyPress(const cec_keypress &key) {
	int uinputKey = 0;

	// Check bounds and find uinput code for this cec keypress
	if (key.keycode >= 0 && key.keycode <= CEC_USER_CONTROL_CODE_MAX)
		uinputKey = uinputCecMap[key.keycode];

	if (uinputKey != 0) {
		send_event(EV_KEY, uinputKey, 1);
		sync();
	}

	return 1;
}

int Main::onCecCommand(const cec_command command) {
	return 1;
}
int Main::onCecConfigurationChanged(const libcec_configuration & configuration) {
	return 1;
}

int main (int argc, char *argv[]) {

	// TODO Parse the config

	int daemon(int nochdir, int noclose);

	// Create the main
	Main * main = Main::instance();
	main->loop();

	return 0;
}

