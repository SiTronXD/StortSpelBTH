#include "Room Generator.h"
#include "../deps/glm/src/glm/gtx/rotate_vector.hpp"
#include "vengine/dev/Random.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

RoomGenerator::RoomGenerator(VRandom& random)
	:tiles2D{}, random(random), minMaxPos{}, exitTilesPos{}, middle(0), size(0)
{
	this->tiles2D = new Tile::Type * [WIDTH_HEIGHT];
	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		this->tiles2D[i] = new Tile::Type[WIDTH_HEIGHT];
	}

	this->clear();
}

/*
	TODO: (Not in order)
	* Add support for 1x2 & 2x2 tiles & make sure doors aren't blocked by them
		* Make sure the rock can spawn in the middle
	* Generate multiple rooms
		* Generate paths
		* Generate borders around paths
	* Smoothen out borders
	* Separate the current tile-meshes from their floor
		* Fill the rooms with stuff
	* Only place colliders on first border layer (Fog?)
*/

RoomGenerator::~RoomGenerator()
{
	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		delete[] this->tiles2D[i];
	}
	delete[] this->tiles2D;
}

void RoomGenerator::clear()
{
	this->exitTilesPos[LEFT_P]  = this->minMaxPos[LEFT_P]  = glm::ivec2(-1);
	this->exitTilesPos[RIGHT_P] = this->minMaxPos[RIGHT_P] = glm::ivec2(WIDTH_HEIGHT);
	this->exitTilesPos[UPPER_P] = this->minMaxPos[UPPER_P] = glm::ivec2(-1);
	this->exitTilesPos[LOWER_P] = this->minMaxPos[LOWER_P] = glm::ivec2(WIDTH_HEIGHT);

	this->mainTiles.clear();
	this->bigTiles.clear();
	this->borders.clear();
	this->innerBorders.clear();
	this->exitPathsTiles.clear();

	for (uint32_t i = 0; i < WIDTH_HEIGHT; i++)
	{
		memset(this->tiles2D[i], Tile::Type::Unused, sizeof(Tile::Type) * WIDTH_HEIGHT);
	}
}

void RoomGenerator::generate(bool* doors)
{
	const glm::ivec2 gridMid(WIDTH_HEIGHT / 2);
	this->drawCircle(gridMid, RADIUS);

	glm::vec2 fBranch(0);
	glm::ivec2 iBranch(0);
	for (uint32_t i = 0; i < NUM_BRANCHES; i++)
	{
		fBranch.x = float(this->random.rand() % 2001) * 0.001f - 1.f;
		fBranch.y = float(this->random.rand() % 2001) * 0.001f - 1.f;
		fBranch = glm::normalize(fBranch);
		iBranch = fBranch * (float)BRANCH_DIST;
		iBranch += gridMid;

		for (uint32_t j = 0; j < BRANCH_DEPTH; j++)
		{
			this->drawCircle(iBranch, RADIUS);
			
			if (MAX_ANGLE != 0)
			{
				const float rotAngle = float((int)this->random.rand() % (int)MAX_ANGLE - (int)MAX_ANGLE / 2);
				fBranch = glm::rotate(fBranch, float(rotAngle * (M_PI / 180.f)));
			}
			iBranch += fBranch * (float)BRANCH_DIST;
		}
	}

	this->findMinMax(); // Find minMax tiles & middle

	// Reserve middle tiles for big rock
	getType(this->middle) = Tile::Reserved;
	getType(this->middle + glm::ivec2(1, 0)) = Tile::Reserved;
	getType(this->middle + glm::ivec2(0, 1)) = Tile::Reserved;
	getType(this->middle + glm::ivec2(1, 1)) = Tile::Reserved;

	this->setBigTiles();	// Select spots for 1x2/2x1/2x2
	this->setExits(doors);	// Set doors and make room for them
	this->setBorders();		// Surround the room with borders
	this->finalize();		// Finalize, fill Tile vectors and offset room to origo
}

void RoomGenerator::findMinMax()
{
	glm::ivec2 position(0);
	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile::Type& tile = this->getType(position);

			if (tile != Tile::OneXOne)
				continue;

			if (position.x > this->minMaxPos[LEFT_P].x)
			{
				this->minMaxPos[LEFT_P] = position;
			}
			if (position.x < this->minMaxPos[RIGHT_P].x)
			{
				this->minMaxPos[RIGHT_P] = position;
			}
			if (position.y > this->minMaxPos[UPPER_P].y)
			{
				this->minMaxPos[UPPER_P] = position;
			}
			if (position.y < this->minMaxPos[LOWER_P].y)
			{
				this->minMaxPos[LOWER_P] = position;
			}
		}
	}

	this->middle.x = (this->minMaxPos[LEFT_P].x  + this->minMaxPos[RIGHT_P].x) / 2;
	this->middle.y = (this->minMaxPos[UPPER_P].y + this->minMaxPos[LOWER_P].y) / 2;
	this->size.x = this->minMaxPos[LEFT_P].x  - this->minMaxPos[RIGHT_P].x;
	this->size.y = this->minMaxPos[UPPER_P].y - this->minMaxPos[LOWER_P].y;
}

