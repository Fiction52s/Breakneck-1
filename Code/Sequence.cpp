#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include "ImageText.h"
#include "HUD.h"
#include "StorySequence.h"
#include "Barrier.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "Sequence.h"
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Parallax.h"
#include "Movement.h"
#include "ScoreDisplay.h"
#include "MusicPlayer.h"
#include "ButtonHolder.h"
#include "Config.h"


#include "SequenceW1.h"
#include "SequenceW2.h"
#include "SequenceW3.h"
#include "SequenceW4.h"
#include "SequenceW5.h"
#include "SequenceW6.h"
#include "SequenceW7.h"

using namespace sf;
using namespace std;

#define TIMESTEP 1.0 / 60.0

BasicBossScene *BasicBossScene::CreateScene(GameSession *owner, const std::string &name)
{
	BasicBossScene *bScene = NULL;
	if (name == "birdscene0")
	{
		bScene = new BirdPreFightScene(owner);
	}
	if (name == "birdscene1")
	{
		bScene = new BirdPreFight2Scene(owner);
	}
	else if (name == "crawlerscene0")
	{
		bScene = new CrawlerAttackSeq(owner);
	}
	else if (name == "birdcrawleralliance")
	{
		bScene = new BirdCrawlerAllianceScene(owner);
	}
	else if (name == "birdtigerapproach")
	{
		bScene = new BirdTigerApproachScene(owner);
	}
	else if (name == "coyotescene0")
	{
		bScene = new CoyoteSleepScene(owner);
	}
	else if (name == "coyotescene1")
	{
		bScene = new CoyotePreFightScene(owner);
	}
	else if (name == "coyotescene2")
	{
		bScene = new CoyoteAndSkeletonScene(owner);
	}
	else if (name == "crawlerscene2")
	{
		bScene = new CrawlerPreFight2Scene(owner);
	}
	else if (name == "tigerscene0")
	{
		bScene = new TigerPreFightScene(owner);
	}
	else if (name == "birdtigervsscene")
	{
		bScene = new BirdVSTigerScene(owner);
	}
	else if (name == "gatorscene0")
	{
		bScene = new GatorPreFightScene(owner);
	}
	else if (name == "birdchase")
	{
		bScene = new BirdChaseScene(owner);
	}
	else if (name == "birdfinalfight")
	{
		bScene = new BirdPreFight3Scene(owner);
		
	}
	else if (name == "finalskeletonfight")
	{
		bScene = new FinalSkeletonPreFightScene(owner);
	}
	else if (name == "enterfortress")
	{
		bScene = new TigerAndBirdTunnelScene(owner);
	}
	else if( name == "tigerbirdtunnel")
	{
		bScene = new TigerAndBirdTunnelScene(owner);
	}
	else if (name == "skeletonfight")
	{
		bScene = new SkeletonPreFightScene(owner);
	}
	else if (name == "tigerfight2")
	{
		bScene = new TigerPreFight2Scene(owner);
	}
	else
	{
		assert(0);
	}

	if (bScene != NULL)
		bScene->Init();

	return bScene;
}

Sequence::~Sequence()
{
	if (nextSeq != NULL)
		delete nextSeq;
}

FlashGroup::FlashGroup()
{
	currBGTileset = NULL;
	bg = NULL;
}

void FlashGroup::Reset()
{
	frame = 0;
	done = false;
}

void FlashGroup::SetBG(SceneBG *p_bg)
{
	bg = p_bg;
	if (bg != NULL)
	{
		UpdateBG();
		SetRectCenter(bgQuad, currBGTileset->tileWidth, currBGTileset->tileHeight, Vector2f(960, 540));
		SetRectSubRect(bgQuad, currBGTileset->GetSubRect(0));
	}
}

void FlashGroup::UpdateBG()
{
	if (bg != NULL)
	{
		Tileset *bgTS = bg->GetCurrTileset(frame);
		if (currBGTileset != bgTS)
		{
			currBGTileset = bgTS;
		}

	}
}

void FlashGroup::DrawBG(RenderTarget *target)
{
	if (currBGTileset != NULL)
	{
		target->draw(bgQuad, 4, sf::Quads, currBGTileset->texture);
	}
}

void FlashGroup::TryCurrFlashes()
{
	if (fMap.count(frame) > 0)
	{
		auto &listRef = fMap[frame];
		for (auto it = listRef.begin(); it != listRef.end(); ++it)
		{
			(*it)->image->Flash();
		}
	}

	if (infEndMap.count(frame) > 0)
	{
		auto &listRef = infEndMap[frame];
		for (auto it = listRef.begin(); it != listRef.end(); ++it)
		{
			(*it)->image->StopHolding();
		}
	}
}

