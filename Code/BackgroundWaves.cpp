#include "BackgroundObject.h"
#include "VectorMath.h"
#include "Tileset.h"
#include "DrawLayer.h"
#include <iostream>
#include "Background.h" 
#include "Session.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

BackgroundWaves::BackgroundWaves(Background *p_bg, int p_layer)
	:BackgroundObject(p_bg, p_layer)
{
	ts_0 = bg->GetSizedTileset("Backgrounds/W4/w4_01/waves_1_1920x128.png"); //16 frames
	ts_1 = bg->GetSizedTileset("Backgrounds/W4/w4_01/waves_2_1920x128.png"); //3 frames

	ts = ts_0;//bg->GetSizedTileset("Backgrounds/W1/w1_01/waterfall_w4_128x320.png");

	actionLength[A_IDLE] = 19;

	animFactor[A_IDLE] = 12;

	quads = NULL;

	scrollSpeedX = 0;

	repetitionFactor = 1;

	loopWidth = 1920;
	//ClearRect(quad);
}

BackgroundWaves::~BackgroundWaves()
{
}

void BackgroundWaves::Load(nlohmann::basic_json<> &jobj)
{
	Vector2i bgPos;

	bgPos.x = jobj["bgPos"][0];
	bgPos.y = jobj["bgPos"][1];

	if (repetitionFactor == 0)
	{
		bgPos.x -= 960;
	}

	bgPos.y -= 540;

	myPos = Vector2f(bgPos);

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
	}
}

void BackgroundWaves::ProcessAction()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
	}

	if (frame / animFactor[action] >= 16)
	{
		ts = ts_1;
	}
	else
	{
		ts = ts_0;
	}
}

sf::IntRect BackgroundWaves::GetSubRect()
{
	int f = frame / animFactor[action];
	if (f >= 16)
	{
		f -= 16;
	}

	return ts->GetSubRect(f);
}