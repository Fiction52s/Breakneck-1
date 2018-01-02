#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <SFML/Graphics.hpp>
#include "Movement.h"
//#include "Actor.h"

struct MovementSequence;
struct Actor;

struct Camera
{
	Camera();
	float GetZoom();
	void Set( sf::Vector2f &pos, float zFactor,
		int zLevel );
	void SetRumble( int xFactor, int yFactor, int duration );
	void UpdateRumble();
	void EaseOutOfManual( int frames );
	void SetManual( bool man );
	void SetMovementSeq( MovementSequence *move,
		bool relative );
	void Ease(sf::Vector2f &pos, float zFactor,
		int numFrames, CubicBezier &bez );
	void UpdateEase();
	

	sf::Vector2f GetPos();
	sf::Vector2f manualPos;
	sf::Vector2f startManualPos;
	float startManualZoom;
	float manualZoom;
	MovementSequence *currMove;
	bool relativeMoveSeq;
	sf::Vector2f sequenceStartPos;
	float sequenceStartZoom;

	int easeOutCount;
	int easeOutFrame;
	bool easingOut;

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
	bool easing;
	//new
	double left;
	double top;
	double bottom;
	double right;

	bool bossCrawler;

	void Update2( Actor *a );
	void UpdateReal( Actor *a );
	void Update( Actor *a );
	void UpdateVS( Actor *a, 
		Actor *a2 );

private:
	
	int easeFrame;
	sf::Vector2f startEase;
	sf::Vector2f endEase;
	float startEaseZFactor;
	float endEaseZFactor;
	int numEaseFrames;
	CubicBezier easeBez;
};

#endif