void FlashGroup::Update()
{
	if (frame == numFrames)
	{
		done = true;
	}

	if (!done)
	{
		TryCurrFlashes();

		UpdateBG();

		++frame;
	}
	//SetRectSubRect(bgQuad, bgTilesets[0]->)
	//might want to put the frame == numFrames down here, not sure yet.
}

void FlashGroup::Init()
{
	numFrames = 0;

	if (fList.size() > 0)
	{
		int counter = 0;
		for (auto it = fList.begin(); it != fList.end(); ++it)
		{
			counter += (*it)->delayedStart;
			if (counter < 0)
			{
				assert(0);
				counter = 0;
			}

			(*it)->startFrame = counter;
			fMap[counter].push_back((*it));

			if (!(*it)->simul)
			{
				if (!(*it)->image->infiniteHold)
				{
					counter += (*it)->image->GetNumFrames();
				}
			}
			
		}

		FlashInfo *lastFlash = fList.back();

		assert(!lastFlash->image->infiniteHold);
		numFrames = lastFlash->startFrame + lastFlash->image->GetNumFrames();

		for (auto it = fList.begin(); it != fList.end(); ++it)
		{
			if ((*it)->image->infiniteHold)
			{
				infEndMap[(numFrames - 1) - (*it)->image->infiniteHoldEarlyEnd].push_back((*it));
			}
		}
	}
}

void FlashGroup::AddSimulFlash(FlashedImage *fi, int delayedFrames )
{
	fList.push_back(new FlashInfo(fi, delayedFrames, true ));
}

bool FlashGroup::IsDone()
{
	return done;
}

void FlashGroup::AddSeqFlash(FlashedImage *fi,
	int delayedStart)
{
	//delayed start by a negative amount
	fList.push_back(new FlashInfo(fi, delayedStart, false));
}

FlashGroup::~FlashGroup()
{
	for (auto it = fList.begin(); it != fList.end(); ++it)
	{
		delete (*it);
	}
}


PanInfo::PanInfo(sf::Vector2f &pos,
	sf::Vector2f &diff,
	int start, int len)
{
	totalDelta = diff;
	startFrame = start;
	frameLength = len;
	origPos = pos;
}

sf::Vector2f PanInfo::GetCurrPos(int f)
{
	Vector2f dest = origPos + totalDelta;
	double ff = (f - startFrame) + 1;
	double a = ff / frameLength;
	float bezVal = bez.GetValue(a);
	Vector2f final = dest * bezVal + origPos * (1.f - bezVal);
	return final;
}

FlashedImage::FlashedImage(Tileset *ts,
	int tileIndex, int appearFrames,
	int holdFrames,
	int disappearFrames,
	sf::Vector2f &pos)
{
	ts_image = ts;
	ts_split = NULL;
	ts_splitBorder = NULL;
	splitShader = NULL;
	bg = NULL;
	bgSplitShader = NULL;
	currBGTileset = NULL;

	SetRectSubRect(spr, ts->GetSubRect(tileIndex));
	SetRectColor(spr, Color(Color::White));
	//spr.setTexture(*ts->texture);
	//spr.setTextureRect();
	//spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);

	origPos = pos;

	Reset();

	aFrames = appearFrames;
	if (holdFrames <= 0)
	{
		infiniteHoldEarlyEnd = -holdFrames;
		infiniteHold = true;
		holdFrames = 10000000;
	}
	else
	{
		infiniteHold = false;
	}
	hFrames = holdFrames;
	dFrames = disappearFrames;

	
	//splitShader.setUniform("texture", *ts_image->texture);

	//splitShader.setUniform("toColor", ColorGL(keyColor));//Glsl::Vec4( keyColor.r, keyColor.g, keyColor.b, keyColor.a ) );
	//splitShader.setUniform("auraColor", ColorGL(auraColor));//Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a) );
}



FlashedImage::~FlashedImage()
{
	for (auto it = panList.begin(); it != panList.end(); ++it)
	{
		delete (*it);
	}
	
	if (splitShader != NULL)
		delete splitShader;
}

