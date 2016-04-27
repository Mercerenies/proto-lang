
%code top {
    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include <memory>
    #include <string>
    #include "Reader.hpp"
    #include "Standard.hpp"
    #ifdef __cplusplus
    extern "C" int yylex();
    extern "C" int yyparse();
    extern "C" FILE *yyin;
    extern "C" int line_num;
    #else
    int yylex();
    int yyparse();
    FILE *yyin;
    int line_num;
    #endif
    void yyerror(const char*);
}

%code requires {

    #ifndef __cplusplus
    typedef char bool;
    #endif
    struct Expr;
    struct List;

    struct Expr {
        int line;
        struct Expr* lhs;
        char* name;
        struct List* args;
        bool equals;
        struct Expr* rhs;
        bool method;
        bool isNumber;
        double number;
        bool isInt;
        long integer;
        bool isBigInt;
        bool isString;
        bool isSymbol;
        bool isList;
    };

    struct List {
        struct Expr* car;
        struct List* cdr;
    };

    #ifdef __cplusplus
    extern "C" {
    #endif
        void cleanupE(struct Expr* stmt);
        void cleanupL(struct List* stmt);
        void yyerror(const char*);
        struct Expr* makeExpr();
        struct List* makeList();
    #ifdef __cplusplus
    }
    #endif

}

%union {
    long ival;
    double dval;
    char* sval;
    struct List* argval;
    struct Expr* exprval;
}

%error-verbose

%type <argval> lines
%type <exprval> line
%type <exprval> stmt
%type <exprval> rhs
%type <argval> arglist
%type <argval> arglist1
%type <exprval> arg
%type <exprval> chain
%type <exprval> literal
%type <argval> linelist
%type <argval> literallist
%type <argval> literallist1
%type <exprval> listlit
%type <exprval> rhschain

%token <sval> NAME
%token <dval> NUMBER
%token <ival> INTEGER
%token <sval> BIGINT
%token <sval> STRING
%token <sval> SYMBOL
%token LISTLIT

%%

toplevel:
    lines { setCurrentLine($1); }
    ;
lines:
    line lines { $$ = makeList(); $$->car = $1; $$->cdr = $2; } |
    /* empty */ { $$ = makeList(); }
line:
    stmt '.'
    ;
stmt:
    chain NAME rhs {
        $$ = $3;
        if ($$ == NULL)
            $$ = makeExpr();
        $$->name = $2;
        $$->lhs = $1;
    } |
    literal |
    '(' stmt ')' { $$ = $2; }
    ;
rhs:
    /* empty */ { $$ = NULL; } |
    rhschain { $$ = makeExpr(); $$->args = makeList(); $$->args->car = $1; $$->args->cdr = makeList(); } |
    '=' stmt { $$ = makeExpr(); $$->equals = true; $$->rhs = $2; } |
    ':' arglist { $$ = makeExpr(); $$->args = $2; }
    ;
arglist:
    /* empty */ { $$ = makeList(); } |
    arg arglist1 { $$ = makeList(); $$->car = $1; $$->cdr = $2; }
    ;
arglist1:
    /* empty */ { $$ = makeList(); } |
    ',' arg arglist1 { $$ = makeList(); $$->car = $2; $$->cdr = $3; }
    ;
arg:
    chain NAME { $$ = makeExpr(); $$->lhs = $1; $$->name = $2; } |
    '(' stmt ')' { $$ = $2; } |
    literal
    ;
chain:
    chain NAME { $$ = makeExpr(); $$->lhs = $1; $$->name = $2; } |
    '(' stmt ')' { $$ = $2; } |
    literal |
    /* empty */ { $$ = NULL; }
    ;
literal:
    '{' linelist '}' { $$ = makeExpr(); $$->method = true; $$->args = $2; } |
    NUMBER { $$ = makeExpr(); $$->isNumber = true; $$->number = $1; } |
    INTEGER { $$ = makeExpr(); $$->isInt = true; $$->integer = $1; } |
    BIGINT { $$ = makeExpr(); $$->isBigInt = true; $$->name = $1; } |
    STRING { $$ = makeExpr(); $$->isString = true; $$->name = $1; } |
    SYMBOL { $$ = makeExpr(); $$->isSymbol = true; $$->name = $1; } |
    '[' arglist ']' { $$ = makeExpr(); $$->isList = true; $$->args = $2; } |
    LISTLIT literallist ']' { $$ = makeExpr(); $$->isList = true; $$->args = $2; }
    ;
linelist:
    line linelist { $$ = makeList(); $$->car = $1; $$->cdr = $2; } |
    /* empty */ { $$ = makeList(); }
    ;
literallist:
    /* empty */ { $$ = makeList(); } |
    listlit literallist1 { $$ = makeList(); $$->car = $1; $$->cdr = $2; }
    ;
literallist1:
    /* empty */ { $$ = makeList(); } |
    ',' listlit literallist1 { $$ = makeList(); $$->car = $2; $$->cdr = $3; }
    ;
listlit:
    STRING { $$ = makeExpr(); $$->isString = true; $$->name = $1; } |
    SYMBOL { $$ = makeExpr(); $$->isSymbol = true; $$->name = $1; } |
    NUMBER { $$ = makeExpr(); $$->isNumber = true; $$->number = $1; } |
    INTEGER { $$ = makeExpr(); $$->isInt = true; $$->integer = $1; } |
    BIGINT { $$ = makeExpr(); $$->isBigInt = true; $$->name = $1; } |
    '[' literallist ']' { $$ = makeExpr(); $$->isList = true; $$->args = $2; } |
    NAME { $$ = makeExpr(); $$->isSymbol = true; $$->name = $1; }
    ;
rhschain:
    literal { $$ = $1; } |
    '(' stmt ')' { $$ = $2; } |
    literal NAME rhschain { $$ = makeExpr(); $$->lhs = $1; $$->name = $2; $$->args = makeList();
                            $$->args->car = $3; $$->args->cdr = makeList(); } |
    '(' stmt ')' NAME rhschain { $$ = makeExpr(); $$->lhs = $2; $$->name = $4; $$->args = makeList();
                                 $$->args->car = $5; $$->args->cdr = makeList(); }
    ;

%%

extern "C" {

void yyerror(const char* str) {
    std::ostringstream oss;
    oss << "Error on line " << line_num << "! " << str;
    throw oss.str();
}

void cleanupE(struct Expr* stmt) {
    if (stmt->lhs != 0) {
        cleanupE(stmt->lhs);
        free(stmt->lhs);
    }
    if (stmt->rhs != 0) {
        cleanupE(stmt->rhs);
        free(stmt->rhs);
    }
    if (stmt->args != 0) {
        cleanupL(stmt->args);
        free(stmt->args);
    }
    if (stmt->name != 0)
        free(stmt->name);
}

void cleanupL(struct List* stmt) {
    if (stmt->car != 0) {
        cleanupE(stmt->car);
        free(stmt->car);
    }
    if (stmt->cdr != 0) {
        cleanupL(stmt->cdr);
        free(stmt->cdr);
    }
}

struct Expr* makeExpr() {
    struct Expr* expr = new Expr();
    expr->line = line_num;
    return expr;
}

struct List* makeList() {
    return new List();
}

}
