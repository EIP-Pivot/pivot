#pragma once

#include <array>
#include <bitset>

class Component
{
public:
    virtual void init() {}
    virtual void update() {}
    virtual void draw() {}
    virtual ~Component() {}
};

using ComponentID = std::size_t;

inline ComponentID getComponentTypeID()
{
    static ComponentID lastID = 0;
    return lastID++;
}

template <typename T>
inline ComponentID getComponentTypeID() noexcept
{
    static ComponentID typeID = getComponentTypeID();
    return typeID;
}

constexpr std::size_t maxComponent = 32;

using ComponentBitSet = std::bitset<maxComponent>;
using ComponentArray = std::array<Component *, maxComponent>;
