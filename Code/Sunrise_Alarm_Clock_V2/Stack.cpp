#include "Stack.h"

Stack::Stack() {
  top = -1;  // Initialize top of the stack
}

bool Stack::isEmpty() {
  return top == -1;
}

bool Stack::isFull() {
  return top == MAX_STACK_SIZE - 1;
}

void Stack::push(MenuItem* item) {  
  if (!isFull()) {
    items[++top] = item;
  }
}

MenuItem* Stack::pop() {  
  if (!isEmpty()) {
    return items[top--];
  }
  return nullptr;  // Return nullptr to indicate an empty stack
}

MenuItem* Stack::peek() {  
  if (!isEmpty()) {
    return items[top];
  }
  return nullptr;  // Return nullptr to indicate an empty stack
}
