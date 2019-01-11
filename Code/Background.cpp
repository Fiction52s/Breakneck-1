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
	string paletteFile = string("Resources/") + bgStr + "_palette.png";
	string shapeFile = bgStr + "_shape.png";

	Tileset *ts_bg = owner->GetTileset(bgFile, 1920, 1080);
	Tileset *ts_shape = owner->GetTileset(shapeFile, 1920, 1080);
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

Background::Background(GameSession *owner, const string &bgName)
{
	stringstream ss;

	string folder = "Backgrounds/";

	//int eType = envLevel + 1; //adjust for alex naming -_-
	//ss << folder << "w" << envType + 1 << "_BG";
	ss << folder << bgName;

	//ss << eType;

	string bgStr = ss.str();

	string bgFile = bgStr + ".png";
	string paletteFile = string("Resources/") + bgStr + "_palette.png";
	string shapeFile = bgStr + "_shape.png";

	Tileset *ts_bg = owner->GetTileset(bgFile, 1920, 1080);
	Tileset *ts_shape = owner->GetTileset(shapeFile, 1920, 1080);
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

Background::Background(MainMenu *mm)
{
	stringstream ss;

	string folder = "Title/";

	//int eType = envLevel + 1; //adjust for alex naming -_-
	//ss << folder << "w" << envType + 1 << "_BG";

	//ss << eType;

	string bgStr = "Resources/Backgrounds/w1_BG1";// = ss.str();



	string bgFile = folder + "title_base_1920x1080.png";
	string paletteFile = bgStr + "_palette.png";
	string shapeFile = folder + "titleshade_1920x1080.png";//bgStr + "_shape.png";

	//Tileset *ts_bg = mm->tilesetManager.GetTileset(bgFile, 1920, 1080);
	Tileset *ts_shape = mm->tilesetManager.GetTileset(shapeFile, 1920, 1080);
	bool loadPalette = palette.loadFromFile(paletteFile);
	assert(loadPalette);

	//background.setTexture(*ts_bg->texture);
	//background.setOrigin(background.getLocalBounds().width / 2, background.getLocalBounds().height / 2);
	//background.setPosition(0, 0);

	SetRectCenter(backgroundSky, 1920, 1080, Vector2f(0, 0));

	shape.setTexture(*ts_shape->texture);
	shape.setOrigin(shape.getLocalBounds().width / 2, shape.getLocalBounds().height / 2);
	shape.setPosition(0, 0);

	transFrames = 60 * 3;

	bgView.setCenter(0, 0);
	bgView.setSize(1920, 1080);

	Reset();
}

void Background::UpdateSky()
{
	SetRectColor(backgroundSky, GetSkyColor() );
}

sf::Color Background::GetSkyColor()
{
	int ind = frame / transFrames;
	Color startColor = palette.getPixel(ind, 0);
	Color endColor;
	if (ind < 3)
	{
		endColor = palette.getPixel(ind + 1, 0);
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
	return blendColor;
}

sf::Color Background::GetShapeColor()
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
	return blendColor;
}

void Background::UpdateShape()
{
	shape.setColor(GetShapeColor());
}

void Background::Update()
{
	UpdateSky();
	UpdateShape();

	++frame;

	if (frame == transFrames * 4)
	{
		frame = 0;
	}
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