
# Chapter 2 - Lexical Structure

## Encoding

Latitude programs are assumed to be encoded in UTF-8.

## Whitespace

The following characters are considered to be whitespace: space
(`U+0020`), horizontal tab (`U+0009`), carriage return (`U+000D`), and
line feed (`U+000A`). In general, whitespace is used to separate
tokens, so unless otherwise stated, multiple whitespace characters
will be treated as equivalent to a single character of whitespace.

[TODO: Should some non-ASCII characters be whitespace?]

## Character Sets

Aside from whitespace characters, there are three other classes of
characters in the Latitude parser: special, semi-special, and normal.

 * Whitespace characters, as discussed in the section on whitespace,
   are generally ignored and are used to separate tokens.
 * Special characters are treated specially when they appear in the
   program, except in specific circumstances such as the inside of a
   string. They are used as parts of syntactic constructs and can
   never appear in an identifier. These are the dot (`U+002E`), comma
   (`U+002C`), colon (`U+003A`), semicolon (`U+003B`), parentheses
   (`U+0028` and `U+0029`), brackets (`U+005B` and `U+005D`), braces
   (`U+007B` and `U+007D`), single quotes (`U+0027`), and double
   quotes (`U+0022`).
 * Semi-special characters are treated specially when they appear
   following whitespace. As such, they cannot be the first character
   in an identifier, but they can appear in the middle or at the end
   of an identifier. These are the digits (`U+0030` to `U+0039`),
   tilde (`U+007E`), hash (`U+0023`), and at-sign (`U+0040`).
 * Normal characters can appear anywhere in an identifier. Any
   character that is not in any of the above classes is considered
   normal.
 * The backtick (`U+0060`) and backslash (`U+005C`) are reserved for
   future extensions to the language and should not be used in
   identifiers.

## Identifiers

An identifier is a nonempty string of characters, of which the first
must be a normal character and the remaining characters must be either
normal or semi-special.

There are two types of identifiers: standard identifiers and operator
identifiers. For the most part, they are equivalent. However, operator
identifiers will always expect argument lists when called as
methods. An operator identifier is one that consists only of operator
characters. Any identifier which is not an operator identifier is a
standard identifier.

Operator characters are characters which satisfy all of the following
properties.
 1. The character can appear in Latitude identifiers.
 2. The character is a punctuation or symbol character, according to
    the Unicode general category.
 3. The character is not the dollar sign (`U+0024`).

Note that an identifier ending with an equal-sign (`=`) is often
called an assignment identifier, as there is a special syntax which
invokes methods with these names in a convenient way.

The following exceptions take precedence over the above rule.
 * Three consecutive dots (`...`) form a valid identifier, called the
   ellipsis identifier (and its corresponding variable is called the
   ellipsis variable). As a consequence of this, a single dot is
   considered a statement terminator, a pair of consecutive dots is
   almost always a syntax error, and three dots in a row is parsed as
   the ellipsis identifier.
 * Two colons in a row (`::`) form a valid identifier. Note that there
   is no assignment identifier corresponding to this, as `::=` is a
   special syntactic token and not a valid identifier.
 * The following, on their own, are not valid identifiers: `=`, `<-`,
   and `=>`. These lexemes are treated specially by the language for
   various syntactic constructs. However, identifiers which happen to
   contain these strings are acceptable, so `==` or `<=>`, for
   example, are perfectly acceptable as identifiers.
 * A plus sign (`+`) or minus sign (`-`) followed by at least one
   digit (`0` to `9`) is not a valid *start* to an identifier. That
   is, any string which begins with a plus sign or minus sign followed
   by at least one digit, followed by any other characters is not a
   valid identifier, as it will be parsed as a numerical literal.

Note also that identifiers that begin with a dollar-sign (`$`) are
treated specially by scope resolution.

## Code Structure

A source file consists of zero or more lines, as defined by the
following grammar.

