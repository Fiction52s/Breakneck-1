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

	sf::Vector2f GetTransformedOffset();
	sf::Vector2f GetScaledOffset();

	void UpdateGrabPoints();

	bool RectContainsPoint(sf::Vector2f &pos);
	sf::Vector2f GetRectPoint(int i);

	void UpdateScaleOrigin();

	sf::Vector2f GetCenter();
	void UpdateRotationAnchor();

	CircleGroup *circleGroup;

	sf::Vector2f rotationAnchor;
	sf::Vector2f originOffset;
	sf::Vector2f size;

	sf::Vector2f origCenter;
	sf::Vector2f origSize;

	sf::Vector2f startClick;

	int scalePoint;
	sf::Vector2f scaleAnchor;
	int scaleAnchorIndex;
	void SetScaleAnchor();
	sf::Vector2f scaleAlong;
	sf::Vector2f scaleOther;

	int GetClickedScalePoint(sf::Vector2f &pos);
	bool ClickedRotatePoint(sf::Vector2f &pos);

	float scalePointRadius;
	float rotatePointRadius;

	bool clickedNothing;

	float rotationStart;

	sf::Vector2f scale;
	float rotation; //degrees
	sf::RectangleShape tRect;
};

#endif