void FlashedImage::SetBG(SceneBG * p_bg)
{
	bg = p_bg;
	if (bg != NULL)
	{
		if (splitShader != NULL)
		{
			bgSplitShader = new Shader;

			if (!bgSplitShader->loadFromFile("Resources/Shader/split_shader.frag", sf::Shader::Fragment))
			{
				cout << "couldnt load enemy split shader" << endl;
				assert(false);
			}

			bgSplitShader->setUniform("u_splitTexture", *ts_split->texture);
			bgSplitShader->setUniform("offset", Vector2f(0, 0));
			bgSplitShader->setUniform("currAlpha", 1.f);

			UpdateBG();

			//tileindex might be different later
			SetRectSubRectGL(split, currBGTileset->GetSubRect(0), Vector2f(currBGTileset->texture->getSize()));
		}
		else
		{

		}
	}
}

void FlashedImage::SetSplit(Tileset *ts, Tileset *borderTS, int tileIndex, sf::Vector2f &pos)
{
	assert(splitShader == NULL);

	ts_split = ts;
	ts_splitBorder = borderTS;

	//shares the same tile index and position of the split

	if (ts_splitBorder != NULL)
	{
		SetRectSubRect(splitBorder, borderTS->GetSubRect(tileIndex));
		SetRectCenter(splitBorder, ts->tileWidth, ts->tileHeight, pos);
	}
	

	SetRectSubRectGL(spr, ts->GetSubRect(tileIndex), Vector2f(ts->texture->getSize()));

	splitSize = ts_split->texture->getSize();
	SetRectSubRect(split, ts->GetSubRect(tileIndex));
	SetRectCenter(split, ts->tileWidth, ts->tileHeight, pos);

	splitShader = new Shader;
	

	if (!splitShader->loadFromFile("Resources/Shader/split_shader.frag", sf::Shader::Fragment))
	{
		cout << "couldnt load enemy split shader" << endl;
		assert(false);
	}

	splitShader->setUniform("u_splitTexture", *ts_split->texture);
	splitShader->setUniform("u_texture", *ts_image->texture);
	splitShader->setUniform("offset", Vector2f(0, 0));
	splitShader->setUniform("currAlpha", 1.f);
}

bool FlashedImage::IsDone()
{
	return (!flashing && frame > 0);
}

int FlashedImage::GetNumFrames()
{
	return aFrames + hFrames + dFrames;
}

void FlashedImage::Reset()
{
	frame = 0;
	flashing = false;
	position = origPos;
	SetRectCenter(spr, ts_image->tileWidth, ts_image->tileHeight, position);
	currPan = NULL;

	UpdateBG();
}

void FlashedImage::AddPan(sf::Vector2f &diff,
	int startFrame, int frameLength)
{
	panList.push_back(new PanInfo( position, diff, startFrame, frameLength));
}

void FlashedImage::AddPanX(float xDiff,
	int startFrame, int frameLength)
{
	panList.push_back(new PanInfo(position, Vector2f(xDiff, 0), startFrame, frameLength));
}

void FlashedImage::AddPanY(float yDiff,
	int startFrame, int frameLength)
{
	panList.push_back(new PanInfo(position, Vector2f( 0, yDiff), startFrame, frameLength));
}

void FlashedImage::Flash()
{
	flashing = true;
	frame = 0;
	SetRectColor(spr, Color(255, 255, 255, 0));
	if( splitShader != NULL )
		splitShader->setUniform("currAlpha", 0.f);
}

bool FlashedImage::IsFadingIn()
{
	return (flashing && frame < aFrames);
}

bool FlashedImage::IsHolding()
{
	return (flashing && frame >= aFrames && frame < aFrames + hFrames);
}

int FlashedImage::GetFramesUntilDone()
{
	if (!flashing)
	{
		return 0;
	}
	else
	{
		int totalFrames = aFrames + hFrames + dFrames;
		return (totalFrames - 1) - frame;
	}
}

void FlashedImage::StopHolding()
{
	frame = aFrames + hFrames;
}

void FlashedImage::Update()
{
	if (!flashing)
		return;

	for (auto it = panList.begin(); it != panList.end(); ++it)
	{
		if ((*it)->startFrame == frame)
		{
			currPan = (*it);
		}
	}

	int a = 0;
	if (IsFadingIn())
	{
		a = (frame / (float)aFrames) * 255.f;
	}
	else if (IsHolding())
	{
		a = 255;
	}
	else //fading out
	{
		int fr = frame - (aFrames + hFrames);
		a = (1.f - fr / (float)dFrames) * 255.f;
	}
	SetRectColor(spr, Color(255, 255, 255, a));
	if (splitShader != NULL)
	{
		float aColor = a / 255.f;
		splitShader->setUniform("currAlpha", aColor);
	}
		

	if (frame == aFrames + hFrames + dFrames)
	{
		flashing = false;
	}

	float tStep = TIMESTEP;

	if (currPan != NULL)
	{
		int fr = frame - currPan->startFrame;
		if (fr == currPan->frameLength)
		{
			currPan = NULL;
		}
		else
		{
			position = currPan->GetCurrPos(frame);
			SetRectCenter(spr, ts_image->tileWidth, ts_image->tileHeight, position);

			if (splitShader != NULL)
			{
				Vector2f offset = position - origPos;

				
				offset.x /= splitSize.x;
				offset.y /= splitSize.y;

				splitShader->setUniform("offset", offset);
			}
		}
	}

	UpdateBG();
	

	++frame;
}

