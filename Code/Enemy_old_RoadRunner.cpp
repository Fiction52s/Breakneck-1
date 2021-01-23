#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_RoadRunner.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

RoadRunner::RoadRunner(GameSession *owner, bool p_hasMonitor, Edge *g, double q, int p_level )
	:Enemy(owner, EnemyType::EN_ROADRUNNER, p_hasMonitor, 2), facingRight(true)
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

	//gravity = V2d(0, .6);
	maxGroundSpeed = 70;
	action = IDLE;
	dead = false;

	maxFallSpeed = 40;

	double height = 128;
	double width = 128;

	startGround = g;
	startQuant = q;
	frame = 0;

	mover = new GroundMover(owner, g, q, 40, true, this);
	mover->AddAirForce(V2d(0, .5));
	
	mover->SetSpeed(0);

	ts = owner->GetTileset("Enemies/roadrunner_256x256.png", 256, 256);
	sprite.setScale(scale, scale);
	sprite.setTexture(*ts->texture);

	actionLength[IDLE] = 1;
	actionLength[BURROW] = 10;
	actionLength[UNBURROW] = 20;
	actionLength[RUN] = 1;
	actionLength[HOP] = 1;
	//actionLength[STALK] = 30;
	//actionLength[RUSH] = 1;


	animFactor[IDLE] = 1;
	animFactor[BURROW] = 1;
	animFactor[UNBURROW] = 1;
	animFactor[RUN] = 1;
	animFactor[HOP] = 1;
	//animFactor[STALK] = 1;
	//animFactor[RUSH] = 1;

	position = mover->physBody.globalPosition;
	V2d gPoint = g->GetPoint(q);

	receivedHit = NULL;

	double size = max(width, height);
	spawnRect = sf::Rect<double>(gPoint.x - size, gPoint.y - size, size * 2, size * 2);

	int hurtRad = 40;
	int hitRad = 40;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	SetupBodies(1, 1);
	AddBasicHurtCircle(hurtRad);
	AddBasicHitCircle(hitRad);
	hitBody->hitboxInfo = hitboxInfo;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);

	ResetEnemy();
}

RoadRunner::~RoadRunner()
{
	delete mover;
}

void RoadRunner::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
}

void RoadRunner::ResetEnemy()
{
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	action = IDLE;
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);

	position = mover->physBody.globalPosition;

	frame = 0;

	dead = false;

	receivedHit = NULL;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateSprite();
	UpdateHitboxes();
}

void RoadRunner::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if (ground != NULL)
	{

		V2d knockbackDir(1, -1);
		knockbackDir = normalize(knockbackDir);
		if (mover->groundSpeed > 0)
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
			hitboxInfo->knockback = 15;
		}
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = mover->physBody.globalPosition;
	hurtBox.globalPosition = mover->physBody.globalPosition;
}

void RoadRunner::ActionEnded()
{
	if (frame == actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case BURROW:
			action = UNDERGROUND;
			break;
		case UNBURROW:
			action = RUN;
			break;
		case RUN:
			break;
		/*case STALK:
			action = RUSH;
			frame = 0;
			break;
		case RUSH:
			break;*/
		}
	}
}

void RoadRunner::FrameIncrement()
{
	Actor *player = owner->GetPlayer(0);
	//if (action == RUN || action == UNBURROW )
	//{
	//	if (player->IsMovingLeft())
	//	{
	//		playerNotMovingLeftCounter = 0;
	//	}
	//	else
	//	{
	//		++playerNotMovingLeftCounter;
	//	}

	//	if (player->IsMovingRight())
	//	{
	//		playerNotMovingRightCounter = 0;
	//	}
	//	else
	//	{

	//	}
	//	//can check number of frames that you arent moving right/left
	//}
	
}

