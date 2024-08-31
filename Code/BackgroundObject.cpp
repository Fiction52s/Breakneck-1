#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"

using namespace sf;
using namespace std;

BackgroundObject::BackgroundObject(TilesetManager *p_tm)
{
	tm = p_tm;
	depthLayer = -1;
	scrollSpeedX = 0;
	repetitionFactor = 1; //zero means never repeats, 1 is repeat every screen.
	Reset();
}

BackgroundObject::~BackgroundObject()
{

}

void BackgroundObject::Reset()
{
	action = 0;
	frame = 0;
	scrollOffset = 0;
}

void BackgroundObject::Load(std::ifstream &is )
{
	is >> depthLayer;
	is >> scrollSpeedX;
	is >> repetitionFactor;
}

void BackgroundObject::Update(const sf::Vector2f &camPos)
{
}

void BackgroundObject::Update(const sf::Vector2f &camPos, int numFrames)
{
	for (int i = 0; i < numFrames; ++i)
	{
		Update(camPos);
	}
}

void BackgroundObject::Draw(sf::RenderTarget *target)
{
	//target->draw(quad, 4, sf::Quads);
}

void BackgroundObject::LayeredDraw(int p_depthLevel, sf::RenderTarget *target)
{

}



BackgroundWaterfall::BackgroundWaterfall( TilesetManager *p_tm)
	:BackgroundObject( p_tm )
{
	ts = p_tm->GetSizedTileset("Backgrounds/W1/w1_01/waterfall_w4_128x320.png");

	actionLength[A_IDLE] = 12;

	animFactor[A_IDLE] = 3;

	quads = NULL;

	//ClearRect(quad);
}

BackgroundWaterfall::~BackgroundWaterfall()
{
	if (quads != NULL)
	{
		delete[] quads;
	}
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
		quads = new Vertex[4 * 2];
		SetRectTopLeft(quads, ts->tileWidth, ts->tileHeight, Vector2f(pos));
		SetRectTopLeft(quads + 4, ts->tileWidth, ts->tileHeight, Vector2f(pos));
	}
	else
	{
		quads = new Vertex[4];

		SetRectTopLeft(quads, ts->tileWidth, ts->tileHeight, Vector2f(pos));
	}
	//SetRectColor(quad, Color::Red);
}

void BackgroundWaterfall::Update(const sf::Vector2f &camPos)
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
	}

	if (repetitionFactor > 0)
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer) * .5f;
		Vector2f extra(-1920 + myPos.x, 0);

		Vector2f cPos = camPos;
		float testScrollOffset = extra.x + scrollOffset;
		cPos.x -= testScrollOffset;
		if (testScrollOffset * depth > 1920)
			testScrollOffset = 0;
		if (testScrollOffset * depth < -1920)
		{
			testScrollOffset = 0;
		}

		float maxWidth = 1920 * repetitionFactor;

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


		Vector2f realPos(camPos.x + off, camPos.y);
		//realPos = camPos;
		realPos.x -= 960;

		//realPos.x += myPos.x;

		for (int i = 0; i < 2; ++i)
		{
			SetRectTopLeft(quads + i * 4, ts->tileWidth, ts->tileHeight, Vector2f(realPos.x + 1920 * i, myPos.y));
		}
	}


	SetRectSubRect(quads, ts->GetSubRect(frame / animFactor[action]));
	if (repetitionFactor > 0 )//== 1)
	{
		SetRectSubRect(quads + 4, ts->GetSubRect(frame / animFactor[action]));
	}

	++frame;
}

void BackgroundWaterfall::Draw(sf::RenderTarget *target)
{
	oldView = target->getView();

	if (repetitionFactor == 0 )
	{
		float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer);
		//the .5 is because the scrolling bgs do it, fix it soon
		Vector2f newCenter = Vector2f(oldView.getCenter().x * depth * .5f, 0);// -extraOffset;

		newView.setCenter(newCenter);
		newView.setSize(1920, 1080);

		target->setView(newView);

		target->draw(quads, 4, sf::Quads, ts->texture);
	}
	else
	{
		Vector2f newCenter = Vector2f(oldView.getCenter().x, 0);// -extraOffset;
		newView.setCenter(newCenter);
		newView.setSize(1920, 1080);
		target->setView(newView);

		//if (repetitionFactor == 1)
		{
			target->draw(quads, 8, sf::Quads, ts->texture);
		}
		/*else
		{
			target->draw(quads, 4, sf::Quads, ts->texture);
		}*/
	}
	
	target->setView(oldView);

	//target->draw(quad, 4, sf::Quads, ts->texture);
}

void BackgroundWaterfall::LayeredDraw(int p_depthLevel, sf::RenderTarget *target)
{
	if (p_depthLevel == depthLayer)
	{
		Draw(target);
	}
}