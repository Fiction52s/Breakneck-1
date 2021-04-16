#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Bat.h"
#include "Actor.h"

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


Bat::Bat( ActorParams *ap )
	:Enemy( EnemyType::EN_BAT, ap )
{
	SetNumActions(A_Count);
	SetEditorActions(FLY, 0, 0);

	SetLevel(ap->GetLevel());

	pathFollower.SetParams(ap);

	bulletSpeed = 10;
	framesBetween = 60;

	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::BAT, 16, 1, GetPosition(), 
		V2d( 1, 0 ), 0, 120, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );	
	launchers[0]->hitboxInfo->damage = 18;

	int pSize = pathFollower.path.size();
	for( int i = 0; i < pSize - 1; ++i )
	{
		V2d A( pathFollower.path[i].x, pathFollower.path[i].y );
		V2d B(pathFollower.path[i+1].x, pathFollower.path[i+1].y );
		double len = length(A - B);
		double speed = 4;
		int fra = ceil( len / speed);
		testSeq.AddLineMovement( A, B, CubicBezier( .42,0,.58,1 ), fra );
	}
	if( pSize == 1 )
	{
		testSeq.AddMovement( new WaitMovement( 60 ) );
	}
	testSeq.InitMovementDebug();
	testSeq.Reset();
	
	retreatMove = retreatSeq.AddLineMovement(V2d(0, 0), V2d(0, 0), CubicBezier(), 60);	
	returnMove = returnSeq.AddLineMovement(V2d(0, 0), V2d(0, 0), CubicBezier(), 20);

	frame = 0;

	animationFactor = 5;

	ts = sess->GetSizedTileset("Enemies/W2/bat_208x272.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 8;
	hitboxInfo->kbDir = V2d(1, 0);

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = sess->GetTileset( "FX/bullet_explode3_64x64.png", 64, 64 );

	cutObject->Setup(ts, 53, 52, scale);

	visualLength[FLAP] = 23;
	visualLength[KICK] = 29;

	visualMult[FLAP] = 2;
	visualMult[KICK] = 1;

	ResetEnemy();
}

void Bat::SetLevel(int lev)
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

void Bat::SetActionEditLoop()
{
	Enemy::SetActionEditLoop();
	currVisual = FLAP;
	visFrame = 0;
}

void Bat::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );
}

void Bat::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
	
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
	facingRight = true;
	
	currBasePos = startPosInfo.GetPosition();

	action = FLY;
	frame = 0;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	UpdateHitboxes();

	UpdateSprite();

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
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
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

	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();
	V2d diff = playerPos - position;
	
	V2d startPos = startPosInfo.GetPosition();
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
		int f = fireCounter % 90;

		if (f % 5 == 0 && f >= 25 && f < 50)
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
	//Actor *p = owner->GetPlayer(index);
	V2d playerPos = sess->GetPlayerPos(index);
	if (playerPos.x > GetPosition().x)
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
		ms->Update(slowMultiple, 10);
	}
	else
	{
		ms->Update(slowMultiple);
	}

	currPosInfo.SetPosition(currBasePos + ms->position);
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
	sprite.setPosition( GetPositionF() );
	sprite.setOrigin(sprite.getLocalBounds().width/2, sprite.getLocalBounds().height / 2);
}

void Bat::EnemyDraw( sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}

void Bat::HandleHitAndSurvive()
{
	fireCounter = 0;
}