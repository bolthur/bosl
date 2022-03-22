
# Syntax grammar

## General

```ebnf
program               → declaration* EOF ;
```

## Declarations

```ebnf
declaration           → declaration_fn | declaration_const | declaration_let | statement;

declaration_fn        → "fn" function ;
declaration_const     → "const" IDENTIFIER ":" ( "pointer" )? TYPE_IDENTIFIER "=" expression ";" ;
declaration_let       → "let" IDENTIFIER ":" ( "pointer" )? TYPE_IDENTIFIER ( "=" expression )? ";" ;

## Statements

statement             → statement_expression | statement_if | statement_print | statement_return | statement_while | statement_pointer | block;

statement_expression  → expression ";" ;
statement_if          → "if" "(" expression ")" statement ( "else" statement )? ;
statement_print       → "print" "(" expression ")" ";" ;
statement_return      → "return" expression? ";" ;
statement_while       → "while" "(" expression ")" statement;
statement_pointer     → "pointer" IDENTIFIER statement;
block                 → "{" declaration "}"
```

## Expressions

```ebnf
expression            → assignment ;
assignment            → IDENTIFIER "=" assignment | logic_or ;
logic_or              → logic_and ( "||" logic_and )* ;
logic_and             → or ( "&&" or )* ;
or                    → xor ( "|" xor )* ;
xor                   → and ( "^" and )* ;
and                   → equality ( "&" equality )* ;
equality              → comparison ( ( "!=" | "==" ) comparison )* ;
comparison            → term ( ( ">" | ">=" | "<" | "<=" | "<<" | ">>" ) term )* ;
term                  → factor ( ( "-" | "+" ) factor )*
factor                → unary ( ( "/" | "*" ) unary )* ;
unary                 → ( "!" | "-" | "+" | "~" ) unary | call | load | pointer ;
call                  → primary ( "(" arguments? ")" )* ;
load                  → "load" IDENTIFIER ;
pointer               → "pointer" IDENTIFIER ;
primary               → "true" | "false" | "null" | NUMBER | STRING | IDENTIFIER | "(" expression ")";
```

## Utility rules

```ebnf
function              → IDENTIFIER "(" parameters? ")" block ( "=" "load" IDENTIFIER )? ;
parameters            → ( IDENTIFIER ":" ( "pointer" )? TYPE_IDENTIFIER ) ( "," ( IDENTIFIER ":" ( "pointer" )? TYPE_IDENTIFIER ) )* ;
arguments             → expression ( "," expression )* ;
```

## Lexical Grammar

```ebnf
NUMBER                → DIGIT+ ( ( "." DIGIT+ ) | ( "x" HEXNUMBER+ ) )? ;
STRING                → "\"" <any char except "\"">* "\"" ;
IDENTIFIER            → ALPHA ( ALPHA | DIGIT )* ;
HEXNUMBER             → "a" ... "z" | "A" ... "Z" | "0" ... "9";
ALPHA                 → "a" ... "z" | "A" ... "Z" | "_" ;
DIGIT                 → "0" ... "9" ;
```
