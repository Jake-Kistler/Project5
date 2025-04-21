#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <pthread.h>
#include <cmath>

// function prototypes:
int get_left_child(int index);
int get_right_child(int index);

// Just a note for myself:
// Should the node have it's own thread control or does this need to be handled by a global strcture?


// the binary tree node structure
struct ThreadNode{
    int index; // the postion in the array (o - 2N-2)
    int level; // level in the tree (by convertion root is level 1)
    int postion; // Position from left on it's level 
    bool is_Leaf; // Is this a leaf node or not

    int start_index; // start index for the input array passed to the threads
    int end_index; // end index for the input array passed to the threads

    int *input_array; // the input array passed to the threads
    int *result_array; // the result array passed to the threads

    pthread_t thread; // the thread for this node
}

// The methods that act on this structure
int get_left_child(int index)
{
    return 2 * i + 1;
}

int get_right_child(int index)
{
    return 2 * i + 2;
}

#endif // BINARY_TREE_H