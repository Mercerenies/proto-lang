#ifndef _BYTECODE_HPP_
#define _BYTECODE_HPP_

#include <map>
#include <vector>
#include <string>
#include <stack>
#include <type_traits>
#include <boost/variant.hpp>
#include "Symbol.hpp"
#include "Number.hpp"
#include "Proto.hpp"

class AssemblerLine;

using InstrSeq = std::deque<unsigned char>;

enum class Instr : unsigned char {
    MOV = 0x01, PUSH = 0x02, POP = 0x03, GETL = 0x04, GETD = 0x05, ESWAP = 0x06, ECLR = 0x07,
        ESET = 0x08, SYM = 0x09, NUM = 0x0A, INT = 0x0B, FLOAT = 0x0C, NSWAP = 0x0D, CALL = 0x0E,
        XCALL = 0x0F, XCALL0 = 0x10, RET = 0x11, CLONE = 0x12, RTRV = 0x13, RTRVD = 0x14, STR = 0x15,
        SSWAP = 0x16, EXPD = 0x17, MTHD = 0x18, LOAD = 0x19, SETF = 0x1A, PEEK = 0x1B
        };

enum class Reg : unsigned char {
    PTR = 0x01, SLF = 0x02, RET = 0x03, LEX = 0x04, DYN = 0x05, ARG = 0x06, STO = 0x07,
        CONT = 0x08, STACK = 0x09, ERR0 = 0x0A, ERR1 = 0x0B, SYM = 0x0C, NUM0 = 0x0D,
        NUM1 = 0x0E, STR0 = 0x0F, STR1 = 0x10, MTHD = 0x11
        };

using RegisterArg = boost::variant<Reg, long, std::string, InstrSeq>;

class InstructionSet {
public:
    typedef std::function<bool(const RegisterArg&)> Validator;
    typedef std::vector<Validator> ValidList;
private:
    bool initialized;
    std::map<Instr, ValidList> props;
    static InstructionSet iset;
    void initialize();
    InstructionSet() = default;
public:
    static InstructionSet& getInstance();
    bool hasInstruction(Instr instr);
    ValidList getParams(Instr instr);
};

class AssemblerError {
private:
    std::string message;
public:
    AssemblerError();
    AssemblerError(std::string message);
    std::string getMessage();
};

bool isRegister(const RegisterArg& arg);
bool isObjectRegister(const RegisterArg& arg);
bool isStackRegister(const RegisterArg& arg);
bool isStringRegisterArg(const RegisterArg& arg);
bool isLongRegisterArg(const RegisterArg& arg);
bool isAsmRegisterArg(const RegisterArg& arg);
void appendRegisterArg(const RegisterArg& arg, InstrSeq& seq);
void appendInstruction(const Instr& instr, InstrSeq& seq);

class AssemblerLine {
private:
    Instr command;
    std::vector<RegisterArg> args;
public:
    AssemblerLine() = default;
    void setCommand(Instr);
    void addRegisterArg(const RegisterArg&);
    void validate(); // Throws if invalid
    void appendOnto(InstrSeq&) const;
};

template <typename... Ts>
AssemblerLine makeAssemblerLine(Instr instr, Ts... args) {
    AssemblerLine line;
    line.setCommand(instr);
    std::array<RegisterArg, sizeof...(args)> args0 = { args... };
    for (auto& arg : args0)
        line.addRegisterArg(arg);
    line.validate();
    return line;
}

struct IntState {
    ObjectPtr ptr, slf, ret;
    std::stack<ObjectPtr> lex, dyn, arg, sto;
    InstrSeq cont;
    std::stack<InstrSeq> stack;
    bool err0, err1;
    Symbolic sym;
    Number num0, num1;
    std::string str0, str1;
    InstrSeq mthd;
};

IntState intState();

unsigned char popChar(InstrSeq& state);
long popLong(InstrSeq& state);
std::string popString(InstrSeq& state);
Reg popReg(InstrSeq& state);
Instr popInstr(InstrSeq& state);
InstrSeq popLine(InstrSeq& state);

void executeInstr(Instr instr, IntState& state);

void doOneStep(IntState& state);

#endif // _BYTECODE_HPP_
