#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Movement.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )


Jay::Jay( GameSession *owner, bool p_hasMonitor, Vector2i &startPos, 
	sf::Vector2i &endPos )
	:Enemy( owner, EnemyType::PATROLLER, p_hasMonitor, 1 ), deathFrame( 0 ),
	jayVA( sf::Quads, 2 * 4 ), shieldVA( sf::Quads, 2 * 4 )
	//pathVA( sf::Quads, 4 )
{
	receivedHit = NULL;
	position.x = startPos.x;
	position.y = startPos.y;

	redPos = position;
	bluePos = V2d( endPos.x, endPos.y );
	redNodePos = position;

	int tileHeight = 64;
	//int mults = ceil( dist 
	double dist = length( bluePos - redPos );
	double numT = dist / tileHeight; //rounded down
	int numTiles = numT;
	double remainder = numT - numTiles;
	if( remainder > 0 )
	{
		numTiles += 1;
	}
	numWallTiles = numTiles;
	int numVertices = numTiles * 4;

	wallVA = new VertexArray( sf::Quads, numVertices );
	localWallPoints = new Vector2f[numVertices];

	initHealth = 40;
	health = initHealth;

	double left = min( redPos.x, bluePos.x );
	double top = min( redPos.y, bluePos.y );
	double right = max( redPos.x, bluePos.x );
	double bot = max( redPos.y, bluePos.y );

	double width = right - left;
	double height = bot - top;


	spawnRect = sf::Rect<double>( left, top, width, height );
	
	actionLength[PROTECTED] = 10;
	actionLength[FIRE] = 10;
	actionLength[RECOVER] = 10;

	animFactor[PROTECTED] = 1;
	animFactor[FIRE] = 1;
	animFactor[RECOVER] = 1;

	//speed = 2;
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Jay.png", 80, 80 );
	ts = owner->GetTileset( "jay_100x100.png", 100, 100 );
	ts_shield = owner->GetTileset( "jayshield_128x128.png", 128, 128 );
	ts_connection = owner->GetTileset( "jaywall_64x64.png", 64, 64 ); 

	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( position.x, position.y );
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


	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	triggerBox.type = CollisionBox::Hit;
	triggerBox.isCircle = false;
	//SetupWaiting();

	//seq.AddLineMovement( point0, point1, CubicBezier( 0, 0, 1, 1 ), moveFrames );
	
	//seq1.AddLineMovement( point1, point0, CubicBezier( 0, 0, 1, 1 ), moveFrames );
	ResetEnemy();
	
	UpdatePath();

	wallTileWidth = 64;

	wallFrame = 0;
	wallDuration = 1;
	wallAnimFactor = 10;

	SetupJays();
	SetupWall();
}

void Jay::UpdateJays()
{
	IntRect redRect = ts->GetSubRect( 0 );
	IntRect blueRect = ts->GetSubRect( 1 );

	jayVA[0].texCoords = Vector2f( redRect.left, redRect.top );
	jayVA[1].texCoords = Vector2f( redRect.left + redRect.width, redRect.top );
	jayVA[2].texCoords = Vector2f( redRect.left + redRect.width, redRect.top + redRect.height );
	jayVA[3].texCoords = Vector2f( redRect.left, redRect.top + redRect.height );

	jayVA[4].texCoords = Vector2f( blueRect.left, blueRect.top );
	jayVA[5].texCoords = Vector2f( blueRect.left + blueRect.width, blueRect.top );
	jayVA[6].texCoords = Vector2f( blueRect.left + blueRect.width, blueRect.top + blueRect.height );
	jayVA[7].texCoords = Vector2f( blueRect.left, blueRect.top + blueRect.height );
}

void Jay::SetupJays()
{
	IntRect testRect = ts->GetSubRect( 0 );
	//IntRect blueRect = ts->GetSubRect( 1 );
	jayVA[0].position = Vector2f( redPos.x - testRect.width, redPos.y - testRect.height );
	jayVA[1].position = Vector2f( redPos.x + testRect.width, redPos.y - testRect.height );
	jayVA[2].position = Vector2f( redPos.x + testRect.width, redPos.y + testRect.height );
	jayVA[3].position = Vector2f( redPos.x - testRect.width, redPos.y + testRect.height );

	jayVA[0].position = Vector2f( bluePos.x - testRect.width, bluePos.y - testRect.height );
	jayVA[1].position = Vector2f( bluePos.x + testRect.width, bluePos.y - testRect.height );
	jayVA[2].position = Vector2f( bluePos.x + testRect.width, bluePos.y + testRect.height );
	jayVA[3].position = Vector2f( bluePos.x - testRect.width, bluePos.y + testRect.height );
}

