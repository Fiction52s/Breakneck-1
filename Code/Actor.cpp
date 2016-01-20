#include "Actor.h"
#include "GameSession.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>
#define COLOR_TEAL Color( 0, 0xee, 0xff )

Actor::Actor( GameSession *gs )
	:owner( gs ), dead( false )
	{
		lastWire = 0;
		inBubble = false;
		oldInBubble = false;
		hasRedKey = false;
		hasGreenKey = false;
		hasBlueKey = false;
		slopeTooSteepLaunchLimitX = .1;
		steepSlideFastGravFactor = 1.9 / 3.0;
		steepSlideGravFactor = 1.3 / 3.0;
		
		steepClimbGravFactor = .7;
		steepClimbFastFactor = .5;

		
		//testLight = owner->ActivateLight( 200, 15, COLOR_TEAL );
		//testLight->pos = Vector2i( 0, 0 );
		testLight = new Light( owner, Vector2i( 0, 0 ), COLOR_TEAL , 200, 15 ); 

		//activeEdges = new Edge*[16]; //this can probably be really small I don't think it matters. 
		//numActiveEdges = 0;
		assert( Shader::isAvailable() && "help me" );
		if (!sh.loadFromFile("player_shader.frag", sf::Shader::Fragment))
		//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
		{
			cout << "PLAYER SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "player shader not loaded" );
		}

		/*if( !timeSlowShader.loadFromFile( "timeslow_shader.frag", sf::Shader::Fragment ) )
		{
			cout << "TIME SLOW SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "time slow shader not loaded" );
		}*/

		ts_dodecaSmall = owner->GetTileset( "dodecasmall.png", 180, 180 );
		ts_dodecaBig = owner->GetTileset( "dodecabig.png", 360, 360 );

		

		if( !fairBuffer.loadFromFile( "fair.ogg" ) )
		{
			assert( 0 && "failed to load test fair noise" );
		}
		fairSound.setBuffer( fairBuffer);
		fairSound.setVolume( 0 );

		if( !runTappingBuffer.loadFromFile( "runtapping.ogg" ) )
		{
			assert( 0 && "failed to load test runtapping noise" );
		}
		runTappingSound.setBuffer( runTappingBuffer);
		runTappingSound.setVolume( 0 );

		if( !playerHitBuffer.loadFromFile( "playerhit.ogg" ) )
		{
			assert( 0 && "failed to load test runtapping noise" );
		}
		playerHitSound.setBuffer( playerHitBuffer );
		playerHitSound.setVolume( 0 );

		if( !dashStartBuffer.loadFromFile( "dashstart.ogg" ) )
		{
			assert( 0 && "failed to load test dashstart noise" );
		}
		dashStartSound.setBuffer( dashStartBuffer);
		dashStartSound.setVolume( 0 );

		//testBuffer.loadFromSamples( dashStartBuffer.getSamples(), dashStartBuffer.getSampleCount(),
		//	dashStartBuffer.getChannelCount(), dashStartBuffer.getSampleRate() / 5 );

		//Vector2i lightPos( position.x, position.y );
		//Color lightColor = Color::Green;
		//playerLight = new Light( owner, lightPos, lightColor );
		//dashStartSound.setBuffer( testBuffer );
		
		
		framesGrinding = 0;
		percentCloneChanged = 0;
		percentCloneRate = .01;
		changingClone = false;

		desperationMode = false;
		maxDespFrames = 120;
		despCounter = 0;

		framesSinceClimbBoost = 0;
		climbBoostLimit = 5;

		holdJump = false;

		bounceBoostSpeed = 4.7;

		offsetX = 0;
		sprite = new Sprite;
		velocity = Vector2<double>( 0, 0 );
		
		CollisionBox cb;
		cb.type = CollisionBox::Hit;
		cb.isCircle = true;
		cb.offset.x = 32;
		cb.offset.y = -8;
		//cb.offsetAngle = 0;
		cb.rw = 64;
		cb.rh = 64;

		for( int i = 0; i < MAX_GHOSTS; ++i )
		{
			ghosts[i] = new PlayerGhost;
		}

		//setup hitboxes
		{
		for( int j = 4; j < 10; ++j )
		{
			fairHitboxes[j] = new list<CollisionBox>;
			fairHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->fairHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->fairHitboxes[j]->push_back( cb );			
			}
		}


		cb.offset.x = 0;
		cb.offset.y = -8;

		for( int j = 0; j <= 5; ++j )
		{
			uairHitboxes[j] = new list<CollisionBox>;
			uairHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->uairHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->uairHitboxes[j]->push_back( cb );			
			}
		}


		cb.offset.x = 0;
		cb.offset.y = 32;
		for( int j = 0; j <= 9; ++j )
		{
			dairHitboxes[j] = new list<CollisionBox>;
			dairHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->dairHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->dairHitboxes[j]->push_back( cb );			
			}
		}

		cb.rw = 48;
		cb.rh = 48;
		cb.offset.x = 32;
		cb.offset.y = 0;
		for( int j = 1; j <= 4; ++j )
		{
			standNHitboxes[j] = new list<CollisionBox>;
			standNHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->standNHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->standNHitboxes[j]->push_back( cb );			
			}
		}

		cb.rw = 48;
		cb.rh = 48;
		cb.offset.x = 32;
		cb.offset.y = 0;
		for( int j = 2; j <= 7; ++j )
		{
			standDHitboxes[j] = new list<CollisionBox>;
			standDHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->standDHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->standDHitboxes[j]->push_back( cb );			
			}
		}

		cb.rw = 48;
		cb.rh = 48;
		cb.offset.x = 0;
		cb.offset.y = -70;
		for( int j = 2; j <= 7; ++j )
		{
			standUHitboxes[j] = new list<CollisionBox>;
			standUHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->standUHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->standUHitboxes[j]->push_back( cb );			
			}
		}
		}

		queryMode = "";
		wallThresh = .9999;
		//tileset setup
		{

		bounceFlameOn = false;
		
		ts_airBounceFlame = owner->GetTileset( "bouncejumpflame.png", 128, 128 );
		ts_runBounceFlame = owner->GetTileset( "bouncerunflame.png", 128, 96 );

		ts_bounceBoost = owner->GetTileset( "bounceboost_256x192.png", 256, 192 );

		
		airBounceFlameFrames = 20 * 3;
		runBounceFlameFrames = 21 * 3;

		actionLength[DAIR] = 10 * 2;
		tileset[DAIR] = owner->GetTileset( "dair.png", 96, 64 );
		normal[DAIR] = owner->GetTileset( "dair_NORMALS.png", 96, 64 );

		actionLength[DASH] = 45;
		tileset[DASH] = owner->GetTileset( "dash.png", 128, 64 );
		normal[DASH] = owner->GetTileset( "dash_NORMALS.png", 64, 64 );

		actionLength[DOUBLE] = 28 + 10;
		tileset[DOUBLE] = owner->GetTileset( "double.png", 64, 64 );
		normal[DOUBLE] = owner->GetTileset( "double_NORMALS.png", 64, 64 );
		
		actionLength[FAIR] = 10 * 2;
		tileset[FAIR] = owner->GetTileset( "fair.png", 80, 64 );
		normal[FAIR] = owner->GetTileset( "fair_NORMALS.png", 80, 64 );

		actionLength[JUMP] = 2;
		tileset[JUMP] = owner->GetTileset( "jump.png", 64, 64 );
		normal[JUMP] = owner->GetTileset( "jump_NORMALS.png", 64, 64 );

		actionLength[LAND] = 1;
		tileset[LAND] = owner->GetTileset( "land.png", 64, 64 );
		normal[LAND] = owner->GetTileset( "land_NORMALS.png", 64, 64 );

		actionLength[LAND2] = 1;
		tileset[LAND2] = owner->GetTileset( "land2.png", 64, 64 );
		normal[LAND2] = owner->GetTileset( "land2_NORMALS.png", 64, 64 );

		actionLength[RUN] = 10 * 4;
		tileset[RUN] = owner->GetTileset( "run2.png", 80, 48 );
		normal[RUN] = owner->GetTileset( "run_NORMALS.png", 80, 48 );

		actionLength[SLIDE] = 1;
		tileset[SLIDE] = owner->GetTileset( "slide.png", 64, 64 );
		normal[SLIDE] = owner->GetTileset( "slide_NORMALS.png", 64, 64 );

		actionLength[SPRINT] = 8 * 3;
		tileset[SPRINT] = owner->GetTileset( "sprint.png", 128, 64 );		
		//tileset[SPRINT] = owner->GetTileset( "sprint_96x48.png", 96, 48 );		
		normal[SPRINT] = owner->GetTileset( "sprint_NORMALS.png", 128, 64 );		

		actionLength[STAND] = 20 * 8;
		tileset[STAND] = owner->GetTileset( "stand.png", 64, 64 );
		normal[STAND] = owner->GetTileset( "stand_NORMALS.png", 64, 64 );

		actionLength[STANDD] = 8 * 2;
		tileset[STANDD] = owner->GetTileset( "standd.png", 96, 48 );
		normal[STANDD] = owner->GetTileset( "standd_NORMALS.png", 96, 48 );

		actionLength[STANDN] = 5 * 2;
		tileset[STANDN] = owner->GetTileset( "standn.png", 128, 64 );
		normal[STANDN] = owner->GetTileset( "standn_NORMALS.png", 128, 64 );

		actionLength[STANDU] = 20;
		tileset[STANDU] = owner->GetTileset( "standu.png", 64, 80 );
		normal[STANDU] = owner->GetTileset( "standu_NORMALS.png", 64, 80 );

		actionLength[UAIR] = 6 * 3;
		tileset[UAIR] = owner->GetTileset( "uair.png", 80, 80 );
		normal[UAIR] = owner->GetTileset( "uair_NORMALS.png", 80, 80 );

		actionLength[WALLCLING] = 1;
		tileset[WALLCLING] = owner->GetTileset( "wallcling.png", 64, 64 );
		normal[WALLCLING] = owner->GetTileset( "wallcling_NORMALS.png", 64, 64 );

		actionLength[WALLJUMP] = 9 * 2;
		tileset[WALLJUMP] = owner->GetTileset( "walljump.png", 64, 64 );
		normal[WALLJUMP] = owner->GetTileset( "walljump_NORMALS.png", 64, 64 );

		actionLength[GRINDBALL] = 1;
		tileset[GRINDBALL] = owner->GetTileset( "grindball.png", 32, 32 );
		normal[GRINDBALL] = owner->GetTileset( "grindball_NORMALS.png", 32, 32 );

		actionLength[STEEPSLIDE] = 1;
		tileset[STEEPSLIDE] = owner->GetTileset( "steepslide.png", 64, 32 );
		normal[STEEPSLIDE] = owner->GetTileset( "steepslide_NORMALS.png", 64, 32 );

		actionLength[AIRDASH] = 27;
		tileset[AIRDASH] = owner->GetTileset( "airdash.png", 64, 64 );
		normal[AIRDASH] = owner->GetTileset( "airdash_NORMALS.png", 64, 64 );

		actionLength[STEEPCLIMB] = 8 * 4;
		tileset[STEEPCLIMB] = owner->GetTileset( "steepclimb.png", 128, 64 );
		normal[STEEPCLIMB] = owner->GetTileset( "steepclimb_NORMALS.png", 128, 64 );

		actionLength[AIRHITSTUN] = 1;
		tileset[AIRHITSTUN] = owner->GetTileset( "hurt_air.png", 64, 64 );
		normal[AIRHITSTUN] = owner->GetTileset( "hurt_air_NORMALS.png", 64, 64 );

		actionLength[GROUNDHITSTUN] = 1;
		tileset[GROUNDHITSTUN] = owner->GetTileset( "hurt_stand.png", 64, 64 );
		normal[GROUNDHITSTUN] = owner->GetTileset( "hurt_stand_NORMALS.png", 64, 64 );

		actionLength[WIREHOLD] = 1;
		tileset[WIREHOLD] = owner->GetTileset( "steepslide.png", 64, 32 );
		normal[WIREHOLD] = owner->GetTileset( "steepslide_NORMALS.png", 64, 32 );

		actionLength[BOUNCEAIR] = 1;
		tileset[BOUNCEAIR] = owner->GetTileset( "bounce.png", 96, 96 );
		normal[BOUNCEAIR] = owner->GetTileset( "bounce.png", 96, 96 );

		actionLength[BOUNCEGROUND] = 15;
		tileset[BOUNCEGROUND] = owner->GetTileset( "bounce.png", 96, 96 );
		normal[BOUNCEGROUND] = owner->GetTileset( "bounce_NORMALS.png", 96, 96 );

		actionLength[BOUNCEGROUNDEDWALL] = 30;
		tileset[BOUNCEGROUNDEDWALL] = owner->GetTileset( "bouncegroundedwall.png", 80, 48 );
		normal[BOUNCEGROUNDEDWALL] = owner->GetTileset( "bouncegroundedwall_NORMALS.png", 80, 48 );

		actionLength[DEATH] = 44 * 2;
		tileset[DEATH] = owner->GetTileset( "death.png", 64, 64 );
		//normal[DEATH] = owner->GetTileset( "death_NORMALS.png", 64, 64 );

		

		}
		tsgsdodeca = owner->GetTileset( "dodeca.png", 64, 64 ); 	
		tsgstriblue = owner->GetTileset( "triblue.png", 64, 64 ); 	
		tsgstricym = owner->GetTileset( "tricym.png", 128, 128 ); 	
		tsgstrigreen = owner->GetTileset( "trigreen.png", 64, 64 ); 	
		tsgstrioran = owner->GetTileset( "trioran.png", 128, 128 ); 	
		tsgstripurp = owner->GetTileset( "tripurp.png", 128, 128 ); 	
		tsgstrirgb = owner->GetTileset( "trirgb.png", 128, 128 ); 	

		gsdodeca.setTexture( *tsgsdodeca->texture);
		gstriblue.setTexture( *tsgstriblue->texture);
		gstricym.setTexture( *tsgstricym->texture);
		gstrigreen.setTexture( *tsgstrigreen->texture);
		gstrioran.setTexture( *tsgstrioran->texture);
		gstripurp.setTexture( *tsgstripurp->texture);
		gstrirgb.setTexture( *tsgstrirgb->texture);

		ts_fairSword1 = owner->GetTileset( "fairsword1.png", 144, 128 );
		fairSword1.setTexture( *ts_fairSword1->texture );

		ts_dairSword1 = owner->GetTileset( "dairsword1.png", 128, 144 );
		dairSword1.setTexture( *ts_dairSword1->texture );

		ts_uairSword1 = owner->GetTileset( "uairsword1.png", 160, 128 );
		uairSword1.setTexture( *ts_uairSword1->texture );

		ts_standingNSword1 = owner->GetTileset( "standnsword1.png", 112, 80 );
		standingNSword1.setTexture( *ts_standingNSword1->texture );

		ts_standingDSword1 = owner->GetTileset( "standdsword1.png", 160, 48 );
		standingDSword1.setTexture( *ts_standingDSword1->texture );

		ts_standingUSword1 = owner->GetTileset( "standusword1.png", 64, 128 );
		standingUSword1.setTexture( *ts_standingUSword1->texture );


		ts_fx_hurtSpack = owner->GetTileset( "hurtspack.png", 64, 64 );

		ts_fx_dashStart = owner->GetTileset( "fx_dashstart.png", 96, 32 );
		ts_fx_dashRepeat = owner->GetTileset( "fx_dashrepeat.png", 96, 16 );
		ts_fx_land = owner->GetTileset( "fx_land.png", 80, 32 );
		ts_fx_bigRunRepeat = owner->GetTileset( "fx_bigrunrepeat.png", 176, 112 );

		


		//ts_bounceRun = owner->GetTileset( "bouncerun.png", 128, 64 );
		//ts_bounceSprint = owner->GetTileset( "bouncesprint.png", 128, 64 );

		grindActionLength = 32;

		action = JUMP;
		frame = 1;
		
		timeSlowStrength = 5;
		slowMultiple = 1;
		slowCounter = 1;

		reversed = false;

		grindActionCurrent = 0;

		framesInAir = 0;
		wallJumpFrameCounter = 0;
		wallJumpMovementLimit = 12; //10 frames

		steepThresh = .4; // go between 0 and 1

		gravity = 1.9;
		maxFallSpeed = 60;//100;

		wallJumpStrength.x = 10;
		wallJumpStrength.y = 25;
		clingSpeed = 3;

		dashSpeed = 12;
		offSlopeByWallThresh = dashSpeed;//18;
		slopeLaunchMinSpeed = 15;//dashSpeed * .7;
		steepClimbSpeedThresh = dashSpeed;

		jumpStrength = 27.5;

		hasDoubleJump = true;
		doubleJumpStrength = 26.5;

		ground = NULL;
		movingGround = NULL;
		groundSpeed = 0;
		maxNormalRun = 100;
	
		facingRight = true;
		collision = false;
	
		airAccel = 1.5;
		maxAirXSpeed = 100;
		
		airDashSpeed = 12;

		airSlow = 1;//.3;

		groundOffsetX = 0;

		grindEdge = NULL;
		grindMovingTerrain = NULL;
		grindQuantity = 0;
		grindSpeed = 0;

		maxRunInit = 8;
		maxAirXControl = maxRunInit;

		maxGroundSpeed = 100;
		runAccelInit = .5;
		
		runAccel = .03;
		sprintAccel = .85;

		holdDashAccel = .07;
		bounceFlameAccel = .15;

		dashHeight = 10;
		normalHeight = 20;
		doubleJumpHeight = 10;
		sprintHeight = 16;

		hasAirDash = true;
		hasGravReverse = true;

		//CollisionBox b;
		b.isCircle = false;
		//b.offsetAngle = 0;
		b.offset.x = 0;
		b.offset.y = 0;
		b.rw = 10;
		b.rh = normalHeight;
		b.type = CollisionBox::BoxType::Physics;

		
		b.type = b.Physics;

		
		hurtBody.offset.x = 0;
		hurtBody.offset.y = 0;
		hurtBody.isCircle = false;
		hurtBody.rw = 10;
		hurtBody.rh = normalHeight;
		hurtBody.type = CollisionBox::BoxType::Hurt;

		currHitboxes = NULL;
		currHitboxInfo = NULL;

		currHitboxInfo = new HitboxInfo();
		currHitboxInfo->damage = 100;
		currHitboxInfo->drain = 0;
		currHitboxInfo->hitlagFrames = 0;
		currHitboxInfo->hitstunFrames = 30;
		currHitboxInfo->knockback = 0;

		receivedHit = NULL;
		hitlagFrames = 0;
		hitstunFrames = 0;
		invincibleFrames = 0;

		/*wireEdge = NULL;
		wireState = 0;
		pointNum = 0;
		maxLength = 100;
		minLength = 32;*/

		leftWire = new Wire( this, false );
		rightWire = new Wire( this, true );

		bounceEdge = NULL;
		bounceMovingTerrain = NULL;
		bounceGrounded = false;


		record = false;
		blah = false;

		touchEdgeWithLeftWire= false;
		touchEdgeWithRightWire= false;
		ghostFrame = 0;

		recordedGhosts = 0;

		ts_bubble = owner->GetTileset( "timemiddle.png", 160, 160 );
		bubbleSprite.setTexture( *ts_bubble->texture );

		currBubble = 0;
		bubbleRadius = 160;
		bubbleLifeSpan = 240;

		for( int i = 0; i < maxBubbles; ++i )
		{
			bubbleFramesToLive[i] = 0;
			//bubblePos[i]
		}
		ts_fx_airdash = owner->GetTileset( "fx_airdash.png", 32, 32 );
		ts_fx_double = owner->GetTileset( "fx_double.png", 80 , 60 );
		ts_fx_gravReverse = owner->GetTileset( "fx_gravreverse.png", 64 , 32 );



		


		bool noPowers = false;
		if( noPowers )
		{
			hasPowerAirDash = false;
			hasPowerGravReverse = false;
			hasPowerBounce = false;
			hasPowerGrindBall = false;
			hasPowerTimeSlow = false;
			hasPowerLeftWire = false;
			hasPowerRightWire = false;
			hasPowerClones = 0;
		}
		else
		{
			hasPowerAirDash = true;
			hasPowerGravReverse = true;
			hasPowerBounce = true;
			hasPowerGrindBall = true;
			hasPowerTimeSlow = true;

			//wire still under development
			hasPowerLeftWire = true;
			hasPowerRightWire = true;
			//hasPowerClones = MAX_GHOSTS;
			hasPowerClones = 0;
		}

		//do this a little later.
		


		//only set these parameters again if u get a power or lose one.
		sh.setParameter( "hasPowerAirDash", hasPowerAirDash );
		sh.setParameter( "hasPowerGravReverse", hasPowerGravReverse );
		sh.setParameter( "hasPowerBounce", hasPowerBounce );
		sh.setParameter( "hasPowerGrindBall", hasPowerGrindBall );
		sh.setParameter( "hasPowerTimeSlow", hasPowerTimeSlow );
		sh.setParameter( "hasPowerLeftWire", hasPowerLeftWire );
		sh.setParameter( "hasPowerRightWire", hasPowerRightWire );
		//sh.setParameter( "hasPowerClones", hasPowerClones > 0 );

		//for( int i = 0; i < MAX_MOTION_GHOSTS; ++i )
		//{
		//	motionGhosts[i] = 
		//}
	}

void Actor::ActionEnded()
{
	if( frame >= actionLength[action] )
	{
		switch( action )
		{
		case STAND:
			frame = 0;
			break;
		case RUN:
			frame = 0;
			break;
		case JUMP:
			frame = 1;
			break;
		case LAND:
			frame = 0;
			break;
		case LAND2:
			frame = 0;
			break;
		case WALLCLING:
			frame = 0;
			break;
		case WALLJUMP:
			action = JUMP;
			frame = 1;
			break;
		case STANDN:

			if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.B )
				{
					action = DASH;
				}
				else
				{
					action = RUN;
				}
				facingRight = currInput.LRight();
			}
			else
			{
				action = STAND;	
			}
			frame = 0;
			break;
		case STANDD:
			if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.B )
				{
					action = DASH;
				}
				else
				{
					action = RUN;
				}
				facingRight = currInput.LRight();
			}
			else
			{
				action = STAND;	
			}
			frame = 0;
			break;
		case STANDU:
			if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.B )
				{
					action = DASH;
				}
				else
				{
					action = RUN;
				}
				facingRight = currInput.LRight();
			}
			else
			{
				action = STAND;	
			}
			frame = 0;
			break;
		case FAIR:
			action = JUMP;
			frame = 1;
			break;
		case DAIR:
			action = JUMP;
			frame = 1;
			break;
		case UAIR:
			action = JUMP;
			frame = 1;
			break;
		case DASH:
			action = STAND;
			frame = 0;
			break;
		case DOUBLE:
			action = JUMP;
			frame = 1;
			break;
		case SLIDE:
			frame = 0;
			break;
		case SPRINT:
			frame = 0;
			break;
		case GRINDBALL:
			frame = 0;
			break;
		case AIRDASH:
			{
				//cout << "inBubble: " << inBubble << endl;
				if( inBubble || rightWire->state == Wire::PULLING )
				{
					frame = actionLength[AIRDASH] - 1;
					airDashStall = true;
				}
				else
				{
					action = JUMP;
					frame = 1;
				}
			break;
			}
		case STEEPCLIMB:
			frame = 0;
			break;
		case AIRHITSTUN:
			frame = 0;
			break;
		case GROUNDHITSTUN:
			frame = 0;
			break;
		case WIREHOLD:
			frame = 0;
			break;
		case BOUNCEAIR:
			frame = 0;
			break;
		case BOUNCEGROUND:
			frame = 0;
			break;
		case BOUNCEGROUNDEDWALL:
			action = STAND;
			frame = 0;
		case DEATH:
		
			frame = 0;
			break;
		}
	}
}

