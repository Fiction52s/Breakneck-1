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

using namespace std;
using namespace sf;

void HungryComboer::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedEating)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		maxEdible = jParams->numJuggles;
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

	string typeName = ap->GetTypeName();
	if (typeName == "hungrycomboer")
	{
		returnsToPlayer = false;
		maxWaitFrames = 180;
		limitedEating = false;
	}
	else if (typeName == "limitedhungrycomboer")
	{
		returnsToPlayer = false;
		maxWaitFrames = 180;
		limitedEating = true;
		//not implemented, not sure if want
	}
	else if ( typeName == "hungryreturncomboer")
	{
		sprite.setColor(Color::Green);
		returnsToPlayer = true;
		maxWaitFrames = 360;
		limitedEating = false;
	}
	else if (typeName == "limitedhungryreturncomboer")
	{
		sprite.setColor(Color::Green);
		returnsToPlayer = true;
		maxWaitFrames = 360;
		limitedEating = true;
		//not implemented, not sure if want
	}
	

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

	gravFactor = 1.0;

	maxFallSpeed = 15;

	ts = sess->GetSizedTileset("Enemies/Comboer_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	origSize = 48;

	BasicCircleHurtBodySetup(origSize);
	BasicCircleHitBodySetup(origSize);

	hitBody.hitboxInfo = hitboxInfo;

	comboObj = new ComboObject(this);
	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	comboObj->enemyHitBody.BasicCircleSetup(origSize, GetPosition());
	comboObj->enemyHitboxFrame = 0;

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

	ResetEnemy();
}

HungryComboer::~HungryComboer()
{
}
void HungryComboer::ResetEnemy()
{
	sprite.setRotation(0);

	growthLevel = 0;

	scale = origScale;
	UpdateScale();

	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;

	velocity = V2d(0, 0);
	
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	action = S_FLOAT;
	frame = 0;

	
	numEaten = 0;

	UpdateEatenNumberText(maxEdible);

	chaseTarget = NULL;

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
		numEatenText.setString(to_string(reps));
		numEatenText.setOrigin(numEatenText.getLocalBounds().left
			+ numEatenText.getLocalBounds().width / 2,
			numEatenText.getLocalBounds().top
			+ numEatenText.getLocalBounds().height / 2);
	}
}

void HungryComboer::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	velocity = vel;
}

void HungryComboer::Throw(V2d vel)
{
	velocity = vel;
}

void HungryComboer::Return()
{
	sess->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	scale = origScale;
	UpdateScale();
	UpdateEatenNumberText(0);

	numEaten = 0;

	numHealth = maxHealth;
}

void HungryComboer::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;
}

void HungryComboer::PopThrow()
{
	V2d dir;

	dir = Get8Dir(receivedHit->hDir);

	Pop();

	Throw(dir * GetFlySpeed());

	sess->PlayerAddActiveComboObj(comboObj);
}

void HungryComboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		Actor *player = sess->GetPlayer(0);
		if (numHealth <= 0)
		{
			action = S_FLY;
			chaseTarget = NULL;
			frame = 0;
			PopThrow();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
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
			DefaultHurtboxesOn();
			break;
		case S_RETURN:
			UpdateEatenNumberText(maxEdible);
			SetCurrPosInfo(startPosInfo);
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			break;
		}
	}


	if (action == S_TRACKENEMY)
	{
		if (chaseTarget != NULL && chaseTarget->dead)
		{
			chaseTarget = NULL;
		}

		//if (chaseTarget == NULL)
		{
			Enemy *foundEnemy = NULL;
			int foundIndex = 0;
			if (GetClosestEnemyPos(GetPosition(), 1000, foundEnemy, foundIndex))
			{
				chaseTarget = foundEnemy;
				chaseIndex = foundIndex;
			}
		}
	}

	if ((action == S_TRACKENEMY && chaseTarget != NULL) || action == S_TRACKPLAYER)
	{
		double accel = 1;// .5;//.5;
		V2d trackPos = GetTrackPos();
		V2d trackDir = normalize(trackPos - GetPosition());
		velocity += trackDir * accel;
		double fSpeed = GetFlySpeed();
		if (length(velocity) > fSpeed)
		{
			velocity = normalize(velocity) * fSpeed;
		}
	}
}

double HungryComboer::GetFlySpeed()
{
	double fSpeed = flySpeed;
	fSpeed -= growthLevel * 1.0;
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

void HungryComboer::ComboKill(Enemy *e)
{
	if (chaseTarget == e)
	{
		chaseTarget = NULL;
	}

	if (returnsToPlayer)
	{
		action = S_TRACKPLAYER;
		frame = 0;
	}

	++numEaten;

	if (limitedEating && numEaten == maxEdible)
	{
		if (hasMonitor && !suppressMonitor)
		{
			sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
				sess->GetPlayer(0), 1, GetPosition());
			suppressMonitor = true;
		}

		action = S_RETURN;
		frame = 0;

		Return();

		return;
	}
	UpdateEatenNumberText(maxEdible - numEaten);
	
	if (growthLevel < numGrowthLevels)
	{
		growthLevel++;
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


V2d HungryComboer::GetTrackPos()
{
	if (action == S_TRACKENEMY)
	{
		if (chaseTarget != NULL)
		{
			return chaseTarget->GetCamPoint(chaseIndex);
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
	V2d movementVec = velocity;
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

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
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
	if (action == S_FLY || action == S_TRACKENEMY || action == S_TRACKPLAYER)
	{
		if (waitFrame == maxWaitFrames)
		{
			action = S_RETURN;
			frame = 0;
			Return();
		}
		else
		{
			waitFrame++;
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
	++currHits;
	waitFrame = 0;
	if (hitLimit > 0 && currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
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
		sprite.setTextureRect(ts->GetSubRect(tile));
		break;
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
	DrawSprite(target, sprite);

	if (limitedEating)
	{
		target->draw(numEatenText);
	}
}