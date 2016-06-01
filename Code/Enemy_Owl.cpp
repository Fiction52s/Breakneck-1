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


Owl::Owl( GameSession *owner, Vector2i &pos, int p_bulletSpeed, int p_framesBetweenFiring, bool p_facingRight )
	:Enemy( owner, EnemyType::OWL ), deathFrame( 0 ),flyingBez( 0, 0, 1, 1 )
{
	//movementRadius = 300;
	retreatRadius = 400;
	chaseRadius = 600;
	shotRadius = 800;
	flySpeed = 5.0;
	velocity = V2d( 0, 0 );
	action = NEUTRAL;
	frame = 0;
	actionLength[NEUTRAL] = 30;
	actionLength[FIRE] = 60;
	actionLength[RETREAT] = 30;
	actionLength[CHASE] = 30;
	actionLength[REST] = 60;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;

	facingRight = p_facingRight;

	bulletSpeed = p_bulletSpeed;
	framesBetween = p_framesBetweenFiring;

	deathFrame = 0;
	
	launcher = new Launcher( this, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	//ts = owner->GetTileset( "Owl.png", 80, 80 );
	ts = owner->GetTileset( "bat_48x48.png", 48, 48 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );

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

void Owl::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Owl::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	if( b->bounceCount == 2 )
	{
		b->launcher->DeactivateBullet( b );
	}
	else
	{
		V2d en = edge->Normal();
		if( pos == edge->v0 )
		{
			en = normalize( b->position - pos );
		}
		else if( pos == edge->v1 )
		{
			en = normalize( b->position - pos );
		}
		double d = dot( b->velocity, en );
		V2d ref = b->velocity - (2.0 * d * en);
		b->velocity = ref;
		cout << "ref: " << ref.x << ", " << ref.y << endl;
		//b->velocity = -b->velocity;
		b->bounceCount++;
	}
}

void Owl::BulletHitPlayer(BasicBullet *b )
{
	owner->player.ApplyHit( b->launcher->hitboxInfo );
}

void Owl::ResetEnemy()
{
	velocity = V2d( 0, 0 );
	action = NEUTRAL;
	//testSeq.Reset();
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

void Owl::ActionEnded()
{
	Actor &player = owner->player;
	double len = length( player.position - position );
	if( frame == actionLength[action] )
	{
		if( action == FIRE )
		{
			if( len > shotRadius )
			{
				action = REST;
			}
			else if( len > chaseRadius )
			{
				action = CHASE;
				velocity = normalize( player.position - position ) * 2.5;
			}
			else if( len < retreatRadius )
			{
				action = RETREAT;
				velocity = normalize( player.position - position ) * -2.5;
			}
			else
			{
				action = NEUTRAL;
			}	
		}
		else if( action == REST )
		{
			if( len > shotRadius )
			{
				//stay the same
			}
			else if( len > chaseRadius )
			{
				action = CHASE;
				velocity = normalize( player.position - position ) * 2.5;
			}
			else if( len < retreatRadius )
			{
				action = RETREAT;
				velocity = normalize( player.position - position ) * -2.5;
			}
			else
			{
				action = NEUTRAL;
				velocity = V2d( 0, 0 );
			}	
		}
		else
		{
			action = FIRE;
			velocity = V2d( 0, 0 );
		}
		frame = 0;
	}
}


void Owl::UpdatePrePhysics()
{
	ActionEnded();

	Actor &player = owner->player;

	switch( action )
	{
	case NEUTRAL:
		cout << "neutral: " << frame << endl;
		break;
	case FIRE:
		cout << "fire: " << frame << endl;
		break;
	case RETREAT:
		cout << "retreat: " << frame << endl;
		
		break;
	case CHASE:
		cout << "chase" << endl;
		break;
	case REST:
		cout << "rest" << endl;
		break;
	default:
		cout << "what" << endl;
	}


	/*if( action == RETREAT )
	{
		velocity = normalize( player.position - position ) * -2.5;
	}
	else if( action == NEUTRAL )
	{
		velocity = normalize( player.position - position ) * 2.5;
	}*/

	launcher->UpdatePrePhysics();

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
		launcher->position = position;
		launcher->facingDir = normalize( owner->player.position - position );
		launcher->Fire();
	}
}

void Owl::UpdatePhysics()
{	
	specterProtected = false;
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

	position += velocity / NUM_STEPS / (double)slowMultiple;

	launcher->UpdatePhysics();

	if( !dead && !dying )
	{
		//testSeq.Update();
		//position = testSeq.position;
		PhysicsResponse();
	}

	return;
}

void Owl::PhysicsResponse()
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
			
		//	cout << "Owl received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Owl just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Owl::UpdatePostPhysics()
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

void Owl::UpdateSprite()
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

void Owl::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		if( monitor != NULL )
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

void Owl::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor( COLOR_BLUE );
		enemyCircle.setRadius( 50 );
		enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
		enemyCircle.setPosition( position.x, position.y );
		target->draw( enemyCircle );

		if( monitor != NULL )
		{
			monitor->miniSprite.setPosition( position.x, position.y );
			target->draw( monitor->miniSprite );
		}
	}
}

bool Owl::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Owl::UpdateHitboxes()
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
pair<bool,bool> Owl::PlayerHitMe()
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
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player.currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
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

bool Owl::PlayerSlowingMe()
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

void Owl::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Owl::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Owl::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}