#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <SFML/Graphics.hpp>
#include "Actor.h"

struct Camera
{
	Camera();
	float GetZoom();
	void Set( sf::Vector2f &pos, float zFactor,
		int zLevel );
	void SetRumble( int xFactor, int yFactor, int duration );
	void UpdateRumble();

	int rumbleFrame;
	int rumbleLength;
	int rumbleX;
	int rumbleY;
	bool rumbling;
	int rX;
	int rY;

	sf::Vector2f offset;
	sf::Vector2f maxOffset;
	sf::Vector2f pos;
	float zoomFactor;
	float zoomOutRate;
	float zoomInRate;
	float offsetRate;
	float maxZoom;
	float minZoom;
	float zoomLevel;
	float zoomLevel1;
	float zoomLevel2;
	float zoomLevel3;

	int numActive;
	int framesActive;
	int framesFalling;
	
	sf::Vector2f testOffset;
	float testZoom;

	bool manual;

	//new
	double left;
	double top;
	double bottom;
	double right;

	bool bossCrawler;

	void Update2( Actor *a );
	void UpdateReal( Actor *a );
	void Update( Actor *a );
};

#endif