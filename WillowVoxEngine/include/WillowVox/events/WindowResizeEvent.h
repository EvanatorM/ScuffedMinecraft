#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>

namespace WillowVox
{
    class WILLOWVOX_API WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int newWidth, int newHeight) 
         : Event(Type::WindowResize), m_newWidth(newWidth), m_newHeight(newHeight) {}

        std::string ToString() const override { return "WindowResizeEvent"; }

        int m_newWidth, m_newHeight;
    };
}