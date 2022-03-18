# BOSL

`BOSL` which stands for `Bolthur Obligated Scripting Language` is a more or less
simple scripting language which is used in raspi iomem server for more complex
mmio sequences than readm write, wait until true / false / not equal / equal.

More specific grammar information might be found [here](markdown/grammar.md).

## Language overview

Overview about the language itself.

### Data types

Overview about supported builtin data types and values

```js
// following data types are possible
// integer
int8 // 8 bit signed integer
uint8 // 8 bit unsigned integer
int16 // 16 bit signed integer
uint16 // 16 bit unsigned integer
int32 // 32 bit signed integer
uint32 // 32 bit unsigned integer
int64 // 64 bit signed integer
uint64 // 64 bit unsigned integer
// float
float
ufloat
// string
string
// bool
bool
// nothing
void
```

Following constants are builtin:

```js
// boolean values
true;
false;
// no value
null;
```

Here are a few examples of possible supported values beside using builtin:

```js
// integer
1234;
// float
12.34;
// hex
0x1234
// strings
"String with content";
"";
"1234";
```

### Arithmetic operators

Overview about supported arithmetic operators

```js
1 + 2 // addition
2 - 1 // subtraction
2 * 5 // multiplication
3 / 2 // division
3 % 2 // modulo
-4 // negotiation
```

### Relational operators

Overview about supported relational operators

```js
1 < 2 // lower than
1 <= 2 // lower than or equal
5 > 2 // greater than
5 >= 2 // greater than or equal
5 == 5 // equal
5 != 10 // not equal

// Comparing same type
5 == 10 // false
5 == 5 // true
"foo" != "bar" // true
"foo" == "foo" // true

// comparing different types
1234 == "foobar" // false
123 == "123" // false
123 == 0x123 // false
```

### Logical operators

Overview about supported logical operations

```js
// not operator
! true // false
! false // true

// logical and
a && b

// logical or
a || b
```

### Bitwise operators

Overview about supported bitwise operations

```js
& // bitwise and
| // bitwise inclusive or
^ // bitwise exclusive or
~ // binary ones complement
<< // binary left shift
>> // binary right shidt

// examples
0xCC & 0xAA // bitwise and example, result: 0x88
0x55 & 0xAA // bitwise inclusive or example, result: 0xFF
0x55 ^ 0xFF // bitwise exclusive or example, result: 0xAA
~0xF0 // binary ones complement example, result: 0x0F
~0xF0 >> 4 // binary left shift example, result: 0x0F
~0x0f << 4 // binary right shift example, result: 0xF0
```

### Assignment operatiors

Overview about supported assignment operators

```js
= // normal assignment

// assignment example
a = 5 + 10 // assign value of addition to a
```

### Operators precedence

| Category       |  Operator  | Associativity |
| -------------- | :--------: | ------------- |
| postfix        | ()         | left to right |
| unary          | + - ! ~    | right to left |
| multiplicative | * / %      | left to right |
| additive       | + -        | left to right |
| shift          | << >>      | left to right |
| equality       | == !=      | left to right |
| bitwise and    | &          | left to right |
| bitwise or     | \|         | left to right |
| bitwise xor    | ^          | left to right |
| logical and    | &&         | left to right |
| logical or     | \|\|       | left to right |
| assignment     | =          | right to left |
| comma          | ,          | left to right |

### Statements

Simple overview about statements

```js
// statements have to be ended with a semicolon

// statement example
print( "Hello" );

// statements in a series / group
{
  print( "hello" );
  print( " " );
  print( "world" );
}
```

### Variables / Constants

Overview about variables. Variables are created using keyword `let` in the following
schema: `let <name>: [pointer] <type>`.

After the keyword let, the name of the variable is defined followed by a colon.
Right after the colon you specify the type, with optional keyword `pointer` marking
the variable as pointer to something other.

Similar scheme is also used for constants. Here we've exchange only `let` by `const`:
`const <name>: [pointer] <type>`.

Possible types are `int[8|16|32|64]`, `uint[8|16|32|64]`, `float`, `ufloat`
and `string`.

```js
// normal variables are reserved by using 'let' followed by bit width
let a: uint32 = 5;
let b: float = 5.5;
let c: uint16 = 0xFF;
let my_string: string = "string";
let null_value: uint8;
// pointer
let pointer_to_a: pointer uint32 = a;
// constants
const foo: uint16 = 0xAB;

// change value with pointer variable
pointer_to_a = 5;
// increment pointer to next address
pointer pointer_to_a = pointer pointer_to_a + 1;
```

### Control flow

Overview about control flow

```js
// if condition
if ( expression ) {
  print "foo";
}

// if with else condition
if ( expression ) {
  print "foo";
} else {
  print "bar";
}

// if with elseif condition
if ( expression1 ) {
  print "expression1";
} elseif ( expression2 ) {
  print "expression2";
} else {
  print "else";
}

// while loop
let i: uint32 = 0;
while ( i < 5 ) {
  print i * 2;
  i = i + 1;
}

// for loop
for ( let i: uint32 = 0; i < 5; i = i + 1 ) {
  print i * 2;
}
```

### Functions

Functions are written by starting with the keyword fn in the following schema:
`fn <name>( [<parameter_name>: <type>, ...] ): <type> {}`.

After the keyword `fn` the name is set followed by round brackets with optional
parameters. After the round brackets the return type is specified with a colon
and the type. Finally the function body is added surrounded by braces.

To return a value from within a function, return keyword has to be used.

Parameter types are mandatory and have to be passed after a colon. Return type
is also mandatory and has to be passed after a colon after closing round bracket.

```js
// function without return
fn foo( parameter1: uint32 ): void {
  print parameter1;
}

// function with return
fn bar( parameter1: uint16 ): uint16 {
  return 2 * parameter1;
}
```

### Loading values from application that embeds the script

It's possible to load values provided by container application using keyword
`load` as assignment value:

```js
// import a pointer to some memory
let p2: pointer uint32 = load pointer uint32 some_pointer_name;
// import some value
let val: uint32 = load uint32 some_value_name;

// change content pointer is pointing to ( changes also content outside the container )
p2 = 5;
// read content pointer is pointing tosgcheck
let v: uint32 = p2;
```

### Loading funcrions from application that embeds the script for execution

To use functions the container application is providing, some sort of function
alias has to be added. The function is written as usual and the body is followed
by assignment and `load fn` and a name. The body should be left empty as it's
content is not going to be considered, but with empty body.

```js
// make a function usable within script provided by container
fn foo( parameter1: uint16, parameter2: uint16 ): void {} = load fn some_function_name_foo;
fn foo2( parameter1: uint16, parameter2: uint16 ): void {
  // this content is completely ignored here
  let never_executed: uint16 = 5;
} = load fn some_function_name_foo;
fn bar( parameter1: uint16 ): uint16 {} = load fn some_function_name_bar;
// call the aliased function
foo( 5, 6 );
let a = bar( 10 );
```

### Builtin functions

```js
print( str_to_print: string ) // print a string
```

## API

Overview about the api on how to use the parse scripts within a container application.

### Generic

TBD

### Overloading builtin functions in container

It's possible to overload builting functions in container, which might become
necessary, when you want to route all prints to some custom special printing
routine printing to memory.

```c
// TBD
```

### Providing functions from container at script

It's possible to provide additional functions defined in container to be used
from within the script by creating an alias within the script.

```c
// TBD
```

### Providing variables from container at script

It's possible to provide variables from container, which then can be laoded from
within the script itself using the keyword `load <type>` or `load pointer <type>`.

```c
// TBD
```
