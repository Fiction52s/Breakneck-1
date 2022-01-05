#ifndef __TIMERHUD_H__
#define __TIMERHUD_H__

#include <SFML/Graphics.hpp>
#include "Movement.h"

struct TimerText;
struct ImageText;
struct Session;
struct Tileset;
struct PaletteShader;

struct TimerHUD
{
	enum Action
	{
		NORMAL,
		GROW,
		LARGE,
		SHRINK,
		HIDDEN,
		SHOW_MODIFIER,
		Count
	};

	int actionLength[Count];

	TimerText *timer;
	ImageText *centiSecondTimer;
	
	bool modifier;

	Tileset *ts_text;
	sf::Vector2f center;

	CubicBezier growBez;
	CubicBezier shrinkBez;

	PaletteShader *textShader;
	Session *sess;

	Action action;
	int frame;

	float baseScale;
	float grownScale;
	int currNumFrames;
	float currScale;
	float centiScale;

	TimerHUD( bool modifier );
	~TimerHUD();

	void SetNumFrames(int frames);
	void SetModifiedPlus(int frames);
	void SetModifiedMinus(int frames);
	void Update();
	void SetCenter(sf::Vector2f &p_center);
	void SetColor(sf::Color c);
	void Reset();
	void Drain(int frameDamage);
	sf::Vector2f GetRightCenter();
	void Grow();
	void Shrink();
	void SetScale(float f);
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
};

//struct TimerModifierHUD
//{
//	enum Action
//	{
//		NORMAL,
//		Count
//	};
//
//	int actionLength[Count];
//	Action action;
//	int frame;
//	ImageText *modifierTimer;
//	ImageText *modifierCentiSecondTimer;
//	PaletteShader *modifierTextShader;
//	Session *sess;
//	Tileset *ts_text;
//	TimerHUD *tHUD;
//
//	TimerModifierHUD( TimerHUD *tHUD );
//	~TimerModifierHUD();
//
//	void Update();
//	void UpdateSprite();
//	void SetNumFrames(int frames);
//	void Draw(sf::RenderTarget *target);
//};

#endif