#pragma once

#include <Logger.hpp>

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifdef NDEBUG
    #define DEBUG_FUNCTION
#else
    #define DEBUG_FUNCTION logger LOGGER_ACCESS trace(__PRETTY_FUNCTION__) << "Entered";
#endif
