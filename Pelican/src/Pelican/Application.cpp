#include "Application.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace Pelican
{
	void Application::Run()
	{
		if (!glfwInit())
		{
			std::cerr << "Failed to initialize GLFW!" << std::endl;
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello Pelican Engine!", nullptr, nullptr);
		if (!window)
		{
			std::cerr << "Failed to create GLFW window!" << std::endl;
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "Failed to initialize GLAD!" << std::endl;
			glfwTerminate();
			return;
		}

		glViewport(0, 0, 1280, 720);

		while (!glfwWindowShouldClose(window))
		{
			glClearColor(0.8f, 0.3f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glfwPollEvents();

			glfwSwapBuffers(window);
		}

		glfwTerminate();
	}
}
