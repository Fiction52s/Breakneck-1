#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Cactus.h"

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

Cactus::Cactus( GameSession *owner, bool p_hasMonitor, Edge *g, double q )
		:Enemy( owner, EnemyType::EN_CACTUS, p_hasMonitor, 2 ), ground( g ),
		edgeQuantity( q )
{

	bulletSpeed = 6;
	
	firingCounter = 120;
	initHealth = 60;
	health = initHealth;

	double width = 64; //112;
	double height = 64;

	//ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	ts = owner->GetTileset( "curveturret_64x64.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	
	int maxShotguns = 10;

	ts_shotgun = owner->GetTileset("Enemies/shroom_jelly_160x160.png", 160, 160);

	shotgunVA = new Vertex[maxShotguns * 4];

	shotgunPool = new ObjectPool;
	
	for (int i = 0; i < maxShotguns; ++i)
	{
		CactusShotgun *cs = new CactusShotgun(owner, this, shotgunPool, i);
		shotgunPool->AddToInactiveList(cs);
	}
	

	ts_bulletExplode = owner->GetTileset("bullet_explode3_64x64.png", 64, 64);

	V2d gn = g->Normal();

	V2d gAlong = normalize( g->v1 - g->v0 );

	position = gPoint + gn * height / 2.0;

	double angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	actionLength[IDLE] = 1;
	actionLength[SHOOT] = 20;
	actionLength[ACTIVE] = 1;

	animFactor[IDLE] = 1;
	animFactor[SHOOT] = 1;
	animFactor[ACTIVE] = 1;

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

	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	hitBody->hitboxInfo = hitboxInfo;

	double size = max( width, height );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );

	ResetEnemy();
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
	dead = false;
	frame = 0;
	health = initHealth;
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	UpdateHitboxes();
	UpdateSprite();
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

	Actor *player = owner->GetPlayer(0);
	double dist = length(player->position - position);

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

void Cactus::EnemyDraw(sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
	target->draw(shotgunVA, 10 * 4, sf::Quads, ts_shotgun->texture);
}

void Cactus::DirectKill()
{
	health = 0;
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

	Actor *player = owner->GetPlayer(0);
	V2d playerDir = normalize(player->position - position);

	shot->SetParams(position, playerDir);
	owner->AddEnemy(shot);
}

void Cactus::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( 0 ) );//frame / animationFactor ) );
}

void Cactus::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;// + gn * 8.0;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;// + gn * 8.0;
	hitBox.globalAngle = 0;
}

CactusShotgun::CactusShotgun(GameSession *owner, Cactus *p, ObjectPool *pool, int poolIndex )
	:Enemy(owner, EnemyType::EN_CACTUSSHOTGUN, false, 2), parent( p ), PoolMember( poolIndex )
{
	va = parent->shotgunVA + poolIndex * 4;
	myPool = pool;
	bulletSpeed = 6;

	double width = 64; //112;
	double height = 64;

	
	//ts_bulletExplode = owner->GetTileset("bullet_explode3_64x64.png", 64, 64);

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 16;
	hurtBox.rh = 16;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);

	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 16;
	hitBox.rh = 16;
	hitBody->AddCollisionBox(0, hitBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	hitBody->hitboxInfo = hitboxInfo;

	double size = max(width, height);

	numLaunchers = 4;
	launchers = new Launcher*[numLaunchers];

	Vector2f dir = normalize(Vector2f(1, 1));
	sf::Transform t;
	for (int i = 0; i < 4; ++i)
	{
		launchers[i] = new Launcher(this, BasicBullet::CACTUS_SHOTGUN, owner, 3, 3, position, V2d(t.transformPoint(dir)),
			PI / 6.0, 90, false);
		launchers[i]->SetBulletSpeed(bulletSpeed);
		launchers[i]->hitboxInfo->damage = 18;
		t.rotate(90);
	}
	

	spawnRect = sf::Rect<double>(position.x - size / 2, position.y - size / 2, size, size);

	actionLength[CHASINGPLAYER] = 120;
	actionLength[BLINKING] = 40;
	actionLength[SHOOTING] = 20;
	actionLength[PUSHBACK] = 40;
	actionLength[STASIS] = 180;

	animFactor[CHASINGPLAYER] = 1;
	animFactor[BLINKING] = 1;
	animFactor[SHOOTING] = 1;
	animFactor[STASIS] = 1;
	animFactor[PUSHBACK] = 1;
	

	ResetEnemy();
}

void CactusShotgun::SetParams(V2d &p_position, V2d &dir)
{
	position = p_position;
	velocity = dir * bulletSpeed;
	ResetEnemy();
}

void CactusShotgun::BulletHitTerrain(BasicBullet *b,
	Edge *edge,
	sf::Vector2<double> &pos)
{
	//never hits terrain
}

void CactusShotgun::BulletHitPlayer(BasicBullet *b)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	owner->ActivateEffect(EffectLayer::IN_FRONT, parent->ts_bulletExplode, b->position, true, angle, 6, 2, true);
	owner->GetPlayer(0)->ApplyHit(b->launcher->hitboxInfo);
	b->launcher->DeactivateBullet(b);
	//might have all the bullets explode/dissipate if one of them hits you.
}

void CactusShotgun::ResetEnemy()
{
	action = CHASINGPLAYER;
	dead = false;
	frame = 0;
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	//UpdateSprite();
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

			Vector2f dir(normalize(owner->GetPlayer(0)->position - position));
			Transform t;
			for (int i = 0; i < 4; ++i)
			{
				launchers[i]->facingDir = V2d( t.transformPoint( dir ) );
				launchers[i]->position = position;
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
			frame = 0;
			break;
		case EXPLODING:
			dead = true;
			numHealth = 0;
			myPool->DeactivatePoolMember(this);
			ClearSprite();
			break;
		}
	}
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
	Actor *player = owner->GetPlayer(0);
	
	ActionEnded();

	switch (action)
	{
	case CHASINGPLAYER:
	{
		double dist = length(player->position - position);
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
		dir += normalize(player->position - position) / 60.0;
		dir = normalize(dir);
		velocity = dir * bulletSpeed;
		break;
	}
	case BLINKING:
	{
		V2d dir = normalize(velocity);
		dir += normalize(player->position - position) / 10.0;
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
	position += velocity / ((double)slowMultiple) / numPhysSteps;
	//cout << "position: " << position.x << ", " << position.y << endl;
	//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
}

void CactusShotgun::EnemyDraw(sf::RenderTarget *target)
{
	//DrawSpriteIfExists(target, sprite);
}

void CactusShotgun::DirectKill()
{
	health = 0;
	receivedHit = NULL;
	ClearSprite();
	myPool->DeactivatePoolMember(this);
}

void CactusShotgun::UpdateSprite()
{
	SetRectCenter(va, parent->ts_shotgun->tileWidth, parent->ts_shotgun->tileHeight, Vector2f(position));
	SetRectSubRect(va, parent->ts_shotgun->GetSubRect(1));
	//sprite.setTextureRect(ts->GetSubRect(0));//frame / animationFactor ) );
}

void CactusShotgun::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;// + gn * 8.0;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;// + gn * 8.0;
	hitBox.globalAngle = 0;
}