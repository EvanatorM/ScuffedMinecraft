#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>

namespace WillowVox
{
    class WILLOWVOX_API WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() : Event(Type::WindowClose) {}

        std::string ToString() const override { return "WindowCloseEvent"; }
    };
}