void FlashedImage::UpdateBG()
{
	if (bg != NULL)
	{
		Tileset *bgTS = bg->GetCurrTileset(frame);
		if (currBGTileset != bgTS)
		{
			currBGTileset = bgTS;
			bgSplitShader->setUniform("u_texture", *currBGTileset->texture);
		}

	}
}

void FlashedImage::Draw(sf::RenderTarget *target)
{
	if (flashing)
	{
		if (ts_split != NULL )
		{
			if (bg != NULL)
			{
				target->draw(split, 4, sf::Quads, bgSplitShader);
			}
			else
			{
				target->draw(split, 4, sf::Quads, ts_split->texture);
			}
			
			target->draw(spr, 4, sf::Quads, splitShader);

			if (ts_splitBorder != NULL)
			{
				target->draw(splitBorder, 4, sf::Quads, ts_splitBorder->texture);
			}
		}
		else
		{
			target->draw(spr, 4, sf::Quads, ts_image->texture);
		}
	}

}

SceneBG::SceneBG(const std::string &p_name, list<Tileset*> &p_tilesets, int p_animFactor)
{
	name = p_name;
	tilesets.resize(p_tilesets.size());
	int i = 0;
	for (auto it = p_tilesets.begin(); it != p_tilesets.end(); ++it)
	{
		tilesets[i] = (*it);
		++i;
	}
	animFactor = p_animFactor;
}

Tileset * SceneBG::GetCurrTileset(int frame)
{
	int numBG = tilesets.size();
	if (numBG > 1 )
	{
		int f = (frame / animFactor) % numBG;
		return tilesets[f];
		//SetRectSubRect(bgQuad, tilesets[0]->GetSubRect(0));
	}
	else
	{
		return tilesets[0];
	}
}

//void SceneBG::Draw(sf::RenderTarget *target)
//{
//
//}

BasicBossScene::BasicBossScene(GameSession *p_owner,
	EntranceType et )
	:owner(p_owner)
{
	entranceType = et;
	barrier = NULL;
	currConvGroup = NULL;
	fadeFrames = 60;
	state = 0;
	numStates = 0;
	stateLength = NULL;
	entranceIndex = 0;
}

void BasicBossScene::Reset()
{
	state = 0;
	frame = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		(*it).second->Reset();
	}

	for (auto it = flashes.begin(); it != flashes.end(); ++it)
	{
		(*it).second->Reset();
	}

	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		(*it).second->Reset();
	}

	for (auto it = movies.begin(); it != movies.end(); ++it)
	{
		(*it).second->stop();
	}

	for (auto it = flashGroups.begin(); it != flashGroups.end(); ++it)
	{
		(*it).second->Reset();
	}
	cIndex = 0;
	currMovie = NULL;

	currFlashGroup = NULL;
}

BasicBossScene::~BasicBossScene()
{
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = flashes.begin(); it != flashes.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = movies.begin(); it != movies.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = flashGroups.begin(); it != flashGroups.end(); ++it)
	{
		delete (*it).second;
	}

	if (stateLength != NULL)
		delete[] stateLength;
}

void BasicBossScene::Init()
{
	AddShots();
	AddPoints();

	AddFlashes();
	AddEnemies();
	AddGroups();
	AddMovies();

	Reset();
	SetupStates();

	SpecialInit();
}

SceneBG * BasicBossScene::AddBG( const std::string &bgName, std::list<Tileset*> &anim,
	int animFactor)
{
	SceneBG *sbg = new SceneBG(bgName, anim, animFactor);
	
	assert(bgs.count(bgName) == 0);

	bgs[bgName] = sbg;

	return sbg;
}

SceneBG *BasicBossScene::GetBG(const std::string &name)
{
	assert(bgs.count(name) > 0);
	return bgs[name];
}

