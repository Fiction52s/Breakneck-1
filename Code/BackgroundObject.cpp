#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"

using namespace sf;
using namespace std;

BackgroundObject::BackgroundObject(TilesetManager *p_tm, int p_loopWidth )
{
	tm = p_tm;
	depthLayer = -1;
	scrollSpeedX = 0;
	repetitionFactor = 1; //zero means never repeats, 1 is repeat every screen.
	numQuads = 0;
	ts = NULL;
	quads = NULL;
	loopWidth = p_loopWidth;
	Reset();
}

BackgroundObject::~BackgroundObject()
{
	if (quads != NULL)
	{
		delete[] quads;
	}
}

void BackgroundObject::Reset()
{
	action = 0;
	frame = 0;
	scrollOffset = 0;
}

void BackgroundObject::Load(std::ifstream &is )
{
	//is >> depthLayer;
	is >> scrollSpeedX;
	is >> repetitionFactor;
}

void BackgroundObject::ProcessAction()
{
	//empty
}

void BackgroundObject::UpdateQuads( float realX )
{
	IntRect sub = GetSubRect();
	if (repetitionFactor > 0)
	{
		for (int i = 0; i < numQuads; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(realX + loopWidth * i, myPos.y));
		}
	}
	else
	{
		for (int i = 0; i < numQuads; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(myPos));
		}
	}

	for (int i = 0; i < numQuads; ++i)
	{
		SetRectSubRect(quads + i * 4, sub);
	}
}

void BackgroundObject::Update(const sf::Vector2f &camPos)
{
	ProcessAction();

	if (repetitionFactor > 0)
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer) * .5f;
		//Vector2f extra(-loopWidth + myPos.x, 0);
		Vector2f extra(myPos.x, 0);

		Vector2f cPos = camPos;
		float testScrollOffset = extra.x + scrollOffset;
		cPos.x -= testScrollOffset;
		if (testScrollOffset * depth > loopWidth)
			testScrollOffset = 0;
		if (testScrollOffset * depth < -loopWidth)
		{
			testScrollOffset = 0;
		}

		float maxWidth = loopWidth * repetitionFactor;

		float camXAbs = abs(cPos.x * depth);
		int m = 0;
		while (camXAbs > maxWidth)
		{
			camXAbs -= maxWidth;
			++m;
		}

		float off = camXAbs;

		if (cPos.x > 0)
			off = -off;
		else if (cPos.x < 0)
		{
			off = off - maxWidth;
		}

		if (scrollOffset * depth > maxWidth)
			scrollOffset = 0;
		if (scrollOffset * depth < -maxWidth)
		{
			scrollOffset = 0;
		}
		scrollOffset += scrollSpeedX;// *updateFrames;

		float realX = (camPos.x + off) -1920 / 2;

		UpdateQuads(realX);
		
	}
	else
	{
		UpdateQuads(0);
	}

	++frame;
}

void BackgroundObject::Update(const sf::Vector2f &camPos, int numFrames)
{
	for (int i = 0; i < numFrames; ++i)
	{
		Update(camPos);
	}
}

void BackgroundObject::DrawObject(sf::RenderTarget *target)
{
	target->draw(quads, 4 * numQuads, sf::Quads, ts->texture);
}

void BackgroundObject::Draw(sf::RenderTarget *target)
{
	oldView = target->getView();

	if (repetitionFactor == 0)
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer);
		//the .5 is because the scrolling bgs do it, fix it soon
		Vector2f newCenter = Vector2f(oldView.getCenter().x * depth * .5f, 0);// -extraOffset;

		newView.setCenter(newCenter);
		newView.setSize(1920, 1080);

		target->setView(newView);

		DrawObject(target);
	}
	else
	{
		Vector2f newCenter = Vector2f(oldView.getCenter().x, 0);// -extraOffset;
		newView.setCenter(newCenter);
		newView.setSize(1920, 1080);
		target->setView(newView);

		//if (repetitionFactor == 1)
		{
			DrawObject(target);
		}
		/*else
		{
		target->draw(quads, 4, sf::Quads, ts->texture);
		}*/
	}

	target->setView(oldView);
}

void BackgroundObject::LayeredDraw(int p_depthLevel, sf::RenderTarget *target)
{
	if (p_depthLevel == depthLayer)
	{
		Draw(target);
	}
}

sf::IntRect BackgroundObject::GetSubRect()
{
	return IntRect(0, 0, 0, 0);
}

BackgroundTile::BackgroundTile(TilesetManager *p_tm, const std::string &p_folder, int p_loopWidth)
	:BackgroundObject( p_tm, p_loopWidth )
{
	folder = p_folder;
}

sf::IntRect BackgroundTile::GetSubRect()
{
	return subRect;
}

