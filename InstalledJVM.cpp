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
#pragma warning (disable:4786)

#include "InstalledJVM.h"
#include <algorithm>
#include <jni.h>
#include <windows.h>

using namespace std;

InstalledJVM::InstalledJVM(const char *vers, const char *runtimeLibPath)
    : _version(vers), _library(runtimeLibPath)
{
}

static vector<InstalledJVM> installedVMs;
static bool hasPopulatedInstalledVMs;

/*static*/ const vector<InstalledJVM>& InstalledJVM::installedJVMs()
{
    if (hasPopulatedInstalledVMs) {
	return installedVMs;
    }

    hasPopulatedInstalledVMs = true;

    HKEY key;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			       "SOFTWARE\\JavaSoft\\Java Runtime Environment",
			       0,
			       KEY_READ,
			       &key);

    if (result != ERROR_SUCCESS) {
	return installedVMs;
    }

    char buffer[1024];
    DWORD bufferSize = sizeof(buffer);

    for (int i = 0; RegEnumKeyEx(key, i, buffer, &bufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS; i++) {
	
	HKEY versionKey;
	
	result = RegOpenKeyEx(key, buffer, 0, KEY_READ, &versionKey);

	if (result == ERROR_SUCCESS) {
	    unsigned char libBuffer[1024];
	    unsigned long libBufferSize = sizeof(libBuffer);

	    DWORD type;
	    result = RegQueryValueEx(versionKey, "RuntimeLib", NULL, &type, libBuffer, &libBufferSize);

	    if (result == ERROR_SUCCESS) {
		installedVMs.push_back(InstalledJVM(buffer, (const char *)libBuffer));
	    }

	}

	RegCloseKey(versionKey);

	bufferSize = sizeof(buffer);
    }
    RegCloseKey(key);

    sort(installedVMs.begin(), installedVMs.end());

    return installedVMs;
}

/*static*/ const InstalledJVM *InstalledJVM::findSuitableJVM(const vector<InstalledJVM>& vms, const Configuration& config)
{
    // First look for the preferred VM
    if (config.preferredJVM != NULL) {
	for (int i = 0; i < vms.size(); i++) {
	    if (vms[i].version() == *config.preferredJVM) {
		return &vms[i];
	    }
	}
    }

    // Remove VMs earlier than the minimum
    int firstSuitableVM = 0;
    if (config.minimumJVM != NULL) {
	for (; firstSuitableVM < vms.size(); firstSuitableVM++) {
	    if (vms[firstSuitableVM].version() >= *config.minimumJVM) {
		break;
	    }
	}
    }

    // Remove VMs later than the maximum
    int lastSuitableVM = vms.size() - 1;
    if (config.maximumJVM != NULL) {
	for (; lastSuitableVM >= 0; lastSuitableVM--) {
	    if (vms[lastSuitableVM].version() <= *config.maximumJVM) {
		break;
	    }
	}
    }

    if (lastSuitableVM > 0 && lastSuitableVM >= firstSuitableVM) {
	return &vms[lastSuitableVM];
    }

    return NULL;
}


