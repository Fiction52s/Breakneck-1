#include "Actor.h"
#include "GameSession.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>
#include "PowerOrbs.h"

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>
#define COLOR_TEAL Color( 0, 0xee, 0xff )

Actor::Actor( GameSession *gs )
	:owner( gs ), dead( false )
	{
		enemiesKilledThisFrame = 0;
		toggleBounceInput = gs->controller.keySettings.toggleBounce;
		toggleTimeSlowInput = gs->controller.keySettings.toggleTimeSlow;
		toggleGrindInput = gs->controller.keySettings.toggleGrind;
		speedParticleRate = 10; //20
		speedParticleCounter = 1;
		followerPos = V2d( 0, 0 );
		followerVel = V2d( 0, 0 );
		followerFac = 1.0 / 60.0;
		hitGoal = false;
		ground = NULL;
		//re = new RotaryParticleEffect( this );
		//re1 = new RotaryParticleEffect( this );
		pTrail = new ParticleTrail( this );
		//re1->angle += PI;
		//ae = new AirParticleEffect( position );

		level1SpeedThresh = 22;//32;
		level2SpeedThresh = 45;
		speedChangeUp = .5;//03;//.5;
		speedChangeDown = .03;//.005;//.07;

		speedLevel = 0;
		speedBarTarget = 0;
		currentSpeedBar = 0;

		Vector2f facePos( 24, 0 );
		ts_kinFace = owner->GetTileset( "kinportrait_320x288.png", 320, 288 );
		kinFace.setTexture( *ts_kinFace->texture );
		kinFace.setTextureRect( ts_kinFace->GetSubRect( 0 ) );

		kinUnderOutline.setTexture( *ts_kinFace->texture );
		kinUnderOutline.setTextureRect( ts_kinFace->GetSubRect( 0 ) );
		kinUnderOutline.setPosition( facePos );

		kinTealOutline.setTexture( *ts_kinFace->texture );
		kinTealOutline.setTextureRect( ts_kinFace->GetSubRect( 1 ) );
		kinTealOutline.setPosition( facePos );

		kinBlueOutline.setTexture( *ts_kinFace->texture );
		kinBlueOutline.setTextureRect( ts_kinFace->GetSubRect( 2 ) );
		kinBlueOutline.setPosition( facePos );

		kinPurpleOutline.setTexture( *ts_kinFace->texture );
		kinPurpleOutline.setPosition( facePos );
		kinPurpleOutline.setTextureRect( ts_kinFace->GetSubRect( 3 ) );
		//kinFace.setPosition( 2, 48 );
		//kinFace.setPosition( 1920 / 2 - 512, 0 );
		kinFace.setPosition( facePos );

		SetExpr( Expr::Expr_NEUTRAL );

		motionGhostSpacing = 6;
		ghostSpacingCounter = 0;

		drainCounterMax = 10;
		drainCounter = 0;
		currentCheckPoint = NULL;
		flashFrames = 0;
		test = false;

		lastWire = 0;
		inBubble = false;
		oldInBubble = false;

		numKeys = 0;
		
		

		gateTouched = NULL;

		
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

		if (!despFaceShader.loadFromFile("colorswap_shader.frag", sf::Shader::Fragment))
		//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
		{
			cout << "desp face SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "desp shader not loaded" );
		}
		//uniform vec4 toColor;
		//uniform vec4 fromColor;
		Color c( 0x66, 0xee, 0xff );
		despFaceShader.setParameter( "fromColor", c );

		
		
		//swordShader.setParameter("u_texture", sf::Shader::CurrentTexture);

		/*if( !timeSlowShader.loadFromFile( "timeslow_shader.frag", sf::Shader::Fragment ) )
		{
			cout << "TIME SLOW SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "time slow shader not loaded" );
		}*/

		ts_dodecaSmall = owner->GetTileset( "dodecasmall.png", 180, 180 );
		ts_dodecaBig = owner->GetTileset( "dodecabig.png", 360, 360 );

		soundBuffers[S_RUN_STEP1] = owner->soundManager->GetSound( "Audio/Sounds/run1.ogg" );
		soundBuffers[S_RUN_STEP2] = owner->soundManager->GetSound( "Audio/Sounds/run2.ogg" );
		soundBuffers[S_SPRINT_STEP1] = owner->soundManager->GetSound( "Audio/Sounds/sprint1.ogg" );
		soundBuffers[S_SPRINT_STEP2] = owner->soundManager->GetSound( "Audio/Sounds/sprint2.ogg" );
		soundBuffers[S_DASH_START] = owner->soundManager->GetSound( "Audio/Sounds/dash.ogg" );

		soundBuffers[S_HIT] = owner->soundManager->GetSound( "Audio/Sounds/kin_hitspack_short.ogg" );
		soundBuffers[S_HURT] = owner->soundManager->GetSound( "Audio/Sounds/hurt_spack.ogg" );
		soundBuffers[S_HIT_AND_KILL] = owner->soundManager->GetSound( "Audio/Sounds/kin_hitspack.ogg" );
		soundBuffers[S_HIT_AND_KILL_KEY] = owner->soundManager->GetSound( "Audio/Sounds/key_kill.ogg" );
		soundBuffers[S_FAIR] = owner->soundManager->GetSound( "Audio/Sounds/fair.ogg" );
		soundBuffers[S_DAIR] = owner->soundManager->GetSound( "Audio/Sounds/dair.ogg" );
		soundBuffers[S_UAIR] = owner->soundManager->GetSound( "Audio/Sounds/uair.ogg" );
		soundBuffers[S_WALLJUMP] = owner->soundManager->GetSound( "Audio/Sounds/walljump.ogg" );
		soundBuffers[S_WALLATTACK] = owner->soundManager->GetSound( "Audio/Sounds/wallattack.ogg" );
		soundBuffers[S_GRAVREVERSE] = owner->soundManager->GetSound( "Audio/Sounds/gravreverse.ogg" );
		soundBuffers[S_BOUNCEJUMP] = owner->soundManager->GetSound( "Audio/Sounds/bounce.ogg" );
		soundBuffers[S_STANDATTACK] = owner->soundManager->GetSound( "Audio/Sounds/standattack.ogg" );
		soundBuffers[S_TIMESLOW] = owner->soundManager->GetSound( "Audio/Sounds/timeslow.ogg" );
		soundBuffers[S_ENTER] = owner->soundManager->GetSound( "Audio/Sounds/enter.ogg" );
		soundBuffers[S_EXIT] = owner->soundManager->GetSound( "Audio/Sounds/exit.ogg" );

		

		/*if( !fairBuffer.loadFromFile( "fair.ogg" ) )
		{
			assert( 0 && "failed to load test fair noise" );
		}
		fairSound.setBuffer( fairBuffer);
		fairSound.setVolume( 0 );*/

		/*if( !runTappingBuffer.loadFromFile( "runtapping.ogg" ) )
		{
			assert( 0 && "failed to load test runtapping noise" );
		}
		runTappingSound.setBuffer( runTappingBuffer);
		runTappingSound.setVolume( 0 );*/

		/*if( !playerHitBuffer.loadFromFile( "playerhit.ogg" ) )
		{
			assert( 0 && "failed to load test runtapping noise" );
		}
		playerHitSound.setBuffer( playerHitBuffer );
		playerHitSound.setVolume( 0 );
*/
		//if( !dashStartBuffer.loadFromFile( "dashstart.ogg" ) )
		//if( !dashStartBuffer.loadFromFile( "a_dash.wav" ) )
		/*if( !dashStartBuffer.loadFromFile( "a_dash.ogg" ) )
		{
			assert( 0 && "failed to load test dashstart noise" );
		}
		dashStartSound.setBuffer( dashStartBuffer);
		dashStartSound.setVolume( 0 );*/
		//dashStartSound.setVolume( 10 );
		//dashStartSound.setLoop( true );

		//if( !jumpBuffer.loadFromFile( "a_jump.ogg" ) )
		//{
		//	assert( 0 && "failed to load test jump noise" );
		//}
		//jumpSound.setBuffer( jumpBuffer);
		////jumpSound.setVolume( 20 );
		//jumpSound.setVolume( 0 );

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
		maxDespFrames = 60 * 5;
		despCounter = 0;

		

		holdJump = false;
		steepJump = false;

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
		//for( int j = 4; j < 10; ++j )
		for( int j = 0; j < 8 *2; ++j )
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
		cb.offset.y = -14;

		for( int j = 0; j <= 12; ++j )
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


		cb.rw = 64;
		cb.rh = 64;
		cb.offset.x = 0;
		cb.offset.y = 32;
		for( int j = 2; j <= 9; ++j )
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

		
		cb.rw = 64;
		cb.rh = 64;
		cb.offset.x = 36;
		cb.offset.y = -6;
		//for( int j = 1; j <= 4; ++j )
		for( int j = 0; j < 6 * 4; ++j )
		{
			standHitboxes[j] = new list<CollisionBox>;
			standHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->standHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->standHitboxes[j]->push_back( cb );			
			}
		}

		cb.rw = 64;
		cb.rh = 64;
		cb.offset.x = 36;
		cb.offset.y = -6;
		//for( int j = 1; j <= 4; ++j )
		for( int j = 0; j < 8; ++j )
		{
			dashHitboxes[j] = new list<CollisionBox>;
			dashHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->dashHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->dashHitboxes[j]->push_back( cb );			
			}
		}

		cb.rw = 64;
		cb.rh = 64;
		cb.offset.x = 36;
		cb.offset.y = -6;
		//for( int j = 1; j <= 4; ++j )
		for( int j = 0; j < 4 * 4; ++j )
		{
			steepClimbHitboxes[j] = new list<CollisionBox>;
			steepClimbHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->steepClimbHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->steepClimbHitboxes[j]->push_back( cb );			
			}
		}

		cb.rw = 64;
		cb.rh = 64;
		cb.offset.x = 36;
		cb.offset.y = -6;
		//for( int j = 1; j <= 4; ++j )
		for( int j = 0; j < 6 * 3; ++j )
		{
			steepSlideHitboxes[j] = new list<CollisionBox>;
			steepSlideHitboxes[j]->push_back( cb );

			for( int i = 0; i < MAX_GHOSTS; ++i )
			{
				//ghosts[i] = new PlayerGhost;
				ghosts[i]->steepSlideHitboxes[j] = new list<CollisionBox>;
				ghosts[i]->steepSlideHitboxes[j]->push_back( cb );			
			}
		}

		queryMode = "";
		wallThresh = .9999;
		//tileset setup
		

		bounceFlameOn = false;
		
		ts_airBounceFlame = owner->GetTileset( "bouncejumpflame.png", 128, 128 );
		ts_runBounceFlame = owner->GetTileset( "bouncerunflame.png", 128, 96 );

		ts_bounceBoost = owner->GetTileset( "bounceboost_256x192.png", 256, 192 );

		
		airBounceFlameFrames = 20 * 3;
		runBounceFlameFrames = 21 * 3;

		actionLength[WALLATTACK] = 8 * 2;
		tileset[WALLATTACK] = owner->GetTileset( "wallattack_64x128.png", 64, 128 );
		normal[WALLATTACK] = owner->GetTileset( "dair_NORMALS.png", 96, 64 );

		actionLength[DAIR] = 10 * 2;
		tileset[DAIR] = owner->GetTileset( "dair_80x80.png", 80, 80 );
		normal[DAIR] = owner->GetTileset( "dair_NORMALS.png", 96, 64 );

		actionLength[DASH] = 45;
		tileset[DASH] = owner->GetTileset( "dash_96x48.png", 96, 48 );
		normal[DASH] = owner->GetTileset( "dash_NORMALS.png", 64, 64 );

		actionLength[DOUBLE] = 28 + 10;
		tileset[DOUBLE] = owner->GetTileset( "double_64x64.png", 64, 64 );
		normal[DOUBLE] = owner->GetTileset( "double_NORMALS.png", 64, 64 );
		
		actionLength[FAIR] = 8 * 2;
		tileset[FAIR] = owner->GetTileset( "fair_64x64.png", 64, 64 );
		normal[FAIR] = owner->GetTileset( "fair_NORMALS.png", 80, 64 );

		actionLength[JUMP] = 2;
		tileset[JUMP] = owner->GetTileset( "jump_64x64.png", 64, 64 );
		normal[JUMP] = owner->GetTileset( "jump_NORMALS.png", 64, 64 );

		actionLength[LAND] = 1;
		tileset[LAND] = owner->GetTileset( "land_64x64.png", 64, 64 );
		normal[LAND] = owner->GetTileset( "land_NORMALS.png", 64, 64 );

		actionLength[LAND2] = 1;
		tileset[LAND2] =  owner->GetTileset( "land_64x64.png", 64, 64 );
		normal[LAND2] = owner->GetTileset( "land2_NORMALS.png", 64, 64 );

		actionLength[RUN] = 10 * 4;
		tileset[RUN] = owner->GetTileset( "run_64x64.png", 64, 64 );
		normal[RUN] = owner->GetTileset( "run_NORMALS.png", 80, 48 );

		actionLength[SLIDE] = 1;
		tileset[SLIDE] = owner->GetTileset( "slide_64x64.png", 64, 64 );
		normal[SLIDE] = owner->GetTileset( "slide_NORMALS.png", 64, 64 );

		actionLength[SPRINT] = 8 * 4;
		tileset[SPRINT] = owner->GetTileset( "sprint_128x48.png", 128, 48 );		
		//tileset[SPRINT] = owner->GetTileset( "sprint_96x48.png", 96, 48 );		
		normal[SPRINT] = owner->GetTileset( "sprint_NORMALS.png", 128, 64 );		

		actionLength[STAND] = 20 * 8;
		tileset[STAND] = owner->GetTileset( "stand_64x64.png", 64, 64 );
		normal[STAND] = owner->GetTileset( "stand_NORMALS.png", 64, 64 );

		actionLength[DASHATTACK] = 8 * 2;
		tileset[DASHATTACK] = owner->GetTileset( "dash_attack_128x96.png", 128, 96 );
		normal[DASHATTACK] = owner->GetTileset( "standd_NORMALS.png", 96, 48 );

		actionLength[STANDN] = 4 * 4;
		tileset[STANDN] = owner->GetTileset( "standn_96x64.png", 96, 64 );
		normal[STANDN] = owner->GetTileset( "standn_NORMALS.png", 128, 64 );

		actionLength[UAIR] = 6 * 3;
		tileset[UAIR] = owner->GetTileset( "uair_80x80.png", 80, 80 );
		normal[UAIR] = owner->GetTileset( "uair_NORMALS.png", 80, 80 );

		actionLength[WALLCLING] = 1;
		tileset[WALLCLING] = owner->GetTileset( "wallcling_64x64.png", 64, 64 );
		normal[WALLCLING] = owner->GetTileset( "wallcling_NORMALS.png", 64, 64 );

		actionLength[WALLJUMP] = 9 * 2;
		tileset[WALLJUMP] = owner->GetTileset( "walljump_64x64.png", 64, 64 );
		normal[WALLJUMP] = owner->GetTileset( "walljump_NORMALS.png", 64, 64 );

		actionLength[GRINDBALL] = 1;
		tileset[GRINDBALL] = owner->GetTileset( "grindball_64x64.png", 64, 64 );
		normal[GRINDBALL] = owner->GetTileset( "grindball_NORMALS.png", 32, 32 );

		actionLength[STEEPSLIDE] = 1;
		tileset[STEEPSLIDE] = owner->GetTileset( "steepslide_80x48.png", 80, 48 );
		normal[STEEPSLIDE] = owner->GetTileset( "steepslide_NORMALS.png", 64, 32 );

		actionLength[STEEPCLIMBATTACK] = 4 * 4;
		tileset[STEEPCLIMBATTACK] = owner->GetTileset( "climb_att_128x48.png", 128, 48 );
		normal[STEEPCLIMBATTACK] = owner->GetTileset( "standd_NORMALS.png", 96, 48 );

		actionLength[STEEPSLIDEATTACK] = 6 * 3;
		tileset[STEEPSLIDEATTACK] = owner->GetTileset( "steep_att_80x64.png", 80, 64 );
		normal[STEEPSLIDEATTACK] = owner->GetTileset( "standd_NORMALS.png", 96, 48 );

		actionLength[AIRDASH] = 27;
		tileset[AIRDASH] = owner->GetTileset( "airdash_80x80.png", 80, 80 );
		normal[AIRDASH] = owner->GetTileset( "airdash_NORMALS.png", 64, 64 );

		actionLength[STEEPCLIMB] = 8 * 4;
		tileset[STEEPCLIMB] = owner->GetTileset( "steepclimb_128x64.png", 128, 64 );
		normal[STEEPCLIMB] = owner->GetTileset( "steepclimb_NORMALS.png", 128, 64 );

		actionLength[AIRHITSTUN] = 1;
		tileset[AIRHITSTUN] = owner->GetTileset( "hurt_64x64.png", 64, 64 );
		normal[AIRHITSTUN] = owner->GetTileset( "hurt_air_NORMALS.png", 64, 64 );

		actionLength[GROUNDHITSTUN] = 1;
		tileset[GROUNDHITSTUN] = owner->GetTileset( "hurt_64x64.png", 64, 64 );
		normal[GROUNDHITSTUN] = owner->GetTileset( "hurt_stand_NORMALS.png", 64, 64 );

		actionLength[WIREHOLD] = 1;
		tileset[WIREHOLD] = owner->GetTileset( "steepslide_80x48.png", 80, 48 );
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
		tileset[DEATH] = owner->GetTileset( "death_128x96.png", 128, 96 );
		//normal[DEATH] = owner->GetTileset( "death_NORMALS.png", 64, 64 );

		actionLength[JUMPSQUAT] = 3;
		tileset[JUMPSQUAT] = owner->GetTileset( "jump_64x64.png", 64, 64 );
		normal[JUMPSQUAT] = owner->GetTileset( "jump_NORMALS.png", 64, 64 );
		
		actionLength[INTRO] = 10 * 4;
		tileset[INTRO] = owner->GetTileset( "intro_0_160x80.png", 160, 80 );
		normal[INTRO] = owner->GetTileset( "intro_0_160x80.png", 160, 80 );

		actionLength[EXIT] = 27 * 2;
		tileset[EXIT] = owner->GetTileset( "exit_0_128x160.png", 128, 160 );
		normal[EXIT] = owner->GetTileset( "exit_0_128x160.png", 128, 160 );

		
		actionLength[RIDESHIP] = 1;
		tileset[RIDESHIP] = owner->GetTileset( "dive_80x80.png", 80, 80 );
		normal[RIDESHIP] = owner->GetTileset( "dive_80x80.png", 80, 80 );

		actionLength[SKYDIVE] = 9 * 2;
		tileset[SKYDIVE] = owner->GetTileset( "walljump_64x64.png", 64, 64 );
		normal[SKYDIVE] = owner->GetTileset( "walljump_NORMALS.png", 64, 64 );

		actionLength[SKYDIVETOFALL] = 10 * 4;
		tileset[SKYDIVETOFALL] = owner->GetTileset( "intro_0_160x80.png", 160, 80 );
		normal[SKYDIVETOFALL] = owner->GetTileset( "intro_0_160x80.png", 160, 80 );


		actionLength[GOALKILL] = 72 * 2;
		ts_goalKillArray = new Tileset*[5];
		ts_goalKillArray[0] = owner->GetTileset( "goal_w02_killa_384x256.png", 384, 256 );
		ts_goalKillArray[1] = owner->GetTileset( "goal_w02_killb_384x256.png", 384, 256 );
		ts_goalKillArray[2] = owner->GetTileset( "goal_w02_killc_384x256.png", 384, 256 );
		ts_goalKillArray[3] = owner->GetTileset( "goal_w02_killd_384x256.png", 384, 256 );
		ts_goalKillArray[4] = owner->GetTileset( "goal_w02_kille_384x256.png", 384, 256 );

		actionLength[GOALKILLWAIT] = 2;

		actionLength[SPAWNWAIT] = 120;
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

		ts_fairSword[0] = owner->GetTileset( "fair_sworda_192x192.png", 192, 192 );
		ts_fairSword[1] = owner->GetTileset( "fair_swordb_256x256.png", 256, 256 );
		ts_fairSword[2] = owner->GetTileset( "fair_swordc_384x384.png", 384, 384 );

		ts_dairSword[0] = owner->GetTileset( "dair_sworda_144x192.png", 144, 192 );
		ts_dairSword[1] = owner->GetTileset( "dair_swordb_192x208.png", 192, 208 );
		ts_dairSword[2] = owner->GetTileset( "dair_swordc_256x256.png", 256, 256 );

		ts_uairSword[0] = owner->GetTileset( "uair_sworda_160x160.png", 160, 160 );
		ts_uairSword[1] = owner->GetTileset( "uair_swordb_224x224.png", 224, 224 );
		ts_uairSword[2] = owner->GetTileset( "uair_swordc_384x384.png", 384, 384 );

		ts_standingNSword[0] = owner->GetTileset( "stand_sworda_272x160.png", 272, 160 );
		ts_standingNSword[1] = owner->GetTileset( "stand_swordb_304x160.png", 304, 160 );
		ts_standingNSword[2] = owner->GetTileset( "stand_swordc_336x160.png", 336, 160 );

		ts_dashAttackSword[0] = owner->GetTileset( "dash_sworda_256x256.png", 256, 256 );
		ts_dashAttackSword[1] = owner->GetTileset( "dash_swordb_256x256.png", 256, 256 );
		ts_dashAttackSword[2] = owner->GetTileset( "dash_swordc_256x304.png", 256, 304 );

		ts_wallAttackSword[0] = owner->GetTileset( "wall_sworda_128x256.png", 128, 256 );
		ts_wallAttackSword[1] = owner->GetTileset( "wall_swordb_128x288.png", 128, 288 );
		ts_wallAttackSword[2] = owner->GetTileset( "wall_swordc_160x384.png", 160, 384 );

		ts_steepSlideAttackSword[0] = owner->GetTileset( "steep_att_sworda_288x128.png", 288, 128 );
		ts_steepSlideAttackSword[1] = owner->GetTileset( "steep_att_swordb_320x144.png", 320, 144 );
		ts_steepSlideAttackSword[2] = owner->GetTileset( "steep_att_swordc_352x156.png", 352, 156 );

		ts_steepClimbAttackSword[0] = owner->GetTileset( "climb_att_sworda_256x80.png", 256, 80 );
		ts_steepClimbAttackSword[1] = owner->GetTileset( "climb_att_swordb_320x96.png", 320, 96 );
		ts_steepClimbAttackSword[2] = owner->GetTileset( "climb_att_swordc_352x112.png", 352, 112 );

		ts_fx_hurtSpack = owner->GetTileset( "hurt_spack_128x160.png", 128, 160 );

		ts_fx_dashStart = owner->GetTileset( "fx_dashstart_192x160.png", 192, 160 );
		ts_fx_dashRepeat = owner->GetTileset( "fx_dashrepeat_192x128.png", 192, 128 );
		ts_fx_land = owner->GetTileset( "fx_land_128x128.png", 128, 128 );

		ts_fx_runStart = owner->GetTileset( "fx_runstart_96x96.png", 96, 96 );
		ts_fx_sprint = owner->GetTileset( "fx_sprint_192x192.png", 192, 192 );
		ts_fx_run = owner->GetTileset( "fx_run_160x128.png", 160, 128 );

		ts_fx_bigRunRepeat = owner->GetTileset( "fx_bigrunrepeat.png", 176, 112 );
		ts_fx_jump = owner->GetTileset( "fx_jump_160x64.png", 160, 64 );
		ts_fx_wallJump = owner->GetTileset( "fx_walljump_128x160.png", 128, 160 );
		ts_fx_double = owner->GetTileset( "fx_doublejump_196x160.png", 196 , 160 );
		ts_fx_gravReverse = owner->GetTileset( "fx_grav_reverse_128x128.png", 128 , 128 );
		ts_fx_chargeBlue0 = owner->GetTileset( "elec_01_128x128.png", 128, 128 );
		ts_fx_chargeBlue1 = owner->GetTileset( "elec_03_128x128.png", 128, 128 );
		ts_fx_chargeBlue2 = owner->GetTileset( "elec_04_128x128.png", 128, 128 );
		ts_fx_chargePurple = owner->GetTileset( "elec_02_128x128.png", 128, 128 );
		ts_fx_airdashDiagonal = owner->GetTileset( "fx_airdash_diagonal_1_128x128.png", 128, 128 );
		ts_fx_airdashUp = owner->GetTileset( "fx_airdash_up_1_128x128.png", 128, 128 );
		ts_fx_airdashSmall = owner->GetTileset( "fx_airdash.png", 32, 32 );		
		ts_fx_airdashHover = owner->GetTileset( "fx_airdash_hold_1_96x80.png", 96, 80 ); 
		ts_fx_death_1a = owner->GetTileset( "death_fx_1a_256x256.png", 256, 256 );
		ts_fx_death_1b = owner->GetTileset( "death_fx_1b_128x80.png", 128, 80 );
		ts_fx_death_1c = owner->GetTileset( "death_fx_1c_128x128.png", 128, 128 );
		ts_fx_death_1d = owner->GetTileset( "death_fx_1d_48x48.png", 48, 48 );
		ts_fx_death_1e = owner->GetTileset( "death_fx_1e_160x160.png", 160, 160 );
		ts_fx_death_1f = owner->GetTileset( "death_fx_1f_160x160.png", 160, 160 );

		if (!swordShaders[0].loadFromFile("colorswap_shader.frag", sf::Shader::Fragment))
		{
			cout << "SWORD SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "sword shader not loaded" );
		}
		swordShaders[0].setParameter( "fromColor", COLOR_TEAL );
		//swordShaders[1] = swordShaders[0];
		if (!swordShaders[1].loadFromFile("colorswap_shader.frag", sf::Shader::Fragment))
		{
			cout << "SWORD SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "sword shader not loaded" );
		}
		swordShaders[1].setParameter( "fromColor", Color( 43, 167, 255 ) );
		//swordShaders[2] = swordShaders[0];
		if (!swordShaders[2].loadFromFile("colorswap_shader.frag", sf::Shader::Fragment))
		{
			cout << "SWORD SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "sword shader not loaded" );
		}
		swordShaders[2].setParameter( "fromColor", Color( 140, 145, 255 ) );


		//ts_bounceRun = owner->GetTileset( "bouncerun.png", 128, 64 );
		//ts_bounceSprint = owner->GetTileset( "bouncesprint.png", 128, 64 );

		grindActionLength = 32;
		SetActionExpr( SPAWNWAIT );
		frame = 0;
		
		timeSlowStrength = 5;
		slowMultiple = 1;
		slowCounter = 1;

		reversed = false;

		grindActionCurrent = 0;

		framesInAir = 0;
		wallJumpFrameCounter = 0;
		wallJumpMovementLimit = 12; //10 frames

		steepThresh = .4; // go between 0 and 1

		steepSlideGravFactor = .4;
		steepSlideFastGravFactor = .5;

		wallJumpStrength.x = 10;
		wallJumpStrength.y = 20;
		clingSpeed = 3;

		slopeTooSteepLaunchLimitX = .1;
		
		
		steepClimbGravFactor = .31;//.7;
		steepClimbFastFactor = .2;
		framesSinceClimbBoost = 0;
		climbBoostLimit = 5;
		

		

		hasDoubleJump = true;
		

		ground = NULL;
		movingGround = NULL;
		groundSpeed = 0;
		maxNormalRun = 60; //adjust up w/ more power?
	
		facingRight = true;
		collision = false;
	
		airAccel = 1.5;
		
		gravity = 1;//1.9; // 1 
		jumpStrength = 21.5;//18;//25;//27.5; // 2 
		doubleJumpStrength = 20;//17;//23;//26.5;
		dashSpeed = 9;//12; // 3
		airDashSpeed = dashSpeed;
		maxFallSpeedSlow = 30;//30;//100; // 4
		maxFallSpeedFast = 60;


		offSlopeByWallThresh = dashSpeed;//18;
		slopeLaunchMinSpeed = 5;//dashSpeed * .7;
		steepClimbSpeedThresh = dashSpeed - 1;
		slideGravFactor = .45;

		
		maxRunInit = 4;
		maxAirXControl = 6;//maxRunInit;
		airSlow = .7;//.3;

		groundOffsetX = 0;

		grindEdge = NULL;
		grindMovingTerrain = NULL;
		grindQuantity = 0;
		grindSpeed = 0;

		
		

		//max ground speed should probably start around 60-80 and then get powerups to rise to 100
		//for world 1 lets do the lowest number for the beta
		double maxXSpeed = 60;
		maxGroundSpeed = maxXSpeed;
		maxAirXSpeed = maxXSpeed;


		runAccelInit = .5;
		
		runAccel = .03;
		sprintAccel = .3;//.85;

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
		hurtBody.rw = 7;//10;
		hurtBody.rh = 15;//normalHeight - 5;//normalHeight;
		hurtBody.type = CollisionBox::BoxType::Hurt;

		currHitboxes = NULL;
		currHitboxInfo = NULL;

		currHitboxInfo = new HitboxInfo();
		currHitboxInfo->damage = 20;
		currHitboxInfo->drainX = 0;
		currHitboxInfo->drainY = 0;
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
		//ts_fx_airdash = owner->GetTileset( "fx_airdash.png", 32, 32 );
		


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
	//cout << "length: " << actionLength[action] << endl;
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
		case JUMPSQUAT:
			frame = 0;
			//never happens
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
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case STANDN:

			if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.B )
				{
					action = DASH;
					//re->Reset();
					//re1->Reset();
				}
				else
				{
					SetActionExpr( RUN );
				}
				facingRight = currInput.LRight();
			}
			else
			{
				SetActionExpr( STAND );
			}
			frame = 0;
			break;
		case DASHATTACK:
			if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.B )
				{
					action = DASH;
					//re->Reset();
					//re1->Reset();
				}
				else
				{
					SetActionExpr( RUN );
				}
				facingRight = currInput.LRight();
			}
			else
			{
				SetActionExpr( STAND );
			}
			frame = 0;
			break;

		case FAIR:
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case WALLATTACK:
			SetActionExpr( WALLCLING );
			frame = 0;
			break;
		case STEEPCLIMBATTACK:
			SetActionExpr( STEEPCLIMB );
			frame = 0;
			break;
		case STEEPSLIDEATTACK:
			SetActionExpr( STEEPSLIDE );
			frame = 0;
			break;
		case DAIR:
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case UAIR:
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case DASH:
			//dashStartSound.stop();
			SetActionExpr( STAND );
			frame = 0;
			break;
		case DOUBLE:
			SetActionExpr( JUMP );
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
				if( inBubble )//|| rightWire->state == Wire::PULLING )
				{
					frame = actionLength[AIRDASH] - 1;
					airDashStall = true;
				}
				else
				{
					SetActionExpr( JUMP );
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
			break;
		case INTRO:
			//cout << "intro over" << endl;
			action = JUMP;
			frame = 1;
			break;
		case EXIT:
			//owner->goalDestroyed = true;
			
			//frame = 0;
			break;
		case SPAWNWAIT:
			action = INTRO;
			frame = 0;
			break;
		case GOALKILL:
			action = GOALKILLWAIT;
			frame = 0;
			owner->scoreDisplay->Activate();
			break;
		case GOALKILLWAIT:
			//action = EXIT;
			frame = 0;
			break;
		case DEATH:
		
			frame = 0;
			break;
		}
	}
}

