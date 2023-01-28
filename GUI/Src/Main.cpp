#include "UI/CPUWindows.h"
#include "Utils/Core.h"

#include <cstdio>
#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

static void GLFWErrorCallback(int error, const char* description)
{
	std::printf("GLFW Error %d: %s\n", error, description);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	glfwSetErrorCallback(&GLFWErrorCallback);
	if (!glfwInit())
		return 1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	auto window = glfwCreateWindow(1280, 720, "Profiler GUI", nullptr, nullptr);
	if (!window)
		return 2;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&glfwGetProcAddress)))
		return 3;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io                          = ImGui::GetIO();
	io.ConfigFlags                       |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags                       |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags                       |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding              = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	bool showDemoWindow = true;
	bool showCPUCores   = true;
	// UI::CPUCoresData cpuCoresData {};
	UI::TimelineOptions timelineOptions {};

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(mainViewport->Pos);
		ImGui::SetNextWindowSize(mainViewport->Size);
		ImGui::SetNextWindowViewport(mainViewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

		ImGuiWindowFlags DockspaceWindowFlags = ImGuiWindowFlags_MenuBar |
												ImGuiWindowFlags_NoDocking |
												ImGuiWindowFlags_NoTitleBar |
												ImGuiWindowFlags_NoCollapse |
												ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoMove |
												ImGuiWindowFlags_NoBringToFrontOnFocus |
												ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("MainDockspaceWindow", nullptr, DockspaceWindowFlags);
		ImGui::PopStyleVar(3);

		ImGui::DockSpace(ImGui::GetID("MainDockspace"));
		if (ImGui::BeginMainMenuBar())
		{
			// m_MenuBarHeight = ImGui::GetWindowHeight();

			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Show Demo Window"))
					showDemoWindow = true;
				if (ImGui::MenuItem("Show CPU Cores"))
					showCPUCores = true;
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showCPUCores)
		{
			ImGui::Begin("Timeline", &showCPUCores);

			UI::DefaultTimelineStyle(&timelineOptions);
			UI::DrawTimescale(&timelineOptions);

			ImGui::End();
		}
		// UI::ShowCPUCores(&showCPUCores, &cpuCoresData, 1.0f / 144.0f);

		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);

		ImGui::End();

		ImGui::Render();

		{
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			glViewport(0, 0, w, h);
		}
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			auto origContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(origContext);
		}

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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