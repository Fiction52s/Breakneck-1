#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_WireBall.h"
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

void WireBall::UpdateParamsSettings()
{
	Enemy::UpdateParamsSettings();

	JugglerParams *jParams = (JugglerParams*)editParams;
	juggleReps = jParams->numJuggles;

	if (juggleReps == 0)
	{
		limitedJuggles = false;
	}
	else
	{
		limitedJuggles = true;
		UpdateJuggleRepsText(juggleReps);
	}
}

void WireBall::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		break;
	case 3:
		scale = 3.0;
		break;
	}
}

WireBall::WireBall(ActorParams *ap)
	:Enemy(EnemyType::EN_WIREBALL, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	numJugglesText.setFont(sess->mainMenu->arial);
	numJugglesText.setFillColor(Color::White);
	numJugglesText.setOutlineColor(Color::Black);
	numJugglesText.setOutlineThickness(3);
	numJugglesText.setCharacterSize(32);

	flySpeed = 14;
	maxWaitFrames = 180;

	UpdateParamsSettings();

	CreateSurfaceMover(startPosInfo, 32, this);



	guidedDir = NULL;

	hitLimit = -1;

	action = S_FLOAT;

	ts = GetSizedTileset("Enemies/General/comboers_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/

	BasicCircleHurtBodySetup(32);
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
	comboObj->enemyHitboxInfo->hType = HitboxInfo::YELLOW;

	comboObj->enemyHitBody.BasicCircleSetup(32, GetPosition());

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 10;
	actionLength[S_BOUNCE] = 10;
	actionLength[S_RETURN] = 30;


	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_BOUNCE] = 1;
	animFactor[S_RETURN] = 1;

	ResetEnemy();
}

WireBall::~WireBall()
{
	if (guidedDir != NULL)
		delete[] guidedDir;
}

void WireBall::ResetEnemy()
{
	data.currHits = 0;
	data.currJuggle = 0;
	data.doneBeingHittable = false;

	sprite.setRotation(0);

	comboObj->Reset();
	surfaceMover->SetVelocity(V2d(0, 0));
	DefaultHurtboxesOn();
	//DefaultHitboxesOn();
	action = S_FLOAT;
	frame = 0;
	receivedHit.SetEmpty();


	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();
	surfaceMover->AddAirForce(V2d(0, 1));

	UpdateJuggleRepsText(juggleReps);

	UpdateHitboxes();

	UpdateSprite();
}

void WireBall::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	surfaceMover->SetVelocity(vel);
}

void WireBall::Throw(V2d vel)
{
	surfaceMover->SetVelocity(vel);
}

void WireBall::Return()
{
	action = S_RETURN;
	frame = 0;

	sess->PlayerRemoveActiveComboer(comboObj);

	HurtboxesOff();

	data.doneBeingHittable = true;

	data.currJuggle = 0;

	UpdateJuggleRepsText(0);

	numHealth = maxHealth;
}

void WireBall::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++data.currJuggle;

	HurtboxesOff();
	data.waitFrame = 0;

	UpdateJuggleRepsText(juggleReps - data.currJuggle);
}

void WireBall::PopThrow()
{
	V2d dir;

	action = S_FLY;
	frame = 0;

	dir = receivedHit.hDir;//normalize(receivedHit->hDir);
						   //cout << "dir: " << dir.x << "," << dir.y << endl;
	if (dir.x == 0 && dir.y == 0)
	{
		dir = -normalize(surfaceMover->GetVel());
		assert(dir.x != 0 || dir.y != 0);
		action = S_BOUNCE;
		frame = 0;
	}

	/*V2d hit(0, -20);

	double extraX = 8;

	if (dir.x != 0)
	{
	hit.x += dir.x * extraX;
	}

	bool pFacingRight = sess->PlayerIsFacingRight(0);

	if (dir.y == 1 && dir.x == 0)
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

	}*/

	/*if (dir.y < 0)
	{
	dir.y *= 2;
	}
	V2d hit = surfaceMover->velocity + dir * flySpeed;*/

	if (dir.y < 0)
	{
		dir.y *= 2;
	}
	V2d hit = dir * flySpeed;

	Pop();

	Throw(hit);

	sess->PlayerAddActiveComboObj(comboObj);
}

void WireBall::UpdateJuggleRepsText(int reps)
{
	if (limitedJuggles)
	{
		data.juggleTextNumber = reps;
		numJugglesText.setString(to_string(reps));
		numJugglesText.setOrigin(numJugglesText.getLocalBounds().left
			+ numJugglesText.getLocalBounds().width / 2,
			numJugglesText.getLocalBounds().top
			+ numJugglesText.getLocalBounds().height / 2);
		numJugglesText.setPosition(sprite.getPosition());
	}
}

