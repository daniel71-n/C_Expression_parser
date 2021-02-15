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
   PV = previous version
*/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ------------------ Structs and Typedefs ------------------- */

// actual Expression tree that gets built, traversed, and deallocated at the end
typedef struct expression_tree *ExTree;


/* A wrapper struct with a pointer to an ExTree that it's associated with, and a pointer
   to the expression string that the ExTree is built from (this expression string is a dynamically
   allocated char array returned by ExP_refine() ). 
   The purpose of this wrapper is primarily to track that expression string 
   (char array) ptr so that it can get freed together with the tree itself when the 
   tree gets deallocated. 
   ExTreeWrapper is passed to tree_destroy() and tree_init().
   The member pointer expression_tree is what gets passed to all the tree-handling functions.
*/
typedef struct expression_tree_wrapper *ExTreeWrapper;


struct expression_tree{
    char *token;    // a string (char pointer) as a token (substring) in an expression string
    ExTree left;
    ExTree right;
};

struct expression_tree_wrapper{
    char *expression_string;   // the expression that expression_tree was built from
    ExTree expression_tree;     // the expression tree built from the expression string
};


// --------------------------------------------------------------------------------
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// --------------------------------------------------------------------------------



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ----------------------- Private Functions ------------------------- */


/*               * * * ExTree functions * * *                         */


static void ExTree_init(ExTreeWrapper *tree_wrapper, char *expression_string){
    /* Initialize an ExTreeWrapper.
       Specifically--
       - allocate memory to an ExTreeWrapper pointer
       - set its expression_tree member to NULL
       - set its expression_string member to the second argument

       The whole point of this wrapper is essentially the expression_string
       field. That way, when ExTree_destroy() is called to tear down the 
       expression tree, deallocating all the associated memory, the memory
       associated with expression_string will also get deallocated,
       avoiding any memory leaks. 
       In other words, it makes for added convenience and consistency
       as far as keeping track of the memory that needs to be deallocated.
       Otherwise, it's not actually needed for any tree-specific handling.
    */
    ExTreeWrapper temp = malloc(sizeof(struct expression_tree_wrapper));
    if (!temp){
        *tree_wrapper = NULL;
        return;
    }
    *tree_wrapper = temp;

    (*tree_wrapper)->expression_tree = NULL;
    (*tree_wrapper)->expression_string = expression_string;
}


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
static int32_t ExP_eval(char *operator, int32_t left_operand, int32_t right_operand);
//
static int32_t ExTree_traverse(ExTree tree){
    /* Traverse the expression tree 'tree', and compute a 
       result, then return it. 


       ******** IMPLEMENTATION NOTES *********

       The initial idea was for it to return a char array,
       and ExP_eval() would also take char array arguments,
       internally converting them to integers by calling 
       str_to_int(). That worked well, except str_to_int()
       dynamically allocates memory for a char array and returns it.
       That proved to be a huge pain in terms of freeing all that memory,
       leading to a lot of memory leaks. Memory couldn't conveniently
       be freed. 
       So instead I decided to change both ExP_eval() and ExTree_traverse:
       the former takes int-type operands instead of char array-operands,
       and the latter returns an int (int32_t) type. The operand tokens
       are converted to ints with str_to_int() before being passed to
       ExP_eval(), thus essentially doing the exact opposite of the
       initial implementation.
    */
    if(tree->left == NULL && tree->right == NULL){
        return str_to_int(tree->token);
    }
    else{
        int32_t left = ExTree_traverse(tree->left); 
        int32_t right = ExTree_traverse(tree->right);
        int32_t res = ExP_eval(tree->token, left, right);
        return res;

    }
}

static char *ExTree_traverse_postorder(ExTree ex_tree, char **string_ref){
    /* Traverse the expression tree ex_tree in post-order, building
       a postfix expression, stored in *string_ref. 
       
       This modifies the string that string_ref points to back in caller space.
       Thus the caller should mainting two pointers to its string: one to be modified 
       by ExTree_traverse_post_order() and one kept intact, out of harm's way.
    */
    if (!ex_tree){
        return NULL;
    }
    ExTree_traverse_postorder(ex_tree->left, string_ref);
    ExTree_traverse_postorder(ex_tree->right, string_ref);

    // copy the token in the current tree node into the array
    int32_t copied = str_copy(*string_ref, ex_tree->token);
    // str_copy() returns the index in string_ref where it finished copying
    // the token, +1; so if the token is 2 chars-long, the returned value will be
    // 2, having copied one char into index0, one into index1, and stopped at 2,
    // which is what it then returns
    (*string_ref)+= copied;     // move to the next index available for subsequent insertions

    // follow it with white space
    *(*string_ref) = ' ';
    (*string_ref)++;    // move forward in the string;

    // return a pointer in the newly formed expression, where the computing left off
    // the whole expression ahs been computed. The caller should use this poimnter
    // to insert a NULL there and terminate the string
    return *string_ref;
}


