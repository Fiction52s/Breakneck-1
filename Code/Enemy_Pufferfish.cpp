#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Pufferfish.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Pufferfish::Pufferfish(ActorParams *ap)
	:Enemy(EnemyType::EN_PUFFERFISH, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	bulletSpeed = 5;

	actionLength[NEUTRAL] = 4;
	actionLength[PUFF] = 1;
	actionLength[HOLDPUFF] = 1;
	actionLength[UNPUFF] = 1;
	actionLength[BLAST] = 1;
	actionLength[RECOVER] = 20;

	animFactor[NEUTRAL] = 16;//4;
	animFactor[PUFF] = 30;
	animFactor[HOLDPUFF] = 30;
	animFactor[UNPUFF] = 30;
	animFactor[BLAST] = 60;
	animFactor[RECOVER] = 1;//17 * 5;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::PUFFERFISH, 8, 8, GetPosition(), V2d(0, 1), 2 * PI, 90, false);
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->hType = HitboxInfo::ORANGE;
	launchers[0]->Reset();

	ts = GetSizedTileset("Enemies/W4/puffer_256x256.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::ORANGE;

	puffRadius = 800;
	unpuffRadius = 1000;

	BasicCircleHitBodySetup(32, 0, V2d( 0, 0 ), V2d());
	BasicCircleHurtBodySetup(32, 0, V2d( 0, 0 ), V2d());

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Pufferfish::HandleNoHealth()
{
	/*if (action == HOLDPUFF)
	{
		Fire();
	}*/

	if (action == HOLDPUFF)
	{
		cutObject->SetSubRectFront(12);
		cutObject->SetSubRectBack(11);
	}
	else
	{
		cutObject->SetSubRectFront(10);
		cutObject->SetSubRectBack(9);
	}
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Pufferfish::SetLevel(int lev)
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

void Pufferfish::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	data.velocity = V2d(0, 0);

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	hurtBody.GetCollisionBoxes(0).front().offset = V2d(0, 0);
	hitBody.GetCollisionBoxes(0).front().offset = V2d(0, 0);
	hurtBody.GetCollisionBoxes(0).front().rw = 32;
	hitBody.GetCollisionBoxes(0).front().rw = 32;


	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Pufferfish::ActionEnded()
{
	int blah = actionLength[action] * animFactor[action];
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case PUFF:
			action = HOLDPUFF;
			break;
		case HOLDPUFF:
			Fire();
			action = RECOVER;
			frame = 0;
			//action = BLAST;
			//frame = 0;
			//Fire();
			break;
		case UNPUFF:
			action = NEUTRAL;
			break;
		case BLAST:
			action = RECOVER;
			break;
		case RECOVER:
			action = NEUTRAL;
			break;
		}
	}
}

void Pufferfish::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	switch (action)
	{
	case NEUTRAL:
		if (dist < puffRadius)
		{
			action = PUFF;
			frame = 0;
		}
		break;
	case PUFF:
		
		break;
	case HOLDPUFF:
	{
		/*int numEnemiesKilledLastFrame = sess->GetPlayerEnemiesKilledLastFrame(0);

		if (numEnemiesKilledLastFrame > 0)
		{
			action = BLAST;
			frame = 0;
		}
		else */
	
		if(dist >= unpuffRadius)
		{
			action = UNPUFF;
			frame = 0;
		}
		break;
	}
	case UNPUFF:
		break;
	case BLAST:
		break;
	case RECOVER:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
	case PUFF:
		break;
	case HOLDPUFF:
		break;
	case UNPUFF:
		break;
	case BLAST:
		break;
	case RECOVER:
		break;
	}

	if (action == BLAST && frame == 1 && slowCounter == 1)// frame == 0 && slowCounter == 1 )
	{
		Fire();
	}

	if (dist < DEFAULT_DETECT_RADIUS)
	{
		data.velocity = PlayerDir() * 1.0;
	}
	else if (dist > DEFAULT_IGNORE_RADIUS)
	{
		data.velocity = V2d(0, 0);
	}
	

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	float rw = -1;
	double yOffset = 0;
	switch (action)
	{
	case NEUTRAL:
		rw = 32;
		yOffset = 0;
		//hurtBody.GetCollisionBoxes(0).front().rw = scale * 
		break;
	case PUFF:
		if (frame < 6)
		{
			rw = 48;
			yOffset = 0;
		}
		else if (frame < 15)
		{
			rw = 50;
			yOffset = 0;
		}
		else if( frame < 23 )
		{
			rw = 64;
			yOffset = -10;
		}
		else
		{
			rw = 64;
			yOffset = -10;
		}
		break;
	case HOLDPUFF:
		rw = 64;
		yOffset = -10;
		break;
	case UNPUFF:
		if (frame < 6)
		{
			rw = 64;
			yOffset = -10;
		}
		else if (frame < 15)
		{
			rw = 64;
			yOffset = -10;
		}
		else if (frame < 23)
		{
			rw = 50;
			yOffset = 0;
		}
		else
		{
			rw = 48;
			yOffset = 0;
		}
		break;
	case BLAST:


	//	tile = 9;
	//	break;
	//case RECOVER:
	//	if (frame < (actionLength[RECOVER] * animFactor[RECOVER]) / 2)
	//	{
	//		tile = 8;
	//		//tile = 3;
	//	}
	//	else
	//	{
	//		tile = 0;
	//	}

		rw = 64;
		yOffset = -10;
		break;
	case RECOVER:
		
		if (frame < (actionLength[RECOVER] * animFactor[RECOVER]) / 2)
		{
			rw = 50;
			yOffset = 0;
		}
		else
		{
			rw = 32;
			yOffset = 0;
		}

		break;
	}
	assert(rw != -1);

	hurtBody.GetCollisionBoxes(0).front().offset = V2d(0, yOffset);
	hitBody.GetCollisionBoxes(0).front().offset = V2d(0, yOffset);
	hurtBody.GetCollisionBoxes(0).front().rw = rw;
	hitBody.GetCollisionBoxes(0).front().rw = rw;
}

