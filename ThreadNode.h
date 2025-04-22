#ifndef THREAD_NODE_H
#define THREAD_NODE_H

#include <pthread.h>
#include <cmath>

int get_level(int index);
int get_position(int index);

// Binary tree node structure for threaded sum
struct ThreadNode {
    int index;           // index in tree array (0 to totalThreads-1)
    int level;           // level in the tree (root is level 1)
    int position;        // position within the level
    bool is_leaf = false;

    int start;           // start index in input array (only for leaf)
    int end;             // end index in input array (only for leaf)

    int* input_array;    // shared input array
    int* result_array;   // shared result array

    pthread_t thread;

    // Computation phase
    pthread_mutex_t compute_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t compute_cond = PTHREAD_COND_INITIALIZER;
    bool compute_ready = false;

    // Termination phase
    pthread_mutex_t terminate_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t terminate_cond = PTHREAD_COND_INITIALIZER;
    bool terminate_ready = false;
};

// bad practice here but I am lazy and don't want to have another header file 
struct ThreadContext {
    ThreadNode* all_nodes;
    int index;
};

// Tree helper functions
 int get_level(int index) {
    return std::floor(std::log2(index + 1)) + 1;
}

 int get_position(int index) {
    int level = get_level(index);
    return index - ((1 << (level - 1)) - 1);
}

#endif // THREAD_NODE_H
