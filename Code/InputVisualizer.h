#ifndef __INPUTVISUALIZER_H__
#define __INPUTVISUALIZER_H__

#include <SFML\Graphics.hpp>
#include "Input.h"

struct InputVisualizer
{
	enum InputTypes : int
	{
		CONTROL = 0,
		ACTION = 1,
		POWER = 4
	};


	sf::Vertex *quads;
	InputVisualizer();
	void Update(ControllerState &currInput);
	void Draw(sf::RenderTarget *target);

	sf::Vector2f pos;
	sf::Vector2f stickRel;
	sf::Vector2f actionRel;
	sf::Vector2f powerRel;

	const static int TOTALQUADS = 9;

};

#endif