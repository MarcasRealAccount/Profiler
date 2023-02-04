#include "Profiler/Utils/UTF.h"

#include <cstring>
#include <cwchar>

namespace Profiler::Utils::UTF
{
	std::size_t CToWStream(const char* cstr, std::size_t* cstrLength, wchar_t* wstr, std::size_t* wstrLength)
	{
		if (!cstr || !cstrLength || !wstrLength) return 0;

		const char*    cur = cstr;
		std::mbstate_t state {};
		if (!wstr)
		{
			*wstrLength = std::mbsrtowcs(nullptr, &cur, 0, &state);
			return *wstrLength;
		}

		auto readLen = std::mbsrtowcs(wstr, &cur, *wstrLength, &state);
		if (cur == nullptr)
			*cstrLength = 0; // TODO(MarcasRealAccount): Fix idk?
		else
			*cstrLength = cur - cstr;
		*wstrLength = readLen;
		return *wstrLength;
	}

	std::size_t CToU8Stream(const char* cstr, std::size_t* cstrLength, char8_t* u8str, std::size_t* u8strLength)
	{
		if (!cstr || !cstrLength || !u8strLength) return 0;

		if (!u8str)
		{
			*u8strLength = *cstrLength;
		}
		else
		{
			std::size_t toCopy = std::min(*cstrLength, *u8strLength);
			std::memcpy(u8str, cstr, toCopy);
			*cstrLength  = toCopy;
			*u8strLength = toCopy;
		}
		return *cstrLength; // TODO(MarcasRealAccount): Return number of unicode codepoints copied
	}

	std::size_t CToU16Stream(const char* cstr, std::size_t* cstrLength, char16_t* u16str, std::size_t* u16strLength);
	std::size_t CToU32Stream(const char* cstr, std::size_t* cstrLength, char32_t* u32str, std::size_t* u32strLength);

	std::size_t WToCStream(const wchar_t* wstr, std::size_t* wstrLength, char* cstr, std::size_t* cstrLength)
	{
		if (!wstr || !wstrLength || !cstrLength) return 0;

		const wchar_t* cur = wstr;
		std::mbstate_t state {};
		if (!cstr)
		{
			*cstrLength = std::wcsrtombs(nullptr, &cur, 0, &state);
			return *cstrLength;
		}

		auto readLen = std::wcsrtombs(cstr, &cur, *cstrLength, &state);
		if (cur == nullptr)
			*wstrLength = 0; // TODO(MarcasRealAccount): Fix idk?
		else
			*wstrLength = cur - wstr;
		*cstrLength = readLen;
		return *cstrLength;
	}

	std::size_t WToU8Stream(const wchar_t* wstr, std::size_t* wstrLength, char8_t* u8str, std::size_t* u8strLength);
	std::size_t WToU16Stream(const wchar_t* wstr, std::size_t* wstrLength, char16_t* u16str, std::size_t* u16strLength);
	std::size_t WToU32Stream(const wchar_t* wstr, std::size_t* wstrLength, char32_t* u32str, std::size_t* u32strLength);

	std::size_t U8ToCStream(const char8_t* u8str, std::size_t* u8strLength, char* cstr, std::size_t* cstrLength)
	{
		if (!u8str || !u8strLength || !cstrLength) return 0;

		if (!cstr)
		{
			*cstrLength = *u8strLength;
		}
		else
		{
			std::size_t toCopy = std::min(*u8strLength, *cstrLength);
			std::memcpy(cstr, u8str, toCopy);
			*u8strLength = toCopy;
			*cstrLength  = toCopy;
		}
		return *u8strLength; // TODO(MarcasRealAccount): Return number of unicode codepoints copied
	}

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
} // namespace Profiler::Utils::UTF