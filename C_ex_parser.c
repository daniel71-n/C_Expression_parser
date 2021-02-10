#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pstrings.h"
#include "stack.h"
#include "C_ex_parser.h"


typedef struct expression_tree *ExTree;
struct expression_tree{
    char *token;
    ExTree left;
    ExTree right;
};

static ExTree ExTree_new(char *token){
    ExTree new = malloc(sizeof(struct expression_tree));
    if(!new){
        return NULL;
    }
    new->left = NULL;
    new->right = NULL;
    new->token = token;

    return new;
}

static ExTree ExTree_insert_right(ExTree tree, ExTree right_child){
    /* right_child is a tree with both children set to NULL.
       right_child is ro be makde the right child of tree
    */
   tree->right = right_child; 
   return tree;

}

static ExTree ExTree_insert_left(ExTree tree, ExTree left_child){
    /* right_child is a tree with both children set to NULL.
       right_child is ro be makde the right child of tree
    */
   tree->left = left_child; 
   return tree;

}


static bool ExP_is_operator(char the_char){
    /*Return true if the_char is an operator. 
      False otherwise.
    */
    switch(the_char){
        case '+':
            return true;
            break;  // not needed

        case '-':
            return true;
            break;

        case '*':
        case 'x':
            return true;
            break;

        case '/':
            return true;
            break;

        default:
            return false;
            break;
    }
}


static char *ExT_refine(char *unformatted){
    /* Copy the contents from the unformatted array into another array,
       allocated dynamically, but add whitespace around the operator
       tokens as needed.

       Return this 'refined' array to the caller for safe parsing.
    */
      char *refined = malloc(sizeof(char) * 2*str_len(unformatted));
        if (!refined){
            return NULL;
        }

    uint32_t ind_refined = 0;
    for (uint32_t i = 0; unformatted[i] != '\0'; i++){

        if(unformatted[i] == ' '){
            // don't add space if there's one just before
            if (refined[ind_refined-1] != ' '){
                refined[ind_refined] = ' ';
                ind_refined++;
            }
        }

        else if(ExP_is_operator(unformatted[i])){
            //add space before it if there's no space at that index in the array already
            //added. no out-of-bounds error can occur since the left-most element is 
            //always an operand in a valid reverse polish notation expression
            if (refined[ind_refined-1] != ' '){
                refined[ind_refined] = ' ';
                ind_refined++;
            }
            refined[ind_refined] = unformatted[i];
            //add a space after it as well
            refined[ind_refined+1] = ' ';
            ind_refined += 2;
        }

        else{
            refined[ind_refined] = unformatted[i];
            ind_refined++;
        }

    }
    refined[ind_refined-1] = '\0';
    return refined;
}


char *ExT_tokenize(char string_arg[], char delimiter){
    /* Return the next token in string_arg when called.

       A token is a substring delimitated by delimiter.
       e.g. 'my_name_is_x' has 4 tokens and 3 delimiter
       instances..

       The initial call to the function has to specify the 
       string to tokenize. After that, unless another string
       needs to be tokenized, the function is called with NULL
       as the first argument, to indicate that it should continue
       tokenizing the same string.

       The function maintains internal state by using static variables.
       Since the whole input string is not tokenized at once, but bit by
       bit with each call, this is a form of lazy evaluation.
    */
    // static variables, only initialized when string_arg is not NULL 

    static char *input_string;
    static char *token;    // a pointer to the next token found;
    static int32_t index;
    static char *res;

    // if string_arg is not NULL, (re) initialize the static variables (internal state)
     if (string_arg){
        input_string = ExT_refine(string_arg);
        token = input_string;
        index = 0;
        res = NULL;
     }
   

    while(input_string[index] != '\0'){
        if (input_string[index] == delimiter){
            res = token; 
            input_string[index] = '\0';  // replace the delimiter with a NUL, to terminate the string (token) here
            token = &input_string[index+1];
            index++;

            return res;
        }
        index++;
    }
    // the loop is exhausted; last token -> return it
    // NOTE: what's left here is the rest of the original input string from after the last token
    // until the final Nul

    // return this last bit
    res = token;
    // then set token to null, so that on the next call the reurn value is null
    token = NULL;
    return res;
}


