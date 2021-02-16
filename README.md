# C_expression_parser
Parse, compute, and convert between pre,in, and post-fix notation expressions


EXAMPLES

INPUT: 1 * 2 + 3 / 4
POSTFIX: 1 2 * 3 4 / +
PREFIX + * 1 2 / 3 4

INPUT: 1 * (2 + 3 / 4)
POSTFIX: 1 2 3 4 / + *
PREFIX: * 1 + 2 / 3 4

INPUT: 7 * ((2 / 1) * (3 - 1) * 4 - (1 + 11))
POSTFIX: 7 2 1 / 3 1 - * 4 * 1 11 + - *
PREFIX: * 7 - * * / 2 1 - 3 1 4 + 1 11



EVALUATE
 INPUT: 7 * ((2 / 1) * (3 - 1) * 4 - (1 + 11))
 RESULT: 28

