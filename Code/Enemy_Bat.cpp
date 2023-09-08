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

	retreatMove = retreatSeq.AddLineMovement(V2d(0, 0), V2d(0, 0), CubicBezier(), 60);
	returnMove = returnSeq.AddLineMovement(V2d(0, 0), V2d(0, 0), CubicBezier(), 20);

	SetLevel(ap->GetLevel());

	pathFollower.SetParams(ap);

	bulletSpeed = 10;
	framesBetween = 60;

	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::BAT, 16, 1, GetPosition(), 
		V2d( 1, 0 ), 0, 120, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );	
	launchers[0]->hitboxInfo->damage = 60;
	launchers[0]->hitboxInfo->hType = HitboxInfo::GREEN;

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
	
	

	frame = 0;

	animationFactor = 5;

	ts = GetSizedTileset("Enemies/W2/bat_208x272.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 8;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::GREEN;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

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
		//retreatMove->SetFrameDuration(120);
		break;
	case 2:
		scale = 1.2;
		//retreatMove->SetFrameDuration(60);
		//maxHealth += 2;
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
	data.currVisual = FLAP;
	data.visFrame = 0;
}

void Bat::ResetEnemy()
{
	data.currVisual = FLAP;
	data.visFrame = 0;
	data.framesSinceBothered = 0;
	data.fireCounter = 0;

	data.fireConfirmed = false;

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

	data.visFrame = 0;
	data.currVisual = FLAP;
}

void Bat::FrameIncrement()
{
	if (data.fireConfirmed)
	{
		++data.fireCounter;
	}
	++data.framesSinceBothered;
	++data.visFrame;
}

void Bat::ProcessState()
{
	if ( data.visFrame == visualLength[data.currVisual] * visualMult[data.currVisual])
	{
		switch (data.currVisual)
		{
		case FLAP:
			data.visFrame = 0;
			break;
		case KICK:
			data.visFrame = 0;
			data.currVisual = FLAP;
			break;
		}
	}

	double detectRange = 300;
	double dodgeRange = 250 / 2;
	if (level == 2)
	{
		dodgeRange = 250;
	}

	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();
	V2d diff = playerPos - position;
	
	V2d startPos = startPosInfo.GetPosition();
	V2d pDir = normalize(diff);

	double returnSpeedFactor = 200.0;
	if (level == 2)
	{
		returnSpeedFactor = 400;
	}

	if (action == FLY)
	{
		if (data.framesSinceBothered >= 60 && startPos != currBasePos )
		{
			action = RETURN;
			//frame = 0;
			V2d diff = startPos - position;
			returnMove->end = diff;
			double diffLen = length(diff);
			//returnMove->duration = diffLen / 100.0 * 60 * 10;
			returnMove->SetFrameDuration(diffLen / returnSpeedFactor * 60);
			currBasePos = position;
			returnSeq.Reset();
		}
		if (length(diff) < detectRange)
		{
			data.framesSinceBothered = 0;
			action = RETREAT;
			//frame = 0;
			currBasePos = position;
			retreatMove->end = -pDir * dodgeRange;
			//retreatWait->pos = retreatMove->end;
			retreatSeq.Reset();
		}
		else
		{
			if (!testSeq.IsMovementActive())
			{
				testSeq.Reset();
			}
		}
	}
	else if (action == RETREAT)
	{
		if (!retreatSeq.IsMovementActive())
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
			data.framesSinceBothered = 0;
			action = RETREAT;
			//frame = 0;
			currBasePos = position;
			retreatMove->end = -pDir * dodgeRange;
			retreatSeq.Reset();
		}
		else if (!returnSeq.IsMovementActive())
		{
			retreatSeq.Reset();
			testSeq.Reset();
			action = FLY;
			//frame = 0;
			currBasePos = position;
		}
	}

	if (data.fireCounter == 90)
	{
		data.fireConfirmed = false;
		data.fireCounter = 0;
	}

	if (PlayerDist() < 600)
	{
		if (data.fireCounter == 0)
		{
			data.fireConfirmed = true;
		}
	}
	

	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	if( slowCounter == 1 )//&& action == FLY )
	{
		int f = data.fireCounter % 90;

		int numBulletsToShoot = 3;
		if (level == 2)
		{
			numBulletsToShoot = 5;
		}
		
		if (data.fireConfirmed)
		{
			//if (f % 5 == 0 && f >= 5 * 5 && f < 5 * (5 + numBulletsToShoot))
			if (f % 5 == 0 && f >= 5 * 0 && f < 5 * (0 + numBulletsToShoot))
			{
				launchers[0]->position = position;
				launchers[0]->facingDir = pDir;
				launchers[0]->Fire();
			}
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
	data.currVisual = KICK;
	data.visFrame = 0;
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

	ms->Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);

	currPosInfo.SetPosition(currBasePos + ms->GetPos());
	//cout << "basePos: " << currBasePos.x << ", " << currBasePos.y << "   ms: " << ms->position.x << ", " << ms->position.y << endl;
}

void Bat::UpdateSprite()
{
	int trueFrame = 0;
	switch (data.currVisual)
	{
	case FLAP:
		trueFrame = data.visFrame / visualMult[data.currVisual];
		break;
	case KICK:
		trueFrame = data.visFrame / visualMult[data.currVisual] + visualLength[FLAP];
		break;
	}
	sprite.setTextureRect( ts->GetSubRect( trueFrame) );
	sprite.setPosition( GetPositionF() );
	sprite.setOrigin(sprite.getLocalBounds().width/2, sprite.getLocalBounds().height / 2);
}

void Bat::HandleHitAndSurvive()
{
	data.fireCounter = 0;
}

int Bat::GetNumStoredBytes()
{
	return sizeof(MyData) + testSeq.GetNumStoredBytes() + retreatSeq.GetNumStoredBytes() + returnSeq.GetNumStoredBytes() + pathFollower.GetNumStoredBytes()
		+ GetNumStoredLauncherBytes();
}

void Bat::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	testSeq.StoreBytes(bytes);
	bytes += testSeq.GetNumStoredBytes();

	retreatSeq.StoreBytes(bytes);
	bytes += retreatSeq.GetNumStoredBytes();

	returnSeq.StoreBytes(bytes);
	bytes += returnSeq.GetNumStoredBytes();

	pathFollower.StoreBytes(bytes);
	bytes += pathFollower.GetNumStoredBytes();

	StoreBytesForLaunchers(bytes);
	bytes += GetNumStoredLauncherBytes();
}

void Bat::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	testSeq.SetFromBytes(bytes);
	bytes += testSeq.GetNumStoredBytes();

	retreatSeq.SetFromBytes(bytes);
	bytes += retreatSeq.GetNumStoredBytes();

	returnSeq.SetFromBytes(bytes);
	bytes += returnSeq.GetNumStoredBytes();

	pathFollower.SetFromBytes(bytes);
	bytes += pathFollower.GetNumStoredBytes();

	SetLaunchersFromBytes(bytes);
	bytes += GetNumStoredLauncherBytes();
}