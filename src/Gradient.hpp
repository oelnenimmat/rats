#pragma once

#include "vectors.hpp"
#include "gui.hpp"
#include "math.hpp"

struct Gradient
{
	struct Point
	{
		float4 color;
		float position;
	};

	Gradient() : point_count(2)
	{	
		points[0] = { float4(0,0,0,1), 0 };
		points[1] = { float4(1,1,1,1), 1 };
	}
	
	static constexpr int 	max_points = 16;
	int 					point_count;
	Point 					points[max_points];

	inline float4 			evaluate(float position) const;

	// Probably for editor only. May be used freely though.
	inline int 		add_point(float position, ImColor const & color);
	inline void		remove_point(int index);
	inline void 	set_point_color (int index, float4 new_color);
	inline int 		set_point_position(int index, float new_position);

	inline int 		sort_down(int index);
	inline int 		sort_up(int index);
};


inline SERIALIZE_STRUCT(Gradient::Point const & point)
{
	serializer.write("color", point.color);
	serializer.write("position", point.position);
}

inline DESERIALIZE_STRUCT(Gradient::Point & point)
{
	serializer.read("color", point.color);
	serializer.read("position", point.position);
}

inline SERIALIZE_STRUCT(Gradient const & gradient)
{
	serializer.write_array("points", gradient.point_count, gradient.points);
}

inline DESERIALIZE_STRUCT(Gradient & gradient)
{
	serializer.read_array("points", gradient.point_count, gradient.points);
}

int Gradient::add_point(float position, ImColor const & color)
{
	if (point_count >= max_points)
	{
		return -1;
	}

	position = rats::clamp(position, 0.0f, 1.0f);
	Point new_point;
	new_point.color.x = color.Value.x;
	new_point.color.y = color.Value.y;
	new_point.color.z = color.Value.z;
	new_point.color.w = color.Value.w;
	new_point.position = position;
	
	int index = point_count;
	point_count += 1;
	points[index] = new_point;
	index = sort_down(index);  

	return index;
}

void Gradient::remove_point(int index)
{
	// Swap removed point to end one by one, and decrement the counter
	for (int i = index; i < point_count; i++)
	{
		std::swap(points[i], points[i + 1]);
	}
	point_count -= 1;
}

int Gradient::set_point_position(int index, float new_position)
{
	new_position = rats::clamp(new_position, 0.0f, 1.0f);
	float delta = new_position - points[index].position;

	points[index].position = new_position;

	index = delta < 0 ? sort_down(index) : sort_up(index);
	return index;
}

void Gradient::set_point_color (int index, float4 new_color)
{
	points[index].color = new_color;
}

float4 Gradient::evaluate(float position) const
{
	if (point_count == 0)
	{
		return float4(0,0,0,0);
	}

	// if points count == 1, then either (position < points[0].position) 
	// or (position > points[points_count - 1].position) is true
	if (position < points[0].position)
	{
		return points[0].color;
	}

	if (position > points[point_count - 1].position)
	{
		return points[point_count - 1].color;
	}

	int upper_index = 1;
	while (position > points[upper_index].position)
	{
		upper_index += 1;
	}

	Point lower = points[upper_index - 1];
	Point upper = points[upper_index];

	float t = (position - lower.position) / (upper.position - lower.position);

	return lerp(lower.color, upper.color, t);
}

int Gradient::sort_down(int index)
{
	while(index > 0 && (points[index - 1].position > points[index].position))
	{
		std::swap(points[index -1], points[index]);
		index -= 1;
	}
	return index;
}

int Gradient::sort_up(int index)
{
	while(index < (point_count - 1) && (points[index + 1].position < points[index].position))
	{
		std::swap(points[index +1], points[index]);
		index += 1;
	}
	return index;
}

struct ImGradientWrap : IImGradient
{
	Gradient & gradient;

	ImGradientWrap (Gradient & gradient) : gradient(gradient) {}

	int get_point_count () override
	{
		return gradient.point_count;
	}
	
	ImGradientPoint get_point (int index) override
	{ 
		Gradient::Point point = gradient.points[index];
		ImGradientPoint imgui_point;
		imgui_point.color.x = point.color.x;
		imgui_point.color.y = point.color.y;
		imgui_point.color.z = point.color.z;
		imgui_point.color.w = point.color.w;
		imgui_point.position = point.position;
		return imgui_point;
	}

	int add_point (float position, ImColor const & color) override
	{ 
		return gradient.add_point(position, color);
	}

	void remove_point (int index) override
	{
		gradient.remove_point(index);
	}

	void set_point_color (int index, ImVec4 new_color) override
	{
		gradient.set_point_color(index, float4(new_color.x, new_color.y, new_color.z, new_color.w));
	}
	
	int set_point_position (int index, float new_position) override
	{
		return gradient.set_point_position(index, new_position);
	}

	ImVec4 evaluate (float position) override
	{
		float4 color = gradient.evaluate(position);
		return ImVec4(color.r, color.g, color.b, color.a);
	}
};


namespace gui
{
	inline bool edit(char const * label, Gradient & gradient)
	{
		ImGradientWrap wrap = ImGradientWrap(gradient);

		return GradientEditor(label, wrap);
	}
}