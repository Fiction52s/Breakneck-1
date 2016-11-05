#include "Boss.h"
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



Boss_Skeleton::Boss_Skeleton( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::TURTLE, false, 6 ), deathFrame( 0 ), moveBez( 0, 0, 1, 1 ),
	DOWN( 0, 1 ), LEFT( -1, 0 ), RIGHT( 1, 0 ), UP( 0, -1 ), pathVA( sf::Quads, MAX_PATH_SIZE * 4 ),
	flowerVA( sf::Quads, 200 * 4 ), linkVA( sf::Quads, 248 * 4 )
{
	//current num links is 248	
	position.x = pos.x;
	position.y = pos.y;
	for( int i = 0; i < 200 * 4; ++i )
	{
		flowerVA[i].position = Vector2f( 0, 0 );
		flowerVA[i].color = Color::Red;

		
	}

	for( int i = 0; i < 248 * 4; ++i )
	{
		linkVA[i].position = Vector2f( 0, 0 );
		linkVA[i].color = Color::Green;
	}

	testNumLinks = 0;
	testIndex = 0;
	testLength = 32;
	CreateQuadrant();

	do
	{
	//	cout << "START ITERATION----- " << i << endl;
		ClearPathVA();
		FlowerNode *fn = CreatePath();
	//	cout << "FINISHED ITERATION----- " << i << endl;
		if( fn != NULL )
			break;
	}
	while( true );

	//for( int i = 0; i < 10; ++i )
	
	//while( true );
	
	testCircle.setRadius( 30 );
	testCircle.setFillColor( Color::Red );
	testCircle.setOrigin( testCircle.getLocalBounds().width / 2, 
		testCircle.getLocalBounds().height / 2 );

	testFinalCircle.setRadius( 30 );
	testFinalCircle.setFillColor( Color::Black );
	testFinalCircle.setOrigin( testFinalCircle.getLocalBounds().width / 2, 
		testFinalCircle.getLocalBounds().height / 2 );
	//ClearPathVA();

	nodeTravelFrames = 5;
	travelFrame = 0;
	travelIndex = 0;
	testFrame = 0;
	gridRatio = 1;
	gridSizeRatio = 64;
	gridOriginPos = V2d( pos.x, pos.y );
	pathSize = MAX_PATH_SIZE;
	moveX = false;
	//xIndexMove = 0;
	///yIndexMove = 0;
	//loop = false; //no looping on Boss_Skeleton for now

	bulletSpeed = 5;

	action = PLANMOVE;

	/*animFactor[NEUTRAL] = 1;
	animFactor[FIRE] = 1;
	animFactor[FADEIN] = 1;
	animFactor[FADEOUT] = 1;
	animFactor[INVISIBLE] = 1;

	actionLength[NEUTRAL] = 3;
	actionLength[FIRE] = 20;
	actionLength[FADEIN] = 60;
	actionLength[FADEOUT] = 90;
	actionLength[INVISIBLE] = 30;*/

	fireCounter = 0;
	receivedHit = NULL;
	

	originalPos = pos;

	deathFrame = 0;
	
	//launcher = new Launcher( this, owner, 12, 12, position, V2d( 1, 0 ), 2 * PI, 90, true );
	launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Boss_Skeleton.png", 80, 80 );
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
	hitboxInfo->damage = 18;
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

void Boss_Skeleton::ResetEnemy()
{
	do
	{
	//	cout << "START ITERATION----- " << i << endl;
		ClearPathVA();
		FlowerNode *fn = CreatePath();
	//	cout << "FINISHED ITERATION----- " << i << endl;
		if( fn != NULL )
			break;
	}
	while( true );
	/*for( int i = 0; i < 200 * 4; ++i )
	{
		flowerVA[i].position = Vector2f( 0, 0 );
		flowerVA[i].color = Color::Red;
	}*/
	testIndex = 0;
	//CreateQuadrant();

	travelFrame = 0;
	travelIndex = 0;
	action = PLANMOVE;
	//ClearPathVA();
	testFrame = 0;

	moveX = false;
	moveIndex = Vector2i( 0, 0 );

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

void Boss_Skeleton::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Boss_Skeleton::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	b->launcher->DeactivateBullet( b );
}

void Boss_Skeleton::BulletHitPlayer(BasicBullet *b )
{
	owner->player->ApplyHit( b->launcher->hitboxInfo );
}

void Boss_Skeleton::ActionEnded()
{
	if( frame == actionLength[action] )
	{
	switch( action )
	{
	
	}
	}
}

void Boss_Skeleton::UpdatePrePhysics()
{
	ActionEnded();

	launcher->UpdatePrePhysics();

	switch( action )
	{
	case PLANMOVE:
		break;
	case MOVE:
		break;
	case SHOOT:
		break;
	}

	//++travelFrame;
	//if( travelFrame == nodeTravelFrames )
	//{
	//	travelFrame = 0;
	//	travelIndex++;
	//	if( travelIndex == pathSize )
	//	{
	//		frame = 0;
	//		moveIndex = finalIndex;
	//	}
	//}
	//switch( action )
	//{
	//case PLANMOVE:
	//	if( frame == 0 )
	//	{
	//		//CreatePath();
	//		travelFrame = 0;
	//		travelIndex = 0;			
	//	}
	//	break;
	//case MOVE:
	//	break;
	//case SHOOT:
	//	break;
	//}

	//UpdatePathVA();
	

	if( !dead && !dying && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dying = true;
			//cout << "dying" << endl;
		}

		receivedHit = NULL;
	}

	//if( !dying && !dead && action == FIRE && frame == actionLength[FIRE] - 1 )// frame == 0 && slowCounter == 1 )
	//{
	//	//cout << "firing" << endl;
	//	launcher->position = position;
	//	launcher->facingDir = normalize( owner->player->position - position );
	//	//cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
	//	//	launcher->facingDir.y << endl;
	//	launcher->Fire();
	//	fireCounter = 0;
	//	//testLauncher->Fire();
	//}

	/*if( latchedOn )
	{
		basePos = owner->player->position + offsetPlayer;
	}*/
}

