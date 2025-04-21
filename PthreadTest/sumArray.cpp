#include<iostream>
#include<pthread.h>

struct sumArgs{
    int *arr; // the array to sum
    int start; // the start index
    int end; // the end index
};

void *sumArray(void *arg){
    sumArgs* args = static_cast<sumArgs*>(arg); // cast the argument to the right type

    int *array = args->arr;
    int start = args->start;
    int end = args->end;

    int *result = new int(0);
    for(int i = start; i <= end; i++){
        *result += array[i];
    }

    return result; // return the result
}

int main(){
    int arr[] = {1,2,3,4,5,6,7,8,9,10}; // the array to sum
    int size = sizeof(arr) / sizeof(arr[0]); // the size of the array

    pthread_t thread; // the thread
    sumArgs args = {arr, 0, size - 1};

    pthread_create(&thread, nullptr, sumArray, &args); // create the thread

    void *result; // the result of the thread
    pthread_join(thread, &result);

    int *sum = static_cast<int*>(result); // cast the result to the right type
    std::cout << "The sum is: " << *sum << std::endl; // print the result

    delete sum; // delete the result
    return 0;
}