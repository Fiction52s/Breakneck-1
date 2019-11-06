#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BounceJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

BounceJuggler::BounceJuggler(GameSession *owner, bool p_hasMonitor, Vector2i pos,
	list<Vector2i> &path, int p_level, int jReps)
	:Enemy(owner, EnemyType::EN_BOUNCEJUGGLER, p_hasMonitor, 1, false)
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

	maxWaitFrames = 180;

	juggleReps = path.size();

	mover = new SurfaceMover(owner, NULL, 0, 64 * scale);
	mover->surfaceHandler = this;

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

BounceJuggler::~BounceJuggler()
{
	if (guidedDir != NULL)
		delete[] guidedDir;

	delete mover;
}

void BounceJuggler::ResetEnemy()
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

	mover->velocity = V2d(0, 0);
	mover->ground = NULL;
	mover->physBody.globalPosition = position;

	UpdateHitboxes();

	

	currJuggle = 0;

	UpdateSprite();
}

void BounceJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	velocity = vel;
}

void BounceJuggler::Throw(V2d vel)
{
	velocity = vel;
}

void BounceJuggler::Return()
{
	owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	currJuggle = 0;

	numHealth = maxHealth;
}

void BounceJuggler::Pop()
{
	owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;
}

void BounceJuggler::PopThrow()
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


	double speed = 20;

	V2d hit = dir * speed;
	
	Pop();

	Throw(hit);

	owner->GetPlayer(0)->AddActiveComboObj(comboObj);
}

void BounceJuggler::ProcessHit()
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

void BounceJuggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_RETURN:
			position = origPos;
			mover->physBody.globalPosition = position;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			break;
			/*case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
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

void BounceJuggler::HandleNoHealth()
{

}

void BounceJuggler::Move()
{
	mover->velocity = velocity;
	mover->Move(slowMultiple, numPhysSteps);
	position = mover->physBody.globalPosition;
}

void BounceJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLY:
	case S_BOUNCE:
	{
		Move();
		break;
	}
	}
}

void BounceJuggler::FrameIncrement()
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
	}
}

void BounceJuggler::HandleEntrant(QuadTreeEntrant *qte)
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

void BounceJuggler::ComboHit()
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

void BounceJuggler::UpdateSprite()
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

void BounceJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

CollisionBox &BounceJuggler::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void BounceJuggler::UpdateHitboxes()
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

void BounceJuggler::HitTerrainAerial(Edge * edge, double quant)
{
	V2d pos = edge->GetPoint(quant);
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
			en = normalize(position - pos);
		}
		else if (pos == edge->v1)
		{
			en = normalize(position - pos);
		}
		double d = dot(velocity, en);
		V2d ref = velocity - (2.0 * d * en);
		velocity = ref;
		mover->ground = NULL;
	}

	action = S_BOUNCE;
	frame = 0;
}