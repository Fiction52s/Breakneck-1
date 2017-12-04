#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gorilla.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )


Gorilla::Gorilla( GameSession *owner, bool p_hasMonitor, Vector2i &pos, 
	int wallWidth, int p_followFrames )//, int p_recoveryLoops )
	:Enemy( owner, EnemyType::GORILLA, p_hasMonitor, 5 ), deathFrame( 0 ), approachAccelBez( 1,.01,.86,.32 ) 
{
	affectCameraZoom = false;	
	idealRadius = 300;
	followFrames = p_followFrames;
	recoveryLoops = 1;
	wallWidth = 400;
	wallHitboxWidth = wallWidth;
	wallHitboxHeight = 50; //wallHeight;

	wallHitbox.isCircle = false;
	wallHitbox.rw = wallHitboxWidth / 2;
	wallHitbox.rh = wallHitboxHeight / 2;

	actionLength[WAKEUP] = 60;
	actionLength[ALIGN] = 60;
	actionLength[FOLLOW] = followFrames;
	actionLength[ATTACK] = 4;//60;
	actionLength[RECOVER] = 2;//60;

	actionLength[WAKEUP] = 1;
	animFactor[ALIGN] = 1;
	animFactor[FOLLOW] = 1;
	animFactor[ATTACK] = 15;
	animFactor[RECOVER] = 60;

	action = WAKEUP;

	latchedOn = false;

	awakeCap = 60;
	//offsetPlayer 
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	origPosition = position;

	approachFrames = 180 * 3;
	totalFrame = 0;

	//latchedOn = true; 
	//V2d dirFromPlayer = normalize( owner->GetPlayer( 0 )->position - position );
	//double fromPlayerAngle =  atan2( dirFromPlayer.y, dirFromPlayer.x ) + PI;
	//cout << "dirfrom: " << dirFromPlayer.x << ", " << dirFromPlayer.y << endl;
	//cout << "from player angle: " << fromPlayerAngle << endl;
	//testSeq.AddRadialMovement( 1, 0, 2 * PI * 3, 
	//	true, V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1), approachFrames );
	
	//testSeq.InitMovementDebug();

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 64, pos.y - 64, 64 * 2, 64 * 2 );

	basePos = position;
	
	//speed = pspeed;

	//speed = 2;
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Gorilla.png", 80, 80 );
	ts = owner->GetTileset( "gorilla_320x256.png", 320, 256 );

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

	ts_wall = owner->GetTileset( "gorillawall_400x50.png", 400, 50 );
	wallSprite.setTexture( *ts_wall->texture );
	wallSprite.setTextureRect( ts_wall->GetSubRect( 0 ) );
	wallSprite.setOrigin( wallSprite.getLocalBounds().width / 2, wallSprite.getLocalBounds().height / 2 );

	//hitboxInfo->kbDir;
	awakeFrames = 0;
	
	
	latchStartAngle = 0;
	dead = false;

	//awake = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	deathPartingSpeed = .4;
	deathVector = V2d( 1, -1 );

	facingRight = true;
	 
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	origFacingRight = facingRight;

	int detectionSize = 64; //need to make resting sprite larger, ending sprite smaller
	detectionRect = sf::Rect<double>( position.x - detectionSize, position.y - detectionSize,
		detectionSize * 2, detectionSize * 2 );

	slowMultiple = 1;
	slowCounter = 1;
	recoveryCounter = 0;
	createWallFrame = 20;
}

void Gorilla::HandleEntrant( QuadTreeEntrant *qte )
{

}

void Gorilla::ResetEnemy()
{
	action = WAKEUP;
	facingRight = origFacingRight;
	affectCameraZoom = false;	
	//awake = false;
	awakeFrames = 0;
	latchStartAngle = 0;
	latchedOn = false;
	totalFrame = 0;
	//testSeq.Reset();
	//testSeq.Update();
	dead = false;
	deathFrame = 0;
	frame = 0;
	basePos = origPosition;
	position = basePos;
	
	receivedHit = NULL;
	slowMultiple = 1;
	slowCounter = 1;
	recoveryCounter = 0;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Gorilla::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case WAKEUP:
			frame = 0;
			break;
		case ALIGN:
			frame = 0;
			break;
		case FOLLOW:
			action = ATTACK;
			frame = 0;
			break;
		case ATTACK:
			recoveryCounter = 0;
			action = RECOVER;
			frame = 0;
			break;
		case RECOVER:
			++recoveryCounter;
			//if( recoveryCounter == recoveryLoops )
			{
				latchedOn = true;
				offsetPlayer = basePos - owner->GetPlayer( 0 )->position;//owner->GetPlayer( 0 )->position - basePos;
				origOffset = offsetPlayer;//length( offsetPlayer );
				V2d offsetDir = normalize( offsetPlayer );
				//latchStartAngle = atan2( offsetDir.y, offsetDir.x );
				//cout << "latchStart: " << latchStartAngle << endl;
				//testSeq.Update();
				basePos = owner->GetPlayer( 0 )->position;


				V2d playerPos = owner->GetPlayer( 0 )->position;
				action = ALIGN;
				double currRadius = length( offsetPlayer );
				alignMoveFrames = 60 * 5 * NUM_STEPS;//(int)((abs(idealRadius - currRadius)) / 3.0) * NUM_STEPS * 5;
				frame = 0;
				alignFrames = 0;
				recoveryCounter = 0;

				if( playerPos.x < position.x )
				{
					facingRight = false;
				}
				else
				{
					facingRight = true;
				}
				
				//affectCameraZoom = true;
				
				//cout << "recover move frames: " << alignMoveFrames << endl;

					
				//cout << "JUST LATCHING NOW" << endl;
				

				//double currRadius = length( offsetPlayer );
				//alignMoveFrames = (int)((abs(idealRadius - currRadius)) / 3.0) * NUM_STEPS * 5;
			}
			break;
		}
	}
}