void Boss_Skeleton::UpdatePhysics()
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

	launcher->UpdatePhysics();

	if( !dead && !dying )
	{
		/*if( action == NEUTRAL )
		{
			Actor *player = owner->player;
			if( length( player->position - position ) < 300 )
			{
				action = FADEOUT;
				frame = 0;
			}
		}*/
		PhysicsResponse();
	}
	return;
}

void Boss_Skeleton::PhysicsResponse()
{
	if( !dead && !dying && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( 6, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, facingRight );
			
		//	cout << "Boss_Skeleton received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Boss_Skeleton just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Boss_Skeleton::ClearPathVA()
{
	for( int i = 0; i < MAX_PATH_SIZE * 4; ++i )
	{
		pathVA[i].color = Color::Blue;
		pathVA[i].position = Vector2f( 0, 0 );
	}
}

void Boss_Skeleton::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();
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

void Boss_Skeleton::UpdateSprite()
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

void Boss_Skeleton::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead && !dying )
	{
		if( hasMonitor && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );

			cs.setFillColor( Color::Black );

			//cs.setFillColor( monitor-> );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		
		
		target->draw( sprite );
		target->draw( flowerVA );
		target->draw( linkVA );
		target->draw( pathVA );
		//target->draw( pathVA );

		/*if( action == PLANMOVE )
		{
			target->draw( testFinalCircle );
			target->draw( testCircle );
			
		}*/
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

void Boss_Skeleton::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor( COLOR_BLUE );
		enemyCircle.setRadius( 50 );
		enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
		enemyCircle.setPosition( position.x, position.y );
		target->draw( enemyCircle );

		/*if( hasMonitor && !suppressMonitor )
		{
			monitor->miniSprite.setPosition( position.x, position.y );
			target->draw( monitor->miniSprite );
		}*/
	}
}

