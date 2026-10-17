#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
using namespace std;

string toLowerCase(string text);

struct IntNode {
    int data;
    IntNode* next;
};

class IntList {
public:
    IntNode* head;
    int count;

    IntList();
    ~IntList();
    IntList(const IntList& other);

    void pushBack(int value);
    void pushFront(int value);
    bool contains(int value);
    int getAt(int position);
    int size();
    bool isEmpty();
    void clear();
};

class IntStack {
public:
    int items[500];
    int top;

    IntStack();
    void push(int value);
    int pop();
    int peek();
    bool isEmpty();
    int size();
    void clear();
};

class IntQueue {
public:
    int items[500];
    int frontIndex;
    int rearIndex;
    int count;

    IntQueue();
    void enqueue(int value);
    int dequeue();
    int getFront();
    bool isEmpty();
    int size();
    void clear();
};

struct HashEntry {
    string key;
    int value;
    bool isOccupied;
};

class HashTable {
public:
    HashEntry table[211];

    HashTable();
    int hashFunction(string key);
    bool insert(string key, int value);
    int get(string key);
};

#endif
