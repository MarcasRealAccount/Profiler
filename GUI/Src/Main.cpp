#include "UI/CPUWindows.h"
#include "Utils/Core.h"

#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <vector>

#include <Profiler/Profiler.h>

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
	// glfwSwapInterval(0);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&glfwGetProcAddress)))
		return 3;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io                          = ImGui::GetIO();
	io.ConfigFlags                       |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags                       |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags                       |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImFont* defaultFont = io.Fonts->AddFontDefault();

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding              = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	bool                           showDemoWindow  = true;
	bool                           showCoresView   = true;
	bool                           showThreadView  = true;
	bool                           showRuntimeView = true;
	UI::TimelineOptions            timelineOptions {};
	std::vector<UI::TimelineEntry> cpu0Timeline;
	cpu0Timeline.reserve(100);
	for (std::size_t i = 0; i < 100; ++i)
		cpu0Timeline.emplace_back(UI::TimelineEntry { i, i + 1, nullptr, 32 << 16 | 32 << 8 | 32 });

	bool                    supportsIndividualCoreUsages  = Profiler::GetRuntimeAbilities().hasFlag(Profiler::RuntimeAbilities::ProcessIndividualCoreUsages);
	bool                    supportsIndividualDriveUsages = Profiler::GetRuntimeAbilities().hasFlag(Profiler::RuntimeAbilities::ProcessIndividualDriveUsages);
	std::size_t             coreCount                     = supportsIndividualCoreUsages ? Profiler::GetTotalCoreCount() : 1;
	std::size_t             driveCount                    = supportsIndividualDriveUsages ? Profiler::GetDriveCount() : 1;
	Profiler::CoreCounter*  coreCounters                  = new Profiler::CoreCounter[coreCount];
	Profiler::CoreCounter*  processCoreCounters           = new Profiler::CoreCounter[coreCount];
	Profiler::DriveInfo*    driveInfos                    = new Profiler::DriveInfo[driveCount];
	Profiler::DriveCounter* driveCounters                 = new Profiler::DriveCounter[driveCount];
	Profiler::DriveCounter* processDriveCounters          = new Profiler::DriveCounter[driveCount];
	if (supportsIndividualDriveUsages)
		Profiler::GetDriveInfos(driveCount, driveInfos);

	using Clock            = std::chrono::high_resolution_clock;
	auto previousFrameTime = Clock::now();
	while (!glfwWindowShouldClose(window))
	{
		auto   currentFrameTime = Clock::now();
		double deltaTime        = std::chrono::duration_cast<std::chrono::duration<double>>(currentFrameTime - previousFrameTime).count();
		double invDeltaTime     = 1.0 / deltaTime;
		previousFrameTime       = currentFrameTime;
		glfwPollEvents();

		UI::TimelineStateUpdate(&timelineOptions, deltaTime);

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
				if (ImGui::MenuItem("Show Cores View"))
					showCoresView = true;
				if (ImGui::MenuItem("Show Thread View"))
					showThreadView = true;
				if (ImGui::MenuItem("Show Runtime View"))
					showRuntimeView = true;
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showCoresView)
		{
			ImGui::Begin("Cores View##CoresView", &showCoresView);

			UI::DefaultTimelineStyle(&timelineOptions);

			UI::TimelineZoomingInWindow(&timelineOptions, invDeltaTime);
			UI::TimelineOffsettingInWindow(&timelineOptions, invDeltaTime);
			UI::DrawTimescale(&timelineOptions);
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());
			UI::DrawTimeline(&timelineOptions, cpu0Timeline.size(), cpu0Timeline.data());

			ImGui::End();
		}

		if (showThreadView)
		{
			ImGui::Begin("Thread View##ThreadView", &showThreadView);

			UI::DefaultTimelineStyle(&timelineOptions);

			UI::TimelineZoomingInWindow(&timelineOptions, invDeltaTime);
			UI::TimelineOffsettingInWindow(&timelineOptions, invDeltaTime);
			UI::DrawTimescale(&timelineOptions);
			UI::DrawTimeline(&timelineOptions, 0, nullptr);

			ImGui::End();
		}

		if (showRuntimeView)
		{
			ImGui::Begin("Runtime View##RuntimeView", &showRuntimeView);

			Profiler::Process curProcess = Profiler::GetCurrentProcess();

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("CPU Usage##CPUUsage"))
			{
				ImGui::Text("Process / Total");

				double totalProcessUsage = 0.0;
				double totalTotalUsage   = 0.0;

				Profiler::GetCoreUsages(0, coreCount, coreCounters);
				Profiler::GetCoreUsages(curProcess, coreCount, processCoreCounters);
				if (supportsIndividualCoreUsages)
				{
					for (std::size_t i = 0; i < coreCount; ++i)
					{
						totalProcessUsage += processCoreCounters[i].Usage;
						totalTotalUsage   += coreCounters[i].Usage;
					}
					totalProcessUsage /= coreCount;
					totalTotalUsage   /= coreCount;
				}
				else
				{
					totalProcessUsage = processCoreCounters[0].Usage;
					totalTotalUsage   = coreCounters[0].Usage;
				}
				ImGui::Text("Total: %.1lf%% / %.1lf%%", totalProcessUsage * 100.0, totalTotalUsage * 100.0);
				if (supportsIndividualCoreUsages)
					for (std::size_t i = 0; i < coreCount; ++i)
						ImGui::Text("Core %lld: %.1lf%% / %.1lf%%", processCoreCounters[i].Usage * 100.0, coreCounters[i].Usage * 100.0);

				ImGui::TreePop();
			}

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Memory Usage##MemoryUsage"))
			{
				ImGui::Text("Process / Total");

				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNode("Total##Total"))
				{
					Profiler::MemoryCounters counters {};
					Profiler::GetMemoryUsage(0, counters);
					ImGui::Text("Physical: %lld / %lld (%.1lf%%)", counters.CurrentWorkingSet, counters.PeakWorkingSet, static_cast<double>(counters.CurrentWorkingSet) / static_cast<double>(counters.PeakWorkingSet) * 100.0);
					ImGui::Text("Virtual: %lld", counters.Private);

					ImGui::TreePop();
				}

				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNode("Process##Process"))
				{
					Profiler::MemoryCounters counters {};
					Profiler::GetMemoryUsage(curProcess, counters);
					ImGui::Text("Physical: %lld", counters.CurrentWorkingSet);
					ImGui::Text("Virtual: %lld", counters.Private);
					ImGui::Text("Paged: %lld", counters.CurrentPagedPool);
					ImGui::Text("Non Paged: %lld", counters.CurrentNonPagedPool);

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Drive Usage##DriveUsage"))
			{
				if (supportsIndividualDriveUsages)
				{
					std::size_t newDriveCount = Profiler::GetDriveCount();
					if (newDriveCount != driveCount)
					{
						delete[] driveInfos;
						delete[] driveCounters;
						delete[] processDriveCounters;
						driveInfos           = new Profiler::DriveInfo[driveCount];
						driveCounters        = new Profiler::DriveCounter[driveCount];
						processDriveCounters = new Profiler::DriveCounter[driveCount];
						Profiler::GetDriveInfos(driveCount, driveInfos);
					}
				}

				Profiler::GetDriveUsages(0, driveCount, driveCounters);
				Profiler::GetDriveUsages(curProcess, driveCount, processDriveCounters);
				ImGui::Text("Process / Total");
				if (supportsIndividualDriveUsages)
				{
					for (std::size_t i = 0; i < driveCount; ++i)
					{
						auto& driveInfo = driveInfos[i];
						if (ImGui::TreeNode("Drive %s##Drive%s", driveInfo.Name.c_str(), driveInfo.Name.c_str()))
						{
							ImGui::Text("Size: %lld", driveInfo.Size);
							ImGui::Text("Read: %lld / %lld", processDriveCounters[i].BytesRead, driveCounters[i].BytesRead);
							ImGui::Text("Written: %lld / %lld", processDriveCounters[i].BytesWritten, driveCounters[i].BytesWritten);
							ImGui::Text("Usage: %.1lf%% / %.1lf%%", processDriveCounters[i].Usage, driveCounters[i].Usage);

							ImGui::TreePop();
						}
					}
				}
				else
				{
					ImGui::Text("Read: %lld / %lld", processDriveCounters[0].BytesRead, driveCounters[0].BytesRead);
					ImGui::Text("Written: %lld / %lld", processDriveCounters[0].BytesWritten, driveCounters[0].BytesWritten);
					ImGui::Text("Usage: %.1lf%% / %.1lf%%", processDriveCounters[0].Usage, driveCounters[0].Usage);
				}

				ImGui::TreePop();
			}

			ImGui::End();
		}

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

	delete[] processDriveCounters;
	delete[] driveCounters;
	delete[] driveInfos;
	delete[] processCoreCounters;
	delete[] coreCounters;

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