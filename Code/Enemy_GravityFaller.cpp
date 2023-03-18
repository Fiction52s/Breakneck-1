#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include <cstdlib>
#include "Enemy_GravityFaller.h"
#include "Shield.h"

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

GravityFaller::GravityFaller(ActorParams *ap)
	:Enemy(EnemyType::EN_GRAVITYFALLER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 10;
	actionLength[DOWNCHARGE] = 10;
	actionLength[FALLDOWN] = 10;
	actionLength[UPCHARGE] = 10;
	actionLength[FALLUP] = 10;
	actionLength[REVERSEUPTODOWN] = 10;
	actionLength[REVERSEDOWNTOUP] = 10;


	animFactor[IDLE] = 1;
	animFactor[DOWNCHARGE] = 1;
	animFactor[FALLDOWN] = 1;
	animFactor[UPCHARGE] = 1;
	animFactor[FALLUP] = 1;
	animFactor[REVERSEUPTODOWN] = 1;
	animFactor[REVERSEDOWNTOUP] = 1;

	maxFallSpeed = 25;

	double width = 128;
	double height = 128;
	ts = GetSizedTileset("Enemies/W2/gravity_faller_128x128.png");


	//shield = new Shield(Shield::ShieldType::T_BLOCK, 64 * scale, 3, this);

	CreateSurfaceMover(startPosInfo, 30, this);

	gravity = 1.0;//gravityFactor / 64.0;

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 20;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::GREEN;

	BasicCircleHurtBodySetup(40);
	BasicCircleHitBodySetup(40);
	hitBody.hitboxInfo = hitboxInfo;

	chargeLength = 20;

	cutObject->Setup(ts, 6, 5, scale);

	ResetEnemy();
}

void GravityFaller::SetLevel(int lev)
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

void GravityFaller::ResetEnemy()
{
	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	action = IDLE;
	frame = 0;
	data.fallFrames = 0;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	
	data.chargeFrame = 0;

	facingRight = false;

	Edge *startEdge = startPosInfo.GetEdge();
	if ( startEdge != NULL)
	{
		startNormal = startEdge->Normal();
	}
	

	UpdateHitboxes();

	//shield->Reset();

	/*if (!hasMonitor)
	{
		currShield = shield;
		
	}

	shield->SetPosition(GetPosition());*/
	

	UpdateSprite();
}

void GravityFaller::FrameIncrement()
{
	if (action == DOWNCHARGE || action == UPCHARGE)
	{
		data.chargeFrame++;
	}
	else if (action == FALLDOWN || action == FALLUP)
	{
		data.fallFrames++;
	}
}

void GravityFaller::UpdateHitboxes()
{
	Edge *ground = surfaceMover->ground;
	//if (ground != NULL)
	//{
	//	V2d gn = ground->Normal();

	//	V2d knockbackDir(1, -1);
	//	knockbackDir = normalize(knockbackDir);
	//	if (surfaceMover->groundSpeed > 0)
	//	{
	//		hitboxInfo->kbDir = knockbackDir;
	//		hitboxInfo->knockback = 15;
	//	}
	//	else
	//	{
	//		hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
	//		hitboxInfo->knockback = 15;
	//	}
	//	//hitBody.globalAngle = angle;
	//	//hurtBody.globalAngle = angle;
	//}
	//else
	//{
	//	//hitBody.globalAngle = 0;
	//	//hurtBody.globalAngle = 0;
	//}
	BasicUpdateHitboxes();
}

void GravityFaller::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}
}