void Jay::SetupWall()
{
	VertexArray &wva = *wallVA;

	V2d along = normalize( bluePos - redPos );
	V2d other( along.y, -along.x );

	double wallHeight = 64 / 2;

	V2d start = redPos;
	V2d end;
	for( int i = 0; i < numWallTiles; ++i )
	{
		start = redPos + (i * wallTileWidth) * along;
		if( i == numWallTiles - 1 )
		{
			end = bluePos;
		}
		else
		{
			end = start + wallTileWidth * along;
		}

		V2d topLeft = start + wallHeight * other;
		V2d topRight = end + wallHeight * other;

		V2d botRight = end - wallHeight * other;
		V2d botLeft = start - wallHeight * other;

		
		localWallPoints[i*4+0] = Vector2f( topLeft.x, topLeft.y ) - Vector2f( redPos.x, redPos.y );
		localWallPoints[i*4+1] = Vector2f( topRight.x, topRight.y ) - Vector2f( redPos.x, redPos.y );
		localWallPoints[i*4+2] = Vector2f( botRight.x, botRight.y ) - Vector2f( redPos.x, redPos.y );
		localWallPoints[i*4+3] = Vector2f( botLeft.x, botLeft.y ) - Vector2f( redPos.x, redPos.y );
	}
}

void Jay::UpdateWall()
{
	VertexArray &wva = *wallVA;
	if( action == FIRE )
	{
		for( int i = 0; i < numWallTiles * 4; ++i )
		{
			wva[i].position = Vector2f( redNodePos.x, redNodePos.y ) + localWallPoints[i];
		}
	}

	//add more when you have it changing states
	IntRect ir = ts_wall->GetSubRect( wallFrame / wallAnimFactor );
	for( int i = 0; i < numWallTiles-1; ++i )
	{
		wva[i*4+0].texCoords = Vector2f( ir.left, ir.top );
		wva[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		wva[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		wva[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
	}
	
	wva[(numWallTiles-1)*4+0].texCoords = Vector2f( ir.left, ir.top );
	wva[(numWallTiles-1)*4+1].texCoords = Vector2f( ir.left + remainder, ir.top );
	wva[(numWallTiles-1)*4+2].texCoords = Vector2f( ir.left + remainder, ir.top + ir.height );
	wva[(numWallTiles-1)*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
}

void Jay::SetupWaiting()
{
	V2d normDiff = moveDir;

	V2d midPoint = position + moveDir * moveDistance / 2.0;

	triggerBox.globalAngle = atan2( -normDiff.x, normDiff.y );
	triggerBox.globalPosition = midPoint;
	triggerBox.rw = 20;
	triggerBox.rh = moveDistance / 2;
	
	angle = atan2( normDiff.y, normDiff.x ) / PI * 180.f;
	sprite.setRotation( angle );
	UpdateHitboxes();
}

void Jay::UpdatePath()
{
	V2d startPoint = position;
	V2d endPoint = position + moveDir * moveDistance;

	V2d along = normalize( endPoint - startPoint );
	V2d other( along.y, -along.x );

	double halfWidth = 10;

	V2d topLeft = startPoint + other * halfWidth;
	V2d topRight = endPoint + other * halfWidth;
	V2d botRight = endPoint - other * halfWidth;
	V2d botLeft = startPoint - other * halfWidth;

	pathVA[0].position = Vector2f( topLeft.x, topLeft.y );
	pathVA[1].position = Vector2f( topRight.x, topRight.y );
	pathVA[2].position = Vector2f( botRight.x, botRight.y );
	pathVA[3].position = Vector2f( botLeft.x, botLeft.y );

	sprite.setRotation( atan2( moveDir.y, moveDir.x ) / PI * 180.f );

	if( action != CHARGE_START )
	{
		pathVA[0].color = Color::White;
		pathVA[1].color = Color::White;
		pathVA[2].color = Color::White;
		pathVA[3].color = Color::White;
	}
	else
	{
		pathVA[0].color = Color::Magenta;
		pathVA[1].color = Color::Magenta;
		pathVA[2].color = Color::Magenta;
		pathVA[3].color = Color::Magenta;
	}
}

void Jay::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case WAITING:
			frame = 0;
			break;
		case CHARGE_START:
			action = CHARGE_REPEAT;
			frame = 0;
			seq.Reset();
			seq.movementList->start = position;
			seq.movementList->end = position + moveDir * moveDistance;
			break;
		case CHARGE_REPEAT:
			frame = 0;
			break;
		case TURNING:
			facingRight = !facingRight;
			action = WAITING;
			frame = 0;

			break;
		}
	}
}

void Jay::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Jay::ResetEnemy()
{
	if( origEndPoint.x < origStartPoint.x )
		facingRight = false;
	else
	{
		facingRight = true;
	}
	

	seq.Reset();
	//seq1.Reset();
	start0 = false;
	triggered = false;
	dead = false;
	deathFrame = 0;
	position = origStartPoint;
	receivedHit = NULL;
	action = WAITING;
	frame = 0;
	slowMultiple = 1;
	slowCounter = 1;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Jay::UpdatePrePhysics()
{
	ActionEnded();
	switch( action )
	{
	case WAITING:
		{
			
		}
		break;
	case CHARGE_START:
		break;
	case CHARGE_REPEAT:
		break;
	case TURNING:
		break;
	}

	if( !dead && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		

		//cout << "health now: " << health << endl;
		//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		//owner->Pause( 5 );
		health -= 20;
		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			//AttemptSpawnMonitor();
			dead = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}
		receivedHit = NULL;
		
		
	}

	triggered = false;
}

void Jay::UpdatePhysics()
{
	//cout << "setting to targetnode: " << targetNode << endl;
	//position = V2d( path[targetNode].x, path[targetNode].y );
	specterProtected = false;

	if( !dead )
	{
		//if( 
		if( action == CHARGE_REPEAT )
		{
			if( seq.currMovement == NULL )
			{
				//cout << "turning 0" << endl;
				action = TURNING;
				
				frame = 0;
				V2d oldStart = seq.movementList->start;
				V2d oldEnd = seq.movementList->end;
				moveDir = normalize( oldStart - oldEnd ); 
				UpdatePath();
				SetupWaiting();
			}
			else
			{
				seq.Update( slowMultiple );
				position = seq.position;
			}
		}
		
		//cout << "position: " << position.x << ", " << position.y << 
		//	", newpos: " << testSeq.position.x 
		//	<< ", " << testSeq.position.y << endl;
		
		//PhysicsResponse();
	}
	
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

	if( PlayerSlowingWall() )
	{
		if( slowMultipleWall == 1 )
		{
			slowCounterWall = 1;
			slowMultipleWall = 5;
		}
	}
	else
	{
		slowMultipleWall = 1;
		slowCounterWall = 1;
	}

	if( dead )
		return;


	PhysicsResponse();
}

void Jay::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
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
			//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 6, 3, true );
			

		//	cout << "Jay received damage of: " << receivedHit->damage << endl;
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
		//	cout << "Jay just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		if( action == WAITING && !triggered )
		{
			if( triggerBox.Intersects( owner->player->hurtBody ) )
			{
				triggered = true;
			}
		}
	}
}