void Actor::UpdatePrePhysics()
{
	owner->powerBar.Use( 1 );	
	

	//cout << "startvel : " << velocity.x << ", " << velocity.y << endl;	
	for( int i = MAX_MOTION_GHOSTS-1; i > 0; --i )
	{
		motionGhosts[i] = motionGhosts[i-1];
	}
	motionGhosts[0] = *sprite;

	

	if( action == DEATH )
	{
		if( frame >= actionLength[action] ) 
		{
			dead = true;
			frame = 0;
		}
		return;
	}

	if( desperationMode )
	{
		//cout << "desperation: " << despCounter << endl;
		despCounter++;
		if( despCounter == maxDespFrames )
		{
			desperationMode = false;
			action = DEATH;
			rightWire->Reset();
			leftWire->Reset();
			slowCounter = 1;
			frame = 0;
			owner->deathWipe = true;
		}
	}

	if( hasPowerClones &&  ( (currInput.RUp() && !prevInput.RUp()) || ( currInput.rightPress && !prevInput.rightPress ) ) )
	{
		if( record == 0 )
		{
			SaveState();
			owner->SaveState();
			recordedGhosts = 1;
			ghosts[record]->currFrame = 0;
			ghostFrame = 0;
			owner->powerBar.Use( 20 );
			record++;
			changingClone = true;
			percentCloneChanged = 0;
			owner->Pause( 60 );
			//percentCloneRate = .01;
		}
		else
		{
			if( recordedGhosts < MAX_GHOSTS )
			{
				cout << "creating ghost: " << recordedGhosts + 1 << ", of " << MAX_GHOSTS << endl;
				LoadState();
				owner->LoadState();
				recordedGhosts++;
				ghosts[record-1]->totalRecorded = ghosts[record-1]->currFrame;
				ghosts[record]->currFrame = 0;
				ghostFrame = 1;
				record++;
				changingClone = true;
				owner->Pause( 60 );
				percentCloneChanged = 0;
			}
			
			
		}

		
		
		//testGhost->currFrame = 0;

		//record = true;
		blah = false;
	}
		

		
	if( record > 0 && ( ( currInput.RDown() && !prevInput.RDown() ) || ghosts[record-1]->currFrame == PlayerGhost::MAX_FRAMES - 1 ) )
	{
		//record = false;
		ghosts[record-1]->totalRecorded = ghosts[record-1]->currFrame;
		record = 0;
		LoadState();
		owner->LoadState(); 
		blah = true;
		ghostFrame = 1;
		//cout << "recordedGhosts: " << recordedGhosts << endl;
		owner->powerBar.Charge( 20 );
	}



	if( reversed )
	{
		bool up = currInput.LUp();
		bool down = currInput.LDown();

		if( up ) currInput.leftStickPad -= 1;
		if( down ) currInput.leftStickPad -= 2;

		if( up ) currInput.leftStickPad += 2;
		if( down ) currInput.leftStickPad += 1;
	}

	ActionEnded();
	V2d gNorm;
	if( ground != NULL )
		gNorm = ground->Normal();

	if( receivedHit != NULL && action != DEATH )
	{
		hitlagFrames = receivedHit->hitlagFrames;
		hitstunFrames = receivedHit->hitstunFrames;
		invincibleFrames = 30;//receivedHit->damage;
		
		owner->ActivateEffect( ts_fx_hurtSpack, position, true, 0, 12, 1, facingRight );
		owner->Pause( 6 );
		//cout << "damaging player with: " << receivedHit->damage << endl;
		bool dmgSuccess = owner->powerBar.Damage( receivedHit->damage );
		if( true )
		{
			if( grindEdge != NULL )
			{
				//do something different for grind ball? you don't wanna be hit out at a sensitive moment
				owner->powerBar.Damage( receivedHit->damage ); //double damage for now
				grindSpeed *= .8;
			}
			else if( ground == NULL )
			{
				action = AIRHITSTUN;
				frame = 0;
			}
			else
			{
				action = GROUNDHITSTUN;
				frame = 0;
			}
			bounceEdge = NULL;
		}
		
		if( !dmgSuccess && !desperationMode )
		{
			desperationMode = true;
			despCounter = 0;
			//action = DEATH;
			//frame = 0;
		}



		
		receivedHit = NULL;
	}

	//cout << "hitstunFrames: " << hitstunFrames << endl;
	//choose action

	
	bool canStandUp = true;
	if( b.rh < normalHeight )
	{
		canStandUp = CheckStandUp();
		if( canStandUp )
		{
			b.rh = normalHeight;
			//cout << "setting to normal height" << endl;
			b.offset.y = 0;
		}
	}

	//cout << "can stand up: " << canStandUp << endl;

	if( bounceFlameOn && !currInput.X )
	{
		bounceFlameOn = false;
	}

	switch( action )
	{
	case STAND:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
			//	bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}


			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				bounceFlameOn = false;
				runTappingSound.stop();
				break;
			}


			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{

						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed < 0 )
							facingRight = true;
						else 
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( (groundSpeed > 0 && gNorm.x < 0) || (groundSpeed < 0 && gNorm.x > 0) )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed < 0 )
							facingRight = true;
						else 
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}

		
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			else if( currInput.B && !prevInput.B )
			{
				action = DASH;
				frame = 0;
			}
			else if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.LDown() )
				{
					action = SPRINT;
					frame = 0;
				}
				else
				{
					action = RUN;
					frame = 0;
				}
				break;
				
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LDown() )
				{
					action = STANDD;
					frame = 0;
				}
				else if( currInput.LUp() )
				{
					action = STANDU;
					frame = 0;
				}
				else
				{
					action = STANDN;
					frame = 0;
				}
			}
			else if( currInput.LDown() )
			{
				action = SLIDE;
				frame = 0;
			}
			
			break;
		}
	case RUN:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}

			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				bounceFlameOn = false;
				runTappingSound.stop();
				break;
			}


			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				runTappingSound.stop();
				break;
			}

			

			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						runTappingSound.stop();
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						runTappingSound.stop();
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						runTappingSound.stop();
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						runTappingSound.stop();
						break;
					}
				}
			}



			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LDown() )
				{
					action = STANDD;
					frame = 0;
				}
				else if( currInput.LUp() )
				{
					action = STANDU;
					frame = 0;
				}
				else
				{
					action = STANDN;
					frame = 0;
				}

				runTappingSound.stop();
				break;
			}

			if( currInput.B && !prevInput.B )
			{
				action = DASH;
				frame = 0;
				runTappingSound.stop();
				break;
			}


			bool t = (!currInput.LUp() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ));
			if(!( currInput.LLeft() || currInput.LRight() ) )//&& t )
			{
				if( currInput.LDown())
				{
					action = SLIDE;
					frame = 0;

				}
				else if( currInput.LUp() )
				{
					//stay running

					break;
				}
				else
				{
					action = STAND;
					frame = 0;
				}
				runTappingSound.stop();
				break;
				
			}
			else
			{

				if( facingRight && currInput.LLeft() )
				{
					
					if( ( currInput.LDown() && gNorm.x < 0 ) || ( currInput.LUp() && gNorm.x > 0 ) )
					{
						action = SPRINT;
					}
					
					groundSpeed = 0;
					facingRight = false;
					frame = 0;
					runTappingSound.stop();
					break;
				}
				else if( !facingRight && currInput.LRight() )
				{
					if( ( currInput.LDown() && gNorm.x > 0 ) || ( currInput.LUp() && gNorm.x < 0 ) )
					{
						action = SPRINT;
					}

					groundSpeed = 0;
					facingRight = true;
					frame = 0;
					runTappingSound.stop();
					break;
				}
				else if( (currInput.LDown() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ))
					|| (currInput.LUp() && ((gNorm.x < 0 && facingRight) || ( gNorm.x > 0 && !facingRight ) )) )
				{
					
					action = SPRINT;
					frame = frame / 4;

					if( frame < 3 )
					{
						frame = frame + 1;
					}
					else if ( frame == 8)
					{
						frame = 7;
					}

					else if ( frame == 9)
					{
						frame = 0;
					}
					frame = frame * 3;

					runTappingSound.stop();
					break;
				}

			}
			
			break;
		}
	case JUMP:
		{

			/*if( currInput.rightTrigger > 200 && prevInput.rightTrigger <= 200 && wireState == 0 )
			{
			//	action = WIREHOLD;
				framesFiring = 0;
				frame = 0;
				wireState = 1;
				break;
			}*/

			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				airBounceFrame = 0;
				oldBounceEdge = NULL;
				bounceMovingTerrain = NULL;
				break;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}

			if( hasPowerAirDash )
			{
				if( hasAirDash && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					frame = 0;
					break;
				}
			}

			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				break;
			}


			//cout << CheckWall( true ) << endl;
			
			if( CheckWall( false ) )
			{
				//cout << "special walljump right" << endl;
				if( !currInput.LDown() && currInput.LRight() && !prevInput.LRight() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = true;

					if( currInput.A )
					{
						longWallJump = true;
					}
					else
					{
						longWallJump = false;
					}
					break;
				}
			}
			
			
			if( CheckWall( true ) )
			{		
				//cout << "special walljump left" << endl;
				if( !currInput.LDown() && currInput.LLeft() && !prevInput.LLeft() )
				{
					
					action = WALLJUMP;
					frame = 0;
					facingRight = false;

					if( currInput.A )
					{
						longWallJump = true;
					}
					else
					{
						longWallJump = false;
					}
					break;
				}
			}

			

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LUp() )
				{
					action = UAIR;
					frame = 0;
				}
				else if( currInput.LDown() )
				{
					action = DAIR;
					frame = 0;
				}
				else
				{
					action = FAIR;
					frame = 0;
				}
			}

			break;
		}
	case DOUBLE:
		{

			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				airBounceFrame = 0;
				oldBounceEdge = NULL;
				bounceMovingTerrain = NULL;
				break;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}

			if( hasPowerAirDash )
			{
				if( hasAirDash && !prevInput.B && currInput.B )
				{
					action = AIRDASH;
					bounceFlameOn = false;
					airDashStall = false;
					frame = 0;
					break;
				}
			}

			if( CheckWall( false ) )
			{
				if( !currInput.LDown() && currInput.LRight() && !prevInput.LRight() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = true;

					if( currInput.A )
					{
						longWallJump = true;
					}
					else
					{
						longWallJump = false;
					}
					break;
				}
			}
			
			
			if( CheckWall( true ) )
			{				
				if( !currInput.LDown() && currInput.LLeft() && !prevInput.LLeft() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = false;

					if( currInput.A )
					{
						longWallJump = true;
					}
					else
					{
						longWallJump = false;
					}
					break;
				}
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LUp() )
				{
					action = UAIR;
					frame = 0;
				}
				else if( currInput.LDown() )
				{
					action = DAIR;
					frame = 0;
				}
				else
				{
					action = FAIR;
					frame = 0;
				}
			}

			break;
		}
	case LAND:
	case LAND2:
		{
			//buffered grind ball works
			if( hasPowerGrindBall && currInput.Y )//&& !prevInput.Y )
			{
				//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
				if( !reversed || ( hasPowerGravReverse && reversed ) )
				{
					SetActionGrind();
					break;
				}
			}


			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
				
					if( groundSpeed < 0 && gNorm.x > 0 || groundSpeed > 0 && gNorm.x < 0 )
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
							
						action = STEEPCLIMB;

						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
				}
				else
				{
					if( ( currInput.B && !( reversed && (!currInput.LLeft() && !currInput.LRight() ) ) ) || !canStandUp )
					{
						action = DASH;
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
					}
					else if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
			else
			{
			
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
				}
				else
				{
					if( currInput.B || !canStandUp )
					{
						action = DASH;
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
						facingRight = currInput.LRight();
					}
					else if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
		

			break;
		}
	case WALLCLING:
		{
			if( !currInput.LDown() && ( (facingRight && currInput.LRight()) || (!facingRight && currInput.LLeft() ) ) )
			{
				action = WALLJUMP;
				frame = 0;

				if( currInput.A )
				{
					longWallJump = true;
				}
				else
				{
					longWallJump = false;
				}
				//facingRight = !facingRight;
			}

			break;
		}
	case WALLJUMP:
		{
			if( hasPowerAirDash )
			{
				if( hasAirDash && !prevInput.B && currInput.B )
				{
					action = AIRDASH;
					airDashStall = false;
					frame = 0;
					break;
				}
			}


			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				break;
			}

			if( CheckWall( false ) )
			{
				if( !currInput.LDown() && currInput.LRight() && !prevInput.LRight() )
				{
					action = WALLJUMP;

					if( currInput.A )
					{
						longWallJump = true;
					}
					else
					{
						longWallJump = false;
					}

					frame = 0;
					facingRight = true;
					break;
				}
			}
			
			
			if( CheckWall( true ) )
			{				
				if( !currInput.LDown() && currInput.LLeft() && !prevInput.LLeft() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = false;

					if( currInput.A )
					{
						longWallJump = true;
					}
					else
					{
						longWallJump = false;
					}
					break;
				}
			}

		
			{
				if( currInput.rightShoulder && !prevInput.rightShoulder )
				{
					if( !currInput.LLeft() && !currInput.LRight() )
					{
						if( currInput.LUp() )
						{
							action = UAIR;
							frame = 0;
							break;
						}
						else if( currInput.LDown() )
						{
							action = DAIR;
							frame = 0;
							break;
						}
					}

					action = FAIR;
					frame = 0;
				}
			}
			break;
		}
	case FAIR:
		{
			break;
		}
	case DAIR:
		{
			break;
		}
	case UAIR:
		{
			break;
		}
	case DASH:
		{

			//don't break becaus eyou can cancel this
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}

			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				bounceFlameOn = false;
				SetActionGrind();

				//runTappingSound.stop();
				break;
			}

			if( reversed )
			{
				if( -gNorm.y > -steepThresh )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}

			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LDown() )
				{
					action = STANDD;
					frame = 0;
				}
				else if( currInput.LUp() )
				{
					action = STANDU;
					frame = 0;
				}
				else
				{
					action = STANDN;
					frame = 0;
				}
				break;
			}

			if( canStandUp )
			{
				if( !currInput.B )
				{
					if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
			else
			{
				if( frame == actionLength[DASH] - 2 )
					frame = 10;
			}
			break;
		}
	case SLIDE:
		{

			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}

			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				bounceFlameOn = false;
				SetActionGrind();
				break;
			}

			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				action = STANDD;
				frame = 0;
				break;
			}

			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
				
					if( groundSpeed < 0 && gNorm.x > 0 || groundSpeed > 0 && gNorm.x < 0 )
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
							
						action = STEEPCLIMB;

						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
				}
				else
				{
					//you can't dash on the ceiling with no horizontal input. probably a weakness
					if( ( currInput.B && !( reversed && (!currInput.LLeft() && !currInput.LRight() ) ) ) || !canStandUp )
					{
						action = DASH;
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
					}
					else if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
			else
			{
			
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
				}
				else
				{
					if( currInput.B || !canStandUp )
					{
						cout << "start dash" << endl;
						action = DASH;
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
						facingRight = currInput.LRight();
					}
					else if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
			break;
		









			
			/*else if( currInput.B && !prevInput.B )
			{
					action = DASH;
					frame = 0;
					break;
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				action = STANDD;
				frame = 0;
				break;
			}
			else if( !currInput.LLeft() && !currInput.LRight() )
			{
				if( !currInput.LDown() )
				{
					action = STAND;
					frame = 0;
					break;
				}
			}
			else
			{
				if( currInput.LDown() )
				{
					facingRight = currInput.LRight();
					action = SPRINT;
					frame = 0;
					break;
				}
				else
				{
					facingRight = currInput.LRight();
					action = RUN;
					frame = 0;
					break;
				}
			}*/
		}
	case SPRINT:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
			//	bounceGrounded = false;
				bounceFlameOn = false;
			}


			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				bounceFlameOn = false;
				SetActionGrind();
				break;
			}

			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			//if( canStandUp )
			//{
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LDown() )
				{
					action = STANDD;
					frame = 0;
				}
				else if( currInput.LUp() )
				{
					action = STANDU;
					frame = 0;
				}
				else
				{
					action = STANDN;
					frame = 0;
				}
				break;
			}

			
			if( currInput.B && !prevInput.B )
			{
					action = DASH;
					frame = 0;
			}

			if(!( currInput.LLeft() || currInput.LRight() ))
			{
				if( currInput.LDown())
				{
					action = SLIDE;
					frame = 0;
				}
				else if( currInput.LUp() && ( (gNorm.x < 0 && facingRight) || (gNorm.x > 0 && !facingRight) ) )
				{
					break;
				}
				else
				{
					action = STAND;
					frame = 0;
				}
				break;
				
			}
			else
			{
				if( facingRight && currInput.LLeft() )
				{
					
					if( ( currInput.LDown() && gNorm.x < 0 ) || ( currInput.LUp() && gNorm.x > 0 ) )
					{
						frame = 0;
					}
					else
					{
						action = RUN;
					}

					groundSpeed = 0;
					facingRight = false;
					frame = 0;
					break;
				}
				else if( !facingRight && currInput.LRight() )
				{
					if( ( currInput.LDown() && gNorm.x > 0 ) || ( currInput.LUp() && gNorm.x < 0 ) )
					{
						frame = 0;	
					}
					else
					{
						action = RUN;
						
					}

					groundSpeed = 0;
					facingRight = true;
					frame = 0;
					break;
				}
				else if( !( (currInput.LDown() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ))
					|| (currInput.LUp() && ((gNorm.x < 0 && facingRight) || ( gNorm.x > 0 && !facingRight ) )) ) )
				{
					action = RUN;
					frame = frame / 3;
					if( frame < 3)
					{
						frame = frame + 1;
					}
					else if ( frame == 3 || frame == 4)
					{
						frame = 7;
					}
					else if ( frame == 5 || frame == 6)
					{
						frame = 8;
					}
					else if ( frame == 7)
					{
						frame = 2;
					}
					frame = frame * 4;
					break;
				}

			}
			//}
			
			break;
		}
	case STANDN:
		{
			break;
		}
	case STANDD:
		{
			break;
		}
	case STANDU:
		{
			break;
		}
	case GRINDBALL:
		{
		
			if( !currInput.Y )//&& grindEdge->Normal().y < 0 )
			{
				V2d op = position;

				V2d grindNorm = grindEdge->Normal();

				if( grindNorm.y < 0 )
				{
					double extra = 0;
					if( grindNorm.x > 0 )
					{
						offsetX = b.rw;
						extra = .1;
					}
					else if( grindNorm.x < 0 )
					{
						offsetX = -b.rw;
						extra = -.1;
					}
					else
					{
						offsetX = 0;
					}
				
					position.x += offsetX + extra;

					position.y -= normalHeight + .1;

					if( !CheckStandUp() )
					{
						position = op;
					}
					else
					{
						framesNotGrinding = 0;
						hasAirDash = true;
						hasGravReverse = true;
						hasDoubleJump = true;
						lastWire = 0;
						ground = grindEdge;
						movingGround = grindMovingTerrain;
						edgeQuantity = grindQuantity;
						action = LAND;
						frame = 0;
						groundSpeed = grindSpeed;

						if( currInput.LRight() )
						{
							facingRight = true;
							if( groundSpeed < 0 )
							{
								groundSpeed = 0;
							}
						}
						else if( currInput.LLeft() )
						{
							facingRight = false;
							if( groundSpeed > 0 )
							{
								groundSpeed = 0;
							}
						}



						grindEdge = NULL;
						reversed = false;
					}

				}
				else
				{
					
					if( grindNorm.x > 0 )
					{
						position.x += b.rw + .1;
					}
					else if( grindNorm.x < 0 )
					{
						position.x += -b.rw - .1;
					}

					if( grindNorm.y > 0 )
						position.y += normalHeight + .1;

					if( !CheckStandUp() )
					{
						position = op;
					}
					else
					{
						//abs( e0n.x ) < wallThresh )

						if( !hasPowerGravReverse || ( abs( grindNorm.x ) >= wallThresh || !hasGravReverse ) )
						{
							framesNotGrinding = 0;
							if( reversed )
							{
								velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * -grindSpeed;
							}
							else
							{
								velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
							}
							
							action = JUMP;
							frame = 0;
							ground = NULL;
							movingGround = NULL;
							grindEdge = NULL;
							grindMovingTerrain = NULL;
							reversed = false;
						}
						else
						{
						//	velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
							if( grindNorm.x > 0 )
							{
								offsetX = b.rw;
							}
							else if( grindNorm.x < 0 )
							{
								offsetX = -b.rw;
							}
							else
							{
								offsetX = 0;
							}

							hasAirDash = true;
							hasGravReverse = true;
							hasDoubleJump = true;
							lastWire = 0;


							ground = grindEdge;
							movingGround = grindMovingTerrain;
							groundSpeed = -grindSpeed;
							edgeQuantity = grindQuantity;
							grindEdge = NULL;
							reversed = true;
							hasGravReverse = false;
								
							if( currInput.LRight() )
							{
								if( groundSpeed < 0 )
								{
									//cout << "bleh2" << endl;
									groundSpeed = 0;
								}
								facingRight = true;
							//	groundSpeed = abs( groundSpeed );
							}
							else if( currInput.LLeft() )
							{
								facingRight = false;
								if( groundSpeed > 0 )
								{
									//cout << "bleh1" << endl;
									groundSpeed = 0;
								}
							//	groundSpeed = -abs( groundSpeed );
							}

							action = LAND2;
							frame = 0;
							framesNotGrinding = 0;
						}
					}
				}		
				//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			}
			break;
		}
	case STEEPSLIDE:
		{

			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}

			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				break;
			}

			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}

			if( reversed )
			{
				if( -gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
				}
			}
			else
			{
				if( gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
					//not steep
				/*if( currInput.LLeft() || currInput.RRight() )
				{
					if( currInput.LLeft() && currInput.LDown() && gNorm.x < 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else if( currInput.LLeft() && currInput.LUp() && gNorm.x > 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else if( currInput.LRight() && currInput.LDown() && gNorm.x > 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else if( currInput.LRight() && currInput.LUp() && gNorm.x < 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else
					{
						action = RUN;
						frame = 0;
					}
				}
				else
				{
					if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
				}*/
				}
				else
				{
					//is steep
					if( ( gNorm.x < 0 && groundSpeed > 0 ) || (gNorm.x > 0 && groundSpeed < 0 ) )
					{
						action = STEEPCLIMB;
						frame = 1;
					}
				}
			}
			
			break;
		}
	case AIRDASH:
		{
			if( !currInput.B )//|| ( oldInBubble && !inBubble ) )
			{
				action = JUMP;
				frame = 1;
				
				if( rightWire->state == Wire::PULLING || leftWire->state == Wire::PULLING )
				{
				}
				else
				{
					velocity = V2d( 0, 0 );
				}
			}
			//else if( currInput.A && !prevInput.A && hasDoubleJump )
			else if( currInput.A && !prevInput.A && hasDoubleJump && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( !currInput.LLeft() && !currInput.LRight() )
				{
					if( currInput.LUp() )
					{
						action = UAIR;
						frame = 0;
						break;
					}
					else if( currInput.LDown() )
					{
						action = DAIR;
						frame = 0;
						break;
					}
				}

				action = FAIR;
				frame = 0;
			}
			break;
		}
	case STEEPCLIMB:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				//bounceGrounded = true;
				bounceFlameOn = true;
				runBounceFrame = 0;
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				bounceFlameOn = false;
			}


			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				break;
			}

			if( currInput.A && !prevInput.A && framesSinceClimbBoost > climbBoostLimit )
			{
				//cout << "climb" << endl;
				framesSinceClimbBoost = 0;
				double sp = 28.0;
				double extra = 10.0;
				if( gNorm.x > 0 && currInput.LLeft() )
				{
					groundSpeed = std::min( groundSpeed - extra, -sp );
				}
				else if( gNorm.x < 0 && currInput.LRight() )
				{
					groundSpeed = std::max( groundSpeed + extra, sp );
				}
				else
				{
					action = JUMP;
					frame = 0;
				}
				break;
			}
			else
			{
				//purposely counts outside of time slow so you can get extra boosts in time slow for now
				
			}

			if( reversed )
			{
				if( -gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
				}

				if( gNorm.x > 0 && groundSpeed >= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = true;
				}
				else if( gNorm.x < 0 && groundSpeed <= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = false;	
				}
			}
			else
			{
				if( gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
					//not steep
				}

				if( gNorm.x > 0 && groundSpeed >= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = true;
				}
				else if( gNorm.x < 0 && groundSpeed <= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = false;	
				}
			}

			
			break;
		}
	case AIRHITSTUN:
		{
			if( hitstunFrames == 0 )
			{
				action = JUMP;
				frame = 1;
			}
			break;
		}
	case GROUNDHITSTUN:
		{
			if( hitstunFrames == 0 )
			{
				action = LAND;
				frame = 0;
			}
			break;
		}
	case WIREHOLD:
		break;
	case BOUNCEAIR:
		{
			if( !currInput.X )
			{
				action = JUMP;
				frame = 1;
			}

			if( hasPowerAirDash )
			{
				if( hasAirDash && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					frame = 0;
					break;
				}
			}

			/*if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				bounceFlameOn = true;
				airBounceFrame = 13 * 3;
				break;
			}*/

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				bounceFlameOn = true;
				airBounceFrame = 13 * 3;

				if( currInput.LUp() )
				{
					action = UAIR;
					frame = 0;
				}
				else if( currInput.LDown() )
				{
					action = DAIR;
					frame = 0;
				}
				else
				{
					action = FAIR;
					frame = 0;
				}
			}
			break;
		}
	case BOUNCEGROUND:
		{
			if( !currInput.X )
			{


				action = JUMP;
				frame = 1;
				bounceFlameOn = false;
				bounceEdge = NULL;
				bounceMovingTerrain = NULL;
				break;
			}

			framesSinceBounce = 0;
			V2d bn = bounceNorm;//bounceEdge->Normal();
			if( frame == actionLength[BOUNCEGROUND] - 1 || boostBounce )
			{
				framesInAir = 0;
				action = BOUNCEAIR;
				oldBounceEdge = bounceEdge;
				oldBounceNorm  = bounceNorm;
				frame = 0;
				

				int option = 0; //0 is ground, 1 is wall, 2 is ceiling
			//	V2d bounceNorm;
				if( bn.y < 0 )
				{
					//cout << "prevel: " << velocity.x << ", " << velocity.y << endl;
					if( bn.y > -steepThresh )
					{
						if( bn.x > 0  && storedBounceVel.x < 0 )
						{
							//cout << "A" << endl;
							velocity = V2d( abs(storedBounceVel.x), -abs(storedBounceVel.y) );
						}
						else if( bn.x < 0 && storedBounceVel.x > 0 )
						{
						//	cout << "B" << endl;
							velocity = V2d( -abs(storedBounceVel.x), -abs(storedBounceVel.y) );
						}
						else
						{
							double lenVel = length( storedBounceVel );
							double reflX = cross( normalize( -storedBounceVel ), bn );
							double reflY = dot( normalize( -storedBounceVel ), bn );
							V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
							//velocity = V2d( abs(storedBounceVel.x), -abs(storedBounceVel.y) );
							//cout << "reflx: " << reflX <<", refly: " << reflY << endl;
							velocity = normalize( reflX * edgeDir + reflY * bn ) * lenVel;
							//cout << "set vel: " << velocity.x << ", " << velocity.y << endl;
						}
						//if( 
						//bounceNorm.y = -1;
					}
					else
					{
						//cout << "DD" << endl;
						velocity = V2d( storedBounceVel.x, -abs(storedBounceVel.y) );//length( storedBounceVel ) * bounceEdge->Normal();
					}
				}
				else if( bn.y > 0 )
				{
					if( -bn.y > -steepThresh )
					{
						if( bn.x > 0 && storedBounceVel.x < 0 )
						{
						//	cout << "C" << endl;
							velocity = V2d( abs(storedBounceVel.x), storedBounceVel.y );
						}
						else if( bn.x < 0 && storedBounceVel.x > 0 )
						{
						//	cout << "D" << endl;
							velocity = V2d( -abs(storedBounceVel.x), storedBounceVel.y );
						}
						else
						{
							double lenVel = length( storedBounceVel );
							double reflX = cross( normalize( -storedBounceVel ), bn );
							double reflY = dot( normalize( -storedBounceVel ), bn );
							V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
							velocity = normalize( reflX * edgeDir + reflY * bn ) * lenVel;
						}
						
						//if( 
						//bounceNorm.y = -1;
					}
					else
					{
						velocity = V2d( storedBounceVel.x, abs(storedBounceVel.y) );//length( storedBounceVel ) * bounceEdge->Normal();
					//	cout << "E: " << velocity.x << ", " << velocity.y << endl;
						
					}
				}
				else
				{
				//	cout << "F" << endl;
					velocity = V2d( -storedBounceVel.x, storedBounceVel.y ); 
				}

				//velocity += V2d( 0, -gravity * slowMultiple );


				/*double lenVel = length( storedBounceVel );
				double reflX = cross( normalize( -storedBounceVel ), bn );
				double reflY = dot( normalize( -storedBounceVel ), bn );
				V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
				velocity = normalize( reflX * edgeDir + reflY * bn ) * lenVel;*/

				if( boostBounce )
				{
					velocity += normalize( velocity ) * bounceBoostSpeed / (double)slowMultiple;
					boostBounce = false;
				}


				//velocity = length( storedBounceVel ) * bounceEdge->Normal();
				//ground = NULL;
				bounceEdge = NULL;
				bounceMovingTerrain = NULL;

				//if( ground != NULL )
				//	ground = NULL;
			}

			//V2d( storedBounceVel.x, storedBounceVel.x ) ;
			
			
			/*if( reversed )
			{
				if( -bn.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					/*if( groundSpeed > 0 && bn.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						//climbing
					}
					else
					{
						//falling
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						//climbing
						break;
					}
					else
					{
						//falling
						break;
					}
				}
			}*/


			
			
			break;
		}
	case BOUNCEGROUNDEDWALL:
		{
			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				bounceFlameOn = false;
				runTappingSound.stop();
				break;
			}


			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				runTappingSound.stop();
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				if( currInput.LDown() )
				{
					action = STANDD;
					frame = 0;
				}
				else if( currInput.LUp() )
				{
					action = STANDU;
					frame = 0;
				}
				else
				{
					action = STANDN;
					frame = 0;
				}

				runTappingSound.stop();
				break;
			}
			break;
		}

	case DEATH:
		{

			break;
		}
	}
	
	currHitboxes = NULL;
	//cout << "premidvel: " << velocity.x << ", " << velocity.y << endl;	
