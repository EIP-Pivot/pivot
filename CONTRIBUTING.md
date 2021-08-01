# Contributing to Pivot Game Engine

## Create a new package

``` sh
# New library (must start with lib !)
bdep new --package -t lib -l c++,mxx=mxx <library-name>
# New binary
bdep new --package -t bin -l c++,mxx=mxx <binary-name>
```

Change the first line of the `build/root.build` to support modules:

```
cxx.std = experimental
cxx.features.modules = true
```
