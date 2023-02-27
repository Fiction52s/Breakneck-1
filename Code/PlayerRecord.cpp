#include "PlayerRecord.h"
#include "Actor.h"
#include <string>
#include <fstream>
#include <assert.h>
#include "Buf.h"
#include <iostream>
#include "GameSession.h"
#include "MainMenu.h"
#include <vector>
#include "globals.h"
#include "SaveFile.h"
#include "LogMenu.h"
#include "ReplayHUD.h"

using namespace sf;
using namespace std;


ReplayGhost::ReplayGhost(PlayerReplayer *p_pReplayer)
	:pReplayer(p_pReplayer), sprBuffer(NULL), playerSkinShader( "player")
{
	frame = 0;
	action = 0;

	playerSkinShader.SetSkin(Actor::SKIN_GHOST);

	playerSkinShader.pShader.setUniform("u_invincible", 0.f);
	playerSkinShader.pShader.setUniform("u_super", 0.f);
	playerSkinShader.pShader.setUniform("u_slide", 0.f);
}

ReplayGhost::~ReplayGhost()
{
	if (sprBuffer != NULL)
	{
		delete[] sprBuffer;
	}
}

void ReplayGhost::Read(ifstream &is)
{
	sprBuffer = new SprInfo[pReplayer->numTotalFrames];
	is.read((char*)sprBuffer, sizeof(SprInfo) * pReplayer->numTotalFrames);
}

void ReplayGhost::Draw(RenderTarget *target)
{
	if (frame == pReplayer->numTotalFrames)
		return;

	if (!pReplayer->player->IsVisibleAction(action))
	{
		return;
	}

	if (frame >= 0 && frame < pReplayer->numTotalFrames)
	{
		target->draw(replaySprite, &playerSkinShader.pShader);
	}
}

void ReplayGhost::Reset()
{
	frame = 0;
}

void ReplayGhost::UpdateReplaySprite()
{
	if (frame == pReplayer->numTotalFrames)
		return;

	//if (pReplayer->player->action == Actor::SPAWNWAIT)
	//	return;


	SprInfo &info = sprBuffer[frame];
	Tileset *ts = pReplayer->player->tileset[(Actor::Action)info.action];
	replaySprite.setTexture(*ts->texture);

	action = info.action;
	Actor::Action a = (Actor::Action)info.action;
	/*if (a == Actor::JUMPSQUAT)
	{
		cout << "setting jumpsquat in ghost. frame: " << frame << endl;
	}*/
	IntRect ir = ts->GetSubRect(info.tileIndex);

	replaySprite.setRotation(info.rotation);


	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	playerSkinShader.pShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
		(ir.left + ir.width) / width, (ir.top + ir.height) / height));

	if (info.flipX)
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	if (info.flipY)
	{
		ir.top += ir.height;
		ir.height = -ir.height;
	}
	

	replaySprite.setTextureRect(ir);
	replaySprite.setOrigin(info.origin.x, info.origin.y);
	replaySprite.setRotation(info.rotation);
	replaySprite.setPosition(info.position.x, info.position.y);

	++frame;
}


ReplayPlayer::ReplayPlayer(PlayerReplayer *p_pReplayer)
	:pReplayer(p_pReplayer), inputBuffer(NULL)
{
	frame = 0;
}

ReplayPlayer::~ReplayPlayer()
{
	delete[] inputBuffer;
}

void ReplayPlayer::Read(ifstream &is)
{
	assert(inputBuffer == NULL);
	inputBuffer = new COMPRESSED_INPUT_TYPE[pReplayer->numTotalFrames];
	is.read((char*)inputBuffer, pReplayer->numTotalFrames * COMPRESSED_INPUT_SIZE);
}

void ReplayPlayer::Reset()
{
	SetToStart();
	pReplayer->player->SetAllUpgrades(pReplayer->replayManager->header.bUpgradeField);//pReplayer->//pReplayer->bUpgradeField);
	pReplayer->player->currPowerMode = pReplayer->startPowerMode;
}

void ReplayPlayer::SetToStart()
{
	frame = 0;
}


void ReplayPlayer::UpdateInput( ControllerState &state )//ControllerDualStateQueue *controllerInput)
{
	if (frame == pReplayer->numTotalFrames)
		return;

	bool start = state.start;
	bool back = state.back;

	state.SetFromCompressedState(inputBuffer[frame]);

	state.start = start;
	state.back = back;

	//cout << "replaying input: " << inputBuffer[frame] << " on frame " << frame << "\n";

	/*if (state.A)
	{
		cout << "replay held A down on frame: " << frame << "\n";
	}*/
	//bool start = controllerInput->GetCurrState().start;
	//bool back = controllerInput->GetCurrState().back;

	//controllerInput->states[0].SetFromCompressedState(inputBuffer[frame]);
	

	//controllerInput->states[0].start = start;
	//controllerInput->states[0].back = back;

	++frame;
}

PlayerRecorder::PlayerRecorder(Actor *p_player)
	:player(p_player)
{
	numTotalFrames = -1;
	frame = -1;
}