void Gorilla::UpdatePrePhysics()
{
	ActionEnded();

	V2d playerPos = owner->GetPlayer( 0 )->position;

	switch( action )
	{
	case WAKEUP:
		{
			//cout << "wakeup" << endl;
			Camera &cam = owner->cam;
			double camWidth = 960 * cam.GetZoom();
			double camHeight = 540 * cam.GetZoom();
			sf::Rect<double> screenRect( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );

			if( screenRect.intersects( detectionRect ) )
			{
				++awakeFrames;
				if( awakeFrames == awakeCap )
				{
					//awake = true;
					action = ALIGN;
					alignFrames = 0;
					affectCameraZoom = true;
					frame = 0;
					if( playerPos.x < position.x )
					{
						facingRight = false;
					}
					else
					{
						facingRight = true;
					}

					
					//cout << "JUST LATCHING NOW" << endl;
					latchedOn = true;
					offsetPlayer = basePos - owner->GetPlayer( 0 )->position;//owner->GetPlayer( 0 )->position - basePos;
					origOffset = offsetPlayer;//length( offsetPlayer );
					V2d offsetDir = normalize( offsetPlayer );
					//latchStartAngle = atan2( offsetDir.y, offsetDir.x );
					//cout << "latchStart: " << latchStartAngle << endl;
					//testSeq.Update();
					basePos = owner->GetPlayer( 0 )->position;

					double currRadius = length( offsetPlayer );
					alignMoveFrames = 60 * 5 * NUM_STEPS;//(int)((abs(idealRadius - currRadius)) / 3.0) * NUM_STEPS * 5;
					//launchStartAngle / PI * 180;
					
				}
			}
			else
			{
				awakeFrames--;
				if( awakeFrames < 0 )
					awakeFrames = 0;
			}
		}
		break;
	case ALIGN:
		{
			//cout << "wakeup" << endl;
			//cout << "align: " << alignFrames << ", move frames: " << alignMoveFrames << endl;
			if( alignFrames >= alignMoveFrames )
			{
				action = FOLLOW;
				frame = 0;
			}
		}
		break;
	case FOLLOW:
		{
			//cout << "follow" << endl;	
		}
		break;
	case ATTACK:

		//cout << "attack" << endl;
		if( frame == createWallFrame )
		{

			//position = playerPos + offsetPlayer;
			
			
			V2d test = position - playerPos;
			//origOffset = test;
			

			//V2d n = normalize( origOffset );
			//V2d n1 = normalize( test );

			//cout << "origoffset: " << n.x << ", " << n.y << endl;
			//cout << "realoffset: " << n1.x << ", " << n1.y << endl;

			V2d playerDir = -normalize( origOffset );
			wallHitbox.globalPosition = position + playerDir * 100.0;
			wallHitbox.globalAngle = atan2( playerDir.x, -playerDir.y );// / PI * 180.0;

			wallSprite.setPosition( wallHitbox.globalPosition.x, 
				wallHitbox.globalPosition.y );
			wallSprite.setRotation( wallHitbox.globalAngle / PI * 180.0 );
			//cout << "angle: " << wallHitbox.globalAngle << endl;
			//wallHitbox.globalAngle 

			latchedOn = false;
			basePos = position;
		}
		break;
	case RECOVER:
		//cout << "recover" << endl;
		break;
	}


	//if( action == APPROACH && offsetPlayer.x == 0 && offsetPlayer.y == 0 )
	//{
	//	action = BITE;
	//	frame = 0;
	//}

	if( !dead && receivedHit != NULL )
	{
		//owner->Pause( 5 );
		
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
		}

		receivedHit = NULL;
	}	
}

