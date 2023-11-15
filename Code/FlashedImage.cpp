#include "Sequence.h"
#include "Session.h"
#include <iostream>

using namespace std;
using namespace sf;



FlashedImage::FlashedImage(Tileset *ts,
	int tileIndex, int appearFrames,
	int holdFrames,
	int disappearFrames,
	sf::Vector2f &pos)
{
	ts_image = ts;
	ts_split = NULL;
	ts_splitBorder = NULL;
	splitShader = NULL;
	bg = NULL;
	bgSplitShader = NULL;
	currBGTileset = NULL;

	SetRectSubRect(spr, ts->GetSubRect(tileIndex));
	SetRectColor(spr, Color(Color::White));
	//spr.setTexture(*ts->texture);
	//spr.setTextureRect();
	//spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

	origPos = pos;

	Reset();

	aFrames = appearFrames;
	if (holdFrames <= 0)
	{
		infiniteHoldEarlyEnd = -holdFrames;
		infiniteHold = true;
		holdFrames = 10000000;
	}
	else
	{
		infiniteHold = false;
	}
	hFrames = holdFrames;
	dFrames = disappearFrames;


	//splitShader.setUniform("texture", *ts_image->texture);

	//splitShader.setUniform("toColor", ColorGL(keyColor));//Glsl::Vec4( keyColor.r, keyColor.g, keyColor.b, keyColor.a ) );
	//splitShader.setUniform("auraColor", ColorGL(auraColor));//Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a) );
}



FlashedImage::~FlashedImage()
{
	for (auto it = panList.begin(); it != panList.end(); ++it)
	{
		delete (*it);
	}

	if (splitShader != NULL)
		delete splitShader;

	if (bgSplitShader != NULL)
		delete bgSplitShader;
}

void FlashedImage::SetBG(SceneBG * p_bg)
{
	bg = p_bg;
	if (bg != NULL)
	{
		if (splitShader != NULL)
		{
			bgSplitShader = new Shader;

			if (!bgSplitShader->loadFromFile("Resources/Shader/split.frag", sf::Shader::Fragment))
			{
				cout << "couldnt load enemy split shader" << endl;
				assert(false);
			}

			bgSplitShader->setUniform("u_splitTexture", *ts_split->texture);
			bgSplitShader->setUniform("offset", Vector2f(0, 0));
			bgSplitShader->setUniform("currAlpha", 1.f);

			UpdateBG();

			//tileindex might be different later
			SetRectSubRectGL(split, currBGTileset->GetSubRect(0), Vector2f(currBGTileset->texture->getSize()));
		}
		else
		{

		}
	}
}

void FlashedImage::SetSplit(Tileset *ts, Tileset *borderTS, int tileIndex, sf::Vector2f &pos)
{
	assert(splitShader == NULL);

	ts_split = ts;
	ts_splitBorder = borderTS;

	//shares the same tile index and position of the split

	if (ts_splitBorder != NULL)
	{
		SetRectSubRect(splitBorder, borderTS->GetSubRect(tileIndex));
		SetRectCenter(splitBorder, ts->tileWidth, ts->tileHeight, pos);
	}


	SetRectSubRectGL(spr, ts->GetSubRect(tileIndex), Vector2f(ts->texture->getSize()));

	splitSize = ts_split->texture->getSize();
	SetRectSubRect(split, ts->GetSubRect(tileIndex));
	SetRectCenter(split, ts->tileWidth, ts->tileHeight, pos);

	splitShader = new Shader;


	if (!splitShader->loadFromFile("Resources/Shader/split.frag", sf::Shader::Fragment))
	{
		cout << "couldnt load enemy split shader" << endl;
		assert(false);
	}

	splitShader->setUniform("u_splitTexture", *ts_split->texture);
	splitShader->setUniform("u_texture", *ts_image->texture);
	splitShader->setUniform("offset", Vector2f(0, 0));
	splitShader->setUniform("currAlpha", 1.f);
}

bool FlashedImage::IsDone()
{
	return (!flashing && frame > 0);
}

int FlashedImage::GetNumFrames()
{
	return aFrames + hFrames + dFrames;
}

