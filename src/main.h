#include "uinput.h"
#include "libcec.h"
#include <limits.h>

class Main : public CecCallback {

	private:

		// Main controls
		Cec cec;
		UInput uinput;
		char cec_name[HOST_NAME_MAX];

		// Some config params
		bool makeActive;

		//
		bool running; // TODO Change this to be threadsafe!. Voiatile or better

		//
		Main();
		virtual ~Main();

		// Not implemented to avoid copying the singleton
		Main(Main const&);
		void operator=(Main const&);

		static void signalHandler(int sigNum);

		static const std::vector<__u16> & setupUinputMap();
		
		char *getCecName();

	public:

		static const std::vector<__u16> uinputCecMap;

		int onCecLogMessage(const CEC::cec_log_message &message);
		int onCecKeyPress(const CEC::cec_keypress &key);
		int onCecCommand(const CEC::cec_command &command);
		int onCecConfigurationChanged(const CEC::libcec_configuration & configuration);

		static Main & instance();

		void loop();
		void stop();

		void listDevices();

		void setMakeActive(bool active) {this->makeActive = active;};
};
