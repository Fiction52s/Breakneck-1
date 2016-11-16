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
	:Enemy( owner, EnemyType::PATROLLER, p_hasMonitor, 1 ),
	jayVA( sf::Quads, 2 * 4 ), shieldVA( sf::Quads, 2 * 4 )
	//pathVA( sf::Quads, 4 )
{
	dead = false;
	receivedHitRed = NULL;
	receivedHitBlue = NULL;
	position.x = startPos.x;
	position.y = startPos.y;

	redPos = position;
	bluePos = V2d( endPos.x, endPos.y );
	redNodePos = redPos;
	origDiff = bluePos - redPos;


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
	redHealth = initHealth;
	blueHealth = initHealth;
	//health = initHealth;

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
	actionLength[WAITTOFIRE] = 10;

	animFactor[PROTECTED] = 1;
	animFactor[FIRE] = 6;
	animFactor[RECOVER] = 1;
	animFactor[WAITTOFIRE] = 3;

	//speed = 2;
	frame = 0;

	animationFactor = 5;

	redHurtBody.globalPosition = redPos;
	redHurtBody.globalAngle = 0;
	redHitBody.globalPosition = redPos;
	redHitBody.globalAngle = 0;

	blueHurtBody.globalPosition = bluePos;
	blueHurtBody.globalAngle = 0;
	blueHitBody.globalPosition = bluePos;
	blueHitBody.globalAngle = 0;

	//ts = owner->GetTileset( "Jay.png", 80, 80 );
	ts = owner->GetTileset( "jay_100x100.png", 100, 100 );
	ts_shield = owner->GetTileset( "jayshield_128x128.png", 128, 128 );
	ts_wall = owner->GetTileset( "jaywall_64x64.png", 64, 64 ); 

	//sprite.setTexture( *ts->texture );
	//sprite.setTextureRect( ts->GetSubRect( frame ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	//sprite.setPosition( position.x, position.y );
	//position.x = 0;
	//position.y = 0;

	V2d normDiff = normalize( origDiff );

	V2d midPoint = ( redPos + bluePos ) / 2.0;

	redHurtBody.type = CollisionBox::Hurt;
	redHurtBody.isCircle = true;
	redHurtBody.globalAngle = 0;
	redHurtBody.offset.x = 0;
	redHurtBody.offset.y = 0;
	redHurtBody.rw = 16;
	redHurtBody.rh = 16;

	redHitBody.type = CollisionBox::Hit;
	redHitBody.isCircle = true;
	redHitBody.globalAngle = 0;
	redHitBody.offset.x = 0;
	redHitBody.offset.y = 0;
	redHitBody.rw = 16;
	redHitBody.rh = 16;

	blueHurtBody.type = CollisionBox::Hurt;
	blueHurtBody.isCircle = true;
	blueHurtBody.globalAngle = 0;
	blueHurtBody.offset.x = 0;
	blueHurtBody.offset.y = 0;
	blueHurtBody.rw = 16;
	blueHurtBody.rh = 16;

	blueHitBody.type = CollisionBox::Hit;
	blueHitBody.isCircle = true;
	blueHitBody.globalAngle = 0;
	blueHitBody.offset.x = 0;
	blueHitBody.offset.y = 0;
	blueHitBody.rw = 16;
	blueHitBody.rh = 16;

	//wallHitBody.type = CollisionBox::Hit;
	//wallHitBody.isCircle = false;
	//wallHitBody.globalAngle = 0;
	//wallHitBody.offset.x = 0;
	//wallHitBody.offset.y = 0;
	//wallHitBody.rw = 16;
	//wallHitBody.rh = 16;

	triggerBox.type = CollisionBox::Hit;
	triggerBox.isCircle = false;
	triggerBox.globalAngle = atan2( -normDiff.x, normDiff.y );
	triggerBox.globalPosition = midPoint;
	triggerBox.rw = 20;
	triggerBox.rh = length( origDiff ) / 2.0;

	wallHitBody.type = CollisionBox::Hit;
	wallHitBody.isCircle = false;
	wallHitBody.globalAngle = atan2( -normDiff.x, normDiff.y );
	wallHitBody.globalPosition = midPoint;
	wallHitBody.rw = 20;
	wallHitBody.rh = length( origDiff ) / 2.0;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	wallHitboxInfo = new HitboxInfo;
	wallHitboxInfo->damage = 18;
	wallHitboxInfo->drainX = 0;
	wallHitboxInfo->drainY = 0;
	wallHitboxInfo->hitlagFrames = 0;
	wallHitboxInfo->hitstunFrames = 10;
	wallHitboxInfo->knockback = 4;

	shieldHitboxInfo = new HitboxInfo;
	shieldHitboxInfo->damage = 18;
	shieldHitboxInfo->drainX = 0;
	shieldHitboxInfo->drainY = 0;
	shieldHitboxInfo->hitlagFrames = 0;
	shieldHitboxInfo->hitstunFrames = 10;
	shieldHitboxInfo->knockback = 4;

	
	//hitboxInfo->kbDir;


	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	
	//SetupWaiting();

	//seq.AddLineMovement( point0, point1, CubicBezier( 0, 0, 1, 1 ), moveFrames );
	
	//seq1.AddLineMovement( point1, point0, CubicBezier( 0, 0, 1, 1 ), moveFrames );
	ResetEnemy();
	
	//UpdatePath();

	wallTileWidth = 64;

	wallFrame = 0;
	wallDuration = 1;
	wallAnimFactor = 10;

	SetupJays();
	SetupWall();

	

	
}