void RoomGenerator::setBigTiles()
{
	const uint32_t MAX_SEARCH = 100;
	uint32_t searchCounter = 0;
	glm::ivec2 pos(0);
	bool vertical;

	// First part of loops is for 2x2, second part is for 1x2/2x1
	for (uint32_t i = 0; i < MAX_TWO_X_TWO + MAX_ONE_X_TWO; i++)
	{
		searchCounter = 0;

		while (searchCounter++ < MAX_SEARCH)
		{
			pos.x = (int)this->random.rand() % this->size.x + this->minMaxPos[RIGHT_P].x;
			pos.y = (int)this->random.rand() % this->size.y + this->minMaxPos[LOWER_P].y;
			
			if (i < MAX_TWO_X_TWO)
			{
				if (this->canPlaceTwoXTwo(pos) && this->random.rand() % 100 < TWO_X_TWO_CHANCE)
				{
					this->placeTwoXTwo(pos);
					searchCounter = MAX_SEARCH;
				}
			}
			else
			{
				vertical = this->random.rand() % 2;
				if (this->canPlaceOneXTwo(pos, vertical) && this->random.rand() % 100 < ONE_X_TWO_CHANCE)
				{
					this->placeOneXTwo(pos, vertical);
					searchCounter = MAX_SEARCH;
				}
			}
		}
	}
}

void RoomGenerator::setExits(bool* doors)
{
	const glm::ivec2 maxOffsets(this->size / 2);

	glm::ivec2 doorsPos[4] = 
	{
		{ this->minMaxPos[LEFT_P].x,  this->middle.y + (this->random.rand() % maxOffsets.y - maxOffsets.y / 2) },
		{ this->minMaxPos[RIGHT_P].x, this->middle.y + (this->random.rand() % maxOffsets.y - maxOffsets.y / 2) },

		{ this->middle.x + (this->random.rand() % maxOffsets.x - maxOffsets.x / 2), this->minMaxPos[UPPER_P].y },
		{ this->middle.x + (this->random.rand() % maxOffsets.x - maxOffsets.x / 2), this->minMaxPos[LOWER_P].y }
	};

	const glm::ivec2 offsets[4] =
	{
		{0, RADIUS / 2},
		{0, RADIUS / 2},
		{RADIUS / 2, 0},
		{RADIUS / 2, 0}
	};

	const glm::ivec2 dirs[4] =
	{
		{ 1,  0},
		{-1,  0},
		{ 0,  1},
		{ 0, -1}
	};

	glm::ivec2 adjacent(0);
	for (int i = 0; i < 4; i++)
	{
		if (doors[i])
		{
			doorsPos[i] += offsets[i];

			this->drawCircle(doorsPos[i], RADIUS);
			this->drawCircle(doorsPos[i] + -dirs[i] * (int)RADIUS, RADIUS - 1u);

			doorsPos[i] += dirs[i] * (int)RADIUS;

			if (this->onEdge(doorsPos[i]))
			{
				this->getType(doorsPos[i]) = Tile::Exit;
				this->exitTilesPos[i] = doorsPos[i];
			}
			else
			{
				glm::ivec2 lastValid = doorsPos[i];
				for (int j = 1; j < (int)BORDER_SIZE + 1; j++)
				{
					adjacent = doorsPos[i] + dirs[i] * j;
					if (this->isValid(adjacent))
					{
						this->getType(adjacent) = Tile::Exit;
						lastValid = adjacent;
					}
				}
				this->exitTilesPos[i] = lastValid + dirs[i];
			}
		}
	}
}

