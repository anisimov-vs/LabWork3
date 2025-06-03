# Laboratory Work 3

## Author

Анисимов Василий Сергеевич, группа 24.Б81-мм

## Contacts

st129629@student.spbu.ru

## Description

This project implements a C++ header-only skip-list container `jump_list` with an STL-style associative interface under C++20. It features bidirectional and reverse iterators, concept-based constraints, exception safety, full iterator operations, and comparison operators.

### Files

- `include/jump_list.h`: Header-only implementation of the `jump_list` container.
- `tests/test.cpp`: GoogleTest suite for verifying functionality, iterators, and exception safety.
- `CMakeLists.txt`: CMake configuration for building and testing under C++20 with GoogleTest and CTest.

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Run Tests

```bash
ctest
```

### Usage

Include the header and use like an STL container:

```cpp
#include <jump_list.h>
#include <iostream>

int main() {
    jump_list<int> jl;
    jl.insert({3, 1, 4, 1, 5});
    for (auto& value : jl) {
        std::cout << value << ' ';
    }
    // Output: 1 1 3 4 5
    return 0;
}
``` 