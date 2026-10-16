#pragma once

#include <wv/input/Key.h>

namespace WillowVox
{
    class Window;

    enum class MouseMode
    {
	    NORMAL,
	    DISABLED,
	    HIDDEN
    };

    class Input
    {
    public:
        static void Init();
        static void ResetStates();

        static bool GetKey(Key key);
        static bool GetKeyDown(Key key);
        static bool GetKeyUp(Key key);

        static bool GetMouseButton(int button);
        static bool GetMouseButtonDown(int button);
        static bool GetMouseButtonUp(int button);

        static glm::vec2 GetMousePos();
        static glm::vec2 GetMouseDelta();
        static glm::vec2 GetMouseScrollDelta();

        static void SetMouseMode(MouseMode mode);

    private:
        static bool _pressed[348];
        static bool _released[348];
        static bool _mousePressed[7];
        static bool _mouseReleased[7];
        static double _mouseX, _mouseY, _prevMouseX, _prevMouseY;
        static double _scrollX, _scrollY;

        static Window* _window;
    };
}