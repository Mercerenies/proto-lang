#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include "Instructions.hpp"
#include "Assembler.hpp"

template <typename T>
struct serialize_t;

template <>
struct serialize_t<long> {

    using type = long;

    template <typename OutputIterator>
    void serialize(type arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <>
struct serialize_t<char> {

    using type = char;

    template <typename OutputIterator>
    void serialize(type arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <>
struct serialize_t<Reg> {

    using type = Reg;

    template <typename OutputIterator>
    void serialize(type arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <>
struct serialize_t<Instr> {

    using type = Instr;

    template <typename OutputIterator>
    void serialize(type arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <>
struct serialize_t<std::string> {

    using type = std::string;

    template <typename OutputIterator>
    void serialize(const type& arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <>
struct serialize_t<FunctionIndex> {

    using type = FunctionIndex;

    template <typename OutputIterator>
    void serialize(const type& arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <>
struct serialize_t<AssemblerLine> {

    using type = AssemblerLine;

    template <typename OutputIterator>
    void serialize(const type& arg, OutputIterator& iter) const;
    template <typename InputIterator>
    type deserialize(InputIterator& iter) const;

};

template <typename T, typename OutputIterator>
void serialize(const T& arg, OutputIterator& iter);

template <typename T, typename OutputIterator>
T deserialize(OutputIterator& iter);

template <typename OutputIterator, typename... Ts>
void serializeVariant(const boost::variant<Ts...>& arg, OutputIterator& iter);

// ----

template <typename OutputIterator>
auto serialize_t<long>::serialize(type arg, OutputIterator& iter) const -> void {
    long val1 = arg;
    if (val1 < 0)
        *iter++ = 0xFF;
    else
        *iter++ = 0x00;
    val1 = abs(val1);
    for (int i = 0; i < 4; i++) {
        *iter++ = (unsigned char)(val1 % 256);
        val1 /= 256;
    }
}

template <typename InputIterator>
auto serialize_t<long>::deserialize(InputIterator& iter) const -> type {
    int sign = 1;
    if (*iter > 0)
        sign *= -1;
    ++iter;
    long value = 0;
    long pow = 1;
    for (int i = 0; i < 4; i++) {
        value += pow * (long)(*iter);
        ++iter;
        pow <<= 8;
    }
    return sign * value;
}

template <typename OutputIterator>
auto serialize_t<char>::serialize(type arg, OutputIterator& iter) const -> void {
    *iter++ = (unsigned char)arg;
}

template <typename InputIterator>
auto serialize_t<char>::deserialize(InputIterator& iter) const -> type {
    char ch = *iter;
    ++iter;
    return ch;
}

template <typename OutputIterator>
auto serialize_t<Reg>::serialize(type arg, OutputIterator& iter) const -> void {
    *iter++ = (unsigned char)arg;
}

template <typename InputIterator>
auto serialize_t<Reg>::deserialize(InputIterator& iter) const -> type {
    unsigned char ch = *iter;
    ++iter;
    return (Reg)ch;
}

template <typename OutputIterator>
auto serialize_t<Instr>::serialize(type arg, OutputIterator& iter) const -> void {
    *iter++ = (unsigned char)arg;
}

template <typename InputIterator>
auto serialize_t<Instr>::deserialize(InputIterator& iter) const -> type {
    unsigned char ch = *iter;
    ++iter;
    return (Instr)ch;
}

template <typename OutputIterator>
auto serialize_t<std::string>::serialize(const type& arg, OutputIterator& iter) const -> void {
    for (char ch : arg) {
        if (ch == 0) {
            *iter++ = '\0';
            *iter++ = '.';
        } else {
            *iter++ = ch;
        }
    }
    *iter++ = '\0';
    *iter++ = '\0';
}

template <typename InputIterator>
auto serialize_t<std::string>::deserialize(InputIterator& iter) const -> type {
    std::string str;
    unsigned char ch;
    while (true) {
        ch = *iter;
        ++iter;
        if (ch == '\0') {
            ch = *iter;
            ++iter;
            if (ch == '.')
                str += '\0';
            else if (ch == '\0')
                break;
        } else {
            str += ch;
        }
    }
    return str;
}

template <typename OutputIterator>
auto serialize_t<FunctionIndex>::serialize(const type& arg, OutputIterator& iter) const -> void {
    // No need for a sign bit; this is an index so it's always nonnegative
    int val1 = arg.index;
    for (int i = 0; i < 4; i++) {
        *iter++ = (unsigned char)(val1 % 256);
        val1 /= 256;
    }
}

template <typename InputIterator>
auto serialize_t<FunctionIndex>::deserialize(InputIterator& iter) const -> type {
    int value = 0;
    int pow = 1;
    for (int i = 0; i < 4; i++) {
        value += pow * (long)(*iter);
        ++iter;
        pow <<= 8;
    }
    return { value };
}

template <typename OutputIterator>
auto serialize_t<AssemblerLine>::serialize(const type& instr, OutputIterator& iter) const -> void {
    ::serialize<Instr>(instr.getCommand(), iter);
    for (const auto& arg : instr.arguments()) {
        serializeVariant(arg, iter);
    }
}

template <typename InputIterator>
struct _AsmArgDeserializeVisitor {
    InputIterator& iter;

    template <typename T>
    RegisterArg operator()(Proxy<T>) {
        return deserialize<T>(iter);
    }

};

template <typename InputIterator>
auto serialize_t<AssemblerLine>::deserialize(InputIterator& iter) const -> type {
    Instr instr = ::deserialize<Instr>(iter);
    AssemblerLine instruction { instr };
    _AsmArgDeserializeVisitor<InputIterator> visitor { iter };
    for (const auto& arg : getAsmArguments(instr)) {
        instruction.addRegisterArg(callOnAsmArgType(visitor, arg));
    }
    return instruction;
}

template <typename T, typename OutputIterator>
void serialize(const T& arg, OutputIterator& iter) {
    serialize_t<T>().serialize(arg, iter);
}

template <typename T, typename OutputIterator>
T deserialize(OutputIterator& iter) {
    return serialize_t<T>().deserialize(iter);
}

template <typename OutputIterator>
struct _VariantSerializeVisitor : boost::static_visitor<void> {
    OutputIterator& iter;

    _VariantSerializeVisitor(OutputIterator& iter) : iter(iter) {}

    template <typename T>
    void operator()(const T& arg) {
        serialize(arg, iter);
    }

};

template <typename OutputIterator, typename... Ts>
void serializeVariant(const boost::variant<Ts...>& arg, OutputIterator& iter) {
    _VariantSerializeVisitor<OutputIterator> visitor { iter };
    boost::apply_visitor(visitor, arg);
}

#endif // SERIALIZE_HPP