void BasicBossScene::AddMovie(const std::string &movieName)
{
	string path = "Resources/Movie/";
	string fileType = ".ogv";
	string fullName = path + movieName + fileType;

	assert(movies.count(movieName) == 0);

	sfe::Movie *mov = new sfe::Movie;
	if (!mov->openFromFile(fullName))
	{
		cout << "movie not loaded: " << fullName << endl;
		assert(false);
	}
	mov->fit(sf::FloatRect(0, 0, 1920, 1080));

	movies[movieName] = mov;
}

void BasicBossScene::UpdateFlashes()
{
	for (auto it = flashes.begin(); it != flashes.end(); ++it)
	{
		(*it).second->Update();
	}
}

void BasicBossScene::UpdateMovie()
{
	assert(currMovie != NULL);

	sfe::Status movStatus = currMovie->getStatus();
	if (frame == 0)
	{
		currMovie->setVolume(owner->mainMenu->config->GetData().musicVolume);
		currMovie->setPlayingOffset(sf::Time::Zero);
		currMovie->play();
	}
	else
	{
		if (movieStopFrame == -1 )
		{
			currMovie->update();

			if (owner->GetCurrInput(0).A)
			{
				currMovie->pause();
			}
		}

		if (movieStopFrame == -1 && (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped
			|| movStatus == sfe::Status::Paused ))
		{
			movieStopFrame = frame;
			if (movieFadeFrames == 0)
			{
				EndCurrState();
			}
			else
			{
				frame = stateLength[state] - movieFadeFrames;
				owner->Fade(false, movieFadeFrames, movieFadeColor);
			}
		}

		if (frame == stateLength[state] - 1)
		{
			if (owner->originalMusic != NULL)
			{
				MainMenu *mm = owner->mainMenu;
				mm->musicPlayer->TransitionMusic(owner->originalMusic, 60);
			}
		}
	}
}

void BasicBossScene::AddEnemy(const std::string &enName, Enemy *e)
{
	assert(enemies.count(enName) == 0);

	owner->fullEnemyList.push_back(e);
	enemies[enName] = e;
}

void BasicBossScene::SetConvGroup(const std::string &groupName)
{
	assert(groups.count(groupName) == 1);
	currConvGroup = groups[groupName];
	cIndex = 0;
}

FlashedImage * BasicBossScene::AddFlashedImage(const std::string &imageName, Tileset *ts, int tileIndex,
	int appearFrames, int holdFrames, int disappearFrames, sf::Vector2f &pos)
{
	assert(flashes.count(imageName) == 0);

	FlashedImage *fi = new FlashedImage(ts, tileIndex, appearFrames, holdFrames, disappearFrames, pos);
	flashes[imageName] = fi;
	flashList.push_back(fi);

	return fi;
}

void BasicBossScene::AddGroup(const std::string &groupName, const std::string &fileName)
{
	ConversationGroup *cg = new ConversationGroup(owner);
	cg->Load(fileName);

	assert(groups.count(groupName) == 0);

	groups[groupName] = cg;
}

void BasicBossScene::AddShot(const std::string &shotName)
{
	shots[shotName] = owner->cameraShotMap[shotName];
}

void BasicBossScene::AddPoint(const std::string &poiName)
{
	points[poiName] = owner->poiMap[poiName];
}

void BasicBossScene::ConvUpdate()
{
	Conversation *conv = GetCurrentConv();
	if (frame == 0)
	{
		conv->Show();
	}

	if (owner->GetCurrInput(0).A && !owner->GetPrevInput(0).A)
	{
		conv->NextSection();
	}
	if (owner->GetCurrInput(0).B)
	{
		conv->SetRate(1, 5);
	}
	else
	{
		conv->SetRate(1, 1);
	}

	if (!conv->Update())
	{
		if (cIndex < currConvGroup->numConvs - 1)
		{
			++cIndex;

			Conversation *newconv = currConvGroup->GetConv(cIndex);

			newconv->Show();
		}
		else
		{
			EndCurrState();
		}
	}
}

void BasicBossScene::StartRunning()
{
	owner->SetPlayerInputOn(false);
}

void BasicBossScene::StartEntranceRun(bool fr,
	double maxSpeed, const std::string &n0,
	const std::string &n1)
{
	PoiInfo *kinStart = points[n0];
	PoiInfo *kinStop = points[n1];
	owner->GetPlayer(0)->SetStoryRun(fr, maxSpeed, kinStart->edge, kinStart->edgeQuantity, kinStop->edge,
		kinStop->edgeQuantity);
}

