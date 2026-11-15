#pragma once

inline void bubbleSort(int items[], int keys[], int n, int& comparisons, int& swaps) {
    comparisons = swaps = 0;
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            comparisons++;
            if (keys[j] > keys[j + 1]) {
                int tk = keys[j]; keys[j] = keys[j + 1]; keys[j + 1] = tk;
                int ti = items[j]; items[j] = items[j + 1]; items[j + 1] = ti;
                swaps++;
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

inline void insertionSort(int items[], int keys[], int n, int& comparisons, int& moves) {
    comparisons = moves = 0;
    for (int i = 1; i < n; i++) {
        int keyVal = keys[i], itemVal = items[i], j = i - 1;
        while (j >= 0) {
            comparisons++;
            if (keys[j] > keyVal) {
                keys[j + 1] = keys[j];
                items[j + 1] = items[j];
                moves++;
                j--;
            } else break;
        }
        keys[j + 1] = keyVal;
        items[j + 1] = itemVal;
    }
}

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
