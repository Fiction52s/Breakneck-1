#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_HungryComboer.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "MainMenu.h"
#include "Actor.h"

using namespace std;
using namespace sf;

HungryComboer::HungryComboer(ActorParams *ap)
	:Enemy(EnemyType::EN_HUNGRYCOMBOER, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_COMBOER;

	SetNumActions(A_Count);
	SetEditorActions(S_FLY, S_FLY, 0);

	SetLevel(ap->GetLevel());

	origScale = scale;
	
	maxWaitFrames = 120;

	maxEdible = 3;

	flySpeed = 15;//12;

	homingRadius = 800;

	UpdateParamsSettings();

	action = S_FLOAT;

	myHitSound = GetSound("Enemies/Comboers/comboer_hungry");

	ts = GetSizedTileset("Enemies/W5/hungry_comboer_160x160.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::RED;*/

	origSize = 48;

	BasicCircleHurtBodySetup(origSize);
	//BasicCircleHitBodySetup(origSize);

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
	comboObj->enemyHitboxInfo->hType = HitboxInfo::RED;

	comboObj->enemyHitBody.BasicCircleSetup(origSize, GetPosition());

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 20;//13;
	actionLength[S_TRACKENEMY] = 13;
	actionLength[S_RETURN] = 7;
	actionLength[S_DIE] = 7;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_TRACKENEMY] = 3;
	animFactor[S_RETURN] = 5;
	animFactor[S_DIE] = 5;

	//actionLength[S_WAIT] = 10;
	//animFactor[S_WAIT] = 1;

	ResetEnemy();
}

HungryComboer::~HungryComboer()
{
}

void HungryComboer::ResetEnemy()
{
	data.waitFrame = 0;
	data.numEaten = 0;
	scale = origScale;
	UpdateScale();

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	comboObj->Reset();

	data.velocity = V2d(0, 0);
	
//	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	action = S_FLOAT;
	frame = 0;

	data.chaseTarget = NULL;
	data.chaseIndex = 0;

	UpdateHitboxes();


	UpdateSprite();
}

void HungryComboer::SetLevel(int lev)
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

void HungryComboer::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	data.velocity = vel;
}

void HungryComboer::Throw(V2d vel)
{
	data.velocity = vel;
}

void HungryComboer::Return()
{
	action = S_RETURN;
	frame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();
	HitboxesOff();

	receivedHit.SetEmpty();

	scale = origScale;
	UpdateScale();

	data.numEaten = 0;

	numHealth = maxHealth;
}

void HungryComboer::Pop()
{
	sess->ActivateSoundAtPos(GetPosition(), myHitSound);
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	HurtboxesOff();
	HitboxesOff();
	data.waitFrame = 0;
}

void HungryComboer::PopThrow()
{
	V2d dir;

	dir = Get8Dir(receivedHit.hDir);


	if (action == S_TRACKENEMY)
	{
		Eat();
	}

	Pop();

	Throw(dir * GetFlySpeed());

	if (dir.x < 0)
	{
		facingRight = false;
	}
	else if( dir.x > 0 )
	{
		facingRight = true;
	}

	sess->PlayerAddActiveComboObj(comboObj);
}

void HungryComboer::IHitPlayer(int index)
{
	if (action != S_FLOAT)
	{
		//Eat();
	}
}

void HungryComboer::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			action = S_FLY;
			data.chaseTarget = NULL;
			frame = 0;
			PopThrow();

			if (hasMonitor && !suppressMonitor)
			{
				ActivateDarkAbsorbParticles();
				suppressMonitor = true;
				PlayKeyDeathSound();
			}

			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();

		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
	}
}

void HungryComboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_FLY:
			action = S_TRACKENEMY;
			//HurtboxesOff();
			//DefaultHurtboxesOn();
			frame = 0;
			break;
		case S_RETURN:
			SetCurrPosInfo(startPosInfo);
			//DefaultHitboxesOn();
			DefaultHurtboxesOn();
			action = S_FLOAT;
			frame = 0;
			sprite.setRotation(0);

			if (PlayerDir().x >= 0)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}
			break;
		case S_DIE:
		{
			numHealth = 0;
			dead = true;
			break;
		}
		}
	}

	if (action == S_TRACKENEMY )
	{
		if (data.chaseTarget != NULL && !data.chaseTarget->IsValidTrackEnemy())
		{
			data.chaseTarget = NULL;
		}

		//if (chaseTarget == NULL)
		{
			Enemy *foundEnemy = NULL;
			int foundIndex = 0;
			if (GetClosestEnemyPos(GetPosition(), homingRadius, foundEnemy, foundIndex))
			{
				action = S_TRACKENEMY;
				data.chaseTarget = foundEnemy;
				data.chaseIndex = foundIndex;
			}
			else
			{
				action = S_FLY;//S_TRACKPLAYER;
				data.chaseTarget = NULL;
				//frame = 0;
			}
		}
	}

	if (action == S_TRACKENEMY && data.chaseTarget != NULL)
	{
		double accel = 1.75;//1.5;//max( 1.0, 3.0 / ((double)data.waitFrame / maxWaitFrames));// / 3;//1.5;// .5;//.5;
		V2d trackPos = GetTrackPos();
		V2d trackDir = normalize(trackPos - GetPosition());
		data.velocity += trackDir * accel;
		double fSpeed = GetFlySpeed();
		if (length(data.velocity) > fSpeed)
		{
			data.velocity = normalize(data.velocity) * fSpeed;
		}
	}
}

