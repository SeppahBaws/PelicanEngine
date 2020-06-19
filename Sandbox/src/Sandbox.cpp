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
	return new MyApp();
}