void FlashedImage::Reset()
{
	frame = 0;
	flashing = false;
	position = origPos;
	SetRectCenter(spr, ts_image->tileWidth, ts_image->tileHeight, position);
	currPan = NULL;

	UpdateBG();
}

void FlashedImage::AddPan(sf::Vector2f &diff,
	int startFrame, int frameLength)
{
	panList.push_back(new PanInfo(position, diff, startFrame, frameLength));
}

void FlashedImage::AddPanX(float xDiff,
	int startFrame, int frameLength)
{
	panList.push_back(new PanInfo(position, Vector2f(xDiff, 0), startFrame, frameLength));
}

void FlashedImage::AddPanY(float yDiff,
	int startFrame, int frameLength)
{
	panList.push_back(new PanInfo(position, Vector2f(0, yDiff), startFrame, frameLength));
}

void FlashedImage::Flash()
{
	flashing = true;
	frame = 0;
	SetRectColor(spr, Color(255, 255, 255, 0));
	if (splitShader != NULL)
		splitShader->setUniform("currAlpha", 0.f);
}

bool FlashedImage::IsFadingIn()
{
	return (flashing && frame < aFrames);
}

bool FlashedImage::IsHolding()
{
	return (flashing && frame >= aFrames && frame < aFrames + hFrames);
}

int FlashedImage::GetFramesUntilDone()
{
	if (!flashing)
	{
		return 0;
	}
	else
	{
		int totalFrames = aFrames + hFrames + dFrames;
		return (totalFrames - 1) - frame;
	}
}

void FlashedImage::StopHolding()
{
	frame = aFrames + hFrames;
}

void FlashedImage::Update()
{
	if (!flashing)
		return;

	for (auto it = panList.begin(); it != panList.end(); ++it)
	{
		if ((*it)->startFrame == frame)
		{
			currPan = (*it);
		}
	}

	int a = 0;
	if (IsFadingIn())
	{
		a = (frame / (float)aFrames) * 255.f;
	}
	else if (IsHolding())
	{
		a = 255;
	}
	else //fading out
	{
		int fr = frame - (aFrames + hFrames);
		a = (1.f - fr / (float)dFrames) * 255.f;
	}
	SetRectColor(spr, Color(255, 255, 255, a));
	if (splitShader != NULL)
	{
		float aColor = a / 255.f;
		splitShader->setUniform("currAlpha", aColor);
	}


	if (frame == aFrames + hFrames + dFrames)
	{
		flashing = false;
	}

	float tStep = TIMESTEP;

	if (currPan != NULL)
	{
		int fr = frame - currPan->startFrame;
		if (fr == currPan->frameLength)
		{
			currPan = NULL;
		}
		else
		{
			position = currPan->GetCurrPos(frame);
			SetRectCenter(spr, ts_image->tileWidth, ts_image->tileHeight, position);

			if (splitShader != NULL)
			{
				Vector2f offset = position - origPos;


				offset.x /= splitSize.x;
				offset.y /= splitSize.y;

				splitShader->setUniform("offset", offset);
			}
		}
	}

	UpdateBG();


	++frame;
}

void FlashedImage::UpdateBG()
{
	if (bg != NULL)
	{
		Tileset *bgTS = bg->GetCurrTileset(frame);
		if (currBGTileset != bgTS)
		{
			currBGTileset = bgTS;
			bgSplitShader->setUniform("u_texture", *currBGTileset->texture);
		}

	}
}

void FlashedImage::Draw(sf::RenderTarget *target)
{
	if (flashing)
	{
		if (ts_split != NULL)
		{
			if (bg != NULL)
			{
				target->draw(split, 4, sf::Quads, bgSplitShader);
			}
			else
			{
				target->draw(split, 4, sf::Quads, ts_split->texture);
			}

			target->draw(spr, 4, sf::Quads, splitShader);

			if (ts_splitBorder != NULL)
			{
				target->draw(splitBorder, 4, sf::Quads, ts_splitBorder->texture);
			}
		}
		else
		{
			target->draw(spr, 4, sf::Quads, ts_image->texture);
		}
	}

}