void RoadRunner::ProcessState()
{
	V2d playerPos = owner->GetPlayerPos(0);

	ActionEnded();

	double playerDist = length(playerPos - position);

	switch (action)
	{
	case IDLE:
		if (playerDist < 300 )
		{
			action = BURROW;
			frame = 0;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
		}
		break;
	case BURROW:
		if (frame == 5 && slowCounter == 1)
		{
			SetHitboxes(NULL, 0);
			SetHurtboxes(NULL, 0);
		}
		break;
	case UNDERGROUND:
	{
		bool uleft = playerPos.x < position.x - 300 && owner->PlayerIsMovingLeft();
		bool uright = playerPos.x > position.x + 300 && owner->PlayerIsMovingRight();
		if( uleft || uright )
		{
			if (uleft)
				facingRight = false;
			else
			{
				facingRight = true;
			}
			action = UNBURROW;
			frame = 0;
		}
		break;
	}
	case UNBURROW:
		if(frame == 10 && slowCounter == 1)
		{
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
		}
		break;
	case RUN:
		if (facingRight)
		{
			if (playerPos.x < position.x - 200 )//|| ( !player->IsMovingRight() && player->action != Actor::JUMPSQUAT ))
			{
				action = BURROW;
				frame = 0;
				mover->SetSpeed(0);
				break;
			}
		}
		else
		{
			if (playerPos.x > position.x + 200 )// || (!player->IsMovingLeft() && player->action != Actor::JUMPSQUAT))
			{
				action = BURROW;
				frame = 0;
				mover->SetSpeed(0);
				break;
			}
		}

		double accel = .5;//.5;
		if (facingRight)
		{
			//mover->SetSpeed(15);
			mover->SetSpeed(mover->groundSpeed + accel);
		}
		else
		{
			//mover->SetSpeed(-15);
			mover->SetSpeed(mover->groundSpeed - accel);
		}

		if (mover->groundSpeed > maxGroundSpeed)
			mover->SetSpeed(maxGroundSpeed);
		else if (mover->groundSpeed < -maxGroundSpeed)
			mover->SetSpeed(-maxGroundSpeed);


		/*if (playerDist < 200)
		{
			if (action == RUN)
				action = STALK;

			if (action == STALK)
			{
				if (player->ground != NULL && player->action != Actor::JUMPSQUAT)
				{
					mover->SetSpeed(player->groundSpeed);
				}
			}
		}*/
		break;
	}

	//cout << "moverspeed: " << mover->groundSpeed << endl;
}

//void RoadRunner::ProcessState()
//{
//	Actor *player = owner->GetPlayer(0);
//
//	ActionEnded();
//
//	double playerDist = length(player->position - position);
//
//	switch (action)
//	{
//	case IDLE:
//		if (playerDist < 300)
//		{
//			action = BURROW;
//			frame = 0;
//			SetHitboxes(hitBody, 0);
//			SetHurtboxes(hurtBody, 0);
//		}
//		break;
//	case BURROW:
//		if (frame == 5 && slowCounter == 1)
//		{
//			SetHitboxes(NULL, 0);
//			SetHurtboxes(NULL, 0);
//		}
//		break;
//	case UNDERGROUND:
//	{
//		bool uleft = player->position.x < position.x - 300 && player->IsMovingLeft();
//		bool uright = player->position.x > position.x + 300 && player->IsMovingRight();
//		if (uleft || uright)
//		{
//			if (uleft)
//				facingRight = false;
//			else
//			{
//				facingRight = true;
//			}
//			action = UNBURROW;
//			frame = 0;
//		}
//		break;
//	}
//	case UNBURROW:
//		if (frame == 10 && slowCounter == 1)
//		{
//			SetHitboxes(hitBody, 0);
//			SetHurtboxes(hurtBody, 0);
//		}
//		break;
//	case RUN:
//	case STALK:
//	case RUSH:
//		if (facingRight)
//		{
//			if (player->position.x < position.x - 200)//|| ( !player->IsMovingRight() && player->action != Actor::JUMPSQUAT ))
//			{
//				action = BURROW;
//				frame = 0;
//				mover->SetSpeed(0);
//				break;
//			}
//		}
//		else
//		{
//			if (player->position.x > position.x + 200)// || (!player->IsMovingLeft() && player->action != Actor::JUMPSQUAT))
//			{
//				action = BURROW;
//				frame = 0;
//				mover->SetSpeed(0);
//				break;
//			}
//		}
//
//		double accel = 4;//.5;
//		if (facingRight)
//		{
//			//mover->SetSpeed(15);
//			mover->SetSpeed(mover->groundSpeed + accel);
//		}
//		else
//		{
//			//mover->SetSpeed(-15);
//			mover->SetSpeed(mover->groundSpeed - accel);
//		}
//
//		if (mover->groundSpeed > maxGroundSpeed)
//			mover->SetSpeed(maxGroundSpeed);
//		else if (mover->groundSpeed < -maxGroundSpeed)
//			mover->SetSpeed(-maxGroundSpeed);
//
//
//		if (playerDist < 200)
//		{
//			if (action == RUN)
//				action = STALK;
//
//			if (action == STALK)
//			{
//				if (player->ground != NULL && player->action != Actor::JUMPSQUAT)
//				{
//					mover->SetSpeed(player->groundSpeed);
//				}
//			}
//		}
//		break;
//	}
//
//	//cout << "moverspeed: " << mover->groundSpeed << endl;
//}