//	hurtBody.isCircle = false;
//	hurtBody.rw = 10;
//	hurtBody.rh = normalHeight;

	//react to action
	switch( action )
	{
	case STAND:
		{
		groundSpeed = 0;
		break;
		}
	case RUN:
		{
			//cout << "frame: " << frame << endl;
			RunMovement();
		break;
		}
	case JUMP:
		{
		if( frame == 0 )
		{
			if( ground != NULL ) //this should always be true but we haven't implemented running off an edge yet
			{
				if( reversed )
				{
					if( bounceFlameOn )
						airBounceFrame = 13 * 3;
					//so you dont jump straight up on a nearly vertical edge
					double blah = .3;

					V2d dir( 0, 0 );
					if( (groundSpeed > 0 && gNorm.x > 0) || ( groundSpeed < 0 && gNorm.x < 0 ) )
					{
						dir = V2d( -blah, 0 );
					}

					V2d trueNormal = normalize(dir + normalize(ground->v1 - ground->v0 ));
					velocity = -groundSpeed * trueNormal;
					//velocity = -groundSpeed * normalize(V2d( 0, -1 ) + normalize(ground->v1 - ground->v0 ));
					ground = NULL;
					movingGround = NULL;
					frame = 1; //so it doesnt use the jump frame when just dropping
					reversed = false;
					//facingRight = !facingRight;
					

				}
				else
				{

					if( bounceFlameOn )
						airBounceFrame = 13 * 3;

					double blah = .3;

					V2d dir( 0, 0 );
					if( (groundSpeed > 0 && gNorm.x > 0) || ( groundSpeed < 0 && gNorm.x < 0 ) )
					{
						dir = V2d( blah, 0 );
					}
					
					V2d trueNormal = normalize(dir + normalize(ground->v1 - ground->v0 ));
					velocity = groundSpeed * trueNormal;
					
					if( currInput.B )
					{
						if( currInput.LRight() )
						{
							if( velocity.x < dashSpeed )
								velocity.x = dashSpeed;
						}
						else if( currInput.LLeft() )
						{
							if( velocity.x > -dashSpeed )
								velocity.x = -dashSpeed;
						}
					}
					if( velocity.y > 0 )
					{
						//min jump velocity for jumping off of edges.
						if( abs(velocity.x) < dashSpeed && length( velocity ) >= dashSpeed )
						{
					//		cout << "here: " << velocity.x << endl;
							if( velocity.x > 0 )
							{
								velocity.x = dashSpeed;
							}
							else
							{
								velocity.x = -dashSpeed;
							}
						}

						velocity.y = 0;
					}
					velocity.y -= jumpStrength;
					ground = NULL;
					movingGround = NULL;
					holdJump = true;
				}
				
			}

			
		}
		else
		{


			if( holdJump && velocity.y >= -8 )
				holdJump = false;



			if( holdJump && !currInput.A && framesInAir > 1 )
			{
				if( velocity.y < -8 )
				{
					velocity.y = -8;
				}
			}

			if( framesInAir > 1 || velocity.y < 0 )
				AirMovement();

			//cout << "midvel : " << velocity.x << ", " << velocity.y << endl;	
			/*if( currInput.LLeft() )
			{
				if( velocity.x > dashSpeed )
				{
					velocity.x -= airAccel;
				}
				else if( velocity.x > -maxAirXControl )
				{
					velocity.x = -maxAirXControl;
				}
			}
			else if( currInput.LRight() )
			{
				if( velocity.x < -dashSpeed )
				{
					velocity.x += airAccel;
				}
				else if( velocity.x < maxAirXControl )
				{
					velocity.x = maxAirXControl;
				}
			}
			else if( !currInput.LUp() && !currInput.LDown() )
			{
				if( velocity.x > dashSpeed )
				{
					velocity.x -= airSlow;
					if( velocity.x < dashSpeed ) 
					{
						velocity.x = dashSpeed;
					}
				}
				else if( velocity.x > 0 )
				{
					velocity.x = 0;
				}
				else if( velocity.x < -dashSpeed )
				{
					velocity.x += airSlow;
					if( velocity.x > -dashSpeed ) 
					{
						velocity.x = -dashSpeed;
					}
				}
				else if( velocity.x < 0 )
				{
					velocity.x += airSlow;
					if( velocity.x > 0 ) velocity.x = 0;
					else if( velocity.x >= -dashSpeed )
					{
						velocity.x = 0;
					}
				}
			}*/

			/*if( currInput.LLeft() )
			{
				if( velocity.x > -maxAirXControl )
				{
					velocity.x -= airAccel;
					if( velocity.x < -maxAirXControl )
						velocity.x = -maxAirXControl;
				}
				
			}
			else if( currInput.LRight() )
			{
				if( velocity.x < maxAirXControl )
				{
					velocity.x += airAccel;
					if( velocity.x > maxAirXControl )
						velocity.x = maxAirXControl;
				}
				//cout << "setting velocity.x to : "<< maxAirXControl << endl;
				
			}
			else if( !currInput.LUp() && !currInput.LDown() )
			{
				if( velocity.x > 0 )
				{
					velocity.x -= airSlow;
					if( velocity.x < 0 ) velocity.x = 0;
					else if( velocity.x <= dashSpeed )
					{
						velocity.x = 0;
					}
				}
				else if( velocity.x < 0 )
				{
					velocity.x += airSlow;
					if( velocity.x > 0 ) velocity.x = 0;
					else if( velocity.x >= -dashSpeed )
					{
						velocity.x = 0;
					}
				}
			}*/
			//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;
			
		}
		break;
		}
	case WALLCLING:
		{
			
			if( velocity.y > clingSpeed )
			{
				//cout << "running wallcling" << endl;
				velocity.y = clingSpeed;
			}
			AirMovement();
			
			break;
		}
	case WALLJUMP:
		{
			if( frame == 0 )
			{
				wallJumpFrameCounter = 0;
				double strengthX = wallJumpStrength.x;
				double strengthY = wallJumpStrength.y;

				if( !longWallJump )
				{
					strengthX = strengthX + 4;
				}
				else
				{
					strengthY = strengthY + 3;
				}
				

				if( facingRight )
				{
					velocity.x = strengthX;
				}
				else
				{
					velocity.x = -strengthX;
				}

				velocity.y = -strengthY;
			}
			else if( frame >= wallJumpMovementLimit )
			{
				AirMovement();
			}
			break;
		}
	case FAIR:
		{
			//currHitboxes = fairHitboxes;
			if( fairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = fairHitboxes[frame];
			}

			if( frame == 0 )
			{
				fairSound.play();
			}
			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{
				cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << endl;

				AirMovement();
			}

			break;
		}
	case DAIR:
		{
			if( dairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = dairHitboxes[frame];
			}

			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{		
				AirMovement();
			}
			break;
		}
	case UAIR:
		{
			if( uairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = uairHitboxes[frame];
			}

			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{	
				AirMovement();
			}
			break;
		}
	case DASH:
		{
			b.rh = dashHeight;
			b.offset.y = (normalHeight - dashHeight);
			if( reversed )
				b.offset.y = -b.offset.y;
			if( currInput.LLeft() && facingRight )
			{
				facingRight = false;
				groundSpeed = -dashSpeed;
				frame = 0;
			}
			else if( currInput.LRight() && !facingRight )
			{
				facingRight = true;
				groundSpeed = dashSpeed;
				frame = 0;
			}
			else if( !facingRight )
			{
				if( groundSpeed > -dashSpeed )
					groundSpeed = -dashSpeed;
			}
			else
			{
				if( groundSpeed < dashSpeed )
					groundSpeed = dashSpeed;
			}

			double minFactor = .2;
			double factor = abs( gNorm.x );
			factor = std::max( factor, minFactor );

			if( currInput.LDown() && (( facingRight && gNorm.x > 0 ) || ( !facingRight && gNorm.x < 0 ) ) )
			{
				if( facingRight )
				{
					groundSpeed += sprintAccel * factor / slowMultiple;
				}
				else 
				{
					groundSpeed -= sprintAccel * factor / slowMultiple;
				}
			}
			else if( currInput.LUp() && (( facingRight && gNorm.x > 0 ) || ( !facingRight && gNorm.x < 0 ) ) )
			{
				double upMax = .3;

				factor = std::min( factor, upMax );

				if( facingRight )
				{
					groundSpeed += sprintAccel * factor / slowMultiple; 
				}
				else 
				{	
					groundSpeed -= sprintAccel * factor / slowMultiple; 
				}
			}
			else
			{
				if( !bounceFlameOn )
				{
					if( facingRight )
					{
						groundSpeed += holdDashAccel / slowMultiple;
					}
					else
					{
						groundSpeed -= holdDashAccel / slowMultiple;
					}
				}
			}

			if( bounceFlameOn )
			{
				if( facingRight )
					groundSpeed += bounceFlameAccel / slowMultiple;
				else
					groundSpeed -= bounceFlameAccel / slowMultiple;
			}
			break;
		}
	case DOUBLE:
		{
			b.rh = doubleJumpHeight;
		//	b.offset.y = -5;
			if( frame == 0 )
			{

			
			
				owner->ActivateEffect( ts_fx_double, 
					V2d( position.x, position.y - 60), false, 0, 12, 2, facingRight );
			
				//velocity = groundSpeed * normalize(ground->v1 - ground->v0 );
				if( velocity.y > 0 )
					velocity.y = 0;
				velocity.y = -doubleJumpStrength;
				hasDoubleJump = false;

				if( currInput.LLeft() )
				{
					if( velocity.x > -maxRunInit )
					{
						velocity.x = -maxRunInit;
					}
				}
				else if( currInput.LRight() )
				{
					if( velocity.x < maxRunInit )
					{
						velocity.x = maxRunInit;
					}
				}
				else
				{
					velocity.x = 0;
				}
			}
			else
			{
				
						
				AirMovement();
				//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;
			
			}
			break;
		}
	case SLIDE:
		{
			double fac = gravity * 2.0 / 3;
			if( reversed )
			{
				groundSpeed += dot( V2d( 0, fac), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				groundSpeed += dot( V2d( 0, fac), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			//groundSpeed = 
			break;
		}
	case SPRINT:
		{
			if( b.rh > sprintHeight || canStandUp )
			{
				b.rh = sprintHeight;
				b.offset.y = (normalHeight - sprintHeight);

				if( reversed )
					b.offset.y = -b.offset.y;
			}

			

			if( !facingRight )//currInput.LLeft() )
			{
				if( groundSpeed > 0 )
				{
					groundSpeed = 0;
				}
				else
				{
					if( groundSpeed > -maxRunInit )
					{
						groundSpeed -= runAccelInit * 2 / slowMultiple;
						if( groundSpeed < -maxRunInit )
							groundSpeed = -maxRunInit;
					}
					else
					{
						double minFactor = .2;
						double factor = abs( gNorm.x );
						factor = std::max( factor, minFactor );

						if( gNorm.x > 0 )
						{
							//up a slope
							double upMax = .3;
							
							if( factor > upMax  )
								factor = upMax;

							if( bounceFlameOn )
							{
								groundSpeed -= bounceFlameAccel / slowMultiple;
							}

							groundSpeed -= sprintAccel * factor / slowMultiple; 
						}
						else
						{	
							if( bounceFlameOn )
							{
								groundSpeed -= bounceFlameAccel / slowMultiple;
							}

							groundSpeed -= sprintAccel * factor / slowMultiple;
							//down a slope
						}
					}
				
				}
				facingRight = false;
			}
			//else if( currInput.LRight() )
			else
			{
				if (groundSpeed < 0 )
					groundSpeed = 0;
				else
				{
					V2d gn = ground->Normal();
					if( groundSpeed < maxRunInit )
					{
						groundSpeed += runAccelInit * 2 / slowMultiple;
						if( groundSpeed > maxRunInit )
							groundSpeed = maxRunInit;
					}
					else
					{
						double minFactor = .2;
						double factor = abs( gNorm.x );
						factor = std::max( factor, minFactor );

						if( gNorm.x < 0 )
						{
							//up a slope
							double upMax = .3;
							
							if( factor > upMax  )
								factor = upMax;

							if( bounceFlameOn )
							{
								groundSpeed += bounceFlameAccel / slowMultiple;
							}

							groundSpeed += sprintAccel * factor / slowMultiple; 
						}
						else
						{	

							if( bounceFlameOn )
							{
								groundSpeed += bounceFlameAccel / slowMultiple;
							}

							groundSpeed += sprintAccel * factor / slowMultiple;
							//down a slope
						}
					}
				}
				facingRight = true;
			}

			break;
		}
	case STANDN:
		{
			if( standNHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standNHitboxes[frame];
			}

			AttackMovement();
			break;

		}
	case STANDU:
		{
			if( standUHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standUHitboxes[frame];
			}

			AttackMovement();

			break;
		}
	case STANDD:
		{

			if( standDHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standDHitboxes[frame];
			}

			AttackMovement();
			break;
		}
	case GRINDBALL:
		{
			if( reversed )
			{
				//facingRight = !facingRight;
				//reversed = false;
				
			}
			velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			//cout << "grindspeedin update: " << grindSpeed << endl;
			
			framesGrinding++;
			
			//else
			//grindSpeed =  ;
			break;
		}
	case STEEPSLIDE:
		{
			//if( groundSpeed > 0 )
			double fac = gravity * steepSlideGravFactor;//gravity * 2.0 / 3.0;

			if( currInput.LDown() )
			{
				//cout << "fast slide" << endl;
				fac = gravity * steepSlideFastGravFactor;
			}

			if( reversed )
			{

				groundSpeed += dot( V2d( 0, fac), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				

				groundSpeed += dot( V2d( 0, fac), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			break;
		}
	case AIRDASH:
		{
		//	hurtBody.isCircle = true;
		//	hurtBody.rw = 10;
		//	hurtBody.rh = 10;

			

			/*if( frame == 0 )
			{
				


				hasAirDash = false;
				startAirDashVel = V2d( velocity.x, 0 );//velocity;//
			}
			velocity = V2d( 0, 0 );//startAirDashVel;
			//velocity = V2d( 0, 0 ) velocity.x, -gravity / slowMultiple );

			if( currInput.LUp() )
			{
				if( startAirDashVel.y > 0 )
				{
					startAirDashVel.y = 0;
					velocity.y = 0;
					velocity.y = -airDashSpeed;
				}
				else
				{
					velocity.y = -airDashSpeed;
				}
				
			}
			else if( currInput.LDown() )
			{
				if( startAirDashVel.y < 0 )
				{
					startAirDashVel.y = 0;
					velocity.y = 0;
					velocity.y = airDashSpeed;
				}
				else
				{
					velocity.y = airDashSpeed;
				}
				
			}


			if( currInput.LLeft() )
			{
				if( startAirDashVel.x > 0 )
				{
					startAirDashVel.x = 0;
					velocity.x = 0;
					velocity.x = -airDashSpeed;
				}
				else
				{
					velocity.x = min( startAirDashVel.x, -airDashSpeed );
				}
				facingRight = false;
				//velocity.y -= gravity / slowMultiple;
			}
			else if( currInput.LRight() )
			{
				if( startAirDashVel.x < 0 )
				{
					startAirDashVel.x = 0;
					velocity.x = 0;
					velocity.x = airDashSpeed;
				}
				else
				{
					velocity.x = max( startAirDashVel.x, airDashSpeed );
				}
				facingRight = true;
				//velocity.y -= gravity / slowMultiple;
			}
			
			if( velocity.x == 0 && velocity.y == 0 )
			{
				startAirDashVel = V2d( 0, 0 );
			}

			velocity.y -= gravity / slowMultiple;
			*/
			if( rightWire->state == Wire::PULLING || leftWire->state == Wire::PULLING )
			{
				if( frame == 0 )
				{
					//hasAirDash = false;
					startAirDashVel = velocity;//V2d( velocity.x, 0 );//velocity;//
				}

				V2d wireDir;
				if( rightWire->numPoints == 0 )
				{
					wireDir = normalize( position - rightWire->anchor.pos );
				}
				else
				{
					wireDir = normalize( position - rightWire->points[rightWire->numPoints-1].pos );
				}

				V2d wn( wireDir.y, -wireDir.x );

				bool forwardWire = false;
				bool backWardsWire = false;

				double airDashFactor = .2;//airDashSpeed
				V2d ad;

				double r = dot( velocity, wn );
				if( abs(r) > 15 )
				{
					if( dot( velocity, wn ) > 0 )
					{
				//		velocity += wn * airDashFactor;
						//cout << "ccw" << endl;
					}
					else
					{
				//		velocity -= wn * airDashFactor;
						//cout << "cw" << endl;
					}

				}
				else
				{
					if( dot( velocity, wn ) > 0 )
					{
				//		velocity += wn * 10.0;
						//cout << "ccw" << endl;
					}
					else
					{
				//		velocity -= wn * 10.0;
						//cout << "cw" << endl;
					}
					
				}
				//if( velocity.y < 0 )
				//	velocity.y -= gravity / slowMultiple;
				//velocity = V2d( 0, 0 );
			}
			else
			{
				if( frame == 0 )
				{
					hasAirDash = false;
					startAirDashVel = V2d( velocity.x, 0 );//velocity;//
				}
				velocity = V2d( 0, 0 );//startAirDashVel;
			
				double keepHorizontalLimit = 30;
				double removeSpeedFactor = .5;

				if( currInput.LUp() )
				{
					if( !(currInput.LLeft() || currInput.LRight() ) && abs(startAirDashVel.x) >= keepHorizontalLimit )
					{
						velocity.x = startAirDashVel.x * removeSpeedFactor;
						//cout << "velocity.x: " << velocity.x << endl;
					}
					
					velocity.y = -airDashSpeed;
				}
				else if( currInput.LDown() )
				{
					if( !(currInput.LLeft() || currInput.LRight() ) && abs(startAirDashVel.x) >= keepHorizontalLimit )
					{	
						velocity.x = startAirDashVel.x * removeSpeedFactor;
						//cout << "velocity.x: " << velocity.x << endl;
					}

					velocity.y = airDashSpeed;
				}


				if( currInput.LLeft() )
				{
					if( startAirDashVel.x > 0 )
					{
						startAirDashVel.x = 0;
						velocity.x = -airDashSpeed;
					}
					else
					{
						velocity.x = min( startAirDashVel.x, -airDashSpeed );
					}
					facingRight = false;
				
				}
				else if( currInput.LRight() )
				{
					if( startAirDashVel.x < 0 )
					{
						startAirDashVel.x = 0;
						velocity.x = airDashSpeed;
					}
					else
					{
						velocity.x = max( startAirDashVel.x, airDashSpeed );
					}
					facingRight = true;
				}
			
				if( velocity.x == 0 && velocity.y == 0 )
				{
					startAirDashVel = V2d( 0, 0 );
				}

				velocity.y -= gravity / slowMultiple;
			}
			

			break;
		}
	case STEEPCLIMB:
		{
			//if( groundSpeed > 0 )

			//the factor is just to make you climb a little farther
			float factor = steepClimbGravFactor;//.7 ;
			if( currInput.LUp() )
			{
				//cout << "speeding up climb!" << endl;
				factor = steepClimbFastFactor;//.5;
			}

			if( reversed )
			{
				groundSpeed += dot( V2d( 0, gravity * factor), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				groundSpeed += dot( V2d( 0, gravity * factor), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			
			break;
		}
	case AIRHITSTUN:
		{
			hitstunFrames--;
			break;
		}
	case GROUNDHITSTUN:
		{
			hitstunFrames--;
			break;
		}
	case WIREHOLD:
		{
			break;
		}
	case BOUNCEAIR:
		{
			if( framesInAir > 10 ) //to prevent you from clinging to walls awkwardly
			{
			//	cout << "movement" << endl;
				AirMovement();
			}
			else
			{
			//	cout << "not movement" << endl;
			}
			break;
		}
	case BOUNCEGROUND:
		{
			

			if( !boostBounce && currInput.A && !prevInput.A )
			{
				
				boostBounce = true;

				
				V2d bouncePos = bounceEdge->GetPoint( bounceQuant );
				V2d bn = bounceEdge->Normal();
				double angle = atan2( bn.x, -bn.y );
				bouncePos += bn * 80.0;
				owner->ActivateEffect( ts_bounceBoost, bouncePos, false, angle, 30, 1, facingRight );
			}

			velocity.x = 0;
			velocity.y = 0;
			groundSpeed = 0;
			break;
		}
	case BOUNCEGROUNDEDWALL:
		{
			//cout << "isfacing right: " << facingRight << endl;
			if( frame == 0 )
			{
				//cout << "storedBounceGround: " << groundSpeed << endl;
				//storedBounceGroundSpeed = groundSpeed;
				groundSpeed = 0;
			}
			else if( frame == 6 )
			{
				
				groundSpeed = -storedBounceGroundSpeed;
				cout << "set ground speed to: " << groundSpeed << endl;
			}
			break;
		}
	case DEATH:
		{
			velocity.x = 0;
			velocity.y = 0;
			groundSpeed = 0;
			break;
		}
	}

	if( action != GRINDBALL )
	{
		//for camera smoothing
		framesNotGrinding++;
	}
	
	if( blah || record > 1 )
	{
		int playback = recordedGhosts;
		if( record > 1 )
			playback--;

		for( int i = 0; i < playback; ++i )
		{
			if( ghostFrame < ghosts[i]->totalRecorded )
				ghosts[i]->UpdatePrePhysics( ghostFrame );
		}
		//testGhost->UpdatePrePhysics( ghostFrame );
	}

	Wire::WireState oldLeftWireState = leftWire->state;
	Wire::WireState oldRightWireState = rightWire->state;


	if( hasPowerLeftWire && action != GRINDBALL )
	{
		leftWire->ClearDebug();
		leftWire->UpdateAnchors( V2d( 0, 0 ) );
		leftWire->UpdateState( touchEdgeWithLeftWire );
	}

	if( hasPowerRightWire && action != GRINDBALL )
	{
		rightWire->ClearDebug();
		rightWire->UpdateAnchors( V2d( 0, 0 ) );
		rightWire->UpdateState( touchEdgeWithRightWire );
	}
	

	if( ground == NULL && bounceEdge == NULL && action != DEATH )
	{
		if( velocity.x > maxAirXSpeed )
			velocity.x = maxAirXSpeed;
		else if( velocity.x < -maxAirXSpeed )
			velocity.x = -maxAirXSpeed;

		if( velocity.y > 0 && velocity.y < 10 )
		{
			velocity += V2d( 0, gravity / slowMultiple * .6 );
		}
		else
		{
			velocity += V2d( 0, gravity / slowMultiple );
		}

		if( velocity.y > maxFallSpeed )
			velocity.y = maxFallSpeed;
	}
	else
	{
		if( groundSpeed > maxGroundSpeed )
			groundSpeed = maxGroundSpeed;
		else if( groundSpeed < -maxGroundSpeed )
		{
			groundSpeed = -maxGroundSpeed;
		}
	}

//	wire->UpdateAnchors();
	
	//if( false )
	Wire *wire = rightWire;
	//while( wire != leftWire )
	// = position;
	//wPos += V2d( rightWire->offset.x, rightWire->offset.y );
	/*if( player->facingRight )
	{
		offset.x = -abs( offset.x );
	}
	else
	{
		offset.x = abs( offset.x );
	}*/
	//V2d playerPos;
	/*double angle = GroundedAngle();
	double x = sin( angle );
	double y = -cos( angle );
	V2d gNormal( x, y ); 
	V2d other( -gNormal.y, gNormal.x );

	if( ground != NULL )
	{
		V2d pp = ground->GetPoint( edgeQuantity );
		wPos = pp + gNormal * normalHeight;
	}
	else
	{
		wPos = position;
	}
	wPos += gNormal * (double)rightWire->offset.y + other * (double)rightWire->offset.x;*/
	
	double accel = .15;
	double triggerSpeed = 17;
	double doubleWirePull = 2.0;
	if( framesInAir > 1 )
	if( rightWire->state == Wire::PULLING && leftWire->state == Wire::PULLING )
	{	
		lastWire = 0;
		V2d rwPos = rightWire->storedPlayerPos;
		V2d lwPos = rightWire->storedPlayerPos;
		V2d newVel1, newVel2;
		V2d wirePoint = wire->anchor.pos;
		if( wire->numPoints > 0 )
			wirePoint = wire->points[wire->numPoints-1].pos;

		V2d wireDir1 = normalize( wirePoint - rwPos );
		V2d tes =  normalize( rwPos - wirePoint );
		double temp = tes.x;
		tes.x = tes.y;
		tes.y = -temp;

		V2d old = velocity;
		//velocity = dot( velocity, tes ) * tes;


		V2d future = rwPos + velocity;
		
		V2d seg = wirePoint - rwPos;
		double segLength = length( seg );
		V2d diff = wirePoint - future;
		
		//wire->segmentLength -= 10;
		if( length( diff ) > wire->segmentLength )
		{
			future += normalize(diff) * ( length( diff ) - ( wire->segmentLength) );
			newVel1 = future - rwPos;
		}

		
		wire = leftWire;

		wirePoint = wire->anchor.pos;
		if( wire->numPoints > 0 )
			wirePoint = wire->points[wire->numPoints-1].pos;

		V2d wireDir2 = normalize( wirePoint - lwPos );
		tes =  normalize( lwPos - wirePoint );
		temp = tes.x;
		tes.x = tes.y;
		tes.y = -temp;

		old = velocity;
		//velocity = dot( velocity, tes ) * tes;


		future = lwPos + velocity;
		
		seg = wirePoint - lwPos;
		segLength = length( seg );
		diff = wirePoint - future;
		
		//wire->segmentLength -= 10;
		if( length( diff ) > wire->segmentLength )
		{
			future += normalize(diff) * ( length( diff ) - ( wire->segmentLength) );
			newVel2 = future - lwPos;
		}

		V2d totalVelDir =  normalize( (newVel1 + newVel2 ) );//normalize( wireDir1 + wireDir2 );
		//velocity = dot( (newVel1 + newVel2)/ 2.0, totalVelDir ) * normalize( totalVelDir );

		totalVelDir = normalize( wireDir1 + wireDir2 );

		V2d otherDir( totalVelDir.y, -totalVelDir.x );
		double dotvel =dot( velocity, otherDir );
		if( dotvel > 0 )
		{
			velocity += -otherDir * 1.0 / (double)slowMultiple;
		}
		else if( dotvel < 0 )
		{
			velocity += otherDir * 1.0 / (double)slowMultiple;
		}
		else
		{
		}

		totalVelDir.x *= doubleWirePull / (double)slowMultiple;
		if( totalVelDir.y < 0 )
			totalVelDir.y *= ( doubleWirePull + 1 )/ (double)slowMultiple;
		else
			totalVelDir.y *= ( doubleWirePull )/ (double)slowMultiple;
		velocity += totalVelDir;
		//velocity = ( dot( velocity, totalVelDir ) + 4.0 ) * totalVelDir; //+ V2d( 0, gravity / slowMultiple ) ;
		///velocity += totalVelDir * doubleWirePull / (double)slowMultiple;
	}
	else if( rightWire->state == Wire::PULLING )
	{
		//lastWire = 1;
		V2d wPos = rightWire->storedPlayerPos;
		V2d wirePoint = wire->anchor.pos;
		if( wire->numPoints > 0 )
			wirePoint = wire->points[wire->numPoints-1].pos;

		V2d tes =  normalize( wPos - wirePoint );
		double temp = tes.x;
		tes.x = tes.y;
		tes.y = -temp;

		double val = dot( velocity, normalize( wirePoint - wPos ) );
		V2d otherTes;
		//if( val > 0 )
		{
			otherTes = val * normalize( wirePoint - wPos );
		}
		
		 

		V2d old = velocity;
		
		if( normalize( wirePoint - wPos ).y > 0 )
		{
		//	velocity -= V2d( 0, gravity );
		}
		
		
		double speed = dot( velocity, tes ); 
		

		if( speed > triggerSpeed )
		{
			speed += accel;
		}
		else if( speed < -triggerSpeed )
		{
			speed -= accel;
		}
		else
		{
			
		
		}

		V2d wireDir = normalize( wirePoint - wPos );
		double otherAccel = .5;
		if( abs( wireDir.x ) < .7 )
			{
				if( wireDir.y < 0 )
				{
					if( currInput.LLeft() )
					{
						speed -= otherAccel;
					}
					else if( currInput.LRight() )
					{
						speed += otherAccel;
					}
				}
				else if( wireDir.y > 0 )
				{
					if( currInput.LLeft() )
					{
						speed += otherAccel;
					}
					else if( currInput.LRight() )
					{
						speed -= otherAccel;
					}
				}
			}
		else
		{
			if( wireDir.x > 0 )
			{
				if( currInput.LUp() )
				{
					speed -= otherAccel;
				}
				else if( currInput.LDown() )
				{
					speed += otherAccel;
				}
			}
			else if( wireDir.x < 0 )
			{
				if( currInput.LUp() )
				{
					speed += otherAccel;
				}
				else if( currInput.LDown() )
				{
					speed -= otherAccel;
				}
			}

			
		}

		velocity = speed * tes;
		velocity += otherTes;
		//velocity += otherTes;


		V2d future = wPos + velocity;
		
		V2d seg = wirePoint - wPos;
		double segLength = length( seg );
		V2d diff = wirePoint - future;
		
		//wire->segmentLength -= 10;
		if( length( diff ) > wire->segmentLength )
		{
			future += normalize(diff) * ( length( diff ) - ( wire->segmentLength) );
			velocity = future - wPos;
		}
	}
	else if( leftWire->state == Wire::PULLING  )
	{
		//lastWire = 2;
		wire = leftWire;
		V2d wPos = leftWire->storedPlayerPos;
		V2d wirePoint = wire->anchor.pos;
		if( wire->numPoints > 0 )
			wirePoint = wire->points[wire->numPoints-1].pos;

		V2d tes =  normalize( wPos - wirePoint );
		double temp = tes.x;
		tes.x = tes.y;
		tes.y = -temp;

		double val = dot( velocity, normalize( wirePoint - wPos ) );
		V2d otherTes;
		//if( val > 0 )
		{
			otherTes = val * normalize( wirePoint - wPos );
		}
		
		 

		V2d old = velocity;
		
		if( normalize( wirePoint - wPos ).y > 0 )
		{
		//	velocity -= V2d( 0, gravity );
		}
		
		//double accel = .3;
		double speed = dot( velocity, tes ); 

		if( speed > triggerSpeed )
		{
			speed += accel;
		}
		else if( speed < -triggerSpeed )
		{
			speed -= accel;
		}
		else
		{
			
		
		}

		V2d wireDir = normalize( wirePoint - wPos );
		double otherAccel = .5;
		if( abs( wireDir.x ) < .7 )
			{
				if( wireDir.y < 0 )
				{
					if( currInput.LLeft() )
					{
						speed -= otherAccel;
					}
					else if( currInput.LRight() )
					{
						speed += otherAccel;
					}
				}
				else if( wireDir.y > 0 )
				{
					if( currInput.LLeft() )
					{
						speed += otherAccel;
					}
					else if( currInput.LRight() )
					{
						speed -= otherAccel;
					}
				}
			}
		else
		{
			if( wireDir.x > 0 )
			{
				if( currInput.LUp() )
				{
					speed -= otherAccel;
				}
				else if( currInput.LDown() )
				{
					speed += otherAccel;
				}
			}
			else if( wireDir.x < 0 )
			{
				if( currInput.LUp() )
				{
					speed += otherAccel;
				}
				else if( currInput.LDown() )
				{
					speed -= otherAccel;
				}
			}

			
		}

		velocity = speed * tes;
		velocity += otherTes;
		//velocity += otherTes;


		V2d future = wPos + velocity;
		
		V2d seg = wirePoint - wPos;
		double segLength = length( seg );
		V2d diff = wirePoint - future;
		
		//wire->segmentLength -= 10;
		if( length( diff ) > wire->segmentLength )
		{
			future += normalize(diff) * ( length( diff ) - ( wire->segmentLength) );
			velocity = future - wPos;
		}
	}

	if( ground != NULL )
	{
		lastWire = 0;
	}

	
	

	for( int i = 0; i < maxBubbles; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			bubbleFramesToLive[i]--;
		}
	}


	bool cloneBubbleCreated = false;
	V2d cloneBubbleCreatedPos;



	if( blah || record > 1 )
	{
		int playback = recordedGhosts;
		if( record > 1 )
			playback--;

		for( int i = 0; i < playback; ++i )
		{
			if( ghostFrame < ghosts[i]->totalRecorded )
			{
				if( ghosts[i]->states[ghostFrame].createBubble )
				{
					cloneBubbleCreated = true;
					cloneBubbleCreatedPos = ghosts[i]->states[ghostFrame].position;
					cout << "creating bubble: " << ghostFrame << endl;
					break;
				}
			}
		}
		
	}

	bool bubbleCreated = false;
	oldInBubble = inBubble;
	inBubble = false;

	if( hasPowerTimeSlow )
	{
		//calculate this all the time so I can give myself infinite airdash
		for( int i = 0; i < maxBubbles; ++i )
		{
			if( bubbleFramesToLive[i] > 0 )
			{
				//if( IsQuadTouchingCircle( hurtB
				if( length( position - bubblePos[i] ) < bubbleRadius )
				{
					inBubble = true;
					break;
				}
			}
		}
	}

	if( !inBubble && action == AIRDASH && airDashStall )
	{
		action = JUMP;
		frame = 1;
	}

	if( hasPowerTimeSlow && currInput.leftShoulder|| cloneBubbleCreated )
	{
		
		

		

		if( (!prevInput.leftShoulder  && !inBubble) || cloneBubbleCreated )
		{
			if( bubbleFramesToLive[currBubble] == 0 )
			{
				inBubble = true;
				bubbleFramesToLive[currBubble] = bubbleLifeSpan;

				if( !cloneBubbleCreated )
				{
					bubblePos[currBubble] = position;
				}
				else
				{
					bubblePos[currBubble] = cloneBubbleCreatedPos;
				}
				

				++currBubble;
				if( currBubble == maxBubbles )
				{
					currBubble = 0;
				}

				bubbleCreated = true;
			}			
		}

		if( inBubble )
		{
			if( slowMultiple == 1 )
			{
				slowCounter = 1;
				slowMultiple = timeSlowStrength;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		}
	}
	else
	{
		slowCounter = 1;
		slowMultiple = 1;
	}

	if( record > 0 )
	{
		PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
		p.createBubble = bubbleCreated;
		/*if( p.createBubble )
		{
			cout << "recording clone bubble: " << ghosts[record-1]->currFrame << endl;
		}*/
	}

	
	//cout << "position: " << position.x << ", " << position.y << endl;
//	cout << "velocity: " << velocity.x << ", " << velocity.y << endl;m
	
	
	oldVelocity.x = velocity.x;
	oldVelocity.y = velocity.y;

	//cout << "pre vel: " << velocity.x << ", " << velocity.y << endl;

	//if( ground != NULL )
	//	cout << "groundspeed: " << groundSpeed << endl;
	


	

	touchEdgeWithLeftWire = false;
	touchEdgeWithRightWire = false;
	oldAction = action;
	collision = false;
	groundedWallBounce = false;
	//if( ground == NULL )
	//cout << "final vel: " << velocity.x << ", " << velocity.y << endl;
	
}

bool Actor::CheckWall( bool right )
{
	double wThresh = 5;
	V2d vel;
	if( right )
	{
		vel.x = wThresh;
	}
	else
	{
		vel.x = -wThresh;
	}
	V2d newPos = (position) + vel;
	Contact test;
	//test.collisionPriority = 10000;
	//test.edge = NULL;


	minContact.collisionPriority = 10000;
	minContact.edge = NULL;
	minContact.resolution = V2d( 0, 0 );
	minContact.movingPlat = NULL;
	col = false;
	queryMode = "checkwall";
	tempVel = vel;

	//sf::Rect<double> r( 
	Rect<double> r( position.x + tempVel.x + b.offset.x - b.rw, position.y + tempVel.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );


	owner->terrainTree->Query( this, r );
	
	queryMode = "moving_checkwall";
	for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
	{
		currMovingTerrain = (*it);
		(*it)->Query( this, r );
	}
	


	if( !col )
	{
		return false;
	}

	bool wally = false;
	if( minContact.edge != NULL )
	{
		V2d oldv0 = minContact.edge->v0;
		V2d oldv1 = minContact.edge->v1;

		if( minContact.movingPlat != NULL )
		{
			cout << "moving plat blah" << endl;
			minContact.edge->v0 += minContact.movingPlat->position;
			minContact.edge->v1 += minContact.movingPlat->position;
		}

		double quant = minContact.edge->GetQuantity( test.position );

		if( minContact.movingPlat!= NULL )
		{
			minContact.edge->v0 = oldv0;
			minContact.edge->v1 = oldv1;
		}

		
		bool zero = false;
		bool one = false;
		if( quant <= 0 )
		{
			zero = true;
			quant = 0;
		}
		else if( quant >= length( minContact.edge->v1 - minContact.edge->v0 ) )
		{
			one = true;
			quant = length( minContact.edge->v1 - minContact.edge->v0 );
		}

		//if( !zero && !one )
		//	return false;

		//cout << "zero: " << zero << ", one: " << one << endl;
		//cout << "haha: "  << quant << ", " << length( test.edge->v1 - test.edge->v0 ) << endl;
		Edge *e = minContact.edge;
		V2d en = e->Normal();
		Edge *e0 = e->edge0;
		Edge *e1 = e->edge1;

	//	cout << "here: " << test.position.x << ", " << test.position.y << " .. " << e->v0.x << ", " << e->v0.y << endl;	

		/*CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( e->GetPoint( quant ).x, e->GetPoint( quant ).y );

		owner->window->draw( cs );*/


		if( approxEquals(en.x,1) || approxEquals(en.x,-1) )
		{
			//wallNormal = minContact.edge->Normal();
			return true;
		}

		
		if( en.y > 0 && abs( en.x ) > .8 )
		{
			//cout << "here" << endl;
			return true;
		}

		if( (zero && en.x < 0 && en.y < 0 ) )
		{
			//cout << "?>>>>>" << endl;
			V2d te = e0->v0 - e0->v1;
			if( te.x > 0 )
			{
				return true;
			}
		}
		
		if( (one && en.x < 0 && en.y > 0 ) )
		{
			//cout << "%%%%%" << endl;
			V2d te = e1->v1 - e1->v0;
			if( te.x > 0 )
			{
				return true;
			}
		}

		if( (one && en.x < 0 && en.y < 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x < 0 )
			{
				return true;
			}
		}
		
		if( (zero && en.x > 0 && en.y < 0 ) )
		{
			V2d te = e0->v0 - e0->v1;
			if( te.x > 0 )
			{	
				return true;
			}
		}
	
		if( ( one && en.x > 0 && en.y < 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x < 0 )
			{
				return true;
			}
		}
		if( (zero && en.x > 0 && en.y > 0 ) )
		{
			V2d te = e0->v0 - e0->v1;
			if( te.x < 0 )
			{
				return true;
			}
		}
		

		{
		//	cout << en.x << ", " << en.y << endl;
		//	cout << "misery" << endl;
		}
	}
	return false;

}

bool Actor::CheckStandUp()
{
	if( b.rh > normalHeight )
	{
		cout << "WEIRD" << endl;
		return false;
	//	return true;
	}
	else
	{
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - normalHeight, 2 * b.rw, 2 * normalHeight );
	//	Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight * 2, 2 * b.rw, 2 * normalHeight * 2 );

	//	Rect<double> r( position.x + offsetX + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);

		//hope this doesnt make weird issues sometimes ;_;
		double ex = .001;
		Rect<double> r;
		if( reversed )
		{
			r = Rect<double>( position.x + b.offset.x - b.rw, position.y - ex/*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight + 2 * ex);
		}
		else
		{
			r = Rect<double>( position.x + b.offset.x - b.rw, position.y - ex /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight + 2* ex);
		}
		sf::RectangleShape rs;
		rs.setSize( Vector2f(r.width, r.height ));
		rs.setFillColor( Color::Yellow );
		rs.setPosition( r.left, r.top );
		/*r.left -= 1;
		r.width += 1;
		r.top -= 1;
		r.height += 1;*/
		
//		owner->preScreenTex->draw( rs );

		//owner->window->draw( rs );

		queryMode = "check";
		checkValid = true;
	//	Query( this, owner->testTree, r );
		owner->terrainTree->Query( this, r );

		for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
		{
			(*it)->Query( this, r );
		}
		//cout << "col number: " << possibleEdgeCount << endl;
		possibleEdgeCount = 0;
		return checkValid;
	}
	
}

bool Actor::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;
	Rect<double> oldR( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	position += vel;
	
	Rect<double> newR( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	minContact.collisionPriority = 1000000;
	
	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	

	double ex = 1;
	Rect<double> r( minLeft - ex, minTop - ex, (maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2 );

	col = false;

	tempVel = vel;
	minContact.edge = NULL;


	//cout << "---STARTING QUERY--- vel: " << vel.x << ", " << vel.y << endl;
	queryMode = "resolve";

	Edge *oldGround = ground;
	double oldGs = groundSpeed;
	
//	Query( this, owner->testTree, r );

	//cout << "Start resolve" << endl;
	owner->terrainTree->Query( this, r );



	queryMode = "moving_resolve";
	for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
	{
		currMovingTerrain = (*it);
		(*it)->Query( this, r );
	}

	if( col )
	{
		if( col )
		{
			//cout << "norm: " << minContact.normal.x << ", " << minContact.normal.y << endl;
		}
		if( minContact.normal.x == 0 && minContact.normal.y == 0 )
		{
			
			minContact.normal = minContact.edge->Normal();
			//cout << "setting the normal to: " << minContact.normal.x << ", " <<minContact.normal.y << endl;
		}
		else
		{
			if( oldGround != NULL )
			{
				if( oldGs > 0 && minContact.edge == oldGround->edge0 

					|| ( oldGs > 0 && minContact.edge == oldGround->edge1 ) 
					|| minContact.edge == oldGround )
				{
					//col = false;
					//cout << "setting false" << endl;
				}
			}
			
		}

		

		if( false )//if( col )//if( false )////if( col )//
		{
			cout << "pos: " << minContact.position.x << ", " << minContact.position.y << endl;
			cout << "performing: " << endl 
				<< "normal: " << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y
				<< " res: " << minContact.resolution.x << ", " << minContact.resolution.y 
				<< " realNormal: " << minContact.normal.x << ", " << minContact.normal.y
				<< "vel: " << tempVel.x << ", " << tempVel.y << endl;
		}
	}

	queryMode = "grass";
	testGrassCount = 0;
	owner->grassTree->Query( this, r );

	if( testGrassCount > 0 )
	{
		action = DEATH;
		rightWire->Reset();
		leftWire->Reset();
		slowCounter = 1;
		frame = 0;
		owner->deathWipe = true;

		owner->powerBar.Damage( 1000000 );
	}

	queryMode = "gate";
	owner->testGateCount = 0;
	owner->gateTree->Query( this, r );

	if( owner->testGateCount > 0 )
	{
		action = DEATH;
		rightWire->Reset();
		leftWire->Reset();
		slowCounter = 1;
		frame = 0;
		owner->deathWipe = true;

		owner->powerBar.Damage( 1000000 );
	}

	//need to fix the quad tree but this works!
	//cout << "test grass count: " << testGrassCount << endl;
	//if( minContact.edge != NULL )
	//	cout << "blah: " <<  minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << endl;

	return col;
}

V2d Actor::UpdateReversePhysics()
{
	V2d leftGroundExtra( 0, 0 );
	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );
	wallNormal.x = 0;
	wallNormal.y = 0;
	if( grindEdge != NULL )
	{
		movement = grindSpeed / (double)slowMultiple / NUM_STEPS;
	}
	else if( ground != NULL )
	{
		movement = groundSpeed / (double)slowMultiple / NUM_STEPS;
	}
	else
	{
		movementVec = velocity / (double)slowMultiple / NUM_STEPS;
	}

	movement = -movement;

		
	while( (ground != NULL && !approxEquals( movement, 0 )) || ( ground == NULL && length( movementVec ) > 0 ) )
	{
		if( ground != NULL )
		{
			double steal = 0;
			if( movement > 0 )
			{
				if( movement > maxMovement )
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else 
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}


			double extra = 0;
			bool leaveGround = false;
			double q = edgeQuantity;

			V2d gNormal = ground->Normal();
			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			gNormal = -gNormal;
			e0n = -e0n;
			e1n = -e1n;
			offsetX = -offsetX;

			double m = movement;
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			

			bool transferLeft =  q == 0 && movement < 0 //&& (groundSpeed < -steepClimbSpeedThresh || e0n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
				|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ));
			bool transferRight = q == groundLength && movement > 0 //&& (groundSpeed > steepClimbSpeedThresh || e1n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
			
			if( transferLeft )
			{
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				V2d nextNorm = e0n;
				double yDist = abs( gNormal.x ) * -groundSpeed;
				if( nextNorm.y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && /*!currInput.LLeft() &&*/ gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x <= 0 ) )
				{
					//cout << "e0n: " << e0n.x << ", " << e0n.y << endl;
					if( e0n.x > 0 && e0n.y > -steepThresh )
					{
						//cout << "c" << endl;
						if( groundSpeed <= steepClimbSpeedThresh )
						{
							offsetX = -offsetX;
							groundSpeed = 0;
							break;
						}
						else
						{
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
					else if( gNormal.x > 0 && gNormal.y > -steepThresh )
					{
						cout << "A" << endl;
						reversed = false;
						velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;

						movementVec.y += .01;
						if( movementVec.x <= .1 )
						{
							movementVec.x = .1;
						}

						if( movingGround != NULL )
					{
						movementVec += currMovingTerrain->vel / (double)slowMultiple;
						cout << "6 movementvec is now: " << movementVec.x << ", " << movementVec.y <<
							", because of: " << currMovingTerrain->vel.x << ", " << currMovingTerrain->vel.y << endl;
					}
						
						//leftGroundExtra.y = .01;
						//leftGroundExtra.x = .01;


						leftGround = true;
						action = JUMP;
						frame = 1;
						rightWire->UpdateAnchors( V2d( 0, 0 ) );
						leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;
						movingGround = NULL;

						leftGroundExtra = movementVec;
						return leftGroundExtra;
					}
					else
					{
						if( e0n.y > -steepThresh )
						{
							if( e0n.x < 0 )
							{
								if( gNormal.x >= -slopeTooSteepLaunchLimitX )
								{
									cout << "A2" << endl;
									reversed = false;
									velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;

									movementVec.y += .01;
									if( movementVec.x <= .1 )
									{
										movementVec.x = .1;
									}	

									if( movingGround != NULL )
									{
										movementVec += currMovingTerrain->vel / (double)slowMultiple / NUM_STEPS;
										cout << "7 movementvec is now: " << movementVec.x << ", " << movementVec.y <<
											", because of: " << currMovingTerrain->vel.x << ", " << currMovingTerrain->vel.y << endl;
									}

									leftGroundExtra.y = .01;
									leftGroundExtra.x = .01;

									leftGround = true;
									action = JUMP;
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									movingGround = NULL;

									return leftGroundExtra;
								}
								else
								{
									facingRight = true;
									action = STEEPSLIDE;
									frame = 0;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = next;
									q = length( ground->v1 - ground->v0 );	
								}
							}
							else if( e0n.x > 0 )
							{
								facingRight = false;
								action = STEEPCLIMB;
								frame = 0;
								rightWire->UpdateAnchors( V2d( 0, 0 ) );
								leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
							else
							{
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
						}
						else
						{
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
				}
				else
				{
					//cout  <<  "reverse left" << endl;
					//cout << "d" << endl;
					reversed = false;
					velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;

					movementVec.y += .01;
					if( movementVec.x <= .1 )
					{
						movementVec.x = .1;
					}

					
					if( movingGround != NULL )
					{
						movementVec += currMovingTerrain->vel / (double)slowMultiple;
						cout << "2 movementvec is now: " << movementVec.x << ", " << movementVec.y <<
							", because of: " << currMovingTerrain->vel.x << ", " << currMovingTerrain->vel.y << endl;
					}
					

					//cout << "vel: " << velocity.x << ", " << velocity.y << endl;
					//cout << "move: " << movementVec.x << ", " << movementVec.y << endl;

					leftGround = true;
					action = JUMP;
					frame = 1;
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					ground = NULL;
					movingGround = NULL;

					//leftGroundExtra.y = .01;
					//leftGroundExtra.x = .1;
					leftGroundExtra = movementVec;
					return leftGroundExtra;

					//break;
				}
			}
			else if( transferRight )
			{
				Edge *next = ground->edge1;
				V2d nextNorm = e1n;
				double yDist = abs( gNormal.x ) * -groundSpeed;
				if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && /*!currInput.LRight() && */gNormal.x < 0 && yDist > slopeLaunchMinSpeed && nextNorm.x >= 0 ) )
				{
					//cout << "e1n: " << e1n.x << ", " << e1n.y << endl;
					if( e1n.x < 0 && e1n.y > -steepThresh )
					{
					//	cout << "a" << endl;
						if( groundSpeed >= -steepClimbSpeedThresh )
						{
							groundSpeed = 0;
							offsetX = -offsetX;
							break;
						}
						else
						{
							ground = next;
							q = 0;
						}
					}
					else if( gNormal.x < 0 && gNormal.y > -steepThresh )
					{
						reversed = false;
						//cout << "b" << endl;
						velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;

						movementVec.y += .01;
						if( movementVec.x >= -.1 )
						{
							movementVec.x = -.1;
						}

						if( movingGround != NULL )
					{
						movementVec += currMovingTerrain->vel / (double)slowMultiple;
						cout << "3 movementvec is now: " << movementVec.x << ", " << movementVec.y <<
							", because of: " << currMovingTerrain->vel.x << ", " << currMovingTerrain->vel.y << endl;
					}

						leftGround = true;
						action = JUMP;
						frame = 1;
						rightWire->UpdateAnchors( V2d( 0, 0 ) );
						leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;
						movingGround = NULL;
						
						leftGroundExtra = movementVec;
						//leftGroundExtra.y = .01;
						//leftGroundExtra.x = -.01;
						return leftGroundExtra;

					}
					else
					{
						if( e1n.y > -steepThresh )
						{
							if( e1n.x > 0 )
							{
								if( gNormal.x <= slopeTooSteepLaunchLimitX )
								{
									//cout << "B2, extra: " << extra << endl;
									reversed = false;
									velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;

									movementVec.y += .01;
									if( movementVec.x >= -.1 )
									{
										movementVec.x = -.1;
									}

									if( movingGround != NULL )
					{
						movementVec += currMovingTerrain->vel / (double)slowMultiple;
						cout << "4 movementvec is now: " << movementVec.x << ", " << movementVec.y <<
							", because of: " << currMovingTerrain->vel.x << ", " << currMovingTerrain->vel.y << endl;
					}

									leftGround = true;
									action = JUMP;
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									movingGround = NULL;

									leftGroundExtra = movementVec;
									//leftGroundExtra.y = .01;
									//leftGroundExtra.x = -.01;
									return leftGroundExtra;

								}
								else
								{
									facingRight = false;
									action = STEEPSLIDE;
									frame = 0;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = next;
									q = 0;
								}
							}
							else if( e1n.x < 0 )
							{
								//cout << "setting to climb??" << endl;
								facingRight = true;
								action = STEEPCLIMB;
								frame = 0;
								rightWire->UpdateAnchors( V2d( 0, 0 ) );
								leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = next;
								q = 0;
							}
							else
							{
								ground = next;
								q = 0;
							}
						}
						else
						{
							ground = next;
							q = 0;
						}
					}
				}
				else
				{
					velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;

					//cout  <<  "reverse right" << endl;
					movementVec.y += .01;
					if( movementVec.x >= -.1 )
					{
						movementVec.x = -.1;
					}

					if( movingGround != NULL )
					{
						movementVec += currMovingTerrain->vel / (double)slowMultiple;
						cout << "5 movementvec is now: " << movementVec.x << ", " << movementVec.y <<
							", because of: " << currMovingTerrain->vel.x << ", " << currMovingTerrain->vel.y << endl;
					}

					action = JUMP;
					frame = 1;
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					leftGround = true;
					reversed = false;
					ground = NULL;
					movingGround = NULL;

					//leftGroundExtra.y = .01;
					//leftGroundExtra.x = -.01;
					leftGroundExtra = movementVec;

					return leftGroundExtra;
					//cout << "leaving ground RIGHT!!!!!!!!" << endl;
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//cout << "slide: " << q << ", " << offsetX << endl;
				if( movement > 0 )
					extra = (offsetX + movement) - b.rw;
				else 
				{
					extra = (offsetX + movement) + b.rw;
				}
				double m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offsetX = b.rw;
					}
					else
					{
						offsetX = -b.rw;
					}
				}
				else
				{
					movement = 0;
					offsetX += m;
				}

				/*if( approxEquals( m, 0 ) )
				{
					cout << "reverse blahh: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;	
					if( groundSpeed > 0 )
					{
						//cout << "transfer left "<< endl;
						Edge *next = ground->edge0;
						V2d nextNorm = e0n;
						if( nextNorm.y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && groundSpeed < -slopeLaunchMinSpeed && nextNorm.x < gNormal.x ) )
						{
							if( e0n.x > 0 && e0n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
							{
								groundSpeed = 0;
								offsetX = -offsetX;
								break;
							}
							else
							{
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
						}
						else if( abs( e0n.x ) >= wallThresh )
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							reversed = false;
							velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
							leftGround = true;

							ground = NULL;
						}
					}
					else if( groundSpeed < 0 )
					{
						Edge *next = ground->edge1;
						V2d nextNorm = e1n;
						if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && groundSpeed > slopeLaunchMinSpeed && nextNorm.x > 0 ) )
						{

							if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh )
							{
								groundSpeed = 0;
								offsetX = -offsetX;
								break;
							}
							ground = next;
							q = 0;
						}
						else if( abs( e1n.x ) >= wallThresh )
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
							leftGround = true;
							reversed = false;
							ground = NULL;
							//cout << "leaving ground RIGHT!!!!!!!!" << endl;
						}

					}
					
				}*/

				//wire problem could arise later because i dont update anchors when i hit an edge.
				if(!approxEquals( m, 0 ) )
				{
				
					V2d oldPos = position;
					bool hit = ResolvePhysics( V2d( -m, 0 ));
					//cout << "hit: " << hit << endl;
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.normal;//minContact.edge->Normal();
						//eNorm = -eNorm;
						m = -m;
						cout << "eNorm: " << eNorm.x << ", " << eNorm.y << ", m: " << m << endl;
						if( eNorm.y > 0 )
						{

							//this could be a problem later hopefully i solved it!
							bool speedTransfer = (eNorm.x < 0 && eNorm.y < steepThresh 
								&& groundSpeed > 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y < steepThresh 
									&& groundSpeed < 0 && groundSpeed <= steepClimbSpeedThresh);


							//bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed >= -steepClimbSpeedThresh)
							//		|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed <= steepClimbSpeedThresh)

							if( minContact.position.y <= position.y - b.rh + 5 && !speedTransfer )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									cout << "a" << endl;
									ground = minContact.edge;
									movingGround = minContact.movingPlat;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( minContact.position );

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}

									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									cout << "b" << endl;
									ground = minContact.edge;
									movingGround = minContact.movingPlat;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( minContact.position );

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}

									edgeQuantity = q;
									offsetX = b.rw;
									continue;
								}
								

							}
							else
							{
								cout << "c2:" << speedTransfer << endl;
								offsetX -= minContact.resolution.x;
								groundSpeed = 0;
								offsetX = -offsetX;
								break;
							}
						}
						else
						{

							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}

							cout << "d" << endl;
							offsetX -= minContact.resolution.x;
							groundSpeed = 0;
							offsetX = -offsetX;
							break;
						}
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
						leftWire->UpdateAnchors( wVel );
						rightWire->UpdateAnchors( wVel );
					}

				}
				else
				{
					//cout << "yo" << endl;
					//offsetX = -offsetX;
					//break;
					
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				

				if( approxEquals( m, 0 ) )
				{
					cout << "reverse secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					if( groundSpeed > 0 )
					{
						double yDist = abs( gNormal.x ) * -groundSpeed;
						Edge *next = ground->edge0;
						V2d nextNorm = e0n;
						if( nextNorm.y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x < gNormal.x ) )
						{
							if( e0n.x > 0 && e0n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
							{
								groundSpeed = 0;
								offsetX = -offsetX;
								break;
							}
							else
							{
								//ground = next;
								//q = length( ground->v1 - ground->v0 );	
								cout << "possible bug reversed. solved secret??" << endl;
							}
						}
						else if( abs( e0n.x ) >= wallThresh )
						{
							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}

							groundSpeed = 0;
							offsetX = -offsetX;
							break;
						}
						else
						{
							reversed = false;
							velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
							leftGround = true;

							ground = NULL;
							movingGround = NULL;
						}
					}
					else if( groundSpeed < 0 )
					{
						cout << "right"<< endl;
						Edge *next = ground->edge1;
						V2d nextNorm = e1n;
						double yDist = abs( gNormal.x ) * -groundSpeed;
						if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && nextNorm.x > 0 ) )
						{

							if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh )
							{
								groundSpeed = 0;
								offsetX = -offsetX;
								break;
							}
							else
							{
								cout << "possible other bug reversed. solved secret??" << endl;
								//ground = next;
								//q = 0;
							}
						}
						else if( abs( e1n.x ) >= wallThresh )
						{
							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}

							offsetX = -offsetX;
							groundSpeed = 0;
							break;
						}
						else
						{
							velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
							leftGround = true;
							reversed = false;
							ground = NULL;
							movingGround = NULL;
							//cout << "leaving ground RIGHT!!!!!!!!" << endl;
						}

					}
					
					//groundSpeed = 0;
					//offsetX = -offsetX;
					//break;
				}

				else // is this correct?
				//if( !approxEquals( m, 0 ) )
				{	
					//wire problem could arise later because i dont update anchors when i hit an edge.
					V2d oldPos = position;
					//cout << "moving: " << (normalize( ground->v1 - ground->v0 ) * m).x << ", " << 
					//	( normalize( ground->v1 - ground->v0 ) * m).y << endl;

					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					//cout << "hit: " << hit << endl;
					if( hit && (( m > 0 && ( minContact.edge != ground->edge0) ) || ( m < 0 && ( minContact.edge != ground->edge1 ) ) ) )
					{
						V2d eNorm = minContact.normal;//minContact.edge->Normal();
						eNorm = -eNorm;
						/*if( minContact.position.y < position.y + b.offset.y - b.rh + 5 && eNorm.y >= 0 )
						{
							if( minContact.position == minContact.edge->v0 ) 
							{
							//	cout << "edit1" << endl;
								if( minContact.edge->edge0 != ground ) //this line and the other one below are
									//because of how collision positions work on flat surfaces
								if( minContact.edge->edge0->Normal().y >= 0 )
								{
									minContact.edge = minContact.edge->edge0;
									eNorm = minContact.edge->Normal();
									eNorm = -eNorm;
								}
							}
							else if( minContact.position == minContact.edge->v1 )
							{
							//	cout << "edit2" << endl;

								if( minContact.edge->edge1 != ground )//same as above in terms of use
								if( minContact.edge->edge1->Normal().y >= 0 )
								{
									minContact.edge = minContact.edge->edge1;
									eNorm = minContact.edge->Normal();
									eNorm = -eNorm;
								}
							}
						}*/
						
						//cout<< "blah" << endl;
						if( eNorm.y < 0 )
						{
							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh);

							//regular
						//	bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
						//			|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);

							//bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
							//		|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);
							//cout << "speed transfer: " << speedTransfer << endl;
							if( minContact.position.y <= position.y + minContact.resolution.y - b.rh + b.offset.y + 5 && !speedTransfer)
							{
								double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									
								if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
								{
									cout << "BROKEN OFFSET: " << test << endl;
								}
								else
								{	
									//cout << "c" << endl;   
									//cout << "eNorm: " << eNorm.x << ", " << eNorm.y << endl;
									ground = minContact.edge;
									movingGround = minContact.movingPlat;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( minContact.position );

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}

									V2d eNorm = minContact.edge->Normal();			
									offsetX = position.x + minContact.resolution.x - minContact.position.x;
									offsetX = -offsetX;


									//wtf is this doing?
									//edgeQuantity = 0;
									//groundSpeed = 0;
									//break;

								}
							}
							else
							{
								cout << "xx" << endl;

								
								V2d oldv0 = ground->v0;
								V2d oldv1 = ground->v1;

								if( movingGround != NULL )
								{
									ground->v0 += movingGround->position;
									ground->v1 += movingGround->position;
								}

								q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);

								if( movingGround != NULL )
								{
									ground->v0 = oldv0;
									ground->v1 = oldv1;
								}

								
								groundSpeed = 0;
								edgeQuantity = q;
								offsetX = -offsetX;
								break;
							}
						}
						else
						{
							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}


							//cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;

							V2d oldv0 = ground->v0;
							V2d oldv1 = ground->v1;

							if( movingGround != NULL )
							{
								ground->v0 += movingGround->position;
								ground->v1 += movingGround->position;
							}

							q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);

							if( movingGround != NULL )
							{
								ground->v0 = oldv0;
								ground->v1 = oldv1;
							}

							groundSpeed = 0;
							offsetX = -offsetX;
							edgeQuantity = q;
							break;
						}						
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
						leftWire->UpdateAnchors( wVel );
						rightWire->UpdateAnchors( wVel );
					}
					
					
				}
			
			}

			offsetX = -offsetX;

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;	
		}
	}
	return leftGroundExtra;
}

