Post4 (Post-Forth)
==================

Copyright 2007, 2024 Anthony Howe.  All rights reserved.


Building
---------

Post4 is written in ISO C11 using only one extension, `Labels As Values`, which is supported by `gcc` and `clang` compilers.  The bulk of the floating point support is provided by ISO C11 `libm`, which is enabled by default.  It can be disabled.

        $ ./configure --disable-math


### Floating Point Words

#### >FLOAT
( `caddr` `u` -- `F:f` `bool` )  
Convert the numeric floating point string `caddr` `u` into an internal floating point value `f` on the float stack.  Return true if `caddr` `u` is valid string, otherwise false. 

- - -
#### F\!
( `F:f` `faddr` -- )  
Store `f` at `faddr`.

- - -
#### F*
(F: `f1` `f2` -- `f3` )  
Multiply the top two cells on the float stack.

- - -
#### F**
(F: `f1` `f2` -- `f3` )  
Raise `f1` to the power `f2`, giving the product `f3`.

- - -
#### F+
(F: `f1` `f2` -- `f3` )  
Add the top two cells on the float stack.

- - -
#### F-
(F: `f1` `f2` -- `f3` )  
Subtract `f2` from `f1` placing the result `f3` on the float stack.

- - -
#### F/
(F: `f1` `f2` -- `f3` )  
Divide the top two cells on the float stack.

- - -
#### F.
(F: `f` -- )  
Display, with a trailing space, the top number on the float stack using fixed-point notation:

        [-] 〈digits〉.〈digits0〉

- - -
#### F0\<
(F: `f1` `f2` -- `f3` )  
`bool` is true if and only if `f` is less than zero (0).

- - -
#### F0=
(F: `f` -- ) ( -- `bool` )  
`bool` is true if and only if `f` equals  zero (0).

- - -
#### F\<
(F: `f1` `f2` --  ) ( -- `bool`)  
`bool` is true if and only if `f1` is less than `f2`.

- - -
#### F>S
(F: `f` -- )( -- `n`)  
Convert the float `f` to a single-cell signed integer `n`, the fractional portion of `f` is discarded.

- - -
#### F@
( `faddr` -- ) (F: -- `f` )  
Fetch from `faddr` the value `f` stored there.

- - -
#### FABS
(F: `f1` -- `f2` )  
`f2` is the absolute value of `f1`.

- - -
#### FALIGN
( -- )  
If the data-space pointer is not float aligned, `reserve` enough space to align it.

- - -
#### FALIGNED
( `addr` -- `faddr` )  
`faddr` is the first float aligned address greater than or equal to `addr`.

- - -
#### FCONSTANT
(F: `f` -- ) ( `<spaces>name` -- )  
Define the word `name` to represent the float constant value `f`.  When `name` is executed, the value `f` is pushed on to the float stack.

- - -
#### FCOS
(F: `f1` -- `f2` )  
`f2` is the cosine of the radian angle `f1`.

- - -
#### FDEPTH
( -- `u` )  
Number of cells on the float stack before `f` was placed there.

- - -
#### FDROP
(F: `f` -- )  
Remove the top of the float stack.

- - -
#### FDUP
(F: `f` -- `f` `f` )  
Duplicate `f` on the float stack.

- - -
#### FEXP
(F: `f1` -- `f2` )  
Raise `e` to the power `f1`, giving `f2`.

- - -
#### FFIELD:
( `n1` `<spaces>name` -- `n2` )  
Skip leading space delimiters.  Parse name delimited by a space.  Offset is the first float aligned value greater than or equal to `n1`.  `n2 = offset + 1` float.

- - -
#### FLITERAL
(F: `f` -- ) immediate  
Compile `f` into the definition so that it is later pushed onto the float stack during execution of the definition.

- - -
#### FLN
(F: `f1` -- `f2` )  
`f2` is the natural logarithm of `f1`.

- - -
#### FLOAT+
( `faddr1` -- `faddr2` )  
Add the size in address units of a float to `faddr1` giving `faddr2`.

- - -
#### FLOATS
( `n1` -- `n2` )  
`n2` is the size in address units of `n1` floating point numbers.

- - -
#### FLOG
(F: `f1` -- `f2` )  
`f2` is the base-10 logarithm of `f1`.

- - -
#### FLOOR
(F: `f1` -- `f2` )  
Round `f1` to an integral value using the "round toward negative infinity" rule, giving `f2`.

