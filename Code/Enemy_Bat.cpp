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


Bat::Bat( GameSession *owner, Vector2i pos, list<Vector2i> &pathParam, bool loopP, float pspeed )
	:Enemy( owner, EnemyType::BAT ), deathFrame( 0 )
{
	

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	


	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	pathLength = pathParam.size() + 1;
	//cout << "pathLength: " << pathLength << endl;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
		//path.push_back( (*it) );

	}

	//make composite beziers
	if( pathLength == 1 )
	{

	}
	else
	{
		//for( int i = 0; i < pathLength; ++i )
		//{

		//}
	}

	V2d sqTest0 = position;
	V2d sqTest1 = position + V2d( 300, 0 );
	V2d sqTest2 = position + V2d( 300, 300 );
	V2d sqTest3 = position + V2d( 0, 200 );

	testSeq.AddLineMovement( sqTest0, sqTest1, 
		CubicBezier(1,.03,.07,.72 ), 60 );
	testSeq.AddCubicMovement( sqTest1, sqTest2, sqTest3, sqTest0,
		CubicBezier(1,.03,.07,.72 ), 60 );
	//	CubicBezier( 0, .03, .1, 1), 60 );

	/*testSeq.AddMovement( new BezierMovement( 
		&GetCubicValue, 60 ,
		sqTest0, sqTest1,sqTest2 , sqTest3 ) );
	testSeq.AddMovement( new BezierMovement( 
		&GetCubicValue, 60 ,
		sqTest3, sqTest0,sqTest1 , sqTest2 ) );*/

	//testSeq.InitMovementDebug();

	V2d pos2Test = position + V2d( 200, 0 );
	V2d pos3Test = position - V2d( 200, 0 );
	

	V2d filler( 0, 0 );

	V2d p1 = position * .1 + pos2Test * .9;
	V2d p2 = position * .5 + pos2Test * .5;

	V2d blah = p1 + V2d( 0, 100 );
	V2d blah1 = p2 - V2d( 0, 100 );

	V2d p_p1 = pos2Test * .1 + pos3Test * .9;
	V2d p_p2 = pos2Test * .5 + pos3Test * .5;
	/*testSeq.AddMovement( new BezierMovement( 
		&GetCubicValue, 60 ,
		position, blah,blah1 , pos2Test ) );
	testSeq.AddMovement( new WaitMovement( pos2Test, 30 ) ); 
	testSeq.AddMovement( new BezierMovement( 
		&GetCubicValue, 60 ,
		pos2Test, p_p1, p_p2, pos3Test ) );*/

	loop = loopP;
	
	speed = pspeed;



	//speed = 2;
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Bat.png", 80, 80 );
	ts = owner->GetTileset( "Bat_48x48.png", 48, 48 );
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

	targetNode = 1;
	forward = true;

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

void Bat::HandleEntrant( QuadTreeEntrant *qte )
{

}

void Bat::ResetEnemy()
{
	testSeq.Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	targetNode = 1;
	forward = true;
	dead = false;
	deathFrame = 0;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Bat::UpdatePrePhysics()
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

void Bat::UpdatePhysics()
{
	testSeq.Update();

	position = testSeq.position;

	return;

	double movement = speed / NUM_STEPS;
	
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

	if( pathLength > 1 )
	{
		movement /= (double)slowMultiple;

		while( movement != 0 )
		{
			//cout << "movement loop? "<< endl;
			V2d targetPoint = V2d( path[targetNode].x, path[targetNode].y );
			V2d diff = targetPoint - position;
			double len = length( diff );
			if( len >= abs( movement ) )
			{
				position += normalize( diff ) * movement;
				movement = 0;
			}
			else
			{
				position += diff;
				movement -= length( diff );
				AdvanceTargetNode();	
			}
		}
	}

	PhysicsResponse();
}

void Bat::PhysicsResponse()
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
			
		//	cout << "Bat received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Bat just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Bat::AdvanceTargetNode()
{
	if( loop )
	{
		++targetNode;
		if( targetNode == pathLength )
			targetNode = 0;
	}
	else
	{
		if( forward )
		{
			++targetNode;
			if( targetNode == pathLength )
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if( targetNode < 0 )
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void Bat::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	

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

void Bat::UpdateSprite()
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

void Bat::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		if( monitor != NULL )
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

void Bat::DrawMinimap( sf::RenderTarget *target )
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

bool Bat::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Bat::UpdateHitboxes()
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
pair<bool,bool> Bat::PlayerHitMe()
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

bool Bat::PlayerSlowingMe()
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

void Bat::DebugDraw( RenderTarget *target )
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

void Bat::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.forward = forward;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
	stored.targetNode = targetNode;
}

void Bat::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	forward = stored.forward;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
	targetNode = stored.targetNode;
}