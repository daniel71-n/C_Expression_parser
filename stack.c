#include "stack.h"
#include <stdlib.h>

/* ********************************************************** */
/*                  Implementation Notes

    Stacks and queues are essentially the inverse of one another 
    in that the former always removes the oldest item  added, 
    while the latter removes the latest item. Both offer linear
    (sequential) access to the data, just like a linked list.

    It's worth noting that stacks and queues are not 
    data structures, but abstract data types. Conceptually, 
    stacks and queues are to be thought of as an interface rather 
    than in implementation terms. That is to say, both of them can 
    easily be implemented using either arrays or linked lists 
    at the implementation level. 
    Arrays arguably lend themselves better to stacks and queues that 
    hold a fixed number of items (since arrays are fixed in size, 
    unless they're dynamic arrays) whereas linked lists are 
    best used when the number of items is indefinite. 

    Whatever the underlying data structure, stacks and queues are 
    defined by the interface they provide, which is in terms of 
    the data flow: first-in-first-first-out for queues and 
    last-in-first-out for stacks.

    --------------Overflow and underflow--------------------
    This implementation of a stack doesn't have a fixed size, i.e. 
    there's no stack overflow scenario that's accounted for.
    This could easily be incorporated, however. For example, 
    by having the Stack_push() function check the count member 
    and stop pushing new items when its value has reached a defined limit.

    Stack 'underflow', is, on the other hand, accounted for, as Stack_pop() 
    will return a NULL pointer when called on an empty queue.

*/








/*  *********************** PRIVATE FUNCTIONS ********************* */
/*  --------------------------------------------------------------  */ 

static void Stack_init_local_P(Stack the_stack){
    /* Initialize stack_ptr by initializing its values. 

       Malloc is not called - heap memory is not allocated. 
       This means the stack has local scope (defined in a function)
       and thus automatic duration: i.e. it will be automatically
       deallocated when it goes out of scope. 
       Therefore you don't need (you musn't) call Stack_destroy
       on a stack initialized with Stack_init_local().

       Calling free() (which Stack_destroy() does internally)
       on memory that wasn't allocated dynamically is UNDEFINED
       BEHAVIOR.

       If the stack being initialized is instead supposed to
       be heap-allocated, use Stack_init() instead.
    */
    the_stack->count = 0;
    the_stack->top = NULL;
};



static StackItem Stack_pop_whole_P(Stack the_stack){
    /* Meant to be called from Stack_upend(). 

       Stack_pop() returns Stack->top->contents,
       which is a void pointer contained inside StackItem.

       Stack_pop_whole_P() returns Stack->top, on the other hand,
       which is a StackItem. 
       This is used for moving a StackItem from one stack onto
       another - as used by the Stack_upend() function. 
       Because of that, the StackItem is NOT FREED by Stack_pop_whole_P()-- 
       whereas Stack_pop() also calls free() on Stack->top before 
       returning its 'contents' value.
    */
    if (!the_stack->count){
        return the_stack->top;
    };
   
    the_stack->count--;
    StackItem item = the_stack->top;
    the_stack->top = the_stack->top->previous;
    
    return item;
};




/*  ************************* END PRIVATE ************************* */
/*  --------------------------------------------------------------  */ 





void Stack_init(Stack *stack_ptr){
    /* Initialize a Stack object. 

       Takes a Stack reference pointer and calls malloc to allocate
       heap space for a Stack struct. If successful, 
       the inner members are initialized to zero
       or NULL, as appropriate.

       This function is called when the Stack is meant to be
       dynamic - that is to say, heap memory is allocated to 
       it.

       Example
       Stack *somestack;
       Stack_init(&somestack);  // allocates heap memory to *somestack

       Since dynamic memory is allocated, it's imperative that
       Stack_destroy() be called to tear down the stack and
       free all the memory when the stack is no longer used.


       The Stack_init_local() function (below) should be called instead when 
       the stack is meant to have local scope and thus automatic duration. 
        
    */
    Stack temp = malloc(sizeof(struct stack));

    if (!temp){
        *stack_ptr = NULL;
        return;
    };

    *stack_ptr = temp; 
    (*stack_ptr)->count = 0;
    (*stack_ptr)->top= NULL;
};




unsigned int Stack_count(Stack the_stack){
    /* Return the number of items on the stack*/
    return the_stack->count;
};



void Stack_push(Stack the_stack, StackItem the_item){
    /* Add an item to the stack.
    
    Note that the second argument is a StackItem pointer. 
    The StackItem struct has a 'contents' field which is 
    a void pointer. This can be used to point to any data type,
    be it char, int, whatever. 
    This means that if you want to push X onto the stack, you will
    have to declare a StackItem and set it to point to X.

    For example: 

        int someint = 6;
        StackItem item1; 
        item1.contents = &someint; 
        Stack_push(&item1);


    Note that since items1.contents is a void pointer, 
    the consumer will have to know how to handle this 
    when returned by the Stack_pop() subroutine.
    */
    the_item->previous = the_stack->top;
    the_stack->top= the_item;
    the_stack->count++;
};



