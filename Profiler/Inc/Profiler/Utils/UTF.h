#pragma once

#include <cstddef>
#include <cstdint>

#include <string>

namespace Profiler::Utils::UTF
{
	std::size_t CToWStream(const char* cstr, std::size_t* cstrLength, wchar_t* wstr, std::size_t* wstrLength);
	std::size_t CToU8Stream(const char* cstr, std::size_t* cstrLength, char8_t* u8str, std::size_t* u8strLength);
	std::size_t CToU16Stream(const char* cstr, std::size_t* cstrLength, char16_t* u16str, std::size_t* u16strLength);
	std::size_t CToU32Stream(const char* cstr, std::size_t* cstrLength, char32_t* u32str, std::size_t* u32strLength);

	std::size_t WToCStream(const wchar_t* wstr, std::size_t* wstrLength, char* cstr, std::size_t* cstrLength);
	std::size_t WToU8Stream(const wchar_t* wstr, std::size_t* wstrLength, char8_t* u8str, std::size_t* u8strLength);
	std::size_t WToU16Stream(const wchar_t* wstr, std::size_t* wstrLength, char16_t* u16str, std::size_t* u16strLength);
	std::size_t WToU32Stream(const wchar_t* wstr, std::size_t* wstrLength, char32_t* u32str, std::size_t* u32strLength);

	std::size_t U8ToCStream(const char8_t* u8str, std::size_t* u8strLength, char* cstr, std::size_t* cstrLength);
	std::size_t U8ToWStream(const char8_t* u8str, std::size_t* u8strLength, wchar_t* wstr, std::size_t* wstrLength);
	std::size_t U8ToU16Stream(const char8_t* u8str, std::size_t* u8strLength, char16_t* u16str, std::size_t* u16strLength);
	std::size_t U8ToU32Stream(const char8_t* u8str, std::size_t* u8strLength, char32_t* u32str, std::size_t* u32strLength);

	std::size_t U16ToCStream(const char16_t* u16str, std::size_t* u16strLength, char* cstr, std::size_t* cstrLength);
	std::size_t U16ToWStream(const char16_t* u16str, std::size_t* u16strLength, wchar_t* wstr, std::size_t* wstrLength);
	std::size_t U16ToU8Stream(const char16_t* u16str, std::size_t* u16strLength, char8_t* u8str, std::size_t* u8strLength);
	std::size_t U16ToU32Stream(const char16_t* u16str, std::size_t* u16strLength, char32_t* u32str, std::size_t* u32strLength);

	std::size_t U32ToCStream(const char32_t* u32str, std::size_t* u32strLength, char* cstr, std::size_t* cstrLength);
	std::size_t U32ToWStream(const char32_t* u32str, std::size_t* u32strLength, wchar_t* wstr, std::size_t* wstrLength);
	std::size_t U32ToU8Stream(const char32_t* u32str, std::size_t* u32strLength, char8_t* u8str, std::size_t* u8strLength);
	std::size_t U32ToU16Stream(const char32_t* u32str, std::size_t* u32strLength, char16_t* u16str, std::size_t* u16strLength);

	inline std::wstring CToW(std::string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		CToWStream(str.data(), &strlen, nullptr, &requiredSize);
		std::wstring out(requiredSize, '\0');
		CToWStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u8string CToU8(std::string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		CToU8Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u8string out(requiredSize, '\0');
		CToU8Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u16string CToU16(std::string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		CToU16Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u16string out(requiredSize, '\0');
		CToU16Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u32string CToU32(std::string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		CToU32Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u32string out(requiredSize, '\0');
		CToU32Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::string WToC(std::wstring_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		WToCStream(str.data(), &strlen, nullptr, &requiredSize);
		std::string out(requiredSize, '\0');
		WToCStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u8string WToU8(std::wstring_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		WToU8Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u8string out(requiredSize, '\0');
		WToU8Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u16string WToU16(std::wstring_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		WToU16Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u16string out(requiredSize, '\0');
		WToU16Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u32string WToU32(std::wstring_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		WToU32Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u32string out(requiredSize, '\0');
		WToU32Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::string U8ToC(std::u8string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U8ToCStream(str.data(), &strlen, nullptr, &requiredSize);
		std::string out(requiredSize, '\0');
		U8ToCStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::wstring U8ToW(std::u8string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U8ToWStream(str.data(), &strlen, nullptr, &requiredSize);
		std::wstring out(requiredSize, '\0');
		U8ToWStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u16string U8ToU16(std::u8string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U8ToU16Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u16string out(requiredSize, '\0');
		U8ToU16Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u32string U8ToU32(std::u8string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U8ToU32Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u32string out(requiredSize, '\0');
		U8ToU32Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::string U16ToC(std::u16string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U16ToCStream(str.data(), &strlen, nullptr, &requiredSize);
		std::string out(requiredSize, '\0');
		U16ToCStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::wstring U16ToW(std::u16string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U16ToWStream(str.data(), &strlen, nullptr, &requiredSize);
		std::wstring out(requiredSize, '\0');
		U16ToWStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u8string U16ToU8(std::u16string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U16ToU8Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u8string out(requiredSize, '\0');
		U16ToU8Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u32string U16ToU32(std::u16string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U16ToU32Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u32string out(requiredSize, '\0');
		U16ToU32Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::string U32ToC(std::u32string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U32ToCStream(str.data(), &strlen, nullptr, &requiredSize);
		std::string out(requiredSize, '\0');
		U32ToCStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::wstring U32ToW(std::u32string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U32ToWStream(str.data(), &strlen, nullptr, &requiredSize);
		std::wstring out(requiredSize, '\0');
		U32ToWStream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u8string U32ToU8(std::u32string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U32ToU8Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u8string out(requiredSize, '\0');
		U32ToU8Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}

	inline std::u16string U32ToU16(std::u32string_view str)
	{
		std::size_t strlen       = str.size();
		std::size_t requiredSize = 0;
		U32ToU16Stream(str.data(), &strlen, nullptr, &requiredSize);
		std::u16string out(requiredSize, '\0');
		U32ToU16Stream(str.data(), &strlen, out.data(), &requiredSize);
		return out;
	}
} // namespace Profiler::Utils::UTF