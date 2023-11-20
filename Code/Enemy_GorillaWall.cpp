#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GorillaWall.h"
#include "Actor.h"

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

GorillaWallPool::GorillaWallPool()
{
	//ts = NULL;
	numBullets = 20;

	Session *sess = Session::GetSession();
	ts = sess->GetSizedTileset("Enemies/W6/gorillawall_400x50.png");//sess->GetSizedTileset("Enemies/Bosses/Bird/bird_shuriken_256x256.png");
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i] = new GorillaWall(this, verts + 4 * i);
	}
	//ts = sess->GetSizedTileset("Enemies/Bosses/Bird/shuriken01_128x128.png");

}

GorillaWallPool::~GorillaWallPool()
{
	for (int i = 0; i < numBullets; ++i)
	{
		delete bulletVec[i];
	}

	delete[] verts;
}

void GorillaWallPool::Reset()
{
	GorillaWall *sb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		sb = bulletVec[i];
		if (sb->spawned)
		{
			sb->Die();
		}

		sb->Reset();
	}

	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i]->Reset();
	}
}

void GorillaWallPool::SetEnemyIDsAndAddToGame()
{
	GorillaWall *sb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		sb = bulletVec[i];
		sb->SetEnemyIDAndAddToAllEnemiesVec();
	}
}

GorillaWall * GorillaWallPool::Throw(V2d &pos, V2d &dir)
{
	GorillaWall *sb = NULL;
	GorillaWall *thrownBullet = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		sb = bulletVec[i];
		if (!sb->spawned)
		{
			sb->Throw(pos, dir);
			thrownBullet = sb;
			break;
		}
	}
	return thrownBullet;
}

void GorillaWallPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

void GorillaWallPool::DebugDraw(sf::RenderTarget *target)
{
	GorillaWall *sb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		sb = bulletVec[i];
		sb->DebugDraw(target);
	}
}

//int GorillaWallPool::GetNumStoredBytes()
//{
//	int total = 0;
//	GorillaWall *sb = NULL;
//	for (int i = 0; i < numBullets; ++i)
//	{
//		sb = bulletVec[i];
//		total += sb->GetNumStoredBytes();
//	}
//
//	return total;
//}
//
//void GorillaWallPool::StoreBytes(unsigned char *bytes)
//{
//	GorillaWall *sb = NULL;
//	for (int i = 0; i < numBullets; ++i)
//	{
//		sb = bulletVec[i];
//		sb->StoreBytes(bytes);
//		bytes += sb->GetNumStoredBytes();
//	}
//}
//
//void GorillaWallPool::SetFromBytes(unsigned char *bytes)
//{
//	memcpy(&data, bytes, sizeof(MyData));
//	SetBasicEnemyData(data);
//	bytes += sizeof(MyData);
//}

GorillaWall::GorillaWall(GorillaWallPool *p_pool, sf::Vertex *myQuad)
	:Enemy(EnemyType::EN_GORILLAWALL, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(THROWN, 0, 0);

	actionLength[THROWN] = 1;
	animFactor[THROWN] = 1;

	pool = p_pool;

	quad = myQuad;

	wallLength = 300;
	wallThickness = 50;


	ts = pool->ts;
	//ts = GetSizedTileset("Enemies/Bosses/Bird/shuriken01_128x128.png");
	//sprite.setTexture(*ts->texture);
	//sprite.setScale(scale, scale);

	ts->SetQuadSubRect(myQuad, 0);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 5;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;
	hitboxInfo->knockbackOnBlock = false;

	
	hitBody.SetupNumFrames(1);
	hitBody.SetupNumBoxesOnFrame(0, 1);
	hitBody.AddBasicRect(0, wallLength / 2, wallThickness / 2, 0, V2d());

	hitBody.hitboxInfo = hitboxInfo;

	hurtBody.SetupNumFrames(1);
	hurtBody.SetupNumBoxesOnFrame(0, 1);
	hurtBody.AddBasicRect(0, wallLength / 2, wallThickness / 2, 0, V2d());


	throwSpeed = 10;
	
	//acceleration = .1;
	//maxSpeed = 25;

	ResetEnemy();
}

void GorillaWall::ResetEnemy()
{
	ClearRect(quad);

	dead = false;
	facingRight = true;

	action = THROWN;
	frame = 0;

	data.velocity = V2d();
	data.framesToLive = -1;

	HurtboxesOff();
	//DefaultHurtboxesOn()
	DefaultHitboxesOn();

	UpdateHitboxes();

	//UpdateSprite();
}

