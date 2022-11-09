#pragma once

enum PerkType : int
{
	hpUpPerk = 0,
	dmgUpPerk = 1,
	attackSpeedUpPerk = 2,
	emptyPerk = 3
};

struct Perks
{
	float multiplier;
	PerkType perkType;
};