//// Copyright (c) 2018 Silvio Mayolo
//// See LICENSE.txt for licensing details


#include <utility>
#include <list>
#include "Precedence.hpp"
#include "Parents.hpp"
#include "Reader.hpp"

struct ListElem {
    List* list;
    bool provided;
};

// Simple binary tree data structure for intermediate results.
// Branches do not have data, so a node will either have a List* OR a
// left/right, not both.
struct OpTree {
    bool isLeaf;
    ListElem leaf;
    std::string name;
    OpTree* left;
    OpTree* right;
};

// An empty string signals the end of input
using op_pair_t = std::pair<ListElem, std::string>;

OperatorTable::OperatorTable(ObjectPtr table)
    : impl(table) {}

OperatorData OperatorTable::lookup(std::string op) const {

    Symbolic name = Symbols::get()[op];
    ObjectPtr val = objectGet(impl, name);

    // Doesn't exist? Fine, just use the default.
    if (val == nullptr)
        return { DEFAULT_PRECEDENCE, Associativity::LEFT };

    // We have the object; it had better be what we're looking for.
    OperatorData result;

    // Let's find precedence
    ObjectPtr prec = objectGet(val, Symbols::get()["prec"]);
    if (prec == nullptr) {
        result.precedence = DEFAULT_PRECEDENCE;
    } else if (auto value = boost::get<Number>(&prec->prim())) {
        if (value->hierarchyLevel() > 0)
            throw ParseError("Invalid operator table at " + op);
        auto value1 = value->asSmallInt();
        if ((value1 < MIN_PRECEDENCE) || (value1 > MAX_PRECEDENCE))
            throw ParseError("Invalid operator table at " + op);
        result.precedence = (int)value1;
    } else {
        // Not a number
        throw ParseError("Invalid operator table at " + op);
    }

    // Now associativity
    ObjectPtr assoc = objectGet(val, Symbols::get()["assoc"]);
    if (assoc != nullptr)
        assoc = objectGet(assoc, Symbols::get()["inner"]);
    if (assoc == nullptr) {
        result.associativity = Associativity::LEFT;
    } else if (auto value = boost::get<Symbolic>(&assoc->prim())) {
        std::string name = Symbols::get()[*value];
        if (name == "left")
            result.associativity = Associativity::LEFT;
        else if (name == "right")
            result.associativity = Associativity::RIGHT;
        else if (name == "none")
            result.associativity = Associativity::NONE;
        else
            throw ParseError("Invalid operator table at " + op);
    } else {
        // Not a symbol
        throw ParseError("Invalid operator table at " + op);
    }

    return result;

}

std::list<op_pair_t> exprToSeq(Expr* expr) {
    std::list<op_pair_t> result;
    std::string op = "";
    while ((expr != nullptr) && (expr->isOperator)) {
        result.push_front({ { expr->args, expr->argsProvided }, op });
        op = std::string(expr->name);
        Expr* temp = expr;
        expr = expr->lhs;
        assert(temp->rhs == nullptr);
        temp->lhs = nullptr;
        temp->args = nullptr;
        cleanupE(temp);
    }
    if (expr == nullptr) {
        expr = makeExpr();
        expr->isDummy = true;
    }
    List* dummy = makeList();
    dummy->car = expr;
    dummy->cdr = makeList();
    dummy->cdr->car = nullptr;
    dummy->cdr->cdr = nullptr;
    result.push_front({ { dummy, true }, op });
    return result;
}

bool shouldPop(const std::stack< std::pair<std::string, OperatorData> >& ops,
               const std::pair<std::string, OperatorData>& curr) {
    if (ops.empty())
        return false;
    auto& top = ops.top();
    if (top.second.precedence > curr.second.precedence) {
        return true;
    } else if (top.second.precedence < curr.second.precedence) {
        return false;
    } else {
        if (top.second.associativity != curr.second.associativity) {
            std::ostringstream err;
            err << "Operators " << top.first << " and " << curr.first
                << " have contradictory associativity";
            throw ParseError(err.str());
        } else {
            switch (top.second.associativity) {
            case Associativity::LEFT:
                return true;
            case Associativity::RIGHT:
                return false;
            case Associativity::NONE:
                {
                    std::ostringstream err;
                    err << "Operators " << top.first << " and " << curr.first
                        << " do not associate";
                    throw ParseError(err.str());
                }
            default:
                assert(false); // Pleeeeeeeease don't run this line of code :)
            }
        }
    }
}

