#include "Utils/Core.h"

#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	if (!glfwInit())
		return 1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto window = glfwCreateWindow(1280, 720, "Profiler GUI", nullptr, nullptr);
	if (!window)
		return 1;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

#if BUILD_IS_SYSTEM_WINDOWS

	#undef APIENTRY
	#include <Windows.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	return main(__argc, __argv);
}

#endif