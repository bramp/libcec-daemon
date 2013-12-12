#include <cstddef>
#include <libcec/cec.h>

#include <memory>
#include <map>

class CecCallback {
	public:
		virtual ~CecCallback() {}

		// Virtual methods to handle callbacks
		virtual int onCecLogMessage(const CEC::cec_log_message & message) = 0;
		virtual int onCecKeyPress  (const CEC::cec_keypress & key) = 0;
		virtual int onCecCommand   (const CEC::cec_command & command) = 0;
		virtual int onCecConfigurationChanged(const CEC::libcec_configuration & configuration) = 0;
};

/**
 * Simple wrapper class around libcec
 */
class Cec {

	private:

		static std::map<CEC::cec_user_control_code, const char *> & setupUserControlCodeName();

		// Members for the libcec interface
		CEC::ICECCallbacks callbacks;
		CEC::libcec_configuration config;

		const std::unique_ptr<CEC::ICECAdapter> cec;

		// Inits the CECAdapter and returns it
		CEC::ICECAdapter * CecInit(const char * name, CecCallback *callback);

	public:

		const static std::map<CEC::cec_user_control_code, const char *> cecUserControlCodeName;

		Cec(const char *name, CecCallback *callback);
		virtual ~Cec();

		/**
		 * List all found adapters and prints them out
		 */
		std::ostream & listDevices(std::ostream & out);

		/**
		 * Opens the first adapter it finds
		 */
		void open();

		/**
		 * Closes the open adapter
		 */
		void close();

		void makeActive();

	// These are just wrapper functions, to map C callbacks to C++
	friend int cecLogMessage (void *cbParam, const CEC::cec_log_message &message);
	friend int cecKeyPress   (void *cbParam, const CEC::cec_keypress &key);
	friend int cecCommand    (void *cbParam, const CEC::cec_command &command);
	friend int cecConfigurationChanged (void *cbParam, const CEC::libcec_configuration & configuration);
};


// Some helper << methods
std::ostream& operator<<(std::ostream &out, const CEC::cec_log_message & message);
std::ostream& operator<<(std::ostream &out, const CEC::cec_keypress & key);
std::ostream& operator<<(std::ostream &out, const CEC::cec_command & command);
std::ostream& operator<<(std::ostream &out, const CEC::libcec_configuration & configuration);
