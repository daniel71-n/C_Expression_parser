#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "pstrings.h"


unsigned int str_len(char string_arg[]){
/* Find and return the length of string_arg. 

   A string ends when Nul is encountered, i.e.
   all strings are Nul-terminated. If a char array
   is not Nul terminated, then it's not a string,
   and the length can't be determined correctly.
*/
    unsigned int ind = 0, count = 0;

    while(string_arg[ind] != '\0'){
        ind++;
        count++;
    }
    return count;
};


void str_rev(char string_to_reverse[], char string_reversed[]){
/* Reverse string_to_reverse, writing the result to the string_reversed char array.*/

    int orig_length = str_len(string_to_reverse);     //  get the length of 'orig'; this doesn't count the terminating NULL character
    string_reversed[orig_length] = '\0';  // all strings need to be NULL-terminated

    unsigned int ind = 0;
    for(unsigned i = 0, k = orig_length-1; i < orig_length; i++, k--){
        string_reversed[k] = string_to_reverse[i];
    }
}



void str_rev_ip(char string_arg[]){
    /* Reverse the string argument (an array of chars) IN PLACE */
    
    unsigned int str_length = str_len(string_arg);  
    unsigned int largest_index = str_length-1;  // the length is -1 since counting starts at 0. 
    char temp; 

    if (str_length % 2 == 0){
        // if the length is even
        // printf("length is even, %i\n", str_length)
        for (unsigned int start_index=0, end_index=largest_index; start_index<end_index; start_index++, end_index--){
            // stop the iteration as soon as start_index < end_index becomes start_index > end_index
            // which will happen when they both move on from the two median elements
            temp = string_arg[start_index];
            string_arg[start_index] = string_arg[end_index];
            string_arg[end_index] = temp;
        }
    }
    else if (str_length % 2 == 1){
        // if the length is odd
        // printf("length is odd: %i\n", str_length);
        for (unsigned int start_index=0, end_index=largest_index; start_index != end_index; start_index++, end_index--){
            // stop the iteration when start_index and end_index become equal - which will
            // happen when both indexes reach the median element 
            temp=string_arg[start_index];
            string_arg[start_index] = string_arg[end_index];
            string_arg[end_index] = temp;
        }
    }
}



int32_t str_to_int(char string_arg[]){
    /* Convert string_arg to an integer, and return that.

       string_arg must not contain non-numeric characters 
       or be NULL, and it must be NUL-terminated.
    */
    int32_t res = 0;    // store the final result
    uint8_t digit = 0;  // store each digit as it's computed
    uint8_t sign = 0;   // if 1, add negative sign, if 0, keep positive
    uint8_t i = 0;  // index;
    
    if (string_arg[0] == '-'){
        sign = 1;   // it's a negative number
        i = 1;  // start computing after the sign
    }

    // do this until a NUL is found, which terminates the string
    for (; string_arg[i] != '\0'; i++){

    // 7 = the offset from '0' to '7' (i.e. = '7' - '0'),
    // since '7' is equal to '0' + 7
        digit = string_arg[i] - '0';    
        // each digit to the right is an order of magnitude lower than the digit to its
        // left
        res = res * 10 + digit;     // bump res up one order of magnitude, and add to it the next value to its right
    }
   
    // negative : make the number negative
    if (sign){
        res = 0 - res;  // e.g. -73 is 73 below 0
    }
    return res;
}



char *str_tokenize(char string_arg[], char delimiter){
    /* Return the next token in string_arg when called.

       A token is a substring delimitated by delimiter.
       e.g. 'my_name_is_x' has 4 tokens and 3 delimiters.

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
        token = string_arg;
        input_string = string_arg;
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




uint8_t str_count_digits(int32_t num){
    /* Count digits in num */
    uint8_t digits = 0;

    while(num){
        digits++;
        num /= 10;  // dividing by 10 will get rid of the rightmost digit
    }
    
    return digits;
}


char *str_from_int(int32_t num){
    /*Conver num to "num" */
   
    // used to find out the size that the array should be, and also used to index the
    // array afterward, down below in the while loop
    uint8_t array_size = str_count_digits(num);
    uint8_t iterations = array_size;    // the number of times the while at the bottom needs to execute
    uint8_t digit = 0;

    // if num is 0
    if (!array_size){
        return NULL;
    }

    // the array that will hold the ASCII representation of the digits in num
    char *res = malloc(sizeof(char) * (array_size+1));
    if (!res){
        return NULL;
    }

    res[array_size] = '\0';     // NUL-terminate the string
    array_size--;

    digit = num % 10;  // 17 % 10 is 7, and so is  % 10    
    num /= 10; // drop the rightmost digit
     // for (uint8_t i = 0, end = array_size; i < end; i++){
    while(iterations){
       res[array_size] = '0' + digit;    // convert to ASCII and write to array
       array_size--;

       digit = num % 10;
       num /= 10;
       iterations--;
    }
    return res;
}


