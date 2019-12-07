#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_RelativeComboer.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

RelativeComboer::RelativeComboer(GameSession *owner, bool p_hasMonitor, Vector2i pos,
	list<Vector2i> &path, int p_level, int jReps, bool p_detachOnKill)
	:Enemy(owner, EnemyType::EN_RELATIVECOMBOER, p_hasMonitor, 1, false)
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

	detachOnKill = p_detachOnKill;

	maxWaitFrames = 180;

	flySpeed = 5;

	juggleReps = path.size();

	guidedDir = NULL;
	if (juggleReps > 0)
	{
		Vector2i prev(0, 0);
		guidedDir = new V2d[juggleReps];
		int ind = 0;
		for (auto it = path.begin(); it != path.end(); ++it)
		{
			if (it != path.begin())
			{
				it--;
				prev = (*it);
				it++;
			}

			guidedDir[ind] = normalize(V2d((*it) - prev));

			++ind;
		}
	}

	if (guidedDir == NULL)
		juggleReps = jReps;


	position.x = pos.x;
	position.y = pos.y;
	origPos = position;
	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	hitLimit = -1;

	action = S_FLOAT;

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

	SetupBodies(1, 1);
	AddBasicHurtCircle(48);
	AddBasicHitCircle(48);
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

	maxLatchFrames = 20 * 60;

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 30;
	actionLength[S_ATTACHEDWAIT] = 10;
	actionLength[S_WAIT] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_ATTACHEDWAIT] = 1;
	animFactor[S_WAIT] = 1;
	animFactor[S_RETURN] = 6;

	ResetEnemy();
}

RelativeComboer::~RelativeComboer()
{
	if (guidedDir != NULL)
		delete[] guidedDir;
}

void RelativeComboer::ResetEnemy()
{
	latchFrame = 0;
	
	sprite.setColor(Color::White);
	specialPauseFrames = 0;
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
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
	
	UpdateHitboxes();

	basePos = origPos;
	latchedOn = false;
	position = origPos;

	currJuggle = 0;

	UpdateSprite();
}

void RelativeComboer::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	velocity = vel;
}

void RelativeComboer::Throw(V2d vel)
{
	velocity = vel;
}

void RelativeComboer::Return()
{
	owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	currJuggle = 0;

	numHealth = maxHealth;

	sprite.setColor(Color::White);
}

void RelativeComboer::Pop()
{
	owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;
}

void RelativeComboer::PopThrow()
{
	V2d dir;

	if (guidedDir == NULL)
	{
		dir = Get8Dir(receivedHit->hDir);
	}
	else
	{
		dir = guidedDir[currJuggle];
	}	

	Pop();

	Throw(dir * flySpeed);

	owner->GetPlayer(0)->AddActiveComboObj(comboObj);
}

bool RelativeComboer::CanBeHitByComboer()
{
	return false;
}

void RelativeComboer::ProcessHit()
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
				frame = 0;
				latchedOn = true;
				latchFrame = 0;
				offsetPos = position - player->position;
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

void RelativeComboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_FLY:
			action = S_ATTACHEDWAIT;
			sprite.setColor(Color::Red);
			break;
		case S_RETURN:
			position = origPos;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			latchedOn = false;
			//basePos = origPos;
			break;
		}
	}
}

void RelativeComboer::HandleNoHealth()
{

}

void RelativeComboer::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = velocity;
	movementVec /= slowMultiple * numStep;

	offsetPos += movementVec;
}

void RelativeComboer::UpdateEnemyPhysics()
{
	if (latchedOn)
	{
		basePos = owner->GetPlayer(0)->position;// + offsetPlayer;
	}
	else
	{


	}

	if (specialPauseFrames == 0)
	{


		switch (action)
		{
		case S_FLY:
		{
			Move();
			break;
		}
		}

	}

	if (latchedOn)
	{
		position = offsetPos + basePos;
	}
}

void RelativeComboer::FrameIncrement()
{
	if (specialPauseFrames > 0)
	{
		--specialPauseFrames;
	}
	if (action == S_FLY )
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

	if (latchedOn)
	{
		latchFrame++;
		if (latchFrame == maxLatchFrames)
		{
			action = S_RETURN;
			frame = 0;
			Return();
			return;
		}
	}

}

void RelativeComboer::ComboKill(Enemy *e)
{
	//assert(action == S_FLY);
	action = S_WAIT;
	frame = 0;
	latchedOn = false;
	SetHurtboxes(hurtBody, 0);
	sprite.setColor(Color::Blue);
	owner->GetPlayer(0)->hasDoubleJump = true;
	owner->GetPlayer(0)->hasAirDash = true;
}

void RelativeComboer::HandleEntrant(QuadTreeEntrant *qte)
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

void RelativeComboer::ComboHit()
{
	//pauseFrames = 5;
	specialPauseFrames = 5;
	++currHits;
	velocity = V2d(0, 0);
	if (hitLimit > 0 && currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
		Return();
	}
}

void RelativeComboer::UpdateSprite()
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

void RelativeComboer::EnemyDraw(sf::RenderTarget *target)
{
	bool b = (owner->pauseFrames < 2 && ( pauseFrames < 2 && specialPauseFrames < 2 )) || (receivedHit == NULL && ( pauseFrames < 2 && specialPauseFrames < 2));
	if (hasMonitor && !suppressMonitor)
	{
		if (b)
		{
			target->draw(sprite, keyShader);
		}
		else
		{
			target->draw(sprite, hurtShader);
		}
		target->draw(*keySprite);
	}
	else
	{
		cout << "specialPauseFrames: " << specialPauseFrames << ", b: " << (int)b << endl;
		if (b)
		{
			target->draw(sprite);
		}
		else
		{
			target->draw(sprite, hurtShader);
		}
	}
}

CollisionBox &RelativeComboer::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void RelativeComboer::UpdateHitboxes()
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