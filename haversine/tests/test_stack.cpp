#include <iostream>

#include "../haversine_types.h"

#include "../haversine_stack.cpp"

#define ASSERTM(condition, message)                                             \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__   \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            return 1;                                                          \
        }                                                                      \
    } while (false)

#define ASSERT(condition)                                                      \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__   \
                      << " line " << __LINE__ << std::endl;                    \
            return 1;                                                          \
        }                                                                      \
    } while (false)

typedef int (*TestProtocol)();

int test_stack_1() {
    Stack::Stack stack = Stack::create();
    Stack::push(&stack, 32);
    Stack::push(&stack, 24);
    u64 actual_value = Stack::pop(&stack);
    ASSERT(actual_value == 24);
    actual_value = Stack::pop(&stack);
    ASSERT(actual_value == 32);
    return 0;
}

int test_stack_2() {
    Stack::Stack stack = Stack::create();
    Stack::push(&stack, 1);
    u64 actual_value = Stack::pop(&stack);
    ASSERT(actual_value == 1);
    return 0;
}

TestProtocol tests_to_run[] = {
    test_stack_1,
    test_stack_2,
};

int main(int argc, char *argv[]) {
    for (u64 i = 0; i < ArrayCount(tests_to_run); i++) {
        b32 result = tests_to_run[i]();
        if (result) {
            printf("Test failed\n");
        } else {
            printf("Test passed\n");
        }
    }
    return 0;
}
