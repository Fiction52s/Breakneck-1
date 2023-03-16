#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Skunk.h"
#include "Shield.h"

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

Skunk::Skunk(ActorParams *ap)
	:Enemy(EnemyType::EN_SKUNK, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 11 * 5;
	actionLength[LAND] = 1;
	actionLength[HOP] = 2;
	actionLength[WALK] = 60;
	actionLength[WAKEUP] = 30;
	actionLength[EXPLODE] = 60;

	gravity = .8;
	maxGroundSpeed = 3;
	maxFallSpeed = 20;
	runAccel = 1.0;
	runDecel = runAccel * 3.0;

	explosionRadius = 300;


	attentionRadius = 800;//800;
	ignoreRadius = 2000;

	CreateGroundMover(startPosInfo, 40, true, this);
	groundMover->AddAirForce(V2d(0, gravity));
	groundMover->SetSpeed(0);

	

	ts = GetSizedTileset("Enemies/W6/skunk_128x128.png");

	SetOffGroundHeight(128 / 2);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

	data.explosion.BasicCircleSetup(explosionRadius * scale, 0, V2d());
	data.explosion.hitboxInfo = hitboxInfo;

	explosionHitboxInfo.damage = 180;
	explosionHitboxInfo.drainX = 0;
	explosionHitboxInfo.drainY = 0;
	explosionHitboxInfo.hitlagFrames = 6;
	explosionHitboxInfo.hitstunFrames = 30;
	explosionHitboxInfo.knockback = 20;
	explosionHitboxInfo.hitPosType = HitboxInfo::HitPosType::OMNI;
	
	data.explosion.hitboxInfo = &explosionHitboxInfo;
	

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(0);
	cutObject->SetScale(scale);

	Color exploColor = Color::Red;
	exploColor.a = 100;
	testCircle.setFillColor(exploColor);
	testCircle.setRadius(explosionRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);

	ResetEnemy();
}

void Skunk::SetLevel(int lev)
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

void Skunk::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
	//testMover->physBody.DebugDraw(target);
}

void Skunk::ResetEnemy()
{
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	action = IDLE;

	facingRight = true;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Skunk::ActionEnded()
{
	if (frame == actionLength[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case WAKEUP:
			action = WALK;
			frame = 0;
			break;
		case WALK:
			action = HOP;
			frame = 0;
			if (facingRight)
			{
				groundMover->Jump(V2d(maxGroundSpeed, -10));
			}
			else
			{
				groundMover->Jump(V2d(-maxGroundSpeed, -10));
			}
			break;
		case HOP:
			frame = 1;
			break;
		case LAND:
			action = WALK;
			frame = 0;
			break;
		case EXPLODE:
			DefaultHurtboxesOn();
			DefaultHitboxesOn();
			action = WALK;
			frame = 0;
			break;
		}
	}
}

void Skunk::ProcessState()
{
	//cout << "vel: " << testMover->velocity.x << ", " << testMover->velocity.y << endl;
	//Actor *player = owner->GetPlayer( 0 );
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	double dist = PlayerDist();

	ActionEnded();

	switch (action)
	{
	case IDLE:
	{
		if (dist < attentionRadius)
		{
			action = WALK;
			frame = 0;
		}
		break;
	}

	case WALK:
	{
		double dist = length(playerPos - position);
		if (dist >= ignoreRadius)
		{
			action = IDLE;
			frame = 0;
		}
		break;
	}

	case HOP:
		break;
		//case ATTACK:
		//	break;
	case LAND:
		break;
	}

	switch (action)
	{
	case IDLE:
		groundMover->SetSpeed(0);
		//cout << "idle: " << frame << endl;
		break;
	case WALK:
		if (facingRight)
		{
			if (playerPos.x < position.x - 50)
			{
				facingRight = false;
			}
		}
		else
		{
			if (playerPos.x > position.x + 50)
			{
				facingRight = true;
			}
		}

		if (facingRight) //clockwise
		{
			double accelFactor = runAccel;
			if (groundMover->groundSpeed < 0)
			{
				accelFactor = runDecel;
			}
			groundMover->SetSpeed(groundMover->groundSpeed + accelFactor);
		}
		else
		{
			double accelFactor = runAccel;
			if (groundMover->groundSpeed > 0)
			{
				accelFactor = runDecel;
			}
			groundMover->SetSpeed(groundMover->groundSpeed - accelFactor);
		}

		if (groundMover->groundSpeed > maxGroundSpeed)
			groundMover->SetSpeed(maxGroundSpeed);
		else if (groundMover->groundSpeed < -maxGroundSpeed)
			groundMover->SetSpeed(-maxGroundSpeed);
		break;
	case HOP:
		//cout << "jump: " << frame << endl;
		break;
		//	case ATTACK:
		//	{
		//		testMover->SetSpeed( 0 );
		//	}
		//	break;
	case LAND:
	{
		//	cout << "land: " << frame << endl;
		//testMover->SetSpeed( 0 );
	}
	break;
	case EXPLODE:
		break;
	default:
		//cout << "WAATATET" << endl;
		break;
	}
}

void Skunk::UpdateEnemyPhysics()
{
	if (numHealth > 0) //!dead
	{
		groundMover->Move(slowMultiple, numPhysSteps);

		if (groundMover->ground == NULL)
		{
			if (groundMover->velocity.y > maxFallSpeed)
			{
				groundMover->velocity.y = maxFallSpeed;
			}
			else if (groundMover->velocity.y < -maxFallSpeed)
			{
				groundMover->velocity.y = -maxFallSpeed;
			}
		}


		V2d gn(0, -1);
		if (groundMover->ground != NULL)
		{
			gn = groundMover->ground->Normal();
		}
	}
}

void Skunk::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor)
			{
				//sess->CollectKey();
			}

			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();

			action = EXPLODE;

			if (groundMover->ground != NULL)
			{
				groundMover->SetSpeed(0);
			}
			frame = 0;
			data.explosion.SetBasicPos(GetPosition());
			SetHitboxes(&data.explosion, 0);
			HurtboxesOff();
			//sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());

		}

		

		receivedHit.SetEmpty();
	}

	//if (action != EXPLODE && !dead && ReceivedHit() && numHealth > 0)
	//{
	//	sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
	//	ConfirmHitNoKill();
	//	action = EXPLODE;
	//	frame = 0;
	//	SetHitboxes(NULL, 0);
	//	SetHurtboxes(NULL, 0);

	//	V2d dir;

	//	facingRight = !facingRight;

	//	comboObj->enemyHitboxInfo->hDir = -playerDir;//receivedHit->hDir;
	//	dir = -playerDir;
	//	velocity = dir * speed;

	//	sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());
	//}
	//else
	//{
	//	Enemy::ProcessHit();
	//}

	//might add more later to return
}