//eventually need to change resolve physics so that the player can't miss going by enemies. i understand the need now
//for universal substeps. guess box2d makes more sense now doesn't it XD
void Actor::UpdatePhysics()
{
	if( action == DEATH )
	{
		return;
	}

	
	


	double temp_groundSpeed = groundSpeed / slowMultiple;
	V2d temp_velocity = velocity / (double)slowMultiple;
	double temp_grindSpeed = grindSpeed / slowMultiple;

	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );
	wallNormal.x = 0;
	wallNormal.y = 0;
	if( grindEdge != NULL )
	{
		if( reversed )
		{
			reversed = false;
			grindSpeed = -grindSpeed;
		}
		movement = temp_grindSpeed / NUM_STEPS;
	}
	else if( ground != NULL )
	{
		movement = temp_groundSpeed / NUM_STEPS;
	}
	else
	{
		movementVec = temp_velocity / NUM_STEPS;
	}

	if( physicsOver )
	{
		//still need to do hitbox/hurtbox responses if hes not moving
		return;
	}
	

	if( grindEdge != NULL )
	{
		Edge *e0 = grindEdge->edge0;
		Edge *e1 = grindEdge->edge1;
		V2d e0n = e0->Normal();
		V2d e1n = e1->Normal();
		
		double q = grindQuantity;
		while( !approxEquals(movement, 0 ) )
		{
			//cout << "movement: " << movement << endl;
			double gLen = length( grindEdge->v1 - grindEdge->v0 );
			if( movement > 0 )
			{
				double extra = q + movement - gLen;
				if( extra > 0 )
				{
					movement -= gLen - q;
					grindEdge = e1;
					q = 0;
					V2d v0 = grindEdge->v0;

					sf::Rect<double> r( v0.x - 1, v0.y - 1, 2, 2 );
					queryMode = "gate";
					owner->testGateCount = 0;
					owner->gateTree->Query( this, r );

					if( owner->testGateCount > 0 )
					{
						action = DEATH;
						rightWire->Reset();
						leftWire->Reset();
						slowCounter = 1;
						frame = 0;
						owner->deathWipe = true;

						owner->powerBar.Damage( 1000000 );
						return;
					}
					
				}
				else
				{
					q += movement;
					movement = 0;
				}
			}
			else if( movement < 0 )
			{
				double extra = q + movement;
				if( extra < 0 )
				{
					movement -= movement - extra;

					V2d v0 = grindEdge->v0;
					sf::Rect<double> r( v0.x - 1, v0.y - 1, 2, 2 );
					queryMode = "gate";
					owner->testGateCount = 0;
					owner->gateTree->Query( this, r );

					if( owner->testGateCount > 0 )
					{
						action = DEATH;
						rightWire->Reset();
						leftWire->Reset();
						slowCounter = 1;
						frame = 0;
						owner->deathWipe = true;

						owner->powerBar.Damage( 1000000 );
						return;
					}

					grindEdge = e0;
					q = length( e0->v1 - e0->v0 );
				}
				else
				{
					q += movement;
					movement = 0;
				}
			}
		}
		grindQuantity = q;

		PhysicsResponse();
		return;
	}
	else if( reversed )
	{
		//if you slide off a reversed edge you need a little push so you dont slide through the point.
		V2d reverseExtra = UpdateReversePhysics();
		if( reverseExtra.x == 0 && reverseExtra.y == 0 )
		{
			PhysicsResponse();
			return;
		}
		movementVec = reverseExtra;
		
	}

	while( (ground != NULL && !approxEquals( movement, 0 ) ) || ( ground == NULL && length( movementVec ) > 0 ) )
	{
		if( ground != NULL )
		{
			double steal = 0;
			if( movement > 0 )
			{
				if( movement > maxMovement )
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else 
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}


			double extra = 0;
			bool leaveGround = false;
			double q = edgeQuantity;

			V2d gNormal = ground->Normal();


			double m = movement;
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			bool transferLeft =  q == 0 && movement < 0 //&& (groundSpeed < -steepClimbSpeedThresh || e0n.y <= -steepThresh || e0n.x <= 0 )
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0)  ) 
				|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ) );
			bool transferRight = q == groundLength && movement > 0 //(groundSpeed < -steepClimbSpeedThresh || e1n.y <= -steepThresh || e1n.x >= 0 )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ));
		//	cout << "transferRight: " << transferRight << ": offset: " << offsetX << endl;
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
		//	cout << "speed: " << groundSpeed << ", pass: " << (offsetX == -b.rw ) << ", " << (e1n.x <=0 ) << ", " << (q == groundLength && movement > 0) << ", q: " << q << ", len: " << groundLength << endl;

			//on reverse doesnt need to fly up off of edges
			if( transferLeft )
			{
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				double yDist = abs( gNormal.x ) * groundSpeed;
				//cout << "yDist: " << yDist << ", -slopeluanchspeed: " << -slopeLaunchMinSpeed << endl;
				if( next->Normal().y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() /*&& !currInput.LLeft()*/ && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && next->Normal().x <= 0 ) )
				{
					if( e0n.x > 0 && e0n.y > -steepThresh )
					{
						if( groundSpeed >= -steepClimbSpeedThresh )
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							//cout << "steep transfer left" << endl;
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
					else if( gNormal.x > 0 && gNormal.y > -steepThresh )
					{
					
						velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;

						movementVec.y -= .01;
						if( movementVec.x >= -.01 )
						{
							movementVec.x = -.01;
						}

						leftGround = true;
						action = JUMP;
						frame = 1;
						rightWire->UpdateAnchors( V2d( 0, 0 ) );
						leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;
						movingGround = NULL;
						//bounceEdge = NULL;
						//grindEdge = NULL;
					}
					else
					{
					//	cout << "e0ny: " << e0n.y << ", gs: " << groundSpeed << "st: " << steepThresh <<
					//		", scst: " << steepClimbSpeedThresh  << endl;

						if( e0n.y > -steepThresh )
						{
							if( e0n.x < 0 )
							{
								if( gNormal.x >= -slopeTooSteepLaunchLimitX )
								{
									velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;

									movementVec.y -= .01;
									if( movementVec.x >= -.01 )
									{
										movementVec.x = -.01;
									}

									leftGround = true;
									action = JUMP;
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									movingGround = NULL;
								}
								else
								{
									facingRight = false;
									action = STEEPSLIDE;
									frame = 0;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = next;
									q = length( ground->v1 - ground->v0 );	
								}
							}
							else if( e0n.x > 0 )
							{
								facingRight = false;
								action = STEEPCLIMB;
								frame = 0;
								rightWire->UpdateAnchors( V2d( 0, 0 ) );
								leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
							else
							{
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
						}
						else
						{
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
				}
				else
				{
				//	cout << "leave left 2" << endl;
					velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * (extra);
					//cout << "b4 vec: " << movementVec.x << ", " << movementVec.y << endl;
					movementVec.y -= .01;
					if( movementVec.x >= -.01 )
					{
						movementVec.x = -.01;
					}
					//cout << "after vec: " << movementVec.x << ", " << movementVec.y << endl;
					leftGround = true;
					action = JUMP;
					frame = 1;
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					ground = NULL;
					movingGround = NULL;
				}
			}
			else if( transferRight )
			{
			//	cout << "transferRight!" << endl;
				double yDist = abs( gNormal.x ) * groundSpeed;
				Edge *next = ground->edge1;
				if( next->Normal().y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && /*!currInput.LRight() &&*/ gNormal.x < 0 && yDist > slopeLaunchMinSpeed && next->Normal().x >= 0 ) )
				{
					if( e1n.x < 0 && e1n.y > -steepThresh )
					{
						if( groundSpeed <= steepClimbSpeedThresh )
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							ground = next;
							q = 0;
							//cout << "steep transfer right" << endl;
						}
					}
					else if( gNormal.x < 0 && gNormal.y > -steepThresh )
					{
						//cout << "leave right 1" << endl;
						velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;

						movementVec.y -= .01;
						if( movementVec.x <= .01 )
						{
							movementVec.x = .01;
						}
						leftGround = true;
						action = JUMP;
						frame = 1;
						rightWire->UpdateAnchors( V2d( 0, 0 ) );
						leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;
						movingGround = NULL;
						//break;
					}
					else
					{

						if( e1n.y > -steepThresh)
						{
							if( e1n.x > 0 )
							{
								if( gNormal.x <= slopeTooSteepLaunchLimitX )
								{
									//cout << "bab" << endl;
									velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;

									movementVec.y -= .01;
									if( movementVec.x <= .01 )
									{
										movementVec.x = .01;
									}

									leftGround = true;
									action = JUMP;
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									movingGround = NULL;

								}
								else
								{
									//cout << "slidin" << endl;
									facingRight = true;
									action = STEEPSLIDE;
									frame = 0;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = next;
									q = 0;
								}
							}
							else if( e1n.x < 0 )
							{
								cout << "this here??" << endl;
								facingRight = true;
								action = STEEPCLIMB;
								frame = 0;
								rightWire->UpdateAnchors( V2d( 0, 0 ) );
								leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = next;
								q = 0;
							}
							else
							{
								ground = next;
								q = 0;
							}
							
						}
						else
						{
							ground = next;
							q = 0;
						}
						
						
					}
					
				}
				else
				{
					
					velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;
					
					movementVec.y -= .01;
					if( movementVec.x <= .01 )
					{
						movementVec.x = .01;
					}

					leftGround = true;
					ground = NULL;
					movingGround = NULL;

					action = JUMP;
					frame = 1;
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					//break;
					//cout << "leaving ground RIGHT!!!!!!!!" << endl;
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//cout << "slide: " << q << ", " << offsetX << endl;
				if( movement > 0 )
					extra = (offsetX + movement) - b.rw;
				else 
				{
					extra = (offsetX + movement) + b.rw;
				}
				double m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offsetX = b.rw;
					}
					else
					{
						offsetX = -b.rw;
					}
				}
				else
				{
					movement = 0;
					offsetX += m;
					/*if( abs( offsetX ) > b.rw + .00001 )
					{
						cout << "off: " << offsetX << endl;
						assert( false );
					}*/
				}

				if(!approxEquals( m, 0 ) )
				{
					V2d oldPos = position;
					bool hit = ResolvePhysics( V2d( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.edge->Normal();

						/*if( minContact.position.y > position.y + b.offset.y + b.rh - 5 && minContact.edge->Normal().y >= 0 )
						{
							if( minContact.position == minContact.edge->v0 ) 
							{
								if( minContact.edge->edge0->Normal().y <= 0 )
								{
									minContact.edge = minContact.edge->edge0;
									//eNorm = minContact.edge->Normal();
								}
							}
							//cout << "here" << endl;
						}*/

						if( eNorm.y < 0 )
						{

							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);
							if( minContact.position.y >= position.y + b.rh - 5 && !speedTransfer)
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;
									movingGround = minContact.movingPlat;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( minContact.position );

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}
									
									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									movingGround = minContact.movingPlat;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( minContact.position );

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}

									edgeQuantity = q;
									offsetX = b.rw;
									continue;
								}
								

							}
							else
							{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
							}
						}
						else
						{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
						}
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
						leftWire->UpdateAnchors( wVel );
						rightWire->UpdateAnchors( wVel );
					}
				}
			}
			else
			{
				//cout << "other" << endl;
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{

					//cout << "movement: " << movement << ", extra: " << extra << endl;
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				
				/*if( abs( offsetX ) > b.rw + .00001 )
				{
					cout << "off: " << offsetX << endl;
						assert( false );
				}*/

				if( approxEquals( m, 0 ) )
				{

					cout << "secret1: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX << ", " << groundSpeed <<  endl;

					if( groundSpeed > 0 )
					{
						
						Edge *next = ground->edge1;
						double yDist = abs( gNormal.x ) * groundSpeed;
						if( next->Normal().y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && next->Normal().x >= 0 ) )
						{
							if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
							{
								groundSpeed = 0;
								break;
							}
							else
							{
								//ground = next;
								//q = 0;
								cout << "possible bug. solved secret left??" << endl;
								break;
							}
					
						}
						else if( abs( e1n.x ) >= wallThresh )
						{
							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}

							groundSpeed = 0;
							break;
						}
						else
						{
							velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
							leftGround = true;
							ground = NULL;
							movingGround = NULL;
							//cout << "leaving ground RIGHT!!!!!!!!" << endl;
						}
					}
					else if( groundSpeed < 0 )
					{
						double yDist = abs( gNormal.x ) * groundSpeed;
						Edge *next = ground->edge0;
						if( next->Normal().y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && next->Normal().x < gNormal.x ) )
						{
							if( e0n.x > 0 && e0n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh )
							{
								groundSpeed = 0;
								break;
							}
							else
							{
								cout << "possible bug. solved secret??" << endl;
								//ground = next;
								//q = length( ground->v1 - ground->v0 );	
								break;
							}
						}
						else if( abs( e0n.x ) >= wallThresh )
						{
							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}

							groundSpeed = 0;
							break;
						}
						else
						{
							velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
							leftGround = true;

							ground = NULL;
							movingGround = NULL;
						}
					}
				}

				//only want this to fire if secret doesn't happen
				else//if( !approxEquals( m, 0 ) )
				{	
					bool down = true;
					V2d oldPos = position;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
							//cout << "change hit" << endl;
						if( down)
						{
							V2d eNorm = minContact.normal;
							
							if( minContact.position == minContact.edge->v0 )
							{

							}
							//minContact.edge->Normal();
							if( minContact.position.y > position.y + b.offset.y + b.rh - 5 && minContact.edge->Normal().y >= 0 )
							{
								if( minContact.position == minContact.edge->v0 ) 
								{
									if( minContact.edge->edge0->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge0;
										//eNorm = minContact.edge->Normal();
									}
								}
								/*else if( minContact.position == minContact.edge->v1 )
								{
									if( minContact.edge->edge1->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge1;
										eNorm = minContact.edge->Normal();
									}
								}*/
							}



							if( eNorm.y < 0 )
							{
								//bool 
								//cout << "min:" << minContact.position.x << ", " << minContact.position.y  << endl;
								//cout << "lel: " << position.y + minContact.resolution.y + b.rh - 5 << endl;
								//cout << "res: " << minContact.resolution.y << endl;

								/*CircleShape cs;
								cs.setFillColor( Color::Cyan );
								cs.setRadius( 20 );
								cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
								cs.setPosition( minContact.resolution.x, minContact.resolution.y );

								owner->window->draw( cs );
								cs.setPosition( position.x, position.y + minContact.resolution.y + b.rh - 5);
									cs.setRadius( 10 );
								cs.setFillColor( Color::Magenta );
								owner->window->draw( cs );*/
								bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);

								if( minContact.position.y >= position.y + minContact.resolution.y + b.rh + b.offset.y - 5  && !speedTransfer)
								{
									double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									
									if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
									{
										cout << "BROKEN OFFSET: " << test << endl;
									}
									else
									{	
										
										//cout << "cxxxx" << endl;
										ground = minContact.edge;
										movingGround = minContact.movingPlat;

										V2d oldv0 = ground->v0;
										V2d oldv1 = ground->v1;

										if( movingGround != NULL )
										{
											ground->v0 += movingGround->position;
											ground->v1 += movingGround->position;
										}

										q = ground->GetQuantity( minContact.position );

										if( movingGround != NULL )
										{
											ground->v0 = oldv0;
											ground->v1 = oldv1;
										}
										V2d eNorm = minContact.normal;//minContact.edge->Normal();			
										offsetX = position.x + minContact.resolution.x - minContact.position.x;
									}

									/*if( offsetX < -b.rw || offsetX > b.rw )
									{
										cout << "BROKEN OFFSET: " << offsetX << endl;
										assert( false && "T_T" );
									}*/
								}
								else
								{
									cout << "xx" << endl;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}
									
									groundSpeed = 0;
									edgeQuantity = q;
									break;
								}
							}
							else
							{
								V2d testVel = normalize(ground->v1 - ground->v0 ) * groundSpeed;
								//maybe want to do this on some ceilings but its hard for now. do i need it?
								if( currInput.LUp() && testVel.y < -offSlopeByWallThresh && eNorm.y == 0 )
								{
									assert( abs(eNorm.x ) > wallThresh );
									cout << "testVel: " << testVel.x << ", " << testVel.y << endl;
									velocity = testVel;
						
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
									leftGround = true;
									ground = NULL;
									movingGround = NULL;

									action = JUMP;
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
								}
								else
								{
									if( bounceFlameOn && abs( groundSpeed ) > 1)
									{
										storedBounceGroundSpeed = groundSpeed * slowMultiple;
										groundedWallBounce = true;
									}
									//cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;

									V2d oldv0 = ground->v0;
									V2d oldv1 = ground->v1;

									if( movingGround != NULL )
									{
										ground->v0 += movingGround->position;
										ground->v1 += movingGround->position;
									}

									q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);

									if( movingGround != NULL )
									{
										ground->v0 = oldv0;
										ground->v1 = oldv1;
									}

								
									groundSpeed = 0;
									edgeQuantity = q;
								}
								break;
							}
						}
						else
						{
							cout << "Sdfsdfd" << endl;

						V2d oldv0 = ground->v0;
							V2d oldv1 = ground->v1;

							if( movingGround != NULL )
							{
								ground->v0 += movingGround->position;
								ground->v1 += movingGround->position;
							}

							q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);

							if( movingGround != NULL )
							{
								ground->v0 = oldv0;
								ground->v1 = oldv1;
							}

							groundSpeed = 0;
							edgeQuantity = q;
							break;
						}
						
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
						leftWire->UpdateAnchors( wVel );
						rightWire->UpdateAnchors( wVel );
					}
				}

				
			/*	else
				{
					edgeQuantity = q;
					cout << "secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
				//	assert( false && "secret!" );
					break;
					//offsetX = -offsetX;
			//		cout << "prev: " << e0n.x << ", " << e0n.y << endl;
					//break;
				}*/
					
			}

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
		else
		{
			V2d stealVec(0,0);
			double moveLength = length( movementVec );
			V2d velDir = normalize( movementVec );
			if( moveLength > maxMovement )
			{
				stealVec = velDir * ( moveLength - maxMovement);
				movementVec = velDir * maxMovement;
			}

			V2d newVel( 0, 0 );
			V2d oldPos = position;

			bool tempCollision = ResolvePhysics( movementVec );

			V2d extraVel(0, 0);
			if( tempCollision  )
			{
				collision = true;			
				position += minContact.resolution;

				Edge *e = minContact.edge;
				V2d en = e->Normal();
				Edge *e0 = e->edge0;
				Edge *e1 = e->edge1;
				V2d e0n = e0->Normal();
				V2d e1n = e1->Normal();

				if( minContact.position.y > position.y + b.offset.y + b.rh - 5 && minContact.edge->Normal().y >= 0 )
				{
					if( minContact.position == minContact.edge->v0 ) 
					{
						if( minContact.edge->edge0->Normal().y <= 0 )
						{
							minContact.edge = minContact.edge->edge0;
						}
					}
				}

				if( abs(minContact.edge->Normal().x) > wallThresh )
				{
					wallNormal = minContact.edge->Normal();
				}


				V2d extraDir =  normalize( minContact.edge->v1 - minContact.edge->v0 );

				if( (minContact.position == e->v0 && en.x < 0 && en.y < 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v1 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}

				else if( (minContact.position == e->v1 && en.x < 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v0 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{	
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v1 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = V2d( 1, 0 );//extraDir;
					}
				}
				else if( (minContact.position == e->v0 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = V2d( 1, 0 );
					}
				}

				


				if( (minContact.position == e->v1 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
					}
				}
				else if( (minContact.position == e->v0 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
					}
				}
				
				if( minContact.normal.x != 0 || minContact.normal.y != 0 )
				{
					if( abs( minContact.normal.x ) > wallThresh || ( minContact.normal.y > 0 && abs( minContact.normal.x ) > .9 ) )
					{
						wallNormal = minContact.normal;
					}
					extraDir = V2d( minContact.normal.y, -minContact.normal.x );
				}				
				
				extraVel = dot( normalize( velocity ), extraDir ) * extraDir * length(minContact.resolution);
				
				newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );
				
				if( length( stealVec ) > 0 )
				{
					stealVec = length( stealVec ) * normalize( extraVel );
				}
				if( approxEquals( extraVel.x, lastExtra.x ) && approxEquals( extraVel.y, lastExtra.y ) )
				{
					break;		
				}
				if( length( extraVel ) > 0 )
				{
					lastExtra.x = extraVel.x;
					lastExtra.y = extraVel.y;
				}
			}
			else if( length( stealVec ) == 0 )
			{

				movementVec.x = 0;
				movementVec.y = 0;
			}

			V2d wVel = position - oldPos;

			leftWire->UpdateAnchors( wVel );
			rightWire->UpdateAnchors( wVel );
			

			int maxJumpHeightFrame = 10;

			if( tempCollision && leftWire->state == Wire::PULLING )
			{
				touchEdgeWithLeftWire = true;
			}

			if( tempCollision && rightWire->state == Wire::PULLING )
			{
				touchEdgeWithRightWire = true;
			}

			bool bounceOkay = true;
			
			int trueFramesInAir = framesInAir;
			if( tempCollision )
			{
				framesInAir = maxJumpHeightFrame + 1;
			}

			if( tempCollision )
			{
				if( bounceEdge != NULL )
				{
					bounceOkay = false;
					bounceEdge = NULL;
					oldBounceEdge = NULL;
					action = JUMP;
					frame = 1;
					break;
				}
				V2d en = minContact.normal;
				
				if( en.y <= 0 && en.y > -steepThresh )
				{
					if( en.x < 0 && velocity.x < 0 
			  		|| en.x > 0 && velocity.x > 0 )
						bounceOkay = false;
				}
				else if( en.y >= 0 && -en.y > -steepThresh )
				{
					if( en.x < 0 && velocity.x < 0 
			  		|| en.x > 0 && velocity.x > 0 )
						bounceOkay = false;
				}
				else if( en.y == 0  )
				{
					if( en.x < 0 && velocity.x < 0 
			  		|| en.x > 0 && velocity.x > 0 )
						bounceOkay = false;
				}
				else if( en.y < 0 )
				{
					if( velocity.y < 0 )
						bounceOkay = false;
				}
				else if( en.y > 0 )
				{
					if( velocity.y > 0 )
						bounceOkay = false;
				}
			}

			

			if( ( action == BOUNCEAIR || action == BOUNCEGROUND || bounceFlameOn ) && tempCollision && bounceOkay )
			{
				//this condition might only work when not reversed? does it matter?
				if( bounceEdge == NULL )//|| ( bounceEdge != NULL && minContact.edge->Normal().y < 0 && bounceEdge->Normal().y >= 0 ) )
				{
					bounceEdge = minContact.edge;
					bounceMovingTerrain = minContact.movingPlat;
					bounceNorm = minContact.normal;
				

					V2d oldv0 = bounceEdge->v0;
					V2d oldv1 = bounceEdge->v1;

					if( bounceMovingTerrain != NULL )
					{
						bounceEdge->v0 += bounceMovingTerrain->position;
						bounceEdge->v1 += bounceMovingTerrain->position;
					}

					bounceQuant = bounceEdge->GetQuantity( minContact.position );
				

					if( bounceMovingTerrain != NULL )
					{
						bounceEdge->v0 = oldv0;
						bounceEdge->v1 = oldv1;
					}

					offsetX = ( position.x + b.offset.x )  - minContact.position.x;

					movement = 0;
					break;
					//cout << "bouncing: " << bounceQuant << endl;
				}
				else
				{
					//if( oldBounceEdge != NULL && minContact.edge != oldBounceEdge && action == BOUNCEAIR && framesInAir < 11 )
					//if( bounceEdge != NULL && minContact.edge != bounceEdge )
					{
						if( action == BOUNCEAIR )
						{
							cout << "bounce air" << endl;
						}
						else
						{
							cout << "bounce ground" << endl;
						}
						cout << "stopped it here! framesinair: " << trueFramesInAir << endl;
						bounceEdge = NULL;
						oldBounceEdge = NULL;
						action = JUMP;
						frame = 1;
						break;
					}
					
					/*oldBounceEdge = bounceEdge;
					bounceEdge = minContact.edge;
					bounceNorm = minContact.normal;
					bounceMovingTerrain = minContact.movingPlat;*/
				}
				
				
			//	cout << "offset now!: " << offsetX << endl;
				//groundSpeed = 0;
			//	cout << "bouncing" << endl;
			}
			//else if( ((action == JUMP && !holdJump) || framesInAir > maxJumpHeightFrame ) && tempCollision && minContact.edge->Normal().y < 0 && abs( minContact.edge->Normal().x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			else if( ((action == JUMP && !holdJump) || framesInAir > maxJumpHeightFrame || action == WALLCLING ) && tempCollision && minContact.normal.y < 0 && abs( minContact.normal.x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			{
				
				//b.rh = dashHeight;
				//cout << "edge: " << minContact.edge->v0.x << ", " << minContact.edge->v0.y << ", v1: " << minContact.edge->v1.x << ", " << minContact.edge->v1.y << endl;
				//cout << "pos: " << position.x << ", " << position.y << ", minpos: " << minContact.position.x << ", " << minContact.position.y << endl;
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;

				//cout << "offsetX: " << offsetX << endl;

				//if( offsetX > b.rw + .00001 || offsetX < -b.rw - .00001 ) //to prevent glitchy stuff
				if( false )
				{
					//cout << "prevented glitchy offset: " << offsetX << endl;
				}
				else
				{
					//if( offsetX > b.rw + .00001 || offsetX < -b.rw - .00001 )

					if( offsetX > b.rw + .00001 || offsetX < -b.rw - .00001 ) //stops glitchyness with _\ weird offsets
					{
						//assert( minContact.edge->Normal().y == -1 );
						cout << "normal that offset is glitchy on: " << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << ", offset: " << offsetX 
							<< ", truenormal: " << minContact.normal.x << ", " << minContact.normal.y << endl;
						cout << "position.x: " << position.x << ", minx " << minContact.position.x << endl;
						if( offsetX > 0 )
						{
							offsetX = b.rw;
							minContact.position.x = position.x - b.rw;
						}
						else
						{
							offsetX = -b.rw;
							minContact.position.x = position.x + b.rw;
						}
					}

				if( b.rh == doubleJumpHeight )
				{
					b.offset.y = (normalHeight - doubleJumpHeight);
				}

				//if( reversed )
				//	b.offset.y = -b.offset.y;

				//cout << "LANDINGGGGGG------" << endl;
				assert( !(minContact.normal.x == 0 && minContact.normal.y == 0 ) );
				//cout << "normal: " << minContact.normal.x << ", " << minContact.normal.y << endl;
				//if(!( minContact.normal.x == 0 && minContact.normal.y == 0 ) && minContact.edge->Normal().y == 0 )
				//{
				//	minContact.edge = minContact.edge->edge0;
				//}
				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				movingGround = minContact.movingPlat;

				V2d oldv0 = ground->v0;
				V2d oldv1 = ground->v1;

				if( movingGround != NULL )
				{
					ground->v0 += movingGround->position;
					ground->v1 += movingGround->position;
				}

				edgeQuantity = minContact.edge->GetQuantity( minContact.position );

				if( movingGround != NULL )
				{
					ground->v0 = oldv0;
					ground->v1 = oldv1;
				}

				V2d alongVel = V2d( -minContact.normal.y, minContact.normal.x );
				
				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = dot( velocity, alongVel );//normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				//cout << "setting groundSpeed: " << groundSpeed << endl;
				V2d gNorm = ground->Normal();//minContact.normal;//ground->Normal();
				

				//if( gNorm.y <= -steepThresh )
				{
					hasGravReverse = true;
					hasAirDash = true;
					hasDoubleJump = true;
					lastWire = 0;
				}

				if( velocity.x < 0 && gNorm.y <= -steepThresh )
				{
					groundSpeed = min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ) * .7);
				}
				else if( velocity.x > 0 && gNorm.y <= -steepThresh )
				{
					groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ) * .7 );
				}
				//groundSpeed  = max( abs( velocity.x ), ( - ) );
				
				if( velocity.x < 0 )
				{
				//	groundSpeed = -groundSpeed;
				}

				//cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << ", offset: " << offsetX << endl;

				movement = 0;
			
				
				
				//cout << "offsetX: " <<offsetX << endl;
				//cout << "offsetX: " << offsetX << endl;
				//cout << "offset now!: " << offsetX << endl;
				//V2d gn = ground->Normal();
				
				if( ground->Normal().x > 0 && offsetX < b.rw && !approxEquals( offsetX, b.rw ) )					
				{
					//cout << "super secret fix offsetx1: " << offsetX << endl;
					//offsetX = b.rw;
				}
				if( ground->Normal().x < 0 && offsetX > -b.rw && !approxEquals( offsetX, -b.rw ) ) 
				{
					//cout << "super secret fix offsetx2: " << offsetX << endl;
					//offsetX = -b.rw;
				}
				}
				//cout << "groundinggg" << endl;
			}
			else if( hasPowerGravReverse && hasGravReverse && tempCollision && currInput.B && currInput.LUp() && minContact.normal.y > 0 && abs( minContact.normal.x ) < wallThresh && minContact.position.y <= position.y - b.rh + b.offset.y + 1 )
			{
				if( b.rh == doubleJumpHeight )
				{
					b.offset.y = (normalHeight - doubleJumpHeight);
				}
				//b.rh = dashHeight;
				
				//if( reversed )
					//b.offset.y = -b.offset.y;


				if( minContact.edge->Normal().y <= 0 )
				{
					if( minContact.position == minContact.edge->v0 ) 
					{
						if( minContact.edge->edge0->Normal().y >= 0 )
						{
							minContact.edge = minContact.edge->edge0;
							//eNorm = minContact.edge->Normal();
						}
					}
					//cout << "here" << endl;
				}
				/*if(  minContact.edge->Normal().y  0 )
				{
					if( minContact.position == minContact.edge->v0 ) 
					{
						if( minContact.edge->edge0->Normal().y <= 0 )
						{
							minContact.edge = minContact.edge->edge0;
							//eNorm = minContact.edge->Normal();
						}
					}
					//cout << "here" << endl;
				}*/

				hasGravReverse = false;
				hasAirDash = true;
				hasDoubleJump = true;
				reversed = true;
				lastWire = 0;

				b.offset.y = -b.offset.y;
				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				movingGround = minContact.movingPlat;

				V2d oldv0 = ground->v0;
				V2d oldv1 = ground->v1;

				if( movingGround != NULL )
				{
					ground->v0 += movingGround->position;
					ground->v1 += movingGround->position;
				}

				edgeQuantity = minContact.edge->GetQuantity( minContact.position );

				if( movingGround != NULL )
				{
					ground->v0 = oldv0;
					ground->v1 = oldv1;
				}

				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = 0;
				//groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				V2d gno = ground->Normal();


				double angle = atan2( gno.x, -gno.y );
				owner->ActivateEffect( ts_fx_gravReverse, position, false, angle, 36, 1, facingRight );
				//cout << "gno: " << gno.x << ", " << gno.y << endl;
				if( -gno.y > -steepThresh )
				{
				//	cout << "a" << endl;
					groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );
					if( velocity.x < 0 )
					{
					//	groundSpeed = -min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
					else if( velocity.x > 0 )
					{
					//	groundSpeed = -max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
					//groundSpeed = 0;
				}
				else
				{
				//	cout << "b" << endl;
					groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );
					if( velocity.x < 0 )
					{
						//groundSpeed = min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
					else if( velocity.x > 0 )
					{
						//groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
				}

				//cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << endl;

				movement = 0;
			
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;

				if( ground->Normal().x > 0 && offsetX < b.rw && !approxEquals( offsetX, b.rw ) )					
				{
				//	cout << "super secret fix offsetx122: " << offsetX << endl;
				//	offsetX = b.rw;
				}
				if( ground->Normal().x < 0 && offsetX > -b.rw && !approxEquals( offsetX, -b.rw ) ) 
				{
				//	cout << "super secret fix offsetx222: " << offsetX << endl;
				//	offsetX = -b.rw;
				}
			}
			else if( tempCollision )
			{

				//cout << "setting newvel" << endl;
				velocity = newVel;
			}
			else
			{
				//cout << "no temp collision" << endl;
			}

			if( length( extraVel ) > 0 )
			{
				movementVec = stealVec + extraVel;
			//	cout << "x1: " << movementVec.x << ", " << movementVec.y << endl;
			}

			else
			{
				movementVec = stealVec;
				//cout << "x2:  " << movementVec.x << ", " << movementVec.y << endl;
			//	cout << "x21: " << stealVec.x << ", " << stealVec.y << endl;
				//cout << "x22: " << movementVec.x << ", " << movementVec.y << endl;
			}


		}
	}

	PhysicsResponse();
}

