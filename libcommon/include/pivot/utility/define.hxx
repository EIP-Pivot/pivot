#pragma once

#define PIVOT_MACRO_EXPENDER2(X, Y) X##Y
#define PIVOT_MACRO_EXPENDER(X, Y) PIVOT_MACRO_EXPENDER2(X, Y)

#define STR(x) #x

#define PIVOT_NO_COPY(Name)      \
    Name(const Name &) = delete; \
    Name &operator=(const Name &);

#define PIVOT_NO_MOVE(Name) \
    Name(Name &&) = delete; \
    Name &operator=(Name &&);

#define PIVOT_NO_COPY_NO_MOVE(Name) PIVOT_NO_COPY(Name) PIVOT_NO_MOVE(Name)