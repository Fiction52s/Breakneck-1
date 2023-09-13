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

WireJuggler::WireJuggler( ActorParams *ap )
	:Enemy(EnemyType::EN_WIREJUGGLER, ap )
{
	enemyDrawLayer = ENEMYDRAWLAYER_COMBOER;
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

	const string &typeName = ap->GetTypeName();

	if (typeName == "bluewirejuggler")
	{
		jType = T_BLUE;//testing
	}
	else if (typeName == "redwirejuggler")
	{
		jType = T_RED;
	}
	else if (typeName == "magentawirejuggler")
	{
		jType = T_MAGENTA;
	}
	else
	{
		cout << "invalid typename: " << typeName << endl;
		assert(0);
	}

	UpdateParamsSettings();

	maxWaitFrames = 150;

	gravFactor = .8;//.45;

	gDir = V2d(0, 1);
	
	/*switch (jType)
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
	}*/
	maxFallSpeed = 15;

	ts = GetSizedTileset("Enemies/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setScale(scale, scale);

	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/

	BasicCircleHurtBodySetup(48);
	//BasicCircleHitBodySetup(48);

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
	comboObj->enemyHitboxInfo->hType = HitboxInfo::MAGENTA;

	comboObj->enemyHitBody.BasicCircleSetup(48 * scale, GetPosition());

	ResetEnemy();
}

WireJuggler::~WireJuggler()
{
}

void WireJuggler::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 2.5;
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
	data.currHits = 0;
	data.velocity = V2d(0, 0);
	data.waitFrame = 0;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);

	comboObj->Reset();

	DefaultHurtboxesOn();
	dead = false;
	action = S_FLOAT;
	frame = 0;

	receivedHit.SetEmpty();

	UpdateHitboxes();

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
	w->player->RestoreAirOptions();
}

HitboxInfo * WireJuggler::IsHit(int pIndex)
{
	if (currHurtboxes == NULL)
		return NULL;

	Actor *player = sess->GetPlayer(pIndex);

	Wire *wire = player->IntersectMyWireHitboxes(this, currHurtboxes, currHurtboxFrame);
	if (wire != NULL)
	{
		HandleWireHit(wire);
		return wire->tipHitboxInfo;
	}


	return NULL;
}

void WireJuggler::HandleWireUnanchored(Wire *w)
{
	action = S_JUGGLE;
	frame = 0;
}

void WireJuggler::HandleWireAnchored(Wire *w)
{
	action = S_HELD;
	data.velocity = V2d(0, 0);
	frame = 0;
}

void WireJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	data.velocity = vel;
}

void WireJuggler::Throw(V2d vel)
{
	data.velocity = vel;
}

void WireJuggler::Return()
{
	action = S_RETURN;
	frame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();

	numHealth = maxHealth;
}

void WireJuggler::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;

	//HurtboxesOff();

	data.waitFrame = 0;
}

void WireJuggler::PopThrow()
{
	action = S_POP;
	frame = 0;

	V2d dir;

	dir = receivedHit.hDir;//normalize(receivedHit->hDir);

	V2d hit(0, -20);

	if (dir.y < 0)
	{
		dir.y *= 2;
	}

	hit = dir * 12.0;//20.0;

	/*double extraX = 8;

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
	}*/

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
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		/*if (receivedHit.hType == HitboxInfo::WIREHITRED)
		{
			numHealth -= 1;
		}
		else if (receivedHit.hType == HitboxInfo::WIREHITBLUE)
		{
			numHealth -= 1;
		}*/

		numHealth -= 1;

		if (numHealth <= 0)
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

void WireJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			SetCurrPosInfo(startPosInfo);
			DefaultHurtboxesOn();
			action = S_FLOAT;
			frame = 0;
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
		}
	}

	if (action == S_POP && data.velocity.y >= 0 )
	{
		action = S_JUGGLE;
		frame = 0;
	}
}

void WireJuggler::HandleNoHealth()
{

}

void WireJuggler::Move()
{
	double numStep = numPhysSteps;
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * numStep;

	currPosInfo.position += movementVec;

	data.velocity += gDir * (gravFactor / numStep / slowMultiple);

	if (data.velocity.y > maxFallSpeed)
	{
		data.velocity.y = maxFallSpeed;
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

	comboObj->enemyHitboxInfo->hDir = normalize(data.velocity);
}

void WireJuggler::FrameIncrement()
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

void WireJuggler::ComboHit()
{
	pauseFrames = 5;
	++data.currHits;
	/*if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		
		Return();
	}*/
}

void WireJuggler::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	switch (jType)
	{
	case T_BLUE:
		tile = 12;
		break;
	case T_RED:
		tile = 13;
		break;
	case T_MAGENTA:
		tile = 14;
		break;
	}


	if (currHurtboxes != NULL)
	{
		sprite.setColor(Color::White);
	}
	else
	{
		sprite.setColor(Color::Blue);
	}

	sprite.setTextureRect(ts->GetSubRect(tile));

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void WireJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int WireJuggler::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void WireJuggler::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void WireJuggler::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}