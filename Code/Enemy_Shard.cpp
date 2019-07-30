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

Shard::Shard( GameSession *p_owner, Vector2i pos, int w, int li )
	:Enemy( p_owner, EnemyType::EN_SHARD, false, w+1 )
{
	for (int i = 0; i < 5; ++i)
	{
		geoGroup.AddGeo(new SpinningTri(0 + i * PI / 2.5, Vector2f(pos)));
	}
	geoGroup.AddGeo(new Laser(0, Vector2f(pos)));

	geoGroup.AddGeo(new MovingRing(32, 20, 10, 200, 20, Vector2f(pos), Vector2f(pos),
		Color::Cyan, Color( 0, 0, 100, 10 ), 30));
	geoGroup.Init();
	world = w;
	localIndex = li;

	shardType = Shard::GetShardType(w, localIndex);

	initHealth = 60;
	health = initHealth;

	//hopefully this doesnt cause deletion bugs
	radius = 400;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;
	startPos = position;

	initHealth = 20;
	health = initHealth;

	caught = false;

	spawnRect = sf::Rect<double>( pos.x - 32, pos.y - 32, 32 * 2, 32 * 2 );
	
	frame = 0;

	ts_sparkle = owner->GetTileset("Menu/shard_sparkle_64x64.png", 64, 64);

	ts_explodeCreate = owner->GetTileset("FX/shard_explode_01_256x256.png", 256, 256);

	sparklePool = new EffectPool(EffectType::FX_REGULAR, 3, 1.f);
	sparklePool->ts = ts_sparkle;

	switch (w)
	{
	case 0://ShardType::SHARD_W1_TEACH_JUMP:
	default:
		ts = owner->GetTileset("Shard/shards_w1_192x192.png", 192, 192);
		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(localIndex));
		break;
	/*default:
		assert(0);
		break;*/
	}

	//cout << "world: " << world << endl;
	/*switch( p_owner->mh->envType )
	{
	case 0:
		ts = owner->GetTileset( "shards_w1_64x64.png", 64, 64 );
		break;
	case 1:
		ts = owner->GetTileset( "shards_w2_64x64.png", 64, 64 );
		break;
	case 2:
		ts = owner->GetTileset( "shards_w3_64x64.png", 64, 64 );
		break;
	case 3:
		ts = owner->GetTileset( "shards_w4_64x64.png", 64, 64 );
		break;
	case 4:
		ts = owner->GetTileset( "shards_w5_64x64.png", 64, 64 );
		break;
	case 5:
		ts = owner->GetTileset( "shards_w6_64x64.png", 64, 64 );
		break;
	case 6:
		ts = owner->GetTileset( "shards_w7_64x64.png", 64, 64 );
		break;
	}*/

	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );


	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);


	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	hitBody->hitboxInfo = NULL;

	actionLength[FLOAT] = 120;
	actionLength[DISSIPATE] = 10;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;

	ResetEnemy();
}

Shard::~Shard()
{
	delete sparklePool;
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

ShardType Shard::GetShardType(const std::string &str)
{
	return shardTypeMap[str];
}

ShardType Shard::GetShardType(int w, int li)
{
	int totalIndex = w * 22 + li;
	if (totalIndex >= SHARD_Count)
	{
		totalIndex = 0;
	}

	ShardType st = (ShardType)totalIndex;
	return st;
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

	owner->ActivateEffect(EffectLayer::IN_FRONT,
		ts_explodeCreate, position, true, 0, 12, 3, true);

	SetHitboxes(NULL, 0);
	SetHurtboxes(NULL, 0);
	//Capture();
}

void Shard::Capture()
{
	owner->absorbShardParticles->Activate(owner->GetPlayer(0), 1, position);
	owner->shardsCapturedField->SetBit(shardType, true);
	if (owner->saveFile != NULL)
	{
		assert(!owner->saveFile->shardField.GetBit(shardType));

		//both give you the shard and mark it as a new shard
		owner->saveFile->shardField.SetBit(shardType, true);
		owner->saveFile->newShardField.SetBit(shardType, true);
		owner->saveFile->Save();
	}
	//owner->absorbDarkParticles->Activate(owner->GetPlayer(0), 1, position);
	
	//owner->mainMenu->GetCurrentProgress()->Save(); //might need to multithread at some point. this can be annoying
}

void Shard::DirectKill()
{

}

void Shard::ResetEnemy()
{
	geoGroup.Reset();

	totalFrame = 0;
	sparklePool->Reset();
	action = FLOAT;
	frame = 0;
	receivedHit = NULL;
	
	dead = false;
	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
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
		}
	}
	

	//case DISSI

	if (action == FLOAT)
	{
		int floatFrames = 240;
		double floatAmount = 4.0;
		int t = totalFrame % floatFrames;
		float tf = t;
		tf /= (floatFrames - 1);
		double f = cos(2 * PI * tf);
		f -= .5;
		position = startPos;
		position.y += f * floatAmount;
	}
	geoGroup.Update();
	
	sparklePool->Update();

	Vector2f sparkleCenter(position);

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
	sprite.setPosition(position.x, position.y);
}

void Shard::EnemyDraw( sf::RenderTarget *target )
{
	geoGroup.Draw(target);
	

	if (action != DISSIPATE)
	{
		target->draw(sprite);
		sparklePool->Draw(target);
	}

	
}

void Shard::DrawMinimap( sf::RenderTarget *target )
{
}

void Shard::UpdateHitboxes()
{
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
}