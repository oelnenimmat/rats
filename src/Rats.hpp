#pragma once

#include "vectors.hpp"
#include "Gradient.hpp"
#include "memory.hpp"

#include "VoxelRenderer.hpp"

void break_point_marker() {}

template <typename T> constexpr int enum_names = 0;
template <typename T> constexpr int enum_count = array_length(enum_names<T>);
template <typename T> constexpr char const * enum_to_c_string(T value)
{
	return enum_names<T>[(int)value];
}


#define ENUM_NAMES_C_STRING(enum_type) template<> constexpr char const * enum_names<enum_type> [] =

// @basic_enum_names
enum struct RatBehaviour : int
{
	flee = 0,
	attack,
};

ENUM_NAMES_C_STRING(RatBehaviour)
{
	"flee",
	"attack"
};


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

	float start_interact_distance;
	float stop_interact_distance;

	float avoid_distance;

	float attack_strike_distance;

	float remove_after_fall_height;

	RatBehaviour behaviour;
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
	serializer.write("start_interact_distance", rat_settings.start_interact_distance);
	serializer.write("stop_interact_distance", rat_settings.stop_interact_distance);
	serializer.write("avoid_distance", rat_settings.avoid_distance);
	serializer.write("attack_strike_distance", rat_settings.attack_strike_distance);
	serializer.write("remove_after_fall_height", rat_settings.remove_after_fall_height);
	serializer.write("behaviour", rat_settings.behaviour);
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
	serializer.read("start_interact_distance", rat_settings.start_interact_distance);
	serializer.read("stop_interact_distance", rat_settings.stop_interact_distance);
	serializer.read("avoid_distance", rat_settings.avoid_distance);
	serializer.read("attack_strike_distance", rat_settings.attack_strike_distance);
	serializer.read("remove_after_fall_height", rat_settings.remove_after_fall_height);
	serializer.read("behaviour", rat_settings.behaviour);
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
		gui.edit("start_interact_distance", rat_settings.start_interact_distance);
		gui.edit("stop_interact_distance", rat_settings.stop_interact_distance);
		gui.edit("avoid_distance", rat_settings.avoid_distance);
		gui.edit("attack_strike_distance", rat_settings.attack_strike_distance);
		gui.edit("remove_after_fall_height", rat_settings.remove_after_fall_height);
		gui.edit_enum("behaviour", &rat_settings.behaviour);

		return gui.dirty;
	}
}

enum struct RatState
{
	idle = 0,
	moving,
	escape,
	fall,
	attack,
	strike
};

ENUM_NAMES_C_STRING(RatState)
{
	"idle",
	"moving",
	"escape",
	"fall",
	"attack",
	"strike",
};

struct Rat
{
	// float3 		position;
	float 		state_timer;
	RatState 	state;
	float3 		move_target_position;
	float3 		velocity;
	float3 		strike_target_position;
};

float3 move_rat_towards_position(
	float3 			current_position,
	float3 const & 	move_target_position,
	World const & 	world,
	float 			speed,
	float3 const &  avoid_vector,
	float 			delta_time,
	bool * 			on_ground_after_move
)
{
	float3 to_target = move_target_position - current_position + avoid_vector;
	to_target.y = 0;
	float3 end_position = current_position + normalize(to_target) * speed * delta_time;

	float walkable_terrain_height;
	*on_ground_after_move = get_closest_height_below_position(
		world, 
		end_position + float3(0, 0.5, 0),
		float3(-0.05f, 0.0f, -0.05f),
		float3(0.05f, 0.2f, 0.05f),
		&walkable_terrain_height
	);

	if (*on_ground_after_move)
	{
		end_position.y = walkable_terrain_height;
	}		

	return end_position;
}

struct RatRenderer
{
	VoxelObject voxel_object;
	float3 		world_position;
	float3 		render_offset;
	float 		WS_to_VS;
};

struct RatSystem
{
	RatSettings 	settings;	
	Array<float3> 	positions;
	Array<Rat> 		rats;
	bool 			draw_bounds;

	float cumulative_spawn_time;
	int spawned_count;
	int gone_count;

