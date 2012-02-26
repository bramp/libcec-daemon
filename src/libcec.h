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

		const std::unique_ptr<CEC::ICECAdapter> cec;

		CEC::ICECAdapter * CecInit() const;

		virtual int onCecLogMessage(const CEC::cec_log_message &message) = 0;
		virtual int onCecKeyPress  (const CEC::cec_keypress &key) = 0;
		virtual int onCecCommand   (const CEC::cec_command command) = 0;
		virtual int onCecConfigurationChanged(const CEC::libcec_configuration & configuration) = 0;

	public:
		Cec();
		virtual ~Cec();
		//void listAdapters();

		void open();

	// These are just wrapper functions, to map C callbacks to C++
	friend int cecLogMessage (void *cbParam, const CEC::cec_log_message &message);
	friend int cecKeyPress   (void *cbParam, const CEC::cec_keypress &key);
	friend int cecCommand    (void *cbParam, const CEC::cec_command &command);
	friend int cecConfigurationChanged (void *cbParam, const CEC::libcec_configuration & configuration);
};