void Jay::UpdatePostPhysics()
{
	if( triggered )
	{
		action = CHARGE_START;
		frame = 0;
		currMoveFrame = 0;

		
		//start0 = !start0;

		//seq.Reset();

		if( start0 )
		{
			
		}
		else
		{
		//	seq1.Reset();
		}

	}

	if( !dead && receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	/*if( receivedHit != NULL && !dead )
	{
		
		
	}*/

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	if( dead && deathFrame == 60 )
	{
		//owner->ActivateEffect( ts_testBlood, position, true, 0, 15, 2, true );
		owner->RemoveEnemy( this );
		//return;
	}

	if( action == CHARGE_START )
	{
		V2d playerDir = normalize( owner->player->position - position );
		moveDir = playerDir;
		UpdatePath();
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
}

void Jay::UpdateSprite()
{
	//ts->GetSubRect( frame / animationFactor ) );
	
	IntRect ir;// = ts->GetSubRect( 0 );
	switch( action )
	{
		case WAITING:
			ir = ts->GetSubRect( 0 );
			break;
		case CHARGE_START:
			ir = ts->GetSubRect( 1 );
			break;
		case CHARGE_REPEAT:
			ir = ts->GetSubRect( 2 );
			break;
		case TURNING:
			ir = ts->GetSubRect( frame / animFactor[TURNING] + 4 );
			break;
	}

	
	if( facingRight )
	{
		sprite.setTextureRect( ir );
	}
	else
	{
		ir.left += ir.width;
		ir.width = -ir.width;
		//ir.top += ir.height;
		//ir.height = -ir.height;
		sprite.setTextureRect( ir );
		//sprite.setRotation( -angle );
	}
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( position.x, position.y );


	if( dead )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 9 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 8 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
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
}

void Jay::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		target->draw( pathVA );
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

		
		target->draw( topDeathSprite );
	}



}

