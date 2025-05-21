#include "RecordedAnimation.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include "nlohmann\json.hpp"
#include <fstream>
#include <iostream>
#include "VectorMath.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;
using namespace nlohmann;

RecordedAnimation::RecordedAnimation(TilesetManager *tm, const std::string &folder, const std::string &name )
{
	ifstream is;
	is.open("Resources/" + folder + "/" + name + ".json");

	assert(is.is_open());

	json j;
	is >> j;

	totalFrames = j["totalFrames"];
	recordSize = Vector2f(j["recordSizeX"], j["recordSizeY"]);
	numTilesets = j["numImages"];
	scale = j["scale"];

	is.close();

	tilesetVec.resize(numTilesets);

	stringstream ss;

	for (int i = 0; i < numTilesets; ++i)
	{
		ss.str("");
		ss.clear();

		ss << folder << "/" << name << "_" << i << ".png";
		tilesetVec[i] = tm->GetTileset(ss.str(), recordSize.x * scale, recordSize.y * scale);
	}
	displayScale = 1.f;
	animFactor = 4;

	Reset();
}

RecordedAnimation::~RecordedAnimation()
{
}

void RecordedAnimation::Reset()
{
	frame = 0;
	ClearRect(quad);
	SetTopLeft(topLeft);
	ts_curr = NULL;

	UpdateQuad();
}

void RecordedAnimation::SetCenter(sf::Vector2f p)
{
	topLeft = Vector2f(p.x - recordSize.x * scale * .5 * displayScale, p.y - recordSize.y * scale * .5 * displayScale);
	SetRectCenter(quad, recordSize.x * scale * displayScale, recordSize.y * scale * displayScale, p);
}

void RecordedAnimation::SetTopLeft(sf::Vector2f p)
{
	topLeft = p;
	SetRectTopLeft(quad, recordSize.x * scale * displayScale, recordSize.y * scale * displayScale, topLeft);
}

void RecordedAnimation::SetScale(float s)
{
	displayScale = s;
}

void RecordedAnimation::Update()
{
	if (frame == totalFrames * animFactor)
	{
		frame = 0;
	}

	UpdateQuad();

	++frame;
}

void RecordedAnimation::UpdateQuad()
{
	int numTileX = 2048.f / (recordSize.x * scale);
	int numTileY = 2048.f / (recordSize.y * scale);
	int numFramesPerImage = numTileX * numTileY;

	int tilesetIndex = (frame / animFactor) / numFramesPerImage;
	ts_curr = tilesetVec[tilesetIndex];

	int currTile = (frame / animFactor) % numFramesPerImage;

	cout << "tilesetIndex: " << tilesetIndex << ", tile: " << currTile << "\n";

	SetRectSubRect(quad, ts_curr->GetSubRect(currTile));
}

void RecordedAnimation::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads, ts_curr->texture);
}