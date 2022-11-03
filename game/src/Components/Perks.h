#pragma once

enum PerkType { empty, dmgUp, hpUp, attackSpeedUp };

struct Perks
{
	float multiplier;
	PerkType perkType;
};