bool Boss_Skeleton::IHitPlayer()
{

	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Boss_Skeleton::UpdateHitboxes()
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
pair<bool,bool> Boss_Skeleton::PlayerHitMe()
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

bool Boss_Skeleton::PlayerSlowingMe()
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

void Boss_Skeleton::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Boss_Skeleton::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Boss_Skeleton::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}

bool Boss_Skeleton::DirIsValid( sf::Vector2i &testIndex,
	sf::Vector2i &testDir )
{
	Vector2i temp = testIndex + testDir;
	if( temp.x >= 0 && temp.x < GRID_SIZE && temp.y >= 0 && temp.y < GRID_SIZE )
	{
		return true;
	}
	else
	{
		return false;
	}
}

Boss_Skeleton::FlowerNode * Boss_Skeleton::CreatePath()
{
	int numNodes = 0;
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++ j )
		{
			
			//cout << "testing : " << i << ", " << j << endl;
			FlowerNode *fn = nodes[i][j];

			if( fn == NULL )
				continue; 

			for( int k = 0; k < 3; ++k )
			{
				fn->connectsOpen[k] = true;
			}

			//cout << "making not part of path: " << fn->position.x << ", " << fn->position.y << endl;
			fn->partOfPath = false;
			

			if( fn->numConnects == 3 )
			{
				int r = rand() % 3;
				fn->connectsOpen[r] = false;
			}
			else if( fn->numConnects == 2 )
			{
				//entrance or exit node
				//int r = rand() % 2;
				//fn->connectsOpen[r] = false;
			}
			else
			{
				continue;
				//assert( 0 );
			}
		}
	}

	FlowerNode *prev = NULL;
	FlowerNode *curr = nodes[1 + HALF_GRID][0 + HALF_GRID];

	for( int i = 0; i < MAX_PATH_SIZE; ++i )
	{
		//cout << "start wat--------------------------------" << endl;
		
		if( curr->partOfPath )
		{
			cout << i << " part of path already?: curr: " << curr->position.x << ", " << curr->position.y << endl;
			//return NULL;
			break;
		}

		//cout << "adding to path: " << curr->position.x << ", " << curr->position.y << endl;
		path[numNodes] = curr;
		curr->partOfPath = true;


		if( prev == NULL )
		{
		}
		else
		{
			//cout << "thing: " << i << endl;
			Vector2i blah = curr->position - prev->position;
			V2d along = normalize( V2d( blah.x, blah.y ) );
			V2d other( along.y, -along.x );
			V2d start = V2d( prev->position.x, prev->position.y );
			V2d end = V2d( curr->position.x, curr->position.y );
			double w = 5;
			V2d sl = start + other * w;
			V2d sr = start - other * w;
			V2d el = end + other * w;
			V2d er = end - other * w;

			pathVA[(i-1) * 4 + 0].position = Vector2f( sl.x, sl.y );
			pathVA[(i-1) * 4 + 1].position = Vector2f( el.x, el.y );
			pathVA[(i-1) * 4 + 2].position = Vector2f( er.x, er.y );
			pathVA[(i-1) * 4 + 3].position = Vector2f( sr.x, sr.y );
		}
		
		if( curr->dest )
		{
			//cout << "dest reached: " << i << endl;
			if( i < 15 )
				return NULL;
			else
				return curr;
		}
		else
		{
			if( prev == NULL )
			{
				//assert( numConnects == 2 );
				//start

				//itll always take the first option for now. gotta fix later
				prev = curr;
				if( curr->connectsOpen[0] )
				{
					curr = curr->connects[0];
				}
				else //1 is open
				{
					curr = curr->connects[1];
				}
			}
			else
			{
				prev->partOfPath = true;
				if( curr->numConnects == 2 )
				{
					bool p0 = curr->connects[0]->partOfPath;
					bool p1 = curr->connects[1]->partOfPath;
					//cout << "0: " << curr->connects[0] << ", 1: " << curr->connects[1] << ", prev: " << prev << endl;
					//cout << "prevstuff: " << (int)prev->partOfPath << endl;
					if( p0 && p1 )
					{
						cout << "this thing" << endl;
						//return NULL;
						break;
						//return;
					}
					else if( p0 )
					{
						prev = curr;
						curr = curr->connects[1];
					}
					else if( p1 )
					{
						prev = curr;
						curr = curr->connects[0];
					}
					else
					{
						assert( false );
					}

					//cout << "this thing" << endl;
					//return;
				}
				else
				{
					for( int n = 0; n < 3; ++n )
					{	
						if( curr->connectsOpen[n] && !curr->connects[n]->partOfPath )
						{
							prev = curr;
							curr = curr->connects[n];
							break;
						}
					}
				}
			}
			//prev = curr;
		}
	}

	return NULL;
}