void GorillaWall::UpdateHitboxes()
{
	V2d position = GetPosition();

	
	CollisionBox &wallHitbox = hitBody.GetCollisionBoxes(0).front();

	wallHitbox.globalPosition = GetPosition();

	CollisionBox &wallHurtbox = hurtBody.GetCollisionBoxes(0).front();

	wallHurtbox.globalPosition = GetPosition();
	//wallHitbox.globalAngle = atan2(dir.x, -dir.y);
}

void GorillaWall::SetLevel(int lev)
{
	level = lev;
	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void GorillaWall::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		Die();

		receivedHit.SetEmpty();
	}
}

bool GorillaWall::IsSlowed(int index)
{
	//needs to use the hitboxes because of the head,
	//the anchor should not slow the head down.

	//SetHitboxes(&hitBody, hFrame );
	Actor *player = sess->GetPlayer(index);
	return (player->IntersectMySlowboxes(&hitBody, 0));
}

void GorillaWall::Throw(V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	hitboxInfo->canBeBlocked = true;

	data.framesToLive = 120;
	data.velocity = dir * throwSpeed;

	HurtboxesOff();
	DefaultHitboxesOn();

	action = THROWN;
	frame = 0;

	CollisionBox &wallHitbox = hitBody.GetCollisionBoxes(0).front();

	wallHitbox.globalPosition = pos;
	wallHitbox.globalAngle = atan2(dir.x, -dir.y);

	CollisionBox &wallHurtbox = hurtBody.GetCollisionBoxes(0).front();

	wallHurtbox.globalPosition = pos;
	wallHurtbox.globalAngle = atan2(dir.x, -dir.y);

	UpdateSprite();

	/*wallSprite.setPosition(wallHitbox.globalPosition.x,
		wallHitbox.globalPosition.y);
	wallSprite.setRotation(wallHitbox.globalAngle / PI * 180.0);*/

	//currWallHitboxes = &data.wallHitBody;

	//data.wallVel = playerDir * 10.0;//2.0;

}

void GorillaWall::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

void GorillaWall::FrameIncrement()
{
	if (data.framesToLive > 0)
	{
		--data.framesToLive;

		if (data.framesToLive == 0)
		{
			Die();
		}
	}
}

void GorillaWall::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case THROWN:
			frame = 0;
			break;
		case THROWN_VULNERABLE:
			frame = 0;
			break;
		}

	}

	//V2d vDir = PlayerDir();//normalize(data.velocity);
	//double speed = length(data.velocity);
	//speed += acceleration;
	//if (speed > maxSpeed)
	//{
	//	speed = maxSpeed;
	//}

	//data.velocity = speed * vDir;
}

void GorillaWall::IHitPlayer(int index)
{
	if (action == THROWN || action == THROWN_VULNERABLE)
	{
		Die();
	}
}

void GorillaWall::UpdateEnemyPhysics()
{
	if (numHealth > 0)
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}

	if (slowMultiple > 1 && currHitboxes != NULL)
	{
		DefaultHurtboxesOn();
		HitboxesOff();
		action = THROWN_VULNERABLE;
		frame = 0;
	}
	else if (slowMultiple == 1 && currHitboxes == NULL)
	{
		DefaultHitboxesOn();
		HurtboxesOff();
		action = THROWN;
		frame = 0;
	}
	//Enemy::UpdateEnemyPhysics();

	//hitboxInfo->hitPosType = HitboxInfo::GetAirType(surfaceMover->GetVel());
}

void GorillaWall::UpdateSprite()
{
	if (action == THROWN)
	{
		SetRectColor(quad, Color::White);
	}
	else if (action == THROWN_VULNERABLE)
	{
		SetRectColor(quad, Color::Blue);
	}

	CollisionBox &wallHitbox = hitBody.GetCollisionBoxes(0).front();
	SetRectRotation( quad, wallHitbox.globalAngle, wallLength, wallThickness, Vector2f(wallHitbox.globalPosition));
}

void GorillaWall::EnemyDraw(sf::RenderTarget *target)
{
	//DrawSprite(target, sprite);
}

bool GorillaWall::IsHitFacingRight()
{
	return data.velocity.x > 0;//surfaceMover->GetVel().x > 0;
}


int GorillaWall::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GorillaWall::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void GorillaWall::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}