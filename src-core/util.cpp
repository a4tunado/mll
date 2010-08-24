#include "util.h"

using std::string;

namespace mll {

std::ostream& NullStream() {
    static std::ostream nullStream(0);
    return nullStream;
}

template<>
string ToString(const string& input) {
    return input;
}

template<>
string ToString(const bool& input) {
    return input ? "true" : "false";
}

template<>
string FromString(const string& input) {
    return input;
}

template<>
bool FromString(const string& input) {
    string local(input);
    ToLower(&local);
    if (local == "true") {
        return true;
    } else if (local == "false") {
        return false;
    } else {
        return FromString<int>(input) != 0;
    }
}



} // namespace mll