void *Stack_pop(Stack the_stack){
    /* Remove and return an item from the stack.
       This item is always the most recently added item, 
       according to the FIFO principle.

       If called on an empty stack, NULL is returned.


       **** NOTE ****
       By 'item', what's meant is not actually a StackItem
       but the practical value that a StackItem was created
       and pushed onto the stack for.

       That is to say, Stack->top actually gets popped off
       the stack, but what's returned is not Stack->top,
       but Stack->top->contents, which is a void pointer
       pointing to whatever type was fed to Stack_make_item()
       that was then pushed onto the stack with Stack_push().

       This means that the caller should know how to deal with
       or interpret the returned value (what to cast this void
       pointer to). 

       Stack->top gets deallocated by calling free() on it.


       **********************
       For a function that returns Stack->top instead of 
       Stack->top->contents, see Stack_upend() and
       Stack_pop_whole_p().
    */
    if (!the_stack->count){
        return the_stack->top;
    };

    the_stack->count--;
    void *return_val = the_stack->top->contents;
    StackItem temp = the_stack->top->previous;
    free(the_stack->top);  
    the_stack->top = temp;

    return return_val;
};




void *Stack_peek(Stack the_stack){
    /* Return the top of the stack (most recently
       added item) but without removing it from 
       the stack, such that this is the item that will 
       be returned by Stack_pop(), when called.

       Returns NULL if called on an empty queue.

       **********
       NOTE
       The value returned isn't actually Stack->top,
       but Stack->top->contents (read the notes for 
       Stack_pop()  --> unlike Stack_pop(), though,
       Stack_peek() does NOT deallocate Stack->top.
    */
    return the_stack->top->contents; 
};




Stack Stack_upend(Stack the_stack){
    /* Upend a stack and return the new stack. 

       This is done by initializing a second stack first,
       with Stack_init_local().
       The items are popped off the first stack one by one
       and pushed onto the second one as this happens.

       The first stack is then pointed to the top of this second 
       stack. 
       The second stack has automatic duration- so it will go
       out of scope when the function exits. 
    */
    struct stack newstack;     // local-scope, automatic-duration stack
    // this is not a pointer to a stack struct (like the Stack)
    Stack_init_local_P(&newstack);
    
    StackItem current = Stack_pop_whole_P(the_stack);

    while (current){
        Stack_push(&newstack, current);
        current = Stack_pop_whole_P(the_stack);
    };

    the_stack->top = newstack.top;
    the_stack->count = newstack.count;

    return the_stack;
};



StackItem Stack_make_item(void *data){
    /* Dynamically allocate memory for a single node
       and return a pointer to it.

       The 'contents' member of the node, which is
       a void pointer, is pointer to the function 
       parameter, whatever it is. 

       The caller should then manually call Stack_push()
       with the value returned by this function to actually
       push the node onto the stack.

       --------------------------------------
       Example

       int myint = 5;
       StackItem item = Stack_make_item(&myint); 
       Stack_push(somestack, item);
       --------------------------------------

       The purpose of this function is therefore
       to essentially encapsulate any type of pointer***
       into a StackItem, which is then returned to the
       caller to be fed to Stack_push(). 

       This is the only way to create a StackItem
       to pass to Stack_push(), i.e. the interface
       the caller is supposed to use.


       ***Note that the parameter has to be a POINTER
       TO something, rather than SOMETHING.
       I.e.
       These won't work:
            StackItem newitem = Stack_make_item('5');
            Stack_make_item(struct foo);
        

        The way this stack implementation is meant to be 
        used is to push and pop pointer to types, rather
        than the types themselves.

    */
    struct stackitem *new_item = malloc(sizeof(struct stackitem));
    if (new_item){
        new_item->contents = data;
    };
    return new_item;
};






void Stack_destroy(Stack *stack_ptr){
    /* Call free on *stack_ptr, and set it to NULL. 
    */
    if (! (*stack_ptr)){    // Stack_ptr is already NULL, nothing to free
        return;
    }
    if ((*stack_ptr)->top){     // if the Stack isn't NULL, and Stack->top isn't NULL either, it means there are still items to deallocate
        StackItem current = (*stack_ptr)->top;
        StackItem previous = current->previous;
        
        while(current){
           free(current); 
           current = previous;
           previous = (previous) ? previous->previous : NULL ;  // if previous is NULL, trying to access its 'previous' member will end in a crash 
        };

    }   
    // no items to dallocate, but Stack is still not NULL: deallocate that
    free(*stack_ptr);
    *stack_ptr = NULL;

};


