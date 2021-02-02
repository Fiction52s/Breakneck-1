#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Turtle.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Turtle::Turtle( ActorParams *ap )
	:Enemy( EnemyType::EN_TURTLE, ap )
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	bulletSpeed = 5;

	action = NEUTRAL;

	animFactor[NEUTRAL] = 2;
	animFactor[FIRE] = 2;
	animFactor[FADEIN] = 15;
	animFactor[FADEOUT] = 5;
	animFactor[INVISIBLE] = 1;

	actionLength[NEUTRAL] = 1;
	actionLength[FIRE] = 15;
	actionLength[FADEIN] = 4;//60;
	actionLength[FADEOUT] = 17;//90;
	actionLength[INVISIBLE] = 30;

	fireCounter = 0;
	
	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::TURTLE, 12, 12, GetPosition(), V2d( 1, 0 ), PI * .5, 90, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->Reset();

	ts = sess->GetSizedTileset("Enemies/turtle_80x64.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(36);
	cutObject->SetSubRectBack(37);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = sess->GetSizedTileset("FX/bullet_explode3_64x64.png");

	ResetEnemy();
}

void Turtle::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Turtle::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 4.0;
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

void Turtle::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}

	receivedHit = NULL;
}

void Turtle::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	b->launcher->DeactivateBullet( b );
}

void Turtle::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}


void Turtle::ResetEnemy()
{
	if( GetPosition().x < sess->playerOrigPos[0].x )
		facingRight = false;
	else
		facingRight = true;
	fireCounter = 0;

	action = NEUTRAL;
	frame = 0;
	
	

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();	
}

void Turtle::ActionEnded()
{
	int blah = actionLength[action] * animFactor[action];
	//cout << "frame: " << frame << ", actionlength: " << blah << endl;
	V2d playerPos = sess->GetPlayerPos();

	if( frame == actionLength[action] * animFactor[action] )
	{
	switch( action )
	{
	case NEUTRAL:
		frame = 0;
		break;
	case FIRE:
		action = FADEOUT;
		frame = 0;
		break;
	case INVISIBLE:
		currPosInfo.position = playerTrackPos;
		if (playerPos.x < GetPosition().x)
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}
		action = FADEIN;
		frame = 0;
		break;
	case FADEIN:
		action = FIRE;
		frame = 0;
		break;
	case FADEOUT:
		action = INVISIBLE;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);
		playerTrackPos = playerPos;
		break;
	}
	}
}

void Turtle::ProcessState()
{
	ActionEnded();

	switch (action)
	{
	case NEUTRAL:
		if (PlayerDist(0) <= 600)
		{
			action = FIRE;
			frame = 0;
		}
		//cout << "NEUTRAL";
		break;
	case FIRE:
		//cout << "FIRE";
		break;
	case INVISIBLE:
		//cout << "INVISIBLE";
		break;
	case FADEIN:
		//cout << "FADEIN";
		break;
	case FADEOUT:
		//cout << "FADEOUT";
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
	case FIRE:
		break;
	case INVISIBLE:
		break;
	case FADEIN:
		if (frame == 5 && slowCounter == 1)
		{
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
		}
		break;
	case FADEOUT:
		break;
	}

	if (action == FIRE && frame == 1 && slowCounter == 1)// frame == 0 && slowCounter == 1 )
	{
		launchers[0]->position = GetPosition();
		launchers[0]->facingDir = normalize(sess->GetPlayerPos(0) - GetPosition());
		launchers[0]->Reset();
		launchers[0]->Fire();
		fireCounter = 0;
	}
}

void Turtle::UpdateEnemyPhysics()
{	
}

void Turtle::UpdateSprite()
{
	int trueFrame;
	switch( action )
	{
	case NEUTRAL:
		trueFrame = 0;
		break;
	case FIRE:
		trueFrame = frame / animFactor[FIRE] + 21;
		break;
	case INVISIBLE:
		return;
		break;
	case FADEIN:
		trueFrame = frame / animFactor[FADEIN] + 17;
		break;
	case FADEOUT:
		trueFrame = frame / animFactor[FADEOUT];
		break;
	}

	//cout << "trueFrame: " << trueFrame << ", action: " << action << endl;
	IntRect ir = ts->GetSubRect( trueFrame );
	if( !facingRight )
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}

	//sprite.setScale( 2, 2 );
	sprite.setTextureRect( ir );
	sprite.setOrigin( sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2 );
	sprite.setPosition( GetPositionF());
}

void Turtle::EnemyDraw( sf::RenderTarget *target )
{
	if( action != INVISIBLE )
		DrawSprite(target, sprite);
}