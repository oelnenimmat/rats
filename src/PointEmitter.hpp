// #pragma once

// struct PointData
// {
// 	float3 position;
// 	float3 color;
// 	float speed;
// };

// struct PointEmitter
// {
// 	float3 color;
// 	float3 color_variation;
// 	float point_speed;
// 	float point_speed_variation;

// 	PointEmitter() = default;

// 	PointEmitter(float3 color, float3 color_variation, float point_speed) :
// 		color(color),
// 		color_variation(color_variation),
// 		point_speed(point_speed),
// 		point_speed_variation(0.1)
//  	{}

// 	PointData emit(float3 min, float3 max)
// 	{
// 		PointData p;
// 		p.position = random_float3(min, max);
		
// 		// Note(Leo): BUG_FIX: float3 divisions by int caused ambiguous operator error, since we had also defined
// 		// conversion constructor for int3 (so compiler did not know whether to use "float3 / (float)int"
// 		// or "(int3)float / int"). It was fixes by marking the constructor explicit.
// 		p.color = color + random_float3(-color_variation, color_variation) * color;

// 		p.speed = point_speed + random_float(-point_speed_variation, point_speed_variation) * point_speed;
// 		return p;
// 	}
// };

// MINIMA_META_INFO(PointEmitter)
// {
// 	return members(
// 		member("color", &PointEmitter::color),
// 		member("color_variation", &PointEmitter::color_variation),
// 		member("point_speed", &PointEmitter::point_speed),
// 		member("point_speed_variation", &PointEmitter::point_speed_variation)
// 	);
// }

// namespace gui
// {
// 	inline bool edit(char const * label, PointEmitter & emitter)
// 	{
// 		auto helper = gui_helper(label);
		
// 		if (helper.open)
// 		{
// 			helper.edit(ColorEdit3("color", &emitter.color));
// 			helper.edit(SliderFloat3("color variation", &emitter.color_variation.x, 0.0f, 1.0f));
// 			helper.edit(DragFloat("point speed", &emitter.point_speed, 0.01f, 0.0f, FLT_MAX));
// 			helper.edit(SliderFloat("point speed variation", &emitter.point_speed_variation, 0.0f, 1.0f));
// 		}

// 		return helper.dirty;		
// 	}
// }