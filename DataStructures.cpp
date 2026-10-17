#include "DataStructures.h"
#include <iostream>

string toLowerCase(string text) {
    string result = text;
    for (int i = 0; i < (int)result.length(); i++) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] = result[i] + ('a' - 'A');
        }
    }
    return result;
}

IntList::IntList() {
    head = NULL;
    count = 0;
}

IntList::~IntList() {
    clear();
}

IntList::IntList(const IntList& other) {
    head = NULL;
    count = 0;
    IntNode* temp = other.head;
    while (temp != NULL) {
        pushBack(temp->data);
        temp = temp->next;
    }
}

void IntList::pushBack(int value) {
    IntNode* newNode = new IntNode();
    newNode->data = value;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        IntNode* temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    count++;
}

void IntList::pushFront(int value) {
    IntNode* newNode = new IntNode();
    newNode->data = value;
    newNode->next = head;
    head = newNode;
    count++;
}

bool IntList::contains(int value) {
    IntNode* temp = head;
    while (temp != NULL) {
        if (temp->data == value) {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

int IntList::getAt(int position) {
    if (position < 0 || position >= count) {
        return -1;
    }
    IntNode* temp = head;
    for (int i = 0; i < position; i++) {
        temp = temp->next;
    }
    return temp->data;
}

int IntList::size() {
    return count;
}

bool IntList::isEmpty() {
    return (count == 0);
}

void IntList::clear() {
    IntNode* temp = head;
    while (temp != NULL) {
        IntNode* nextNode = temp->next;
        delete temp;
        temp = nextNode;
    }
    head = NULL;
    count = 0;
}

IntStack::IntStack() {
    top = -1;
}

void IntStack::push(int value) {
    if (top < 499) {
        top++;
        items[top] = value;
    }
}

int IntStack::pop() {
    if (top >= 0) {
        int val = items[top];
        top--;
        return val;
    }
    return -1;
}

int IntStack::peek() {
    if (top >= 0) {
        return items[top];
    }
    return -1;
}

bool IntStack::isEmpty() {
    return (top == -1);
}

int IntStack::size() {
    return top + 1;
}

void IntStack::clear() {
    top = -1;
}

IntQueue::IntQueue() {
    frontIndex = 0;
    rearIndex = 0;
    count = 0;
}

void IntQueue::enqueue(int value) {
    if (count < 500) {
        items[rearIndex] = value;
        rearIndex = (rearIndex + 1) % 500;
        count++;
    }
}

int IntQueue::dequeue() {
    if (count > 0) {
        int val = items[frontIndex];
        frontIndex = (frontIndex + 1) % 500;
        count--;
        return val;
    }
    return -1;
}

int IntQueue::getFront() {
    if (count > 0) {
        return items[frontIndex];
    }
    return -1;
}

bool IntQueue::isEmpty() {
    return (count == 0);
}

int IntQueue::size() {
    return count;
}

void IntQueue::clear() {
    frontIndex = 0;
    rearIndex = 0;
    count = 0;
}

HashTable::HashTable() {
    for (int i = 0; i < 211; i++) {
        table[i].key = "";
        table[i].value = -1;
        table[i].isOccupied = false;
    }
}

int HashTable::hashFunction(string key) {
    int sum = 0;
    for (int i = 0; i < (int)key.length(); i++) {
        sum = sum * 31 + (unsigned char)key[i];
    }
    if (sum < 0) {
        sum = -sum;
    }
    return sum % 211;
}

bool HashTable::insert(string key, int value) {
    string lowerKey = toLowerCase(key);
    int index = hashFunction(lowerKey);

    for (int i = 0; i < 211; i++) {
        int pos = (index + i) % 211;
        if (!table[pos].isOccupied) {
            table[pos].key = lowerKey;
            table[pos].value = value;
            table[pos].isOccupied = true;
            return true;
        }
        if (table[pos].key == lowerKey) {
            return false;
        }
    }
    return false;
}

int HashTable::get(string key) {
    string lowerKey = toLowerCase(key);
    int index = hashFunction(lowerKey);

    for (int i = 0; i < 211; i++) {
        int pos = (index + i) % 211;
        if (!table[pos].isOccupied) {
            return -1;
        }
        if (table[pos].key == lowerKey) {
            return table[pos].value;
        }
    }
    return -1;
}
