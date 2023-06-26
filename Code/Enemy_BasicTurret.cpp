#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BasicTurret.h"
#include "Shield.h"
#include "Actor.h"
//#include 


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


BasicTurret::BasicTurret(ActorParams *ap )
		:Enemy( EnemyType::EN_BASICTURRET, ap )
{
	SetNumActions(A_Count);
	SetEditorActions(ATTACK, WAIT, 0);

	SetLevel(ap->GetLevel());

	framesWait = 60;
	bulletSpeed = 10;

	double width = 208;
	double height = 176;

	fireSound = GetSound("Enemies/turret_shoot");

	ts = GetSizedTileset("Enemies/W1/turret_208x176.png");

	width *= scale;
	height *= scale;
	detectRad = 1500;

	SetOffGroundHeight(height / 2.f - 30 * scale);

	//SetCurrPosInfo(ap->posInfo);
	
	//testShield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);
	
	
	ts_bulletExplode = GetSizedTileset("FX/bullet_explode1_64x64.png");

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->hType = HitboxInfo::BLUE;

	BasicCircleHurtBodySetup(64);
	BasicCircleHitBodySetup(64);
	hitBody.hitboxInfo = hitboxInfo;

	animationFactor = 4;

	Edge *ground = startPosInfo.GetEdge();
	V2d gn(0, -1);
	V2d launchPos = GetPosition();

	if (ground != NULL)
	{
		gn = ground->Normal();
		launchPos = ground->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
	}

	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::BASIC_TURRET, 16, 1, launchPos, gn, 0, 300 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->hitboxInfo->hType = HitboxInfo::BLUE;

	cutObject->Setup(ts, 12, 11, scale, 0);
	
	//testShield->SetPosition(GetPosition());

	ResetEnemy();
}

BasicTurret::~BasicTurret()
{
	//delete testShield;
}

void BasicTurret::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	if (startPosInfo.ground != NULL)
	{
		launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
		launchers[0]->facingDir = startPosInfo.GetEdge()->Normal();
	}

	//testShield->SetPosition(GetPosition());
}

void BasicTurret::SetLevel(int lev)
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


void BasicTurret::ResetEnemy()
{
	//launchers[0]->Reset();
	frame = 0;

	action = WAIT;
	SetHurtboxes(&hurtBody, 0);
	SetHitboxes(&hitBody, 0);

	//cutObject->SetRotation(sprite.getRotation());

	//currShield = testShield;
	//testShield->Reset();

	UpdateSprite();
}

void BasicTurret::BulletHitTerrain( BasicBullet *b,
		Edge *edge, V2d &pos )
{
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );

	if( b->launcher->def_e == NULL )
		b->launcher->SetDefaultCollision(b->framesToLive, edge, pos);
}

void BasicTurret::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
	

	b->launcher->DeactivateBullet( b );
}

void BasicTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	switch (action)
	{
		case WAIT:
		{
			if (length(playerPos - GetPosition()) < detectRad)
			{
				action = ATTACK;
				frame = 0;
			}
			break;
		}
		case ATTACK:
		{
			if (frame == 11 * animationFactor)
			{
				frame = 0;
				if (length(playerPos - GetPosition()) >= DEFAULT_IGNORE_RADIUS)
				{
					action = WAIT;
					frame = 0;
				}
			}
			else if (frame == 3 * animationFactor && slowCounter == 1)
			{
				launchers[0]->Fire();
				//sess->ActivateSoundAtPos(GetPosition(), fireSound);
				//launchers[1]->Fire();
				//launchers[2]->Fire();
			}
			break;
		}
	}
}

void BasicTurret::UpdatePreLauncherPhysics()
{
	if (!prelimBox.Intersects(sess->GetPlayer(0)->hurtBody))
	{
		launchers[0]->skipPlayerCollideForSubstep = true;
	}
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);

	prelimBox.DebugDraw(CollisionBox::Hit, target);
}

void BasicTurret::DirectKill()
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

void BasicTurret::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}

void BasicTurret::UpdateSprite()
{
	if (action == WAIT)
	{
		sprite.setTextureRect(ts->GetSubRect(0));
	}
	else
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

void BasicTurret::SetupPreCollision()
{

}

V2d BasicTurret::SetupPrelimBox()
{
	Launcher *launcher = launchers[0];

	launcher->def_e = NULL;
	V2d finalPos;

	launcher->interactWithTerrain = true;
	launcher->interactWithPlayer = false;

	launcher->Reset();
	launcher->Fire();
	BasicBullet *bb = launcher->activeBullets;
	bool collide = true;

	while (launcher->GetActiveCount() > 0)
	{
		launcher->UpdatePrePhysics();


		launcher->UpdatePhysics(0, true);
		launcher->UpdatePhysics(1, true);

		/*if (slowCounter == slowMultiple && bb->framesToLive == 1 )
		{

		}*/

		launcher->UpdatePostPhysics();

		if (bb->framesToLive == 0)
		{
			finalPos = bb->position;
			collide = false;
		}
	}

	launcher->interactWithTerrain = false;
	launcher->interactWithPlayer = true;

	if (collide)
	{
		finalPos = launcher->def_pos;
	}

	return finalPos;
}

void BasicTurret::Setup()
{
	Edge *ground = startPosInfo.GetEdge();
	V2d gn(0, -1);
	V2d launchPos = GetPosition();

	Launcher *launcher = launchers[0];

	gn = ground->Normal();
	launchPos = ground->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);

	launcher->position = launchPos;
	launcher->facingDir = gn;

	V2d finalPos = SetupPrelimBox();//TurretSetup();

	double rad = Launcher::GetRadius(launcher->bulletType);
	double width = length(finalPos - launcher->position) + rad * 2;

	prelimBox.isCircle = false;
	prelimBox.rw = width / 2;
	prelimBox.rh = rad;

	V2d norm = currPosInfo.GetEdge()->Normal();


	V2d along = currPosInfo.GetEdge()->Along();//normalize(ground->v1 - ground->v0);

	prelimBox.globalAngle = atan2(norm.y, norm.x);

	prelimBox.globalPosition = (finalPos + launcher->position) / 2.0;

	cutObject->SetRotation(sprite.getRotation());

	SetSpawnRect();
}

int BasicTurret::GetNumStoredBytes()
{
	//return 0;
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void BasicTurret::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);

	bytes += launchers[0]->GetNumStoredBytes();
}

void BasicTurret::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	bytes += sizeof(MyData);
	SetBasicEnemyData(data);

	launchers[0]->SetFromBytes(bytes);

	bytes += launchers[0]->GetNumStoredBytes();
}