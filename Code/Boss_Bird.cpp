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

Boss_Bird::Boss_Bird( GameSession *owner, Vector2i pos )
	:Enemy( owner, EnemyType::TURTLE ), deathFrame( 0 ), moveBez( 0, 0, 1, 1 ),
	DOWN( 0, 1 ), LEFT( -1, 0 ), RIGHT( 1, 0 ), UP( 0, -1 ), pathVA( sf::Quads, MAX_PATH_SIZE * 4 )
{
	testCircle.setRadius( 30 );
	testCircle.setFillColor( Color::Red );
	testCircle.setOrigin( testCircle.getLocalBounds().width / 2, 
		testCircle.getLocalBounds().height / 2 );

	testFinalCircle.setRadius( 30 );
	testFinalCircle.setFillColor( Color::Black );
	testFinalCircle.setOrigin( testFinalCircle.getLocalBounds().width / 2, 
		testFinalCircle.getLocalBounds().height / 2 );
	ClearPathVA();

	nodeTravelFrames = 20;
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
	//loop = false; //no looping on Boss_Bird for now

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
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;

	deathFrame = 0;
	
	launcher = new Launcher( this, owner, 12, 12, position, V2d( 1, 0 ), 2 * PI, 90, true );
	launcher->SetBulletSpeed( bulletSpeed );	

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	//animationFactor = 5;

	//ts = owner->GetTileset( "Boss_Bird.png", 80, 80 );
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

void Boss_Bird::ResetEnemy()
{
	travelFrame = 0;
	travelIndex = 0;
	action = PLANMOVE;
	ClearPathVA();
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

void Boss_Bird::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Boss_Bird::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	b->launcher->DeactivateBullet( b );
}

void Boss_Bird::BulletHitPlayer(BasicBullet *b )
{
	owner->player.ApplyHit( b->launcher->hitboxInfo );
}

void Boss_Bird::ActionEnded()
{
	if( frame == actionLength[action] )
	{
	switch( action )
	{
	
	}
	}
}

void Boss_Bird::UpdatePrePhysics()
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

	++travelFrame;
	if( travelFrame == nodeTravelFrames )
	{
		travelFrame = 0;
		travelIndex++;
		if( travelIndex == pathSize )
		{
			frame = 0;
			moveIndex = finalIndex;
		}
	}
	switch( action )
	{
	case PLANMOVE:
		if( frame == 0 )
		{
			CreatePath();
			travelFrame = 0;
			travelIndex = 0;			
		}
		break;
	case MOVE:
		break;
	case SHOOT:
		break;
	}

	UpdatePathVA();
	

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

	//if( !dying && !dead && action == FIRE && frame == actionLength[FIRE] - 1 )// frame == 0 && slowCounter == 1 )
	//{
	//	//cout << "firing" << endl;
	//	launcher->position = position;
	//	launcher->facingDir = normalize( owner->player.position - position );
	//	//cout << "shooting bullet at: " << launcher->facingDir.x <<", " <<
	//	//	launcher->facingDir.y << endl;
	//	launcher->Fire();
	//	fireCounter = 0;
	//	//testLauncher->Fire();
	//}

	/*if( latchedOn )
	{
		basePos = owner->player.position + offsetPlayer;
	}*/
}

void Boss_Bird::UpdatePhysics()
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
			Actor &player = owner->player;
			if( length( player.position - position ) < 300 )
			{
				action = FADEOUT;
				frame = 0;
			}
		}*/
		PhysicsResponse();
	}
	return;
}

void Boss_Bird::PhysicsResponse()
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
			
		//	cout << "Boss_Bird received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Boss_Bird just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Boss_Bird::ClearPathVA()
{
	for( int i = 0; i < MAX_PATH_SIZE * 4; ++i )
	{
		pathVA[i].position = Vector2f( 0, 0 );
		pathVA[i].color = COLOR_GREEN;
	}
}

void Boss_Bird::UpdatePostPhysics()
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

void Boss_Bird::UpdateSprite()
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

void Boss_Bird::Draw( sf::RenderTarget *target )
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
			target->draw( cs );
		}
		
		
		target->draw( sprite );
		target->draw( pathVA );

		if( action == PLANMOVE )
		{
			target->draw( testFinalCircle );
			target->draw( testCircle );
			
		}
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

void Boss_Bird::DrawMinimap( sf::RenderTarget *target )
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

bool Boss_Bird::IHitPlayer()
{

	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Boss_Bird::UpdateHitboxes()
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
pair<bool,bool> Boss_Bird::PlayerHitMe()
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

bool Boss_Bird::PlayerSlowingMe()
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

void Boss_Bird::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Boss_Bird::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Boss_Bird::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}

