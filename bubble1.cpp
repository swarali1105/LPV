#include <iostream>
#include <cstdlib>
#include <omp.h>
 
using namespace std;
 
// Sequential Bubble Sort
void sequentialBubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; ++i)
        for (int j = 0; j < n - i - 1; ++j)
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
}
 
// Parallel Bubble Sort (Odd-Even Transposition)
void parallelBubbleSort(int arr[], int n) {
    for (int i = 0; i < n; ++i) {
        int phase = i % 2;
        #pragma omp parallel for
        for (int j = phase; j < n - 1; j += 2) {
            if (arr[j] > arr[j + 1])
                swap(arr[j], arr[j + 1]);
        }
    }
}
 
void printArray(int arr[], int n, const string& label) {
    cout << label << ": ";
    for (int i = 0; i < n; ++i)
        cout << arr[i] << " ";
    cout << "\n\n";
}
 
int main() {
    int n, numThreads;
    cout << "Enter the size of the array: ";
    cin >> n;
 
    cout << "Enter number of threads to use: ";
    cin >> numThreads;
    omp_set_num_threads(numThreads);
 
    int* original = new int[n];
    for (int i = 0; i < n; ++i)
        original[i] = rand() % 1000;
 
    int* arrSeq = new int[n];
    int* arrPar = new int[n];
    for (int i = 0; i < n; ++i)
        arrSeq[i] = arrPar[i] = original[i];
 
    printArray(original, n, "\nOriginal Array");
 
    double start, end;
    double time_seq_bubble, time_par_bubble;
 
    // Sequential Bubble Sort
    start = omp_get_wtime();
    sequentialBubbleSort(arrSeq, n);
    end = omp_get_wtime();
    time_seq_bubble = end - start;
    printArray(arrSeq, n, "Sorted (Sequential Bubble)");
 
    // Parallel Bubble Sort
    start = omp_get_wtime();
    parallelBubbleSort(arrPar, n);
    end = omp_get_wtime();
    time_par_bubble = end - start;
    printArray(arrPar, n, "Sorted (Parallel Bubble)");
 
    // Final timing results
    cout << "=== Timing Results ===\n";
    cout << "Sequential Bubble Sort: " << time_seq_bubble << " seconds\n";
    cout << "Parallel Bubble Sort  : " << time_par_bubble << " seconds\n";
 
    delete[] original;
    delete[] arrSeq;
    delete[] arrPar;
 
    return 0;
    }