void Gorilla::UpdatePhysics()
{
	if( dead )
		return;

	specterProtected = false;
	if( latchedOn )
	{
		basePos = owner->GetPlayer( 0 )->position;// + offsetPlayer;
	}
	else
	{
		
		
	}

	V2d offsetDir = normalize( offsetPlayer );

	switch( action )
	{
	case WAKEUP:
		//frame = 0;
		break;
	case ALIGN:
		{
			V2d idealOffset = offsetDir * idealRadius;
			CubicBezier b( 0, 0, 1, 1 );
			double f = b.GetValue( (alignFrames + owner->substep) / (double)alignMoveFrames );
			offsetPlayer = origOffset * (1.0 - f) + idealOffset * f;
			alignFrames += 5 / slowMultiple;
			//cout << "moveFrames: " << alignMoveFrames << ", alignFrames: " << alignFrames << 
			//	"substep: " << owner->substep << endl;
			//++alignFrames;
		}
		//frame = 0;
		break;
	case FOLLOW:
		//cout << "follow: " << frame << endl;
		//frame = 0;
		break;
	case ATTACK:
		//cout << "ATTACK: " << frame << endl;
		//action = RECOVER;
		//frame = 0;
		break;
	case RECOVER:
		//cout << "recovery: " << frame << endl;
		//action = ALIGN;
		//frame = 0;
		break;
	}

	if( latchedOn )
	{
		position = basePos + offsetPlayer;
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

	

	PhysicsResponse();
}

void Gorilla::PhysicsResponse()
{
	

	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->GetPlayer( 0 )->ConfirmHit( 6, 5, .8, 6 );


			if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
			{
				owner->GetPlayer( 0 )->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->GetPlayer( 0 )->frame << endl;

			//owner->GetPlayer( 0 )->frame--;
			//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, facingRight );
			
		//	cout << "Gorilla received damage of: " << receivedHit->damage << endl;
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
			//cout << "Gorilla hit player Gorilla pos: " <<
			//	position.x << ", " << position.y << ", playerpos: "
			//	<< owner->GetPlayer( 0 )->position.x << ", " << owner->GetPlayer( 0 )->position.y << endl;
		//	cout << "Gorilla just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Gorilla::UpdatePostPhysics()
{
	/*if( latchedOn )
	{
		basePos = owner->GetPlayer( 0 )->position;
		position = basePos + offsetPlayer;
	}*/

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
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}
	
	UpdateSprite();

	if( slowCounter == slowMultiple )
	{
		++frame;
		//if( latchedOn && totalFrame < approachFrames )
		//{
		//	++totalFrame;
		//}
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

	/*if( frame == 10 * animationFactor )
	{
		frame = 0;
	}*/

	//cout << "action: " << action << endl; 

	

	
}

void Gorilla::UpdateSprite()
{
	//close is 2
	//3 is biting
	if( !dead )
	{
		V2d diff = owner->GetPlayer( 0 )->position - position;
		double lenDiff = length( diff );
		IntRect ir;
		switch( action )
		{
		case WAKEUP:
			ir = ts->GetSubRect( 0 );
			break;
		case ALIGN:
			ir = ts->GetSubRect( 0 );
			break;
		case FOLLOW:
			ir = ts->GetSubRect( 1 );
			break;
		case ATTACK:
			ir = ts->GetSubRect( frame / animFactor[ATTACK] + 2 );
			break;
		case RECOVER:
			ir = ts->GetSubRect( frame / animFactor[RECOVER] + 6 );
			break;
		}
		
		if( !facingRight )
		{
			ir = sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height );
		}

		sprite.setTextureRect( ir  );

		sprite.setPosition( position.x, position.y );
	}
	else
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

void Gorilla::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
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

		if( (action == ATTACK && frame > createWallFrame) || action == RECOVER )
		{
			target->draw( wallSprite );
		}
		
		//cout << "drawing bat: " << sprite.getPosition().x
		//	<< ", " << sprite.getPosition().y << endl;
	}
	else
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

void Gorilla::DrawMinimap( sf::RenderTarget *target )
{
	//cout << "Draw minimap" << endl;
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

bool Gorilla::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	

	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	else if( ( ( action == ATTACK && frame > createWallFrame ) || action == RECOVER ) && wallHitbox.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo ); //replace later
		return true;
	}
	
	/*if( action == EXPLODE )
	{
		if( hitBody.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( hitboxInfo );
			return true;
		}
	}*/
	return false;
}

void Gorilla::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Gorilla::PlayerHitMe( int index )
{
	if( action == WAKEUP )
		return pair<bool,bool>(false,false);

	Actor *player = owner->GetPlayer( 0 );
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
			//player->Gorillas[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Gorilla::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
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

void Gorilla::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		/*if( testSeq.currMovement != NULL )
		{
			if( testSeq.currMovement->vertices != NULL )
			{
				testSeq.currMovement->DebugDraw( target );
			}
		}*/
		if( ( action == ATTACK && frame >= createWallFrame ) || action == RECOVER )
		{
			wallHitbox.DebugDraw( target );

			/*V2d playerPos = owner->GetPlayer( 0 )->position;
			sf::Vertex blah[2] = { 
				Vertex( Vector2f( position.x, position.y ), Color::Red ),
				Vertex( Vector2f( playerPos.x, playerPos.y ), Color::Red ) };
*/
			//target->draw( blah, 2, sf::Lines );
		}
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}


//camera bug
//show rectangle
//adjust damage
void Gorilla::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
}

void Gorilla::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
}