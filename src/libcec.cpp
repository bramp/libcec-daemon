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
			CECDestroy(ptr);
	}
};

ICECAdapter * Cec::CecInit(const char * name) const {
	cec_device_type_list deviceTypes;
	deviceTypes.Clear(); // We have to clear before using
	deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

	// Init everything
	void * cec = CECInit(name, deviceTypes);
	if (cec == NULL)
		throw std::runtime_error("Failed to init libcec");

	return (ICECAdapter *)cec;
}

Cec::Cec(const char * name) : cec(CecInit(name), ICECAdapterDeleter()) {

	cerr << "Cec::Cec" << endl;

	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.CBCecLogMessage = &::cecLogMessage;
	callbacks.CBCecKeyPress   = &::cecKeyPress;
	callbacks.CBCecCommand    = &::cecCommand;
	callbacks.CBCecConfigurationChanged = &::cecConfigurationChanged;

	cec->EnableCallbacks(this, &callbacks);
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