void Jay::DrawMinimap( sf::RenderTarget *target )
{
	/*CircleShape enemyCircle;
	enemyCircle.setFillColor( COLOR_BLUE );
	enemyCircle.setRadius( 50 );
	enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
	enemyCircle.setPosition( position.x, position.y );
	target->draw( enemyCircle );

	if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/

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

bool Jay::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}

	if( action == FIRE )
	{
		if( wallHitBody.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( wallHitboxInfo );
			return true;
		}

		if( wallNodeHitboxRed.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( wallHitboxInfo );
			return true;
		}

		if( wallNodeHitboxBlue.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( wallHitboxInfo );
			return true;
		}
	}


	return false;
}

void Jay::UpdateHitboxes()
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
pair<bool,bool> Jay::PlayerHitMe()
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

bool Jay::PlayerSlowingMe()
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

bool Jay::PlayerSlowingWall()
{
	Actor *player = owner->player;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			V2d A = wallHitBody.globalPosition + V2d( -wallHitBody.rw * cos( wallHitBody.globalAngle ) + -c.rh * -sin( c.globalAngle ), -c.rw * sin( c.globalAngle ) + -wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			V2d B = wallHitBody.globalPosition + V2d( wallHitBody.rw * cos( wallHitBody.globalAngle ) + -c.rh * -sin( c.globalAngle ), c.rw * sin( c.globalAngle ) + -wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			V2d C = wallHitBody.globalPosition + V2d( wallHitBody.rw * cos( wallHitBody.globalAngle ) + c.rh * -sin( c.globalAngle ), c.rw * sin( c.globalAngle ) + wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			V2d D = wallHitBody.globalPosition + V2d( -wallHitBody.rw * cos( wallHitBody.globalAngle ) + c.rh * -sin( c.globalAngle ), -c.rw * sin( c.globalAngle ) + wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			
			V2d bPos = player->bubblePos[i];

			if( IsQuadTouchingCircle( A, B, C, D, bPos, player->bubbleRadius ) )
			{
				return true;
			}

			if( length( redNodePos - bPos ) < player->bubbleRadius + wallNodeHitboxRed.rw )
			{
				return true;
			}

			V2d blueNode = redNodePos + origDiff;
			if( length( blueNode - bPos ) < player->bubbleRadius + wallNodeHitboxBlue.rw )
			{
				return true;
			}
		}
	}
	return false;
}

void Jay::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
		triggerBox.DebugDraw( target );
	}
}

void Jay::SaveEnemyState()
{
	//stored.dead = dead;
	///stored.deathFrame = deathFrame;
	//stored.forward = forward;
//	stored.frame = frame;
	//stored.hitlagFrames = hitlagFrames;
	///stored.hitstunFrames = hitstunFrames;
	//stored.position = position;
//	stored.targetNode = targetNode;
}

void Jay::LoadEnemyState()
{
//	dead = stored.dead;
//	deathFrame = stored.deathFrame;
//	forward = stored.forward;
	//frame = stored.frame;
	//hitlagFrames = stored.hitlagFrames;
	//hitstunFrames = stored.hitstunFrames;
//	position = stored.position;
///	targetNode = stored.targetNode;
}