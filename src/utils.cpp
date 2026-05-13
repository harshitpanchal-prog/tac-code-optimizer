#include "utils.h"

// Returns true for integer literals, including negatives (e.g. "-2", "0", "42").
// A bare "-" is not a number.
bool isNumber(const string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start == s.size()) return false;  // bare "-"
    for (size_t i = start; i < s.size(); i++)
        if (!isdigit(s[i])) return false;
    return true;
}

// A temporary is named 't' followed by one or more digits (e.g. t1, t2, t12).
// User-defined variables (a, b, x, y, result, ...) are NOT temporaries.
bool isTemporary(const string& s) {
    if (s.size() < 2 || s[0] != 't') return false;
    for (size_t i = 1; i < s.size(); i++)
        if (!isdigit(s[i])) return false;
    return true;
}