void BasicBossScene::StartEntranceStand(bool fr,
	const std::string &n)
{
	PoiInfo *kinStand = points[n];
	assert(kinStand->edge != NULL);
	Actor *player = owner->GetPlayer(0);
	player->facingRight = fr;
	player->SetGroundedPos(kinStand->edge, kinStand->edgeQuantity);
	player->StandInPlace();
}

void BasicBossScene::SetCameraShot(const std::string &n)
{
	CameraShot *shot = shots[n];
	owner->cam.Set(shot->centerPos, shot->zoom, owner->cam.zoomLevel);
}

void BasicBossScene::SetEntranceRun()
{
	string entranceIndexStr = to_string(entranceIndex);
	string startStr = "kinstart" + entranceIndexStr;
	string endStr = "kinstop" + entranceIndexStr;

	StartEntranceRun(true, 10.0, startStr, endStr);
}

void BasicBossScene::SetEntranceIndex(int ind)
{
	entranceIndex = ind;
}

void BasicBossScene::SetEntranceStand()
{
	string standString = "kinstand" + to_string(entranceIndex);

	StartEntranceStand(true, standString);
}

void BasicBossScene::AddStartPoint()
{
	string startStr = "kinstart" + to_string(entranceIndex);
	AddPoint(startStr);
}

void BasicBossScene::AddStopPoint()
{
	string stopStr = "kinstop" + to_string(entranceIndex);
	AddPoint(stopStr);
}

void BasicBossScene::AddStandPoint()
{
	string standStr = "kinstand" + to_string(entranceIndex);
	AddPoint(standStr);
}

void BasicBossScene::AddStartAndStopPoints()
{
	AddStartPoint();
	AddStopPoint();
}

void BasicBossScene::SetEntranceShot()
{
	//if (shots.count("scenecam") == 1)
	{
		SetCameraShot("scenecam");
	}
	//else
	{
		//assert(0);
	}
}


void BasicBossScene::EntranceUpdate()
{
	Actor *player = owner->GetPlayer(0);

	if (entranceType == RUN)
	{
		if (frame == 0)
		{
			owner->Fade(false, fadeFrames, Color::Black);
			owner->adventureHUD->Hide(fadeFrames);
			player->Wait();
			owner->cam.SetManual(true);
		}
		else if (frame == fadeFrames)
		{
			barrier->Trigger();
			owner->RemoveAllEnemies();
			owner->Fade(true, fadeFrames, Color::Black);
			SetEntranceShot();
			SetEntranceRun();
		}
	}
	else if( entranceType == APPEAR)
	{
		//owner->adventureHUD->Hide(fadeFrames);
	}
}

void BasicBossScene::SetPlayerStandPoint(const std::string &n,
	bool fr)
{
	assert(points.count(n) == 1);

	PoiInfo *pi = points[n];
	Actor *player = owner->GetPlayer(0);
	player->SetStandInPlacePos(pi->edge, pi->edgeQuantity, fr);
}

void BasicBossScene::SetPlayerStandDefaultPoint(bool fr)
{
	string standStr = "kinstand" + to_string(entranceIndex);
	SetPlayerStandPoint(standStr, fr);
}

void BasicBossScene::ReturnToGame()
{
	owner->SetPlayerInputOn(true);
	owner->adventureHUD->Show(60);
	owner->cam.EaseOutOfManual(60);
}

bool BasicBossScene::IsAutoRunState()
{
	if (entranceType == RUN)
	{
		return state == 0;
	}
	else
	{
		return false;
	}
	
}

void BasicBossScene::Wait()
{
	state = 1;
	frame = 0;
}

Conversation *BasicBossScene::GetCurrentConv()
{
	if (currConvGroup != NULL)
	{
		return currConvGroup->GetConv(cIndex);
	}
	else
	{
		return NULL;
	}
}

void BasicBossScene::SetCurrMovie(const std::string &name, int movFadeFrames, Color movFadeColor )
{
	assert(movies.count(name) == 1);
	currMovie = movies[name];
	movieFadeFrames = movFadeFrames;
	movieFadeColor = movFadeColor;
	movieStopFrame = -1;
}

bool BasicBossScene::Update()
{
	Actor *player = owner->GetPlayer(0);

	if (frame == stateLength[state] && state != numStates)
	{
		++state;
		frame = 0;

		if (currMovie != NULL)
			currMovie = NULL;
	}

	if (state == numStates)
	{
		ReturnToGame();
		return false;
	}

	if (IsAutoRunState() && !player->IsAutoRunning() && frame > 60)
	{
		state++;
		frame = 0;
		//Wait();
	}

	UpdateFlashes();

	UpdateFlashGroup();

	UpdateState();

	++frame;

	return true;
}