void RoomGenerator::setBorders()
{
	const int NUM = 8;
	const glm::ivec2 DIRS[NUM] =
	{
		{ 1,  0},
		{-1,  0},
		{ 0,  1},
		{ 0, -1},

		{ 1,  1},
		{-1,  1},
		{ 1, -1},
		{-1, -1},
	};

	glm::ivec2 position(0);
	glm::ivec2 adjacent(0);
	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile::Type& currTile = this->getType(position);
			if (currTile == Tile::Unused)
			{
				for (uint32_t i = 0; i < NUM; i++)
				{
					for (int j = 1; j < (int)BORDER_SIZE + 1; j++)
					{
						adjacent = position + DIRS[i] * j;
						if (this->isValid(adjacent))
						{
							const Tile::Type& adj = this->getType(adjacent);
							if (adj == Tile::OneXOne || adj == Tile::TwoXTwo ||
								adj == Tile::OneXTwo || adj == Tile::TwoXOne)
							{
								currTile = Tile::Border; 
								i = NUM;
								j = BORDER_SIZE + 1;
							}
						}
					}
				}
			}
		}
	}

	for (position.x = 0; position.x < WIDTH_HEIGHT; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_HEIGHT; position.y++)
		{
			Tile::Type& currTile = this->getType(position);
			if (currTile == Tile::OneXOne || currTile == Tile::Exit || 
				currTile == Tile::OneXTwo || currTile == Tile::TwoXOne || currTile == Tile::TwoXTwo)
			{
				for (int i = 0; i < 4; i++)
				{
					adjacent = position + DIRS[i];
					if (this->isValid(adjacent))
					{
						if (this->getType(adjacent) == Tile::Border)
						{
							this->getType(adjacent) = Tile::InnerBorder;
						}
					}
				}
			}
		}
	}
}

