#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SpecterBullet.h"
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

SpecterBulletPool::SpecterBulletPool()
{
	//ts = NULL;
	numBullets = 20;


	Session *sess = Session::GetSession();
	ts = sess->GetSizedTileset("Bosses/Bird/bird_shuriken_256x256.png");
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i] = new SpecterBullet( this, verts + 4 * i);
	}

	

	//ts = sess->GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
	
}

SpecterBulletPool::~SpecterBulletPool()
{
	for (int i = 0; i < numBullets; ++i)
	{
		delete bulletVec[i];
	}

	delete[] verts;
}

void SpecterBulletPool::Reset()
{
	SpecterBullet *sb = NULL;
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

void SpecterBulletPool::SetEnemyIDsAndAddToGame()
{
	SpecterBullet *sb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		sb = bulletVec[i];
		sb->SetEnemyIDAndAddToAllEnemiesVec();
	}
}

SpecterBullet * SpecterBulletPool::Throw(V2d &pos, V2d &dir )
{
	SpecterBullet *sb = NULL;
	SpecterBullet *thrownBullet = NULL;
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

void SpecterBulletPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

SpecterBullet::SpecterBullet(SpecterBulletPool *p_pool, sf::Vertex *myQuad)
	:Enemy(EnemyType::EN_SPECTERBULLET, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(THROWN, 0, 0);

	actionLength[THROWN] = 1;
	animFactor[THROWN] = 1;

	pool = p_pool;

	quad = myQuad;

	ts = pool->ts;
	//ts = GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	throwSpeed = 10;
	acceleration = .1;
	maxSpeed = 25;

	ResetEnemy();
}

void SpecterBullet::ResetEnemy()
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

void SpecterBullet::SetLevel(int lev)
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

void SpecterBullet::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		Die();

		receivedHit.SetEmpty();
	}
}

bool SpecterBullet::IsSlowed(int index)
{
	//needs to use the hitboxes because of the head,
	//the anchor should not slow the head down.
	Actor *player = sess->GetPlayer(index);
	return (player->IntersectMySlowboxes(currHitboxes, currHitboxFrame));
}

void SpecterBullet::Throw(V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	hitboxInfo->canBeBlocked = true;

	data.framesToLive = 120;
	data.velocity = dir * throwSpeed;

	SetHurtboxes(NULL);

	action = THROWN;
	frame = 0;

}

void SpecterBullet::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

void SpecterBullet::FrameIncrement()
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

void SpecterBullet::ProcessState()
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

	V2d vDir = PlayerDir();//normalize(data.velocity);
	double speed = length(data.velocity);
	speed += acceleration;
	if (speed > maxSpeed)
	{
		speed = maxSpeed;
	}

	data.velocity = speed * vDir;
}

void SpecterBullet::IHitPlayer(int index)
{
	if (action == THROWN || action == THROWN_VULNERABLE )
	{
		Die();
	}
}

void SpecterBullet::UpdateEnemyPhysics()
{
	if (numHealth > 0)
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}

	if (slowMultiple > 1 && currHurtboxes == NULL )
	{
		DefaultHurtboxesOn();
		action = THROWN_VULNERABLE;
		frame = 0;
	}
	//Enemy::UpdateEnemyPhysics();

	//hitboxInfo->hitPosType = HitboxInfo::GetAirType(surfaceMover->GetVel());
}

void SpecterBullet::UpdateSprite()
{
	if (action == THROWN)
	{
		SetRectColor(quad, Color::White);
	}
	else if (action == THROWN_VULNERABLE)
	{
		SetRectColor(quad, Color::Blue);
	}

	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, ts->tileWidth * .25, ts->tileHeight * .25, GetPositionF());
}

void SpecterBullet::EnemyDraw(sf::RenderTarget *target)
{
	//DrawSprite(target, sprite);
}

bool SpecterBullet::IsHitFacingRight()
{
	return data.velocity.x > 0;//surfaceMover->GetVel().x > 0;
}


int SpecterBullet::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void SpecterBullet::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void SpecterBullet::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}