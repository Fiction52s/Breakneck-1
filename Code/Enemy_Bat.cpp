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


Bat::Bat( GameSession *owner, Vector2i pos, 
	list<Vector2i> &pathParam, int p_bulletSpeed,
	//int p_nodeDistance, 
	int p_framesBetweenNodes, bool p_loop )
	:Enemy( owner, EnemyType::BAT ), deathFrame( 0 )
{
	keyFrame = 0;
	ts_testKey = owner->GetTileset( "key_w01_1_128x128.png", 128, 128 );


	shader = new Shader();
	if( !shader->loadFromFile( "key_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "couldnt load enemy key shader" << endl;
		assert( false );
	}

	loop = p_loop;
	//loop = false; //no looping on bat for now

	fireCounter = 0;
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	bulletSpeed = p_bulletSpeed;
	//nodeDistance = p_nodeDistance;
	framesBetween = p_framesBetweenNodes;

	deathFrame = 0;
	
	ts_hitSpack = owner->GetTileset( "hit_spack_2_128x128.png", 128, 128 );

	launcher = new Launcher( this, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	pathLength = pathParam.size() + 1;
	if( loop )
	{
		
		cout << "looping bat" << endl;
		assert( false );
		//tough cuz of set node distance from each other. for now don't use it.
	}
	else
	{
		//the road back
		if( pathParam.size() > 0 )
		{
			pathLength += pathParam.size();
		}
	}
	
	path = new Vector2i[pathLength];
	path[0] = pos;
	path[pathLength-1] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
	}

	if( pathLength == 1 )
	{

	}
	else
	{
		list<Vector2i>::reverse_iterator rit = pathParam.rbegin();
		++rit; //start at second item
		
		for(  ;rit != pathParam.rend(); ++rit )
		{
			path[index] = (*rit) + pos;
			++index;
		}
	}

	V2d sqTest0 = position;
	V2d sqTest1 = position + V2d( 0, -150 );
	V2d sqTest2 = position + V2d( 150, -150 );
	V2d sqTest3 = position + V2d( 300, -150 );
	V2d sqTest4 = position + V2d( 300, 0 );

	for( int i = 0; i < pathLength - 1; ++i )
	{
		V2d A( path[i].x, path[i].y );
		V2d B( path[i+1].x, path[i+1].y );
		//A += position;
		//B += position;
		testSeq.AddLineMovement( A, B, CubicBezier( .42,0,.58,1 ), 60 );
	}
	if( pathLength == 1 )
	{
		testSeq.AddMovement( new WaitMovement( position, 60 ) );
	}
	testSeq.InitMovementDebug();
	testSeq.Reset();
	
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Bat.png", 80, 80 );
	ts = owner->GetTileset( "Bat_144x176.png", 144, 176 );
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

	spawnRect = sf::Rect<double>( position.x - 200, position.y - 200,
		400, 400 );

	//cout << "finish init" << endl;
}

void Bat::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}



void Bat::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	b->launcher->DeactivateBullet( b );
}

void Bat::BulletHitPlayer(BasicBullet *b )
{
	owner->player->ApplyHit( b->launcher->hitboxInfo );
}


void Bat::ResetEnemy()
{
	keyFrame = 0;
	fireCounter = 0;
	testSeq.Reset();
	launcher->Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	//targetNode = 1;
	//forward = true;
	dead = false;
	dying = false;
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
	if( keyFrame == 16 * 2 )
	{
		keyFrame = 0;
	}

	if( testSeq.currMovement == NULL )
	{
		//cout << "resetting" << endl;
		testSeq.Reset();
		//testSeq.currMovement = testSeq.movementList;
		//testSeq.currMovementStartTime = 0;
	}

	launcher->UpdatePrePhysics();

	if( !dead && !dying && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( monitor != NULL )
				owner->keyMarker->CollectKey();
			//AttemptSpawnMonitor();
			dying = true;
			//cout << "dying" << endl;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}

	if( !dying && !dead && fireCounter == framesBetween - 1 )// frame == 0 && slowCounter == 1 )
	{
		launcher->position = position;
		launcher->facingDir = normalize( owner->player->position - position );
		//cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
		//	launcher->facingDir.y << endl;
		launcher->Fire();
		fireCounter = 0;
		//testLauncher->Fire();
	}

	/*if( latchedOn )
	{
		basePos = owner->player->position + offsetPlayer;
	}*/
}

void Bat::UpdatePhysics()
{	
	specterProtected = false;
	if( !dead && !dying )
	{
		testSeq.Update();
		//cout << "position: " << position.x << ", " << position.y << 
		//	", newpos: " << testSeq.position.x 
		//	<< ", " << testSeq.position.y << endl;
		position = testSeq.position;
		PhysicsResponse();
	}

	launcher->UpdatePhysics();

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

	return;
}

void Bat::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		

		if( result.first && !specterProtected )
		{

			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_GREEN, 5, .8, 2 * 6 * 2 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );
			
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

void Bat::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->ActivateEffect( ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( ts_testBlood, position, true, 0, 15, 2, true );
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

	if( slowCounter == slowMultiple )
	{
		++keyFrame;
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

	if( frame == 5 * animationFactor )
	{
		frame = 0;
	}


	/*if( owner->totalGameFrames % 60 == 0 )
	{
		owner->ActivateEffect( ts_testKey, position, false, 0, 16, 2, true );
	}*/
	

	if( dead && launcher->GetActiveCount() == 0 )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
	launcher->UpdateSprites();
}

void Bat::UpdateSprite()
{
	if( !dying && !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
		sprite.setPosition( position.x, position.y );

		if( monitor != NULL && !suppressMonitor )
		{
			testKeySprite.setTexture( *ts_testKey->texture );
			testKeySprite.setTextureRect( ts_testKey->GetSubRect( keyFrame / 2 ) );
			testKeySprite.setOrigin( testKeySprite.getLocalBounds().width / 2, 
				testKeySprite.getLocalBounds().height / 2 );
			testKeySprite.setPosition( position.x, position.y );

		}
		//testKeySprite.setTextureRect( ts_testKey->GetSubRect( 
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

void Bat::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		if( monitor != NULL && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );

			cs.setFillColor( Color::Black );

			//cs.setFillColor( monitor-> );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			//target->draw( cs );
		}

		if( shader != NULL && monitor != NULL && !suppressMonitor )
		{
			target->draw( sprite, shader );
			target->draw( testKeySprite );
		}
		else
		{
			target->draw( sprite );
		}
		
		//cout << "drawing bat: " << sprite.getPosition().x
		//	<< ", " << sprite.getPosition().y << endl;
	}
	else if( !dead )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );*/
		}
		
		target->draw( topDeathSprite );
	}



}

void Bat::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying && monitor != NULL && !suppressMonitor )
	{
		CircleShape cs;
		cs.setRadius( 50 );
		cs.setFillColor( Color::White );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );
		target->draw( cs );
	}
}

bool Bat::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
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

	if( owner->player->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->player->groundSpeed * ( owner->player->ground->v1 - owner->player->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->player->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Bat::PlayerHitMe()
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
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player->currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
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

bool Bat::PlayerSlowingMe()
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
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Bat::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}