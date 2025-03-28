#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>

namespace WillowVox
{
    class WILLOWVOX_API MouseReleaseEvent : public Event
    {
    public:
        MouseReleaseEvent(int button) 
            : Event(Type::MouseRelease), m_button(button) {}

        std::string ToString() const override { return "MouseReleaseEvent"; }
    
        int m_button;
    };
}