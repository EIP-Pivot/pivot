# game-engine

## Dependencies
- cmake >= 3.11
- Vulkan ~1.2
- Doxygen

## Build
```sh 
cmake -B build && cmake --build build --parallel 4
```

## Compile Command
```sh
cmake . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

## Build the documentation

``` sh
cmake -B build -DBUILD_DOCUMENTATION=ON && cmake --build build --target doc
```

