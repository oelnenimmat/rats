#pragma once

#include <MetaStuff/Meta.h>
#define MY_ENGINE_META_INFO(Type) template<> inline auto meta::registerMembers<Type>()

enum MetaMemberFlagBits : int
{
	META_MEMBER_FLAGS_COLOR 		= 1,
	META_MEMBER_FLAGS_COLOR_HDR 	= 2,
	META_MEMBER_FLAGS_NORMALIZED 	= 4,
};

/*
Use like:

struct Ab
{
	int a;
	float b;
};

MY_ENGINE_META_INFO(Ab)
{
	return members(
		member("a", &Ab::a),
		member("b", &Ab::b)
	);
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>

template <typename T>
std::ostream & meta_write_to_std_ostream(std::ostream & os, T const & t)
{
	std::stringstream ss;
	ss << "(";

	meta::doForAllMembers<T>([&ss, &t](auto& member) { ss << member.get(t) << ","; });

	// Erase last comma
	ss.seekp(-1, std::ios_base::cur);
	ss << ")";

	return os << ss.str();
}

#define MY_ENGINE_META_STD_OSTREAM_OPERATOR(type) inline std::ostream & operator << (std::ostream & os, type const & t) { return meta_write_to_std_ostream(os, t); } 

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gui.hpp"

template <typename T, typename TMember>
constexpr bool is_member_type = std::is_same_v<meta::get_member_type<TMember>, T>;

namespace meta
{
/*
	template <typename T, typename = void>
	struct has_edit_helper : std::false_type {};

	template <typename T>
	struct has_edit_helper<T, 
		std::void_t<decltype(edit(std::declval<T&>())) >> : std::true_type {};

	template <typename T>
	constexpr bool has_edit = has_edit_helper<T>::value;
*/
	// ------------------------------------------------------------------------

	template <typename T, typename = void>
	struct has_validate_edit_helper : std::false_type {};

	template <typename T>
	struct has_validate_edit_helper<T, 
		std::void_t<decltype(validate_edit(std::declval<T&>())) >> : std::true_type {};

	template <typename T>
	constexpr bool has_validate_edit = has_validate_edit_helper<T>::value;

}

template<typename T>
bool meta_gui_default_edit(T & t)
{
	using namespace gui;

	auto helper = gui_helper();

	meta::doForAllMembers<T>([&helper, &t](auto& member)
	{
		if (member.canGetRef())
		{
			if constexpr (is_member_type<float3, decltype(member)>)
			{
				switch(member.flags)
				{
					case META_MEMBER_FLAGS_COLOR:
						helper.edit(ColorEdit3(member.getName(), &member.getRef(t)));
						return;

					case META_MEMBER_FLAGS_COLOR_HDR:
						helper.edit(ColorEdit3(member.getName(), &member.getRef(t), ImGuiColorEditFlags_HDR));
						return;

					case META_MEMBER_FLAGS_NORMALIZED:
						if (helper.edit(DragFloat3(member.getName(), &member.getRef(t), 0.01f)))
						{
							member.getRef(t) = normalize(member.getRef(t));
						}
						return;

				}				
				helper.edit(edit(member.getName(), member.getRef(t)));
			}

			helper.edit(edit(member.getName(), member.getRef(t)));
		}
		/*
		else if (member.hasGetter())
		{
			default_display(member.getName(), member.getCopy(t));
		}
		*/
	});

	if constexpr (meta::has_validate_edit<T>)
	{
		if (helper.dirty)
		{
			validate_edit(t);
		}
	}

	return helper.dirty;
}

#define MY_ENGINE_META_DEFAULT_EDIT(type) namespace gui { \
	inline bool edit(type & t) { return meta_gui_default_edit<type>(t); }\
	inline bool edit(char const * label, type & t) { return meta_gui_default_edit<type>(t); }\
}

// namespace gui
// {
// 	// template<typename T, typename = std::enable_if_t<meta::isRegistered<T>()>>
// 	template <typename T>
// 	bool edit(T & t)
// 	{
// 		return meta_gui_default_edit(t);
// 	}

// 	// template<typename T, typename = std::enable_if_t<!meta::isRegistered<T>()>, typename = void>
// 	// bool edit(T & t)
// 	// {
// 	// 	Text("No editor defined");
// 	// 	return false;
// 	// }


// 	// template<typename T, typename = std::enable_if_t<meta::isRegistered<T>()>>
// 	template <typename T>
// 	bool edit(char const * label, T & t)
// 	{
// 		return meta_gui_default_edit(t);
// 	}

// 	// template<typename T, typename = std::enable_if_t<!meta::isRegistered<T>()>, typename = void>
// 	// bool edit(char const * label, T & t)
// 	// {
// 	// 	Text("No editor defined");
// 	// 	return false;
// 	// }

// 	// #define MY_ENGINE_META_DEFAULT_EDIT(type) t
// }

///////////////////////////////////////////////////////////////////////////////////////////////////