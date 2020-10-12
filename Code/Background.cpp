#include "Background.h"
#include "GameSession.h"
#include <sstream>
#include <assert.h>
#include <random>
#include "MainMenu.h"

using namespace std;
using namespace sf;

ScrollingBackground::ScrollingBackground(Tileset *p_ts, int index,
	int p_depthLevel, float p_scrollSpeedX)
	:ts(p_ts), va(sf::Quads, 2 * 4), depthLevel(p_depthLevel),
	depth(p_depthLevel * .01f), tsIndex(index), scrollSpeedX(p_scrollSpeedX)
{
	assert(p_ts != NULL);
	tsSource = ts->sourceName;
	SetTileIndex(tsIndex);
	SetLeftPos(Vector2f(0, 0));
	scrollOffset = 0;
	extraZoom = 1.f;
	extraOffset = Vector2f(0, 0);
}

ScrollingBackground::~ScrollingBackground()
{

}

void ScrollingBackground::Update(const Vector2f &camPos, int updateFrames )
{
	Vector2f cPos = camPos;
	cPos.x -= scrollOffset;
	if (scrollOffset * depth > 1920)
		scrollOffset = 0;
	if (scrollOffset * depth < -1920)
	{
		scrollOffset = 0;
	}
	int repeatWidth = 1920 * 2;
	int p = floor(cPos.x * depth + .5f);

	int px = abs(p);
	int pxx = (px % repeatWidth) - repeatWidth / 2;

	float camXAbs = abs(cPos.x * depth);
	int m = 0;
	while (camXAbs > 1920)
	{
		camXAbs -= 1920;
		++m;
	}

	float off = camXAbs;

	if (cPos.x > 0)
		off = -off;
	else if (cPos.x < 0)
	{
		off = off - 1920;
	}

	//cout << "off: " << off << endl;

	//if( p > 0 )
	//	off = -off;

	if (p > 0)
		pxx = -pxx;


	scrollOffset += scrollSpeedX * updateFrames;

	//cout << "pxx: " << pxx << ", modified:  " << (pxx / .5f) << endl;
	//pxx = floor( pxx * .5 + .5 );

	//Vector2f realPos( camPos.x + pxx, camPos.y + relPos.y );
	Vector2f realPos(camPos.x + off, camPos.y);
	//realPos = camPos;
	realPos.x -= 960;
	SetLeftPos(realPos);//camPos.x );//realPos.x );
						//	cout << "set real pos: " << realPos.x << endl;
}

void ScrollingBackground::SetLeftPos(Vector2f &pos)
{
	xPos = pos.x;
	float currX = pos.x;
	float top = -540;//pos.y - 540;
	float width = 1920;
	float height = 1080;
	for (int i = 0; i < 2; ++i)
	{
		va[i * 4 + 0].position = Vector2f(currX, top);
		va[i * 4 + 1].position = Vector2f(currX + width, top);
		va[i * 4 + 2].position = Vector2f(currX + width, top + height);
		va[i * 4 + 3].position = Vector2f(currX, top + height);

		currX += width;
	}
}

void ScrollingBackground::SetTileIndex(int index)
{
	tsIndex = index;
	IntRect subRect = ts->GetSubRect(tsIndex);
	for (int i = 0; i < 2; ++i)
	{
		va[i * 4 + 0].texCoords = Vector2f(subRect.left, subRect.top);
		va[i * 4 + 1].texCoords = Vector2f(subRect.left + subRect.width, subRect.top);
		va[i * 4 + 2].texCoords = Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
		va[i * 4 + 3].texCoords = Vector2f(subRect.left, subRect.top + subRect.height);
	}
}

void ScrollingBackground::Set(sf::Vector2f &pos, float zoom)
{
	extraOffset = pos;
	extraZoom = zoom;
}

void ScrollingBackground::Draw(RenderTarget *target)
{
	oldView.setCenter(target->getView().getCenter());
	oldView.setSize(target->getView().getSize());

	newView.setCenter(Vector2f( oldView.getCenter().x, 0) - extraOffset);
	newView.setSize(Vector2f(1920, 1080) / extraZoom );
	
	target->setView(newView);

	target->draw(va, ts->texture);

	target->setView(oldView);
}

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

	ts_bg = owner->GetTileset(bgFile, 1920, 1080);
	ts_shape = owner->GetTileset(shapeFile, 1920, 1080);
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

	show = true;

	deleteTilesets = true;

	Reset();
}

string Background::GetBGNameFromBGInfo(const std::string &fileName)
{
	ifstream is;
	stringstream fss;
	fss << "Resources/BGInfo/" << fileName << ".bg";
	string fStr = fss.str();

	is.open(fStr);

	if (is.is_open())
	{
		string bgStr;
		is >> bgStr;

		is.close();

		return bgStr;
		//background = new Background(tm, bgStr);
	}
	else
	{
		return string("bg error");
	}

		//background = new Background(this, mh->envLevel, mh->envType);
}




