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
#include "AbsorbParticles.h"
#include "Actor.h"

using namespace std;
using namespace sf;

void HungryComboer::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();

	JugglerParams *jParams = (JugglerParams*)editParams;
	maxEdible = jParams->numJuggles;

	if (maxEdible == 0)
	{
		limitedEating = false;
	}
	else
	{
		limitedEating = true;
		UpdateEatenNumberText(maxEdible);
	}
}

HungryComboer::HungryComboer(ActorParams *ap)
	:Enemy(EnemyType::EN_HUNGRYCOMBOER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(S_FLY, S_FLY, 0);

	SetLevel(ap->GetLevel());

	origScale = scale;
	
	maxWaitFrames = 180;

	numEatenText.setFont(sess->mainMenu->arial);
	numEatenText.setFillColor(Color::White);
	numEatenText.setOutlineColor(Color::Black);
	numEatenText.setOutlineThickness(3);
	numEatenText.setCharacterSize(32);

	flySpeed = 12;

	UpdateParamsSettings();

	numGrowthLevels = 4;

	hitLimit = -1;

	action = S_FLOAT;

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::RED;

	origSize = 48;

	BasicCircleHurtBodySetup(origSize);
	BasicCircleHitBodySetup(origSize);

	hitBody.hitboxInfo = hitboxInfo;

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
	actionLength[S_FLY] = 20;

	actionLength[S_TRACKPLAYER] = 10;
	actionLength[S_TRACKENEMY] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_TRACKPLAYER] = 1;
	animFactor[S_TRACKENEMY] = 1;
	animFactor[S_RETURN] = 6;

	//actionLength[S_WAIT] = 10;
	//animFactor[S_WAIT] = 1;

	ResetEnemy();
}

HungryComboer::~HungryComboer()
{
}
void HungryComboer::ResetEnemy()
{
	sprite.setRotation(0);

	data.growthLevel = 0;

	scale = origScale;
	UpdateScale();

	data.currHits = 0;
	comboObj->Reset();

	data.velocity = V2d(0, 0);
	
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	action = S_FLOAT;
	frame = 0;

	data.waitFrame = 0;
	data.numEaten = 0;

	UpdateEatenNumberText(maxEdible);

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

void HungryComboer::UpdateEatenNumberText(int reps)
{
	if (limitedEating)
	{
		data.numEatenTextNumber = reps;
		numEatenText.setString(to_string(reps));
		numEatenText.setOrigin(numEatenText.getLocalBounds().left
			+ numEatenText.getLocalBounds().width / 2,
			numEatenText.getLocalBounds().top
			+ numEatenText.getLocalBounds().height / 2);
		numEatenText.setPosition(sprite.getPosition());
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
	UpdateEatenNumberText(0);

	data.numEaten = 0;

	numHealth = maxHealth;
}

void HungryComboer::Pop()
{
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

	Pop();

	Throw(dir * GetFlySpeed());

	sess->PlayerAddActiveComboObj(comboObj);
}

void HungryComboer::IHitPlayer(int index)
{
	if (action != S_FLOAT)
	{
		Eat();
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

			if (!limitedEating)
			{
				if (hasMonitor && !suppressMonitor)
				{
					sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
						sess->GetPlayer(0), 1, GetPosition());
					suppressMonitor = true;
					PlayKeyDeathSound();
				}

				sess->PlayerConfirmEnemyNoKill(this);
				ConfirmHitNoKill();
			}

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
			action = S_TRACKPLAYER;//S_TRACKENEMY;
			frame = 0;
			DefaultHurtboxesOn();
			DefaultHitboxesOn();
			break;
		case S_RETURN:
			UpdateEatenNumberText(maxEdible);
			SetCurrPosInfo(startPosInfo);
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			action = S_FLOAT;
			frame = 0;
			break;
		}
	}

	/*if (action == S_TRACKENEMY)
	{
		if (sess->GetPlayer(0)->IsBlockAction(sess->GetPlayer(0)->action))
		{
			action = S_WAIT;
			frame = 0;
			data.waitFrame = 0;
			data.chaseTarget = NULL;
			data.velocity = V2d();
			DefaultHurtboxesOn();
		}
	}*/


	if (action == S_TRACKENEMY || action == S_TRACKPLAYER )
	{
		if (data.chaseTarget != NULL && data.chaseTarget->dead)
		{
			data.chaseTarget = NULL;
		}

		//if (chaseTarget == NULL)
		{
			Enemy *foundEnemy = NULL;
			int foundIndex = 0;
			double checkRadius = 300;
			if (GetClosestEnemyPos(GetPosition(), checkRadius, foundEnemy, foundIndex))
			{
				action = S_TRACKENEMY;
				data.chaseTarget = foundEnemy;
				data.chaseIndex = foundIndex;
			}
			else
			{
				action = S_TRACKPLAYER;
				data.chaseTarget = NULL;
				//frame = 0;
			}
		}
	}

	if ((action == S_TRACKENEMY && data.chaseTarget != NULL) || action == S_TRACKPLAYER)
	{
		double accel = 1.5;//max( 1.0, 3.0 / ((double)data.waitFrame / maxWaitFrames));// / 3;//1.5;// .5;//.5;
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
	fSpeed -= data.growthLevel * 1.0;
	return fSpeed;
}

//checks to see if it can home on
bool HungryComboer::IsValidTrackEnemy(Enemy *e)
{
	if (e != this && e->numHealth > 0 )
	{
		if (e->type == EnemyType::EN_HUNGRYCOMBOER)
		{
			return false;
		}

		return true;
	}

	return false;
}

void HungryComboer::Eat()
{
	++data.numEaten;

	if (limitedEating && data.numEaten == maxEdible)
	{
		if (hasMonitor && !suppressMonitor)
		{
			sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
			sess->GetPlayer(0), 1, GetPosition());
			suppressMonitor = true;
			PlayKeyDeathSound();
		}
		Return();
	}
	else
	{
		UpdateEatenNumberText(maxEdible - data.numEaten);

		if (data.growthLevel < numGrowthLevels)
		{
			data.growthLevel++;
			scale += .2;
			UpdateScale();
		}
		else
		{
			//scale = origScale;
			//growthLevel = 0;
			//UpdateScale();
		}
	}
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
	Enemy::DirectKill();
	sess->PlayerRemoveActiveComboer(comboObj);
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
	case S_TRACKPLAYER:
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
	hitBody.GetCollisionBoxes(0).front().rw = scale * origSize;
	hitBody.GetCollisionBoxes(0).front().rh = scale * origSize;
	hurtBody.GetCollisionBoxes(0).front().rw = scale * origSize;
	hurtBody.GetCollisionBoxes(0).front().rh = scale * origSize;
}