```
<line> ::= <stmt> "."
<stmt> ::= <chain> <stdname> <rhs> |
           <chain> <opname> <rhs1> |
           <literalish>
<rhs> ::= λ |
          <shortarglist> |
          ":=" <stmt> |
          "::=" <stmt> |
          ":" <arglist> |
          <shortarglist> "=" <stmt> |
          "<-" <stmt>
<rhs1> ::= <verysimplechainl> |
           <shortarglist1> |
           ":=" <stmt> |
           "::=" <stmt> |
           ":" <arglist> |
           <shortarglist> "=" <stmt> |
           "<-" <stmt>
<arglist> ::= λ |
              <arg> <arglist1>
<arglist1> ::= λ |
               "," <arg> <arglist1>
<arg> ::= <chain> <stdname> |
          <chain> <opname> <name> |
          <simplechain> <stdname> <shortarglist> |
          <chain> <opname> <shortarglist1> |
          <literalish>
<chain> ::= <chain> <opname> <name> |
            <simplechain>
<simplechain> ::= <simplechain> <stdname> |
                  <simplechain> <stdname> <shortarglist> |
                  <literalish> |
                  λ
<verysimplechain> ::= <verysimplechainl> <stdname> |
                      <verysimplechainl> <stdname> <shortarglist> |
<verysimplechainl> ::= <verysimplechain> |
                       <literal> |
                       λ
<shortarglist> ::= "(" <arglist> ")" |
                   "(" <chain> <name> ":" <arg> ")" |
                   <literal>
<shortarglist1> ::= "(" <arglist> ")" |
                    "(" <chain> <name> ":" <arg> ")"
<literalish> ::= "~" <ename> <literalish> |
                 "(" <stmt> ")" |
                 <literal>
<literal> ::= "{" <linelist> "}" |
              <number> |
              "\"" <string> "\"" |
              "'" <ename> |
              "'(" <symbol> ")" |
              "~" <ename> |
              "[" <arglist> "]" |
              "'[" <literallist> "]" |
              "#\"" <rawstring> "\"#" |
              [ "-" | "+" ] "0" <letter> { <alphanum> } |
              "#'" <ename>
<linelist> ::= <line> <linelist> |
               λ
<literallist> ::= <listlist> <literallist1> |
                  λ
<literallist1> ::= "," <listlist> <literallist1> |
                   λ
<listlit> ::= "\"" <string> "\"" |
              "'" <ename> |
              "~" <ename> |
              <number> |
              "'(" <symbol> ")" |
              "[" <literallist> "]" |
              <rawstring> |
              <name>
<name> ::= <stdname> |
           <opname>
```

Note that a `<letter>` is any single lowercase or capital alphabetic
letter. Similarly, an `<alphanum>` is a letter or single digit. A
`<stdname>` is any valid standard identifier, and an `<opname>` is any
valid operator identifier. An `<ename>` is any nonempty sequence of
normal or semi-special characters.

