/*
Copyright (c) 2004, Garrick Toubassi

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <cstdio>
#include <io.h>

#include "Configuration.h"
#include "InstalledJVM.h"

#include <vector>

using namespace std;

static void parseCommandLine(string& userDLL, string& userVersion);
static void fatalError(const char *title, const string& message);
static void fatalError(const string& message);

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
    Configuration config;
    string userDLL, userVersion;

    parseCommandLine(userDLL, userVersion);
    
    const vector<InstalledJVM>& vms = InstalledJVM::installedJVMs();
    const InstalledJVM *vm = NULL;

    if (userDLL.size() > 0) {
	// The user specified the raw path to a jvm.dll
	// Note this leaks.
	if (_access(userDLL.c_str(), 0) != 0) {
	    string message = "The requested Java Virtual Machine (";
	    message += userDLL + ") specified via the -vm command line ";
	    message += "flag cannot be found.  Please ";
	    message += "specify a different dll or visit java.sun.com ";
	    message += "to download and install the desired version of Java.";
	    fatalError(message);
	}
	vm = new InstalledJVM("", userDLL.c_str());
    }
    else if (userVersion.size() > 0) {
	JVMVersion version(userVersion.c_str());

	// The user specified a version ala "1.4.2"
	for (int i = vms.size() - 1; i >= 0; i--) {
	    if (version.matchSpecified(vms[i].version())) {
		vm = &vms[i];
		break;
	    }
	}

	if (vm == NULL) {
	    string message = "The requested version of Java (";
	    message += userVersion + ") specified via the -java command line ";
	    message += "flag cannot be found.  Please ";
	    message += "specify a different version or visit java.sun.com ";
	    message += "to download and install this version of Java.";
	    fatalError(message);
	}
    }
    else {
	vm = InstalledJVM::findSuitableJVM(vms, config);

	if (vm == NULL) {
	    string error = "Cannot find a suitable version of Java.  ";
	    error += "Please visit java.sun.com to download and install Java";

	    if (config.minimumJVM == NULL) {
		if (config.preferredJVM == NULL) {
		    error += ".";
		}
		else {
		    error += " version " + config.preferredJVM->version() + ".";
		}
	    }
	    else {
		error += " version " + config.minimumJVM->version() + " or later.";
	    }
	    
	    fatalError("Cannot find Java", error);
	}
    }

    string error;
    boolean success = vm->launch(config, error);

    if (!success) {
	string message = "An error occurred trying to launch Java ";
	if (vm->version().version().size() > 0) {
	    message += "(version " + vm->version().version() + ") ";
	}
	message += "located at " + vm->libraryPath() + ":  ";
	message += "Please check that the installation is correct or reinstall.";
	if (error.length() > 0) {
	    message += "\n\n(Technical details: " + error + ")";
	}
	fatalError(message);
    }

    return 0;
}

static void parseCommandLine(string& userDLL, string& userVersion)
{
    for (int i = 0; i < __argc; i++) {
	if (!strcmp("-vm", __argv[i])) {
	    if (i < __argc - 1) {
		userDLL = __argv[++i];
	    }
	}
	else if (!strcmp("-java", __argv[i])) {
	    if (i < __argc - 1) {
		userVersion = __argv[++i];
	    }
	}
    }
}

static void fatalError(const char *title, const string& message)
{
    MessageBox(NULL, message.c_str(), title, MB_OK | MB_ICONERROR | MB_APPLMODAL);
    exit(1);
}

static void fatalError(const string& message)
{
    fatalError(NULL, message);
}

