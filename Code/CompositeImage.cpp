#include "CompositeImage.h"
#include "VectorMath.h"

using namespace std;
using namespace sf;


CompositeImage::CompositeImage( TilesetManager *tm, const std::string &baseName, int p_numImages, sf::Vector2f p_tileSize, sf::Vector2f p_gridReps)
	:numImages( p_numImages ), tileSize( p_tileSize ), gridReps( p_gridReps )
{
	tilesets = new Tileset*[numImages];
	quads = new Vertex[numImages * 4];

	//assumes no triple digits
	string base = baseName + "_";;//"Ship/ShipTest/traveltest_1024x1024_";
	for (int i = 0; i < numImages; ++i)
	{
		string curr = base;
		if (i < 10)
		{
			curr += "0" + to_string(i) + ".png";
		}
		else
		{
			curr += to_string(i) + ".png";
		}

		tilesets[i] = tm->GetTileset(curr);
		SetRectSubRect(quads + i * 4, tilesets[0]->GetSubRect(0));
	}
}

CompositeImage::~CompositeImage()
{
	delete[] tilesets;
	delete[] quads;
}

void CompositeImage::SetTopLeft(sf::Vector2f topLeft)
{
	int ind = 0;
	for (int y = 0; y < gridReps.y; ++y)
	{
		for (int x = 0; x < gridReps.x; ++x)
		{
			ind = y * 4 + x;
			SetRectTopLeft(quads + ind * 4, tileSize.x, tileSize.y, topLeft + Vector2f(x * tileSize.x, y * tileSize.y));
		}
	}
}

void CompositeImage::SetCenter(sf::Vector2f center)
{
	Vector2f topLeft = center - Vector2f(gridReps.x / 2 * tileSize.x, gridReps.y / 2 * tileSize.y);
	SetTopLeft(topLeft);
}

void CompositeImage::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < numImages; ++i)
	{
		target->draw(quads + i * 4, 4, sf::Quads, tilesets[i]->texture);
	}
}