void Actor::PhysicsResponse()
{
	//velocity *= (double)slowMultiple;
	//groundSpeed *= slowMultiple;
	//grindSpeed *= slowMultiple;

	V2d gn;

	if( grindEdge != NULL )
	{
		framesInAir = 0;

		V2d oldv0 = grindEdge->v0;
		V2d oldv1 = grindEdge->v1;


		if( grindMovingTerrain != NULL )
		{
			grindEdge->v0 += grindMovingTerrain->position;
			grindEdge->v1 += grindMovingTerrain->position;
		}


		V2d grindPoint = grindEdge->GetPoint( grindQuantity );

		if( grindMovingTerrain != NULL )
		{
			grindEdge->v0 = oldv0;
			grindEdge->v1 = oldv1;
		}

		position = grindPoint;
	}
	else if( bounceEdge != NULL )
	{
		V2d bn = bounceNorm;

		if( action == BOUNCEAIR || bounceFlameOn )
		{
			physicsOver = true;
			//cout << "BOUNCING HERE" << endl;

			storedBounceVel = velocity;
			bounceFlameOn = false;

			action = BOUNCEGROUND;
			boostBounce = false;
			frame = 0;

			if( bn.y <= 0 && bn.y > -steepThresh )
			{
				if( storedBounceVel.x > 0 && bn.x < 0 && facingRight || storedBounceVel.x < 0 && bn.x > 0 && !facingRight )
				{
					facingRight = !facingRight;
				}
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				if( storedBounceVel.x > 0 && bn.x < 0 && facingRight || storedBounceVel.x < 0 && bn.x > 0 && !facingRight )
					facingRight = !facingRight;
			}
			else if( bn.y == 0  )
			{
				facingRight = !facingRight;
			}
			else if( bn.y < 0 )
			{
				hasGravReverse = true;
				hasDoubleJump = true;
				hasAirDash = true;
				lastWire = 0;

				if( abs( storedBounceVel.y ) < 10 )
				{
					//cout << "land: " << abs(storedBounceVel.y) << endl;
					bounceFlameOn = true;
					runBounceFrame = 4 * 3;
					action = LAND;
					frame = 0;
					//bounceEdge = NULL;
					ground = bounceEdge;
					edgeQuantity = bounceQuant;
					bounceEdge = NULL;
					
				}
			}

			if( bn.y != 0 )
			{
				if( bounceEdge != NULL )
				{
					V2d oldv0 = bounceEdge->v0;
					V2d oldv1 = bounceEdge->v1;

					if( bounceMovingTerrain != NULL )
					{
						bounceEdge->v0 += bounceMovingTerrain->position;
						bounceEdge->v1 += bounceMovingTerrain->position;
					}

					position = bounceEdge->GetPoint( bounceQuant );

					if( bounceMovingTerrain != NULL )
					{
						bounceEdge->v0 = oldv0;
						bounceEdge->v1 = oldv1;
					}
				}
				else
				{
					V2d oldv0 = ground->v0;
					V2d oldv1 = ground->v1;

					if( movingGround != NULL )
					{
						ground->v0 += movingGround->position;
						ground->v1 += movingGround->position;
					}

					position = ground->GetPoint( bounceQuant );

					if( movingGround != NULL )
					{
						ground->v0 = oldv0;
						ground->v1 = oldv1;
					}
				}
		
				position.x += offsetX + b.offset.x;

				if( bn.y > 0 )
				{
					{
						position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}
				else
				{
					if( bn.y < 0 )
					{
						position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}
			}
		}

	}
	else if( ground != NULL )
	{
		framesInAir = 0;
		gn = ground->Normal();
		if( collision )
		{
			if( action == AIRHITSTUN )
			{
				action = GROUNDHITSTUN;
				frame = 0;
			}
			else
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					action = LAND2;
					rightWire->UpdateAnchors(V2d( 0, 0 ));
					leftWire->UpdateAnchors(V2d( 0, 0 ));
					frame = 0;
				}
				else
				{
					//cout << "l" << endl;
					action = LAND;
					rightWire->UpdateAnchors(V2d( 0, 0 ));
					leftWire->UpdateAnchors(V2d( 0, 0 ));
					frame = 0;
				}
			}
		}


		V2d oldv0 = ground->v0;
		V2d oldv1 = ground->v1;

		if( movingGround != NULL )
		{
			ground->v0 += movingGround->position;
			ground->v1 += movingGround->position;
		}

		Vector2<double> groundPoint = ground->GetPoint( edgeQuantity );

		if( movingGround != NULL )
		{
			ground->v0 = oldv0;
			ground->v1 = oldv1;
		}
		
		position = groundPoint;
		
		position.x += offsetX + b.offset.x;

		if( reversed )
		{
			if( gn.y > 0 || abs( offsetX ) != b.rw )
			{
				position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
			}
		}
		else
		{
			if( gn.y < 0 || abs( offsetX ) != b.rw )
			{
				position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
			}
		}

		if( reversed )
		{
			if( ( action == STEEPCLIMB || action == STEEPSLIDE ) && (-gn.y <= -steepThresh || !approxEquals( abs( offsetX ), b.rw ) ) )
			{
				action = LAND2;
				frame = 0;
			}
		}
		else
		{
			
			if( ( action == STEEPCLIMB || action == STEEPSLIDE ) && (gn.y <= -steepThresh || !approxEquals( abs( offsetX ), b.rw ) ) )
			{
				action = LAND2;
				frame = 0;
			}
			else
			{

			}
		}
		
	}
	else
	{
		

		if( action == GROUNDHITSTUN )
		{
			action = AIRHITSTUN;
			frame = 0;
		}

		if( action != AIRHITSTUN )
		{
			//oldAction = action;
			if( collision )
			{
				if( length( wallNormal ) > 0 && oldVelocity.y >= 8 )
				{
					if( wallNormal.x > 0)
					{
						if( currInput.LLeft() )
						{
							//cout << "setting to wallcling" << endl;
							facingRight = true;
							action = WALLCLING;
							frame = 0;
						}
					}
					else
					{
						if( currInput.LRight() )
						{
							//cout << "setting to wallcling" << endl;
							facingRight = false;
							action = WALLCLING;
							frame = 0;
						}
					
					}
				}
			}
			
			else if( oldAction == WALLCLING )
			{
				bool stopWallClinging = false;
				if( collision && length( wallNormal ) > 0 )
				{
					if( wallNormal.x > 0 )
					{
						if( !currInput.LLeft() )
						{
							stopWallClinging = true;
						}
					}
					else
					{
						if( !currInput.LRight() )
						{
							stopWallClinging = true;
						}
					}
				}
				else
				{
					stopWallClinging = true;
					
				}

				if( stopWallClinging )
				{
					//cout << "stop wall clinging" << endl;
					action = JUMP;
					frame = 1;
				}
			}

			if( leftGround )
			{
				action = JUMP;
				frame = 1;
				
			}
		}
	}

	if( groundedWallBounce )
	{
		action = BOUNCEGROUNDEDWALL;
		frame = 0;

		//already bounced by here i think
		if( storedBounceGroundSpeed < 0 )
		{
			cout << "facing right: " << groundSpeed << endl;
			facingRight = true;
		}
		else
		{
			cout << "facing left: " << groundSpeed << endl;
			facingRight = false;
		}
	}

	rightWire->UpdateAnchors(V2d( 0, 0 ));
	leftWire->UpdateAnchors(V2d( 0, 0 ));

	UpdateHitboxes();
}

