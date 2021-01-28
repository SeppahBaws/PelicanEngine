#pragma once
#include "Application.h"

int main(int, char**)
{
	Pelican::Application* app = Pelican::CreateApplication();

	app->Run();

	delete app;
}
