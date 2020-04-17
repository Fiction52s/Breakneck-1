#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Comboer.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


Comboer::Comboer(Vector2i pos, list<Vector2i> &pathParam, bool loopP, 
	int p_level)
	:Enemy(EnemyType::EN_COMBOER, false, 1, false)
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

	action = S_FLOAT;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	pathLength = pathParam.size() + 1;
	//cout << "pathLength: " << pathLength << endl;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for (list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it)
	{
		path[index] = (*it) + pos;
		++index;
		//path.push_back( (*it) );

	}

	loop = loopP;

	//eventually maybe put this on a multiplier for more variation?
	//doubt ill need it though
	speed = 15;//pspeed;
	frame = 0;
	
	//ts = owner->GetTileset( "Comboer.png", 80, 80 );
	ts = sess->GetTileset("Enemies/comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
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
	//hitboxInfo->kbDir;

	targetNode = 1;
	forward = true;

	dead = false;

	facingRight = true;

	

	//bloodSprite.setTexture( *ts_testBlood->texture );

	//UpdateHitboxes();

	actionLength[S_FLOAT] = 18;
	actionLength[S_SHOT] = 3;
	actionLength[S_EXPLODE] = 20;

	animFactor[S_FLOAT] = 2;
	animFactor[S_SHOT] = 6;
	animFactor[S_EXPLODE] = 1;


	shootLimit = 120;
	hitLimit = 6;

	ResetEnemy();
}

//Comboer::Comboer(GameSession *owner, std::ifstream &is)
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
//	Comboer *enemy = new Comboer(this, hasMonitor, Vector2i(xPos, yPos), localPath, loop, speed, Comboer::ComboerType::T_STRAIGHT);
//}

void Comboer::ResetEnemy()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	shootFrames = 0;
	currHits = 0;
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	velocity = V2d(0, 0);
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	targetNode = 1;
	forward = true;
	dead = false;
	action = S_FLOAT;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;


	UpdateHitboxes();

	UpdateSprite();
}

void Comboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		action = S_SHOT;
		frame = 0;
		//SetHitboxes(hitBody, 0);
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;

		comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		dir = normalize(receivedHit->hDir);
		/*switch (receivedHit->hDir)
		{
		case HitboxInfo::LEFT:
			dir = V2d(-1, 0);
			break;
		case HitboxInfo::RIGHT:
			dir = V2d(1, 0);
			break;
		case HitboxInfo::UP:
			dir = V2d(0, -1);
			break;
		case HitboxInfo::DOWN:
			dir = V2d(0, 1);
			break;
		case HitboxInfo::UPLEFT:
			dir = V2d(-1, -1);
			break;
		case HitboxInfo::UPRIGHT:
			dir = V2d(1, -1);
			break;
		case HitboxInfo::DOWNLEFT:
			dir = V2d(-1, 1);
			break;
		case HitboxInfo::DOWNRIGHT:
			dir = V2d(1, 1);
			break;
		default: 
			assert(0);
		
		}*/


		if (dir.x != 0 && dir.y != 0 )
		{
			double absX = abs(dir.x);
			double absY = abs(dir.y);
			if (absX - absY > -.01 )
			{
				dir.y = 0;
			}
			else
			{
				dir.x = 0;
			}
		}

		dir = normalize(dir);



		velocity = dir * speed;

		IntRect ir;
		
		

		if (velocity.x == 0 || velocity.y == 0)
		{
			ir = ts->GetSubRect(1);
			sprite.setTextureRect(ts->GetSubRect(1));
		}
		else
		{
			ir = ts->GetSubRect(2);
			
		}

		if( velocity.x < 0 )
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
			else if( velocity.x < 0 && velocity.y > 0 )
			{
				sprite.setRotation(-90);
			}
		}

		sprite.setTextureRect(ir);

		/*if (velocity.y > 0 && velocity.x < 0)
		{
			sprite.setTextureRect(sf::IntRect(ir.left, ir.top, -ir.width, ir.height));
		}*/


		sess->PlayerAddActiveComboObj(comboObj);
	}
}

void Comboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			sess->PlayerRemoveActiveComboer(comboObj);
			break;
		}
	}

	//V2d playerPos = owner->GetPlayer(0)->position;
}

void Comboer::HandleNoHealth()
{
	
}

void Comboer::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLOAT:
	{
		double movement = speed / numPhysSteps;

		if (pathLength > 1)
		{
			movement /= (double)slowMultiple;

			while (movement != 0)
			{
				//cout << "movement loop? "<< endl;
				V2d targetPoint = V2d(path[targetNode].x, path[targetNode].y);
				V2d diff = targetPoint - position;
				double len = length(diff);
				if (len >= abs(movement))
				{
					position += normalize(diff) * movement;
					movement = 0;
				}
				else
				{
					position += diff;
					movement -= length(diff);
					AdvanceTargetNode();
				}
			}
		}
		break;
	}
	case S_SHOT:
	{
		
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		position += movementVec;
		break;
	}
	}

	comboObj->enemyHitboxInfo->hDir = normalize(velocity);
}

void Comboer::AdvanceTargetNode()
{
	if (loop)
	{
		++targetNode;
		if (targetNode == pathLength)
			targetNode = 0;
	}
	else
	{
		if (forward)
		{
			++targetNode;
			if (targetNode == pathLength)
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if (targetNode < 0)
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void Comboer::FrameIncrement()
{
	/*if (action == CHARGEDFLAP)
	{
	++fireCounter;
	}*/
	if (action == S_SHOT)
	{
		if (shootFrames == shootLimit)
		{
			action = S_EXPLODE;
			frame = 0;
		}
		else
		{
			++shootFrames;
		}
	}
}

void Comboer::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void Comboer::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);
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

void Comboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}