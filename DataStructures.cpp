#pragma once
#include <string>
using namespace std;

inline string toLowerCase(string text) {
    string result = text;
    for (int i = 0; i < (int)result.length(); i++) {
        if (result[i] >= 'A' && result[i] <= 'Z') result[i] += ('a' - 'A');
    }
    return result;
}

struct IntNode {
    int data;
    IntNode* next;
};

struct IntList {
    IntNode* head;
    int count;

    IntList() : head(NULL), count(0) {}
    ~IntList() { clear(); }

    IntList(const IntList& other) : head(NULL), count(0) {
        for (IntNode* t = other.head; t; t = t->next) pushBack(t->data);
    }

    IntList& operator=(const IntList& other) {
        if (this != &other) {
            clear();
            for (IntNode* t = other.head; t; t = t->next) pushBack(t->data);
        }
        return *this;
    }

    void pushBack(int value) {
        IntNode* n = new IntNode{value, NULL};
        if (!head) head = n;
        else {
            IntNode* t = head;
            while (t->next) t = t->next;
            t->next = n;
        }
        count++;
    }

    int getAt(int position) const {
        if (position < 0 || position >= count) return -1;
        IntNode* t = head;
        for (int i = 0; i < position; i++) t = t->next;
        return t->data;
    }

    int size() const { return count; }
    bool isEmpty() const { return count == 0; }

    void clear() {
        while (head) {
            IntNode* next = head->next;
            delete head;
            head = next;
        }
        count = 0;
    }
};

struct IntStack {
    int items[500];
    int top;

    IntStack() : top(-1) {}
    void push(int value) { if (top < 499) items[++top] = value; }
    int pop() { return top >= 0 ? items[top--] : -1; }
    bool isEmpty() const { return top == -1; }
};

struct IntQueue {
    int items[500];
    int frontIndex;
    int rearIndex;
    int count;

    IntQueue() : frontIndex(0), rearIndex(0), count(0) {}
    void enqueue(int value) {
        if (count < 500) {
            items[rearIndex] = value;
            rearIndex = (rearIndex + 1) % 500;
            count++;
        }
    }
    int dequeue() {
        if (count <= 0) return -1;
        int val = items[frontIndex];
        frontIndex = (frontIndex + 1) % 500;
        count--;
        return val;
    }
    bool isEmpty() const { return count == 0; }
};

struct HashEntry {
    string key;
    int value;
    bool isOccupied;
};

struct HashTable {
    HashEntry table[211];

    HashTable() {
        for (int i = 0; i < 211; i++) table[i] = {"", -1, false};
    }

    int hashFunction(string key) const {
        int sum = 0;
        for (int i = 0; i < (int)key.length(); i++) sum = sum * 31 + (unsigned char)key[i];
        if (sum < 0) sum = -sum;
        return sum % 211;
    }

    bool insert(string key, int value) {
        string lowerKey = toLowerCase(key);
        int idx = hashFunction(lowerKey);
        for (int i = 0; i < 211; i++) {
            int pos = (idx + i) % 211;
            if (!table[pos].isOccupied) {
                table[pos] = {lowerKey, value, true};
                return true;
            }
            if (table[pos].key == lowerKey) return false;
        }
        return false;
    }

    int get(string key) const {
        string lowerKey = toLowerCase(key);
        int idx = hashFunction(lowerKey);
        for (int i = 0; i < 211; i++) {
            int pos = (idx + i) % 211;
            if (!table[pos].isOccupied) return -1;
            if (table[pos].key == lowerKey) return table[pos].value;
        }
        return -1;
    }
};
