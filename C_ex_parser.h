#include <stdint.h>


// enum used for specifying the type of notation (e.g. in ExP_compute())
typedef enum expression_notation{PREFIX, INFIX, POSTFIX} ex_notation;



/* Compute expression. Expression can be either a (valid, properly formatted),
 * prefix, postfix, or infix -notation expression. 
 * If prefix, ex_notation is PREFIX.
 * If postfix, ex_notation is POSTFIX.
 * If infix, ex_notation is INFIX.
 *
 * 'Properly Formatted' means no non-numerical characters are allowed
 * (with the exception of parenthess in infix notation expressions),
 * the rules surrounding whitespace are that there's no requirement
 * for there to be whitespace between operators or an operator
 * and an operand, but operands need to be separated from one 
 * another with whitespace.
 * Whitespace is not necessary either between an operand or operator
 * and a parenthesis, in infix notation.
 * I.e.|  +47 3 | is easily parsable, but +473 is not, since
 * it's impossible to tell with any certainty which and how many
 * digits belong to which operator.
*/
int32_t ExP_compute(char expression[], ex_notation NOTATION);

/* Convert a prefix or infix expression to a postfix expression */
char *ExP_to_postfix(char expression[], ex_notation NOTATION);

/* Convert a postfix or infix expression to a prefix expression */
char *ExP_to_prefix(char expression[], ex_notation NOTATION);

/* Convert a postfix or prefix expression to an infix notation expression */
char *ExP_to_infix(char expression[], ex_notation NOTATION);





