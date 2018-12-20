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


Turtle::Turtle( GameSession *owner, bool p_hasMonitor, Vector2i pos )
	:Enemy( owner, EnemyType::EN_TURTLE, p_hasMonitor, 2 )
{
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

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Turtle.png", 80, 80 );
	ts = owner->GetTileset( "turtle_80x64.png", 80, 64 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(36);
	cutObject->SetSubRectBack(37);
	//position.x = 0;
	//position.y = 0;
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
	hitBody->AddCollisionBox(0, hitBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hitBody->hitboxInfo = hitboxInfo;
	
	dead = false;

	ResetEnemy();

	UpdateHitboxes();

	slowCounter = 1;
	slowMultiple = 1;

	ts_bulletExplode = owner->GetTileset( "bullet_explode3_64x64.png", 64, 64 );
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
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
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

		if( owner->GetPlayer( 0 )->position.x < position.x )
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}
		position = owner->GetPlayer( 0 )->position;
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
		launchers[0]->facingDir = normalize(owner->GetPlayer(0)->position - position);
		launchers[0]->Reset();
		launchers[0]->Fire();
		fireCounter = 0;
	}
}

void Turtle::UpdateEnemyPhysics()
{	
	if (action == NEUTRAL)
	{
		Actor *player = owner->GetPlayer(0);
		if (length(player->position - position) < 600)
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

	sprite.setScale( 2, 2 );
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

void Turtle::UpdateHitboxes()
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