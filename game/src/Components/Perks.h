#pragma once

enum PerkType : int
{
	hpUpPerk = 0,
	dmgUpPerk = 1,
	attackSpeedUpPerk = 2,
	movementUpPerk = 3,
	staminaUpPerk = 4,
	emptyPerk = 5,
};

const static std::vector<std::string> PERK_NAMES
{
	"health",
	"damage",
	"attack speed",
	"movement speed",
	"stamina",
};

struct Perks
{
	float multiplier;
	PerkType perkType;
};