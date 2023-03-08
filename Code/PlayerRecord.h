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
#include <fstream>
#include <vector>
#include "VectorMath.h"

struct ReplayHUD;

struct Actor;
struct PlayerReplayer;
struct NameTag;
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

	V2d playerPos; //for desyncs and tracking the name tag
};

struct PlayerRecordHeader
{
	int numberOfPlayers;
	int ver;

	BitField bUpgradeField;
	BitField bUpgradesTurnedOnField;
	BitField bLogField;

	PlayerRecordHeader();
	~PlayerRecordHeader();

	void SetFields();
	bool IsShardCaptured(int ind);
	bool IsLogCaptured(int ind);
	void SetVer(int v);
	void Read(std::istream &is);
	void Write(std::ofstream &of);
};

struct PlayerRecorder
{
	const static int MAX_RECORD = 3600 * 60;

	//header
	int numTotalFrames;
	int skinIndex;
	int startPowerMode;

	std::string displayName;


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
	NameTag *nameTag;

	PlayerReplayer *pReplayer;

	PlayerSkinShader playerSkinShader;
	int action;
	sf::Sprite replaySprite;
	SprInfo *sprBuffer;
	int frame;

	ReplayGhost(PlayerReplayer *pReplayer);
	~ReplayGhost();
	void DrawNameTag(sf::RenderTarget *target);
	void Reset();
	void Read(std::istream &is);
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
	void Read(std::istream &is);
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

	std::string displayName;

	ReplayGhost *replayGhost;
	ReplayPlayer *replayPlayer;

	PlayerReplayManager *replayManager;

	Actor *player;

	PlayerReplayer(Actor *p, PlayerReplayManager *p_replayManager);
	~PlayerReplayer();
	int GetFramesBeforeGoal();
	void SetDisplayName(const std::string &n);
	bool Read(std::istream &is );
	//bool OpenGhost(const boost::filesystem::path &fileName);
};

struct PlayerReplayManager
{
	PlayerRecordHeader header;
	std::vector<PlayerReplayer*> repVec;

	bool ghostsActive;
	bool replaysActive;

	ReplayHUD *replayHUD;

	PlayerReplayManager();
	~PlayerReplayManager();
	bool IsReplayOn(int pIndex);
	bool IsReplayHUDOn(int pIndex);
	bool IsGhostOn(int pIndex);
	PlayerReplayer *GetReplayer(int pIndex);
	void Reset();
	void SetToStart();
	bool LoadFromFile(const boost::filesystem::path &fileName);
	bool LoadFromStream(std::istream &is);
	void AddGhostsToVec(std::vector<ReplayGhost*> &vec);
	void DrawGhostNameTags(sf::RenderTarget *target);
	void UpdateGhostNameTagsPixelPos(sf::RenderTarget *target);
};

#endif