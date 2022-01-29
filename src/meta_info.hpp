#pragma once

// #include <MetaStuff/Meta.h>
// #define MINIMA_META_INFO(Type) template<> inline auto meta::registerMembers<Type>()
// #define MINIMA_META_INFO(Type) template<> inline auto meta::registerMembers<Type>()

#define SERIALIZE(object, member) json[#member] = object.member;
#define DESERIALIZE(object, member) get_if_value_exists(json, #member, object.member);

enum MetaMemberFlagBits : int
{
	META_MEMBER_FLAGS_COLOR 		= 1,
	META_MEMBER_FLAGS_COLOR_HDR 	= 2,
};

/*
Use like:

struct Ab
{
	int a;
	float b;
};

MINIMA_META_INFO(Ab)
{
	return members(
		member("a", &Ab::a),
		member("b", &Ab::b, META_MEMBER_FLAGS)
	);
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gui.hpp"

/*
template<typename T>
bool meta_gui_default_edit(T & t)
{
	bool dirty = false;

	meta::doForAllMembers<T>([&dirty, &t](auto& member)
	{
		if (member.canGetRef())
		{
			dirty = gui::edit(member.getName(), member.getRef(t), member.flags) || dirty;
		}
	});

	if (dirty)
	{
		gui::validate_edit(t);
	}

	return dirty;
}
*/

namespace gui
{
	/*
	One of the following can be defined as free function overload for any given struct.
	Actually, multiple can, but that kinda makes no sense, as only one edit will be called
	(unless they are chained as in here, which makes no sense) and work done in validate_edit
	can also be done in explicit edit function directly, and it would need to be called explicitly
	from there. So, just pick one.

	validate_edit: Use this when default edit is sufficient, but some operations need to be done
	anytime object is edited.

	edit(item): Use this for "complex" types with multiple fields. They are intended to be used inside
	a container helper, like "collapsing_box" etc.

	edit(label, item): Use these for simple single line widgets, where there is a label
	and ImGui builtin or custom widget

	edit(label, item, flags): Same as previous, but with flags. Use with "overloaded"
	data types, like float3 which sometimes need to be edited as color and sometimes vector.

	How this works:
	1. any explicit edit function is free to do anything
	2. for types that there are no explicit overload, one of below will be used.
	3. the default meta_gui_default_edit will call the overload with label, item and flags
		if there is no explicit overload, the templated default one here will be used, which
		calls the second overload. If no explicit version of that exists, default here will be used
		and so on and forthwards.
	*/

	template <typename T>
	void validate_edit(T & item) {}

	template<typename T>
	bool edit(T & item)
	{

		Text("No Editor!");
		return false;
		// return meta_gui_default_edit(item);
	}

	template<typename T>
	bool edit(char const * label, T & item)
	{
		bool edited = false;
		if (TreeNode(label))
		{
			edited = edit(item);
			TreePop();
			Separator();
		}
		return edited;
	}

	template<typename T>
	bool edit(char const * label, T & item, int flags)
	{
		return edit(label, item);
	}
}
