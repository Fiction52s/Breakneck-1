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

	firingCounter = 0;

	SetLevel(ap->GetLevel());

	framesWait = 60;
	bulletSpeed = 10;
	receivedHit = NULL;

	double width = 208;
	double height = 176;

	fireSound = sess->GetSound("Enemies/turret_shoot");

	ts = sess->GetTileset("Enemies/turret_208x176.png", width, height);//"basicturret_128x80.png", width, height );
	ts_aura = sess->GetTileset("Enemies/turret_aura_208x176.png", width, height);

	width *= scale;
	height *= scale;
	detectRad = 1500;
	

	SetOffGroundHeight(height / 2.f - 30 * scale);

	//SetCurrPosInfo(ap->posInfo);
	
	testShield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);
	//testShield->SetPosition(GetPosition());

	auraSprite.setTexture(*ts_aura->texture);

	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);
	
	
	ts_bulletExplode = sess->GetTileset( "FX/bullet_explode1_64x64.png", 64, 64 );

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3*60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

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
	launchers[0]->hitboxInfo->damage = 18;	

	cutObject->Setup(ts, 12, 11, scale, 0);
	
	testShield->SetPosition(GetPosition());

	ResetEnemy();
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
	dead = false;
	frame = 0;

	action = WAIT;
	SetHurtboxes(&hurtBody, 0);
	SetHitboxes(&hitBody, 0);

	//cutObject->SetRotation(sprite.getRotation());

	currShield = testShield;
	testShield->Reset();

	UpdateSprite();
}

void BasicTurret::BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos )
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
	sess->PlayerApplyHit( playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position );

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
				if (length(playerPos - GetPosition()) >= detectRad)
				{
					action = WAIT;
					frame = 0;
				}
			}
			else if (frame == 3 * animationFactor && slowCounter == 1)
			{
				launchers[0]->Fire();
				sess->ActivateSoundAtPos(GetPosition(), fireSound);
				//launchers[1]->Fire();
				//launchers[2]->Fire();
			}
			break;
		}
	}
}

void BasicTurret::UpdatePreLauncherPhysics()
{
	for (int i = 0; i < 1; ++i)
	{
		if (!prelimBox[i].Intersects(sess->GetPlayer(i)->hurtBody))
		{
			launchers[i]->skipPlayerCollideForSubstep = true;
		}
	}
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);

	for (int i = 0; i < 3; ++i)
	{
		prelimBox[i].DebugDraw( CollisionBox::Hit, target);
	}
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
	receivedHit = NULL;
}

void BasicTurret::EnemyDraw(sf::RenderTarget *target )
{
	target->draw(auraSprite);
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
	
	SyncSpriteInfo(auraSprite, sprite);
}

void BasicTurret::SetupPreCollision()
{

}

void BasicTurret::Setup()
{
	Edge *ground = startPosInfo.GetEdge();
	V2d gn(0, -1);
	V2d launchPos = GetPosition();

	gn = ground->Normal();
	launchPos = ground->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);

	launchers[0]->position = launchPos;
	launchers[0]->facingDir = gn;

	for (int li = 0; li < 1; ++li)
	{
		V2d finalPos = TurretSetup();

		double rad = Launcher::GetRadius(launchers[li]->bulletType);
		double width = length(finalPos - launchers[li]->position) + rad * 2;

		prelimBox[li].isCircle = false;
		prelimBox[li].rw = width / 2;
		prelimBox[li].rh = rad;

		V2d norm = currPosInfo.GetEdge()->Normal();


		V2d along = currPosInfo.GetEdge()->Along();//normalize(ground->v1 - ground->v0);
		if (li == 0)
		{
			prelimBox[li].globalAngle = atan2(norm.y, norm.x);
		}
		else
		{
			prelimBox[li].globalAngle = atan2(along.y, along.x);
		}
		
		prelimBox[li].globalPosition = (finalPos + launchers[li]->position) / 2.0;
	}

	cutObject->SetRotation(sprite.getRotation());

	SetSpawnRect();
}