void RoomGenerator::finalize()
{
	const glm::vec2 fMiddle = middle;
	glm::vec2 fPosition(0.f);
	glm::ivec2 iPosition(0);

	for (iPosition.x = 0; iPosition.x < WIDTH_HEIGHT; iPosition.x++)
	{
		for (iPosition.y = 0; iPosition.y < WIDTH_HEIGHT; iPosition.y++)
		{
			Tile::Type& curTile = this->getType(iPosition);

			if (this->onEdge(iPosition) && curTile != Tile::Unused)
			{
				curTile = Tile::Border;
			}
			
			fPosition = iPosition;

			switch (curTile)
			{
			default:
				break;
			case Tile::Border:
				this->borders.emplace_back(Tile::Border, fPosition - fMiddle);
				break;
			case Tile::InnerBorder:
				this->innerBorders.emplace_back(Tile::InnerBorder, fPosition - fMiddle);
				break;

			case Tile::OneXOne:
				this->mainTiles.emplace_back(Tile::OneXOne, fPosition - fMiddle);
				break;

			case Tile::TwoXOne:
				this->bigTiles.emplace_back(Tile::TwoXOne, (fPosition + glm::vec2(0.5f, 0.f)) - fMiddle);
				this->getType(iPosition + glm::ivec2(1, 0)) = Tile::Reserved;
				break;

			case Tile::OneXTwo:
				this->bigTiles.emplace_back(Tile::OneXTwo, (fPosition + glm::vec2(0.f, 0.5f)) - fMiddle);
				this->getType(iPosition + glm::ivec2(0, 1)) = Tile::Reserved;
				break;

			case Tile::TwoXTwo:
				this->bigTiles.emplace_back(Tile::TwoXTwo, (fPosition + glm::vec2(0.5f)) - fMiddle);
				this->getType(iPosition + glm::ivec2(1, 0)) = Tile::Reserved;
				this->getType(iPosition + glm::ivec2(0, 1)) = Tile::Reserved;
				this->getType(iPosition + glm::ivec2(1, 1)) = Tile::Reserved;
				break;

			case Tile::Exit:
				this->exitPathsTiles.emplace_back(Tile::Exit, fPosition - fMiddle);
				break;
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		this->exitTilesPos[i] -= this->middle;
	}
}

void RoomGenerator::drawCircle(const glm::ivec2& center, uint32_t radius)
{
	const glm::ivec2 start = center - glm::ivec2((int)radius);

	glm::ivec2 currentPoint{};
	for (currentPoint.x = start.x; currentPoint.x < start.x + (int)radius * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + (int)radius * 2 + 1; currentPoint.y++)
		{
			if (this->isValid(currentPoint))
			{
				Tile::Type& curTile = this->getType(currentPoint);
				if (curTile == Tile::Unused && curTile != Tile::Reserved)
				{
					// check if tile is within radius to center
					// glm::dot only accepts floating point numbers
					const glm::vec2 centerToCurr(currentPoint - center);
					if (glm::dot(centerToCurr, centerToCurr) <= ((float)radius + 0.5f) * ((float)radius + 0.5f))
					{
						curTile = Tile::Type::OneXOne;
					}
				}
			}
		}
	}
}

bool RoomGenerator::canPlaceOneXTwo(const glm::ivec2& pos, bool vertical)
{
	const glm::ivec2 adjacent(pos.x + 1 * !vertical, pos.y + 1 * vertical);

	if (!this->isValid(pos) || !this->isValid(adjacent))
	{
		return false;
	}

	if (this->getType(pos) != Tile::OneXOne || this->getType(adjacent) != Tile::OneXOne)
	{
		return false;
	}

	glm::ivec2 curPos(0);
	const int thicc = (int)BIG_TILE_MIN_DIST;
	for (curPos.x = pos.x - thicc; curPos.x <= (pos.x + (int)!vertical + thicc); curPos.x++)
	{
		for (curPos.y = pos.y - thicc; curPos.y <= (pos.y + (int)vertical + thicc); curPos.y++)
		{
			if (this->isValid(curPos))
			{
				const Tile::Type& curTile = this->getType(curPos);
				if (curTile != Tile::OneXOne && curTile != Tile::Unused)
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool RoomGenerator::canPlaceTwoXTwo(const glm::ivec2& pos)
{
	if (!this->isValid(pos) || !this->isValid(pos + glm::ivec2(1, 0)) ||
		!this->isValid(pos + glm::ivec2(0, 1)) || !this->isValid(pos + glm::ivec2(1, 1)))
	{
		return false;
	}

	if (this->getType(pos) != Tile::OneXOne || this->getType(pos + glm::ivec2(1, 0)) != Tile::OneXOne ||
		this->getType(pos + glm::ivec2(0, 1)) != Tile::OneXOne || this->getType(pos + glm::ivec2(1, 1)) != Tile::OneXOne)
	{
		return false;
	}

	glm::ivec2 curPos(0);
	const int thicc = (int)BIG_TILE_MIN_DIST;
	for (curPos.x = pos.x - thicc; curPos.x <= (pos.x + 1 + thicc); curPos.x++)
	{
		for (curPos.y = pos.y - thicc; curPos.y <= (pos.y + 1 + thicc); curPos.y++)
		{
			if (this->isValid(curPos))
			{
				const Tile::Type& curTile = this->getType(curPos);
				if (curTile != Tile::OneXOne && curTile != Tile::Unused)
				{
					return false;
				}
			}
		}
	}
	
	return true;
}

void RoomGenerator::placeTwoXTwo(const glm::ivec2& pos)
{
	this->getType(pos) = Tile::TwoXTwo;
	this->getType(pos + glm::ivec2(1, 0)) = Tile::TwoXTwo;
	this->getType(pos + glm::ivec2(0, 1)) = Tile::TwoXTwo;
	this->getType(pos + glm::ivec2(1, 1)) = Tile::TwoXTwo;
}

void RoomGenerator::placeOneXTwo(const glm::ivec2& pos, bool vertical)
{
	const glm::ivec2 adjacent(pos.x + 1 * !vertical, pos.y + 1 * vertical);

	this->getType(pos)		= vertical ? Tile::OneXTwo : Tile::TwoXOne;
	this->getType(adjacent) = vertical ? Tile::OneXTwo : Tile::TwoXOne;
}

const glm::ivec2* RoomGenerator::getMinMax() const
{
	return this->minMaxPos;
}

const glm::ivec2* RoomGenerator::getExits() const
{
	return this->exitTilesPos;
}

const glm::ivec2& RoomGenerator::getMiddle() const
{
	return this->middle;
}

Tile::Type& RoomGenerator::getType(const glm::ivec2& pos)
{
	return this->tiles2D[pos.x][pos.y];
}

bool RoomGenerator::isValid(const glm::ivec2& pos)
{
	return (pos.x >= 0 && pos.x < WIDTH_HEIGHT && pos.y >= 0 && pos.y < WIDTH_HEIGHT);
}

bool RoomGenerator::onEdge(const glm::ivec2& pos)
{
	return (pos.x == 0 || pos.x == WIDTH_HEIGHT - 1 ||
			pos.y == 0 || pos.y == WIDTH_HEIGHT - 1);
}

uint32_t RoomGenerator::getNumMainTiles() const
{
	return (int)this->mainTiles.size();
}

uint32_t RoomGenerator::getNumBigTiles() const
{
	return (int)this->bigTiles.size();
}

uint32_t RoomGenerator::getNumBorders() const
{
	return (int)this->borders.size();
}

uint32_t RoomGenerator::getNumInnerBorders() const
{
	return (int)this->innerBorders.size();
}

uint32_t RoomGenerator::getNumExitTiles() const
{
	return (int)this->exitPathsTiles.size();
}

const Tile& RoomGenerator::getMainTile(uint32_t index) const
{
	return this->mainTiles[index];
}

const Tile& RoomGenerator::getBigTile(uint32_t index) const
{
	return this->bigTiles[index];
}

const Tile& RoomGenerator::getBorder(uint32_t index) const
{
	return this->borders[index];
}

const Tile& RoomGenerator::getInnerBorder(uint32_t index) const
{
	return this->innerBorders[index];
}

const Tile& RoomGenerator::getExitTile(uint32_t index) const
{
	return this->exitPathsTiles[index];
}
