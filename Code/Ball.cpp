#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Ball.h"
#include "Eye.h"
#include "KeyMarker.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

void Ball::UpdateParamsSettings()
{
	/*JugglerParams *jParams = (JugglerParams*)editParams;
	juggleReps = jParams->numJuggles;*/
}

void Ball::SetLevel(int lev)
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

Ball::Ball(ActorParams *ap)
	:Enemy(EnemyType::EN_BALL, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	flySpeed = 14;
	maxWaitFrames = 5000;

	UpdateParamsSettings();

	CreateSurfaceMover(startPosInfo, 32, this);
	

	guidedDir = NULL;

	hitLimit = -1;

	action = S_FLOAT;

	ts = sess->GetSizedTileset("Enemies/Ball_64x64.png");
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
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	comboObj->enemyHitBody.BasicCircleSetup(32, GetPosition());
	comboObj->enemyHitboxFrame = 0;

	actionLength[S_FLOAT] = 18;
	actionLength[S_FLY] = 10;
	actionLength[S_BOUNCE] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_BOUNCE] = 1;
	animFactor[S_RETURN] = 6;

	ResetEnemy();
}

Ball::~Ball()
{
	if (guidedDir != NULL)
		delete[] guidedDir;
}

void Ball::ResetEnemy()
{
	sprite.setRotation(0);
	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	surfaceMover->velocity = V2d(0, 0);
	DefaultHurtboxesOn();
	//DefaultHitboxesOn();
	action = S_FLOAT;
	frame = 0;
	receivedHit = NULL;
	currJuggle = 0;

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();
	surfaceMover->AddAirForce(V2d(0, 1));

	UpdateHitboxes();

	UpdateSprite();
}

void Ball::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	surfaceMover->velocity = vel;
}

void Ball::Throw(V2d vel)
{
	surfaceMover->velocity = vel;
}

void Ball::Return()
{
	sess->PlayerRemoveActiveComboer(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	currJuggle = 0;

	numHealth = maxHealth;
}

void Ball::Pop()
{
	sess->PlayerConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;
}

void Ball::PopThrow()
{
	V2d dir;

	dir = receivedHit->hDir;//normalize(receivedHit->hDir);
							//cout << "dir: " << dir.x << "," << dir.y << endl;
	if (dir.x == 0 && dir.y == 0)
	{
		dir = -normalize(surfaceMover->velocity);
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

void Ball::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		//Actor *player = owner->GetPlayer(0);

		if (numHealth <= 0)
		{
			if (currJuggle == juggleReps)
			{
				if (hasMonitor && !suppressMonitor)
				{
					sess->CollectKey();
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
				action = S_FLY;
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

void Ball::ProcessState()
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

void Ball::HandleNoHealth()
{

}

//void Ball::Move()
//{
//	mover->velocity = velocity;
//	mover->Move(slowMultiple, numPhysSteps);
//	position = mover->physBody.globalPosition;
//}

void Ball::UpdateEnemyPhysics()
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

	comboObj->enemyHitboxInfo->hDir = normalize(surfaceMover->velocity);
}

void Ball::FrameIncrement()
{
	if (action == S_FLY || action == S_BOUNCE)
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

		if (waitFrame > 16)
		{
			DefaultHurtboxesOn();
		}
	}
}

void Ball::ComboHit()
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

void Ball::ComboKill(Enemy *e)
{
	if (level == 2)
	{
		V2d playerDir = normalize(sess->GetPlayerPos(0) - GetPosition());
		Throw(playerDir * flySpeed);

		action = S_BOUNCE;
		frame = 0;
		DefaultHurtboxesOn();
	}
}

void Ball::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
		sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Ball::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

V2d Ball::GetBounceVel(V2d &normal)
{
	double d = dot(surfaceMover->velocity, normal);
	V2d ref = surfaceMover->velocity - (2.0 * d * normal);
	return ref;
}

void Ball::HitTerrainAerial(Edge * edge, double quant)
{
	V2d pos = edge->GetPosition(quant);
	/*if (b->bounceCount == 2)
	{
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);
	owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);
	}
	else*/
	{
		V2d en = edge->Normal();
		if (pos == edge->v0)
		{
			en = normalize(GetPosition() - pos);
		}
		else if (pos == edge->v1)
		{
			en = normalize(GetPosition() - pos);
		}
		double d = dot(surfaceMover->velocity, en);
		double damp = 1.0;
		V2d ref = GetBounceVel(en);
		surfaceMover->velocity = ref;
		surfaceMover->ground = NULL;
	}

	action = S_BOUNCE;
	frame = 0;
	//SetHitboxes(hitBody, 0);
	//DefaultHurtboxesOn();
}