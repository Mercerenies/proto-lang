#include "Garnish.hpp"
#include "Reader.hpp"
#include <sstream>

using namespace std;

ObjectPtr garnish(ObjectPtr global, bool value) {
    if (value)
        return eval("meta True.", global, global);
    else
        return eval("meta False.", global, global);
}

ObjectPtr garnish(ObjectPtr global, boost::blank value) {
    return eval("meta Nil.", global, global);
}

ObjectPtr garnish(ObjectPtr global, std::string value) {
    ObjectPtr string = eval("meta String.", global, global);
    ObjectPtr val = clone(string);
    val.lock()->prim(value);
    return val;
}

ObjectPtr garnish(ObjectPtr global, double value) {
    ObjectPtr num = eval("meta Number.", global, global);
    ObjectPtr val = clone(num);
    val.lock()->prim(value);
    return val;
}

class PrimToStringVisitor : public boost::static_visitor<std::string> {
public:
    std::string operator()(boost::blank& val) const {
        return "()";
    }
    std::string operator()(double& val) const {
        ostringstream oss;
        oss << val;
        return oss.str();
    }
    std::string operator()(std::string& val) const {
        ostringstream oss;
        oss << '"';
        for (char ch : val) {
            if (ch == '"')
                oss << "\\\"";
            else
                oss << ch;
        }
        oss << '"';
        return oss.str();
    }
    std::string operator()(Method& val) const {
        ostringstream oss;
        oss << &val;
        return oss.str();
    }
    std::string operator()(SystemCall& val) const {
        ostringstream oss;
        oss << &val;
        return oss.str();
    }
    std::string operator()(StreamPtr& val) const {
        ostringstream oss;
        oss << val;
        return oss.str();
    }
    std::string operator()(Symbolic& val) const {
        ostringstream oss;
        oss << '\'';
        oss << Symbols::get()[val];
        return oss.str();
    }
};

std::string primToString(ObjectPtr obj) {
    return boost::apply_visitor(PrimToStringVisitor(), obj.lock()->prim());
}

class DumpObjectVisitor : public boost::static_visitor<ObjectPtr> {
private:
    ObjectPtr curr, mthd, lex, dyn;
public:
    DumpObjectVisitor(ObjectPtr curr, ObjectPtr mthd, ObjectPtr lex, ObjectPtr dyn)
        : curr(curr) , mthd(mthd), lex(lex) , dyn(dyn) {}
    template <typename T>
    ObjectPtr operator()(T& val) const {
        return mthd;
    }
};

template <>
ObjectPtr DumpObjectVisitor::operator()<Method>(Method& val) const {
    ObjectPtr result = getInheritedSlot(meta(lex), Symbols::get()["Nil"]);
    return callMethod(result, curr.lock(), mthd, clone(dyn));
}
template <>
ObjectPtr DumpObjectVisitor::operator()<SystemCall>(SystemCall& val) const {
    return val(std::list<ObjectPtr>());
}

void dumpObject(ObjectPtr lex, ObjectPtr dyn, Stream& stream, ObjectPtr obj) {
    ObjectPtr toString0 = getInheritedSlot(obj, Symbols::get()["toString"]);
    ObjectPtr asString0 =
        boost::apply_visitor(DumpObjectVisitor(obj, toString0, lex, dyn),
                             toString0.lock()->prim());
    if (auto str = boost::get<std::string>(&asString0.lock()->prim()))
        stream.writeLine(*str);
    else
        stream.writeLine("<?>");
    for (auto key : keys(obj)) {
        auto value = getInheritedSlot(obj, Symbols::get()[key]);
        ObjectPtr toString1 = getInheritedSlot(value, Symbols::get()["toString"]);
        ObjectPtr asString1 =
            boost::apply_visitor(DumpObjectVisitor(value, toString1, lex, dyn),
                                 toString1.lock()->prim());
        stream.writeText("  ");
        stream.writeText(key);
        stream.writeText(": ");
        if (auto str = boost::get<std::string>(&asString1.lock()->prim()))
            stream.writeLine(*str);
        else
            stream.writeLine("<?>");
    }
}
