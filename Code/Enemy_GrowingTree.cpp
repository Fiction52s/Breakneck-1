#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GrowingTree.h"
#include "Actor.h"

using namespace std;
using namespace sf;

GrowingTree::GrowingTree( ActorParams *ap )
	:Enemy( EnemyType::EN_GROWINGTREE, ap )
	
{
	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL0, 0, 0);

	SetLevel(ap->GetLevel());

	actionLength[NEUTRAL0] = 2;
	actionLength[NEUTRAL1] = 2;
	actionLength[NEUTRAL2] = 2;

	actionLength[ATTACK0] = 60;
	actionLength[LEVEL0TO1] = 30;
	actionLength[ATTACK1] = 60;
	actionLength[LEVEL1TO2] = 30;
	actionLength[ATTACK2] = 60;

	animFactor[NEUTRAL0] = 1;
	animFactor[NEUTRAL1] = 1;
	animFactor[NEUTRAL2] = 1;
	animFactor[ATTACK0] = 1;
	animFactor[LEVEL0TO1] = 1;
	animFactor[ATTACK1] = 1;
	animFactor[LEVEL1TO2] = 1;
	animFactor[ATTACK2] = 1;

	//32, 0, 1000

	

	attentionRadius = 1000;
	ignoreRadius = 1200;

	totalBullets = 32;
	startPowerLevel = 0;
	pulseRadius = 1000;
	repsToLevelUp = 3;

	ts = GetSizedTileset("Enemies/W5/sprout_160x160.png");
	sprite.setTexture( *ts->texture );

	double height = 160 * scale;


	SetOffGroundHeight(height / 2.f);

	double bulletSpeed0 = 5;
	double bulletSpeed1 = 15;
	double bulletSpeed2 = 25;
	double bulletSpeed = 10;

	int framesToLive = 60 * 3;
	//int framesToLive0 = ( pulseRadius * 2 ) / bulletSpeed + .5;
	//int framesToLive1 = (pulseRadius * 2) / bulletSpeed + .5;
	//int framesToLive2 = (pulseRadius * 2) / bulletSpeed + .5;

	//BasicBullet::GROWING_TREE
	SetNumLaunchers(3);
	launchers[0] = new Launcher( this, BasicBullet::BAT, 8, 1, GetPosition(), V2d( 1, 0 ), 0, framesToLive, false );
	launchers[0]->SetBulletSpeed(bulletSpeed0);
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->hitboxInfo->hType = HitboxInfo::RED;

	launchers[1] = new Launcher(this, BasicBullet::PATROLLER, 8, 1, GetPosition(), V2d(1, 0), 0, framesToLive, false);
	launchers[1]->SetBulletSpeed(bulletSpeed1);
	launchers[1]->hitboxInfo->damage = 60;
	launchers[1]->hitboxInfo->hType = HitboxInfo::RED;

	launchers[2] = new Launcher(this, BasicBullet::BIG_OWL, 8, 1, GetPosition(), V2d(1, 0), 0, framesToLive, false);
	launchers[2]->SetBulletSpeed(bulletSpeed2);
	launchers[2]->hitboxInfo->damage = 60;
	launchers[2]->hitboxInfo->hType = HitboxInfo::RED;

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	SetOffGroundHeight(ts->tileHeight / 2.f - 0 * scale);
	

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;
	hitboxInfo->hType = HitboxInfo::RED;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	
	cutObject->SetTileset(ts);
	cutObject->SetScale(scale);

	ts_bulletExplode = GetSizedTileset( "FX/bullet_explode3_64x64.png");

	ResetEnemy();
}

GrowingTree::~GrowingTree()
{
}

void GrowingTree::HandleNoHealth()
{
	cutObject->rotateAngle = sprite.getRotation();
}

void GrowingTree::SetLevel( int lev)
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

void GrowingTree::ResetEnemy()
{
	/*switch (startPowerLevel)
	{

	}*/
	launchers[0]->position = GetPosition();
	launchers[1]->position = GetPosition();
	launchers[2]->position = GetPosition();

	action = NEUTRAL0;
	
	data.repCounter = 0;

	data.powerLevel = 0;

	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void GrowingTree::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case NEUTRAL0:
			break;
		case NEUTRAL1:
			break;
		case NEUTRAL2:
			break;
		case ATTACK0:
			++data.repCounter;
			if (data.repCounter == repsToLevelUp)
			{
				action = ATTACK1;
				data.repCounter = 0;
				data.powerLevel = 1;
			}
			break;
		case LEVEL0TO1:
			action = ATTACK1;
			break;
		case ATTACK1:
			++data.repCounter;
			if (data.repCounter == repsToLevelUp)
			{
				action = ATTACK2;
				data.repCounter = 0;
				data.powerLevel = 2;
			}
			break;
		case LEVEL1TO2:
			action = ATTACK2;
			break;
		case ATTACK2:
			break;

			
		}
	}
}