Background *Background::SetupFullBG(const std::string &fName,
	TilesetManager *tm, bool p_deleteTilesets )
{
	ifstream is;
	stringstream fss;
	fss << "Resources/BGInfo/" << fName << ".bg";
	string fStr = fss.str();
	string eStr = ".png";
	string parDirStr = "Parallax/";

	try {
		is.open(fStr);
	}
	catch (std::system_error &e)
	{
		return NULL;
		//cout << "blizzle: " << e.code().message() << endl;
		//assert(0);
	}

	Background *newBG = NULL;
	if (is.is_open())
	{
		string bgStr;
		is >> bgStr;

		newBG = new Background(tm, bgStr);

		newBG->deleteTilesets = p_deleteTilesets;

		int numPar;
		is >> numPar;

		string pStr;
		int tsIndex;
		int depthLevel;
		float scrollSpeed;
		for (int i = 0; i < numPar; ++i)
		{
			is >> pStr;

			is >> tsIndex;

			is >> depthLevel;

			is >> scrollSpeed;

			newBG->scrollingBackgrounds.push_back(
				new ScrollingBackground(
					tm->GetTileset(parDirStr + pStr + eStr, 1920, 1080), tsIndex, depthLevel, scrollSpeed));
		}

		is.close();
	}
	
	return newBG;
}

Background::Background(TilesetManager *p_tm, const string &bgName)
	:tm(p_tm)
{
	stringstream ss;

	string folder = "Backgrounds/";

	name = bgName;

	//int eType = envLevel + 1; //adjust for alex naming -_-
	//ss << folder << "w" << envType + 1 << "_BG";
	ss << folder << bgName;

	//ss << eType;

	string bgStr = ss.str();

	bgSourceName = bgStr + ".png";
	string paletteFile = string("Resources/") + bgStr + "_palette.png";
	shapeSourceName = bgStr + "_shape.png";

	ts_bg = tm->GetTileset(bgSourceName, 1920, 1080);
	ts_shape = tm->GetTileset(shapeSourceName, 1920, 1080);

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

	show = true;

	Reset();
}

Background::Background(MainMenu *mm)
{
	stringstream ss;

	string folder = "Title/";

	//int eType = envLevel + 1; //adjust for alex naming -_-
	//ss << folder << "w" << envType + 1 << "_BG";

	show = true;
	//ss << eType;

	string bgStr = "Resources/Backgrounds/w1_BG1";// = ss.str();



	bgSourceName = folder + "title_base_1920x1080.png";
	string paletteFile = bgStr + "_palette.png";
	shapeSourceName = folder + "titleshade_1920x1080.png";//bgStr + "_shape.png";

	ts_bg = NULL;
	ts_shape = mm->tilesetManager.GetTileset(shapeSourceName, 1920, 1080);
	bool loadPalette = palette.loadFromFile(paletteFile);
	assert(loadPalette);

	tm = &mm->tilesetManager;

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

	show = true;

	Reset();
}

Background::~Background()
{
	if (deleteTilesets)
	{
		if (ts_bg != NULL)
		{
			tm->DestroyTilesetIfExists(bgSourceName);
		}
		if (ts_shape != NULL)
		{
			tm->DestroyTilesetIfExists(shapeSourceName);
		}
	}

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		if (deleteTilesets)
		{
			tm->DestroyTilesetIfExists((*it)->tsSource);
		}
		
		delete (*it);
	}
}

void Background::Set(Vector2f &pos, float zoom )
{
	bgView.setCenter(-pos);
	bgView.setSize(1920 / zoom, 1080 / zoom);

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Set(pos, zoom);
	}
}

void Background::DestroyTilesets()
{
	tm->DestroyTileset(ts_bg);
	tm->DestroyTileset(ts_shape);

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		tm->DestroyTileset((*it)->ts);
	}
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

void Background::Update( const Vector2f &camPos, int updateFrames )
{
	UpdateSky();
	UpdateShape();

	frame += updateFrames;

	if (frame >= transFrames * 4)
	{
		frame = frame % (transFrames * 4);
	}

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Update(camPos, updateFrames );
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
	if (!show)
		return;

	sf::View oldView = target->getView();
	target->setView(bgView);

	target->draw(backgroundSky, 4, sf::Quads);
	target->draw(shape);
	target->draw(background);

	target->setView(oldView);

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		//might not need to mess w/ views here anymore
		(*it)->Draw(target);
	}

	
}

void Background::Show()
{
	show = true;
}

void Background::Hide()
{
	show = false;
}

void Background::FlipShown()
{
	show = !show;
}