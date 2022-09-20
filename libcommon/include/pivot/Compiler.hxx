#pragma once

#include <string>

#include "pivot/debug.hxx"

#if defined(COMPILER_CLANG)
    #include "pivot/Compilers/ClangCompiler.hxx"
#elif defined(COMPILER_GNU)
    #include "pivot/Compilers/GNUCompiler.hxx"
#elif defined(COMPILER_MSVC)
    #include "pivot/Compilers/MSVCCompiler.hxx"
#else
static_assert(false, "Unsuported Platform !")
#endif
