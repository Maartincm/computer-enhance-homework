#include "haversine_types.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace Stack {

#define STACK_SIZE 1024

struct Stack {
  u64 size;
  void *current;
};

Stack create() {
  void *mem = malloc(STACK_SIZE);
  return Stack{0, mem};
}

void push(Stack *stack, void *data, u64 size) {
  stack->size += size;
  if (stack->size > STACK_SIZE) {
    throw std::runtime_error("Stack overflow.");
  }
  memcpy(stack->current, data, size);
  stack->current = (u8 *)stack->current + size;
}

void *pop(Stack *stack, u64 size) {
  stack->size -= size;
  if (stack->size < 0) {
    throw std::runtime_error("Stack underflow.");
  }
  stack->current = (u8 *)stack->current - size;
  return (void *)stack->current;
}

void *peek(Stack *stack, u64 size) {
  void *peek_loc = (u8 *)stack->current - size;
  if (stack->size < size) {
    throw std::runtime_error("Stack underflow.");
  }
  return peek_loc;
}
}