static char *ExTree_traverse_preorder(ExTree ex_tree, char **string_ref){
    /* Traverse ex_tree in pre-order and build a char array based on its 
       nodes. Since the traversal is pre-order, the result will be an
       expression in prefix notation.
       
       This modifies the string that string_ref points to back in caller space.
       Thus the caller should mainting two pointers to its string. 

       One to be modified by ExTree_traverse_preorder() and one kept intact,
       out of harm's way.
    */
    if (!ex_tree){
        return NULL;
    }

    // copy the token in the current tree node into the array
    int32_t copied = str_copy(*string_ref, ex_tree->token);
    // str_copy() returns the index in string_ref where it finished copying
    // the token, +1; so if the token is 2 chars-long, the returned value will be
    // 2, having copied one char into index0, one into index1, and stopped at 2,
    // which is what it then returns
    (*string_ref)+= copied;     // move to the next index available for subsequent insertions
 

    // follow it with white space 
    *(*string_ref) = ' ';
    (*string_ref)++;    // move forward in the string;

    // return a pointer in the newly formed expression, where the computing left off
    // the whole expression has been computed. The caller should decrement this pointer
    // and insert a NULL there to terminate the string (i.e. the previous character is
    // white space, and that's where the NUL should be inserted, since the string ends here)
    ExTree_traverse_preorder(ex_tree->left, string_ref);
    ExTree_traverse_preorder(ex_tree->right, string_ref);

    return *string_ref;
}



// declaration here because it's used in the following function; (defined in down below in
// the file, in another section)
static bool ExP_is_operator(char the_char);

static char *ExTree_traverse_inorder(ExTree ex_tree, char **string_ref){
    /* Traverse ex_tree in-order, building an infix expression stored in
       *string_ref.

       This modifies the string that string_ref points to back in caller space.
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



static void ExTree_cut_down(ExTree tree){
    /* Recursively free all memory allocated to tree.

       Meant to be called by ExTree_destroy().
    */
    if (tree == NULL){
        return;
    }
    ExTree_cut_down(tree->left);
    ExTree_cut_down(tree->right);

    free(tree);
    return;
}


