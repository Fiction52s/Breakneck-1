#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "SaveFile.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "VisualEffects.h"
#include "PauseMenu.h"
#include "ShardMenu.h"
#include "Actor.h"

#include "ParticleEffects.h"

#include "Enemy_LogItem.h"

using namespace std;
using namespace sf;

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

void LogItem::UpdateParamsSettings()
{
	int oldShardType = logType;

	LogParams *lParams = (LogParams*)editParams;
	logWorld = lParams->lInfo.world;
	localIndex = lParams->lInfo.localIndex;
	logType = 0;//GetShardTypeFromWorldAndIndex(shardWorld, localIndex);//shardWorld * 22 + localIndex;//Shard::GetShardType(shardWorld, localIndex);

	/*if (logType != oldShardType)
	{
		ts = Shard::GetShardTileset(shardWorld, sess);

		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(localIndex));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	}*/
}

void LogItem::Setup()
{
	caught = false;
	geoGroup.SetBase(GetPositionF());
}

LogItem::LogItem(ActorParams *ap)//Vector2i pos, int w, int li )
	:Enemy(EnemyType::EN_POWERITEM, ap)//, false, w+1 )
{
	SetNumActions(Count);
	SetEditorActions(FLOAT, FLOAT, 0);

	radius = 400;
	logType = 0;

	ts = sess->GetSizedTileset("Enemies/poweritem_128x128.png");

	//UpdateParamsSettings();

	alreadyCollected = false;

	if (sess->HasLog(logType) )
	{
		alreadyCollected = true;
	}

	sprite.setColor(Color::Red);

	/*if (sess->IsShardCaptured(shardType))
	{
	alreadyCollected = true;
	}*/

	//if (!alreadyCollected)
	//{
	//	sess->TryCreatePowerItemResources();
	//}

	testEmitter = NULL;
	ts_sparkle = NULL;
	ts_explodeCreate = NULL;
	sparklePool = NULL;

	if (!alreadyCollected)
	{
		testEmitter = new ShapeEmitter(6, 300);// PI / 2.0, 2 * PI, 1.0, 2.5);
		testEmitter->CreateParticles();
		testEmitter->SetPos(GetPositionF());
		testEmitter->SetRatePerSecond(30);

		ts_sparkle = sess->GetTileset("Menu/shard_sparkle_64x64.png", 64, 64);

		ts_explodeCreate = sess->GetTileset("FX/shard_explode_01_256x256.png", 256, 256);

		sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
		sparklePool->ts = ts_sparkle;

		BasicCircleHurtBodySetup(32);
		BasicCircleHitBodySetup(32);

		hitBody.hitboxInfo = NULL;

		geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
			Color::Cyan, Color(0, 0, 100, 0), 60));
		geoGroup.Init();
	}


	sprite.setTexture(*ts->texture);

	actionLength[FLOAT] = 120;
	actionLength[DISSIPATE] = 10;
	actionLength[LAUNCH] = 60;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;
	animFactor[LAUNCH] = 1;

	ResetEnemy();

	SetSpawnRect();
}

LogItem::~LogItem()
{
	if (testEmitter != NULL)
	{
		delete testEmitter;
	}

	if (sparklePool != NULL)
	{
		delete sparklePool;
	}
}

void LogItem::ResetEnemy()
{
	if (!alreadyCollected)
	{
		if (sess->HasLog( logType ))
		{
			alreadyCollected = true;
		}
	}

	SetCurrPosInfo(startPosInfo);

	geoGroup.Reset();
	totalFrame = 0;

	if (sparklePool != NULL)
	{
		sparklePool->Reset();
	}

	action = FLOAT;
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();

	if (!alreadyCollected)
	{
		SetHitboxes(&hitBody, 0);
		SetHurtboxes(&hurtBody, 0);

		sprite.setColor(Color(255, 255, 255, 255));
		testEmitter->Reset();
		testEmitter->SetOn(false);
	}
	else
	{
		sprite.setColor(Color(255, 255, 255, 40));
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);
	}

	rootPos = GetPosition();
}

void LogItem::FrameIncrement()
{
	++totalFrame;
}

void LogItem::IHitPlayer(int index)
{
	if (action == FLOAT)
		DissipateOnTouch();
}

void LogItem::DissipateOnTouch()
{
	action = DISSIPATE;
	frame = 0;

	sess->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, GetPosition(), true, 0, 12, 3, true);

	HitboxesOff();
	HurtboxesOff();

	Capture();
}

void LogItem::Capture()
{
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();

		game->UnlockLog(logType);
	}
}

void LogItem::DirectKill()
{

}

void LogItem::Launch()
{
	action = LAUNCH;
	frame = 0;
}

void LogItem::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLOAT:
			frame = 0;
			break;
		case DISSIPATE:
			numHealth = 0;
			dead = true;
			Capture();
			break;
		case LAUNCH:
			action = FLOAT;
			frame = 0;
			break;
		}
	}


	if (action == FLOAT)
	{
		int floatFrames = 240;
		double floatAmount = 4.0;
		int t = totalFrame % floatFrames;
		float tf = t;
		tf /= (floatFrames - 1);
		double f = cos(2 * PI * tf);
		f -= .5;
		currPosInfo.position = rootPos;
		currPosInfo.position.y += f * floatAmount;
	}
	else if (action == LAUNCH)
	{
		currPosInfo.position.y -= 5.0;
		rootPos = currPosInfo.position;

		cout << "position.y: " << currPosInfo.position.y << endl;
	}

	//testEmitter->Update();
	if (!alreadyCollected)
	{
		sparklePool->Update();
		if (!geoGroup.Update())
		{
			geoGroup.Reset();
			geoGroup.Start();
		}

		Vector2f sparkleCenter(GetPositionF());

		if (totalFrame % 60 == 0)
		{
			Vector2f off(rand() % 101 - 50, rand() % 101 - 50);
			EffectInstance ei;

			int r = rand() % 3;
			if (r == 0)
			{
				ei.SetParams(sparkleCenter + off,
					Transform(Transform::Identity), 11, 5, 0);
			}
			else if (r == 1)
			{
				ei.SetParams(sparkleCenter + off,
					Transform(Transform::Identity), 10, 5, 11);
			}
			else if (r == 2)
			{
				ei.SetParams(sparkleCenter + off,
					Transform(Transform::Identity), 10, 5, 11);
			}

			sparklePool->ActivateEffect(&ei);
		}
	}



}

void LogItem::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOAT)
	{
		DissipateOnTouch();
	}
}

void LogItem::UpdateSprite()
{
	int tile = 0;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void LogItem::EnemyDraw(sf::RenderTarget *target)
{
	if (!alreadyCollected)
	{
		geoGroup.Draw(target);

		if (action != DISSIPATE)
		{
			target->draw(sprite);
			sparklePool->Draw(target);
		}
		//testEmitter->Draw(target);
	}
	else
	{
		target->draw(sprite);
	}
}

void LogItem::DrawMinimap(sf::RenderTarget *target)
{
}