#ifndef __EYE_H__
#define __EYE_H__

#include <SFML/Graphics.hpp>

struct Detector
{
	enum State
	{
		S_WAITING,
		S_STARTUP,
		S_TARGET,
		S_ACTIVE,
		S_UNTARGET,
		S_SHUTDOWN,
		Count
	};
	
	State detState;
	//Detector( int startupLength, int targetLength, int )
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
	void ProcessState();
	
};

struct GameSession;
struct Tileset;
struct Patroller;
struct PatrollerEye
{
	enum State
	{
		S_WAITING,
		S_STARTUP,
		S_WAITTOTARGET,
		S_TARGET,
		S_ACTIVE,
		S_UNTARGET,
		S_SHUTDOWN,
		S_Count
	};
	sf::Vertex eye[8];
	sf::Vertex *highlight;
	Tileset *ts_eye;
	void Reset();
	PatrollerEye::PatrollerEye(GameSession *owner,
		Patroller *p);
	State state;
	void UpdateSprite();
	bool IsEyeActivated();
	void Draw(sf::RenderTarget *target,
		sf::Shader *sh = NULL );
	void ProcessState(sf::Vector2f &targetPos);
	void SetPosition(sf::Vector2f &pos);
	sf::Vector2f pos;
	int actionLength[S_Count];
	int animFactor[S_Count];
	int frame;
	sf::Transform rotateEye;
	float angle;
	float activateRange;
	float trackRange;
	float deactivateTrackRange;
	float deactivateRange;

	Patroller *parent;
};

#endif