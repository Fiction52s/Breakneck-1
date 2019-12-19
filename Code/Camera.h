#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <SFML/Graphics.hpp>
#include "Movement.h"
#include "Input.h"

struct MovementSequence;
struct Actor;

struct Camera
{
	Camera();
	float GetZoom();
	void Set( sf::Vector2f &pos, float zFactor,
		int zLevel );
	void Reset();
	void SetRumble( int xFactor, int yFactor, int duration );
	void StopRumble();
	void UpdateRumble();
	void EaseOutOfManual( int frames );
	void SetManual( bool man );
	void SetMovementSeq( MovementSequence *move,
		bool relative );
	void Ease(sf::Vector2f &pos, float zFactor,
		int numFrames, CubicBezier &bez );
	void UpdateEase();
	sf::Vector2f GetNewOffset( sf::Vector2<double> &pVel);
	GameSession *owner;
	sf::Vector2f offsetVel;
	
	sf::FloatRect GetRect();
	sf::Vector2f GetPos();
	sf::Vector2f manualPos;
	sf::Vector2f startManualPos;
	float startManualZoom;
	float manualZoom;
	MovementSequence *currMove;
	bool relativeMoveSeq;
	sf::Vector2f sequenceStartPos;
	float sequenceStartZoom;

	sf::Vector2f moveFrames;


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

	//int bType;

	double GetEnemyZoomTarget( Actor *a );
	void Update( Actor *a );
	void UpdateBossFight(int bossFightType);
	void ManualUpdate( Actor *a );
	void UpdateZoomLevel(ControllerState &con, ControllerState &prevcon);
	void UpdateVS( Actor *a, 
		Actor *a2 );
	double GetMovementZoomTarget( Actor *player );
	double GetNextMovementZoom( double moveZoom );
	double GetNextEnemyZoom(double enemyZoom);
	int slowMultiple;

	sf::Vector2<double> playerPos;
	sf::Vector2<double> GetPlayerVel( Actor *player);
	void SetMovementOffset( sf::Vector2<double> &pVel );
	void UpdateBarrier(Actor *player, float &xChangePos, 
		float &xChangeNeg, float &yChangePos, 
		float &yChangeNeg);
	int GetActiveEnemyCount( Actor *player,
		double &minX, double &maxX, 
		double &minY, double &maxY);
	void UpdateEaseOut();

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