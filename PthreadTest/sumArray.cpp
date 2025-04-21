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

    int middle = size / 2; // the middle index

    sumArgs args1 = {arr, 0, middle - 1}; // the first half
    sumArgs args2 = {arr, middle, size - 1}; // the second half
    pthread_t thread1, thread2; // the threads

    void *result1, *result2; // the results of the threads
    pthread_create(&thread1, nullptr, sumArray, &args1); // create the first thread
    pthread_create(&thread2, nullptr, sumArray, &args2); // create the second thread


    pthread_join(thread1, &result1);
    pthread_join(thread2, &result2);

    // collect and add results
    int sum1 = *static_cast<int*>(result1); // cast the result to the right type
    int sum2 = *static_cast<int*>(result2); 
    int total_sum = sum1 + sum2; // the total sum

    std::cout << "Sum of the first half is: " << sum1 << std::endl;
    std::cout << "Sum of the second half is: " << sum2 << std::endl; 
    std::cout << "Total sum is: " << total_sum << std::endl;

    delete static_cast<int*>(result1); // free the memory
    delete static_cast<int*>(result2); // free the memory

    return 0;
}