void Pufferfish::Fire()
{
	launchers[0]->position = GetPosition();
	launchers[0]->facingDir = V2d(1, 0);//PlayerDir();
	launchers[0]->Reset();
	launchers[0]->Fire();
}

void Pufferfish::UpdateEnemyPhysics()
{
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;
}

void Pufferfish::UpdateSprite()
{

	/*switch (action)
	{
	case NEUTRAL:
		sprite.setScale(scale, scale);
		break;
	case PUFF:
		break;
	case HOLDPUFF:
		sprite.setScale(scale * 2.0, scale * 2.0);
		break;
	case UNPUFF:
		break;
	case BLAST:
		break;
	case RECOVER:
		sprite.setScale(scale * .8, scale * .8);
		break;
	}*/

	int tile = 0;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		break;
	case PUFF:
	{
		if (frame < 6)
		{
			tile = 4;
		}
		else if (frame < 15)
		{
			tile = 5;
		}
		else if (frame < 23)
		{
			tile = 6;
		}
		else
		{
			tile = 7;
		}
		break;
	}
	case HOLDPUFF:
		tile = 7;
		break;
	case UNPUFF:
		if (frame < 6)
		{
			tile = 7;
		}
		else if (frame < 15)
		{
			tile = 6;
		}
		else if (frame < 23)
		{
			tile = 5;
		}
		else
		{
			tile = 4;
		}
		//tile = 9;
		break;
	case BLAST:
		tile = 9;
		break;
	case RECOVER:
		if (frame < (actionLength[RECOVER] * animFactor[RECOVER]) / 2)
		{
			tile = 8;
			//tile = 3;
		}
		else
		{
			tile = 0;
		}
		
		break;
	}

	ts->SetSubRect(sprite, tile, facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

int Pufferfish::GetNumStoredBytes()
{
	return sizeof(MyData) + GetNumStoredLauncherBytes();
}

void Pufferfish::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	StoreBytesForLaunchers(bytes);
	bytes += GetNumStoredLauncherBytes();
}

void Pufferfish::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	SetLaunchersFromBytes(bytes);
	bytes += GetNumStoredLauncherBytes();
}