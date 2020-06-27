// ReSharper disable once CppUnusedIncludeDirective
#include <vld.h>
#include <iostream>
#include <Pelican.h>

class MyApp final : public Pelican::Application
{
public:
	MyApp() = default;
	virtual ~MyApp() = default;
};

Pelican::Application* Pelican::CreateApplication()
{
	// return new MyApp();
	return new Pelican::Application();
}
