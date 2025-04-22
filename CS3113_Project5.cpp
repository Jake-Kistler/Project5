// Jake Austin Kistler
// University of Oklahoma
// Intro to Operating systems
// project 5, working with threads, my bread and butter in personal projects although they are in Rust
// Feature me, ! Remember the extra compiler 


#include <iostream>
#include <unistd.h>         // for usleep (The pole function)
#include <vector>
#include "ThreadNode.h"

// coding style has changed to what I have been forced to adopt both in professional work and in my software engineering class :(

// As a note, pthread accepts void pointer args that have to be explicty (meaning static casted) into other types
// the same goes for the retun types of functions that use pthreads 


// Each function will run this function
void* compute_terminate(void* arg) {
    ThreadContext* context = static_cast<ThreadContext*>(arg);
    ThreadNode* node = &context->all_nodes[context->index];

    ThreadNode* all_nodes = context->all_nodes;

    int idx = node->index;
    pthread_t tid = pthread_self();

    // Phase 1: Wait to be signaled to compute
    pthread_mutex_lock(&node->compute_mutex);
    while (!node->compute_ready)
        pthread_cond_wait(&node->compute_cond, &node->compute_mutex);

    pthread_mutex_unlock(&node->compute_mutex);

    // compute the sum here
    if (node->is_leaf) {
        int sum = 0;
        for (int i = node->start; i <= node->end; ++i)
            sum += node->input_array[i];

        node->result_array[idx] = sum;
        if (idx == 0) {
            std::cout << "[Thread Index " << idx << "] computed final sum: " << node->result_array[idx] << std::endl;
            std::cout << "\n[Thread Index " << idx << "] now initiates tree cleanup.\n" << std::endl;
        } 
        else {
            std::cout << "[Thread Index " << idx << "] [Level " << node->level
            << ", Position " << node->position << "] [TID " << tid
            << "] computed leaf sum: " << sum << std::endl;
  
        }
        
    } 
    else {
        // we've hit the first node and now we have to wait for the children to finish 
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;

        while (node->result_array[left] == -1 || node->result_array[right] == -1)
            usleep(1000); // polling here

        int leftVal = node->result_array[left];
        int rightVal = node->result_array[right];
        node->result_array[idx] = leftVal + rightVal;

        std::cout << "[Thread Index " << idx << "] [Level " << node->level
        << ", Position " << node->position << "] [TID " << tid << "] received:\n";

        std::cout << "Left Child [Index " << left
        << ", Level " << all_nodes[left].level
        << ", Position " << all_nodes[left].position
        << ", TID " << all_nodes[left].thread
        << "]: " << leftVal << std::endl;

        std::cout << "Right Child [Index " << right
        << ", Level " << all_nodes[right].level
        << ", Position " << all_nodes[right].position
        << ", TID " << all_nodes[right].thread
        << "]: " << rightVal << std::endl;

        if (idx == 0) {
            std::cout << "[Thread Index " << idx << "] computed final sum: " << node->result_array[idx] << std::endl;
            std::cout << "\n[Thread Index " << idx << "] now initiates tree cleanup.\n" << std::endl;
        } else {
            std::cout << "[Thread Index " << idx << "] computed sum: " << node->result_array[idx] << std::endl;
        }

    }

    // Phase 2: Wait for termination signal
    pthread_mutex_lock(&node->terminate_mutex);
    while (!node->terminate_ready)
        pthread_cond_wait(&node->terminate_cond, &node->terminate_mutex);

    pthread_mutex_unlock(&node->terminate_mutex);

    // Signal children and wait for them if internal node
    if (!node->is_leaf) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;

        // Signal left child
        pthread_mutex_lock(&all_nodes[left].terminate_mutex);
        all_nodes[left].terminate_ready = true;

        pthread_cond_signal(&all_nodes[left].terminate_cond);
        pthread_mutex_unlock(&all_nodes[left].terminate_mutex);

        // Signal right child
        pthread_mutex_lock(&all_nodes[right].terminate_mutex);
        all_nodes[right].terminate_ready = true;

        pthread_cond_signal(&all_nodes[right].terminate_cond);
        pthread_mutex_unlock(&all_nodes[right].terminate_mutex);

        // Join children
        pthread_join(all_nodes[left].thread, nullptr);
        pthread_join(all_nodes[right].thread, nullptr);
    }

    std::cout << "[Thread Index " << idx << "] [Level " << node->level
              << ", Position " << node->position << "] terminated." << std::endl;

    delete context;
    return nullptr;
}


