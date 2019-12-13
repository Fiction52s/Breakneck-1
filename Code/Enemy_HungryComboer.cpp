#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_HungryComboer.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

HungryComboer::HungryComboer(GameSession *owner, bool p_hasMonitor, Vector2i pos,
	int p_level, int jReps, bool p_returns)
	:Enemy(owner, EnemyType::EN_HUNGRYCOMBOER, p_hasMonitor, 1, false)
{
	level = p_level;

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

	origScale = scale;

	returnsToPlayer = p_returns;

	maxWaitFrames = 180;

	flySpeed = 12;

	numGrowthLevels = 4;
	
	juggleReps = jReps;

	position.x = pos.x;
	position.y = pos.y;
	origPos = position;
	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	hitLimit = -1;

	action = S_FLOAT;

	gravFactor = 1.0;

	if (returnsToPlayer)
	{
		sprite.setColor(Color::Green);
	}
	else
	{
		//sprite.setColor(Color::Red);
	}
	maxFallSpeed = 15;


	ts = owner->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	origSize = 48;

	SetupBodies(1, 1);
	AddBasicHurtCircle(origSize);
	AddBasicHitCircle(origSize);
	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

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

	comboObj->enemyHitBody = new CollisionBody(1);
	comboObj->enemyHitBody->AddCollisionBox(0, hitBody->GetCollisionBoxes(0)->front());
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
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);

	growthLevel = 0;

	scale = origScale;
	UpdateScale();

	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	velocity = V2d(0, 0);
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	dead = false;
	action = S_FLOAT;
	frame = 0;
	receivedHit = NULL;
	position = origPos;
	UpdateHitboxes();

	currJuggle = 0;
	chaseTarget = NULL;

	UpdateSprite();
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
	owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	scale = origScale;
	UpdateScale();

	currJuggle = 0;

	numHealth = maxHealth;
}

void HungryComboer::Pop()
{
	owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
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

	owner->GetPlayer(0)->AddActiveComboObj(comboObj);
}

void HungryComboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		Actor *player = owner->GetPlayer(0);
		if (numHealth <= 0)
		{
			if (currJuggle == juggleReps)
			{
				if (hasMonitor && !suppressMonitor)
				{
					owner->keyMarker->CollectKey();
					suppressMonitor = true;
				}

				player->ConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

				action = S_RETURN;
				frame = 0;

				Return();
			}
			else
			{
				action = S_FLY;
				chaseTarget = NULL;
				frame = 0;
				PopThrow();
			}
		}
		else
		{
			owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
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
			SetHurtboxes(hurtBody, 0);
			break;
		case S_RETURN:
			position = origPos;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
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
			if (GetClosestEnemyPos(owner, position, 1000, foundEnemy, foundIndex))
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
		V2d trackDir = normalize(trackPos - position);
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
		return owner->GetPlayer(0)->position;
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

	position += movementVec;

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
}

void HungryComboer::UpdateScale()
{
	sprite.setScale(scale, scale);
	hitBody->GetCollisionBoxes(0)->front().rw = scale * origSize;
	hitBody->GetCollisionBoxes(0)->front().rh = scale * origSize;
	hurtBody->GetCollisionBoxes(0)->front().rw = scale * origSize;
	hurtBody->GetCollisionBoxes(0)->front().rh = scale * origSize;
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

void HungryComboer::HandleEntrant(QuadTreeEntrant *qte)
{
	Enemy *en = (Enemy*)qte;
	if (en->type == EnemyType::EN_JUGGLERCATCHER)
	{
		JugglerCatcher *catcher = (JugglerCatcher*)qte;

		CollisionBox &hitB = hurtBody->GetCollisionBoxes(0)->front();
		if (catcher->CanCatch() && catcher->hurtBody->Intersects(catcher->currHurtboxFrame, &hitB))
		{
			//catcher->Catch();
			//action = S_EXPLODE;
			//frame = 0;
		}
	}
}

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
	sprite.setPosition(position.x, position.y);

	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}
}

void HungryComboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

CollisionBox &HungryComboer::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void HungryComboer::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	GetEnemyHitbox().globalPosition = position;

	if (owner->GetPlayer(0)->ground != NULL)
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->groundSpeed * (owner->GetPlayer(0)->ground->v1 - owner->GetPlayer(0)->ground->v0));
	}
	else
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->velocity);
	}

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
}