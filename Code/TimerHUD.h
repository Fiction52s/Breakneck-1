#ifndef __TIMERHUD_H__
#define __TIMERHUD_H__

#include <SFML/Graphics.hpp>

struct TimerText;
struct ImageText;
struct Session;
struct Tileset;

struct TimerHUD
{
	TimerText *timer;
	ImageText *centiSecondTimer;
	Session *sess;
	Tileset *ts_text;

	TimerHUD();
	~TimerHUD();

	void SetNumFrames(int frames);
	void Update();
	void Drain(int frameDamage);
	bool IsDone();
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
};

#endif