void Boss_Skeleton::UpdatePathVA()
{
	/*V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );
	 
	Vector2i testIndex = moveIndex;
	for( int i = 0; i <= travelIndex; ++i )
	{
		
		
		Vector2i dir( path[i].x, path[i].y );
		V2d along;
		if( dir == LEFT )
		{
			along = trueLeft;
		}
		else if( dir == DOWN ) 
		{
			along  = trueDown;
		}
		else if( dir == UP )
		{
			along  = trueUp;
		}
		else if( dir == RIGHT )
		{
			along  = trueRight;
		}
		
		V2d norm( along.y, -along.x );

		double height = 10;

		V2d gridIndexPos = trueRight * (double)testIndex.x + trueDown * (double)testIndex.y;
		gridIndexPos *= gridSizeRatio;


		double val = moveBez.GetValue( (double)travelFrame / nodeTravelFrames );
		V2d curr = gridIndexPos + gridOriginPos;
		V2d next = curr;
		if( i == travelIndex )
		{
			
			next += along * gridSizeRatio * val;
			testCircle.setPosition( next.x, next.y );
		}
		else
		{
			next += along * gridSizeRatio;
		}

		V2d c0 = curr + norm * height;
		V2d c1 = next + norm * height;
		V2d c2 = next - norm * height;
		V2d c3 = curr - norm * height;


		pathVA[i*4 + 0].position = Vector2f( c0.x, c0.y );
		pathVA[i*4 + 1].position = Vector2f( c1.x, c1.y );
		pathVA[i*4 + 2].position = Vector2f( c2.x, c2.y );
		pathVA[i*4 + 3].position = Vector2f( c3.x, c3.y );

		testIndex += path[i];
	}*/
}

void Boss_Skeleton::CreateNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex,
		int yIndex)
{
	FlowerNode *fn = new FlowerNode( basePos + Vector2i( testLength * xIndex, 
		testLength * yIndex ) );
	assert( nodes[xIndex][yIndex] == NULL );
	nodes[xIndex][yIndex] = fn;
	
	int blah = 10;
	cout << "node " << testIndex << ": " << xIndex << ", " << yIndex << endl;
	//cout << "position: " << fn->position.x << ", " << fn->position.y << endl;
	flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
	flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
	flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
	flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
	testIndex++;
}

Boss_Skeleton::FlowerNode * Boss_Skeleton::CreateFlowerNode( Vector2i &basePos, int xIndex, int yIndex )
{
	//bounds: xIndex [-8,8], yIndex [-8,8]

	int blah = 10;

	//int realXIndex = xIndex + 8;
	//int realYIndex = yIndex + 8;

	FlowerNode *fn = new FlowerNode( basePos + Vector2i( testLength * xIndex, 
		testLength * yIndex ) );
	//cout << "node " << testIndex << ": " << xIndex << ", " << yIndex << endl;

	flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
	flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
	flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
	flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
	testIndex++;
	
	return fn;
}

