// =====================================================================
// File: DataStructures.h
// Purpose: Simple, beginner-friendly hand-made data structures:
//          1. IntList   - Singly Linked List of numbers
//          2. IntStack  - Array-based Stack (LIFO: Last In, First Out)
//          3. IntQueue  - Array-based Queue (FIFO: First In, First Out)
//          4. HashTable - Open Addressing Hash Table (string -> int)
//
// No friend functions, no static variables, no STL containers.
// All functions use call-by-value.
// =====================================================================
#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
using namespace std;

// Helper function to convert a string to lowercase (call by value)
string toLowerCase(string text);

// ---------------------------------------------------------------------
// 1. SINGLY LINKED LIST
// Stores integer values (such as station or bus indexes).
// ---------------------------------------------------------------------
struct IntNode {
    int data;        // the value stored in this node
    IntNode* next;   // pointer to the next node in the list
};

class IntList {
public:
    IntNode* head;   // pointer to the first node
    int count;       // number of elements in the list

    IntList();
    ~IntList();
    IntList(const IntList& other); // copy constructor so call-by-value works safely

    void pushBack(int value);       // add value to the end of the list
    void pushFront(int value);      // add value to the start of the list
    bool contains(int value);       // check if value is in the list
    int getAt(int position);        // get value at index (0, 1, 2...)
    int size();                     // returns number of elements
    bool isEmpty();                 // returns true if empty
    void clear();                   // deletes all nodes
};

// ---------------------------------------------------------------------
// 2. STACK (Array implementation)
// Standard LIFO data structure.
// ---------------------------------------------------------------------
class IntStack {
public:
    int items[500];  // fixed array to hold items
    int top;         // index of the top item (-1 when empty)

    IntStack();
    void push(int value);  // add item to top
    int pop();             // remove and return top item (-1 if empty)
    int peek();            // view top item without removing
    bool isEmpty();        // true if stack is empty
    int size();            // number of items in stack
    void clear();          // reset stack
};

// ---------------------------------------------------------------------
// 3. QUEUE (Circular array implementation)
// Standard FIFO data structure.
// ---------------------------------------------------------------------
class IntQueue {
public:
    int items[500];   // fixed array to hold items
    int frontIndex;   // index of the front element
    int rearIndex;    // index where next element will be added
    int count;        // number of items currently in queue

    IntQueue();
    void enqueue(int value);  // add to rear of queue
    int dequeue();            // remove and return front element (-1 if empty)
    int getFront();           // view front element without removing
    bool isEmpty();           // true if queue is empty
    int size();               // number of items in queue
    void clear();             // reset queue
};

// ---------------------------------------------------------------------
// 4. HASH TABLE (Open addressing with linear probing)
// Maps a station name (string) to station array index (int).
// ---------------------------------------------------------------------
struct HashEntry {
    string key;       // station name in lowercase
    int value;        // station index (0, 1, 2...)
    bool isOccupied;  // true if this slot has data
};

class HashTable {
public:
    HashEntry table[211]; // table size is a prime number (211)

    HashTable();
    int hashFunction(string key);        // calculates hash index
    bool insert(string key, int value);  // inserts key-value pair
    int get(string key);                 // returns value or -1 if not found
};

#endif