bool InstalledJVM::launch(const Configuration& config, string& error) const
{
    typedef jint (JNICALL *CreateJavaVM_t)(JavaVM **pvm, JNIEnv **env, void *args);
    typedef jint (JNICALL *GetDefaultJavaVMInitArgs_t)(void *args);

    JNIEnv *env;
    JavaVM *jvm;
    jclass cls;
    jmethodID mid;
    jthrowable throwable = NULL;

    JavaVMInitArgs vm_args;
    vm_args.version = 0x00010002;
    vm_args.ignoreUnrecognized = JNI_TRUE;
    /* Create the Java VM */

    HINSTANCE vmLibrary = LoadLibrary(_library.c_str());

    if (!vmLibrary) {
	error = "Failed to load dll: " + _library;
	return false;
    }

    CreateJavaVM_t CreateJavaVM = (CreateJavaVM_t)GetProcAddress(vmLibrary, "JNI_CreateJavaVM");
    if (!CreateJavaVM) {
	error = "Failed to find dll entrypoint JNI_CreateJavaVM";
	return false;
    }

    GetDefaultJavaVMInitArgs_t GetDefaultJavaVMInitArgs = (GetDefaultJavaVMInitArgs_t)GetProcAddress(vmLibrary, "JNI_GetDefaultJavaVMInitArgs");
    if (!GetDefaultJavaVMInitArgs) {
	error = "Failed to find dll entrypoint JNI_GetDefaultJavaVMInitArgs";
	return false;
    }

    vm_args.nOptions = config.vmArgs.size();
    if (config.classpath.size() > 0) {
	vm_args.nOptions++;
    }

    JavaVMOption *options = new JavaVMOption[vm_args.nOptions];

    int currOption = 0;
    char classpath[100*MAX_PATH] = "";
    if (config.classpath.size() > 0) {
	string classPathOption("-Djava.class.path=");
	classPathOption += config.classpath;

	strcpy(classpath, classPathOption.c_str());
	options[currOption].optionString = classpath;
	currOption++;
    }

    int i;
    for (i = 0; i < config.vmArgs.size(); i++) {
	options[currOption].optionString = (char *)config.vmArgs[i].c_str();
	currOption++;
    }

    vm_args.options = options;

    jint result = CreateJavaVM(&jvm, &env, &vm_args);
    if (result < 0) {
	error = "Call to JNI_CreateJavaVM failed";
	return false;
    }

    bool returnValue = false;

    int pos;
    std::string mainClass = config.mainClass;
    while ((pos = mainClass.find(".")) != string::npos) {
	mainClass.replace(pos, strlen("."), "/");
    }

    cls = env->FindClass(mainClass.c_str());
    if (cls == NULL) {
	error = "Could not find " + config.mainClass;
        goto destroy;
    }

    mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
    if (mid == NULL) {
	error = "Could not find static method main in " + mainClass;
        goto destroy;
    }

    jobjectArray args;
    args = (jobjectArray)env->NewObjectArray(config.javaArgs.size(),
         env->FindClass("java/lang/String"),
         env->NewStringUTF(""));

    if (args == NULL) {
	error = "Could not create string array";
	goto destroy;
    }

    for (i = 0; i < config.javaArgs.size(); i++) {
	jstring str = env->NewStringUTF(config.javaArgs[i].c_str());

	if (str == NULL) {
	    error = "Could not create string";
	    goto destroy;
	}

	env->SetObjectArrayElement(args, i, str);
    }

    env->CallStaticVoidMethod(cls, mid, args);

    throwable = env->ExceptionOccurred();
    env->ExceptionClear();

    // If an exception was raised, extract the message.
    if (throwable) {
	jclass throwableClass = env->GetObjectClass(throwable);

	if (throwableClass != NULL) {
	    jclass classClass = env->FindClass("java/lang/Class");

	    if (classClass != NULL) {
		jmethodID getMessageMID = env->GetMethodID(throwableClass, "getMessage", "()Ljava/lang/String;");
		if (getMessageMID != NULL) {
		    jstring message = (jstring)env->CallObjectMethod(throwable, getMessageMID);
		    if (message != NULL) {
			const char *str = env->GetStringUTFChars(message, 0);
			error = str;
			env->ReleaseStringUTFChars(message, str);
		    }
		}
	    }
	}
    }
    else {
        returnValue = true;
    }

destroy:

    jvm->DestroyJavaVM();
    delete options;
    return returnValue;
}

const JVMVersion& InstalledJVM::version() const
{
    return _version;
}

const std::string& InstalledJVM::libraryPath() const
{
    return _library;
}


bool InstalledJVM::operator<(const InstalledJVM& other) const
{
    return _version < other._version;
}

