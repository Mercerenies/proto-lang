#ifndef UNICODE_HPP
#define UNICODE_HPP

#include "pl_Unidata.h"
#include <string>
#include <optional>

class UniChar {
private:
    long codepoint;
public:
    explicit UniChar(long cp);
    operator std::string();
    long codePoint();
    uni_class_t genCat();
    UniChar toUpper();
    UniChar toLower();
    UniChar toTitle();
};

long uniOrd(UniChar ch);
UniChar uniChr(long cp);

std::optional<UniChar> charAt(std::string str, long i);
std::optional<long> nextCharPos(std::string str, long i);

#endif // UNICODE_HPP
