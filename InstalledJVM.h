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

#ifndef INSTALLED_JVM_H
#define INSTALLED_JVM_H

#include <string>

#include "Configuration.h"
#include "JVMVersion.h"


class InstalledJVM
{
public:

    static const std::vector<InstalledJVM>& installedJVMs();

    static const InstalledJVM *findSuitableJVM(const std::vector<InstalledJVM>& vms,
					       const Configuration& config);

    InstalledJVM(const char *vers, const char *runtimeLibPath);

    // If successful, returns true, if an error occurs, or an exception
    // is raised from within the java program, false is returned and the
    // error string may be populated with a suitable error.
    bool launch(const Configuration& config, std::string& error) const;

    const JVMVersion& version() const;
    const std::string& libraryPath() const;

    bool operator<(const InstalledJVM& other) const;

private:
    JVMVersion _version;
    std::string _library;
};


#endif
