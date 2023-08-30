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
	SetEditorActions(IDLE, 0, 0);

	SetLevel(ap->GetLevel());

	actionLength[IDLE] = 2;
	actionLength[ATTACK] = 11;//150;

	animFactor[IDLE] = 1;
	animFactor[ATTACK] = 3;
	actionLength[RECOVER] = 150 - actionLength[ATTACK] * animFactor[ATTACK];

	//actionLength[NEUTRAL0] = 2;
	//actionLength[NEUTRAL1] = 2;
	//actionLength[NEUTRAL2] = 2;

	//actionLength[ATTACK0] = 150;//60;
	//actionLength[LEVEL0TO1] = 30;
	//actionLength[ATTACK1] = 60;
	//actionLength[LEVEL1TO2] = 30;
	//actionLength[ATTACK2] = 60;

	//animFactor[NEUTRAL0] = 1;
	//animFactor[NEUTRAL1] = 1;
	//animFactor[NEUTRAL2] = 1;
	//animFactor[ATTACK0] = 1;
	//animFactor[LEVEL0TO1] = 1;
	//animFactor[ATTACK1] = 1;
	//animFactor[LEVEL1TO2] = 1;
	//animFactor[ATTACK2] = 1;

	//32, 0, 1000

	

	//attentionRadius = 1000;
	//ignoreRadius = 1200;

	totalBullets = 32;
	startPowerLevel = 0;
	pulseRadius = 1000;
	repsToLevelUp = 100;//1;//3;

	ts = GetSizedTileset("Enemies/W5/claw_128x160.png");
	sprite.setTexture( *ts->texture );

	double height = 160 * scale;


	//SetOffGroundHeight(400 * scale);

	double bulletSpeed0 = 13;
	double bulletSpeed1 = 13;
	double bulletSpeed2 = 13;
	double bulletSpeed = 10;

	int framesToLive = 30;//60;//60 * 3;
	//int framesToLive0 = ( pulseRadius * 2 ) / bulletSpeed + .5;
	//int framesToLive1 = (pulseRadius * 2) / bulletSpeed + .5;
	//int framesToLive2 = (pulseRadius * 2) / bulletSpeed + .5;

	//BasicBullet::GROWING_TREE
	SetNumLaunchers(1);

	launchers[0] = new Launcher(this, BasicBullet::GROWING_TREE, 8, 3, GetPosition(), V2d(1, 0), /*PI * 2*/ PI * 2, 60, false);
	launchers[0]->SetBulletSpeed(bulletSpeed0);
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->hitboxInfo->hType = HitboxInfo::RED;

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	SetOffGroundHeight(50 * scale);//ts->tileHeight / 2.f - 20 * scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;
	hitboxInfo->hType = HitboxInfo::RED;

	/*BasicCircleHurtBodySetup(48, 0, V2d(0, -30), GetPosition());
	BasicCircleHitBodySetup(48, 0, V2d(0, -30), GetPosition());*/

	BasicCircleHitBodySetup(32, 0, V2d( 0, 15 ), GetPosition());
	BasicCircleHurtBodySetup(32, 0, V2d( 0, 15 ), GetPosition() );

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
		scale = 2.0;
		//scale = 1.0;
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

	

	action = IDLE;


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
		switch (action)
		{
		case IDLE:
			break;
		case ATTACK:
		{
			action = RECOVER;
			frame = 0;
			break;
		}
		case RECOVER:
		{
			if (PlayerDist() > 1000)
			{
				action = IDLE;
				frame = 0;
			}
			else
			{
				action = ATTACK;
				frame = 0;
			}
			break;
		}
		}
	
	}
}


void GrowingTree::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	switch (action)
	{
	case IDLE:
		if (dist < DEFAULT_DETECT_RADIUS - 200)
		{
			action = ATTACK;
			frame = 0;
		}
		break;
	}

	if (action == ATTACK && frame == 0 && slowCounter == 1)
	{
		V2d gNorm = currPosInfo.GetEdge()->Normal();
		launchers[0]->position = GetPosition() + gNorm * 60.0;

		launchers[0]->facingDir = PlayerDir();
		//launchers[0]->facingDir = startPosInfo.GetEdge()->Normal();
		launchers[0]->Fire();
	}

	//if (action == ATTACK0)
	//{
	//	//if (frame % 5 == 0 && frame < 30 )
	//	if( frame == 1 && slowCounter == slowMultiple)
	//	{
	//		launchers[data.powerLevel]->facingDir = startPosInfo.GetEdge()->Normal();//PlayerDir();
	//		launchers[data.powerLevel]->Fire();
	//	}
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
	case IDLE:
		tileIndex = 0;
		break;
	case ATTACK:
		tileIndex = frame / animFactor[ATTACK] + 1;
		break;
	case RECOVER:
		tileIndex = 11;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(tileIndex));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	Vector2f posF = GetPositionF();
	sprite.setPosition(posF);
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

void GrowingTree::DirectKill()
{
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

	Enemy::DirectKill();
}

void GrowingTree::BulletHitTerrain(BasicBullet *b, 
	Edge *edge, V2d &pos)
{
	/*V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);*/
}


void GrowingTree::UpdateBullet(BasicBullet *b)
{
	V2d pDir = normalize(sess->GetPlayerPos(0) - b->position);

	double accel = .7;//.3;//1.0;
	double maxVel = b->launcher->bulletSpeed;
	b->velocity += pDir * accel;
	if (length(b->velocity) > maxVel)
	{
		b->velocity = normalize(b->velocity) * maxVel;
	}
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