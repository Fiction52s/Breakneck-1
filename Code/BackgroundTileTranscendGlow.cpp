#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"
#include <iostream>

using namespace sf;
using namespace std;

BackgroundTileTranscendGlow::BackgroundTileTranscendGlow(Background *p_bg, const std::string &p_folder, int p_layer)
	:BackgroundTile( p_bg, p_folder, p_layer )
{
	folder = p_folder;
}


BackgroundTileTranscendGlow::~BackgroundTileTranscendGlow()
{

}

void BackgroundTileTranscendGlow::UpdateQuads(float realX)
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

	int breatheFrames = 30;
	int fullBreatheFrames = breatheFrames * 2;
	float breathe = (float)(frame % fullBreatheFrames) / fullBreatheFrames;
	breathe *= 2.f;

	if (breathe > 1.0)
	{
		breathe = 2.f - breathe;
	}

	for (int i = 0; i < numQuads; ++i)
	{
		SetRectSubRect(quads + i * 4, sub);
		SetRectColor(quads + i * 4, Color(255, 255, 255, 255.f * breathe));
	}

	for (int i = 0; i < numQuads; ++i)
	{
		ClearRect(quads + i * 4);
		//SetRectSubRect(quads + i * 4, sub);
		//SetRectColor(quads + i * 4, Color(255, 255, 255, 255.f * breathe));
	}
}

void BackgroundTileTranscendGlow::ProcessAction()
{

}

sf::IntRect BackgroundTileTranscendGlow::GetSubRect()
{
	return subRect;
}