void Actor::CheckHoldJump()
{
	if( hasDoubleJump )
	{
		if( holdJump && velocity.y >= -8 )
			holdJump = false;

		
		if( holdJump && ((!steepJump && !currInput.A) || (steepJump && !currInput.LUp() ) ) && framesInAir > 2 )
		{
			if( velocity.y < -8 )
			{
				velocity.y = -8;
			}
		}
	}
	else
	{
		if( holdDouble && velocity.y >= -8 )
			holdDouble = false;

		if( holdDouble && !currInput.A && framesInAir > 2 )
		{
			if( velocity.y < -8 )
			{
				velocity.y = -8;
			}
		}
	}
}

bool Actor::AirAttack()
{
	if( currInput.rightShoulder && !prevInput.rightShoulder )
	{
		if( currInput.LUp() )
		{
			SetAction( UAIR);
			frame = 0;
		}
		else if( currInput.LDown() )
		{
			SetAction( DAIR );
			frame = 0;
		}
		else
		{
			SetAction( FAIR );
			frame = 0;
		}
		return true;
	}
	return false;
}

void Actor::UpdatePrePhysics()
{
	
	if( owner->drain && !desperationMode && action != SPAWNWAIT && action != INTRO && action != GOALKILL && action != EXIT && action != GOALKILLWAIT )
	{
		if( drainCounter == drainCounterMax)
		{
			bool stillHasHealth = owner->powerWheel->Use( 1 );	

			if( !stillHasHealth )
			{
				desperationMode = true;
				despCounter = 0;
			}
			drainCounter = 0;
		}
		else
		{
			drainCounter++;
		}
	}
	//cout << "vel at beg: " << velocity.x << ", " << velocity.y << endl;
	
	

	//cout << "startvel : " << velocity.x << ", " << velocity.y << endl;	

	enemiesKilledThisFrame = 0;

	

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
		

		int transFrames = 8;
		
		

		Color blah[8];// = { Color( 0x00, 0xff, 0xff ), Color(0x00, 0xbb, 0xff ) };
		blah[0] = Color( 0x00, 0xff, 0xff );
		blah[1] = Color( 0x00, 0xbb, 0xff );
		int cIndex = 2;
		if( hasPowerAirDash )
		{
			blah[cIndex] = Color( 0x00, 0x55, 0xff );
			cIndex++;
		}
		if( hasPowerGravReverse )
		{
			blah[cIndex] = Color( 0x00, 0xff, 0x88 );
			cIndex++;
		}
		if( hasPowerBounce )
		{
			blah[cIndex] = Color( 0xff, 0xff, 0x33 );
			cIndex++;
		}

		if( hasPowerGrindBall )
		{
			blah[cIndex] = Color( 0xff, 0x88, 0x00 );
			cIndex++;
		}

		if( hasPowerTimeSlow )
		{
			blah[cIndex] = Color( 0xff, 0x00, 0x00 );
			cIndex++;
		}

		if( hasPowerRightWire || hasPowerLeftWire )
		{
			blah[cIndex] = Color( 0xff, 0x33, 0xaa );
			cIndex++;
		}
		int numColors = cIndex;

		int tFrame = despCounter % transFrames;
		int ind = (despCounter / transFrames) % numColors;
		Color currCol = blah[ind];
		Color nextCol;
		if( ind == numColors - 1 )
		{
			nextCol = blah[0];
		}
		else
		{
			nextCol = blah[ind+1];
		}
		float fac = (float)tFrame / transFrames;
		currentDespColor.r = floor(currCol.r * ( 1.f - fac ) + nextCol.r * fac + .5);
		currentDespColor.g = floor(currCol.g * ( 1.f - fac ) + nextCol.g * fac + .5);
		currentDespColor.b = floor(currCol.b * ( 1.f - fac ) + nextCol.b * fac + .5);

		float overallFac = (float)despCounter / 60;
		overallFac = std::min( overallFac, 1.f );
		Color auraColor( 0x66, 0xdd, 0xff );
		auraColor.r = floor( auraColor.r * ( 1.f - overallFac ) + Color::Black.r * fac + .5 );
		auraColor.g = floor( auraColor.g * ( 1.f - overallFac ) + Color::Black.g * fac + .5 );
		auraColor.b = floor( auraColor.b * ( 1.f - overallFac ) + Color::Black.b * fac + .5 );
		sh.setParameter( "despColor", currentDespColor );
		despFaceShader.setParameter( "toColor", currentDespColor );
		sh.setParameter( "auraColor", auraColor );
		//currentDespColor


		//cout << "desperation: " << despCounter << endl;
		despCounter++;
		if( despCounter == maxDespFrames )
		{
			desperationMode = false;
			if( owner->powerWheel->activeOrb > 0 || owner->powerWheel->activeLevel > 0
				||  owner->powerWheel->activeSection > 0 )
			{
				owner->powerWheel->mode = PowerWheel::NORMAL;
				//you gathered health in desp mode!
			}
			else
			{
				action = DEATH;
				rightWire->Reset();
				leftWire->Reset();
				slowCounter = 1;
				frame = 0;
				owner->deathWipe = true;
			}
			
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
			owner->powerWheel->Use( 20 );
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

	if( action == INTRO || action == SPAWNWAIT || action == GOALKILL || action == EXIT 
		|| action == RIDESHIP )
	{
		

		if( action == INTRO && frame == 0 )
		{
			owner->soundNodeList->ActivateSound( soundBuffers[S_ENTER] );
		}
		else if( action == EXIT && frame == 30 )
		{
			owner->soundNodeList->ActivateSound( soundBuffers[S_EXIT] );
		}
		return;
	}
	else if( action == GOALKILLWAIT )
	{
		if( currInput.A && !prevInput.A && owner->scoreDisplay->waiting )
		{
			//owner->scoreDisplay->Reset();
			owner->scoreDisplay->Deactivate();
			//owner->scoreDisplay->Activate();
		}
		
		if( !owner->scoreDisplay->active )
		{
			action = EXIT;
			frame = 0;
		}
		return;
	}

	V2d gNorm;
	if( ground != NULL )
		gNorm = ground->Normal();

	if( receivedHit != NULL && action != DEATH )
	{
		hitlagFrames = receivedHit->hitlagFrames;
		hitstunFrames = receivedHit->hitstunFrames;
		invincibleFrames = receivedHit->hitstunFrames + 20;//25;//receivedHit->damage;
		
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_hurtSpack, position, true, 0, 12, 1, facingRight );
		owner->Pause( 6 );

		owner->soundNodeList->ActivateSound( soundBuffers[S_HURT] );

		SetExpr( Expr::Expr_HURT );
		//expr = Expr::Expr_HURT;
		
		//cout << "damaging player with: " << receivedHit->damage << endl;
		bool dmgSuccess = owner->powerWheel->Damage( receivedHit->damage );
		if( true )
		{
			if( grindEdge != NULL )
			{
				//do something different for grind ball? you don't wanna be hit out at a sensitive moment
				//owner->powerWheel->Damage( receivedHit->damage ); //double damage for now bleh
				//grindSpeed *= .8;
				
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
						owner->powerWheel->Damage( receivedHit->damage );
						
						//apply extra damage since you cant stand up
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
						groundSpeed = grindSpeed;
						/*action = LAND;
						frame = 0;
						

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
						}*/

						action = GROUNDHITSTUN;
						frame = 0;

						if( receivedHit->knockback > 0 )
						{
							groundSpeed = receivedHit->kbDir.x * receivedHit->knockback;
						}
						else
						{
							groundSpeed *= (1-receivedHit->drainX) * abs(grindNorm.y) + (1-receivedHit->drainY) * abs(grindNorm.x);
						}

						if( toggleGrindInput )
						{
							currInput.Y = false;
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

						owner->powerWheel->Damage( receivedHit->damage );
						
						//apply extra damage since you cant stand up
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
							

							//SetActionExpr( JUMP );
							action = AIRHITSTUN;
							frame = 0;
							if( receivedHit->knockback > 0 )
							{
								velocity = receivedHit->knockback * receivedHit->kbDir;
							}
							else
							{
								velocity.x *= (1 - receivedHit->drainX);
								velocity.y *= (1 - receivedHit->drainY);
							}

							if( toggleGrindInput )
							{
								currInput.Y = false;
							}

						//	frame = 0;
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

								
							//if( currInput.LRight() )
							//{
							//	if( groundSpeed < 0 )
							//	{
							//		//cout << "bleh2" << endl;
							//		groundSpeed = 0;
							//	}
							//	facingRight = true;
							////	groundSpeed = abs( groundSpeed );
							//}
							//else if( currInput.LLeft() )
							//{
							//	facingRight = false;
							//	if( groundSpeed > 0 )
							//	{
							//		//cout << "bleh1" << endl;
							//		groundSpeed = 0;
							//	}
							////	groundSpeed = -abs( groundSpeed );
							//}

							//action = LAND2;


							action = GROUNDHITSTUN;
							frame = 0;

							if( toggleGrindInput )
							{
								currInput.Y = false;
							}

							if( receivedHit->knockback > 0 )
							{
								groundSpeed = receivedHit->kbDir.x * receivedHit->knockback;
							}
							else
							{
								groundSpeed *= (1-receivedHit->drainX) * abs(grindNorm.y) + (1-receivedHit->drainY) * abs(grindNorm.x);
							}

							frame = 0;
							framesNotGrinding = 0;

							double angle = GroundedAngle();



							owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
							owner->soundNodeList->ActivateSound( soundBuffers[S_GRAVREVERSE] );
						}
					}
				}		

			}
			else if( ground == NULL )
			{
				action = AIRHITSTUN;
				frame = 0;
				if( receivedHit->knockback > 0 )
				{
					velocity = receivedHit->knockback * receivedHit->kbDir;
				}
				else
				{
					velocity.x *= (1 - receivedHit->drainX);
					velocity.y *= (1 - receivedHit->drainY);
				}
				
			}
			else
			{
				action = GROUNDHITSTUN;
				frame = 0;

				if( receivedHit->knockback > 0 )
				{
					groundSpeed = receivedHit->kbDir.x * receivedHit->knockback;
				}
				else
				{
					groundSpeed *= (1-receivedHit->drainX) * abs(gNorm.y) + (1-receivedHit->drainY) * abs(gNorm.x);
				}
				
				//dot( receivedHit->kbDir, normalize( ground->v1 - ground->v0 ) ) * receivedHit->knockback;
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
	//cout << cout << "toggle bounce: " << (int)toggleBounceInput << endl;
	bool justToggledBounce = false;
	if( bounceFlameOn )
	{
		if( toggleBounceInput )
		{
			if( currInput.X && !prevInput.X )
			{
				bounceFlameOn = false;
				bounceGrounded = false;
				justToggledBounce = true;
			}
		}
		else
		{
			//assert( !toggleBounceInput );
			if( !currInput.X )
			{
				bounceFlameOn = false;
				bounceGrounded = false;
			}
		}
	}
	
	
	if( action == AIRHITSTUN )
	{
		if( hitstunFrames == 0 )
		{
			SetActionExpr( JUMP );
			frame = 1;
			prevInput = ControllerState();
		}
		
	}
	else if( action == GROUNDHITSTUN )
	{
		if( hitstunFrames == 0 )
		{
			SetActionExpr( LAND );
			frame = 0;
			prevInput = ControllerState();
		}
		
	}

	switch( action )
	{
	case STAND:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn && !justToggledBounce )
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
				////runTappingSound.stop();
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
						cout << "this steep 2" << endl;
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
						cout << "this steep 3" << endl;
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
				SetActionExpr( JUMPSQUAT );
				//action = JUMPSQUAT;
				bufferedAttack = false;
				frame = 0;
				break;
			}
			else if( currInput.B && !prevInput.B )
			{
				action = DASH;
				//re->Reset();
				//re1->Reset();
				frame = 0;
			}
			else if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.LDown() )
				{
					SetActionExpr( SPRINT );
					frame = 0;
				}
				else
				{
					SetActionExpr( RUN );
					
					frame = 0;
				}
				break;
				
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				GroundAttack();
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
				////runTappingSound.stop();
				break;
			}


			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMPSQUAT );
				bufferedAttack = false;
				frame = 0;
				////runTappingSound.stop();
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
						////runTappingSound.stop();
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						////runTappingSound.stop();
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
						////runTappingSound.stop();
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						////runTappingSound.stop();
						break;
					}
				}
			}



			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				GroundAttack();

				////runTappingSound.stop();
				break;
			}

			if( currInput.B && !prevInput.B )
			{
				action = DASH;
				/*re->Reset();
				re1->Reset();*/
				frame = 0;
				////runTappingSound.stop();
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
					SetActionExpr( STAND );
					frame = 0;
				}
				////runTappingSound.stop();
				break;
				
			}
			else
			{

				if( facingRight && currInput.LLeft() )
				{
					
					if( ( currInput.LDown() && gNorm.x < 0 ) || ( currInput.LUp() && gNorm.x > 0 ) )
					{
						SetActionExpr( SPRINT );
					}
					
					groundSpeed = 0;
					facingRight = false;
					frame = 0;
					//runTappingSound.stop();
					break;
				}
				else if( !facingRight && currInput.LRight() )
				{
					if( ( currInput.LDown() && gNorm.x > 0 ) || ( currInput.LUp() && gNorm.x < 0 ) )
					{
						SetActionExpr( SPRINT );
					}

					groundSpeed = 0;
					facingRight = true;
					frame = 0;
					//runTappingSound.stop();
					break;
				}
				else if( (currInput.LDown() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ))
					|| (currInput.LUp() && ((gNorm.x < 0 && facingRight) || ( gNorm.x > 0 && !facingRight ) )) )
				{
					
					SetActionExpr( SPRINT );
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
					frame = frame * 4;

					//runTappingSound.stop();
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

			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

					break;
				}
			}

			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				holdDouble = true;
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

			

			AirAttack();

			break;
		}
	case JUMPSQUAT:
		{
			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				bufferedAttack = true;
			}

			if( frame == actionLength[JUMPSQUAT] - 1 )
			{
				SetActionExpr( JUMP );
				frame = 0;
				groundSpeed = storedGroundSpeed;
			}
		}
		break;
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

			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					action = AIRDASH;
					bounceFlameOn = false;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

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

			AirAttack();

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
						/*re->Reset();
						re1->Reset();*/
						action = DASH;
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
						frame = 0;
					}
					else if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						SetActionExpr( STAND );
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
						if( gNorm.x > 0 )
						{
							facingRight = true;
						}
						else
						{
							facingRight = false;
						}
						/*if( groundSpeed > 0 )
							facingRight = true;
						else if( groundSpeed < 0 )
							facingRight = false;
						else
						{
							if( g
						}*/
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
				}
				else
				{
					if( currInput.A && !prevInput.A )
					{
						SetActionExpr( JUMPSQUAT );
						bufferedAttack = false;
						frame = 0;
						////runTappingSound.stop();
						break;
					}

					if( currInput.B || !canStandUp )
					{
						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
						else
						{
							if( currInput.LDown() )
							{
								if( groundSpeed > 0 )//velocity.x > 0 )
								{
									facingRight = true;
								}
								else if( groundSpeed < 0 )//velocity.x < 0 )
								{
									facingRight = false;
								}
								else
								{
									if( gNorm.x > 0 )
									{
										facingRight = true;
									}
									else if( gNorm.x < 0 )
									{
										facingRight = false;
									}
								}
							}
							
						}
						
						action = DASH;
						frame = 0;
						/*re->Reset();
						re1->Reset();*/

						
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
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
						SetActionExpr( STAND );
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

				/*if( currInput.A )
				{
					longWallJump = true;
				}
				else
				{
					longWallJump = false;
				}*/
				//facingRight = !facingRight;
			}
			else if( currInput.A && !prevInput.A )
			{
				if( hasDoubleJump )
				{
					action = DOUBLE;
					frame = 0;
				}
			}
			else if( currInput.LDown() )
			{
				SetActionExpr( JUMP );
				frame = 1;
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				SetActionExpr( WALLATTACK );
				frame = 0;
			}


			break;
		}
	case WALLJUMP:
		{
			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					action = AIRDASH;
					airDashStall = false;

					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

					break;
				}
			}


			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				holdDouble = true;
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

		
			
			AirAttack();
			
			break;
		}
	case WALLATTACK:
		{
			//cout << "WALL ATTACK" << endl;
			if( frame > 6 )
			{
			if( currInput.LDown() )
			{
				action = JUMP;
				frame = 1;
				break;
			}
			else
			{
				if( !facingRight )
				{
					if( currInput.LLeft() )
					{
						action = WALLJUMP;
						frame = 0;
						break;
					}
				}
				else
				{
					if( currInput.LRight() )
					{
						action = WALLJUMP;
						frame = 0;
						break;
					}
				}
			}
			}

			

			if( currAttackHit && frame > 0 )
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
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

					break;
				}
			}

			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				holdDouble = true;
				break;
			}

			}
			break;
		}
	case FAIR:
		{
			if( currAttackHit && frame > 0 )
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

			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

					break;
				}
			}

			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				holdDouble = true;
				break;
			}

			AirAttack();

			}
			break;
		}
	case DAIR:
		{
			if( currAttackHit && frame > 0 )
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

			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

					break;
				}
			}

			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				frame = 0;
				holdDouble = true;
				break;
			}

			AirAttack();
			}
			break;
		}
	case UAIR:
		{
			if( currAttackHit && frame > 0 )
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

			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}

					break;
				}
			}

			if( hasDoubleJump && currInput.A && !prevInput.A && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				holdDouble = true;
				frame = 0;
				break;
			}

			AirAttack();
			}
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
				//dashStartSound.setLoop( false );
				////runTappingSound.stop();
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
						//cout << "steep clzzzimb" << endl;
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
				SetActionExpr( JUMPSQUAT );
				bufferedAttack = false;
				frame = 0;
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				GroundAttack();
				break;
			}

			if( canStandUp )
			{
				if( !currInput.B )
				{
					if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
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
				//cout << "cant stand up" << endl;
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
				SetActionExpr( JUMPSQUAT );
				bufferedAttack = false;
				frame = 0;
				break;
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				action = STANDN;
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
						/*re->Reset();
						re1->Reset();*/
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
						else
						{
							if( gNorm.x > 0 && groundSpeed > 0 )
							{
								facingRight = true;
							}
							else if( gNorm.x < 0 && groundSpeed < 0 )
							{
								facingRight = false;
							}
							else
							{
								if( groundSpeed > 0 )//velocity.x > 0 )
								{
									facingRight = true;
								}
								else if( groundSpeed < 0 )//velocity.x < 0 )
								{
									facingRight = false;
								}
								else
								{
									if( gNorm.x > 0 )
									{
										facingRight = true;
									}
									else if( gNorm.x < 0 )
									{
										facingRight = false;
									}
								}
							}

						}
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
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
						//cout << "start dash" << endl;
						action = DASH;
						/*re->Reset();
						re1->Reset();*/
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
						else
						{
							if( gNorm.x > 0 && groundSpeed > 0 )
							{
								facingRight = true;
							}
							else if( gNorm.x < 0 && groundSpeed < 0 )
							{
								facingRight = false;
							}
							else
							{
								if( groundSpeed > 0 )//velocity.x > 0 )
								{
									facingRight = true;
								}
								else if( groundSpeed < 0 )//velocity.x < 0 )
								{
									facingRight = false;
								}
								else
								{
									if( gNorm.x > 0 )
									{
										facingRight = true;
									}
									else if( gNorm.x < 0 )
									{
										facingRight = false;
									}
								}
							}
						}
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
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
				SetActionExpr( JUMPSQUAT );
				bufferedAttack = false;
				frame = 0;
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				GroundAttack();
				break;
			}

			
			if( currInput.B && !prevInput.B )
			{
					action = DASH;
					/*re->Reset();
					re1->Reset();*/
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
						SetActionExpr( RUN );
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
						SetActionExpr( RUN );
					}

					groundSpeed = 0;
					facingRight = true;
					frame = 0;
					break;
				}
				else if( !( (currInput.LDown() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ))
					|| (currInput.LUp() && ((gNorm.x < 0 && facingRight) || ( gNorm.x > 0 && !facingRight ) )) ) )
				{
					SetActionExpr( RUN );
					frame = frame / 4;
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
			if( currAttackHit && frame > 0 )
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
					//dashStartSound.setLoop( false );
					////runTappingSound.stop();
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
							//cout << "steep clzzzimb" << endl;
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
					SetActionExpr( JUMPSQUAT );
					bufferedAttack = false;
					frame = 0;
					break;
				}

				if( currInput.rightShoulder && !prevInput.rightShoulder )
				{
					GroundAttack();
					break;
				}
			}
			break;
		}
	case DASHATTACK:
		{
			if( currAttackHit && frame > 0 )
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
					//dashStartSound.setLoop( false );
					////runTappingSound.stop();
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
							//cout << "steep clzzzimb" << endl;
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
					SetActionExpr( JUMPSQUAT );
					bufferedAttack = false;
					frame = 0;
					break;
				}

				if( currInput.rightShoulder && !prevInput.rightShoulder )
				{
					GroundAttack();
					break;
				}
			}
			break;
		}
	case STEEPSLIDEATTACK:
		{
			if( currAttackHit && frame > 0 )
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
					SetActionExpr( JUMPSQUAT );
					bufferedAttack = false;
					frame = 0;
					break;
				}

				if( currInput.rightShoulder && !prevInput.rightShoulder )
				{
					action = STEEPSLIDEATTACK;
					frame = 0;
					break;
				}

				if( currInput.B && !prevInput.B )
				//if( currInput.A && !prevInput.A )
				{
					if( gNorm.x < 0 && currInput.LRight() )
					{
						action = STEEPCLIMB;
						facingRight = true;
						groundSpeed = 10;
						frame = 0;
					}
					else if( gNorm.x > 0 && currInput.LLeft() )
					{
						action = STEEPCLIMB;
						facingRight = false;
						groundSpeed = -10;
						frame = 0;
					}
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
						cout << "is it really this wtf" << endl;
						action = LAND2;
						frame = 0;
						//not steep
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
			}
			break;
		}
	case STEEPCLIMBATTACK:
		{
			if( currAttackHit && frame > 0 )
			{
				
			}
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
							

							SetActionExpr( JUMP );
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

							double angle = GroundedAngle();

							owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
							owner->soundNodeList->ActivateSound( soundBuffers[S_GRAVREVERSE] );
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
				SetActionExpr( JUMPSQUAT );
				bufferedAttack = false;
				frame = 0;
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				action = STEEPSLIDEATTACK;
				frame = 0;
				break;
			}

			if( currInput.B && !prevInput.B )
			//if( currInput.A && !prevInput.A )
			{
				if( gNorm.x < 0 && currInput.LRight() )
				{
					action = STEEPCLIMB;
					facingRight = true;
					groundSpeed = 10;
					frame = 0;
				}
				else if( gNorm.x > 0 && currInput.LLeft() )
				{
					action = STEEPCLIMB;
					facingRight = false;
					groundSpeed = -10;
					frame = 0;
				}
				/*else
				{
					action = JUMPSQUAT;
					bufferedAttack = false;
					frame = 0;
				}*/
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
					cout << "is it really this wtf" << endl;
					action = LAND2;
					frame = 0;
					//not steep
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
				SetActionExpr( JUMP );
				frame = 1;
				
				if( rightWire->state == Wire::PULLING || leftWire->state == Wire::PULLING )
				{
				}
				else
				{
					velocity = V2d( 0, 0 );
				}
				break;
			}
			//else if( currInput.A && !prevInput.A && hasDoubleJump )
			else if( currInput.A && !prevInput.A && hasDoubleJump && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				action = DOUBLE;
				holdDouble = true;
				frame = 0;
				break;
			}

			AirAttack();
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

			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMPSQUAT );
				bufferedAttack = false;
				frame = 0;
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				action = STEEPCLIMBATTACK;
				frame = 0;
				break;
			}

			bool fallAway = false;
			if( reversed )
			{
			}
			else
			{
				if( facingRight )
				{
					if( currInput.LLeft() )
					{
						fallAway = true;
					}
				}
				else
				{
					if( currInput.LRight() )
					{
						fallAway = true;
					}
				}
			}
			
			if( fallAway )
			{
				SetActionExpr( JUMP );
				frame = 0;
				steepJump = true;
				break;
			}

			if( currInput.B && !prevInput.B && framesSinceClimbBoost > climbBoostLimit )
			{
				//cout << "climb" << endl;
				framesSinceClimbBoost = 0;
				double sp = 5;//jumpStrength + 1;//28.0;
				double extra = 5.0;
				if( gNorm.x > 0 )//&& currInput.LLeft() )
				{
					groundSpeed = std::min( groundSpeed - extra, -sp );
				}
				else if( gNorm.x < 0 )// && currInput.LRight() )
				{
					groundSpeed = std::max( groundSpeed + extra, sp );
				}
				/*else
				{
					
				}*/
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
					//cout << "blahzzz" << endl;
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
			
			break;
		}
	case GROUNDHITSTUN:
		{
			
			break;
		}
	case WIREHOLD:
		break;
	case BOUNCEAIR:
		{
			if( !currInput.X )
			{
				SetActionExpr( JUMP );
				frame = 1;
			}

			if( hasPowerAirDash && ( rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING ) )
			{
				if( ( hasAirDash || inBubble ) && !prevInput.B && currInput.B )
				{
					bounceFlameOn = false;
					action = AIRDASH;
					airDashStall = false;
					
					//special unlimited airdash
					if( inBubble && !hasAirDash )
					{
						frame = actionLength[AIRDASH] - 1;
					}
					else
					{
						frame = 0;
					}
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

			if( AirAttack() )
			{
				bounceFlameOn = true;
				airBounceFrame = 13 * 3;
			}
			break;
		}
	case BOUNCEGROUND:
		{
			if( !currInput.X )
			{


				SetActionExpr( JUMP );
				velocity = storedBounceVel;
				frame = 1;
				bounceFlameOn = false;
				bounceEdge = NULL;
				bounceMovingTerrain = NULL;
				break;
			}

			
			
			V2d bn = bounceNorm;//bounceEdge->Normal();
			bool framesDone = frame == actionLength[BOUNCEGROUND] - 1;
			if( boostBounce || (framesDone && bn.y >= 0 ) )
			{
				owner->soundNodeList->ActivateSound( soundBuffers[S_BOUNCEJUMP] );
				framesInAir = 0;
				action = BOUNCEAIR;
				oldBounceEdge = bounceEdge;
				oldBounceNorm  = bounceNorm;
				frame = 0;
				

				int option = 0; //0 is ground, 1 is wall, 2 is ceiling

				

				//double lenVel = length( storedBounceVel );
				//double reflX = cross( normalize( -storedBounceVel ), bn );
				//double reflY = dot( normalize( -storedBounceVel ), bn );
				//V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
				//V2d ref = normalize( reflX * edgeDir + reflY * bn ) * lenVel;

				//double c = cos( -currInput.leftStickRadians );
				//double s = sin( -currInput.leftStickRadians );
				//V2d left( c, s );

				//double dd = dot( ref, left );
				//double cc = cross( ref, left );

				////V2d ne( ref.x * c + ref.y * -s, ref.x * s + ref.y * c );
				//V2d eft = left + V2d( 0, -1 );
				//eft /= 2.0;
				//velocity = left * lenVel;
				
				
				
				
				
				//dot( ref, eft );//dd * edgeDir + cc * bn;
				//cout << "setting vel: " << velocity.x << ", " << velocity.y << endl;
				//double dd = dot( 
				//velocity = normalize( 

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
						
						
						if( storedBounceVel.x > 0 && currInput.LLeft() )
						{
							storedBounceVel.x = -dashSpeed;
						}
						else if( storedBounceVel.x < 0 && currInput.LRight() )
						{
							storedBounceVel.x = dashSpeed;
						}
						else if( storedBounceVel.x == 0 ) 
						{
							if( currInput.LLeft() )
							{
								storedBounceVel.x = -maxAirXControl;
							}
							else if( currInput.LRight() )
							{
								storedBounceVel.x = maxAirXControl;
							}
						}

						
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
						/*if( storedBounceVel.x > 0 && currInput.LLeft() )
						{
							storedBounceVel.x = -dashSpeed;
						}
						else if( storedBounceVel.x < 0 && currInput.LRight() )
						{
							storedBounceVel.x = dashSpeed;
						}
						else */
						if( storedBounceVel.x == 0 ) 
						{
							if( currInput.LLeft() )
							{
								storedBounceVel.x = -maxAirXControl;
							}
							else if( currInput.LRight() )
							{
								storedBounceVel.x = maxAirXControl;
							}
						}


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

				if( boostBounce && framesSinceBounce > 8 )
				{
					velocity += normalize( velocity ) * bounceBoostSpeed / (double)slowMultiple;
					boostBounce = false;
				}
				else if( boostBounce )
				{
					boostBounce = false;
				}
				framesSinceBounce = 0;

				//velocity = length( storedBounceVel ) * bounceEdge->Normal();
				//ground = NULL;
				bounceEdge = NULL;
				bounceMovingTerrain = NULL;

				//if( ground != NULL )
				//	ground = NULL;
			}
			else if( framesDone )
			{
				if( bn.y < 0 )
				{
					V2d alongVel = V2d( -bn.y, bn.x );
					ground = bounceEdge;
					edgeQuantity = bounceQuant;
					bounceEdge = NULL;
					bounceMovingTerrain = NULL;

					if( bn.y > -steepThresh )
					{
						
					}
					else
					{
					}
					action = LAND;
					frame = 0;


					V2d testVel = storedBounceVel;

				
					if( testVel.y > 20 )
					{
						testVel.y *= .7;
					}
					else if( testVel.y < -30 )
					{
				
						testVel.y *= .5;
					}

					if( currInput.LLeft() || currInput.LRight() || currInput.LDown() || currInput.LUp() )
					{
						groundSpeed = dot( testVel, alongVel );
					}
					else
					{
						if( gNorm.y > -steepThresh )
						{
							groundSpeed = dot( testVel, alongVel );
						}
						else
						{
							groundSpeed = 0;
						}
					}

					//normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
					//cout << "setting groundSpeed: " << groundSpeed << endl;
					//V2d gNorm = ground->Normal();//minContact.normal;//ground->Normal();
					gNorm = ground->Normal();

					//if( gNorm.y <= -steepThresh )
					{
						hasGravReverse = true;
						hasAirDash = true;
						hasDoubleJump = true;
						lastWire = 0;
					}

					if( testVel.x < 0 && gNorm.y <= -steepThresh )
					{
						groundSpeed = min( testVel.x, dot( testVel, normalize( ground->v1 - ground->v0 ) ) * .7);
						//cout << "left boost: " << groundSpeed << endl;
					}
					else if( testVel.x > 0 && gNorm.y <= -steepThresh )
					{
						groundSpeed = max( testVel.x, dot( testVel, normalize( ground->v1 - ground->v0 ) ) * .7 );
						//cout << "right boost: " << groundSpeed << endl;
					}

					
				}
				else
				{
					action = JUMP;
					frame = 1;
					velocity = storedBounceVel;
					bounceEdge = NULL;
					bounceMovingTerrain = NULL;
				}
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
				//runTappingSound.stop();
				break;
			}


			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMP );
				frame = 0;
				//runTappingSound.stop();
				break;
			}

			if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				GroundAttack();

				//runTappingSound.stop();
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
				//jumpSound.play();

				if( reversed )
				{
					if( bounceFlameOn )
						airBounceFrame = 13 * 3;
					//so you dont jump straight up on a nearly vertical edge
					double blah = .5;

					V2d dir( 0, 0 );
					if( (groundSpeed > 0 && gNorm.x > 0) || ( groundSpeed < 0 && gNorm.x < 0 ) )
					{
						/*if( groundSpeed > 0 )
						{
							dir = V2d( blah, 0 );
							cout << "bbb" << endl;
						}
						else
						{
							dir = V2d( blah, 0 );
							cout << "aaa" << endl;
						}*/
						
					}
					else if( ( groundSpeed > 0 && gNorm.x < 0 ) || ( groundSpeed < 0 && gNorm.x > 0 ) )
					{
						//cout << "this!" << endl;
						/*if( groundSpeed > 0 )
						{
							dir = V2d( 2, 0 );
						}
						else
						{
							dir = V2d( -2, 0 );
						}*/
						
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

					double blah = .25;

					V2d dir( 0, 0 );
					if( (groundSpeed > 0 && gNorm.x > 0) || ( groundSpeed < 0 && gNorm.x < 0 ) )
					{
						dir = V2d( blah, 0 );
						//dir = V2d( blah, 0 );
						if( groundSpeed > 0 )
						{
							cout << "right" << endl;
							
						}
						else
						{
							cout << "left" << endl;
							//dir = V2d( -blah, 0 );
							//dir = V2d( -blah, 0 );
						}
						
					}
					else if( ( groundSpeed > 0 && gNorm.x < 0 ) || ( groundSpeed < 0 && gNorm.x > 0 ) )
					{
						//cout << "this!" << endl;
						double bb = 0;
						//dir = V2d( bb, 0 );
						if( groundSpeed > 0 )
						{
							//dir = V2d( blah, 0 );
						}
						else
						{
							//dir = V2d( -blah, 0 );
						}
						
					}
					
					V2d trueNormal = normalize(dir + normalize(ground->v1 - ground->v0 ));
					velocity = groundSpeed * trueNormal;
					if( velocity.y < 0 )
					{
						velocity.y *= .7;
					}
					
					if( ( groundSpeed > 0 && gNorm.x < 0 ) || ( groundSpeed < 0 && gNorm.x > 0 ) )
					{
						//cout << "this!" << endl;
						if( groundSpeed > 0 )
						{
							//velocity.y /= 2;
							//dir = V2d( .7, 0 );
						}
						else
						{
							//velocity.y /= 2;
							//dir = V2d( -.7, 0 );
						}
						
					}

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

					if( steepJump )
					{
						velocity.y -= jumpStrength * .75;
					}
					else
					{
						velocity.y -= jumpStrength;
					}

					V2d pp = ground->GetPoint( edgeQuantity );
					double ang = GroundedAngle();
					V2d fxPos;
					if( (approxEquals( ang, 0 ) && !reversed ) || (approxEquals(ang, PI) && reversed ))
						fxPos = V2d( pp.x + offsetX, pp.y );
					else
						fxPos = pp;

					fxPos += gNorm * 16.0;
					
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_jump, fxPos , false, ang, 12, 2, facingRight );

					ground = NULL;
					movingGround = NULL;
					holdJump = true;

					
					//steepJump = false;
				}
				
			}

			
		}
		else
		{
			//if( bufferedAttack )
			//{
			//	bufferedAttack = false;
			//}
			//cout << "vel at beg: " << velocity.x << ", " << velocity.y << endl;
			CheckHoldJump();

			if( framesInAir > 1 || velocity.y < 0 )
				AirMovement();
			//cout << "vel at end: " << velocity.x << ", " << velocity.y << endl;
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
	case JUMPSQUAT:
		{
			if( frame == 0 )
			{
				storedGroundSpeed = groundSpeed;
				groundSpeed = 0;
			}
		}
		break;
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
				//double xDiff = 20;
				//if( facingRight )
				//	xDiff = -xDiff;
				//owner->ActivateEffect( ts_fx_wallJump, V2d( position.x + xDiff, position.y ), false, 0, 12, 4, facingRight );
				wallJumpFrameCounter = 0;
				double strengthX = wallJumpStrength.x;
				double strengthY = wallJumpStrength.y;

				if( !longWallJump )
				{
					strengthX = strengthX;
				}
				else
				{
					strengthY = strengthY;// + 3;
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

				owner->soundNodeList->ActivateSound( soundBuffers[S_WALLJUMP] );
			}
			else if( frame >= wallJumpMovementLimit )
			{
				AirMovement();
			}
			break;
		}
	case WALLATTACK:
		{
			//CheckHoldJump();

			//currHitboxes = fairHitboxes;
			if( wallHitboxes.count( frame ) > 0 )
			{
				currHitboxes = wallHitboxes[frame];
			}

			if( frame == 0 )
			{
				currAttackHit = false;
				owner->soundNodeList->ActivateSound( soundBuffers[S_WALLATTACK] );
			//	fairSound.play();
			}

			if( velocity.y > clingSpeed )
			{
				//cout << "running wallcling" << endl;
				velocity.y = clingSpeed;
			}
			AirMovement();

			//if( wallJumpFrameCounter >= wallJumpMovementLimit )
			//{
			//	//cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << endl;

			//	AirMovement();
			//}

			break;
		}
	case FAIR:
		{
			


			CheckHoldJump();

			//currHitboxes = fairHitboxes;
			if( fairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = fairHitboxes[frame];
			}

			if( frame == 0 && slowCounter == 1)
			{
				owner->soundNodeList->ActivateSound( soundBuffers[S_FAIR] );
				currAttackHit = false;
				//fairSound.play();
			}
			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{
				//cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << endl;

				AirMovement();
			}

			break;
		}
	case DAIR:
		{
			CheckHoldJump();

			if( dairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = dairHitboxes[frame];
			}

			if( frame == 0 && slowCounter == 1 )
			{
				owner->soundNodeList->ActivateSound( soundBuffers[S_DAIR] );
				currAttackHit = false;
			}


			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{		
				AirMovement();
			}
			break;
		}
	case UAIR:
		{
			CheckHoldJump();

			if( uairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = uairHitboxes[frame];
			}

			if( frame == 0 && slowCounter == 1)
			{
				owner->soundNodeList->ActivateSound( soundBuffers[S_UAIR] );
				currAttackHit = false;
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
				framesSinceDouble = 0;
			
				//add direction later
				owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_double, 
					V2d( position.x, position.y - 20), false, 0, 14, 2, facingRight );
			
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
				
				CheckHoldJump();
				
						
				AirMovement();
				//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;
			
			}
			break;
		}
	case SLIDE:
		{
			//double fac = gravity * 2.0 / 3;
			if( reversed )
			{
				groundSpeed += dot( V2d( 0, slideGravFactor * gravity), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				groundSpeed += dot( V2d( 0, slideGravFactor * gravity), normalize( ground->v1 - ground->v0 )) / slowMultiple;
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
			if( standHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standHitboxes[frame];
			}

			if( frame == 0 && slowCounter == slowMultiple )
			{
				owner->soundNodeList->ActivateSound( soundBuffers[S_STANDATTACK] );
				currAttackHit = false;
			}

			AttackMovement();
			break;

		}
	case STEEPCLIMBATTACK:
		{
			if( steepClimbHitboxes.count( frame ) > 0 )
			{
				currHitboxes = steepClimbHitboxes[frame];
			}

			if( frame == 0 )
			{
				currAttackHit = false;
			}

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

			//AttackMovement();
			break;

		}
	case STEEPSLIDEATTACK:
		{
			if( steepSlideHitboxes.count( frame ) > 0 )
			{
				currHitboxes = steepSlideHitboxes[frame];
			}

			if( frame == 0 )
			{
				currAttackHit = false;
			}

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
			//AttackMovement();
			break;

		}
	case DASHATTACK:
		{
			if( dashHitboxes.count( frame ) > 0 )
			{
				currHitboxes = dashHitboxes[frame];
			}

			if( frame == 0 )
			{
				currAttackHit = false;
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
					hasAirDash = false;
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

			//cout << "groundspeed: " << groundSpeed << endl;
			
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
			if( framesInAir > 8 ) //to prevent you from clinging to walls awkwardly
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
				//owner->soundNodeList->ActivateSound( soundBuffers[S_BOUNCEJUMP] );
				boostBounce = true;

				
				V2d bouncePos = bounceEdge->GetPoint( bounceQuant );
				V2d bn = bounceEdge->Normal();
				double angle = atan2( bn.x, -bn.y );
				bouncePos += bn * 80.0;
				owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bounceBoost, bouncePos, false, angle, 30, 1, facingRight );
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

		//if( velocity.y > 0 && velocity.y < 10 )
		//{
			//velocity += V2d( 0, gravity / slowMultiple * .6 );
		//	velocity += V2d( 0, gravity / slowMultiple * .3 );
		//}
		if( velocity.y >= maxFallSpeedSlow )
		{
			velocity += V2d( 0, gravity * .5 / slowMultiple );
		}
		else if( velocity.y < 0 )
		{
			velocity += V2d( 0, gravity * 1.2 / slowMultiple );
		}
		else if( abs( velocity.y ) < 4 && action != AIRDASH )
		{
			velocity += V2d( 0, gravity / slowMultiple );
			//velocity += V2d( 0, gravity / slowMultiple * .4 );
		}
		else
		{
			velocity += V2d( 0, gravity / slowMultiple );
		}

		if( velocity.y > maxFallSpeedFast )
			velocity.y = maxFallSpeedFast;
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
		//correction for momentum
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

		V2d totalAcc;
		totalAcc.x = totalVelDir.x * doubleWirePull / (double)slowMultiple;
		if( totalVelDir.y < 0 )
			totalAcc.y = totalVelDir.y * ( doubleWirePull + 1 )/ (double)slowMultiple;
		else
			totalAcc.y = totalVelDir.y * ( doubleWirePull )/ (double)slowMultiple;
		
		double beforeAlongAmount = dot( velocity, totalVelDir );

		if( beforeAlongAmount >= 20 )
		{
			totalAcc *= .5;
		}
		//if( length( velocity ) > 20.0 )
		//{
		//	totalAcc *= .5;
		//}
		//totalVel *= dot( totalVelDir, rightWire->
		velocity += totalAcc;

		double afterAlongAmount = dot( velocity, totalVelDir );
		double maxAlong = 45.0;

		if( afterAlongAmount > maxAlong )
		{
			velocity -= ( afterAlongAmount - maxAlong ) * totalVelDir;
		}
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

	if( toggleTimeSlowInput && !inBubble && oldInBubble )
	{
		currInput.leftShoulder = false;

		/*if( currInput.leftShoulder )
		{
			
		}*/
	}

	

	if( !inBubble && action == AIRDASH && airDashStall )
	{
	SetActionExpr( JUMP );
		frame = 1;
	}

	if( ( hasPowerTimeSlow && currInput.leftShoulder ) || cloneBubbleCreated )
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
				owner->soundNodeList->ActivateSound( soundBuffers[S_TIMESLOW] );
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
	

	//cout << "groundspeed: " << groundSpeed << endl;
	

	touchEdgeWithLeftWire = false;
	touchEdgeWithRightWire = false;
	oldAction = action;
	collision = false;
	groundedWallBounce = false;

	
	//if( ground == NULL )
	//cout << "final vel: " << velocity.x << ", " << velocity.y << endl;
	//cout << "before position: " << position.x << ", " << position.y << endl;
	
}

void Actor::SetAction( Action a )
{
	action = a;
	if( slowMultiple > 1 )
	{
		slowMultiple = 1;
	}
}

bool Actor::CheckWall( bool right )
{
	double wThresh = 8;
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
	
	/*queryMode = "moving_checkwall";
	for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
	{
		currMovingTerrain = (*it);
		(*it)->Query( this, r );
	}*/
	


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

		//if( (zero && en.x < 0 && en.y < 0 ) )
		//{
		//	//cout << "?>>>>>" << endl;
		//	V2d te = e0->v0 - e0->v1;
		//	if( te.x > 0 )
		//	{
		//		return true;
		//	}
		//}
		
		if( (one && en.x < 0 && en.y > 0 ) )
		{
			//cout << "%%%%%" << endl;
			V2d te = e1->v1 - e1->v0;
			if( te.x > 0 )
			{
				return true;
			}
		}

		/*if( (one && en.x < 0 && en.y < 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x < 0 )
			{
				return true;
			}
		}*/
		
		/*if( (zero && en.x > 0 && en.y < 0 ) )
		{
			V2d te = e0->v0 - e0->v1;
			if( te.x > 0 )
			{	
				return true;
			}
		}*/
	
		/*if( ( one && en.x > 0 && en.y < 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x < 0 )
			{
				return true;
			}
		}*/
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

		
		double ex = .1;
		Rect<double> r;

		/*if( action != GRINDBALL )
		{*/
		if( reversed )
		{
			r = Rect<double>( position.x + b.offset.x - b.rw - ex, position.y - ex/*+ b.offset.y*/ - normalHeight, 2 * b.rw + 2 * ex, 2 * normalHeight + 2 * ex);
		}
		else
		{
			r = Rect<double>( position.x + b.offset.x - b.rw - ex, position.y - ex /*+ b.offset.y*/ - normalHeight, 2 * b.rw + 2 * ex, 2 * normalHeight + 2 * ex);
		}
		//}
		//else
		//{
		//	V2d p = grindEdge->GetPoint( grindQuantity );
		//	if( reversed )
		//	{
		//		r = Rect<double>( p - b.rw - ex, position.y - ex/*+ b.offset.y*/ - normalHeight, 2 * b.rw + 2 * ex, 2 * normalHeight + 2 * ex);
		//	}
		//	else
		//	{
		//		r = Rect<double>( position.x + b.offset.x - b.rw - ex, position.y - ex /*+ b.offset.y*/ - normalHeight, 2 * b.rw + 2 * ex, 2 * normalHeight + 2 * ex);
		//	}
		//}

		queryMode = "check";
		checkValid = true;
	//	Query( this, owner->testTree, r );
		owner->terrainTree->Query( this, r );

		for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
		{
			(*it)->Query( this, r );
		}
		possibleEdgeCount = 0;

		/*if( checkValid )
			cout << "check valid" << endl;
		else
		{
			cout << "cant stand up" << endl;
		}*/
		return checkValid;

	}
	
}

bool Actor::ResolvePhysics( V2d vel )
{
//	cout << "vel: " << vel.x << ", " << vel.y << endl;
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

	//collision = false;
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


	//cout << "owner: " << owner << ", tree: " << owner->terrainTree << endl;
	testr = false;
	queryMode = "moving_resolve";
	for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
	{
		currMovingTerrain = (*it);
		(*it)->Query( this, r );
	}

	if( col )
	{
		storedContact = minContact;
		/*if( minContact.movingPlat != NULL )
		{
			velocity += minContact.movingPlat->vel;
		}*/
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

		owner->powerWheel->Damage( 1000000 );
	}

	queryMode = "item";
	owner->itemTree->Query( this, r );

	queryMode = "envplant";
	owner->envPlantTree->Query( this, r );

	//queryMode = "gate";
	//owner->testGateCount = 0;
	//owner->gateTree->Query( this, r );

	/*if( owner->testGateCount > 0 )
	{
		cout << "ON" << endl;
		action = DEATH;
		rightWire->Reset();
		leftWire->Reset();
		slowCounter = 1;
		frame = 0;
		owner->deathWipe = true;

		owner->powerBar.Damage( 1000000 );
	}*/

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

			//bool a = q == 0 && movement < 0;
			//bool c = ((gNormal.x == 0 && e0n.x == 0 )
			//	|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
			//	|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ));

			
			bool transferRight = q == groundLength && movement > 0 //&& (groundSpeed > steepClimbSpeedThresh || e1n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
			
			//cout << "a: " << a << " b: " << c << endl;
			if( transferLeft )
			{
				//cout << "transfer left" << endl;
				if( e0->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)e0->info;
					if( CanUnlockGate( g ) )
					{
						owner->UnlockGate( g );

						if( e0 == g->edgeA )
						{
							gateTouched = g->edgeB;
						//	owner->ActivateZone( g->zoneB );
						}
						else
						{
							gateTouched = g->edgeA;
						//	owner->ActivateZone( g->zoneA );
						}
					}
				}
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

						cout << "airborne 2" << endl;
						leftGround = true;
						SetActionExpr( JUMP );
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

									cout << "airborne 1" << endl;
									leftGroundExtra.y = .01;
									leftGroundExtra.x = .01;

									leftGround = true;
									SetActionExpr( JUMP );
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
					cout << "airborne 0" << endl;
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
					SetActionExpr( JUMP );
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
				if( e1->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)e1->info;
					if( CanUnlockGate( g ) )
					{
						owner->UnlockGate( g );

						if( e1 == g->edgeA )
						{
							gateTouched = g->edgeB;
						}
						else
						{
							gateTouched = g->edgeA;
						}
					}
				}

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
						SetActionExpr( JUMP );
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
									cout << "airborne 3" << endl;
									leftGround = true;
									SetActionExpr( JUMP );
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
					cout << "airborne 4" << endl;
					SetActionExpr( JUMP );
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

								if( e0->edgeType == Edge::CLOSED_GATE )
								{
									cout << "OPENING GATE HERE I THOUGHT THIS WASNT NECESSARY A" << endl;
									Gate *g = (Gate*)e0->info;

									if( CanUnlockGate( g ) )
									{
										//g->SetLocked( false );
										owner->UnlockGate( g );

										if( e0 == g->edgeA )
										{
											gateTouched = g->edgeB;
											//owner->ActivateZone( g->zoneB );
										}
										else
										{
											gateTouched = g->edgeA;
										//	owner->ActivateZone( g->zoneA );
										}

										offsetX = -offsetX;
										break;
									}

								}


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
							if( e0->edgeType == Edge::CLOSED_GATE )
							{
								Gate *g = (Gate*)e0->info;
								//g->SetLocked( false );

								if( CanUnlockGate( g ) )
								{
									owner->UnlockGate( g );

									if( e0 == g->edgeA )
									{
										gateTouched = g->edgeB;
										//owner->ActivateZone( g->zoneB );
									}
									else
									{
										gateTouched = g->edgeA;
										//owner->ActivateZone( g->zoneA );
									}

									offsetX = -offsetX;
									break;
								}

							}

							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}
							//cout << "xxxxxx" << endl;
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
								if( e1->edgeType == Edge::CLOSED_GATE )
								{
									cout << "OPENING GATE HERE I THOUGHT THIS WASNT NECESSARY B" << endl;
									Gate *g = (Gate*)e1->info;

									if( CanUnlockGate( g ) )
									{
										//g->SetLocked( false );
										owner->UnlockGate( g );

										if( e1 == g->edgeA )
										{
											gateTouched = g->edgeB;
											//owner->ActivateZone( g->zoneB );
										}
										else
										{
											gateTouched = g->edgeA;
											//owner->ActivateZone( g->zoneA );
										}

										offsetX = -offsetX;
										break;

									}

								}


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
							//attemping to fix reverse secret issues on gates
							if( e1->edgeType == Edge::CLOSED_GATE )
							{
								Gate *g = (Gate*)e1->info;

								if( CanUnlockGate( g ) )
								{
									//g->SetLocked( false );
									owner->UnlockGate( g );

									if( e1 == g->edgeA )
									{
										gateTouched = g->edgeB;
										//owner->ActivateZone( g->zoneB );
									}
									else
									{
										gateTouched = g->edgeA;
										//owner->ActivateZone( g->zoneA );
									}

									offsetX = -offsetX;
									break;
								}
							}

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

					V2d resMove = normalize( ground->v1 - ground->v0 ) * m;
					bool hit = ResolvePhysics( resMove );
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
								double sum = position.x + minContact.resolution.x;
								double diff = sum - minContact.position.x;
								//double test = position.x + minContact.resolution.x - minContact.position.x;
								/*cout << "pos: " << position.x << ", res: " << minContact.resolution.x
									<< ", minContact: " << minContact.position.x << endl;
								cout << "sum: " << sum << ", diff: " << diff << endl;
								cout << "test: " << test << endl;
								cout << "oldpos: " << oldPos.x << ", vel: " << resMove.x << endl;*/
								if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
								{
									//this is for corner border cases
									cout << "REVERSED CORNER BORDER CASE: " << test << endl;
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

void Actor::UpdateFullPhysics()
{
	possibleEdgeCount = 0;
	Rect<double> oldR( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	
	//position += vel;
	
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
	

	double ex = 5;//1
	Rect<double> r( minLeft - ex, minTop - ex, (maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2 );

	col = false;

	tempVel = V2d( 0, 0 );
	minContact.edge = NULL;

	

	Edge *oldGround = ground;
	double oldGs = groundSpeed;
	
	testr = true;
	queryMode = "moving_resolve";
	for( list<MovingTerrain*>::iterator it = owner->movingPlats.begin(); it != owner->movingPlats.end(); ++it )
	{
		currMovingTerrain = (*it);
		(*it)->Query( this, r );
	}

	if( col )
	{
		if( minContact.movingPlat != NULL )
		{

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

		//position += minContact.resolution;
		//cout << "ressssolve: " << minContact.resolution.x << ", " << minContact.resolution.y << endl;
	}

	if( col )
	{

		collision = true;			
		position += minContact.resolution;

		//if( minContact.movingPlat != NULL )
		//{
		//	//velocity += minContact.movingPlat->vel * NUM_STEPS;
		//}

		//cout << "resolving: " << minContact.resolution.x << ", " << minContact.resolution.y << endl;
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
				
		double blah = length( velocity ) - length( minContact.resolution );
		//cout << "blah: " << blah << endl;
		//wish i knew what this one meant
		//extraVel = dot( normalize( velocity ), extraDir ) * extraDir * length(minContact.resolution);
		//extraVel = (length( velocity ) - length( minContact.resolution )) * extraDir;
		if( dot( velocity, extraDir ) < 0 )
		{
			//extraVel = -extraVel;
		}

		//might still need some more work
		//extraVel = dot( normalize( velocity ), extraDir ) * length( minContact.resolution ) * extraDir;
				
		V2d newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );

		velocity = newVel;
				
		/*if( length( stealVec ) > 0 )
		{
			stealVec = length( stealVec ) * normalize( extraDir );
		}
		if( approxEquals( extraVel.x, lastExtra.x ) && approxEquals( extraVel.y, lastExtra.y ) )
		{
			break;		
		}
		if( length( extraVel ) > 0 )
		{
			lastExtra.x = extraVel.x;
			lastExtra.y = extraVel.y;
		}*/
		int maxJumpHeightFrame = 10;

		if( ((action == JUMP && /*!holdJump*/false) || ( framesInAir > maxJumpHeightFrame || velocity.y > 0 ) || action == WALLCLING || action == WALLATTACK ) && minContact.normal.y < 0 && abs( minContact.normal.x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
		{
			//if( minContact.movingPlat != NULL )
			//	minContact.position += minContact.movingPlat->vel;//normalize( minContact.edge->v1 - minContact.edge->v0 ) * dot( minContact.movingPlat->vel, normalize( minContact.edge->v1 - minContact.edge->v0 ) );


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

				//minContact.position += minContafct
			}

				

			edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				
			//edgeQuantity -= .01;
			//cout << "landing edge quantity is: " << edgeQuantity << ", edge length is: " << length( ground->v1 - ground->v0 ) << endl;

			if( movingGround != NULL )
			{

				//normalize( minContact.edge->v1 - minContact.edge->v0 ) * dot( minContact.movingPlat->vel, normalize( minContact.edge->v1 - minContact.edge->v0 ) );
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			V2d alongVel = V2d( -minContact.normal.y, minContact.normal.x );
				
			double groundLength = length( ground->v1 - ground->v0 );

			V2d gNorm = ground->Normal();

			V2d testVel = velocity;

			//might use those steep slope things again to make sure he doesnt climb too fast
			//maybe adjust these for frames in the air. don't let you touch the edge the second you jump
			//so that you let your velocity die down a little
			if( testVel.y > 20 )
			{
				testVel.y *= .7;
			}
			else if( testVel.y < -30 )
			{
				//testVel.y = -30;
				testVel.y *= .5;
			}
			//testVel.y /= 2.0
			//cout << "groundspeed: " << groundSpeed << endl;

			if( currInput.LLeft() || currInput.LRight() || currInput.LDown() || currInput.LUp() )
			{
				groundSpeed = dot( testVel, alongVel );
			}
			else
			{
				if( gNorm.y > -steepThresh )
				{
					groundSpeed = dot( testVel, alongVel );
				}
				else
				{
					groundSpeed = 0;
				}
			}

			//normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
			//cout << "setting groundSpeed: " << groundSpeed << endl;
			//V2d gNorm = ground->Normal();//minContact.normal;//ground->Normal();
			gNorm = ground->Normal();

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
				//cout << "left boost: " << groundSpeed << endl;
			}
			else if( velocity.x > 0 && gNorm.y <= -steepThresh )
			{
				groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ) * .7 );
				//cout << "right boost: " << groundSpeed << endl;
			}
			//groundSpeed  = max( abs( velocity.x ), ( - ) );
				
			if( velocity.x < 0 )
			{
			//	groundSpeed = -groundSpeed;
			}

			//cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << ", offset: " << offsetX << endl;

			//movement = 0;
			
				
				
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
			}

		PhysicsResponse();
	}

	

}

//int blah = 0;
void Actor::UpdatePhysics()
{
	if( action == INTRO || action == SPAWNWAIT || action == GOALKILL || action == EXIT || action == GOALKILLWAIT
		|| action == RIDESHIP )
		return;
	/*if( blah == 0 )
	{
		blah = 1;
		cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
	}
	else
	{
		blah = 0;
	}
	*/
	//cout << "before position: " << position.x << ", " << position.y << endl;
	
	//cout << "position: " << position.x << ", " << position.y << endl;
	if( movingGround != NULL )
	{
		position += movingGround->vel;// + normalize( movingGround->vel ) * .01;
	}
	else
	{
		//UpdateFullPhysics();
	}

	if( action == DEATH )
	{
		return;
	}

	if( test )
		return;
	
	

	double temp_groundSpeed = groundSpeed / slowMultiple;
	V2d temp_velocity = velocity / (double)slowMultiple;
	//cout << "temp velocity: " << temp_velocity.x << ", " << temp_velocity.y << endl;
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
		if( movement != 0 && abs( movement ) < .00001 )
		{
			//maybe here I should reduce the groundspeed to 0? 
			//i seemed to solve the weird teleportation/super fast movement
			//glitch from before but I'm still not quite sure how it works
			//you just get a huge movement value somehow from having a really
			//low groundspeed in a weird circumstance. hopefully it 
			//doesn't cause any more problems
			//happens mashing jump into a steep slope w/ an acute in-cut ceiling

			//cout << "what movement: " << movement << ", " << temp_groundSpeed << endl;
			return;
		}
		
	}
	else
	{
		movementVec = temp_velocity / NUM_STEPS;
		//cout << "movelength: " << moveLength << endl;
		//cout << "movevec: " << movementVec.x << ", " << movementVec.y << endl;
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
					
					
					V2d v0 = grindEdge->v0;
					V2d v1 = grindEdge->v1;
					

					if( e1->edgeType == Edge::CLOSED_GATE )
					{
						Gate *gg = (Gate*)e1->info;
						if( gg->gState == Gate::SOFT || gg->gState == Gate::SOFTEN )
						{
							if( CanUnlockGate( gg ) )
							{
								//cout << "unlock gate" << endl;
								owner->UnlockGate( gg );

								if( e1 == gg->edgeA )
								{
									gateTouched = gg->edgeB;

									//owner->ActivateZone( g->zoneB );
								}
								else
								{
									gateTouched = gg->edgeA;
									//owner->ActivateZone( g->zoneA );
								}
								
								e1 = grindEdge->edge1;
							}
						}
					}
					grindEdge = e1;
					q = 0;
					//sf::Rect<double> r( v0.x - 1, v0.y - 1, 2, 2 );
					//sf::Rect<double> r( ( v0.x + v1.x ) / 2 - 1, ( v0.y + v1.y ) / 2 - 1, 2, 2 );
					//queryMode = "gate";
					//owner->testGateCount = 0;
					//owner->gateTree->Query( this, r );

					//Gate *currg = owner->gateList;
					//while( currg != NULL )
					//{
					//	if( CanUnlockGate( currg ) )
					//	{
					//		cout << "unlock gate" << endl;
					//		owner->UnlockGate( currg );

					//		if( e0 == currg->edgeA )
					//		{
					//			gateTouched = currg->edgeB;

					//			//owner->ActivateZone( g->zoneB );
					//		}
					//		else
					//		{
					//			gateTouched = currg->edgeA;
					//			//owner->ActivateZone( g->zoneA );
					//		}
					//		//break;
					//	}
					//	currg = currg->next;
					//}
					
						/*action = DEATH;
						rightWire->Reset();
						leftWire->Reset();
						slowCounter = 1;
						frame = 0;
						owner->deathWipe = true;

						owner->powerBar.Damage( 1000000 );*/
						//return;
					
					
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
					/*queryMode = "gate";
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
					}*/

					
					//q = length( e0->v1 - e0->v0 );


					if( e0->edgeType == Edge::CLOSED_GATE )
					{
						Gate *gg = (Gate*)e0->info;
						if( gg->gState == Gate::SOFT || gg->gState == Gate::SOFTEN )
						{
							if( CanUnlockGate( gg ) )
							{
								//cout << "unlock gate" << endl;
								owner->UnlockGate( gg );

								if( e0 == gg->edgeA )
								{
									gateTouched = gg->edgeB;

									//owner->ActivateZone( g->zoneB );
								}
								else
								{
									gateTouched = gg->edgeA;
									//owner->ActivateZone( g->zoneA );
								}
								
								e0 = grindEdge->edge0;
							}
						}
					}
					grindEdge = e0;
					q = length( grindEdge->v1 - grindEdge->v0 );
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

	do
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
				if( ground->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)ground->info;
					if( g->edgeA == ground )
					{
						cout << "i am edgeA w/ edge0: ";
					}
					else
					{
						cout << "i am edgeB w/ edge0: ";
					}
					cout << ground->edge0 << ", ";

					if( ground->edge0 == g->temp0next )
					{
						cout << "temp0next" << endl;
					}
					else if( ground->edge0 == g->temp0prev )
					{
						cout << "temp0prev" << endl;
					}
					else if( ground->edge0 == g->temp1prev )
					{
						cout << "temp1prev" << endl;
					}
					else if( ground->edge0 == g->temp1prev )
					{
						cout << "temp1prev" << endl;
					}
				}
				//cout << "gNormal: " << gNormal.x << ", " << gNormal.y << ", edge0: " << ground->edge0->Normal().x 
				//	<< ", " << ground->edge0->Normal().y << endl;
				if( e0->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)e0->info;
					cout << "testing for unlock gate" << endl;
					if( CanUnlockGate( g ) )
					{
						cout << "unlock gate" << endl;
						owner->UnlockGate( g );

						if( e0 == g->edgeA )
						{
							gateTouched = g->edgeB;

							//owner->ActivateZone( g->zoneB );
						}
						else
						{
							gateTouched = g->edgeA;
							//owner->ActivateZone( g->zoneA );
						}
						//break;
					}
				}
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
							//cout << "tff" << endl;
							//cout << "steep transfer left" << endl;
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
					else if( gNormal.x > 0 && gNormal.y > -steepThresh )
					{
						ground = next;
						q = length( ground->v1 - ground->v0 );	
						//cout << "airborne 5" << endl;
					/*cout << "airborne 5" << endl;
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
						movingGround = NULL;*/
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
									//cout << "airborne 6" << endl;
									leftGround = true;
									SetActionExpr( JUMP );
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									movingGround = NULL;
								}
								else
								{
								//	cout << "this steep" << endl;
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
							//	cout << "this steepclimb" << endl;
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
					//cout << "airborne 7" << endl;
					//cout << "after vec: " << movementVec.x << ", " << movementVec.y << endl;
					leftGround = true;
					SetActionExpr( JUMP );
					frame = 1;
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					ground = NULL;
					movingGround = NULL;
				}
			}
			else if( transferRight )
			{
				if( e1->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)e1->info;

					if( CanUnlockGate( g ) )
					{
						owner->UnlockGate( g );

						if( e1 == g->edgeA )
						{
							gateTouched = g->edgeB;
							//owner->ActivateZone( g->zoneB );
						}
						else
						{
							gateTouched = g->edgeA;
							//owner->ActivateZone( g->zoneA );
						}
					}
				}
			//	cout << "transferRight!" << endl;
				double yDist = abs( gNormal.x ) * groundSpeed;
				Edge *next = ground->edge1;
				if( next->Normal().y < 0 && abs( e1n.x ) < wallThresh 
					&& !(currInput.LUp() && /*!currInput.LRight() &&*/ gNormal.x < 0 
					&& yDist > slopeLaunchMinSpeed && next->Normal().x >= 0 ) )
				{
					if( e1n.x < 0 && e1n.y > -steepThresh )
					{
						if( groundSpeed <= steepClimbSpeedThresh && action != STEEPCLIMB )
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							ground = next;
							q = 0;
					//		cout << "steep transfer right" << endl;
						}
					}
					else if( gNormal.x < 0 && gNormal.y > -steepThresh )
					{
						ground = next;
						q = 0;
					//	cout << "airborne 13" << endl;
						//cout << "leave right 1" << endl;
						/*velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;

						movementVec.y -= .01;
						if( movementVec.x <= .01 )
						{
							movementVec.x = .01;
						}
						cout << "airborne 8" << endl;
						leftGround = true;
						action = JUMP;
						frame = 1;
						rightWire->UpdateAnchors( V2d( 0, 0 ) );
						leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;
						movingGround = NULL;*/
						
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
						//			cout << "airborne 9" << endl;
									leftGround = true;
									SetActionExpr( JUMP );
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									movingGround = NULL;

								}
								else
								{
						//			cout << "this steep 1" << endl;
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
						//		cout << "this here??" << endl;
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
					//cout << "air because wall" << endl;
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
					//cout << "airborne 10" << endl;
					SetActionExpr( JUMP );
					frame = 1;
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					//break;
					//cout << "leaving ground RIGHT!!!!!!!!" << endl;
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//cout << "co: " << changeOffset << endl;
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

						//cout << "checking for airborne" << endl;
						
						
						double yDist = abs( gNormal.x ) * groundSpeed;
						if( gNormal.x < 0 
							&& e1n.y < 0 
							&& abs( e1n.x ) < wallThresh 
							&& e1n.x >= 0 
							&& yDist > slopeLaunchMinSpeed
							&& currInput.LUp() )
						{

							velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;

							movementVec.y -= .01;
							if( movementVec.x <= .01 )
							{
								movementVec.x = .01;
							}
							cout << "real slope jump A" << endl;
							leftGround = true;
							SetActionExpr( JUMP );
							frame = 1;
							rightWire->UpdateAnchors( V2d( 0, 0 ) );
							leftWire->UpdateAnchors( V2d( 0, 0 ) );
							ground = NULL;
							movingGround = NULL;

							break;
						}
						else
						{
							if( gNormal.x < 0 && gNormal.y > -steepThresh && e1n.x >= 0
								&& abs( e1n.x ) < wallThresh && groundSpeed > 5 )
							{
								velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
								movementVec = normalize( ground->v1 - ground->v0 ) * extra;

								movementVec.y -= .01;
								if( movementVec.x <= .01 )
								{
									movementVec.x = .01;
								}
								//why did i put these in again? from steep slope right
								cout << "real slope jump D" << endl;
								leftGround = true;
								SetActionExpr( JUMP );
								frame = 1;
								rightWire->UpdateAnchors( V2d( 0, 0 ) );
								leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = NULL;
								movingGround = NULL;
								break;
							}
							else
							{
								q = groundLength;
							}
							
						}
						//here is where i really lift off
					}
					else
					{
						double yDist = abs( gNormal.x ) * groundSpeed;
						if( gNormal.x > 0 
							&& e1n.y < 0 
							&& abs( e1n.x ) < wallThresh 
							&& e1n.x <= 0 
							&& yDist > slopeLaunchMinSpeed 
							&& currInput.LUp() )
						{

							velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;

							movementVec.y -= .01;
							if( movementVec.x <= .01 )
							{
								movementVec.x = .01;
							}
							cout << "real slope jump B" << endl;
							leftGround = true;
							SetActionExpr( JUMP );
							frame = 1;
							rightWire->UpdateAnchors( V2d( 0, 0 ) );
							leftWire->UpdateAnchors( V2d( 0, 0 ) );
							ground = NULL;
							movingGround = NULL;
							break;
						}
						else
						{
							if( gNormal.x > 0 && gNormal.y > -steepThresh && e0n.x <= 0
								&& abs( e0n.x ) < wallThresh && groundSpeed < -5 )
							{
								velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
								movementVec = normalize( ground->v1 - ground->v0 ) * extra;

								movementVec.y -= .01;
								if( movementVec.x <= .01 )
								{
									movementVec.x = .01;
								}
								cout << "real slope jump C" << endl;
								leftGround = true;
								SetActionExpr( JUMP );
								frame = 1;
								rightWire->UpdateAnchors( V2d( 0, 0 ) );
								leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = NULL;
								movingGround = NULL;
								break;
							}
							else
							{
								q = 0;
								//cout << "setting 0-1" << endl;
							}
							
							
						}
						
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

					//cout << "secret1: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX << ", " << groundSpeed <<  endl;

					if( groundSpeed > 0 )
					{
						
						Edge *next = ground->edge1;
						double yDist = abs( gNormal.x ) * groundSpeed;
						if( next->Normal().y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && next->Normal().x >= 0 ) )
						{
							if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
							{

								if( e1->edgeType == Edge::CLOSED_GATE )
								{
									//cout << "similar secret but not reversed B" << endl;
									Gate *g = (Gate*)e1->info;
									//g->SetLocked( false );
									if( CanUnlockGate( g ) )
									{
										owner->UnlockGate( g );

										if( e1 == g->edgeA )
										{
											gateTouched = g->edgeB;
											//owner->ActivateZone( g->zoneB );
										}
										else
										{
											gateTouched = g->edgeA;
											//owner->ActivateZone( g->zoneA );
										}

										break;
									}

								}

								groundSpeed = 0;
								break;
							}
							else
							{
								//ground = next;
								//q = 0;
							//	cout << "possible bug. solved secret left??" << endl;
								break;
							}
					
						}
						else if( abs( e1n.x ) >= wallThresh )
						{
							//cout << "right wall" << endl;
							if( e1->edgeType == Edge::CLOSED_GATE )
							{
							//	cout << "similar secret but not reversed A" << endl;
								Gate *g = (Gate*)e1->info;

								if( CanUnlockGate( g ) )
								{
									//g->SetLocked( false );
									owner->UnlockGate( g );

									if( e1 == g->edgeA )
									{
										gateTouched = g->edgeB;
										//owner->ActivateZone( g->zoneB );
									}
									else
									{
										gateTouched = g->edgeA;
										//owner->ActivateZone( g->zoneA );
									}

									break;
								}
							}


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
							//cout << "LEFT GROUND" << endl;
							velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
							leftGround = true;
							ground = NULL;
							movingGround = NULL;
						//	cout << "leaving ground RIGHT!!!!!!!!" << endl;
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
								if( e0->edgeType == Edge::CLOSED_GATE )
								{
							//		cout << "similar secret but not reversed C" << endl;
									Gate *g = (Gate*)e0->info;

									if( CanUnlockGate( g ) )
									{
										//g->SetLocked( false );
										owner->UnlockGate( g );

										if( e0 == g->edgeA )
										{
											gateTouched = g->edgeB;
											//owner->ActivateZone( g->zoneB );
										}
										else
										{
											gateTouched = g->edgeA;
											//owner->ActivateZone( g->zoneA );
										}

										break;
									}

								}

								groundSpeed = 0;
								break;
							}
							else
							{
						//		cout << "possible bug. solved secret??" << endl;
								//ground = next;
								//q = length( ground->v1 - ground->v0 );	
								break;
							}
						}
						else if( abs( e0n.x ) >= wallThresh )
						{
							if( e0->edgeType == Edge::CLOSED_GATE )
								{
							//		cout << "similar secret but not reversed D" << endl;
									Gate *g = (Gate*)e0->info;
									//g->SetLocked( false );

									if( CanUnlockGate( g ) )
									{
										owner->UnlockGate( g );

										if( e0 == g->edgeA )
										{
											gateTouched = g->edgeB;
											//owner->ActivateZone( g->zoneB );
										}
										else
										{
											gateTouched = g->edgeA;
											//owner->ActivateZone( g->zoneA );
										}

										break;
									}
								}

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
						//	cout << "not sure leaving ground left" << endl;
							movingGround = NULL;
						}
					}
				}

				//only want this to fire if secret doesn't happen
				else//if( !approxEquals( m, 0 ) )
				{	
					bool down = true;
					V2d oldPos = position;
					
					V2d resMove = normalize( ground->v1 - ground->v0 ) * m;
					bool hit = ResolvePhysics( resMove );

					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						//cout << "totally hit" << endl;
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
									//double test = position.x + b.offset.x - minContact.resolution.x - minContact.position.x;
									//double test = position.x + minContact.resolution.x - minContact.position.x;
									double test = position.x + minContact.resolution.x - minContact.position.x;
									/*cout << "pos: " << position.x << ", res: " << minContact.resolution.x
										<< ", minContact: " << minContact.position.x << endl;;
									cout << "test: " << test << endl;
									cout << "oldpos: " << oldPos.x << ", vel: " << resMove.x << endl;*/
									if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
									{
										//corner border case. hope it doesn't cause problems
										cout << "CORNER BORDER CASE: " << test << endl;
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
									else
									{	
										
										///cout << "cxxxx" << endl;
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

										//hopefully this doesn't cause any bugs. if it does i know exactly where to find it

										//CHANGED OFFSET
										offsetX = position.x + minContact.resolution.x - minContact.position.x;
										//offsetX = position.x - minContact.position.x;
										//cout << "offsetX is now: " << offsetX << endl;

									}

									/*if( offsetX < -b.rw || offsetX > b.rw )
									{
										cout << "BROKEN OFFSET: " << offsetX << endl;
										assert( false && "T_T" );
									}*/
								}
								else
								{
									//cout << "xx" << endl;

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
								//if( currInput.LUp() && testVel.y < -offSlopeByWallThresh && eNorm.y == 0 )

								//might cause some weird stuff w/ bounce but i can figure it out later
								if( testVel.y < -offSlopeByWallThresh && eNorm.y == 0 && !bounceFlameOn )
								{
									assert( abs(eNorm.x ) > wallThresh );
							//		cout << "testVel: " << testVel.x << ", " << testVel.y << endl;
									velocity = testVel;
						
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
									leftGround = true;
									ground = NULL;
									movingGround = NULL;
							//		cout << "airborne 11" << endl;
									SetActionExpr( JUMP );
									frame = 1;
									rightWire->UpdateAnchors( V2d( 0, 0 ) );
									leftWire->UpdateAnchors( V2d( 0, 0 ) );
								}
								else
								{
									if( bounceFlameOn && abs( groundSpeed ) > 1)
									{
										if( action != STEEPCLIMB )
										{
											storedBounceGroundSpeed = groundSpeed * slowMultiple;
											groundedWallBounce = true;
										}
										
									}

									//cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;
									//if( action != STEEPCLIMB )
									if( action != STEEPCLIMB || minContact.position == minContact.edge->v0 )
									{
									

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

								//	cout << "this case?: " << eNorm.x << ", " << eNorm.y << endl;
								
									groundSpeed = 0;
									edgeQuantity = q;
									}
									else //STEEPCLIMB
									{
										velocity = testVel;
						
										//movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
										//leftGround = true;
										
										//cout << "airborne 12" << endl;
										SetActionExpr( JUMPSQUAT );
										frame = 1;
										
										rightWire->UpdateAnchors( V2d( 0, 0 ) );
										leftWire->UpdateAnchors( V2d( 0, 0 ) );
										break;

									}
								}
								break;
							}
						}
						else
						{
					//		cout << "Sdfsdfd" << endl;

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
				//cout << "stealVecset: " << stealVec.x << ", " << stealVec.y << ", movelength: "
				//	<< moveLength << endl;
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

				//if( minContact.movingPlat != NULL )
				//{
				//	//velocity += minContact.movingPlat->vel * NUM_STEPS;
				//}

				
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
				
				double blah = length( velocity ) - length( minContact.resolution );
				//cout << "blah: " << blah << endl;
				//wish i knew what this one meant
				//extraVel = dot( normalize( velocity ), extraDir ) * extraDir * length(minContact.resolution);
				//extraVel = (length( velocity ) - length( minContact.resolution )) * extraDir;
				if( dot( velocity, extraDir ) < 0 )
				{
					//extraVel = -extraVel;
				}

				//might still need some more work
				extraVel = dot( normalize( velocity ), extraDir ) * length( minContact.resolution ) * extraDir;
				
				//extraVel = V2d( 0, 0 );
				newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );
				//if( length( newVel ) < .0001 )
				//{
				//	//velocity = V2d( 0, 0 );
				//	break;
				//}
				//if( length( extraVel ) < .0001 )
				//{
				//	extraVel = V2d( 0, 0 ); //weird bug fix?
				//}
				//cout << "newVel: " << newVel.x << ", " << newVel.y << ", extraVel: "
				//	<< extraVel.x << ", " << extraVel.y << endl;
				
				
				//if( minContact.movingPlat != NULL )
				//{
				//	if( dot( newVel, normalize( minContact.movingPlat->vel ) ) < minContact.movingPlat->speed )
				//	{
				//		V2d temp = newVel;
				//		newVel = minContact.movingPlat->vel;
				//		newVel.y = temp.y;
				//	}
				//	//newVel += minContact.movingPlat->vel;
				//}
				//cout << "vel: " << velocity.x << ", " << velocity.y << endl;
				//cout << "newvel: " << newVel.x << ", " << newVel.y << endl;
				//newVel = extraDir * length( velocity );//extraVel;
				//newVel = dot( normalize( velocity ), extraDir ) * length( minContact.resolution ) * extraDir;
				//extraVel = newVel;
				
				if( length( stealVec ) > 0 )
				{
					stealVec = length( stealVec ) * normalize( extraDir );
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

			//note: when reversed you won't cancel on a jump onto a small ceiling. i hope this mechanic is okay
			//also theres a jump && false condition that would need to be changed back
			if( tempCollision && minContact.normal.y >= 0 )
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
					SetActionExpr( JUMP );
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

					offsetX = ( position.x + b.offset.x ) - minContact.position.x;
					
					/*if( b.rh == doubleJumpHeight )
					{
						b.offset.y = (normalHeight - doubleJumpHeight);
					}*/

					if( b.rh < normalHeight )
					{
						if( minContact.normal.y > 0 )
							b.offset.y = -(normalHeight - b.rh);
						else if( minContact.normal.y < 0 )
							b.offset.y = (normalHeight - b.rh);
					}
					else
					{
						b.offset.y = 0;
					}

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
						SetActionExpr( JUMP );
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
			else if( ((action == JUMP && /*!holdJump*/false) || ( framesInAir > maxJumpHeightFrame || velocity.y > 0 ) || action == WALLCLING || action == WALLATTACK ) && tempCollision && minContact.normal.y < 0 && abs( minContact.normal.x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			{
				if( minContact.movingPlat != NULL )
				{
					//minContact.position += minContact.movingPlat->vel * minContact.collisionPriority;//(1 -minContact.collisionPriority );
					minContact.position -= minContact.movingPlat->vel;
				}
				//	minContact.position += minContact.movingPlat->vel;//normalize( minContact.edge->v1 - minContact.edge->v0 ) * dot( minContact.movingPlat->vel, normalize( minContact.edge->v1 - minContact.edge->v0 ) );


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

				/*if( b.rh == doubleJumpHeight )
				{
					b.offset.y = (normalHeight - doubleJumpHeight);
				}*/

				if( b.rh < normalHeight )
				{
					if( minContact.normal.y > 0 )
						b.offset.y = -(normalHeight - b.rh);	
					else if( minContact.normal.y < 0 )
						b.offset.y = (normalHeight - b.rh);
				}
				else
				{
					b.offset.y = 0;
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

					//minContact.position += minContafct
				}

				

				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				
				//edgeQuantity -= .01;
				//cout << "landing edge quantity is: " << edgeQuantity << ", edge length is: " << length( ground->v1 - ground->v0 ) << endl;

				if( movingGround != NULL )
				{

					//normalize( minContact.edge->v1 - minContact.edge->v0 ) * dot( minContact.movingPlat->vel, normalize( minContact.edge->v1 - minContact.edge->v0 ) );
					ground->v0 = oldv0;
					ground->v1 = oldv1;
				}

				V2d alongVel = V2d( -minContact.normal.y, minContact.normal.x );
				
				double groundLength = length( ground->v1 - ground->v0 );

				V2d gNorm = ground->Normal();

				V2d testVel = velocity;

				//testVel.y *= .7;
				if( testVel.y > 20 )
				{
					testVel.y *= .7;
				}
				else if( testVel.y < -30 )
				{
					//testVel.y = -30;
					testVel.y *= .5;
				}
				//testVel.y /= 2.0
				//cout << "groundspeed: " << groundSpeed << endl;

				if( currInput.LLeft() || currInput.LRight() || currInput.LDown() || currInput.LUp() )
				{
					groundSpeed = dot( testVel, alongVel );
				}
				else
				{
					if( gNorm.y > -steepThresh )
					{
						groundSpeed = dot( testVel, alongVel );
					}
					else
					{
						groundSpeed = 0;
					}
				}

				//normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				//cout << "setting groundSpeed: " << groundSpeed << endl;
				//V2d gNorm = ground->Normal();//minContact.normal;//ground->Normal();
				gNorm = ground->Normal();

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
					//cout << "left boost: " << groundSpeed << endl;
				}
				else if( velocity.x > 0 && gNorm.y <= -steepThresh )
				{
					groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ) * .7 );
					//cout << "right boost: " << groundSpeed << endl;
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
				/*if( b.rh == doubleJumpHeight )
				{
					b.offset.y = (normalHeight - doubleJumpHeight);
				}*/

				if( b.rh < normalHeight )
				{
					b.offset.y = -(normalHeight - b.rh);
					/*if( minContact.normal.y > 0 )
						
					else if( minContact.normal.y < 0 )
						b.offset.y = (normalHeight - b.rh);*/
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

				//b.offset.y = -b.offset.y;
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

				//if( reversed )
				//{
				owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
				owner->soundNodeList->ActivateSound( soundBuffers[S_GRAVREVERSE] );
				//}
			}
			else if( tempCollision && hasPowerGrindBall && action == AIRDASH && currInput.Y && velocity.y != 0 && abs( minContact.normal.x ) >= wallThresh  )
			{
				Edge *e = minContact.edge;
				V2d mp = minContact.position;
				double q = e->GetQuantity( mp );
				ground = e;
				edgeQuantity = q;

				if( e->Normal().x > 0 )
				{
					groundSpeed = velocity.y;
				}
				else
				{
					groundSpeed = -velocity.y;
				}
				
				SetActionGrind();
				//if( hasPowerGrindBall && currInput.Y //&& !prevInput.Y
				//	&& action == AIRDASH && length( wallNormal ) > 0 )
				//{
				//	//assert( minContact.edge != NULL );
				//	Edge *e = storedContact.edge;
				//	V2d mp = storedContact.position;
				//	double q = e->GetQuantity( mp );

				//	

				//	//cout << "grinding" << endl;
				//}
			}
			else if( tempCollision )
			{
				if( minContact.movingPlat != NULL )
				{
					//velocity = newVel + minContact.movingPlat->vel;
				//	minContact.position -= minContact.movingPlat->vel;
				}
				//else
				{
					
					velocity = newVel;
					
				}
				
				
				

			}
			else
			{
				//cout << "no temp collision" << endl;
			}

			if( length( extraVel ) > 0 )
			{
				movementVec = stealVec + extraVel;
				//cout << "steal: " << stealVec.x << ", " << stealVec.y << endl;
				//cout << "movement: " << movementVec.x << ", " << movementVec.y 
				//	<< ", steal: " << stealVec.x << ", " << stealVec.y << ", extra: "
				//	<< extraVel.x << ", " << extraVel.y << endl;
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
	while( (ground != NULL && !approxEquals( movement, 0 ) ) || ( ground == NULL && length( movementVec ) > 0 ) );
	/*if( ground == NULL )
	{
		cout << "not grounded now" << endl;
	}*/

	
	PhysicsResponse();
}

void Actor::GroundAttack()
{
	SetAction( STANDN );
	frame = 0;
	//if( currInput.B )//action == DASH )
	//{
	//	
	//}
	//else
	//{
	//	action = DASHATTACK;
	//	frame = 0;
	//}
}

void Actor::PhysicsResponse()
{
	//velocity *= (double)slowMultiple;
	//groundSpeed *= slowMultiple;
	//grindSpeed *= slowMultiple;


	V2d gn;
	//Edge *e;
	if( grindEdge != NULL )
	{
		//e = grindEdge;
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
		//e = bounceEdge;
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
				hasGravReverse = true;
				hasDoubleJump = true;
				hasAirDash = true;
				lastWire = 0;
				if( storedBounceVel.x > 0 && bn.x < 0 && facingRight 
					|| storedBounceVel.x < 0 && bn.x > 0 && !facingRight )
				{
					facingRight = !facingRight;
				}
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				if( storedBounceVel.x > 0 && bn.x < 0 && facingRight 
					|| storedBounceVel.x < 0 && bn.x > 0 && !facingRight )
				{
					facingRight = !facingRight;
				}
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
		//e = ground;
		bool leaveGround = false;
		if( ground->edgeType == Edge::CLOSED_GATE )
		{
			Gate *g = (Gate*)ground->info;
				
			if( CanUnlockGate( g ) )
			{
				if( ground == g->edgeA )
				{
					gateTouched = g->edgeB;
				}
				else
				{
					gateTouched = g->edgeA;
				}

				owner->UnlockGate( g );

				SetActionExpr( JUMP );
				frame = 1;
				
				framesInAir = 0;

				velocity = groundSpeed * normalize( ground->v1 - ground->v0 );
				//velocity = V2d( 0, 0 );
				leaveGround = true;
				ground = NULL;
				//return;

			}
		}
		/*if( ground->edgeType == Edge::OPEN_GATE )
		{
			cout << "SETTING TO JUMP" << endl;
			action = JUMP;
			frame = 1;
			ground = NULL;
			framesInAir = 0;
		}*/

		if( !leaveGround )
		{
		framesInAir = 0;
		gn = ground->Normal();
		if( collision )
		{
			if( action == AIRHITSTUN )
			{
				//cout << "setting to ground hitstun!!!" << endl;
				action = GROUNDHITSTUN;
				frame = 0;
			}
			else if( action != GROUNDHITSTUN && action != LAND2 && action != LAND )
			{
				//cout << "Action: " << action << endl;
				if( currInput.LLeft() || currInput.LRight() )
				{
					//cout << "blahaaa" << endl;
					action = LAND2;
					rightWire->UpdateAnchors(V2d( 0, 0 ));
					leftWire->UpdateAnchors(V2d( 0, 0 ));
					frame = 0;
				}
				else
				{
					//cout << "blahaaa" << endl;
					//cout << "blahbbb" << endl;
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
				cout << "here no: " << action << ", " << offsetX << endl;
				action = LAND2;
				frame = 0;
			}
			else
			{

			}
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

		if( action != AIRHITSTUN && action != WALLATTACK && action != AIRDASH )
		{
			//oldAction = action;
			if( collision )
			{
				//if( hasPowerGrindBall && currInput.Y //&& !prevInput.Y
				//	&& action == AIRDASH && length( wallNormal ) > 0 )
				//{
				//	//assert( minContact.edge != NULL );
				//	Edge *e = storedContact.edge;
				//	V2d mp = storedContact.position;
				//	double q = e->GetQuantity( mp );

				//	ground = e;
				//	groundSpeed = velocity.y;
				//	SetActionGrind();

				//	//cout << "grinding" << endl;
				//}
				if( length( wallNormal ) > 0 && oldVelocity.y >= 0 )
				{
					if( wallNormal.x > 0)
					{
						if( currInput.LLeft() && !currInput.LDown() )
						{
							//cout << "setting to wallcling" << endl;
							facingRight = true;
							action = WALLCLING;
							frame = 0;
						}
					}
					else
					{
						if( currInput.LRight() && !currInput.LDown() )
						{
							//cout << "setting to wallcling" << endl;
							facingRight = false;
							action = WALLCLING;
							frame = 0;
						}
					
					}
				}
			}
			
			else if( oldAction == WALLCLING || oldAction == WALLATTACK )
			{
				bool stopWallClinging = false;
				if( collision && length( wallNormal ) > 0 )
				{
					if( wallNormal.x > 0 )
					{
						if( !currInput.LLeft() || currInput.LDown() )
						{
							stopWallClinging = true;
						}
					}
					else
					{
						if( !currInput.LRight() || currInput.LDown() )
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
					SetActionExpr( JUMP );
					frame = 1;
				}
			}

			if( leftGround )
			{
				SetActionExpr( JUMP );
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

	if( grindEdge != NULL )
	{
		//cout << "blah grind: " << grindEdge << endl;
	}
	if( gateTouched != NULL )
	{
		Edge *edge = gateTouched;
		Gate *g = (Gate*)gateTouched->info;


		V2d A( b.globalPosition.x - b.rw, b.globalPosition.y - b.rh );
		V2d B( b.globalPosition.x + b.rw, b.globalPosition.y - b.rh );
		V2d C( b.globalPosition.x + b.rw, b.globalPosition.y + b.rh );
		V2d D( b.globalPosition.x - b.rw, b.globalPosition.y + b.rh );
		V2d nEdge = normalize( edge->v1 - edge->v0 );

		double crossA = cross( A - edge->v0, nEdge );
		double crossB = cross( B - edge->v0, nEdge );
		double crossC = cross( C - edge->v0, nEdge );
		double crossD = cross( D - edge->v0, nEdge );

		bool activate = crossA > 0 && crossB > 0 && crossC > 0 && crossD > 0;
		 

		g->SetLocked( true );

		if( grindEdge != NULL )
		{
			Edge *otherEdge;
			if( edge == g->edgeA )
				otherEdge = g->edgeB;
			else
				otherEdge = g->edgeA;

			/*cout << "grindEdge: " << grindEdge << ", e1: " << grindEdge->edge1
				<<", e0: : " << grindEdge->edge0 << endl;
			cout << "edge: " << edge << ", edge1: " << edge->edge1 << ", edge0: "
				<< edge->edge0 << endl;
			cout << "other: " << otherEdge << ", oth1: :" << otherEdge->edge1 << ", oth0: " << otherEdge->edge0 << endl;*/
			if( grindEdge == edge->edge0 || grindEdge == edge->edge1 )
			{
				//this could be flawed. needs more testing

				activate = true;
			}
		}

		if( ground != NULL 
			&& ground != gateTouched 
			&& ( ( groundSpeed > 0 && ground->edge0 == gateTouched )
			|| ( groundSpeed < 0 && ground->edge1 == gateTouched ) ) )
		{
			//glitch here because you are actually grounded ON the gate, so this doesnt work out.
			activate = true;
		}
		else
		{
			//cout << "groundSpeed: " << groundSpeed << ", " << 
		}
		g->SetLocked( false );

		if( activate )
		{
			//lock all the gates from this zone now that I chose one
			
			



			owner->SuppressEnemyKeys( g->type );			

			if( edge == g->edgeA )
			{
				if( g->zoneB != NULL && g->zoneB->active )
				{
					list<Edge*> &zGates = g->zoneB->gates;
					for( list<Edge*>::iterator it = zGates.begin(); it != 
						zGates.end(); ++it )
					{
						Gate *og = (Gate*)(*it)->info;
						if( g == og )
							continue;
						if( (og->gState == Gate::HARD
							|| og->gState == Gate::SOFT
							|| og->gState == Gate::HARDEN
							|| og->gState == Gate::SOFTEN ) )
						{
							og->gState = Gate::LOCKFOREVER;
						}
					}
				}
				owner->ActivateZone( g->zoneA );
			}
			else
			{
				if( g->zoneA != NULL && g->zoneA->active )
				{
					list<Edge*> &zGates = g->zoneA->gates;
					for( list<Edge*>::iterator it = zGates.begin(); it != 
						zGates.end(); ++it )
					{
						Gate *og = (Gate*)(*it)->info;
						if( g == og )
							continue;
						if( (og->gState == Gate::HARD
							|| og->gState == Gate::SOFT
							|| og->gState == Gate::HARDEN
							|| og->gState == Gate::SOFTEN ) )
						{
							og->gState = Gate::LOCKFOREVER;
						}
					}
				}
				owner->ActivateZone( g->zoneB );
			}
			//cout << "clear!---------------------------------" << endl;
			
			if( g->reformBehindYou )
			{
				//Gate::GateState::
				owner->LockGate( g );
				
				g->gState = Gate::REFORM;
				g->frame = 0;
				//cout << "LOCKING BEHIND YOU" << endl;
			}
			else
			{
				g->gState = Gate::DISSOLVE;
				g->frame = 0;
			}


			
			
			//for( int i = 0; i < owner->numGates; ++i )
			//{
			//	Gate *og = owner->gates[i];
			//	if( g == og )
			//		continue;
			//	else
			//	{
			//		if( og->keyGate && (og->gState == Gate::HARD
			//			|| og->gState == Gate::SOFT
			//			|| og->gState == Gate::HARDEN
			//			|| og->gState == Gate::SOFTEN )
			//			&& (og->zoneA != NULL && og->zoneA->active)
			//				|| (og->zoneB != NULL && og->zoneB->active ) )
			//		{
			//			og->gState = Gate::LOCKFOREVER;
			//			//frame = 0;
			//			//frame = 0;
			//		}

			//	}
			//}

			

			

			/*while( owner->unlockedGateList != NULL )
			{
				owner->LockGate( gList );

			}*/
			//g->type = Gate::BLUE;
			//



			//set gate action to disperse
			//maybe have another gate action when you're on the gate and its not sure whether to blow up or not
			//it only enters this state if you already unlock it though
			gateTouched = NULL;
		}
		else if( crossA < 0 && crossB < 0 && crossC < 0 && crossD < 0 )
		{
			gateTouched = NULL;
			owner->LockGate( g );
			cout << "went back" << endl;
		}
		else
		{
			//cout << "not clear" << endl;
		}
	}

	//only for motion ghosts
	//UpdateSprite();

	if( ghostSpacingCounter == motionGhostSpacing )
	{
		ghostSpacingCounter = 0;
		for( int i = MAX_MOTION_GHOSTS-1; i > 0; --i )
		{
			motionGhosts[i] = motionGhosts[i-1];
		}
		motionGhosts[0] = *sprite;
	}
	else
	{
		ghostSpacingCounter++;
	}
}

void Actor::UpdateHitboxes()
{
	double angle = 0;
	V2d gn;
	V2d gd; 

	if( grindEdge != NULL )
	{
		gn = grindEdge->Normal();
		gd = normalize( grindEdge->v1 - grindEdge->v0 );
		angle = atan2( gn.x, -gn.y );
	}
	else if( ground != NULL )
	{	
		if( !approxEquals( abs(offsetX), b.rw ) )
		{
			gn = V2d( 0, -1 );
			gd = V2d( 1, 0 );
			if( reversed )
			{
				cout << "BLAH BLAH" << endl;
				angle = PI;
				gn = V2d( 0, 1 );
				gd = V2d( -1, 0 );
			}
			//this should never happen
		}
		else
		{
			gn = ground->Normal();
			angle = atan2( gn.x, -gn.y );
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
		//hurtBody.isCircle = true;
		hurtBody.rw = 10;
		hurtBody.rh = 10;
		//hurtBody.offset = 
	}
	else
	{
		if( action == DASH )
		{
			hurtBody.rh = dashHeight;
		}
		else if( action == SPRINT )
		{
			hurtBody.rh = sprintHeight;
		}
		else if( action == STEEPCLIMB )
		{
			hurtBody.rh = 12;
		}
		else if( action == DOUBLE )
		{
			hurtBody.rh = doubleJumpHeight;
		}
		else
		{
			hurtBody.rh = normalHeight;//15;
		}

		hurtBody.isCircle = false;
		hurtBody.rw = 7;
		
		
		hurtBody.offset = b.offset;
	}

	
	if( grindEdge != NULL )
	{
		hurtBody.globalPosition = grindEdge->GetPoint( grindQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
		hurtBody.globalAngle = angle;
	}
	else if( ground != NULL )
	{
		if( gn.x == 0 )
		{
			hurtBody.globalPosition = position + hurtBody.offset ;//+ V2d( 0, -hurtBody.rh );
			hurtBody.globalAngle = angle;
		}
		else if( gn.y > -steepThresh && !reversed )
		{
			double xoff = 0;
			if( gn.x > 0 )
			{
				xoff = 10;
			}
			else if( gn.x < 0 )
			{
				xoff = -10;
			}

			
			//might not work reversed
			hurtBody.globalPosition = ground->GetPoint( edgeQuantity ) + V2d( 0, -1 ) * (double)(b.rh) + V2d( xoff, 0 );// + hurtBody.rh );

			if( action == STEEPCLIMB )
			{
			}
			else if( action == STEEPSLIDE )
			{
				hurtBody.globalPosition += V2d( 0, 15 );
			}
			hurtBody.globalAngle = 0;
			//hurtBody.offset = V2d( 0, 0 );
		}
		else if( -gn.y > -steepThresh && reversed )
		{
			double xoff = 0;
			if( gn.x > 0 )
			{
				xoff = 10;
			}
			else if( gn.x < 0 )
			{
				xoff = -10;
			}

			
			//might not work reversed
			hurtBody.globalPosition = ground->GetPoint( edgeQuantity ) + V2d( 0, 1 ) * (double)(b.rh) + V2d( xoff, 0 );// + hurtBody.rh );

			if( action == STEEPCLIMB )
			{
			}
			else if( action == STEEPSLIDE )
			{
				hurtBody.globalPosition -= V2d( 0, 15 );
			}
			hurtBody.globalAngle = PI;
		}
		else
		{
			hurtBody.globalPosition = ground->GetPoint( edgeQuantity ) + gn * (double)(b.rh);// + hurtBody.rh );
			hurtBody.globalAngle = angle;
		}
	}
	else
	{
		hurtBody.globalPosition = position + hurtBody.offset;
		hurtBody.globalAngle = angle;
	}
	

	
	
	
	//cout << "hurtbody offset: " << hurtBody.offset.x << ", " << hurtBody.offset.y << endl;
	
	//if( ground != NULL )
	//{
	//	hurtBody.globalPosition = ground->GetPoint( edgeQuantity ) + gn * (double)( b.rh + 5 );
	//	if( angle == 0 || approxEquals( angle, PI ) )
	//	{
	//	//	hurtBody.globalPosition.x += offsetX;
	//	}
	//	hurtBody.globalAngle = angle;
	//}
	//else
	//{
	//	hurtBody.globalPosition = position;
	//	hurtBody.globalAngle = 0;
	//}

	
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
	//			hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	//hurtBody.globalPosition = position;

	b.globalPosition = position + b.offset;
	b.globalAngle = 0;
		
}

void Actor::UpdatePostPhysics()
{
	
	//cout << "action: " << action << endl;
	test = false;



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
	{
		gn = ground->Normal();
	}

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

		if( frame % 1 == 0 )
		{
			double startRadius = 64;
			double endRadius = 500;

			double part = frame / 88.f;

			double currRadius = startRadius * ( 1.f - part ) + endRadius * ( part );

			int randAng = (rand() % 360);
			double randAngle = randAng / 180.0 * PI;
			//randAngle += PI / 2.0;
			V2d pos( sin( randAngle ) * currRadius, -cos( randAngle ) * currRadius );

			pos += position;
			double fxAngle = randAngle - PI / 3.5;
			//cout << "randAngle: " << randAngle << endl;

			//cout << "randang: " << randAng << endl;
			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_death_1c, pos, false, fxAngle, 12, 2, true );
		}
		//if( slowCounter == slowMultiple )
		//{
	
		int an = 6;
		int extra = 24;
		if( frame < 11 * an + extra && frame >= extra )
		{
			int f = (frame - extra) / an;
			expr = (Expr)(Expr_DEATH + f);
		}
	


		kinFace.setTextureRect( ts_kinFace->GetSubRect( expr + 4 ) );


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
		p.showSword = false;
	}

	/*if( action != DASH && dashStartSound.getStatus() == Sound::Playing )
	{
		dashStartSound.stop();
	}*/
	/*switch( oldAction )
	{
	case DASH:
		{
			if( action != DASH )
			{
				dashStartSound.stop();
			}
		}
	}*/

	if( hitGoal && action != GOALKILL && action != EXIT && action != GOALKILLWAIT )
	{
		SetActionExpr( GOALKILL );
		frame = 0;
		position = owner->goalNodePos;
		rightWire->Reset();
		leftWire->Reset();
		desperationMode = false;
	}

	UpdateSprite();

	switch( action )
	{
	case AIRDASH:
		{
			bool horizontal = abs( velocity.y ) < .21;
			bool stopped = horizontal && velocity.x == 0;
			if( frame % 1 == 0 && !stopped )
			{
				//owner->ActivateEffect( ts_fx_airdashSmall, V2d( position.x, position.y + 0 ), false, 0, 12, 4, facingRight );
			}

			if( frame % 4 == 0 )
			{
				if( stopped )
				{
					if( frame % 4 == 0 )
						owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashHover, V2d( position.x, position.y + 70 ), false, 0, 12, 4, facingRight );
				}
				else if( horizontal )
				{
					//cout << "STUFF???" << endl;
					if( velocity.x > 0 )
					{
						owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashUp, V2d( position.x - 64, position.y - 18 ), false, PI / 2.0, 15, 3, true );
					}
					else
					{
						owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashUp, V2d( position.x + 64, position.y - 18 ), false, -PI / 2.0, 15, 3, true );
					}
				}
				else if( velocity.x == 0 && velocity.y < 0 )
				{
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashUp, V2d( position.x, position.y + 64 ), false, 0, 15, 3, facingRight );
				}
				else if( velocity.x == 0 && velocity.y > 0 )
				{
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashUp, V2d( position.x, position.y + 0 ), false, PI, 15, 3, facingRight );
				}
				else if( velocity.x > 0 && velocity.y > 0 )
				{
					V2d pos = V2d( position.x - 40, position.y - 60 );
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, pos, false, PI, 15, 3, true );//facingRight );
				}
				else if( velocity.x < 0 && velocity.y > 0 )
				{
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, V2d( position.x + 40, position.y - 60 ), false, PI, 15, 3, false );//facingRight );
				}
				else if( velocity.x < 0 && velocity.y < 0 )
				{
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, V2d( position.x + 54, position.y + 60 ), false, 0, 15, 3, true );
				}
				else if( velocity.x > 0 && velocity.y < 0 )
				{
					owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, V2d( position.x - 54, position.y + 60 ), false, 0, 15, 3, false );
				}
				
				//cout << "airdash fx" << endl;
				
			}

			
		}
		break;
	}

	double speed;
	if( ground != NULL ) //ground
	{
		speed = abs(groundSpeed);
	}
	else //air
	{
		speed = length( velocity );
	}

	if( action != DEATH && action != EXIT && action != GOALKILL
		&& action != GOALKILLWAIT && action != RIDESHIP )
	{
	if( speed > currentSpeedBar )
	{
		currentSpeedBar += speedChangeUp;
		if( currentSpeedBar > speed )
			currentSpeedBar = speed;//currentSpeedBar * (1.0 -fUp) + speed * fUp;
	}
	else if( speed < currentSpeedBar )
	{
		currentSpeedBar -= speedChangeDown;
		if( currentSpeedBar < speed )
		{
			currentSpeedBar = speed;
		}
		//currentSpeedBar = currentSpeedBar * (1.0 -fDown) + speed * fDown;
	}

	if( currentSpeedBar >= level2SpeedThresh )
	{
		speedLevel = 2;
	}
	else if( currentSpeedBar >= level1SpeedThresh )
	{
		speedLevel = 1;
	}
	else
	{
		speedLevel = 0;
	}

	if( speedParticleCounter == speedParticleRate )
	{
		if( speedLevel == 1 )
		{
			Tileset *tset = NULL;
			int randTex = rand() % 3;
			if( randTex == 0 )
				tset = ts_fx_chargeBlue0;
			else if( randTex == 1 )
				tset = ts_fx_chargeBlue1;
			else
				tset = ts_fx_chargeBlue2;

			int rx = 30;
			int ry = 30;

			if( ground != NULL )
			{
				
				double angle = GroundedAngle();
				V2d groundPos = ground->GetPoint( edgeQuantity );
				
				V2d truePos =  groundPos + gn * normalHeight;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d( randx, randy );

				

				owner->ActivateEffect( EffectLayer::IN_FRONT, tset, truePos, false, angle, 6, 3, facingRight );
			}
			else
			{
				V2d truePos = position;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d( randx, randy );
				double angle = atan2( gn.x, gn.y );
				owner->ActivateEffect( EffectLayer::IN_FRONT, tset, truePos, false, angle, 6, 3, facingRight );
			}
		
		}
		else if( speedLevel == 2 )
		{
			int rx = 30;
			int ry = 30;
			if( ground != NULL )
			{
				V2d groundPos = ground->GetPoint( edgeQuantity );
				V2d truePos =  groundPos + gn * normalHeight;//.0;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d( randx, randy );
				double angle = GroundedAngle();
				
				owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_chargePurple, truePos, false, angle, 6, 3, facingRight );
			}
			else
			{
				V2d truePos = position;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d( randx, randy );
				double angle = atan2( gn.x, gn.y );
				owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_chargePurple, truePos, false, angle, 6, 3, facingRight );
			}
		}

		speedParticleCounter = 0;
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
	if( action == FAIR || action == UAIR || action == DAIR || action == STANDN || action == DASHATTACK )
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

	rotaryAngle = sprite->getRotation() / 180 * PI;

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
		framesSinceDouble++;



		if( action == BOUNCEAIR && oldBounceEdge != NULL )
		{
			framesSinceBounce++;
		}

		++frame;
		//cout << "frame: " << frame << endl;

		++framesSinceClimbBoost;
		++speedParticleCounter;
		

		slowCounter = 1;

		if( invincibleFrames > 0 )
			--invincibleFrames;

		if( flashFrames > 0 )
			--flashFrames;
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


	

	
	
	//double transitionFramesUp = 60 * 3;
	//double transitionFramesDown = 60 * 3;
	//double fUp = 1.0 / transitionFramesUp;
	//double fDown = 1.0 / transitionFramesDown;

	

	if( action == DASH )
	{
		/*re->Update( position );	
		re1->Update( position );*/
	}

	pTrail->Update( position );

	

	

	CircleShape cs;
	cs.setFillColor( Color::Transparent );
	switch( speedLevel )
	{
	case 0:
		cs.setOutlineColor( Color::Blue );
		cs.setOutlineThickness( 10 );
		cs.setRadius( 32 );
		break;
	case 1:
		cs.setOutlineColor( Color::Magenta );
		cs.setOutlineThickness( 10 );
		cs.setRadius( 64 );
		break;
	case 2:
		cs.setOutlineColor( Color::Green );
		cs.setOutlineThickness( 10 );
		cs.setRadius( 128 );
		break;
	}
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	speedCircle = cs;

	if( expr == Expr_NEUTRAL || expr == Expr_SPEED1 || expr == Expr_SPEED2 )
	{
		switch( speedLevel )
		{
		case 0:
			expr = Expr_NEUTRAL;
			break;
		case 1:
			expr = Expr_SPEED1;
			break;
		case 2:
			expr = Expr_SPEED2;
			break;
		}
		//despCounter == maxDespFrames )
		if( desperationMode )
		{
			expr = Expr_DESP;
		}
	}


	kinFace.setTextureRect( ts_kinFace->GetSubRect( expr + 4 ) );


	followerPos += followerVel;

	V2d testVel = position - followerPos;
	if( ground != NULL )
	{
		//testVel = groundSpeed * normalize( ground->v1 - ground->v0 );
	}
	followerVel = followerVel * ( 1 - followerFac ) + testVel * followerFac;

	if( ground != NULL ) //doesn't work when grinding or bouncing yet
	{
		velocity = normalize( ground->v1 - ground->v0) * groundSpeed;
	}

	if( action == EXIT && frame == actionLength[EXIT] )
		owner->goalDestroyed = true;
	/*switch( expr )
	{
	case Expr::NEUTRAL:
		{
			kinFace.setTextureRect( ts_kinFace->GetSubRect( Expr::NEUTRAL ) );
		}
		break;
	case Expr::HURT:
		{
			kinFace.setTextureRect( ts_kinFace->GetSubRect( Expr::NEUTRAL ) );
		}
		break;
	}*/

	//playerLight->pos.x = position.x;
	//playerLight->pos.y = position.y;
}

