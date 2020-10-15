#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Cactus.h"
#include "Shield.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Cactus::Cactus( ActorParams *ap )
		:Enemy( EnemyType::EN_CACTUS, ap )
{
	SetNumActions(A_Count);
	SetEditorActions(ACTIVE, ACTIVE, 0);

	SetLevel(ap->GetLevel());

	bulletSpeed = 6;
	
	firingCounter = 120;

	double width = 144; //112;
	double height = 96;

	ts = sess->GetSizedTileset("Enemies/curveturret_144x96.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	ts_shotgun = sess->GetTileset("Enemies/shroom_jelly_160x160.png", 160, 160);

	shotgunVA = new Vertex[MAX_SHOTGUNS * 4];

	shotgunPool = new ObjectPool;
	
	for (int i = 0; i < MAX_SHOTGUNS; ++i)
	{
		CactusShotgun *cs = new CactusShotgun( this, shotgunPool, i);
		shotgunArray[i] = cs;
		shotgunPool->AddToInactiveList(cs);
	}

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);
	

	actionLength[IDLE] = 1;
	actionLength[SHOOT] = 20;
	actionLength[ACTIVE] = 1;

	animFactor[IDLE] = 1;
	animFactor[SHOOT] = 1;
	animFactor[ACTIVE] = 1;
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	
	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

Cactus::~Cactus()
{
	shotgunPool->DestroyAllMembers();
	delete shotgunPool;
	delete[] shotgunVA;
}

void Cactus::ResetEnemy()
{
	CactusShotgun *cs = (CactusShotgun*)shotgunPool->activeListStart;
	while (cs != NULL)
	{
		cs->Reset();
		cs = (CactusShotgun*)cs->pmnext;
	}
	
	shotgunPool->Reset();
	action = IDLE;
	frame = 0;
	
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();

	currShield = shield;
	shield->Reset();
	shield->SetPosition(GetPosition());
}

void Cactus::SetLevel(int lev)
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

void Cactus::ActionEnded()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case ACTIVE:
			break;
		case SHOOT:
			action = IDLE;
			break;
		}
	}
}

void Cactus::ProcessState()
{
	ActionEnded();

	//Actor *player = owner->GetPlayer(0);
	V2d playerPos = sess->GetPlayerPos(0);
	double dist = length(playerPos - GetPosition());

	switch (action)
	{
	case IDLE:
		if (dist < 700)
		{
			action = ACTIVE;
			frame = 0;
			framesWait = 0;
		}
		break;
	case SHOOT:
		break;
	case ACTIVE:
		if (framesWait == firingCounter)
		{
			action = SHOOT;
			frame = 0;
		}
		break;
	}

	switch (action)
	{
	case IDLE:
		break;
	case ACTIVE:
		break;
	case SHOOT:
		if (frame == 0 && slowCounter == 1)
		{
			ThrowShotgun();
		}
		break;
	}
}

void Cactus::FrameIncrement()
{
	if (action == ACTIVE)
	{
		framesWait++;
	}
}

void Cactus::UpdateEnemyPhysics()
{
	
}

//this might not be what i want to do. need to go over stuff later.
void Cactus::Draw(sf::RenderTarget *target)
{
	if (cutObject != NULL)
	{
		if (dead && cutObject->active)
		{
			cutObject->Draw(target);
		}
		else if (!dead)
		{
			DrawSprite(target, sprite);
			if (currShield != NULL)
			{
				currShield->Draw(target);
			}
		}
	}
	else
	{
		EnemyDraw(target);
		if (currShield != NULL)
		{
			currShield->Draw(target);
		}
	}

	target->draw(shotgunVA, MAX_SHOTGUNS * 4, sf::Quads, ts_shotgun->texture);
}

void Cactus::DirectKill()
{
	receivedHit = NULL;
}

void Cactus::ThrowShotgun()
{
	action = SHOOT;
	frame = 0;
	//sprite.setTexture(*tilesets[MOVE]->texture);
	//sprite.setTextureRect(tilesets[MOVE]->GetSubRect(1));

	CactusShotgun *shot = (CactusShotgun*)shotgunPool->ActivatePoolMember();
	assert(shot != NULL);

	shot->spawned = false;
	//shot->Reset();

	//Actor *player = owner->GetPlayer(0);

	V2d playerDir = normalize(sess->GetPlayerPos(0) - GetPosition());

	shot->SetParams(GetPosition(), playerDir);
	sess->AddEnemy(shot);
}

int Cactus::SetLaunchersStartIndex(int ind)
{
	int currIndex = ind;

	for (int i = 0; i < MAX_SHOTGUNS; ++i)
	{
		currIndex = shotgunArray[i]->SetLaunchersStartIndex(currIndex);
	}

	return currIndex;
}

void Cactus::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

