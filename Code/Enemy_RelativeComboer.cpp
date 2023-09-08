#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_RelativeComboer.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "MainMenu.h"
#include "AbsorbParticles.h"
#include "Actor.h"

using namespace std;
using namespace sf;

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void RelativeComboer::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();
	if (limitedKills)
	{
		JugglerParams *jParams = (JugglerParams*)editParams;
		maxKilled = jParams->numJuggles;
		UpdateKilledNumberText(maxKilled);
	}
}

RelativeComboer::RelativeComboer(ActorParams *ap )
	:Enemy(EnemyType::EN_RELATIVECOMBOER, ap )
{
	enemyDrawLayer = ENEMYDRAWLAYER_COMBOER;

	SetNumActions(S_Count);
	SetEditorActions(S_FLY, S_FLY, 0);

	SetLevel(ap->GetLevel());

	string typeName = ap->GetTypeName();

	if (typeName == "relativecomboer")
	{
		limitedKills = false;
		detachOnKill = false;
	}
	else if (typeName == "relativecomboerdetach")
	{
		detachOnKill = true;
		limitedKills = false;
	}
	

	numKilledText.setFont(sess->mainMenu->arial);
	numKilledText.setFillColor(Color::White);
	numKilledText.setOutlineColor(Color::Black);
	numKilledText.setOutlineThickness(3);
	numKilledText.setCharacterSize(32);

	UpdateParamsSettings();

	maxWaitFrames = 180;

	flySpeed = 5;

	hitLimit = -1;

	lineMovement = flySeq.AddLineMovement(V2d(), V2d(), CubicBezier(), 30);
	flySeq.InitMovementDebug();

	

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	BasicCircleHurtBodySetup(48);

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

	comboObj->enemyHitBody.BasicCircleSetup(48, GetPosition());

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
}

void RelativeComboer::ResetEnemy()
{
	data.latchFrame = 0;


	flySeq.Reset();
	
	//sprite.setColor(Color::White);
	data.specialPauseFrames = 0;
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	data.currHits = 0;
	comboObj->Reset();
	data.velocity = V2d(0, 0);
	DefaultHurtboxesOn();
	action = S_FLOAT;
	frame = 0;
	
	data.basePos = startPosInfo.position;
	data.latchedOn = false;

	data.numKilled = 0;

	UpdateKilledNumberText(maxKilled);

	UpdateHitboxes();
	UpdateSprite();
}

void RelativeComboer::SetLevel(int lev)
{

}

void RelativeComboer::UpdateKilledNumberText(int reps)
{
	if (limitedKills)
	{
		data.numKilledTextNumber = reps;
		numKilledText.setString(to_string(reps));
		numKilledText.setOrigin(numKilledText.getLocalBounds().left
			+ numKilledText.getLocalBounds().width / 2,
			numKilledText.getLocalBounds().top
			+ numKilledText.getLocalBounds().height / 2);
	}
}

void RelativeComboer::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	data.velocity = vel;
}

void RelativeComboer::Throw(V2d vel)
{
	data.velocity = vel;
}

void RelativeComboer::Return()
{
	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();
	//SetHitboxes(NULL, 0);

	data.numKilled = 0;

	numHealth = maxHealth;

	//sprite.setColor(Color::White);
}

void RelativeComboer::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	HurtboxesOff();
	data.waitFrame = 0;
}

void RelativeComboer::PopThrow()
{
	action = S_FLY;
	frame = 0;
	data.latchedOn = true;
	data.latchFrame = 0;
	data.offsetPos = GetPosition() - sess->GetPlayerPos(0);
	

	V2d dir;

	dir = receivedHit.hDir;

	data.offsetDest = receivedHit.hDir * 250.0;

	lineMovement->start = data.offsetPos;
	lineMovement->end = data.offsetDest;
	flySeq.Reset();

	Pop();

	//Throw(dir * flySpeed);

	sess->PlayerAddActiveComboObj(comboObj);
}

bool RelativeComboer::CanBeHitByComboer()
{
	return false;
}

void RelativeComboer::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			
			PopThrow();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
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
			//sprite.setColor(Color::Red);
			break;
		case S_RETURN:
			UpdateKilledNumberText(maxKilled);
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
			data.latchedOn = false;
			//basePos = origPos;
			break;
		}
	}

	if (action == S_ATTACHEDWAIT)
	{
		if (sess->GetPlayer(0)->IsBlockAction(sess->GetPlayer(0)->action))
		{
			action = S_WAIT;
			frame = 0;
			data.waitFrame = 0;
			data.latchedOn = false;
			DefaultHurtboxesOn();
		}
	}
}

void RelativeComboer::HandleNoHealth()
{

}

void RelativeComboer::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * numStep;

	data.offsetPos += movementVec;
}

void RelativeComboer::UpdateEnemyPhysics()
{
	if (data.latchedOn)
	{
		data.basePos = sess->GetPlayerPos(0);
	}
	else
	{


	}

	if (data.specialPauseFrames == 0)
	{


		switch (action)
		{
		case S_FLY:
		{
			//Move();
			//double factor = 
			//data.offsetPos = 
			data.offsetPos = flySeq.GetPos();
			flySeq.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);
			break;
		}
		}

	}

	if (data.latchedOn)
	{
		currPosInfo.position = data.offsetPos + data.basePos;
	}

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void RelativeComboer::FrameIncrement()
{
	if (data.specialPauseFrames > 0)
	{
		--data.specialPauseFrames;
	}
	if (action == S_FLY || action == S_WAIT )
	{
		if (data.waitFrame == maxWaitFrames)
		{
			action = S_RETURN;
			frame = 0;
			Return();
		}
		else
		{
			data.waitFrame++;
		}

	}

	if (data.latchedOn)
	{
		data.latchFrame++;
		if (data.latchFrame == maxLatchFrames)
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
	if (detachOnKill)
	{
		action = S_WAIT;
		frame = 0;
		data.latchedOn = false;
		DefaultHurtboxesOn();
		//sprite.setColor(Color::Blue);
	}

	++data.numKilled;

	if (limitedKills && data.numKilled == maxKilled)
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
	UpdateKilledNumberText(maxKilled - data.numKilled);

	sess->PlayerRestoreAirOptions(0);
}

void RelativeComboer::ComboHit()
{
	//pauseFrames = 5;
	data.specialPauseFrames = 5;
	++data.currHits;
	data.velocity = V2d(0, 0);
	if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
		Return();
	}
}

void RelativeComboer::UpdateSprite()
{
	if (data.latchedOn)
	{
		data.basePos = sess->GetPlayerPos(0);
		currPosInfo.position = data.basePos + data.offsetPos;
	}

	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	if (detachOnKill)
	{
		sprite.setTextureRect(ts->GetSubRect(10));
	}
	else
	{
		sprite.setTextureRect(ts->GetSubRect(11));
	}
	
	if (action == S_ATTACHEDWAIT)
	{
		sprite.setColor(Color::Yellow);
	}
	else
	{
		sprite.setColor(Color::White);
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	



	

	if (limitedKills)
	{
		numKilledText.setPosition(sprite.getPosition());
	}
}

void RelativeComboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedKills)
	{
		target->draw(numKilledText);
	}
}

int RelativeComboer::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void RelativeComboer::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void RelativeComboer::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	UpdateKilledNumberText(data.numKilledTextNumber);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}