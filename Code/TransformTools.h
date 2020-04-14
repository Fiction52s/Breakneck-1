#ifndef __TRANSFORMTOOLS_H__
#define __TRANSFORMTOOLS_H__

#include <SFML\Graphics.hpp>

struct TransformTools
{
	enum Mode
	{
		NONE,
		MOVE,
		SCALE,
		ROTATE
	};

	Mode mode;

	TransformTools();
	void Reset(sf::Vector2f &p_center, sf::Vector2f &p_size);
	void Draw(sf::RenderTarget *target);
	void Update( sf::Vector2f &worldPos, bool mouseDown );

	sf::Vector2f GetRotationAnchor();

	sf::Vector2f center;
	sf::Vector2f originOffset;
	sf::Vector2f size;

	sf::Vector2f origCenter;
	sf::Vector2f origSize;

	sf::Vector2f startClick;

	float rotationStart;

	float scale;
	float rotation; //degrees
	sf::RectangleShape tRect;
};

#endif