void PlayerRecorder::StopRecording()
{
	numTotalFrames = frame;
	frame = -1;

	//ghostHeader.gType = GhostHeader::G_SINGLE_LEVEL_COMPLETE;
	//ghostHeader.numberOfPlayers = 1;
}

void PlayerRecorder::StartRecording()
{
	frame = 0;
	numTotalFrames = -1;

	//only works for 1 player atm

	startPowerMode = player->currPowerMode;
}

void PlayerRecorder::RecordGhostFrame()
{
	if (frame < 0)
		return;

	if (frame >= MAX_RECORD)
		return;

	/*if (player->action != Actor::SPAWNWAIT)
	{
	}*/

	//ghost update
	if (player->simulationMode)
	{
		return;
	}

	SprInfo &info = sprBuffer[frame];
	info.position = player->sprite->getPosition();
	info.origin = player->sprite->getOrigin();
	info.rotation = player->sprite->getRotation();
	info.flipX = player->flipTileX;
	info.flipY = player->flipTileY;
	info.action = (int)player->spriteAction;
	info.tileIndex = player->currTileIndex;
	info.speedLevel = player->speedLevel;

	++frame;
}

void PlayerRecorder::RecordReplayFrame()
{
	if (frame < 0)
		return;

	if (frame >= MAX_RECORD)
		return;

	//replay update
	ControllerState &s = player->currInput;
	COMPRESSED_INPUT_TYPE compressedInputs = s.GetCompressedState();
	inputBuffer[frame] = compressedInputs;

	//cout << "recording input: " << compressedInputs << " on frame " << frame << "\n";

	/*if (s.A)
	{
		cout << "recording held A down on frame: " << frame << "\n";
	}*/

	//frame gets incremented when the ghost is stored

	//++frame;
}

void PlayerRecorder::Write(ofstream &of )
{
	of.write((char*)&numTotalFrames, sizeof(numTotalFrames));
	of.write((char*)&skinIndex, sizeof(skinIndex));
	of.write((char*)&startPowerMode, sizeof(startPowerMode));
	
	of.write((char*)sprBuffer, numTotalFrames * sizeof(SprInfo));
	of.write((char*)inputBuffer, numTotalFrames * COMPRESSED_INPUT_SIZE);
}

void PlayerRecorder::StopRecordingAndWriteToFile(const std::string &fileName)
{

}

PlayerRecordHeader::PlayerRecordHeader()
	:numberOfPlayers(0),
	bUpgradeField(Session::PLAYER_OPTION_BIT_COUNT),
	bUpgradesTurnedOnField(Session::PLAYER_OPTION_BIT_COUNT),
	bLogField(LogDetailedInfo::MAX_LOGS)
{
	SetVer(1);
}

PlayerRecordHeader::~PlayerRecordHeader()
{
}

void PlayerRecordHeader::SetVer(int v)
{
	ver = v;
}

void PlayerRecordHeader::SetFields()
{
	GameSession *game = GameSession::GetSession();
	if (game != NULL && game->saveFile != NULL)
	{
		bUpgradeField.Set(game->saveFile->upgradeField);//sess->GetPlayer(0)->bStartHasUpgradeField);
		bUpgradesTurnedOnField.Reset();

		bLogField.Set(game->saveFile->logField);
	}
	else
	{
		bUpgradeField.Reset();
		bUpgradesTurnedOnField.Reset();
		bLogField.Reset();
	}
}

bool PlayerRecordHeader::IsShardCaptured(int ind)
{
	return bUpgradeField.GetBit(Actor::SHARD_START_INDEX + ind);
}

bool PlayerRecordHeader::IsLogCaptured(int ind)
{
	return bLogField.GetBit(ind);
}

void PlayerRecordHeader::Read(std::ifstream &is)
{
	is.read((char*)&ver, sizeof(ver)); //read in the basic vars
	is.read((char*)&numberOfPlayers, sizeof(numberOfPlayers));

	bUpgradeField.LoadBinary(is);
	bUpgradesTurnedOnField.LoadBinary(is);
	bLogField.LoadBinary(is);

	assert(numberOfPlayers > 0 && numberOfPlayers <= 4);
}

void PlayerRecordHeader::Write(std::ofstream &of)
{
	of.write((char*)&ver, sizeof(ver));
	of.write((char*)&numberOfPlayers, sizeof(numberOfPlayers));

	bUpgradeField.SaveBinary(of);
	bUpgradesTurnedOnField.SaveBinary(of);
	bLogField.SaveBinary(of);
}

PlayerReplayer::PlayerReplayer(Actor *p, PlayerReplayManager *p_replayManager)
{
	replayManager = p_replayManager;
	player = p;
	replayGhost = new ReplayGhost(this);
	replayPlayer = new ReplayPlayer(this);
}

PlayerReplayer::~PlayerReplayer()
{
	delete replayGhost;
	delete replayPlayer;
}

