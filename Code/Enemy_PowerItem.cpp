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
#include "PowerSequence.h"

#include "ParticleEffects.h"

#include "Enemy_PowerItem.h"

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

//std::map<std::string, ShardType> Shard::shardTypeMap;
//std::map<ShardType, std::string> Shard::shardStrMap;

//void Shard::UpdateParamsSettings()
//{
//	int oldShardType = shardType;
//
//	ShardParams *sParams = (ShardParams*)editParams;
//	shardWorld = sParams->shInfo.world;
//	localIndex = sParams->shInfo.localIndex;
//	shardType = Shard::GetShardType(shardWorld, localIndex);
//
//	if (shardType != oldShardType)
//	{
//		switch (shardWorld)
//		{
//		case 0://ShardType::SHARD_W1_TEACH_JUMP:
//		default:
//			ts = sess->GetTileset("Shard/shards_w1_192x192.png", 192, 192);
//			sprite.setTexture(*ts->texture);
//			sprite.setTextureRect(ts->GetSubRect(localIndex));
//			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
//			break;
//		}
//	}
//}

void PowerItem::Setup()
{
	caught = false;
	geoGroup.SetBase(GetPositionF());
}

PowerItem::PowerItem(ActorParams *ap)//Vector2i pos, int w, int li )
	:Enemy(EnemyType::EN_POWERITEM, ap)//, false, w+1 )
{
	SetNumActions(Count);
	SetEditorActions(FLOAT, FLOAT, 0);

	radius = 400;
	powerIndex = -1;

	powerIndex = ap->GetLevel() - 1;

	switch (powerIndex)
	{
	case 0:
		sprite.setColor(COLOR_BLUE);
		break;
	case 1:
		sprite.setColor(COLOR_GREEN);
		break;
	case 2:
		sprite.setColor(COLOR_YELLOW);
		break;
	case 3:
		sprite.setColor(COLOR_ORANGE);
		break;
	case 4:
		sprite.setColor(COLOR_RED);
		break;
	case 5:
		sprite.setColor(COLOR_MAGENTA);
		break;
	}

	ts = sess->GetSizedTileset("Enemies/poweritem_128x128.png");

	UpdateParamsSettings();

	alreadyCollected = false;

	if (sess->GetPlayer(0)->HasUpgrade(Actor::UPGRADE_POWER_AIRDASH + powerIndex))
	{
		alreadyCollected = true;
	}

	/*if (sess->IsShardCaptured(shardType))
	{
		alreadyCollected = true;
	}*/

	if (!alreadyCollected)
	{
		sess->TryCreatePowerItemResources();
	}

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

PowerItem::~PowerItem()
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

void PowerItem::ResetEnemy()
{
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

void PowerItem::FrameIncrement()
{
	++totalFrame;
}

void PowerItem::IHitPlayer(int index)
{
	if (action == FLOAT)
		DissipateOnTouch();
}

void PowerItem::DissipateOnTouch()
{
	action = DISSIPATE;
	frame = 0;

	sess->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, GetPosition(), true, 0, 12, 3, true);

	SetHitboxes(NULL, 0);
	SetHurtboxes(NULL, 0);

	GetPowerSequence *gps = (GetPowerSequence*)sess->getPowerSeq;
	gps->powerItem = this;
	gps->Reset();
	sess->SetActiveSequence(gps);

	Actor *player = sess->GetPlayer(0);
	player->SetAction(Actor::GETSHARD);
	player->frame = 0;
	player->velocity = V2d(0, 0);
	//Capture();
}

void PowerItem::Capture()
{
	sess->GetPlayer(0)->SetStartUpgrade(Actor::UPGRADE_POWER_AIRDASH + powerIndex, true);

	if (powerIndex == 5)
	{
		//left wire also
		sess->GetPlayer(0)->SetStartUpgrade(Actor::UPGRADE_POWER_AIRDASH + powerIndex + 1, true);
	}

	//if (sess->IsSessTypeGame())
	//{
	//	GameSession *game = GameSession::GetSession();
	//	if (game->saveFile != NULL)
	//	{
	//		assert(!game->saveFile->shardField.GetBit(shardType));
	//		//both give you the shard and mark it as a new shard
	//		game->saveFile->shardField.SetBit(shardType, true);
	//		game->saveFile->newShardField.SetBit(shardType, true);
	//		game->saveFile->Save();
	//	}
	//}
}

void PowerItem::DirectKill()
{

}

void PowerItem::Launch()
{
	action = LAUNCH;
	frame = 0;
}

void PowerItem::ProcessState()
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

void PowerItem::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOAT)
	{
		DissipateOnTouch();
	}
}

