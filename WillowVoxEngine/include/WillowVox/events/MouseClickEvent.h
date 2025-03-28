#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>

namespace WillowVox
{
    class WILLOWVOX_API MouseClickEvent : public Event
    {
    public:
        MouseClickEvent(int button) 
            : Event(Type::MouseClick), m_button(button) {}

        std::string ToString() const override { return "MouseClickEvent"; }
    
        int m_button;
    };
}