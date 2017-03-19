#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Sequence.h"

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

Nexus::Nexus( GameSession *owner, Edge *g, double q, int nexusIndex )
		:Enemy( owner, EnemyType::NEXUS, false, 1 ), ground( g ), edgeQuantity( q )
{
	enterNexus1Seq = new EnterNexus1Seq( owner );

	action = NORMAL;
	initHealth = 40;
	health = initHealth;

	double height = 1024;
	ts = owner->GetTileset( "Nexus/nexus_w1_1024x1024.png", 1024, 1024 );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint( edgeQuantity );
	//cout << "player " << owner->player->position.x << ", " << owner->player->position.y << endl;
	//cout << "gPoint: " << gPoint.x << ", " << gPoint.y << endl;

	receivedHit = NULL;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * 200.0;//height / 2.0;

	entrancePos = gPoint + gn * 200.0;

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );// / 2 );
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

	spawnRect = sf::Rect<double>( gPoint.x - 800, gPoint.y - 800, 800 * 2, 800 * 2 );

	//ts_death = owner->GetTileset( "foottrapdeath.png", 160, 80 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );
	//deathVector = V2d( 1, -1 );

	ResetEnemy();
}

void Nexus::ResetEnemy()
{
	action = NORMAL;
	//cout << "reset" << endl;
	health = initHealth;
	frame = 0;
	deathFrame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;

	enterNexus1Seq->Reset();
}

void Nexus::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Nexus::UpdatePrePhysics()
{
	//cout << "dead: " << dead << endl;
	if( !dead && receivedHit != NULL )
	{	
		
		//gotta factor in getting hit by a clone
		health -= 20;
		//cout << "damaging: " << health << endl;
		if( health <= 0 )
		{
			//cout << "attempting. blue key is: " << owner->player->hasBlueKey << endl;
			//AttemptSpawnMonitor();
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}

		

		receivedHit = NULL;
	}

	if( frame == 0 )
	{
		
	}
}

void Nexus::UpdatePhysics()
{
	specterProtected = false;
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
	}

	if( action == NORMAL && length( owner->player->position - entrancePos ) < 30.0 )
	{
		action = ENTERED;
		owner->player->EnterNexus( 0, entrancePos );
		owner->activeSequence = enterNexus1Seq;
		enterNexus1Seq->nexus = this;
	}

	if( !dead && receivedHit == NULL )
	{
		//UpdateHitboxes();

		//pair<bool, bool> result = PlayerHitMe();
		//if( result.first )
		//{
		//	//cout << "hit here!" << endl;
		//	//triggers multiple times per frame? bad?
		//	owner->player->ConfirmHit( COLOR_BLUE, 5, .8, 6 );
		//	/*owner->player->test = true;
		//	owner->player->currAttackHit = true;
		//	owner->player->flashColor = COLOR_BLUE;
		//	owner->player->flashFrames = 5;
		//	owner->player->currentSpeedBar += .8;
		//	owner->player->swordShaders[owner->player->speedLevel]setParameter( "energyColor", COLOR_BLUE );
		//	owner->player->desperationMode = false;
		//	owner->powerBar.Charge( 2 * 6 * 3 );*/

		//	if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
		//	{
		//		owner->player->velocity.y = 4;//.5;
		//	}
		//}

		//if( IHitPlayer() )
		//{
		////	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		//}
	}
}

void Nexus::UpdatePostPhysics()
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
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
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

void Nexus::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		target->draw( sprite );
		/*if( hasMonitor && !suppressMonitor )
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
			
		}*/
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

void Nexus::DrawMinimap( sf::RenderTarget *target )
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

bool Nexus::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

pair<bool, bool> Nexus::PlayerHitMe()
{
	Actor *player = owner->player;

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

bool Nexus::PlayerSlowingMe()
{
	Actor *player = owner->player;
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

void Nexus::UpdateSprite()
{
	//sprite.setTextureRect( ts->GetSubRect( 0 ) );

	

	//sprite.setPosition( position.x, position.y );

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
		//if( hasMonitor && !suppressMonitor )
		//{
		//	//keySprite.setTexture( *ts_key->texture );
		//	keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
		//	keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
		//		keySprite->getLocalBounds().height / 2 );
		//	keySprite->setPosition( position.x, position.y );

		//}
	}
	//sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void Nexus::DebugDraw(sf::RenderTarget *target)
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Nexus::UpdateHitboxes()
{
	hurtBody.globalPosition = position - gn * 10.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position - gn * 10.0;
	hitBody.globalAngle = 0;
}

bool Nexus::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void Nexus::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
//	stored.hitlagFrames = hitlagFrames;
//	stored.hitstunFrames = hitstunFrames;
}

void Nexus::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
//	hitlagFrames = stored.hitlagFrames;
//	hitstunFrames = stored.hitstunFrames;
}

