#include "Fader.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "KinBoostScreen.h"
#include <sstream>

using namespace std;
using namespace sf;

Fader::Fader()
{
	Reset();
	fadeRect.setSize(Vector2f(1920, 1080));
	currLayer = EffectLayer::IN_FRONT_OF_UI;
}

void Fader::Fade(bool in, int frames, sf::Color c, bool skipKin, int layer)
{
	if (in)
	{
		fadeLength = frames;
		fadingIn = true;
		fadingOut = false;
	}
	else
	{
		fadeLength = frames;
		fadingIn = false;
		fadingOut = true;
	}
	fadeRect.setFillColor(Color(c.r, c.g, c.b, 255));

	fadeFrame = 0;

	fadeSkipKin = skipKin;

	currLayer = layer;
}

void Fader::CrossFade(int fadeOutFrames, int pauseFrames, int fadeInFrames, sf::Color c, bool skipKin)
{
	crossFadeOutLength = fadeOutFrames;
	crossFadeInLength = fadeInFrames;
	crossPauseLength = pauseFrames;
	crossSkipKin = skipKin;
	crossColor = c;
	crossFading = true;
	Fade(false, fadeOutFrames, c, skipKin, EffectLayer::IN_FRONT_OF_UI);
}

bool Fader::IsFullyFadedOut()
{
	return fadeAlpha == 255;
}

void Fader::Reset()
{
	fadeLength = 0;
	crossFading = false;
	fadeAlpha = 0;
	fadingIn = false;
	fadingOut = false;
}

bool Fader::IsFading()
{
	return fadingIn || fadingOut;
}

void Fader::Clear()
{
	fadingIn = false;
	fadingOut = false;
	fadeAlpha = 0;
	fadeLength = 0;
	Color oldColor = fadeRect.getFillColor();
	fadeRect.setFillColor(Color(oldColor.r, oldColor.g, oldColor.b, fadeAlpha));
}

void Fader::Update()
{
	if (!fadingIn && !fadingOut)
		return;
	//cout << "fade frame: " << fadeFrame << endl;

	++fadeFrame;

	if (fadeFrame > fadeLength && fadeLength > 0)
	{
		if (crossFading)
		{
			if ( fadingOut && fadeFrame > fadeLength + crossPauseLength)
			{
				Fade(true, crossFadeInLength, crossColor, crossSkipKin, currLayer);
			}
			else if( fadingIn )
			{
				crossFading = false;
				fadingIn = false;
			}
		}
		else
		{
			fadingIn = false;
			fadingOut = false;
		}
		
		return;
	}

	if (fadingIn)
	{
		fadeAlpha = floor((255.0 - 255.0 * fadeFrame / (double)fadeLength) + .5);
	}
	else if (fadingOut)
	{
		fadeAlpha = floor(255.0 * fadeFrame / (double)fadeLength + .5);
	}

	Color oldColor = fadeRect.getFillColor();
	fadeRect.setFillColor(Color(oldColor.r, oldColor.g, oldColor.b, fadeAlpha));
}

void Fader::Draw(int layer, sf::RenderTarget *target )
{
	if (layer != currLayer)
	{
		return;
	}

	if (fadeAlpha > 0 && fadeLength > 0)
	{
		target->draw(fadeRect);
	}
}

Swiper::SwipeTypeInfo::SwipeTypeInfo()
{
	loaded = false;
	tilesets = NULL;
}

Swiper::SwipeTypeInfo::~SwipeTypeInfo()
{
	if (tilesets != NULL)
	{
		delete[] tilesets;
	}
}

sf::IntRect Swiper::SwipeTypeInfo::GetSubRect(int index)
{
	int imageIndex = index / 4;
	int tIndex = index % 4;

	return tilesets[imageIndex]->GetSubRect(tIndex);
}

Tileset *Swiper::SwipeTypeInfo::GetTileset(int index)
{
	int imageIndex = index / 4;
	return tilesets[imageIndex];
}

void Swiper::LoadSwipeType(MainMenu *mm, SwipeType st)
{
	swipeTypeMap[st].Load(mm);
}
void Swiper::SwipeTypeInfo::Load( MainMenu *mm)
{
	if (!loaded)
	{
		tilesets = new Tileset*[numImages];
		string png = ".png";
		string underscore = "_";
		string path = "KinBoost/";
		int currImage = 0;
		for (int i = 0; i < numImages; ++i)
		{
			currImage = i / 4;
			tilesets[i] = mm->tilesetManager.GetTileset(path + baseName + underscore + to_string(currImage) + png, 960, 540 );
		}
		loaded = true;
	}
}

bool Swiper::SwipeTypeInfo::IsPostWipe(int index)
{
	return (index >= fullWipeIndex);
}

std::map<Swiper::SwipeType, Swiper::SwipeTypeInfo> Swiper::swipeTypeMap;

Swiper::Swiper()
{
	if (swipeTypeMap.empty())
	{
		SwipeTypeInfo & stW1 = swipeTypeMap[W1];
		stW1.numImages = 1;
		stW1.numTiles = 4;
		stW1.fullWipeIndex = 1;
		stW1.baseName = "kinswipe_960x540";
	}
	swipeSpr.setScale(2, 2);

	Reset();
}

bool Swiper::IsPostWipe()
{
	if (!swiping)
		return false;

	int f = swipeFrame / animFactor;
	return currInfo->IsPostWipe(f);
}

bool Swiper::IsSwiping()
{
	return swiping;
}

void Swiper::Swipe( SwipeType t, int af, bool ignorePlayer)
{
	currInfo = &swipeTypeMap[t];
	swipeType = t;
	animFactor = af;
	swiping = true;
	swipeFrame = 0;
	swipeLength = currInfo->numTiles;
	skipKin = ignorePlayer;
}

void Swiper::Update()
{
	if (!swiping)
		return;

	if (swipeFrame == swipeLength * animFactor)
	{
		swiping = false;
	}
	else
	{
		int f = swipeFrame / animFactor;
		if (f % 4 == 0)
		{
			swipeSpr.setTexture(*currInfo->GetTileset(f)->texture);
		}
		swipeSpr.setTextureRect(currInfo->GetSubRect(f));

		swipeFrame++;
	}
}

void Swiper::Reset()
{
	swiping = false;
	swipeFrame = 0;
	swipeLength = 0;
	animFactor = 1;
}

void Swiper::Draw(sf::RenderTarget *target)
{
	target->draw(swipeSpr);
}