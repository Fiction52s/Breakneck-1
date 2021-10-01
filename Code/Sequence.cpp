#include "Session.h"
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
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Movement.h"
#include "ScoreDisplay.h"
#include "MusicPlayer.h"
#include "ButtonHolder.h"
#include "Config.h"

#include "Sequence.h"
#include "SequenceW1.h"
#include "SequenceW2.h"
#include "SequenceW3.h"
#include "SequenceW4.h"
#include "SequenceW5.h"
#include "SequenceW6.h"
#include "SequenceW7.h"

using namespace sf;
using namespace std;

Sequence *Sequence::CreateScene(const std::string &name)
{
	Sequence *bScene = NULL;
	if (name == "kinhouse")
	{
		bScene = new StoryScene(name);
	}
	else if (name == "birdscene0")
	{
		bScene = new BirdPreFightScene;
	}
	else  if (name == "birdscene1")
	{
		bScene = new BirdPreFight2Scene;
	}
	else if (name == "crawlerscene0")
	{
		bScene = new CrawlerAttackScene;
	}
	else if (name == "crawlerscene1")
	{
		bScene = new CrawlerPreFightScene;
	}
	else if (name == "crawlerscene2")
	{
		bScene = new CrawlerPostFightScene;
	}
	else if (name == "crawlerscene3")
	{
		bScene = new CrawlerPreFight2Scene;
	}
	else if (name == "birdcrawleralliance")
	{
		bScene = new BirdCrawlerAllianceScene;
	}
	else if (name == "birdtigeralliancescene")
	{
		bScene = new BirdTigerAllianceScene;
	}
	else if (name == "birdtigerapproach")
	{
		bScene = new BirdTigerApproachScene;
	}
	else if (name == "coyotescene0")
	{
		bScene = new CoyoteSleepScene;
	}
	else if (name == "coyotescene1")
	{
		bScene = new CoyotePreFightScene;
	}
	else if (name == "coyotescene2")
	{
		bScene = new CoyoteAndSkeletonScene;
	}
	else if (name == "coyotepost")
	{
		bScene = new CoyotePostFightScene;
	}
	else if (name == "coyotew6entrance")
	{
		bScene = new EnterFortressScene;
	}
	else if (name == "tigerscene0")
	{
		bScene = new TigerPreFightScene;
	}
	else if (name == "birdtigervsscene")
	{
		bScene = new BirdVSTigerScene;
	}
	else if (name == "gatorscene0")
	{
		bScene = new GatorPreFightScene;
	}
	else if (name == "birdchase")
	{
		bScene = new BirdChaseScene;
	}
	else if (name == "birdfinalfight")
	{
		bScene = new BirdPreFight3Scene;

	}
	else if (name == "finalskeletonfight")
	{
		bScene = new FinalSkeletonPreFightScene;
	}
	else if (name == "enterfortress")
	{
		bScene = new EnterFortressScene;
	}
	else if (name == "tigerbirdtunnel")
	{
		bScene = new TigerAndBirdTunnelScene;
	}
	else if (name == "skeletonfight")
	{
		bScene = new SkeletonPreFightScene;
	}
	else if (name == "tigerfight2")
	{
		bScene = new TigerPreFight2Scene;
	}
	else if (name == "posttigerfight")
	{
		bScene = new TigerPostFightScene;
	}
	else
	{
		assert(0);
	}

	if (bScene != NULL)
		bScene->Init();

	return bScene;
}

Sequence::Sequence()
	:frameCount(-1),frame(0),nextSeq(NULL)
{
	sess = Session::GetSession();
	currConvGroup = NULL;
	state = 0;
	numStates = 0;
	stateLength = NULL;
	barrier = NULL;
}

Sequence::~Sequence()
{
	if (nextSeq != NULL)
		delete nextSeq;

	if (stateLength != NULL)
		delete[] stateLength;

	for (auto it = flashGroups.begin(); it != flashGroups.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = flashes.begin(); it != flashes.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = movies.begin(); it != movies.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = bgs.begin(); it != bgs.end(); ++it)
	{
		delete (*it).second;
	}
}

FlashGroup * Sequence::AddFlashGroup(const std::string &n)
{
	assert(flashGroups.count(n) == 0);

	FlashGroup *fg = new FlashGroup();
	flashGroups[n] = fg;
	return fg;
}