double HungryComboer::GetFlySpeed()
{
	double fSpeed = flySpeed;
	//fSpeed -= data.growthLevel * 1.0;
	return fSpeed;
}

//checks to see if it can home on
bool HungryComboer::CheckIfEnemyIsTrackable(Enemy *e)
{
	if (e != this && e->numHealth > 0 )
	{
		if (e->type == EnemyType::EN_HUNGRYCOMBOER)
		{
			return false;
		}

		return e->IsValidTrackEnemy();
	}

	return false;
}

void HungryComboer::Eat()
{
	++data.numEaten;

	scale += .2;
	UpdateScale();
}

void HungryComboer::ComboKill(Enemy *e)
{
	if (data.chaseTarget == e)
	{
		data.chaseTarget = NULL;
	}

	/*if (returnsToPlayer)
	{
		action = S_TRACKPLAYER;
		frame = 0;
	}*/

	Eat();
}

void HungryComboer::DirectKill()
{
	sess->PlayerRemoveActiveComboer(comboObj);
	action = S_DIE;
	frame = 0;
}

V2d HungryComboer::GetTrackPos()
{
	if (action == S_TRACKENEMY)
	{
		if (data.chaseTarget != NULL)
		{
			return data.chaseTarget->GetCamPoint(data.chaseIndex);
		}
		else
		{
			assert(0);
			return V2d(0, 0);
		}
	}
	else
	{
		return sess->GetPlayerPos(0);
	}
}

void HungryComboer::HandleNoHealth()
{

}

void HungryComboer::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * numStep;

	currPosInfo.position += movementVec;

	/*velocity += gDir * (gravFactor / numStep / slowMultiple);

	if (reversedGrav)
	{
		if (velocity.y < -maxFallSpeed)
		{
			velocity.y = -maxFallSpeed;
		}
	}
	else
	{
		if (velocity.y > maxFallSpeed)
		{
			velocity.y = maxFallSpeed;
		}
	}*/
}

void HungryComboer::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLY:
	case S_TRACKENEMY:
	{
		Move();
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void HungryComboer::UpdateScale()
{
	sprite.setScale(scale, scale);
	//hitBody.GetCollisionBoxes(0).front().rw = scale * origSize;
	//hitBody.GetCollisionBoxes(0).front().rh = scale * origSize;
	hurtBody.GetCollisionBoxes(0).front().rw = scale * origSize;
	hurtBody.GetCollisionBoxes(0).front().rh = scale * origSize;
}

void HungryComboer::FrameIncrement()
{
	if (action == S_FLY || action == S_TRACKENEMY )
	{
		if (data.waitFrame == maxWaitFrames || data.numEaten >= maxEdible )
		{
			Return();
		}
		else
		{
			data.waitFrame++;
		}
	}
}

bool HungryComboer::CanComboHit(Enemy *e)
{
	if (e->type == EN_HUNGRYCOMBOER)
	{
		return false;
	}

	return true;
}

void HungryComboer::ComboHit()
{
	pauseFrames = 6;
	data.waitFrame = 0;
}

void HungryComboer::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		break;
	case S_FLY:
		tile = 0;
		break;
	case S_TRACKENEMY:
		tile = frame / animFactor[action];
		break;
	case S_RETURN:
	case S_DIE:
		tile = frame / animFactor[action] + 13;
		break;
	}


	ts->SetSubRect(sprite, tile, !facingRight);

	if (action == S_FLOAT)
	{
		sprite.setRotation(0);
	}
	else if (action == S_FLY || action == S_TRACKENEMY)
	{
		double ang = GetVectorAngleCW(data.velocity);
		if (facingRight)
		{
			sprite.setRotation(ang / PI * 180.0);
		}
		else
		{
			sprite.setRotation(ang / PI * 180.0 + 180);
		}
	}
	


	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void HungryComboer::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);

	sf::CircleShape cs;
	cs.setRadius(homingRadius);
	cs.setPosition(sprite.getPosition());
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	cs.setFillColor(Color(255, 0, 0, 50));
	target->draw(cs);
}

int HungryComboer::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void HungryComboer::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void HungryComboer::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}