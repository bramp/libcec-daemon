#include "libcec.h"

#include <iostream>
#include <cstddef>
#include <stdexcept>

using namespace CEC;

using std::cout;
using std::cerr;
using std::endl;

// cecloader has to be after some #includes and using namespaces :(
#include <cecloader.h>


int cecLogMessage(void *cbParam, const cec_log_message &message) {
	return ((Cec*)cbParam)->onCecLogMessage(message);
}

int cecKeyPress(void *cbParam, const cec_keypress &key) {
	return ((Cec*)cbParam)->onCecKeyPress(key);
}

int cecCommand(void *cbParam, const cec_command &command) {
	return ((Cec*)cbParam)->onCecCommand(command);
}

int cecConfigurationChanged(void *cbParam, const libcec_configuration & configuration) {
	return ((Cec*)cbParam)->onCecConfigurationChanged(configuration);
}

struct ICECAdapterDeleter : std::default_delete<ICECAdapter> {
	//ICECAdapterDeleter() = default;

	void operator()(ICECAdapter* ptr) const {
		if (ptr)
			UnloadLibCec(ptr);
	}
};

ICECAdapter * Cec::CecInit(const char * name) {
	config.Clear();

	config.deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
	strncpy(config.strDeviceName, name, sizeof(config.strDeviceName));

	callbacks.CBCecLogMessage           = &::cecLogMessage;
	callbacks.CBCecKeyPress             = &::cecKeyPress;
	callbacks.CBCecCommand              = &::cecCommand;
	callbacks.CBCecConfigurationChanged = &::cecConfigurationChanged;
	config.callbackParam                = this;
	config.callbacks                    = &callbacks;

	return (ICECAdapter *)LibCecInitialise(&config);
}

Cec::Cec(const char * name) : cec(CecInit(name), ICECAdapterDeleter()) {

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
    if ( !cec->Open(devices[0].comm) ) {
    	throw std::runtime_error("Failed to open adapter");
    }

    cerr << "Opened " << devices[0].path << endl;
}

void Cec::close() {
	cec->Close();
}

/**
 * Prints the name of all found adapters
 */
void Cec::listAdapters() {
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
		cout << "[" << (int)i << "] port:" << devices[i].comm << " path:" << devices[i].path << endl;
	}

}