bool Cactus::LaunchersAreDone()
{
	if (shotgunPool->numActiveMembers == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

CactusShotgun::CactusShotgun(Cactus *p, ObjectPool *pool, int poolIndex )
	:Enemy(EnemyType::EN_CACTUSSHOTGUN, NULL), parent( p ), PoolMember( poolIndex )
{
	SetNumActions(A_Count);

	va = parent->shotgunVA + poolIndex * 4;
	myPool = pool;
	bulletSpeed = 6;

	double width = 64; //112;
	double height = 64;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	double size = max(width, height);

	SetNumLaunchers(4);
	Vector2f dir = normalize(Vector2f(1, 1));
	sf::Transform t;
	for (int i = 0; i < 4; ++i)
	{
		launchers[i] = new Launcher(this, BasicBullet::CACTUS_SHOTGUN,
			3, 3, GetPosition(), V2d(t.transformPoint(dir)),
			PI / 6.0, 90, false);
		launchers[i]->SetBulletSpeed(bulletSpeed);
		launchers[i]->hitboxInfo->damage = 18;
		t.rotate(90);
	}

	actionLength[CHASINGPLAYER] = 120;
	actionLength[BLINKING] = 40;
	actionLength[SHOOTING] = 20;
	actionLength[PUSHBACK] = 40;
	actionLength[STASIS] = 180;
	actionLength[EXPLODING] = 10;

	animFactor[CHASINGPLAYER] = 1;
	animFactor[BLINKING] = 1;
	animFactor[SHOOTING] = 1;
	animFactor[STASIS] = 1;
	animFactor[PUSHBACK] = 1;
	animFactor[EXPLODING] = 1;
	

	ResetEnemy();
}

void CactusShotgun::SetParams(V2d &p_position, V2d &dir)
{
	ResetEnemy();
	currPosInfo.position = p_position;
	velocity = dir * bulletSpeed;
}

void CactusShotgun::BulletHitTerrain(BasicBullet *b,
	Edge *edge,
	sf::Vector2<double> &pos)
{
	//never hits terrain
}

void CactusShotgun::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, parent->ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
	//might have all the bullets explode/dissipate if one of them hits you.
}

void CactusShotgun::ResetEnemy()
{
	action = CHASINGPLAYER;
	dead = false;
	frame = 0;
	
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	ClearSprite();
	UpdateHitboxes();	
}

void CactusShotgun::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case CHASINGPLAYER:
			action = BLINKING;
			break;
		case BLINKING:
		{
			action = SHOOTING;

			Vector2f dir(normalize(sess->GetPlayerPos(0) - GetPosition()));
			Transform t;
			for (int i = 0; i < 4; ++i)
			{
				launchers[i]->facingDir = V2d( t.transformPoint( dir ) );
				launchers[i]->position = GetPosition();
				launchers[i]->Fire();
				t.rotate(90);
			}
			/*
			launchers[0]->position = position;
			launchers[0]->Fire();*/
			break;
		}
		case SHOOTING:
			action = STASIS;
			break;
		case PUSHBACK:
			break;
		case STASIS:
			action = EXPLODING;
			//frame = 0;
			break;
		case EXPLODING:
			dead = true;
			numHealth = 0;
			//myPool->DeactivatePoolMember(this);
			//ClearSprite();
			break;
		}
	}
}

void CactusShotgun::HandleRemove()
{
	myPool->DeactivatePoolMember(this);
	ClearSprite();
}

void CactusShotgun::HandleNoHealth()
{
	ClearSprite();
}

void CactusShotgun::ClearSprite()
{
	parent->shotgunVA[(index) * 4 + 0].position = Vector2f(0, 0);
	parent->shotgunVA[(index) * 4 + 1].position = Vector2f(0, 0);
	parent->shotgunVA[(index) * 4 + 2].position = Vector2f(0, 0);
	parent->shotgunVA[(index) * 4 + 3].position = Vector2f(0, 0);
}

void CactusShotgun::ProcessState()
{
	//Actor *player = owner->GetPlayer(0);
	
	ActionEnded();

	V2d playerPos = sess->GetPlayerPos(0);

	switch (action)
	{
	case CHASINGPLAYER:
	{
		double dist = length(playerPos - GetPosition());
		if (dist < 250)
		{
			action = BLINKING;
			frame = 0;
		}
		break;
	}
	case BLINKING:
		break;
	case SHOOTING:
		break;
	case PUSHBACK:
		break;
	}

	switch (action)
	{
	case CHASINGPLAYER:
	{
		V2d dir = normalize(velocity);
		dir += normalize(playerPos - GetPosition()) / 60.0;
		dir = normalize(dir);
		velocity = dir * bulletSpeed;
		break;
	}
	case BLINKING:
	{
		V2d dir = normalize(velocity);
		dir += normalize(playerPos - GetPosition()) / 10.0;
		dir = normalize(dir);
		velocity = dir * bulletSpeed;
		break;
	}
	case SHOOTING:
		velocity = V2d(0, 0);
		break;
	case PUSHBACK:
		break;
	}
}

void CactusShotgun::UpdateEnemyPhysics()
{
	currPosInfo.position += velocity / ((double)slowMultiple) / numPhysSteps;
	//cout << "position: " << position.x << ", " << position.y << endl;
	//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
}

void CactusShotgun::DirectKill()
{
	receivedHit = NULL;
	ClearSprite();
	myPool->DeactivatePoolMember(this);
}

void CactusShotgun::UpdateSprite()
{
	SetRectCenter(va, parent->ts_shotgun->tileWidth, parent->ts_shotgun->tileHeight, 
		GetPositionF());
	SetRectSubRect(va, parent->ts_shotgun->GetSubRect(1));
	//sprite.setTextureRect(ts->GetSubRect(0));//frame / animationFactor ) );
}