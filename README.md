# game-engine

## Dependencies
- cmake >= 3.11
- Vulkan ~1.2

## Build
```sh 
mkdir build && cd build && cmake .. && cmake --build .  --parallel 4
```

## Compile Command
```sh
cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cp compile_commands.json ..
```