void Boss_Skeleton::CreateAxisNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex, int yIndex )
{
	FlowerNode *fn;
	int realX = xIndex + HALF_GRID;
	int realY = yIndex + HALF_GRID;
	fn = CreateFlowerNode( basePos, xIndex, yIndex );
	nodes[realX][realY] = fn;

	if( xIndex != 0 )
	{
		realX = -xIndex + HALF_GRID;
		realY = yIndex + HALF_GRID;
		fn = CreateFlowerNode( basePos, -xIndex, yIndex );
		nodes[realX][realY] = fn;

		if( yIndex != 0 )
		{
			realX = -xIndex + HALF_GRID;
			realY = -yIndex + HALF_GRID;
			fn = CreateFlowerNode( basePos, -xIndex, -yIndex );
			nodes[realX][realY] = fn;
		}
	}

	if( yIndex != 0 )
	{
		realX = xIndex + HALF_GRID;
		realY = -yIndex + HALF_GRID;
		fn = CreateFlowerNode( basePos, xIndex, -yIndex );
		nodes[realX][realY] = fn;
	}
}

void Boss_Skeleton::CreateMirrorNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex,
		int yIndex)
{
	int blah = 10;
	if( xIndex == yIndex )
	{
		CreateAxisNode( basePos, nodes, xIndex, yIndex );
	}
	else
	{
		CreateAxisNode( basePos, nodes, xIndex, yIndex );
		CreateAxisNode( basePos, nodes, yIndex, xIndex);

		/*FlowerNode *fn;
		int realX = xIndex + 8;
		int realY = yIndex + 8;
		fn = CreateFlowerNode( basePos, xIndex, yIndex );
		nodes[realX][realY] = fn;

		realX = -xIndex + 8;
		realY = yIndex + 8;
		fn = CreateFlowerNode( basePos, -xIndex, yIndex );
		nodes[realX][realY] = fn;

		realX = -xIndex + 8;
		realY = -yIndex + 8;
		fn = CreateFlowerNode( basePos, -xIndex, -yIndex );
		nodes[realX][realY] = fn;

		realX = xIndex + 8;
		realY = -yIndex + 8;
		fn = CreateFlowerNode( basePos, xIndex, -yIndex );
		nodes[realX][realY] = fn;


		FlowerNode *fn = new FlowerNode( basePos + Vector2i( testLength * xIndex, 
			testLength * yIndex ) );
		nodes[xIndex][yIndex] = fn;
		flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
		flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
		flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
		flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
		testIndex++;
		
		fn = new FlowerNode( basePos + Vector2i( testLength * yIndex, 
		testLength * xIndex ) );
		nodes[yIndex][xIndex] = fn;
		flowerVA[testIndex*4+0].position = Vector2f( fn->position.x - blah, fn->position.y - blah );
		flowerVA[testIndex*4+1].position = Vector2f( fn->position.x + blah, fn->position.y - blah );
		flowerVA[testIndex*4+2].position = Vector2f( fn->position.x + blah, fn->position.y + blah );
		flowerVA[testIndex*4+3].position = Vector2f( fn->position.x - blah, fn->position.y + blah );
		testIndex++;*/
	}
	
}