void Sequence::Reset()
{
	state = 0;
	frame = 0;
	cIndex = 0;
	currMovie = NULL;
	currFlashGroup = NULL;

	for (auto it = flashGroups.begin(); it != flashGroups.end(); ++it)
	{
		(*it).second->Reset();
	}

	for (auto it = flashes.begin(); it != flashes.end(); ++it)
	{
		(*it).second->Reset();
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
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
}

void Sequence::SetFlashGroup(const std::string & n)
{
	currFlashGroup = flashGroups[n];
}

void Sequence::UpdateFlashGroup()
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
}

void Sequence::UpdateFlashes()
{
	for (auto it = flashes.begin(); it != flashes.end(); ++it)
	{
		(*it).second->Update();
	}
}

void Sequence::UpdateMovie()
{
	assert(currMovie != NULL);

	sfe::Status movStatus = currMovie->getStatus();
	if (frame == 0)
	{
		currMovie->setVolume(sess->mainMenu->config->GetData().musicVolume);
		currMovie->setPlayingOffset(sf::Time::Zero);
		currMovie->play();
	}
	else
	{
		if (movieStopFrame == -1)
		{
			currMovie->update();

			if (sess->GetCurrInput(0).A)
			{
				currMovie->pause();
			}
		}

		if (movieStopFrame == -1 && (movStatus == sfe::Status::End || movStatus == sfe::Status::Stopped
			|| movStatus == sfe::Status::Paused))
		{
			movieStopFrame = frame;
			if (movieFadeFrames == 0)
			{
				EndCurrState();
			}
			else
			{
				frame = stateLength[state] - movieFadeFrames;
				sess->Fade(false, movieFadeFrames, movieFadeColor);
			}
		}

		if (frame == stateLength[state] - 1)
		{
			if (sess->originalMusic != NULL)
			{
				MainMenu *mm = sess->mainMenu;
				mm->musicPlayer->TransitionMusic(sess->originalMusic, 60);
			}
		}
	}
}

void Sequence::AddEnemy(const std::string &enName, Enemy *e)
{
	assert(enemies.count(enName) == 0);

	sess->fullEnemyList.push_back(e);
	enemies[enName] = e;
}

void Sequence::SetConvGroup(const std::string &groupName)
{
	assert(groups.count(groupName) == 1);
	currConvGroup = groups[groupName];
	cIndex = 0;
}

V2d Sequence::GetPointPos(const std::string &s)
{
	assert(points.count(s) == 1);
	return points[s]->pos;
}

PoiInfo *Sequence::GetPoint(const std::string &s)
{
	assert(points.count(s) == 1);
	return points[s];
}

FlashedImage * Sequence::AddFlashedImage(const std::string &imageName, Tileset *ts, int tileIndex,
	int appearFrames, int holdFrames, int disappearFrames, sf::Vector2f &pos)
{
	assert(flashes.count(imageName) == 0);

	FlashedImage *fi = new FlashedImage(ts, tileIndex, appearFrames, holdFrames, disappearFrames, pos);
	flashes[imageName] = fi;
	flashList.push_back(fi);

	return fi;
}

void Sequence::AddGroup(const std::string &groupName, const std::string &fileName)
{
	ConversationGroup *cg = new ConversationGroup;
	cg->Load(fileName);

	assert(groups.count(groupName) == 0);

	groups[groupName] = cg;
}

void Sequence::AddShot(const std::string &shotName)
{
	shots[shotName] = sess->cameraShotMap[shotName];
}

void Sequence::AddPoint(const std::string &poiName)
{
	points[poiName] = sess->poiMap[poiName];
}

void Sequence::AddMovie(const std::string &movieName)
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

