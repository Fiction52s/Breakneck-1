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
#include "NameTag.h"

using namespace sf;
using namespace std;


ReplayGhost::ReplayGhost(PlayerReplayer *p_pReplayer)
	:pReplayer(p_pReplayer), sprBuffer(NULL), playerSkinShader( "player")
{
	frame = 0;
	action = 0;

	nameTag = new NameTag;

	playerSkinShader.SetSkin(Actor::SKIN_GHOST);

	playerSkinShader.pShader.setUniform("u_invincible", 0.f);
	playerSkinShader.pShader.setUniform("u_super", 0.f);
	playerSkinShader.pShader.setUniform("u_slide", 0.f);
}

ReplayGhost::~ReplayGhost()
{
	delete nameTag;

	if (sprBuffer != NULL)
	{
		delete[] sprBuffer;
	}
}

void ReplayGhost::SetSkin(int ind)
{
	playerSkinShader.SetSkin(ind);
}

void ReplayGhost::SetToDefaultSkin()
{
	playerSkinShader.SetSkin(Actor::SKIN_GHOST);
}

void ReplayGhost::SetToReplaySkin()
{
	SetSkin(pReplayer->skinIndex);
}

void ReplayGhost::Read(istream &is)
{
	int readSize = sizeof(SprInfo);
	if (pReplayer->replayManager->header.ver == 1)
	{
		readSize = 36; //old size

		sprBuffer = new SprInfo[pReplayer->numTotalFrames];
		memset(sprBuffer, 0, sizeof(SprInfo) * pReplayer->numTotalFrames);

		for (int i = 0; i < pReplayer->numTotalFrames; ++i)
		{
			is.read((char*)(&sprBuffer[i]), readSize);
		}
	}
	else
	{
		sprBuffer = new SprInfo[pReplayer->numTotalFrames];
		memset(sprBuffer, 0, sizeof(SprInfo) * pReplayer->numTotalFrames);
		is.read((char*)sprBuffer, sizeof(SprInfo) * pReplayer->numTotalFrames);
	}
}

void ReplayGhost::DrawNameTag(sf::RenderTarget *target)
{
	if (frame == pReplayer->numTotalFrames)
		return;

	if (!pReplayer->player->IsVisibleAction(action))
	{
		return;
	}

	nameTag->Draw(target);
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

	action = info.action;

	if (ts != NULL)
	{
		replaySprite.setTexture(*ts->texture);
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

		if (pReplayer->replayManager->header.ver == 1)
		{
			nameTag->SetPos(replaySprite.getPosition());
		}
		else
		{
			nameTag->SetPos(Vector2f(info.playerPos));
		}
	}

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

void ReplayPlayer::Read(istream &is)
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
	pReplayer->player->nameTag->SetActive(true);
	pReplayer->player->nameTag->SetName(pReplayer->displayName);
}

void ReplayPlayer::SetToStart()
{
	frame = 0;
}

//the replay is 1 more frame than the "score" because of how the frames are counted. not a big deal. just keep it in mind
void ReplayPlayer::UpdateInput( ControllerState &state )//ControllerDualStateQueue *controllerInput)
{
	if (frame == pReplayer->numTotalFrames)
		return;

	bool start = state.start;
	bool back = state.back;

	state.SetFromCompressedState(inputBuffer[frame]);

	state.start = start;
	state.back = back;

	//cout << "replaying input frame: " << frame << " out of " << pReplayer->numTotalFrames << "\n";

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

	skinIndex = 0;
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

	displayName = player->GetDisplayName();

	skinIndex = player->sess->GetPlayerNormalSkin(player->actorIndex);

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
	info.playerPos = player->position;
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

	int numChars = displayName.size();
	of.write((char*)&numChars, sizeof(numChars));

	if (numChars > 0)
	{
		of.write(&displayName[0], numChars);
	}
	
	of.write((char*)sprBuffer, numTotalFrames * sizeof(SprInfo));
	of.write((char*)inputBuffer, numTotalFrames * COMPRESSED_INPUT_SIZE);
}

void PlayerRecorder::StopRecordingAndWriteToFile(const std::string &fileName)
{

}

