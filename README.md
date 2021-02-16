# C_expression_parser
Parse, compute, and convert between pre,in, and post-fix notation expressions


*EXAMPLES*

INPUT: 1 * 2 + 3 / 4<br>
POSTFIX: 1 2 * 3 4 / + <br>
PREFIX + * 1 2 / 3 4 <br>
<br>
<br>
INPUT: 1 * (2 + 3 / 4)<br>
POSTFIX: 1 2 3 4 / + * \<br>
PREFIX: * 1 + 2 / 3 4 \<br>
<br>
<br>
INPUT: 7 * ((2 / 1) * (3 - 1) * 4 - (1 + 11))<br>
POSTFIX: 7 2 1 / 3 1 - * 4 * 1 11 + - *<br>
PREFIX: * 7 - * * / 2 1 - 3 1 4 + 1 11 \<br>
<br>
<br>
<br>
EVALUATE<br>
 INPUT: 7 * ((2 / 1) * (3 - 1) * 4 - (1 + 11)) <br>
 RESULT: 28

