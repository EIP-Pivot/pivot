#pragma once

#include <exception>

namespace pivot::ecs::script
{

/// Invalid, mixed or mis-matched indent errors
class InvalidIndentException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Token was expected but reached end of file errors
class UnexpectedEOFException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Incorrect token type errors
class UnexpectedTokenTypeException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Incorrect token value errors
class UnexpectedTokenValueException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Incorrect node type errors
class UnexpectedNodeTypeException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Incorrect node value errors
class UnexpectedNodeValueException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Unknown type errors
class UnknownTypeException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Invalid operation errors
class InvalidOperation : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Invalid errors
class InvalidException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

}    // end of namespace pivot::ecs::script