static char *ExP_eval(char *operator, char *left_operand, char *right_operand){
    /* Evaluate the expression consisting of the two operands and
       operator and return the result.

       The arguments are all char arrays. They're first converted to 
       integers (the operands) and then evaluated and the result stored
       in a variable. 
       Before returning,  the variable is converted to a char pointer (array).
    */
    int result;
    switch (operator[0]){
        case '+':
            result = str_to_int(left_operand) + str_to_int(right_operand); 
            break;  

        case '-':
            result = str_to_int(left_operand) - str_to_int(right_operand); 
            break;

        case '*':
        case 'x':
            result = str_to_int(left_operand) * str_to_int(right_operand); 
            break;

        case '/':
            result = str_to_int(left_operand) / str_to_int(right_operand); 
            break;

        default:
            return false;
            break;
    }
    return str_from_int(result);

}

// returns an ExTree object, which is an expression tree.
ExTree Parse_postfix(char postfix_expression[]){
    /* Parse postfix_expression and build an expression 
       tree out of it and return that. 

       postfix_expression is assumed to be a valid 
       expression in polish postfix notation. The first
       character is always an operand and the last character
       is always an operator. 
       
       Blank spaces are used to separate operands. I.e.
       '374+' is not a valid expression, since it could be
       either 37+4 or 3+74, depending on how the parsing is
       done. '3 7+' and '3 7 +', on the other hand, are both
       valid expressins. Spaces are necessary between operands,
       but not necessary between operators or an operator and an
       operand. 

       The postfix_expression argument is a string, and as such
       it has to be Nul-terminated, or it's not valid input.
    */
    Stack operands_stack;
    Stack_init(&operands_stack);

    ExTree result = NULL;
    char *current = ExT_tokenize(postfix_expression, ' ');
    while(current){
        if(!ExP_is_operator(current[0])){
            // make current a new tree with no children
            ExTree new = ExTree_new(current);
            
            // push this new childless tree onto the stack
            StackItem operand = Stack_make_item(new);
            Stack_push(operands_stack, operand);

            current = ExT_tokenize(NULL, ' '); 
        }
        else{   // current is an operator, compute
            // make a new tree with the operator as the key and 
            // the two operands as its children
            ExTree new_tree = ExTree_new(current);
            // get the children from the stack
            ExTree right_operand = Stack_pop(operands_stack);
            ExTree left_operand = Stack_pop(operands_stack);
             // result = ExP_eval(current, left_operand, right_operand);
            
            // assign the children to the parent tree
            new_tree = ExTree_insert_right(new_tree, right_operand);
            new_tree = ExTree_insert_left(new_tree, left_operand);
            result = new_tree;

            // push the tree onto the stack
            StackItem new = Stack_make_item(new_tree);
            Stack_push(operands_stack, new);

            current = ExT_tokenize(NULL, ' '); 
        }
    }
    return result; 
}



static char *ExTree_traverse(ExTree tree){
    if(tree->left == NULL && tree->right == NULL){
        return tree->token;
    }
    else{
        char *left = ExTree_traverse(tree->left); 
        char *right = ExTree_traverse(tree->right);
        char *res = ExP_eval(tree->token, left, right);
        return res;
    }
}

int32_t Ex_compute(char postfix_expression[]){
    /* Parse the postfix_expression, build an expression tree,
       then traverse this tree and evaluate the expression,
       then return the computed result.
    */
    ExTree expression_tree = Parse_postfix(postfix_expression);
    char *res = ExTree_traverse(expression_tree); 
    return str_to_int(res);
}