A `<number>` is a string of characters which satisfies one of the
following regular expressions.

    [-+]?[0-9]+(\.[0-9]+)([eE][-+]?[0-9]+)?
    [-+]?[0-9]+([eE][-+]?[0-9]+)
    [-+]?[0-9]+
    [-+]?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?i
    [-+]?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?[-+][0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?i

The first two forms construct a floating-point number; the third form
constructs an integer; the fourth and fifth construct complex
numbers. For more information on the different types of numbers, refer
to [Number](../ii_standard_library/number.md).

Additionally, integer literals can be prefixed with `0x`, `0b`, or
`0o` (case insensitive, following the optional sign) to indicate that
the literal be parsed in hexadecimal, binary, or octal, respectively.

A `<symbol>` consists of any sequence of non-close-paren characters
and backslash literals. Any character other than a backslash (`\`) or
a close-parenthesis (`)`) is interpreted literally as part of the
symbol's name. This includes spaces and newlines. A backslash
interprets the character immediately following it literally, even if
the character following it is another backslash or a
close-parenthesis. Additionally, the backslash escape sequences,
including Unicode escape sequences, that can be used in string
literals can be used in symbol literals enclosed in parentheses.

## String Literals

A `<string>` consists of a sequence of non-double-quote characters and
backslash escape sequences. Any character other than a backslash (`\`)
or a double-quote (`"`) is interpreted literally and placed into the
string literal. A quotation mark closes the string literal. Note that
newlines can be placed within strings freely and will be interpreted
as part of the string.

A backslash causes the character immediately following it to be
interpreted specially. The following special translations take place.

| Sequence | Result   |
| -------- | -------- |
| `\n`     | `U+000A` |
| `\r`     | `U+000D` |
| `\t`     | `U+0009` |
| `\a`     | `U+0007` |
| `\b`     | `U+0008` |
| `\f`     | `U+000C` |
| `\v`     | `U+000B` |

Additionally, strings can contain Unicode escape sequences. A Unicode
escape sequence consists of `\u`, followed either by four hex digits
or up to six hex digits enclosed in curly braces. This sequence will
be interpreted as a single Unicode character, whose code point is
equal to the hex digits, interpreted as a hexadecimal numeral.

A backslash followed by any other character (including a second
backslash or a double quote, but excluding a `u`) will be treated as
that second character literally. A backslash followed by a `u` which
is not a part of a Unicode escape sequence is an invalid lexical token
and will result in a parse error.

A `<rawstring>` starts with a `#`, followed by the opening delimiter,
then any text, then the closing delimiter. The valid delimiter pairs
are `()`, `[]`, `{}`, and `""`. Inside the string's contents,
backslash is only considered a special character when succeeded by one
of the delimiter characters or another backslash. In any other case,
it is treated literally. Additionally, unescaped brackets of the same
type as the string delimiters can appear in the string, so long as
they appear in balanced pairs. Aside from the difference in parsing,
raw string literals are equivalent to ordinary string literals and are
treated the same way at runtime.

## Comments

Line comments in Latitude start with a semicolon (`;`) and continue
until the next newline character (`U+000A`). Block comments start with
an opening brace followed by a star (`{*`) and end with a star
followed by a closing brace (`*}`). Note that block comments can be
nested, so matching pairs of `{*` and `*}` can appear within a block
comment so long as they are balanced. Comments can be placed in the
code anywhere that whitespace would be ignored, so in particular
comments cannot be placed in the middle of an identifier or inside a
string.

Finally, an additional type of line comment is allowed. If a line
begins with `#!` then that entire line will be ignored by the parser.
This allows Unix-style shebang lines at the start of the file.

## Operator Precedence

Latitude has a notion of operator precedence which can be customized
at runtime. When a new file is loaded or a string is evaluated, the
current operator table is accessed through the `operators` slot on the
current lexical meta object. This slot should contain a dictionary.
Each value of this dictionary shall have slots `prec` and `assoc`
defined on it, to the specification of
the [`Operator`](../ii_standard_library/operator.md) object.

Whenever a chain of operators is encountered, the precedence rules are
considered. Operators with higher precedence will bind more tightly
than those with low precedence. Operators with the same precedence and
the same associativity will associate together, and those with the
same precedence but no associativity or differing associativity will
trigger a `ParseError`. The built-in precedence rules are as follows.

| Operator(s)            | Precedence | Associativity |
| ---------------------- | ---------- | ------------- |
| =~ == === < <= > >= /= | 5          | None          |
| ++                     | 10         | Left          |
| <>                     | 15         | Left          |
| <\|                    | 20         | Right         |
| \|>                    | 25         | Left          |
| (new operators)        | 30         | Left          |
| + -                    | 35         | Left          |
| /                      | 40         | Left          |
| *                      | 45         | Left          |
| ^                      | 50         | Right         |

[[up](.)]
<br/>[[prev - Chapter 1 - Introduction](ch1_intro.md)]
<br/>[[next - Chapter 3 - Object Model](ch3_object.md)]