void Skunk::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}


void Skunk::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (action == EXPLODE)
	{
		target->draw(testCircle);
	}
}

void Skunk::IHitPlayer(int index)
{
	if (action == EXPLODE)
	{
		explosionHitboxInfo.kbDir = normalize(V2d(2, -1));//PlayerDir();

		if (PlayerDir().x < 0)
		{
			explosionHitboxInfo.kbDir.x = -explosionHitboxInfo.kbDir.x;
		}
		if (explosionHitboxInfo.kbDir.x == 0 && explosionHitboxInfo.kbDir.y == 0)
		{
			explosionHitboxInfo.kbDir = normalize(V2d(1, -1));
		}
	}
	
	/*V2d playerPos = sess->GetPlayerPos(index);
	if (dot(normalize(playerPos - GetPosition()), hitboxInfo->kbDir) < 0)
	{
		hitboxInfo->kbDir = -hitboxInfo->kbDir;
	}*/

	/*if (action != WAIT)
	{
		action = HITTING;
		frame = 0;
	}*/
}

void Skunk::UpdateSprite()
{

	IntRect r = ts->GetSubRect(0);
	if (!facingRight)
	{
		r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
	}

	sprite.setTextureRect(r);


	int extraVert = 50;
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height - extraVert);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(groundMover->GetAngleDegrees());

	testCircle.setPosition(GetPositionF());
}

void Skunk::HitTerrain(double &q)
{
	//cout << "hit terrain? " << endl;
}

bool Skunk::StartRoll()
{
	return false;
}

void Skunk::FinishedRoll()
{

}

void Skunk::HitOther()
{
	//cout << "hit other" << endl;
	if (action == WALK)
	{
		if ((facingRight && groundMover->groundSpeed < 0)
			|| (!facingRight && groundMover->groundSpeed > 0))
		{
			//cout << "here" << endl;
			groundMover->SetSpeed(0);
		}
		else
		{
			Hop();
		}
	}
}

void Skunk::Hop()
{
	double jumpStrength = -10;

	V2d v;
	if (facingRight)
	{
		v = V2d(maxGroundSpeed, jumpStrength);
	}
	else
	{
		v = V2d(-maxGroundSpeed, jumpStrength);
	}

	groundMover->Jump(v);

	action = HOP;
	frame = 0;
}

void Skunk::ReachCliff()
{
	if ((facingRight && groundMover->groundSpeed < 0)
		|| (!facingRight && groundMover->groundSpeed > 0))
	{
		groundMover->SetSpeed(0);
		return;
	}

	Hop();
}

void Skunk::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Skunk::Land()
{
	if (action != EXPLODE)
	{
		action = LAND;
		frame = 0;
	}
}

int Skunk::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Skunk::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Skunk::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}