bool PlayerReplayer::Read(ifstream & is)
{
	//init = true;
	//frame = 0;

	is.read((char*)&numTotalFrames, sizeof(numTotalFrames));
	is.read((char*)&skinIndex, sizeof(skinIndex));
	is.read((char*)&startPowerMode, sizeof(startPowerMode));
	
	replayGhost->Read(is);
	replayPlayer->Read(is);

	//return false on failure
	return true;
}

PlayerRecordingManager::PlayerRecordingManager(int numPlayers)
{
	header.numberOfPlayers = numPlayers;
	recorderVec.resize(numPlayers);

	Session *sess = Session::GetSession();

	for (int i = 0; i < numPlayers; ++i)
	{
		recorderVec[i] = new PlayerRecorder(sess->GetPlayer(i));
	}
}

PlayerRecordingManager::~PlayerRecordingManager()
{
	/*for (int i = 0; i < recorderVec.size(); ++i)
	{
		delete recorderVec[i];
	}*/

	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		delete (*it);
	}
	recorderVec.clear();
}


void PlayerRecordingManager::StopRecording()
{
	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		(*it)->StopRecording();
	}
}

void PlayerRecordingManager::StartRecording()
{
	//header.bUpgradeField.Set(player->bStartHasUpgradeField);

	header.SetFields();

	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		(*it)->StartRecording();
	}
}

void PlayerRecordingManager::RestartRecording()
{
	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		(*it)->StopRecording();
	}

	StartRecording();
}


void PlayerRecordingManager::RecordGhostFrames()
{
	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		(*it)->RecordGhostFrame();
	}
}

void PlayerRecordingManager::RecordReplayFrame( int index )
{
	recorderVec[index]->RecordReplayFrame();
}

void PlayerRecordingManager::RecordReplayFrames()
{
	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		(*it)->RecordReplayFrame();
	}
}

void PlayerRecordingManager::WriteToFile(const std::string &fileName)
{
	bool validFrames = false;
	for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
	{
		if ((*it)->numTotalFrames > 0)
		{
			validFrames = true;
		}
	}

	if (!validFrames)
	{
		return;
	}

	ofstream of;
	of.open(fileName, ios::binary | ios::out);
	if (of.is_open())
	{
		header.Write(of);

		assert(header.numberOfPlayers == recorderVec.size());

		for (auto it = recorderVec.begin(); it != recorderVec.end(); ++it)
		{
			(*it)->Write(of);
		}

		of.close();
	}
	else
	{
		cout << "player recorder fileName failed: " << fileName << endl;
		assert(false && "failed to open file to write to");
	}
}

PlayerReplayManager::PlayerReplayManager()
{
	replayHUD = new ReplayHUD;
}

PlayerReplayManager::~PlayerReplayManager()
{
	delete replayHUD;

	for (auto it = repVec.begin(); it != repVec.end(); ++it)
	{
		delete (*it);
	}
	repVec.clear();
}

bool PlayerReplayManager::LoadFromFile(const boost::filesystem::path &fileName)
{
	ifstream is;

	is.open(fileName.string(), ios::binary);
	if (is.is_open())
	{
		//init = true;
		//frame = 0;

		header.Read(is);

		repVec.resize(header.numberOfPlayers);

		Session *sess = Session::GetSession();

		for (int i = 0; i < header.numberOfPlayers; ++i)
		{
			repVec[i] = new PlayerReplayer(sess->GetPlayer(i), this);
			repVec[i]->Read(is);
		}

		is.close();
		return true;
	}
	//return false on failure
	return false;
}

void PlayerReplayManager::Reset()
{
	for (auto it = repVec.begin(); it != repVec.end(); ++it)
	{
		if (replaysActive)
		{
			(*it)->replayPlayer->Reset();
		}
		
		if (ghostsActive)
		{
			(*it)->replayGhost->Reset();
		}
	}

	replayHUD->Reset();
}

bool PlayerReplayManager::IsReplayOn(int pIndex)
{
	if (header.numberOfPlayers <= pIndex)
	{
		return false;
	}

	return replaysActive;
}

bool PlayerReplayManager::IsReplayHUDOn(int pIndex)
{
	Session *sess = Session::GetSession();
	Actor *player = sess->GetPlayer(pIndex);
	int action = player->action;

	if (player->IsExitAction(action) || player->IsGoalKillAction(action) || action == Actor::GOALKILLWAIT)
	{
		return false;
	}

	return true;
	/*if ( player->IsIntroAction(action) || (player->IsGoalKillAction(action) && action != Actor::GOALKILLWAIT) || action == EXIT
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || IsSequenceAction(action) || action == EXITBOOST)
	{*/
}

bool PlayerReplayManager::IsGhostOn(int pIndex)
{
	if (header.numberOfPlayers <= pIndex)
	{
		return false;
	}

	return ghostsActive;
}

void PlayerReplayManager::SetToStart()
{
	for (auto it = repVec.begin(); it != repVec.end(); ++it)
	{
		(*it)->replayPlayer->SetToStart();
	}
}

PlayerReplayer *PlayerReplayManager::GetReplayer(int pIndex)
{
	if (pIndex < 0 || pIndex >= header.numberOfPlayers)
	{
		assert(0);
		return NULL;
	}

	return repVec[pIndex];
}