void Boss_Skeleton::CreateLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex0,
		int yIndex0, int xIndex1, int yIndex1 )
{
	//cout << "create link. fn0: " << xIndex0 << ", " << yIndex0 << ", fn1: " << xIndex1 << ", " << yIndex1 << endl;
	FlowerNode *fn0 = nodes[xIndex0][yIndex0];
	FlowerNode *fn1 = nodes[xIndex1][yIndex1];

	assert( fn0 != NULL );
	assert( fn1 != NULL );

	if( fn0->numConnects >= 3 )
	{
		cout << "fn0: " << fn0->numConnects << endl;
		cout << "numlinks: " << testNumLinks << endl;
		cout << "xindex0: " << xIndex0-HALF_GRID << ", yindex0: " << yIndex0-HALF_GRID << endl;
		cout << "xindex1: " << xIndex1-HALF_GRID << ", yindex1: " << yIndex1-HALF_GRID << endl;
		cout << "fn0pos: " << fn0->position.x << ", " << fn0->position.y << endl;
		assert( fn0->numConnects < 3 );
	}
	else if( fn1->numConnects >= 3 )
	{
		cout << "fn1: " << fn1->numConnects << endl;
		cout << "numlinks: " << testNumLinks << endl;
		cout << "xindex0: " << xIndex0-HALF_GRID << ", yindex0: " << yIndex0-HALF_GRID << endl;
		cout << "xindex1: " << xIndex1-HALF_GRID << ", yindex1: " << yIndex1-HALF_GRID << endl;
		cout << "fn1pos: " << fn1->position.x << ", " << fn1->position.y << endl;
		assert( fn1->numConnects < 3 );
	}
	
	

	fn0->connects[fn0->numConnects] = fn1;
	fn1->connects[fn1->numConnects] = fn0;

	//setup visuals

	Vector2i sub = fn1->position - fn0->position;
	V2d along = V2d( sub.x, sub.y );
	double len = length( along );
	double w = 5;
	along = normalize( along );

	V2d other = V2d( along.y, -along.x );
	V2d start( fn0->position.x, fn0->position.y );
	V2d end( fn1->position.x, fn1->position.y );

	V2d sl = start + other * w;
	V2d sr = start - other * w;
	V2d el = end + other * w;
	V2d er = end - other * w;

	linkVA[testNumLinks*4+0].position = Vector2f( sl.x, sl.y );
	linkVA[testNumLinks*4+1].position = Vector2f( el.x, el.y );
	linkVA[testNumLinks*4+2].position = Vector2f( er.x, er.y );
	linkVA[testNumLinks*4+3].position = Vector2f( sr.x, sr.y );


	fn0->numConnects++;
	fn1->numConnects++;

	++testNumLinks;

	//cout << "end of link creation" << endl;
}

void Boss_Skeleton::CreateMirrorLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE], 
	int xIndex0, int yIndex0, 
	int xIndex1, int yIndex1 )
{
	bool a = xIndex0 == yIndex0;
	bool b = xIndex1 == yIndex1;
	if( a && b )
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
	}
	else if( a )
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
		CreateAxisLink( nodes, xIndex0, yIndex0, yIndex1, xIndex1 );
	}
	else if( b )
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
		CreateAxisLink( nodes, yIndex0, xIndex0, xIndex1, yIndex1 );
	}
	//else if( yIndex0 == 0 || yIndex1 == 0 )
	//{
	//	cout << "creating single link" << endl;
	//	CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
	//	//CreateAxisLink( nodes, yIndex0, xIndex0, yIndex1, xIndex1 );
	//}
	else
	{
		CreateAxisLink( nodes, xIndex0, yIndex0, xIndex1, yIndex1 );
		CreateAxisLink( nodes, yIndex0, xIndex0, yIndex1, xIndex1 );
	}

}

void Boss_Skeleton::CreateAxisLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0, int yIndex0, int xIndex1, int yIndex1 )
{
	int realX0 = xIndex0 + HALF_GRID;
	int realY0 = yIndex0 + HALF_GRID;
	int realX1 = xIndex1 + HALF_GRID;
	int realY1 = yIndex1 + HALF_GRID;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );

	if( xIndex0 != 0 || xIndex1 != 0 )
	{
		realX0 = -xIndex0 + HALF_GRID;
		realY0 = yIndex0 + HALF_GRID;
		realX1 = -xIndex1 + HALF_GRID;
		realY1 = yIndex1 + HALF_GRID;
		CreateLink( nodes, realX0, realY0, realX1, realY1 );

		if( yIndex0 != 0 || yIndex1 != 0 )
		{
			realX0 = -xIndex0 + HALF_GRID;
			realY0 = -yIndex0 + HALF_GRID;
			realX1 = -xIndex1 + HALF_GRID;
			realY1 = -yIndex1 + HALF_GRID;
			CreateLink( nodes, realX0, realY0, realX1, realY1 );
		}

	}

	if( yIndex0 != 0 || yIndex1 != 0 )
	{
		realX0 = xIndex0 + HALF_GRID;
		realY0 = -yIndex0 + HALF_GRID;
		realX1 = xIndex1 + HALF_GRID;
		realY1 = -yIndex1 + HALF_GRID;
		CreateLink( nodes, realX0, realY0, realX1, realY1 );
	}
	//if( xIndex0 != 0 && xIndex1 != 0 )
	//{
	//	realX0 = -xIndex0 + 8;
	//	realY0 = yIndex0 + 8;
	//	realX1 = -xIndex1 + 8;
	//	realY1 = yIndex1 + 8;
	//	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	//}
	//else if( xIndex0 == 0 )
	//{
	//	
	//}
	/*if( xIndex0 != 0 || xIndex1 != 0 )
	{*/
		
	//}
	
	/*if( xIndex0 != 0 || xIndex1 != 0 || yIndex0 != 0 || yIndex1 != 0 )
	{*/
		
	//}
	
	/*if( yIndex0 != 0 || yIndex1 != 0 )
	{*/
		
	//}

	/*int realX0 = xIndex0 + 8;
	int realY0 = yIndex0 + 8;
	int realX1 = xIndex1 + 8;
	int realY1 = yIndex1 + 8;

	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );

	realX0 = -xIndex0 + 8;
	realY0 = yIndex0 + 8;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );

	realX0 = -xIndex0 + 8;
	realY0 = -yIndex0 + 8;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );

	realX0 = xIndex0 + 8;
	realY0 = -yIndex0 + 8;
	CreateLink( nodes, realX0, realY0, realX1, realY1 );
	Blah1( nodes, realX0, realY0, xIndex1, yIndex1 );*/
}

