#pragma once

#include "Engine.hpp"
#include "Character.hpp"
#include "Rats.hpp"

void game_reset_timeline(Engine & engine)
{
	reset(engine.character);
	reset(engine.rats);
	reset(engine.rats_2);
}
