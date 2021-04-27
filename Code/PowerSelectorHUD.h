#ifndef __POWERSELECTOR_H__
#define __POWERSELECTOR_H__

#include <SFML\Graphics.hpp>
#include "Input.h"

struct Tileset;
struct Session;
struct PowerSelector
{
	enum State
	{
		STATIC,
		SWITCHING,
	};

	Tileset *ts;
	Session *sess;
	sf::Sprite sprite;

	sf::Vertex deactivatedQuads[3 * 4];

	int currPowerMode;

	State state;
	int frame;

	PowerSelector();

	void SetPosition(sf::Vector2f &pos);

	void Update(int currPowerMode );

	void Draw(sf::RenderTarget *target);
};

#endif