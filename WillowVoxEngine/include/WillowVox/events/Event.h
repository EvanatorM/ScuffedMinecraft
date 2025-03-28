#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <string>

namespace WillowVox
{
    class WILLOWVOX_API Event
    {
    public:
        enum class Type
        {
            None = 0,
            KeyPress, KeyRelease,
            MouseMove, MouseClick, MouseRelease, MouseScroll,
            WindowResize, WindowClose
        };

        Event(Type type) : m_type(type), m_handled(false) {}

        Type GetType() const { return m_type; }
        bool IsHandled() const { return m_handled; }
        void MarkHandled() { m_handled = true; }

        virtual std::string ToString() const { return "Event"; }

    private:
        Type m_type;
        bool m_handled;
    };
}