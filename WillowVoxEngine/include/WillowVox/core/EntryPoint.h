#pragma once

#include <WillowVox/core/Application.h>

extern WillowVox::Application* WillowVox::CreateApplication();

int main(int argc, char** argv)
{
	auto app = WillowVox::CreateApplication();
	app->Run();
	delete app;
	//WillowVox::Logger::CloseLogger();
}