void Sequence::ConvUpdate()
{
	Conversation *conv = GetCurrentConv();
	if (frame == 0)
	{
		conv->Show();
	}

	if (sess->GetCurrInput(0).A && !sess->GetPrevInput(0).A)
	{
		conv->NextSection();
	}
	if (sess->GetCurrInput(0).X)
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

SceneBG * Sequence::AddBG(const std::string &bgName, std::list<Tileset*> &anim,
	int animFactor)
{
	SceneBG *sbg = new SceneBG(bgName, anim, animFactor);

	assert(bgs.count(bgName) == 0);

	bgs[bgName] = sbg;

	return sbg;
}

SceneBG *Sequence::GetBG(const std::string &name)
{
	assert(bgs.count(name) > 0);
	return bgs[name];
}

void Sequence::SetCameraShot(const std::string &n)
{
	CameraShot *shot = shots[n];
	sess->cam.Set(shot->centerPos, shot->zoom, sess->cam.zoomLevel);
}

bool Sequence::IsLastFrame()
{
	int sLen = stateLength[state];

	return frame == (sLen - 1);
}

bool Sequence::IsCamMoving()
{
	return sess->cam.easing;
}

void Sequence::BasicFlashUpdateState(const std::string &flashName)
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

void Sequence::EaseShot(const std::string &shotName, int frames,
	CubicBezier bez)
{
	assert(shots.count(shotName) == 1);

	CameraShot *shot = shots[shotName];
	sess->cam.Ease(Vector2f(shot->centerPos), shot->zoom, frames, bez);
}

void Sequence::EasePoint(const std::string &pointName, float targetZoom,
	int frames, CubicBezier bez)
{
	assert(points.count(pointName) == 1);

	PoiInfo *pi = points[pointName];
	sess->cam.Ease(Vector2f(pi->pos), targetZoom, frames, bez);
}

void Sequence::Flash(const std::string &flashName)
{
	assert(flashes.count(flashName) == 1);

	flashes[flashName]->Flash();
}

void Sequence::EndCurrState()
{
	int sLen = stateLength[state];
	frame = sLen - 1;
}

Conversation *Sequence::GetCurrentConv()
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

void Sequence::SetCurrMovie(const std::string &name, int movFadeFrames, Color movFadeColor)
{
	assert(movies.count(name) == 1);
	currMovie = movies[name];
	movieFadeFrames = movFadeFrames;
	movieFadeColor = movFadeColor;
	movieStopFrame = -1;
}

void Sequence::AddSeqFlashToGroup(FlashGroup *fGroup,
	const std::string &n, int delayedStart)
{
	assert(flashes.count(n) == 1);

	fGroup->AddSeqFlash(flashes[n], delayedStart);
}

void Sequence::AddSimulFlashToGroup(FlashGroup *fGroup,
	const std::string &n, int waitFrames)
{
	assert(flashes.count(n) == 1);
	fGroup->AddSimulFlash(flashes[n], waitFrames);
}

void Sequence::Rumble(int x, int y, int duration)
{
	sess->cam.SetRumble(x, y, duration);
}

void Sequence::RumbleDuringState(int x, int y)
{
	if (frame == 0)
	{
		Rumble(x, y, stateLength[state]);
	}
}

void Sequence::SetNumStates(int count)
{
	numStates = count;
	stateLength = new int[numStates];
}

void Sequence::DrawFlashes(sf::RenderTarget *target)
{
	for (auto it = flashList.begin(); it != flashList.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void Sequence::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	View v = target->getView();
	target->setView(sess->uiView);

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

void Sequence::Init()
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

bool Sequence::StateIncrement()
{
	if (frame == stateLength[state] && state != numStates)
	{
		++state;
		frame = 0;

		if (currMovie != NULL)
			currMovie = NULL;
	}

	/*if (sess->GetCurrInput(0).PUp())
	{
		state = numStates;
		frame = stateLength[state];
	}*/

	if ( state == numStates)
	{
		ReturnToGame();
		return false;
	}

	return true;
}

bool Sequence::Update()
{
	if (!StateIncrement())
		return false;

	UpdateFlashes();

	UpdateFlashGroup();

	UpdateState();

	++frame;

	return true;
}


#include "StorySequence.h"
StoryScene::StoryScene(const std::string &name)
{
	story = new StorySequence;
	story->Load(name);
}

StoryScene::~StoryScene()
{
	delete story;
}

void StoryScene::Reset()
{
	Sequence::Reset();
	story->Reset();
}

void StoryScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHOWSTORY] = 999999;
}

void StoryScene::ReturnToGame()
{
	sess->SetGameSessionState(GameSession::RUN);
}

void StoryScene::UpdateState()
{
	story->Update(sess->GetPrevInput(0), sess->GetCurrInput(0));

	if (frame == 0)
	{
		sess->SetGameSessionState(GameSession::SEQUENCE);
	}
}

void StoryScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	story->DrawLayer(target, layer);
}