#include <cec.h>

#include <memory>
//#include <boost/shared_ptr.hpp>

// These are just wrapper functions, to map C callbacks to C++
int cecLogMessage(void *cbParam, const CEC::cec_log_message &message);
int cecKeyPress  (void *cbParam, const CEC::cec_keypress &key);
int cecCommand   (void *cbParam, const CEC::cec_command &command);
int cecConfigurationChanged(void *cbParam, const CEC::libcec_configuration & configuration);

class Cec {

	private:

		CEC::ICECCallbacks callbacks;
		CEC::libcec_configuration config;

		const std::unique_ptr<CEC::ICECAdapter> cec;

		// Inits the CECAdapter and returns it
		CEC::ICECAdapter * CecInit(const char * name);

		// Virtual methods to handle callbacks
		virtual int onCecLogMessage(const CEC::cec_log_message &message) = 0;
		virtual int onCecKeyPress  (const CEC::cec_keypress &key) = 0;
		virtual int onCecCommand   (const CEC::cec_command command) = 0;
		virtual int onCecConfigurationChanged(const CEC::libcec_configuration & configuration) = 0;

	public:
		Cec(const char *name);
		virtual ~Cec();

		/**
		 * List all found adapters and prints them out
		 */
		void listAdapters();

		void open();
		void close();

	// These are just wrapper functions, to map C callbacks to C++
	friend int cecLogMessage (void *cbParam, const CEC::cec_log_message &message);
	friend int cecKeyPress   (void *cbParam, const CEC::cec_keypress &key);
	friend int cecCommand    (void *cbParam, const CEC::cec_command &command);
	friend int cecConfigurationChanged (void *cbParam, const CEC::libcec_configuration & configuration);
};
