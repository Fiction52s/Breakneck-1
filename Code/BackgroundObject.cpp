#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"

using namespace sf;
using namespace std;

BackgroundObject::BackgroundObject(TilesetManager *p_tm)
{
	tm = p_tm;
	Reset();
}

BackgroundObject::~BackgroundObject()
{

}

void BackgroundObject::Reset()
{
	action = 0;
	frame = 0;
}

void BackgroundObject::Load(std::ifstream &is )
{
	is >> depthLayer;
	is >> scrollSpeedX;
}

void BackgroundObject::Update()
{

}

void BackgroundObject::Update(int numFrames)
{
	for (int i = 0; i < numFrames; ++i)
	{
		Update();
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

	ClearRect(quad);
}

//BackgroundWaterfall::~BackgroundWaterfall()
//{
//
//}

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

	SetRectTopLeft(quad, ts->tileWidth, ts->tileHeight, Vector2f(pos));
	//SetRectColor(quad, Color::Red);
}

void BackgroundWaterfall::Update()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
	}

	SetRectSubRect(quad, ts->GetSubRect( frame / animFactor[action]));

	++frame;
}

void BackgroundWaterfall::Draw(sf::RenderTarget *target)
{
	oldView = target->getView();
	//oldView.setCenter(target->getView().getCenter());
	//oldView.setSize(target->getView().getSize());
	float depth = DrawLayer::GetDrawLayerDepthFactor(depthLayer);

	//newView.setCenter(Vector2f( oldView.getCenter().x, 0) - extraOffset);
	//Vector2f newCenter = oldView.getCenter() *depth;

	//the .5 is because the scrolling bgs do it, fix it soon
	Vector2f newCenter = Vector2f(oldView.getCenter().x * depth * .5f, 0);// -extraOffset;
	/*if (oldView.getCenter().y > 0 )
	{
	newCenter.y = 0;
	}*/
	newView.setCenter(newCenter);//Vector2f(oldView.getCenter().x, oldView.getCenter().y) - extraOffset);
								 //newView.setSize(oldView.getSize() / depth);//1920, 1080) / (zoom * extraZoom ));
	newView.setSize(1920, 1080);//1920, 1080) / (zoom * extraZoom ));
								//newView.setRotation(oldView.getRotation());

	target->setView(newView);

	//target->draw(va, ts->texture);
	//target->draw(quad, 8, sf::Quads, &parallaxShader);
	target->draw(quad, 4, sf::Quads, ts->texture);

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