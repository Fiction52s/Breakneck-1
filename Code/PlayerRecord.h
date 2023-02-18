#ifndef __PLAYER_RECORD_H__
#define __PLAYER_RECORD_H__

#include <SFML\Graphics.hpp>
#include "Input.h"
#include <boost\filesystem.hpp>
#include <list>
#include <map>
#include "ItemSelector.h"
#include "UIWindow.h"
#include "PlayerSkinShader.h"
#include "BitField.h"

struct Actor;
struct PlayerReplayer;
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

struct PlayerRecordHeader
{
	int numberOfPlayers;
	int ver;

	BitField bUpgradeField;
	BitField bUpgradesTurnedOnField;
	BitField bLogField;
	BitField bShardField;

	PlayerRecordHeader();
	~PlayerRecordHeader();

	void SetFields();
	void SetVer(int v);
	void Read(std::ifstream &is);
	void Write(std::ofstream &of);
};

struct PlayerRecorder
{
	const static int MAX_RECORD = 3600 * 60;

	//header
	int numTotalFrames;
	int skinIndex;
	int startPowerMode;


	SprInfo sprBuffer[MAX_RECORD]; //1 hour
	COMPRESSED_INPUT_TYPE inputBuffer[MAX_RECORD];

	int frame;
	Actor *player;

	PlayerRecorder(Actor *p_player);
	void StopRecording();
	void StartRecording();
	void RecordGhostFrame();
	void RecordReplayFrame();
	void Write(std::ofstream &of);
	void StopRecordingAndWriteToFile(const std::string &fileName);
};

struct PlayerRecordingManager
{
	PlayerRecordHeader header;
	std::vector<PlayerRecorder*> recorderVec;

	PlayerRecordingManager(int numPlayers);
	~PlayerRecordingManager();
	void StopRecording();
	void StartRecording();
	void RestartRecording();
	void RecordGhostFrames();
	void RecordReplayFrame(int index );
	void RecordReplayFrames();
	void WriteToFile(const std::string &fileName);
};

//play back the players sprite info
struct ReplayGhost
{
	PlayerReplayer *pReplayer;

	PlayerSkinShader playerSkinShader;
	int action;
	sf::Sprite replaySprite;
	SprInfo *sprBuffer;
	int frame;

	ReplayGhost(PlayerReplayer *pReplayer);
	~ReplayGhost();
	void Reset();
	void Read(std::ifstream &is);
	void UpdateReplaySprite();
	void Draw(sf::RenderTarget *target);
};

//replay the player's inputs to set up a replay playback
struct ReplayPlayer
{
	PlayerReplayer *pReplayer;

	COMPRESSED_INPUT_TYPE *inputBuffer;
	int frame;

	ReplayPlayer(PlayerReplayer *pReplayer);
	~ReplayPlayer();
	void Read(std::ifstream &is);
	void Reset();
	void SetToStart();
	void UpdateInput(ControllerState &state);//ControllerDualStateQueue *controllerInput);
};

struct PlayerReplayManager;

struct PlayerReplayer
{
	int numTotalFrames;
	int skinIndex;
	int startPowerMode;

	ReplayGhost *replayGhost;
	ReplayPlayer *replayPlayer;

	PlayerReplayManager *replayManager;

	Actor *player;

	PlayerReplayer(Actor *p, PlayerReplayManager *p_replayManager);
	~PlayerReplayer();
	bool Read(std::ifstream &is );
	//bool OpenGhost(const boost::filesystem::path &fileName);
};

struct PlayerReplayManager
{
	PlayerRecordHeader header;
	std::vector<PlayerReplayer*> repVec;

	bool ghostsActive;
	bool replaysActive;

	PlayerReplayManager();
	~PlayerReplayManager();
	bool IsReplayOn(int pIndex);
	bool IsGhostOn(int pIndex);
	PlayerReplayer *GetReplayer(int pIndex);
	void Reset();
	void SetToStart();
	bool LoadFromFile(const boost::filesystem::path &fileName);
	
};

#endif