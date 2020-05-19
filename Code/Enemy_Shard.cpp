#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Shard.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "VisualEffects.h"
#include "PauseMenu.h"
#include "ShardMenu.h"
#include "Actor.h"
#include "ShardSequence.h"

#include "ParticleEffects.h"

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

std::map<std::string, ShardType> Shard::shardTypeMap;
std::map<ShardType, std::string> Shard::shardStrMap;

void Shard::UpdateParamsSettings()
{
	int oldShardType = shardType;

	ShardParams *sParams = (ShardParams*)editParams;
	shardWorld = sParams->world;
	localIndex = sParams->localIndex;
	shardType = Shard::GetShardType(shardWorld, localIndex);

	if (shardType != oldShardType)
	{
		switch (shardWorld)
		{
		case 0://ShardType::SHARD_W1_TEACH_JUMP:
		default:
			ts = sess->GetTileset("Shard/shards_w1_192x192.png", 192, 192);
			sprite.setTexture(*ts->texture);
			sprite.setTextureRect(ts->GetSubRect(localIndex));
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			break;
		}
	}
}

void Shard::SetupShardMaps()
{
	shardTypeMap["SHARD_W1_TEACH_JUMP"] = SHARD_W1_0_TEACH_JUMP;
	shardTypeMap["BACKWARDS_DASH_JUMP"] = SHARD_W1_1_BACKWARDS_DASH_JUMP;
	shardTypeMap["SHARD_W1_GET_AIRDASH"] = SHARD_W1_2_GET_AIRDASH;
	for (auto it = shardTypeMap.begin(); it != shardTypeMap.end(); ++it)
	{
		shardStrMap[(*it).second] = (*it).first;
	}
}

void Shard::Setup()
{
	caught = false;
	geoGroup.SetBase(GetPositionF());
}

Shard::Shard(ActorParams *ap )//Vector2i pos, int w, int li )
	:Enemy( EnemyType::EN_SHARD, ap )//, false, w+1 )
{
	SetNumActions(Count);
	SetEditorActions(FLOAT, FLOAT, 0);

	ShardParams *sParams = (ShardParams*)ap;

	//SetCurrPosInfo(startPosInfo);

	if (sess->IsSessTypeGame())
	{
		game = GameSession::GetSession();
	}
	else
	{
		game = NULL;
	}

	if (game != NULL)
	{
		game->TryCreateShardResources();
	}
	
	testEmitter = new ShapeEmitter(6, 300);// PI / 2.0, 2 * PI, 1.0, 2.5);
	testEmitter->CreateParticles();
	testEmitter->SetPos(GetPositionF());
	//testEmitter->SetTileset( owner->GetTileset("FX/fatgoku.png", 0, 0));
	testEmitter->SetRatePerSecond(30);
	
	//testEmitter->AddForce(Vector2f(0, .1));

	radius = 400;

	shardType = -1;
	UpdateParamsSettings();

	ts_sparkle = sess->GetTileset("Menu/shard_sparkle_64x64.png", 64, 64);

	ts_explodeCreate = sess->GetTileset("FX/shard_explode_01_256x256.png", 256, 256);

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	
	//sprite.setPosition( GetPositionF() );

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = NULL;

	actionLength[FLOAT] = 120;
	actionLength[DISSIPATE] = 10;
	actionLength[LAUNCH] = 60;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;
	animFactor[LAUNCH] = 1;

	geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, Vector2f( 0, 0 ), Vector2f( 0, 0 ),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	geoGroup.Init();

	/*geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, GetPositionF(), GetPositionF(),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	geoGroup.Init();*/

	ResetEnemy();

	SetSpawnRect();
	//geoGroup.AddGeo(new MovingRing(32, 20, 200, 10, 20, GetPositionF(), GetPositionF(),
	//	Color::Cyan, Color(0, 0, 100, 0), 60));
	//geoGroup.Init();
}

Shard::~Shard()
{
	delete testEmitter;
	delete sparklePool;
}

void Shard::ResetEnemy()
{
	SetCurrPosInfo(startPosInfo);

	geoGroup.Reset();
	totalFrame = 0;
	sparklePool->Reset();
	action = FLOAT;
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();

	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);

	testEmitter->Reset();
	testEmitter->SetOn(false);

	//position = startPosInfo.GetPosition();
	rootPos = GetPosition();
}

Tileset *Shard::GetShardTileset(int w, TilesetManager *ttm)
{
	Tileset *ts = NULL;
	switch (w)
	{
	case 0:
	default:
		ts = ttm->GetTileset("Shard/shards_w1_192x192.png", 192, 192);
	}

	return ts;
}

void Shard::FrameIncrement()
{
	++totalFrame;
}

int Shard::GetShardType(const std::string &str)
{
	return shardTypeMap[str];
}

int Shard::GetShardType(int w, int li)
{
	int totalIndex = w * 22 + li;
	if (totalIndex >= SHARD_Count)
	{
		totalIndex = 0;
	}
	return totalIndex;
	//return shardTypeMap[str];
}

