#ifndef __REPLAY_HUD_H__
#define __REPLAY_HUD_H__

#include <SFML\Graphics.hpp>

struct Tileset;
struct Session;

struct ReplayHUD
{
	sf::Vertex bgQuad[4];

	sf::Vertex buttonIconQuads[4 * 2];
	sf::Text playText;
	sf::Text advanceFrameText;

	bool goingToNextFrame;
	Session *sess;

	Tileset *ts_buttons;

	ReplayHUD();
	~ReplayHUD();

	void SetTopLeft(sf::Vector2f pos);
	void Reset();
	void Update();
	bool IsGoingToNextFrame();
	void Draw(sf::RenderTarget *target);
};

#endif