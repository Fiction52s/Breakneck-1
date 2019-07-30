#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Bat.h"

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


Bat::Bat( GameSession *owner, bool p_hasMonitor, Vector2i pos, 
	list<Vector2i> &pathParam, int p_bulletSpeed,
	//int p_nodeDistance, 
	int p_framesBetweenNodes, bool p_loop )
	:Enemy( owner, EnemyType::EN_BAT, p_hasMonitor, 2 )
{
	loop = p_loop;
	//loop = false; //no looping on bat for now

	fireCounter = 0;
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	bulletSpeed = p_bulletSpeed;
	//nodeDistance = p_nodeDistance;
	framesBetween = p_framesBetweenNodes;

	startPos = V2d(pos);

	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::BAT, owner, 16, 1, position, V2d( 1, 0 ), 0, 120 );
	launchers[0]->SetBulletSpeed( bulletSpeed );	
	launchers[0]->hitboxInfo->damage = 18;
	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	pathLength = pathParam.size() + 1;
	if( loop )
	{
		//not implemented right maybe??
	}
	else
	{
		//the road back
		if( pathParam.size() > 0 )
		{
			pathLength += pathParam.size();
		}
	}
	
	path = new Vector2i[pathLength];
	path[0] = Vector2i( 0, 0 );
	path[pathLength-1] = Vector2i( 0, 0 );

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it);// +pos;
		++index;
	}

	if( pathLength == 1 )
	{

	}
	else
	{
		list<Vector2i>::reverse_iterator rit = pathParam.rbegin();
		++rit; //start at second item
		
		for(  ;rit != pathParam.rend(); ++rit )
		{
			path[index] = (*rit);// +pos;
			++index;
		}
	}

	V2d sqTest0 = position;
	V2d sqTest1 = position + V2d( 0, -150 );
	V2d sqTest2 = position + V2d( 150, -150 );
	V2d sqTest3 = position + V2d( 300, -150 );
	V2d sqTest4 = position + V2d( 300, 0 );

	for( int i = 0; i < pathLength - 1; ++i )
	{
		V2d A( path[i].x, path[i].y );
		V2d B( path[i+1].x, path[i+1].y );
		double len = length(A - B);
		double speed = 4;
		int fra = ceil( len / speed);
		//A += position;
		//B += position;
		testSeq.AddLineMovement( A, B, CubicBezier( .42,0,.58,1 ), fra * 10 );
	}
	if( pathLength == 1 )
	{
		testSeq.AddMovement( new WaitMovement( 60 * 10 ) );
	}
	testSeq.InitMovementDebug();
	testSeq.Reset();
	
	retreatMove = retreatSeq.AddLineMovement(V2d(0, 0), V2d(0, 0), CubicBezier(), 60 * 10);
	//retreatWait = new WaitMovement(60 * 10);
	//retreatSeq.AddMovement(retreatWait);
	
	returnMove = returnSeq.AddLineMovement(V2d(0, 0), V2d(0, 0), CubicBezier(), 20 * 10);


	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Bat.png", 80, 80 );
	ts = owner->GetTileset( "Enemies/bat_208x272.png", 208, 272 );
	ts_aura = owner->GetTileset("Enemies/bat_aura_208x272.png", 208, 272);
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	//position.x = 0;
	//position.y = 0;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 16;
	hurtBox.rh = 16;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 16;
	hitBox.rh = 16;
	hitBody->hitboxInfo = hitboxInfo;
	hitBody->AddCollisionBox(0, hitBox);

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	facingRight = true;
	
	ts_bulletExplode = owner->GetTileset( "FX/bullet_explode3_64x64.png", 64, 64 );

	UpdateHitboxes();

	spawnRect = sf::Rect<double>( position.x - 200, position.y - 200,
		400, 400 );

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(53);
	cutObject->SetSubRectBack(52);


	visualLength[FLAP] = 23;
	visualLength[KICK] = 29;

	visualMult[FLAP] = 2;
	visualMult[KICK] = 1;

	ResetEnemy();
}


void Bat::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );
}

void Bat::BulletHitPlayer(BasicBullet *b )
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}


void Bat::ResetEnemy()
{
	currVisual = FLAP;
	visFrame = 0;
	framesSinceBothered = 0;
	fireCounter = 0;
	testSeq.Reset();
	retreatSeq.Reset();
	returnSeq.Reset();
	dead = false;
	
	currBasePos = startPos;

	position = startPos;
	receivedHit = NULL;

	action = FLY;
	frame = 0;

	currBasePos = startPos;

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;

	visFrame = 0;
	currVisual = FLAP;
}