void Actor::SetActivePowers(
		bool p_canAirDash,
		bool p_canGravReverse,
		bool p_canBounce,
		bool p_canGrind,
		bool p_canTimeSlow,
		bool p_canWire )
{
	hasPowerAirDash = p_canAirDash;
	hasPowerGravReverse = p_canGravReverse;
	hasPowerBounce = p_canBounce;
	hasPowerGrindBall = p_canGrind;
	hasPowerTimeSlow = p_canTimeSlow;
	hasPowerRightWire = p_canWire;
	hasPowerLeftWire = p_canWire;
}

void Actor::HandleEntrant( QuadTreeEntrant *qte )
{
	Edge *e = (Edge*)qte;

	if( e->edgeType == Edge::OPEN_GATE )
	{
		return;
	}

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
		
		V2d temp0prev = e->edge0->v0;
		V2d temp1prev = e->edge0->v1;

		V2d temp0next = e->edge1->v0;
		V2d temp1next = e->edge1->v1;

		//e->v0 += currMovingTerrain->oldPosition;
		//e->v1 += currMovingTerrain->oldPosition;

		V2d oldv0 = e->v0 + currMovingTerrain->oldPosition;
		V2d oldv1 = e->v1 + currMovingTerrain->oldPosition;

		e->v0 += currMovingTerrain->position;
		e->v1 += currMovingTerrain->position;

		e->edge0->v0 += currMovingTerrain->position;
		e->edge0->v1 += currMovingTerrain->position;

		e->edge1->v0 += currMovingTerrain->position;
		e->edge1->v1 += currMovingTerrain->position;

		//e->v0 += currMovingTerrain->position;
		//e->v1 += currMovingTerrain->position;

		if( e->Normal().y == -1 )
		{
			//cout << "testing the ground!: " << e->v0.x << ", " << e->v0.y << " and " <<
			//	e->v1.x << ", " << e->v1.y << endl;
		
		}
		V2d blah( tempVel - currMovingTerrain->vel );
		//cout << "tempnew: " << blah.x << ", " << blah.y << endl;
		//Contact *c = owner->coll.collideEdge( position + b.offset - currMovingTerrain->vel / NUM_STEPS, b, e, tempVel - currMovingTerrain->vel/ NUM_STEPS, V2d( 0, 0 ) );//currMovingTerrain->vel );
		//Contact *c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS /*+ normalize( currMovingTerrain->vel ) * 1.0*/, b, e, tempVel - currMovingTerrain->vel/ NUM_STEPS - normalize( currMovingTerrain->vel ) * 1.0, V2d( 0, 0 ) );//currMovingTerrain->vel );
		Contact *c = NULL;
		//if( true )
		double oldLen = length( position - oldv0 );
		double newLen = length( position - e->v0 );

		double oldLen1 = length( position - oldv1 );
		double newLen1 = length( position - e->v1 );

		V2d en = e->Normal();
		//cout << "oldLen: " << oldLen << ", newLen: " << newLen << endl;
		//if( ground != NULL )
		//{
		//	c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS, b, e, tempVel - currMovingTerrain->vel / NUM_STEPS- normalize( currMovingTerrain->vel ) * 1.0, V2d( 0, 0 ) );
		//}
		double d = dot( normalize( currMovingTerrain->vel ), en );

		if( !testr )
		if( ground != NULL )
		{
			//return;
			c = owner->coll.collideEdge( position + b.offset, b, e, tempVel, V2d( 0, 0 ) );
		}
		else
		{
			if( d >= 0 )
			{
				c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel + normalize( currMovingTerrain->vel ) * .2, 
					b, e, tempVel - currMovingTerrain->vel - normalize( currMovingTerrain->vel ) * .5 , V2d( 0, 0 ) );
			}
			else
			{
				c = owner->coll.collideEdge( position + b.offset, b, e, tempVel, V2d( 0, 0 ) );//currMovingTerrain->vel );	
			}

			/*c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel, 
				b, e, tempVel - currMovingTerrain->vel, V2d( 0, 0 ) );*/



			//c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel + normalize( currMovingTerrain->vel ) * .2, 
			//	b, e, tempVel - currMovingTerrain->vel - normalize( currMovingTerrain->vel ) * .5 , V2d( 0, 0 ) );
		}

		if( testr  )
		{
		
		if( d >= 0 )//oldLen >= newLen || oldLen1 >= newLen1 )
		{
			c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel + normalize( currMovingTerrain->vel ) * .2, 
				b, e, tempVel - currMovingTerrain->vel - normalize( currMovingTerrain->vel ) * .5 , V2d( 0, 0 ) );//currMovingTerrain->vel );	
			//if( ground != NULL )
			//{
			//	cout << "a: " << d << endl;
			//	//c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS, b, e, tempVel, V2d( 0, 0 ) );//currMovingTerrain->vel );
			//	//c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS + normalize( currMovingTerrain->vel ) * 1.0, b, e, tempVel, V2d( 0, 0 ) );
			//	c = owner->coll.collideEdge( position + b.offset, b, e, tempVel + currMovingTerrain->vel, V2d( 0, 0 ) );//currMovingTerrain->vel );	
			//}
			//else
			//{
			//	cout << "b: " << d << endl;
			//	
			//}
			
			
		}
		//else if( ground != NULL )
		//{
		//	//cout << "b" << endl;
		//	//c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS /*- normalize( currMovingTerrain->vel ) * .1*/, b, e, tempVel - currMovingTerrain->vel/ NUM_STEPS - normalize( currMovingTerrain->vel ) * 1.0, V2d( 0, 0 ) );//currMovingTerrain->vel );
		//	c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS, b, e, tempVel, V2d( 0, 0 ) );
		//}
		else
		{
			//cout << "c: " << d << endl;
			c = owner->coll.collideEdge( position + b.offset, b, e, tempVel, V2d( 0, 0 ) );//currMovingTerrain->vel );
			//if( ground != NULL )
			//{
			//	c = owner->coll.collideEdge( position + b.offset + currMovingTerrain->vel / NUM_STEPS, b, e, tempVel, V2d( 0, 0 ) );//currMovingTerrain->vel );
			//}
			//else
			//{
			//	
			//}
			
		}
		}

		 
		//Contact *c = owner->coll.collideEdge( position + b.offset, b, e, tempVel - currMovingTerrain->vel / NUM_STEPS, V2d( 0, 0 ) );//currMovingTerrain->vel );

		e->v0 = temp0;
		e->v1 = temp1;

		e->edge0->v0 = temp0prev;
		e->edge0->v1 = temp1prev;
		e->edge1->v0 = temp0next;
		e->edge1->v1 = temp1next;

		//if( c != NULL )
		//{
		//	cout << "moving resolve!: " << e->Normal().x << ", " << e->Normal().y << endl;
		//}

		if( c != NULL )	//	|| minContact.collisionPriority < -.001 && c->collisionPriority >= 0 )
		{	
			//cout << "col" << endl;
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
				//if( testr )
				//{
				//	if( e->Normal().y >= 0 ) //not ground
				//	{
				//		return;
				//	}
				//}
				//cout << "SURFACE. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << ", pri: " << c->collisionPriority << endl;
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


			//position += c->movingPlat->vel;
			//position += c->resolution;
			if( !col || (minContact.collisionPriority < 0 ) 
				|| (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
				
				//position += minContact.resolution;
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
			else
			{
				//position += minContact.resolution;
				//position += minContact.resolution;
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

		//so you can run on gates without transfer issues hopefully
		if( ground != NULL && ground->edgeType == Edge::CLOSED_GATE )
		{
			Gate *g = (Gate*)ground->info;
			Edge *edgeA = g->edgeA;
			Edge *edgeB = g->edgeB;
			if( ground == g->edgeA )
			{
				if( e == edgeB->edge0 
					|| e == edgeB->edge1
					|| e == edgeB )
				{
					return;
				}

				
			}
			else if( ground == g->edgeB )
			{
				if( e == edgeA->edge0 
					|| e == edgeA->edge1
					|| e == edgeA )
				{
					return;
				}
			}
		}
		else if( ground != NULL )
		{
			if( groundSpeed > 0 )
			{
				if( ground->edge0->edgeType == Edge::CLOSED_GATE )
				{
					Gate *g = (Gate*)ground->edge0->info;
					Edge *e0 = ground->edge0;
					if( e0 == g->edgeA )
					{
						Edge *edgeB = g->edgeB;
						if( e == edgeB->edge0 
							|| e == edgeB->edge1
							|| e == edgeB )
						{
							return;
						}
					}
					else if( e0 == g->edgeB )
					{
						Edge *edgeA = g->edgeA;
						if( e == edgeA->edge0 
							|| e == edgeA->edge1
							|| e == edgeA )
						{
							return;
						}
					}
				}
			}
			else if( groundSpeed < 0 )
			{
				if( ground->edge1->edgeType == Edge::CLOSED_GATE )
				{
					Gate *g = (Gate*)ground->edge1->info;
					Edge *e1 = ground->edge1;
					if( e1 == g->edgeA )
					{
						Edge *edgeB = g->edgeB;
						if( e == edgeB->edge0 
							|| e == edgeB->edge1
							|| e == edgeB )
						{
							return;
						}
					}
					else if( e1 == g->edgeB )
					{
						Edge *edgeA = g->edgeA;
						if( e == edgeA->edge0 
							|| e == edgeA->edge1
							|| e == edgeA )
						{
							return;
						}
					}
				}
			}
		}

		Contact *c = owner->coll.collideEdge( position + b.offset , b, e, tempVel, V2d( 0, 0 ) );

		if( c != NULL )
		{
			//cout << "c isnt null: " << e->Normal().x << ", " << e->Normal().y << endl;
		bool surface = ( c->normal.x == 0 && c->normal.y == 0 );

		//these make sure its a point of conention and not the other edge end point
		double len0 = length( c->position - e->v0 );
		double len1 = length( c->position - e->v1 );
		
		
		if( e->edge0->edgeType == Edge::CLOSED_GATE && len0 < 1 )
		{
			//cout << "len0: " << len0 << endl;
			V2d pVec = normalize( position - e->v0 );
			double pAngle = atan2( -pVec.y, pVec.x );

			if( pAngle < 0 )
			{
				pAngle += 2 * PI;
			}

			Edge *e0 = e->edge0;
			Gate *g = (Gate*)e0->info;

			V2d startVec = normalize( e0->v0 - e->v0 );
			V2d endVec = normalize( e->v1 - e->v0 );

			double startAngle = atan2( -startVec.y, startVec.x );
			if( startAngle < 0 )
			{
				startAngle += 2 * PI;
			}
			double endAngle = atan2( -endVec.y, endVec.x );
			if( endAngle < 0 )
			{
				endAngle += 2 * PI;
			}

			double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;

			if( endAngle < startAngle )
			{
				if( pAngle >= endAngle || pAngle <= startAngle )
				{
					
				}
				else
				{
				//	cout << "blahblah a. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;
				}
			}
			else
			{
				if( pAngle >= startAngle && pAngle <= endAngle )
				{
					
					//cout << "startVec: " << startVec.x << ", " << startVec.y << ", end: " << endVec.x << ", " << endVec.y <<
					//	", p: " << pVec.x << ", " << pVec.y << endl;
					
				}
				else
				{
				//	cout << "blahblah b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;
					/*cout << "startVec: " << startVec.x << ", " << startVec.y << ", end: " << endVec.x << ", " << endVec.y <<
						", p: " << pVec.x << ", " << pVec.y << endl;
					cout << "return b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;*/
				}
			}
			

		}
		else if( e->edge1->edgeType == Edge::CLOSED_GATE && len1 < 1 )
		{
			//cout << "len1: " << len1 << endl;
			V2d pVec = normalize( position - e->v1 );
			double pAngle = atan2( -pVec.y, pVec.x );

			if( pAngle < 0 )
			{
				pAngle += 2 * PI;
			}

			Edge *e1 = e->edge1;
			Gate *g = (Gate*)e1->info;

			V2d startVec = normalize( e->v0 - e->v1 );
			V2d endVec = normalize( e1->v1 - e->v1 );

			double startAngle = atan2( -startVec.y, startVec.x );
			if( startAngle < 0 )
			{
				startAngle += 2 * PI;
			}
			double endAngle = atan2( -endVec.y, endVec.x );
			if( endAngle < 0 )
			{
				endAngle += 2 * PI;
			}
			
			double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;

			//double temp = startAngle;
			//startAngle = endAngle;
			//endAngle = temp;

			if( endAngle < startAngle )
			{
				if( pAngle >= endAngle || pAngle <= startAngle )
				{
					
				}
				else
				{
					//cout << "edge: " << e->Normal().x << ", " << e->Normal().y << ", return a. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;
				}
			}
			else
			{
				if( pAngle >= startAngle && pAngle <= endAngle )
				{
					//cout << "startVec: " << startVec.x << ", " << startVec.y << ", end: " << endVec.x << ", " << endVec.y <<
					//	", p: " << pVec.x << ", " << pVec.y << endl;
					
				}
				else
				{
					//cout << "edge: " << e->Normal().x << ", " << e->Normal().y << ", return b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;

					/*cout << "startVec: " << startVec.x << ", " << startVec.y << ", end: " << endVec.x << ", " << endVec.y <<
						", p: " << pVec.x << ", " << pVec.y << endl;
					cout << "return b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;*/
				}
			}
		}
		

		/*if( e->edge0->edgeType == Edge::CLOSED_GATE )
		{
			double q = 
			Edge *e0 = e->edge0;
			Gate *g = (Gate*)e0->info;
			Edge *ea = g->edgeA;
			double ca = cross( position - ea->v0, normalize( ea->v1 - ea->v0 ) );
			if( ca > 0 )
			{
				if( e0 == g->edgeB )
				{
					return;
				}
			}
			else if( ca < 0 )
			{
				if( e0 == g->edgeA )
				{
					return;
				}
			}
			else
			{
				assert( 0 && "gate collision stuff" );
			}
		}
		else if( e->edge1->edgeType == Edge::CLOSED_GATE )
		{
			Edge *e1 = e->edge1;
			Gate *g = (Gate*)e1->info;
			Edge *ea = g->edgeA;
			double ca = cross( position - ea->v0, normalize( ea->v1 - ea->v0 ) );
			if( ca > 0 )
			{
				if( e1 == g->edgeB )
				{
					return;
				}
			}
			else if( ca < 0 )
			{
				if( e1 == g->edgeA )
				{
					return;
				}
			}
			else
			{
				assert( 0 && "gate collision stuff" );
			}
		}*/
			

		
		
		
		
		

		//if( c != NULL )	//	|| minContact.collisionPriority < -.001 && c->collisionPriority >= 0 )
		//{
			if( c->edge->edgeType == Edge::OPEN_GATE )
			{
				//cout << "GATEEEEee" << endl;
				return;
			}
			else if( c->edge->edgeType == Edge::CLOSED_GATE )
			{
				
				//c->edge->edgeType = Edge::OPEN_GATE;
				Gate *g = (Gate*)c->edge->info;//owner->gateMap[c->edge];
				
				if( CanUnlockGate( g ) )
				{

					if( c->edge == g->edgeA )
					{
						gateTouched = g->edgeB;
					}
					else
					{
						gateTouched = g->edgeA;
					}

					owner->UnlockGate( g );

					return;

				}
			}

			if( ( c->normal.x == 0 && c->normal.y == 0 ) ) //non point
			{
				//cout << "SURFACE. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << ", pri: " << c->collisionPriority << endl;
			}
			else //point
			{
				//cout << "POINT. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << endl;
			}

			if( c->weirdPoint )
			{
		//		cout << "weird point " << endl;
				
				Edge *edge = e;
				Edge *prev = edge->edge0;
				Edge *next = edge->edge1;

				V2d v0 = edge->v0;
				V2d v1 = edge->v1;				

				//note: approxequals could be broken slightly
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

			bool closedGate = (c->edge->edgeType == Edge::CLOSED_GATE);
			bool minGate = (minContact.edge != NULL && minContact.edge->edgeType == Edge::CLOSED_GATE );

			if( !col || (minContact.collisionPriority < 0 ) 
				|| (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) )//|| ( closedGate && !minGate ) )
			{	
				if( c->collisionPriority == minContact.collisionPriority )//&& !closedGate )
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
		//cout << "checking: " << e << endl;
		if( (grindEdge == NULL && ground == e) || grindEdge == e )
			return;

		//Edge *testEdge = ground;
		
			
		

		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight );
		//Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		if ( action != GRINDBALL )
		{
			//cout << "here" << endl;
			if( ground != NULL )
			{

				

				V2d en = e->Normal();
				if( reversed )
				{
					if( en.y >= 0 )
					{
						return;
					}
				}
				else
				{
					if( e == ground->edge1 )
					{
						double c = cross( normalize(e->v1 - ground->v0),
							normalize( ground->v1 - ground->v0 ) );
						if( en.y >= 0 && c < 0 )
						{
							return;
						}
					}
					else if( e == ground->edge0 )
					{
						double c = cross( normalize(e->v0 - ground->v1),
							normalize( ground->v0 - ground->v1 ) );
						if( en.y >= 0 && c > 0 )
						{
							return;
						}
					}

					if( en.y <= 0 )
					{
						


						return;
					}
				}
				
				
				//for travelling so you don't hit the other side of the gate on accident
				if( ground->edgeType == Edge::CLOSED_GATE )
				{
					Gate *g = (Gate*)ground->info;
					if( ground == g->edgeA )
					{
						if( e == g->edgeB || e == g->edgeB->edge0 || e == g->edgeB->edge1 )
						{
							return;
						}
					}
					else
					{
						if( e == g->edgeA || e == g->edgeA->edge0 || e == g->edgeA->edge1 )
						{
							return;
						}
					}
					
					if( e == g->edgeA && ground == g->edgeB
						|| e == g->edgeB && ground == g->edgeA )//|| e == g->edgeB )
					{
						
						//cout << "returnning early" << endl;
						return;
					}
				}
				else if( groundSpeed > 0 && ground->edge1->edgeType == Edge::CLOSED_GATE )
				{
					Edge *e1 = ground->edge1;
					Gate *g = (Gate*)e1->info;
					if( e == g->edgeA && e1 == g->edgeB 
						|| e == g->edgeB && e1 == g->edgeA )
					{
						return;
					}
				}
				else if( groundSpeed < 0 && ground->edge0->edgeType == Edge::CLOSED_GATE )
				{
					Edge *e0 = ground->edge0;
					Gate *g = (Gate*)e0->info;
					if( e == g->edgeA && e0 == g->edgeB 
						|| e == g->edgeB && e0 == g->edgeA )
					{
						return;
					}
				}
				
			}
		}
		else
		{
			if( grindEdge->edgeType == Edge::CLOSED_GATE )
				{
					Gate *g = (Gate*)grindEdge->info;
					if( grindEdge == g->edgeA )
					{
						if( e == g->edgeB || e == g->edgeB->edge0 || e == g->edgeB->edge1 )
						{
							return;
						}
					}
					else
					{
						if( e == g->edgeA || e == g->edgeA->edge0 || e == g->edgeA->edge1 )
						{
							return;
						}
					}
					
					if( e == g->edgeA && grindEdge == g->edgeB
						|| e == g->edgeB && grindEdge == g->edgeA )//|| e == g->edgeB )
					{
						
						//cout << "returnning early" << endl;
						return;
					}
				}
				else if( grindSpeed > 0 && grindEdge->edge1->edgeType == Edge::CLOSED_GATE )
				{
					Edge *e1 = grindEdge->edge1;
					Gate *g = (Gate*)e1->info;
					if( e == g->edgeA && e1 == g->edgeB 
						|| e == g->edgeB && e1 == g->edgeA )
					{
						return;
					}
				}
				else if( grindSpeed < 0 && grindEdge->edge0->edgeType == Edge::CLOSED_GATE )
				{
					Edge *e0 = grindEdge->edge0;
					Gate *g = (Gate*)e0->info;
					if( e == g->edgeA && e0 == g->edgeB 
						|| e == g->edgeB && e0 == g->edgeA )
					{
						return;
					}
				}
		}

		//cout << "valid is false" << endl;
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

		if( e->edgeType == Edge::OPEN_GATE )
		{
			return;
		}

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
	//else if( queryMode == "gate" )
	//{
	//	Gate *g = (Gate*)qte;
	//	
	//	if( action == GRINDBALL )
	//	{
	//		if( g->locked && ( g->edgeA->v0 == grindEdge->v0 || g->edgeA->v1 == grindEdge->v0 ) )
	//		{
	//			if( CanUnlockGate( g )
	//			{
	//				owner->UnlockGate( g );

	//				if( e0 == g->edgeA )
	//				{
	//					gateTouched = g->edgeB;
	//				//	owner->ActivateZone( g->zoneB );
	//				}
	//				else
	//				{
	//					gateTouched = g->edgeA;
	//				//	owner->ActivateZone( g->zoneA );
	//				}
	//				//g->locked = false;
	//				//hasKey[g->type] = false;
	//			}
	//			else
	//			{
	//				++owner->testGateCount;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	//		cout << "this better not be happening lol" << endl;
	//		if( g->locked && g->edgeA->IsTouchingBox( r ) )
	//		{
	//			if( g->keyGate && hasKey[g->type] )
	//			{
	//				g->locked = false;
	//				hasKey[g->type] = false;
	//			}
	//			else
	//			{
	//				++owner->testGateCount;
	//			}
	//		}
	//	}
	//}
	else if( queryMode == "item" )
	{
		Critical *c = (Critical*)qte;

		if( c->active )
		{
			if( currHitboxes != NULL )
			{
				for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
				{
					if( (*it).Intersects( c->box ) )
					{
						currentCheckPoint = c;
						//for( int i = 2; i < Gate::GateType::Count; ++i )
						//{
						//	c->hadKey[i] = hasKey[i];
						//}
						owner->activatedZoneList = NULL;
						owner->inactiveEnemyList = NULL;
						owner->unlockedGateList = NULL;
						//cout << "destroy critical connection yay" << endl;
						c->active = false;
						return;
						//activate critical connection yay
					}
				}
			}

			if( hurtBody.Intersects( c->box ) )
			{
				currentCheckPoint = c;
				/*for( int i = 2; i < Gate::GateType::Count; ++i )
				{
					c->hadKey[i] = hasKey[i];
				}*/
				owner->inactiveEnemyList = NULL;
				owner->unlockedGateList = NULL;
				owner->activatedZoneList = NULL;

				c->active = false;
				
				return;
			}
		}
	

		//check with my hurtbox also!
	}
	else if( queryMode == "envplant" )
	{
		EnvPlant *ev = (EnvPlant*)qte;
		
		if( !ev->activated )
		{
			ev->activated = true;
			if( ground != NULL )
			{
				ev->particle->dir = normalize( normalize( ground->v1 - ground->v0 ) * groundSpeed );
			}
			else
			{
				ev->particle->dir = normalize( velocity );
			}
			
			if( owner->activeEnvPlants == NULL )
			{
				ev->next = NULL;
				owner->activeEnvPlants = ev;
			}
			else
			{
				ev->next = owner->activeEnvPlants;
				owner->activeEnvPlants = ev;
			}
			ev->frame = 0;
		}
	}
	/*else if( queryMode == "specter" )
	{
		SpecterArea *sp = (SpecterArea*)qte;
		V2d center( sp->testRect.left + radius,
			sp->testRect.top + radius );
		CollisionBox b;
		b.isCircle = true;
	}*/
	
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
	/*double c = cos( -currInput.leftStickRadians);
	double s = sin( -currInput.leftStickRadians);
	V2d left( c, s );
	CircleShape ccs;
	ccs.setRadius( 10 );
	ccs.setOrigin( ccs.getLocalBounds().width/2, ccs.getLocalBounds().height / 2 );
	ccs.setPosition( position.x + left.x * 100, position.y + left.y * 100 );
	target->draw( ccs );*/
	//target->draw( speedCircle );
	/*if( action == DASH )
	{
		target->draw( *re->particles );
		target->draw( *re1->particles );
	}*/
	//target->draw( *pTrail->particles );

	if( bounceFlameOn && action != DEATH && action != EXIT && action != GOALKILL
		&& action != GOALKILLWAIT )
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
		else if( gs >= dashSpeed * 1.7 )
		{
			showMotionGhosts = 3;
		}
		else if( gs >= dashSpeed * 2 ) 
		{
			showMotionGhosts = 4;
		}
		else if( gs >= dashSpeed * 2.3 )
		{
			showMotionGhosts = 5;
		}
	}
	else
	{
		double len = length( velocity );
		if( len >= dashSpeed * 2 )
		{
			showMotionGhosts = 5;
		}
		else if( len >= dashSpeed * 1.7 )
		{
			showMotionGhosts = 4;
		}
		else if( len >= dashSpeed * 1.5 )
		{
			showMotionGhosts = 3;
		}
		else if( len >= dashSpeed )
		{
			showMotionGhosts = 2;
		}
		else
		{
			showMotionGhosts = 1;
		}
	}

	showMotionGhosts = 4;
	if( showMotionGhosts && !desperationMode )
	{
		//swordShader.setParameter( "isTealAlready", 0 );
		//
		//for( int i = 0; i < showMotionGhosts; ++i )
		//{
		//	float opac = .5 * ((MAX_MOTION_GHOSTS - i ) / (float)MAX_MOTION_GHOSTS);
		//	swordShader.setParameter( "opacity", opac );
		//	//cout << "setting : " << i << endl;
		//	//motionGhosts[i].setColor( Color( 50, 50, 255, 50 ) );
		//	//motionGhosts[i].setColor( Color( 50, 50, 255, 100 * ( 1 - (double)i / showMotionGhosts ) ) );
		//	//target->draw( fairSword1, &swordShader );
		//	target->draw( motionGhosts[i], &swordShader );
		//}
	}

	if( action != GRINDBALL )
	{

		//RayCast( this, owner->testTree, position, V2d( position.x - 100, position.y ) );
		
		

		Vector2i vi = Mouse::getPosition();
		//Vector2i vi = owner->window->mapCoordsToPixel( Vector2f( position.x, position.y ) );//sf::Vector2f( 0, -300 ) );

		Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
		//Vector3f blahblah( vi.x / (float)owner->window->getSize().x, 
		//	(owner->window->getSize().y - vi.y) / (float)owner->window->getSize().x, .015 );

		
		
		if( action != DEATH && action != SPAWNWAIT && action != GOALKILL && action != GOALKILLWAIT )
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
			cs.setRadius( 40 );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( 0, -300 );*/
			

			//target->draw( spr, &sh );






			//sh.setParameter( "u_texture",( *owner->GetTileset( "run2.png" , 80, 48 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			//sh.setParameter( "u_normals", *owner->GetTileset( "run_NORMALS.png", 80, 48 )->texture );


			//sh.setParameter( "u_texture",( *owner->GetTileset( "run.png" , 128, 64 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			//sh.setParameter( "u_normals", *owner->GetTileset( "run_normal.png", 128, 64 )->texture );
			
			sh.setParameter( "u_texture", *tileset[action]->texture ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
			sh.setParameter( "u_normals", *normal[action]->texture );

			target->draw( *sprite, &sh );
		}
		else
		{
			target->draw( *sprite );
		}
		

		if( showSword )
		{
			sf::Shader &swordSh = swordShaders[speedLevel];
			//swordShader.setParameter( "isTealAlready", 1 );
			switch( action )
			{
			case FAIR:
				{
					if( flashFrames > 0 )
					{
						target->draw( fairSword, &swordSh );
						//cout << "shader!" << endl;
					}
					else
					{
						target->draw( fairSword );
					}
					break;
				}
			case DAIR:
				{
					if( flashFrames > 0 )
						target->draw( dairSword, &swordSh );
					else
						target->draw( dairSword );
					break;
				}
			case UAIR:
				{
					if( flashFrames > 0 )
						target->draw( uairSword, &swordSh );
					else
						target->draw( uairSword );
					break;
				}
			case STANDN:
				{
					
					if( flashFrames > 0 )
					{
						target->draw( standingNSword, &swordSh );
						//cout << "Standn" << endl;
					}
					else
						target->draw( standingNSword );
					break;
				}
			case DASHATTACK:
				{
					if( flashFrames > 0 )
						target->draw( dashAttackSword, &swordSh );
					else
						target->draw( dashAttackSword );
					break;
				}
			case WALLATTACK:
				if( flashFrames > 0 )
						target->draw( wallAttackSword, &swordSh );
					else
						target->draw( wallAttackSword );
					break;
				break;
			case STEEPCLIMBATTACK:
				if( flashFrames > 0 )
						target->draw( steepClimbAttackSword, &swordSh );
					else
						target->draw( steepClimbAttackSword );
					break;
				break;
			case STEEPSLIDEATTACK:
				if( flashFrames > 0 )
						target->draw( steepSlideAttackSword, &swordSh );
					else
						target->draw( steepSlideAttackSword);
					break;
				break;
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
				if( g->states[ghostFrame-1].showSword )
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

	/*Color followerCol = Color::Red;
	sf::Vertex follower[2] = 
	{
		Vertex( Vector2f( position.x, position.y ), followerCol ),
		Vertex( Vector2f( followerPos.x, followerPos.y ), followerCol )
	};
	target->draw( follower, 2, sf::Lines );*/
	
	
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
	//b.DebugDraw( target );

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

void Actor::UpdateSprite()
{

	V2d gn( 0, 0 );
	if( ground != NULL )
	{
		gn = ground->Normal();
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
			V2d pp = ground->GetPoint( edgeQuantity );
			double angle = GroundedAngle();
			V2d along = normalize( ground->v1 - ground->v0 );

			double xExtraStartRun = 0.0;//5.0
			if( frame == 0 && ( 
					( currInput.LLeft() && !prevInput.LLeft() ) 
				||  ( currInput.LRight() && !prevInput.LRight() ) )  )
			{
				owner->ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart,
				pp + gn * 32.0 + along * xExtraStartRun, false, angle, 6, 3, facingRight );
				//runTappingSound.stop();
				//runTappingSound.play();
			}
		
		double xExtraStart = -48.0;
		if( !facingRight )
			xExtraStart = -xExtraStart;
		if( frame == 3 * 4 )
		{
			owner->ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_run,
				pp + gn * 48.0 + along * xExtraStart, false, angle, 8, 3, facingRight );
			owner->soundNodeList->ActivateSound( soundBuffers[S_RUN_STEP1] );
		}
		else if( frame == 8 * 4 )
		{
			owner->ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_run,
				pp + gn * 48.0 + along * xExtraStart, false, angle, 8, 3, facingRight );
			//owner->ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashStart, 
			//		pp + gn * 32.0 + along * xExtraStart , false, angle, 9, 3, facingRight );
			owner->soundNodeList->ActivateSound( soundBuffers[S_RUN_STEP2] );
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
			//double angle = GroundedAngle();
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
		if( frame == 2 * 4 )
		{
			owner->soundNodeList->ActivateSound( soundBuffers[S_SPRINT_STEP1] );
		}
		else if( frame == 6 * 4 )
		{
			owner->soundNodeList->ActivateSound( soundBuffers[S_SPRINT_STEP2] );
		}
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
				sprite->setTextureRect( tileset[SPRINT]->GetSubRect( frame / 4 ) );
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
				ir = tileset[SPRINT]->GetSubRect( frame / 4 );
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

		/*if( frame == 0 )
		{
			ir = tileset[JUMP]->GetSubRect( 0 );
		}
		else */if( velocity.y < -15)
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
	case JUMPSQUAT:
		{
			sprite->setTexture( *(tileset[JUMPSQUAT]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[JUMPSQUAT]->GetSubRect( 0 ) );
			}
			else
			{
				sf::IntRect ir = tileset[JUMPSQUAT]->GetSubRect( 0 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}


			double angle = GroundedAngle();
			if( reversed )
			{
				if( -gn.y > -steepThresh )
				{
					angle = PI;
				}
				//need to fill this in for reversed!
			}
			else
			{
				if( gn.y > -steepThresh )
				{
					angle = 0;
				}
			}
			//cout << "angle: " << angle << endl;
			

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

			//if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			//sprite->setPosition( pp.x + offsetX, pp.y );
			
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

			fxPos += gn * 48.0;

			//cout << "activating" << endl;
			owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_land, fxPos, false, angle, 8, 2, facingRight );
		}

		


		break;
		}
	case LAND2: 
		{
		sprite->setTexture( *(tileset[LAND2]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[LAND2]->GetSubRect( 1 ) );
		}
		else
		{
			sf::IntRect ir = tileset[LAND2]->GetSubRect( 1 );
				
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
			if( frame == 0 )
			{
				V2d fxPos = position;
				if( facingRight )
				{
					fxPos += V2d( 0, 0 );
				}
				else
				{
					fxPos += V2d( 0, 0 );
				}
				owner->ActivateEffect( EffectLayer::IN_FRONT, ts_fx_wallJump, fxPos, false, 0, 8, 2, facingRight );
				
				//cout << "ACTIVATING WALLJUMP" << endl;
			}

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
			int startFrame = 0;
			showSword = true;

			sprite->setTexture( *(tileset[STANDN]->texture));

			Tileset *curr_ts = ts_standingNSword[speedLevel];

			if( showSword )
			{
				standingNSword.setTexture( *curr_ts->texture );
			}
			//Vector2i offset( 24, -16 );
			//Vector2i offset( 24, 0 );
			//Vector2i offset( 32, 0 );
			Vector2i offset( 0, -16 );


			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STANDN]->GetSubRect( frame / 4 ) );

				if( showSword )
					standingNSword.setTextureRect( curr_ts->GetSubRect( frame / 4 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[STANDN]->GetSubRect( frame / 4 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
				
				if( showSword  )
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / 4 - startFrame );
					standingNSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}

				
			}

			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword )
			{
				standingNSword.setOrigin( standingNSword.getLocalBounds().width / 2, standingNSword.getLocalBounds().height);
				standingNSword.setRotation( angle / PI * 180 );

			
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

			standingNSword.setPosition( pos.x, pos.y );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = standingNSword;
			}

			break;
		}
	case DASHATTACK:
		{
			int startFrame = 0;
			showSword = true;//frame / 2 >= startFrame && frame / 2 <= 7;
			Tileset *curr_ts = ts_dashAttackSword[speedLevel];

			if( showSword )
			{
				dashAttackSword.setTexture( *curr_ts->texture );
			}

			sprite->setTexture( *(tileset[DASHATTACK]->texture));

			Vector2i offset( 0, 0 );

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[DASHATTACK]->GetSubRect( frame / 2 ) );

				if( showSword )
					dashAttackSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[DASHATTACK]->GetSubRect( frame / 2 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword  )
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / 2 - startFrame );
					dashAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}
			}
			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword )
			{
				dashAttackSword.setTexture( *curr_ts->texture );
				dashAttackSword.setOrigin( dashAttackSword.getLocalBounds().width / 2, dashAttackSword.getLocalBounds().height);
				dashAttackSword.setRotation( angle / PI * 180 );
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

			dashAttackSword.setPosition( pos.x, pos.y );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = dashAttackSword;
			}

			break;
		}
	case STEEPCLIMBATTACK:
		{
			int startFrame = 0;
			showSword = true;//frame / 2 >= startFrame && frame / 2 <= 7;
			Tileset *curr_ts = ts_steepClimbAttackSword[speedLevel];
			int animFactor = 4;

			if( showSword )
			{
				steepClimbAttackSword.setTexture( *curr_ts->texture );
			}

			sprite->setTexture( *(tileset[STEEPCLIMBATTACK]->texture));

			Vector2i offset( 0, 0 );

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STEEPCLIMBATTACK]->GetSubRect( frame / animFactor ) );

				if( showSword )
					steepClimbAttackSword.setTextureRect( curr_ts->GetSubRect( frame / animFactor - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[STEEPCLIMBATTACK]->GetSubRect( frame / animFactor );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword  )
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / animFactor - startFrame );
					steepClimbAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}
			}
			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword )
			{
				steepClimbAttackSword.setTexture( *curr_ts->texture );
				steepClimbAttackSword.setOrigin( steepClimbAttackSword.getLocalBounds().width / 2, steepClimbAttackSword.getLocalBounds().height);
				steepClimbAttackSword.setRotation( angle / PI * 180 );
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

			steepClimbAttackSword.setPosition( pos.x, pos.y );

			/*if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = dashAttackSword;
			}*/
			break;
		}
	case STEEPSLIDEATTACK:
		{
			int startFrame = 0;
			showSword = true;//frame / 2 >= startFrame && frame / 2 <= 7;
			Tileset *curr_ts = ts_steepSlideAttackSword[speedLevel];

			if( showSword )
			{
				steepSlideAttackSword.setTexture( *curr_ts->texture );
			}

			sprite->setTexture( *(tileset[STEEPSLIDEATTACK]->texture));

			Vector2i offset( 0, 0 );

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STEEPSLIDEATTACK]->GetSubRect( frame / 3 ) );

				if( showSword )
					steepSlideAttackSword.setTextureRect( curr_ts->GetSubRect( frame / 3 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[STEEPSLIDEATTACK]->GetSubRect( frame / 3 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword  )
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / 3 - startFrame );
					steepSlideAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}
			}
			
			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword )
			{
				steepSlideAttackSword.setTexture( *curr_ts->texture );
				steepSlideAttackSword.setOrigin( steepSlideAttackSword.getLocalBounds().width / 2, steepSlideAttackSword.getLocalBounds().height);
				steepSlideAttackSword.setRotation( angle / PI * 180 );
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

			steepSlideAttackSword.setPosition( pos.x, pos.y );
			break;
		}
	case WALLATTACK:
		{
			Tileset *curr_ts = ts_wallAttackSword[speedLevel];
			//cout << "fair frame : " << frame / 2 << endl;
			int startFrame = 0;
			showSword = true;//frame >= startFrame && frame / 2 <= 9;

			if( showSword )
			{
				wallAttackSword.setTexture( *curr_ts->texture );
			}

			sprite->setTexture( *(tileset[WALLATTACK]->texture));

			//Vector2i offset( 32, -16 );
			Vector2i offset( -8, -8 );

			if( facingRight )
			{
				
				sprite->setTextureRect( tileset[WALLATTACK]->GetSubRect( frame / 2 ) );
				//sprite->setTextureRect( tileset[FAIR]->GetSubRect( frame ) );
				if( showSword )
					//fairSword1.setTextureRect( ts_fairSword1->GetSubRect( frame - startFrame ) );
					wallAttackSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[WALLATTACK]->GetSubRect( frame / 2 );
				//sf::IntRect ir = tileset[FAIR]->GetSubRect( frame );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				
				if( showSword  )
				{
					offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame / 2 - startFrame );
					//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
					wallAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}
					
			}

			if( showSword )
			{
				wallAttackSword.setOrigin( wallAttackSword.getLocalBounds().width / 2, wallAttackSword.getLocalBounds().height / 2 );
				wallAttackSword.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = wallAttackSword;
			}
			
			break;
		}
	case FAIR:
		{
			Tileset *curr_ts = ts_fairSword[speedLevel];
			//cout << "fair frame : " << frame / 2 << endl;
			int startFrame = 0;
			showSword = true;//frame >= startFrame && frame / 2 <= 9;

			if( showSword )
			{
				fairSword.setTexture( *curr_ts->texture );
			}

			sprite->setTexture( *(tileset[FAIR]->texture));

			//Vector2i offset( 32, -16 );
			Vector2i offset( 0, 0 );

			if( facingRight )
			{
				
				sprite->setTextureRect( tileset[FAIR]->GetSubRect( frame / 2 ) );
				//sprite->setTextureRect( tileset[FAIR]->GetSubRect( frame ) );
				if( showSword )
					//fairSword1.setTextureRect( ts_fairSword1->GetSubRect( frame - startFrame ) );
					fairSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[FAIR]->GetSubRect( frame / 2 );
				//sf::IntRect ir = tileset[FAIR]->GetSubRect( frame );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				
				if( showSword  )
				{
					offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame / 2 - startFrame );
					//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
					fairSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}
					
			}

			if( showSword )
			{
				fairSword.setOrigin( fairSword.getLocalBounds().width / 2, fairSword.getLocalBounds().height / 2 );
				fairSword.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = fairSword;
			}
			
			break;
		}
	case DAIR:
		{
			Tileset *curr_ts = ts_dairSword[speedLevel];
			int startFrame = 0;
			//showSword = frame / 2 >= startFrame && frame / 2 <= 9;
			showSword = true;

			if( showSword )
			{
				dairSword.setTexture( *curr_ts->texture );
			}

			Vector2i offsetArr[3];
			offsetArr[0] = Vector2i( 0, 24 );
			offsetArr[1] = Vector2i( 0, 48 );
			offsetArr[2] = Vector2i( 0, 72 );

			Vector2i offset = offsetArr[speedLevel];

			sprite->setTexture( *(tileset[DAIR]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[DAIR]->GetSubRect( frame / 2 ) );

				if( showSword )
					dairSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
			}
			else
			{
				//offset.x = -offset.x;

				sf::IntRect ir = tileset[DAIR]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword  )
				{
					//offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame / 2 - startFrame );
					dairSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}
			}

			if( showSword )
			{
				dairSword.setOrigin( dairSword.getLocalBounds().width / 2, dairSword.getLocalBounds().height / 2 );
				dairSword.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case UAIR:
		{
			Tileset *curr_ts = ts_uairSword[speedLevel];
			int startFrame = 0;
			showSword = true;
			//showSword = frame / 3 >= startFrame && frame / 3 <= 5;

			if( showSword )
			{
				uairSword.setTexture( *curr_ts->texture );
			}

			sprite->setTexture( *(tileset[UAIR]->texture));

			Vector2i offset( 0, 0 );
			//Vector2i offset( 8, -24 );

			if( facingRight )
			{
				sprite->setTextureRect( tileset[UAIR]->GetSubRect( frame / 3 ) );

				if( showSword )
					uairSword.setTextureRect( curr_ts->GetSubRect( frame / 3 - startFrame ) );
			}
			else
			{
				sf::IntRect ir = tileset[UAIR]->GetSubRect( frame / 3 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				if( showSword )
				{
					//offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame / 3 - startFrame );
					uairSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}

				offset.x = -offset.x;
			}

			if( showSword )
			{
				uairSword.setOrigin( uairSword.getLocalBounds().width / 2, uairSword.getLocalBounds().height / 2 );
				uairSword.setPosition( position.x + offset.x, position.y + offset.y );
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
			
			if( frame == 0 )//&& currInput.B && !prevInput.B )
			{
				//dashStartSound.stop();
				//if( slowMultiple != 1)
				//	dashStartSound.setPitch( .2 );
				//else
				//	dashStartSound.setPitch( 1 );
				//cout << "playing dash sound" << endl;
				//dashStartSound.play();
				//dashStartSound.setLoop( true );
			}

			//if( slowMultiple != 1)
			//		dashStartSound.setPitch( .2 );
			//	else
			//		dashStartSound.setPitch( 1 );

			sprite->setTexture( *(tileset[DASH]->texture));

			//3-8 is the cycle
			sf::IntRect ir;
			int checkFrame;
			if( frame / 2 < 1 )
			{
				checkFrame = frame / 2;
				ir = tileset[DASH]->GetSubRect( checkFrame );
			}
			else if( frame < actionLength[DASH] - 1 )
			{
				checkFrame = 1 + ( (frame/2 - 1) % 5 );
				ir = tileset[DASH]->GetSubRect( checkFrame );
			}
			else
			{
				checkFrame = 6; //9 + (6 - ( actionLength[DASH] - frame )) / 2;
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

			V2d along = normalize( ground->v1 - ground->v0 );
			double xExtraRepeat = 64;
			double xExtraStart = 80;

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				xExtraRepeat = -xExtraRepeat;
				xExtraStart = -xExtraStart;
			}
			

			if( movingGround != NULL )
			{
				ground->v0 = oldv0;
				ground->v1 = oldv1;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				pp.x += offsetX;

			sprite->setPosition( pp.x, pp.y );

			if( frame == 0 && currInput.B && !prevInput.B )
			{
				owner->ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashStart, 
					pp + gn * 64.0 + along * xExtraStart , false, angle, 9, 3, facingRight );
				owner->soundNodeList->ActivateSound( soundBuffers[S_DASH_START] );
			}
			else if( frame % 5 == 0 )
			{
				owner->ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashRepeat, 
					pp + gn * 32.0 + along * xExtraRepeat, false, angle, 12, 3, facingRight );
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
				//playerHitSound.stop();
				//playerHitSound.play();
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
				sprite->setTextureRect( tileset[GROUNDHITSTUN]->GetSubRect( 1 ) );
			}
			else
			{
				sf::IntRect ir = tileset[GROUNDHITSTUN]->GetSubRect( 1 );
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
				if( bounceFrame == 2 )
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
				else
				{
					//if( facingRight )//bounceFrame == 4 )
					//{
					//	//sprite->setOrigin( sprite->getLocalBounds().width / 2, 0);
					//	sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
					//}
					//else
					//{
					//	sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
					//}	
					sprite->setOrigin( sprite->getLocalBounds().width / 2, normalHeight );
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
	case INTRO:
		{
			sprite->setTexture( *(tileset[INTRO]->texture));
			sprite->setTextureRect( tileset[INTRO]->GetSubRect( frame / 4 ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case EXIT:
		{
			sprite->setTexture( *(tileset[EXIT]->texture));
			sprite->setTextureRect( tileset[EXIT]->GetSubRect( frame / 2 ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height );
			sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );//position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case GOALKILL:
		{
			
			//radius is 24. 100 pixel offset
			int tsIndex = (frame / 2) / 16;
			int realFrame = (frame / 2 ) % 16;
			cout << "goalkill index: " << tsIndex << ", realFrame: " << realFrame << endl;
			Tileset *tsT = ts_goalKillArray[tsIndex];
			sprite->setTexture( *(tsT->texture));
			sprite->setTextureRect( tsT->GetSubRect( realFrame ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );//- 24.f );
			sprite->setRotation( 0 );
			break;
		}
	case GOALKILLWAIT:
		{
			Tileset *tsT = ts_goalKillArray[4];
			sprite->setTexture( *(tsT->texture));
			sprite->setTextureRect( tsT->GetSubRect( 7 ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );
			sprite->setRotation( 0 );
			break;
		}
	case SPAWNWAIT:
		{
			break;
		}
	case DEATH:
		{
			

			break;
		}
	case RIDESHIP:
		{
			sprite->setTexture( *(tileset[RIDESHIP]->texture));
			int tFrame = ( frame - 60 ) / 5;
			
			if( tFrame < 0 )
			{
				tFrame = 0;
			}
			else if( tFrame >= 5 )
			{
				tFrame = 5;
			}
			else
			{
				tFrame++;
			}
			sprite->setTextureRect( tileset[RIDESHIP]->GetSubRect( tFrame ) );
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
		}
		break;
	case SKYDIVE:
		sprite->setTexture( *(tileset[SKYDIVE]->texture));
		sprite->setTextureRect( tileset[SKYDIVE]->GetSubRect( 0 ) );
		sprite->setOrigin( sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );
		sprite->setRotation( 0 );
		break;
	case SKYDIVETOFALL:
		sprite->setTexture( *(tileset[SKYDIVETOFALL]->texture));
		sprite->setTextureRect( tileset[SKYDIVETOFALL]->GetSubRect( 0 ) );
		sprite->setOrigin( sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );
		sprite->setRotation( 0 );
		break;
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

	
}

void Actor::ConfirmEnemyKill( Enemy *e )
{
	if( e->hasMonitor && !e->suppressMonitor )
	{
		owner->soundNodeList->ActivateSound( soundBuffers[S_HIT_AND_KILL_KEY] );
	}
	else
	{
		owner->soundNodeList->ActivateSound( soundBuffers[S_HIT_AND_KILL] );
	}
	enemiesKilledThisFrame++;
	//for the growing tree
	//wrong
}

void Actor::ConfirmEnemyNoKill( Enemy *e )
{
	owner->soundNodeList->ActivateSound( soundBuffers[S_HIT] );
}

void Actor::ConfirmHit( Color p_flashColor, 
		int p_flashFrames, double speedBar, int charge)
{
	currentSpeedBar += speedBar;
	test = true;
	currAttackHit = true;
	flashColor = p_flashColor;
	flashFrames = p_flashFrames;
	for( int i = 0; i < 3; ++i )
	{
		swordShaders[i].setParameter( "toColor", p_flashColor );
	}

	owner->powerWheel->Charge( charge );
	desperationMode = false;
	//owner->player->test = true;
	//desperationMode = false;
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
			//else if( speedyJump && velocity.x > -dashSpeed )
			//{
			//	velocity.x = -dashSpeed;
			//}
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
			//else if( speedyJump && velocity.x < dashSpeed )
			//{
			//	velocity.x = dashSpeed;
			//}
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
				velocity.x += -airSlow;
				if( velocity.x < 0 ) velocity.x = 0;
				else if( velocity.x >= -dashSpeed )
				{
					velocity.x = 0;
				}
				//velocity.x = 0;
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
	else if( currInput.LDown() || currInput.LUp() )
	{
		//groundspeed stays the same
		
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

bool Actor::CanUnlockGate( Gate *g )
{
	//if( g->gState == Gate::REFORM || g->gState == Gate::LOCKFOREVER 
	//	|| g->gState == Gate::DISSOLVE
	//	|| g->gState )
	if( g->gState == Gate::OPEN )//!g->locked )
	{
		cout << "return early" << endl;
		return false;
	}

	bool canUnlock = false;

	bool enoughKeys = (owner->keyMarker->keysRequired == 0);
	//cout << "this gate is still locked" << endl;

	/*if( g->type == Gate::GREY && g->gState != Gate::LOCKFOREVER
		&& g->gState != Gate::REFORM )
	{
		cout << "gstate: " << (int)g->gState << endl;
		canUnlock = true;
	}
	else */
	if( g->type == Gate::BLACK )
	{
		canUnlock = false;
	}
	else if( enoughKeys && g->gState != Gate::LOCKFOREVER
		&& g->gState != Gate::REFORM )
	{
		//cout << "have keys: " << numKeys <<
		//	"need keys: " << g->requiredKeys << endl;
		canUnlock = true;
	}

	return canUnlock;
}

bool Actor::CaptureMonitor( Monitor * m )
{
	assert( m != NULL );

	//int gType = (int)m->monitorType + 1;
	if( numKeys == 6 )
	{
		cout << "ALREADY HAS SIX KEYS" << endl;

		return false;
		//return false;
	}
	else
	{
		//cout << "GIVING ME A KEY: " << (int)gType << endl;
		//hasKey[gType]++;
		numKeys++;
		owner->keyMarker->CollectKey();
		return true;
	}
}

void Actor::SetExpr( Expr ex )
{
	expr = ex;
}

void Actor::SetActionExpr( Action a )
{
	//SetExpr( Expr_NEUTRAL );
	switch( action )
	{
	case STAND:
		break;
	case RUN:
		//SetExpr( Expr_NEUTRAL );
		break;
	case SPRINT:
		break;
	}


	switch( a )
	{
	case JUMP:
		steepJump = false;
	case WALLJUMP:
	case DASH:
	case LAND:
	case LAND2:
	case DOUBLE:
	case WALLCLING:
	case SLIDE:
	case STEEPSLIDE:
	case STEEPCLIMB:
	case STAND:
		SetExpr( Expr_NEUTRAL );
		break;
	case RUN:
		//SetExpr( Expr_RUN );
		//SetExpr( Expr_NEUTRAL );
		break;
	case SPRINT:
		//SetExpr( Expr_SPRINT );
		break;
	case GOALKILL:
		SetExpr( Expr_NEUTRAL );
		break;
	}

	SetAction( a );
	//action = a;
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
			if( standHitboxes.count( frame ) > 0 )
			{
				currHitboxes = standHitboxes[frame];
			}
			break;
		}
	case DASHATTACK:
		{
			if( dashHitboxes.count( frame ) > 0 )
			{
				currHitboxes = dashHitboxes[frame];
			}
			break;
		}
	case WALLATTACK:
		{
			if( wallHitboxes.count( frame ) > 0 )
			{
				currHitboxes = wallHitboxes[frame];
			}
			break;
		}
	case STEEPCLIMBATTACK:
		{
			if( steepClimbHitboxes.count( frame ) > 0 )
			{
				currHitboxes = steepClimbHitboxes[frame];
			}
			break;
		}
	case STEEPSLIDEATTACK:
		{
			if( steepSlideHitboxes.count( frame ) > 0 )
			{
				currHitboxes = steepSlideHitboxes[frame];
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


