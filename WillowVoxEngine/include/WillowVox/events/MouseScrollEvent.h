#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>

namespace WillowVox
{
    class WILLOWVOX_API MouseScrollEvent : public Event
    {
    public:
        MouseScrollEvent(float xOffset, float yOffset)
            : Event(Type::MouseScroll), m_xOffset(xOffset), m_yOffset(yOffset) {}

        std::string ToString() const override { return "MouseScrollEvent"; }

        float m_xOffset, m_yOffset;
    };
}