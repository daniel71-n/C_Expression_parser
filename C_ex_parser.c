#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pstrings.h"
#include "stack.h"
#include "C_ex_parser.h"


/* ******************************************************* */
/* -------------> Abbreviations <------------------------ */
/*
   ExP = expression parser
   ExTree = expression tree
    et ( in ExP_prefix_build_et() ) = ditto
*/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ------------------ Structs and Typedefs ------------------- */

typedef struct expression_tree *ExTree;
struct expression_tree{
    char *token;    // a string (char pointer) as a token (substring) in an expression string
    ExTree left;
    ExTree right;
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ----------------------- Private Functions ------------------------- */


/*               * * * ExTree functions * * *                         */

static ExTree ExTree_new(char *token){
    /* Allocate memory for a new tree, initialize the 
       children pointers to NULL and the token field
       to the argument passed, then return an ExTree.
    */
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
       right_child is to be makde the right child of tree.

       Return tree after setting its 'right' field to right_child.
    */
   tree->right = right_child; 
   return tree;

}

static ExTree ExTree_insert_left(ExTree tree, ExTree left_child){
    /* right_child is a tree with both children set to NULL.
       right_child is to be makde the right child of tree
       
       Return tree after setting its 'right' field to right_child.
    */
   tree->left = left_child; 
   return tree;

}


// forward declaration of ExP_eval, since it's defined down below in the next section,
// but referred to in t he body of ExTree_traverse()
static char *ExP_eval(char *operator, char *left_operand, char *right_operand);
//
static char *ExTree_traverse(ExTree tree){
    /* Traverse the expression tree 'tree', and compute a 
       result, then return it. 

       The traversal order - in-order, pre-order, post-order
       corresonds to the expression notation : infix, prefix,
       and postfix, respectively. 

       This makes it straightforward to convert between notations.

       It takes an Extree and returns a string (char array).
    */
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

static char *ExTree_traverse_postorder(ExTree ex_tree, char **string_ref){
    /* This modifies the string that string_ref points to back in caller space.
       Thus the caller should mainting two pointers to its string. 

       One to be modified by ExTree_traverse_post_order() and one kept intact,
       out of harm's way.
    */
    if (!ex_tree){
        return NULL;
    }
    ExTree_traverse_postorder(ex_tree->left, string_ref);
    ExTree_traverse_postorder(ex_tree->right, string_ref);

    // copy the token in the current tree node into the array
    // precede it  with white space
    *(*string_ref) = ' ';
    (*string_ref)++;    // move forward in the string;
    // how many characters the token consisted of+1 (when the token string reached NULL,
    // and the index stopped incrementing in str_copy())
    int32_t copied = str_copy(*string_ref, ex_tree->token);
    (*string_ref)+= copied;

    // return a pointer in the newly formed expression, where the computing left off
    // the whole expression ahs been computed. The caller should use this poimnter
    // to insert a NULL there and terminate the string
    return *string_ref;
}


static char *ExTree_traverse_preorder(ExTree ex_tree, char **string_ref){
    /* This modifies the string that string_ref points to back in caller space.
       Thus the caller should mainting two pointers to its string. 

       One to be modified by ExTree_traverse_preorder() and one kept intact,
       out of harm's way.
    */
    if (!ex_tree){
        return NULL;
    }

    // copy the token in the current tree node into the array
    // precede it with white space 
    *(*string_ref) = ' ';
    (*string_ref)++;    // move forward in the string;
    // how many characters the token consisted of+1 (when the token string reached NULL,
    // and the index stopped incrementing in str_copy())
    int32_t copied = str_copy(*string_ref, ex_tree->token);
    (*string_ref)+= copied;

    // return a pointer in the newly formed expression, where the computing left off
    // the whole expression ahs been computed. The caller should use this poimnter
    // to insert a NULL there and terminate the string
    ExTree_traverse_preorder(ex_tree->left, string_ref);
    ExTree_traverse_preorder(ex_tree->right, string_ref);

    return *string_ref;
}

// declaration here because it's used in the following function; (defined in down below in
// the file, in another section)
static bool ExP_is_operator(char the_char);

static char *ExTree_traverse_inorder(ExTree ex_tree, char **string_ref){
    /* This modifies the string that string_ref points to back in caller space.
       Thus the caller should mainting two pointers to its string. 

       One to be modified by ExTree_traverse_preorder() and one kept intact,
       out of harm's way.
    */
    if (!ex_tree){
        return NULL;
    }

    // only insert parentheses if the current token is an operator 
    // this bool will be checked twice below to determine the insertion of both
    // the opening and the closing parenthesis
    bool parentheses = false; 
    parentheses = ExP_is_operator(ex_tree->token[0]);

    // copy the token in the current tree node into the array
    if (parentheses){
        *(*string_ref) = '(';
        (*string_ref)++;    // move forward in the string;
    }
    ExTree_traverse_inorder(ex_tree->left, string_ref);

    // how many characters the token consisted of+1 (when the token string reached NULL,
    // and the index stopped incrementing in str_copy())
    int32_t copied = str_copy(*string_ref, ex_tree->token);
    (*string_ref)+= copied;

    // return a pointer in the newly formed expression, where the computing left off
    // the whole expression ahs been computed. The caller should use this poimnter
    // to insert a NULL there and terminate the string
    ExTree_traverse_inorder(ex_tree->right, string_ref);

    // add closing parenthesis as well (if parentheses is true)
    if (parentheses){
        *(*string_ref) = ')';
        (*string_ref)++;
    }

    return *string_ref;
}




/*               * * * ExP functions * * *                         */


static bool ExP_is_operator(char the_char){
    /* Determine whether the_char is an operator.
       If it is, return true, else false.
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


static char *ExP_eval(char *operator, char *left_operand, char *right_operand){
    /* Evaluate the expression consisting of the two operands and
       operator and return the result.

       The arguments are all char arrays, so is the return value. They're 
       first converted to integers (the operands) and then evaluated 
       and the result stored in a variable. 
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

        case '^':   // exponentiation
            result = str_to_int(left_operand) << str_to_int(right_operand); 

        default:
            return false;
            break;
    }
    return str_from_int(result);

}

static char *ExP_refine(char *unformatted, ex_notation NOTATION){
    /* Copy the contents from the unformatted array into another array,
       allocated dynamically, but add whitespace around the operator
       tokens as needed.

       Return this 'refined' array to the caller for safe parsing.
       Called to 'sanitize' the input to be fed to ExP_tokenize().
    */
      char *refined = malloc(sizeof(char) * 2*str_len(unformatted));
        if (!refined){
            return NULL;
        }

    uint32_t ind_refined = 0;   // track the position in the 'refined' array
    refined[ind_refined] = '\0';    // initialize to NUL

    for (uint32_t i = 0; unformatted[i] != '\0'; i++){

        if(unformatted[i] == ' '){
            // don't add space if there's one just before
            if (ind_refined > 0 && refined[ind_refined-1] != ' '){     // if the previous char is not whitespace
                refined[ind_refined] = ' ';     // then add whitespace here
                ind_refined++;
            }
        }

        else if(ExP_is_operator(unformatted[i])){
            //add space before it if there's no space at that index in the array already
            //added. no out-of-bounds error can occur since the left-most element is 
            //always an operand in a valid postfix expression 
            // edit: it will crash, of course, if it's a prefix expression, since the
            // first char is an operator there
            if (ind_refined > 0 && refined[ind_refined-1] != ' '){
                refined[ind_refined] = ' ';
                ind_refined++;
            }
            refined[ind_refined] = unformatted[i];
            //add a space after it as well
            refined[ind_refined+1] = ' ';
            ind_refined += 2;
        }
            // if it's not whitespace or an operator, insert it as it is. 
            // this counts on the input being an otherwise valid expression and not
            // containing non-numeric character and whatnot
        else{
            refined[ind_refined] = unformatted[i];
            ind_refined++;
        }

    } 
    // postfix expressions always end with an operator, so white space will be added after
    // it, and the index in the array incremented to after that newly-added space. 
    // ind-refined-1 below thus refers to the white space. Set it to null, since the
    // expression ends here.
    if (NOTATION == POSTFIX){
        refined[ind_refined-1] = '\0';
    }
    return refined;
}


static char *ExP_tokenize(char string_arg[], char delimiter){
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
        input_string = string_arg;
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



// returns an ExTree object, which is an expression tree.
static ExTree ExP_parse_postfix(char postfix_expression[]){
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

    postfix_expression = ExP_refine(postfix_expression, POSTFIX); 
    char *current = ExP_tokenize(postfix_expression, ' ');

    while(current){
        if(!ExP_is_operator(current[0])){
            // make current a new tree with no children
            ExTree new = ExTree_new(current);
            
            // push this new childless tree onto the stack
            StackItem operand = Stack_make_item(new);
            Stack_push(operands_stack, operand);

            current = ExP_tokenize(NULL, ' '); 
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

            current = ExP_tokenize(NULL, ' '); 
        }
    }
    Stack_destroy(&operands_stack);
    return result; 
}


static void ExP_prefix_build_et(ExTree *tree_ref, char *token){
    /* Called from inside ExP_parse_prefix(). 

       It returns nothing, as it builds a tree back in the caller's space,
       as it traverses it.

       Used to build the expression tree for ExP_parse_prefix(). The calling
       function in this case serves as a nonlocal-scope providing wrapper.
    */
    if (!token){ // token is NULL
       return; 
    }
    else{
        if (!(*tree_ref)){  // if the place in the tree is void, continue building
            ExTree tree = ExTree_new(token);
            *tree_ref = tree;
        }

            // we've placed this subtree in the right place.
            // now : either return, if the token is an operand,
            // or recurse, if the token is an operator

            // if token[0] is an operator (accepted operators are always one-char wide here)
        if (!ExP_is_operator(*token)){
            return;
        }
        else{   // the token is an operator: recurse, and advance down the expression string by getting the next token
            // recurse first to the left, then to the right
            // because the first operand found is the left operand, the next one the right one

            //arguments: 1) a pointer to the 'left' pointer field  of the tree pointer
            //that tree_ref pointers to (pretty difficult to succintly explain)
            // 2) a call to ExP_tokenize which returns the next token in the expression
            ExP_prefix_build_et(&(*tree_ref)->left, ExP_tokenize(NULL, ' '));
            // recurse and go down the right child node in *tree_ref
            ExP_prefix_build_et(&(*tree_ref)->right, ExP_tokenize(NULL, ' '));
        }
    }
}



static ExTree ExP_parse_prefix(char* exp){
    /* Parse prefix_expression and build an expression 
       tree out of it and return that. 

       prefix_expression is assumed to be a valid 
       expression in prefix notation. The first
       character is always an operator and the last character
       is always an operand. 
       
       Blank spaces are used to separate operands. I.e.
       '+374' is not a valid expression, since it could be
       either 37+4 or 3+74, depending on how the parsing is
       done. '+3 7' and '+  3 7 ', on the other hand, are both
       valid expressions. Spaces are necessary between operands,
       but not necessary between operators or an operator and an
       operand. 

       The postfix_expression argument is a string, and as such
       it has to be Nul-terminated, or it's not valid input.
    */
      
    // if the current char in exp is NULL : end of the expression 
    
    exp = ExP_refine(exp, PREFIX);
    if (!exp){
        return NULL;
    }
    char *token = ExP_tokenize(exp, ' ');
    ExTree expression_tree = NULL;
    ExP_prefix_build_et(&expression_tree, token);
    
    return expression_tree;
} 
  
 
 

    
/* ------------------------- END PRIVATE ----------------------------- */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int32_t ExP_compute(char expression[], ex_notation NOTATION){
    /* Parse the postfix_expression, build an expression tree,
       then traverse this tree and evaluate the expression,
       then return the computed result.
    */
    char *res = NULL;


    switch(NOTATION){

        case(PREFIX):
        {
            ExTree expression_tree = ExP_parse_prefix(expression);
            res = ExTree_traverse(expression_tree); 
            break;
        }

        case(POSTFIX):
        {
           ExTree expression_tree = ExP_parse_postfix(expression);
           res = ExTree_traverse(expression_tree); 
           break;
       }

        default:
        {
            printf("Function incorrectly called.\n");
        }
    }

    if (!res){
        return -1;
    }else{
        return str_to_int(res);
    }
}



char *ExP_to_postfix(char expression[]){
    /* Convert from prefix to postfix */
    // make it twice the size of expression, to play it safe and be able to add proper
    // spacing
    uint8_t size = str_len(expression) * 2;

    // this pointer will be changed by ExTree_traverse_postorder()
    // so a second pointer, left intact, pointing to the start of the string, is needed
    char *changeable = malloc(sizeof(char) * size); 
    char *intact = changeable;


    if(!changeable){
        return NULL;
    }

    ExTree expression_tree = ExP_parse_prefix(expression);
    ExTree_traverse_postorder(expression_tree, &changeable);

    // changeable will at this point point to where a terminating null needs to be
    // inserted.
    *changeable= '\0';

    // intact was left intact, so it sstill points to the start of the string
    //but
    // since ExTree_traverse_preorder (and postorder) insert a whitespace char before
    // every token in the expression tree, the first char in the string will be
    // whitespace.
    // remove that
    intact++;

    return intact;
}






char *ExP_to_prefix(char expression[]){
    /* Convert from prefix to postfix */
    // make it twice the size of expression, to play it safe and be able to add proper
    // spacing
    uint8_t size = str_len(expression) * 2;

    // this pointer will be changed by ExTree_traverse_postorder()
    // so a second pointer, left intact, pointing to the start of the string, is needed
    char *changeable = malloc(sizeof(char) * size); 
    char *intact = changeable;


    if(!changeable){
        return NULL;
    }

    ExTree expression_tree = ExP_parse_postfix(expression);
    ExTree_traverse_preorder(expression_tree, &changeable);

    // changeable will at this point point to where a terminating null needs to be
    // inserted.
    *changeable= '\0';
    // since ExTree_traverse_preorder (and postorder) insert a whitespace char before
    // every token in the expression tree, the first char in the string will be
    // whitespace.
    // remove that
    intact++;

    // intact was left intact, so it sstill points to the start of the string
    return intact;
}


char *ExP_to_infix(char expression[], ex_notation NOTATION){
    /* Convert from expression to postfix */

    // make it twice the size of expression, to play it safe and be able to add proper
    // spacing
    uint8_t size = str_len(expression) * 2;

    // this pointer will be changed by ExTree_traverse_postorder()
    // so a second pointer, left intact, pointing to the start of the string, is needed
    char *changeable = malloc(sizeof(char) * size); 
    char *intact = changeable;


    if(!changeable){
        return NULL;
    }
    
    ExTree expression_tree = NULL;
    if (NOTATION == PREFIX){
        expression_tree = ExP_parse_prefix(expression);
    }else if (NOTATION == POSTFIX){
        expression_tree = ExP_parse_postfix(expression);
    }
    ExTree_traverse_inorder(expression_tree, &changeable);

    // changeable will at this point point to where a terminating null needs to be
    // inserted.
    *changeable= '\0';
    // since ExTree_traverse_preorder (and postorder) insert a whitespace char before
    // every token in the expression tree, the first char in the string will be
    // whitespace.
    // remove that
    intact++;

    // intact was left intact, so it sstill points to the start of the string
    return intact;
}



