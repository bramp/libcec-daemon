/**
 * libcec-daemon
 * A simple daemon to connect libcec to uinput.
 * by Andrew Brampton
 */
#include <cec.h>

#include <cstdio>
#include <iostream>
#include <stdint.h>
#include <cstddef>

#include <boost/shared_ptr.hpp>

using namespace CEC;

using std::cout;
using std::cerr;
using std::endl;

// cecloader has to be after some #includes and using namespaces :(
#include <cecloader.h>

using boost::shared_ptr;
using namespace CEC;

// Cleans up after CECInit
void cecDeleter(ICECAdapter* ptr) {
	if (ptr)
		CECDestroy(ptr);
}

void listAdapters() {
	//TODO
}

int cecLogMessage(void *UNUSED(cbParam), const cec_log_message &message) {
  return 0;
}

int cecKeyPress(void *UNUSED(cbParam), const cec_keypress &UNUSED(key)) {
  return 0;
}

int cecCommand(void *UNUSED(cbParam), const cec_command &UNUSED(command)) {
  return 0;
}

int cecConfigurationChanged(void *UNUSED(cbParam), const libcec_configuration & configuration) {
	return 0;
}

int main (int argc, char *argv[]) {
	cec_device_type_list deviceTypes;
	deviceTypes.Clear(); // We have to clear before using
	deviceTypes.Add(CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

	ICECCallbacks callbacks;
	callbacks.CBCecLogMessage = &cecLogMessage;
	callbacks.CBCecKeyPress   = &cecKeyPress;
	callbacks.CBCecCommand    = &cecCommand;
	callbacks.CBCecConfigurationChanged = &cecConfigurationChanged;

	// Init everything
	void * cec_v = CECInit("test", deviceTypes);
	if (cec_v == NULL) {
		cerr << "Failed to init libcec" << endl;
		return -1;
	}

	shared_ptr<ICECAdapter> cec((ICECAdapter *)cec_v, cecDeleter);

	cec->EnableCallbacks(NULL, &callbacks);

	// Search for adapters
    cec_adapter devices[10];
    uint8_t ret = cec->FindAdapters(devices, 10, NULL);
    if (ret < 0) {
    	cerr << "Error occurred searching for adapters" << endl;
    	return -1;
    }

    if (ret == 0) {
    	cerr << "No adapters found" << endl;
    	return -1;
    }

    // Just use the first found
    if (! cec->Open(devices[0].comm) ) {
    	cerr << "Failed to open adapter " << devices[0].path << endl;
    	return -1;
    }

}
