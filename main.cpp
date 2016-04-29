extern "C" {
#include "lex.yy.h"
#include "Parser.tab.h"
}
#include "Reader.hpp"
#include "Standard.hpp"
#include "Garnish.hpp"
#include "GC.hpp"
#include "Symbol.hpp"
#include "Cont.hpp"
#include "REPL.hpp"
#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char** argv) {

    ObjectPtr global;
    try {
        try {
            try {
                global = spawnObjects();
            } catch (ProtoError& err) {
                cerr << "An error occurred while loading the standard library. "
                     << "Please report this." << endl;
                auto stream = errStream();
                stream->writeLine("*** STD EXCEPTION ***");
                dumpObject({ err.getObject(), err.getObject() }, *stream, err.getObject());
                return 1;
            }
            runREPL(global);
        } catch (ProtoError& err) {
            auto stream = errStream();
            stream->writeLine("*** TOPLEVEL EXCEPTION ***");
            //auto bloop = getInheritedSlot({ global, global }, err.getObject(), Symbols::get()["message"]);
            //auto bleep = getInheritedSlot({ global, global }, err.getObject(), Symbols::get()["slotName"]);
            //cerr << boost::get<string>(bloop.lock()->prim()) << endl;
            //cerr << Symbols::get()[boost::get<Symbolic>(bleep.lock()->prim())] << endl;
            global.lock()->put(Symbols::get()["$except"], err.getObject());
            dumpObject({ global, global }, *stream, err.getObject());
            return 1;
        }
    } catch (...) {
        return 1;
    }

    return 0;
}
