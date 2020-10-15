# Safe Stack.

## Homework #3. for ISP RAS course.

Stack with memory protection mechanisms

1. Every operation can fail (and throw corresponding exception).
2. Stack has canaries before and after it's fields.
2. Stack calculates checksum of its fields and checks it before every
operation.
3. If object is moved out to a new place, it is marked as invalid
(`size` becomes bigger than `capacity`). Any operation on invalid object
throws exception.

## Structure

* app/ - Applications (each application has it's own `main` function)
  * main.cpp - Main application
* docs/ - Documentation pages
  * mainpage.md - Documentation main page
* extern/ - external libraries (i.e. googletest)
* include/ - header files
  * safe_stack/ - safe stack header files
    * hash.h - small library for computing object's hash
    * safe_stack.h - stack class definition, exception types and helper functions
* test/ - program tests
  * hash_test.cpp - tests for hash function
  * safe_stack_test.cpp - tests for stack

## How to build

I used Ubuntu 20.04 on WSL

```bash
git clone --recursive https://github.com/pema4/safe_stack
cd safe_stack
cmake -B build
cmake --build build -t tests
build/test/tests
cmake --build build -t main
build/app/main
```