	int alive_count() const { return spawned_count - gone_count; }


	struct
	{
		int idle;
		int moving;
		int escaping;
		int attacking;
		int falling;
		int striking;
	} counts;

	int current_strike_count;
};

void reset(RatSystem & rats)
{
	rats.cumulative_spawn_time 	= 0;
	rats.spawned_count 			= 0;
	rats.gone_count 			= 0;
}

void discard_rat(RatSystem & rats, int index)
{

	int dump_index = rats.rats.length() - rats.gone_count - 1;
	// todo: why not this
	// int last_alive_index = rats.spawned_count - rats.gone_count;

	rats.positions[index] 	= rats.positions[dump_index];
	rats.rats[index] 		= rats.rats[dump_index];

	rats.gone_count 			+= 1;
}

namespace gui
{
	inline bool edit(RatSystem & rats)
	{
		auto gui = gui_helper();
		gui.edit(edit(rats.settings));
		gui.edit(Checkbox("draw_bounds", &rats.draw_bounds));

		if (BeginChild("ratlist", ImVec2(0, 200), true))
		{
			for (int i = 0; i < rats.rats.length() - rats.gone_count; i++)
			{
				Value(rats.positions[i]); SameLine(); Text("state: %s", enum_to_c_string(rats.rats[i].state));
			}
		}
		EndChild();	

		int spawn_count = (int)std::floor(rats.cumulative_spawn_time * rats.settings.spawn_rate_per_second);

		Value("cumulative_spawn_time", rats.cumulative_spawn_time);
		Text("Gone: %i/%i(%i)", rats.gone_count, rats.spawned_count, (int)rats.rats.length());

		Value("Current attack count", rats.current_strike_count);

		Value("idle", rats.counts.idle);
		Value("moving", rats.counts.moving);
		Value("escaping", rats.counts.escaping);
		Value("attacking", rats.counts.attacking);
		Value("striking", rats.counts.striking);
		Value("falling", rats.counts.falling);

		if (Button("Generate"))
		{
			reset(rats);
		}

		return gui.dirty;
	}
}

void init(RatSystem & rats, Allocator & persistent_allocator)
{
	rats.rats = Array<Rat>(rats.settings.count, persistent_allocator);
	rats.positions = Array<float3>(rats.settings.count, persistent_allocator);
	reset(rats);
}

void cleanup(RatSystem & rats, Allocator & same_persistent_allocator)
{

}

void prepare_frame(RatRenderer & renderer, VoxelSettings const & voxel_settings, float3 proximity_target)
{
	float WS_to_VS = voxel_settings.WS_to_VS();
	clear_slice_data(renderer.voxel_object.map.nodes);

	float3 render_chunk_size = float3(renderer.voxel_object.map.size_in_chunks) * voxel_settings.CS_to_WS();

	float3 render_chunk_position_WS = proximity_target - render_chunk_size / 2.0f;
	int3 render_chunk_position_CS 	= int3(floor(render_chunk_position_WS * voxel_settings.WS_to_CS()));
	int3 render_chunk_position_VS 	= render_chunk_position_CS * voxel_settings.voxels_in_chunk;

	renderer.voxel_object.position_VS = render_chunk_position_VS;

	renderer.render_offset = float3(render_chunk_position_VS) * voxel_settings.VS_to_WS();

	renderer.WS_to_VS = voxel_settings.WS_to_VS();
}