void Actor::UpdateHitboxes()
{
	double angle = 0;
	V2d gn;
	V2d gd; 
	if( ground != NULL )
	{
		if( !approxEquals( abs(offsetX), b.rw ) )
		{
			gn = V2d( 0, -1 );
			gd = V2d( 1, 0 );
			if( reversed )
			{
				angle = PI;
				gn = V2d( 0, 1 );
				gd = V2d( -1, 0 );
			}
			//this should never happen
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
			gn = ground->Normal();
			gd = normalize( ground->v1 - ground->v0 );
		}
	}

	if( currHitboxes != NULL )
	{
		for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
		{
			if( ground != NULL )
			{
				(*it).globalAngle = angle;
			}
			else
			{
				(*it).globalAngle = 0;
			}

			double offX = (*it).offset.x;
			double offY = (*it).offset.y;

			if( ( !facingRight && !reversed ) || ( facingRight && reversed ) )
				offX = -offX;

			//if( reversed )
			//	offY = -offY;

			V2d pos = position;
			if( ground != NULL )
			{
			//	V2d gn = ground->Normal();
				pos = V2d( sprite->getPosition().x, sprite->getPosition().y );

				pos += gd * offX + gn * -offY + gn * (double)sprite->getLocalBounds().height / 2.0;
				//pos += gd * offX + gn * -offY; //+ V2d( offsetX, 0 );
			}
			else
			{
				pos += V2d( offX, offY );// + V2d( offsetX, 0 );
			}

			(*it).globalPosition = pos;
			//(*it).globalPosition = position + V2d( offX * cos( (*it).globalAngle ) + offY * sin( (*it).globalAngle ), 
			//	offX * -sin( (*it).globalAngle ) + offY * cos( (*it).globalAngle ) );

			//(*it).globalPosition = position + (*it).offset;
		
		}
	}
	
	
	if( action == AIRDASH )
	{
		hurtBody.isCircle = true;
		hurtBody.rw = b.rw;
		hurtBody.rh = b.rw;
		//hurtBody.offset = 
	}
	else
	{
		hurtBody.isCircle = false;
		hurtBody.rw = b.rw;
		hurtBody.rh = b.rh;
		
		hurtBody.offset = b.offset;
	}

	
	
	//cout << "hurtbody offset: " << hurtBody.offset.x << ", " << hurtBody.offset.y << endl;
	
	hurtBody.globalPosition = position + hurtBody.offset;
	hurtBody.globalAngle = angle;
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
	//			hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	//hurtBody.globalPosition = position;

	b.globalPosition = position + b.offset;
	b.globalAngle = 0;
		
}

void Actor::UpdatePostPhysics()
{
	


	//rightWire->UpdateState( false );
	if( rightWire->numPoints == 0 )
	{
	//	rightWire->segmentLength = length( rightWire->anchor.pos - position );//rightWire->totalLength;
	}
	else
	{
	//	rightWire->segmentLength = length( rightWire->points[rightWire->numPoints-1].pos - position );
	}

	
	V2d gn;
	if( ground != NULL )
		gn = ground->Normal();

	if( action == DEATH )
	{
		sh.setParameter( "On0", false );
		sh.setParameter( "On1", false );
		sh.setParameter( "On2", false );
		sh.setParameter( "despFrame", -1 );

		sprite->setTexture( *(tileset[DEATH]->texture));
		if( facingRight )
		{
			sprite->setTextureRect( tileset[DEATH]->GetSubRect( frame / 2 ) );
		}
		else
		{
			sf::IntRect ir = tileset[DEATH]->GetSubRect( frame / 2 );
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );
		sprite->setRotation( 0 );

		//if( slowCounter == slowMultiple )
		//{
		
		++frame;
			//slowCounter = 1;
		//}
		//else
		//	slowCounter++;
		return;
	}

	if( record > 0 )
	{

		PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
		p.showSword1 = false;
	}

	switch( action )
	{
	case STAND:
		{	
			
		sprite->setTexture( *(tileset[STAND]->texture));
			
		//sprite->setTextureRect( tilesetStand->GetSubRect( frame / 4 ) );
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[STAND]->GetSubRect( frame / 8 ) );
		}
		else
		{
			sf::IntRect ir = tileset[STAND]->GetSubRect( frame / 8 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}


		if( ground != NULL )
		{
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}
			
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			sprite->setRotation( angle / PI * 180 );


			//cout << "angle: " << angle / PI * 180  << endl;
		}

		//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		//sprite->setPosition( position.x, position.y );
		//cout << "setting to frame: " << frame / 4 << endl;
		break;
		}
	case RUN:
		{	
			
		if( frame == 0 )
			{
				runTappingSound.stop();
				runTappingSound.play();
			}

		

		//if( bounceGrounded )
		//{
		////	sprite->setTexture( *(ts_bounceRun->texture));
		//}
		//else
		//{
		//	
		//}

		sprite->setTexture( *(tileset[RUN]->texture));
		
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			/*if( bounceGrounded )
			{
				sprite->setTextureRect( ts_bounceRun->GetSubRect( frame / 4 ) );
			}
			else*/
			{
				sprite->setTextureRect( tileset[RUN]->GetSubRect( frame / 4 ) );
			}
			
		}
		else
		{
			sf::IntRect ir;                                              
			/*if( bounceGrounded )
			{
				ir = ts_bounceRun->GetSubRect( frame / 4 );
			}
			else*/
			{
				ir = tileset[RUN]->GetSubRect( frame / 4 );
			}
			 
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
			

		if( ground != NULL )
		{
			double angle = GroundedAngle();
			//cout << "angle: " << angle << endl;
			//sprite->setOrigin( b.rw, 2 * b.rh );
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}


			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			//sprite->setPosition( position.x, position.y );
			
			//sprite->setPosition( position.x, position.y );
			//cout << "angle: " << angle / PI * 180  << endl;

			if( frame % 5 == 0 && abs( groundSpeed ) > 0 )
			{
				//owner->ActivateEffect( ts_fx_bigRunRepeat, pp + gn * 56.0, false, angle, 24, 1, facingRight );
			}
		}
		break;
		}
	case SPRINT:
		{	
		
		/*if( bounceGrounded )
		{
			sprite->setTexture( *(ts_bounceSprint->texture));
		}
		else*/
		{
			sprite->setTexture( *(tileset[SPRINT]->texture));
		}
		
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			/*if( bounceGrounded )
			{
				sprite->setTextureRect( ts_bounceSprint->GetSubRect( frame / 3 ) );
			}
			else*/
			{
				sprite->setTextureRect( tileset[SPRINT]->GetSubRect( frame / 3 ) );
			}
			
		}
		else
		{
			sf::IntRect ir;                                              
			/*if( bounceGrounded )
			{
				ir = ts_bounceSprint->GetSubRect( frame / 3 );
			}
			else*/
			{
				ir = tileset[SPRINT]->GetSubRect( frame / 3 );
			}

			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
			

		if( ground != NULL )
		{
			double angle = GroundedAngle();

			//sprite->setOrigin( b.rw, 2 * b.rh );
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
		}
		break;
		}
	case JUMP:
		{
		sprite->setTexture( *(tileset[JUMP]->texture));
		{
		sf::IntRect ir;

		if( frame == 0 )
		{
			ir = tileset[JUMP]->GetSubRect( 0 );
		}
		else if( velocity.y < -15)
		{
			ir = tileset[JUMP]->GetSubRect( 1 );
		}
		else if( velocity.y < 7 )
		{
			ir = tileset[JUMP]->GetSubRect( 2 );
		}
		else if( velocity.y < 9 )
		{
			ir = tileset[JUMP]->GetSubRect( 3 );
		}
		else if( velocity.y < 12 )
		{
			ir = tileset[JUMP]->GetSubRect( 4 );
		}
		else if( velocity.y < 35)
		{
			ir = tileset[JUMP]->GetSubRect( 5 );
		}
		else if( velocity.y < 37 )
		{
			ir = tileset[JUMP]->GetSubRect( 6 );
		}
		else if( velocity.y < 40 )
		{
			ir = tileset[JUMP]->GetSubRect( 7 );
		}
		else
		{
			ir = tileset[JUMP]->GetSubRect( 8 );
		}

		if( frame > 0 )
		{
			sprite->setRotation( 0 );
		}

		if( ( !facingRight && !reversed ) )
		{
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		else
		{
			sprite->setTextureRect( ir );
		}
		}
		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );

		break;
		}
	case LAND: 
		{

		

		sprite->setTexture( *(tileset[LAND]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[LAND]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[LAND]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}


		double angle = GroundedAngle();
		

		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
		sprite->setRotation( angle / PI * 180 );
		
		V2d oldv0 = ground->v0;
		V2d oldv1 = ground->v1;

		if( movingGround != NULL )
		{
			ground->v0 += movingGround->position;
			ground->v1 += movingGround->position;
		}

		V2d pp = ground->GetPoint( edgeQuantity );

		if( movingGround != NULL )
		{
			ground->v0 = oldv0;
			ground->v1 = oldv1;
		}

		if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
			sprite->setPosition( pp.x + offsetX, pp.y );
		else
			sprite->setPosition( pp.x, pp.y );

		if( frame == 0 )
		{
			V2d fxPos;
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				fxPos = V2d( pp.x + offsetX, pp.y );
			else
				fxPos = pp;

			fxPos += gn * 8.0;

			owner->ActivateEffect( ts_fx_land, fxPos, false, angle, 14, 1, facingRight );
		}


		break;
		}
	case LAND2: 
		{
		sprite->setTexture( *(tileset[LAND2]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[LAND2]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[LAND2]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		
		double angle = GroundedAngle();

		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
		sprite->setRotation( angle / PI * 180 );
		
		V2d oldv0 = ground->v0;
		V2d oldv1 = ground->v1;

		if( movingGround != NULL )
		{
			ground->v0 += movingGround->position;
			ground->v1 += movingGround->position;
		}

		V2d pp = ground->GetPoint( edgeQuantity );

		if( movingGround != NULL )
		{
			ground->v0 = oldv0;
			ground->v1 = oldv1;
		}

		if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
			sprite->setPosition( pp.x + offsetX, pp.y );
		else
			sprite->setPosition( pp.x, pp.y );
		break;
		}
	case WALLCLING:
		{
		sprite->setTexture( *(tileset[WALLCLING]->texture));
		if( facingRight )
		{
			sprite->setTextureRect( tileset[WALLCLING]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[WALLCLING]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );
		sprite->setRotation( 0 );
		break;
		}
	case WALLJUMP:
		{
			sprite->setTexture( *(tileset[WALLJUMP]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[WALLJUMP]->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[WALLJUMP]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case SLIDE:
		{
		sprite->setTexture( *(tileset[SLIDE]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[SLIDE]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[SLIDE]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}

		double angle = GroundedAngle();

		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
		sprite->setRotation( angle / PI * 180 );
		
		V2d oldv0 = ground->v0;
		V2d oldv1 = ground->v1;

		if( movingGround != NULL )
		{
			ground->v0 += movingGround->position;
			ground->v1 += movingGround->position;
		}

		V2d pp = ground->GetPoint( edgeQuantity );

		if( movingGround != NULL )
		{
			ground->v0 = oldv0;
			ground->v1 = oldv1;
		}

		if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
		break;
		}
	case STANDN:
		{
			int startFrame = 1;
			showSword1 = frame / 2 >= startFrame && frame / 2 <= 4;

			sprite->setTexture( *(tileset[STANDN]->texture));

			//Vector2i offset( 24, -16 );
			Vector2i offset( 24, 0 );


			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STANDN]->GetSubRect( frame / 2 ) );

				if( showSword1 )
					standingNSword1.setTextureRect( ts_standingNSword1->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[STANDN]->GetSubRect( frame / 2 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
				
				if( showSword1  )
				{
					sf::IntRect irSword = ts_standingNSword1->GetSubRect( frame / 2 - startFrame );
					standingNSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}

				
			}

			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword1 )
			{
				standingNSword1.setOrigin( standingNSword1.getLocalBounds().width / 2, standingNSword1.getLocalBounds().height);
				standingNSword1.setRotation( angle / PI * 180 );

			
				//standingNSword1.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			//V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y ) + V2d( offset.x * cos( angle ) + offset.y * sin( angle ), 
			//offset.x * -sin( angle ) +  offset.y * cos( angle ) );
			V2d pos = V2d( sprite->getPosition().x, sprite->getPosition().y );
			V2d truDir( -trueNormal.y, trueNormal.x );//normalize( ground->v1 - ground->v0 );

			pos += trueNormal * (double)offset.y;
			pos += truDir * (double)offset.x;

			standingNSword1.setPosition( pos.x, pos.y );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword1 = showSword1;
				p.swordSprite1 = standingNSword1;
			}

			break;
		}
	case STANDD:
		{
			int startFrame = 2;
			showSword1 = frame / 2 >= startFrame && frame / 2 <= 7;


			sprite->setTexture( *(tileset[STANDD]->texture));

			Vector2i offset( 24, 0 );

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STANDD]->GetSubRect( frame / 2 ) );

				if( showSword1 )
					standingDSword1.setTextureRect( ts_standingDSword1->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[STANDD]->GetSubRect( frame / 2 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword1  )
				{
					sf::IntRect irSword = ts_standingDSword1->GetSubRect( frame / 2 - startFrame );
					standingDSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}
			}
			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword1 )
			{
				standingDSword1.setOrigin( standingDSword1.getLocalBounds().width / 2, standingDSword1.getLocalBounds().height);
				standingDSword1.setRotation( angle / PI * 180 );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			V2d pos = V2d( sprite->getPosition().x, sprite->getPosition().y );
			V2d truDir( -trueNormal.y, trueNormal.x );//normalize( ground->v1 - ground->v0 );

			pos += trueNormal * (double)offset.y;
			pos += truDir * (double)offset.x;

			standingDSword1.setPosition( pos.x, pos.y );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword1 = showSword1;
				p.swordSprite1 = standingDSword1;
			}

			break;
		}
	case STANDU:
		{
			int startFrame = 1;
			showSword1 = frame >= startFrame && frame <= 18;


			sprite->setTexture( *(tileset[STANDU]->texture));

			Vector2i offset( 0, 32 );

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STANDU]->GetSubRect( frame ) );

				if( showSword1 )
					standingUSword1.setTextureRect( ts_standingUSword1->GetSubRect( frame - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[STANDU]->GetSubRect( frame );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword1  )
				{
					sf::IntRect irSword = ts_standingUSword1->GetSubRect( frame - startFrame );
					standingUSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}
			}
			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword1 )
			{
				standingUSword1.setOrigin( standingUSword1.getLocalBounds().width / 2, standingUSword1.getLocalBounds().height);
				standingUSword1.setRotation( angle / PI * 180 );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			V2d pos = V2d( sprite->getPosition().x, sprite->getPosition().y );
			V2d truDir( -trueNormal.y, trueNormal.x );//normalize( ground->v1 - ground->v0 );

			pos += trueNormal * (double)offset.y;
			pos += truDir * (double)offset.x;

			standingUSword1.setPosition( pos.x, pos.y );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword1 = showSword1;
				p.swordSprite1 = standingUSword1;
			}
			break;
		}
	case FAIR:
		{
			


			int startFrame = 1;
			showSword1 = frame / 2 >= startFrame && frame / 2 <= 9;
			sprite->setTexture( *(tileset[FAIR]->texture));

			Vector2i offset( 32, -16 );

			if( facingRight )
			{
				
				sprite->setTextureRect( tileset[FAIR]->GetSubRect( frame / 2 ) );
				if( showSword1 )
					fairSword1.setTextureRect( ts_fairSword1->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[FAIR]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				
				if( showSword1  )
				{
					offset.x = -offset.x;

					sf::IntRect irSword = ts_fairSword1->GetSubRect( frame / 2 - startFrame );
					fairSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}
					
			}

			if( showSword1 )
			{
				fairSword1.setOrigin( fairSword1.getLocalBounds().width / 2, fairSword1.getLocalBounds().height / 2 );
				fairSword1.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword1 = showSword1;
				p.swordSprite1 = fairSword1;
			}
			
			break;
		}
	case DAIR:
		{
			int startFrame = 0;
			showSword1 = frame / 2 >= startFrame && frame / 2 <= 9;

			Vector2i offset( 0, 40 );

			sprite->setTexture( *(tileset[DAIR]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[DAIR]->GetSubRect( frame / 2 ) );

				if( showSword1 )
					dairSword1.setTextureRect( ts_dairSword1->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				//offset.x = -offset.x;

				sf::IntRect ir = tileset[DAIR]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword1  )
				{
					//offset.x = -offset.x;

					sf::IntRect irSword = ts_dairSword1->GetSubRect( frame / 2 - startFrame );
					dairSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}
			}

			if( showSword1 )
			{
				dairSword1.setOrigin( dairSword1.getLocalBounds().width / 2, dairSword1.getLocalBounds().height / 2 );
				dairSword1.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case UAIR:
		{
			int startFrame = 0;
			showSword1 = frame / 3 >= startFrame && frame / 3 <= 5;
			sprite->setTexture( *(tileset[UAIR]->texture));

			Vector2i offset( 8, -24 );

			if( facingRight )
			{
				sprite->setTextureRect( tileset[UAIR]->GetSubRect( frame / 3 ) );

				if( showSword1 )
					uairSword1.setTextureRect( ts_uairSword1->GetSubRect( frame / 3 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[UAIR]->GetSubRect( frame / 3 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword1 )
				{
					//offset.x = -offset.x;

					sf::IntRect irSword = ts_uairSword1->GetSubRect( frame / 3 - startFrame );
					uairSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}

				offset.x = -offset.x;
			}

			if( showSword1 )
			{
				uairSword1.setOrigin( uairSword1.getLocalBounds().width / 2, uairSword1.getLocalBounds().height / 2 );
				uairSword1.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case DOUBLE:
		{
	
			int fr = frame;
			if ( frame > 27)
			{
				fr = 27;
			}
			sprite->setTexture( *(tileset[DOUBLE]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[DOUBLE]->GetSubRect( fr / 1 ) );
			}
			else
			{
				sf::IntRect ir = tileset[DOUBLE]->GetSubRect( fr / 1 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case DASH:
		{
			
			if( frame == 0 )
			{
				dashStartSound.stop();
				//if( slowMultiple != 1)
				//	dashStartSound.setPitch( .2 );
				//else
				//	dashStartSound.setPitch( 1 );
				dashStartSound.play();
			}

			//if( slowMultiple != 1)
			//		dashStartSound.setPitch( .2 );
			//	else
			//		dashStartSound.setPitch( 1 );

			sprite->setTexture( *(tileset[DASH]->texture));

			//3-8 is the cycle
			sf::IntRect ir;
			int checkFrame;
			if( frame / 2 < 3 )
			{
				checkFrame = frame / 2;
				ir = tileset[DASH]->GetSubRect( checkFrame );
			}
			else if( frame < actionLength[DASH] - 6 )
			{
				checkFrame = 3 + ( (frame/2 - 3) % 6 );
				ir = tileset[DASH]->GetSubRect( checkFrame );
			}
			else
			{
				checkFrame = 9 + (6 - ( actionLength[DASH] - frame )) / 2;
				ir = tileset[DASH]->GetSubRect( checkFrame );
			}
		//	cout << "checkframe: " << checkFrame << endl;


			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( ir );
			}
			else
			{
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}

			
			double angle = GroundedAngle();
			

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			if( frame == 0 )
			{
				owner->ActivateEffect( ts_fx_dashStart, 
					pp + gn * 16.0, false, angle, 7, 3, facingRight );
			}
			else if( frame % 5 == 0 )
			{
				owner->ActivateEffect( ts_fx_dashRepeat, 
					pp + gn * 8.0, false, angle, 12, 3, facingRight );
			}

			break;
		}
	case GRINDBALL:
		{
			assert( grindEdge != NULL );
			sprite->setTexture( *(tileset[GRINDBALL]->texture));

			sf::IntRect ir;
			
			ir = tileset[GRINDBALL]->GetSubRect( 0 );
			
			grindActionCurrent += grindSpeed / 20;
			while( grindActionCurrent >= grindActionLength )
			{
				grindActionCurrent -= grindActionLength;
			}
			while( grindActionCurrent < 0 )
			{
				grindActionCurrent += grindActionLength;
			}

			int grindActionInt = grindActionCurrent;




			gsdodeca.setTextureRect( tsgsdodeca->GetSubRect( grindActionInt * 5 % grindActionLength   ) );
			gstriblue.setTextureRect( tsgstriblue->GetSubRect( grindActionInt * 5 % grindActionLength ) );
			gstricym.setTextureRect( tsgstricym->GetSubRect( grindActionInt ) );
			gstrigreen.setTextureRect( tsgstrigreen->GetSubRect( grindActionInt * 5 % grindActionLength ) );
			gstrioran.setTextureRect( tsgstrioran->GetSubRect( grindActionInt ));
			gstripurp.setTextureRect( tsgstripurp->GetSubRect( grindActionInt ) );
			gstrirgb.setTextureRect( tsgstrirgb->GetSubRect( grindActionInt ) );

			

			if( facingRight )
			{
				sprite->setTextureRect( ir );
			}
			else
			{
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}

			double angle = 0;

			if( !approxEquals( abs(offsetX), b.rw ) )
			{

			}
			else
			{
				angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		//	sprite->setRotation( angle / PI * 180 );
			sprite->setRotation( 0 );
			
			V2d oldv0 = grindEdge->v0;
			V2d oldv1 = grindEdge->v1;


			if( grindMovingTerrain != NULL )
			{
				grindEdge->v0 += grindMovingTerrain->position;
				grindEdge->v1 += grindMovingTerrain->position;
			}

			V2d pp = grindEdge->GetPoint( grindQuantity );

			if( grindMovingTerrain != NULL )
			{
				grindEdge->v0 = oldv0;
				grindEdge->v1 = oldv1;
			}
			
			sprite->setPosition( pp.x, pp.y );


			gsdodeca.setOrigin( gsdodeca.getLocalBounds().width / 2, gsdodeca.getLocalBounds().height / 2);
			gstriblue.setOrigin( gstriblue.getLocalBounds().width / 2, gstriblue.getLocalBounds().height / 2);
			gstricym.setOrigin( gstricym.getLocalBounds().width / 2, gstricym.getLocalBounds().height / 2);
			gstrigreen.setOrigin( gstrigreen.getLocalBounds().width / 2, gstrigreen.getLocalBounds().height / 2);
			gstrioran.setOrigin( gstrioran.getLocalBounds().width / 2, gstrioran.getLocalBounds().height / 2);
			gstripurp.setOrigin( gstripurp.getLocalBounds().width / 2, gstripurp.getLocalBounds().height / 2);
			gstrirgb.setOrigin( gstrirgb.getLocalBounds().width / 2, gstrirgb.getLocalBounds().height / 2);


			gsdodeca.setPosition( pp.x, pp.y );
			gstriblue.setPosition( pp.x, pp.y );
			gstricym.setPosition( pp.x, pp.y );
			gstrigreen.setPosition( pp.x, pp.y );
			gstrioran.setPosition( pp.x, pp.y );
			gstripurp.setPosition( pp.x, pp.y );
			gstrirgb.setPosition( pp.x, pp.y );

			break;
		}
	case STEEPSLIDE:
		{
			sprite->setTexture( *(tileset[STEEPSLIDE]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STEEPSLIDE]->GetSubRect( 0 ) );
			//	sprite->setOrigin( sprite->getLocalBounds().width - 10, sprite->getLocalBounds().height);
			}
			else
			{
				sf::IntRect ir = tileset[STEEPSLIDE]->GetSubRect( 0 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			//	sprite->setOrigin( 10, sprite->getLocalBounds().height);
			}

			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
				//this should never happen
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height );
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			sprite->setPosition( pp.x, pp.y );
			//if( angle == 0 )
			//	sprite->setPosition( pp.x + offsetX, pp.y );
			//else
			//	sprite->setPosition( pp.x, pp.y );
			break;
		}
	case AIRDASH:
		{
			if( frame % 1 == 0 )
			{
				owner->ActivateEffect( ts_fx_airdash, V2d( position.x, position.y + 25 ), false, 0, 10, 4, facingRight );
			}


			sprite->setTexture( *(tileset[AIRDASH]->texture));

			int f = 0;
			if( currInput.LUp() )
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					f = 2;
				}
				else
				{
					f = 1;
				}
			}
			else if( currInput.LDown() )
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					f = 4;
				}
				else
				{
					f = 5;
				}
			}
			else
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					f = 3;
				}
				else
				{
					f = 0;
				}
			}
			if( facingRight )
			{
				
				sprite->setTextureRect( tileset[AIRDASH]->GetSubRect( f ) );
			}
			else
			{
				sf::IntRect ir = tileset[AIRDASH]->GetSubRect( f );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case STEEPCLIMB:
		{
			sprite->setTexture( *(tileset[STEEPCLIMB]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STEEPCLIMB]->GetSubRect( frame / 4 ) );
			//	sprite->setOrigin( sprite->getLocalBounds().width - 10, sprite->getLocalBounds().height);
			}
			else
			{
				sf::IntRect ir = tileset[STEEPCLIMB]->GetSubRect( frame / 4 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			//	sprite->setOrigin( 10, sprite->getLocalBounds().height);
			}

			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
				//this should never happen
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height );
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			sprite->setPosition( pp.x, pp.y );
			//if( angle == 0 )
			//	sprite->setPosition( pp.x + offsetX, pp.y );
			//else
			//	sprite->setPosition( pp.x, pp.y );
			break;
		}
	case AIRHITSTUN:
		{
			if( frame == 0 )
			{
				playerHitSound.stop();
				playerHitSound.play();
			}

			sprite->setTexture( *(tileset[AIRHITSTUN]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[AIRHITSTUN]->GetSubRect( 0 ) );
			}
			else
			{
				sf::IntRect ir = tileset[AIRHITSTUN]->GetSubRect( 0 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case GROUNDHITSTUN:
		{
			sprite->setTexture( *(tileset[GROUNDHITSTUN]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[GROUNDHITSTUN]->GetSubRect( 0 ) );
			}
			else
			{
				sf::IntRect ir = tileset[GROUNDHITSTUN]->GetSubRect( 0 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			
			double angle = GroundedAngle();

		

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			break;
		}
	case WIREHOLD:
		{
			sprite->setTexture( *(tileset[WIREHOLD]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[WIREHOLD]->GetSubRect( 0 ) );
			}
			else
			{
				sf::IntRect ir = tileset[WIREHOLD]->GetSubRect( 0 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case BOUNCEAIR:
		{
			int bounceFrame = 0;
			if( oldBounceEdge == NULL )
			{
				bounceFrame = 6;
			}
			else if( framesSinceBounce < 10 )
			{
				V2d bn = oldBounceNorm;//oldBounceEdge->Normal();
				if( bn.y <= 0 && bn.y > -steepThresh )
				{
					bounceFrame = 3;
					if( facingRight )
					{
						//facingRight = false;

					}
				}
				else if( bn.y >= 0 && -bn.y > -steepThresh )
				{
					
					bounceFrame = 3;
				}
				else if( bn.y == 0 )
				{
					bounceFrame = 3;
				}
				else if( bn.y < 0 )
				{
					bounceFrame = 1;
				}
				else if( bn.y > 0 )
				{
					bounceFrame = 5;
				}
			}
			else
			{
				bounceFrame = 6;
			}
			
			

			sprite->setTexture( *(tileset[BOUNCEAIR]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[BOUNCEAIR]->GetSubRect( bounceFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[BOUNCEAIR]->GetSubRect( bounceFrame );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case BOUNCEGROUND:
		{
			int bounceFrame = 0;
			V2d bn = bounceNorm;//bounceEdge->Normal();

			if( bn.y <= 0 && bn.y > -steepThresh )
			{
				if( (bn.x > 0 && storedBounceVel.x >= 0) || (bn.x < 0 && storedBounceVel.x <= 0 ) )
					bounceFrame = 0;
				else
					bounceFrame = 2;
				//facingRight = !facingRight;
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				if( (bn.x > 0 && storedBounceVel.x >= 0) || (bn.x < 0 && storedBounceVel.x <= 0 ) )
					bounceFrame = 4;
				else
					bounceFrame = 2;
				//facingRight = !facingRight;
			}
			else if( bn.y == 0 )
			{
				bounceFrame = 2;
			//	facingRight = !facingRight;
			}
			else if( bn.y < 0 )
			{
				bounceFrame = 0;
			}
			else if( bn.y > 0 )
			{
				bounceFrame = 4;
			}



			
			sprite->setTexture( *(tileset[BOUNCEGROUND]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[BOUNCEGROUND]->GetSubRect( bounceFrame  ) );
			}
			else
			{
				sf::IntRect ir = tileset[BOUNCEGROUND]->GetSubRect( bounceFrame  );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}


			//bool extraCase = ( offsetX < 0 && approxEquals( edgeQuantity, 0 ) )
			//	|| ( offsetX > 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) );

			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
						angle = PI;
			}
			else
			{
				angle = atan2( bn.x, -bn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

			if( abs( bn.x ) >= wallThresh )
			{
				if( bn.x > 0 )
				{
					sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
				}
				else
				{
					sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
				}
			}
			else if( bn.y <= 0 && bn.y > -steepThresh )
			{
				if( bounceFrame == 0 )
				{
					sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - normalHeight);
				}
				else
				{
					if( facingRight )
					{
						
						sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
					}
					else
					{
						sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
						
					}
					
					//sprite->setOrigin( 10, sprite->getLocalBounds().height / 2);
				}
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				if( facingRight )//bounceFrame == 4 )
				{
					//sprite->setOrigin( sprite->getLocalBounds().width / 2, 0);
					sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
				}
				else
				{
					sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
				}	
			}
			else if( bn.y < 0 )
			{
				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - normalHeight);
			}
			else if( bn.y > 0 )
			{
				//cout << "this one" << endl;
				sprite->setOrigin( sprite->getLocalBounds().width / 2, normalHeight );
			}

			
			sprite->setPosition( position.x, position.y );
		//		sprite->setRotation( angle / PI * 180 );

			V2d oldv0 = bounceEdge->v0;
			V2d oldv1 = bounceEdge->v1;

			if( bounceMovingTerrain != NULL )
			{
				bounceEdge->v0 += bounceMovingTerrain->position;
				bounceEdge->v1 += bounceMovingTerrain->position;
			}

			V2d pp = bounceEdge->GetPoint( bounceQuant );

			if( bounceMovingTerrain != NULL )
			{
				bounceEdge->v0 = oldv0;
				bounceEdge->v1 = oldv1;
			}
				
				//if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				//	sprite->setPosition( pp.x + offsetX, pp.y );
				//else
				//	sprite->setPosition( pp.x, pp.y );

			break;
		}
	case BOUNCEGROUNDEDWALL:
		{
			//if( frame == 0 && slowCounter == 1 )
			//{
				
			//}

			sprite->setTexture( *(tileset[BOUNCEGROUNDEDWALL]->texture));

			sf::IntRect ir;
			if( frame < 6 )
			{
				ir = tileset[BOUNCEGROUNDEDWALL]->GetSubRect( 0 );
			}
			else if( frame < 20 )
			{
				ir = tileset[BOUNCEGROUNDEDWALL]->GetSubRect( 1 );
			}
			else
			{
				ir = tileset[BOUNCEGROUNDEDWALL]->GetSubRect( 2 );
			}
			


			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( ir );
			}
			else
			{
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}

			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			if( frame < 6 )
			{
				if( ( facingRight && !reversed ) || (!facingRight && reversed ) )
				{
					sprite->setOrigin( sprite->getLocalBounds().width / 2 - 3, sprite->getLocalBounds().height);
				}
				else
				{
					sprite->setOrigin( sprite->getLocalBounds().width / 2 + 3, sprite->getLocalBounds().height);
				}
				
				angle = 0;
				if( reversed )
					angle = PI;
			}
			else
			{
				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			}
			
			sprite->setRotation( angle / PI * 180 );
			
			V2d oldv0 = ground->v0;
			V2d oldv1 = ground->v1;

			if( movingGround != NULL )
			{
				ground->v0 += movingGround->position;
				ground->v1 += movingGround->position;
			}

			V2d pp = ground->GetPoint( edgeQuantity );

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}


			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			break;
		}
	case DEATH:
		{
		
		}
	}

	if( bounceFlameOn )
	{
		if( ground == NULL )
		{
			bounceFlameSprite.setTexture( *ts_airBounceFlame->texture );
			bounceFlameSprite.setTextureRect( ts_airBounceFlame->GetSubRect( airBounceFrame / 3 ) );

			//double angle = 0;
			//angle = atan2( velocity.y, velocity.x );

			if( velocity.x < 0  )
			{
				sf::IntRect r = bounceFlameSprite.getTextureRect();
				bounceFlameSprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
			}

			bounceFlameSprite.setOrigin( bounceFlameSprite.getLocalBounds().width / 2, bounceFlameSprite.getLocalBounds().height / 2 );
			bounceFlameSprite.setPosition( sprite->getPosition() );
			
			//bounceFlameSprite.setRotation( angle / PI * 180 );
			bounceFlameSprite.setRotation( 0 );
		}
		else
		{
			bounceFlameSprite.setTexture( *ts_runBounceFlame->texture );
			bounceFlameSprite.setTextureRect( ts_runBounceFlame->GetSubRect( runBounceFrame / 3 ) );
			if( ( groundSpeed < 0 && !reversed ) || ( groundSpeed > 0 && reversed ) )
			{
				sf::IntRect r = bounceFlameSprite.getTextureRect();
				bounceFlameSprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
			}
			bounceFlameSprite.setOrigin( bounceFlameSprite.getLocalBounds().width / 2, bounceFlameSprite.getLocalBounds().height );
			bounceFlameSprite.setPosition( sprite->getPosition() );
			bounceFlameSprite.setRotation( sprite->getRotation() );
			
		}
	}
	
	

	Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );

	owner->lightsAtOnce = 0;
	owner->tempLightLimit = 3;
	queryMode = "lights"; 
	owner->lightTree->Query( this, r );

	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
	//owner->preScreenTex->map
	
	
	
	testLight->pos.x = position.x;
	testLight->pos.y = position.y;
	if( action == FAIR || action == UAIR || action == DAIR || action == STANDN || action == STANDU || action == STANDD )
	{
		testLight->brightness = 40;
		testLight->radius = 150;
	}
	else
	{
		testLight->brightness = 15;
		testLight->radius = 100;
	}
	//vi0 = vi1 = vi2 = vi;
	

	
	
	
	

	//cout << "lights captured!: " << owner->lightsAtOnce << endl;
	//cout << "pos0: " << pos0.x << ", " << pos0.y << endl;
	//cout << "pos1: " << pos1.x << ", " << pos1.y << endl;
	//cout << "pos2: " << pos2.x << ", " << pos2.y << endl;

	//lighting stuff
	{
	bool on[9];
	for( int i = 0; i < 9; ++i )
	{
		on[i] = false;
	}

	sh.setParameter( "AmbientColor", 1, 1, 1, 1 );
	sh.setParameter( "Resolution", 1920, 1080 );//owner->window->getSize().x, owner->window->getSize().y );
	sh.setParameter( "right", (facingRight && !reversed) || (facingRight && reversed ) );
	sh.setParameter( "zoom", owner->cam.GetZoom() );

	float windowx = 1920;//owner->window->getSize().x;
	float windowy = 1080;//owner->window->getSize().y;

	if( owner->lightsAtOnce > 0 )
	{

		float depth0 = owner->touchedLights[0]->depth;
		Vector2i vi0 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[0]->pos.x, owner->touchedLights[0]->pos.y ) );
		Vector3f pos0( vi0.x / windowx, ( windowy - vi0.y) / windowy, depth0 ); 
		Color c0 = owner->touchedLights[0]->color;
		
		//sh.setParameter( "On0", true );
		on[0] = true;
		sh.setParameter( "LightPos0", pos0 );//Vector3f( 0, -300, .075 ) );
		sh.setParameter( "LightColor0", c0.r / 255.0, c0.g / 255.0, c0.b / 255.0, 1 );
		sh.setParameter( "Radius0", owner->touchedLights[0]->radius );
		sh.setParameter( "Brightness0", owner->touchedLights[0]->brightness);
		
	}
	if( owner->lightsAtOnce > 1 )
	{
		float depth1 = owner->touchedLights[1]->depth;
		Vector2i vi1 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[1]->pos.x, owner->touchedLights[1]->pos.y ) );
		
		Vector3f pos1( vi1.x / windowx, ( windowy - vi1.y) / windowy, depth1 ); 
		Color c1 = owner->touchedLights[1]->color;
		
		on[1] = true;
		//sh.setParameter( "On1", true );
		sh.setParameter( "LightPos1", pos1 );//Vector3f( 0, -300, .075 ) );
		sh.setParameter( "LightColor1", c1.r / 255.0, c1.g / 255.0, c1.b / 255.0, 1 );
		sh.setParameter( "Radius1", owner->touchedLights[1]->radius );
		sh.setParameter( "Brightness1", owner->touchedLights[1]->brightness);
	}
	if( owner->lightsAtOnce > 2 )
	{
		float depth2 = owner->touchedLights[2]->depth;
		Vector2i vi2 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[2]->pos.x, owner->touchedLights[2]->pos.y ) );
		Vector3f pos2( vi2.x / windowx, ( windowy - vi2.y) / windowy, depth2 ); 
		Color c2 = owner->touchedLights[2]->color;
		
		on[2] = true;
		//sh.setParameter( "On2", true );
		sh.setParameter( "LightPos2", pos2 );//Vector3f( 0, -300, .075 ) );
		sh.setParameter( "LightColor2", c2.r / 255.0, c2.g / 255.0, c2.b / 255.0, 1 );
		sh.setParameter( "Radius2", owner->touchedLights[2]->radius );
		sh.setParameter( "Brightness2", owner->touchedLights[2]->brightness);
	}
	if( owner->lightsAtOnce > 3 )
	{
		float depth3 = owner->touchedLights[3]->depth;
		Vector2i vi3 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[3]->pos.x, owner->touchedLights[3]->pos.y ) );
		Vector3f pos3( vi3.x / windowx, ( windowy - vi3.y) / windowy, depth3 ); 
		Color c3 = owner->touchedLights[3]->color;
		
		on[3] = true;
		//sh.setParameter( "On3", true );
		sh.setParameter( "LightPos3", pos3 );
		sh.setParameter( "LightColor3", c3.r / 255.0, c3.g / 255.0, c3.b / 255.0, 1 );
		sh.setParameter( "Radius3", owner->touchedLights[3]->radius );
		sh.setParameter( "Brightness3", owner->touchedLights[3]->brightness);
	}
	if( owner->lightsAtOnce > 4 )
	{
		float depth4 = owner->touchedLights[4]->depth;
		Vector2i vi4 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[4]->pos.x, owner->touchedLights[4]->pos.y ) );
		Vector3f pos4( vi4.x / windowx, ( windowy - vi4.y) / windowy, depth4 ); 
		Color c4 = owner->touchedLights[4]->color;
		
		
		on[4] = true;
		sh.setParameter( "LightPos4", pos4 );
		sh.setParameter( "LightColor4", c4.r / 255.0, c4.g / 255.0, c4.b / 255.0, 1 );
		sh.setParameter( "Radius4", owner->touchedLights[4]->radius );
		sh.setParameter( "Brightness4", owner->touchedLights[4]->brightness);
	}
	if( owner->lightsAtOnce > 5 )
	{
		float depth5 = owner->touchedLights[5]->depth;
		Vector2i vi5 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[5]->pos.x, owner->touchedLights[5]->pos.y ) );
		Vector3f pos5( vi5.x / windowx, ( windowy - vi5.y) / windowy, depth5 ); 
		Color c5 = owner->touchedLights[5]->color;
		
		
		on[5] = true;
		sh.setParameter( "LightPos5", pos5 );
		sh.setParameter( "LightColor5", c5.r / 255.0, c5.g / 255.0, c5.b / 255.0, 1 );
		sh.setParameter( "Radius5", owner->touchedLights[5]->radius );
		sh.setParameter( "Brightness5", owner->touchedLights[5]->brightness);
	}
	if( owner->lightsAtOnce > 6 )
	{
		float depth6 = owner->touchedLights[6]->depth;
		Vector2i vi6 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[6]->pos.x, owner->touchedLights[6]->pos.y ) );
		Vector3f pos6( vi6.x / windowx, ( windowy - vi6.y) / windowy, depth6 ); 
		Color c6 = owner->touchedLights[6]->color;
		
		on[6] = true;
		sh.setParameter( "LightPos6", pos6 );
		sh.setParameter( "LightColor6", c6.r / 255.0, c6.g / 255.0, c6.b / 255.0, 1 );
		sh.setParameter( "Radius6", owner->touchedLights[0]->radius );
		sh.setParameter( "Brightness6", owner->touchedLights[0]->brightness);
	}
	if( owner->lightsAtOnce > 7 )
	{
		float depth7 = owner->touchedLights[7]->depth;
		Vector2i vi7 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[7]->pos.x, owner->touchedLights[7]->pos.y ) );
		Vector3f pos7( vi7.x / windowx, ( windowy - vi7.y) / windowy, depth7 ); 
		Color c7 = owner->touchedLights[7]->color;
		
		on[7] = true;
		sh.setParameter( "LightPos7", pos7 );
		sh.setParameter( "LightColor7", c7.r / 255.0, c7.g / 255.0, c7.b / 255.0, 1 );
		sh.setParameter( "Radius7", owner->touchedLights[7]->radius );
		sh.setParameter( "Brightness7", owner->touchedLights[7]->brightness);
	}
	if( owner->lightsAtOnce > 8 )
	{
		float depth8 = owner->touchedLights[8]->depth;
		Vector2i vi8 = owner->preScreenTex->mapCoordsToPixel( Vector2f( owner->touchedLights[8]->pos.x, owner->touchedLights[8]->pos.y ) );
		Vector3f pos8( vi8.x / windowx, ( windowy - vi8.y) / windowy, depth8 ); 
		Color c8 = owner->touchedLights[8]->color;
		
		on[8] = true;
		sh.setParameter( "LightPos8", pos8 );
		sh.setParameter( "LightColor8", c8.r / 255.0, c8.g / 255.0, c8.b / 255.0, 1 );
		sh.setParameter( "Radius8", owner->touchedLights[8]->radius );
		sh.setParameter( "Brightness8", owner->touchedLights[8]->brightness);
	}

	sh.setParameter( "On0", on[0] );
	sh.setParameter( "On1", on[1] );
	sh.setParameter( "On2", on[2] );
	sh.setParameter( "On3", on[3] );
	sh.setParameter( "On4", on[4] );
	sh.setParameter( "On5", on[5] );
	sh.setParameter( "On6", on[6] );
	sh.setParameter( "On7", on[7] );
	sh.setParameter( "On8", on[8] );
	
	}

	if( desperationMode )
	{
		//cout << "sending this parameter! "<< endl;
		sh.setParameter( "despFrame", despCounter );
	}
	else
	{
		
		sh.setParameter( "despFrame", -1 );
	}




	if( record > 0 )
	{
		PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
		p.action = (PlayerGhost::Action)action;
		p.frame = frame;
		p.angle = sprite->getRotation() / 180 * PI;
		p.position = position;
		p.s = *sprite;
		//p.position = V2d(sprite->getPosition();
		ghosts[record-1]->currFrame++;
	}

	if( ghostFrame < PlayerGhost::MAX_FRAMES )
		ghostFrame++;
	/*else
	{
		ghosts[record-1]->totalRecorded = ghosts[record-1]->currFrame;
		record = 0;
		LoadState();
		owner->LoadState(); 
		blah = true;
		ghostFrame = 1;
		owner->powerBar.Charge( 20 );
	}*/


	

	if( slowCounter == slowMultiple )
	{
		if( wallJumpFrameCounter < wallJumpMovementLimit )
			wallJumpFrameCounter++;
		framesInAir++;

		if( action == BOUNCEAIR && oldBounceEdge != NULL )
		{
			framesSinceBounce++;
		}

		++frame;

		++framesSinceClimbBoost;

		

		slowCounter = 1;

		if( invincibleFrames > 0 )
			--invincibleFrames;
	}
	else
		slowCounter++;

	if( bounceFlameOn )
	{
		if( ground == NULL )
		{
			airBounceFrame++;
			if( airBounceFrame == airBounceFlameFrames )
			{
				airBounceFrame = 13 * 3;
			}
		}
		else
		{
			runBounceFrame++;
			if( runBounceFrame == runBounceFlameFrames )
			{
				runBounceFrame = 8 * 3;
			}
		}
	}

	UpdateHitboxes();
	//playerLight->pos.x = position.x;
	//playerLight->pos.y = position.y;
}

