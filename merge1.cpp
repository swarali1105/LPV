#include <iostream>
#include <cstdlib>
#include <omp.h>
 
using namespace std;
 
void merge(int arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
 
    int* L = new int[n1];
    int* R = new int[n2];
 
    for (int i = 0; i < n1; ++i)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; ++j)
        R[j] = arr[m + 1 + j];
 
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1)
        arr[k++] = L[i++];
    while (j < n2)
        arr[k++] = R[j++];
 
    delete[] L;
    delete[] R;
}
 
void sequentialMergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        sequentialMergeSort(arr, l, m);
        sequentialMergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}
 
void parallelMergeSort(int arr[], int l, int r, int depth = 0) {
    if (l < r) {
        int m = l + (r - l) / 2;
        if (depth <= 3) {
            #pragma omp parallel sections
            {
                #pragma omp section
                parallelMergeSort(arr, l, m, depth + 1);
                #pragma omp section
                parallelMergeSort(arr, m + 1, r, depth + 1);
            }
        } else {
            sequentialMergeSort(arr, l, m);
            sequentialMergeSort(arr, m + 1, r);
        }
        merge(arr, l, m, r);
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
    double time_seq_merge, time_par_merge;
 
    // Sequential Merge Sort
    start = omp_get_wtime();
    sequentialMergeSort(arrSeq, 0, n - 1);
    end = omp_get_wtime();
    time_seq_merge = end - start;
    printArray(arrSeq, n, "Sorted (Sequential Merge)");
 
    // Parallel Merge Sort
    start = omp_get_wtime();
    parallelMergeSort(arrPar, 0, n - 1);
    end = omp_get_wtime();
    time_par_merge = end - start;
    printArray(arrPar, n, "Sorted (Parallel Merge)");
 
    // Final timing results
    cout << "=== Timing Results ===\n";
    cout << "Sequential Merge Sort: " << time_seq_merge << " seconds\n";
    cout << "Parallel Merge Sort  : " << time_par_merge << " seconds\n";
 
    delete[] original;
    delete[] arrSeq;
    delete[] arrPar;
 
    return 0;
}