void PowerItem::UpdateSprite()
{
	int tile = 0;

	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void PowerItem::EnemyDraw(sf::RenderTarget *target)
{
	if (!alreadyCollected)
	{


		geoGroup.Draw(target);

		if (action != DISSIPATE)
		{
			target->draw(sprite);
			sparklePool->Draw(target);
		}

		testEmitter->Draw(target);
	}
	else
	{
		target->draw(sprite);
	}
}

void PowerItem::DrawMinimap(sf::RenderTarget *target)
{
}

PowerPopup::PowerPopup()
{
	sess = Session::GetSession();

	powerIndex = -1;

	desc.setCharacterSize(20);
	desc.setFillColor(Color::White);
	desc.setFont(sess->mainMenu->arial);

	descRel = Vector2f(200, 20);
	effectRel = Vector2f(20, 20);
	powerRel = Vector2f(100, 20);

	Tileset *ts_bg = sess->GetTileset("Menu/GetShard/getshardbg.png", 0, 0);
	bgSpr.setTexture(*ts_bg->texture);
}

void PowerPopup::Reset()
{
	state = SHOW;
	frame = 0;
}

void PowerPopup::Update()
{
	++frame;
}

void PowerPopup::Draw(RenderTarget *target)
{
	target->draw(bgSpr);
	//target->draw(powerSpr);
	target->draw(effectSpr);
	target->draw(desc);
}

void PowerPopup::SetPower(int index)
{
	powerIndex = index;



	//Tileset *ts_shard = Shard::GetShardTileset(w, sess);
	//shardSpr.setTexture(*ts_shard->texture);
	//shardSpr.setTextureRect(ts_shard->GetSubRect(li));

	//effectSpr.setTexture(*shardSpr.getTexture());
	//effectSpr.setTextureRect(shardSpr.getTextureRect());

	switch (index)
	{
	case 0:
		desc.setString("You unlocked airdash! Hold dash in the air to dash in any of the 8 directions!");
		break;
	case 1:
		desc.setString("You unlocked gravity cling! Hold dash and up while touching a ceiling to reverse gravity and stick!");
		break;
	case 2:
		desc.setString("You unlocked bounce scorpion! Use left c stick to change modes, and the shield button to toggle!");
		break;
	case 3:
		desc.setString("You unlocked grind! Use right c stick to change modes, and the shield button to move on any surface!");
		break;
	case 4:
		desc.setString("You unlocked time slow bubbles! Use down c stick to change modes, and the shield button to slow down enemies and yourself!");
		break;
	case 5:
		desc.setString("You unlocked double wires! Use the triggers to swing and move around anywhere!");
		break;
	}

	//string test = sess->mainMenu->pauseMenu->shardMenu->GetShardDesc(0, 0);
	//desc.setString(test);
}

void PowerPopup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;
	powerSpr.setPosition(topLeft + powerRel);
	effectSpr.setPosition(topLeft + effectRel);
	desc.setPosition(topLeft + descRel);
	bgSpr.setPosition(topLeft);
}

void PowerPopup::SetCenter(sf::Vector2f &pos)
{
	float width = bgSpr.getLocalBounds().width;
	float height = bgSpr.getLocalBounds().height;

	SetTopLeft(Vector2f(pos.x - width / 2, pos.y - height / 2));
}