#pragma once

enum AbilityType : int
{
	knockbackAbility = 0,
	healAbility = 1,
	emptyAbility = 2
};

const static std::vector<std::string> ABILITY_NAMES
{
	"knockback",
	"heal",
};

struct Abilities
{
	AbilityType abilityType;
};