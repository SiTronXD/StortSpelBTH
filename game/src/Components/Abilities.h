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
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
}; // Just in case of future abilities (no runtime errors during development)

struct Abilities
{
	AbilityType abilityType;
};