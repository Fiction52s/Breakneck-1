#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GroundedGrindJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "MainMenu.h"
#include "AbsorbParticles.h"

using namespace std;
using namespace sf;

void GroundedGrindJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedJuggles)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		juggleReps = jParams->numJuggles;
		UpdateJuggleRepsText(juggleReps);
	}
}

GroundedGrindJuggler::GroundedGrindJuggler(ActorParams *ap)
	:Enemy(EnemyType::EN_GROUNDEDGRINDJUGGLER, ap )
{
	SetNumActions(S_Count);
	SetEditorActions(S_IDLE, S_IDLE, 0);

	SetLevel(ap->GetLevel());

	const string &typeName = ap->GetTypeName();
	if (typeName == "groundedgrindjugglercw")
	{
		limitedJuggles = false;
		clockwise = true;
	}
	else if (typeName == "limitedgroundedgrindjugglercw")
	{
		limitedJuggles = true;
		clockwise = true;
	}
	else if (typeName == "groundedgrindjugglerccw")
	{
		limitedJuggles = false;
		clockwise = false;
	}
	else if (typeName == "limitedgroundedgrindjugglerccw")
	{
		limitedJuggles = true;
		clockwise = false;
	}

	numJugglesText.setFont(sess->mainMenu->arial);
	numJugglesText.setFillColor(Color::White);
	numJugglesText.setOutlineColor(Color::Black);
	numJugglesText.setOutlineThickness(3);
	numJugglesText.setCharacterSize(32);

	friction = .4;

	pushStart = 20;

	maxWaitFrames = 180;

	CreateSurfaceMover(startPosInfo, 10, this);

	UpdateParamsSettings();

	hitLimit = -1;

	action = S_IDLE;

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

	BasicCircleHitBodySetup(48);
	BasicCircleHurtBodySetup(48);

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

	actionLength[S_IDLE] = 18;
	actionLength[S_GRIND] = 20;
	actionLength[S_SLOW] = 10;
	actionLength[S_STOPPED] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_IDLE] = 2;
	animFactor[S_GRIND] = 1;
	animFactor[S_SLOW] = 1;
	animFactor[S_STOPPED] = 1;
	animFactor[S_RETURN] = 6;

	ResetEnemy();
}

GroundedGrindJuggler::~GroundedGrindJuggler()
{
}

void GroundedGrindJuggler::SetLevel(int lev)
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

void GroundedGrindJuggler::UpdateJuggleRepsText(int reps)
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

void GroundedGrindJuggler::ResetEnemy()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	currHits = 0;
	comboObj->Reset();
	velocity = V2d(0, 0);

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	action = S_IDLE;
	frame = 0;

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	UpdateHitboxes();

	currJuggle = 0;

	UpdateJuggleRepsText(juggleReps);

	UpdateSprite();
}

void GroundedGrindJuggler::Push(double strength)
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;

	sess->PlayerAddActiveComboObj(comboObj);

	if (clockwise)
	{
		surfaceMover->SetSpeed(strength);
	}
	else
	{
		surfaceMover->SetSpeed(-strength);
	}

	UpdateJuggleRepsText(juggleReps - currJuggle);
	
}

void GroundedGrindJuggler::Return()
{
	sess->PlayerRemoveActiveComboer(comboObj);

	HitboxesOff();
	HurtboxesOff();

	currJuggle = 0;

	UpdateJuggleRepsText(0);

	numHealth = maxHealth;
}

void GroundedGrindJuggler::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

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
				action = S_GRIND;
				frame = 0;
				Push(pushStart);
			}
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
	}
}

void GroundedGrindJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			UpdateJuggleRepsText(juggleReps);
			surfaceMover->Set(startPosInfo);
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			break;
		case S_GRIND:
			action = S_SLOW;
			DefaultHurtboxesOn();
			break;
		}
	}

	


	/*if (action != S_FLOAT && action != S_EXPLODE && action != S_RETURN)
	{
	sf::Rect<double> r(position.x - 50, position.y - 50, 100, 100);
	owner->activeEnemyItemTree->Query(this, r);
	}*/


	/*if (action == S_POP && ((velocity.y >= 0 && !reversedGrav) || (velocity.y <= 0 && reversedGrav)))
	{
	action = S_JUGGLE;
	frame = 0;
	SetHurtboxes(hurtBody, 0);
	}*/
}

void GroundedGrindJuggler::HandleNoHealth()
{

}

void GroundedGrindJuggler::Move()
{
	surfaceMover->velocity = velocity;
	surfaceMover->Move(slowMultiple, numPhysSteps);
}

void GroundedGrindJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_GRIND:
	case S_SLOW:
	{
		Move();
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
}

void GroundedGrindJuggler::FrameIncrement()
{
	if (action == S_SLOW)
	{
		double speed = surfaceMover->groundSpeed;
		if (speed > 0)
		{
			speed -= friction;
			if (speed < 0)
				speed = 0;
		}
		else if( speed < 0 )
		{
			speed += friction;
			if (speed > 0)
				speed = 0;
		}

		if (speed == 0)
		{
			action = S_STOPPED;
		}

		surfaceMover->SetSpeed(speed);
	}


	if (action == S_SLOW || action == S_GRIND || action == S_STOPPED )
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


void GroundedGrindJuggler::ComboKill(Enemy *e)
{
	/*if (action == S_GRIND)
	{
		action = S_FLY;
		frame = 0;

		mover->SetSpeed(0);

		velocity = mover->ground->Normal() * flySpeed;
		mover->ground = NULL;


		SetHurtboxes(hurtBody, 0);

		owner->PlayerRemoveActiveComboer(comboObj);
	}*/
}

void GroundedGrindJuggler::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (hitLimit > 0 && currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
		Return();
	}
	else
	{
		/*if (action == S_GRIND)
		{
		action = S_FLY;
		frame = 0;

		mover->SetSpeed(0);

		velocity = mover->ground->Normal() * flySpeed;
		mover->ground = NULL;


		SetHurtboxes(hurtBody, 0);

		owner->PlayerRemoveActiveComboer(comboObj);

		}*/
	}
}

void GroundedGrindJuggler::UpdateSprite()
{
	

	int tile = 0;
	switch (action)
	{
	case S_IDLE:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(7));

	if (action == S_SLOW)
	{
		if (clockwise)
		{
			//sprite.setColor(Color::Red);
		}
		else
		{
			//sprite.setColor(Color::Cyan);
		}
		
	}
	else if (action == S_STOPPED)
	{
		if (clockwise)
		{
		//	sprite.setColor(Color::Yellow);
		}
		else
		{
			//sprite.setColor(Color::Cyan);
		}
		
	}
	else
	{
		if (clockwise)
		{
			//sprite.setColor(Color::White);
		}
		else
		{
			//sprite.setColor(Color::Green);
		}
		
	}

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	if (limitedJuggles)
	{
		numJugglesText.setPosition(sprite.getPosition());
	}
}

void GroundedGrindJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedJuggles)
	{
		target->draw(numJugglesText);
	}
}