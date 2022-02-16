#pragma once

#include "vectors.hpp"
#include "Gradient.hpp"
#include "memory.hpp"

#include "VoxelRenderer.hpp"

void break_point_marker() {}

// @serialize
// @gui_edit
struct RatSettings
{
	int count 				= 10;
	float3 world_min 		= float3(0,0,0);
	float3 world_max 		= float3(2,2,2);
	float3 spawn_position;
	float spawn_rate_per_second;

	Gradient colors;

	float speed;

	float start_escape_distance;
	float stop_escape_distance;

	float remove_after_fall_height;
};

inline SERIALIZE_STRUCT(RatSettings const & rat_settings)
{
	serializer.write("count", rat_settings.count);
	serializer.write("world_min", rat_settings.world_min);
	serializer.write("world_max", rat_settings.world_max);
	serializer.write("spawn_position", rat_settings.spawn_position);
	serializer.write("spawn_rate_per_second", rat_settings.spawn_rate_per_second);
	serializer.write("colors", rat_settings.colors);
	serializer.write("speed", rat_settings.speed);
	serializer.write("start_escape_distance", rat_settings.start_escape_distance);
	serializer.write("stop_escape_distance", rat_settings.stop_escape_distance);
	serializer.write("remove_after_fall_height", rat_settings.remove_after_fall_height);
}

inline DESERIALIZE_STRUCT(RatSettings & rat_settings)
{
	serializer.read("count", rat_settings.count);
	serializer.read("world_min", rat_settings.world_min);
	serializer.read("world_max", rat_settings.world_max);
	serializer.read("spawn_position", rat_settings.spawn_position);
	serializer.read("spawn_rate_per_second", rat_settings.spawn_rate_per_second);
	serializer.read("colors", rat_settings.colors);
	serializer.read("speed", rat_settings.speed);
	serializer.read("start_escape_distance", rat_settings.start_escape_distance);
	serializer.read("stop_escape_distance", rat_settings.stop_escape_distance);
	serializer.read("remove_after_fall_height", rat_settings.remove_after_fall_height);
}

namespace gui
{
	inline bool edit (RatSettings & rat_settings)
	{
		auto gui = gui_helper();
		gui.edit("count", rat_settings.count);
		gui.edit("world_min", rat_settings.world_min);
		gui.edit("world_max", rat_settings.world_max);
		gui.edit("spawn_position", rat_settings.spawn_position);
		gui.edit("spawn_rate_per_second", rat_settings.spawn_rate_per_second);
		gui.edit("colors", rat_settings.colors);
		gui.edit("speed", rat_settings.speed);
		gui.edit("start_escape_distance", rat_settings.start_escape_distance);
		gui.edit("stop_escape_distance", rat_settings.stop_escape_distance);
		gui.edit("remove_after_fall_height", rat_settings.remove_after_fall_height);
		return gui.dirty;
	}
}

enum struct RatState
{
	idle,
	moving,
	escape,
	fall,
	attack,
	gone
};

// template<typename T>
// struct EnumTraits {};

// template<typename T>
// constexpr EnumTraits<T> enum_traits() { return EnumTraits<T>{}; }

// template<>
// struct EnumTraits<RatState>
// {
// 	static constexpr int count = 3;
// 	static constexpr char const * names [] =
// 	{
// 		"idle",
// 		"moving",
// 		"escape"
// 	};
// };

char const * to_c_string(RatState s)
{
	switch(s)
	{
		case RatState::idle: return "idle";
		case RatState::moving: return "moving";
		case RatState::escape: return "escape";
		case RatState::fall: return "fall";
		case RatState::attack: return "attack";
		case RatState::gone: return "gone";
	}
}

struct Rat
{
	float3 position;


	float state_timer;
	RatState state;
	float3 target_position;

	float y_velocity;
};

struct RatRenderer
{
	VoxelObject voxel_object;
	float3 		world_position;
};

struct RatSystem
{
	RatRenderer renderer;
	RatSettings settings;	
	Array<Rat> 	rats;
	bool 		draw_bounds;

	float cumulative_spawn_time;

	int spawned_count;
	int gone_count;

	// VoxelObject voxel_object;
};

void reset_rats(RatSystem & rats)
{
	rats.cumulative_spawn_time 	= 0;
	rats.spawned_count 			= 0;
	rats.gone_count 			= 0;
	// rats.gone_count = 0;
	// for(int i = 0; i < rats.rats.length(); i++)
	// {
	// 	rats.rats[i].position 		= random_float3(rats.settings.world_min, rats.settings.world_max);
	// 	rats.rats[i].state 			= RatState::idle;
	// 	rats.rats[i].state_timer 	= 0;
	// 	rats.rats[i].y_velocity 	= 0;
	// }
}

