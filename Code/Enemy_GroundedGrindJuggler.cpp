#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GroundedGrindJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

GroundedGrindJuggler::GroundedGrindJuggler(GameSession *owner, bool p_hasMonitor, Edge *ground, double q , int p_level, int jReps, bool cw)
	:Enemy(owner, EnemyType::EN_GROUNDEDGRINDJUGGLER, p_hasMonitor, 1, false)
{
	level = p_level;
	clockwise = cw;

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

	friction = .4;

	startGround = ground;
	startQuant = q;

	pushStart = 20;

	maxWaitFrames = 180;

	mover = new SurfaceMover(owner, ground, q, 10 * scale);
	mover->surfaceHandler = this;

	juggleReps = jReps;

	V2d gPoint = ground->GetPoint(q);
	//sprite.setPosition( testMover->physBody.globalPosition.x,
	//	testMover->physBody.globalPosition.y );

	position = mover->physBody.globalPosition;

	spawnRect = sf::Rect<double>(position.x - 16, position.y - 16, 16 * 2, 16 * 2);

	hitLimit = -1;

	action = S_IDLE;

	ts = owner->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	//sprite.setPosition(position.x, position.y);



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
	delete mover;
}

void GroundedGrindJuggler::ResetEnemy()
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
	action = S_IDLE;
	frame = 0;
	receivedHit = NULL;
	//position = origPos;

	mover->velocity = V2d(0, 0);
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);

	position = mover->physBody.globalPosition;

	UpdateHitboxes();



	currJuggle = 0;

	UpdateSprite();
}

void GroundedGrindJuggler::Push(double strength)
{
	owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;

	owner->GetPlayer(0)->AddActiveComboObj(comboObj);

	if (clockwise)
	{
		mover->SetSpeed(strength);
	}
	else
	{
		mover->SetSpeed(-strength);
	}
	
}

void GroundedGrindJuggler::Return()
{
	owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	

	//mover->ground = NULL;

	currJuggle = 0;

	numHealth = maxHealth;
}

void GroundedGrindJuggler::ProcessHit()
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
				action = S_GRIND;
				frame = 0;
				Push(pushStart);
			}
		}
		else
		{
			owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
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
			//position = origPos;
			//mover->physBody.globalPosition = position;

			mover->ground = startGround;
			mover->edgeQuantity = startQuant;
			mover->roll = false;
			mover->UpdateGroundPos();
			mover->SetSpeed(0);

			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
			break;*/

		case S_GRIND:
			action = S_SLOW;
			SetHurtboxes(hurtBody, 0);
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
	mover->velocity = velocity;
	mover->Move(slowMultiple, numPhysSteps);
	position = mover->physBody.globalPosition;
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
}

void GroundedGrindJuggler::FrameIncrement()
{
	if (action == S_SLOW)
	{
		double speed = mover->groundSpeed;
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

		mover->SetSpeed(speed);
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

void GroundedGrindJuggler::HandleEntrant(QuadTreeEntrant *qte)
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

		owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
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

		owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);

		}*/
	}
}

void GroundedGrindJuggler::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);

	int tile = 0;
	switch (action)
	{
	case S_IDLE:
		tile = 0;
		sprite.setTextureRect(ts->GetSubRect(tile));
		break;
	}

	if (action == S_SLOW)
	{
		if (clockwise)
		{
			sprite.setColor(Color::Red);
		}
		else
		{
			sprite.setColor(Color::Cyan);
		}
		
	}
	else if (action == S_STOPPED)
	{
		if (clockwise)
		{
			sprite.setColor(Color::Yellow);
		}
		else
		{
			sprite.setColor(Color::Cyan);
		}
		
	}
	else
	{
		if (clockwise)
		{
			sprite.setColor(Color::White);
		}
		else
		{
			sprite.setColor(Color::Green);
		}
		
	}
}

void GroundedGrindJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

CollisionBox &GroundedGrindJuggler::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void GroundedGrindJuggler::UpdateHitboxes()
{
	position = mover->physBody.globalPosition;

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