#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Juggler.h"
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_JugglerCatcher.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


Juggler::Juggler(GameSession *owner, bool p_hasMonitor, Vector2i pos, int p_level)
	:Enemy(owner, EnemyType::EN_JUGGLER, p_hasMonitor, 1, false)
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

	juggleReps = 100;

	action = S_FLOAT;
	position.x = pos.x;
	position.y = pos.y;

	origPos = position;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	frame = 0;

	//ts = owner->GetTileset( "Juggler.png", 80, 80 );
	ts = owner->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);
	sprite.setColor(Color::Red);


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

	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	dead = false;

	actionLength[S_FLOAT] = 18;
	actionLength[S_POP] = 10;
	actionLength[S_JUGGLE] = 10;
	

	actionLength[S_RETURN] = 3;
	actionLength[S_EXPLODE] = 20;

	animFactor[S_FLOAT] = 2;
	animFactor[S_JUGGLE] = 1;
	animFactor[S_RETURN] = 6;
	animFactor[S_EXPLODE] = 1;

	gravFactor = 1.0;
	maxFallSpeed = 15;

	reversed = true;

	ResetEnemy();
}

//Juggler::Juggler(GameSession *owner, std::ifstream &is)
//{
//	int xPos, yPos;
//
//	is >> xPos;
//	is >> yPos;
//
//	int hasMonitor;
//	is >> hasMonitor;
//
//	int pathLength;
//	list<Vector2i> localPath;
//	ReadPath(is, pathLength, localPath);
//
//	bool loop;
//	ReadBool(is, loop);
//
//	int speed;
//	is >> speed;
//
//	//Airdasher *enemy = new Airdasher(this, hasMonitor, Vector2i(xPos, yPos));
//	Juggler *enemy = new Juggler(this, hasMonitor, Vector2i(xPos, yPos), localPath, loop, speed, Juggler::JugglerType::T_STRAIGHT);
//}

void Juggler::ResetEnemy()
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
	UpdateHitboxes();

	currJuggle = 0;

	UpdateSprite();
}

void Juggler::Throw(double a, double strength)
{
	V2d vel(strength, 0);
	RotateCCW(vel, a);
	velocity = vel;
}

void Juggler::Throw(V2d vel)
{
	velocity = vel;
}

void Juggler::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		if (action == S_POP)
			return;

		
		numHealth -= 1;

		Actor *player = owner->GetPlayer(0);
		if (numHealth <= 0)
		{
			if (currJuggle == juggleReps)
			{
				if (hasMonitor && !suppressMonitor)
				{
					owner->keyMarker->CollectKey();
				}

				player->ConfirmEnemyKill(this);
				ConfirmKill();
			}
			else
			{
				player->ConfirmEnemyNoKill(this);
				ConfirmHitNoKill();

				numHealth = maxHealth;
				++currJuggle;
				action = S_POP;
				frame = 0;

				SetHitboxes(NULL, 0);



				V2d dir = receivedHit->hDir;

				comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;
				
				V2d hit(0, -20);

				double extraX = 8;

				if (dir.x != 0)
				{
					hit.x += dir.x * extraX;
				}
				if (dir.y == 1 && dir.x == 0 )
				{
					hit.y += 3;
					if (player->facingRight)
					{
						hit.x -= extraX / 2.0;
					}
					else
					{
						hit.x += extraX / 2.0;
					}
					
				}

				
				//Throw(GetVectorAngleCCW(dir), 15);
				Throw(hit);
				

				//IntRect ir;



				/*if (velocity.x == 0 || velocity.y == 0)
				{
					ir = ts->GetSubRect(1);
					sprite.setTextureRect(ts->GetSubRect(1));
				}
				else
				{
					ir = ts->GetSubRect(2);

				}

				if (velocity.x < 0)
				{
					ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
				}
				if (velocity.x == 0)
				{
					if (velocity.y > 0)
					{
						sprite.setRotation(90);
					}
					else
					{
						sprite.setRotation(-90);
					}
				}
				else
				{
					if (velocity.x > 0 && velocity.y > 0)
					{
						sprite.setRotation(90);
					}
					else if (velocity.x < 0 && velocity.y > 0)
					{
						sprite.setRotation(-90);
					}
				}*/

				//sprite.setTextureRect(ir);

				//owner->GetPlayer(0)->AddActiveComboObj(comboObj);
			}

		}
		else
		{
			owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
	}
}

void Juggler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
			break;
		}
	}

	if (action != S_FLOAT && action != S_EXPLODE && action != S_RETURN)
	{
		sf::Rect<double> r(position.x - 50, position.y - 50, 100, 100);
		owner->activeEnemyItemTree->Query(this, r);
	}
	

	if (action == S_POP && velocity.y >= 0)
	{
		action = S_JUGGLE;
		frame = 0;
	}

	V2d playerPos = owner->GetPlayer(0)->position;
}

void Juggler::HandleNoHealth()
{

}

void Juggler::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLOAT:
	{
		break;
	}
	case S_POP:
	case S_JUGGLE:
	{
		double f;
		double numStep = numPhysSteps;
		V2d movementVec = velocity;
		movementVec /= slowMultiple * numStep;

		position += movementVec;

		//cout << "movementVec: " << movementVec.y << " velocity: " << velocity.y << "\n";

		V2d gDir(0, 1);//normalize(owner->GetPlayer(0)->position - position);

		velocity +=  gDir * (gravFactor / numStep / slowMultiple);

		//if (length(velocity) > 40.0)
		//{
		//	velocity = normalize(velocity) * 40.0;
		//}
		//velocity.y += gravFactor / numStep;

		if (velocity.y > maxFallSpeed)
		{
			velocity.y = maxFallSpeed;
		}

		break;
	}
	}


}

void Juggler::FrameIncrement()
{
	if (action == S_JUGGLE)
	{
		/*{
			action = S_FLOAT;
			frame = 0;
			owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			position = endDashPos;
		}*/
	}
	/*if (action == S_DASH)
	{
	dashStep +=
	}*/
	/*if (action == CHARGEDFLAP)
	{
	++fireCounter;
	}*/
	//if (action == S_DASH)
	//{
	//	if (frame == dashFrames)
	//	{
	//		action = S_FLOAT;
	//		frame = 0;
	//	}
	//	else
	//	{
	//		//++shootFrames;
	//	}
	//}
}

void Juggler::HandleEntrant(QuadTreeEntrant *qte)
{
	Enemy *en = (Enemy*)qte;
	if (en->type == EnemyType::EN_JUGGLERCATCHER)
	{
		JugglerCatcher *catcher = (JugglerCatcher*)qte;

		CollisionBox &hitB = hurtBody->GetCollisionBoxes(0)->front();
		if (catcher->CanCatch() && catcher->hurtBody->Intersects(catcher->currHurtboxFrame, &hitB))
		{
			catcher->Catch();
			action = S_EXPLODE;
			frame = 0;
		}
	}
}

void Juggler::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void Juggler::UpdateSprite()
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
	/*int tIndex = 0;
	switch (action)
	{
	case S_FLOAT:
	tIndex = 0;
	break;
	case S_SHOT:

	break;
	case S_EXPLODE:
	break;
	}
	sprite.setTextureRect(ts->GetSubRect(frame));*/
}

void Juggler::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

CollisionBox &Juggler::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void Juggler::UpdateHitboxes()
{
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
}