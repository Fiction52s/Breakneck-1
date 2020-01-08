#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
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
	else if (name == "crawlerscene0")
	{
		bScene = new CrawlerAttackSeq(owner);
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

ShipExitSeq::ShipExitSeq( GameSession *p_owner )
	:owner( p_owner )
{
	enterTime = 60;
	exitTime = 60 + 60;
	center.AddLineMovement( V2d( 0, 0 ), V2d( 0, 0 ), 
		CubicBezier( 0, 0, 1, 1 ), 60 );
	//shipMovement.AddCubicMovement( 
	shipMovement.AddLineMovement( V2d( 0, 0 ), 
		V2d( 0, 0 ), CubicBezier( 0, 0, 1, 1 ), enterTime );
	shipMovement.AddLineMovement( V2d( 0, 0 ), 
		V2d( 0, 0 ), CubicBezier( 0, 0, 1, 1 ), exitTime );

	ts_ship = owner->GetTileset("Ship/ship_exit_864x540.png", 864, 540);
	shipSprite.setTexture( *ts_ship->texture );
	shipSprite.setTextureRect( ts_ship->GetSubRect( 0 ) );
	//shipSprite.setOrigin(560, 700);
	//shipSprite.setOrigin(960, 700);
	//shipSprite.setOrigin(960 / 2, 700);
	shipSprite.setOrigin(421, 425);

	storySeq = new StorySequence(owner);
	storySeq->Load("world1outro");
	//assert(mov.openFromFile("Resources/Movie/kin_ship.ogv"));
	//mov.fit(sf::FloatRect(0, 0, 1920, 1080));

	stateLength[SHIP_SWOOP]= 1000000;
	//stateLength[FADEOUT] = 90;

	stateLength[STORYSEQ] = 1000000;
}

ShipExitSeq::~ShipExitSeq()
{
	delete storySeq;
}

bool ShipExitSeq::Update()
{
	Actor *player = owner->GetPlayer( 0 );

	if (frame == stateLength[state] && state != END)
	{
		int s = state;
		s++;
		state = (State)s;
		frame = 0;

		if (state == END)
		{
			return false;
		}
	}

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
			m->end = abovePlayer + V2d(1500, -900) + V2d( 1500, -900 );

			origPlayer = owner->GetPlayer(0)->position;
			attachPoint = abovePlayer;//V2d(player->position.x, player->position.y);//abovePlayer.y + 170 );
		}
		else  if (frame == startGrabWire)
		{
			owner->GetPlayer(0)->GrabShipWire();
		}

		for (int i = 0; i < NUM_STEPS; ++i)
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
			owner->state = GameSession::SEQUENCE;
		}

		if (frame == (enterTime + exitTime) - 60)
		{
			owner->Fade(false, 60, Color::Black);
		}

		shipSprite.setPosition(shipMovement.position.x,
			shipMovement.position.y);
		break;
	}
	//case FADEOUT:
	//{
	//	if (frame == 30)
	//	{
			//owner->Fade(false, 60, Color::Black);
	//	}
	//	break;
	//}
	case STORYSEQ:
	{
		if (frame == 0)
		{
			owner->ClearFade();
			owner->SetStorySeq(storySeq);
		}
		
		
		//if (!storySeq->Update(owner->GetPrevInputUnfiltered(0), owner->GetCurrInputUnfiltered(0)))
		if( owner->currStorySequence == NULL )
		{
			frame = stateLength[STORYSEQ] - 1;
			owner->goalDestroyed = true;
		}

		break;
	}
	}
	++frame;

	return true;
}

void ShipExitSeq::Draw( RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	if (state == SHIP_SWOOP)
	{
		target->draw(shipSprite);
	}
	else if (state == STORYSEQ)
	{
		//mov.update();
		//target->draw(mov);
		storySeq->Draw(target);
	}
	
}

void ShipExitSeq::Reset()
{
	frame = 0;
	state = SHIP_SWOOP;
	storySeq->Reset();
}

FlashedImage::FlashedImage(Tileset *ts,
	int tileIndex, int appearFrames,
	int holdFrames,
	int disappearFrames,
	sf::Vector2f &pos)
{
	spr.setTexture(*ts->texture);
	spr.setTextureRect(ts->GetSubRect(tileIndex));
	spr.setOrigin(spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2);
	spr.setPosition(pos);

	Reset();

	aFrames = appearFrames;
	hFrames = holdFrames;
	dFrames = disappearFrames;
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
}

void FlashedImage::Flash()
{
	flashing = true;
	frame = 0;
}

bool FlashedImage::IsFadingIn()
{
	return (flashing && frame < aFrames);
}

bool FlashedImage::IsHolding()
{
	return (flashing && frame >= aFrames && frame < aFrames + hFrames);
}

void FlashedImage::StopHolding()
{
	frame = aFrames + hFrames;
}

void FlashedImage::Update()
{
	if (!flashing)
		return;

	int a = 0;
	if (IsFadingIn())
	{
		a = (frame / (float)aFrames) * 255.f;
	}
	else if (IsHolding())
	{
		a = 255;
	}
	else
	{
		int fr = frame - (aFrames + hFrames);
		a = (1.f - fr / (float)dFrames) * 255.f;
	}
	spr.setColor(Color(255, 255, 255, a));

	if (frame == aFrames + hFrames + dFrames)
	{
		flashing = false;
	}

	++frame;
}

void FlashedImage::Draw(sf::RenderTarget *target)
{
	if (flashing)
	{
		target->draw(spr);
	}

}

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
	cIndex = 0;
	currMovie = NULL;
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
				frame = stateLength[state] - 1;
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
}

void BasicBossScene::AddFlashedImage(const std::string &imageName, Tileset *ts, int tileIndex,
	int appearFrames, int holdFrames, int disappearFrames, sf::Vector2f &pos)
{
	assert(flashes.count(imageName) == 0);

	FlashedImage *fi = new FlashedImage(ts, tileIndex, appearFrames, holdFrames, disappearFrames, pos);
	flashes[imageName] = fi;
	flashList.push_back(fi);
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
	StartEntranceRun(true, 10.0, "kinstart", "kinstop");
}

void BasicBossScene::SetEntranceStand()
{
	StartEntranceStand(true, "kinstand");
}

void BasicBossScene::SetEntranceShot()
{
	SetCameraShot("scenecam");
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

void BasicBossScene::ReturnToGame()
{
	owner->SetPlayerInputOn(true);
	owner->adventureHUD->Show(60);
	owner->cam.EaseOutOfManual(60);
}

bool BasicBossScene::IsEntering()
{
	return state == 0;
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

	if (entranceType == RUN)
	{
		if (IsEntering() && !player->IsAutoRunning() && frame > 60)
		{
			Wait();
		}
	}

	UpdateState();

	UpdateFlashes();

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

void BasicBossScene::EndCurrState()
{
	int sLen = stateLength[state];
	if (sLen == -1)
	{
		frame = -2;
	}
	else
	{
		frame = stateLength[state] - 1;
	}
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

void BasicBossScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer != EffectLayer::IN_FRONT)
	{
		return;
	}

	View v = target->getView();
	target->setView(owner->uiView);

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
