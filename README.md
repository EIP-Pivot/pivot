# Game Engine

## Build 2

This project uses the staging version of Build2.

You can install it like this:

``` sh
wget https://stage.build2.org/0/0.14.0-a.0/build2-install-0.14.0-a.0-stage.sh
sh build2-install-0.14.0-a.0-stage.sh <install-dir>
```

Don't forget to add it to your path if you don't install it in the system
directories.

## Building

### Create the build configuration

``` sh
bdep init --config-create ../build-gcc @gcc cc config.cxx=g++
```

### Build the project

``` sh
b
```

## Other

### Create `compile_commands.json`

``` sh
b -vn clean update |& compiledb
```