std::string Shard::GetShardString(ShardType st)
{
	return shardStrMap[st];
}

std::string Shard::GetShardString(int w, int li)
{
	int shardIndex = 22 * w + li;

	if (shardIndex < ShardType::SHARD_Count)
	{
		return GetShardString((ShardType)shardIndex);
	}
	else
	{
		return string("-------");
	}
}

void Shard::IHitPlayer(int index)
{
	if( action == FLOAT )
		DissipateOnTouch();
}

void Shard::DissipateOnTouch()
{
	action = DISSIPATE;
	frame = 0;

	sess->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, GetPosition(), true, 0, 12, 3, true);

	SetHitboxes(NULL, 0);
	SetHurtboxes(NULL, 0);

	if (game != NULL)
	{
		GetShardSequence *gss = (GetShardSequence*)game->getShardSeq;
		gss->shard = this;
		game->getShardSeq->Reset();
		game->SetActiveSequence(game->getShardSeq);
	}
	
	Actor *player = sess->GetPlayer(0);
	player->SetAction(Actor::GETSHARD);
	player->frame = 0;
	player->velocity = V2d(0, 0);
	//Capture();
}

void Shard::Capture()
{
	//owner->absorbShardParticles->Activate(owner->GetPlayer(0), 1, position);

	if (game != NULL)
	{
		game->shardsCapturedField->SetBit(shardType, true);
		if (game->saveFile != NULL)
		{
			assert(!game->saveFile->shardField.GetBit(shardType));

			//both give you the shard and mark it as a new shard
			game->saveFile->shardField.SetBit(shardType, true);
			game->saveFile->newShardField.SetBit(shardType, true);
			game->saveFile->Save();
		}

	}

	
	//owner->state = GameSession::SEQUENCE;
	//owner->absorbDarkParticles->Activate(owner->GetPlayer(0), 1, position);
	
	//owner->mainMenu->GetCurrentProgress()->Save(); //might need to multithread at some point. this can be annoying
}

void Shard::DirectKill()
{

}



void Shard::Launch()
{
	action = LAUNCH;
	frame = 0;
}

void Shard::ProcessState()
{
	if( frame == actionLength[action] * animFactor[action])
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
	sparklePool->Update();
	if (!geoGroup.Update())
	{
		geoGroup.Reset();
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

void Shard::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOAT)
	{
		DissipateOnTouch();
	}
}

void Shard::UpdateSprite()
{
	int tile = 0;

	//sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setPosition(GetPositionF());
}

void Shard::EnemyDraw( sf::RenderTarget *target )
{
	geoGroup.Draw( target );

	if (action != DISSIPATE)
	{
		target->draw(sprite);
		sparklePool->Draw(target);
	}

	testEmitter->Draw(target);
}

void Shard::DrawMinimap( sf::RenderTarget *target )
{
}

ShardPopup::ShardPopup(GameSession *p_owner)
	:owner( p_owner )
{
	w = -1;
	li = -1;

	desc.setCharacterSize(20);
	desc.setFillColor(Color::White);
	desc.setFont(owner->mainMenu->arial);

	descRel = Vector2f(200, 20);
	effectRel = Vector2f(20, 20);
	shardRel = Vector2f(100, 20);

	Tileset *ts_bg = owner->GetTileset("Menu/GetShard/getshardbg.png", 0, 0 );
	bgSpr.setTexture(*ts_bg->texture);
}

void ShardPopup::Reset()
{
	state = SHOW;
	frame = 0;
}

void ShardPopup::Update()
{
	++frame;
}

void ShardPopup::Draw(RenderTarget *target)
{
	target->draw(bgSpr);
	target->draw(shardSpr);
	target->draw(effectSpr);
	target->draw(desc);
}

void ShardPopup::SetShard(int p_w, int p_li)
{
	w = p_w;
	li = p_li;

	Tileset *ts_shard = Shard::GetShardTileset(w, owner);
	shardSpr.setTexture(*ts_shard->texture);
	shardSpr.setTextureRect(ts_shard->GetSubRect(li));

	effectSpr.setTexture(*shardSpr.getTexture());
	effectSpr.setTextureRect(shardSpr.getTextureRect());

	string test = owner->pauseMenu->shardMenu->GetShardDesc(0, 0);
	desc.setString(test);
}

void ShardPopup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;
	shardSpr.setPosition(topLeft + shardRel);
	effectSpr.setPosition(topLeft + effectRel);
	desc.setPosition(topLeft + descRel);
	bgSpr.setPosition(topLeft);
}

void ShardPopup::SetCenter(sf::Vector2f &pos)
{
	float width = bgSpr.getLocalBounds().width;
	float height = bgSpr.getLocalBounds().height;

	SetTopLeft(Vector2f( pos.x - width / 2, pos.y - height / 2));
}