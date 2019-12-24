#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_WireJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "Wire.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

WireJuggler::WireJuggler(GameSession *owner, bool p_hasMonitor, Vector2i pos,
	list<Vector2i> &path, int p_level, int jReps, JuggleType juggleType )
	:Enemy(owner, EnemyType::EN_WIREJUGGLER, p_hasMonitor, 1, false)
{
	jType = juggleType;

	level = p_level;

	switch (level)
	{
	case 1:
		scale = 2.0;
		break;
	case 2:
		scale = 3.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 4.0;
		maxHealth += 5;
		break;
	}

	reversedGrav = false;

	maxWaitFrames = 180;

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

	gravFactor = 1.0;

	gDir = V2d(0, 1);
	
	switch (jType)
	{
	case T_WHITE:
		break;
	case T_BLUE:
		sprite.setColor(Color::Blue);
		break;
	case T_RED:
		sprite.setColor(Color::Red);
		break;
	case T_MAGENTA:
		sprite.setColor(Color::Magenta);
		break;
	}
	maxFallSpeed = 15;

	reversed = true;

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

	actionLength[S_FLOAT] = 18;
	actionLength[S_POP] = 10;
	actionLength[S_JUGGLE] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_POP] = 1;
	animFactor[S_JUGGLE] = 1;
	animFactor[S_RETURN] = 6;

	ResetEnemy();
}

WireJuggler::~WireJuggler()
{
	if (guidedDir != NULL)
		delete[] guidedDir;
}

void WireJuggler::ResetEnemy()
{
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
	position = origPos;
	UpdateHitboxes();

	currJuggle = 0;

	UpdateSprite();
}

bool WireJuggler::CanBeAnchoredByWire(bool red)
{
	switch (jType)
	{
	case T_WHITE:
		return true;
	case T_BLUE:
		return red;
	case T_RED:
		return !red;
	case T_MAGENTA:
		return true;
	}
}

void WireJuggler::HandleWireHit(Wire *w)
{
	w->HitEnemy(position);
	w->player->RestoreAirDash();
	w->player->RestoreDoubleJump();
}

void WireJuggler::HandleWireUnanchored(Wire *w)
{
	action = S_JUGGLE;
	frame = 0;
}

void WireJuggler::HandleWireAnchored(Wire *w)
{
	action = S_HELD;
	velocity = V2d(0, 0);
	frame = 0;
}

void WireJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	velocity = vel;
}

void WireJuggler::Throw(V2d vel)
{
	velocity = vel;
}

void WireJuggler::Return()
{
	owner->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	currJuggle = 0;

	numHealth = maxHealth;
}

void WireJuggler::Pop()
{
	owner->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	//SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;
}

void WireJuggler::PopThrow()
{
	V2d dir;

	if (guidedDir == NULL)
	{
		dir = receivedHit->hDir;//normalize(receivedHit->hDir);
	}
	else
	{
		dir = guidedDir[currJuggle];
	}


	V2d hit(0, -20);

	double extraX = 8;

	if (dir.x != 0)
	{
		hit.x += dir.x * extraX;
	}

	bool pFacingRight = owner->PlayerIsFacingRight(0);

	if (((dir.y == 1 && !reversedGrav) || (dir.y == -1 && reversedGrav))
		&& dir.x == 0)
	{
		hit.y += 3;
		if (pFacingRight)
		{
			hit.x -= extraX / 2.0;
		}
		else
		{
			hit.x += extraX / 2.0;
		}

	}

	if (reversedGrav)
	{
		hit.y = -hit.y;
	}

	Pop();

	Throw(hit);

	owner->PlayerAddActiveComboObj(comboObj);
}

bool WireJuggler::CanBeHitByWireTip(bool red)
{
	switch (jType)
	{
	case T_WHITE:
		return true;
	case T_BLUE:
		return !red;
	case T_RED:
		return red;
	case T_MAGENTA:
		return true;
	}
}

bool WireJuggler::CanBeHitByComboer()
{
	return false;
}

void WireJuggler::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		if (receivedHit->hType == HitboxInfo::WIREHITRED)
		{
			numHealth -= 1;
		}
		else if (receivedHit->hType == HitboxInfo::WIREHITBLUE)
		{
			numHealth -= 1;
		}

		if (numHealth <= 0)
		{
			if (currJuggle == juggleReps)
			{
				if (hasMonitor && !suppressMonitor)
				{
					owner->keyMarker->CollectKey();
					suppressMonitor = true;
				}

				owner->PlayerConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

				action = S_RETURN;
				frame = 0;

				Return();
			}
			else
			{
				action = S_POP;
				frame = 0;
				PopThrow();
			}
		}
		else
		{
			owner->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
	}
}

void WireJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			position = origPos;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
		}
	}

	if (action == S_POP && ((velocity.y >= 0 && !reversedGrav) || (velocity.y <= 0 && reversedGrav)))
	{
		action = S_JUGGLE;
		frame = 0;
		SetHurtboxes(hurtBody, 0);
	}
}

void WireJuggler::HandleNoHealth()
{

}

void WireJuggler::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = velocity;
	movementVec /= slowMultiple * numStep;

	position += movementVec;

	velocity += gDir * (gravFactor / numStep / slowMultiple);

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
	}
}

void WireJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_POP:
	case S_JUGGLE:
	{
		Move();
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
}

void WireJuggler::FrameIncrement()
{
	if (action == S_POP || action == S_JUGGLE)
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

void WireJuggler::HandleEntrant(QuadTreeEntrant *qte)
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

void WireJuggler::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (hitLimit > 0 && currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
		Return();
	}
}

void WireJuggler::UpdateSprite()
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

void WireJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}