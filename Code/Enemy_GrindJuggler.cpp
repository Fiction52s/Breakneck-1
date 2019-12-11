#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GrindJuggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"
#include "Rail.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

GrindJuggler::GrindJuggler(GameSession *owner, bool p_hasMonitor, Vector2i pos, int p_level, int jReps, bool cw)
	:Enemy(owner, EnemyType::EN_GRINDJUGGLER, p_hasMonitor, 1, false)
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

	flySpeed = 10;

	maxWaitFrames = 100000;//180;

	mover = new SurfaceRailMover(owner, NULL, 0, 10 * scale);
	mover->SetHandler(this);
	//mover->surfaceHandler = this;

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
	actionLength[S_GRIND] = 10;
	actionLength[S_RETURN] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_FLY] = 1;
	animFactor[S_GRIND] = 1;
	animFactor[S_RETURN] = 6;

	ResetEnemy();
}

GrindJuggler::~GrindJuggler()
{
	delete mover;
}

void GrindJuggler::ResetEnemy()
{
	mover->collisionOn = true;
	mover->railCollisionOn = true;
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	currRail = NULL;
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

void GrindJuggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	mover->velocity = vel;
}

void GrindJuggler::Throw(V2d vel)
{
	mover->velocity = vel;
}

void GrindJuggler::Return()
{
	owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);

	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);

	mover->ground = NULL;

	currJuggle = 0;

	numHealth = maxHealth;
}

void GrindJuggler::Pop()
{
	owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
	ConfirmHitNoKill();
	numHealth = maxHealth;
	++currJuggle;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	waitFrame = 0;
}

void GrindJuggler::PopThrow()
{
	V2d dir;

	dir = Get8Dir(receivedHit->hDir);//normalize(receivedHit->hDir);
							//cout << "dir: " << dir.x << "," << dir.y << endl;
	if (dir.x == 0 && dir.y == 0)
	{
		dir = normalize(V2d(velocity.y, -velocity.x));//-normalize(velocity);
		assert(dir.x != 0 || dir.y != 0);
		action = S_FLY;
		frame = 0;
	}

	double speed = 14;

	V2d hit = dir * speed;

	Pop();

	Throw(hit);

	owner->GetPlayer(0)->AddActiveComboObj(comboObj);
}

void GrindJuggler::ProcessHit()
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

void GrindJuggler::ProcessState()
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

void GrindJuggler::HandleNoHealth()
{

}

void GrindJuggler::ExtraQueries(Rect<double> &r)
{
	//Rect<double> r = mover->physBody.GetAABB();//hitBody->GetCollisionBoxes(0)->front().GetAABB();
	/*if (action == S_FLY)
	{
		owner->railEdgeTree->Query(this, r);
	}*/
}

void GrindJuggler::Move()
{
	mover->Move(slowMultiple, numPhysSteps);
	//mover->

	position = mover->physBody.globalPosition;
}

void GrindJuggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLY:
	case S_GRIND:
	{
		Move();
		break;
	}
	case S_RAILGRIND:
	{
		Move();
		//currRail->
		break;
	}
		
	}
}

void GrindJuggler::FrameIncrement()
{
	if (action == S_FLY || action == S_GRIND)
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

void GrindJuggler::HandleEntrant(QuadTreeEntrant *qte)
{
	Edge *e = (Edge*)qte;
	Rail *rail = (Rail*)e->info;

	V2d pos = mover->physBody.globalPosition;
	V2d tempVel = mover->tempVel;

	//if (IsEdgeTouchingCircle(e->v0, e->v1, mover->physBody.globalPosition, mover->physBody.rw))
	
	V2d r;
	V2d eFocus;
	bool ceiling = false;
	V2d en = e->Normal();
	if (en.y > 0)
	{
		r = e->v0 - e->v1;
		eFocus = e->v1;
		ceiling = true;
	}
	else
	{
		r = e->v1 - e->v0;
		eFocus = e->v0;
	}

	V2d along = normalize(r);
	double lenR = length(r);
	double q = dot(pos - eFocus, along);

	double c = cross(pos - e->v0, along);
	double preC = cross((pos - tempVel) - e->v0, along);

	double alongQuantVel = dot(velocity, along);

	bool cStuff = (c >= 0 && preC <= 0) || (c <= 0 && preC >= 0);

	if (cStuff && q >= 0 && q <= lenR)//&& alongQuantVel != 0)
	{
		V2d rn(along.y, -along.x);

		railEdge = e;
		//prevRail = (Rail*)grindEdge->info;


		LineIntersection li = lineIntersection(position, position - tempVel, railEdge->v0, railEdge->v1);
		if (!li.parallel)
		{
			V2d p = li.position;
			railQuant = railEdge->GetQuantity(p);

			position = p;
		}
		else
		{
			assert(0);

			//probably just start at the beginning or end of the rail based on which is closer?
		}

		if (ceiling) //ceiling rail
		{
			railSpeed = -10;
		}
		else
		{
			railSpeed = 10;
		}

		mover->physBody.globalPosition = position;
		mover->ground = railEdge;
		mover->edgeQuantity = railQuant;
		mover->SetSpeed(10);
		
		action = S_RAILGRIND;
		frame = 0;
		mover->velocity = along * 10.0;//V2d(0, 0);
	}
	//Enemy *en = (Enemy*)qte;
	//if (en->type == EnemyType::EN_JUGGLERCATCHER)
	//{
	//	JugglerCatcher *catcher = (JugglerCatcher*)qte;

	//	CollisionBox &hitB = hurtBody->GetCollisionBoxes(0)->front();
	//	if (catcher->CanCatch() && catcher->hurtBody->Intersects(catcher->currHurtboxFrame, &hitB))
	//	{
	//		//catcher->Catch();
	//		//action = S_EXPLODE;
	//		//frame = 0;
	//	}
	//}
}


void GrindJuggler::ComboKill(Enemy *e)
{
	if (action == S_GRIND)
	{
		action = S_FLY;
		frame = 0;

		mover->SetSpeed(0);

		mover->velocity = mover->ground->Normal() * flySpeed;
		mover->ground = NULL;


		SetHurtboxes(hurtBody, 0);

		owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
	}
}

void GrindJuggler::ComboHit()
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

void GrindJuggler::UpdateSprite()
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

void GrindJuggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

CollisionBox &GrindJuggler::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void GrindJuggler::UpdateHitboxes()
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

void GrindJuggler::HitTerrainAerial(Edge * edge, double quant)
{
	V2d pos = edge->GetPoint(quant);

	if (clockwise)
	{
		mover->SetSpeed(flySpeed);
	}
	else
	{
		mover->SetSpeed(-flySpeed);
	}
	
	action = S_GRIND;
	frame = 0;
	//sprite.setScale(scale * 2, scale * 2);
	
	///*if (b->bounceCount == 2)
	//{
	//V2d norm = edge->Normal();
	//double angle = atan2(norm.y, -norm.x);
	//owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	//b->launcher->DeactivateBullet(b);
	//}
	//else*/
	//{
	//	V2d en = edge->Normal();
	//	if (pos == edge->v0)
	//	{
	//		en = normalize(position - pos);
	//	}
	//	else if (pos == edge->v1)
	//	{
	//		en = normalize(position - pos);
	//	}
	//	double d = dot(velocity, en);
	//	V2d ref = velocity - (2.0 * d * en);
	//	velocity = ref;
	//	mover->ground = NULL;
	//}

	//action = S_BOUNCE;
	//frame = 0;
	////SetHitboxes(hitBody, 0);
	//SetHurtboxes(hurtBody, 0);
}