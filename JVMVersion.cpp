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

#include "JVMVersion.h"


// 1.4.2_05-b04
// part 1 = 1
// part 2 = 4
// part 3 = 2
// part 4 = 5
// part 5 = 4
// All unspecified parts are assummed 0
const char *versionSeparators = "._-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

JVMVersion::JVMVersion(const char *vers)
{
    versionString = vers;

    char *versBuffer = strdup(vers);
    int i = 0;
    for (char *cp = strtok(versBuffer, "._"); cp != NULL && versionParts.size() < 5; cp = strtok(NULL, "._"), i++) {
	versionParts.push_back(atoi(cp));
    }

    free(versBuffer);
}

const std::string& JVMVersion::version() const
{
    return versionString;
}

// Returns true if all parts that are specified within this JVMVersion are
// equal to the equivalent parts in 'other'.  For example:
//
// JVMVersion("1.4") == JVMVersion("1.4.1") == false
// JVMVersion("1.4").matchSpecified(JVMVersion("1.4.1")) == true
// JVMVersion("1.4.1").matchSpecified(JVMVersion("1.4")) == false
bool JVMVersion::matchSpecified(const JVMVersion& other) const
{
    for (int i = 0; i < versionParts.size(); i++) {
	if (i >= other.versionParts.size()) {
	    return false;
	}
	if (versionParts[i] != other.versionParts[i]) {
	    return false;
	}
    }
    return true;
}

bool JVMVersion::operator<(const JVMVersion& other) const
{
    for (int i = 0; i < 5; i++) {
	int part = i < versionParts.size() ? versionParts[i] : 0;
	int otherPart = i < other.versionParts.size() ? other.versionParts[i] : 0;

	if (part > otherPart) {
	    return false;
	}
	if (part < otherPart) {
	    return true;
	}
    }
    return false;
}

bool JVMVersion::operator<=(const JVMVersion& other) const
{
    return (*this < other) || (*this == other);
}

bool JVMVersion::operator>(const JVMVersion& other) const
{
    return !(*this <= other);
}

bool JVMVersion::operator>=(const JVMVersion& other) const
{
    return !(*this < other);
}

bool JVMVersion::operator==(const JVMVersion& other) const
{
    for (int i = 0; i < 5; i++) {
	int part = i < versionParts.size() ? versionParts[i] : 0;
	int otherPart = i < other.versionParts.size() ? other.versionParts[i] : 0;

	if (part != otherPart) {
	    return false;
	}
    }
    return true;
}
