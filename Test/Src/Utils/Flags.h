#pragma once

#include <concepts>
#include <utility>

namespace Utils
{
	namespace Detail
	{
		template <class T>
		concept ShouldNotRef = std::is_pod_v<T> && sizeof(T) < 16;
		template <class T>
		concept ShouldRef = !(std::is_pod_v<T> && sizeof(T) < 16);

		template <class T>
		concept HasBitwiseAnd = requires(T t) { { t & t } -> std::convertible_to<T>; };
		template <class T>
		concept HasBitwiseOr = requires(T t) { { t | t } -> std::convertible_to<T>; };
		template <class T>
		concept HasBitwiseNot = requires(T t) { { ~t } -> std::convertible_to<T>; };
		template <class T>
		concept HasLeftShift = requires(T t, std::size_t n) { { t << n } -> std::convertible_to<T>; };
		template <class T>
		concept HasRightShift = requires(T t, std::size_t n) { { t >> n } -> std::convertible_to<T>; };
		template <class T>
		concept HasEquals = requires(T t) { { t == t } -> std::convertible_to<bool>; };
		template <class T>
		concept HasLessThan = requires(T t) { { t < t } -> std::convertible_to<bool>; };
		template <class T>
		concept HasGreaterThan = requires(T t) { { t > t } -> std::convertible_to<bool>; };

		template <class T>
		concept Flaggable = HasBitwiseOr<T> && HasBitwiseAnd<T> && HasBitwiseNot<T> && HasLeftShift<T> && HasRightShift<T> &&
							HasEquals<T> && HasLessThan<T> && HasGreaterThan<T>;
	} // namespace Detail

	template <Detail::Flaggable T>
	struct Flags
	{
	public:
		T Value;

	public:
		constexpr Flags(std::convertible_to<T> auto&& value) noexcept
			: Value(static_cast<T>(value)) {}

		constexpr Flags& operator=(std::convertible_to<T> auto&& value) noexcept { Value = static_cast<T>(value); }

		constexpr bool hasFlag(std::convertible_to<Flags> auto&& flags) const { return (Value & flags) == flags; }

		// clang-format off
		constexpr operator T() requires Detail::ShouldNotRef<T> { return Value; }
		constexpr operator T&() requires Detail::ShouldRef<T> { return Value; }
		constexpr operator const T&() const requires Detail::ShouldRef<T> { return Value; }

		constexpr Flags& operator|=(std::convertible_to<Flags> auto&& flags) { Value = Value | static_cast<T>(flags.Value); return *this; }
		constexpr Flags& operator&=(std::convertible_to<Flags> auto&& flags) { Value = Value & static_cast<T>(flags.Value); return *this; }
		constexpr Flags& operator^=(std::convertible_to<Flags> auto&& flags) { Value = Value ^ static_cast<T>(flags.Value); return *this; }
		constexpr Flags& operator>>=(std::size_t count) { Value = Value >> count; return *this; }
		constexpr Flags& operator<<=(std::size_t count) { Value = Value << count; return *this; }
		constexpr friend Flags operator|(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return { lhs.Value | static_cast<T>(rhs.Value) }; }
		constexpr friend Flags operator&(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return { lhs.Value & static_cast<T>(rhs.Value) }; }
		constexpr friend Flags operator^(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return { lhs.Value ^ static_cast<T>(rhs.Value) }; }
		constexpr friend Flags operator~(const Flags& flags) { return { ~flags.Value }; }
		constexpr friend Flags operator>>=(const Flags& flags, std::size_t count) { return { flags.Value >> count }; }
		constexpr friend Flags operator<<=(const Flags& flags, std::size_t count) { return { flags.Value << count }; }

		constexpr friend bool operator==(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return lhs.Value == static_cast<T>(rhs.Value); }
		constexpr friend bool operator<(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return lhs.Value < static_cast<T>(rhs.Value); }
		constexpr friend bool operator>(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return lhs.Value > static_cast<T>(rhs.Value); }
		constexpr friend bool operator!=(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return !(lhs == rhs); }
		constexpr friend bool operator>=(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return !(lhs < rhs); }
		constexpr friend bool operator<=(const Flags& lhs, std::convertible_to<Flags> auto&& rhs) { return !(lhs > rhs); }

		// clang-format on
	};
} // namespace Utils