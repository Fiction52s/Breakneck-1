#include "Fader.h"

using namespace std;
using namespace sf;

Fader::Fader()
{
	Reset();
	fadeRect.setSize(Vector2f(1920, 1080));
}

void Fader::Fade(bool in, int frames, sf::Color c, bool skipKin)
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
}

void Fader::CrossFade(int fadeOutFrames, int pauseFrames, sf::Color c, bool skipKin)
{
	crossFadeLength = fadeOutFrames;
	crossPauseLength = pauseFrames;
	crossSkipKin = skipKin;
	crossColor = c;
	crossFading = true;
	Fade(false, fadeOutFrames, c, skipKin);
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
				Fade(true, crossFadeLength, crossColor, crossSkipKin);
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

void Fader::Draw(sf::RenderTarget *target)
{
	if (fadeAlpha > 0 && fadeLength > 0)
	{
		target->draw(fadeRect);
	}
}