#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_AttractJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "MainMenu.h"
#include "AbsorbParticles.h"
#include "Actor.h"
#include "Wire.h"


using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void AttractJuggler::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedJuggles)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		juggleReps = jParams->numJuggles;
		UpdateJuggleRepsText(juggleReps);
	}
}

void AttractJuggler::SetLevel(int lev)
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

AttractJuggler::AttractJuggler(ActorParams *ap)
	:Enemy(EnemyType::EN_ATTRACTJUGGLER, ap)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_POP] = 60;
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
	if (typeName == "attractjuggler")
	{
		limitedJuggles = false;
	}
	else if (typeName == "limitedattractjuggler")
	{
		limitedJuggles = true;
	}

	UpdateParamsSettings();

	maxWaitFrames = 180;

	gravFactor = 1.4;

	//sprite.setColor(Color::Red);
	maxFallSpeed = 25;

	reversed = true;

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(0));

	sprite.setScale(scale, scale);


	BasicCircleHurtBodySetup(48);

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

	ResetEnemy();
}

AttractJuggler::~AttractJuggler()
{
}

void AttractJuggler::UpdateJuggleRepsText(int reps)
{
	if (limitedJuggles)
	{
		data.juggleTextNumber = reps;
		numJugglesText.setString(to_string(reps));
		numJugglesText.setOrigin(numJugglesText.getLocalBounds().left
			+ numJugglesText.getLocalBounds().width / 2,
			numJugglesText.getLocalBounds().top
			+ numJugglesText.getLocalBounds().height / 2);
	}
}

void AttractJuggler::ResetEnemy()
{
	hitLimit = -1;
	action = S_FLOAT;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	data.currHits = 0;
	comboObj->Reset();
	data.velocity = V2d(0, 0);

	DefaultHurtboxesOn();
	//DefaultHitboxesOn();

	dead = false;
	action = S_FLOAT;
	frame = 0;
	receivedHit.SetEmpty();
	UpdateHitboxes();

	data.currJuggle = 0;
	data.waitFrame = 0;

	UpdateJuggleRepsText(juggleReps);

	UpdateSprite();
}

void AttractJuggler::HandleWireHit(Wire *w)
{
	w->HitEnemy(GetPosition());
	w->player->RestoreAirOptions();
}

bool AttractJuggler::CanBeHitByWireTip(bool red)
{
	return false; //true activates ability to be hit by wire
}

bool AttractJuggler::CanBeAnchoredByWire(bool red)
{
	return false;
}

void AttractJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	data.velocity = vel;
}

void AttractJuggler::Throw(V2d vel)
{
	data.velocity = vel;
}

void AttractJuggler::Return()
{
	action = S_RETURN;
	frame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	data.currJuggle = 0;

	UpdateJuggleRepsText(0);

	numHealth = maxHealth;
}

void AttractJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++data.currJuggle;

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	data.waitFrame = 0;

	UpdateJuggleRepsText(juggleReps - data.currJuggle);
}

void AttractJuggler::PopThrow()
{
	V2d dir;

	dir = receivedHit.hDir;

	V2d hit = 20.0 * dir;

	Pop();

	Throw(hit);

	sess->PlayerAddActiveComboObj(comboObj);
}

void AttractJuggler::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		//Actor *player = owner->GetPlayer(0);
		if (numHealth <= 0)
		{
			if (limitedJuggles && data.currJuggle == juggleReps - 1)
			{
				if (hasMonitor && !suppressMonitor)
				{
					sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
						sess->GetPlayer(0), 1, GetPosition());
					suppressMonitor = true;
				}

				sess->PlayerConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

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

void AttractJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_POP:
			action = S_JUGGLE;
			frame = 0;
			DefaultHurtboxesOn();
			break;
		case S_RETURN:
			UpdateJuggleRepsText(juggleReps);
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
			//DefaultHitboxesOn();
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
		}
	}
}

void AttractJuggler::HandleNoHealth()
{

}

void AttractJuggler::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * numStep;

	currPosInfo.position += movementVec;

	V2d gDir = normalize( sess->GetPlayerPos(0) - GetPosition() );

	data.velocity += gDir * (gravFactor / numStep / slowMultiple);

	if (length(data.velocity) > maxFallSpeed)
	{
		data.velocity = normalize(data.velocity) * maxFallSpeed;
	}
}

void AttractJuggler::UpdateEnemyPhysics()
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

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void AttractJuggler::FrameIncrement()
{
	if (action == S_POP || action == S_JUGGLE)
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

void AttractJuggler::ComboHit()
{
	pauseFrames = 5;
	++data.currHits;
	if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		Return();
	}
}

void AttractJuggler::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(15));


	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	if (limitedJuggles)
	{
		numJugglesText.setPosition(sprite.getPosition());
	}
}

void AttractJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedJuggles)
	{
		target->draw(numJugglesText);
	}
}

int AttractJuggler::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void AttractJuggler::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void AttractJuggler::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	UpdateJuggleRepsText(data.juggleTextNumber);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}