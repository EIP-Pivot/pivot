#pragma once

#include "pivot/ecs/Core/types.hxx"
#include <any>
#include <unordered_map>

/*! \cond
 */
class Event
{
public:
    Event() = delete;

    explicit Event(EventId type): mType(type) {}

    template <typename T>
    void SetParam(ParamId id, T value)
    {
        mData[id] = value;
    }

    template <typename T>
    T GetParam(ParamId id)
    {
        return std::any_cast<T>(mData[id]);
    }

    EventId GetType() const { return mType; }

private:
    EventId mType{};
    std::unordered_map<EventId, std::any> mData{};
};
/*! \endcond
 */