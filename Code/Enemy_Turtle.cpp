#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Turtle.h"

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


Turtle::Turtle( GameSession *owner, bool p_hasMonitor, Vector2i pos, int p_level )
	:Enemy( owner, EnemyType::EN_TURTLE, p_hasMonitor, 2 )
{
	level = p_level;

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
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;
	
	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::TURTLE, owner, 12, 12, position, V2d( 1, 0 ), 2 * PI, 90, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->Reset();

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;


	//ts = owner->GetTileset( "Turtle.png", 80, 80 );
	ts = owner->GetTileset( "Enemies/turtle_80x64.png", 80, 64 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
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

	SetupBodies(1, 1);
	AddBasicHurtCircle(16);
	AddBasicHitCircle(16);

	hitBody->hitboxInfo = hitboxInfo;
	
	dead = false;

	ResetEnemy();

	UpdateHitboxes();

	ts_bulletExplode = owner->GetTileset( "FX/bullet_explode3_64x64.png", 64, 64 );
	//cout << "finish init" << endl;
}

void Turtle::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}


void Turtle::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}

	receivedHit = NULL;
}

void Turtle::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	b->launcher->DeactivateBullet( b );
}

void Turtle::BulletHitPlayer(BasicBullet *b )
{
	owner->PlayerApplyHit(b->launcher->hitboxInfo);
}


void Turtle::ResetEnemy()
{
	if( position.x < owner->originalPos.x )
		facingRight = false;
	else
		facingRight = true;
	fireCounter = 0;

	dead = false;
	frame = 0;
	position.x = originalPos.x;
	position.y = originalPos.y;
	receivedHit = NULL;
	action = NEUTRAL;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateHitboxes();

	UpdateSprite();	
}

void Turtle::ActionEnded()
{
	int blah = actionLength[action] * animFactor[action];
	//cout << "frame: " << frame << ", actionlength: " << blah << endl;
	V2d playerPos = owner->GetPlayerPos();

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
		position = playerTrackPos;
		if (playerPos.x < position.x)
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
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
		}
		break;
	case FADEOUT:
		break;
	}

	if (action == FIRE && frame == 1 && slowCounter == 1)// frame == 0 && slowCounter == 1 )
	{
		launchers[0]->position = position;
		launchers[0]->facingDir = normalize(owner->GetPlayerPos() - position);
		launchers[0]->Reset();
		launchers[0]->Fire();
		fireCounter = 0;
	}
}

void Turtle::UpdateEnemyPhysics()
{	
	V2d playerPos = owner->GetPlayerPos(0);
	if (action == NEUTRAL)
	{
		if (length(playerPos - position) < 600)
		{
			action = FIRE;
			frame = 0;
		}
	}
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
	sprite.setPosition( position.x, position.y );
}

void Turtle::EnemyDraw( sf::RenderTarget *target )
{
	if( action != INVISIBLE )
		DrawSpriteIfExists(target, sprite);
}