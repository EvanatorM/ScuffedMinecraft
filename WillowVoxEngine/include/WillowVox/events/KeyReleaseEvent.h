#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>
#include <WillowVox/input/Key.h>

namespace WillowVox
{
    class WILLOWVOX_API KeyReleaseEvent : public Event
    {
    public:
        KeyReleaseEvent(Key key) 
            : Event(Type::KeyRelease), m_key(key) {}

        std::string ToString() const override { return "KeyReleaseEvent"; }
    
        Key m_key;
    };
}