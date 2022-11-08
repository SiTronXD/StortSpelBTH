#pragma once

enum AbilityType : int
{
	knockbackAbility = 0,
	healAbility = 1,
	emptyAbility = 2
};

struct Abilities
{
	AbilityType abilityType;
};