//version 1 is the default
//version 2
	//-Added a V2d playerPos to the replayghost sprite info to track desyncs and also for the nametag

PlayerRecordHeader::PlayerRecordHeader()
	:numberOfPlayers(0),
	bUpgradeField(Session::PLAYER_OPTION_BIT_COUNT),
	bUpgradesTurnedOnField(Session::PLAYER_OPTION_BIT_COUNT),
	bLogField(LogDetailedInfo::MAX_LOGS)
{
	SetVer(2);
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

void PlayerRecordHeader::Read(std::istream &is)
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

PlayerReplayer::PlayerReplayer(PlayerReplayManager *p_replayManager)
{
	replayManager = p_replayManager;
	player = NULL;
	replayGhost = new ReplayGhost(this);
	replayPlayer = new ReplayPlayer(this);
}

PlayerReplayer::~PlayerReplayer()
{
	delete replayGhost;
	delete replayPlayer;
}

void PlayerReplayer::SetPlayer(Actor *p)
{
	if (player == NULL)
	{
		player = p;

		assert(player != NULL);
	}
}

void PlayerReplayer::SetDisplayName(const std::string &n)
{
	displayName = n;
	replayGhost->nameTag->SetName(displayName);
}

int PlayerReplayer::GetFramesBeforeGoal()
{
	return numTotalFrames - 1;
}

bool PlayerReplayer::Read(istream & is)
{
	//init = true;
	//frame = 0;
	
	//is.read()

	is.read((char*)&numTotalFrames, sizeof(numTotalFrames));

	is.read((char*)&skinIndex, sizeof(skinIndex));

	if (replayManager->header.ver == 1)
	{
		//if (skinIndex < 0 || skinIndex >= Actor::SKIN_Count)
		//{
		//	skinIndex = 0; //1.0 version has skins that are uninitialized
		//}
		skinIndex = 0;
	}

	is.read((char*)&startPowerMode, sizeof(startPowerMode));

	if (replayManager->header.ver > 1)
	{
		int numChars = -1;
		is.read((char*)&numChars, sizeof(numChars));

		if (numChars > 0)
		{
			is.read(&displayName[0], numChars);
		}
		else
		{
			displayName = "NOT SET";
		}
	}

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
		LoadFromStream(is);

		is.close();
		return true;
	}
	//return false on failure
	return false;
}

bool PlayerReplayManager::LoadFromStream( std::istream &is)
{
	header.Read(is);

	repVec.resize(header.numberOfPlayers);

	for (int i = 0; i < header.numberOfPlayers; ++i)
	{
		repVec[i] = new PlayerReplayer(this);
		repVec[i]->Read(is);
	}

	Session *sess = Session::GetSession();
	if (sess != NULL)
	{
		SetPlayers();
	}

	return true;
}

void PlayerReplayManager::SetPlayers()
{
	Session *sess = Session::GetSession();

	assert(sess != NULL);
	for (int i = 0; i < header.numberOfPlayers; ++i)
	{
		repVec[i]->SetPlayer(sess->GetPlayer(i));
	}
}

void PlayerReplayManager::AddGhostsToVec(std::vector<ReplayGhost*> &ghosts, bool useReplaySkins)
{
	for (auto it = repVec.begin(); it != repVec.end(); ++it)
	{
		ghosts.push_back((*it)->replayGhost);

		if (useReplaySkins)
		{
			(*it)->replayGhost->SetToReplaySkin();
		}
		else
		{
			(*it)->replayGhost->SetToDefaultSkin();
		}
	}
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

void PlayerReplayManager::DrawGhostNameTags(sf::RenderTarget *target)
{
	if (ghostsActive && !replaysActive)
	{
		for (auto it = repVec.begin(); it != repVec.end(); ++it)
		{
			(*it)->replayGhost->DrawNameTag(target);
		}
	}
}

void PlayerReplayManager::UpdateGhostNameTagsPixelPos(sf::RenderTarget *target)
{
	if (ghostsActive && !replaysActive)
	{
		for (auto it = repVec.begin(); it != repVec.end(); ++it)
		{
			(*it)->replayGhost->nameTag->UpdatePixelPos(target);
		}
	}
}