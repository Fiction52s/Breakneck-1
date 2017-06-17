#ifndef __PLAYER_RECORD_H__
#define __PLAYER_RECORD_H__

#include <SFML\Graphics.hpp>
#include "Input.h"

struct Actor;

//what is recorded each frame for the ghost
struct SprInfo
{
	sf::Vector2f position;
	sf::Vector2f origin;
	float rotation;
	bool flipX;
	bool flipY;

	int action;
	int tileIndex;

	int speedLevel;
};

//record the players sprite info
struct RecordGhost
{
	RecordGhost(Actor *p);
	int frame;
	void StartRecording();
	void StopRecording();
	int numTotalFrames;
	Actor *player;
	void WriteToFile(const std::string &fileName);
	void RecordFrame();
	const static int MAX_RECORD = 3600 * 60;
	SprInfo sprBuffer[MAX_RECORD]; //1 hour
};

//play back the players sprite info
struct ReplayGhost
{
	bool init;
	ReplayGhost(Actor *p);
	bool OpenGhost(const std::string &fileName);
	sf::Sprite replaySprite;

	void UpdateReplaySprite();

	void Draw(sf::RenderTarget *target);

	SprInfo *sprBuffer;
	Actor *player;
	int frame;
	int numTotalFrames;
};

struct RecordGhost;
struct ReplayGhost;

//record the player's inputs to create a full replay of a map
struct RecordPlayer
{
	RecordPlayer(Actor *p);
	int frame;
	void StartRecording();
	void StopRecording();
	int numTotalFrames;
	Actor *player;
	void WriteToFile(const std::string &fileName);
	void RecordFrame();
	const static int MAX_RECORD = 3600 * 60;
	ControllerState inputBuffer[MAX_RECORD]; //1 hour
};


//replay the player's inputs to set up a replay playback
struct ReplayPlayer
{
	bool init;
	ReplayPlayer(Actor *p);
	bool OpenReplay(const std::string &fileName);
	void UpdateInput(ControllerState &state);
	ControllerState *inputBuffer;
	Actor *player;
	int frame;
	int numTotalFrames;
};

#endif