namespace gui
{
	inline bool edit(RatSystem & rats)
	{
		auto gui = gui_helper();
		gui.edit(edit(rats.settings));
		gui.edit(Checkbox("draw_bounds", &rats.draw_bounds));

		gui.edit(edit("render chunk position", rats.renderer.world_position));

		if (BeginChild("ratlist", ImVec2(0, 200), true))
		{
			for (Rat const & rat : rats.rats)
			{
				Value(rat.position); SameLine(); Text("state: %s", to_c_string(rat.state));
			}
		}
		EndChild();	

		int spawn_count = (int)std::floor(rats.cumulative_spawn_time * rats.settings.spawn_rate_per_second);

		Value("cumulative_spawn_time", rats.cumulative_spawn_time);
		Value("spawned_count", spawn_count);
		Text("Gone: %i/%i(%i)", rats.gone_count, rats.spawned_count, (int)rats.rats.length());

		if (Button("Generate"))
		{
			reset_rats(rats);
		}

		return gui.dirty;
	}
}

void init(RatSystem & rats, Allocator & persistent_allocator)
{
	rats.rats = Array<Rat>(rats.settings.count, persistent_allocator);
	// rats.voxel_object = voxel_object;
	reset_rats(rats);
}

void draw_rats(RatSystem & rats, VoxelRenderer & renderer, float3 proximity_target)
{
	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();
	clear_slice_data(rats.renderer.voxel_object.map.nodes);

	float3 render_chunk_size = float3(rats.renderer.voxel_object.map.size_in_chunks) * renderer.draw_options->voxel_settings.CS_to_WS();
	float3 render_chunk_position_WS = proximity_target - render_chunk_size / 2.0f;

	// int3 voxels_in_mega_chunks = int3(4,4,4) * renderer.draw_options->voxel_settings.voxels_in_chunk;

	// int3 render_chunk_position_VS = int3(floor(render_chunk_position_WS * WS_to_VS));
	// int3 render_chunk_position_CS = render_chunk_position_VS / renderer.draw_options->voxel_settings.voxels_in_chunk;
	// int3 render_chunk_position_MCS = render_chunk_position_CS / int3(4,4,4);

	// render_chunk_position_VS = render_chunk_position_MCS * voxels_in_mega_chunks;

	// rats.renderer.voxel_object.position_VS = render_chunk_position_VS; //int3(floor(render_chunk_position_WS * voxel_settings.WS_to_VS()));
	// render_chunk_position_WS = float3(render_chunk_position_VS) / WS_to_VS;

	int3 render_chunk_position_CS = int3(floor(render_chunk_position_WS * renderer.draw_options->voxel_settings.WS_to_CS()));

	int3 render_chunk_position_VS = render_chunk_position_CS * renderer.draw_options->voxel_settings.voxels_in_chunk;

	rats.renderer.voxel_object.position_VS = render_chunk_position_VS;


	float3 render_offset = float3(render_chunk_position_VS) * renderer.draw_options->voxel_settings.VS_to_WS();



	for (int i = 0; i < rats.spawned_count; i++)
	{
		if (rats.rats[i].state == RatState::gone)
		{
			continue;
		}

		float3 start_WS = rats.rats[i].position - render_offset;
		float3 size_WS 	= float3(0.1, 0.2, 0.1);
		if (rats.rats[i].state == RatState::idle)
		{
			size_WS.y = 0.4;
		}

		int3 start_VS 	= int3(floor(start_WS * WS_to_VS));
		int3 end_VS 	= max(start_VS + int3(1,1,1), int3(floor((start_WS + size_WS) * WS_to_VS)));

		for (int z = start_VS.z; z <= end_VS.z; z++)
		for (int y = start_VS.y; y <= end_VS.y; y++)
		for (int x = start_VS.x; x <= end_VS.x; x++)
		{
			auto & node = get_node(rats.renderer.voxel_object.map, x, y, z);
			// auto & node = get_dynamic_map_voxel(renderer, int3(x,y,z));
			node.material() = 1;
			node.color = rats.settings.colors.evaluate(SmallXXHash::seed(i).get_float_A_01());

			node.normal() = normalize(float3(
				(float)(x == end_VS.x) - (float)(x == start_VS.x),
				(float)(y == end_VS.y) - (float)(y == start_VS.y),
				(float)(z == end_VS.z) - (float)(z == start_VS.z)
			));
		}
	}
}

