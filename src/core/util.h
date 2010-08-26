#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <sstream>
#include <vector>

namespace mll {

//! Converts each character in the string to lower case
inline void ToLower(std::string* data) {
    for (std::string::iterator it = data->begin(); it != data->end(); ++it) {
        *it = tolower(*it);
    }
}

//! Fills the vector with numbers from 0 to count - 1.
inline void InitIndexes(int count, std::vector<int>* indexes) {
    indexes->resize(count);
    for (int i = 0; i < count; ++i) {
        indexes->at(i) = i;
    }
}

//! Converts T to string
template<class T>
std::string ToString(const T& input) {
    std::stringstream stream;
    stream << input;
    return stream.str();
}

template<>
std::string ToString(const std::string& input);

template<>
std::string ToString(const bool& input);

//! Converts string to T
template<class T>
T FromString(const std::string& input) {
    T res;
    std::stringstream stream;
    stream << input;
    stream >> res;
    return res;
}

template<>
std::string FromString(const std::string& input);

template<>
bool FromString(const std::string& input);

//! Null stream
std::ostream& NullStream();

} // namespace mll

#endif // UTIL_H_
