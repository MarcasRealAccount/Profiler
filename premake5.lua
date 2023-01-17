workspace("Profiler")
	common:setConfigsAndPlatforms()
	common:addCoreDefines()

	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("On")
	flags("MultiProcessorCompile")

	startproject("Test")
	project("Profiler")
		location("Profiler/")
		warnings("Extra")

		common:outDirs(true)
		kind("StaticLib")

		includedirs({ "%{prj.location}/Inc/" })
		files({
			"%{prj.location}/Inc/**",
			"%{prj.location}/Src/**"
		})
		removefiles({ "*.DS_Store" })

		pkgdeps({ "fmt" })

		common:addActions()

	project("Test")
		location("Test/")
		warnings("Extra")

		common:outDirs()
		common:debugDir()

		kind("ConsoleApp")

		includedirs({ "%{prj.location}/Src/" })
		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

		links({ "Profiler" })
		externalincludedirs({ "Profiler/Inc/" })

		common:addActions()