/*
drop to ground
move to random positions
move away from player
move towards player (up to a distance) in other state
*/
void update_rats(
	RatSystem & rats,
	World const & world,
	float3 player_position,
	float delta_time
)
{
	if (rats.spawned_count < rats.rats.length())
	{
		rats.cumulative_spawn_time 	+= delta_time;
		int spawn_count = (int)std::floor(rats.cumulative_spawn_time * rats.settings.spawn_rate_per_second);

		if (spawn_count > 0)
		{
			int rats_left_to_spawn = rats.rats.length() - rats.spawned_count;
			spawn_count = rats::min(spawn_count, rats_left_to_spawn);


			// float spawn_time 	= delta_time + rats.cumulative_spawn_time;
			// int spawn_count 	= std::floor(rats.settings.spawn_rate_per_second * spawn_time);
			rats.cumulative_spawn_time -= spawn_count / rats.settings.spawn_rate_per_second;


			for (int i = 0; i < spawn_count; i++)
			{
				Rat & rat 		= rats.rats[i + rats.spawned_count];
				rat.position 	= rats.settings.spawn_position;
				rat.state_timer = 0;
				rat.state 		= RatState::idle;
				rat.y_velocity  = 0;
			}

			rats.spawned_count += spawn_count;
		}
	}

	// for (Rat & rat : rats.rats)
	for (int i = 0; i < rats.spawned_count; i++)
	{
		Rat & rat = rats.rats[i];

		// this is now differently so we dont accidentally set its state to realive
		if (rat.state == RatState::gone)
		{
			continue;
		}


		if (length(rat.position - player_position) < rats.settings.start_escape_distance)
		{
			rat.state = RatState::escape;
		}

		{
			float walkable_terrain_height;
			bool above_walkable_terrain = get_closest_height_below_position(
				world,
				rat.position + float3(0, 0.5f, 0),
				float3(-0.05f, 0.0f, -0.05f),
				float3(0.05f, 0.2f, 0.05f),
				&walkable_terrain_height
			);

			if (above_walkable_terrain == false || (walkable_terrain_height < rat.position.y))
			{
				rat.state = RatState::fall;
			}
		}


		switch (rat.state)
		{
			case RatState::idle:
			{	
				if (rat.state_timer > 0)
				{
					rat.state_timer -= delta_time;
				}
				else
				{
					rat.target_position = random_float3(rats.settings.world_min, rats.settings.world_max);
			
					float walkable_terrain_height;
					bool above_walkable_terrain = get_closest_height_below_position(
						world, 
						rat.position + float3(0, 0.5, 0),
						float3(-0.05f, 0.0f, -0.05f),
						float3(0.05f, 0.2f, 0.05f),
						&walkable_terrain_height
					);
					if (above_walkable_terrain)
					{
						rat.target_position.y = walkable_terrain_height;
					}

					rat.state = RatState::moving;
				}
			} break;

			case RatState::moving:
			{
				float2 to_target_xz = rat.target_position.xz - rat.position.xz;
				if (length(to_target_xz) > 0.1f)
				{
					rat.position.xz = rat.position.xz + normalize(to_target_xz) * rats.settings.speed * delta_time;
				}
				else
				{
					rat.state = RatState::idle;
					rat.state_timer = random_float(1, 4);
				}

				float walkable_terrain_height;
				bool above_walkable_terrain = get_closest_height_below_position(
					world, 
					rat.position + float3(0, 0.5, 0),
					float3(-0.05f, 0.0f, -0.05f),
					float3(0.05f, 0.2f, 0.05f),
					&walkable_terrain_height
				);

				if (above_walkable_terrain)
				{
					rat.position.y = walkable_terrain_height;
				}
			} break;

			case RatState::escape:
			{
				float3 from_player = rat.position - player_position;

				// float2 to_target_xz = rat.target_position.xz - rat.position.xz;
				// if (length(to_target_xz) > 0.1f)
				if (length(from_player.xz) < rats.settings.stop_escape_distance)
				{
					rat.position.xz = rat.position.xz + normalize(from_player.xz) * rats.settings.speed * delta_time;
				}
				else
				{
					rat.state = RatState::idle;
					rat.state_timer = random_float(1, 4);
				}

				float walkable_terrain_height;
				bool above_walkable_terrain = get_closest_height_below_position(
					world, 
					rat.position + float3(0, 0.5, 0),
					float3(-0.05f, 0.0f, -0.05f),
					float3(0.05f, 0.2f, 0.05f),
					&walkable_terrain_height
				);

				if (above_walkable_terrain)
				{
					rat.position.y = walkable_terrain_height;
				}
			} break;

			case RatState::fall:
			{
				float walkable_terrain_height;
				bool above_walkable_terrain = get_closest_height_below_position(
					world,
					rat.position + float3(0, 0.2, 0), 
					float3(-0.05f, 0.0f, -0.05f),
					float3(0.05f, 0.2f, 0.05f),
					&walkable_terrain_height
				);

				if (above_walkable_terrain && (rat.position.y < walkable_terrain_height))
				{
					rat.position.y = walkable_terrain_height;
					rat.y_velocity = 0;
					rat.state = RatState::idle;
				}
				else
				{
					rat.y_velocity += -10 * delta_time;
					rat.position.y += rat.y_velocity * delta_time;

					if (rat.y_velocity < -1)
					{
						break_point_marker();
					}

					if (rat.position.y < rats.settings.remove_after_fall_height)
					{
						rat.state 		= RatState::gone;
						rats.gone_count += 1 ;
					}
				}

			} break;

			case RatState::attack:
				rat.state = RatState::idle;
				break;

			case RatState::gone:
				// handled at the top of the loop
				break;
		}
	}
}