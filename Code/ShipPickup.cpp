#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

ShipPickup::ShipPickup( GameSession *owner, Edge *g, double q, bool p_facingRight )
		:Enemy( owner, EnemyType::SHIPPICKUP, false, 1 ), ground( g ), edgeQuantity( q ),
		facingRight( p_facingRight )
{

	initHealth = 40;
	health = initHealth;

	double height = 48;
	ts = owner->GetTileset( "Ship/shippickup_128x128.png", 128, height );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint( edgeQuantity );
	//cout << "player " << owner->GetPlayer( 0 )->position.x << ", " << owner->GetPlayer( 0 )->position.y << endl;
	//cout << "gPoint: " << gPoint.x << ", " << gPoint.y << endl;
	

	receivedHit = NULL;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * height / 2.0;

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	frame = 0;
	deathFrame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 64, gPoint.y - 64, 64 * 2, 64 * 2 );

	//ts_death = owner->GetTileset( "ShipPickupdeath.png", 160, 80 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );
	//deathVector = V2d( 1, -1 );
}

void ShipPickup::ResetEnemy()
{
	//cout << "reset" << endl;
	health = initHealth;
	frame = 0;
	deathFrame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;
}

void ShipPickup::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void ShipPickup::UpdatePrePhysics()
{
	//cout << "dead: " << dead << endl;
	if( !dead && receivedHit != NULL )
	{	
		
		//gotta factor in getting hit by a clone
		health -= 20;
		//cout << "damaging: " << health << endl;
		if( health <= 0 )
		{
			//cout << "attempting. blue key is: " << owner->GetPlayer( 0 )->hasBlueKey << endl;
			//AttemptSpawnMonitor();
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}

		

		receivedHit = NULL;
	}

	if( frame == 0 )
	{
		
	}
}

void ShipPickup::UpdatePhysics()
{
	Actor *player = owner->GetPlayer( 0 );

	if( player->ground == ground )
	{
		if( ground->Normal().y == -1 )
		{
			if( abs( ( player->edgeQuantity + player->offsetX ) - edgeQuantity ) < 5 )
			{
				player->ShipPickupPoint( edgeQuantity, facingRight );
				//player->edgeQuantity = edgeQuantity;
			
			}
		}
		else
		{
			if( abs( player->edgeQuantity - edgeQuantity ) < 5 )
			{
				player->ShipPickupPoint( edgeQuantity, facingRight );
				//player->edgeQuantity = edgeQuantity;
			
			}
		}
	}
	/*specterProtected = false;
	if( PlayerSlowingMe() )
	{
		if( slowMultiple == 1 )
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}*/

	//if( !dead && receivedHit == NULL )
	//{
	//	UpdateHitboxes();

	//	pair<bool, bool> result = PlayerHitMe();
	//	if( result.first )
	//	{
	//		//cout << "hit here!" << endl;
	//		//triggers multiple times per frame? bad?
	//		owner->GetPlayer( 0 )->ConfirmHit( COLOR_BLUE, 5, .8, 6 );
	//		/*owner->GetPlayer( 0 )->test = true;
	//		owner->GetPlayer( 0 )->currAttackHit = true;
	//		owner->GetPlayer( 0 )->flashColor = COLOR_BLUE;
	//		owner->GetPlayer( 0 )->flashFrames = 5;
	//		owner->GetPlayer( 0 )->currentSpeedBar += .8;
	//		owner->GetPlayer( 0 )->swordShaders[owner->GetPlayer( 0 )->speedLevel]setParameter( "energyColor", COLOR_BLUE );
	//		owner->GetPlayer( 0 )->desperationMode = false;
	//		owner->powerBar.Charge( 2 * 6 * 3 );*/

	//		if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
	//		{
	//			owner->GetPlayer( 0 )->velocity.y = 4;//.5;
	//		}
	//	}

	//	if( IHitPlayer() )
	//	{
	//	//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
	//	}
	//}
}



void ShipPickup::UpdatePostPhysics()
{
	if( deathFrame == 30 )
	{
		
		owner->RemoveEnemy( this );
		return;
	}

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
	}

	UpdateSprite();

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	
		if( dead )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( frame == 7 * animationFactor )
	{
		frame = 0;
	}

	//cout << "dead post: " << dead << endl;
	
}

void ShipPickup::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		if( hasMonitor && !suppressMonitor )
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite, keyShader );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			target->draw( *keySprite );
		}
		else
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			
		}
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 15 )
		{
			
			//bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			//bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			//bloodSprite.setPosition( position.x, position.y );
			////bloodSprite.setScale( 2, 2 );
			//target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}
}

void ShipPickup::DrawMinimap( sf::RenderTarget *target )
{
	

	if( !dead )
	{
		if( hasMonitor && !suppressMonitor )
		{
			CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::White );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		else
		{
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
	}
}

bool ShipPickup::IHitPlayer( int index )
{
	return false;

	Actor *player = owner->GetPlayer( 0 );
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

pair<bool, bool> ShipPickup::PlayerHitMe( int index )
{
	return pair<bool,bool>( false, false );
	Actor *player = owner->GetPlayer( 0 );

	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player->recordedGhosts; ++i )
	{
		if( player->ghostFrame < player->ghosts[i]->totalRecorded )
		{
			if( player->ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}
	return pair<bool, bool>(false,false);
}

bool ShipPickup::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void ShipPickup::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( 0 ) );//frame / animationFactor ) );
	sprite.setPosition( position.x, position.y );

	if( dead )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 9 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2  );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 8 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{
		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );
		}
	}
	//sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void ShipPickup::DebugDraw(sf::RenderTarget *target)
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}



void ShipPickup::UpdateHitboxes()
{
	hurtBody.globalPosition = position - gn * 10.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position - gn * 10.0;
	hitBody.globalAngle = 0;
}

bool ShipPickup::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void ShipPickup::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
//	stored.hitlagFrames = hitlagFrames;
//	stored.hitstunFrames = hitstunFrames;
}

void ShipPickup::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
//	hitlagFrames = stored.hitlagFrames;
//	hitstunFrames = stored.hitstunFrames;
}

