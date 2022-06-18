#pragma once

#include "pivot/graphics/pivot.hxx"

#include <Logger.hpp>

#ifdef NDEBUG
    #define DEBUG_FUNCTION
#else
    #define DEBUG_FUNCTION logger LOGGER_ACCESS trace(::pivot::function_name()) << "Entered";
#endif
