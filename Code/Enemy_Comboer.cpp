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


Comboer::Comboer(GameSession *owner, bool p_hasMonitor, Vector2i pos, list<Vector2i> &pathParam, bool loopP, int pspeed,
	ComboerType t)
	:Enemy(owner, EnemyType::EN_COMBOER, p_hasMonitor, 1, false), cType( t )
{
	cType = T_GRAVITY;
	switch (cType)
	{
	case T_STRAIGHT:
		break;
	case T_GRAVITY:
		sprite.setColor(Color::Green);
		break;
	case T_REVERSEGRAVITY:
		sprite.setColor(Color::Red);
		break;
	case T_BOUNCE:
		sprite.setColor(Color::Yellow);
		mover = new SurfaceMover(owner, NULL, 0, 64);
		mover->surfaceHandler = this;
		break;
	}

	action = S_FLOAT;
	gravFactor = .8;
	maxFallSpeed = 30;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	initHealth = 80;
	health = initHealth;

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
	ts = owner->GetTileset("Enemies/comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 48;
	hurtBox.rh = 48;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 48;
	hitBox.rh = 48;
	hitBody = new CollisionBody(1);
	hitBody->AddCollisionBox(0, hitBox);

	


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;


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
	comboObj->enemyHitBody->AddCollisionBox(0, hitBox);

	comboObj->enemyHitboxFrame = 0;

	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	//hitboxInfo->kbDir;

	targetNode = 1;
	forward = true;

	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	facingRight = true;

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );

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

	if (cType == T_BOUNCE)
	{
		hitLimit = 12;
	}

	ResetEnemy();
}

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

	if (cType == T_BOUNCE)
	{
		mover->velocity = V2d(0, 0);
		mover->ground = NULL;
		mover->physBody.globalPosition = position;
	}

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
}

void Comboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		action = S_SHOT;
		frame = 0;
		//SetHitboxes(hitBody, 0);
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;

		comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		switch (receivedHit->hDir)
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
		
		}

		if (cType == T_GRAVITY)
		{
			//dir.y -= 1.0;
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

		if (cType == T_GRAVITY)
		{
			velocity.y -= speed / 2;
		}
		else if (cType == T_REVERSEGRAVITY)
		{
			velocity.y += speed / 2;
		}
		
		

		owner->GetPlayer(0)->AddActiveComboObj(comboObj);
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
			owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
			break;
		}
	}

	V2d playerPos = owner->GetPlayer(0)->position;
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

		if (cType == T_BOUNCE)
		{
			mover->velocity = velocity;
			mover->Move(slowMultiple, numPhysSteps);
			position = mover->physBody.globalPosition;
		}
		else
		{
			position += movementVec;
		}
		

		if (cType == T_GRAVITY)
		{
			//double len = length(velocity);
			velocity += V2d(0, gravFactor / (slowMultiple * (double)numPhysSteps) );
			//velocity = normalize(velocity) * len;

			if (velocity.y > maxFallSpeed)
			{
				velocity.y = maxFallSpeed;
			}
		}
		else if (cType == T_REVERSEGRAVITY)
		{
			velocity += V2d(0, -gravFactor / (slowMultiple * (double)numPhysSteps));
			//velocity = normalize(velocity) * len;

			if (velocity.y < -maxFallSpeed)
			{
				velocity.y = -maxFallSpeed;
			}
		}


		
		break;
	}
	}


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

CollisionBox &Comboer::GetEnemyHitbox()
{
	return comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front();
}

void Comboer::UpdateHitboxes()
{
	if (cType == T_BOUNCE)
	{
		position = mover->physBody.globalPosition;
	}

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

void Comboer::HitTerrainAerial(Edge * edge, double quant)
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
		//b->bounceCount++;
		//b->framesToLive = b->launcher->maxFramesToLive;
	}
}