#pragma once

enum PerkType : int
{
	hpUp = 0,
	dmgUp = 1,
	attackSpeedUp = 2,
	empty = 3
};

struct Perks
{
	float multiplier;
	PerkType perkType;
};