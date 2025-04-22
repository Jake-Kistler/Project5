// This project acts us to calculate the sum of an INT array using threads that re in a binary tree 
// For input we get the height of the tree
// The size of the array (numbers of input elements)
// The ints themselve
// Need to calculate the number of leaf nodes 
// detemine if we can divide by the tree height
// IF NOT: Pad 0's to make it so we can do this division
// with this value we can pass off N sized sub arrays to the threads

#include <iostream>
#include <pthread.h>
#include "ThreadNode.h"

void *compute_terminate(void *arg);

int main(int argv, char *argv []){
    // step 1: read input from the file
    int tree_height, array_size;
    std::cin >> tree_height >> array_size;

    // step 2: calculate things about the tree
    array_size = pow(2, tree_height) - 1; // this is the number of leaf nodes

    int leaf_nodes = 1 << (tree_height - 1); // this is the number of leaf nodes
    int total_threads = (1 << tree_height) -1; // total threads in the tree

    // check for 0 padding for the array
    int padding = array_size;
    if(array_size % tree_height != 0){
        padding = ((array_size / tree_height) + 1) * leaf_nodes;
    }

    int *input_array = new int[padding]; // the input array

    // populate the input array (WE NEED THIS FILE BUT FOR NOW I WILL USE 10 user numbers)
    int value_to_read = 0;
    while(value_to_read < 10){
        std::cin >> input_array[value_to_read];
        value_to_read++;
    }

    for(int i = array_size; i < padding; i++){
        input_array[i] = 0; // pad the array with 0's
    }

    int chunk_size = padding / leaf_nodes;
    int *result_array = new int[total_threads];

    for(int i = 0; i < total_threads; i++){
        result_array[i] = 0; // initialize the result array
    }

    ThreadNode *thread_args = new ThreadNode[total_threads]; // the thread arguments
    
    // loop to build the thread arg 
    for(int i = 0; i < total_threads; i++){
        thread_args[i].index = i;
        thread_args[i].level = get_level(i);
        thread_args[i].position = get_position(i);
        thread_args[i].input_array = input_array;
        thread_args[i].result_array = result_array;
    }

}

void *compute_terminate(void *arg){
    ThreadNode *node = static_cast<ThreadNode *>(arg);

    // phase 1: wait for the signal
    pthread_mutex_lock(&node->compuation_mutex);
    while(!node->compute_ready){
        pthread_cond_wait(&node->compuation_condition, &node->compuation_mutex);
    }

    pthread_mutex_unlock(&node->compuation_mutex);

    pthread_t tread_id = pthread_self();
    int index = node->index;

    // computation phase
    if(node->is_leaf){
        int sum = 0;
        for(int i = node->start_index; i <= node->end_index; i++){
            sum += node->input_array[i];
        }

        node->result_array[index] = sum;

        
        std::cout << "[Thread Index " << idx << "] [Level " << node->level
        << ", Position " << node->position << "] [TID " << tid
        << "] computed leaf sum: " << sum << std::endl;
    } // END IF
    else{
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        // wait on the children
        while(node->result_array[left] == -1 || node->result_array[right] == -1){
            usleep(1000);
        }

        int left_sum = node->result_array[left];
        int right_sum = node->result_array[right];
        node->result_array[index] = left_sum + right_sum;

        std::cout << "[Thread Index " << idx << "] [Level " << node->level
        << ", Position " << node->position << "] [TID " << tid << "] received:\n"
        << "  Left child [Index " << left << "]: " << leftVal << "\n"
        << "  Right child [Index " << right << "]: " << rightVal << "\n"
        << "[Thread Index " << idx << "] computed sum: " << node->results[idx] << std::endl;
    } // end else

    // phase 2: wait for the termination
    pthread_mutex_lock(&node->termination_mutex);

    while(!node->terminate_ready){
        pthread_cond_wait(&node->termination_condition, &node->termination_mutex);
    }

    pthread_mutex_unlock(&node->termination_mutex);

    std::cout << "[Thread Index " << idx << "] [Level " << node->level
    << ", Position " << node->position << "] terminated." << std::endl;

    return nullptr;
}