- - -
#### FMAX
(F: `f1` `f2` -- `f3` )  
Float `f3` is the greater of `f1` and `f2`.

- - -
#### FMIN
(F: `f1` `f2` -- `f3` )  
Float `f3` is the lesser of `f1` and `f2`.

- - -
#### FNEGATE
(F: `f1` -- `f2` )  
Negate the float `f1`, giving its arithmetic inverse `f2`.

- - -
#### FOVER
(F: `f1` `f2` -- `f1` `f2` `f1` )  
Copy the second value `f1` below the float stack to the top.

- - -
#### FROT
(F: `f1` `f2` `f3` -- `f2` `f3` `f1` )  
Rotate the top three float stack entries.

- - -
#### FROUND
(F: `f1` -- `f2` )  
Round `f1` to an integral value using the "round to nearest" rule, giving `f2`.

- - -
#### FS.
(F: `f` -- )  
Display, with a trailing space, the top number on the float stack using scientific notation:

        [-] digits[.digits] E digits

- - -
#### FSIN
(F: `f1` -- `f2` )  
`f2` is the sine of the radian angle `f1`.

- - -
#### FSWAP
(F: `f1` `f2` -- `f2` `f1` )  
Exchange the top two float stack items.

- - -
#### FSQRT
(F: `f1` -- `f2` )  
`f2` is the square root `f1`.

- - -
#### FTAN
(F: `f1` -- `f2` )  
`f2` is the tangent of the radian angle `f1`.

- - -
#### FVALUE
( `f` `<spaces>name` -- )  
Create `name` and reserve data-space to store `f` from the float stack.  When `name` is executed, the value `f` is pushed to the float stack.  See `TO`.

        ok 123.45 FVALUE fizz
        ok fizz f.
        123.450000 ok
        ok 0.314159e1 TO fizz
        ok fizz f.
        3.141590 ok

- - -
#### FVARIABLE
( `<spaces>name` -- )  
Create `name` with one float of data.  When `name` is executed push the `faddr` of the float.

- - -
#### PRECISION
( -- `u` )  
Return the number of significant digits currently used by `F.`, `FE.`, or `FS.` as `u`.

- - -
#### REPRESENT
(F: `f` -- )( `caddr` `u` -- `n` `isneg` `isok` )  
Taking the float `f` return the first `u` digits of the significand, where the is an *implied* decimal point to the left of the first digit, the base-10 exponent `n`, `TRUE` if the value is negative, and `TRUE` if the number is valid (not ± infinity nor NaN).  If `f` does not represent a number, where `isok` is `FALSE`, then the values of `n` and `isneg` are undefined.

- - -
#### S>F
(F: -- `f` )( `n` -- )  
Convert the single-cell signed integer to a float `f`.

- - -
#### SET-PRECISION
( u -- )  
Set the number of significant digits currently used by `F.`, `FE.`, or `FS.` to `u`.

- - -

### Post4 Specific Words

#### >f
(F: -- `f` ) (S: `f` -- )  
Move top of the data stack to the float stack *without* format conversion.  `S>F` will convert formats.

- - -
#### _fs
( -- `aaddr` `n` `s` )  
Push the float stack base `aaddr` address, depth `n`, and size `s` (before executing `_fs`).  This stack is a fixed size and grows upward.  See `.fs` and `_stack_dump`.

- - -
#### f,
(F: `f` -- )  
Align and reserve data-space to store `f` there.

- - -
#### f=
(F: `f1` `f2` -- `bool` )  
Return `bool` as true if `f1` and `f2` are exactly equal (same internal representation).

- - -
#### floating-stack
(  -- `u` ) constant  
Size of the float stack.  Zero (0) if the float stack is combined with the data stack.  This is a deviation from `ENVIRONMENT?` queries.

- - -
#### f>
(F: `f` -- ) (S: -- `f` )  
Move top of the float stack to the data stack *without* format conversion.  `F>S` will  convert formats.

- - -
#### f>r
(F: `f` -- ) (R: -- `f` )  
Move top of the float stack to the return stack.

- - -
#### fr>
(R: `f` -- ) (F: -- `f` )  
Move top of the return stack to the float stack.

- - -
#### _fsp!
( `aaddr` -- )  
Store `aaddr` into the float stack pointer.

- - -
#### _fsp@
( -- `aaddr` )  
Fetch the float stack pointer.

- - -
