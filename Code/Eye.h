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

	struct MyData
	{
		int frame;
		int state;
		sf::Vector2f pos;
		float angle;
	};

	MyData data;

	int actionLength[S_Count];
	int animFactor[S_Count];
	float activateRange;
	float trackRange;
	float deactivateTrackRange;
	float deactivateRange;
	Patroller *parent;
	sf::Vertex eye[8];
	sf::Vertex *highlight;
	Tileset *ts_eye;

	PatrollerEye::PatrollerEye(Patroller *p);
	void Reset();
	void UpdateSprite();
	bool IsEyeActivated();
	void Draw(sf::RenderTarget *target,
		sf::Shader *sh = NULL );
	void ProcessState(sf::Vector2f &targetPos);
	void SetPosition(sf::Vector2f &pos);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif