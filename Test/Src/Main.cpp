#include "Utils/Core.h"

#include <Profiler/Profiler.h>

void normalFunc()
{
	auto funcRAII = Profiler::Function(&normalFunc);
}

void funcWithArg(int value)
{
	auto funcRAII = Profiler::Function(&funcWithArg);
	Profiler::IntArg(0, value);
}

void funcWithMultipleArgs(int a, int b, int c, int d)
{
	auto funcRAII = Profiler::Function(&funcWithMultipleArgs);
	Profiler::IntArg(0, a);
	Profiler::IntArg(1, b);
	Profiler::IntArg(2, c);
	Profiler::IntArg(3, d);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	Profiler::Init();
	Profiler::BeginCapturing();

	normalFunc();
	funcWithArg(10);

	Profiler::Frame();

	normalFunc();
	funcWithArg(69);
	funcWithMultipleArgs(1, 2, 3, 4);

	Profiler::EndCapturing();
	Profiler::WriteCaptures();

	Profiler::Deinit();
}