void BasicBossScene::BasicFlashUpdateState( const std::string &flashName )
{
	FlashedImage *fi = flashes[flashName];
	if (frame == 0)
	{
		Conversation *c = GetCurrentConv();
		if (c != NULL)
			c->Hide();

		fi->Flash();
	}
	else if (fi->IsDone())
	{
		frame = stateLength[state] - 1;
	}
}

void BasicBossScene::EaseShot(const std::string &shotName, int frames,
	CubicBezier bez )
{
	assert(shots.count(shotName) == 1);

	CameraShot *shot = shots[shotName];
	owner->cam.Ease(Vector2f(shot->centerPos), shot->zoom, frames, bez);
}

void BasicBossScene::EasePoint(const std::string &pointName, float targetZoom, 
	int frames, CubicBezier bez)
{
	assert(points.count(pointName) == 1);

	PoiInfo *pi = points[pointName];
	owner->cam.Ease(Vector2f(pi->pos), targetZoom, frames, bez);
}

void BasicBossScene::Flash(const std::string &flashName)
{
	assert(flashes.count(flashName) == 1);

	flashes[flashName]->Flash();
}

void BasicBossScene::SetFlashGroup( const std::string & n )
{
	currFlashGroup = flashGroups[n];


	//(*currFlashGroup->currFlash)->image->Flash();
}

void BasicBossScene::UpdateFlashGroup()
{
	if (currFlashGroup == NULL)
	{
		return;
	}

	(*currFlashGroup).Update();

	if ((*currFlashGroup).IsDone())
	{
		currFlashGroup = NULL;
	}
	//FlashedImage *fg = (*currFlashGroup->currFlash)->image;
	//int earlyEnd = (*currFlashGroup->currFlash)->earlyEnd;

	////cout << "updating" << endl;

	//bool startNext = false;
	//if (fg->GetFramesUntilDone() <= earlyEnd)
	//{
	//	startNext = true;
	//}

	//if (startNext)
	//{
	//	currFlashGroup->currFlash++;
	//	if (currFlashGroup->currFlash == currFlashGroup->fList.end())
	//	{
	//		currFlashGroup->done = true;
	//		currFlashGroup = NULL;
	//	}
	//	else
	//	{
	//		(*currFlashGroup->currFlash)->image->Flash();
	//	}
	//}
}

void BasicBossScene::EndCurrState()
{
	int sLen = stateLength[state];
	frame = sLen - 1;
	/*if (sLen == -1)
	{
		frame = -2;
	}
	else
	{
		frame = stateLength[state] - 1;
	}*/
}

bool BasicBossScene::IsLastFrame()
{
	int sLen = stateLength[state];

	return frame == (sLen - 1);
}

bool BasicBossScene::IsCamMoving()
{
	return owner->cam.easing;
}

FlashGroup * BasicBossScene::AddFlashGroup(const std::string &n)
{
	assert(flashGroups.count(n) == 0);

	FlashGroup *fg = new FlashGroup();
	flashGroups[n] = fg;
	return fg;
}

void BasicBossScene::AddSeqFlashToGroup(FlashGroup *fGroup,
	const std::string &n, int delayedStart )
{
	assert(flashes.count(n) == 1);

	fGroup->AddSeqFlash(flashes[n], delayedStart);
}

void BasicBossScene::AddSimulFlashToGroup(FlashGroup *fGroup,
	const std::string &n, int waitFrames )
{
	assert(flashes.count(n) == 1);
	fGroup->AddSimulFlash(flashes[n], waitFrames);
}

void BasicBossScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	View v = target->getView();
	target->setView(owner->uiView);

	//temporarily have this here, eventually move the BGs as another thing you can have a bunch of in a sequence.
	if (currFlashGroup != NULL)
	{
		(*currFlashGroup).DrawBG(target);
	}

	

	DrawFlashes(target);

	if (currConvGroup != NULL)
	{
		Conversation *conv = currConvGroup->GetConv(cIndex);
		conv->Draw(target);
	}
	
	target->setView(v);

	if (currMovie != NULL)
	{
		target->draw(*currMovie);
	}
}

