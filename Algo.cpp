#pragma once

inline void merge(int items[], int keys[], int left, int mid, int right, int& comparisons, int& moves) {
    int n1 = mid - left + 1, n2 = right - mid;
    int Lk[100], Rk[100], Li[100], Ri[100];
    for (int i = 0; i < n1; i++) { Lk[i] = keys[left + i]; Li[i] = items[left + i]; }
    for (int j = 0; j < n2; j++) { Rk[j] = keys[mid + 1 + j]; Ri[j] = items[mid + 1 + j]; }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        comparisons++;
        if (Lk[i] <= Rk[j]) { keys[k] = Lk[i]; items[k] = Li[i++]; }
        else { keys[k] = Rk[j]; items[k] = Ri[j++]; }
        moves++;
        k++;
    }
    while (i < n1) { keys[k] = Lk[i]; items[k] = Li[i++]; moves++; k++; }
    while (j < n2) { keys[k] = Rk[j]; items[k] = Ri[j++]; moves++; k++; }
}

inline void mergeSort(int items[], int keys[], int left, int right, int& comparisons, int& moves) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(items, keys, left, mid, comparisons, moves);
        mergeSort(items, keys, mid + 1, right, comparisons, moves);
        merge(items, keys, left, mid, right, comparisons, moves);
    }
}

inline void runMergeSort(int items[], int keys[], int n, int& comparisons, int& moves) {
    comparisons = moves = 0;
    if (n > 1) mergeSort(items, keys, 0, n - 1, comparisons, moves);
}