void GravityFaller::ProcessState()
{
	ActionEnded();

	
	//Actor *player = owner->GetPlayer(0);

	double distY = PlayerDistY();

	V2d playerPos = sess->GetPlayerPos(0);
	if (action == IDLE)
	{
		if (PlayerDist() < 1000 )
		{
			if (surfaceMover->ground->Normal() == startNormal)
			{
				action = UPCHARGE;
				frame = 0;
				data.chargeFrame = 0;
			}
			else
			{
				action = DOWNCHARGE;
				frame = 0;
				data.chargeFrame = 0;
			}
		}
	}

	if (data.chargeFrame == chargeLength && ( action == DOWNCHARGE || action == UPCHARGE))
	{
		surfaceMover->ClearAirForces();
		if (action == DOWNCHARGE)
		{
			action = FALLDOWN;
			frame = 0;
			surfaceMover->Jump(V2d(0, 0));
			surfaceMover->AddAirForce(startNormal * -gravity);
			data.fallFrames = 0;
		}
		else if (action == UPCHARGE)
		{
			action = FALLUP;
			frame = 0;
			surfaceMover->Jump(V2d(0, 0));
			surfaceMover->AddAirForce(startNormal * gravity);
			data.fallFrames = 0;
		}
	}
	else if (((action == FALLDOWN || action == FALLUP) && data.fallFrames == 30))
		/*|| ( action == FALLDOWN && distY < 0 ) 
		|| (action == FALLUP && distY > 0 ) )*/
	{
		surfaceMover->ClearAirForces();
		if (action == FALLDOWN)
		{
			action = REVERSEDOWNTOUP;
			frame = 0;
			surfaceMover->AddAirForce(startNormal * gravity);
		}
		else if (action == FALLUP)
		{
			action = REVERSEUPTODOWN;
			frame = 0;
			surfaceMover->AddAirForce(startNormal * -gravity);
		}
		
	}
	/*else if ((action == FALLDOWN && distY < -80)
		|| (action == FALLUP && distY > 80))
	{
		surfaceMover->velocity = surfaceMover->velocity / 4.0;
		surfaceMover->ClearAirForces();
		if (action == FALLDOWN)
		{
			action = REVERSEDOWNTOUP;
			frame = 0;
			surfaceMover->AddAirForce(V2d(0, -1));
		}
		else if (action == FALLUP)
		{
			action = REVERSEUPTODOWN;
			frame = 0;
			surfaceMover->AddAirForce(V2d(0, 1));
		}
	}*/
	/*else if (action == FALLUP && distY > 0)
	{
	}*/
	/*else if (action == REVERSEUPTODOWN && mover->velocity.y > 0)
	{
		action = FALLDOWN;
		frame = 0;
		fallFrames = 0;
	}
	else if (action == REVERSEDOWNTOUP && mover->velocity.y < 0)
	{
		action = FALLUP;
		frame = 0;
		fallFrames = 0;
	}*/

	//cout << "vel: " << mover->velocity.x << ", " << mover->velocity.y << endl;
}

void GravityFaller::UpdateEnemyPhysics()
{
	if (surfaceMover->ground != NULL)
	{
	}
	else
	{
		//double grav = gravity;
		////if (action == WALLCLING)
		////{
		////	grav = 0;//.1 * grav;
		////}
		//mover->velocity.y += grav / (numPhysSteps * slowMultiple);

		double velLen = length(surfaceMover->GetVel());
		V2d velDir = normalize(surfaceMover->GetVel());

		if ( velLen >= maxFallSpeed)
		{
			surfaceMover->SetVelocity(velDir * maxFallSpeed);
		}
	}

	//shield->SetPosition(GetPosition());

	Enemy::UpdateEnemyPhysics();
}



void GravityFaller::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void GravityFaller::UpdateSprite()
{
	int tIndex = 0;
	switch (action)
	{
	case IDLE:
		tIndex = 0;
		break;
	case DOWNCHARGE:
		tIndex = 3;
		break;
	case FALLDOWN:
	case REVERSEUPTODOWN:
		tIndex = 4;
		break;
	case UPCHARGE:
	case REVERSEDOWNTOUP:
		tIndex = 1;
		break;
	case FALLUP:
		tIndex = 2;
		break;
	}
	

	float angle = GetVectorAngleCW(startNormal) / PI * 180;

	sprite.setTextureRect(ts->GetSubRect(tIndex));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(angle + 90);
	sprite.setPosition(GetPositionF());
}

void GravityFaller::HitTerrainAerial(Edge * e, double quant)
{
	if (action == FALLDOWN || action == REVERSEDOWNTOUP )
	{
		action = UPCHARGE;
	}
	else if (action == FALLUP || action == REVERSEUPTODOWN )
	{
		action = DOWNCHARGE;
	}

	frame = 0;
	data.chargeFrame = 0;
}

int GravityFaller::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GravityFaller::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void GravityFaller::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}