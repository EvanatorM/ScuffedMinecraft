#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <map>
#include <vector>

// The naming conventions of this project do not seems to be 100% clear 
// but I guess that member functions start with a uppercase letters and 
// functions with lowercase
class KeyInput
{
private:
    std::map<int, bool> keys;
    static std::vector<KeyInput*> instances;
public:
    static glm::vec2 cursorPos;
    static glm::vec2 mouseScroll;
public:
    /// This takes in a vector containing all keys that should be monitord by this instance
    KeyInput(const std::vector<int>& keysToMonitor);
    ~KeyInput();
    bool isKeyDown(int key);
    // This function should only be called once after OpenGL and glfw are all set up and ready to go 
    static void setupCallbacks(GLFWwindow*& window);

private:
    void setKeyDown(int key, bool isDown);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};