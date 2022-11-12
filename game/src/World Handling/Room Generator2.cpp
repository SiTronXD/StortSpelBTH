#include "Room Generator2.h"

RoomGen::RoomGen()
	:tiles2D{}
{
}

RoomGen::~RoomGen()
{
}

void RoomGen::generate(uint32_t width_num, uint32_t radius, uint32_t numBranches, uint32_t branchLength)
{
	tiles.clear();

	tiles2D = new Tile2::Type * [WIDTH_NUM];
	for (uint32_t i = 0; i < WIDTH_NUM; i++)
	{
		tiles2D[i] = new Tile2::Type[WIDTH_NUM];
	}

	WIDTH_NUM = width_num;
	CIRCLE_RADIUS = radius;
	NUM_BRANCHES = numBranches;
	BRANCH_LENGTH = branchLength;


	const glm::ivec2 middle(WIDTH_NUM / 2);
	//genCircle(middle);

	glm::vec2 fBranch(0);
	glm::ivec2 iBranch(0);
	for (uint32_t i = 0; i < NUM_BRANCHES; i++)
	{
		fBranch.x = float(rand() % 2001) * 0.001f - 1.f;
		fBranch.y = float(rand() % 2001) * 0.001f - 1.f;
		fBranch = glm::normalize(fBranch);
		iBranch = fBranch * (float)CIRCLE_RADIUS;

		for (uint32_t j = 0; j < BRANCH_LENGTH; j++)
		{
			genCircle(iBranch);
			iBranch += fBranch * (float)CIRCLE_RADIUS;
		}
	}


	const glm::ivec2 OFFSET(WIDTH_NUM / 2);

	glm::ivec2 position(0);
	for (position.x = 0; position.x < WIDTH_NUM; position.x++)
	{
		for (position.y = 0; position.y < WIDTH_NUM; position.y++)
		{
			switch (tiles2D[position.x][position.y])
			{
			default:
				break;
			case Tile2::Type::Border:

				break;
			case Tile2::Type::OneXOne:
				tiles.emplace_back(Tile2::Type::OneXOne, position - OFFSET);
				break;
			case Tile2::Type::OneXTwo:

				break;
			case Tile2::Type::TwoXTwo:

				break;
			}
		}
	}

	for (uint32_t i = 0; i < WIDTH_NUM; i++)
	{
		delete[] tiles2D[i];
	}
	delete[]tiles2D;
}

void RoomGen::genCircle(const glm::ivec2& center)
{
	const glm::ivec2 start = glm::ivec2(center) - glm::ivec2(CIRCLE_RADIUS);

	glm::ivec2 currentPoint{};
	for (currentPoint.x = start.x; currentPoint.x < start.x + (int)CIRCLE_RADIUS * 2 + 1; currentPoint.x++)
	{
		for (currentPoint.y = start.y; currentPoint.y < start.y + (int)CIRCLE_RADIUS * 2 + 1; currentPoint.y++)
		{
			if (VALID(currentPoint))
			{
				// glm::dot only accepts floating point numbers
				const glm::vec2 vec(currentPoint - center);
				if (glm::dot(vec, vec) <= ((float)CIRCLE_RADIUS + 0.5f) * ((float)CIRCLE_RADIUS + 0.5f))
				{
					tiles2D[currentPoint.x][currentPoint.y] = Tile2::Type::OneXOne;
				}
			}
		}
	}
}

int RoomGen::getNrTiles() const
{
	return (int)tiles.size();
}

int RoomGen::getNrBorders() const
{
	return (int)borders.size();
}

int RoomGen::getNrExitTiles() const
{
	return (int)exitPathsTiles.size();
}

const Tile2& RoomGen::getTile(int index) const
{
	return tiles[index];
}

const Tile2& RoomGen::getBorder(int index) const
{
	return borders[index];
}

const Tile2& RoomGen::getExitTiles(int index) const
{
	return exitPathsTiles[index];
}
