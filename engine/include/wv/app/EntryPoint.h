// Entry point for the application
// This must be included in the main.cpp file of the project.
// CreateApp must be defined in the project.

#pragma once

#include <wv/Logger.h>
#include <wv/app/App.h>

extern WillowVox::App* WillowVox::CreateApp();

int main(int argc, char** argv)
{
    WillowVox::Logger::Init();

    auto app = WillowVox::CreateApp();
    app->Run();
    delete app;

    WillowVox::Logger::Shutdown();
}