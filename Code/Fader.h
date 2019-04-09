#ifndef __FADE_H__
#define __FADE_H__

#include <SFML/Graphics.hpp>

struct Fader
{
	Fader();
	sf::RectangleShape fadeRect;
	int fadeFrame;
	int fadeLength;
	bool fadingIn;
	bool fadingOut;
	int fadeAlpha;
	bool fadeSkipKin;
	int crossFadeFrame;
	int crossPauseLength;
	int crossFadeLength;
	bool crossFading;
	bool crossSkipKin;
	sf::Color crossColor;
	void Fade(bool in, int frames, sf::Color c, bool skipKin = false);
	void CrossFade(int fadeOutFrames,
		int pauseFrames, 
		sf::Color c, bool skipKin = false);
	bool IsFading();
	void Update();
	void Clear();
	void Reset();
	void Draw(sf::RenderTarget *target);
};

#endif