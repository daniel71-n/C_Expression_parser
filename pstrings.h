#include <stdint.h>

void str_rev_ip(char string_arg[]);   // reverse string_arg in place
void str_rev(char string_to_reverse[], char string_reversed[]);  // reverse string_arg and store the result in string_reversed

unsigned int str_len(char string_arg[]);  // return the length of string_arg, not counting the terminating Nul character

// convert a string to an int, provided the string doesn't contain prohibited (non-numeric) characters
int32_t str_to_int(char string_arg[]);  

// get a char array offering the ASCII representation of the digits in num
char *str_from_int(int32_t num);

// count digits in num
uint8_t str_count_digits(int32_t num);

// split string_arg into tokens and return a pointer to the next token on each call
char *str_tokenize(char string_arg[], char delimiter);
