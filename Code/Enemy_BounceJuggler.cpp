#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BounceJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "MainMenu.h"
#include "AbsorbParticles.h"

using namespace std;
using namespace sf;

void BounceJuggler::SetLevel(int lev)
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

BounceJuggler::BounceJuggler(ActorParams *ap)
	:Enemy(EnemyType::EN_BOUNCEJUGGLER, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_COMBOER;

	SetLevel(ap->GetLevel());

	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	flySpeed = 22;
	maxFlyFrames = 90;

	UpdateParamsSettings();

	CreateSurfaceMover(startPosInfo, 64, this);

	hitLimit = -1;

	action = S_FLOAT;

	//t//s = GetSizedTileset("Enemies/comboers_128x128.png");
	ts = GetSizedTileset("Enemies/W3/bounce_comboer_160x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);



	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/

	BasicCircleHurtBodySetup(48);
	//BasicCircleHitBodySetup(48);
	//hitBody.hitboxInfo = hitboxInfo;

	comboObj = new ComboObject(this);
	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->comboer = true;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::YELLOW;

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 10;
	actionLength[S_BOUNCE] = 10;
	actionLength[S_RETURN] = 30;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_BOUNCE] = 1;
	animFactor[S_RETURN] = 1;

	ResetEnemy();
}

BounceJuggler::~BounceJuggler()
{
}
void BounceJuggler::ResetEnemy()
{
	facingRight = true;

	data.flyFrame = 0;
	sprite.setRotation(0);
	comboObj->Reset();
	surfaceMover->SetVelocity(V2d(0, 0));
	DefaultHurtboxesOn();
	//DefaultHitboxesOn();
	action = S_FLOAT;
	frame = 0;
	receivedHit.SetEmpty();

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	UpdateHitboxes();

	UpdateSprite();
}

void BounceJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	surfaceMover->SetVelocity(vel);
}

void BounceJuggler::Throw(V2d vel)
{
	surfaceMover->SetVelocity(vel);
}

void BounceJuggler::Return()
{
	action = S_RETURN;
	frame = 0;

	facingRight = true;

	data.flyFrame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();
	HitboxesOff();

	numHealth = maxHealth;
}

void BounceJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
	ConfirmHitNoKill();
	numHealth = maxHealth;
	
	HurtboxesOff();
	HitboxesOff();
}

void BounceJuggler::PopThrow()
{
	action = S_FLY;
	frame = 0;

	data.flyFrame = 0;

	V2d dir;

	dir = receivedHit.hDir;//normalize(receivedHit->hDir);
						   //cout << "dir: " << dir.x << "," << dir.y << endl;
	if (dir.x == 0 && dir.y == 0)
	{
		//dir = receivedHit.

		dir = -normalize(surfaceMover->GetVel());
		assert(dir.x != 0 || dir.y != 0);
		action = S_BOUNCE;
		frame = 0;
	}

	V2d hit = dir * flySpeed;

	if (dir.x < 0)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}
	
	Pop();

	Throw(hit);

	sess->PlayerAddActiveComboObj(comboObj);
}

void BounceJuggler::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		//Actor *player = owner->GetPlayer(0);

		if (numHealth <= 0)
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();

			PopThrow();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
	}
}

void BounceJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			surfaceMover->Set(startPosInfo);
			//DefaultHitboxesOn();
			DefaultHurtboxesOn();
			action = S_FLOAT;
			frame =  0;
			break;
		}
	}
}

void BounceJuggler::HandleNoHealth()
{

}

bool BounceJuggler::CanComboHit(Enemy *e)
{
	if (e->type == EN_BOUNCEJUGGLER)
	{
		return false;
	}

	return true;
}

void BounceJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLY:
	case S_BOUNCE:
	{
		Enemy::UpdateEnemyPhysics();
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(surfaceMover->GetVel());
}

void BounceJuggler::FrameIncrement()
{
	if (action == S_FLY || action == S_BOUNCE)
	{
		if (data.flyFrame == maxFlyFrames)
		{
			Return();
		}
		else
		{
			data.flyFrame++;
		}
	}
}

void BounceJuggler::ComboHit()
{
	pauseFrames = 5;
	/*++data.currHits;
	if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		Return();
	}*/
}

void BounceJuggler::ComboKill(Enemy *e)
{
	/*if (level == 2)
	{
		V2d playerDir = normalize(sess->GetPlayerPos(0) - GetPosition());
		Throw(playerDir * flySpeed);

		action = S_BOUNCE;
		frame = 0;
	}*/
}

void BounceJuggler::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	ts->SetSubRect(sprite, tile, !facingRight);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	if (action == S_FLOAT)
	{
		sprite.setRotation(0);
	}
	else if (action == S_RETURN)
	{

	}
	else
	{
		V2d vel = normalize(surfaceMover->GetVel());

		double ang = GetVectorAngleCW(vel);
		if (facingRight)
		{
			sprite.setRotation(ang / PI * 180.0);
		}
		else
		{
			sprite.setRotation(ang / PI * 180.0 + 180);
		}
	}	
}
void BounceJuggler::HitTerrainAerial(Edge * edge, double quant)
{
	V2d pos = edge->GetPosition(quant);

	V2d en = edge->Normal();
	if (pos == edge->v0)
	{
		en = normalize(GetPosition() - pos);
	}
	else if (pos == edge->v1)
	{
		en = normalize(GetPosition() - pos);
	}
	double d = dot(surfaceMover->GetVel(), en);
	V2d ref = surfaceMover->GetVel() - (2.0 * d * en);
	surfaceMover->SetVelocity(ref);
	surfaceMover->ground = NULL;

	action = S_BOUNCE;
	frame = 0;
}

int BounceJuggler::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void BounceJuggler::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void BounceJuggler::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}