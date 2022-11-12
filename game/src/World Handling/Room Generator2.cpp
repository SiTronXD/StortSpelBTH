#include "Room Generator2.h"
#include "../deps/glm/src/glm/gtx/rotate_vector.hpp"
#include <cmath>

RoomGen::RoomGen()
	:tiles2D{}
{
}

RoomGen::~RoomGen()
{
}

void RoomGen::set(const RoomGenDescription& desc)
{
	this->desc = desc;

	tiles2D = new Tile2::Type * [desc.widthHeight];
	for (uint32_t i = 0; i < desc.widthHeight; i++)
	{
		tiles2D[i] = new Tile2::Type[desc.widthHeight];
		memset(tiles2D[i], Tile2::Type::Invalid, sizeof(Tile2::Type) * desc.widthHeight);
	}
}

void RoomGen::clear()
{
	tiles.clear();
	borders.clear();
	exitPathsTiles.clear();

	for (uint32_t i = 0; i < desc.widthHeight; i++)
	{
		delete[] tiles2D[i];
	}
	delete[]tiles2D;
	tiles2D = nullptr;
}

void RoomGen::generate()
{
	const glm::ivec2 middle(desc.widthHeight / 2);
	genCircle(middle);

	glm::vec2 fBranch(0);
	glm::ivec2 iBranch(0);
	for (uint32_t i = 0; i < desc.numBranches; i++)
	{
		fBranch.x = float(rand() % 2001) * 0.001f - 1.f;
		fBranch.y = float(rand() % 2001) * 0.001f - 1.f;	
		fBranch = glm::normalize(fBranch);
		iBranch = fBranch * (float)desc.branchDist;
		iBranch += middle;

		for (uint32_t j = 0; j < desc.branchLength; j++)
		{
			genCircle(iBranch);
			
			if (desc.maxAngle != 0)
			{
				const float rotAngle = float(rand() % (int)desc.maxAngle - (int)desc.maxAngle / 2);
				fBranch = glm::rotate(fBranch, float(rotAngle * (M_PI / 180.f)));
			}
			iBranch += fBranch * (float)desc.branchDist;
		}
	}

	this->setBorders();
	this->finalize();
	
}

void RoomGen::setBorders()
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
	glm::ivec2 a(0);
	for (position.x = 0; position.x < desc.widthHeight; position.x++)
	{
		for (position.y = 0; position.y < desc.widthHeight; position.y++)
		{
			if (tiles2D[position.x][position.y] == Tile2::Invalid)
			{
				for (uint32_t i = 0; i < NUM; i++)
				{
					for (int j = 1; j < desc.borderSize + 1; j++)
					{
						a = position + DIRS[i] * j;
						if (VALID(a, desc.widthHeight))
						{
							if (tiles2D[a.x][a.y] == Tile2::OneXOne)
								tiles2D[position.x][position.y] = Tile2::Border;
						}

					}
				}
			}
		}
	}
}

void RoomGen::finalize()
{
	const glm::ivec2 middle(desc.widthHeight / 2);

	glm::ivec2 position(0);
	for (position.x = 0; position.x < desc.widthHeight; position.x++)
	{
		for (position.y = 0; position.y < desc.widthHeight; position.y++)
		{
			Tile2::Type& curTile = tiles2D[position.x][position.y];

			if ((position.x == 0 || position.x == desc.widthHeight - 1 ||
				position.y == 0 || position.y == desc.widthHeight - 1) && curTile == Tile2::OneXOne)
			{
				curTile = Tile2::Border;
			}

			switch (curTile)
			{
			default:
				break;
			case Tile2::Type::Border:
				borders.emplace_back(Tile2::Border, position - middle);
				break;
			case Tile2::Type::OneXOne:
				tiles.emplace_back(Tile2::OneXOne, position - middle);
				break;
			case Tile2::Type::OneXTwo:

				break;
			case Tile2::Type::TwoXTwo:

				break;
			}
		}
	}
}

void RoomGen::genCircle(const glm::ivec2& center)
{
	const glm::ivec2 start = glm::ivec2(center) - glm::ivec2(desc.radius);

	glm::ivec2 currentPoint{};
	for (currentPoint.x = start.x; currentPoint.x < start.x + (int)desc.radius * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + (int)desc.radius * 2 + 1; currentPoint.y++)
		{
			if (VALID(currentPoint, desc.widthHeight))
			{
				// glm::dot only accepts floating point numbers
				const glm::vec2 vec(currentPoint - center);
				if (glm::dot(vec, vec) <= ((float)desc.radius + 0.5f) * ((float)desc.radius + 0.5f))
				{
					tiles2D[currentPoint.x][currentPoint.y] = Tile2::Type::OneXOne;
				}
			}
		}
	}
}

uint32_t RoomGen::getNumTiles() const
{
	return (int)tiles.size();
}

uint32_t RoomGen::getNumBorders() const
{
	return (int)borders.size();
}

uint32_t RoomGen::getNumExitTiles() const
{
	return (int)exitPathsTiles.size();
}

const Tile2& RoomGen::getTile(uint32_t index) const
{
	return tiles[index];
}

const Tile2& RoomGen::getBorder(uint32_t index) const
{
	return borders[index];
}

const Tile2& RoomGen::getExitTiles(uint32_t index) const
{
	return exitPathsTiles[index];
}