void draw_rats(RatSystem const & rats, RatRenderer & renderer)
{
	int alive_count = rats.spawned_count - rats.gone_count;

	for (int i = 0; i < alive_count; i++)
	{
		float3 start_WS = rats.positions[i] - renderer.render_offset;
		float3 size_WS 	= float3(0.1, 0.2, 0.1);
		if (rats.rats[i].state == RatState::idle)
		{
			size_WS.y = 0.4;
		}

		int3 start_VS 	= int3(floor(start_WS * renderer.WS_to_VS));
		int3 end_VS 	= max(start_VS + int3(1,1,1), int3(floor((start_WS + size_WS) * renderer.WS_to_VS)));

		for (int z = start_VS.z; z <= end_VS.z; z++)
		for (int y = start_VS.y; y <= end_VS.y; y++)
		for (int x = start_VS.x; x <= end_VS.x; x++)
		{
			auto & node = get_node(renderer.voxel_object.map, x, y, z);
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

void update_rats(
	RatSystem & rats,
	World const & world,
	float3 player_position,
	float delta_time
)
{
	rats.counts = {};

	rats.current_strike_count = 0;

	if (rats.spawned_count < rats.rats.length())
	{
		rats.cumulative_spawn_time += delta_time;
		int spawn_count = (int)std::floor(rats.cumulative_spawn_time * rats.settings.spawn_rate_per_second);

		if (spawn_count > 0)
		{
			int rats_left_to_spawn 		= rats.rats.length() - rats.spawned_count;
			spawn_count 				= rats::min(spawn_count, rats_left_to_spawn);
			rats.cumulative_spawn_time 	-= spawn_count / rats.settings.spawn_rate_per_second;

			for (int i = 0; i < spawn_count; i++)
			{
				int spawn_index = i + rats.spawned_count - rats.gone_count;

				rats.positions[spawn_index] = rats.settings.spawn_position;
				
				Rat & rat 		= rats.rats[spawn_index];
				rat.state_timer = 0;
				rat.state 		= RatState::idle;
				rat.velocity  	= float3(0,0,0);
			}

			rats.spawned_count += spawn_count;
		}
	}

	int alive_count = rats.spawned_count - rats.gone_count;

	for (int i = 0; i < alive_count; i++)
	{
		Rat & rat = rats.rats[i];
		float3 & rat_position = rats.positions[i];

		if ((rat.state == RatState::idle || rat.state == RatState::moving) &&
		 length(rat_position - player_position) < rats.settings.start_interact_distance)
		{
			switch(rats.settings.behaviour)
			{
				case RatBehaviour::flee:
					rat.state = RatState::escape;
					break;

				case RatBehaviour::attack:
					rat.state = RatState::attack;
					break;
			}
		}

		if (rat.state != RatState::strike && rat.state != RatState::fall)
		{
			float walkable_terrain_height;
			bool above_walkable_terrain = get_closest_height_below_position(
				world,
				rat_position + float3(0, 0.5f, 0),
				float3(-0.05f, 0.0f, -0.05f),
				float3(0.05f, 0.2f, 0.05f),
				&walkable_terrain_height
			);

			if (above_walkable_terrain == false || (walkable_terrain_height < rat_position.y))
			{
				rat.state = RatState::fall;
			}
		}

		float3 avoid_vector = float3(0,0,0);
		for (int u = 0; u < alive_count; u++)
		{
			float3 to_u_rat = rats.positions[u] - rats.positions[i];
			float distance_to_u_rat = length(to_u_rat);

			if (distance_to_u_rat < rats.settings.avoid_distance && distance_to_u_rat > 0.01)
			{
				avoid_vector += -normalize(to_u_rat) * (rats.settings.avoid_distance - distance_to_u_rat);
			}
		}

		switch (rat.state)
		{
			case RatState::idle:
			{	
				rats.counts.idle += 1;

				if (rat.state_timer > 0)
				{
					rat.state_timer -= delta_time;
				}
				else
				{
					rat.move_target_position = random_float3(rats.settings.world_min, rats.settings.world_max);
			
					float walkable_terrain_height;
					bool above_walkable_terrain = get_closest_height_below_position(
						world, 
						rat_position + float3(0, 0.5, 0),
						float3(-0.05f, 0.0f, -0.05f),
						float3(0.05f, 0.2f, 0.05f),
						&walkable_terrain_height
					);
					if (above_walkable_terrain)
					{
						rat.move_target_position.y = walkable_terrain_height;
					}

					rat.state = RatState::moving;
				}
			} break;

			case RatState::moving:
			{
				rats.counts.moving += 1;

				float3 position_before = rat_position;

				bool on_ground_after_move = false;
				rat_position = move_rat_towards_position(
					rat_position,
					rat.move_target_position,
					world,
					rats.settings.speed,
					avoid_vector,
					delta_time,
					&on_ground_after_move
				);

				if (on_ground_after_move == false)
				{
					rat.state = RatState::fall;
					rat.velocity = (rat_position - position_before) / delta_time;

				}

				if (length(rat_position.xz - rat.move_target_position.xz) < 0.1f)
				{
					rat.state 		= RatState::idle;
					rat.state_timer = random_float(1,4);
				}
			} break;

			case RatState::escape:
			{
				rats.counts.escaping += 1;

				float3 from_player = rat_position - player_position;
				float3 position_before = rat_position;

				if (length(from_player.xz) < rats.settings.stop_interact_distance)
				{
					rat_position.xz = rat_position.xz + normalize(from_player.xz) * rats.settings.speed * delta_time;
				}
				else
				{
					rat.state = RatState::idle;
					rat.state_timer = random_float(1, 4);
				}


				float walkable_terrain_height;
				bool on_ground_after_move = get_closest_height_below_position(
					world, 
					rat_position + float3(0, 0.5, 0),
					float3(-0.05f, 0.0f, -0.05f),
					float3(0.05f, 0.2f, 0.05f),
					&walkable_terrain_height
				);

				if (on_ground_after_move)
				{
					rat_position.y = walkable_terrain_height;
				}
				else
				{
					rat.state = RatState::fall;
					rat.velocity = (rat_position - position_before) / delta_time;
				}
			} break;

			case RatState::fall:
			{
				rats.counts.falling += 1;

				float walkable_terrain_height;
				bool above_walkable_terrain = get_closest_height_below_position(
					world,
					rat_position + float3(0, 0.2, 0), 
					float3(-0.05f, 0.0f, -0.05f),
					float3(0.05f, 0.2f, 0.05f),
					&walkable_terrain_height
				);

				if (above_walkable_terrain && (rat_position.y < walkable_terrain_height))
				{
					rat_position.y = walkable_terrain_height;
					rat.velocity = float3(0,0,0);
					rat.state = RatState::escape;
				}
				else
				{
					rat.velocity.y += -10 * delta_time;
					rat.velocity.xz = rat.velocity.xz * (1.0 - 0.2 * delta_time);
					rat_position += rat.velocity * delta_time;

					if (rat_position.y < rats.settings.remove_after_fall_height)
					{
						discard_rat(rats, i);
					}
				}
			} break;

			case RatState::attack:
			{
				rats.counts.attacking += 1;

				// float3 from_player = rat_position - player_position;
				// float3 target_position = player_position + normalize(from_player) * rats.settings.attack_strike_distance;

				float3 position_before = rat_position;

				bool on_ground_after_move;
				rat_position = move_rat_towards_position(
					rat_position,
					player_position,
					world,
					rats.settings.speed,
					avoid_vector,
					delta_time,
					&on_ground_after_move
				);

				float distance_to_target = length(rat_position - player_position);

				if (on_ground_after_move == false)
				{
					rat.state = RatState::fall;
					rat.velocity = (rat_position - position_before) / delta_time;
				}

				if (distance_to_target < rats.settings.attack_strike_distance)
				{
					rat.state = RatState::strike;
					rat.strike_target_position = player_position + float3(0, random_float(1.0f, 1.5f), 0);
				}
				else if (distance_to_target > rats.settings.stop_interact_distance)
				{
					rat.state 		= RatState::idle;
					rat.state_timer = random_float(1,4);
				}
			} break;

			case RatState::strike:
			{
				rats.counts.striking += 1;
			
				float3 position_before = rat_position;

				float3 to_target = rat.strike_target_position - rat_position;
				float3 movement = normalize(to_target) * rats.settings.speed * 1.2 * delta_time;
				
				rat_position += movement;
				to_target -= movement;

				if (length(to_target) < 0.1)
				{
					// hit target player
					if (length(rat_position.xz - player_position.xz) < 0.1)
					{
						rats.current_strike_count += 1;
						rat.state = RatState::escape;
					}

					// miss target player 
					else
					{
						rat.state = RatState::fall;
						rat.velocity = (rat_position - position_before) / delta_time;
					}

				}
				
			} break;
		}
	}
}