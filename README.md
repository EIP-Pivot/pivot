# Game Engine

## Building

### Create the build configuration

``` sh
bdep init --config-create build-gcc @gcc cc config.cxx='g++ -std=c++20'  config.cxx.coptions="-fmodules-ts"
```

## Build the project

``` sh
b
```

