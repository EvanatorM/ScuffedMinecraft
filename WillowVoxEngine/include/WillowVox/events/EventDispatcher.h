#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/events/Event.h>
#include <functional>
#include <unordered_map>
#include <vector>

namespace WillowVox
{
    template <typename T> class EventDispatcher
    {
    public:
        using Listener = std::function<void(T&)>;

        // Register a listener for a specific event type
        void RegisterListener(Listener listener)
        {
            m_listeners.push_back(listener);
        }

        // Unregister all listeners for a specific event type
        void UnregisterAllListeners()
        {
            m_listeners.clear();
        }

        // Dispatch an event to all registered listeners
        void Dispatch(T& event)
        {
            for (auto& l : m_listeners)
            {
                l(event);

                // Stop propagation if event is marked as handled
                if (event.IsHandled())
                    break;
            }
        }

    private:
        // Store all the event listeners
        std::vector<Listener> m_listeners;
    };
}