void BasicBossScene::DrawFlashes(sf::RenderTarget *target)
{
	for (auto it = flashList.begin(); it != flashList.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void BasicBossScene::Rumble(int x, int y, int duration)
{
	owner->cam.SetRumble(x, y, duration);
}

void BasicBossScene::RumbleDuringState(int x, int y)
{
	if (frame == 0)
	{
		Rumble(x, y, stateLength[state]);
	}
}

void BasicBossScene::SetNumStates(int count)
{
	numStates = count;
	stateLength = new int[numStates];
}


ShipExitScene::ShipExitScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{
	enterTime = 60;
	exitTime = 60 + 60;
	center.AddLineMovement(V2d(0, 0), V2d(0, 0),
		CubicBezier(0, 0, 1, 1), 60);
	shipMovement.AddLineMovement(V2d(0, 0),
		V2d(0, 0), CubicBezier(0, 0, 1, 1), enterTime);
	shipMovement.AddLineMovement(V2d(0, 0),
		V2d(0, 0), CubicBezier(0, 0, 1, 1), exitTime);

	ts_ship = owner->GetTileset("Ship/ship_exit_864x540.png", 864, 540);
	shipSprite.setTexture(*ts_ship->texture);
	shipSprite.setTextureRect(ts_ship->GetSubRect(0));
	shipSprite.setOrigin(421, 425);
}

void ShipExitScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHIP_SWOOP] = -1;
}

void ShipExitScene::AddShots()
{
}

void ShipExitScene::AddEnemies()
{
}

void ShipExitScene::AddFlashes()
{
}

void ShipExitScene::ReturnToGame()
{
	//Actor *player = owner->GetPlayer(0);

	//BasicBossScene::ReturnToGame();
}

void ShipExitScene::UpdateState()
{
	Actor *player = owner->GetPlayer(0);
	switch (state)
	{
	case SHIP_SWOOP:
	{
		int shipOffsetY = -200;
		int pOffsetY = -170;
		int sOffsetY = pOffsetY;//shipOffsetY + pOffsetY;
		int jumpLength = 6 * 5;
		int startGrabWire = enterTime - jumpLength;

		if (frame == 0)
		{
			owner->cam.SetManual(true);
			center.movementList->start = V2d(owner->cam.pos.x, owner->cam.pos.y);
			center.movementList->end = V2d(owner->GetPlayer(0)->position.x,
				owner->GetPlayer(0)->position.y - 200);

			center.Reset();
			owner->cam.SetMovementSeq(&center, false);

			abovePlayer = V2d(player->position.x, player->position.y - 300);

			shipMovement.movementList->start = abovePlayer + V2d(-1500, -900);//player->position + V2d( -1000, sOffsetY );
			shipMovement.movementList->end = abovePlayer;//player->position + V2d( 1000, sOffsetY );
			shipMovement.Reset();

			Movement *m = shipMovement.movementList->next;

			m->start = abovePlayer;
			m->end = abovePlayer + V2d(1500, -900) + V2d(1500, -900);

			origPlayer = owner->GetPlayer(0)->position;
			attachPoint = abovePlayer;//V2d(player->position.x, player->position.y);//abovePlayer.y + 170 );
		}
		else  if (frame == startGrabWire)
		{
			owner->GetPlayer(0)->GrabShipWire();
		}

		for (int i = 0; i < NUM_MAX_STEPS; ++i)
		{
			shipMovement.Update();
		}

		int jumpSquat = startGrabWire + 3 * 5;
		int startJump = 4 * 5;//60 - jumpSquat;

		if (frame > enterTime)
		{
			owner->GetPlayer(0)->position = V2d(shipMovement.position.x, shipMovement.position.y + 48.0);
		}
		else if (frame >= jumpSquat && frame <= enterTime)//startJump )
		{
			double adjF = frame - jumpSquat;
			double eTime = enterTime - jumpSquat;
			double a = adjF / eTime;//(double)(frame - (60 - (startJump + 1))) / (60 - (startJump - 1));
									//double a = 
									//cout << "a: " << a << endl;
			V2d pAttachPoint = attachPoint;
			pAttachPoint.y += 48.f;
			owner->GetPlayer(0)->position = origPlayer * (1.0 - a) + pAttachPoint * a;
		}

		if (shipMovement.currMovement == NULL)
		{
			frame = stateLength[SHIP_SWOOP] - 1;
			owner->mainMenu->musicPlayer->FadeOutCurrentMusic(30);
		}

		if (frame == (enterTime + exitTime) - 60)
		{
			owner->Fade(false, 60, Color::Black);
		}

		shipSprite.setPosition(shipMovement.position.x,
			shipMovement.position.y);
		break;
	}
	}
}

void ShipExitScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state == SHIP_SWOOP)
	{
		target->draw(shipSprite);
	}
}