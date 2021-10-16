#pragma once

#include <exception>
#include <iostream>

/// @cond
class EcsException: public std::exception
{
public:
    EcsException(const std::string &message="") throw()
         :_message(message)
    {}
 
     virtual const char* what() const throw()
     {
         return _message.c_str();
     }

    virtual ~EcsException() throw()
    {}
 
private:
    std::string _message;
};
/// @endcond