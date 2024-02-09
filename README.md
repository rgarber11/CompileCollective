# Compile Collective
## Features Added
- [x] Integer Addition, Subtraction, Multiplication, Division
- [x] Parenthetical Expressions
- [x] Some Front End, Back End
- [ ] Functions
- [ ] Statements
- [ ] Classes
- [ ] etc..
## Requirements
- [vcpkg](https://vcpkg.io/en/)
- [CMake](https://cmake.org)
## Setup
1. Ensure vcpkg, CMake, and a C++ compiler are installed on your computer
2. Clone this directory with `git clone`
3. Configure the build using `cmake --preset=debug` (Note this can take a long time the first time it is run)
4. Build with `cmake --build out/Debug`
## Running Compile Collective
1. Navigate to the `out/Debug/src` folder.
2. create an example math expression in a file like `math.coco`:
~~~coco
3 + 5 * (4 - 2)  / 2
~~~
3. Run `./SeniorProject math.coco output.o` to generate an object file from the output. Printed to the screen, you'll see a Postfix version of the expression, along with the LLVM IR
4. Use a C compiler to link the output. (Ex. `clang++ -o main output.o`)
5. Running this executable, you'll see the answer to the math equation
```bash
# ./main
8
```
