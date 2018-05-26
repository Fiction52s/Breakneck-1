#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Shard.h"
#include "SaveFile.h"

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
	shardTypeMap["SHARD_W1_TEACH_JUMP"] = SHARD_W1_TEACH_JUMP;
	for (auto it = shardTypeMap.begin(); it != shardTypeMap.end(); ++it)
	{
		shardStrMap[(*it).second] = (*it).first;
	}
}

Shard::Shard( GameSession *p_owner, Vector2i pos, ShardType p_sType )
	:Enemy( p_owner, EnemyType::EN_SHARD, false, p_owner->mh->envType + 1 ), shardType( p_sType )
{
	initHealth = 60;
	health = initHealth;

	//hopefully this doesnt cause deletion bugs
	radius = 400;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	initHealth = 20;
	health = initHealth;

	caught = false;

	spawnRect = sf::Rect<double>( pos.x - 32, pos.y - 32, 32 * 2, 32 * 2 );
	
	frame = 0;

	switch (shardType)
	{
	case ShardType::SHARD_W1_TEACH_JUMP:
		ts = owner->GetTileset("shards_w1_64x64.png", 64, 64);
		sprite.setTexture(*ts->texture);
		sprite.setTextureRect(ts->GetSubRect(shardType));
		break;
	default:
		assert(0);
		break;
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

	actionLength[FLOAT] = 2;
	actionLength[DISSIPATE] = 20;

	animFactor[FLOAT] = 1;
	animFactor[DISSIPATE] = 1;

	ResetEnemy();
}

ShardType Shard::GetShardType(const std::string &str)
{
	return shardTypeMap[str];
}

std::string Shard::GetShardString(ShardType st)
{
	return shardStrMap[st];
}

void Shard::IHitPlayer(int index)
{
	if( action == FLOAT )
		DissipateOnCapture();
}

void Shard::DissipateOnCapture()
{
	action = DISSIPATE;
	frame = 0;

	if (owner->saveFile != NULL)
	{
		assert(!owner->saveFile->shardField.GetBit(shardType));

		//both give you the shard and mark it as a new shard
		owner->saveFile->shardField.SetBit(shardType, true);
		owner->saveFile->newShardField.SetBit(shardType, true);
	}


	SetHitboxes(NULL, 0);
	SetHurtboxes(NULL, 0);

	owner->absorbDarkParticles->Activate(owner->GetPlayer(0), 1, position, AbsorbParticles::SHARD);

	owner->mainMenu->GetCurrentProgress()->Save(); //might need to multithread at some point. this can be annoying
}


void Shard::ResetEnemy()
{
	
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
			break;
		}
	}
}

void Shard::ProcessHit()
{
	if (!dead && ReceivedHit() && action == FLOAT)
	{
		DissipateOnCapture();
	}
}

void Shard::UpdateSprite()
{
	int tile = 0;

	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setPosition(position.x, position.y);
}

void Shard::EnemyDraw( sf::RenderTarget *target )
{
	target->draw(sprite);
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