#ifndef __TRANSFORMTOOLS_H__
#define __TRANSFORMTOOLS_H__

#include <SFML\Graphics.hpp>

struct CircleGroup;

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
	~TransformTools();
	void Reset(sf::Vector2f &p_center, sf::Vector2f &p_size);
	void Draw(sf::RenderTarget *target);
	void Update( sf::Vector2f &worldPos, bool mouseDown );

	sf::Vector2f GetRotationAnchor();

	void UpdateGrabPoints();

	bool RectContainsPoint(sf::Vector2f &pos);
	sf::Vector2f GetRectPoint(int i);

	CircleGroup *circleGroup;

	sf::Vector2f center;
	sf::Vector2f originOffset;
	sf::Vector2f size;

	sf::Vector2f origCenter;
	sf::Vector2f origSize;

	sf::Vector2f startClick;

	int scalePoint;
	sf::Vector2f scaleAnchor;
	void SetScaleAnchor();

	int GetClickedScalePoint(sf::Vector2f &pos);
	bool ClickedRotatePoint(sf::Vector2f &pos);

	float scalePointRadius;
	float rotatePointRadius;

	bool clickedNothing;

	float rotationStart;

	float scale;
	float rotation; //degrees
	sf::RectangleShape tRect;
};

#endif