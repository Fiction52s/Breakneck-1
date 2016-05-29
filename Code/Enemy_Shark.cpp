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


Shark::Shark( GameSession *owner, Vector2i pos, float pspeed )
	:Enemy( owner, EnemyType::SHARK ), deathFrame( 0 ), approachAccelBez( 1,.01,.86,.32 ) 
{
	latchedOn = false;
	//offsetPlayer 
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	origPosition = position;

	approachFrames = 180 * 3;
	totalFrame = 0;

	//latchedOn = true; 
	V2d dirFromPlayer = normalize( owner->player.position - position );
	double fromPlayerAngle =  atan2( dirFromPlayer.y, dirFromPlayer.x ) + PI;
	cout << "dirfrom: " << dirFromPlayer.x << ", " << dirFromPlayer.y << endl;
	cout << "from player angle: " << fromPlayerAngle << endl;
	testSeq.AddRadialMovement( 1, 0, 2 * PI * 3, 
		true, V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1), approachFrames );
	
	testSeq.InitMovementDebug();

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	basePos = position;
	
	speed = pspeed;

	//speed = 2;
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Shark.png", 80, 80 );
	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	//position.x = 0;
	//position.y = 0;
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 16;
	hitBody.rh = 16;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	
	//hitboxInfo->kbDir;

	
	latchStartAngle = 0;
	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();
}

void Shark::HandleEntrant( QuadTreeEntrant *qte )
{

}

void Shark::ResetEnemy()
{
	latchStartAngle = 0;
	latchedOn = false;
	totalFrame = 0;
	testSeq.Reset();
	//testSeq.Update();
	dead = false;
	deathFrame = 0;
	frame = 0;
	basePos = origPosition;
	position = basePos;

	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Shark::UpdatePrePhysics()
{
	if( !dead && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			AttemptSpawnMonitor();
			dead = true;
		}

		receivedHit = NULL;
	}

	
}

void Shark::UpdatePhysics()
{
	if( latchedOn )
	{
		basePos = owner->player.position;// + offsetPlayer;
	}
	else
	{
		if( length( basePos - owner->player.position ) < 200 )
		{
			cout << "JUST LATCHING NOW" << endl;
			latchedOn = true;
			offsetPlayer = basePos - owner->player.position;//owner->player.position - basePos;
			origOffset = offsetPlayer;//length( offsetPlayer );
			V2d offsetDir = normalize( offsetPlayer );
			latchStartAngle = atan2( offsetDir.y, offsetDir.x );
			//cout << "latchStart: " << latchStartAngle << endl;
			testSeq.Update();
			basePos = owner->player.position;
			//launchStartAngle / PI * 180;
		}
		
	}

	//V2d offsetDir = normalize( offsetPlayer );
	//double newAngle = atan2( offsetDir.y, offsetDir.x ) + PI;
	//", new: " <<
		//newAngle << endl;
	

		 
	//position = basePos + truePosOffset * length( offsetPlayer );// * 2.0;
	if( latchedOn )
	{
		double cs = cos( latchStartAngle );
		double sn = sin( latchStartAngle );

		V2d truePosOffset( testSeq.position.x * cs - 
			testSeq.position.y * sn, 
			testSeq.position.x * sn + testSeq.position.y * cs );
		//cout << "testseq: " << testSeq.position.x << ", " 
		//	<< testSeq.position.y << endl;// ",  new: " <<
			//truePosOffset.x << ", " << truePosOffset.y << endl;
		position = basePos + truePosOffset * length( offsetPlayer );
	/*	theta = deg2rad(angle);

		cs = cos(theta);
		sn = sin(theta);

		x = x * cs - y * sn;
		y = x * sn + y * cs;*/

		testSeq.Update();
		offsetPlayer =  origOffset - origOffset * approachAccelBez.GetValue( ( (double)totalFrame / approachFrames) );
	}

	//return;

	//double movement = speed / NUM_STEPS;
	
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

	if( dead )
		return;

	PhysicsResponse();
}

void Shark::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player.test = true;
			owner->player.currAttackHit = true;
			owner->player.flashColor = COLOR_BLUE;
			owner->player.flashFrames = 5;
			owner->player.currentSpeedBar += .8;
			owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );
			owner->powerBar.Charge( 2 * 6 * 3 );
			owner->player.desperationMode = false;


			if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
			{
				owner->player.velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player.frame << endl;

			//owner->player.frame--;
			owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );
			
		//	cout << "Shark received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
				owner->Pause( 8 );
			}
		
			health -= 20;

			if( health <= 0 )
				dead = true;

			receivedHit = NULL;*/
			//dead = true;
			//receivedHit = NULL;
		}

		if( IHitPlayer() )
		{
			cout << "Shark hit player Shark pos: " <<
				position.x << ", " << position.y << ", playerpos: "
				<< owner->player.position.x << ", " << owner->player.position.y << endl;
		//	cout << "Shark just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Shark::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	

	if( slowCounter == slowMultiple )
	{
		++frame;
		if( latchedOn && totalFrame < approachFrames )
		{
			++totalFrame;
		}
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

	if( frame == 10 * animationFactor )
	{
		frame = 0;
	}



	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void Shark::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
	sprite.setPosition( position.x, position.y );

	botDeathSprite.setTexture( *ts->texture );
	botDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
	botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
	botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
		position.y + deathVector.y * deathPartingSpeed * deathFrame );

	topDeathSprite.setTexture( *ts->texture );
	topDeathSprite.setTextureRect( ts->GetSubRect( 1 ) );
	topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
	topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
		position.y + -deathVector.y * deathPartingSpeed * deathFrame );
}

void Shark::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		if( monitor != NULL && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( COLOR_BLUE );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}



}

void Shark::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape enemyCircle;
	enemyCircle.setFillColor( COLOR_BLUE );
	enemyCircle.setRadius( 50 );
	enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
	enemyCircle.setPosition( position.x, position.y );
	target->draw( enemyCircle );

	if( monitor != NULL && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}
}

bool Shark::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Shark::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if( owner->player.ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->player.groundSpeed * ( owner->player.ground->v1 - owner->player.ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->player.velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Shark::PlayerHitMe()
{
	Actor &player = owner->player;
	if( player.currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player.currHitboxes->begin(); it != player.currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player.currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player.recordedGhosts; ++i )
	{
		if( player.ghostFrame < player.ghosts[i]->totalRecorded )
		{
			if( player.ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); 
					it != player.ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player.currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player.Sharks[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Shark::PlayerSlowingMe()
{
	Actor &player = owner->player;
	for( int i = 0; i < player.maxBubbles; ++i )
	{
		if( player.bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player.bubblePos[i] ) <= player.bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void Shark::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		if( testSeq.currMovement != NULL )
		{
			if( testSeq.currMovement->vertices != NULL )
			{
				testSeq.currMovement->DebugDraw( target );
			}
		}
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Shark::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Shark::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}