bool WireBall::CanComboHit(Enemy *e)
{
	if (e->type == EN_WIREBALL)
	{
		return false;
	}

	return true;
}

void WireBall::ProcessHit()
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
					PlayKeyDeathSound();
				}

				sess->PlayerConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

				PopThrow();


				data.doneBeingHittable = true;

			}
			else
			{
				if (!limitedJuggles)
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

				PopThrow();
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

void WireBall::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			surfaceMover->Set(startPosInfo);
			//DefaultHitboxesOn();
			DefaultHurtboxesOn();
			UpdateJuggleRepsText(juggleReps);
			action = S_FLOAT;
			frame = 0;
			data.doneBeingHittable = false;
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->PlayerRemoveActiveComboer(comboObj);
			break;*/
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

void WireBall::HandleNoHealth()
{

}

bool WireBall::CanBeHitByWireTip(bool red)
{
	return true;
}

//void WireBall::Move()
//{
//	mover->velocity = velocity;
//	mover->Move(slowMultiple, numPhysSteps);
//	position = mover->physBody.globalPosition;
//}

bool WireBall::CanBeAnchoredByWire(bool red)
{
	return false;
}

void WireBall::HandleWireHit(Wire *w)
{
	w->HitEnemy(GetPosition());
	w->player->RestoreAirOptions();
}

HitboxInfo * WireBall::IsHit(int pIndex)
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

void WireBall::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLY:
	case S_BOUNCE:
	{
		Enemy::UpdateEnemyPhysics();
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(surfaceMover->GetVel());
}

void WireBall::FrameIncrement()
{
	if (action == S_FLY || action == S_BOUNCE)
	{
		if (data.waitFrame == maxWaitFrames)
		{
			Return();
		}
		else
		{
			data.waitFrame++;
		}

		if (data.waitFrame > 16 && !data.doneBeingHittable)
		{
			DefaultHurtboxesOn();
		}
	}
}

void WireBall::ComboHit()
{
	pauseFrames = 5;
	data.waitFrame = 0;
	++data.currHits;
	if (hitLimit > 0 && data.currHits >= hitLimit)
	{
		action = S_RETURN;
		frame = 0;
		Return();
	}
}

void WireBall::ComboKill(Enemy *e)
{
	if (level == 2)
	{
		V2d playerDir = normalize(sess->GetPlayerPos(0) - GetPosition());
		Throw(playerDir * flySpeed);

		action = S_BOUNCE;
		frame = 0;

		if (!data.doneBeingHittable)
			DefaultHurtboxesOn();
	}
}

void WireBall::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		//sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(14));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	if (data.doneBeingHittable)
	{
		sprite.setColor(Color::Blue);
	}
	else
	{
		sprite.setColor(Color::White);
	}


	if (limitedJuggles)
	{
		numJugglesText.setPosition(sprite.getPosition());
	}
}

void WireBall::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (limitedJuggles)
	{
		target->draw(numJugglesText);
	}
}

V2d WireBall::GetBounceVel(V2d &normal)
{
	double d = dot(surfaceMover->GetVel(), normal);
	V2d ref = surfaceMover->GetVel() - (2.0 * d * normal);
	return ref;
}

void WireBall::HitTerrainAerial(Edge * edge, double quant)
{
	V2d pos = edge->GetPosition(quant);
	
	V2d en = edge->Normal();
	if (pos == edge->v0)
	{
		en = normalize(GetPosition() - pos);
	}
	else if (pos == edge->v1)
	{
		en = normalize(GetPosition() - pos);
	}
	double d = dot(surfaceMover->GetVel(), en);
	double damp = 1.0;
	V2d ref = GetBounceVel(en);
	surfaceMover->SetVelocity(ref);
	surfaceMover->ground = NULL;

	action = S_BOUNCE;
	frame = 0;

	//SetHitboxes(hitBody, 0);
	//DefaultHurtboxesOn();
}


int WireBall::GetNumStoredBytes()
{
	return sizeof(MyData) + comboObj->GetNumStoredBytes();
}

void WireBall::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	comboObj->StoreBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}

void WireBall::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);

	UpdateJuggleRepsText(data.juggleTextNumber);
	bytes += sizeof(MyData);

	comboObj->SetFromBytes(bytes);
	bytes += comboObj->GetNumStoredBytes();
}