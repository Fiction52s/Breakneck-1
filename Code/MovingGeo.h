#ifndef __MOVING_GEO_H__
#define __MOVING_GEO_H__

#include <SFML\Graphics.hpp>
#include "Movement.h"

sf::Color GetBlendColor(
	sf::Color c0,
	sf::Color c1,
	float progress);

struct MovingGeo
{
	MovingGeo();
	virtual void Draw(sf::RenderTarget *target);
	virtual void Update();

	int numPoints;
	sf::Vertex *points;

};

struct SpinningTri
{
	enum State
	{
		S_EXPANDING,
		S_GROW,
		S_ROTATE,
		S_ROTATE_AND_FADE,
		S_Count
	};


	SpinningTri(float startAngle, sf::Vector2f &center );
	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);

	void SetColor(sf::Color c);
	void SetColorGrad(sf::Color startCol,
		sf::Color endCol);
	void SetColorChange(sf::Color &startC,
		sf::Color &endC, float progress);

	int stateLength[S_Count];
	int maxLength;
	State state;
	int frame;

	float length;
	float width;
	float angle;
	float startAngle;
	sf::Color startColor;
	sf::Color currColor;
	sf::Color fadeColor;

	float finalWidth;
	float startWidth;
	sf::Vector2f center;
	sf::Vertex tri[3];

};

struct Laser
{
	enum State
	{
		S_VERTICALGROW,
		S_WIDEN,
		S_SHRINK,
		S_SLOWGROW,
		S_DISAPPEAR,
		S_Count
	};


	Laser(float startAngle, sf::Vector2f &center);
	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);

	void SetColor(sf::Color c);
	/*void SetColorGrad(sf::Color startCol,
		sf::Color endCol);*/
	void SetColorChange(sf::Color &startC,
		sf::Color &endC, float progress);
	void SetHeight(float h);
	void SetWidth(float w);

	int stateLength[S_Count];
	int maxHeight;
	State state;
	int frame;

	float height;
	float width;
	float angle;
	float startAngle;
	sf::Color startColor;
	sf::Color currColor;
	//sf::Color fadeColor;

	float growWidth;
	float shrinkWidth;
	float startWidth;
	sf::Vector2f center;
	sf::Vertex quad[8];
};

struct Ring
{
	Ring( int p_circlePoints );
	~Ring();
	void Draw(sf::RenderTarget *target);
	void UpdatePoints();
	void Set(sf::Vector2f pos,
		float innerR, float ringWidth);
	void SetColor(sf::Color c);
	void SetShader(sf::Shader *sh);
	sf::Vector2f position;
	float innerRadius;
	float outerRadius;
	sf::Vertex *circle;
	sf::Color color;
	sf::Shader *shader;
	int circlePoints;
};

struct MovingRing
{
	Ring *ring;

	MovingRing(int p_circlePoints,
		float p_startInner,
		float p_startWidth,
		float p_endInner,
		float p_endWidth,
		sf::Vector2f p_startPos,
		sf::Vector2f p_endPos,
		sf::Color p_startCol,
		sf::Color p_endCol,
		int totalFrames);
	void Reset();
	void Update();
	void Draw(sf::RenderTarget *target);
	//void UpdatePoints();
	//void SetColor(sf::Color c);
	//void SetShader(sf::Shader *sh);

	float startInner;
	float startWidth;
	float endInner;
	float endWidth;
	sf::Color startColor;
	sf::Color endColor;

	sf::Vector2f startPos;
	sf::Vector2f endPos;

	int frame;
	int totalFrames;

	CubicBezier sizeBez;
	CubicBezier colorBez;
	CubicBezier posBez;
	CubicBezier innerBez;

	

	
};

#endif