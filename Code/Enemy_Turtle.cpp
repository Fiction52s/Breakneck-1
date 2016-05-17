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


Turtle::Turtle( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::TURTLE ), deathFrame( 0 )
{
	//loop = false; //no looping on Turtle for now

	bulletSpeed = 5;

	action = NEUTRAL;

	animFactor[NEUTRAL] = 1;
	animFactor[FIRE] = 1;
	animFactor[FADEIN] = 1;
	animFactor[FADEOUT] = 1;
	animFactor[INVISIBLE] = 1;

	actionLength[NEUTRAL] = 3;
	actionLength[FIRE] = 20;
	actionLength[FADEIN] = 60;
	actionLength[FADEOUT] = 60;
	actionLength[INVISIBLE] = 30;

	fireCounter = 0;
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;

	deathFrame = 0;
	
	launcher = new Launcher( this, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Turtle.png", 80, 80 );
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

	

	dead = false;
	dying = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	//cout << "finish init" << endl;
}

void Turtle::HandleEntrant( QuadTreeEntrant *qte )
{

}

void Turtle::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{

}

void Turtle::BulletHitPlayer(BasicBullet *b )
{
	owner->player.ApplyHit( b->launcher->hitboxInfo );
}


void Turtle::ResetEnemy()
{
	fireCounter = 0;
	launcher->Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	//targetNode = 1;
	//forward = true;
	dead = false;
	dying = false;
	deathFrame = 0;
	frame = 0;
	position.x = originalPos.x;
	position.y = originalPos.y;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Turtle::ActionEnded()
{
	if( frame == actionLength[action] )
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
		position = owner->player.position;
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
		break;
	}
	}
}

void Turtle::UpdatePrePhysics()
{
	ActionEnded();

	launcher->UpdatePrePhysics();

	switch( action )
	{
	case NEUTRAL:
		cout << "NEUTRAL";
		break;
	case FIRE:
		cout << "FIRE";
		break;
	case INVISIBLE:
		cout << "INVISIBLE";
		break;
	case FADEIN:
		cout << "FADEIN";
		break;
	case FADEOUT: 
		cout << "FADEOUT";
		break;
	}

	cout << " " << frame << endl;

	switch( action )
	{
	case NEUTRAL:
		break;
	case FIRE:
		break;
	case INVISIBLE:
		break;
	case FADEIN:
		break;
	case FADEOUT: 
		break;
	}



	if( !dead && !dying && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			AttemptSpawnMonitor();
			dying = true;
			//cout << "dying" << endl;
		}

		receivedHit = NULL;
	}

	if( !dying && !dead && action == FIRE && frame == actionLength[FIRE] - 1 )// frame == 0 && slowCounter == 1 )
	{
		//cout << "firing" << endl;
		launcher->position = position;
		launcher->facingDir = normalize( owner->player.position - position );
		//cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
		//	launcher->facingDir.y << endl;
		launcher->Fire();
		fireCounter = 0;
		//testLauncher->Fire();
	}

	/*if( latchedOn )
	{
		basePos = owner->player.position + offsetPlayer;
	}*/
}

void Turtle::UpdatePhysics()
{	
	if( !dead )
	{
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
	}

	launcher->UpdatePhysics();

	if( !dead && !dying )
	{
		if( action == NEUTRAL )
		{
			Actor &player = owner->player;
			if( length( player.position - position ) < 300 )
			{
				action = FADEOUT;
				frame = 0;
			}
		}
		PhysicsResponse();
	}
	return;
}

void Turtle::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
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
			
		//	cout << "Turtle received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Turtle just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Turtle::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	

	if( slowCounter == slowMultiple )
	{
		//cout << "fireCounter: " << fireCounter << endl;
		++frame;
		slowCounter = 1;
		++fireCounter;
	
		if( dying )
		{
			//cout << "deathFrame: " << deathFrame << endl;
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( deathFrame == 60 && dying )
	{
		//cout << "switching dead" << endl;
		dying = false;
		dead = true;
		//cout << "REMOVING" << endl;
		//testLauncher->Reset();
		//owner->RemoveEnemy( this );
		//return;
	}

	if( dead && launcher->GetActiveCount() == 0 )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	launcher->UpdateSprites();
}

void Turtle::UpdateSprite()
{
	if( !dying && !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		sprite.setPosition( position.x, position.y );
	}
	if( dying )
	{

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
}

void Turtle::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		if( monitor != NULL && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );

			switch( monitor->monitorType )
			{
			case Monitor::BLUE:
				cs.setFillColor( COLOR_BLUE );
				break;
			case Monitor::GREEN:
				cs.setFillColor( COLOR_GREEN );
				break;
			case Monitor::YELLOW:
				cs.setFillColor( COLOR_YELLOW );
				break;
			case Monitor::ORANGE:
				cs.setFillColor( COLOR_ORANGE );
				break;
			case Monitor::RED:
				cs.setFillColor( COLOR_RED );
				break;
			case Monitor::MAGENTA:
				cs.setFillColor( COLOR_MAGENTA );
				break;
			case Monitor::WHITE:
				cs.setFillColor( COLOR_WHITE );
				break;
			}

			//cs.setFillColor( monitor-> );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );
	}
	else if( !dead )
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

void Turtle::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
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
}

bool Turtle::IHitPlayer()
{
	if( action == FADEIN || action == INVISIBLE )
		return false;

	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Turtle::UpdateHitboxes()
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
pair<bool,bool> Turtle::PlayerHitMe()
{
	if( action == INVISIBLE )
		return pair<bool,bool>(false,false);

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
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); it != player.ghosts[i]->currHitboxes->end(); ++it )
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
			//player.ghosts[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Turtle::PlayerSlowingMe()
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

void Turtle::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Turtle::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Turtle::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}