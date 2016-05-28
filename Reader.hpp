#ifndef _READER_HPP_
#define _READER_HPP_
extern "C" {
#include "Parser.tab.h"
}
#include "Bytecode.hpp"
#include "Proto.hpp"
#include <memory>
#include <functional>
#include <list>

class ExprDeleter {
public:
    void operator()(Expr* x) const;
    void operator()(List* x) const;
};

struct Scope {
    ObjectPtr lex;
    ObjectPtr dyn;
};

using PtrToExpr = std::unique_ptr< Expr, ExprDeleter >;
using PtrToList = std::unique_ptr< List, ExprDeleter >;

class Stmt;

extern "C" void setCurrentLine(List* stmt);
PtrToList getCurrentLine();
std::list< std::unique_ptr<Stmt> > translateCurrentLine();
void clearCurrentLine() noexcept;

/*
 * Parses the string. Will throw an std::string as an exception
 * if a parse error occurs. For this reason, it is often desirable
 * to use `eval`, which captures these parse errors and rethrows them
 * as `ProtoError` objects.
 */
std::list< std::unique_ptr<Stmt> > parse(std::string str);

void eval(IntState& state, std::string str);

void readFile(std::string fname, Scope defScope, IntState& state);

/*
 * A statement. Defines only one method, which executes
 * the statement in a given context.
 */
class Stmt {
private:
    std::string file_name;
    int line_no;
    bool location;
public:
    Stmt(int line_no);
    void disableLocationInformation();
    void stateLine(InstrSeq&);
    void stateFile(InstrSeq&);
    virtual InstrSeq translate() = 0;
    virtual void propogateFileName(std::string name);
};

/*
 * A slot access statement, which may or may not be a method call.
 */
class StmtCall : public Stmt {
public:
    typedef std::list< std::unique_ptr<Stmt> > ArgList;
private:
    std::unique_ptr<Stmt> className;
    std::string functionName;
    ArgList args;
public:
    StmtCall(int line_no, std::unique_ptr<Stmt>& cls, const std::string& func, ArgList& arg);
    virtual InstrSeq translate();
    virtual void propogateFileName(std::string name);
};

/*
 * An assignment statement. Note that the `className` field defaults to the current
 * scope (lexical or dynamic, depending on the name of the variable) if it is not
 * provided.
 */
class StmtEqual : public Stmt {
private:
    std::unique_ptr<Stmt> className;
    std::string functionName;
    std::unique_ptr<Stmt> rhs;
public:
    StmtEqual(int line_no,
              std::unique_ptr<Stmt>& cls, const std::string& func,
              std::unique_ptr<Stmt>& asn);
    virtual InstrSeq translate();
    virtual void propogateFileName(std::string name);
};

/*
 * A method literal.
 */
class StmtMethod : public Stmt {
private:
    std::list< std::shared_ptr<Stmt> > contents;
public:
    StmtMethod(int line_no, std::list< std::shared_ptr<Stmt> >& contents);
    virtual InstrSeq translate();
    virtual void propogateFileName(std::string name);
};

/*
 * A numeric literal.
 */
class StmtNumber : public Stmt {
private:
    double value;
public:
    StmtNumber(int line_no, double value);
    virtual InstrSeq translate();
};

/*
 * A numeric integer literal.
 */
class StmtInteger : public Stmt {
private:
    long value;
public:
    StmtInteger(int line_no, long value);
    virtual InstrSeq translate();
};

/*
 * A numeric integer literal.
 */
class StmtBigInteger : public Stmt {
private:
    std::string value;
public:
    StmtBigInteger(int line_no, const char* value);
    virtual InstrSeq translate();
};

/*
 * A string literal.
 */
class StmtString : public Stmt {
private:
    std::string value;
public:
    StmtString(int line_no, const char* contents);
    virtual InstrSeq translate();
};

/*
 * A symbolic literal.
 */
class StmtSymbol : public Stmt {
private:
    std::string value;
public:
    StmtSymbol(int line_no, const char* contents);
    virtual InstrSeq translate();
};

/*
 * An inline list, which by default evaluates to an Array object.
 */
class StmtList : public Stmt {
public:
    typedef std::list< std::unique_ptr<Stmt> > ArgList;
private:
    ArgList args;
public:
    StmtList(int line_no, ArgList& arg);
    virtual InstrSeq translate();
    virtual void propogateFileName(std::string name);
};

/*
 * A sigil, syntax sugar for some function calls
 */
class StmtSigil : public Stmt {
private:
    std::string name;
    std::unique_ptr<Stmt> rhs;
public:
    StmtSigil(int line_no, std::string name, std::unique_ptr<Stmt> rhs);
    virtual InstrSeq translate();
    virtual void propogateFileName(std::string name);
};

/*
 * A hash-paren syntax, which evaluates to a string that is passed
 * to a meta function but is usually used for DSLs
 */
class StmtHashParen : public Stmt {
private:
    std::string text;
public:
    StmtHashParen(int line_no, std::string text);
    virtual InstrSeq translate();
};

/*
 * A zero-dispatch syntax, usually used to represent numerals
 * of different radices
 */
class StmtZeroDispatch : public Stmt {
private:
    std::string text;
    char symbol;
    char prefix;
public:
    StmtZeroDispatch(int line_no, char sym, char ch, std::string text);
    virtual InstrSeq translate();
};

#endif // _READER_HPP_