int main() {
    int tree_height, array_size;
    std::cin >> tree_height >> array_size;

    int leaf_nodes = 1 << (tree_height - 1); // 2^tree height -1 
    int total_threads = (1 << tree_height) - 1; // also the total number of nodes in the tree

    // Pad array so it's divisible by leaf count
    int padded_size = array_size;

    if (array_size % leaf_nodes != 0) {
        padded_size = ((array_size / leaf_nodes) + 1) * leaf_nodes;
    }

    // create the array
    int* input_array = new int[padded_size];

    // read in input from the file
    for (int i = 0; i < array_size; ++i) {
        std::cin >> input_array[i];
    }


    for (int i = array_size; i < padded_size; ++i) {
        input_array[i] = 0;
    }

    int chunk_size = padded_size / leaf_nodes;
    int* result_array = new int[total_threads];

    for (int i = 0; i < total_threads; ++i)
        result_array[i] = -1;

    // create thread nodes and set the fields
    ThreadNode* thread_args = new ThreadNode[total_threads];

    for (int i = 0; i < total_threads; ++i) {
        thread_args[i].index = i;
        thread_args[i].level = get_level(i);
        thread_args[i].position = get_position(i);
        thread_args[i].input_array = input_array;
        thread_args[i].result_array = result_array;

        // building the leafs
        if (i >= leaf_nodes - 1) {
            int leaf_index = i - (leaf_nodes - 1);
            thread_args[i].is_leaf = true;

            thread_args[i].start = leaf_index * chunk_size;
            thread_args[i].end = (leaf_index + 1) * chunk_size - 1;
        }

        // Wrap args in a context object so thread can access all_nodes
        ThreadContext* context = new ThreadContext{thread_args, i};
        pthread_create(&thread_args[i].thread, nullptr, compute_terminate, context);
    }

    // Phase 1: signal leaf threads
    for (int i = leaf_nodes - 1; i < total_threads; ++i) {
        pthread_mutex_lock(&thread_args[i].compute_mutex);
        thread_args[i].compute_ready = true;

        pthread_cond_signal(&thread_args[i].compute_cond);
        pthread_mutex_unlock(&thread_args[i].compute_mutex);
    }

    // Then signal internal threads
    for (int i = leaf_nodes - 2; i >= 0; --i) {
        pthread_mutex_lock(&thread_args[i].compute_mutex);
        thread_args[i].compute_ready = true;

        pthread_cond_signal(&thread_args[i].compute_cond);
        pthread_mutex_unlock(&thread_args[i].compute_mutex);
    }

    // Wait for root result
    while (result_array[0] == -1)
        usleep(1000); // again polling

    std::cout << "\n[Root] Final sum: " << result_array[0] << std::endl;

    // Phase 2: signal root for termination (the rest is handled recursively)
    pthread_mutex_lock(&thread_args[0].terminate_mutex);
    thread_args[0].terminate_ready = true;

    pthread_cond_signal(&thread_args[0].terminate_cond);
    pthread_mutex_unlock(&thread_args[0].terminate_mutex);

    // wait for the entire tree to finish
    pthread_join(thread_args[0].thread, nullptr);

    // like a good c programmer clean up and reclaim all the memory we used
    delete[] input_array;
    delete[] result_array;
    delete[] thread_args;

    return 0;
}

