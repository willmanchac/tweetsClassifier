#include "DSString.h"
#include <cstring>

DSString::DSString() : data(nullptr), len(0) {}

DSString::DSString(const char* str) {
    len = strlen(str);
    data = new char[len + 1];
    strcpy(data, str);
}
DSString::DSString(const char* begin, const char* end) {
    len = end - begin;
    data = new char[len + 1];
    strncpy(data, begin, len);
    data[len] = '\0'; // Null-terminate the string
}


DSString::DSString(const DSString& other) : len(other.len) {
    data = new char[len + 1];
    strcpy(data, other.data);
}

DSString::~DSString() {
    delete[] data;
}

DSString& DSString::operator=(const DSString& other) {
    if (this != &other) {
        delete[] data;
        len = other.len;
        data = new char[len + 1];
        strcpy(data, other.data);
    }
    return *this;
}

size_t DSString::length() const {
    return len;
}

char& DSString::operator[](size_t index) {
    if (index < len) {
        return data[index];
    }
    throw std::out_of_range("Index out of range");
}
// Const version of operator[]
const char& DSString::operator[](size_t index) const {
    if (index >= len) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}
DSString DSString::operator+(const DSString& other) const {
    DSString result;
    result.len = len + other.len;
    result.data = new char[result.len + 1];
    strcpy(result.data, data);
    strcat(result.data, other.data);
    return result;
}

bool DSString::operator==(const DSString& other) const {
    return strcmp(data, other.data) == 0;
}

char* DSString::c_str() const {
    return data;
}

DSString DSString::substring(size_t start, size_t numChars) const {
    if (start >= len || numChars == 0) {
        return DSString();  // Return an empty DSString if start is out of range or numChars is zero.
    }
    numChars = std::min(numChars, len - start);
    return DSString(data + start, data + start + numChars);
}


DSString DSString::toLower() const {
    DSString lowerStr(*this);
    for (size_t i = 0; i < len; i++) {
        if (isupper(lowerStr.data[i])) {
            lowerStr.data[i] = tolower(lowerStr.data[i]);
        }
    }
    return lowerStr;
}

std::ostream& operator<<(std::ostream& os, const DSString& str) {
    os << str.data;
    return os;
}