void GrowingTree::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	switch (action)
	{
	case NEUTRAL0:
		if (dist < attentionRadius)
		{
			action = ATTACK0;
		}
		break;
	case NEUTRAL1:
		if (dist < attentionRadius)
		{
			action = ATTACK1;
		}
		break;
	case NEUTRAL2:
		if (dist < attentionRadius)
		{
			action = ATTACK2;
		}
		break;
	case ATTACK0:
		if (dist > ignoreRadius)
		{
			action = NEUTRAL0;
		}
		break;
	case LEVEL0TO1:
		action = ATTACK1;
		break;
	case ATTACK1:
		if (dist > ignoreRadius)
		{
			action = NEUTRAL1;
		}
		break;
	case LEVEL1TO2:
		action = ATTACK2;
		break;
	case ATTACK2:
		if (dist > ignoreRadius)
		{
			action = NEUTRAL2;
		}
		break;
	}

	switch (action)
	{
	case NEUTRAL0:
		break;
	case NEUTRAL1:
		break;
	case NEUTRAL2:
		break;
		break;
	case ATTACK0:
		break;
	case LEVEL0TO1:
		action = ATTACK1;
		break;
	case ATTACK1:
		break;
	case LEVEL1TO2:
		action = ATTACK2;
		break;
	case ATTACK2:
		break;
	}

	if ((action == ATTACK0 || action == ATTACK1 || action == ATTACK2)
		&& frame == 1 && slowCounter == 1)
	{
		launchers[data.powerLevel]->facingDir = PlayerDir();
		launchers[data.powerLevel]->Fire();
	}
}

void GrowingTree::Fire()
{
	
	//powerLevel = startPowerLevel;



	////launcher->Reset();
	//Vector2f start( 0, -pulseRadius );

	//Launcher *launcher = launchers[0];

	//Transform t;
	//for( int i = 0; i < totalBullets; ++i )
	//{
	//	Vector2f trans = t.transformPoint( start );
	//	launcher->position = GetPosition() + V2d( trans.x, trans.y );
	//	launcher->facingDir = normalize(GetPosition() - launcher->position );
	//	launcher->Fire();
	//	
	//	t.rotate( 360.f / totalBullets );
	//}
}

void GrowingTree::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}

void GrowingTree::UpdateSprite()
{
	
	int tileIndex = 0;
	switch (action)
	{
	case NEUTRAL0:
		tileIndex = 0;
		break;
	case NEUTRAL1:
		tileIndex = 1;
		break;
	case NEUTRAL2:
		tileIndex = 2;
		break;
	case ATTACK0:
		tileIndex = 0;
		break;
	case LEVEL0TO1:
		tileIndex = 0;
		break;
	case ATTACK1:
		tileIndex = 1;
		break;
	case LEVEL1TO2:
		tileIndex = 1;
		break;
	case ATTACK2:
		tileIndex = 2;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(tileIndex));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	Vector2f posF = GetPositionF();
	sprite.setPosition(posF);
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
	

	switch(data.powerLevel )
	{
	case 0:
	//	sprite.setColor( Color::White );
		break;
	case 1:
	//	sprite.setColor( Color::Blue );
		break;
	case 2:
	//	sprite.setColor( Color::Green );
		break;
	case 3:
	//	sprite.setColor( Color::Red );
		break;
	}
}

void GrowingTree::DirectKill()
{
	Enemy::DirectKill();

	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[i]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit.SetEmpty();
}

void GrowingTree::BulletHitTerrain(BasicBullet *b, 
	Edge *edge, V2d &pos)
{
	/*V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);*/
}

void GrowingTree::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
}

int GrowingTree::GetNumStoredBytes()
{
	int totalLauncherBytes = 0;
	for (int i = 0; i < numLaunchers; ++i)
	{
		totalLauncherBytes += launchers[i]->GetNumStoredBytes();
	}

	return sizeof(MyData) + totalLauncherBytes;
}

void GrowingTree::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->StoreBytes(bytes);
		bytes += launchers[i]->GetNumStoredBytes();
	}
	
}

void GrowingTree::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->SetFromBytes(bytes);
		bytes += launchers[i]->GetNumStoredBytes();
	}
}