#include "KeyInput.h"

#include <algorithm>

std::vector<KeyInput*> KeyInput::instances;
glm::vec2 KeyInput::cursorPos;
glm::vec2 KeyInput::mouseScroll;

KeyInput::KeyInput(const std::vector<int>& keysToMonitor)
{
    for (int key : keysToMonitor)
        keys[key] = false;
    // Add this instance to the list of instances
    KeyInput::instances.push_back(this);
}

KeyInput::~KeyInput()
{
    // Remove this instance from the list of instances
    instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
}

bool KeyInput::isKeyDown(int key)
{
    std::map<int, bool>::iterator it = keys.find(key);
    if (it != keys.end())
        return keys[key];
}

void KeyInput::setKeyDown(int key, bool isDown)
{
    std::map<int, bool>::iterator it = keys.find(key);
    if (it != keys.end())
        keys[key] = isDown;
}

void KeyInput::setupCallbacks(GLFWwindow*& window)
{
    glfwSetKeyCallback(window, KeyInput::keyCallback);
    glfwSetCursorPosCallback(window, KeyInput::cursorPosCallback);
    glfwSetMouseButtonCallback(window, KeyInput::mouseButtonCallBack);
}

void KeyInput::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Send key event to all KeyInput instances
    for (KeyInput* keyInput : instances)
        keyInput->setKeyDown(key, action != GLFW_RELEASE);
}

void KeyInput::mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    // Send key event to all KeyInput instances
    for (KeyInput* keyInput : instances)
        keyInput->setKeyDown(button, action != GLFW_RELEASE);
}

void KeyInput::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    cursorPos.x = xpos;
    cursorPos.y = ypos;
}

void KeyInput::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) 
{
    mouseScroll.x = xoffset;
    mouseScroll.y = yoffset;
}