void RoadRunner::UpdateEnemyPhysics()
{
	if (numHealth > 0) //!dead
	{
		mover->Move(slowMultiple, numPhysSteps);

		if (mover->ground == NULL)
		{
			if (mover->velocity.y > maxFallSpeed)
			{
				mover->velocity.y = maxFallSpeed;
			}
		}

		position = mover->physBody.globalPosition;
	}
}

void RoadRunner::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

void RoadRunner::UpdateSprite()
{
	Edge *ground = mover->ground;
	double edgeQuantity = mover->edgeQuantity;
	V2d gn;

	V2d gPoint;
	if (ground != NULL)
	{
		gPoint = position;//ground->GetPoint( edgeQuantity );
		gn = ground->Normal();
	}
	else
	{

		gPoint = position;
	}


	int originHeight = sprite.getLocalBounds().height / 2;

	IntRect r;
	r = ts->GetSubRect(0);

	if (!facingRight)
	{
		r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
	}
	sprite.setTextureRect(r);

	float extraVert = 0;
	
	double angle;

	angle = 0;

	if (ground != NULL)
	{
		V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(pp.x, pp.y);
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		sprite.setRotation(0);
		sprite.setPosition(gPoint.x, gPoint.y);
	}
	

	return;

	if (mover->ground != NULL)
	{
		if (!mover->roll)
		{
			angle = atan2(gn.x, -gn.y);

			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
			sprite.setRotation(angle / PI * 180);
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			if (mover->groundSpeed > 0)//facingRight)
			{
				V2d vec = normalize(position - mover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
				sprite.setPosition(pp.x, pp.y);
			}
			else
			//else if (mover->groundSpeed < 0)
			{
				V2d vec = normalize(position - mover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
				sprite.setPosition(pp.x, pp.y);
			}
		}
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, originHeight);
		sprite.setRotation(0);
		sprite.setPosition(gPoint.x, gPoint.y);
	}
}

void RoadRunner::HitTerrain(double &q)
{
	//cout << "hit terrain? " << endl;
}

bool RoadRunner::StartRoll()
{
	return false;
}

void RoadRunner::FinishedRoll()
{

}

void RoadRunner::HitOther()
{
	//cout << "hit other" << endl;

	/*if (action == RUN)
	{
		if ((facingRight && testMover->groundSpeed < 0)
			|| (!facingRight && testMover->groundSpeed > 0))
		{
			cout << "here" << endl;
			testMover->SetSpeed(0);
		}
		else if (facingRight && testMover->groundSpeed > 0)
		{
			V2d v = V2d(maxGroundSpeed, -10);
			testMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
		else if (!facingRight && testMover->groundSpeed < 0)
		{
			V2d v = V2d(-maxGroundSpeed, -10);
			testMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
	}
*/
	//cout << "hit other!" << endl;
	//testMover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void RoadRunner::ReachCliff()
{
	if (facingRight)
	{
		mover->Jump(V2d(15, -10));
	}
	else
	{
		mover->Jump(V2d(-15, -10));
	}

	action = HOP;
	frame = 0;
	
	//if ((facingRight && testMover->groundSpeed < 0)
	//	|| (!facingRight && testMover->groundSpeed > 0))
	//{
	//	testMover->SetSpeed(0);
	//	return;
	//}

	////cout << "reach cliff!" << endl;
	////ground = NULL;

	//V2d v;
	//if (facingRight)
	//{
	//	v = V2d(maxGroundSpeed, -10);
	//}
	//else
	//{
	//	v = V2d(-maxGroundSpeed, -10);
	//}

	//testMover->Jump(v);

	//action = JUMP;
	//frame = 0;
}

void RoadRunner::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void RoadRunner::Land()
{
	action = IDLE;
	frame = 0;
}