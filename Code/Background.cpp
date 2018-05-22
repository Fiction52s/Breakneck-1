#include "Background.h"
#include "GameSession.h"
#include <sstream>
#include <assert.h>
#include <random>

using namespace std;
using namespace sf;



Background::Background( GameSession *owner, int envLevel, int envType)
{
	stringstream ss;

	string folder = "Backgrounds/";

	int eType = envLevel + 1; //adjust for alex naming -_-
	ss << folder << "w" << envType + 1 << "_BG";

	ss << eType;

	string bgStr = ss.str();

	

	string bgFile = bgStr + ".png";
	string paletteFile = bgStr + "_palette.png";
	string shapeFile = bgStr + "_shape.png";

	Tileset *ts_bg = owner->GetTileset(bgFile, 1920, 1080);
	Tileset *ts_shape = owner->GetTileset(shapeFile, 1920, 1080);
	Tileset *ts_palette = owner->GetTileset(paletteFile, 1, 1);
	//Image im(rtt->getTexture().copyToImage());
	bool loadPalette = palette.loadFromFile(paletteFile);
	assert(loadPalette);

	background.setTexture(*ts_bg->texture);
	background.setOrigin(background.getLocalBounds().width / 2, background.getLocalBounds().height / 2);
	background.setPosition(0, 0);

	SetRectCenter(backgroundSky, 1920, 1080, Vector2f(0, 0));
	
	shape.setTexture(*ts_shape->texture);
	shape.setOrigin(shape.getLocalBounds().width / 2, shape.getLocalBounds().height / 2);
	shape.setPosition(0, 0);

	transFrames = 60 * 20;

	bgView.setCenter(0, 0);
	bgView.setSize(1920, 1080);

	Reset();
}

void Background::UpdateSky()
{
	int ind = frame / transFrames;
	Color startColor = palette.getPixel( ind, 0 );
	Color endColor;
	if (ind < 3)
	{
		endColor = palette.getPixel(ind+1, 0);
	}
	else
	{
		endColor = palette.getPixel(0, 0);
	}

	float factor = (frame%transFrames) / (float)transFrames;
	float recip = 1.f - factor;
	Color blendColor;
	blendColor.r = startColor.r * recip + endColor.r * factor;
	blendColor.g = startColor.g * recip + endColor.g * factor;
	blendColor.b = startColor.b * recip + endColor.b * factor;

	SetRectColor(backgroundSky, blendColor);
}

void Background::UpdateShape()
{
	int ind = frame / transFrames;
	Color startColor = palette.getPixel(ind, 1);
	Color endColor;
	if (ind < 3)
	{
		endColor = palette.getPixel(ind + 1, 1);
	}
	else
	{
		endColor = palette.getPixel(0, 1);
	}

	float factor = (frame%transFrames) / (float)transFrames;
	float recip = 1.f - factor;
	Color blendColor;
	blendColor.r = startColor.r * recip + endColor.r * factor;
	blendColor.g = startColor.g * recip + endColor.g * factor;
	blendColor.b = startColor.b * recip + endColor.b * factor;

	shape.setColor(blendColor);
}

void Background::Update()
{
	if (frame == transFrames * 4)
	{
		frame = 0;
	}

	UpdateSky();
	UpdateShape();

	++frame;
}

void Background::Reset()
{
	int r = rand() % 4;
	int r2 = rand() % transFrames;
	frame = r * transFrames + r2;
}

void Background::Draw(sf::RenderTarget *target)
{
	sf::View oldView = target->getView();
	target->setView(bgView);

	target->draw(backgroundSky, 4, sf::Quads);
	target->draw(shape);
	target->draw(background);

	target->setView(oldView);
	//target->setView(bgView);
	//target->
}