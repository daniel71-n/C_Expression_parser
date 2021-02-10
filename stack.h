

/* ************************************************************* */
/* ------------------------ OVERVIEW --------------------------- */

/* Implementation of a stack ADT (Abstract Data Type). 
 * A stack offers an interface for manipulating data in
 * a LIFO - last-in-first-out - manner, always removing 
 * ('popping', in stack terminology) the most recently added
 * item -- the item that was last 'pushed' onto the stack. 

 * ************************************************************* */





/* *********************************************************** */
/* --------------- Typedefs and struct definitions ---------- */

/* Stack and StackItem are typedefs for a POINTER
 * to a struct stack and a POINTER to an item struct,
 * respectively. 
 * The reason they typedef pointers to the structs rather than 
 * the structs themselves is for ease of use. Abstracting away 
 * the fact that they're pointers makes the interface more
 * straightforward to use and reason about.
 */
typedef struct stack* Stack;   
typedef struct stackitem* StackItem;

struct stack{
    unsigned int count;     // number of items in the queue
    StackItem top;
};
    
struct stackitem{
    void *contents;     // a void pointer is used so that any type can be pointed to and thus enqueued;
    StackItem previous;
};






/* *************** FUNCTION PROTOTYPES ***************** */
/* ----------------------------------------------------- */

/* 'object' is used very loosely below, of course, as C 
 * doesn't have 'objects' (it's not an OOP language) 
 */


/* Initialize a Stack object.
 * This initializes the inner members of the Stack to the 
 * correct values and allocates Heap memory.
 *
 * Example
     * Stack mystack;
     * Stack_init(&mystack);
*/
void Stack_init(Stack *stack_ptr);
   



/* Takes a pointer to a type (struct, char, int, etc),
 * and encapsulates it inside a StackItem to be passed 
 * to Stack_push(), which it then returns.
 *
 * Specifically, StackItem has an inner void pointer member,
 * which will be set to point to the parameter provided. 
 *
 * This is how Stack_push() should be called: always 
 * call Stack_make_item() first with a pointer to the 
 * type you want to put on the stack, then use its return
 * value to pass to Stack_push().
 *
 * Example
 *  char mychar = 'c';
 *  StackItem new = Stack_make_item(&mychar);
 *  Stack_psuh(somestack, new);
 */ 
StackItem Stack_make_item(void *data);  




/* Push the new_item onto the_stack. 
 *
 * The new_item must be created via the Stack_make_item()
 * function. 
 *
 * Example
 *      int myint = 120;
 *      StackItem newitem= Stack_make_item(&myint); // make myint into an item
 *      Stack_push(somestack, newitem);
 */
void Stack_push(Stack the_stack, StackItem new_item);



/* Returns a void pointer that points to the type that
 * the caller passed in to Stack_make_item(), and then
 * pushed onto the stack by calling Stack_push() with the
 * value returned by Stack_make_item().
 *
 * The caller needs to know how to deal with/interpret
 * this void pointer being returned (i.e. they need to
 * know what it's supposed to point to, so that they 
 * cast it correctly).
 *
 * Example
 *      char mychar = 'a';
 *      // make mychar into a StackItem
 *      StackItem newitem = Stack_make_item(&mychar);
 *      // push newitem onto the stack
 *      Stack_push(somestack, newitem);
 *      
 *      char someotherchar;
 *      // Stack_pop() RETURNS A VOID POINTER, NOT A StackItem!
 *      someotherchar = (*char)Stack_pop(somestack);
 *      // the return value needs to be cast to the correct pointer
 * -----------------------------------------------
 *  The value popped off the stack is the current top 
 *  of the stack -- i.e. the most-recently added item,
 *  in accordance to LIFO ordering.
 */
void *Stack_pop(Stack the_stack);  




/* The same as Stack_pop() (see above), with the difference
 * that nothing gets removed from the stack, such that the
 * next call to Stack_pop() will return the same value
 * returned by a previous call to Stack_peek()
 */
void *Stack_peek(Stack the_stack);  



/* Return the number of items currently on the stack */
unsigned int Stack_count(Stack the_stack); 



/* Upend a stack - turn it upside down, so that the items are in 
 * reverse order, with the former top now being at the bottom, 
 * and the former bottom being at the top 
 */
Stack Stack_upend(Stack the_stack);      




/* Tear down the stack by freeing all the malloc-ated memory
 * and then setting Stack to NULL. 
 *  
 * This has to be called when the stack is no longer needed,
 * to avoid memory leakage.
 */
void Stack_destroy(Stack *stack_ptr);  








