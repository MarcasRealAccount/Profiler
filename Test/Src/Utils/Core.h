#pragma once

#include "Flags.h"

#define BUILD_CONFIG_UNKNOWN 0
#define BUILD_CONFIG_DEBUG   1
#define BUILD_CONFIG_RELEASE 2
#define BUILD_CONFIG_DIST    3

#define BUILD_SYSTEM_UNKNOWN 0
#define BUILD_SYSTEM_WINDOWS 1
#define BUILD_SYSTEM_MACOSX  2
#define BUILD_SYSTEM_LINUX   3

#define BUILD_TOOLSET_UNKNOWN 0
#define BUILD_TOOLSET_MSVC    1
#define BUILD_TOOLSET_CLANG   2
#define BUILD_TOOLSET_GCC     3

#define BUILD_PLATFORM_UNKNOWN 0
#define BUILD_PLATFORM_AMD64   1

#define BUILD_IS_CONFIG_DEBUG ((BUILD_CONFIG == BUILD_CONFIG_DEBUG) || (BUILD_CONFIG == BUILD_CONFIG_RELEASE))
#define BUILD_IS_CONFIG_DIST  ((BUILD_CONFIG == BUILD_CONFIG_RELEASE) || (BUILD_CONFIG == BUILD_CONFIG_DIST))

#define BUILD_IS_SYSTEM_WINDOWS (BUILD_SYSTEM == BUILD_SYSTEM_WINDOWS)
#define BUILD_IS_SYSTEM_MACOSX  (BUILD_SYSTEM == BUILD_SYSTEM_MACOSX)
#define BUILD_IS_SYSTEM_LINUX   (BUILD_SYSTEM == BUILD_SYSTEM_LINUX)
#define BUILD_IS_SYSTEM_UNIX    (BUILD_IS_SYSTEM_MACOSX || BUILD_IS_SYSTEM_LINUX)

#define BUILD_IS_TOOLSET_MSVC  (BUILD_TOOLSET == BUILD_TOOLSET_MSVC)
#define BUILD_IS_TOOLSET_CLANG (BUILD_TOOLSET == BUILD_TOOLSET_CLANG)
#define BUILD_IS_TOOLSET_GCC   (BUILD_TOOLSET == BUILD_TOOLSET_GCC)

#define BUILD_IS_PLATFORM_AMD64 (BUILD_PLATFORM == BUILD_PLATFORM_AMD64)

namespace Core
{
	using EBuildConfig   = Utils::Flags<std::uint16_t>;
	using EBuildSystem   = Utils::Flags<std::uint16_t>;
	using EBuildToolset  = Utils::Flags<std::uint16_t>;
	using EBuildPlatform = Utils::Flags<std::uint16_t>;

	namespace BuildConfig
	{
		static constexpr EBuildConfig Unknown = 0;
		static constexpr EBuildConfig Debug   = 1;
		static constexpr EBuildConfig Dist    = 2;
	} // namespace BuildConfig

	namespace BuildSystem
	{
		static constexpr EBuildSystem Unknown = 0;
		static constexpr EBuildSystem Windows = 1;
		static constexpr EBuildSystem Unix    = 2;
		static constexpr EBuildSystem MacOSX  = 4;
		static constexpr EBuildSystem Linux   = 8;
	} // namespace BuildSystem

	namespace BuildToolset
	{
		static constexpr EBuildToolset Unknown = 0;
		static constexpr EBuildToolset MSVC    = 1;
		static constexpr EBuildToolset Clang   = 2;
		static constexpr EBuildToolset GCC     = 4;
	} // namespace BuildToolset

	namespace BuildPlatform
	{
		static constexpr EBuildPlatform Unknown = 0;
		static constexpr EBuildPlatform AMD64   = 1;
	} // namespace BuildPlatform

	constexpr EBuildConfig GetBuildConfig()
	{
#if BUILD_CONFIG == BUILD_CONFIG_DEBUG
		return BuildConfig::Debug;
#elif BUILD_CONFIG == BUILD_CONFIG_RELEASE
		return BuildConfig::Debug | BuildConfig::Dist;
#elif BUILD_CONFIG == BUILD_CONFIG_DIST
		return BuildConfig::Dist;
#else
		return BuildConfig::Unknown;
#endif
	}

	constexpr EBuildSystem GetBuildSystem()
	{
#if BUILD_SYSTEM == BUILD_SYSTEM_WINDOWS
		return BuildSystem::Windows;
#elif BUILD_SYSTEM == BUILD_SYSTEM_MACOSX
		return BuildSystem::Unix | BuildSystem::MacOSX;
#elif BUILD_SYSTEM == BUILD_SYSTEM_LINUX
		return BuildSystem::Unix | BuildSystem::Linux;
#else
		return BuildSystem::Unknown;
#endif
	}

	constexpr EBuildToolset GetBuildToolset()
	{
#if BUILD_TOOLSET == BUILD_TOOLSET_MSVC
		return BuildToolset::MSVC;
#elif BUILD_TOOLSET == BUILD_TOOLSET_CLANG
		return BuildToolset::Clang;
#elif BUILD_TOOLSET == BUILD_TOOLSET_GCC
		return BuildToolset::GCC;
#else
		return BuildToolset::Unknown;
#endif
	}

	constexpr EBuildPlatform GetBuildPlatform()
	{
#if BUILD_PLATFORM == BUILD_PLATFORM_AMD64
		return BuildPlatform::AMD64;
#else
		return BuildPlatform::Unknown;
#endif
	}

	static constexpr EBuildConfig c_Config        = GetBuildConfig();
	static constexpr bool         c_IsConfigDebug = c_Config.hasFlag(BuildConfig::Debug);
	static constexpr bool         c_IsConfigDist  = c_Config.hasFlag(BuildConfig::Dist);

	static constexpr EBuildSystem c_System          = GetBuildSystem();
	static constexpr bool         c_IsSystemWindows = c_System.hasFlag(BuildSystem::Windows);
	static constexpr bool         c_IsSystemUnix    = c_System.hasFlag(BuildSystem::Unix);
	static constexpr bool         c_IsSystemMacOSX  = c_System.hasFlag(BuildSystem::MacOSX);
	static constexpr bool         c_IsSystemLinux   = c_System.hasFlag(BuildSystem::Linux);

	static constexpr EBuildToolset c_Toolset        = GetBuildToolset();
	static constexpr bool          c_IsToolsetMSVC  = c_Toolset.hasFlag(BuildToolset::MSVC);
	static constexpr bool          c_IsToolsetClang = c_Toolset.hasFlag(BuildToolset::Clang);
	static constexpr bool          c_IsToolsetGCC   = c_Toolset.hasFlag(BuildToolset::GCC);

	static constexpr EBuildPlatform c_Platform        = GetBuildPlatform();
	static constexpr bool           c_IsPlatformAMD64 = c_Platform.hasFlag(BuildPlatform::AMD64);
} // namespace Core