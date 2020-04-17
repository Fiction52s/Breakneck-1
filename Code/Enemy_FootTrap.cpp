#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_FootTrap.h"

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

FootTrap::FootTrap( bool p_hasMonitor, Edge *g, double q )
		:Enemy( EnemyType::EN_FOOTTRAP, p_hasMonitor, 1 ), ground( g ), edgeQuantity( q )
{
	action = LATENT;

	double height = 128;
	ts = sess->GetTileset( "Enemies/foottrap_160x128.png", 160, height );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint( edgeQuantity );

	receivedHit = NULL;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * height / 2.0;

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	hurtBody = new CollisionBody(1); //this wille ventually match animation
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	hitBody->hitboxInfo = hitboxInfo;

	frame = 0;
	//deathFrame = 0;
	animationFactor = 2;

	spawnRect = sf::Rect<double>( gPoint.x - 64, gPoint.y - 64, 64 * 2, 64 * 2 );

	actionLength[LATENT] = 25 * animationFactor;
	actionLength[CHOMPING] = 25 * animationFactor;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateSprite();
	/*actionLength[DYING] = 60;
	actionLength[ROOTPREPARE] = 60;
	actionLength[ROOTWAIT] = 20;
	actionLength[ROOTSTRIKE] = 20;
	actionLength[ROOTSPIKEDYING] = 30;*/
		//deathVector = V2d( 1, -1 );
}

void FootTrap::ResetEnemy()
{
	action = LATENT;
	//cout << "reset" << endl;
	frame = 0;
	//deathFrame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
}

void FootTrap::ProcessState()
{
	if (frame == actionLength[action])
	{
		switch (action)
		{
		case LATENT:
			frame = 0;
			break;
		case CHOMPING:
			frame = 0;
			break;
		//case DYING:
		//	action = ROOTPREPARE;
		//	frame = 0;
		//	break;
		//case ROOTPREPARE:
		//	action = ROOTWAIT;
		//	frame = 0;
		//	break;
		//case ROOTWAIT:
		//	frame = 0;
		//	break;
		//case ROOTSTRIKE:
		//	action = ROOTSPIKEWAIT;
		//	frame = 0;
		//	break;
		//case ROOTSPIKEWAIT:
		//	frame = 0;
		//	break;
		//case ROOTSPIKEDYING:
		//	owner->RemoveEnemy(this);
		//	return;
		//	//actually dead
		//	break;
		}
	}


	V2d playerPos = sess->GetPlayerPos(0);
	switch (action)
	{
	case LATENT:
		if (length(playerPos - position) < 450)
		{
			action = CHOMPING;
			frame = 0;
		}
		break;
	case CHOMPING:
		if (length(playerPos - position) > 450)
		{
			action = LATENT;
			frame = 0;
		}
		break;
	/*case DYING:
		break;
	case ROOTPREPARE:
		break;
	case ROOTWAIT:
		if (length(owner->GetPlayer(0)->position - position) < 200)
		{
			action = ROOTSTRIKE;
			frame = 0;
		}
		break;
	case ROOTSTRIKE:
		break;
	case ROOTSPIKEWAIT:
		break;
	case ROOTSPIKEDYING:
		break;*/
	}
	//cout << "dead: " << dead << endl;
}

void FootTrap::EnemyDraw(sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}

void FootTrap::UpdateSprite()
{
	switch (action)
	{
	case LATENT:
		sprite.setTextureRect(ts->GetSubRect((frame / animationFactor) % 3));
		sprite.setPosition(position.x, position.y);
		break;
	case CHOMPING:
		sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
		sprite.setPosition(position.x, position.y);
		break;
	}
	//if( action == ROOTSPIKEDYING )
	//{
	//	
	//}
	//else
	//{
	//	
	//	case DYING:
	//		botDeathSprite.setTexture(*ts->texture);
	//		botDeathSprite.setTextureRect(ts->GetSubRect(26));
	//		botDeathSprite.setOrigin(botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
	//		botDeathSprite.setPosition(position.x + deathVector.x * deathPartingSpeed * frame,
	//			position.y + deathVector.y * deathPartingSpeed * frame);
	//		botDeathSprite.setRotation(sprite.getRotation());

	//		topDeathSprite.setTexture(*ts->texture);
	//		topDeathSprite.setTextureRect(ts->GetSubRect(25));
	//		topDeathSprite.setOrigin(topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2);
	//		topDeathSprite.setPosition(position.x + -deathVector.x * deathPartingSpeed * frame,
	//			position.y + -deathVector.y * deathPartingSpeed * frame);
	//		topDeathSprite.setRotation(sprite.getRotation());
	//		break;
	//	case ROOTPREPARE:
	//		sprite.setTextureRect(ts->GetSubRect(0));
	//		sprite.setPosition(position.x, position.y);
	//		break;
	//	case ROOTWAIT:
	//		sprite.setTextureRect(ts->GetSubRect(4));
	//		sprite.setPosition(position.x, position.y);
	//		break;
	//	case ROOTSTRIKE:
	//		sprite.setTextureRect(ts->GetSubRect(10));
	//		sprite.setPosition(position.x, position.y);
	//		break;
	//	case ROOTSPIKEWAIT:
	//		sprite.setTextureRect(ts->GetSubRect(15));
	//		sprite.setPosition(position.x, position.y);
	//		break;
	//	case ROOTSPIKEDYING:
	//		sprite.setTextureRect(ts->GetSubRect(20));
	//		sprite.setPosition(position.x, position.y);
	//		break;
	//	}
	//	

		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( sess->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );
		}
	//}
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

