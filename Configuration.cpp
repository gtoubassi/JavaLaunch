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
#pragma warning(disable: 4786) 

#include "Configuration.h"
#include <windows.h>
#include <shlobj.h>
#include <cstring>
#include <cstdio>

using namespace std;

// Hack
#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA             0x001C      
#endif

Configuration::Configuration()
    : preferredJVM(0), minimumJVM(0), maximumJVM(0)
{
    char buffer[MAX_PATH];
    int result = GetModuleFileName(NULL, buffer, sizeof(buffer));

    if (result > 0) {
	char *lastSlash = strrchr(buffer, '\\');
	char *lastDot = strrchr(buffer, '.');

	if (lastSlash == NULL) {
	    installDir = ".";
	    if (lastDot != NULL) {
		*lastDot = '\0';
	    }
	    appName = buffer;
	}
	else {
	    *lastSlash = '\0';
	    installDir = buffer;

	    if (lastDot != NULL && lastDot > lastSlash) {
		*lastDot = '\0';
	    }
	    appName = lastSlash + 1;
	}
    }


    if (SHGetSpecialFolderPath(NULL, buffer, CSIDL_LOCAL_APPDATA, true)) {
	appDataDir = buffer;
	appDataDir += "\\";
	appDataDir += appName;
    }

    load();
}

Configuration::~Configuration()
{
    delete preferredJVM;
    delete minimumJVM;
    delete maximumJVM;
}

string Configuration::expandVariables(string value)
{
    int pos;
    while ((pos = value.find("%INSTALLDIR%")) != string::npos) {
	value.replace(pos, strlen("%INSTALLDIR%"), installDir);
    }

    while ((pos = value.find("%APPNAME%")) != string::npos) {
	value.replace(pos, strlen("%APPNAME%"), appName);
    }

    while ((pos = value.find("%APPDATADIR%")) != string::npos) {
	value.replace(pos, strlen("%APPDATADIR%"), appDataDir);
    }

    return value;
}

void Configuration::load()
{
    std::string configFile = installDir + "\\" + appName + ".conf";

    FILE *fp;
    
    fp = fopen(configFile.c_str(), "r");

    if (fp == NULL) {
	configFile = installDir + "\\Resources\\" + appName + ".conf";
        fp = fopen(configFile.c_str(), "r");
    }

    if (fp == NULL) {
	return;
    }

    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

	if (buffer[0] == '#') {
	    continue;
	}

	char *firstSpace = strchr(buffer, ' ');

	if (!firstSpace) {
	    continue;
	}

	char *key = buffer;
	*firstSpace = '\0';

	do {
    	    firstSpace++;
	} while (*firstSpace && isspace(*firstSpace));

	char *value = firstSpace;

	for (char *end = value + strlen(value) - 1; isspace(*end); end--) {
	    *end = '\0';
	}

	if (strlen(key) == 0 || strlen(value) == 0) {
	    continue;
	}

	if (!strcmp("PreferredVM", key)) {
	    preferredJVM = new JVMVersion(value);
	}
	else if (!strcmp("MinimumVM", key)) {
	    minimumJVM = new JVMVersion(value);
	}
	else if (!strcmp("MaximumVM", key)) {
	    maximumJVM = new JVMVersion(value);
	}
	else if (!strcmp("MainClass", key)) {
	    mainClass = value;
	}
	else if (!strcmp("ClassPath", key)) {
	    if (classpath.size()> 0) {
		classpath += ";";
	    }
	    classpath += expandVariables(value);
	}
	else if (!strcmp("VMArgument", key)) {
	    vmArgs.push_back(expandVariables(value));
	}
	else if (!strcmp("JavaArgument", key)) {
	    javaArgs.push_back(expandVariables(value));
	}
    }

    fclose(fp);
}



