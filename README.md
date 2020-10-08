# Safe Stack.
## Homework #3. for ISP RAS course.

**This is NOT a final version, I will add more documentation, tests and checks**

Not done yet:
* debug dumps

## How to build

I used Ubuntu 20.04 within WSL

```bash
git clone --recursive https://github.com/pema4/safe_stack
cd safe_stack
cmake -B build
cmake --build build -t tests
build/test/tests
cmake --build build -t main
build/app/main
```