void Actor::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	assert( queryMode != "" );

	if( queryMode == "moving_resolve" )
	{
		bool bb = false;
		
		if( ground != NULL && groundSpeed != 0 )
		{
			V2d gn = ground->Normal();
			//bb fixes the fact that its easier to hit corners now, so it doesnt happen while you're running
			
			V2d nextn = ground->edge1->Normal();
			V2d prevn = ground->edge0->Normal();
			bool sup = ( groundSpeed < 0 && gn.x > 0 && prevn.x > 0 && prevn.y < 0 );
			//cout << "sup: " << sup << endl;
			bool a = false;
			bool b = false;
			if( !reversed )
			{
				if( groundSpeed > 0 )
				{
					if( ( ground->edge1 == e 
							&& (( gn.x > 0 && nextn.x > 0 && nextn.y < 0 ) || ( gn.x < 0 && nextn.x < 0 && nextn.y < 0 )) )
						|| ground->edge0 == e )
					{
						a = true;
					}
				}
				else if( groundSpeed < 0 )
				{
					if( ( ground->edge0 == e 
							&& ( ( gn.x < 0 && prevn.x < 0 && prevn.y < 0 ) || ( gn.x > 0 && prevn.x > 0 && prevn.y < 0 ) ) ) 
						|| ground->edge1 == e )
					{
						a = true;
					}
				}
			}
			else
			{
				if( groundSpeed > 0 )
				{
					if( ( ground->edge0 == e 
						&& ( ( gn.x < 0 && prevn.x < 0 && prevn.y > 0 ) || ( gn.x > 0 && prevn.x > 0 && prevn.y > 0 ) ) ) 
						|| ground->edge1 == e )
					{
						//cout << "one" << endl;
						b = true;
					}
				}
				else if( groundSpeed < 0 )
				{
					bool c = ground->edge1 == e;
					bool h = ( gn.x > 0 && nextn.x > 0 && nextn.y > 0 );
					bool g = ( gn.x < 0 && nextn.x < 0 && nextn.y > 0 );
					bool d = h || g;
					bool f = ground->edge0 == e;
					if( (c && d) || f )
					{
						b = true;
					}
				}
			}
		//	a = false;
		//	b = false;
			
			
			//a = !reversed && ((groundSpeed > 0 && gn.x < 0 && nextn.x < 0 && nextn.y < 0) || ( groundSpeed < 0 && gn.x > 0 && prevn.x > 0 && prevn.y < 0 )
			//	|| ( groundSpeed > 0 && gn.x > 0 && nextn.x > 0 && prevn.y < 0 ) || ( groundSpeed < 0 && gn.x < 0 && prevn.x < 0 && prevn.y < 0 ));
			//bool b = reversed && (( gn.x < 0 && nextn.x < 0 || ( gn.x > 0 && prevn.x > 0 )));
			bb = ( a || b );
		}

		
		if( e == ground || bb )
		{
			return;
		}

		V2d temp0 = e->v0;
		V2d temp1 = e->v1;
		
		e->v0 += currMovingTerrain->oldPosition;
		e->v1 += currMovingTerrain->oldPosition;

		//e->v0 += currMovingTerrain->position;
		//e->v1 += currMovingTerrain->position;

		if( e->Normal().y == -1 )
		{
			//cout << "testing the ground!: " << e->v0.x << ", " << e->v0.y << " and " <<
			//	e->v1.x << ", " << e->v1.y << endl;
		
		}
		V2d blah( tempVel - currMovingTerrain->vel );
		//cout << "tempnew: " << blah.x << ", " << blah.y << endl;
		Contact *c = owner->coll.collideEdge( position + b.offset - currMovingTerrain->vel, b, e, tempVel - currMovingTerrain->vel, V2d( 0, 0 ) );//currMovingTerrain->vel );

		e->v0 = temp0;
		e->v1 = temp1;

		//if( c != NULL )
		//{
		//	cout << "moving resolve!: " << e->Normal().x << ", " << e->Normal().y << endl;
		//}

		if( c != NULL )	//	|| minContact.collisionPriority < -.001 && c->collisionPriority >= 0 )
		{	
			/*if( !col || (c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
				if( c->collisionPriority == minContact.collisionPriority )
				{
					if( length(c->resolution) > length(minContact.resolution) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.movingPlat = currMovingTerrain;
						col = true;

					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.movingPlat = currMovingTerrain;
					col = true;
				}
			}*/

			if( ( c->normal.x == 0 && c->normal.y == 0 ) ) //non point
			{
				cout << "SURFACE. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << ", pri: " << c->collisionPriority << endl;
			}
			else //point
			{
				cout << "POINT. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << endl;
			}

			if( c->weirdPoint )
			{
		//		cout << "weird point " << endl;
				
				Edge *edge = e;
				Edge *prev = edge->edge0;
				Edge *next = edge->edge1;

				V2d v0 = edge->v0;
				V2d v1 = edge->v1;				

				if( approxEquals( c->position.x, e->v0.x ) && approxEquals( c->position.y, e->v0.y ) )
				{
					V2d pv0 = prev->v0;
					V2d pv1 = prev->v1;

					V2d pn = prev->Normal();
					V2d en = e->Normal();
					

					if( ground == NULL && pn.y >= 0 && en.y < 0 )
					{
						//cout << "bhaehfdf" << endl; //falling off and you dont want to keep hitting the ground
						assert( !reversed );
						return;
					}

					//ground != NULL
					if( pn.y < en.y )
					{
						//this could cause some glitches. patch them up as they come. prioritizes ground/higher up edges i think? kinda weird
						//cout << "sfdfdsfsdfdsfds" << endl;
						c->edge = prev;
						return;

						//c->normal = V2d( 0, -1 );
					}
				}
				else if( approxEquals( c->position.x, e->v1.x ) && approxEquals( c->position.y, e->v1.y ) )
				{
					V2d nn = next->Normal();
					V2d en = e->Normal();
					if( ground == NULL && en.y < 0 && nn.y >= 0 )
					{
						//cout << "bhaehfdf" << endl;
						//falling off and you dont want to keep hitting the ground
						assert( !reversed );
						return;
					}

					//ground != NULL
					if( nn.y < en.y )
					{
						//this could cause some glitches. patch them up as they come. prioritizes ground/higher up edges i think? kinda weird
						//cout << "herererere" << endl;
					//	return;
						c->edge = next;
						return;
						//c->normal = V2d( 0, -1 );
					}
				}
			}



			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
				

				if( c->collisionPriority == minContact.collisionPriority )
				{
					if(( c->normal.x == 0 && c->normal.y == 0 ) )//|| minContact.normal.y  0 )
					//if( length(c->resolution) > length(minContact.resolution) )
					{
					//	cout << "now the min" << endl;
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.normal = c->normal;
						minContact.movingPlat = currMovingTerrain;
						col = true;
					}
					else
					{
						//cout << "happens here!!!!" << endl;
					}
				}
				else
				{
					//cout << "now the min" << endl;
					//if( minContact.edge != NULL )
					//cout << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << "... " 
					//	<< e->Normal().x << ", " << e->Normal().y << endl;
					minContact.collisionPriority = c->collisionPriority;
					//cout << "pri: " << c->collisionPriority << endl;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.normal = c->normal;
					minContact.movingPlat = currMovingTerrain;
					col = true;
					
				}
			}
		}
	}
	if( queryMode == "resolve" )
	{

		bool bb = false;
		
		if( ground != NULL && groundSpeed != 0 )
		{
			V2d gn = ground->Normal();
			//bb fixes the fact that its easier to hit corners now, so it doesnt happen while you're running
			
			V2d nextn = ground->edge1->Normal();
			V2d prevn = ground->edge0->Normal();
			bool sup = ( groundSpeed < 0 && gn.x > 0 && prevn.x > 0 && prevn.y < 0 );
			//cout << "sup: " << sup << endl;
			bool a = false;
			bool b = false;
			if( !reversed )
			{
				if( groundSpeed > 0 )
				{
					if( ( ground->edge1 == e 
							&& (( gn.x > 0 && nextn.x > 0 && nextn.y < 0 ) || ( gn.x < 0 && nextn.x < 0 && nextn.y < 0 )) )
						|| ground->edge0 == e )
					{
						a = true;
					}
				}
				else if( groundSpeed < 0 )
				{
					if( ( ground->edge0 == e 
							&& ( ( gn.x < 0 && prevn.x < 0 && prevn.y < 0 ) || ( gn.x > 0 && prevn.x > 0 && prevn.y < 0 ) ) ) 
						|| ground->edge1 == e )
					{
						a = true;
					}
				}
			}
			else
			{
				if( groundSpeed > 0 )
				{
					if( ( ground->edge0 == e 
						&& ( ( gn.x < 0 && prevn.x < 0 && prevn.y > 0 ) || ( gn.x > 0 && prevn.x > 0 && prevn.y > 0 ) ) ) 
						|| ground->edge1 == e )
					{
						//cout << "one" << endl;
						b = true;
					}
				}
				else if( groundSpeed < 0 )
				{
					bool c = ground->edge1 == e;
					bool h = ( gn.x > 0 && nextn.x > 0 && nextn.y > 0 );
					bool g = ( gn.x < 0 && nextn.x < 0 && nextn.y > 0 );
					bool d = h || g;
					bool f = ground->edge0 == e;
					if( (c && d) || f )
					{
						b = true;
					}
				}
			}
		//	a = false;
		//	b = false;
			
			
			//a = !reversed && ((groundSpeed > 0 && gn.x < 0 && nextn.x < 0 && nextn.y < 0) || ( groundSpeed < 0 && gn.x > 0 && prevn.x > 0 && prevn.y < 0 )
			//	|| ( groundSpeed > 0 && gn.x > 0 && nextn.x > 0 && prevn.y < 0 ) || ( groundSpeed < 0 && gn.x < 0 && prevn.x < 0 && prevn.y < 0 ));
			//bool b = reversed && (( gn.x < 0 && nextn.x < 0 || ( gn.x > 0 && prevn.x > 0 )));
			bb = ( a || b );
		}

		
		if( e == ground || bb )
		{
			return;
		}


		

		Contact *c = owner->coll.collideEdge( position + b.offset , b, e, tempVel, V2d( 0, 0 ) );
		
		
		
		

		if( c != NULL )	//	|| minContact.collisionPriority < -.001 && c->collisionPriority >= 0 )
		{
			if( c->edge->edgeType == Edge::GATE )
			{
				cout << "GATEEEEee" << endl;
			}

			if( ( c->normal.x == 0 && c->normal.y == 0 ) ) //non point
			{
			//	cout << "SURFACE. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << ", pri: " << c->collisionPriority << endl;
			}
			else //point
			{
			//	cout << "POINT. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << endl;
			}

			if( c->weirdPoint )
			{
		//		cout << "weird point " << endl;
				
				Edge *edge = e;
				Edge *prev = edge->edge0;
				Edge *next = edge->edge1;

				V2d v0 = edge->v0;
				V2d v1 = edge->v1;				

				if( approxEquals( c->position.x, e->v0.x ) && approxEquals( c->position.y, e->v0.y ) )
				{
					V2d pv0 = prev->v0;
					V2d pv1 = prev->v1;

					V2d pn = prev->Normal();
					V2d en = e->Normal();
					

					if( ground == NULL && pn.y >= 0 && en.y < 0 )
					{
						//cout << "bhaehfdf" << endl; //falling off and you dont want to keep hitting the ground
						assert( !reversed );
						return;
					}

					//ground != NULL
					if( pn.y < en.y )
					{
						//this could cause some glitches. patch them up as they come. prioritizes ground/higher up edges i think? kinda weird
						//cout << "sfdfdsfsdfdsfds" << endl;
						c->edge = prev;
						return;

						//c->normal = V2d( 0, -1 );
					}
				}
				else if( approxEquals( c->position.x, e->v1.x ) && approxEquals( c->position.y, e->v1.y ) )
				{
					V2d nn = next->Normal();
					V2d en = e->Normal();
					if( ground == NULL && en.y < 0 && nn.y >= 0 )
					{
						//cout << "bhaehfdf" << endl;
						//falling off and you dont want to keep hitting the ground
						assert( !reversed );
						return;
					}

					//ground != NULL
					if( nn.y < en.y )
					{
						//this could cause some glitches. patch them up as they come. prioritizes ground/higher up edges i think? kinda weird
						//cout << "herererere" << endl;
					//	return;
						c->edge = next;
						return;
						//c->normal = V2d( 0, -1 );
					}
				}
			}

			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) )
			{	
				

				if( c->collisionPriority == minContact.collisionPriority )
				{
					if(( c->normal.x == 0 && c->normal.y == 0 ) )//|| minContact.normal.y  0 )
					//if( length(c->resolution) > length(minContact.resolution) )
					{
					//	cout << "now the min" << endl;
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.normal = c->normal;
						minContact.movingPlat = NULL;
						col = true;
					}
					else
					{
						//cout << "happens here!!!!" << endl;
					}
				}
				else
				{
					//cout << "now the min" << endl;
					//if( minContact.edge != NULL )
					//cout << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << "... " 
					//	<< e->Normal().x << ", " << e->Normal().y << endl;
					minContact.collisionPriority = c->collisionPriority;
					//cout << "pri: " << c->collisionPriority << endl;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.normal = c->normal;
					minContact.movingPlat = NULL;
					col = true;
					
				}
			}
		}
		
	}
	else if( queryMode == "check" )
	{
		//if( ground == e )
		//	return;

		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight );
		//Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		if ( action != GRINDBALL )
		{
			if( ( e->Normal().y <= 0 && !reversed && ground != NULL ) || ( e->Normal().y >= 0 && reversed && ground != NULL ) )
			{
				return;
			}
		}
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		//if( IsEdgeTouchingBox( e, r ) )
		//{
			checkValid = false;

		//}
	}
	else if( queryMode == "moving_check" )
	{
		if ( action != GRINDBALL )
		{
			if( ( e->Normal().y <= 0 && !reversed && ground != NULL ) || ( e->Normal().y >= 0 && reversed && ground != NULL ) )
			{
				return;
			}
		}
		
		checkValid = false;
	}
	else if( queryMode == "checkwall" )
	{
		if( ground == e )
			return;
		Contact *c = owner->coll.collideEdge( position + tempVel , b, e, tempVel, V2d( 0, 0 ) );
		
		if( c != NULL )
			if( !col || (c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
				if( c->collisionPriority == minContact.collisionPriority )
				{
					if( length(c->resolution) > length(minContact.resolution) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.movingPlat = NULL;
						col = true;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.movingPlat = NULL;
					col = true;
					
				}
			}
	}
	else if( queryMode == "moving_checkwall" )
	{
		if( e == ground )
			return;

		V2d temp0 = e->v0;
		V2d temp1 = e->v1;

		e->v0 += currMovingTerrain->position;
		e->v1 += currMovingTerrain->position;

		//e->v0 += currMovingTerrain->position;
		//e->v1 += currMovingTerrain->position;

		if( e->Normal().y == -1 )
		{
			cout << "testing the ground!: " << e->v0.x << ", " << e->v0.y << " and " <<
				e->v1.x << ", " << e->v1.y << endl;
		}

		Contact *c = owner->coll.collideEdge( position + b.offset, b, e, tempVel, V2d( 0, 0 ) );

		e->v0 = temp0;
		e->v1 = temp1;
		
		if( c != NULL )
			if( !col || (c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
				if( c->collisionPriority == minContact.collisionPriority )
				{
					if( length(c->resolution) > length(minContact.resolution) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.movingPlat = currMovingTerrain;
						col = true;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.movingPlat = currMovingTerrain;
					col = true;
					
				}
			}
	}
	else if( queryMode == "lights" )
	{
		Light *light = (Light*)qte;

		//if( light == playerLight )
		//	return;

		if( owner->lightsAtOnce < owner->tempLightLimit )
		{
			owner->touchedLights[owner->lightsAtOnce] = light;
			owner->lightsAtOnce++;
		}
		else
		{
			for( int i = 0; i < owner->lightsAtOnce; ++i )
			{
				if( length( V2d( owner->touchedLights[i]->pos.x, owner->touchedLights[i]->pos.y ) - position ) > length( V2d( light->pos.x, light->pos.y ) - position ) )//some calculation here
				{
					owner->touchedLights[i] = light;
					break;
				}
					
			}
		}
	}
	else if( queryMode == "grass" )
	{
		//cout << "got some grass in here" << endl;
		Grass *g = (Grass*)qte;
		Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
		if( g->IsTouchingBox( r ) )
		{
		//	sf::VertexArray va( sf::Quads, 4 );
			/*va[0].position = Vector2f( g->A.x, g->A.y );
			va[1].position = Vector2f( g->B.x, g->B.y );
			va[2].position = Vector2f( g->C.x, g->C.y );
			va[3].position = Vector2f( g->D.x, g->D.y );*/
			

			/*va[0].color = Color::Red;
			va[1].color = Color::Red;
			va[2].color = Color::Red;
			va[3].color = Color::Red;

			owner->preScreenTex->draw( va );

			CircleShape cs;
			cs.setFillColor( Color::Green );
			cs.setRadius( 10000 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( 0, 0 );//g->A.x, g->A.y );
			cout << "pos: " << g->A.x << ", " << g->A.y << ", playerpos: " << position.x << ", " << position.y << endl;
			
			owner->preScreenTex->draw( cs );*/

			//owner->preScreenTex->draw( va );

			++testGrassCount;
		}
	}
	else if( queryMode == "gate" )
	{
		Gate *g = (Gate*)qte;
		
		if( action == GRINDBALL )
		{
			if( g->locked && ( g->edgeA->v0 == grindEdge->v0 || g->edgeA->v1 == grindEdge->v0 ) )
			{
				if( g->type == Gate::RED && hasRedKey )
				{
					g->locked = false;
					hasRedKey = false;
				}
				else if( g->type == Gate::GREEN && hasGreenKey )
				{
					g->locked = false;
					hasGreenKey = false;
				}
				else if( g->type == Gate::BLUE && hasBlueKey )
				{
					g->locked = false;
					hasBlueKey = false;
				}
				else
				{
					++owner->testGateCount;
				}
			}
		}
		else
		{
			Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );

			if( g->locked && g->edgeA->IsTouchingBox( r ) )
			{
				if( g->type == Gate::RED && hasRedKey )
				{
					g->locked = false;
					hasRedKey = false;
				}
				else if( g->type == Gate::GREEN && hasGreenKey )
				{
					g->locked = false;
					hasGreenKey = false;
				}
				else if( g->type == Gate::BLUE && hasBlueKey )
				{
					g->locked = false;
					hasBlueKey = false;
				}
				else
				{
					++owner->testGateCount;
				}
			}
		}
	}
	
	++possibleEdgeCount;
}

void Actor::ApplyHit( HitboxInfo *info )
{
	//use the first hit you got. no stacking hits for now
	if( invincibleFrames == 0 )
	{
		if( receivedHit == NULL || info->damage > receivedHit->damage )
		{
			receivedHit = info;
		}
	}
}

void Actor::Draw( sf::RenderTarget *target )
{
	if( bounceFlameOn && action != DEATH )
	{
		target->draw( bounceFlameSprite );
	}


	int showMotionGhosts = 0;
	if( ground != NULL )
	{
		double gs = abs( groundSpeed );
		if( gs >= dashSpeed )
		{
			showMotionGhosts = 1;
		}
		else if( gs >= dashSpeed * 1.5 )
		{
			showMotionGhosts = 2;
		}
	}
	else
	{
		double len = length( velocity );
		if( len >= dashSpeed )
		{
			showMotionGhosts = 2;
		}
		else
		{
			showMotionGhosts = 1;
		}
	}

	if( showMotionGhosts )
	{
		for( int i = 0; i < showMotionGhosts; ++i )
		{
		
			motionGhosts[i].setColor( Color( 50, 50, 255, 50 ) );
			target->draw( motionGhosts[i] );
		}
	}

	if( action != GRINDBALL )
	{

		//RayCast( this, owner->testTree, position, V2d( position.x - 100, position.y ) );
		
		

		Vector2i vi = Mouse::getPosition();
		//Vector2i vi = owner->window->mapCoordsToPixel( Vector2f( position.x, position.y ) );//sf::Vector2f( 0, -300 ) );

		Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
		//Vector3f blahblah( vi.x / (float)owner->window->getSize().x, 
		//	(owner->window->getSize().y - vi.y) / (float)owner->window->getSize().x, .015 );

		
		
		if( action != DEATH )
		//if( action == RUN )
		{
			//sh.setParameter( "u_texture",( *owner->GetTileset( "run.png" , 128, 64 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			//sh.setParameter( "u_normals", *owner->GetTileset( "run_normal.png", 128, 64 )->texture );
			/*Sprite spr;
			
			spr.setTexture( *owner->GetTileset( "testrocks.png", 300, 225)->texture );
			spr.setOrigin( spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2 );
			if( !facingRight )
			{
				sf::IntRect r = spr.getTextureRect();
				spr.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
			}
			spr.setPosition( sprite->getPosition() );
			
			// global positions first. then zooming

			
			sh.setParameter( "u_texture",( *owner->GetTileset( "testrocks.png" , 300, 225 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			sh.setParameter( "u_normals", *owner->GetTileset( "testrocksnormal.png", 300, 225 )->texture );
			sh.setParameter( "Resolution", owner->window->getSize().x, owner->window->getSize().y );
			//sh.setParameter( "LightPos", blahblah );//Vector3f( 0, -300, .075 ) );
			//sh.setParameter( "LightColor", 1, .8, .6, 1 );
			sh.setParameter( "AmbientColor", .6, .6, 1, .8 );
			//sh.setParameter( "Falloff", Vector3f( .4, 3, 20 ) );
			sh.setParameter( "right", (facingRight && !reversed) || (facingRight && reversed ) );
			sh.setParameter( "zoom", owner->cam.GetZoom() );
			//cout << "right: " << (float)facingRight << endl;

			CircleShape cs;
			cs.setFillColor( Color::Magenta );
			cs.setRadius( 30 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( 0, -300 );*/
			

			//target->draw( spr, &sh );






			//sh.setParameter( "u_texture",( *owner->GetTileset( "run2.png" , 80, 48 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			//sh.setParameter( "u_normals", *owner->GetTileset( "run_NORMALS.png", 80, 48 )->texture );


			//sh.setParameter( "u_texture",( *owner->GetTileset( "run.png" , 128, 64 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			//sh.setParameter( "u_normals", *owner->GetTileset( "run_normal.png", 128, 64 )->texture );

			sh.setParameter( "u_texture", *tileset[action]->texture ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			sh.setParameter( "u_normals", *normal[action]->texture );
			//sprite->setScale( 2, 2 );

			target->draw( *sprite, &sh );
			//target->draw( cs );
		}
		else
		{
			target->draw( *sprite );
		}
		

		if( showSword1 )
		{
			switch( action )
			{
			case FAIR:
				{
					target->draw( fairSword1 );
					break;
				}
			case DAIR:
				{
					target->draw( dairSword1 );
					break;
				}
			case UAIR:
				{
					target->draw( uairSword1 );
					break;
				}
			case STANDN:
				{
				/*	sf::RectangleShape rs;
					rs.setSize( Vector2f( abs(standingNSword1.getTextureRect().width), 
						abs(standingNSword1.getTextureRect().height )) );
					rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
					//rs.setRotation( standingNSword1.getRotation() );
					rs.setPosition( standingNSword1.getPosition() );
					rs.setFillColor( Color::Red );
					target->draw( rs );*/
					target->draw( standingNSword1 );
					break;
				}
			case STANDD:
				{
					target->draw( standingDSword1 );
					break;
				}
			case STANDU:
				{
					target->draw( standingUSword1 );
					break;
				}
			}
		}
	}
	else
	{
		target->draw( *sprite );
		target->draw( gsdodeca );
		target->draw( gstriblue );
		target->draw( gstricym );
		target->draw( gstrigreen );
		target->draw( gstrioran );
		target->draw( gstripurp );
		target->draw( gstrirgb );
	}

	if( blah || record > 1 )
	{
		int playback = recordedGhosts;
		if( record > 1 )
			playback--;
			
		for( int i = 0; i < playback; ++i )
		{
			PlayerGhost *g = ghosts[i];
			if( ghostFrame-1 < g->totalRecorded )
			{
				target->draw( g->states[ghostFrame-1].s );
				if( g->states[ghostFrame-1].showSword1 )
					target->draw( g->states[ghostFrame-1].swordSprite1 );
			}
			
		}

		
		//PlayerGhost *g = ghosts[record-1];
		
	}

	for( int i = 0; i < maxBubbles; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			bubbleSprite.setTextureRect( ts_bubble->GetSubRect( bubbleFramesToLive[i] % 11 ) );
			bubbleSprite.setScale( 2, 2 );
			bubbleSprite.setOrigin( bubbleSprite.getLocalBounds().width / 2, bubbleSprite.getLocalBounds().height / 2 );
			bubbleSprite.setPosition( bubblePos[i].x, bubblePos[i].y );
			bubbleSprite.setColor( Color( 255, 255, 255, 200 ) );
			//CircleShape cs;
		//	cs.setFillColor( sf::Color::Transparent );
			//cs.setRadius( 

			//target->draw( bubbleSprite );// &timeSlowShader );
		}
	}
}

void Actor::DodecaLateDraw(sf::RenderTarget *target)
{
	sf::Sprite dodecaSprite;
	int dodecaFactor = 1;
	for( int i = 0; i < maxBubbles; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			int trueFrame = bubbleLifeSpan - ( bubbleFramesToLive[i] );
			//cout << "trueFrame: " << trueFrame << endl;
			if( trueFrame / dodecaFactor < 9 )
			{
				dodecaSprite.setTexture( *ts_dodecaSmall->texture );
				dodecaSprite.setTextureRect( ts_dodecaSmall->GetSubRect( trueFrame / dodecaFactor ) );
				dodecaSprite.setOrigin( dodecaSprite.getLocalBounds().width / 2, dodecaSprite.getLocalBounds().height / 2 );
			}
			else
			{
				dodecaSprite.setTexture( *ts_dodecaBig->texture );
				dodecaSprite.setTextureRect( ts_dodecaBig->GetSubRect( trueFrame / dodecaFactor - 9 ) );
				dodecaSprite.setOrigin( dodecaSprite.getLocalBounds().width / 2, dodecaSprite.getLocalBounds().height / 2 );
			}
			dodecaSprite.setPosition( bubblePos[i].x, bubblePos[i].y );

			target->draw( dodecaSprite );
		}
	}
}

void Actor::DebugDraw( RenderTarget *target )
{
	if( currHitboxes != NULL )
	{
		for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
		{
			(*it).DebugDraw( target );
		}
	}

	sf::CircleShape cs;
	cs.setOutlineThickness( 10 );
	cs.setOutlineColor( Color::Red );
	cs.setFillColor( Color::Transparent );
	cs.setRadius( 160 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	//target->draw( cs );
	

	hurtBody.DebugDraw( target );
	b.DebugDraw( target );

	for( int i = 0; i < recordedGhosts; ++i )
	{
		ghosts[i]->DebugDraw( target );
	}

	/*if( blah )
	{
		for( int i = 0; i < recordedGhosts; ++i )
		{
			if( ghostFrame < ghosts[i]->totalRecorded )
			{
				sf::Rect<double> rd = ghosts[i]->states[ghostFrame].screenRect;
				sf::RectangleShape rs;
				rs.setPosition( rd.left, rd.top );
				rs.setSize( sf::Vector2f( rd.width, rd.height ) );
				rs.setFillColor( Color::Transparent );
				rs.setOutlineColor( Color::Red );
				rs.setOutlineThickness( 10 );
				target->draw( rs );
			}
				//ghosts[i]->UpdatePrePhysics( ghostFrame );
		}
		//testGhost->UpdatePrePhysics( ghostFrame );
	}*/

	leftWire->DebugDraw( target );
	rightWire->DebugDraw( target );

}

void Actor::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{

	/*if( rayPortion > 1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - position ) < length( rcEdge->GetPoint( rcQuantity ) - position ) ) )
	{
		rcEdge = edge;
		rcQuantity = edgeQuantity;
	}*/
}

double Actor::GroundedAngle()
{
	if( ground == NULL )
	{
		//cout << "ground is null" << endl;
		return 0;
	}
	
	V2d gn = ground->Normal();
	//cout << "gn: " << gn.x << ", " << gn.y << endl;

	double angle = 0;
	
	bool extraCase;
	if( !reversed )
	{
		V2d e0n = ground->edge0->Normal();
		V2d e1n = ground->edge1->Normal();
		bool a = ( offsetX > 0 && approxEquals( edgeQuantity, 0 ) && e0n.x < 0 );
		bool b =( offsetX < 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) && e1n.x > 0 );
		extraCase = a || b;
		//cout << "extra: " << extraCase << endl;
	}
	else
	{
		V2d e0n = ground->edge0->Normal();
		V2d e1n = ground->edge1->Normal();
		bool a = ( offsetX > 0 && approxEquals( edgeQuantity, 0 ) && e0n.x < 0 );
		bool b = ( offsetX < 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) && e1n.x > 0 );
		extraCase = a || b;
		//cout << "extraCSe : " << a <<", " << b << ", edge: " << edgeQuantity << ", " << length( ground->v1 - ground->v0 )
		//	 << ", " << approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) << endl;
	}
	//bool extraCaseRev = reversed && (( offsetX > 0 && approxEquals( edgeQuantity, 0 ) )
	//	|| ( offsetX < 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) ) );
	//cout << "offsetX: " << offsetX << ", b.rw: " << b.rw << endl;

	//approxequals is broken????????

	//note: approxequals is broken??
	bool okayOffset = abs( abs(offsetX) - b.rw ) < .001;
	if( !okayOffset || extraCase )
	{
		//cout << "bad offset: " << offsetX << endl;
		if( reversed )
			angle = PI;
	}
	else
	{
		angle = atan2( gn.x, -gn.y );
	}

	return angle;
}