void Bat::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[0]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	health = 0;
	receivedHit = NULL;
}


void Bat::FrameIncrement()
{
	++fireCounter;
	++framesSinceBothered;
	++visFrame;
}

void Bat::ProcessState()
{
	if (visFrame == visualLength[currVisual] * visualMult[currVisual])
	{
		switch (currVisual)
		{
		case FLAP:
			visFrame = 0;
			break;
		case KICK:
			visFrame = 0;
			currVisual = FLAP;
			break;
		}
	}

	double detectRange = 300;
	double dodgeRange = 250;

	V2d playerPos = owner->GetPlayer(0)->position;
	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);
	if (action == FLY)
	{
		if (framesSinceBothered >= 60 && startPos != currBasePos )
		{
			action = RETURN;
			//frame = 0;
			V2d diff = startPos - position;
			returnMove->end = diff;
			double diffLen = length(diff);
			returnMove->duration = diffLen / 100.0 * 60 * 10;
			currBasePos = position;
			returnSeq.Reset();
		}
		if (length(diff) < detectRange)
		{
			framesSinceBothered = 0;
			action = RETREAT;
			//frame = 0;
			currBasePos = position;
			retreatMove->end = -pDir * dodgeRange;
			//retreatWait->pos = retreatMove->end;
			retreatSeq.Reset();
		}
		else
		{
			if (testSeq.currMovement == NULL)
			{
				testSeq.Reset();
			}
		}
	}
	else if (action == RETREAT)
	{
		if (retreatSeq.currMovement == NULL)
		{
			retreatSeq.Reset();
			testSeq.Reset();
			action = FLY;
			//frame = 0;
			currBasePos = position;
		}
	}
	else if (action == RETURN)
	{
		if (length(diff) < detectRange)
		{
			framesSinceBothered = 0;
			action = RETREAT;
			//frame = 0;
			currBasePos = position;
			retreatMove->end = -pDir * dodgeRange;
			retreatSeq.Reset();
		}
		else if (returnSeq.currMovement == NULL)
		{
			retreatSeq.Reset();
			testSeq.Reset();
			action = FLY;
			//frame = 0;
			currBasePos = position;
		}
	}


	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	if( slowCounter == 1 )//&& action == FLY )
	{
		int f = fireCounter % 60;

		if (f % 5 == 0 && f < 25)
		{
			launchers[0]->position = position;
			launchers[0]->facingDir = pDir;
			launchers[0]->Fire();
		}	
	}

	/*switch (action)
	{
	case FLY:
		cout << "fly" << endl;
		break;
	case RETREAT:
		cout << "retreat" << endl;
		break;
	case RETURN:
		cout << "return" << endl;
		break;
	}*/
}

void Bat::IHitPlayer(int index)
{
	currVisual = KICK;
	visFrame = 0;
	Actor *p = owner->GetPlayer(index);
	if (p->position.x > position.x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}
}

void Bat::UpdateEnemyPhysics()
{	
	MovementSequence *ms = NULL;
	if (action == FLY)
	{
		ms = &testSeq;
	}
	else if( action == RETREAT )
	{
		ms = &retreatSeq;
	}
	else if (action == RETURN)
	{
		ms = &returnSeq;
	}

	if (numPhysSteps == 1)
	{
		for( int i = 0; i < 10; ++i )
			ms->Update(slowMultiple);
	}
	else
	{
		ms->Update(slowMultiple);
	}

	position = currBasePos + ms->position;
	//cout << "basePos: " << currBasePos.x << ", " << currBasePos.y << "   ms: " << ms->position.x << ", " << ms->position.y << endl;
}

void Bat::UpdateSprite()
{
	int trueFrame = 0;
	switch (currVisual)
	{
	case FLAP:
		trueFrame = visFrame / visualMult[currVisual];
		break;
	case KICK:
		trueFrame = visFrame / visualMult[currVisual] + visualLength[FLAP];
		break;
	}
	sprite.setTextureRect( ts->GetSubRect( trueFrame) );
	sprite.setPosition( position.x, position.y );

	if( hasMonitor && !suppressMonitor )
	{
		//keySprite.setTexture( *ts_key->texture );
		keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
		keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
			keySprite->getLocalBounds().height / 2 );
		keySprite->setPosition( position.x, position.y );
	}

	SyncSpriteInfo(auraSprite, sprite);
}

void Bat::EnemyDraw( sf::RenderTarget *target )
{
	target->draw(auraSprite);
	DrawSpriteIfExists(target, sprite);
}

void Bat::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}

void Bat::HandleHitAndSurvive()
{
	fireCounter = 0;
}