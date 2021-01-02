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
#include "MainMenu.h"
#include "AbsorbParticles.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void WireJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedJuggles)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		juggleReps = jParams->numJuggles;
		UpdateJuggleRepsText(juggleReps);
	}
}

WireJuggler::WireJuggler( ActorParams *ap )
	:Enemy(EnemyType::EN_WIREJUGGLER, ap )
{
	//jType = juggleType;

	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_POP] = 10;
	actionLength[S_JUGGLE] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_POP] = 1;
	animFactor[S_JUGGLE] = 1;
	animFactor[S_RETURN] = 6;

	SetLevel(ap->GetLevel());

	numJugglesText.setFont(sess->mainMenu->arial);
	numJugglesText.setFillColor(Color::White);
	numJugglesText.setOutlineColor(Color::Black);
	numJugglesText.setOutlineThickness(3);
	numJugglesText.setCharacterSize(32);

	const string &typeName = ap->GetTypeName();

	if (typeName == "bluewirejuggler")
	{
		jType = T_BLUE;//testing
		limitedJuggles = false;
	}
	else if (typeName == "limitedbluewirejuggler")
	{
		jType = T_BLUE; //testing
		limitedJuggles = true;
	}
	else if (typeName == "redwirejuggler")
	{
		jType = T_RED;
		limitedJuggles = false;
	}
	else if (typeName == "limitedredwirejuggler")
	{
		jType = T_RED; //testing
		limitedJuggles = true;
	}
	else if (typeName == "magentawirejuggler")
	{
		jType = T_MAGENTA;
		limitedJuggles = false;
	}
	else if (typeName == "limitedmagentawirejuggler")
	{
		jType = T_MAGENTA; //testing
		limitedJuggles = true;
	}
	else
	{
		cout << "invalid typename: " << typeName << endl;
		assert(0);
	}

	UpdateParamsSettings();

	reversedGrav = false;

	maxWaitFrames = 180;

	hitLimit = -1;

	gravFactor = 1.0;

	gDir = V2d(0, 1);
	
	switch (jType)
	{
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

	ts = sess->GetSizedTileset("Enemies/Comboer_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);

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

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());
	comboObj->enemyHitboxFrame = 0;

	ResetEnemy();
}

WireJuggler::~WireJuggler()
{
}

void WireJuggler::UpdateJuggleRepsText(int reps)
{
	if (limitedJuggles)
	{
		numJugglesText.setString(to_string(reps));
		numJugglesText.setOrigin(numJugglesText.getLocalBounds().left
			+ numJugglesText.getLocalBounds().width / 2,
			numJugglesText.getLocalBounds().top
			+ numJugglesText.getLocalBounds().height / 2);
	}
}

void WireJuggler::SetLevel(int lev)
{
	level = lev;

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
}

void WireJuggler::ResetEnemy()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);

	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	velocity = V2d(0, 0);


	DefaultHitboxesOn();
	DefaultHurtboxesOn();
	dead = false;
	action = S_FLOAT;
	frame = 0;

	UpdateHitboxes();

	currJuggle = 0;

	UpdateSprite();
}

bool WireJuggler::CanBeAnchoredByWire(bool red)
{
	if ((red && jType == T_BLUE) || (!red && jType == T_RED))
	{
		return true;
	}

	return false;
}

void WireJuggler::HandleWireHit(Wire *w)
{
	w->HitEnemy(GetPosition());
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
	sess->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	currJuggle = 0;

	UpdateJuggleRepsText(0);

	numHealth = maxHealth;
}

void WireJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
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

	dir = receivedHit->hDir;//normalize(receivedHit->hDir);

	V2d hit(0, -20);

	double extraX = 8;

	if (dir.x != 0)
	{
		hit.x += dir.x * extraX;
	}

	bool pFacingRight = sess->PlayerIsFacingRight(0);

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

	sess->PlayerAddActiveComboObj(comboObj);
}

bool WireJuggler::CanBeHitByWireTip(bool red)
{
	switch (jType)
	{
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
			if ( limitedJuggles && currJuggle == juggleReps - 1)
			{
				if (hasMonitor && !suppressMonitor)
				{
					sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
						sess->GetPlayer(0), 1, GetPosition());
					suppressMonitor = true;
				}

				sess->PlayerConfirmEnemyNoKill(this);
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
			sess->PlayerConfirmEnemyNoKill(this);
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
			UpdateJuggleRepsText(juggleReps);
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
			DefaultHitboxesOn();
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
		DefaultHurtboxesOn();
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

	currPosInfo.position += movementVec;

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

	if (limitedJuggles)
	{
		numJugglesText.setPosition(sprite.getPosition());
	}
}

void WireJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedJuggles)
	{
		target->draw(numJugglesText);
	}
}