pair<bool,bool> Jay::PlayerHitMe()
{
	return pair<bool,bool>(false, false);
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
	jayVA[0].position = Vector2f( redPos.x - testRect.width / 2, redPos.y - testRect.height / 2 );
	jayVA[1].position = Vector2f( redPos.x + testRect.width / 2, redPos.y - testRect.height / 2 );
	jayVA[2].position = Vector2f( redPos.x + testRect.width / 2, redPos.y + testRect.height / 2 );
	jayVA[3].position = Vector2f( redPos.x - testRect.width / 2, redPos.y + testRect.height / 2 );

	jayVA[0].position = Vector2f( bluePos.x - testRect.width / 2, bluePos.y - testRect.height / 2 );
	jayVA[1].position = Vector2f( bluePos.x + testRect.width / 2, bluePos.y - testRect.height / 2 );
	jayVA[2].position = Vector2f( bluePos.x + testRect.width / 2, bluePos.y + testRect.height / 2 );
	jayVA[3].position = Vector2f( bluePos.x - testRect.width / 2, bluePos.y + testRect.height / 2 );
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

void Jay::UpdatePath()
{
	/*V2d startPoint = position;
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
	}*/
}

void Jay::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case PROTECTED:
			frame = 0;
			break;
		case WAITTOFIRE:
			action = FIRE;
			frame = 0;
			break;
		case FIRE:
			action = RECOVER;
			frame = 0;
			break;
		}
	}
}

void Jay::HandleEntrant( QuadTreeEntrant *qte )
{
	/*SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}*/
}

void Jay::ResetEnemy()
{
	if( origEndPoint.x < origStartPoint.x )
		facingRight = false;
	else
	{
		facingRight = true;
	}

	triggered = false;
	dead = false;
	redDead = false;
	blueDead = false;
	//deathFrame = 0;
	//position = origStartPoint;
	receivedHitRed = NULL;
	receivedHitBlue = NULL;
	action = PROTECTED;
	frame = 0;

	redDeathFrame = 0;
	blueDeathFrame = 0;

	slowMultiple = 1;
	slowCounter = 1;

	wallFrame = 0;
	shieldFrame = 0;

	slowCounterWall = 1;
	slowMultipleWall = 1;
	redNodePos = redPos;
	//UpdateWall();

	UpdateHitboxes();

	//UpdateSprite();
	health = initHealth;
	
}

