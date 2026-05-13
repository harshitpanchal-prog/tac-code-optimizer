#ifndef UTILS_H
#define UTILS_H

#include <string>
using namespace std;

bool isNumber(const string& s);

// Returns true for compiler-generated temporaries (t1, t2, t3, ...)
// i.e. the name is 't' followed by one or more digits.
bool isTemporary(const string& s);

#endif