void Boss_Skeleton::CreateZeroLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0, int yIndex0, int xIndex1, int yIndex1, int dir )
{
	
}

void Boss_Skeleton::CreateQuadrant()
{
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++j )
		{
			nodes[i][j] = NULL;
			
		}
	}

	int LEFT = 0;
	int RIGHT = 1;
	int UP = 2;
	int DOWN = 2;
	
	int length = 128;
	Vector2i basePos( position.x, position.y );
	//8x8
	//make multiple center nodes
	
	{
	CreateMirrorNode( basePos, nodes, 1, 0 );
	CreateMirrorNode( basePos, nodes, 4, 0 );
	CreateMirrorNode( basePos, nodes, 5, 0 );
	CreateMirrorNode( basePos, nodes, 8, 0 );

	CreateMirrorNode( basePos, nodes, 2, 1 );
	CreateMirrorNode( basePos, nodes, 3, 1 );
	CreateMirrorNode( basePos, nodes, 6, 1 );
	CreateMirrorNode( basePos, nodes, 7, 1 );


	CreateMirrorNode( basePos, nodes, 4, 2 );
	CreateMirrorNode( basePos, nodes, 5, 2 );
	CreateMirrorNode( basePos, nodes, 4, 3 );
	CreateMirrorNode( basePos, nodes, 5, 4 );
	CreateMirrorNode( basePos, nodes, 6, 3 );
	CreateMirrorNode( basePos, nodes, 6, 4 );
	CreateMirrorNode( basePos, nodes, 7, 5 );
	CreateMirrorNode( basePos, nodes, 7, 6 );
	CreateMirrorNode( basePos, nodes, 8, 7 );

	CreateMirrorNode( basePos, nodes, 2, 2 );
	CreateMirrorNode( basePos, nodes, 3, 3 );
	CreateMirrorNode( basePos, nodes, 5, 5 );
	CreateMirrorNode( basePos, nodes, 6, 6 );
	CreateMirrorNode( basePos, nodes, 8, 8 );

	//new
	CreateMirrorNode( basePos, nodes, 7, 3 );
	CreateMirrorNode( basePos, nodes, 8, 2 );
	CreateMirrorNode( basePos, nodes, 9, 2 );
	CreateMirrorNode( basePos, nodes, 10, 1 );
	CreateMirrorNode( basePos, nodes, 9, 0 );
	CreateMirrorNode( basePos, nodes, 11, 0 );


	}


	//CreateMirrorLink( nodes, 6, 6, 7, 6 );

	CreateMirrorLink( nodes, 2, 2, 2, 1 );
	CreateMirrorLink( nodes, 2, 2, 3, 3 );

	CreateMirrorLink( nodes, 3, 3, 4, 3 );

	CreateMirrorLink( nodes, 5, 5, 5, 4 );
	CreateMirrorLink( nodes, 5, 5, 6, 6 );

	CreateMirrorLink( nodes, 6, 6, 7, 6 );

	
	CreateMirrorLink( nodes, 2, 1, 3, 1 );
	CreateMirrorLink( nodes, 3, 1, 4, 2 );
	CreateMirrorLink( nodes, 3, 1, 4, 0 );
	
	CreateMirrorLink( nodes, 6, 1, 7, 1 );
	CreateMirrorLink( nodes, 6, 1, 5, 2 );
	CreateMirrorLink( nodes, 6, 3, 5, 2 );
	CreateMirrorLink( nodes, 6, 3, 6, 4 );
	CreateMirrorLink( nodes, 6, 4, 5, 4 );
	CreateMirrorLink( nodes, 6, 4, 7, 5 );
	

	CreateMirrorLink( nodes, 5, 4, 4, 3 );
	CreateMirrorLink( nodes, 4, 3, 4, 2 );

	CreateMirrorLink( nodes, 5, 2, 4, 2 );
	

	CreateMirrorLink( nodes, 1, 0, 2, 1 );
	//CreateAxisLink( nodes, 1, 0, 0, 1 );
	//CreateMirrorLink( nodes, 1, 1, 1, 1 );

	
	CreateMirrorLink( nodes, 5, 0, 4, 0 );
	CreateMirrorLink( nodes, 7, 1, 8, 0 );
	CreateMirrorLink( nodes, 5, 0, 6, 1 );

	CreateMirrorLink( nodes, 7, 5, 7, 6 );
	CreateMirrorLink( nodes, 8, 7, 8, 8 );
	CreateMirrorLink( nodes, 8, 7, 7, 6 );

	//new
	CreateMirrorLink( nodes, 6, 3, 7, 3 );
	CreateMirrorLink( nodes, 7, 3, 8, 2 );
	CreateMirrorLink( nodes, 7, 1, 8, 2 );
	CreateMirrorLink( nodes, 8, 2, 9, 2 );
	CreateMirrorLink( nodes, 9, 2, 10, 1 );
	CreateMirrorLink( nodes, 9, 0, 10, 1 );
	CreateMirrorLink( nodes, 11, 0, 10, 1 );
	CreateMirrorLink( nodes, 8, 0, 9, 0 );

	
	
	for( int i = 0; i < GRID_SIZE; ++i )
	{
		for( int j = 0; j < GRID_SIZE; ++j )
		{
			if( nodes[i][j] != NULL )
				nodes[i][j]->dest = false;
			
		}
	}

	nodes[11+HALF_GRID][0+HALF_GRID]->dest = true;
	nodes[-11+HALF_GRID][0+HALF_GRID]->dest = true;
	nodes[0+HALF_GRID][11+HALF_GRID]->dest = true;
	nodes[0+HALF_GRID][-11+HALF_GRID]->dest = true;
	nodes[8+HALF_GRID][8+HALF_GRID]->dest = true;
	nodes[-8+HALF_GRID][8+HALF_GRID]->dest = true;
	nodes[-8+HALF_GRID][-8+HALF_GRID]->dest = true;
	nodes[8+HALF_GRID][-8+HALF_GRID]->dest = true;


	//cout << "testNumLInks: " << testNumLinks << endl;
	//nodes[][0] = new FlowerNode( basePos + Vector2i( length, 0 ) );
	//int blah = 0;
	//bool mode = false;
	//while( blah < 8 )
	//{
	//	if( !mode )
	//	{
	//		//double leap

	//	}
	//}
	


	//nodes[1][0]->connects[0] = nodes[0][0];
	//nodes[1][0]->connects[0] = nodes[0][0];
	//for( int i = 0; i < 8; ++i )
	//{
	//	for( int j = 0; j < 8; ++j )
	//	{
	//		blah[i][j] = new FlowerNode( basePos + Vector2i( length * i, length * j ) );
	//	}
	//}
	
	
}