void Jay::UpdatePrePhysics()
{
	


	if( dead )
	{
		owner->RemoveEnemy( this );
		return;
	}

	ActionEnded();


	if( redDead && redDeathFrame == 60 && !blueDead )
	{
		action = SOLO;
		frame = 0;
		//dying = true;
	}
	else if( blueDead && blueDeathFrame == 60 && !redDead )
	{
		action = SOLO;
		frame = 0;
		//dying = true;
	}

	switch( action )
	{
	case PROTECTED:
		{
			cout << "protected: " << frame << endl;
		}
		break;
	case FIRE:
		cout << "fire: " << frame << endl;
		break;
	case RECOVER:
		cout << "recover: " << frame << endl;
		break;
	case SOLO:
		cout << "solo: " << frame << endl;
		break;
	}

	if( !redDead && receivedHitRed != NULL && (action == FIRE || action == RECOVER ) )
	{
		redHealth -= 20;
		if( redHealth <= 0 )
		{
			if( !blueDead && hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			redDead = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}
		receivedHitRed = NULL;
	}

	if( !blueDead && receivedHitBlue != NULL && (action == FIRE || action == RECOVER ) )
	{
		blueHealth -= 20;
		if( blueHealth <= 0 )
		{
			if( !redDead && hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			blueDead = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}
		receivedHitBlue = NULL;
	}

	triggered = false;
}

void Jay::UpdatePhysics()
{
	specterProtected = false;

	
	if( action == FIRE )
	{
		redNodePos += wallVel / NUM_STEPS / (double)slowMultipleWall;
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
	UpdateHitboxes();

	Actor *player = owner->player;
	if( !redDead && receivedHitRed == NULL )
	{
		pair<bool,bool> result = PlayerHitRed();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( 6, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}
		}

		if( redHitBody.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( hitboxInfo );
			//return true;
		}
	}

	if( !blueDead && receivedHitBlue == NULL )
	{
		pair<bool,bool> result = PlayerHitBlue();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( 6, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}
		}

		if( blueHitBody.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( hitboxInfo );
			//return true;
		}
	}


	if( IHitPlayer() )
	{
	//	cout << "Jay just hit player for " << hitboxInfo->damage << " damage!" << endl;
	}

	if( action == PROTECTED && !triggered )
	{
		if( triggerBox.Intersects( owner->player->hurtBody ) )
		{
			triggered = true;
		}
	}
}

void Jay::UpdatePostPhysics()
{
	if( triggered )
	{
		action = WAITTOFIRE;
		frame = 0;
		Actor *player = owner->player;
		if( player->ground == NULL && player->grindEdge == NULL )
		{
			wallVel = player->velocity;
		}
		else if( player->ground == NULL )
		{
			wallVel = normalize( player->grindEdge->v1 - player->grindEdge->v0 ) * player->grindSpeed;
		}
		else
		{
			wallVel = normalize( player->ground->v1 - player->ground->v0 ) * player->groundSpeed;
		}
	}

	if( !redDead && receivedHitRed != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + redPos ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}
	if( !blueDead && receivedHitBlue != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + bluePos ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	/*if( receivedHit != NULL && !dead )
	{
		
		
	}*/

	if( redDeathFrame == 0 && redDead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, redPos, true, 0, 15, 2, true );
	}
	if( blueDeathFrame == 0 && blueDead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, bluePos, true, 0, 15, 2, true );
	}

	


	UpdateSprite();

	if( slowCounter == slowMultiple )
	{
		++frame;
		
		slowCounter = 1;
	
		if( redDead )
		{
			redDeathFrame++;
		}
		if( blueDead )
		{
			blueDeathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( slowCounterWall == slowMultipleWall )
	{
		++wallFrame;
		++shieldFrame;
		slowCounterWall = 1;
	
		

	}
	else
	{
		slowCounterWall++;
	}
}

void Jay::UpdateSprite()
{
	//ts->GetSubRect( frame / animationFactor ) );
	
	//IntRect ir;// = ts->GetSubRect( 0 );
	//switch( action )
	//{
	//	case WAITING:
	//		ir = ts->GetSubRect( 0 );
	//		break;
	//	case CHARGE_START:
	//		ir = ts->GetSubRect( 1 );
	//		break;
	//	case CHARGE_REPEAT:
	//		ir = ts->GetSubRect( 2 );
	//		break;
	//	case TURNING:
	//		ir = ts->GetSubRect( frame / animFactor[TURNING] + 4 );
	//		break;
	//}

	//
	//if( facingRight )
	//{
	//	sprite.setTextureRect( ir );
	//}
	//else
	//{
	//	ir.left += ir.width;
	//	ir.width = -ir.width;
	//	//ir.top += ir.height;
	//	//ir.height = -ir.height;
	//	sprite.setTextureRect( ir );
	//	//sprite.setRotation( -angle );
	//}
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	//sprite.setPosition( position.x, position.y );
	UpdateJays();

	if( dead )
	{
		/*botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 9 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 8 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );*/
	}
	else
	{
		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			/*keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );*/

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
			if( owner->pauseFrames < 2 || ( receivedHitRed == NULL && receivedHitBlue == NULL ) )
			{
				sf::RenderStates rs;
				rs.texture = ts->texture;
				rs.shader = keyShader;
				target->draw( jayVA, rs );
				//target->draw( sprite, keyShader );
			}
			else
			{
				target->draw( jayVA, hurtShader );
				//target->draw( sprite, hurtShader );
			}
			target->draw( *keySprite );
		}
		else
		{
			if( owner->pauseFrames < 2 || ( receivedHitRed == NULL && receivedHitBlue == NULL ) )
			{
				target->draw( jayVA, ts->texture );
			//	target->draw( sprite );
			}
			else
			{
				sf::RenderStates rs;
				rs.shader = hurtShader;
				rs.texture = ts->texture;
				target->draw( jayVA, hurtShader );
			}
			
		}

		
	}
	else
	{
		//target->draw( botDeathSprite );

		
		//target->draw( topDeathSprite );
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

	/*if( !dead )
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
	}*/
}

bool Jay::IHitPlayer()
{
	Actor *player = owner->player;
	
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
	if( action == FIRE )
	{
		V2d midPoint = redNodePos + origDiff / 2.0;
		//wallHitBody.globalPosition = midPoint;
		//wallNodeHitboxRed.globalPosition = redNodePos;
		//wallNodeHitboxBlue.globalPosition = redNodePos + origDiff;

	}

	if( owner->player->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->player->groundSpeed * ( owner->player->ground->v1 - owner->player->ground->v0 ) );
		wallHitboxInfo->kbDir = normalize( -owner->player->groundSpeed * ( owner->player->ground->v1 - owner->player->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->player->velocity );
		wallHitboxInfo->kbDir = normalize( -owner->player->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Jay::PlayerHitRed()
{
	Actor *player = owner->player;
	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( redHurtBody.Intersects( (*it) ) )
			{
				hit = true;
				hitRed = true;
				break;
			}
			
		}
		

		if( hit )
		{
			receivedHitRed = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	//for( int i = 0; i < player->recordedGhosts; ++i )
	//{
	//	if( player->ghostFrame < player->ghosts[i]->totalRecorded )
	//	{
	//		if( player->ghosts[i]->currHitboxes != NULL )
	//		{
	//			bool hit = false;
	//			
	//			for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
	//			{
	//				if( hurtBody.Intersects( (*it) ) )
	//				{
	//					hit = true;
	//					break;
	//				}
	//			}
	//	

	//			if( hit )
	//			{
	//				receivedHit = player->currHitboxInfo;
	//				return pair<bool, bool>(true,true);
	//			}
	//		}
	//		//player->ghosts[i]->curhi
	//	}
	//}

	return pair<bool, bool>(false,false);
}

pair<bool,bool> Jay::PlayerHitBlue()
{
	Actor *player = owner->player;
	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( blueHurtBody.Intersects( (*it) ) )
			{
				hit = true;
				hitRed = true;
				break;
			}
			
		}
		

		if( hit )
		{
			receivedHitBlue = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	//for( int i = 0; i < player->recordedGhosts; ++i )
	//{
	//	if( player->ghostFrame < player->ghosts[i]->totalRecorded )
	//	{
	//		if( player->ghosts[i]->currHitboxes != NULL )
	//		{
	//			bool hit = false;
	//			
	//			for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
	//			{
	//				if( hurtBody.Intersects( (*it) ) )
	//				{
	//					hit = true;
	//					break;
	//				}
	//			}
	//	

	//			if( hit )
	//			{
	//				receivedHit = player->currHitboxInfo;
	//				return pair<bool, bool>(true,true);
	//			}
	//		}
	//		//player->ghosts[i]->curhi
	//	}
	//}

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
			V2d A = wallHitBody.globalPosition + V2d( -wallHitBody.rw * cos( wallHitBody.globalAngle ) + -wallHitBody.rh * -sin( wallHitBody.globalAngle ), -wallHitBody.rw * sin( wallHitBody.globalAngle ) + -wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			V2d B = wallHitBody.globalPosition + V2d( wallHitBody.rw * cos( wallHitBody.globalAngle ) + -wallHitBody.rh * -sin( wallHitBody.globalAngle ), wallHitBody.rw * sin( wallHitBody.globalAngle ) + -wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			V2d C = wallHitBody.globalPosition + V2d( wallHitBody.rw * cos( wallHitBody.globalAngle ) + wallHitBody.rh * -sin( wallHitBody.globalAngle ), wallHitBody.rw * sin( wallHitBody.globalAngle ) + wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			V2d D = wallHitBody.globalPosition + V2d( -wallHitBody.rw * cos( wallHitBody.globalAngle ) + wallHitBody.rh * -sin( wallHitBody.globalAngle ), -wallHitBody.rw * sin( wallHitBody.globalAngle ) + wallHitBody.rh * cos( wallHitBody.globalAngle ) );
			
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
		redHurtBody.DebugDraw( target );
		blueHurtBody.DebugDraw( target );

		redHitBody.DebugDraw( target );
		blueHitBody.DebugDraw( target );
		//hitBody.DebugDraw( target );
		//triggerBox.DebugDraw( target );
		wallNodeHitboxRed.DebugDraw( target );
		wallNodeHitboxBlue.DebugDraw( target );
		wallHitBody.DebugDraw( target );
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