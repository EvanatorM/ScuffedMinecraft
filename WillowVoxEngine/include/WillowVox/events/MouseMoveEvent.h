#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>

namespace WillowVox
{
    class WILLOWVOX_API MouseMoveEvent : public Event
    {
    public:
        MouseMoveEvent(float xOffset, float yOffset) 
            : Event(Type::MouseMove), m_xOffset(xOffset), m_yOffset(yOffset) {}

        std::string ToString() const override { return "MouseMoveEvent"; }
    
        float m_xOffset, m_yOffset;
    };
}