#include "Utils/Core.h"

#include <Profiler/ForLoop.h>
#include <Profiler/Frame.h>
#include <Profiler/Function.h>
#include <Profiler/Memory.h>
#include <Profiler/State.h>
#include <Profiler/Thread.h>

#include <array>
#include <thread>

[[nodiscard]] void* operator new(std::size_t count)
{
	void* ptr = std::malloc(count);
	Profiler::MemAlloc(ptr, count);
	return ptr;
}

void operator delete(void* ptr) noexcept
{
	Profiler::MemFree(ptr);
	std::free(ptr);
}

void normalFunc()
{
	auto _func = Profiler::HRFunction(&normalFunc);
}

void normalIFunc()
{
	auto _func = Profiler::Function(&normalIFunc);
}

void funcWithArg(int value)
{
	auto _func = Profiler::HRFunction(&funcWithArg);
	Profiler::IntArg(0, value);
}

void funcWithMultipleArgs(int a, int b, int c, int d)
{
	auto _func = Profiler::HRFunction(&funcWithMultipleArgs);
	Profiler::IntArg(0, a);
	Profiler::IntArg(1, b);
	Profiler::IntArg(2, c);
	Profiler::IntArg(3, d);
}

void loopedFunc(std::size_t count)
{
	auto _func = Profiler::Function(&loopedFunc);
	Profiler::IntArg(0, count);

	{
		auto _loop = Profiler::HRForLoop();
		for (std::size_t i = 0; i < count; ++i)
		{
			auto _iter = Profiler::HRForLoopIter(_loop, i);
			normalFunc();
			auto hehe = new int();
			*hehe     = 10;
			delete hehe;
		}
	}
}

void threadFunc()
{
	auto _thread = Profiler::Thread();

	normalFunc();
	normalIFunc();
	funcWithArg(239874);
	funcWithMultipleArgs(34987234, 239874283, 237984723, 2349782374);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	Profiler::Init();

	Profiler::WantCapturing(true, true);

	/*std::array<std::thread, 16> threads {};
	for (std::size_t i = 0; i < 16; ++i)
		threads[i] = std::thread(&threadFunc);*/

	normalFunc();
	normalIFunc();
	funcWithArg(10);

	Profiler::Frame();

	normalFunc();
	normalIFunc();
	funcWithArg(69);
	funcWithMultipleArgs(1, 2, 3, 4);
	loopedFunc(10);

	/*for (std::size_t i = 0; i < 16; ++i)
		threads[i].join();*/

	Profiler::WantCapturing(false, true);
	Profiler::WriteCaptures();

	Profiler::Deinit();
}