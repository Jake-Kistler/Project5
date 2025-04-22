#include <iostream>
#include <pthread.h>
#include <cmath>
#include <unistd.h>

const int MAX_THREADS = 15;

struct ThreadNode {
    int index;
    int start;
    int end;

    int* arr;
    int* result;

    pthread_t thread;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

    bool ready = false;
};

void* compute_sum(void* arg) {
    ThreadNode* node = static_cast<ThreadNode*>(arg);

    pthread_mutex_lock(&node->mutex);
    while (!node->ready)
        pthread_cond_wait(&node->condition, &node->mutex);
    pthread_mutex_unlock(&node->mutex);

    int id = node->index;

    if (node->start != -1) {
        int sum = 0;
        for (int i = node->start; i <= node->end; i++) {
            sum += node->arr[i];
        }

        node->result[id] = sum;
        std::cout << "[Leaf " << id << "] Sum: " << sum << std::endl;

    } else {
        int left = 2 * id + 1;
        int right = 2 * id + 2;

        while (node->result[left] == -1 || node->result[right] == -1)
            usleep(100);

        node->result[id] = node->result[left] + node->result[right];

        std::cout << "[Internal " << id << "] Combined "
                  << node->result[left] << " + " << node->result[right]
                  << " = " << node->result[id] << std::endl;
    }

    return nullptr;
}

int main() {
    const int H = 3;
    const int N = 1 << (H - 1);
    const int TOTAL_THREADS = (1 << H) - 1;

    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int result[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++)
        result[i] = -1;

    ThreadNode nodes[MAX_THREADS];
    int chunk_size = 8 / N;

    for (int i = 0; i < TOTAL_THREADS; i++) {
        nodes[i].index = i;
        nodes[i].arr = arr;
        nodes[i].result = result;

        if (i >= (1 << (H - 1)) - 1) {
            int leaf_index = i - ((1 << (H - 1)) - 1);
            nodes[i].start = leaf_index * chunk_size;
            nodes[i].end = (leaf_index + 1) * chunk_size - 1;
        } 
        else {
            nodes[i].start = -1;
            nodes[i].end = -1;
        }

        pthread_create(&nodes[i].thread, nullptr, compute_sum, &nodes[i]);
    }

    // Signal leaf threads
    for (int i = (1 << (H - 1)) - 1; i < TOTAL_THREADS; i++) {
        pthread_mutex_lock(&nodes[i].mutex);
        nodes[i].ready = true;

        pthread_cond_signal(&nodes[i].condition);
        pthread_mutex_unlock(&nodes[i].mutex);
    }

    // Signal internal nodes in reverse order
    for (int i = (1 << (H - 1)) - 2; i >= 0; --i) {
        pthread_mutex_lock(&nodes[i].mutex);
        nodes[i].ready = true;
        
        pthread_cond_signal(&nodes[i].condition);
        pthread_mutex_unlock(&nodes[i].mutex);
    }

    for (int i = 0; i < TOTAL_THREADS; i++) {
        pthread_join(nodes[i].thread, nullptr);
    }

    std::cout << "\n[Root] Final sum: " << result[0] << std::endl;
    return 0;
}
