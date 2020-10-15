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
