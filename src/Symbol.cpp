#include "Symbol.hpp"
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cassert>

using namespace std;

Symbols Symbols::instance = Symbols();
long Symbols::gensymIndex = 100L;

Symbols::Symbols()
    : syms(), names(), index(0), parentIndex(0) {
    parentIndex = (*this)["parent"].index;
}

bool Symbols::hasGeneratedName(const std::string& str) {
    if (str == "")
        return false;
    return (str[0] == '~');
}

Symbolic Symbols::gensym() {
    return gensym("G");
}

Symbolic Symbols::gensym(std::string prefix) {
    ++gensymIndex;
    ostringstream oss;
    oss << "~" << prefix << gensymIndex;
    return get()[oss.str()];
}

Symbolic Symbols::natural(int n) {
    if (n <= 0)
        return get()[""];
    Symbolic sym;
    sym.index = - n;
    return sym;
}

Symbols& Symbols::get() noexcept {
    return instance;
}

SymbolType Symbols::symbolType(Symbolic sym) {
    if (sym.index < 0)
        return SymbolType::NATURAL;
    std::string name = get()[sym];
    if (get().hasGeneratedName(name))
        return SymbolType::GENERATED;
    return SymbolType::STANDARD;
};

bool Symbols::isUninterned(const std::string& str) {
    return get().hasGeneratedName(str);
}

bool Symbols::requiresEscape(const std::string& str){
    string str0(str); // Need a copy
    if ((str0 == "") || (str0 == "~"))
        return true;
    sort(str0.begin(), str0.end());
    string special(".,:()[]{}\"\' \t\n");
    special += '\0';
    sort(special.begin(), special.end());
    string str1;
    set_intersection(str0.begin(), str0.end(), special.begin(), special.end(),
                     inserter(str1, str1.begin()));
    return (str1.begin() != str1.end());
}

Symbolic Symbols::parent() {
    return { get().parentIndex };
}

Symbolic Symbols::operator[](const std::string& str) {
    if (isUninterned(str)) {
        syms.emplace_back(str);
        ++index;
        assert((size_t)index == syms.size());
        return { index - 1 };
    } else {
        if (names.find(str) == names.end()) {
            syms.emplace_back(str);
            names.emplace(str, index);
            ++index;
            assert((size_t)index == syms.size());
        }
        Symbolic sym = { names.find(str)->second };
        return sym;
    }
}

std::string Symbols::operator[](const Symbolic& str) {
    if (str.index < 0) { // "Natural" symbol
        ostringstream str0;
        str0 << "~NAT" << abs(str.index);
        return str0.str();
    }
    if ((size_t)str.index >= syms.size())
        return "";
    return syms[str.index];
}

bool operator ==(const Symbolic& a, const Symbolic& b) noexcept {
    return a.index == b.index;
}

bool operator <(const Symbolic& a, const Symbolic& b) noexcept {
    return a.index < b.index;
}
