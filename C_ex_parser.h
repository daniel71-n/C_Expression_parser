#include <stdint.h>


// enum used for specifying the type of notation (e.g. in ExP_compute())
typedef enum expression_notation{PREFIX, INFIX, POSTFIX} ex_notation;



/* Compute expression. Expression can be either a (valid, properly formatted),
 * prefix expression or postfix expression. 
 * If prefix, ex_notation is PREFIX.
 * if postfix, ex_notation is POSTFIX.
 *
 * 'Properly Formatted' means no non-numerical characters are allowed,
 * the rules surrounding whitespace are that there's no requirement
 * for there to be whitespace between operators or an operator
 * and an operand, but operands need to be separated from one 
 * another with whitespace.
 * I.e.|  +47 3 | is easily parsable, but +473 is not, since
 * it's impossible to tell with any certainty which and how many
 * digits belong to which operator.
*/
int32_t ExP_compute(char expression[], ex_notation NOTATION);

/* Convert a prefix expression to a postfix expression */
char *ExP_to_postfix(char expression[]);

/* Convert a postfix expression to a prefix expression */
char *ExP_to_prefix(char expression[]);

/* Convert a postfix or prefix expression to an infix notation expression */
char *ExP_to_infix(char expression[], ex_notation NOTATION);
