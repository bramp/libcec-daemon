/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

#include <cec.h>

#include <cstdio>
#include <iostream>
#include <stdint.h>
#include <cstddef>

#include <boost/shared_ptr.hpp>

//#include "../lib/platform/os.h"
//#include "../lib/implementations/CECCommandHandler.h"

using namespace CEC;
//using namespace PLATFORM;

using std::cout;
using std::cerr;
using std::endl;

// cecloader has to be after some #includes and using namespaces :(
#include <cecloader.h>

/*
ICECCallbacks        g_callbacks;
libcec_configuration g_config;
int                  g_cecLogLevel(CEC_LOG_ALL);
ofstream             g_logOutput;
bool                 g_bShortLog(false);
CStdString           g_strPort;
bool                 g_bSingleCommand(false);
bool                 g_bExit(false);
bool                 g_bHardExit(false);
CMutex               g_outputMutex;

int main (int argc, char *argv[])
{
  g_config.Clear();
  snprintf(g_config.strDeviceName, 13, "CECTester");
  g_config.callbackParam      = NULL;
  g_config.clientVersion      = CEC_CLIENT_VERSION_1_5_0;
  g_callbacks.CBCecLogMessage = &CecLogMessage;
  g_callbacks.CBCecKeyPress   = &CecKeyPress;
  g_callbacks.CBCecCommand    = &CecCommand;
  g_config.callbacks          = &g_callbacks;

  if (!ProcessCommandLineArguments(argc, argv))
    return 0;

  if (g_config.deviceTypes.IsEmpty())
  {
    if (!g_bSingleCommand)
      cerr << "No device type given. Using 'recording device'" << endl;
    g_config.deviceTypes.add(CEC_DEVICE_TYPE_RECORDING_DEVICE);
  }

  ICECAdapter *parser = LibCecInitialise(&g_config);
  if (!parser)
  {
    cerr << "Cannot load libcec.so" << endl;

    if (parser)
      UnloadLibCec(parser);

    return 1;
  }

  if (!g_bSingleCommand)
  {
    CStdString strLog;
    strLog.Format("CEC Parser created - libCEC version %s", parser->ToString((cec_server_version)g_config.serverVersion));
    cout << strLog.c_str() << endl;

    //make stdin non-blocking
  #ifndef __WINDOWS__
    int flags = fcntl(0, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(0, F_SETFL, flags);
  #endif
  }

  if (g_strPort.IsEmpty())
  {
    if (!g_bSingleCommand)
      cout << "no serial port given. trying autodetect: ";
    cec_adapter devices[10];
    uint8_t iDevicesFound = parser->FindAdapters(devices, 10, NULL);
    if (iDevicesFound <= 0)
    {
      if (g_bSingleCommand)
        cout << "autodetect ";
      cout << "FAILED" << endl;
      UnloadLibCec(parser);
      return 1;
    }
    else
    {
      if (!g_bSingleCommand)
      {
        cout << endl << " path:     " << devices[0].path << endl <<
            " com port: " << devices[0].comm << endl << endl;
      }
      g_strPort = devices[0].comm;
    }
  }

  PrintToStdOut("opening a connection to the CEC adapter...");

  if (!parser->Open(g_strPort.c_str()))
  {
    PrintToStdOut("unable to open the device on port %s", g_strPort.c_str());
    UnloadLibCec(parser);
    return 1;
  }

  if (!g_bSingleCommand)
    PrintToStdOut("waiting for input");

  while (!g_bExit && !g_bHardExit)
  {
    string input;
    getline(cin, input);
    cin.clear();

    if (ProcessConsoleCommand(parser, input) && !g_bSingleCommand && !g_bExit && !g_bHardExit)
    {
      if (!input.empty())
        PrintToStdOut("waiting for input");
    }
    else
      g_bExit = true;

    if (!g_bExit && !g_bHardExit)
      CEvent::Sleep(50);
  }

  parser->Close();
  UnloadLibCec(parser);

  if (g_logOutput.is_open())
    g_logOutput.close();

  return 0;
}
*/

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
