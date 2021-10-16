#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <any>
#include <unordered_map>

/// @class Event
///
/// @brief Send event to Event Manager
class Event
{
public:
    /// @cond
    Event() = delete;
    /// @endcond

    /// Create event
    explicit Event(EventId type): mType(type) {}

    /// Set event parameter
    template <typename T>
    void SetParam(ParamId id, T value)
    {
        mData[id] = value;
    }

    /// Get event parameter
    template <typename T>
    T GetParam(ParamId id)
    {
        return std::any_cast<T>(mData[id]);
    }

    /// Get event type
    EventId GetType() const { return mType; }

private:
    EventId mType{};
    std::unordered_map<EventId, std::any> mData{};
};