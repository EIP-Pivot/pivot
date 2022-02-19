#ifndef __SCRIPTING__EXCEPTIONS__
#define __SCRIPTING__EXCEPTIONS__

#include <exception>
#include "pivot/ecs/Core/Scripting/Types.hxx"

class BadIndentException : public std::exception {
    size_t linenb;
    const char* line;
    const char* info;
    
    public:
    	BadIndentException(const char* msg, const char* line_,  size_t linenb_, const char* info_ = "") : std::exception(msg),
        	line (line_),
        	linenb (linenb_),
        	info (info_)
        {
        }
        size_t get_line_nb() const { return linenb; }
        const char* get_line() const { return line; }
        const char* get_info() const { return info; }
};

class InvalidSyntaxException : public std::exception {
    size_t linenb;
    const char* line;
    const char* info;
    
    public:
    	InvalidSyntaxException(const char* msg, const char* line_,  size_t linenb_, const char* info_ = "") : std::exception(msg),
        	line (line_),
        	linenb (linenb_),
        	info (info_)
        {
        }
        size_t get_line_nb() const { return linenb; }
        const char* get_line() const { return line; }
        const char* get_info() const { return info; }
};

class UnknownExpressionException : public std::exception {
    size_t linenb;
    const char* line;
    const char* info;
    
    public:
    	UnknownExpressionException(const char* msg, const char* line_,  size_t linenb_, const char* info_ = "") : std::exception(msg),
        	line (line_),
        	linenb (linenb_),
        	info (info_)
        {
        }
        size_t get_line_nb() const { return linenb; }
        const char* get_line() const { return line; }
        const char* get_info() const { return info; }
};

class UnexpectedStateException : public std::exception {
    size_t linenb;
    const char* line;
	State prevState;
	State newState;
    
    public:
    	UnexpectedStateException(const char* msg, const char* line_,  size_t linenb_, State prevState_, State newState_) : std::exception(msg),
        	line (line_),
        	prevState (prevState_),
        	newState (newState_),
        	linenb (linenb_)
        {
        }
        size_t get_line_nb() const { return linenb; }
        const char* get_line() const { return line; }
        State get_prev_state() const { return prevState; }
        State get_new_state() const { return newState; }
};

class UnhandledException : public std::exception {
    
    public:
    	UnhandledException(const char* msg) : std::exception(msg)
        {
        }
};

#endif