void BackgroundTile::Load(std::ifstream & is)
{
	string pngName;
	Vector2i tileSize;
	Vector2i tilePos;
	Vector2i gamePos;

	//is >> currLayer;

	BackgroundObject::Load(is);

	is >> pngName;

	is >> tileSize.x;
	is >> tileSize.y;

	is >> tilePos.x;
	is >> tilePos.y;

	is >> gamePos.x;
	is >> gamePos.y;

	




	/*is >> depthLayer;
	is >> scrollSpeedX;
	is >> repetitionFactor;*/
	BackgroundObject::Load(is);


	string tsPath = folder + pngName + ".png";

	ts = tm->GetTileset(tsPath);
	/*Vector2i pos;
	is >> pos.x;
	is >> pos.y;*/

	subRect.left = tilePos.x;
	subRect.top = tilePos.y;
	subRect.width = tileSize.x;
	subRect.height = tileSize.y;

	//gamePos.x -= 960;
	gamePos.y -= 540;

	myPos = Vector2f(gamePos);

	if (repetitionFactor > 0)
	{
		numQuads = 2;
	}
	else
	{
		numQuads = 1;
	}

	quads = new Vertex[4 * numQuads];

	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
		//SetRectTopLeft(quads + i * 4, ts->tileWidth, ts->tileHeight, Vector2f(pos));
	}
}



BackgroundWideSpread::BackgroundWideSpread(TilesetManager *p_tm, const std::string &p_folder, int p_loopWidth )
	:BackgroundObject(p_tm, p_loopWidth)
{
	folder = p_folder;
	extraWidth = 0;
}

sf::IntRect BackgroundWideSpread::GetSubRect()
{
	return subRect;
}

void BackgroundWideSpread::Load(std::ifstream & is)
{
	string pngName;
	Vector2i tileSize;
	Vector2i tilePos;
	Vector2i gamePos;

	//is >> currLayer;

	is >> extraWidth;

	BackgroundObject::Load(is);

	is >> pngName;

	is >> tileSize.x;
	is >> tileSize.y;

	is >> tilePos.x;
	is >> tilePos.y;

	is >> gamePos.x;
	is >> gamePos.y;






	/*is >> depthLayer;
	is >> scrollSpeedX;
	is >> repetitionFactor;*/
	//BackgroundObject::Load(is);


	string tsPath = folder + pngName + ".png";

	ts = tm->GetTileset(tsPath);
	/*Vector2i pos;
	is >> pos.x;
	is >> pos.y;*/

	subRect.left = tilePos.x;
	subRect.top = tilePos.y;
	subRect.width = tileSize.x;
	subRect.height = tileSize.y;

	//gamePos.x -= 960;
	gamePos.y -= 540;

	myPos = Vector2f(gamePos);

	if (repetitionFactor > 0)
	{
		numQuads = 2;
	}
	else
	{
		numQuads = 1;
	}

	numQuads *= 2;

	quads = new Vertex[4 * numQuads];

	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
		//SetRectTopLeft(quads + i * 4, ts->tileWidth, ts->tileHeight, Vector2f(pos));
	}
}

void BackgroundWideSpread::UpdateQuads(float realX)
{
	IntRect sub = GetSubRect();
	IntRect sub2 = sub;
	sub2.top += sub.height;
	sub2.width = extraWidth;

	if (repetitionFactor > 0)
	{
		for (int i = 0; i < 2; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub.width, sub.height, Vector2f(realX + loopWidth * i, myPos.y));
			SetRectSubRect(quads + i * 4, sub);
		}

		for (int i = 2; i < 4; ++i)
		{
			SetRectTopLeft(quads + i * 4, sub2.width, sub2.height, Vector2f(realX + loopWidth * (i - 2) + sub.width, myPos.y));
			SetRectSubRect(quads + i * 4, sub2);
		}
	}
	else
	{
		SetRectTopLeft(quads, sub.width, sub.height, Vector2f(myPos));
		SetRectSubRect(quads, sub);

		SetRectTopLeft(quads + 4, sub2.width, sub2.height, Vector2f(myPos.x + sub.width, myPos.y));
		SetRectSubRect(quads + 4, sub2);
	}
}


BackgroundWaterfall::BackgroundWaterfall( TilesetManager *p_tm, int p_loopWidth )
	:BackgroundObject( p_tm, p_loopWidth )
{
	ts = p_tm->GetSizedTileset("Backgrounds/W1/w1_01/waterfall_w4_128x320.png");

	actionLength[A_IDLE] = 12;

	animFactor[A_IDLE] = 3;

	quads = NULL;

	//ClearRect(quad);
}

BackgroundWaterfall::~BackgroundWaterfall()
{
}

void BackgroundWaterfall::Reset()
{
	BackgroundObject::Reset();
}

void BackgroundWaterfall::Load(std::ifstream &is)
{
	BackgroundObject::Load(is);

	Vector2i pos;
	is >> pos.x;
	is >> pos.y;

	pos.x -= 960;
	pos.y -= 540;

	myPos = Vector2f(pos);

	if (repetitionFactor > 0)
	{
		numQuads = 2;
	}
	else
	{
		numQuads = 1;
	}

	quads = new Vertex[4 * numQuads];



	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
		//SetRectTopLeft(quads + i * 4, ts->tileWidth, ts->tileHeight, Vector2f(pos));
	}
}

void BackgroundWaterfall::ProcessAction()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
	}
}

sf::IntRect BackgroundWaterfall::GetSubRect()
{
	return ts->GetSubRect(frame / animFactor[action]);
}