void HungryComboer::FrameIncrement()
{
	if (action == S_FLY || action == S_TRACKENEMY || action == S_TRACKPLAYER )//|| action == S_WAIT )
	{
		if (data.waitFrame == maxWaitFrames)
		{
			Return();
		}
		else
		{
			data.waitFrame++;
		}

	}

}

//void HungryComboer::HandleEntrant(QuadTreeEntrant *qte)
//{
//	Enemy *en = (Enemy*)qte;
//	if (en->type == EnemyType::EN_JUGGLERCATCHER)
//	{
//		JugglerCatcher *catcher = (JugglerCatcher*)qte;
//
//		CollisionBox &hitB = hurtBody.GetCollisionBoxes(0).front();
//		if (catcher->CanCatch() && catcher->hurtBody->Intersects(catcher->currHurtboxFrame, &hitB))
//		{
//			//catcher->Catch();
//			//action = S_EXPLODE;
//			//frame = 0;
//		}
//	}
//}

void HungryComboer::ComboHit()
{
	pauseFrames = 5;
	++data.currHits;
	data.waitFrame = 0;
	if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		Return();
	}
}

void HungryComboer::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(8));
	
	if (action == S_RETURN)
	{
		sprite.setColor(Color::Blue);
	}
	else
	{
		sprite.setColor(Color::White);
	}

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	if (limitedEating)
	{
		numEatenText.setPosition(sprite.getPosition());
	}
}

void HungryComboer::EnemyDraw(sf::RenderTarget *target)
{
	/*sf::CircleShape cs;
	cs.setRadius(300);
	cs.setPosition(sprite.getPosition());
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	cs.setFillColor(Color(255, 0, 0, 100));
	target->draw(cs);*/

	DrawSprite(target, sprite);

	

	if (limitedEating)
	{
		target->draw(numEatenText);
	}
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

	UpdateEatenNumberText(data.numEatenTextNumber);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}