#include "libcec.h"

#include <iostream>
#include <cstddef>

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

// Cleans up after CECInit
void cecDeleter(ICECAdapter* ptr) {
	if (ptr)
		CECDestroy(ptr);
}

//std::default_delete<ICECAdapter>

struct ICECAdapterDeleter : std::default_delete<ICECAdapter> {
	//ICECAdapterDeleter() = default;

	void operator()(ICECAdapter* ptr) const {
		if (ptr)
			CECDestroy(ptr);
	}
};

//class

ICECAdapter * Cec::CecInit(const char * name) const {
	cec_device_type_list deviceTypes;
	deviceTypes.Clear(); // We have to clear before using
	deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

	// Init everything
	void * cec = CECInit(name, deviceTypes);
	if (cec == NULL) {
		cerr << "Failed to init libcec" << endl;
		//return -1;
	}

	return (ICECAdapter *)cec;
}

Cec::Cec(const char * name) : cec(CecInit(name), ICECAdapterDeleter()) {

	ICECCallbacks callbacks;
	callbacks.CBCecLogMessage = &::cecLogMessage;
	callbacks.CBCecKeyPress   = &::cecKeyPress;
	callbacks.CBCecCommand    = &::cecCommand;
	callbacks.CBCecConfigurationChanged = &::cecConfigurationChanged;

	cec->EnableCallbacks(this, &callbacks);
}

Cec::~Cec() {}

void Cec::open() {

	// Search for adapters
    cec_adapter devices[10];
    uint8_t ret = cec->FindAdapters(devices, 10, NULL);
    if (ret < 0) {
    	cerr << "Error occurred searching for adapters" << endl;
    	//return -1;
    }

    if (ret == 0) {
    	cerr << "No adapters found" << endl;
    	//return -1;
    }

    // Just use the first found
    if (! cec->Open(devices[0].comm) ) {
    	cerr << "Failed to open adapter " << devices[0].path << endl;
    	//return -1;
    }
}

