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

	project("GUI")
		location("GUI/")
		warnings("Extra")

		common:outDirs()
		common:debugDir()

		filter("configurations:Debug")
			kind("ConsoleApp")
		filter("configurations:not Debug")
			kind("WindowedApp")
		filter({})

		includedirs({ "%{prj.location}/Src/" })
		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

		links({ "Profiler", "glad" })
		externalincludedirs({
			"Profiler/Inc/",
			"ThirdParty/glad/include/"
		})

		pkgdeps({ "fmt", "glfw", "imgui-docking" })

		filter("system:windows")
			links({ "opengl32.lib" })
		filter({})

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

	group("Dependencies")
	project("glad")
		location("ThirdParty/glad/")
		warnings("Off")
		
		common:outDirs(true)
		kind("StaticLib")
		
		includedirs({ "%{prj.location}/include/" })
		files({
			"%{prj.location}/include/**",
			"%{prj.location}/src/**"
		})
		removefiles({ "*.DS_Store" })