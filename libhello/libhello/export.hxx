#pragma once

// Normally we don't export class templates (but do complete specializations),
// inline functions, and classes with only inline member functions. Exporting
// classes that inherit from non-exported/imported bases (e.g., std::string)
// will end up badly. The only known workarounds are to not inherit or to not
// export. Also, MinGW GCC doesn't like seeing non-exported functions being
// used before their inline definition. The workaround is to reorder code. In
// the end it's all trial and error.

#if defined(LIBHELLO_STATIC)         // Using static.
#  define LIBHELLO_SYMEXPORT
#elif defined(LIBHELLO_STATIC_BUILD) // Building static.
#  define LIBHELLO_SYMEXPORT
#elif defined(LIBHELLO_SHARED)       // Using shared.
#  ifdef _WIN32
#    define LIBHELLO_SYMEXPORT __declspec(dllimport)
#  else
#    define LIBHELLO_SYMEXPORT
#  endif
#elif defined(LIBHELLO_SHARED_BUILD) // Building shared.
#  ifdef _WIN32
#    define LIBHELLO_SYMEXPORT __declspec(dllexport)
#  else
#    define LIBHELLO_SYMEXPORT
#  endif
#else
// If none of the above macros are defined, then we assume we are being used
// by some third-party build system that cannot/doesn't signal the library
// type. Note that this fallback works for both static and shared libraries
// provided the library only exports functions (in other words, no global
// exported data) and for the shared case the result will be sub-optimal
// compared to having dllimport. If, however, your library does export data,
// then you will probably want to replace the fallback with the (commented
// out) error since it won't work for the shared case.
//
#  define LIBHELLO_SYMEXPORT         // Using static or shared.
//#  error define LIBHELLO_STATIC or LIBHELLO_SHARED preprocessor macro to signal libhello library type being linked
#endif
