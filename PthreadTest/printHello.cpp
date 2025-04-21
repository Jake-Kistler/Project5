#include<iostream>
#include<pthread.h>

// this is a basic function for thread creation
void *printHello(void *arg){
    int id = *reinterpret_cast<int*>(arg);
    std::cout << "Hello from thread " << id << std::endl;
    return nullptr;
}

void sample_thread_creation(){
    pthread_t threads [3]; // we have an array of 3 threads
    int ids[3] = {1,2,3}; // the ids for the threads

    for(int i = 0; i < 3; i++){
        // create the threads
        pthread_create(&threads[i], nullptr, printHello, &ids[i]);
    }

    for(int i = 0; i < 3; i++){
        pthread_join(threads[i],nullptr);
    }
}

int main(){
    sample_thread_creation();
    return 0;
}