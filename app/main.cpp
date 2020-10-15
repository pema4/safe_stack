#include "safe_stack/safe_stack.h"
#include <iostream>

using namespace safe_stack;

int main() {
    Stack<std::string> stack;
    stack.push("I don't know what to do in main program!");
    std::cout << "Stack content: " + stack.top() + "\n";
    std::cout << stack;
    return 0;
}