bool Boss_Bird::DirIsValid( sf::Vector2i &testIndex,
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

void Boss_Bird::CreatePath()
{
	ClearPathVA();
	sf::Vector2i testIndex = moveIndex;

	if( testIndex.x == 0 && testIndex.y == 0 )	
	{
		int r = rand() % 2;
		if( r == 0 )
		{
			path[0] = DOWN;
		}
		else
		{
			path[0] = RIGHT;
		}
	}
	else if( testIndex.x == GRID_SIZE-1 && testIndex.y == GRID_SIZE-1 )
	{
		int r = rand() % 2;
		if( r == 0 )
		{
			path[0] = UP;
		}
		else
		{
			path[0] = LEFT;
		}
	}
	else if( testIndex.x == 0 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = UP;
		}
		else if( r == 1 )
		{
			path[0] = RIGHT;
		}
		else
		{
			path[0] = DOWN;
		}
	}
	else if( testIndex.y == 0 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = DOWN;
		}
		else if( r == 1 )
		{
			path[0] = LEFT;
		}
		else
		{
			path[0] = RIGHT;
		}
	}
	else if( testIndex.x == GRID_SIZE-1 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = DOWN;
		}
		else if( r == 1 )
		{
			path[0] = LEFT;
		}
		else
		{
			path[0] = UP;
		}
	}
	else if( testIndex.y == GRID_SIZE-1 )
	{
		int r = rand() % 3;
		if( r == 0 )
		{
			path[0] = LEFT;
		}
		else if( r == 1 )
		{
			path[0] = RIGHT;
		}
		else
		{
			path[0] = UP;
		}
	}
	else
	{
		int r = rand() % 4;
		if( r == 0 )
		{
			path[0] = LEFT;
		}
		else if( r == 1 )
		{
			path[0] = RIGHT;
		}
		else if( r == 2 )
		{
			path[0] = DOWN;
		}
		else
		{
			path[0] = UP;
		}
	}

	testIndex += path[0];
	for( int i = 1; i < pathSize; ++i )
	{
		int numOptions = 0;
		
		sf::Vector2i leftTurn( path[i-1].y, -path[i-1].x );
		bool validLeftTurn = DirIsValid( testIndex, leftTurn );
		sf::Vector2i rightTurn( -path[i-1].y, path[i-1].x );
		bool validRightTurn = DirIsValid( testIndex, rightTurn );
		sf::Vector2i forward = path[i-1];
		bool validForward = DirIsValid( testIndex, forward );
		
		if( validLeftTurn && validRightTurn && validForward )
		{
			cout << "a" << endl;
			int r = rand() % 3;
			if( r == 0 )
			{
				path[i] = leftTurn;
			}
			else if( r == 1 )
			{
				path[i] = rightTurn;
			}
			else
			{
				path[i] = forward;
			}
		}
		else if( validLeftTurn && validRightTurn )
		{
			cout << "b" << endl;
			int r = rand() % 2;
			if( r == 0 )
			{
				path[i] = leftTurn;
			}
			else
			{
				path[i] = rightTurn;
			}
		}
		else if( validForward && validRightTurn )
		{
			cout << "c" << endl;
			int r = rand() % 2;
			if( r == 0 )
			{
				path[i] = forward;
			}
			else
			{
				path[i] = rightTurn;
			}
		}
		else if( validLeftTurn && validForward )
		{
			cout << "d" << endl;
			int r = rand() % 2;
			if( r == 0 )
			{
				path[i] = leftTurn;
			}
			else
			{
				path[i] = forward;
			}
		}
		else if( validLeftTurn )
		{
			path[i] = leftTurn;
		}
		else if( validForward )
		{
			path[i] = forward;
		}
		else if( validRightTurn )
		{
			path[i] = rightTurn;
		}
		else
		{
			Vector2i blahLeft = testIndex + leftTurn;
			Vector2i blahRight = testIndex + rightTurn;
			Vector2i blahForward = testIndex + forward;
			cout << (int)validLeftTurn << ", " << (int)validRightTurn << ", " << (int)validForward << endl;
			cout << "left: " << blahLeft.x << ", " << blahLeft.y << 
				", right: " << blahRight.x << ", " << blahRight.y << 
				", forward: " << blahForward.x << ", " << blahForward.y << endl;
			assert( 0 && "what options is this" );
		}

		
		cout << "testindex: " << testIndex.x 
			<< ", " << testIndex.y << ", path[ " << i << "]: " << path[i].x << ", "<< path[i].y << endl;
		testIndex += path[i];
	}

	finalIndex = testIndex;
	cout << "finalIndex: " << finalIndex.x << ", " << finalIndex.y << endl;

	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
	V2d trueRight( gridRatio, 1.0 / gridRatio );
	V2d trueDown( -gridRatio, 1.0 / gridRatio );
	V2d trueUp( gridRatio, -1.0 / gridRatio );

	V2d gridIndexPos = trueRight * (double)finalIndex.x + trueDown * (double)finalIndex.y;
	gridIndexPos *= gridSizeRatio;
	V2d curr = gridIndexPos + gridOriginPos;

	testFinalCircle.setPosition( curr.x, curr.y );
}

void Boss_Bird::UpdatePathVA()
{
	V2d trueLeft( -gridRatio, -1.0 / gridRatio );
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
	}
}