void doPop(std::stack< std::pair<std::string, OperatorData> >& ops,
           std::stack<OpTree*>& nodes) {
    assert(!ops.empty());
    std::string popped = ops.top().first;
    ops.pop();
    assert(!nodes.empty());
    OpTree* rhs = nodes.top();
    nodes.pop();
    assert(!nodes.empty());
    OpTree* lhs = nodes.top();
    nodes.pop();
    OpTree* node = new OpTree();
    node->name = popped;
    node->left = lhs;
    node->right = rhs;
    nodes.push(node);
}

OpTree* seqToTree(std::list<op_pair_t>& seq, const OperatorTable& table) {
    std::stack<OpTree*> nodes;
    std::stack< std::pair<std::string, OperatorData> > ops;
    for (auto& elem : seq) {
        OpTree* op = new OpTree();
        op->isLeaf = true;
        op->leaf = elem.first;
        nodes.push(op);
        if (elem.second != "") {
            std::pair<std::string, OperatorData> curr = { elem.second, table.lookup(elem.second) };
            while (shouldPop(ops, curr)) {
                doPop(ops, nodes);
            }
            ops.push(curr);
        }
    }
    while (!ops.empty()) {
        doPop(ops, nodes);
    }
    // By this point, we should have exactly one node: the expression node.
    assert(nodes.size() == 1);
    // Great! Then pop it.
    OpTree* result = nodes.top();
    nodes.pop();
    return result;
}

ListElem treeToList(OpTree* tree);
Expr*    treeToExpr(OpTree* tree);

ListElem treeToList(OpTree* tree) {
    if (tree->isLeaf) {
        // Leaf node
        ListElem args = tree->leaf;
        delete tree;
        tree = nullptr;
        // So I think this could maybe be an assertion failure. I
        // don't think it should ever happen in the course of
        // Latitude execution, whereas the equivalent error in
        // treeToExpr could very well occur in normal use.
        if ((args.list->car) && (args.list->car->isDummy)) {
            throw ParseError("Invalid implied argument on right-hand-side of operator");
        }
        return args;
    } else {
        // Branch
        Expr* expr = treeToExpr(tree);
        List* list = makeList();
        list->car = expr;
        list->cdr = makeList();
        list->cdr->car = nullptr;
        list->cdr->cdr = nullptr;
        return { list, true };
    }
}

Expr* treeToExpr(OpTree* tree) {
    if (tree->isLeaf) {
        // Leaf node
        List* args = tree->leaf.list; // Don't care if args provided because we're a LHS, not a RHS
        delete tree;
        tree = nullptr;
        // Must be of length 1
        if ((!args->car) && (!args->cdr))
            throw ParseError("Invalid argument list on left-hand-side of operator");
        if ((args->cdr->car) || (args->cdr->cdr))
            throw ParseError("Invalid argument list on left-hand-side of operator");
        Expr* result = args->car;
        args->car = nullptr;
        cleanupL(args);
        if (result->isDummy) {
            cleanupE(result);
            result = nullptr;
        }
        return result;
    } else {
        // Branch
        Expr* expr = makeExpr();
        expr->lhs = treeToExpr(tree->left);
        auto temp = treeToList(tree->right);
        expr->args = temp.list;
        expr->argsProvided = temp.provided;
        expr->name = (char*)malloc(tree->name.length() + 1);
        strcpy(expr->name, tree->name.c_str());
        delete tree;
        return expr;
    }
}

Expr* reorganizePrecedence(const OperatorTable& table, Expr* expr) {
    auto seq = exprToSeq(expr);
    auto tree = seqToTree(seq, table);
    auto result = treeToExpr(tree);
    return result;
}

OperatorTable getTable(ObjectPtr lex) {

    ObjectPtr meta = objectGet(lex, Symbols::get()["meta"]);
    if (meta == nullptr)
        throw ParseError("Could not find operator table");
    ObjectPtr table = objectGet(meta, Symbols::get()["operators"]);
    if (table == nullptr)
        throw ParseError("Could not find operator table");
    ObjectPtr impl = objectGet(table, Symbols::get()["&impl"]);
    if (impl == nullptr)
        throw ParseError("Could not find operator table");
    return OperatorTable(impl);
}
