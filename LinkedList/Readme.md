# Description to linked list

This is my implementation of simple circular doubly linked list based on kernel list. 
List node is embedded in data structure. It doesn't use memory allocation and can "contain" any structure type you want.

#### Available features:
* traverse: macro, given function, reverse, in bounds
* delete: list, node, several nodes
* insert: head, tail, after/before/N_nodes_away_from element
* swap nodes
* reverse
* count elements
* sort(quicksort): asc, desc

There are some points to review and some questionable solution. Things, important to me i mentioned in **Questions.txt** file.

To compile program run _make_ in terminal in directory with all files.\
To compile program with assert checking all operations run _make CFLAGS+=-DDEBUG_\
To run binary file run _./test_list_ in terminal.

The program is divided in 3 files:\
**list.h** - header file with definitions of functions, macro with comments provided (basically API).\
**list.c** - source file with implementations.\
**test_list.c** - source file, which is just demonstration of functionality.