static void ExTree_destroy(ExTreeWrapper *tree_wrapper_ref){
    /* Free all heap memory associated with *tree_wrapper_ref
       then set tree_wrapper_ref to NULL.
    */
    if (tree_wrapper_ref == NULL){  // nothing to do
        return;
    }
    if (*tree_wrapper_ref == NULL){
        tree_wrapper_ref = NULL;
        return;
    }
    else{
        ExTree_cut_down((*tree_wrapper_ref)->expression_tree);
        free((*tree_wrapper_ref)->expression_string);
        free(*tree_wrapper_ref);
        
        *tree_wrapper_ref = NULL;
    }
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


static int32_t ExP_eval(char *operator, int32_t left_operand, int32_t right_operand){
// PV: static char *ExP_eval(char *operator, char *left_operand, char *right_operand)
    /* Evaluate the expression consisting of the two operands and
       operator and return the result.
    
       The operands are int32_t integers. The calling function has to convert its
       data, if it's in char-array format instead, to int32_t types by calling
       str_to_int().
    */
    int32_t result;
    switch (operator[0]){
        case '+':
            result = left_operand + right_operand; 
            //PV: result = str_to_int(left_operand) + str_to_int(right_operand); 
            break;  

        case '-':
            result = left_operand - right_operand; 
            break;

        case '*':
        case 'x':
            result = left_operand * right_operand; 
            break;

        case '/':
            result = left_operand / right_operand; 
            break;

        case '^':   // exponentiation
            result = left_operand << right_operand; 

        default:
            return false;
            break;
    }
     // PV: return str_from_int(result);
    return result;

}

static char *ExP_refine(char *unformatted, ex_notation NOTATION){
    /* Copy the contents from the unformatted array into another array,
       allocated dynamically, but add whitespace around the operator
       tokens as needed.

       Return this 'refined' array to the caller for safe parsing.
       Called to 'sanitize' the input to be fed to ExP_tokenize().
    */
    int32_t size = 2*str_len(unformatted);
    char *refined = calloc(size, sizeof(char));     // allocate and initialize everything to 0
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
            token = &input_string[index+1]; // make token point to the first char after NULL
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



static ExTreeWrapper ExP_parse_postfix(char postfix_expression[]){
    /* Parse postfix_expression and build an expression 
       tree out of it. Wrap the expression tree inside
       an ExTreeWrapper, and return that.

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

    postfix_expression = ExP_refine(postfix_expression, POSTFIX); 

    // ExTree wrapper with a pointer to an exp tree and a pointer to the expression char
    // array it's based on -- tracks both for deallocation purposes
    ExTreeWrapper tree_wrapper;
    ExTree_init(&tree_wrapper, postfix_expression);

    ExTree result = NULL;
    char *current = ExP_tokenize(postfix_expression, ' ');

    while(current){
        // current is an operand
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
    tree_wrapper->expression_tree = result; 
    return tree_wrapper; 
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
            //that tree_ref points to (pretty difficult to succintly explain)
            // 2) a call to ExP_tokenize() which returns the next token in the expression
            ExP_prefix_build_et(&(*tree_ref)->left, ExP_tokenize(NULL, ' '));
            // recurse and go down the right child node in *tree_ref
            ExP_prefix_build_et(&(*tree_ref)->right, ExP_tokenize(NULL, ' '));
        }
    }
}



static ExTreeWrapper ExP_parse_prefix(char* exp){
    /* Parse the prefix expression exp and build an expression 
       tree out of it. Wrap the expression tree inside
       an ExTreeWrapper, and return that.

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
    // declare and initialize a tree wrapper that tracks exp and the exp tree that will be
    // built from it, for deallocation purposes
    ExTreeWrapper tree_wrapper;
    ExTree_init(&tree_wrapper, exp);

    char *token = ExP_tokenize(exp, ' ');
    ExP_prefix_build_et(&tree_wrapper->expression_tree, token);

    // the refined expression string: no longer needed
     // free(exp);
    
    return tree_wrapper;
} 
  
 
 

    
/* ------------------------- END PRIVATE ----------------------------- */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int32_t ExP_compute(char expression[], ex_notation NOTATION){
    /* Parse the postfix_expression, build an expression tree,
       then traverse this tree and evaluate the expression,
       then return the computed result.
    */
    int32_t res = 0;


    switch(NOTATION){

        case(PREFIX):
        {
            ExTreeWrapper expression_tree_wrapper = ExP_parse_prefix(expression);
            res = ExTree_traverse(expression_tree_wrapper->expression_tree);
            ExTree_destroy(&expression_tree_wrapper);
            break;
        }

        case(POSTFIX):
        {
            ExTreeWrapper expression_tree_wrapper = ExP_parse_postfix(expression);
            res = ExTree_traverse(expression_tree_wrapper->expression_tree);
            ExTree_destroy(&expression_tree_wrapper);
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
        return res;
    }
}



char *ExP_to_postfix(char expression[]){
    /* Convert from prefix to postfix */
    // make it twice the size of expression, to play it safe and be able to add proper
    // spacing

    if (!expression){
        return NULL;
    }
    uint8_t size = str_len(expression) * 2;

    // this pointer will be changed by ExTree_traverse_postorder()
    // so a second pointer, left intact, pointing to the start of the string, is needed
    char *changeable = malloc(sizeof(char) * size); 
    char *intact = changeable;


    if(!changeable){
        return NULL;
    }

    ExTreeWrapper expression_tree_wrapper = ExP_parse_prefix(expression);
    ExTree_traverse_postorder(expression_tree_wrapper->expression_tree, &changeable);

    ExTree_destroy(&expression_tree_wrapper);

    // changeable will at this point point to where traverse_postorder() left off : a
    // white space was inserted at n, then changeable was incremented to n+1. 
    // Thus the string actually ends at n, so a NUL has to be inserted there
    *(--changeable) = '\0';     // decrement changeable and set the char at that index to NUL
 
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

    // parse an expression in postfix notation and produce (return) an expression tree
    ExTreeWrapper  expression_tree_wrapper = ExP_parse_postfix(expression);
    // traverse this tree in preorder, which wil produce a prefix expression
    ExTree_traverse_preorder(expression_tree_wrapper->expression_tree, &changeable);

    ExTree_destroy(&expression_tree_wrapper);

    // changeable will at this point point to where traverse_preorder() left off : a
    // white space was inserted at n, then changeable was incremented to n+1. 
    // Thus the string actually ends at n, so a NUL has to be inserted there
    *(--changeable) = '\0';     // decrement changeable and set the char at that index to NUL

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
    
    ExTreeWrapper expression_tree_wrapper;

    if (NOTATION == PREFIX){
        expression_tree_wrapper = ExP_parse_prefix(expression);
    }else if (NOTATION == POSTFIX){
        expression_tree_wrapper = ExP_parse_postfix(expression);
    }
    ExTree_traverse_inorder(expression_tree_wrapper->expression_tree, &changeable);

    ExTree_destroy(&expression_tree_wrapper);

    // changeable points to where traverse_inorder() left off. i.e. where NULL has to be
    // inserted.
    *changeable = '\0';
    
    // intact was left intact, so it sstill points to the start of the string
    return intact;
}



