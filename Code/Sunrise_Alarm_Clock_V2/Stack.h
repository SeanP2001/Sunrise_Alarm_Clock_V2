#ifndef STACK_H
#define STACK_H

#include "MenuItem.h"  // Include the MenuItem definition

class Stack {
private:
  static const int MAX_STACK_SIZE = 10;
  MenuItem* items[MAX_STACK_SIZE];
  int top;

public:
  Stack();
  bool isEmpty();
  bool isFull();
  void push(MenuItem* item);  
  MenuItem* pop();            
  MenuItem* peek();           
};

#endif // STACK_H