double Actor::GroundedAngleAttack( sf::Vector2<double> &trueNormal )
{
	assert( ground != NULL );
	V2d gn = ground->Normal();

	double angle = 0;

	bool extraCase;
	if( !reversed )
	{
		extraCase = ( offsetX < 0 && approxEquals( edgeQuantity, 0 ) )
		|| ( offsetX > 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) );
	}
	else
	{
		extraCase = ( offsetX > 0 && approxEquals( edgeQuantity, 0 ) )
		|| ( offsetX < 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) );
	}

	if( !approxEquals( abs(offsetX), b.rw ) || extraCase )
	{
		trueNormal = V2d( 0, -1 );
		if( reversed )
		{
			angle = PI;
			trueNormal = V2d( 0, 1 );
		}
	}
	else
	{
		angle = atan2( gn.x, -gn.y );
		trueNormal = gn;
	}

	return angle;
}

void Actor::SaveState()
{
	stored.leftGround = leftGround;
	
	stored.grindActionCurrent = grindActionCurrent;
	stored.prevInput = prevInput;
	stored.currInput = currInput;
	stored.oldVelocity = oldVelocity;
	stored.framesInAir = framesInAir;
	stored.startAirDashVel = startAirDashVel;
	stored.ground = ground;
	stored.hasAirDash = hasAirDash;
	stored.hasGravReverse = hasGravReverse;

	stored.grindEdge = grindEdge;
	stored.grindQuantity = grindQuantity;
	stored.grindSpeed = grindSpeed;

	stored.reversed = reversed;

	stored.edgeQuantity = edgeQuantity;
	
	stored.groundOffsetX = groundOffsetX;

	stored.offsetX = offsetX;

	stored.holdJump = holdJump;

	stored.wallJumpFrameCounter = wallJumpFrameCounter;

	stored.groundSpeed = groundSpeed;

	stored.facingRight = facingRight;
	
	stored.hasDoubleJump = hasDoubleJump;

	stored.slowMultiple = slowMultiple;
	stored.slowCounter = slowCounter;

	stored.wallNormal = wallNormal;

	stored.action = action;
	stored.frame = frame;
	stored.position = position;
	stored.velocity = velocity;
	//CollisionBox *physBox;

	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.invincibleFrames = invincibleFrames;
	stored.receivedHit = receivedHit;

	stored.storedBounceVel = storedBounceVel;
	//stored.leftWire = leftWire;
	stored.bounceEdge = bounceEdge;
	stored.bounceQuant = bounceQuant;

	stored.oldBounceEdge = oldBounceEdge;
	stored.framesSinceBounce = framesSinceBounce;

//	stored.touchEdgeWithWire = touchEdgeWithWire;

	for( int i = 0; i < maxBubbles; ++i )
	{
		stored.bubblePos[i] = bubblePos[i];
		stored.bubbleFramesToLive[i] = bubbleFramesToLive[i];
	}
	stored.currBubble = currBubble;

	stored.bounceNorm = bounceNorm;
	stored.oldBounceNorm = oldBounceNorm;
	stored.groundedWallBounce = groundedWallBounce;
	
	stored.framesGrinding = framesGrinding;
}

void Actor::LoadState()
{
	stored.leftGround;
	
	grindActionCurrent= stored.grindActionCurrent;
	prevInput = stored.prevInput;
	currInput = stored.currInput;
	oldVelocity = stored.oldVelocity;
	framesInAir = stored.framesInAir;
	startAirDashVel = stored.startAirDashVel;
	ground = stored.ground;
	hasAirDash = stored.hasAirDash;
	hasGravReverse = stored.hasGravReverse;

	grindEdge = stored.grindEdge;
	grindQuantity = stored.grindQuantity;
	grindSpeed = stored.grindSpeed;

	reversed = stored.reversed;

	edgeQuantity = stored.edgeQuantity;
	
	groundOffsetX = stored.groundOffsetX;

	offsetX = stored.offsetX;

	holdJump = stored.holdJump;

	wallJumpFrameCounter = stored.wallJumpFrameCounter;

	groundSpeed = stored.groundSpeed;

	facingRight = stored.facingRight;
	
	hasDoubleJump = stored.hasDoubleJump;

	slowMultiple = stored.slowMultiple;
	slowCounter = stored.slowCounter;

	wallNormal = stored.wallNormal;

	action = stored.action;
	frame = stored.frame;
	position = stored.position;
	velocity = stored.velocity;
	//CollisionBox *physBox;

	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	invincibleFrames = stored.invincibleFrames;
	receivedHit = stored.receivedHit;

	storedBounceVel = stored.storedBounceVel;
	//wire = stored.wire;
	bounceEdge = stored.bounceEdge;
	bounceQuant = stored.bounceQuant;

	oldBounceEdge = stored.oldBounceEdge;
	framesSinceBounce = stored.framesSinceBounce;

	//touchEdgeWithWire = stored.touchEdgeWithWire;

	for( int i = 0; i < maxBubbles; ++i )
	{
		bubblePos[i] = stored.bubblePos[i];
		bubbleFramesToLive[i] = stored.bubbleFramesToLive[i];
	}
	currBubble = stored.currBubble;

	bounceNorm = stored.bounceNorm;
	oldBounceNorm = stored.oldBounceNorm;
	groundedWallBounce = stored.groundedWallBounce;

	framesGrinding = stored.framesGrinding;
}

void Actor::AirMovement()
{
	if( leftWire->state == Wire::PULLING || rightWire->state == Wire::PULLING )
	{
	}
	else
	{
		if( currInput.LLeft() )
		{
			if( velocity.x > dashSpeed )
			{
				velocity.x -= airAccel;
			}
			else if( velocity.x > -maxAirXControl )
			{
				velocity.x = -maxAirXControl;
			}
		}
		else if( currInput.LRight() )
		{
			if( velocity.x < -dashSpeed )
			{
				velocity.x += airAccel;
			}
			else if( velocity.x < maxAirXControl )
			{
				velocity.x = maxAirXControl;
			}
		}
		else if( !currInput.LUp() && !currInput.LDown() )
		{
			if( velocity.x > dashSpeed )
			{
				velocity.x -= airSlow;
				if( velocity.x < dashSpeed ) 
				{
					velocity.x = dashSpeed;
				}
			}
			else if( velocity.x > 0 )
			{
				velocity.x = 0;
			}
			else if( velocity.x < -dashSpeed )
			{
				velocity.x += airSlow;
				if( velocity.x > -dashSpeed ) 
				{
					velocity.x = -dashSpeed;
				}
			}
			else if( velocity.x < 0 )
			{
				velocity.x += airSlow;
				if( velocity.x > 0 ) velocity.x = 0;
				else if( velocity.x >= -dashSpeed )
				{
					velocity.x = 0;
				}
			}
		}
	}
}

Vector2i Actor::GetWireOffset()
{
	Vector2i offset;
	switch( action )
	{
	case DASH:
	case DOUBLE:
		offset = Vector2i( 0, 0 );
		break;
	case STEEPSLIDE:
		//cout << "steep slide" << endl;
		offset = Vector2i( 0, 0 );
		break;
	case SPRINT:
		offset = Vector2i( 0, 0 );
		break;
	case LAND:
	case LAND2:
		offset = Vector2i( 0, 0 );
		break;
	default:
		offset = Vector2i( 0, 4.9 );
	}

	if( reversed )
	{
		//offset.y = -offset.y;
	}

	//if( b.rh < normalHeight )
	/*{
		if( ground != NULL )
		{
			offset = Vector2i( 0, 5 );
		}
		else
		{
			offset = Vector2i( 0, 0 );
		}
		
	}*/

	return offset;
}

void Actor::RunMovement()
{
	if( !facingRight )
	{
		if( groundSpeed > 0 )
		{
			groundSpeed = 0;
		}
		else
		{
			if( groundSpeed > -maxRunInit )
			{
				groundSpeed -= runAccelInit / slowMultiple;
				if( groundSpeed < -maxRunInit )
					groundSpeed = -maxRunInit;
			}
			else
			{
				groundSpeed -= runAccel / slowMultiple;
			}
				
		}
		
		if( bounceFlameOn )
		{
			groundSpeed -= bounceFlameAccel / slowMultiple;
		}
		else if( currInput.B )
		{
			groundSpeed -= holdDashAccel / slowMultiple;
		}

		


		facingRight = false;
	}
	else
	{
		if (groundSpeed < 0 )
			groundSpeed = 0;
		else
		{
			if( groundSpeed < maxRunInit )
			{
				groundSpeed += runAccelInit / slowMultiple;
				if( groundSpeed > maxRunInit )
					groundSpeed = maxRunInit;
			}
			else
			{
				groundSpeed += runAccel / slowMultiple;
			}
		}

		if( bounceFlameOn )
		{
			groundSpeed += bounceFlameAccel / slowMultiple;
		}
		else if( currInput.B )
		{
			groundSpeed += holdDashAccel / slowMultiple;
		}

		

		facingRight = true;
	}
}

void Actor::AttackMovement()
{
	if( currInput.LLeft() )
	{
		if( groundSpeed > 0 )
		{
			if( currInput.B )
			{
				groundSpeed = -dashSpeed;
			}
			else
			{
				groundSpeed = 0;
			}
		}
		else
		{
			if( groundSpeed > -dashSpeed && currInput.B )
			{
				groundSpeed = -dashSpeed;
			}
			else if( groundSpeed > -maxRunInit )
			{
				groundSpeed -= runAccelInit / slowMultiple;
				if( groundSpeed < -maxRunInit )
					groundSpeed = -maxRunInit;
			}
			else
			{
				groundSpeed -= runAccel / slowMultiple;
			}
				
		}
	}
	else if( currInput.LRight() )
	{
		if (groundSpeed < 0 )
		{
			if( currInput.B )
			{
				groundSpeed = dashSpeed;
			}
			else
			{
				groundSpeed = 0;
			}
		}
		else
		{
			if( groundSpeed < dashSpeed && currInput.B )
			{
				groundSpeed = dashSpeed;
			}
			else if( groundSpeed < maxRunInit )
			{
				groundSpeed += runAccelInit / slowMultiple;
				if( groundSpeed > maxRunInit )
					groundSpeed = maxRunInit;
			}
			else
			{
				groundSpeed += runAccel / slowMultiple;
			}
		}
	}
	else
	{
		groundSpeed = 0;
	}
}

void Actor::SetActionGrind()
{
	//double gSpeed = groundSpeed;
	//if( reversed )
	//	gSpeed = -gSpeed;
	//groundSpeed = 10;
	if( groundSpeed == 0 )
	{
		if( facingRight )
		{

			//cout << "dashspeed" << endl;
			grindSpeed = dashSpeed;
			//if( reversed )
			//	grindSpeed = -dashSpeed;
		}
		else
		{
			grindSpeed = -dashSpeed;
			//if( reversed )
			//	grindSpeed = dashSpeed;
		}
	}
	else if( groundSpeed > 0 )
	{
		grindSpeed = std::max( groundSpeed, dashSpeed );
	}
	else
	{
		grindSpeed = std::min( groundSpeed, -dashSpeed );
	}
	


	framesGrinding = 0;
	rightWire->Reset();
	leftWire->Reset();
	action = GRINDBALL;
	grindEdge = ground;
	grindMovingTerrain = movingGround;
	frame = 0;
	grindQuantity = edgeQuantity;
				

	if( reversed )
	{
	//	grindSpeed = -grindSpeed;
	}
}

PlayerGhost::PlayerGhost()
	:currFrame( 0 ), currHitboxes( NULL )
{

}

void PlayerGhost::DebugDraw( sf::RenderTarget *target )
{
	if( currHitboxes != NULL )
	{
		for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
		{
			(*it).DebugDraw( target );
		}
	}
	sf::RectangleShape rs;
}

void PlayerGhost::UpdatePrePhysics( int ghostFrame )
{
	Action action = states[ghostFrame].action;
	int frame = states[ghostFrame].frame;
	double angle = states[ghostFrame].angle;
	V2d position = states[ghostFrame].position;

	currHitboxes = NULL;

	switch( action )
	{
	case FAIR:
		{
			if( fairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = fairHitboxes[frame];
			}
			break;
		}
	case UAIR:
		{
			if( uairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = uairHitboxes[frame];
			}
			break;
		}
	case DAIR:
		{
			if( dairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = dairHitboxes[frame];
			}
			break;
		}
	case STANDN:
		{
			if( standNHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standNHitboxes[frame];
			}
			break;
		}
	case STANDU:
		{
			if( standUHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standUHitboxes[frame];
			}
			break;
		}
	case STANDD:
		{
			if( standDHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standDHitboxes[frame];
			}
			break;
		}
	}

	if( currHitboxes != NULL )
	{
		for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
		{
			(*it).globalAngle = angle;

			(*it).globalPosition = position + V2d( (*it).offset.x * cos( (*it).globalAngle ) + (*it).offset.y * sin( (*it).globalAngle ), 
				(*it).offset.x * -sin( (*it).globalAngle ) + (*it).offset.y * cos( (*it).globalAngle ) );

			//(*it).globalPosition = position ;//+ (*it).offset;
		
		}
	}
}


