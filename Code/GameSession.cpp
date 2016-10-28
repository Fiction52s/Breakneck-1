//game session

#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include "EditSession.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "Sequence.h"
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "PauseMenu.h"
#include "Parallax.h"

#define TIMESTEP 1.0 / 60.0
#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

using namespace std;
using namespace sf;

//Ground - 1155aa
//
//Steep Floor - 4488cc
//
//Steep Ceiling - 55aaff
//
//Ceiling - 88ccff
//
//Wall - 6688ee


//Ground - 0d2d7d
//
//Steep Floor - 2244cc
//
//Wall - 0088cc
//
//Steep Ceiling - 2dcded
//
//Ceiling - 99ffff


#define COLOR_GROUND Color( 0x0d, 0x2d, 0x7d )
#define COLOR_STEEP_GROUND Color( 0x22, 0x44, 0xcc )
#define COLOR_STEEP_CEILING Color( 0x2d, 0xcd, 0xed )
#define COLOR_CEILING Color( 0x99, 0xff, 0xff )
#define COLOR_WALL Color( 0x00, 0x88, 0xcc )



ScoreDisplay::ScoreDisplay( GameSession *p_owner, Vector2f &position,
	sf::Font &testFont )
	:scoreBarVA( sf::Quads, 4 * NUM_BARS ), scoreSymbolsVA( sf::Quads, 4 * NUM_BARS ), scoreSheetVA( sf::Quads, 4 * NUM_BARS ),font( testFont )
{
	basePos = position;
	owner = p_owner;
	ts_scoreBar = owner->GetTileset( "score_bar_384x80.png", 384, 80 );
	ts_scoreContinue = owner->GetTileset( "score_continue_384x80.png", 384, 80 );
	ts_scoreSheet = owner->GetTileset( "score_sheet_384x80.png", 384, 80 );
	ts_scoreSymbols = owner->GetTileset( "score_symbol_384x80.png", 384, 80 );

	scoreContinue.setTexture( *ts_scoreContinue->texture );
	scoreContinue.setTextureRect( ts_scoreContinue->GetSubRect( 0 ) );
	scoreContinue.setOrigin( scoreContinue.getLocalBounds().width, 0 );
	scoreContinue.setPosition( 1920, 400 );

	IntRect ir = ts_scoreBar->GetSubRect( 0 );
	
	for( int i = 0; i < NUM_BARS; ++i )
	{
		scoreBarVA[i*4+0].texCoords = Vector2f( ir.left, ir.top );
		scoreBarVA[i*4+1].texCoords = Vector2f( ir.left + ir.width, ir.top );
		scoreBarVA[i*4+2].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
		scoreBarVA[i*4+3].texCoords = Vector2f( ir.left, ir.top + ir.height );
		
		IntRect ir1 = ts_scoreSymbols->GetSubRect( 0 );
		scoreSymbolsVA[ i * 4 + 0 ].texCoords = Vector2f( ir1.left, ir1.top );
		scoreSymbolsVA[ i * 4 + 1 ].texCoords = Vector2f( ir1.left + ir1.width, ir1.top );
		scoreSymbolsVA[ i * 4 + 2 ].texCoords = Vector2f( ir1.left + ir1.width, ir1.top + ir1.height );
		scoreSymbolsVA[ i * 4 + 3 ].texCoords = Vector2f( ir1.left, ir1.top + ir1.height );

		bars[i] = new ScoreBar( i, this );
		bars[i]->SetSymbolTransparency( 0 );
	}

	
	active = false;
	waiting = false;
	//SetScoreBarPos( 0, 0 );
}

void ScoreDisplay::Draw( RenderTarget *target )
{
	if( active )
	{
		target->draw( scoreBarVA, ts_scoreBar->texture );
		target->draw( scoreSheetVA, ts_scoreSheet->texture );
		target->draw( scoreSymbolsVA, ts_scoreSymbols->texture );
		target->draw( scoreContinue );

		if( bars[0]->state == ScoreBar::SHEET_DISPLAY )
		{
			target->draw( time );
		}
	}
}

void ScoreDisplay::Update()
{
	if( !active )
		return; 

	bool allNone = true;
	bool allDisplay = true;
	for( int i = 0; i < NUM_BARS; ++i )
	{
		bars[i]->Update();
		if( bars[i]->state != ScoreBar::NONE )
		{
			allNone = false;
		}

		if( bars[i]->state != ScoreBar::SHEET_DISPLAY )
		{
			allDisplay = false;
		}
	}

	if( allNone )
	{
		active = false;
	}
	if( allDisplay )
	{
		waiting = true;
	}
	
}

void ScoreDisplay::Reset()
{
	active = false;
	for( int i = 0; i < NUM_BARS; ++i )
	{
		bars[i]->state = ScoreBar::NONE;
		bars[i]->frame = 0;
		bars[i]->xDiffPos = 0;
		bars[i]->SetSymbolTransparency( 0 );
	}
}

void ScoreDisplay::Activate()
{
	active = true;
	waiting = false;
	bars[0]->state = ScoreBar::POP_OUT;
	bars[0]->frame = 0;
	
	time.setFont( font );
	time.setCharacterSize( 14 );
	time.setColor( Color::Black );

	stringstream ss;

	int seconds = owner->totalGameFrames / 60;
	int remain = owner->totalGameFrames % 60;
	int centiSecond = floor( (double)remain * (1.0/60.0 * 100.0 ) + .5 );
	
	ss << seconds << " : " << centiSecond << endl;
	time.setString( ss.str() );
	
	//time.setString


	//for( int i = 0; i < NUM_BARS; ++i )
	//{
	//	bars[i]->state = ScoreBar::POP_OUT;
	//	bars[i]->frame = 0;
	//	//bars[i]->xDiffPos = 0;
	//}
	//state = POP_OUT;
	//frame = 0;
}

void ScoreDisplay::Deactivate()
{
	for( int i = 0; i < NUM_BARS; ++i )
	{
		bars[i]->state = ScoreBar::RETRACT;
		bars[i]->frame = 0;
		//bars[i]->xDiffPos = 0;
	}
	waiting = false;

	//active = false;
	//waiting = false;
	//Reset();
}

ScoreDisplay::ScoreBar::ScoreBar( int p_row, ScoreDisplay *p_parent )
	:parent( p_parent ), frame( 0 ), state( NONE ), row( p_row ), xDiffPos( 0 )
{

}

void ScoreDisplay::ScoreBar::SetBarPos( float xDiff )
{
	IntRect ir = parent->ts_scoreBar->GetSubRect( 0 );
	int rowHeight = 100;
	xDiffPos = xDiff;

	VertexArray &scoreBarVA = parent->scoreBarVA;
	VertexArray &scoreSheetVA = parent->scoreSheetVA;
	VertexArray &scoreSymbolsVA = parent->scoreSymbolsVA;
	Vector2f &basePos = parent->basePos;
	
	scoreBarVA[ row * 4 + 0 ].position = Vector2f( basePos.x + xDiff, basePos.y + row * rowHeight );
	scoreBarVA[ row * 4 + 1 ].position = Vector2f( basePos.x + xDiff + ir.width, basePos.y + row * rowHeight );
	scoreBarVA[ row * 4 + 2 ].position = Vector2f( basePos.x + xDiff + ir.width, basePos.y + row * rowHeight + ir.height );
	scoreBarVA[ row * 4 + 3 ].position = Vector2f( basePos.x + xDiff, basePos.y + row * rowHeight + ir.height );

	scoreSheetVA[ row * 4 + 0 ].position = Vector2f( basePos.x + xDiff, basePos.y + row * rowHeight );
	scoreSheetVA[ row * 4 + 1 ].position = Vector2f( basePos.x + xDiff + ir.width, basePos.y + row * rowHeight );
	scoreSheetVA[ row * 4 + 2 ].position = Vector2f( basePos.x + xDiff + ir.width, basePos.y + row * rowHeight + ir.height );
	scoreSheetVA[ row * 4 + 3 ].position = Vector2f( basePos.x + xDiff, basePos.y + row * rowHeight + ir.height );

	scoreSymbolsVA[ row * 4 + 0 ].position = Vector2f( basePos.x + xDiff, basePos.y + row * rowHeight );
	scoreSymbolsVA[ row * 4 + 1 ].position = Vector2f( basePos.x + xDiff + ir.width, basePos.y + row * rowHeight );
	scoreSymbolsVA[ row * 4 + 2 ].position = Vector2f( basePos.x + xDiff + ir.width, basePos.y + row * rowHeight + ir.height );
	scoreSymbolsVA[ row * 4 + 3 ].position = Vector2f( basePos.x + xDiff, basePos.y + row * rowHeight + ir.height );
}

void ScoreDisplay::ScoreBar::ClearSheet() 
{
	/*scoreSheetVA[ row * 4 + 0 ].texCoords = Vector2f( ir.left, ir.top );
	scoreSheetVA[ row * 4 + 1 ].texCoords = Vector2f( ir.left + ir.width, ir.top );
	scoreSheetVA[ row * 4 + 2 ].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	scoreSheetVA[ row * 4 + 3 ].texCoords = Vector2f( ir.left, ir.top + ir.height );*/
}

void ScoreDisplay::ScoreBar::SetSheetFrame( int frame )
{
	VertexArray &scoreSheetVA = parent->scoreSheetVA;
	Tileset *ts_scoreSheet = parent->ts_scoreSheet;
	IntRect ir = ts_scoreSheet->GetSubRect( frame );
	scoreSheetVA[ row * 4 + 0 ].texCoords = Vector2f( ir.left, ir.top );
	scoreSheetVA[ row * 4 + 1 ].texCoords = Vector2f( ir.left + ir.width, ir.top );
	scoreSheetVA[ row * 4 + 2 ].texCoords = Vector2f( ir.left + ir.width, ir.top + ir.height );
	scoreSheetVA[ row * 4 + 3 ].texCoords = Vector2f( ir.left, ir.top + ir.height );
}

void ScoreDisplay::ScoreBar::SetSymbolTransparency( float f )
{
	int n = floor(f * 255.0 + .5);
	VertexArray &scoreSymbolsVA = parent->scoreSymbolsVA;
	scoreSymbolsVA[ row * 4 + 0 ].color = Color( 255, 255, 255, n );
	scoreSymbolsVA[ row * 4 + 1 ].color = Color( 255, 255, 255, n );
	scoreSymbolsVA[ row * 4 + 2 ].color = Color( 255, 255, 255, n );
	scoreSymbolsVA[ row * 4 + 3 ].color = Color( 255, 255, 255, n );

}

void ScoreDisplay::ScoreBar::Update()
{
	switch( state )
	{
	case NONE:
		{
			break;
		}
	case POP_OUT:
		{
			int popoutFrames = 30;
			if( frame == popoutFrames + 1 )
			{
				state = SHEET_APPEAR;
				frame = 0;
				break;
			}
			else if( frame == popoutFrames )
			{
				if( row < NUM_BARS - 1 )
				{
					parent->bars[row+1]->state = ScoreBar::POP_OUT;
					parent->bars[row+1]->frame = 0;
				}
			}
			++frame;

			break;
		}
	case SHEET_APPEAR:
		{
			if( frame == 10 )
			{
				state = SYMBOL_DISPLAY;
				frame = 0;
				break;
			}

			++frame;
			break;
		}
	case SYMBOL_DISPLAY:
		{
			if( frame == 31 )
			{
				state = SHEET_DISPLAY;
				frame = 0;

				if( row == 0 )
				{
					int rowHeight = 100;
					Vector2f &basePos = parent->basePos + Vector2f( 0, rowHeight * row ) 
						+Vector2f( 32, 32 );
					parent->time.setPosition( basePos );
				}
				

				break;
			}
			++frame;
			break;
		}
	case SHEET_DISPLAY:
		{
			break;
		}
	case RETRACT:
		{
			if( frame == 30 )
			{
				state = NONE;
				frame = 0;
				//active = false;
			}

			++frame;
			break;
		}
	}

	switch( state )
	{
	case NONE:
		{
			break;
		}
	case POP_OUT:
		{
			int popoutFrames = 30;
			CubicBezier bez( 0, 0, 1, 1 );
			float z = bez.GetValue( (double)frame / popoutFrames );

			SetBarPos( -384.f * z );
			
			break;
		}
	case SHEET_APPEAR:
		{
			SetSheetFrame( frame );
			break;
		}
	case SYMBOL_DISPLAY:
		{
			int dispFrames = 30;
			CubicBezier bez( 0, 0, 1, 1 );
			float z = bez.GetValue( (double)frame / dispFrames );
			SetSymbolTransparency( z );

			if( row == 0 )
			{

			}
			break;
		}
	case SHEET_DISPLAY:
		{
			break;
		}
	case RETRACT:
		{
			int retractFrames = 30;
			CubicBezier bez( 0, 0, 1, 1 );
			float z = bez.GetValue( (double)frame / retractFrames );

			SetBarPos( -384 * ( 1 - z ) );
			break;
		}
	}
}

PoiInfo::PoiInfo( const std::string &pname, Vector2i &p )
{
	name = pname;
	pos.x = p.x;
	pos.y = p.y;
	edge = NULL;
	barrier = NULL;
}

PoiInfo::PoiInfo( const std::string &pname, Edge *e, double q )
{
	name = pname;
	edge = e;
	edgeQuantity = q;
	pos = edge->GetPoint( edgeQuantity );
	barrier = NULL;
}

Barrier::Barrier( GameSession *p_owner, PoiInfo *p_poi, bool p_x, int p_pos, bool posOp,
	BarrierCallback *cb ): poi( p_poi )
{
	owner = p_owner;
	p_poi->barrier = this;
	callback = cb;
	x = p_x;
	pos = p_pos;
	triggered = false;
	positiveOpen = posOp;
}

bool Barrier::Update( Actor *player )
{
	if( triggered )
		return false;

	V2d playerPos = player->position;

	if( x )
	{
		if( positiveOpen ) //player starts right
		{
			if( playerPos.x < pos )
			{
				triggered = true;
			}
		}
		else //starts left
		{
			if( playerPos.x > pos )
			{
				triggered = true;
			}
		}
	}
	else
	{
		if( positiveOpen ) // player starts below
		{
			if( playerPos.y < pos )
			{
				triggered = true;
			}
		}
		else //player starts above
		{
			if( playerPos.y > pos )
			{
				triggered = true;
			}
		}
	}

	return triggered;
}

void Barrier::SetPositive()
{
	if( x )
	{
		if( owner->player->position.x - pos > 0 )
		{
			positiveOpen = true;
		}
		else
			positiveOpen = false;
	}
	else
	{
		if( owner->player->position.y - pos > 0 )
		{
			positiveOpen = true;
		}
		else
		{
			positiveOpen = false;
		}
	}
}

KeyMarker::KeyMarker( GameSession *p_owner )
{
	owner = p_owner;
	ts_keys = owner->GetTileset( "keys_256x256.png", 256, 256 );
	ts_keyEnergy = owner->GetTileset( "keys_energy_256x256.png", 256, 256 );

	backSprite.setTexture( *ts_keys->texture );
	energySprite.setTexture( *ts_keyEnergy->texture );

	backSprite.setPosition( 1920 - 256 - 40, 1080 - 256 - 40 );
	energySprite.setPosition( backSprite.getPosition() );

	state = ZERO;
	startKeys = 0;
	keysRequired = 0;
	frame = 0;
}

void KeyMarker::CollectKey()
{

	if( keysRequired == 0 )
	{
		return;
	}
	//assert( keysRequired > 0 );
	
	
	//int f = ((maxKeys-1) * 4) + (frame / growMult);

	--keysRequired;

	if( keysRequired == 0 )
	{
		int soundIndex = GameSession::SoundType::S_KEY_COMPLETE_W1 + ( startKeys - 1 );
		owner->soundNodeList->ActivateSound( owner->gameSoundBuffers[soundIndex] );
		state = TOZERO;
		frame = 0;
	}
	else
	{
		SetEnergySprite();
	}
}

void KeyMarker::SetEnergySprite()
{
	int starts[] = { 0, 1, 3, 6, 10, 15 };
	int trueStart = starts[startKeys-1];

	int f = trueStart + (startKeys - keysRequired);
	energySprite.setTextureRect( ts_keyEnergy->GetSubRect( f ) );
}

void KeyMarker::SetStartKeys( int sKeys )
{
	//assert( state == ZERO || state == TOZERO );
	startKeys = sKeys;
	keysRequired = startKeys;
	SetEnergySprite();

	//set bg sprite
	
	if( sKeys > 0 )
	{
		state = FROMZERO;
		frame = 0;
	}
	else
	{
		state = ZERO;
		backSprite.setTextureRect( ts_keys->GetSubRect( 24 ) );
	}
}

void KeyMarker::Draw( sf::RenderTarget *target )
{
	target->draw( backSprite );
	if( state == NONZERO )
	{
		target->draw( energySprite );
	}
}

void KeyMarker::Update()
{
	int growMult = 8;
	int shrinkMult = 8;
	switch( state )
	{
		case TOZERO:
		{
			if( frame == 4 * shrinkMult )
			{
				state = ZERO;
				frame = 0;
				backSprite.setTextureRect( ts_keys->GetSubRect( 24 ) );
				break;
			}

			int f = ((startKeys-1) * 4) + (3 - (frame / shrinkMult));
			backSprite.setTextureRect( ts_keys->GetSubRect( f ) );
			++frame;

			break;
		}
		case FROMZERO:
		{
			if( frame == 4 * growMult )
			{
				state = NONZERO;
				frame = 0;
				break;
			}
			int f = ((startKeys-1) * 4) + (frame / growMult);
			backSprite.setTextureRect( ts_keys->GetSubRect( f ) );
			++frame;
			break;
		}
	}
}

int GameSession::TestVA::bushFrame = 0;
int GameSession::TestVA::bushAnimLength = 20;
int GameSession::TestVA::bushAnimFactor = 8;

GameSession::GameSession( GameController &c, SaveFile *sf, MainMenu *p_mainMenu )
	:controller(c),va(NULL),edges(NULL), activeEnemyList( NULL ), pauseFrames( 0 )
	,groundPar( sf::Quads, 2 * 4 ), undergroundPar( sf::Quads, 4 ), underTransPar( sf::Quads, 2 * 4 ),
	onTopPar( sf::Quads, 4 * 6 ), miniVA( sf::Quads, 4 ), saveFile( sf ),
	cloud0( sf::Quads, 3 * 4 ), cloud1( sf::Quads, 3 * 4 ),
	cloudBot0( sf::Quads, 3 * 4 ), cloudBot1( sf::Quads, 3 * 4 )
{
	
	activeDialogue = NULL;

	keyFrame = 0;
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		effectLists[i] = NULL;
	}
	TestVA::bushFrame = 0;

	mainMenu = p_mainMenu;
	window = mainMenu->window;
	preScreenTex = mainMenu->preScreenTexture;
	postProcessTex = mainMenu->postProcessTexture;
	postProcessTex1 = mainMenu->postProcessTexture1;
	postProcessTex2 = mainMenu->postProcessTexture2;
	mapTex = mainMenu->mapTexture;
	minimapTex = mainMenu->minimapTexture;
	pauseTex = mainMenu->pauseTexture;

	arial.loadFromFile( "arial.ttf" );
	soundNodeList = new SoundNodeList( 10 );
	soundNodeList->SetGlobalVolume( 100 );
	scoreDisplay = new ScoreDisplay( this, Vector2f( 1920, 0 ), arial );
	
	currentZone = NULL;
	Movable::owner = this;
	b_crawler = NULL;
	b_bird = NULL;
	b_coyote = NULL;
	b_tiger = NULL;
	b_gator = NULL;
	b_skeleton = NULL;

	cutPlayerInput = false;
	//powerOrbs = new PowerOrbs( true, true, true, true, true, true);
	//powerOrbs = new PowerOrbs( this, true, true, true, false, false, false);
	
	Vector2f miniPos = Vector2f( 30, 750 );
	miniVA[0].position = miniPos + Vector2f( 0, 0 );
	miniVA[1].position = miniPos + Vector2f( 300, 0 );
	miniVA[2].position = miniPos + Vector2f( 300, 300 );
	miniVA[3].position = miniPos + Vector2f( 0, 300 );

	miniVA[0].texCoords = Vector2f( 0, 0 );
	miniVA[1].texCoords = Vector2f( 300, 0 );
	miniVA[2].texCoords = Vector2f( 300, 300 );
	miniVA[3].texCoords = Vector2f( 0, 300 );

	miniVA[0].color = Color::Red;
	miniVA[1].color = Color::Blue;
	miniVA[2].color = Color::Green;
	miniVA[3].color = Color::Magenta;

	//miniGoalPtrTex.loadFromFile( 

	//minimapCircle.setRadius( 500 );
	//minimapCircle.setFillColor( Color::Red );
	//minimapCircle.setPosition( 100, 100
	//	);
	miniRect.setSize( Vector2f( 300, 300 ) );
	miniRect.setPosition( 500, 500 );
	//miniRect.
	//miniMaskTex = new Texture;
	miniMaskTex.loadFromFile( "minimapmask.png" );

	lifeBarTex.loadFromFile( "lifebar_768x768.png" );
	lifeBarSprite.setTexture( lifeBarTex );
	lifeBarSprite.setPosition( 30, 200 );

	bigBulletVA = NULL;
	preScreenTex->setSmooth( false );
	postProcessTex->setSmooth( false );
	postProcessTex1->setSmooth( false );
	postProcessTex2->setSmooth( false );

	shockTestFrame = 0;

	if (!minimapShader.loadFromFile("minimap_shader.frag", sf::Shader::Fragment ) )
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "minimap SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "minimap shader not loaded" );
	}
	minimapShader.setParameter( "u_mask", miniMaskTex );

	if (!speedBarShader.loadFromFile("speedbar_shader.frag", sf::Shader::Fragment ) )
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "speed bar SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}
	speedBarShader.setParameter( "u_texture", sf::Shader::CurrentTexture );

	if( !glowShader.loadFromFile( "glow_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "glow SHADER NOT LOADING CORRECTLY" << endl;
	}
	glowShader.setParameter( "texSize", Vector2f( 1920, 1080 ) );



	if( !hBlurShader.loadFromFile( "hblur_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "hBlurShader SHADER NOT LOADING CORRECTLY" << endl;
	}
	hBlurShader.setParameter( "texSize", Vector2f( 1920/2, 1080/2 ) );

	if( !vBlurShader.loadFromFile( "vblur_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "vBlurShader SHADER NOT LOADING CORRECTLY" << endl;
	}
	vBlurShader.setParameter( "texSize", Vector2f( 1920/2, 1080/2 ) );
	

	if( !motionBlurShader.loadFromFile( "motionblur_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "motion blur SHADER NOT LOADING CORRECTLY" << endl;
	}
	motionBlurShader.setParameter( "texSize", Vector2f( 1920, 1080 ) );

	

	if( !shockwaveShader.loadFromFile( "shockwave_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "shockwave SHADER NOT LOADING CORRECTLY" << endl;
	}
	shockwaveShader.setParameter( "resolution", Vector2f( 1920, 1080 ) );
	shockwaveShader.setParameter( "texSize", Vector2f( 580, 580 ) );
	shockwaveTex.loadFromFile( "shockwave_580x580.png" );
	shockwaveShader.setParameter( "shockwaveTex", shockwaveTex );


	usePolyShader = true;
	//minimapTex = miniTex;

	minimapShader.setParameter( "u_texture", minimapTex->getTexture() );

	ts_keyHolder = GetTileset( "keyholder.png", 115, 40 );
	keyHolderSprite.setTexture( *ts_keyHolder->texture );
	keyHolderSprite.setPosition( 10, 50 );

	if (!onTopShader.loadFromFile("ontop_shader.frag", sf::Shader::Fragment ) )
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "on top SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if( !flowShader.loadFromFile( "flow_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "flow SHADER NOT LOADING CORRECTLY" << endl;
	}
	
	flowFrameCount = 60;
	flowFrame = 0;
	maxFlowRadius = 10000;
	radDiff = 100;
	flowSpacing = 600;
	maxFlowRings = 40;

	flowShader.setParameter( "radDiff", radDiff );
	flowShader.setParameter( "Resolution", 1920, 1080 );// window->getSize().x, window->getSize().y);
	flowShader.setParameter( "flowSpacing", flowSpacing );
	flowShader.setParameter( "maxFlowRings", maxFlowRadius / maxFlowRings );
	


	if (!mountainShader.loadFromFile("mountain_shader.frag", sf::Shader::Fragment ) )
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "mountain SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	mountainShader.setParameter( "u_texture", *GetTileset( "w1mountains.png", 1920, 512 )->texture );

	if (!mountainShader1.loadFromFile("mountain_shader.frag", sf::Shader::Fragment ) )
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "mountain SHADER 1 NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	mountainShader1.setParameter( "u_texture", *GetTileset( "w1mountains2.png", 1920, 406 )->texture );

	onTopShader.setParameter( "u_texture", *GetTileset( "w1undertrans.png", 1920, 540 )->texture );


	if( !underShader.loadFromFile( "under_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "under shader not loading correctly!" << endl;
		assert( false );
	}
	
	if (!cloneShader.loadFromFile("clone_shader.frag", sf::Shader::Fragment))
	{
		cout << "CLONE SHADER NOT LOADING CORRECTLY" << endl;
	}

	/*if (!enemyKeyShader.loadFromFile("key_shader.frag", sf::Shader::Fragment))
	{
		cout << "KEY SHADER NOT LOADING CORRECTLY" << endl;
	}

	if (!enemyHurtShader.loadFromFile("enemyhurt_shader.frag", sf::Shader::Fragment))
	{
		cout << "HURT SHADER NOT LOADING CORRECTLY" << endl;
	}*/

	stringstream ss;

	for( int i = 1; i <= 17; ++i )
	{
		ss << i;
		string texName = "deathbg" + ss.str() + ".png";
		ss.str( "" );
		ss.clear();
		wipeTextures[i-1].loadFromFile( texName );
	}

	deathWipe = false;
	deathWipeFrame = 0;
	deathWipeLength = 17 * 5;

	//preScreenTex = preTex;

	terrainBGTree = new QuadTree( 1000000, 1000000 );
	//soon make these the actual size of the bordered level
	terrainTree = new QuadTree( 1000000, 1000000 );
	//testTree = new EdgeLeafNode( V2d( 0, 0), 1000000, 1000000);
	//testTree->parent = NULL;
	//testTree->debug = rw;

	enemyTree = new QuadTree( 1000000, 1000000 );

	borderTree = new QuadTree( 1000000, 1000000 ); 

	grassTree = new QuadTree( 1000000, 1000000 ); 

	lightTree = new QuadTree( 1000000, 1000000 );

	gateTree = new QuadTree( 1000000, 1000000 );

	itemTree = new QuadTree( 1000000, 1000000 );

	crawlerReverserTree = new QuadTree( 1000000, 1000000 );

	envPlantTree = new QuadTree( 1000000, 1000000 );

	specterTree = new QuadTree( 1000000, 1000000 );

	listVA = NULL;
	lightList = NULL;

	inactiveEffects = NULL;
	pauseImmuneEffects = NULL;
	inactiveLights = NULL;

	//sets up fx so that they can be used
	for( int i = 0; i < MAX_EFFECTS; ++i )
	{
		AllocateEffect();
	}

	for( int i = 0; i < MAX_DYN_LIGHTS; ++i )
	{
		AllocateLight();
	}
	
	ts_miniIcons = GetTileset( "minimap_icons_64x64.png", 64, 64 );
	kinMinimapIcon.setTexture( *ts_miniIcons->texture );
	kinMinimapIcon.setTextureRect( ts_miniIcons->GetSubRect( 0 ) );
	kinMinimapIcon.setOrigin( kinMinimapIcon.getLocalBounds().width / 2,
		kinMinimapIcon.getLocalBounds().height / 2 );
	kinMinimapIcon.setPosition( 180, preScreenTex->getSize().y - 180 );

	kinMapSpawnIcon.setTexture( *ts_miniIcons->texture );
	kinMapSpawnIcon.setTextureRect( ts_miniIcons->GetSubRect( 1 ) );
	kinMapSpawnIcon.setOrigin( kinMapSpawnIcon.getLocalBounds().width / 2,
		kinMapSpawnIcon.getLocalBounds().height / 2 );
	//kinMapSpawnIcon.setPosition( goalPos.x, goalPos.y );

	

	goalMapIcon.setTexture( *ts_miniIcons->texture );
	goalMapIcon.setTextureRect( ts_miniIcons->GetSubRect( 0 ) );
	goalMapIcon.setOrigin( goalMapIcon.getLocalBounds().width / 2,
		goalMapIcon.getLocalBounds().height / 2 );

	ts_miniCircle = GetTileset( "minimap_circle_320x320.png", 320, 320 );
	miniCircle.setTexture( *ts_miniCircle->texture );
	miniCircle.setOrigin( miniCircle.getLocalBounds().width / 2, miniCircle.getLocalBounds().height / 2 );
	
	miniCircle.setPosition( kinMinimapIcon.getPosition() );

	ts_minimapGateDirection = GetTileset( "minimap_gate_w02_192x64.png", 192, 64 );
	for( int i = 0; i < 6; ++i )
	{
		Sprite &gds = gateDirections[i];
		gds.setTexture( *ts_minimapGateDirection->texture );
		gds.setTextureRect( ts_minimapGateDirection->GetSubRect( 0 ) );
		gds.setOrigin( gds.getLocalBounds().width / 2, 120 + gds.getLocalBounds().height );
		gds.setPosition( miniCircle.getPosition() );
	}

	ts_testParallax = GetTileset( "parallax_w2_01.png", 960, 540 ); 
	testParallaxSprite.setTexture( *ts_testParallax->texture );
	testParallaxSprite.setOrigin( testParallaxSprite.getLocalBounds().width / 2,
		testParallaxSprite.getLocalBounds().height / 2 );

	keyMarker = new KeyMarker( this );

	ts_w1ShipClouds0 = GetTileset( "Ship/cloud_w1_a1_960x128.png", 960, 128 );
	ts_w1ShipClouds1 = GetTileset( "Ship/cloud_w1_b1_960x320.png", 960, 320 );
	ts_ship = GetTileset( "Ship/ship_1728x800.png", 1728, 800 );
	

	shipSprite.setTexture( *ts_ship->texture );
	shipSprite.setTextureRect( ts_ship->GetSubRect( 0 ) );
	shipSprite.setScale( .5, .5 );
	shipSprite.setOrigin( shipSprite.getLocalBounds().width / 2, 
		shipSprite.getLocalBounds().height / 2 );
	
	//enemyTree = new EnemyLeafNode( V2d( 0, 0), 1000000, 1000000);
	//enemyTree->parent = NULL;
	//enemyTree->debug = rw;
}

GameSession::~GameSession()
{
	delete player;
	delete soundManager;

	//for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	//{
	//	cout << "about: " << (*it)->sourceName << ", "
	//		<< (*it)->tileWidth << ", " << (*it)->tileWidth << endl;
	//	//delete (*it);
	//}
	for( int i = 0; i < numGates; ++i )
	{
		delete gates[i];
	}
	delete [] gates;

	for( int i = 0; i < numPoints; ++i )
	{
		delete edges[i];
	}
	delete [] edges;

	for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		delete (*it);
	}

	//tm.ClearTilesets();

	

	delete terrainTree;
	delete enemyTree;
	delete lightTree;
	delete grassTree;
	delete gateTree;
	delete itemTree;
	delete crawlerReverserTree;
	delete envPlantTree;
	delete specterTree;
}

//should only be used to assign a variable. don't use at runtime
Tileset * GameSession::GetTileset( const string & s, int tileWidth, int tileHeight )
{
	return tm.GetTileset( s, tileWidth, tileHeight );
	//make sure to set up tileset here
}

void GameSession::UpdateEnemiesPrePhysics()
{
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePrePhysics();
		current = current->next;
	}
}

void GameSession::UpdateEnemiesPhysics()
{
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	player->physicsOver = false;
	///for( int i = 0; i < NUM_STEPS; ++i )
	//	player->UpdatePhysics( );


	for( substep = 0; substep < NUM_STEPS; ++substep )
	{
		for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
		{
			(*it)->UpdatePhysics();
		}

		player->UpdatePhysics();

		Enemy *current = activeEnemyList;
		while( current != NULL )
		{
			
			current->UpdatePhysics();
		
			current = current->next;
		}
	}
}

void GameSession::UpdateEnemiesPostPhysics()
{
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	int keyLength = 16 * 5;
	keyFrame = totalGameFrames % keyLength;
	

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		Enemy *temp = current->next; //need this in case enemy is removed during its update

		current->UpdatePostPhysics();
		
		if( current->keyShader != NULL )
		{
			float halftot = keyLength / 2;
			float fac;
			if( keyFrame < keyLength / 2 )
			{
				fac = keyFrame / (halftot-1);
			}
			else
			{
				fac = 1.f - ( keyFrame - halftot ) / (halftot-1);
			}
			current->keyShader->setParameter( "prop", fac );
		}

		current = temp;
	}
}

void GameSession::UpdateEffects()
{
	Enemy *curr;
	Enemy *next;
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		curr = effectLists[i];

		while( curr != NULL )
		{
			next = curr->next;

			curr->UpdatePostPhysics();
		
			curr = next;
		}
	}
}

void GameSession::UpdateEnemiesDraw()
{
	CrawlerReverser *cr = drawCrawlerReversers;
	while( cr != NULL )
	{
		cr->Draw( preScreenTex );
		cr = cr->drawNext;
	}
	//CrawlerReverser *cr = 

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
	//	cout << "draw" << endl;
		if( current->type != Enemy::BASICEFFECT && ( pauseFrames < 2 || current->receivedHit == NULL ) )
		{
			current->Draw( preScreenTex );
		}
		current = current->next;
	}
}

void GameSession::UpdateEnemiesSprites()
{
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
	//	current->up();
		current = current->next;
	}
}

void GameSession::DrawEffects( EffectLayer layer )
{
	Enemy *currentEnem = effectLists[layer];
	while( currentEnem != NULL )
	{
		currentEnem->Draw( preScreenTex );	
		currentEnem = currentEnem->next;
	}
}

void GameSession::Test( Edge *e )
{
	cout << "testing" << endl;
}

void GameSession::AddEffect( EffectLayer layer, Enemy *e )
{
	Enemy *& fxList = effectLists[layer];
	if( fxList != NULL )
	{
		fxList->prev = e;
		e->next = fxList;
		fxList = e;
	}
	else
	{
		fxList = e;
	}

	if( player->record > 0 )
	{
		e->spawnedByClone = true;
	}
}

void GameSession::AddEnemy( Enemy *e )
{
	//assert( e->spawned );
//	cout << "ADD ENEMY:" << (int)e << ", type: " << (int)e->type << endl;
	if( e->type == Enemy::BOSS_BIRD )
	{
		//probably will not actually use this and will use a separate spacial trigger or a gate

		//cutPlayerInput = true;
	}
	//if( e->type == Enemy::BASICTURRET )
	//{
	//	cout << "ADDING BASIC TURRET NOW: " << endl;
//	}
	//cout << "adding enemy: " << e->type << endl;
	if( activeEnemyList != NULL )
	{
		activeEnemyList->prev = e;
		e->next = activeEnemyList;
		activeEnemyList = e;
	}
	else
	{
		activeEnemyList = e;
	}

	if( player->record > 0 )
	{
		e->spawnedByClone = true;
	}
	

	/*int counter = 0;
	Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		if( counter > 100 && counter < 150 )
		{
			cout << "e: " << (int)e << ", type: " << (int)e->type << endl;
		}
		curr = curr->next;
		++counter;
	}*/
}

void GameSession::RemoveEnemy( Enemy *e )
{
	assert( activeEnemyList != NULL );
	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if( prev == NULL && next == NULL )
	{
		//cout << "etype: " << (int)e->type << ", activelist: " << (int)activeEnemyList->type << endl;
		activeEnemyList = NULL;
		
	}
	else
	{
		if( e == activeEnemyList )
		{
			assert( next != NULL );
			
			next->prev = NULL;
			
			activeEnemyList = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}

	//if( e->type != e->BASICEFFECT )
	//{
	//	/*if( e->hasMonitor )
	//	{
	//		cout << "adding monitor!" << endl;
	//		e->monitor->position = e->position;
	//		AddEnemy( e->monitor );
	//	}*/

	//	cout << "adding an inactive enemy!" << endl;
	//	//cout << "secret count: " << CountActiveEnemies() << endl;
	//	if( inactiveEnemyList == NULL )
	//	{
	//		inactiveEnemyList = e;
	//		e->next = NULL;
	//		e->prev = NULL;
	//	}
	//	else
	//	{
	//		//cout << "creating more dead clone enemies" << endl;
	//		e->next = inactiveEnemyList;
	//		inactiveEnemyList->prev = e;
	//		inactiveEnemyList = e;
	//	}
	//}

	//might need to give enemies a second next/prev pair for clone power?
	//totally does >.> CLONE POWER
	if( player->record > 0 )
	{
		if( cloneInactiveEnemyList == NULL )
		{
			cloneInactiveEnemyList = e;
			e->next = NULL;
			e->prev = NULL;
			//cout << "creating first dead clone enemy" << endl;

			/*int listSize = 0;
			Enemy *ba = cloneInactiveEnemyList;
			while( ba != NULL )
			{
				listSize++;
				ba = ba->next;
			}

			cout << "size of dead list after first add: " << listSize << endl;*/
		}
		else
		{
			//cout << "creating more dead clone enemies" << endl;
			e->next = cloneInactiveEnemyList;
			cloneInactiveEnemyList->prev = e;
			cloneInactiveEnemyList = e;
		}
	}
	

//	cout << "number of enemies is now: " << CountActiveEnemies() << endl;


	/*if( inactiveEnemyList != NULL )
	{
		inactiveEnemyList->next = e;
	}
	else
	{
		inactiveEnemyList = e;
	}*/
	
	
}

int GameSession::CountActiveEnemies()
{
	Enemy *currEnemy = activeEnemyList;
	int counter = 0;
	while( currEnemy != NULL )
	{
		if( currEnemy->type != currEnemy->BASICEFFECT )
		{
			counter++;	
		}
		currEnemy = currEnemy->next;
	}

	return counter;
}

bool GameSession::LoadLights( ifstream &is, map<int, int> &polyIndex )
{
	int numLights;
	is >> numLights;
	for( int i = 0; i < numLights; ++i )
	{
		int x,y,r,g,b;
		int rad;
		int bright;
		is >> x;
		is >> y;
		is >> r;
		is >> g;
		is >> b;
		is >> rad;
		is >> bright;

		Light *light = new Light( this, Vector2i( x,y ), Color( r,g,b ), rad, bright );
		lightTree->Insert( light );
	}

	return true;
}

bool GameSession::LoadMovingPlats( ifstream &is, map<int, int> &polyIndex )
{
	int numMovingPlats;
	is >> numMovingPlats;
	for( int i = 0; i < numMovingPlats; ++i )
	{
		//dont forget this!
		//int matType;
		//is >> matType;

		//matSet.insert( matType );
		//string matStr;
		//is >> matStr;


		int polyPoints;
		is >> polyPoints;

		list<Vector2i> poly;

		for( int i = 0; i < polyPoints; ++i )
		{
			int px, py;
			is >> px;
			is >> py;
			
			poly.push_back( Vector2i( px, py ) );
		}


			
		list<Vector2i>::iterator it = poly.begin();
		int left = (*it).x;
		int right = (*it).x;
		int top = (*it).y;
		int bottom = (*it).y;
			
		for( ;it != poly.end(); ++it )
		{
			if( (*it).x < left )
				left = (*it).x;

			if( (*it).x > right )
				right = (*it).x;

			if( (*it).y < top )
				top = (*it).y;

			if( (*it).y > bottom )
				bottom = (*it).y;
		}


		//might need to round for perfect accuracy here
		Vector2i center( (left + right ) / 2, (top + bottom) / 2 );

		for( it = poly.begin(); it != poly.end(); ++it )
		{
			(*it).x -= center.x;
			(*it).y -= center.y;
		}

		int pathPoints;
		is >> pathPoints;

		list<Vector2i> path;

		for( int i = 0; i < pathPoints; ++i )
		{
			int x,y;
			is >> x;
			is >> y;
			path.push_back( Vector2i( x, y ) );
		}

			
		MovingTerrain *mt = new MovingTerrain( this, center, path, poly, false, 5 );
		movingPlats.push_back( mt );
	}

	return true;
}

bool GameSession::LoadBGPlats( ifstream &is, map<int, int> &polyIndex )
{
	
	int bgPlatformNum0;
	is >> bgPlatformNum0;
	for( int i = 0; i < bgPlatformNum0; ++i )
	{
		//layer is 1

		int matWorld;
		int matVariation;
		
		is >> matWorld;
		is >> matVariation;

		matSet.insert( pair<int,int>(matWorld,matVariation) );

		int polyPoints;
		is >> polyPoints;
		
		list<Vector2i> poly;
		for( int i = 0; i < polyPoints; ++i )
		{
			int x,y, special;
			is >> x;
			is >> y;
			poly.push_back( Vector2i( x, y ) );
		}



		list<Vector2i>::iterator it = poly.begin();
		list<Edge*> realEdges;
		double left, right, top, bottom;
		for( ; it != poly.end(); ++it )
		{
			Edge *ee = new Edge();
					
  			//edges[currentEdgeIndex + i] = ee;
			ee->v0 = V2d( (*it).x, (*it).y );
			list<Vector2i>::iterator temp = it;
			++temp;
			if( temp == poly.end() )
			{
				ee->v1 = V2d( poly.front().x, poly.front().y );
			}
			else
			{
				ee->v1 = V2d( (*temp).x, (*temp).y );
			}
			
			realEdges.push_back( ee );
			terrainBGTree->Insert( ee );

			double localLeft = min( ee->v0.x, ee->v1.x );
			double localRight = max( ee->v0.x, ee->v1.x );
			double localTop = min( ee->v0.y, ee->v1.y );
			double localBottom = max( ee->v0.y, ee->v1.y ); 
			if( i == 0 )
			{
				left = localLeft;
				right = localRight;
				top = localTop;
				bottom = localBottom;
			}
			else
			{
				left = min( left, localLeft );
				right = max( right, localRight );
				top = min( top, localTop);
				bottom = max( bottom, localBottom);
			}
		}


		for( list<Edge*>::iterator eit = realEdges.begin(); eit != realEdges.end(); ++eit )
		{
			Edge * ee = (*eit);//edges[i + currentEdgeIndex];

			Edge *prev;
			if( eit == realEdges.begin() )
			{
				prev = realEdges.back();
			}
			else
			{
				eit--;
				prev = (*eit);
				eit++;
			}

			Edge *next;
			eit++;
			if( eit == realEdges.end() )
			{
				next = realEdges.front();
				eit--;
			}
			else
			{
				next = (*eit);
				eit--;
			}

			ee->edge0 = prev;
			ee->edge1 = next;
		}


		vector<p2t::Point*> polyline;
		//for( int i = 0; i < polyPoints; ++i )
		for( list<Vector2i>::iterator pit = poly.begin(); pit != poly.end(); ++pit )
		{
			polyline.push_back( new p2t::Point( (*pit).x, (*pit).y ) );
		}

		p2t::CDT * cdt = new p2t::CDT( polyline );
	
		cdt->Triangulate();
		vector<p2t::Triangle*> tris;
		tris = cdt->GetTriangles();
			
		va = new VertexArray( sf::Triangles , tris.size() * 3 );
		VertexArray & v = *va;
		Color testColor( 0x75, 0x70, 0x90 );
		testColor = Color::White;
		Vector2f topLeft( left, top );
		cout << "topLeft: " << topLeft.x << ", " << topLeft.y << endl;
		for( int i = 0; i < tris.size(); ++i )
		{	
			p2t::Point *p = tris[i]->GetPoint( 0 );	
			p2t::Point *p1 = tris[i]->GetPoint( 1 );	
			p2t::Point *p2 = tris[i]->GetPoint( 2 );	
			v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
			v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
			v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );

			Vector2f pp0 = (v[i*3].position - topLeft);
			Vector2f pp1 = (v[i*3+1].position - topLeft);
			Vector2f pp2 = (v[i*3+2].position - topLeft);
			if( i == 0 )
			{
				cout << "pos0: " << pp0.x << ", " << pp0.y << endl;
				cout << "pos1: " << pp1.x << ", " << pp1.y << endl;
				cout << "pos2: " << pp2.x << ", " << pp2.y << endl;
			}
			v[i*3].texCoords = pp0;
			v[i*3+1].texCoords = pp1;
			v[i*3+2].texCoords = pp2;
		}

		polygons.push_back( va );

		VertexArray *polygonVA = va;

		//ground, slope, steep, wall

		//ground

		stringstream ss;

		ss << "Borders/bor_" << matWorld + 1 << "_";

		if( envLevel < 10 )
		{
			ss << "0" << matVariation + 1;
		}
		else
		{
			ss << matVariation + 1;
		}

		ss << ".png";
		
		//Tileset *ts_border = GetTileset( "w1_borders_64x64.png", 8, 64 );
		Tileset *ts_border = GetTileset( ss.str(), 8, 256 );//128 );

		VertexArray *groundVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsFlatGround );
		VertexArray *slopeVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsSlopedGround );
		VertexArray *steepVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsSteepGround );
		VertexArray *wallVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsWall );

		


		bool first = true;
			
		

		TestVA * testva = new TestVA;
		testva->next = NULL;
		//testva->va = va;
		testva->aabb.left = left;
		testva->aabb.top = top;
		testva->aabb.width = right - left;
		testva->aabb.height = bottom - top;
		testva->terrainVA = polygonVA;
		testva->grassVA = NULL;//grassVA;

		testva->ts_border = ts_border;
		testva->groundva = groundVA;
		testva->slopeva = slopeVA;
		testva->steepva = steepVA;
		testva->wallva = wallVA;
		
			
		//cout << "before insert border: " << insertCount << endl;
		borderTree->Insert( testva );
		allVA.push_back( testva );

		//cout << "after insert border: " << insertCount << endl;
		//insertCount++;
			

		delete cdt;
		for( int i = 0; i < polyPoints; ++i )
		{
			delete polyline[i];
		//	delete tris[i];
		}

		//no grass for now
	}
}

bool GameSession::LoadGates( ifstream &is, map<int, int> &polyIndex )
{
	is >> numGates;
	gates = new Gate*[numGates];
	for( int i = 0; i < numGates; ++i )
	{
		int gType;
		int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
		int numKeysRequired;
		string behindyouStr;

		is >> gType;
		//is >> numKeysRequired; 
		is >> poly0Index;
		is >> vertexIndex0;
		is >> poly1Index;
		is >> vertexIndex1;
		is >> behindyouStr;
		
		bool reformBehindYou;
		if( behindyouStr == "+reform" )
		{
			reformBehindYou = true;
		}
		else if( behindyouStr == "-reform" )
		{
			reformBehindYou = false;
		}
		else
		{
			cout << "behind you error: " << behindyouStr << endl;
			assert( false );
		}

		Edge *edge0 = edges[polyIndex[poly0Index] + vertexIndex0];
		Edge *edge1 = edges[polyIndex[poly1Index] + vertexIndex1];

		V2d point0 = edge0->v0;
		V2d point1 = edge1->v0;

		Gate::GateType gateType = (Gate::GateType)gType;
		//if( gateType == Gate::CRITICAL )
		//{
		//	cout << "MAKING NEW CRITICAL" << endl;
		//	Critical *crit = new Critical( point0, point1 );
		//	//wastes space for the gates already made but idk what to change. make it a new system?
		//	//lets try it for now lol

		//	//the extra pointers just get stuffed at the end
		//	--numGates;
		//	--i;
		//	//will have to differentiate later for more items. but not for now!
		//	itemTree->Insert( crit );
		//	
		//	continue;
		//}

		Gate * gate = new Gate( this, gateType, reformBehindYou );

		

		gate->temp0prev = edge0->edge0;
		gate->temp0next = edge0;
		gate->temp1prev = edge1->edge0;
		gate->temp1next = edge1;

			
		

		gate->edgeA = new Edge;
		gate->edgeA->edgeType = Edge::CLOSED_GATE;
		gate->edgeA->info = gate;
		gate->edgeB = new Edge;
		gate->edgeB->edgeType = Edge::CLOSED_GATE;
		gate->edgeB->info = gate;

		

		gate->edgeA->v0 = point0;
		gate->edgeA->v1 = point1;

		gate->edgeB->v0 = point1;
		gate->edgeB->v1 = point0;
			
		gate->next = NULL;
		gate->prev = NULL;

		gate->CalcAABB();

		//gate->v0 = point0;
		//gate->v1 = point1;
			
			
		gates[i] = gate;

		gate->SetLocked( true );

		gate->UpdateLine();

		terrainTree->Insert( gate->edgeA );
		terrainTree->Insert( gate->edgeB );

		cout << "inserting gate: " << gate->edgeA << endl;
		gateTree->Insert( gate );

		//gateMap[gate->edgeA] = gate;
		//gateMap[gate->edgeB] = gate;
	}

	return true;
}

bool GameSession::LoadEnemies( ifstream &is, map<int, int> &polyIndex )
{
	totalNumberBullets = 0;

	int numGroups;
	is >> numGroups;
	for( int i = 0; i < numGroups; ++i )
	{
		string gName;
		is >> gName;
		int numActors;
		is >> numActors;

		for( int j = 0; j < numActors; ++j )
		{
			string typeName;
			is >> typeName;

			Enemy *enem = NULL;

			if( typeName == "goal" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				Goal *enemy = new Goal( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );

				goalPos = enemy->position;

				V2d gPos = enemy->ground->GetPoint( enemy->edgeQuantity );
				V2d norm = enemy->ground->Normal();
				double nodeHeight = 104;
				goalNodePos = gPos + norm * nodeHeight;
				cout << "setting goalPos: " << goalPos.x << ", " << goalPos.y << endl;
			}
			else if( typeName == "healthfly" )
			{
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;

				int color;
				is >> color;
				
				HealthFly::FlyType fType = (HealthFly::FlyType)color;
				//Patroller *enemy = new Patroller( this, Vector2i( xPos, yPos ), localPath, loop, speed );
				//enemy->Monitor::MonitorType
				
				HealthFly *enemy = new HealthFly( this, Vector2i( xPos, yPos ), fType );
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "poi" )
			{
				string air;
				string pname;
				PoiInfo *pi = NULL;

				is >> air;

				if( air == "+air" )
				{
					Vector2i pos;
					is >> pos.x;
					is >> pos.y;
					
					
					is >> pname;

					string barStr;
					is >> barStr;

					int hasCamProperties;
					is >> hasCamProperties;

					pi = new PoiInfo( pname, pos );

					pi->hasCameraProperties = hasCamProperties;

					if( pi->hasCameraProperties )
					{
						float camZoom;
						is >> camZoom;

						pi->cameraZoom = camZoom;
					}
					else
					{
					}

					

					if( barStr == "-" )
					{
					}
					else if( barStr == "x" )
					{
	/*					bleft = owner->poiMap["coyfightleft"]->barrier;
	bright = owner->poiMap["coyfightright"]->barrier;
	btop = owner->poiMap["coyfighttop"]->barrier;
	bbot = owner->poiMap["coyfightbot"]->barrier;*/

						Barrier *b = new Barrier( this, pi, true, pos.x, ( player->position.x > pos.x ),  NULL );
						string na = b->poi->name;
						/*if( na == "coyfightleft"
							|| na == "coyfightright"
							|| na == "coyfighttop"
							|| na == "coyfightbot" )
							b->triggered = true;*/
						barriers.push_back( b );
						
					}
					else if( barStr == "y" )
					{
						Barrier *b = new Barrier( this, pi, false, pos.y, ( player->position.y > pos.y ), NULL );
						//b->triggered = true;
						barriers.push_back( b );
					}
					else
					{
						assert( 0 );
					}

					if( barStr != "-" )
					{
						if( fileName == "Maps/poitest.brknk" )
						{
							if( pname == "birdfighttrigger" )
							{
								/*BirdMeetingCallback *bmc = new BirdMeetingCallback( this );
								barriers.back()->callback = bmc;*/
							}
						}
					}
				}
				else if( air == "-air" )
				{
					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					is >> pname;

					string barStr;
					is >> barStr;

					Edge *e = edges[polyIndex[terrainIndex] + edgeIndex];

					V2d p = e->GetPoint( edgeQuantity );

					pi = new PoiInfo( pname, e,
						edgeQuantity );

					if( barStr == "-" )
					{
					}
					else if( barStr == "x" )
					{
						barriers.push_back( new Barrier( this, pi, true, floor( p.x + .5 ), ( player->position.x > p.x ), NULL  ) );
					}
					else if( barStr == "y" )
					{
						barriers.push_back( new Barrier( this, pi, false, floor( p.y + .5 ), ( player->position.y > p.y ), NULL ) );
					}
					else
					{
						assert( 0 );
					}

					
					//edges[polyIndex[terrainIndex] + edgeIndex]					
				}
				else
				{
					cout << "air failure: " << air << endl;
					assert( 0 );
				}

				poiMap[pname] = pi;
				//poiMap
			}
			else if( typeName == "key" )
			{
				Vector2i pos;
				
				is >> pos.x;
				is >> pos.y;

				int numKeys;
				is >> numKeys;

				keyNumberObjects.push_back( new KeyNumberObj( pos, numKeys ) );
			}

			//w1
			else if( typeName == "patroller" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for( int i = 0; i < pathLength; ++i )
				{
					int localX,localY;
					is >> localX;
					is >> localY;
					localPath.push_back( Vector2i( localX, localY ) );
				}


				bool loop;
				string loopStr;
				is >> loopStr;

				if( loopStr == "+loop" )
				{
					loop = true;
				}
				else if( loopStr == "-loop" )
				{
					loop = false;
				}
				else
				{
					assert( false && "should be a boolean" );
				}


				int speed;
				is >> speed;
				Patroller *enemy = new Patroller( this, hasMonitor, Vector2i( xPos, yPos ), localPath, loop, speed );
				//Specter *enemy = new Specter( this, Vector2i( xPos, yPos ) );
				//enemy->Monitor::MonitorType
				

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "crawlerreverser" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				CrawlerReverser *cr = new CrawlerReverser( this, edges[polyIndex[terrainIndex] + edgeIndex],
					edgeQuantity );

				crawlerReverserTree->Insert( cr );
				//Crawler *enemy = new Crawler( this, edges[polyIndex[terrainIndex] + edgeIndex], 
				//	edgeQuantity, clockwise, speed );
				//enemyTree = Insert( enemyTree, enemy );
				//fullEnemyList.push_back( enemy );

				//enemyTree->Insert( enemy );
			}
			else if( typeName == "bosscrawler" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				Boss_Crawler *enemy = new Boss_Crawler( this, edges[polyIndex[terrainIndex] + edgeIndex],
					edgeQuantity );

				fullEnemyList.push_back( enemy );

				b_crawler = enemy;
			}
			else if( typeName == "basicturret" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				double bulletSpeed;
				is >> bulletSpeed;

				int framesWait;
				is >> framesWait;

				BasicTurret *enemy = new BasicTurret( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait );
				//cout << "turret pos: " << enemy->position.x << ", " << enemy->position.y << endl;
				//cout << "player pos: " << player->position.x << ", " << player->position.y << endl;
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "crawler" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				bool clockwise;
				string cwStr;
				is >> cwStr;

				if( cwStr == "+clockwise" )
					clockwise = true;
				else if( cwStr == "-clockwise" )
					clockwise = false;
				else
				{
					assert( false && "boolean problem" );
				}

				float speed;
				is >> speed;

				if( hasMonitor )
				{
					cout << "preview has monitor" << endl;
				}
				else
				{
					cout << "preview NO MONITOR" << endl;
				}

				//BossCrawler *enemy = new BossCrawler( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				Crawler *enemy = new Crawler( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, clockwise, speed );


				/*if( enemy->hasMonitor )
					cout << "crawler with monitor!" << endl;
				else
					cout << "no monitor here" << endl;*/

				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "foottrap" )
			{
				//cout << "loading foottrap" << endl;
				//always grounded
					

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				FootTrap *enemy = new FootTrap( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				//int nexusIndex = 1;
				//Nexus *enemy = new Nexus( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
				//	nexusIndex );				

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}

			//w2
			else if( typeName == "bat" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for( int i = 0; i < pathLength; ++i )
				{
					int localX,localY;
					is >> localX;
					is >> localY;
					localPath.push_back( Vector2i( localX, localY ) );
				}


				bool loop;
				string loopStr;
				is >> loopStr;

				if( loopStr == "+loop" )
				{
					loop = true;
				}
				else if( loopStr == "-loop" )
				{
					loop = false;
				}
				else
				{
					assert( false && "should be a boolean" );
				}

				int bulletSpeed;
				is >> bulletSpeed;

				//int nodeDistance;
				//is >> nodeDistance;

				int framesBetweenNodes;
				is >> framesBetweenNodes;
				//int speed;
				//is >> speed;
				Bat *enemy = new Bat( this, hasMonitor, Vector2i( xPos, yPos ), localPath, 
					bulletSpeed, framesBetweenNodes, loop );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
				//Boss_Skeleton *enemy 
				//	= new Boss_Skeleton( this, Vector2i( xPos, yPos ) );

				//Turtle *enemy = new Turtle( this, Vector2i( xPos, yPos ) );

				//Pulser *enemy = new Pulser( this, Vector2i( xPos, yPos ), localPath,
				//	framesBetweenNodes, loop );
				//Ghost *enemy = new Ghost( this, Vector2i( xPos, yPos ), 10 );//framesBetweenNodes );
				//Shark * enemy = new Shark( this, Vector2i( xPos, yPos ), 10 );
				
				//specterTree->in
				//CoralNanobots *enemy = new CoralNanobots( this, Vector2i( xPos, yPos ), 10 );
				//Swarm *enemy = new Swarm( this, Vector2i( xPos, yPos ) );
				//Owl *enemy = new Owl( this, Vector2i( xPos, yPos ), 10, 60, true );//bulletSpeed, framesBetweenNodes, true );
				//enemy->Monitor::MonitorType
				
				
				//b_bird
				/*Boss_Bird *enemy 
					= new Boss_Bird( this, Vector2i( xPos, yPos ) );
				b_bird = enemy;*/
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				//fullEnemyList.push_back( enemy );
				//enem = enemy;

				//enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "poisonfrog" )
			{
				//always grounded

			/*	of << (int)monitorType << endl;
	of << gravFactor << endl;
	of << jumpStrength.x << " " << jumpStrength.y << endl;
	of << jumpWaitFrames << endl;*/

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				int gravFactor;
				is >> gravFactor;

				int jumpStrengthX;
				is >> jumpStrengthX;

				int jumpStrengthY;
				is >> jumpStrengthY;

				int jumpFramesWait;
				is >> jumpFramesWait;

				/*bool clockwise;
				string cwStr;
				is >> cwStr;

				if( cwStr == "+clockwise" )
					clockwise = true;
				else if( cwStr == "-clockwise" )
					clockwise = false;
				else
				{
					assert( false && "boolean problem" );
				}*/

				//float speed;
				//is >> speed;

				//BossCrawler *enemy = new BossCrawler( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				PoisonFrog *enemy = new PoisonFrog( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, gravFactor, Vector2i( jumpStrengthX, jumpStrengthY ), jumpFramesWait );//, clockwise );//, speed );

				

				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "stagbeetle" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				bool clockwise;
				string cwStr;
				is >> cwStr;

				if( cwStr == "+clockwise" )
					clockwise = true;
				else if( cwStr == "-clockwise" )
					clockwise = false;
				else
				{
					assert( false && "boolean problem" );
				}

				float speed;
				is >> speed;

				//BossCrawler *enemy = new BossCrawler( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				StagBeetle *enemy = new StagBeetle( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, clockwise, speed );
				//Boss_Coyote *enemy = new Boss_Coyote( this, edges[polyIndex[terrainIndex] + edgeIndex], 
				//	edgeQuantity );
				//Boss_Tiger *enemy = new Boss_Tiger( this, edges[polyIndex[terrainIndex] + edgeIndex], 
				//	edgeQuantity );

				//Badger *enemy = new Badger( this, edges[polyIndex[terrainIndex] + edgeIndex], 
				//	edgeQuantity, clockwise, speed, 10 );

			//	Cheetah *enemy = new Cheetah( this, edges[polyIndex[terrainIndex] + edgeIndex], 
			//		edgeQuantity, clockwise );

				//Spider *enemy = new Spider( this, edges[polyIndex[terrainIndex] + edgeIndex], 
				//	edgeQuantity );

				

				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "curveturret" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				double bulletSpeed;
				is >> bulletSpeed;

				int framesWait;
				is >> framesWait;

				int xGravFactor;
				is >> xGravFactor;

				int yGravFactor;
				is >> yGravFactor;

				bool relative = false;
				string relativeGravStr;
				is >> relativeGravStr;
				if( relativeGravStr == "+relative" )
				{
					relative = true;
				}

				//Cactus *enemy = new Cactus( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait,
				//	Vector2i( xGravFactor, yGravFactor ), relative );
				//Overgrowth *enemy = new Overgrowth( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, 10, 60 );
				CurveTurret *enemy = new CurveTurret( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait,
					Vector2i( xGravFactor, yGravFactor ), relative );

				//cout << "turret pos: " << enemy->position.x << ", " << enemy->position.y << endl;
				//cout << "player pos: " << player->position.x << ", " << player->position.y << endl;
				
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}

			//w3
			else if( typeName == "pulser" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for( int i = 0; i < pathLength; ++i )
				{
					int localX,localY;
					is >> localX;
					is >> localY;
					localPath.push_back( Vector2i( localX, localY ) );
				}


				bool loop;
				string loopStr;
				is >> loopStr;

				if( loopStr == "+loop" )
				{
					loop = true;
				}
				else if( loopStr == "-loop" )
				{
					loop = false;
				}
				else
				{
					assert( false && "should be a boolean" );
				}

				int framesBetweenNodes;
				is >> framesBetweenNodes;

				Pulser *enemy = new Pulser( this,hasMonitor, Vector2i( xPos, yPos ), localPath,
					framesBetweenNodes, loop );
				
				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "cactus" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				int bulletSpeed;
				is >> bulletSpeed;

				int rhythm;
				is >> rhythm;

				int amplitude;
				is >> amplitude;

				//CurveTurret *enemy = new CurveTurret( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait,
				//	Vector2i( xGravFactor, yGravFactor ), relative );
				//Overgrowth *enemy = new Overgrowth( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, 10, 60 );
				Cactus *enemy = new Cactus( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, rhythm,
					amplitude );

				//cout << "turret pos: " << enemy->position.x << ", " << enemy->position.y << endl;
				//cout << "player pos: " << player->position.x << ", " << player->position.y << endl;
				
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "owl" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				
				int moveSpeed;
				is >> moveSpeed;

				int bulletSpeed;
				is >> bulletSpeed;

				int rhythmFrames;
				is >> rhythmFrames;
				

				Owl *enemy = new Owl( this, hasMonitor, Vector2i( xPos, yPos ), moveSpeed,
					bulletSpeed, rhythmFrames );
				
				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "badger" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				int speed;
				is >> speed;

				int jumpStrength;
				is >> jumpStrength;
				
				Badger *enemy = new Badger( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, true,
					speed, jumpStrength );

				
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "bosscoyote" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				Boss_Coyote *enemy = new Boss_Coyote( this, edges[polyIndex[terrainIndex] + edgeIndex],
					edgeQuantity );
				b_coyote = enemy;

				fullEnemyList.push_back( enemy );
			}

			//w4
			else if( typeName == "turtle" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				Vector2i delta( 1000, -1000 );
				Vector2i pos( xPos, yPos );
				Turtle *enemy = new Turtle( this, hasMonitor, Vector2i( xPos, yPos ) );
				//Narwhal *enemy = new Narwhal( this, hasMonitor, pos, pos + delta, 20 ); 
				//Copycat *enemy = new Copycat( this, hasMonitor, pos );	
				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "coral" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;	

				int moveFrames;
				is >> moveFrames;

				//CoralNanobots *enemy = new CoralNanobots( this, hasMonitor, 
				//	Vector2i( xPos, yPos ), moveFrames );
				SecurityWeb * enemy = new SecurityWeb( this,
					hasMonitor, Vector2i( xPos, yPos ), 8, 0, 10 );
				
				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "cheetah" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;
				
				Cheetah *enemy = new Cheetah( this, hasMonitor,
					edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );

				
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "spider" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				int speed;
				is >> speed;
				
				Spider *enemy = new Spider( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
					speed );

				
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}

			//w5
			else if( typeName == "swarm" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				int liveFrames;
				is >> liveFrames;

				Swarm *enemy = new Swarm( this, Vector2i( xPos, yPos ), liveFrames );
				
				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "shark" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				int circleFrames;
				is >> circleFrames;

				Shark *enemy = new Shark( this, hasMonitor, Vector2i( xPos, yPos ), circleFrames );
				
				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "ghost" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				int speed;
				is >> speed;

				Ghost *enemy = new Ghost( this, hasMonitor, Vector2i( xPos, yPos ), speed );
				//Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ),
				//	400, 60, 1 );

				
				
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "overgrowth" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

//				Overgrowth *enemy = new Overgrowth( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
	//				10, 60 );
				/*GrowingTree *enemy = new GrowingTree( this,
					hasMonitor, 
					edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, 
					400 );*/

				GrowingTree * enemy = new GrowingTree( this, hasMonitor,
					edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, 32, 0, 1000 );

				
				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}

			//w6
			else if( typeName == "specter" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				Specter *enemy = new Specter( this, hasMonitor, Vector2i( xPos, yPos ) );
				//Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ),
				//	400, 50, 60, 1 );
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "narwhal" )
			{
					
				int xPos,yPos;

				//always air
				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				Vector2i dest;
				is >> dest.x;
				is >> dest.y;

				int moveFrames;
				is >> moveFrames;

				Narwhal *enemy = new Narwhal( this, hasMonitor, 
					Vector2i( xPos, yPos ),	dest, moveFrames );
				//Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ),
				//	400, 50, 60, 1 );
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "copycat" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				Copycat *enemy = new Copycat( this, hasMonitor, Vector2i( xPos, yPos ) );
				//Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ),
				//	400, 50, 60, 1 );
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			else if( typeName == "gorilla" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;	

				int wallWidth;
				is >> wallWidth;

				int followFrames;
				is >> followFrames;

				//Gorilla *enemy = new Specter( this, hasMonitor, Vector2i( xPos, yPos ) );
				Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ),
					wallWidth, followFrames );
					//400, 60 );
				//give the enemy the monitor inside it. create a new monitor and store it inside the enemy

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );// = Insert( enemyTree, enemy );
			}
			
			else if( typeName == "nexus" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int nexusIndex;
				is >> nexusIndex;

				Nexus *enemy = new Nexus( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
					nexusIndex );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "shippickup" )
			{
				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int facingRight;
				is >> facingRight;

				ShipPickup *enemy = new ShipPickup( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
					facingRight );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else
			{
				assert( false && "not a valid type name" );
			}

			//if( enem != NULL )
			//{
			//	totalNumberBullets += enem->NumTotalBullets();
			//}

		}
	}

	//create VA

	if( totalNumberBullets > 0 )
	{
		bigBulletVA = new VertexArray( sf::Quads, totalNumberBullets * 4 );
		VertexArray &bva = *bigBulletVA;
		for( int i = 0; i < totalNumberBullets * 4; ++i )
		{
			bva[i].position = Vector2f( 0, 0 );
		}
		ts_basicBullets = GetTileset( "bullet_64x64.png", 64, 64 );
	}
	else
	{
		ts_basicBullets = NULL;
	}

	return true;
}

bool GameSession::OpenFile( string fileName )
{
	currentFile = fileName;
	int insertCount = 0;
	ifstream is;
	is.open( fileName );//+ ".brknk" );
	if( is.is_open() )
	{
		is >> envType;

		is >> envLevel;
		

		//cout << "just read it: " << envType << ", " << envLevel << endl;

		is >> leftBounds;
		is >> topBounds;
		is >> boundsWidth;
		is >> boundsHeight;

		is >> numPoints;
		points = new Vector2<double>[numPoints];
		

		is >> player->position.x;
		is >> player->position.y;
		originalPos.x = player->position.x;
		originalPos.y = player->position.y;

		is >> goalPos.x;
		is >> goalPos.y;

		int pointsLeft = numPoints;

		int pointCounter = 0;

		edges = new Edge*[numPoints];

		int polyCounter = 0;
		//could use an array later if i wanted to
		map<int, int> polyIndex;

		while( pointCounter < numPoints )
		{
			int matWorld;
			is >> matWorld;

			int matVariation;
			is >> matVariation;

			
			//matWorld = 6;
			//matWorld = 2;
			matSet.insert( pair<int,int>( matWorld, matVariation ) );

			int polyPoints;
			is >> polyPoints;

			polyIndex[polyCounter] = pointCounter;

			int currentEdgeIndex = pointCounter;
			for( int i = 0; i < polyPoints; ++i )
			{
				int px, py;
				is >> px;
				is >> py;
				//is >> spec;
			
				points[pointCounter].x = px;
				points[pointCounter].y = py;
				++pointCounter;
			}

			double left, right, top, bottom;
			for( int i = 0; i < polyPoints; ++i )
			{
				Edge *ee = new Edge();
					
  				edges[currentEdgeIndex + i] = ee;
				ee->v0 = points[i+currentEdgeIndex];
				if( i < polyPoints - 1 )
					ee->v1 = points[i+1 + currentEdgeIndex];
				else
					ee->v1 = points[currentEdgeIndex];

				terrainTree->Insert( ee );

				double localLeft = min( ee->v0.x, ee->v1.x );
				double localRight = max( ee->v0.x, ee->v1.x );
				double localTop = min( ee->v0.y, ee->v1.y );
				double localBottom = max( ee->v0.y, ee->v1.y ); 
				if( i == 0 )
				{
					left = localLeft;
					right = localRight;
					top = localTop;
					bottom = localBottom;
				}
				else
				{
					left = min( left, localLeft );
					right = max( right, localRight );
					top = min( top, localTop);
					bottom = max( bottom, localBottom);
				}

			}


			for( int i = 0; i < polyPoints; ++i )
			{
				Edge * ee = edges[i + currentEdgeIndex];
				if( i == 0 )
				{
					ee->edge0 = edges[currentEdgeIndex + polyPoints - 1];
					ee->edge1 = edges[currentEdgeIndex + 1];
				}
				else if( i == polyPoints - 1 )
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex];
				
				}
				else
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex + i + 1];
				}
			}

			int edgesWithSegments;
			is >> edgesWithSegments;

			
			list<GrassSegment> segments;
			for( int i = 0; i < edgesWithSegments; ++i )
			{
				int edgeIndex;
				is >> edgeIndex;
				int numSegments;
				is >> numSegments;
				for( int j = 0; j < numSegments; ++j )
				{
					int index;
					is >> index;
					int reps;
					is >> reps;

					segments.push_back( GrassSegment( edgeIndex, index, reps ) );
				}
			}

			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{
				V2d A,B,C,D;
				Edge * currE = edges[currentEdgeIndex + (*it).edgeIndex];
				V2d v0 = currE->v0;
				V2d v1 = currE->v1;

				double grassSize = 22;
				double grassSpacing = -5;

				double edgeLength = length( v1 - v0 );
				double remainder = edgeLength / ( grassSize + grassSpacing );

				double num = floor( remainder ) + 1;

				int reps = (*it).reps;

				V2d edgeDir = normalize( v1 - v0 );
				
				//V2d ABmin = v0 + (v1-v0) * (double)(*it).index / num - grassSize / 2 );
				V2d ABmin = v0 + edgeDir * ( edgeLength * (double)(*it).index / num - grassSize / 2 );
				V2d ABmax = v0 + edgeDir * ( edgeLength * (double)( (*it).index + reps )/ num + grassSize / 2 );
				double height = grassSize / 2;
				V2d normal = normalize( v1 - v0 );
				double temp = -normal.x;
				normal.x = normal.y;
				normal.y = temp;

				A = ABmin + normal * height;
				B = ABmax + normal * height;
				C = ABmax;
				D = ABmin;
				
				Grass * g = new Grass;
				g->A = A;
				g->B = B;
				g->C = C;
				g->D = D;

				grassTree->Insert( g );
			}

			vector<p2t::Point*> polyline;
			for( int i = 0; i < polyPoints; ++i )
			{
				polyline.push_back( new p2t::Point( points[currentEdgeIndex +i].x, points[currentEdgeIndex +i].y ) );

			}

			
			
			p2t::CDT * cdt = new p2t::CDT( polyline );


			//cdt->add
			
			vector<p2t::Triangle*> tris;

			cdt->Triangulate();
				
			tris = cdt->GetTriangles();
			//int adjustedCount = 0;
			////do
			//for( int j = 0; j < 1; ++j )
			//{
			//	adjustedCount = 0;
			//	cdt->Triangulate();
			//	
			//	tris = cdt->GetTriangles();

			//	for( int i = 0; i < tris.size(); ++i )
			//	{
			//		p2t::Point *p = tris[i]->GetPoint( 0 );	
			//		p2t::Point *p1 = tris[i]->GetPoint( 1 );	
			//		p2t::Point *p2 = tris[i]->GetPoint( 2 );

			//		V2d pp0( p->x, p->y );
			//		V2d pp1( p1->x, p1->y );
			//		V2d pp2( p2->x, p2->y );

			//		int f = 200;
			//		if( length( pp0 - pp1 ) > f || length( pp0 - pp2 ) > f
			//			|| length( pp1 - pp2 ) > f )
			//		{
			//			adjustedCount++;
			//			p2t::Point *pNew = new p2t::Point;
			//			V2d mid = ( pp0 + pp1 + pp2 ) / 3.0;
			//			pNew->x = mid.x;
			//			pNew->y = mid.y;
			//			cdt->AddPoint( pNew );
			//		}
			//	}
			//}

			//if( adjustedCount > 0 )
			//{
			//	cdt->Triangulate();
			//	
			//	tris = cdt->GetTriangles();
			//}
			//while( adjustedCount > 0 );

			va = new VertexArray( sf::Triangles , tris.size() * 3 );
			VertexArray & v = *va;
			Color testColor( 0x75, 0x70, 0x90 );
			testColor = Color::White;
			Vector2f topLeft( left, top );
			//cout << "topleft: " << topLeft.x << ", " << topLeft.y << endl;
			for( int i = 0; i < tris.size(); ++i )
			{	
				p2t::Point *p = tris[i]->GetPoint( 0 );	
				p2t::Point *p1 = tris[i]->GetPoint( 1 );	
				p2t::Point *p2 = tris[i]->GetPoint( 2 );	
				v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
				v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
				v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
				/*Vector2f pp0 = (v[i*3].position - topLeft);
				Vector2f pp1 = (v[i*3+1].position - topLeft);
				Vector2f pp2 = (v[i*3+2].position - topLeft);
				pp0 = Vector2f( (int)pp0.x % 1024, (int)pp0.y % 1024 );
				pp1 = Vector2f( (int)pp1.x % 1024, (int)pp1.y % 1024 );
				pp2 = Vector2f( (int)pp2.x % 1024, (int)pp2.y % 1024 );

				if( i == 0 )
				{
					cout << "pos0: " << pp0.x << ", " << pp0.y << endl;
					cout << "pos1: " << pp1.x << ", " << pp1.y << endl;
					cout << "pos2: " << pp2.x << ", " << pp2.y << endl;
				}
				v[i*3].texCoords = pp0;
				v[i*3+1].texCoords = pp1;
				v[i*3+2].texCoords = pp2;*/
			}

			polygons.push_back( va );

			VertexArray *polygonVA = va;

			double totalPerimeter = 0;


			double grassSize = 22;
			double grassSpacing = -5;

			Edge * testEdge = edges[currentEdgeIndex];

			int numGrassTotal = 0;

			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{
				numGrassTotal += (*it).reps + 1;
			}

			VertexArray *grassVA = NULL;
			
			if( numGrassTotal > 0 )
			{
			grassVA = new VertexArray( sf::Quads, numGrassTotal * 4 );

			//cout << "num grass total: " << numGrassTotal << endl;
			VertexArray &grassVa = *va;

			int segIndex = 0;
			int totalGrass = 0;
			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{	
				Edge *segEdge = edges[currentEdgeIndex + (*it).edgeIndex];
				V2d v0 = segEdge->v0;
				V2d v1 = segEdge->v1;

				int start = (*it).index;
				int end = (*it).index + (*it).reps;

				int grassCount = (*it).reps + 1;
				//cout << "Grasscount: " << grassCount << endl;

				double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

				int num = floor( remainder ) + 1;

				for( int i = 0; i < grassCount; ++i )
				{
					//cout << "indexing at: " << i*4 + segIndex * 4 << endl;
					V2d posd = v0 + (v1 - v0 ) * ((double)( i + start ) / num);
					Vector2f pos( posd.x, posd.y );

					Vector2f topLeft = pos + Vector2f( -grassSize / 2, -grassSize / 2 );
					Vector2f topRight = pos + Vector2f( grassSize / 2, -grassSize / 2 );
					Vector2f bottomLeft = pos + Vector2f( -grassSize / 2, grassSize / 2 );
					Vector2f bottomRight = pos + Vector2f( grassSize / 2, grassSize / 2 );

					//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
					//borderVa[i*4].color.a = 10;
					grassVa[(i+totalGrass)*4].position = topLeft;
					grassVa[(i+totalGrass)*4].texCoords = Vector2f( 0, 0 );

					//grassVa[i*4+1].color = Color::Blue;
					//borderVa[i*4+1].color.a = 10;
					grassVa[(i+totalGrass)*4+1].position = bottomLeft;
					grassVa[(i+totalGrass)*4+1].texCoords = Vector2f( 0, grassSize );

					//grassVa[i*4+2].color = Color::Blue;
					//borderVa[i*4+2].color.a = 10;
					grassVa[(i+totalGrass)*4+2].position = bottomRight;
					grassVa[(i+totalGrass)*4+2].texCoords = Vector2f( grassSize, grassSize );

					//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
					//borderVa[i*4+3].color.a = 10;
					grassVa[(i+totalGrass)*4+3].position = topRight;
					grassVa[(i+totalGrass)*4+3].texCoords = Vector2f( grassSize, 0 );
					//++i;
				}
				totalGrass += grassCount;
				segIndex++;
			}
			}
			else
			{
				grassVA = NULL;
			}


			

			//ground, slope, steep, wall

			//ground

			//Tileset *ts_border = GetTileset( "w1_borders_64x64.png", 8, 64 );
			//Tileset *ts_border = GetTileset( "w1_borders_128x128.png", 8, 128 );
			stringstream ss;

			ss << "Borders/bor_" << matWorld + 1 << "_";

			if( envLevel < 10 )
			{
				ss << "0" << matVariation + 1;
			}
			else
			{
				ss << matVariation + 1;
			}

			ss << ".png";
		
			//Tileset *ts_border = GetTileset( "w1_borders_64x64.png", 8, 64 );
			Tileset *ts_border = GetTileset( ss.str(), 8, 256 );

			

			VertexArray *groundVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsFlatGround );
			VertexArray *slopeVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsSlopedGround );
			VertexArray *steepVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsSteepGround );
			VertexArray *wallVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsWall );

			Tileset *ts_plant = GetTileset( "testgrass.png", 32, 32 );
			Tileset *ts_bush = GetTileset( "bush_01_64x64.png", 64, 64 );

			VertexArray *plantVA = SetupPlants( edges[currentEdgeIndex], ts_plant );


			
			

			double polygonArea = 0;
			for( vector<p2t::Triangle*>::iterator it = tris.begin();
				it != tris.end(); ++it )
			{
				polygonArea += GetTriangleArea( (*it) );
			}


			TestVA * testva = new TestVA;
			testva->polyArea = polygonArea;

			VertexArray *bushVA = SetupBushes( 0,  tris, ts_bush );

			//now that I have the area, get a number of random points
			//around the polygon based on how much area there is. 
			//then put plants in those areas

			//VertexArray *decorLayer0VA = SetupDecor0( tris, ts_decor0 );


			//VertexArray *triVA = SetupBorderTris( 0, edges[currentEdgeIndex], ts_border );
			VertexArray *triVA = SetupTransitions( 0, edges[currentEdgeIndex], ts_border );

			Tileset *ts_energyFlow = NULL;//GetTileset( "energyFlow.png", 0, 0 );
			//VertexArray *energyFlowVA = //SetupEnergyFlow( 0, edges[currentEdgeIndex], ts_energyFlow );


			bool first = true;
			
		

			
			testva->plantva = NULL; //temporary
			testva->next = NULL;
			//testva->va = va;
			testva->aabb.left = left;
			testva->aabb.top = top;
			testva->aabb.width = right - left;
			testva->aabb.height = bottom - top;
			testva->terrainVA = polygonVA;
			testva->grassVA = grassVA;
			testva->ts_bush = ts_bush;
			testva->bushVA = bushVA;
			

			testva->ts_border = ts_border;
			testva->groundva = groundVA;
			testva->slopeva = slopeVA;
			testva->steepva = steepVA;
			testva->wallva = wallVA;
			testva->triva = triVA;
			testva->plantva = plantVA;
			testva->ts_plant = ts_plant;
			testva->terrainWorldType = matWorld;
			testva->terrainVariation = matVariation;

			//testva->flowva = energyFlowVA;
			
			//cout << "before insert border: " << insertCount << endl;
			borderTree->Insert( testva );
			allVA.push_back( testva );

			//cout << "after insert border: " << insertCount << endl;
			insertCount++;
			

			delete cdt;
			for( int i = 0; i < polyPoints; ++i )
			{
				delete polyline[i];
			//	delete tris[i];
			}

			//cout << "loaded to here" << endl;
			++polyCounter;
		}
		
		LoadMovingPlats( is, polyIndex );

		LoadBGPlats( is, polyIndex );

		LoadLights( is, polyIndex );

		LoadEnemies( is, polyIndex );
		
		LoadGates( is, polyIndex );

		is.close();

		//loading done. now setup

		SetGlobalBorders();
		CreateZones();
		SetupZones();

		for( int i = 0; i < numGates; ++i )
		{
			Gate *g = gates[i];
			if( g->type == Gate::BIRDFIGHT )
			{
				if( g->zoneA != NULL )
					g->zoneA->showShadow = false;
					//ActivateZone( zone
					//g->zoneA->ac
				//g->SetLocked( false );
				//g->SetLocked( true );
			}
		}
		
		if( poiMap.count( "ship" ) > 0 )
		{
			ResetShipSequence();



			//cloud0a.setPosition( pi->pos.x - 960, pi->pos.y + 270 );
			//cloud0b.setPosition( pi->pos.x - 240, pi->pos.y + 270 );
			//cloud1a.setPosition( pi->pos.x - 960, pi->pos.y + 270 );
			//cloud1b.setPosition( pi->pos.x - 240, pi->pos.y + 270);// + 540 );
			//player->position = poiMap.ship
		}
		else
		{
			drain = true;
			shipSequence = false;
			//normal map
		}

	}
	else
	{

		//new file
		assert( false && "error getting file to edit " );
	}
}

bool cmpPairs(pair<double,int> & a, pair<double,int> & b)
{
	return a.first < b.first;
}

bool cmpPairsDesc( pair<double,int> & a, pair<double,int> & b)
{
	return a.first > b.first;
}

void GameSession::SetGlobalBorders()
{
	borderEdge = NULL;
	//borders not allowed to intersect w/ gates

	V2d topLeft( leftBounds, topBounds );
	V2d topRight( leftBounds + boundsWidth, topBounds );
	V2d bottomRight( leftBounds + boundsWidth, topBounds + boundsHeight );
	V2d bottomLeft( leftBounds, topBounds + boundsHeight );

	//get intersections with top row
	list<pair<double,int>> inters;
	list<Edge*> topEdges;
	list<Edge*> rightEdges;
	list<Edge*> bottomEdges;
	list<Edge*> leftEdges;
	bool segInProcess;
	double segStart;
	int prevIndex;
	bool first;
	

	//top section-----------------
	{
	for( int i = 0; i < numPoints; ++i )
	{
		LineIntersection li = SegmentIntersect( topLeft, topRight, edges[i]->v0, edges[i]->v1 );
		if( !li.parallel ) //or no intersection
		{
			inters.push_back( pair<double, int>( li.position.x, i ) );	
		}
	}

	segInProcess = false;
	segStart = leftBounds;
	first = true;

	inters.sort( cmpPairs );

	for( list<pair<double,int>>::iterator it = inters.begin(); it != inters.end(); ++it )
	{
		int index = (*it).second;
		double xInter = (*it).first;
		Edge *edge = edges[index];
		V2d v0 = edge->v0;
		V2d v1 = edge->v1;

		cout << "processing intersection at: " << xInter << endl;
		
		if( v0.y > topBounds )
		{
			if( first || segInProcess )
			{
				//cout << "a" << endl;
				Edge *newSeg = new Edge;
				newSeg->v0 = V2d( xInter, topBounds );
				newSeg->v1 = V2d( segStart, topBounds );
				newSeg->edgeType = Edge::BORDER;
				
				edge->v1 = newSeg->v0;
				edge->edge1 = newSeg;
				newSeg->edge0 = edge;

				if( segInProcess )
				{
					edges[prevIndex]->edge0 = newSeg;
					newSeg->edge1 = edges[prevIndex];
				}


				topEdges.push_back( newSeg );
				borderEdge = newSeg;


				first = false;
				segInProcess = false;
			}
			
			//corner is outside
		}
		else
		{

			//corner is inside
			first = false;

			segInProcess = true;
			segStart = xInter;
			edge->v0 = V2d( xInter, topBounds );
			prevIndex = index;
			
		}
		
		
		//cout << "intersection at: " << (*it).second << endl;
	}
	
	if( segInProcess )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = topRight;
		newSeg->v1 = V2d( segStart, topBounds );
		newSeg->edgeType = Edge::BORDER;
		
		Edge *edge = edges[prevIndex];
		edge->edge0 = newSeg;
		newSeg->edge1 = edge;

		//cout << "creating final seg!!!" << endl;
		topEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	if( inters.empty() )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = topRight;
		newSeg->v1 = topLeft;
		newSeg->edgeType = Edge::BORDER;

		topEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	}
	//right section-------------------
	{
	inters.clear();

	for( int i = 0; i < numPoints; ++i )
	{
		LineIntersection li = SegmentIntersect( topRight, bottomRight, edges[i]->v0, edges[i]->v1 );
		if( !li.parallel ) //or no intersection
		{
			inters.push_back( pair<double, int>( li.position.y, i ) );	
		}
	}

	segInProcess = false;
	segStart = topBounds;
	prevIndex;
	first = true;

	inters.sort( cmpPairs );

	for( list<pair<double,int>>::iterator it = inters.begin(); it != inters.end(); ++it )
	{
		int index = (*it).second;
		double yInter = (*it).first;
		Edge *edge = edges[index];
		V2d v0 = edge->v0;
		V2d v1 = edge->v1;

		//cout << "processing intersection at: " << yInter << endl;
		
		if( v0.x < topRight.x )
		{
			if( first || segInProcess )
			{
				//cout << "a" << endl;
				Edge *newSeg = new Edge;
				newSeg->v0 = V2d( topRight.x, yInter );
				newSeg->v1 = V2d( topRight.x, segStart );
				newSeg->edgeType = Edge::BORDER;
				
				edge->v1 = newSeg->v0;
				edge->edge1 = newSeg;
				newSeg->edge0 = edge;

				if( segInProcess )
				{
					edges[prevIndex]->edge0 = newSeg;
					newSeg->edge1 = edges[prevIndex];
				}


				rightEdges.push_back( newSeg );
				borderEdge = newSeg;


				first = false;
				segInProcess = false;
			}
			
			//corner is outside
		}
		else
		{

			//corner is inside
			first = false;

			segInProcess = true;
			segStart = yInter;
			edge->v0 = V2d( topRight.x, yInter );
			prevIndex = index;
			
		}		
	}
	
	if( segInProcess )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = bottomRight;
		newSeg->v1 = V2d( topRight.x, segStart );
		newSeg->edgeType = Edge::BORDER;
		
		Edge *edge = edges[prevIndex];
		edge->edge0 = newSeg;
		newSeg->edge1 = edge;

		rightEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	if( inters.empty() )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = bottomRight;
		newSeg->v1 = topRight;
		newSeg->edgeType = Edge::BORDER;

		rightEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	}
	//bottom section
	{
	inters.clear();

	for( int i = 0; i < numPoints; ++i )
	{
		LineIntersection li = SegmentIntersect( bottomRight, bottomLeft, edges[i]->v0, edges[i]->v1 );
		if( !li.parallel ) //or no intersection
		{
			inters.push_back( pair<double, int>( li.position.x, i ) );	
		}
	}

	segInProcess = false;
	segStart = bottomRight.x;
	prevIndex;
	first = true;

	inters.sort( cmpPairsDesc );

	for( list<pair<double,int>>::iterator it = inters.begin(); it != inters.end(); ++it )
	{
		int index = (*it).second;
		double xInter = (*it).first;
		Edge *edge = edges[index];
		V2d v0 = edge->v0;
		V2d v1 = edge->v1;

		//cout << "processing intersection at: " << yInter << endl;
		
		if( v0.y < bottomRight.y )
		{
			if( first || segInProcess )
			{
				//cout << "a" << endl;
				Edge *newSeg = new Edge;
				newSeg->v0 = V2d( xInter, bottomRight.y );
				newSeg->v1 = V2d( segStart, bottomRight.y );
				newSeg->edgeType = Edge::BORDER;
				
				edge->v1 = newSeg->v0;
				edge->edge1 = newSeg;
				newSeg->edge0 = edge;

				if( segInProcess )
				{
					edges[prevIndex]->edge0 = newSeg;
					newSeg->edge1 = edges[prevIndex];
				}


				bottomEdges.push_back( newSeg );
				borderEdge = newSeg;

				first = false;
				segInProcess = false;
			}
			
			//corner is outside
		}
		else
		{

			//corner is inside
			first = false;

			segInProcess = true;
			segStart = xInter;
			edge->v0 = V2d( xInter, bottomRight.y );
			prevIndex = index;
			
		}		
	}
	
	if( segInProcess )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = bottomLeft;
		newSeg->v1 = V2d( segStart, bottomRight.y );
		newSeg->edgeType = Edge::BORDER;
		
		Edge *edge = edges[prevIndex];
		edge->edge0 = newSeg;
		newSeg->edge1 = edge;

		bottomEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	if( inters.empty() )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = bottomLeft;
		newSeg->v1 = bottomRight;
		newSeg->edgeType = Edge::BORDER;

		bottomEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	}
	//left section--------------------
	{
	inters.clear();

	for( int i = 0; i < numPoints; ++i )
	{
		LineIntersection li = SegmentIntersect( bottomLeft, topLeft, edges[i]->v0, edges[i]->v1 );
		if( !li.parallel ) //or no intersection
		{
			inters.push_back( pair<double, int>( li.position.y, i ) );	
		}
	}

	segInProcess = false;
	segStart = bottomLeft.y;
	prevIndex;
	first = true;

	inters.sort( cmpPairsDesc );

	for( list<pair<double,int>>::iterator it = inters.begin(); it != inters.end(); ++it )
	{
		int index = (*it).second;
		double yInter = (*it).first;
		Edge *edge = edges[index];
		V2d v0 = edge->v0;
		V2d v1 = edge->v1;

		//cout << "processing intersection at: " << yInter << endl;
		
		if( v0.x > leftBounds )
		{
			if( first || segInProcess )
			{
				//cout << "a" << endl;
				Edge *newSeg = new Edge;
				newSeg->v0 = V2d( leftBounds, yInter );
				newSeg->v1 = V2d( leftBounds, segStart );
				newSeg->edgeType = Edge::BORDER;
				
				edge->v1 = newSeg->v0;
				edge->edge1 = newSeg;
				newSeg->edge0 = edge;

				if( segInProcess )
				{
					edges[prevIndex]->edge0 = newSeg;
					newSeg->edge1 = edges[prevIndex];
				}


				leftEdges.push_back( newSeg );
				borderEdge = newSeg;


				first = false;
				segInProcess = false;
			}
			
			//corner is outside
		}
		else
		{

			//corner is inside
			first = false;

			segInProcess = true;
			segStart = yInter;
			edge->v0 = V2d( leftBounds, yInter );
			prevIndex = index;
			
		}		
	}
	
	if( segInProcess )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = topLeft;
		newSeg->v1 = V2d( leftBounds, segStart );
		newSeg->edgeType = Edge::BORDER;
		
		Edge *edge = edges[prevIndex];
		edge->edge0 = newSeg;
		newSeg->edge1 = edge;

		leftEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	if( inters.empty() )
	{
		Edge *newSeg = new Edge;
		newSeg->v0 = topLeft;
		newSeg->v1 = bottomLeft;
		newSeg->edgeType = Edge::BORDER;

		leftEdges.push_back( newSeg );
		borderEdge = newSeg;
	}

	}
	//bringing it all together
	int topEdgesSize = topEdges.size();
	int rightEdgesSize = rightEdges.size();
	int bottomEdgesSize = bottomEdges.size();
	int leftEdgesSize = leftEdges.size();

	if( topEdgesSize > 0 && rightEdgesSize > 0 )
	{
		if( topEdges.back()->v0 == rightEdges.front()->v1 )
		{
			topEdges.back()->edge0 = rightEdges.front();
			rightEdges.front()->edge1 = topEdges.back();
		}
	}
	if( rightEdgesSize > 0 && bottomEdgesSize > 0 )
	{
		if( rightEdges.back()->v0 == bottomEdges.front()->v1 )
		{
			rightEdges.back()->edge0 = bottomEdges.front();
			bottomEdges.front()->edge1 = rightEdges.back();
		}
	}
	if( bottomEdgesSize > 0 && leftEdgesSize > 0 )
	{
		if( bottomEdges.back()->v0 == leftEdges.front()->v1 )
		{
			bottomEdges.back()->edge0 = leftEdges.front();
			leftEdges.front()->edge1 = bottomEdges.back();
		}
	}
	if( leftEdgesSize > 0 && topEdgesSize > 0 )
	{
		if( leftEdges.back()->v0 == topEdges.front()->v1 )
		{
			leftEdges.back()->edge0 = topEdges.front();
			topEdges.front()->edge1 = leftEdges.back();
		}
	}

	int debugBorderCount = ( topEdges.size() + rightEdges.size() + bottomEdges.size() + leftEdges.size() ) * 2;
	debugBorders = new VertexArray( sf::Lines, debugBorderCount );
	
	cout << "debugBorderCount: " << debugBorderCount << endl;

	VertexArray &db = *debugBorders;
	int i = 0;
	for( list<Edge*>::iterator it = topEdges.begin(); it != topEdges.end(); ++it )
	{
		db[i * 2].color = Color::Red;
		db[i * 2 + 1].color = Color::Red;
		db[i * 2].position = Vector2f( (*it)->v0.x, (*it)->v0.y );
		db[i * 2 + 1].position = Vector2f( (*it)->v1.x, (*it)->v1.y );

		//cout << "adding edge: " << (*it)->v0.x << ", " << (*it)->v0.y << " to " << (*it)->v1.x << ", " << (*it)->v1.y << endl;
		terrainTree->Insert( (*it) );

		++i;
	}

	for( list<Edge*>::iterator it = rightEdges.begin(); it != rightEdges.end(); ++it )
	{
		db[i * 2].color = Color::Red;
		db[i * 2 + 1].color = Color::Red;
		db[i * 2].position = Vector2f( (*it)->v0.x, (*it)->v0.y );
		db[i * 2 + 1].position = Vector2f( (*it)->v1.x, (*it)->v1.y );

		//cout << "adding edge: " << (*it)->v0.x << ", " << (*it)->v0.y << " to " << (*it)->v1.x << ", " << (*it)->v1.y << endl;
		terrainTree->Insert( (*it) );

		++i;
	}

	for( list<Edge*>::iterator it = bottomEdges.begin(); it != bottomEdges.end(); ++it )
	{
		db[i * 2].color = Color::Red;
		db[i * 2 + 1].color = Color::Red;
		db[i * 2].position = Vector2f( (*it)->v0.x, (*it)->v0.y );
		db[i * 2 + 1].position = Vector2f( (*it)->v1.x, (*it)->v1.y );

		//cout << "adding edge: " << (*it)->v0.x << ", " << (*it)->v0.y << " to " << (*it)->v1.x << ", " << (*it)->v1.y << endl;
		terrainTree->Insert( (*it) );

		++i;
	}

	for( list<Edge*>::iterator it = leftEdges.begin(); it != leftEdges.end(); ++it )
	{
		db[i * 2].color = Color::Red;
		db[i * 2 + 1].color = Color::Red;
		db[i * 2].position = Vector2f( (*it)->v0.x, (*it)->v0.y );
		db[i * 2 + 1].position = Vector2f( (*it)->v1.x, (*it)->v1.y );

		//cout << "adding edge: " << (*it)->v0.x << ", " << (*it)->v0.y << " to " << (*it)->v1.x << ", " << (*it)->v1.y << endl;
		terrainTree->Insert( (*it) );

		++i;
	}

}

void GameSession::CreateZones()
{
	//no gates, no zones!
	for( int i = 0; i < numGates; ++i )
	{
		Gate *g = gates[i];
		//cout << "gate index: " << i << ", a: " << g->edgeA->v0.x << ", " << g->edgeA->v0.y << ", b: "
		//	<< g->edgeA->v1.x << ", " << g->edgeA->v1.y << endl;
		
		Edge *curr = g->edgeA;

		TerrainPolygon tp( NULL );
		V2d v0 = curr->v0;
		V2d v1 = curr->v1;
		list<Edge*> currGates;
		list<Gate*> ignoreGates;
		
		currGates.push_back( curr );
		

		tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;
		while( true )
		{
			if( curr->v0 == g->edgeA->v0 )//curr == g->edgeA )
			{
				//we found a zone!

				if( !tp.IsClockwise() )
				{
					//cout << "found a zone aaa!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;


					for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end() && okayZone; ++zit )
					{
						for( list<Edge*>::iterator cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit )
						{
							for( list<Edge*>::iterator git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git )
							{
								if( (*cit) == (*git) )
								{
									okayZone = false;
								}
							}
							//for( list<Gate*>::iterator git =
							
						}
					}

					if( okayZone )
					{
						Zone *z = new Zone( this, tp );
						z->gates = currGates;
						zones.push_back( z );
					//	cout << "creating a zone with " << currGates.size() << " gatesAAA" << endl;
					//	cout << "actually creating a new zone   1! with " << z->gates.size() << endl;
					}
					

				}
				else
				{
					//cout << "woulda been a zone" << endl;
				}

				break;
			}
			else if( curr == g->edgeB )
			{
				//currGates.push_back( curr );
				//cout << "not a zone even" << endl;
				break;
			}


			tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

			if( curr->edgeType == Edge::CLOSED_GATE )
			{
				Gate *thisGate = (Gate*)curr->info;
				//this loop is so both sides of a gate can't be hit in the same zone
				bool okayGate = true;
				bool quitLoop = false;
				for( list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it )
				{
					Gate *otherGate = (Gate*)(*it)->info;
					

					if( otherGate == thisGate )
					{
						//currGates.erase( it );
						okayGate = false;
						break;
					}
				}

				if( !okayGate )
				{
					currGates.push_back( curr );
					Edge *cc = curr->edge0;
					//TerrainPoint *tempPoint = NULL;
					TerrainPoint *tempPoint = tp.pointEnd;
					tp.RemovePoint( tempPoint );
					delete tempPoint;
					//cout << "removing from a( " << g << " ) start: " << tp.numPoints << endl;
					
					while( true )
					{
						if( cc->edgeType == Edge::CLOSED_GATE )
						{
							Gate *ccGate = (Gate*)cc->info;
							if( ccGate == thisGate )
								break;
							else
							{
								bool foundIgnore = false;
								for( list<Gate*>::iterator it = ignoreGates.begin(); it != ignoreGates.end(); ++it )
								{
									if( (*it) == ccGate )
									{
										foundIgnore = true;
										break;
									}
								}

								if( foundIgnore )
								{
									Edge *temp = cc->edge1;
									ccGate->SetLocked( false );
									cc = temp->edge0;
									ccGate->SetLocked( true );
									continue;
								}
							}
						}


						if( true )
						//if( tp.pointStart != NULL )
						{
							tempPoint = tp.pointEnd;
							tp.RemovePoint( tempPoint );
							delete tempPoint;
							//cout << "removing from a: " << tp.numPoints << endl;

							if( tp.pointStart == tp.pointEnd )
							{
								quitLoop = true;
								break;
							}
						}
						else
						{
							quitLoop = true;
							break;
						}

						cc = cc->edge0;
					}

					if( quitLoop )
					{
						//cout << "quitloop a" << endl;
						break;
					}

					Edge *pr = cc->edge0;
					thisGate->SetLocked( false );
					curr = pr->edge1->edge1;
					//tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );
					ignoreGates.push_back( thisGate );
					thisGate->SetLocked( true );

					//cout << "GATE IS NOT OKAY A: " << tp.numPoints << endl;
					//break;
					continue;
				}
				else
				{
					//cout << "found another gate AA: " <<  << endl;
				}

				//cout << "found another gate AA" << endl;
				currGates.push_back( curr );
			}
			else
			{

			}
			curr = curr->edge1;
		}

		
		currGates.clear();
		ignoreGates.clear();

		curr = g->edgeB;

		
		currGates.push_back( curr );
		

		TerrainPolygon tpb( NULL );

		tpb.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;
		while( true )
		{
			if( curr->v0 == g->edgeB->v0 )//curr == g->edgeB )
			{
				//we found a zone!

				if( !tpb.IsClockwise() )
				{
					//cout << "found a zone bbb!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;
					for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end() && okayZone; ++zit )
					{
						for( list<Edge*>::iterator cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit )
						{
							for( list<Edge*>::iterator git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git )
							{
								if( (*cit) == (*git) )
								{
									okayZone = false;
								}
							}
							//for( list<Gate*>::iterator git =
							
						}
					}

					if( okayZone )
					{
						Zone *z = new Zone( this, tpb );
						//cout << "creating a zone with " << currGates.size() << " gatesBBB" << endl;
						z->gates = currGates;
						zones.push_back( z );
						//cout << "actually creating a new zone   2! with " << z->gates.size() << endl;
					}
					

				}
				else
				{
					//cout << "woulda been a zone" << endl;
				}

				break;
			}
			else if( curr == g->edgeA )
			{
				//currGates.push_back( curr );
				//cout << "not a zone even b" << endl;
				break;
			}


			tpb.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

			if( curr->edgeType == Edge::CLOSED_GATE )
			{
				bool quitLoop = false;
				bool okayGate = true;
				Gate *thisGate = (Gate*)curr->info;
				for( list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it )
				{
					Gate *otherGate = (Gate*)(*it)->info;
					

					if( otherGate == thisGate )
					{
						okayGate = false;
						break;
					}
				}

				if( !okayGate )
				{
					currGates.push_back( curr );
					//TerrainPoint *tempPoint = NULL;
					TerrainPoint *tempPoint = tpb.pointEnd;
					tpb.RemovePoint( tempPoint );
					delete tempPoint;
					//cout << "removing from b( " << g << " ) start: " << tpb.numPoints << endl;

					Edge *cc = curr->edge0;
					
					
					while( true )
					{
						if( cc->edgeType == Edge::CLOSED_GATE )
						{
							
							Gate *ccGate = (Gate*)cc->info;
							if( ccGate == thisGate )
								break;
							else
							{
								bool foundIgnore = false;
								for( list<Gate*>::iterator it = ignoreGates.begin(); it != ignoreGates.end(); ++it )
								{
									if( (*it) == ccGate )
									{
										foundIgnore = true;
										break;
									}
								}

								if( foundIgnore )
								{
									Edge *temp = cc->edge1;
									ccGate->SetLocked( false );
									cc = temp->edge0;
									ccGate->SetLocked( true );
									continue;
								}
								//for( list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it )
								//{
								//	//Gate *otherGate = (Gate*)(*it)->info;
					
								//	if( 
								//	//if( otherGate == thisGate )
								//	//{
								//	//	okayGate = false;
								//	//	break;
								//	//}
								//}
							}
						}

						if( true )
						//if( tpb.pointStart != NULL )
						{
							tempPoint = tpb.pointEnd;
							tpb.RemovePoint( tempPoint );
							delete tempPoint;
							//cout << "removing from b: " << tpb.numPoints << endl;
							if( tpb.pointStart == tpb.pointEnd )
							{
								quitLoop = true;
								break;
							}
								
						}
						else
						{
							quitLoop = true;
							break;
						}
						
						
						cc = cc->edge0;

						
						
					}

					if( quitLoop )
					{
						//cout << "quitloop b" << endl;
						break;
					}

					Edge *pr = cc->edge0;
					thisGate->SetLocked( false );
					curr = pr->edge1->edge1;
					//tpb.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );
					ignoreGates.push_back( thisGate );
					thisGate->SetLocked( true );
					//cout << "GATE IS NOT OKAY B: " << tpb.numPoints << endl;
					continue;
				}
				else
				{
				//	cout << "found another gate BB: " << curr-> << endl;
				}

				
				currGates.push_back( curr );
			}

			curr = curr->edge1;
		}
		
		//tp.AddPoint( new TerrainPoint( 
	}



	for( int i = 0; i < numGates; ++i )
	{
		//gates[i]->SetLocked( true );
	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		//cout << "setting gates in zone: " << (*it) << " which has " << (*it)->gates.size() << " gates " << endl;
		//cout << i << ", it gates: " << (*it)->gates.size() << endl;
		for( list<Edge*>::iterator eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit )
		{
			if( gates[i]->edgeA == (*eit) )
			{
			//	cout << "gate: " << gates[i] << ", gate zone a: " << (*it ) << endl;
				gates[i]->zoneA = (*it);
				//done++;
			}
			else if( gates[i]->edgeB == (*eit) )
			{
			//	cout << "gate: " << gates[i] << ", gate zone B: " << (*it ) << endl;
				//cout << "gate zone B: " << (*it ) << endl;
				gates[i]->zoneB = (*it);
				//done++;
			}
		}
	}
	}

	//list<Gate*> ;
	list<Edge*> outsideGates;

	int numOutsideGates = 0;
	for( int i = 0; i < numGates; ++i )
	{
		Gate *g = gates[i];
		if( g->zoneA == NULL )
		{
			outsideGates.push_back( g->edgeA );
			numOutsideGates++;
		}
		else if( g->zoneB == NULL )
		{
			outsideGates.push_back( g->edgeB );
			numOutsideGates++;
		}
	}

	//cout << "numoutside gates!!: " << numOutsideGates << endl;

	if( numOutsideGates > 0 )
	{
		assert( borderEdge != NULL );

		TerrainPolygon tp( NULL );
		Edge *curr = borderEdge;
		
		tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;

		while( curr != borderEdge )
		{
			tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

			curr = curr->edge1;
		}

		tp.FixWinding();

		Zone *z = new Zone( this, tp );
		z->gates = outsideGates;
		zones.push_back( z );

		for( list<Edge*>::iterator it = outsideGates.begin(); it != outsideGates.end(); ++it )
		{
			Gate *g = (Gate*)(*it)->info;
			if( g->zoneA == NULL )
			{
				g->zoneA = z;
			}
			
			if( g->zoneB == NULL )
			{
				g->zoneB = z;
			}

		}

		
	
		//TerrainPolygon tp( NULL );
		
	}

	/*cout << "gate testing: " << endl;
	for( int i = 0; i < numGates; ++i )
	{
		cout << "gate " << i << ": " << gates[i]->zoneA << ", " << gates[i]->zoneB << endl;
	}*/
}

void GameSession::SetupZones()
{
	//cout << "setupzones" << endl;
	//setup subzones
	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		for( list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2 )
		{
			if( (*it) == (*it2) ) 
				continue;

			if( (*it)->ContainsZone( (*it2) ) )
			{
				cout << "contains zone!" << endl;
				(*it)->subZones.push_back( (*it2) );
			}
		}
	}

	//	cout << "1" << endl;
	//add enemies to the correct zone.
	for( list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it )
	{
		for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit )
		{
			bool hasPoint = (*zit)->ContainsPoint( (*it)->position );
			if( hasPoint )
			{
				bool mostSpecific = true;
				for( list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2 )
				{
					if( (*zit2)->ContainsPoint( (*it)->position ) )
					{
						mostSpecific = false;
						break;
					}
				}

				if( mostSpecific )
				{
					(*it)->zone = (*zit);
				}
			}
		}

		/*if( (*it)->type == Enemy::BOSS_BIRD )
		{
			cout << "whats this" << endl;
			if( (*it)->zone != NULL )
			{
				(*it)->zone->spawnEnemies.push_back( (*it) );
			}
			
		}*/
		//else
		//{
		if( (*it)->zone != NULL )
			(*it)->zone->allEnemies.push_back( (*it) );
		//}


	}

	//set key number objects correctly
	for( list<KeyNumberObj*>::iterator it = keyNumberObjects.begin(); it != keyNumberObjects.end(); ++it )
	{
		Zone *assignZone = NULL;
		V2d cPos( (*it)->pos.x, (*it)->pos.y );
		for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit )
		{
			bool hasPoint = (*zit)->ContainsPoint( cPos );
			if( hasPoint )
			{
				bool mostSpecific = true;
				for( list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2 )
				{
					if( (*zit2)->ContainsPoint( cPos ) )
					{
						mostSpecific = false;
						break;
					}
				}

				if( mostSpecific )
				{
					assignZone = (*zit);
				}
			}
		}

		if( assignZone != NULL )
		{
			assignZone->requiredKeys = (*it)->numKeys;
		}

		delete (*it);
	}


	keyNumberObjects.clear();

	 

	cout << "2" << endl;
	//which zone is the player in?
	for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit )
	{
		//Vector2i truePos = Vector2i( player->position.x, player->position.y );
		bool hasPoint = (*zit)->ContainsPoint( player->position );
		if( hasPoint )
		{
			bool mostSpecific = true;
			for( list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2 )
			{
				if( (*zit2)->ContainsPoint( player->position ) )
				{
					mostSpecific = false;
					break;
				}
			}

			if( mostSpecific )
			{
				originalZone = (*zit);
			}
		}
	}

	if( originalZone != NULL )
	{
		cout << "setting original zone to active: " << originalZone << endl;
		originalZone->active = true;
		currentZone = originalZone;
		keyMarker->SetStartKeys( currentZone->requiredKeys );
	}
	
	cout << "3: numgates: " << numGates << endl;
	cout << "num zones: " << zones.size() << endl;
	//assign correct zones to gates
	//for( int i = 0; i < numGates; ++i )
	//{
	//	

	//	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	//	{
	//		cout << i << ", it gates: " << (*it)->gates.size() << endl;
	//		for( list<Edge*>::iterator eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit )
	//		{
	//			if( gates[i]->edgeA == (*eit) )
	//			{
	//				cout << "gate zone a: " << (*it ) << endl;
	//				gates[i]->zoneA = (*it);
	//				//done++;
	//			}
	//			else if( gates[i]->edgeB == (*eit) )
	//			{
	//				cout << "gate zone B: " << (*it ) << endl;
	//				gates[i]->zoneB = (*it);
	//				//done++;
	//			}
	//		}
	//	}
	//}
	

	
	

	/*for( int i = 0; i < numGates; ++i )
	{
		if( gates[i]->zoneA == gates[i]->zoneB )
		{
			gates[i]->SetLocked( false );
		}
	}*/

	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		(*it)->Init();
	}



	/*for( int i = 0; i < numGates; ++i )
	{
		if( gates[i]->zoneA == gates[i]->zoneB )
		{
			gates[i]->SetLocked( true );
		}
	}*/
}

int GameSession::Run( string fileN )
{
	fadingIn = false;
	fadingOut = false;
	fadeRect.setSize( Vector2f( 1920, 1080 ) );

	soundManager = new SoundManager;

	gameSoundBuffers[S_KEY_COMPLETE_W1] = soundManager->GetSound( "Audio/Sounds/key_complete_w1.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W2] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W3] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W4] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W5] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W6] = soundManager->GetSound( "Audio/Sounds/key_complete_w6.ogg" );
	gameSoundBuffers[S_KEY_ENTER_0] = soundManager->GetSound( "Audio/Sounds/key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_1] = soundManager->GetSound( "Audio/Sounds/key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_2] = soundManager->GetSound( "Audio/Sounds/key_enter_2.ogg" );
	gameSoundBuffers[S_KEY_ENTER_3] = soundManager->GetSound( "Audio/Sounds/key_enter_3.ogg" );
	gameSoundBuffers[S_KEY_ENTER_4] = soundManager->GetSound( "Audio/Sounds/key_enter_4.ogg" );
	gameSoundBuffers[S_KEY_ENTER_5] = soundManager->GetSound( "Audio/Sounds/key_enter_5.ogg" );
	gameSoundBuffers[S_KEY_ENTER_6] = soundManager->GetSound( "Audio/Sounds/key_enter_6.ogg" );
	//gameSoundBuffers[KEY_ENTER_0] = soundManager->GetSound( "Audio/Sounds/key_complete_w1.ogg" );

	//soundManager->GetMusic( "Audio/Music/02_bird_fight.ogg" );
	//currMusic = soundManager->GetMusic( "Audio/Music/w02_Bird_Talk.ogg" );
	//currMusic->setLoop( true );
	//testMusic->
	//currMusic->play();
	//currMusic->
	currMusic = NULL;
	cutPlayerInput = false;
	activeEnvPlants = NULL;
	totalGameFrames = 0;	
	originalZone = NULL;
	
	drawCrawlerReversers = NULL;
	//inactiveLights = NULL;
	inactiveEnemyList = NULL;
	cloneInactiveEnemyList = NULL;
	unlockedGateList = NULL;
	activatedZoneList = NULL;

	ts_leftHUD = GetTileset( "lefthud_560x1080.png", 560, 1080 );
	ts_speedBar = GetTileset( "momentumbar_560x210.png", 560, 210 );
	speedBarSprite.setTexture( *ts_speedBar->texture );

	//speedBarShader.setParameter( "u_texture", *ts_kinFace->texture );

	//speedBarSprite.setPosition( 0, 176 );
	leftHUDSprite.setTexture( *ts_leftHUD->texture );
	leftHUDBlankSprite.setTexture( *ts_leftHUD->texture );
	leftHUDBlankSprite.setTextureRect( ts_leftHUD->GetSubRect( 0 ) );
	leftHUDSprite.setTextureRect( ts_leftHUD->GetSubRect( 1 ) );
	//topbar = GetTileset( "topbar_308x128.png", 308, 128 );
	//topbarSprite.setTexture( *topbar->texture );
	//topbarSprite.setPosition( 2, 16 );

	cloudTileset = GetTileset( "cloud01.png", 1920, 1080 );
	sf::Texture &mountain01Tex = *GetTileset( "mountain01.png", 1920, 1080 / 2 /*540*/ )->texture;

	sf::Texture &underTrans01Tex = *GetTileset( "undertrans01.png", 1920, 650 / 2 )->texture;

	SetupClouds();
	
	undergroundTileset = GetTileset( "terrainworld1.png", 128, 128 );//GetTileset( "underground01.png", 32, 32 );
	undergroundTilesetNormal = GetTileset( "terrainworld1_NORMALS.png", 128, 128 );
	//just to load it
	//GetTileset( "terrainworld1_PATTERN.png", 16, 16 );


	/*undergroundPar[0].color = Color::Red;
	undergroundPar[1].color = Color::Red;
	undergroundPar[2].color = Color::Red;
	undergroundPar[3].color = Color::Red;*/

	undergroundPar[0].position = Vector2f( 0, 0 );
	undergroundPar[1].position = Vector2f( 0, 0 );
	undergroundPar[2].position = Vector2f( 0, 0 );
	undergroundPar[3].position = Vector2f( 0, 0 );


	bool showFrameRate = true;

	

	sf::Text frameRate( "00", arial, 30 );
	frameRate.setColor( Color::Red );

	activeSequence = NULL;

	fileName = fileN;
	
	


	sf::Texture alphaTex;
	alphaTex.loadFromFile( "alphatext.png" );
	Sprite alphaTextSprite(alphaTex);

	//sf::Texture healthTex;
	//healthTex.loadFromFile( "lifebar.png" );
	//sf::Sprite healthSprite( healthTex );
	//healthSprite.setScale( 4, 4 );
	//healthSprite.setPosition( 10, 100 );
	
	//window->setPosition( pos );
	//window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );

	view = View( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	preScreenTex->setView( view );
	//window->setView( view );

	
	uiView = View( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );

	//window->setVerticalSyncEnabled( true );

	
	sf::RectangleShape bDraw;
	bDraw.setFillColor( Color::Red );
	bDraw.setSize( sf::Vector2f(32 * 2, 32 * 2) );
	bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
	bool bdrawdraw = false;

	player = new Actor( this );

	OpenFile( fileName );
	
	
	//enterNexus1Seq = new EnterNexus1Seq( this );

	pauseMenu = new PauseMenu( this );
	//pauseMenu->cOptions->xboxInputAssoc[0];
	mainMenu->controller.SetFilter( pauseMenu->cOptions->xboxInputAssoc[0] );

	goalPulse = new GoalPulse( this, Vector2f( goalPos.x, goalPos.y ) );

	int goalTile = -1;
	switch( envType )
	{
	case 0:
		goalTile = 5;
		break;
	case 1:
		goalTile = 4;
		break;
	case 2:
		goalTile = 4;
		break;
	case 3:
		goalTile = 4;
		break;
	case 4:
		goalTile = 4;
		break;
	case 5:
		goalTile = 4;
		break;
	case 6:
		goalTile = 4;
		break;
	}
	assert( goalTile >= 0 );
	goalMapIcon.setTextureRect( ts_miniIcons->GetSubRect( goalTile ) );

	//use player->setactivepowers to set it up from the level. need to add it
	//to the editor

	powerWheel = new PowerWheel( this, player->hasPowerAirDash, 
		player->hasPowerGravReverse, player->hasPowerBounce, 
		player->hasPowerGrindBall, player->hasPowerTimeSlow, player->hasPowerRightWire);

	sf::Texture backTex;

	stringstream ss;


	int eType = envLevel + 1; //adjust for alex naming -_-
	ss << "Backgrounds/bg_" << envType + 1 << "_";
	if( envLevel < 10 )
	{
		ss << "0" << eType;
	}
	else
	{
		ss << eType;
	}

	ss << ".png";
	 
	cout << "back tex: " << ss.str() << endl;
	cout << "envtype: " << envType << ", eType: " << eType << endl;
	//cout << "loading bg: " << ss.str() << endl;
	if( !backTex.loadFromFile( ss.str() ) )
	{
		assert( 0 && "error loading background texture" );
	}
	background = Sprite( backTex );
	background.setOrigin( background.getLocalBounds().width / 2, background.getLocalBounds().height / 2 );
	background.setPosition( 0, 0 );
	bgView = View( sf::Vector2f( 0, 0 ), sf::Vector2f( 1920, 1080 ) );


	
	flowShader.setParameter( "goalPos", goalPos.x, goalPos.y );
	

	//parTest = RectangleShape( Vector2f( 1000, 1000 ) );
	//parTest.setFillColor( Color::Red );
	//Texture tex;
	//tex.loadFromFile( "cloud01.png" );
	
	//parTest.setTexture( tex ); 
	//parTest.setTexture( *cloudTileset->texture );
	//parTest.setPosition( 0, 0 );

	VertexArray *goalVAstuff = SetupEnergyFlow();

	groundTrans = Transform::Identity;
	groundTrans.translate( 0, 0 );
	

	cam.pos.x = player->position.x;
	cam.pos.y = player->position.y;
	
	sf::Vertex *line = new sf::Vertex[numPoints*2];
	for( int i = 0; i < numPoints; ++i )
	{
		//cout << "i: " << i << endl;
		line[i*2] = sf::Vertex( Vector2f( edges[i]->v0.x, edges[i]->v0.y  ) );
		line[i*2+1] =  sf::Vertex( Vector2f( edges[i]->v1.x, edges[i]->v1.y ) );
	}	

	sf::Vector2<double> nLine( ( line[1].position - line[0].position).x, (line[1].position - line[0].position).y );
	nLine = normalize( nLine );

	sf::Vector2<double> lineNormal( -nLine.y, nLine.x );

	sf::CircleShape circle( 30 );
	circle.setFillColor( Color::Blue );


	//sf::Clock inGameClock;

	sf::Clock gameClock;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	Vector2<double> otherPlayerPos;
	
	double zoomMultiple = 1;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	sf::Vertex border[] =
	{
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor ),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor)
	};

	
	bool skipped = false;
	bool oneFrameMode = false;
	quit = false;

	controller.UpdateState();
	currInput = controller.GetState();
	//ControllerState con = controller.GetState();

	
	
	bool t = currInput.start;//sf::Keyboard::isKeyPressed( sf::Keyboard::Y );
	bool s = t;
	t = false;
	//bool goalPlayerCollision = false;
	int returnVal = 0;

	//polyShader.setParameter( "u_texture", *GetTileset( "terrainworld1.png", 128, 128 )->texture );
	//polyShader.setParameter( "u_texture", *GetTileset( "washworld1.png", 512, 512 )->texture );

	numPolyTypes = matSet.size();
	polyShaders = new Shader[numPolyTypes];

	ts_polyShaders = new Tileset*[numPolyTypes];
	map<pair<int,int>, int> indexConvert;

	//cout << "NUM POLY TYPES: " << numPolyTypes << endl;
	int index = 0;
	for( set<pair<int,int>>::iterator it = matSet.begin(); it != matSet.end(); ++it )
	{
		if (!polyShaders[index].loadFromFile("mat_shader2.frag", sf::Shader::Fragment ) )
		{
			cout << "MATERIAL SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "polygon shader not loaded" );
			usePolyShader = false;
		}

		int matWorld = (*it).first;
		int matVariation = (*it).second;

		cout << "matWorld: " << matWorld << ", matvar: " << matVariation << endl;

		indexConvert[pair<int,int>(matWorld,matVariation)] = index;

		//TerrainPolygon::TerrainType tType = (TerrainPolygon::TerrainType)(*it);

		//tType = //TerrainPolygon::TerrainType::DESERT0;

		stringstream ss1;
		ss1 << "terrain_";
		
		ss1 << matWorld + 1 << "_";
		if( matVariation < 10 )
		{
			ss1 << "0" << matVariation + 1;
		}
		else
		{
			ss1 << matVariation + 1;
		}
		/*switch( tType )
		{
		case TerrainPolygon::TerrainType::MOUNTAIN0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_1_01.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::GLADE0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_2_01.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::GLADE1:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_2_02.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::DESERT0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_3_01.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::COVE0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_4_01.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::JUNGLE0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_5_01.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::FORTRESS0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_6_01.png", 1024, 1024 )->texture );
			break;
		case TerrainPolygon::TerrainType::CORE0:
			polyShaders[index].setParameter( "u_texture", 
				*GetTileset( "terrain_7_01.png", 1024, 1024 )->texture );
			break;
		}*/

		ss1 << ".png";
		ts_polyShaders[index] = GetTileset( ss1.str(), 1024, 1024 );
		cout << "loading: " << ss1.str() << endl;
		polyShaders[index].setParameter( "u_texture", 
			*GetTileset( ss1.str(), 1024, 1024 )->texture );
		//polyShaders[tType]->setParameter( "u_texture", *(ts_poly->texture) );
		polyShaders[index].setParameter( "Resolution", 1920, 1080 );
		polyShaders[index].setParameter( "AmbientColor", 1, 1, 1, 1 );
		polyShaders[index].setParameter( "u_normals", *undergroundTilesetNormal->texture );

		
		++index;
	}

	for( list<TestVA*>::iterator it = allVA.begin(); it != allVA.end(); ++it )
	{
		int realIndex = indexConvert[pair<int,int>((*it)->terrainWorldType,
		(*it)->terrainVariation)];
		//cout << "real index: " << realIndex << endl;
		(*it)->pShader = &polyShaders[realIndex];
		(*it)->ts_terrain = ts_polyShaders[realIndex];
	}

	/*polyShader.setParameter( "u_texture", *(ts_poly->texture) );
	polyShader.setParameter( "Resolution", 1920, 1080 );
	polyShader.setParameter( "AmbientColor", 1, 1, 1, 1 );
	polyShader.setParameter( "u_normals", *undergroundTilesetNormal->texture );*///*GetTileset( "testterrain2_NORMALS.png", 96, 96 )->texture );
	//polyShader.setParameter( "u_normal", *GetTileset( "terrainworld1_NORMALS.png", 128, 128 )->texture );

	//polyShader.setParameter( "u_texture", *GetTileset( "testterrain2.png" , 96, 96 )->texture ); 
	
	//polyShader.setParameter( "u_pattern", *GetTileset( "terrainworld1_PATTERN.png", 16, 16 )->texture );
	Texture & borderTex = *GetTileset( "borders.png", 16, 16 )->texture;

	Texture & grassTex = *GetTileset( "newgrass2.png", 22, 22 )->texture;

	goalDestroyed = false;

	//list<Vector2i> pathTest;
	//list<Vector2i> pointsTest;
	//pathTest.push_back( Vector2i( 200, 0 ) );
	////pathTest.push_back( Vector2i( 0, 100 ) );
	////pathTest.push_back( Vector2i( 100, 100 ) );
	
	//pointsTest.push_back( Vector2i(-100, -100) );
	//pointsTest.push_back( Vector2i(300, 100) );
	//pointsTest.push_back( Vector2i(300, 200) );
	//pointsTest.push_back( Vector2i(-100, 200) );

	////MovingTerrain *mt = new MovingTerrain( this, Vector2i( 900, -600 ), pathTest, pointsTest, false, 2 );
	////movingPlats.push_back( mt );
	
	
	LevelSpecifics();
	//lights.push_back( new Light( this ) );

	View v;
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	window->setView( v );

	//stringstream ss;
	ss.clear(); //needed?

	int frameCounterWait = 20;
	int frameCounter = 0;
	double total = 0;

	cloudView = View( Vector2f( 0, 0 ), Vector2f( 1920, 1080 ) );

	int flowSize = 64;
	//GPUFlow *f = new GPUFlow( Vector2i( player->position.x + 100, player->position.y ), flowSize, flowSize ); 
	//Flow *f = new Flow( Vector2i( player->position.x + 100, player->position.y ), flowSize, flowSize );
	//f->player = &player;

	//Cutscene cut( this, Vector2i( player->position.x, player->position.y ) );
	//cut.LoadFromFile( "gametest" );
	//int cutFrame = 0;

	state = RUN;

	Rain rain( this );
	sf::View rainView( Vector2f( 0, 0 ), Vector2f( 1920, 1080 ) );
	
	Parallax *testPar = new Parallax();

	Tileset *ts_blah = GetTileset( "Parallax/w2_tree_01_1920x1080.png", 1920, 1080 );
	Tileset *ts_cloud0 = GetTileset( "Parallax/w1_cloud_01_1920x1080.png", 1920, 1080 );
	Tileset *ts_cloud1 = GetTileset( "Parallax/w1_cloud_02_1920x1080.png", 1920, 1080 );
	Tileset *ts_cloud2 = GetTileset( "Parallax/w1_cloud_03_1920x1080.png", 1920, 1080 );
	//testPar->AddRepeatingSprite( ts_blah, 0, Vector2f( 0, 0 ), 1920 * 2, 10 );
	//testPar->AddRepeatingSprite( ts_blah, 0, Vector2f( 1920, 0 ), 1920 * 2, 10 );
	//testPar->AddRepeatingSprite( ts_cloud0, 0, Vector2f( 0, 0 ), 1920 * 2, 50 );
	//testPar->AddRepeatingSprite( ts_cloud1, 0, Vector2f( 500, -200 ), 1920 * 2, 30 );
	//testPar->AddRepeatingSprite( ts_cloud2, 0, Vector2f( 1500, 300 ), 1920 * 2, 25 );

	/*Tileset *ts_mountain0 = GetTileset( "Parallax/parallax_w1_mountain_01.png", 1920, 1080 );
	Tileset *ts_mountain1 = GetTileset( "Parallax/parallax_w1_mountain_02.png", 1920, 1080 );
	Tileset *ts_mountain2 = GetTileset( "Parallax/parallax_w1_mountain_03.png", 1920, 1080 );
	Tileset *ts_mountain3 = GetTileset( "Parallax/parallax_w1_mountain_04.png", 1920, 1080 );
	Tileset *ts_mountain4 = GetTileset( "Parallax/parallax_w1_mountain_05.png", 1920, 1080 );
	Tileset *ts_mountain5 = GetTileset( "Parallax/parallax_w1_mountain_06.png", 1920, 1080 );*/


	Tileset *ts_mountain0 = GetTileset( "Parallax/parallax_w2_tree_01.png", 1920, 1080 );
	Tileset *ts_mountain1 = GetTileset( "Parallax/parallax_w2_tree_02.png", 1920, 1080 );
	Tileset *ts_mountain2 = GetTileset( "Parallax/parallax_w2_tree_03.png", 1920, 1080 );
	Tileset *ts_mountain3 = GetTileset( "Parallax/parallax_w2_tree_04.png", 1920, 1080 );
	Tileset *ts_mountain4 = GetTileset( "Parallax/parallax_w2_tree_05.png", 1920, 1080 );
	Tileset *ts_mountain5 = GetTileset( "Parallax/parallax_w2_tree_06.png", 1920, 1080 );
	testPar->AddRepeatingSprite( ts_mountain5, 0, Vector2f( 0, 0 ), 1920 * 2, 5 );
	testPar->AddRepeatingSprite( ts_mountain5, 0, Vector2f( 1920, 0 ), 1920 * 2, 5 );
	testPar->AddRepeatingSprite( ts_mountain4, 0, Vector2f( 0, 0 ), 1920 * 2, 10 );
	testPar->AddRepeatingSprite( ts_mountain4, 0, Vector2f( 1920, 0 ), 1920 * 2, 10 );
	testPar->AddRepeatingSprite( ts_mountain3, 0, Vector2f( 0, 0 ), 1920 * 2, 15 );
	testPar->AddRepeatingSprite( ts_mountain3, 0, Vector2f( 1920, 0 ), 1920 * 2, 15 );
	testPar->AddRepeatingSprite( ts_mountain2, 0, Vector2f( 0, 0 ), 1920 * 2, 20 );
	testPar->AddRepeatingSprite( ts_mountain2, 0, Vector2f( 1920, 0 ), 1920 * 2, 20 );
	testPar->AddRepeatingSprite( ts_mountain1, 0, Vector2f( 0, 0 ), 1920 * 2, 25 );
	testPar->AddRepeatingSprite( ts_mountain1, 0, Vector2f( 1920, 0 ), 1920 * 2, 25 );
	testPar->AddRepeatingSprite( ts_mountain0, 0, Vector2f( 0, 0 ), 1920 * 2, 30 );
	testPar->AddRepeatingSprite( ts_mountain0, 0, Vector2f( 1920, 0 ), 1920 * 2, 30 );
	
	
	
	
	
	

	



	while( !quit )
	{
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
		{
			frameTime = 0.25;	
		}
		//frameTime = 0.167;//0.25;	
        currentTime = newTime;

		if( showFrameRate )
		{
			if( frameCounter == frameCounterWait )
			{
				double blah = 1.0 / frameTime;
				total += blah;
				ss << total / ( frameCounterWait + 1 ) ;
				frameRate.setString( ss.str() );
				ss.clear();
				ss.str( "" );
				frameCounter = 0;
				total = 0;

				
			}
			else
			{
				double blah = 1.0 / frameTime;
				total += blah;
				++frameCounter;
			}

			/*ss << inGameClock.getElapsedTime().asSeconds();
			frameRate.setString( ss.str() );
			ss.clear();
			ss.str("");*/
		}

		//cout << "frameCounter: " << frameCounter << endl;
		//for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
		//{
			//cout << "testt: " << (*it)->sourceName << ", "
			//	<< (*it)->tileWidth << ", " << (*it)->tileWidth << endl;
			//delete (*it);
		//}

		if( state == RUN )
		{
			
		
		accumulator += frameTime;

		window->clear();
		preScreenTex->clear();
		postProcessTex->clear();
		postProcessTex1->clear();
		postProcessTex2->clear();
		

		
		coll.ClearDebug();		

		while ( accumulator >= TIMESTEP  )
        {
		//	cout << "currInputleft: " << currInput.leftShoulder << endl;
			bool skipInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageUp );
			if( oneFrameMode )
			{
				//controller.UpdateState();
				

				ControllerState con;
				//con = controller.GetState();
				
				
				bool tookScreenShot = false;
				bool screenShot = false;
				
				while( true )
				{
					//prevInput = currInput;
					//player->prevInput = currInput;
					controller.UpdateState();
					con = controller.GetState();
					//player->currInput = currInput;
					skipInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageUp );
					
					bool stopSkippingInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageDown );
					screenShot = false;//Keyboard::isKeyPressed( sf::Keyboard::F );// && !tookScreenShot;
					
					if( screenShot )
					{
						cout << "TOOK A SCREENSHOT" << endl;
						tookScreenShot = true;
						Image im = window->capture();

						 time_t now = time(0);
						 char* dt = ctime(&now);
						im.saveToFile( "screenshot.png" );//+ string(dt) + ".png" );
					}
					else
					{
						if( skipInput )
						{
							tookScreenShot = false;
						}
					}
					

					if( !skipped && skipInput )//sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && !skipped )
					{
						skipped = true;
						accumulator = 0;//TIMESTEP;
						
						//currentTime = gameClock.getElapsedTime().asSeconds() - TIMESTEP;

						break;
					}
					if( skipped && !skipInput )//!sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && skipped )
					{
						skipped = false;
						//break;
					}
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::L ) )
					{

						//oneFrameMode = false;
						break;
					}
					//if( sf::Keyboard::isKeyPressed( sf::Keyboard::M ) )
					if( stopSkippingInput )
					{

						oneFrameMode = false;
						break;
					}
					

				}

				window->clear();
			}
			else
			{
				
			}

			if( skipInput )
				oneFrameMode = true;

			bool k = sf::Keyboard::isKeyPressed( sf::Keyboard::K );
			bool levelReset = sf::Keyboard::isKeyPressed( sf::Keyboard::L );
			Enemy *monitorList = NULL;
			if( player->action != Actor::GOALKILLWAIT && player->action != Actor::GOALKILL && player->action != Actor::EXIT && ( k || levelReset || player->dead /*|| (currInput.start && !prevInput.start )*/ ) )
			{
				//levelReset = true;
				//RestartLevel();
			}

			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) )// || currInput.start )
			{
				
				quit = true;
				break;
			}
	
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
			{


				quit = true;
				returnVal = 1;

				break;
			}

			if( goalDestroyed )
			{
				quit = true;
				returnVal = 1;
				break;
			}
			
			if( pauseFrames == 0 )
			{

			prevInput = currInput;

			if( !cutPlayerInput )
				player->prevInput = currInput;

			if( !controller.UpdateState() )
			{
				bool up = Keyboard::isKeyPressed( Keyboard::Up );// || Keyboard::isKeyPressed( Keyboard::W );
				bool down = Keyboard::isKeyPressed( Keyboard::Down );// || Keyboard::isKeyPressed( Keyboard::S );
				bool left = Keyboard::isKeyPressed( Keyboard::Left );// || Keyboard::isKeyPressed( Keyboard::A );
				bool right = Keyboard::isKeyPressed( Keyboard::Right );// || Keyboard::isKeyPressed( Keyboard::D );

			//	bool altUp = Keyboard::isKeyPressed( Keyboard::U );
			//	bool altLeft = Keyboard::isKeyPressed( Keyboard::H );
			//	bool altRight = Keyboard::isKeyPressed( Keyboard::K );
			//	bool altDown = Keyboard::isKeyPressed( Keyboard::J );

				ControllerState keyboardInput;    
				keyboardInput.B = Keyboard::isKeyPressed( Keyboard::X );// || Keyboard::isKeyPressed( Keyboard::Period );
				keyboardInput.rightShoulder = Keyboard::isKeyPressed( Keyboard::C );// || Keyboard::isKeyPressed( Keyboard::Comma );
				keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::D );// || Keyboard::isKeyPressed( Keyboard::M );
				keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Z ) || Keyboard::isKeyPressed( Keyboard::Space );// || Keyboard::isKeyPressed( Keyboard::Slash );
				//keyboardInput.leftTrigger = 255 * (Keyboard::isKeyPressed( Keyboard::F ) || Keyboard::isKeyPressed( Keyboard::L ));
				keyboardInput.leftShoulder = Keyboard::isKeyPressed( Keyboard::LShift );
				keyboardInput.X = Keyboard::isKeyPressed( Keyboard::F );
				keyboardInput.start = Keyboard::isKeyPressed( Keyboard::J );
				keyboardInput.back = Keyboard::isKeyPressed( Keyboard::H );
				keyboardInput.rightTrigger = 255 * Keyboard::isKeyPressed( Keyboard::LControl );
				keyboardInput.leftTrigger = 255 * Keyboard::isKeyPressed( Keyboard::RControl );
			
				keyboardInput.rightStickPad = 0;
				if( Keyboard::isKeyPressed( Keyboard::A ) )
				{
					keyboardInput.rightStickPad += 1 << 1;
				}
				else if( Keyboard::isKeyPressed( Keyboard::S ) )
				{
					keyboardInput.rightStickPad += 1;
				}
				

				
				
				//keyboardInput.rightStickMagnitude
				

				/*if( altRight )
					currInput .altPad += 1 << 3;
				if( altLeft )
					currInput .altPad += 1 << 2;
				if( altUp )
					currInput .altPad += 1;
				if( altDown )
					currInput .altPad += 1 << 1;*/
				
				if( up && down )
				{
					if( prevInput.LUp() )
						keyboardInput.leftStickPad += 1;
					else if( prevInput.LDown() )
						keyboardInput.leftStickPad += ( 1 && down ) << 1;
				}
				else
				{
					keyboardInput.leftStickPad += 1 && up;
					keyboardInput.leftStickPad += ( 1 && down ) << 1;
				}

				if( left && right )
				{
					if( prevInput.LLeft() )
					{
						keyboardInput.leftStickPad += ( 1 && left ) << 2;
					}
					else if( prevInput.LRight() )
					{
						keyboardInput.leftStickPad += ( 1 && right ) << 3;
					}
				}
				else
				{
					keyboardInput.leftStickPad += ( 1 && left ) << 2;
					keyboardInput.leftStickPad += ( 1 && right ) << 3;
				}

				currInput = keyboardInput;
			}
			else
			{
				controller.UpdateState();
				currInput = controller.GetState();

				/*bool leftShoulder = currInput.leftShoulder;
				bool rightShoulder = currInput.rightShoulder;
				bool A = currInput.A;
				bool X = currInput.X;
				bool Y = currInput.Y;
				bool B = currInput.B;
				
				currInput.A = rightShoulder;
				currInput.rightShoulder = A;

				currInput.X = Y;
				currInput.leftShoulder = X;
				currInput.Y = leftShoulder;*/
				


				//currInput.X |= currInput.rightShoulder;

			//currInput.B;//|= currInput.rightTrigger > 200;
	//		cout << "up: " << currInput.LUp() << ", " << (int)currInput.leftStickPad << ", " << (int)currInput.pad << ", " << (int)currInput.rightStickPad << endl;
			}




			if( !cutPlayerInput )
			{
				ControllerState &pCurr = player->currInput;
				//ControllerState &pPrev = player->prevInput;

				
				bool alreadyBounce = pCurr.X;
				bool alreadyGrind = pCurr.Y;
				bool alreadyTimeSlow = pCurr.leftShoulder;
				player->currInput = currInput;
				if( controller.keySettings.toggleBounce )
				{
					if( currInput.X && !prevInput.X )
					{
						pCurr.X = !alreadyBounce;
					}
					else
					{
						pCurr.X = alreadyBounce;
					}
				}
				if( controller.keySettings.toggleGrind )
				{
					if( currInput.Y && !prevInput.Y )
					{
						pCurr.Y = !alreadyGrind;
						//cout << "pCurr.y is now: " << (int)pCurr.Y << endl;
					}
					else
					{
						pCurr.Y = alreadyGrind;
					}
				}
				if( controller.keySettings.toggleTimeSlow )
				{
					if( currInput.leftShoulder && !prevInput.leftShoulder )
					{
						
						pCurr.leftShoulder = !alreadyTimeSlow;
						
					}
					else
					{
						pCurr.leftShoulder = alreadyTimeSlow;
					}
				}



				//else
			}

			}
			else if( pauseFrames > 0 )
			{
				if( player->changingClone )
				{
					player->percentCloneChanged += player->percentCloneRate;
					//if( player->percentCloneChanged >= 1 )
					{
						player->percentCloneChanged = 0;
					//	player->percentCloneChanged = 1;
						player->changingClone = false;
						pauseFrames = 0;
					}

					//pauseFrames--;
					accumulator -= TIMESTEP;
					//break;
					continue;
				}

				player->flashFrames--;
				//cam.offset.y += 10;
				
				
				//cam.Update( &player );
				
				//view fx that are outside of hitlag pausing
				Enemy *currFX = activeEnemyList;
				while( currFX != NULL )
				{
					if( currFX->type == Enemy::BASICEFFECT )
					{
						BasicEffect * be = (BasicEffect*)currFX;
						if( be->pauseImmune )
						{
							currFX->UpdatePostPhysics();
						}
					}
					
					currFX = currFX->next;
				}

				UpdateFade();
				powerWheel->UpdateHide();

				miniCircle.setPosition( 180 + ( powerWheel->basePos.x - powerWheel->origBasePos.x ), preScreenTex->getSize().y - 180 );
				for( int i = 0; i < 6; ++i )
				{
					Sprite &gds = gateDirections[i];
					gds.setPosition( miniCircle.getPosition() );
				}

				Vector2f miniPos = Vector2f( 30, 750 );
				miniPos.x += ( powerWheel->basePos.x - powerWheel->origBasePos.x );
				miniVA[0].position = miniPos + Vector2f( 0, 0 );
				miniVA[1].position = miniPos + Vector2f( 300, 0 );
				miniVA[2].position = miniPos + Vector2f( 300, 300 );
				miniVA[3].position = miniPos + Vector2f( 0, 300 );

				pauseFrames--;
				//accumulator = 0;
				
				accumulator -= TIMESTEP;
				
				

				//currentTime = gameClock.getElapsedTime().asSeconds();
				//break;
				continue;
			}
 
			if( deathWipe )
			{
				deathWipeFrame++;
				if( deathWipeFrame == deathWipeLength )
				{
					deathWipe = false;
					deathWipeFrame = 0;
				}
			}

			if( activeSequence != NULL )// && activeSequence == startSeq )
			{
				if( !activeSequence->Update() )
				{
					activeSequence = NULL;	
				}
				else
				{
					//cout << "player frame: " << player->frame << endl;
				}
			}

			if( activeDialogue != NULL )
			{
				if( currInput.A && !prevInput.A )
				{
					if( activeDialogue->ConfirmDialogue() )
						activeDialogue = NULL;
					//activeDialogue->
				}
			}
			//else
			{
				
				//cout << "before count: " << CountActiveEnemies() << endl;
				totalGameFrames++;
				player->UpdatePrePhysics();

				UpdateEnemiesPrePhysics();

				UpdateEnemiesPhysics();

				player->UpdatePostPhysics();

				if( goalDestroyed )
				{
					quit = true;
					returnVal = 1;
					break;
				}

				if( player->hasPowerLeftWire )
					player->leftWire->UpdateQuads();

				if( player->hasPowerRightWire )
					player->rightWire->UpdateQuads();

				UpdateEnemiesPostPhysics();
				
				for( int i = 0; i < numGates; ++i )
				{
					gates[i]->Update();
				}

				

				//cout << "after count: " << CountActiveEnemies() << endl;

				//cout << "updating loop" << endl;

				//Vector2f oldCam = cam.pos;
				//float oldCamZoom = cam.GetZoom();

				//f->Update();

				//powerOrbs->UpdateStarVA();
				//powerWheel->UpdateStarVA();
				if( player->action != Actor::Action::SPAWNWAIT || player->frame > 20 )
					powerWheel->UpdateSections();

				UpdateEffects();

				keyMarker->Update();

				scoreDisplay->Update();

				soundNodeList->Update();

				goalPulse->Update();

				powerWheel->UpdateHide();


				miniCircle.setPosition( 180 + ( powerWheel->basePos.x - powerWheel->origBasePos.x ), preScreenTex->getSize().y - 180 );
				for( int i = 0; i < 6; ++i )
				{
					Sprite &gds = gateDirections[i];
					gds.setPosition( miniCircle.getPosition() );
				}

				Vector2f miniPos = Vector2f( 30, 750 );
				miniPos.x += ( powerWheel->basePos.x - powerWheel->origBasePos.x );
				miniVA[0].position = miniPos + Vector2f( 0, 0 );
				miniVA[1].position = miniPos + Vector2f( 300, 0 );
				miniVA[2].position = miniPos + Vector2f( 300, 300 );
				miniVA[3].position = miniPos + Vector2f( 0, 300 );
				//kinMinimapIcon.setPosition

				

				//rainView.setCenter(

				

				oldZoom = cam.GetZoom();
				oldCamBotLeft = view.getCenter();
				oldCamBotLeft.x -= view.getSize().x / 2;
				oldCamBotLeft.y += view.getSize().y / 2;

				oldView = view;


				//polyShader.setParameter( "oldZoom", cam.GetZoom() );
				//polyShader.setParameter( "oldBotLeft", view.getCenter().x - view.getSize().x / 2, 
				//	view.getCenter().y + view.getSize().y / 2 );


				cam.Update( player );

				Vector2f camPos = cam.GetPos();

				for( list<Barrier*>::iterator it = barriers.begin();
					it != barriers.end(); ++it )
				{
					bool trig = (*it)->Update( player );
					if( trig )
					{
						TriggerBarrier( (*it) );
					}
				}

				UpdateFade();

				rain.Update();

				testPar->Update( camPos );

				//Vector2f diff = cam.pos - oldCam;

				//cloudVel = Vector2f( -40, 0 );
				//cloud0a.move( cloudVel );
				//cloud0b.move( cloudVel );
				//cloud1a.move( Vector2f( -2, 0 ) );
				//cloud1b.move( Vector2f( -2, 0 ) );
				if( shipSequence )
				{
					


					float oldLeft = cloud0[0].position.x;
					//float oldLeft1 = cloud0[1].position.x;
					//float blah = std::max( 15.f, ( 30.f - relShipVel.x ) );
					float blah = 30.f;
					float newLeft = oldLeft - blah; //cloudVel.x;
					float diff = ( shipStartPos.x - 480 ) - newLeft;
					if( diff >= 480 )
					{
						//cout << "RESETING: " << diff << endl;
						newLeft = shipStartPos.x - 480 - ( diff - 480 );
					}
					else
					{
						//cout << "diff: " << diff << endl;
					}

					float allDiff = newLeft - oldLeft;// - relShipVel.x;
					//cout << "all diff: " << allDiff << endl;
					//allDiff = .5;
					Vector2f cl = relShipVel;
					//cl.y = cl.y / 2;

					middleClouds.move( Vector2f( 0, cl.y ) );// + Vector2f( allDiff, 0 ) );
					for( int i = 0; i < 3 * 4; ++i)
					{
						cloud0[i].position = cl + Vector2f( cloud0[i].position.x + allDiff, cloud0[i].position.y );
						cloud1[i].position = cl + Vector2f( cloud1[i].position.x + allDiff, cloud1[i].position.y );

						cloudBot0[i].position = cl + Vector2f( cloudBot0[i].position.x + allDiff, cloudBot0[i].position.y );
						cloudBot1[i].position = cl + Vector2f( cloudBot1[i].position.x + allDiff, cloudBot1[i].position.y );

					}

					if( shipSeqFrame >= 90 && shipSeqFrame <= 180 )
					{
						int tFrame = shipSeqFrame - 90;
						//cout << "tFrame: " << tFrame << endl;
						//CubicBezier b( 0, 0, 1, 1 );
						//double a = tFrame / 60.0;
						//double v = b.GetValue( a );
						shipSprite.setPosition( shipSprite.getPosition() + relShipVel );

						relShipVel += Vector2f( .3, -.8 );

						/*if( shipSeqFrame >= 100 )
						{
							relShipVel = Vector2f( 0, 0 );
						}
						else
						{
							relShipVel += Vector2f( .3, -.8 );
						}*/
						

						//V2d shipStart = V2d( shipStartPos.x, shipStartPos.y );
						//V2d stuff = shipStart * ( 1.0 - v ) 
						//	+ ( shipStart + V2d( 500, - 500 ) ) * v;
						//shipSprite.setPosition( stuff.x, stuff.y );


					}
					else if( shipSeqFrame == 240 )//121 )
					{
						//cout << "relshipvel: " << relShipVel.x << ", " << relShipVel.y << endl;
						player->action = Actor::JUMP;
						player->frame = 1;
						player->velocity = V2d( 20, 10 );
						player->UpdateSprite();
						shipSequence = false;
						player->hasDoubleJump = false;
						player->hasAirDash = false;
						player->hasGravReverse = false;
						//player->
						//player->rightWire->= false;
					}

					++shipSeqFrame;
				}
				/*if( shipStartPos.x - cloud1b.getPosition().x > 960 )
				{
					
					cout << "condition CLOUD B: " << shipStartPos.x - cloud1b.getPosition().x << endl;
					cloud1b.setPosition( shipStartPos.x + 480, cloud1b.getPosition().y );
				}
				else if( shipStartPos.x - cloud1a.getPosition().x > 960 )
				{
					cout << "condition CLOUD A: " << shipStartPos.x - cloud1a.getPosition().x << endl;
					cloud1a.setPosition( shipStartPos.x + 480, cloud1a.getPosition().y );
				}*/
				//cloud0a.setPosition( pi->pos.x - 480, pi->pos.y + 270 );
				//cloud0b.setPosition( pi->pos.x, pi->pos.y + 270 );
				
				//cloud1a.setPosition( pi->pos.x - 480, pi->pos.y + 270 );
				//cloud1b.setPosition( pi->pos.x, pi->pos.y + 270);// + 540 );


				double camWidth = 960 * cam.GetZoom();
				double camHeight = 540 * cam.GetZoom();
				
				screenRect = sf::Rect<double>( camPos.x - camWidth / 2, camPos.y - camHeight / 2, camWidth, camHeight );
			
				//flowShader.setParameter( "radius0", flow
				
				
				flowRadius = (maxFlowRadius - (maxFlowRadius / flowFrameCount) * flowFrame);

				flowShader.setParameter( "radius", flowRadius / maxFlowRings );
				//cout << "radius: " << flowRadius / maxFlowRings << ", frame: " << flowFrame << endl;
				flowShader.setParameter( "zoom", cam.GetZoom() );
				flowShader.setParameter( "playerPos", player->position.x, player->position.y );


				++flowFrame;
				if( flowFrame == flowFrameCount )
				{
					flowFrame = 0;
				}
				
				int speedLevel = player->speedLevel;
				//speedBarShader.setParameter( "onPortion", (float)speedLevel );
				//speedBarShader.setParameter( "quant", (float)currentSpeedBar );
				float quant = 0;
				if( speedLevel == 0 )
				{
					quant = (float)(player->currentSpeedBar / player->level1SpeedThresh);
				}
				else if( speedLevel == 1 )
				{
					quant = (float)((player->currentSpeedBar-player->level1SpeedThresh) / ( player->level2SpeedThresh - player->level1SpeedThresh) );
				}
				else 
				{
					quant = (float)((player->currentSpeedBar-player->level2SpeedThresh) / ( player->maxGroundSpeed - player->level2SpeedThresh) );
					
				}

				//cout << "quant: " << quant << endl;
				speedBarShader.setParameter( "onPortion", quant );

				queryMode = "enemy";

				tempSpawnRect = screenRect;
				enemyTree->Query( this, screenRect );

				if( player->blah || player->record > 1 )
				{
					int playback = player->recordedGhosts;
					if( player->record > 1 )
						playback--;

					for( int i = 0; i < playback; ++i )
					{
						PlayerGhost *g = player->ghosts[i];
						if( player->ghostFrame < g->totalRecorded )
						{
							//cout << "querying! " << player->ghostFrame << endl;
							tempSpawnRect = g->states[player->ghostFrame].screenRect;
							enemyTree->Query( this, g->states[player->ghostFrame].screenRect );
						}
					}
				}

				queryMode = "crawlerreverser";
				drawCrawlerReversers = NULL;
				crawlerReverserTree->Query( this, screenRect );
				
				EnvPlant *prevPlant = NULL;
				EnvPlant *ev = activeEnvPlants;
				while( ev != NULL )
				{
					EnvPlant *tempNext = ev->next;
					ev->particle->Update( player->position );

					ev->frame++;
					if( ev->frame == ev->disperseLength * ev->disperseFactor )
					{
						VertexArray &eva = *ev->va;
						eva[ev->vaIndex+0].position = Vector2f(0,0);
						eva[ev->vaIndex+1].position = Vector2f(0,0);
						eva[ev->vaIndex+2].position = Vector2f(0,0);
						eva[ev->vaIndex+3].position = Vector2f(0,0);

						if( ev == activeEnvPlants )
						{
							activeEnvPlants = ev->next;
						}
						else
						{
							prevPlant->next = ev->next;
						}
					}
					else
					{
						prevPlant = ev;
					}
					
					ev = tempNext;
				}

				queryMode = "envplant";
				envPlantTree->Query( this, screenRect );

				while( listVA != NULL )
				{
					TestVA *t = listVA->next;
					listVA->next = NULL;
					listVA = t;
				}

				//listVA is null here
				queryMode = "border";
				numBorders = 0;
				borderTree->Query( this, screenRect );


				TestVA *te = listVA;
				while( te != NULL )
				{
					te->UpdateBushes();
					te = te->next;
				}
				TestVA::UpdateBushFrame();

				if( player->dead )
				{
					RestartLevel();
				}
				else if( player->action != Actor::GOALKILLWAIT && player->action != Actor::GOALKILL && player->action != Actor::EXIT )
				{
					//if( Keyboard::isKeyPressed( Keyboard ) )
					if( currInput.start && !prevInput.start )
					{
						state = PAUSE;
						pauseMenu->SetTab( PauseMenu::PAUSE );
						soundNodeList->Pause( true );


					}
					else if( ( currInput.back && !prevInput.back ) || Keyboard::isKeyPressed( Keyboard::G ) )
					{
						state = PAUSE;
						pauseMenu->SetTab( PauseMenu::MAP );
					
						soundNodeList->Pause( true );
					}
				}
				

				if( player->record > 0 )
				{
					player->ghosts[player->record-1]->states[player->ghosts[player->record-1]->currFrame].screenRect =
						screenRect;
				}
			}
			

			accumulator -= TIMESTEP;
		}

		//cout << "every time" << endl;
		//gravity = 1.9;//1.9; // 1 
		//jumpStrength = 27.5; // 2 
		//dashSpeed = 9;//12; // 3
		//airDashSpeed = dashSpeed;
		//maxFallSpeed = 40;//100; // 4

		double gravFactor = .01;
		double jumpStrengthFactor = .01;
		double dashSpeedFactor = .01;
		double maxFallSpeedFactor = .01;

		

		if( Keyboard::isKeyPressed( Keyboard::R ) )
		{
			//player->maxFallSpeedSlo += maxFallSpeedFactor;
			//cout << "maxFallSpeed : " << player->maxFallSpeed << endl;
		}
		


		sf::Event ev;
		while( window->pollEvent( ev ) )
		{
			if( ev.type == Event::MouseWheelMoved )
			{
				if( ev.mouseWheel.delta > 0 )
				{
					zoomMultiple /= 2;
				}
				else if( ev.mouseWheel.delta < 0 )
				{
					zoomMultiple *= 2;
				}
				
				if( zoomMultiple < 1 )
				{
					zoomMultiple = 1;
				}
				else if( zoomMultiple > 65536 )
				{
					zoomMultiple = 65536;
				}
			}
			else if( ev.type == Event::LostFocus )
			{
				if( state == RUN )
					state = PAUSE;
			}
			else if( ev.type == sf::Event::GainedFocus )
			{
				//if( state == PAUSE )
				//	state = RUN;
			}
		}
		Vector2f camOffset;
		
		
		
		
		if( false )//activeSequence != NULL && activeSequence == startSeq )
		{
			activeSequence->Draw( preScreenTex );
			

			preScreenTex->display();
			const Texture &preTex = preScreenTex->getTexture();
		
			Sprite preTexSprite( preTex );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			//preTexSprite.setScale( .5, .5 );		

			window->draw( preTexSprite  );
		}
		else
		{

		Vector2f camPos = cam.GetPos();
		view.setSize( Vector2f( 960 * cam.GetZoom(), 540 * cam.GetZoom()) );
		//view.setSize( cut.cameras[cutFrame].getSize() );
		lastViewSize = view.getSize();

		//view.setCenter( player->position.x + camOffset.x, player->position.y + camOffset.y );
		view.setCenter( camPos.x, camPos.y );
		
		
		//view.setCenter( cut.cameras[cutFrame].getCenter() );



		//cout << "center: " << view.getCenter().x << ", " << view.getCenter().y << endl;
		//view = //cut.cameras[cutFrame];
		
		//
		//view.setCenter( cut.cameras[cutFrame].getCenter() );
		//cout << "view zoom: " << view.getSize().x << ", " << view.getSize().y << endl;

		//preScreenTex->setView( cut.GetView( cutFrame ) );
		lastViewCenter = view.getCenter();

		flowShader.setParameter( "topLeft", view.getCenter().x - view.getSize().x / 2, 
					view.getCenter().y + view.getSize().y / 2 );
		
		//window->setView( bgView );
		preScreenTex->setView( bgView );

		preScreenTex->draw( background );

		//temporary parallax
		View pView;
		float depth = .3;
		int px = floor( view.getCenter().x * depth + .5 );
		int pxx;
		
		if( px >= 0 )
		{
			pxx = (px % (1920 * 2)) - 1920;
			//cout << ">0: " << pxx << ", realx: " << px << endl;
		}
		else
		{
			pxx = -(-px % (1920 * 2)) + 1920;
			//cout << "<0: " << pxx << ", realx: " << px << endl;
		}
		pView.setCenter( Vector2f( pxx, 0 ) );
		pView.setSize( 1920, 1080 );
		//preScreenTex->setView( pView );
		//preScreenTex->draw( testParallaxSprite );
		preScreenTex->setView( view );

		testPar->Draw( preScreenTex );
		

		//window->draw( background );

		
		
		//window->setView( view );

		

		
		//cloudView.setSize( 1920, 1080 );
		cloudView.setCenter( 960, 540 );
		
		//preScreenTex->setView( cut.cameras[cutFrame] );
		

	//	SetParMountains( preScreenTex );

	//	SetParMountains1( preScreenTex );

	//	SetParOnTop( preScreenTex );

		//cavedepth
		//if( SetGroundPar() )
		{
		//	preScreenTex->draw( groundPar, &mountain01Tex );
			//preScreenTex->draw( underTransPar, &underTrans01Tex );
		}
	
		cloudView.setCenter( 960, 540 );	
		preScreenTex->setView( cloudView );
		
		//float depth = 3;
		//parTest.setPosition( orig / depth + ( cam.pos - orig ) / depth );
		//SetCloudParAndDraw();


		
		
		
		
		//cloudView.setCenter( 0, 0 );
		//preScreenTex->setView( cloudView );
		//preScreenTex->setView( cloudView );
		//SetUndergroundParAndDraw();

		
		//float scale = 1 + ( 1 - 1 / ( cam.GetZoom() * depth ) );
		//parTest.setScale( scale, scale );
		//preScreenTex->draw( parTest );
		
		preScreenTex->setView( view );
		
		bDraw.setSize( sf::Vector2f(player->b.rw * 2, player->b.rh * 2) );
		bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
		bDraw.setPosition( player->position.x + player->b.offset.x , player->position.y + player->b.offset.y );
	//	bDraw.setRotation( player->sprite->getRotation() );
		if( bdrawdraw)
		{
			preScreenTex->draw( bDraw );
		}
		//window->draw( bDraw );

	/*	CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( player->position.x, player->position.y );
		window->draw( cs );*/

	
		//player->sh.setParameter( "u_texture", *GetTileset( "testrocks.png", 25, 25 )->texture );
		//player->sh.setParameter( "u_texture1", *GetTileset( "testrocksnormal.png", 25, 25 )->texture );
		
		
		
		
		
		


		//player->sprite->setTextureRect( IntRect( 0, 0, 300, 225 ) );
		//if( false )
		
		while( lightList != NULL )
		{
			Light *l = lightList->next;
			lightList->next = NULL;
			lightList = l;
		}

		queryMode = "lightdisplay";
		lightTree->Query( this, screenRect );

		Light *lightListIter = lightList;
		while( lightListIter != NULL )
		{
			lightListIter->Draw( preScreenTex );
			lightListIter = lightListIter->next;
		}

		if( activeSequence != NULL )
		{
			activeSequence->Draw( preScreenTex );
		}
		
		
		
		sf::RectangleShape rs;
		rs.setSize( Vector2f(64, 64) );
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( otherPlayerPos.x, otherPlayerPos.y  );
		rs.setFillColor( Color::Blue );
		//window->draw( circle );
		//window->draw(line, numPoints * 2, sf::Lines);
		
		//polyShader.setParameter( "u_texture", *GetTileset( "terrainworld1.png" , 128, 128 )->texture ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//polyShader.setParameter( "u_normals", *GetTileset( "terrainworld1_NORMALS.png", 128, 128 )->texture );

		


		Vector2i vi = Mouse::getPosition();
		//Vector2i vi = window->mapCoordsToPixel( Vector2f( player->position.x, player->position.y ) );
		//Vector2i vi = window->mapCoordsToPixel( sf::Vector2f( 0, -300 ) );
		//vi -= Vector2i( view.getSize().x / 2, view.getSize().y / 2 );
		Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
		blahblah.y = 1 - blahblah.y;


		//polyShader.setParameter( "LightPos", blahblah );//Vector3f( 0, -300, .075 ) );
		//polyShader.setParameter( "LightColor", 1, .8, .6, 1 );
		
		//polyShader.setParameter( "Falloff", Vector3f( .4, 3, 20 ) );
		//cout << "window size: " << window->getSize().x << ", " << window->getSize().y << endl;


		Vector2f botLeft( view.getCenter().x - view.getSize().x / 2, 
			view.getCenter().y + view.getSize().y / 2 );

		Vector2f playertest = ( botLeft - oldCamBotLeft ) / 5.f;
		//cout << "test: " << playertest.x << ", " << playertest.y << endl;
		// window->getSize().x, window->getSize().y);
		
		for( int i = 0; i < numPolyTypes; ++i )
		{
			polyShaders[i].setParameter( "zoom", cam.GetZoom() );
			polyShaders[i].setParameter( "topLeft", botLeft ); //just need to change the name topleft eventually
			polyShaders[i].setParameter( "playertest", playertest );
		}
		
		//polyShader.setParameter( "zoom", cam.GetZoom() );
		//polyShader.setParameter( "topLeft", view.getCenter().x - view.getSize().x / 2, 
		//	view.getCenter().y + view.getSize().y / 2 );

		
		//polyShader.setParameter( "u_texture", *GetTileset( "testterrain.png", 32, 32 )->texture );


		//polyShader.setParameter(  = GetTileset( "testterrain.png", 25, 25 )->texture;

		//for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
		//{
		//	if( usePolyShader )
		//	{
		//		

		//		UpdateTerrainShader();

		//		preScreenTex->draw( *(*it ), &polyShader);
		//	}
		//	else
		//	{
		//		preScreenTex->draw( *(*it ) );
		//	}
		//	//GetTileset( "testrocks.png", 25, 25 )->texture );
		//}
		
		

		sf::Rect<double> testRect( view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2,
			view.getSize().x, view.getSize().y );

		

		
		
		

		//screenRect = sf::Rect<double>( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
		
	
		DrawEffects( EffectLayer::BEHIND_TERRAIN );
		
		int timesDraw = 0;
		TestVA * listVAIter = listVA;
		//listVAIter->next = NULL;

		UpdateTerrainShader( screenRect );

		while( listVAIter != NULL )
		//for( int i = 0; i < numBorders; ++i )
		{
			if( listVAIter->grassVA != NULL )
				preScreenTex->draw( *listVAIter->grassVA, &grassTex );

			if( usePolyShader )
			//if(false )
			{

				sf::Rect<double> polyAndScreen;
				sf::Rect<double> aabb = listVAIter->aabb;
				double rightScreen = screenRect.left + screenRect.width;
				double bottomScreen = screenRect.top + screenRect.height;
				double rightPoly = aabb.left + aabb.width;
				double bottomPoly = aabb.top + aabb.height;

				double left = std::max( screenRect.left, aabb.left );

				double right = std::min( rightPoly, rightScreen );
				
				double top = std::max( screenRect.top, aabb.top );

				double bottom = std::min( bottomScreen, bottomPoly );


				polyAndScreen.left = left;
				polyAndScreen.top = top;
				polyAndScreen.width = right - left;
				polyAndScreen.height = bottom - top;
				
				//UpdateTerrainShader( polyAndScreen );//listVAIter->aabb );
				
				/*sf::RectangleShape rs( Vector2f( listVAIter->aabb.width, listVAIter->aabb.height ) );
				rs.setPosition( listVAIter->aabb.left, listVAIter->aabb.top );
				rs.setOutlineColor( Color::Red );
				rs.setOutlineThickness( 3 );
				rs.setFillColor( Color::Transparent );
				preScreenTex->draw( rs );*/
				assert( listVAIter->pShader != NULL );
				preScreenTex->draw( *listVAIter->terrainVA, listVAIter->pShader );// listVAIter->ts_terrain->texture );//listVAIter->pShader );//listVAIter->pShader );
			}
			else
			{
				preScreenTex->draw( *listVAIter->terrainVA );
			}
			//cout << "drawing border" << endl;
			//preScreenTex->draw( *listVAIter->va );



			sf::RenderStates rs;
			rs.texture = listVAIter->ts_border->texture;

			if( listVAIter->triva != NULL )
				preScreenTex->draw( *listVAIter->triva, rs );

			preScreenTex->draw( *listVAIter->wallva, rs );
			preScreenTex->draw( *listVAIter->steepva, rs );
			preScreenTex->draw( *listVAIter->slopeva, rs );
			preScreenTex->draw( *listVAIter->groundva, rs );

			if( listVAIter->bushVA != NULL )
			{
				RenderStates bushRS;
				bushRS.texture = listVAIter->ts_bush->texture;

				preScreenTex->draw( *listVAIter->bushVA, bushRS );
			}

			if( listVAIter->plantva != NULL )
			{
				//rs.texture = listVAIter->ts_plant->texture;
				//preScreenTex->draw( *listVAIter->plantva, rs );
			}

		/*	if( listVAIter->flowva != NULL )
			{
				preScreenTex->draw( *listVAIter->flowva );
			}*/
			//preScreenTex->draw( *listVAIter->va );
			listVAIter = listVAIter->next;
			timesDraw++; 
		}
	
		
		preScreenTex->draw( *goalVAstuff, &flowShader );

		
		//motion blur
		if( false )
		{
			preScreenTex->display();
			//for motion blur


			preScreenTex->setView( uiView );
			sf::RectangleShape rectPost( Vector2f( 1920, 1080 ) );
			rectPost.setPosition( 0, 0 );
			//Vector2f camVel = cam.pos - oldCamCenter;
			
			Vector2f botLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, 
				view.getCenter().y + view.getSize().y / 2 );
			

			motionBlurShader.setParameter( "tex", preScreenTex->getTexture() );
			/*motionBlurShader.setParameter( "oldBotLeft", oldCamBotLeft );
			motionBlurShader.setParameter( "botLeft", botLeft );
			motionBlurShader.setParameter( "oldZoom", oldZoom );
			motionBlurShader.setParameter( "zoom", cam.GetZoom() );*/
			//negative player y because of bottom left origin

			V2d t = player->velocity;
			double maxBlur = 8;
			t.x = t.x / player->maxGroundSpeed * maxBlur;
			t.y = t.y / player->maxGroundSpeed * maxBlur;

			
			Vector2f testVel = Vector2f( player->velocity.x, -player->velocity.y );

			motionBlurShader.setParameter( "testVel", Vector2f( t.x, t.y ) );

			motionBlurShader.setParameter( "g_ViewProjectionInverseMatrix", view.getTransform().getInverse() );
			motionBlurShader.setParameter( "g_previousViewProjectionMatrix", oldView.getTransform() );

			
			postProcessTex->draw( rectPost, &motionBlurShader );

			postProcessTex->display();

			sf::Sprite pptSpr;
			pptSpr.setTexture( postProcessTex->getTexture() );
			//pptSpr.setScale( 2, 2 );
			//RenderStates blahRender;
			//blahRender.blendMode = sf::BlendAdd;//sf::BlendAdd;

			preScreenTex->draw( pptSpr );
			//postProcessTex->display();
			preScreenTex->setView( view );
		}

		for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
		{
			(*it)->Draw( preScreenTex );
		}

		testGateCount = 0;
		queryMode = "gate";
		gateList = NULL;
		gateTree->Query( this, screenRect );

		while( gateList != NULL )
		{
			gateList->Draw( preScreenTex );
			Gate *next = gateList->next;//(Gate*)gateList->edgeA->edge1;
			gateList = next;
		}

		DrawEffects( EffectLayer::BEHIND_ENEMIES );

		//cout << "enemies draw" << endl;
		UpdateEnemiesDraw();

		
		DrawEffects( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES );
		//bigBulletVA->draw( preScreenTex );


		goalPulse->Draw( preScreenTex );

		if( player->action != Actor::GRINDBALL )
		{
			player->leftWire->Draw( preScreenTex );
			player->rightWire->Draw( preScreenTex );
		}

		//cout << "view: " << view.getSize().x << ", " << view.getSize().y << endl;
		//preScreenTex->setView( view );
		//cout << "size what: " << preScreenTex->getView().getSize().x
		//	<< ", " << preScreenTex->getView().getSize().y << endl;

		if( shipSequence )
		{
			//preScreenTex->draw( cloud0a );
			preScreenTex->draw( cloud1, ts_w1ShipClouds1->texture );
			preScreenTex->draw( cloud0, ts_w1ShipClouds0->texture );
			preScreenTex->draw( middleClouds );
			preScreenTex->draw( cloudBot1, ts_w1ShipClouds1->texture );
			preScreenTex->draw( cloudBot0, ts_w1ShipClouds0->texture );
			//preScreenTex->draw( cloud0b );
			preScreenTex->draw( shipSprite );
		}

		if( player->action != Actor::DEATH )
			player->Draw( preScreenTex );


		if( shipSequence )
		{
			///preScreenTex->draw( cloud1a );
			//preScreenTex->draw( cloud1b );
			
		}

		//whited out hit enemies
		Enemy *current = activeEnemyList;
		while( current != NULL )
		{
		//	cout << "draw" << endl;
			if( ( pauseFrames >= 2 && current->receivedHit != NULL ) )
			{
				current->Draw( preScreenTex );
			}
			current = current->next;
		}

		DrawEffects( EffectLayer::IN_FRONT );

		if( ts_basicBullets != NULL )
		{
			preScreenTex->draw( *bigBulletVA, ts_basicBullets->texture );
		}

		rainView.setCenter( (int)view.getCenter().x % 64, (int)view.getCenter().y % 64 );
		rainView.setSize( view.getSize() );
		preScreenTex->setView( rainView );
		//rain.Draw( preScreenTex );
		preScreenTex->setView( view );

		//view.set
		
		//cut.Draw( preScreenTex, cutFrame );
		//cutFrame++;
		//if( cutFrame == cut.totalFrames )
		//{
		//	cutFrame = 0;
		//}
		preScreenTex->setView( view );
		//f->Draw( preScreenTex );
		

		if( false )//if( currInput.back || sf::Keyboard::isKeyPressed( sf::Keyboard::H ) )
		{
			//alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
//			alphaTextSprite.setScale( .5, .5 );
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );

			preScreenTex->draw( alphaTextSprite );
			//window->draw( alphaTextSprite );
		}

		/*Enemy *currFX = active;
		while( currFX != NULL )
		{
			currFX->Draw( window );
			currFX = currFX->next;
		}*/
		
		for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
		{
			//(*it)->DebugDraw( preScreenTex );
			(*it)->Draw( preScreenTex );
		}

		EnvPlant *drawPlant = activeEnvPlants;
		while( drawPlant != NULL )
		{
			//preScreenTex->draw( *drawPlant->particle->particles );
			drawPlant = drawPlant->next;
		}

		

		DebugDrawActors();

		



		//grassTree->DebugDraw( preScreenTex );


		//coll.DebugDraw( preScreenTex );

		//double minimapZoom = 8;// * cam.GetZoom();// + cam.GetZoom();
		double minimapZoom = 16;//12;// * cam.GetZoom();// + cam.GetZoom();

		View vv;
		vv.setCenter( player->position.x, player->position.y );
		vv.setSize( minimapTex->getSize().x * minimapZoom, minimapTex->getSize().y * minimapZoom );

		minimapTex->setView( vv );
		minimapTex->clear( Color( 0, 0, 0, 191 ) );
		//minimapTex->clear( Color( 0, 0, 0, 255 ) );
		

		for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
		{
			(*it)->Draw( minimapTex );
		}
		
		queryMode = "border";
		numBorders = 0;
		sf::Rect<double> minimapRect(vv.getCenter().x - vv.getSize().x / 2.0,
			vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

		borderTree->Query( this, minimapRect );

		Color testColor( 0x75, 0x70, 0x90, 191 );
		listVAIter = listVA;
		while( listVAIter != NULL )
		{
			int vertexCount = listVAIter->terrainVA->getVertexCount();
			for( int i = 0; i < vertexCount; ++i )
			{
				(*listVAIter->terrainVA)[i].color = testColor;
			}
			minimapTex->draw( *listVAIter->terrainVA );
			for( int i = 0; i < vertexCount; ++i )
			{
				(*listVAIter->terrainVA)[i].color = Color::White;
			}

			listVAIter = listVAIter->next;
		}

		

		queryMode = "item";
		drawCritical = NULL;
		itemTree->Query( this, minimapRect );
		while( drawCritical != NULL )
		{
			//cout << "draw crit " << endl;
			drawCritical->Draw( preScreenTex );
			drawCritical = drawCritical->next;
		}

		testGateCount = 0;
		queryMode = "gate";
		gateList = NULL;
		gateTree->Query( this, minimapRect );
		Gate *mGateList = gateList;
		while( gateList != NULL )
		{
			//gateList->Draw( preScreenTex );
			if( gateList->locked )
			{

				V2d along = normalize(gateList->edgeA->v1 - gateList->edgeA->v0);
				V2d other( along.y, -along.x );
				double width = 25;
				
				

				V2d leftGround = gateList->edgeA->v0 + other * -width;
				V2d rightGround = gateList->edgeA->v0 + other * width;
				V2d leftAir = gateList->edgeA->v1 + other * -width;
				V2d rightAir = gateList->edgeA->v1 + other * width;
				//cout << "drawing color: " << gateList->c.b << endl;
				sf::Vertex activePreview[4] =
				{
					//sf::Vertex(sf::Vector2<float>( gateList->v0.x, gateList->v0.y ), gateList->c ),
					//sf::Vertex(sf::Vector2<float>( gateList->v1.x, gateList->v1.y ), gateList->c ),

					sf::Vertex(sf::Vector2<float>( leftGround.x, leftGround.y ), gateList->c ),
					sf::Vertex(sf::Vector2<float>( leftAir.x, leftAir.y ), gateList->c ),


					sf::Vertex(sf::Vector2<float>( rightAir.x, rightAir.y ), gateList->c ),

					
					sf::Vertex(sf::Vector2<float>( rightGround.x, rightGround.y ), gateList->c )
				};
				minimapTex->draw( activePreview, 4, sf::Quads );
			}

			Gate *next = gateList->next;//edgeA->edge1;
			gateList = next;
		}
		

		//CircleShape playerCircle;
		//playerCircle.setFillColor( COLOR_TEAL );
		//playerCircle.setRadius( 60 );//60 );
		//playerCircle.setOrigin( playerCircle.getLocalBounds().width / 2, playerCircle.getLocalBounds().height / 2 );
		//playerCircle.setPosition( vv.getCenter().x, vv.getCenter().y );
		
		if( player->action != Actor::GRINDBALL )
		{
			player->rightWire->DrawMinimap( minimapTex );
			player->leftWire->DrawMinimap( minimapTex );
		}

		/*queryMode = "enemyminimap";
		enemyTree->Query( this, minimapRect );

		Enemy *currEnemy = activeEnemyList;
		int counter = 0;
		
		
		while( currEnemy != NULL )
		{
			currEnemy->DrawMinimap( minimapTex );
			currEnemy = currEnemy->next;
		}*/

		//shouldn't this draw all enemies that are active not just the ones from the current
		//zone?
		if( currentZone != NULL )
		{
			for( list<Enemy*>::iterator it = currentZone->allEnemies.begin(); it != currentZone->allEnemies.end(); ++it )
			{
				(*it)->DrawMinimap( minimapTex );
			}
		}
		else
		{
			//probably inefficient. only happens when there arent any gates. do a little
			//collision check to make sure they're relevant before drawing
			//also dont make circles every frame. just store it in the enemy
			for( list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it )
			{
				(*it)->DrawMinimap( minimapTex );
			}
		}

		minimapTex->display();
		const Texture &miniTex = minimapTex->getTexture();
		//minimapShader.setParameter( "u_texture", minimapTex->getTexture() );

		Sprite minimapSprite( miniTex );
		//minimapSprite.setTexture( miniTex );
		//minimapSprite.set
		//minimapSprite.setPosition( preScreenTex->getSize().x - 300, preScreenTex->getSize().y - 300 );
		minimapSprite.setPosition( 0, preScreenTex->getSize().y - 300 );
		
		//minimapSprite.setScale( .5, .5 );
		//minimapSprite.setColor( Color( 255, 255, 255, 200 ) );
		minimapSprite.setColor( Color( 255, 255, 255, 255 ) );


		
		//for post processing
		preScreenTex->display();

		if( false )
		{

			
			//sf::RectangleShape blah( Vector2f( 1920, 1080 ) );
			Sprite blah;
			blah.setTexture( preScreenTex->getTexture() );
			postProcessTex2->draw( blah );
			//postProcessTex2->clear( Color::Red );
			postProcessTex2->display();



			Vector2f shockSize( 580/2, 580/2 );
			sf::RectangleShape rectPost( shockSize );
			rectPost.setOrigin( rectPost.getLocalBounds().width / 2, rectPost.getLocalBounds().height / 2 );
			rectPost.setPosition( player->position.x, player->position.y ); //testing for now

			Sprite shockSprite;
			shockSprite.setTexture( shockwaveTex );
			shockSprite.setOrigin( shockSprite.getLocalBounds().width / 2, shockSprite.getLocalBounds().height / 2 );
			shockSprite.setPosition( player->position.x, player->position.y );
			//rectPost.setPosition( 0, 0 );

			Vector2f botLeft( view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y );

			shockwaveShader.setParameter( "underTex", postProcessTex2->getTexture() );
			shockwaveShader.setParameter( "shockSize", Vector2f( 580, 580 ) );
			shockwaveShader.setParameter( "botLeft", Vector2f( rectPost.getPosition().x - rectPost.getSize().x / 2 - botLeft.x, 
				rectPost.getPosition().y - rectPost.getSize().y / 2 + rectPost.getSize().y - botLeft.y ) );
			shockwaveShader.setParameter( "zoom", cam.GetZoom() );
			//preScreenTex->draw( shockSprite );

			shockSprite.setScale( (1. / 60.) * shockTestFrame, (1. / 60.) * shockTestFrame );
			preScreenTex->draw( shockSprite, &shockwaveShader );


			shockTestFrame++;
			if( shockTestFrame == 60 )
			{
				shockTestFrame = 0;
			}
			//postProcessTex2->draw( rectPost, &shockwaveShader );
			//postProcessTex2->display();

			//sf::Sprite pptSpr;
			//pptSpr.setTexture( postProcessTex2->getTexture() );
			//preScreenTex->draw( pptSpr );
		}


		preScreenTex->setView( uiView );

		
		
		if( false )
		{
			sf::RectangleShape rectPost( Vector2f( 1920, 1080 ) );
			rectPost.setPosition( 0, 0 );
			//Vector2f camVel = cam.pos - oldCamCenter;
			
			Vector2f botLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, 
				view.getCenter().y + view.getSize().y / 2 );
			

			motionBlurShader.setParameter( "tex", preScreenTex->getTexture() );
			motionBlurShader.setParameter( "oldBotLeft", oldCamBotLeft );
			motionBlurShader.setParameter( "botLeft", botLeft );
			motionBlurShader.setParameter( "oldZoom", oldZoom );
			motionBlurShader.setParameter( "zoom", cam.GetZoom() );

			motionBlurShader.setParameter( "g_ViewProjectionInverseMatrix", view.getTransform().getInverse() );
			motionBlurShader.setParameter( "g_previousViewProjectionMatrix", oldView.getTransform() );

			
			postProcessTex->draw( rectPost, &motionBlurShader );

			postProcessTex->display();

			sf::Sprite pptSpr;
			pptSpr.setTexture( postProcessTex->getTexture() );
			//pptSpr.setScale( 2, 2 );
			//RenderStates blahRender;
			//blahRender.blendMode = sf::BlendAdd;//sf::BlendAdd;

			preScreenTex->draw( pptSpr );
			//postProcessTex->display();
			
		}
		else if( false )
		{
		sf::RectangleShape rectPost( Vector2f( 1920/2, 1080/2 ) );
		rectPost.setPosition( 0, 0 );
		glowShader.setParameter( "tex", preScreenTex->getTexture() );
		//glowShader.setParameter( "old", postProcessTex->getTexture() );
		postProcessTex->draw( rectPost, &glowShader );

		for( int i = 0; i < 3; ++i )
		{
			postProcessTex->display();
			hBlurShader.setParameter( "tex", postProcessTex->getTexture() );
			
			postProcessTex1->draw( rectPost, &hBlurShader );

			postProcessTex1->display();
			vBlurShader.setParameter( "tex", postProcessTex1->getTexture() );
			postProcessTex->draw( rectPost, &vBlurShader );
		}
		


		postProcessTex->display();

		sf::Sprite pptSpr;
		pptSpr.setTexture( postProcessTex->getTexture() );
		pptSpr.setScale( 2, 2 );
		RenderStates blahRender;
		blahRender.blendMode = sf::BlendAdd;

		preScreenTex->draw( pptSpr, blahRender );
		}
		else if( false )
		{
			sf::RectangleShape rectPost( Vector2f( 1920/2, 1080/2 ) );
			rectPost.setPosition( 0, 0 );
			for( int i = 0; i < 3; ++i )
			{
				hBlurShader.setParameter( "tex", preScreenTex->getTexture() );
				postProcessTex->draw( rectPost, &hBlurShader );

				postProcessTex->display();
				vBlurShader.setParameter( "tex", postProcessTex->getTexture() );
				preScreenTex->draw( rectPost, &vBlurShader );

				if( i < 2 )
				{
					preScreenTex->display();
				}
			}
		}

		//postProcessTex->display();

		

		//preScreenTex->draw( minimapSprite );

		
		//preScreenTex->draw( leftHUDBlankSprite );
		//preScreenTex->draw( speedBarSprite, &speedBarShader );
		
		
		if( player->speedLevel == 0 )
		{
			preScreenTex->draw( player->kinUnderOutline );
			preScreenTex->draw( player->kinTealOutline, &speedBarShader );
		}
		else if( player->speedLevel == 1 )
		{
			preScreenTex->draw( player->kinTealOutline );
			preScreenTex->draw( player->kinBlueOutline, &speedBarShader );
		}
		else if( player->speedLevel == 2 )
		{
			preScreenTex->draw( player->kinBlueOutline );
			preScreenTex->draw( player->kinPurpleOutline, &speedBarShader );
		}

		if( player->desperationMode )
		{
			preScreenTex->draw( player->kinFace, &(player->despFaceShader) );
		}
		else
		{
			preScreenTex->draw( player->kinFace );
		}
		
		//else 

		/*sf::Vertex blah[] = 
		{ 
			Vertex(  ),
			Vertex( Vector2f( 300, 0 )),
			Vertex( Vector2f( 300, 300 )),
			Vertex( Vector2f( 0, 300 ) )
		};*/
		//VertexArray va( sf::Quads, 4 );
	

		//preScreenTex->draw( lifeBarSprite );
		preScreenTex->draw( miniVA, &minimapShader );
		preScreenTex->draw( miniCircle );

		if( currentZone != NULL )
		{
			int index = 0;
			list<Edge*> gList = currentZone->gates;
			for( list<Edge*>::iterator it = gList.begin(); it != gList.end(); ++it )
			{
				Gate *tGate = (Gate*)(*it)->info;
				if( tGate->gState == Gate::OPEN || tGate->gState == Gate::DISSOLVE
					|| tGate->gState == Gate::REFORM
					|| tGate->gState == Gate::LOCKFOREVER ||
					tGate->type == Gate::BLACK )
				{
					continue;
				}

				V2d avg = ( tGate->edgeA->v1 + tGate->edgeA->v0 ) / 2.0;

				double rad = minimapRect.width / 2 - 100;
				if( length( player->position - avg ) < rad )
				{
					continue;
				}
				
				V2d dir = normalize( avg - player->position );
				double angle = atan2( dir.y, -dir.x );
				gateDirections[index].setRotation( -angle / PI * 180 - 90 );
				preScreenTex->draw( gateDirections[index] );
				index++;

			}
			
		}
		
		/*for( int i = 0; i < 6; ++i )
		{
			Sprite &gds = gateDirections[i];
			gds.setTexture( *ts_minimapGateDirection->texture );
			gds.setTextureRect( ts_minimapGateDirection->GetSubRect( 0 ) );
			gds.setOrigin( gds.getLocalBounds().width / 2, 300 + gds.getLocalBounds().height );
			gds.setPosition( miniCircle.getPosition() );
		}*/
		
		//inefficient because its in the draw call
		kinMinimapIcon.setPosition( 180 + ( powerWheel->basePos.x - powerWheel->origBasePos.x ), preScreenTex->getSize().y - 180 );
		



		preScreenTex->draw( kinMinimapIcon );
	//minimapSprite.draw( preScreenTex );
		//preScreenTex->draw( minimapSprite, &minimapShader );
		//powerBar.Draw( preScreenTex );
		
		
		//powerOrbs->Draw( preScreenTex );
		
		powerWheel->Draw( preScreenTex );
		keyMarker->Draw( preScreenTex );
		scoreDisplay->Draw( preScreenTex );
		//preScreenTex->draw( leftHUDSprite );

		//window->setView( uiView );
	//	window->draw( healthSprite );
		

		//preScreenTex->draw( topbarSprite );

		//preScreenTex->draw( player->kinFace );
		//topbarSprite.draw( preScreenTex );

		//preScreenTex->draw( keyHolderSprite );


		//note: gotta fix these later for number of keys
	


		if( showFrameRate )
		{
			preScreenTex->draw( frameRate );
		}
		

		preScreenTex->setView( view );
		//window->setView( view );

		

		

		

		
		//terrainTree->DebugDraw( window );
		//DebugDrawQuadTree( window, enemyTree );
	//	enemyTree->DebugDraw( window );
		

		//if( deathWipe )
		//{
		//	//cout << "showing death wipe frame: " << deathWipeFrame << " panel: " << deathWipeFrame / 5 << endl;
		//	wipeSprite.setTexture( wipeTextures[deathWipeFrame / 5] );
		//	wipeSprite.setTextureRect( IntRect( 0, 0, wipeSprite.getTexture()->getSize().x, 
		//		wipeSprite.getTexture()->getSize().y) );
		//	wipeSprite.setOrigin( wipeSprite.getLocalBounds().width / 2, wipeSprite.getLocalBounds().height / 2 );
		//	wipeSprite.setPosition( player->position.x, player->position.y );//view.getCenter().x, view.getCenter().y );
		//	preScreenTex->draw( wipeSprite );
		//}

		

		if( player->action == Actor::DEATH )
		{
			player->Draw( preScreenTex );
		}
	//	preScreenTex->setSmooth( true );
		
		//preTexSprite.setOrigin( preTexSprite.getLocalBounds().width / 2, preTexSprite.getLocalBounds().height / 2 );
		cloneShader.setParameter( "u_texture", preScreenTex->getTexture() );
		cloneShader.setParameter( "newscreen", player->percentCloneChanged );
		cloneShader.setParameter( "Resolution", 1920, 1080 );//window->getSize().x, window->getSize().y);
		cloneShader.setParameter( "zoom", cam.GetZoom() );

		cloneShader.setParameter( "topLeft", view.getCenter().x - view.getSize().x / 2, 
			view.getCenter().y + view.getSize().y / 2 );

		cloneShader.setParameter( "bubbleRadius", player->bubbleRadius );
		
		float windowx = 1920;//window->getSize().x;
		float windowy = 1080;//window->getSize().y;

		Vector2i vi0 = preScreenTex->mapCoordsToPixel( Vector2f( player->bubblePos[0].x, player->bubblePos[0].y ) );
		Vector2f pos0( vi0.x / windowx, -1 + vi0.y / windowy ); 

		Vector2i vi1 = preScreenTex->mapCoordsToPixel( Vector2f( player->bubblePos[1].x, player->bubblePos[1].y ) );
		Vector2f pos1( vi1.x / windowx, -1 + vi1.y / windowy ); 

		Vector2i vi2 = preScreenTex->mapCoordsToPixel( Vector2f( player->bubblePos[2].x, player->bubblePos[2].y ) );
		Vector2f pos2( vi2.x / windowx, -1 + vi2.y / windowy ); 

		Vector2i vi3 = preScreenTex->mapCoordsToPixel( Vector2f( player->bubblePos[3].x, player->bubblePos[3].y ) );
		Vector2f pos3( vi3.x / windowx, -1 + vi3.y / windowy ); 

		Vector2i vi4 = preScreenTex->mapCoordsToPixel( Vector2f( player->bubblePos[4].x, player->bubblePos[4].y ) );
		Vector2f pos4( vi4.x / windowx, -1 + vi4.y / windowy ); 

		Vector2i vi5 = preScreenTex->mapCoordsToPixel( Vector2f( player->bubblePos[5].x, player->bubblePos[5].y ) );
		Vector2f pos5( vi5.x / windowx, -1 + vi5.y / windowy ); 

		//cout << "pos0: " << pos0.x << ", " << pos0.y << endl;
		//cout << "b0frame: " << player->bubbleFramesToLive[0] << endl;
		//cout << "b1frame: " << player->bubbleFramesToLive[1] << endl;
		//cout << "b2frame: " << player->bubbleFramesToLive[2] << endl;

		cloneShader.setParameter( "bubble0", pos0 );
		cloneShader.setParameter( "b0Frame", player->bubbleFramesToLive[0] );
		cloneShader.setParameter( "bubble1", pos1 );
		cloneShader.setParameter( "b1Frame", player->bubbleFramesToLive[1] );
		cloneShader.setParameter( "bubble2", pos2 );
		cloneShader.setParameter( "b2Frame", player->bubbleFramesToLive[2] );
		cloneShader.setParameter( "bubble3", pos3 );
		cloneShader.setParameter( "b3Frame", player->bubbleFramesToLive[3] );
		cloneShader.setParameter( "bubble4", pos4 );
		cloneShader.setParameter( "b4Frame", player->bubbleFramesToLive[4] );
		cloneShader.setParameter( "bubble5", pos5 );
		cloneShader.setParameter( "b5Frame", player->bubbleFramesToLive[5] );
		

		

		//this is so inefficient LOL
		preScreenTex->display();
		
		const Texture &preTex = preScreenTex->getTexture();
		
		Sprite preTexSprite( preTex );
		preTexSprite.setPosition( -960 / 2, -540 / 2 );
		preTexSprite.setScale( .5, .5 );

		preScreenTex->setView( v );
		preScreenTex->draw( preTexSprite, &cloneShader );

		preScreenTex->setView( view );

		//draws the player again on top of everything
		/*if( player->action != Actor::DEATH )
			player->Draw( preScreenTex );*/

		player->DodecaLateDraw( preScreenTex );

		

		//enemyTree->DebugDraw( preScreenTex );

		preScreenTex->draw( *debugBorders );

		preScreenTex->setView( uiView );
		DrawFade( preScreenTex );


		preScreenTex->display();

		const Texture &preTex0 = preScreenTex->getTexture();
		
		preTexSprite.setTexture( preTex0 );
		//preTexSprite.setPosition( -960 / 2, -540 / 2 );
		//preTexSprite.setScale( .5, .5 );

		window->draw( preTexSprite );//, &cloneShader );
		}
		}
		else if( state == PAUSE )
		{
			sf::Event ev;
			while( window->pollEvent( ev ) )
			{
				/*if( ev.type == sf::Event::KeyPressed )
				{
					if( ev.key.code = Keyboard::O )
					{
						state = RUN;
						soundNodeList->Pause( false );
						break;
					}
				}*/
				if( ev.type == sf::Event::GainedFocus )
				{
					//state = RUN;
					//soundNodeList->Pause( false );
					//break;
				}
				else if( ev.type == sf::Event::KeyPressed )
				{
					//if( ev.key.code == Keyboard::
				}
			}

			//savedinput when you enter pause
			prevInput = currInput;

			controller.UpdateState();
			currInput = controller.GetState();
			//cout << "up: " << (int)currInput.LUp() << "down: " << (int)currInput.LDown() <<
			//	", left: " << (int)currInput.LLeft() << ", right: " << (int)currInput.LRight() << endl;
			PauseMenu::UpdateResponse ur = pauseMenu->Update( currInput, prevInput );
			switch( ur )
			{
			case PauseMenu::R_NONE:
				{
					//do nothing as usual
					break;
				}
			case PauseMenu::R_P_RESUME:
				{
					state = GameSession::RUN;
					soundNodeList->Pause( false );
					break;
				}
			case PauseMenu::R_P_RESPAWN:
				{
					state = GameSession::RUN;
					RestartLevel();
					gameClock.restart();
					currentTime = 0;
					accumulator = TIMESTEP + .1;
					frameCounter = 0;
					//soundNodeList->Pause( false );
					//kill sounds on respawn
					break;
				}
			case PauseMenu::R_P_EXITLEVEL:
				{
					quit = true;
					returnVal = 1;
					break;
				}
			case PauseMenu::R_P_EXITTITLE:
				{
					quit = true;
					returnVal = 2;
					break;
				}
			case PauseMenu::R_P_EXITGAME:
				{
					quit = true;
					returnVal = 3;
					break;
				}

			}

			
			//if( currInput.

			/*if( Keyboard::isKeyPressed( Keyboard::O ) )
			{
				state = RUN;
				soundNodeList->Pause( false );
			}*/
			pauseTex->clear();		
			window->clear();
			Sprite preTexSprite;
			preTexSprite.setTexture( preScreenTex->getTexture() );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			preTexSprite.setScale( .5, .5 );
			window->draw( preTexSprite );

			pauseMenu->Draw( pauseTex );
			
			pauseTex->display();
			Sprite pauseMenuSprite;
			pauseMenuSprite.setTexture( pauseTex->getTexture() );
			//bgSprite.setPosition( );
			pauseMenuSprite.setPosition( (1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2 );
			pauseMenuSprite.setScale( .5, .5 );
			window->draw( pauseMenuSprite );

			//draw map

			if( pauseMenu->currentTab == PauseMenu::MAP )
			{
			View vv;
			vv.setCenter( pauseMenu->mapCenter );
			vv.setSize(  mapTex->getSize().x * pauseMenu->mapZoomFactor, mapTex->getSize().y * pauseMenu->mapZoomFactor );

			mapTex->clear();
			mapTex->setView( vv );
			mapTex->clear( Color( 0, 0, 0, 255 ) );
			
			View vuiView;
			vuiView.setSize( Vector2f( mapTex->getSize().x * 1.f, mapTex->getSize().y * 1.f ) );
			vuiView.setCenter( 0, 0 );
			
			for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
			{
				(*it)->Draw( mapTex );
			}

			queryMode = "border";
			numBorders = 0;
			sf::Rect<double> mapRect(vv.getCenter().x - vv.getSize().x / 2.0,
				vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

			borderTree->Query( this, mapRect );

			Color testColor( 0x75, 0x70, 0x90, 191 );
			testColor = Color::Green;
			TestVA * listVAIter = listVA;
			while( listVAIter != NULL )
			{
				int vertexCount = listVAIter->terrainVA->getVertexCount();
				for( int i = 0; i < vertexCount; ++i )
				{
					(*listVAIter->terrainVA)[i].color = testColor;
				}
				mapTex->draw( *listVAIter->terrainVA );
				for( int i = 0; i < vertexCount; ++i )
				{
					(*listVAIter->terrainVA)[i].color = Color::White;
				}

				listVAIter = listVAIter->next;
			}

			testGateCount = 0;
			queryMode = "gate";
			gateList = NULL;
			gateTree->Query( this, mapRect );
			Gate *mGateList = gateList;
			while( gateList != NULL )
			{
				//gateList->Draw( preScreenTex );
				if( gateList->locked )
				{

					V2d along = normalize(gateList->edgeA->v1 - gateList->edgeA->v0);
					V2d other( along.y, -along.x );
					double width = 25;
				
				

					V2d leftGround = gateList->edgeA->v0 + other * -width;
					V2d rightGround = gateList->edgeA->v0 + other * width;
					V2d leftAir = gateList->edgeA->v1 + other * -width;
					V2d rightAir = gateList->edgeA->v1 + other * width;
					//cout << "drawing color: " << gateList->c.b << endl;
					sf::Vertex activePreview[4] =
					{
						//sf::Vertex(sf::Vector2<float>( gateList->v0.x, gateList->v0.y ), gateList->c ),
						//sf::Vertex(sf::Vector2<float>( gateList->v1.x, gateList->v1.y ), gateList->c ),

						sf::Vertex(sf::Vector2<float>( leftGround.x, leftGround.y ), gateList->c ),
						sf::Vertex(sf::Vector2<float>( leftAir.x, leftAir.y ), gateList->c ),


						sf::Vertex(sf::Vector2<float>( rightAir.x, rightAir.y ), gateList->c ),

					
						sf::Vertex(sf::Vector2<float>( rightGround.x, rightGround.y ), gateList->c )
					};
					mapTex->draw( activePreview, 4, sf::Quads );
				}

				Gate *next = gateList->next;//edgeA->edge1;
				gateList = next;
			}


			

			Vector2i b = mapTex->mapCoordsToPixel( Vector2f( player->position.x, player->position.y ) );

			mapTex->setView( vuiView );

			Vector2f realPos = mapTex->mapPixelToCoords( b );
			realPos.x = floor( realPos.x + .5f );
			realPos.y = floor( realPos.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			kinMinimapIcon.setPosition( realPos );
			mapTex->draw( kinMinimapIcon );

			mapTex->setView( vv );			

			Vector2i b1 = mapTex->mapCoordsToPixel( Vector2f( originalPos.x, originalPos.y ) );

			mapTex->setView( vuiView );

			Vector2f realPos1 = mapTex->mapPixelToCoords( b1 );
			realPos1.x = floor( realPos1.x + .5f );
			realPos1.y = floor( realPos1.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			kinMapSpawnIcon.setPosition( realPos1 );
			mapTex->draw( kinMapSpawnIcon );
			
			mapTex->setView( vv );

			Vector2i bGoal = mapTex->mapCoordsToPixel( Vector2f( goalPos.x, goalPos.y ) );

			mapTex->setView( vuiView );

			Vector2f realPosGoal = mapTex->mapPixelToCoords( bGoal );
			realPosGoal.x = floor( realPosGoal.x + .5f );
			realPosGoal.y = floor( realPosGoal.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			goalMapIcon.setPosition( realPosGoal );
			mapTex->draw( goalMapIcon );

			if( currentZone != NULL )
			{
				for( list<Enemy*>::iterator it = currentZone->allEnemies.begin(); it != currentZone->allEnemies.end(); ++it )
				{
					//cout << "drawing map" << endl;
					(*it)->DrawMinimap( mapTex );
					/*if( (*it)->spawned && !(*it)->dead )
					{

					}*/
				}
			}

			//mapTex->clear();
			Sprite mapTexSprite;
			mapTexSprite.setTexture( mapTex->getTexture() );
			mapTexSprite.setOrigin( mapTexSprite.getLocalBounds().width / 2, mapTexSprite.getLocalBounds().height / 2 );
			mapTexSprite.setPosition( 0, 0 );
			

			//pauseTex->setView( bigV );
			//window->setView( bigV );

			//mapTexSprite.setScale( .5, -.5 );
			mapTexSprite.setScale( .5, -.5 );
			//mapTexSprite.setScale( 1, -1 );

			window->draw( mapTexSprite );
			//pauseTex->draw( mapTexSprite );

			//pauseTex->setV
			}


			

		}
		else if( state == MAP )
		{
			window->clear();

			window->setView( v );

			View bigV;
			bigV.setCenter( 0, 0 );
			bigV.setSize( 1920, 1080 );

			Sprite preTexSprite;
			preTexSprite.setTexture( preScreenTex->getTexture() );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			preTexSprite.setScale( .5, .5 );
			window->draw( preTexSprite );

			
			accumulator += frameTime;

			while ( accumulator >= TIMESTEP  )
			{
				prevInput = currInput;

				
				if( !controller.UpdateState() )
				{
					bool up = Keyboard::isKeyPressed( Keyboard::Up );// || Keyboard::isKeyPressed( Keyboard::W );
					bool down = Keyboard::isKeyPressed( Keyboard::Down );// || Keyboard::isKeyPressed( Keyboard::S );
					bool left = Keyboard::isKeyPressed( Keyboard::Left );// || Keyboard::isKeyPressed( Keyboard::A );
					bool right = Keyboard::isKeyPressed( Keyboard::Right );// || Keyboard::isKeyPressed( Keyboard::D );

				//	bool altUp = Keyboard::isKeyPressed( Keyboard::U );
						//	bool altLeft = Keyboard::isKeyPressed( Keyboard::H );
			//	bool altRight = Keyboard::isKeyPressed( Keyboard::K );
			//	bool altDown = Keyboard::isKeyPressed( Keyboard::J );

					ControllerState keyboardInput;    
					keyboardInput.B = Keyboard::isKeyPressed( Keyboard::X );// || Keyboard::isKeyPressed( Keyboard::Period );
					keyboardInput.rightShoulder = Keyboard::isKeyPressed( Keyboard::C );// || Keyboard::isKeyPressed( Keyboard::Comma );
					keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::D );// || Keyboard::isKeyPressed( Keyboard::M );
						keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Z ) || Keyboard::isKeyPressed( Keyboard::Space );// || Keyboard::isKeyPressed( Keyboard::Slash );
				//keyboardInput.leftTrigger = 255 * (Keyboard::isKeyPressed( Keyboard::F ) || Keyboard::isKeyPressed( Keyboard::L ));
					keyboardInput.leftShoulder = Keyboard::isKeyPressed( Keyboard::LShift );
					keyboardInput.X = Keyboard::isKeyPressed( Keyboard::F );
					keyboardInput.start = Keyboard::isKeyPressed( Keyboard::J );
					keyboardInput.back = Keyboard::isKeyPressed( Keyboard::H );
					keyboardInput.rightTrigger = 255 * Keyboard::isKeyPressed( Keyboard::LControl );
					keyboardInput.leftTrigger = 255 * Keyboard::isKeyPressed( Keyboard::RControl );
			
					keyboardInput.rightStickPad = 0;
								if( Keyboard::isKeyPressed( Keyboard::A ) )
				{
					keyboardInput.rightStickPad += 1 << 1;
				}
								else if( Keyboard::isKeyPressed( Keyboard::S ) )
				{
					keyboardInput.rightStickPad += 1;
				}
				
					if( up && down )
					{
						if( prevInput.LUp() )
							keyboardInput.leftStickPad += 1;
						else if( prevInput.LDown() )
							keyboardInput.leftStickPad += ( 1 && down ) << 1;
					}
					else
					{
						keyboardInput.leftStickPad += 1 && up;
						keyboardInput.leftStickPad += ( 1 && down ) << 1;
					}

					if( left && right )
					{
						if( prevInput.LLeft() )
						{
							keyboardInput.leftStickPad += ( 1 && left ) << 2;
						}
						else if( prevInput.LRight() )
						{
							keyboardInput.leftStickPad += ( 1 && right ) << 3;
						}
					}
					else
					{
						keyboardInput.leftStickPad += ( 1 && left ) << 2;
						keyboardInput.leftStickPad += ( 1 && right ) << 3;
					}

					currInput = keyboardInput;
				}
				else
				{
					controller.UpdateState();
					currInput = controller.GetState();
				}

			

				

				if( ( currInput.back && !prevInput.back ) && state == MAP )
				{
					state = RUN;
					soundNodeList->Pause( false );
				}

				accumulator -= TIMESTEP;
			}

			if( state == RUN )
			{
			//	continue;
			}


			//if( !cutPlayerInput )
			//	player->currInput = currInput;


			//double mapZoom = 16;

			//window->clear();
			
			View vv;
			vv.setCenter( pauseMenu->mapCenter );
			vv.setSize(  mapTex->getSize().x * pauseMenu->mapZoomFactor, mapTex->getSize().y * pauseMenu->mapZoomFactor );

			mapTex->clear();
			mapTex->setView( vv );
			mapTex->clear( Color( 0, 0, 0, 255 ) );
			
			View vuiView;
			vuiView.setSize( Vector2f( mapTex->getSize().x * 1.f, mapTex->getSize().y * 1.f ) );
			vuiView.setCenter( 0, 0 );
			
			for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
			{
				(*it)->Draw( mapTex );
			}

			queryMode = "border";
			numBorders = 0;
			sf::Rect<double> mapRect(vv.getCenter().x - vv.getSize().x / 2.0,
				vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

			borderTree->Query( this, mapRect );

			Color testColor( 0x75, 0x70, 0x90, 191 );
			testColor = Color::Green;
			TestVA * listVAIter = listVA;
			while( listVAIter != NULL )
			{
				int vertexCount = listVAIter->terrainVA->getVertexCount();
				for( int i = 0; i < vertexCount; ++i )
				{
					(*listVAIter->terrainVA)[i].color = testColor;
				}
				mapTex->draw( *listVAIter->terrainVA );
				for( int i = 0; i < vertexCount; ++i )
				{
					(*listVAIter->terrainVA)[i].color = Color::White;
				}

				listVAIter = listVAIter->next;
			}

			testGateCount = 0;
			queryMode = "gate";
			gateList = NULL;
			gateTree->Query( this, mapRect );
			Gate *mGateList = gateList;
			while( gateList != NULL )
			{
				//gateList->Draw( preScreenTex );
				if( gateList->locked )
				{

					V2d along = normalize(gateList->edgeA->v1 - gateList->edgeA->v0);
					V2d other( along.y, -along.x );
					double width = 25;
				
				

					V2d leftGround = gateList->edgeA->v0 + other * -width;
					V2d rightGround = gateList->edgeA->v0 + other * width;
					V2d leftAir = gateList->edgeA->v1 + other * -width;
					V2d rightAir = gateList->edgeA->v1 + other * width;
					//cout << "drawing color: " << gateList->c.b << endl;
					sf::Vertex activePreview[4] =
					{
						//sf::Vertex(sf::Vector2<float>( gateList->v0.x, gateList->v0.y ), gateList->c ),
						//sf::Vertex(sf::Vector2<float>( gateList->v1.x, gateList->v1.y ), gateList->c ),

						sf::Vertex(sf::Vector2<float>( leftGround.x, leftGround.y ), gateList->c ),
						sf::Vertex(sf::Vector2<float>( leftAir.x, leftAir.y ), gateList->c ),


						sf::Vertex(sf::Vector2<float>( rightAir.x, rightAir.y ), gateList->c ),

					
						sf::Vertex(sf::Vector2<float>( rightGround.x, rightGround.y ), gateList->c )
					};
					mapTex->draw( activePreview, 4, sf::Quads );
				}

				Gate *next = gateList->next;//edgeA->edge1;
				gateList = next;
			}

			//queryMode = "enemyminimap";
			//enemyTree->Query( this, mapRect );

			//Enemy *currEnemy = activeEnemyList;
			//int counter = 0;
		
			/*while( currEnemy != NULL )
			{
				if( currEnemy->hasMonitor )
				{
					currEnemy->DrawMinimap( mapTex );
					
				}
				currEnemy = currEnemy->next;
			}*/

			if( currentZone != NULL )
			{
				for( list<Enemy*>::iterator it = currentZone->allEnemies.begin(); it != currentZone->allEnemies.end(); ++it )
				{
					//cout << "drawing map" << endl;
					(*it)->DrawMinimap( mapTex );
					/*if( (*it)->spawned && !(*it)->dead )
					{

					}*/
				}
			}

			Vector2i b = mapTex->mapCoordsToPixel( Vector2f( player->position.x, player->position.y ) );

			mapTex->setView( vuiView );

			Vector2f realPos = mapTex->mapPixelToCoords( b );
			realPos.x = floor( realPos.x + .5f );
			realPos.y = floor( realPos.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			kinMinimapIcon.setPosition( realPos );
			mapTex->draw( kinMinimapIcon );

			mapTex->setView( vv );			

			Vector2i b1 = mapTex->mapCoordsToPixel( Vector2f( originalPos.x, originalPos.y ) );

			mapTex->setView( vuiView );

			Vector2f realPos1 = mapTex->mapPixelToCoords( b1 );
			realPos1.x = floor( realPos1.x + .5f );
			realPos1.y = floor( realPos1.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			kinMapSpawnIcon.setPosition( realPos1 );
			mapTex->draw( kinMapSpawnIcon );
			
			mapTex->setView( vv );

			Vector2i bGoal = mapTex->mapCoordsToPixel( Vector2f( goalPos.x, goalPos.y ) );

			mapTex->setView( vuiView );

			Vector2f realPosGoal = mapTex->mapPixelToCoords( bGoal );
			realPosGoal.x = floor( realPosGoal.x + .5f );
			realPosGoal.y = floor( realPosGoal.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			goalMapIcon.setPosition( realPosGoal );
			mapTex->draw( goalMapIcon );
			//mapTex->clear();
			Sprite mapTexSprite;
			mapTexSprite.setTexture( mapTex->getTexture() );
			mapTexSprite.setOrigin( mapTexSprite.getLocalBounds().width / 2, mapTexSprite.getLocalBounds().height / 2 );
			mapTexSprite.setPosition( 0, 0 );
			
			//window->setView( bigV );

			//mapTexSprite.setScale( .5, -.5 );
			mapTexSprite.setScale( .5, -.5 );
			cout << "size: " << mapTexSprite.getLocalBounds().width << ", " << mapTexSprite.getLocalBounds().height << endl;
			//mapTexSprite.setColor( Color::Red );
			window->draw( mapTexSprite );

		}

		window->display();

		
		
	}

	delete [] line;

	//window->setView( window->getDefaultView() );
	//window->clear( Color::Red );
	//window->display();
	if( currMusic != NULL )
	{
		currMusic->stop();
		currMusic = NULL;
	}
	
	soundNodeList->Reset();
	
	return returnVal;
}

void GameSession::HandleEntrant( QuadTreeEntrant *qte )
{
	if( queryMode == "enemy" )
	{
		Enemy *e = (Enemy*)qte;

		

		bool a = e->spawnRect.intersects( tempSpawnRect );
		bool b = ( e->zone == NULL || e->zone->active ); 
		
		//if( e->type == Enemy::NEXUS )
		//{
		//	cout << "zone: " << e->zone << ", " << e->zone->active << endl;
		//	cout << "orig: " << originalZone << ", " << originalZone->active << endl;
		//	cout << "blah: " << e->zone->allEnemies.size() << endl;
		//	//cout << "querying nexus: " << (int)a << ", " << (int)b << ", :: " 
		//		//<< ( e->zone == NULL ) << endl;
		//	
		//}

		//sf::Rect<double> screenRect( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
		if( a && b )
		{
			cout << "spawning enemy! of type: " << e->type << endl;
			assert( e->spawned == false );
			e->spawned = true;

			

			AddEnemy( e );
		}
	}
	else if( queryMode == "enemyminimap" )
	{
		Enemy *e = (Enemy*)qte;

		//if( e->spawnRect.intersects( tempSpawnRect ) )
		//{
			//cout << "spawning enemy! of type: " << e->type << endl;
			if( !e->spawned )
			{
				e->DrawMinimap( minimapTex );
			}
			
			//e->spawned = true;

			

			//AddEnemy( e );
		//}
	}
	else if( queryMode == "border" )
	{
		if( listVA == NULL )
		{
			listVA = (TestVA*)qte;
		//	cout << "1" << endl;
			numBorders++;
		}
		else
		{
			
			TestVA *tva = (TestVA*)qte;
			TestVA *temp = listVA;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tva )
				{
					okay = false;
					break;
				}	
				temp = temp->next;
			}

			if( okay )
			{
			
			//cout << "blah: " << (unsigned)tva << endl;
				tva->next = listVA;
				listVA = tva;
				numBorders++;
				//cout << numBorders + 1 << endl;
			}
		}
		
	}
	else if( queryMode == "lightdisplay" )
	{
		if( lightList == NULL )
		{
			lightList = (Light*)qte;
		}
		else
		{
			
			Light *tlight = (Light*)qte;
			Light *temp = lightList;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tlight )
				{
					okay = false;
					break;
				}	
				temp = temp->next;
			}

			if( okay )
			{
				tlight->next = lightList;
				lightList = tlight;
			}
		}
	}
	else if( queryMode == "lights" )
	{
		Light *light = (Light*)qte;

		if( lightsAtOnce < tempLightLimit )
		{
			touchedLights[lightsAtOnce] = light;
			lightsAtOnce++;
		}
		else
		{
			//for( int i = 0; i < lightsAtOnce; ++i )
			//{
			//	if( length( V2d( touchedLights[i]->pos.x, touchedLights[i]->pos.y ) - position ) > length( V2d( light->pos.x, light->pos.y ) - position ) )//some calculation here
			//	{
			//		touchedLights[i] = light;
			//		break;
			//	}
					
			//}
		}
	
	}
	else if( queryMode == "gate" )
	{
		Gate *g = (Gate*)qte;

		if( gateList == NULL )
		{
			gateList = (Gate*)qte;
			gateList->next = NULL;
			gateList->prev = NULL;
			
			//cout << "setting gate: " << gateList->edgeA << endl;
		}
		else
		{
			g->next = gateList;
			gateList = g;
		}

		//cout << "gate" << endl;
		++testGateCount;
	}
	else if( queryMode == "item" )
	{
		Critical *c = (Critical*)qte;

		if( drawCritical == NULL )
		{
			drawCritical = (Critical*)qte;
			drawCritical->next = NULL;
			drawCritical->prev = NULL;
		}
		else
		{
			c->next = drawCritical;
			drawCritical = c;
		}
	}
	else if( queryMode == "crawlerreverser" )
	{
		CrawlerReverser *cr = (CrawlerReverser*)qte;
		if( drawCrawlerReversers == NULL )
		{
			drawCrawlerReversers = cr;
			cr->drawNext = NULL;
		}
		else
		{
			cr->drawNext = drawCrawlerReversers;
			drawCrawlerReversers = cr;
		}
	}
	else if( queryMode == "envplant" )
	{
		EnvPlant *ep = (EnvPlant*)qte;

		if( !ep->activated )
		{

			int idleLength = ep->idleLength;
			int idleFactor = ep->idleFactor;

			IntRect sub = ep->ts->GetSubRect( (totalGameFrames % ( idleLength * idleFactor )) / idleFactor );
			VertexArray &eva = *ep->va;
			eva[ep->vaIndex + 0].texCoords = Vector2f( sub.left, sub.top );
			eva[ep->vaIndex + 1].texCoords = Vector2f( sub.left + sub.width, sub.top );
			eva[ep->vaIndex + 2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
			eva[ep->vaIndex + 3].texCoords = Vector2f( sub.left, sub.top + sub.height );
		}
		//va[ep->
	}
}

void GameSession::DebugDrawActors()
{
	player->DebugDraw( preScreenTex );
	
	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{
		currEnemy->DebugDraw( preScreenTex );
		currEnemy = currEnemy->next;
	}
}

void GameSession::SuppressEnemyKeys( Gate::GateType gType )
{
	for( list<Enemy*>::iterator it = currentZone->allEnemies.begin();
		it != currentZone->allEnemies.end(); ++it )
	{
		(*it)->suppressMonitor = true;
	}
	//cout << "suppress keys???" << endl;
	//Enemy *currEnemy = activeEnemyList;
	//int mType;
	//while( currEnemy != NULL )
	//{
	//	
	//	if( currEnemy->hasMonitor )
	//	{
	//		currEnemy->suppressMonitor = true;
	//	}
	//	//currEnemy->moni
	//	currEnemy = currEnemy->next;
	//}
}

void GameSession::KillAllEnemies()
{
	Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		Enemy *next = curr->next;

		if( curr->type != Enemy::GOAL )
		{
			curr->DirectKill();
			//curr->health = 0;
		}
		curr = next;	
	}
}

void GameSession::TestVA::UpdateBushFrame()
{
	bushFrame++;
	if( bushFrame == bushAnimLength * bushAnimFactor )
	{
		bushFrame = 0;
	}
}

void GameSession::TestVA::UpdateBushes()
{
	int numBushes = bushVA->getVertexCount() / 4;

	VertexArray &bVA = *bushVA;
	IntRect subRect = ts_bush->GetSubRect( bushFrame / bushAnimFactor );

	for( int i = 0; i < numBushes; ++i )
	{
		bVA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		bVA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		bVA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		bVA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}
	//++bushFrame;
}

void GameSession::TestVA::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool GameSession::TestVA::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( aabb, r );
}

void GameSession::ResetShipSequence()
{
	drain = false;
	player->action = Actor::RIDESHIP;
	player->frame = 0;
	PoiInfo *pi = poiMap["ship"];
	player->position = pi->pos;
	originalPos = player->position;
	shipSprite.setPosition( pi->pos.x - 13, pi->pos.y - 124 );
	//cloud0a.setpo
	shipSequence = true;
	shipSeqFrame = 0;
	shipStartPos = shipSprite.getPosition();//Vector2f( pi->pos.x, pi->pos.y );
	cloudVel = Vector2f( -40, 0 );
	relShipVel = Vector2f( 2, 0 );
	//#0055FF
	middleClouds.setFillColor( Color( 0x00, 0x55, 0xFF ) );
	int middleHeight = 540 * 4;
	middleClouds.setSize( Vector2f( 960, middleHeight ) );
	Vector2f botExtra( 0, middleHeight );
	//middleClouds.setOrigin( middleClouds.getLocalBounds().width / 2,
	//	middleClouds.getLocalBounds().height / 2 );

	IntRect sub0 = ts_w1ShipClouds0->GetSubRect( 0 );
	IntRect sub1 = ts_w1ShipClouds1->GetSubRect( 0 );
			
	Vector2f bottomLeft = Vector2f( pi->pos.x, pi->pos.y ) + Vector2f( -480, 270 ); 
	for( int i = 0; i < 3; ++i )
	{
		Vector2f xExtra( 480 * i, 0 );
		cloud0[i*4+0].position = xExtra + bottomLeft;
		cloud0[i*4+1].position = xExtra + bottomLeft + Vector2f( 0, -sub0.height / 2 );
		cloud0[i*4+2].position = xExtra + bottomLeft + Vector2f( sub0.width / 2, -sub0.height / 2 );
		cloud0[i*4+3].position = xExtra + bottomLeft + Vector2f( sub0.width / 2, 0 );

		cloud0[i*4+0].texCoords = Vector2f( 0, sub0.height );
		cloud0[i*4+1].texCoords = Vector2f( 0, 0 );
		cloud0[i*4+2].texCoords = Vector2f( sub0.width, 0 );
		cloud0[i*4+3].texCoords = Vector2f( sub0.width, sub0.height );

		cloud1[i*4+0].position = xExtra + bottomLeft;
		cloud1[i*4+1].position = xExtra + bottomLeft + Vector2f( 0, -sub1.height / 2 );
		cloud1[i*4+2].position = xExtra + bottomLeft + Vector2f( sub1.width / 2, -sub1.height / 2 );
		cloud1[i*4+3].position = xExtra + bottomLeft + Vector2f( sub1.width / 2, 0 );

		cloud1[i*4+0].texCoords = Vector2f( 0, sub1.height );
		cloud1[i*4+1].texCoords = Vector2f( 0, 0 );
		cloud1[i*4+2].texCoords = Vector2f( sub1.width, 0 );
		cloud1[i*4+3].texCoords = Vector2f( sub1.width, sub1.height );

		cloudBot0[i*4+0].position = botExtra + xExtra + bottomLeft;
		cloudBot0[i*4+1].position = botExtra + xExtra + bottomLeft + Vector2f( 0, sub0.height / 2 );
		cloudBot0[i*4+2].position = botExtra + xExtra + bottomLeft + Vector2f( sub0.width / 2, sub0.height / 2 );
		cloudBot0[i*4+3].position = botExtra + xExtra + bottomLeft + Vector2f( sub0.width / 2, 0 );

		cloudBot0[i*4+0].texCoords = Vector2f( 0, sub0.height );
		cloudBot0[i*4+1].texCoords = Vector2f( 0, 0 );
		cloudBot0[i*4+2].texCoords = Vector2f( sub0.width, 0 );
		cloudBot0[i*4+3].texCoords = Vector2f( sub0.width, sub0.height );

		cloudBot1[i*4+0].position = botExtra + xExtra + bottomLeft;
		cloudBot1[i*4+1].position = botExtra + xExtra + bottomLeft + Vector2f( 0, sub1.height / 2 );
		cloudBot1[i*4+2].position = botExtra + xExtra + bottomLeft + Vector2f( sub1.width / 2, sub1.height / 2 );
		cloudBot1[i*4+3].position = botExtra + xExtra + bottomLeft + Vector2f( sub1.width / 2, 0 );

		cloudBot1[i*4+0].texCoords = Vector2f( 0, sub1.height );
		cloudBot1[i*4+1].texCoords = Vector2f( 0, 0 );
		cloudBot1[i*4+2].texCoords = Vector2f( sub1.width, 0 );
		cloudBot1[i*4+3].texCoords = Vector2f( sub1.width, sub1.height );
	}

	middleClouds.setPosition( pi->pos.x - 480, pi->pos.y + 270 );
}

void GameSession::RespawnPlayer()
{
	soundNodeList->Reset();
	scoreDisplay->Reset();
	player->hitGoal = false;
	currentZone = originalZone;
	if( currentZone != NULL )
		keyMarker->SetStartKeys( currentZone->requiredKeys );
	if( player->currentCheckPoint == NULL )
	{
		player->position = originalPos;
		
		//actually keys should be set based on which ones you had at the last checkpoint
		for( int i = 2; i < Gate::GateType::Count; ++i )
		{
			player->numKeys = 0;
			//player->hasKey[i] = 0;
		}
	}
	else
	{
		player->position = player->currentCheckPoint->pos;

		//might take out checkpoints so idk how this would work
		//for( int i = 2; i < Gate::GateType::Count; ++i )
		//{
			//player->hasKey[i] = player->currentCheckPoint->hadKey[i];
		//}
	}

	//player->seq = Actor::SEQ_NOTHING;
	player->followerPos = player->position;
	player->followerVel = V2d( 0, 0 );
	player->enemiesKilledThisFrame = 0;
	player->gateTouched = NULL;

	if( poiMap.count( "ship" ) > 0 )
	{
		ResetShipSequence();
		//shipSequence = true;
		//relShipVel = Vector2f( 2, 0 );

	}
	else
	{
		player->action = player->INTRO;
		player->frame = 0;
	}
	


	player->velocity.x = 0;
	player->velocity.y = 0;
	player->reversed = false;
	player->b.offset.y = 0;
	player->b.rh = player->normalHeight;
	player->facingRight = true;
	player->offsetX = 0;
	player->prevInput = ControllerState();
	player->currInput = ControllerState();
	player->ground = NULL;
	player->grindEdge = NULL;
	player->bounceEdge = NULL;
	player->dead = false;
	powerBar.points = 100;
	powerBar.layer = 0;
	player->record = 0;
	player->recordedGhosts = 0;
	player->blah = false;
	player->receivedHit = NULL;
	player->speedParticleCounter = 1;
	player->speedLevel = 0;
	player->speedBarTarget = 0;//60;
	player->currentSpeedBar = 0;//60;

	player->bounceFlameOn = false;

	if( player->hasPowerLeftWire )
	{
		player->leftWire->Reset();
	}
	if( player->hasPowerRightWire )
	{
		player->rightWire->Reset();
	}
	
	//powerBar.Reset();
	player->lastWire = 0;
	player->desperationMode = false;

	player->flashFrames = 0;
	
	powerWheel->Reset();
	//currentZone = NULL;
	cam.zoomFactor = 1;
	cam.pos.x = player->position.x;
	cam.pos.y = player->position.y;
	cam.offset = Vector2f( 0, 0 );
	cam.manual = false;
	cam.rumbling = false;

	player->hasDoubleJump = true;
	player->hasAirDash = true;
	player->hasGravReverse = true;

	if( !cam.bossCrawler )
	{
		cam.zoomFactor = 1;
		cam.zoomLevel = 0;
		cam.offset = Vector2f( 0, 0 );
	}
	

	for( int i = 0; i < player->maxBubbles; ++i )
	{
		player->bubbleFramesToLive[i] = 0;
		//if( player->bubbleFramesToLive[i] > 0 )
		//{
			
		//}
	}

	for( int i = 0; i < player->MAX_MOTION_GHOSTS; ++i )
	{
		player->motionGhosts[i].setPosition( player->position.x, player->position.y );
	}

	player->SetExpr( Actor::Expr::Expr_NEUTRAL );
}

void GameSession::ClearFX()
{
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		Enemy *curr = effectLists[i];
		while( curr != NULL )
		{
			Enemy *next = curr->next;
			assert( curr->type == Enemy::BASICEFFECT );
			DeactivateEffect( (BasicEffect*)curr );

			curr = next;
		}
		effectLists[i] = NULL;
	}
}

void GameSession::RestartLevel()
{
	soundNodeList->Clear();

	totalGameFrames = 0;

	if( player->record > 1 )
	{
		player->LoadState();
		LoadState();
	}

	goalPulse->Reset();
	//f->Reset();

	activeDialogue = NULL;

	fadingIn = false;
	fadingOut = false;

	//crawlerFightSeq->Reset();
	//crawlerAfterFightSeq->Reset();
	//enterNexus1Seq->Reset();
	activeSequence = NULL;

	RespawnPlayer();
	pauseFrames = 0;

	ResetEnemies();
	ResetPlants(); //eventually maybe treat these to reset like the rest of the stuff
	//only w/ checkpoints. but for now its always back

	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		(*it)->active = false;
	}

	if( originalZone != NULL )
		originalZone->active = true;
	//
	//later don't relock gates in a level unless there is a "level reset"
	for( int i = 0; i < numGates; ++i )
	{
		gates[i]->SetLocked( true );
		if( gates[i]->type != Gate::BLACK )
			gates[i]->gState = Gate::HARD;
	}

	inactiveEnemyList = NULL;

	for( list<Barrier*>::iterator it = barriers.begin(); it != barriers.end(); ++it )
	{
		(*it)->triggered = false;
	}
				
	pauseImmuneEffects = NULL;
	cloneInactiveEnemyList = NULL;

	cam.SetManual( false );
	//inGameClock.restart();
}

void GameSession::UpdateTerrainShader( const sf::Rect<double> &aabb )
{
	lightsAtOnce = 0;
	tempLightLimit = 9;

	queryMode = "lights"; 
	lightTree->Query( this, aabb );

	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f,  -1 + vi.y / 1080.f, .015 );
	//polyShader.setParameter( "stuff", 10, 10, 10 );
	
/*	Vector3f pos0( vi0.x / 1920.f, (1080 - vi0.y) / 1080.f, .015 ); 
	pos0.y = 1 - pos0.y;
	Vector3f pos1( vi1.x / 1920.f, (1080 - vi1.y) / 1080.f, .015 ); 
	pos1.y = 1 - pos1.y;
	Vector3f pos2( vi2.x / 1920.f, (1080 - vi2.y) / 1080.f, .015 ); 
	pos2.y = 1 - pos2.y;*/
	
	bool on[9];
	for( int i = 0; i < 9; ++i )
	{
		on[i] = false;
	}

	
	float windowx = 1920;//window->getSize().x;
	float windowy = 1080;//window->getSize().y;
	//cout << "windowx: " << windowx << ", " << windowy << endl;

	//if( lightsAtOnce > 0 )
	//{
	//	float depth0 = touchedLights[0]->depth;
	//	Vector2i vi0 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[0]->pos.x, touchedLights[0]->pos.y ) );
	//	

	//	Vector3f pos0( vi0.x / windowx, -1 + vi0.y / windowy, depth0 ); 
	//	//Vector3f pos0( vi0.x / (float)window->getSize().x, ((float)window->getSize().y - vi0.y) / (float)window->getSize().y, depth0 ); 
	//	Color c0 = touchedLights[0]->color;
	//	
	//	//underShader.setParameter( "On0", true );
	//	on[0] = true;
	//	polyShader.setParameter( "LightPos0", pos0 );//Vector3f( 0, -300, .075 ) );
	//	polyShader.setParameter( "LightColor0", c0.r / 255.0, c0.g / 255.0, c0.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius0", touchedLights[0]->radius );
	//	polyShader.setParameter( "Brightness0", touchedLights[0]->brightness);
	//	
	//}
	//if( lightsAtOnce > 1 )
	//{
	//	float depth1 = touchedLights[1]->depth;
	//	Vector2i vi1 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[1]->pos.x, touchedLights[1]->pos.y ) ); 
	//	Vector3f pos1( vi1.x / windowx, -1 + vi1.y / windowy, depth1 ); 
	//	//Vector3f pos1( vi1.x / (float)window->getSize().x, ((float)window->getSize().y - vi1.y) / (float)window->getSize().y, depth1 ); 
	//	Color c1 = touchedLights[1]->color;
	//	
	//	on[1] = true;
	//	//underShader.setParameter( "On1", true );
	//	polyShader.setParameter( "LightPos1", pos1 );//Vector3f( 0, -300, .075 ) );
	//	polyShader.setParameter( "LightColor1", c1.r / 255.0, c1.g / 255.0, c1.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius1", touchedLights[1]->radius );
	//	polyShader.setParameter( "Brightness1", touchedLights[1]->brightness);
	//}
	//if( lightsAtOnce > 2 )
	//{
	//	float depth2 = touchedLights[2]->depth;
	//	Vector2i vi2 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[2]->pos.x, touchedLights[2]->pos.y ) );
	//	Vector3f pos2( vi2.x / windowx, -1 + vi2.y / windowy, depth2 ); 
	//	//Vector3f pos2( vi2.x / (float)window->getSize().x, ((float)window->getSize().y - vi2.y) / (float)window->getSize().y, depth2 ); 
	//	Color c2 = touchedLights[2]->color;
	//	
	//	on[2] = true;
	//	//underShader.setParameter( "On2", true );
	//	polyShader.setParameter( "LightPos2", pos2 );//Vector3f( 0, -300, .075 ) );
	//	polyShader.setParameter( "LightColor2", c2.r / 255.0, c2.g / 255.0, c2.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius2", touchedLights[2]->radius );
	//	polyShader.setParameter( "Brightness2", touchedLights[2]->brightness);
	//}
	//if( lightsAtOnce > 3 )
	//{
	//	float depth3 = touchedLights[3]->depth;
	//	Vector2i vi3 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[3]->pos.x, touchedLights[3]->pos.y ) );
	//	Vector3f pos3( vi3.x / windowx, -1 + vi3.y / windowy, depth3 ); 
	//	//Vector3f pos3( vi3.x / (float)window->getSize().x, ((float)window->getSize().y - vi3.y) / (float)window->getSize().y, depth3 ); 
	//	Color c3 = touchedLights[3]->color;
	//	
	//	on[3] = true;
	//	//underShader.setParameter( "On3", true );
	//	polyShader.setParameter( "LightPos3", pos3 );
	//	polyShader.setParameter( "LightColor3", c3.r / 255.0, c3.g / 255.0, c3.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius3", touchedLights[3]->radius );
	//	polyShader.setParameter( "Brightness3", touchedLights[3]->brightness);
	//}
	//if( lightsAtOnce > 4 )
	//{
	//	float depth4 = touchedLights[4]->depth;
	//	Vector2i vi4 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[4]->pos.x, touchedLights[4]->pos.y ) );
	//	Vector3f pos4( vi4.x / windowx, -1 + vi4.y / windowy, depth4 ); 
	//	//Vector3f pos4( vi4.x / (float)window->getSize().x, ((float)window->getSize().y - vi4.y) / (float)window->getSize().y, depth4 ); 
	//	Color c4 = touchedLights[4]->color;
	//	
	//	
	//	on[4] = true;
	//	polyShader.setParameter( "LightPos4", pos4 );
	//	polyShader.setParameter( "LightColor4", c4.r / 255.0, c4.g / 255.0, c4.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius4", touchedLights[4]->radius );
	//	polyShader.setParameter( "Brightness4", touchedLights[4]->brightness);
	//}
	//if( lightsAtOnce > 5 )
	//{
	//	float depth5 = touchedLights[5]->depth;
	//	Vector2i vi5 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[5]->pos.x, touchedLights[5]->pos.y ) );
	//	Vector3f pos5( vi5.x / windowx, -1 + vi5.y / windowy, depth5 ); 
	//	//Vector3f pos5( vi5.x / (float)window->getSize().x, ((float)window->getSize().y - vi5.y) / (float)window->getSize().y, depth5 ); 
	//	Color c5 = touchedLights[5]->color;
	//	
	//	
	//	on[5] = true;
	//	polyShader.setParameter( "LightPos5", pos5 );
	//	polyShader.setParameter( "LightColor5", c5.r / 255.0, c5.g / 255.0, c5.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius5", touchedLights[5]->radius );
	//	polyShader.setParameter( "Brightness5", touchedLights[5]->brightness);
	//}
	//if( lightsAtOnce > 6 )
	//{
	//	float depth6 = touchedLights[6]->depth;
	//	Vector2i vi6 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[6]->pos.x, touchedLights[6]->pos.y ) );
	//	Vector3f pos6( vi6.x / windowx, -1 + vi6.y / windowy, depth6 ); 
	//	//Vector3f pos6( vi6.x / (float)window->getSize().x, ((float)window->getSize().y - vi6.y) / (float)window->getSize().y, depth6 ); 
	//	Color c6 = touchedLights[6]->color;
	//	
	//	on[6] = true;
	//	polyShader.setParameter( "LightPos6", pos6 );
	//	polyShader.setParameter( "LightColor6", c6.r / 255.0, c6.g / 255.0, c6.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius6", touchedLights[0]->radius );
	//	polyShader.setParameter( "Brightness6", touchedLights[0]->brightness);
	//}
	//if( lightsAtOnce > 7 )
	//{
	//	float depth7 = touchedLights[7]->depth;
	//	Vector2i vi7 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[7]->pos.x, touchedLights[7]->pos.y ) );
	//	Vector3f pos7( vi7.x / windowx, -1 + vi7.y / windowy, depth7 ); 
	//	//Vector3f pos7( vi7.x / (float)window->getSize().x, ((float)window->getSize().y - vi7.y) / (float)window->getSize().y, depth7 ); 
	//	Color c7 = touchedLights[7]->color;
	//	
	//	on[7] = true;
	//	polyShader.setParameter( "LightPos7", pos7 );
	//	polyShader.setParameter( "LightColor7", c7.r / 255.0, c7.g / 255.0, c7.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius7", touchedLights[7]->radius );
	//	polyShader.setParameter( "Brightness7", touchedLights[7]->brightness);
	//}
	//if( lightsAtOnce > 8 )
	//{
	//	float depth8 = touchedLights[8]->depth;
	//	Vector2i vi8 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[8]->pos.x, touchedLights[8]->pos.y ) );
	//	Vector3f pos8( vi8.x / windowx, -1 + vi8.y / windowy, depth8 ); 
	//	//Vector3f pos8( vi8.x / (float)window->getSize().x, ((float)window->getSize().y - vi8.y) / (float)window->getSize().y, depth8 ); 
	//	Color c8 = touchedLights[8]->color;
	//	
	//	on[8] = true;
	//	polyShader.setParameter( "LightPos8", pos8 );
	//	polyShader.setParameter( "LightColor8", c8.r / 255.0, c8.g / 255.0, c8.b / 255.0, 1 );
	//	polyShader.setParameter( "Radius8", touchedLights[8]->radius );
	//	polyShader.setParameter( "Brightness8", touchedLights[8]->brightness);
	//}

	//polyShader.setParameter( "On0", on[0] );
	//polyShader.setParameter( "On1", on[1] );
	//polyShader.setParameter( "On2", on[2] );
	//polyShader.setParameter( "On3", on[3] );
	//polyShader.setParameter( "On4", on[4] );
	//polyShader.setParameter( "On5", on[5] );
	//polyShader.setParameter( "On6", on[6] );
	//polyShader.setParameter( "On7", on[7] );
	//polyShader.setParameter( "On8", on[8] );

	//Color c = player->testLight->color;
	//Vector2i vip = preScreenTex->mapCoordsToPixel( Vector2f( player->testLight->pos.x, player->testLight->pos.y ) );
	//Vector3f posp( vip.x / windowx, -1 + vip.y / windowy, player->testLight->depth ); 
	//polyShader.setParameter( "LightPosPlayer", posp );
	//polyShader.setParameter( "LightColorPlayer", c.r / 255.0, c.g / 255.0, c.b / 255.0, 1 );
	//polyShader.setParameter( "RadiusPlayer", player->testLight->radius );
	//polyShader.setParameter( "BrightnessPlayer", player->testLight->brightness );
	//polyShader.setParameter( "OnD0", true );
}

double GameSession::GetTriangleArea( p2t::Triangle * t )
{
	p2t::Point *p_0 = t->GetPoint( 0 );
	p2t::Point *p_1 = t->GetPoint( 0 );
	p2t::Point *p_2 = t->GetPoint( 0 );

	V2d p0( p_0->x, p_0->y );
	V2d p1( p_1->x, p_1->y );
	V2d p2( p_2->x, p_2->y );

	double len0 = length( p1 - p0 );
	double len1 = length( p2 - p1 );
	double len2 = length( p0 - p2 );

	//s = .5 * (a + b + c)
	//A = sqrt( s(s - a)(s - b)(s - c) )

	double s = .5 * ( len0 + len1 + len2 );
	double A = sqrt( s * ( s - len0 ) * ( s - len1 ) * ( s - len2 ) );

	return A;
}

struct PlantInfo
{
	PlantInfo( Edge*e, double q, double w )
		:edge( e), quant( q), quadWidth(w)
	{
	}
	Edge *edge;
	double quant;
	double quadWidth;
};

sf::VertexArray * GameSession::SetupPlants( Edge *startEdge, Tileset *ts )//, int (*ValidEdge)(sf::Vector2<double> &) )
{
	list<PlantInfo> info;

	int tw = 32;
	int th = 32;

	Edge *te = startEdge;
	do
	{
		//V2d eNorm = te->Normal();
		//int valid = ValidEdge( eNorm );
		int valid = 0;
		if( valid != -1 )//eNorm.x == 0 )
		{
			double len = length( te->v1 - te->v0 );
			int numQuads = len / tw;
			double quadWidth = len / numQuads;
				
			if( numQuads > 0 )
			{
				for(int i = 0; i < numQuads; ++i )
				{
					int r = rand() % 2;
					if( r == 0 )
					{
						info.push_back( PlantInfo( te, quadWidth * i, quadWidth ) );
					}
				}
			}
		}
		te = te->edge1;
	}
	while( te != startEdge );

	int infoSize = info.size();
	int vaSize = infoSize * 4;

	if( infoSize == 0 )
	{
		return NULL;
	}

	//cout << "number of plants: " << infoSize << endl;
	VertexArray *va = new VertexArray( sf::Quads, vaSize );

	int vaIndex = 0;
	for( list<PlantInfo>::iterator it = info.begin(); it != info.end(); ++it )
	{
		V2d groundPoint = (*it).edge->GetPoint( (*it).quant );
		V2d norm = (*it).edge->Normal();
		double w = (*it).quadWidth;
		V2d along = normalize( (*it).edge->v1 - (*it).edge->v0 );
		//V2d other( along.y, -along.x );
		
		V2d groundLeft = groundPoint;
		V2d groundRight = groundPoint + w * along;
		V2d airLeft = groundPoint + norm * (double)th;
		V2d airRight = groundPoint + w * along + norm * (double)th;

		EnvPlant * ep = new EnvPlant( groundLeft,airLeft,airRight,groundRight, vaIndex, va, ts );

		envPlantTree->Insert( ep );

		vaIndex += 4;
	}

	return va;
	/*for( int i = 0; i < infoSize; ++i )
	{

	}*/
}

VertexArray * GameSession::SetupBorderQuads( int bgLayer, 
	Edge *startEdge, Tileset *ts, int (*ValidEdge)(sf::Vector2<double> & ) )
{
	/*int worldNum = 0;
	if( envType < 1 )
	{
		worldNum = 0;
	}
	else if( envType < 2 )
	{

	}*/
	//int worldNum = (int)envType; //temporary

	rayMode = "border_quads";
	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	assert( qt != NULL );

	int tw = 8;//64;
	//int th = 512;
	int numTotalQuads = 0;
	double test = 0;//32.0;
	Edge *te = startEdge;//edges[currentEdgeIndex];
	do
	{
		V2d eNorm = te->Normal();
		int valid = ValidEdge( eNorm );
		if( valid != -1 )//eNorm.x == 0 )
		{
			double len = length( te->v1 - te->v0 ) + test * 2;
			int numQuads = len / tw;
			//double quadWidth = len / numQuads;
				
			if( numQuads == 0 )
			{
				numQuads = 1;
			}

			numTotalQuads += numQuads;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	VertexArray *currVA = new VertexArray( sf::Quads, numTotalQuads * 4 );
	
	

	VertexArray &va = *currVA;
			
	int extra = 0;
	te = startEdge;
	int varietyCounter = 0;
	
	do
	{
		V2d eNorm = te->Normal();
		int valid = ValidEdge( eNorm );
		if( valid != -1 )
		{
			double len = length( te->v1 - te->v0 ) + test * 2;
			int numQuads = len / tw;
			double quadWidth = len / numQuads;

			if( numQuads == 0 )
			{
				numQuads = 1;
				quadWidth = 8;//std::min( len, 16 );
				//quadWidth =;
			}
			


			V2d along = normalize( te->v1 - te->v0 );
			V2d other( along.y, -along.x );

			double out = 40;//16;
			double in = 256 - out;//; - out;
			

			V2d startInner = te->v0 - along * test - other * in;
			V2d startOuter = te->v0 - along * test + other * out;

			for( int i = 0; i < numQuads; ++i )
			{
				//worldNum * 5
				//int valid = ValidEdge( eNorm );
				//add (worldNum * 5) to realIndex to get the correct borders
				int realIndex = valid * 32 + varietyCounter;
				//cout << "real Index: " << realIndex << ", valid: " << valid << ", variety: " << varietyCounter << endl;
				IntRect sub = ts->GetSubRect( realIndex );
				//cout << "left: " << sub.left << ", top: " << sub.top << 
				//	", w: " << sub.width << ", h: " << sub.height << endl;

				double startAlong = (double)i * quadWidth;
				double endAlong = (double)(i+1) * quadWidth;

				V2d currStartInner = startInner + startAlong * along;
				V2d currStartOuter = startOuter + startAlong * along;
				V2d currEndInner = startInner + endAlong * along;
				V2d currEndOuter = startOuter + endAlong * along;
						
				double realHeight0 = 256;//in;//sub.height;
				double realHeight1 = 256;//in;//sub.height;
				
				double d0 = dot( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );
				double c0 = cross( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );

				double d1 = dot( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );
				double c1 = cross( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );

				//if( d0 <= 0

				rcEdge = NULL;
				rayIgnoreEdge = te;
				rayStart = te->v0 - along * test + ( startAlong ) * along;
				rayEnd = currStartInner;//te->v0 + (double)i * quadWidth * along - other * in;
				RayCast( this, qt->startNode, rayStart, rayEnd );


				//start ray
				if( rcEdge != NULL )
				{
					currStartInner = rcEdge->GetPoint( rcQuantity );
					realHeight0 = length( currStartInner - currStartOuter );
				}

				rcEdge = NULL;
				rayStart = te->v0 - along * test + ( endAlong ) * along;
				rayEnd = currEndInner;
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					realHeight1 = length( currEndInner - currStartOuter );
				}

				
				//RayCast( this, terrainTree, position, V2d( position.x - 100, position.y ) );
				
				//if( i == 0 && d0 <= 0 )
				//{
				//	Edge *prev = te->edge0;
				//	V2d pNorm = prev->Normal();
				//	V2d prevEndInner = prev->v1 - pNorm * in;
				//	V2d prevEndOuter = prev->v1 + pNorm * out;
				//	//V2d prevEndOuter = prev->v1 - 

				//	va[extra + i * 4 + 0].position = Vector2f( ( currStartOuter.x + prevEndOuter.x ) / 2.0, 
				//		( currStartOuter.y + prevEndOuter.y ) / 2.0 );
				//	
				//	va[extra + i * 4 + 3].position = Vector2f( (currStartInner.x + prevEndInner.x) / 2.0, 
				//		( currStartInner.y + prevEndInner.y ) / 2.0  );
				//}
				//else
				{
					va[extra + i * 4 + 0].position = Vector2f( currStartOuter.x, currStartOuter.y );
					va[extra + i * 4 + 3].position = Vector2f( currStartInner.x, currStartInner.y );
				}

				/*if( i == numQuads - 1 && d1 <= 0 )
				{
					Edge *next = te->edge1;
					V2d nNorm = next->Normal();
					V2d nextStartInner = next->v0 - nNorm * in;
					V2d nextStartOuter = next->v0 + nNorm * out;
					va[extra + i * 4 + 2].position = Vector2f( ( currEndInner.x + nextStartInner.x ) / 2, 
						( currEndInner.y + nextStartInner.y ) / 2 );

					va[extra + i * 4 + 1].position = Vector2f( ( currEndOuter.x + nextStartOuter.x ) / 2.0, 
						( currEndOuter.y + nextStartOuter.y ) / 2.0 );

				}
				else*/
				{
					va[extra + i * 4 + 2].position = Vector2f( currEndInner.x, currEndInner.y );
					va[extra + i * 4 + 1].position = Vector2f( currEndOuter.x, currEndOuter.y );
				}

				/*va[extra + i * 4 + 2].position = Vector2f( currEndInner.x, currEndInner.y );
				va[extra + i * 4 + 3].position = Vector2f( currStartInner.x, currStartInner.y );*/
				
				
				va[extra + i * 4 + 0].texCoords = Vector2f( sub.left, sub.top );
				va[extra + i * 4 + 1].texCoords = Vector2f( sub.left + sub.width, sub.top );
				va[extra + i * 4 + 2].texCoords = Vector2f( sub.left + sub.width, sub.top + realHeight1);
				va[extra + i * 4 + 3].texCoords = Vector2f( sub.left, sub.top + realHeight0);

				/*va[extra + i * 4 + 0].color = COLOR_BLUE;
				va[extra + i * 4 + 1].color = COLOR_YELLOW;
				va[extra + i * 4 + 2].color = COLOR_MAGENTA;
				va[extra + i * 4 + 3].color = COLOR_TEAL;
*/
				++varietyCounter;
				if( varietyCounter == 32 )
				{
					varietyCounter = 0;
				}
			}

			extra += numQuads * 4;
		}

		
		te = te->edge1;
	}
	while( te != startEdge );

	
	return currVA;
}

sf::VertexArray * GameSession::SetupEnergyFlow1( int bgLayer, Edge *start, Tileset *ts )
{
	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	assert( qt != NULL );
	//goalPos;
	rayMode = "energy_flow";

	Edge *te = start;

	//step one, get a list of the entrance points
	//check their ray distance and determine how many quads we want total,
		//while also storing start and end vectors for each quad

	//store start point and end point for each line
	cout << "part1  of the algorithm" << endl;
	int numTotalQuads = 0;
	list<pair<V2d,V2d>> rays;
	do
	{
		V2d norm = te->Normal();
		V2d v0 = te->v0;
		V2d v1 = te->v1;
		V2d along = normalize( v1 - v0 );
		double c = cross( goalPos - v0, along );
		double maxRayLength = 1000;
		//eventually make this larger because don't want to do it too close to parallel
		if( c > 0 ) //facing the goal
		{
			double len = length( te->v1 - te->v0 );
			V2d mid = (te->v0 + te->v1) / 2.0;
			double lenMid = length( goalPos - mid );
			double transition = 1000.0;
			double spacing = 8 + 8 * ( lenMid / transition );
			double width = 8;

			//double fullSpace = spacing + width;

			//int numStartPoints = len / fullSpace;
			//double sideSpacing = ( len - numStartPoints * fullSpace ) / 2;

			//for( int i = 0; i < numStartPoints; ++i )
			double quant = 4; //start point
			int quads = 0;
			while( quant < len - width )
			{
				double rayLength = 0;
				V2d rayPoint;

				rcEdge = NULL;
				rayStart = v0 + along * quant;
				rayIgnoreEdge = te;
				V2d goalDir = normalize( rayStart - goalPos );
				//cout << "goal dir: " << goalDir.x << ", " << goalDir.y << ", rayStart: " << rayStart.x << ", " << rayStart.y << 
				//	", gaolPos: " << goalPos.x << ", " << goalPos.y << endl;
				rayEnd = rayStart + goalDir * maxRayLength;//rayStart - norm * maxRayLength;
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//start ray
				if( rcEdge != NULL )
				{
					rayPoint = rcEdge->GetPoint( rcQuantity );
					
					rays.push_back( pair<V2d,V2d>(rayStart, rayPoint) );
					
					
					//currStartInner = rcEdge->GetPoint( rcQuantity );
					//realHeight0 = length( currStartInner - currStartOuter );
				}
				else
				{
					rays.push_back( pair<V2d,V2d>(rayStart, rayEnd) );
				}

				//rays.push_back(  pair<V2d,V2d>(rayStart, rayEnd) );

				quant += spacing + width;
				++quads;
			}

			numTotalQuads += quads;
			//numTotalQuads += numQuads;	
			
		}
		te = te->edge1;
	}
	while( te != start );

	if( numTotalQuads == 0 )
	{
		return NULL;
	}

	cout << "part2  of the algorithm: " << numTotalQuads << endl;
	VertexArray *VA = new VertexArray( sf::Quads, numTotalQuads * 4 );

	VertexArray &va = *VA;

	int index = 0;
	for( list<pair<V2d,V2d>>::iterator it = rays.begin(); it != rays.end(); ++it )
	{
		
		V2d start = (*it).first;
		V2d end = (*it).second;

		V2d along = normalize( end - start );
		V2d other( along.y, -along.x );


		double width = 8;
		V2d startLeft = start; //- other * width / 2.0;
		V2d startRight = start + other * width;//width / 2.0;
		V2d endLeft = end;// - other * width / 2.0;
		V2d endRight = end + other * width;// * width / 2.0;

		

		va[index*4+0].color = Color::Red;
		va[index*4+1].color = Color::Red;
		va[index*4+2].color = Color::Red;
		va[index*4+3].color = Color::Red;

		va[index*4+0].position = Vector2f( startLeft.x, startLeft.y );
		va[index*4+1].position = Vector2f( startRight.x, startRight.y );
		va[index*4+2].position = Vector2f( endRight.x, endRight.y );
		va[index*4+3].position = Vector2f( endLeft.x, endLeft.y );


		//va[index*4+3].position = //Color::Red;

		index++;
	}

	return VA;

	//int extra = 0;
	//do
	//{
	//	V2d norm = te->Normal();
	//	V2d v0 = te->v0;
	//	V2d v1 = te->v1;
	//	V2d along = normalize( v1 - v0 );
	//	double c = cross( goalPos - v0, along );
	//	//eventually make this larger because don't want to do it too close to parallel
	//	if( c > 0 ) //facing the goal
	//	{
	//		double len = length( te->v1 - te->v0 );
	//		double spacing = 8;

	//		int numQuads = len / spacing;

	//		double sideSpacing = ( len - numQuads * spacing ) / 2;

	//		va[extra + i * 4 + 0].color = Color::Red;
	//		va[extra + i * 4 + 1].color = Color::Red;
	//		va[extra + i * 4 + 2].color = Color::Red;
	//		va[extra + i * 4 + 3].color = Color::Red;

	//		for( int i = 0; i < numQuads; ++i )
	//		{
	//			//va[extra + i * 4 + 0].po
	//		}

	//					

	//	}

	//	extra += numQuads * 4;
	//	te = te->edge1;
	//}
	//while( te != start );
}

typedef pair<V2d, V2d> pairV2d;
sf::VertexArray * GameSession::SetupEnergyFlow()
{
	int bgLayer = 0;
	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	assert( qt != NULL );
	//goalPos;
	rayMode = "energy_flow";
	//rayIgnoreEdge1 = NULL;
	//rayIgnoreEdge = NULL;

	double angle = 0;
	double divs = 64;
	double moveAngle = (2 * PI) / divs;
	double tau = 2 * PI;
	double startRadius = 50;
	bool insideTerrain = false;
	bool knowInside = false;
	double rayLen = 100;
	double width = 16;

	list<list<pair<V2d,bool>>> allInfo;
	//cout << "number of divs: " << divs << endl;
	for( int i = 0; i < divs; ++i )//while( angle <= PI * 2 )
	//int i = 3;
	//int i = 23;
	{
		rayIgnoreEdge1 = NULL;
		rayIgnoreEdge = NULL;

		allInfo.push_back( list<pair<V2d,bool>>() );
		list<pair<V2d,bool>> &pointList = allInfo.back();
		//cout << "div " << i << endl;
		double angle = (tau / divs) * i;
		V2d rayDir( cos( angle ), sin( angle ) );

		rayStart = goalPos + rayDir * startRadius;
		rayEnd = rayStart + rayDir * rayLen;
		//rayIgnoreEdge->
		//while( rcEdge 
		bool rayOkay = rayEnd.x >= leftBounds && rayEnd.y >= topBounds && rayEnd.x <= leftBounds + boundsWidth 
			&& rayEnd.y <= topBounds + boundsHeight;
		
		
		Edge *cEdge = NULL;
		//list<pair<V2d, bool>> pointList; //if true, then its facing the ray

		
		while( rayOkay )
		{
			//cout << "ray start: " << rayStart.x << ", " << rayStart.y << endl;
			rcEdge = NULL;

			RayCast( this, qt->startNode, rayStart, rayEnd );
			//rayStart = v0 + along * quant;
			//rayIgnoreEdge = te;
			//V2d goalDir = normalize( rayStart - goalPos );
			//rayEnd = rayStart + goalDir * maxRayLength;//rayStart - norm * maxRayLength;
			
			
			//start ray
			if( rcEdge != NULL )
			{
				
				if( rcEdge->edgeType == Edge::BORDER )
				{
				//	cout << "secret break" << endl;
					break;
				}
				
				rayIgnoreEdge1 = rayIgnoreEdge;
				rayIgnoreEdge = rcEdge;

				V2d rn = rcEdge->Normal();
				double d = dot( rn, rayDir );
				V2d hitPoint = rcEdge->GetPoint( rcQuantity );
				if( d > 0 )
				{
					if( pointList.size() > 0 && pointList.back().second == false )
					{
						//cout << "failing here: " << i << endl;
						assert( 0 );
					}
					else
					{
						
						pointList.push_back( pair<V2d,bool>( hitPoint, false ) ); //not facing the ray, so im inside
						//cout << "adding false: " << hitPoint.x << ", " << hitPoint.y << "    " << pointList.size() << endl;
					}
				}
				else if( d < 0 )
				{
					if( pointList.size() > 0 && pointList.back().second == true)
					{
						//cout << "failing here111 " << i << endl;
						assert( 0 );
					}
					else
					{
						
						pointList.push_back( pair<V2d,bool>( hitPoint, true ) ); // facing the ray, so im outside
						//cout << "adding true: " << hitPoint.x << ", " << hitPoint.y << "    " << pointList.size() << endl;
					}
				}
				else
				{

				}
				//rayPoint = rcEdge->GetPoint( rcQuantity );	
				//rays.push_back( pair<V2d,V2d>(rayStart, rayPoint) );
				rayStart = hitPoint;
				rayEnd = hitPoint + rayDir * rayLen;

				
					
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			
			}
			else
			{
				rayStart = rayEnd;
				rayEnd = rayStart + rayDir * rayLen;
			}

			rayOkay = length( (goalPos + rayDir * startRadius) - rayEnd ) <= 10000;
			//rayOkay = rayEnd.x >= leftBounds && rayEnd.y >= topBounds && rayEnd.x <= leftBounds + boundsWidth 
			//	&& rayEnd.y <= topBounds + boundsHeight;
		}

		if( pointList.size() > 0 )
		{
			if( pointList.front().second == false )
			{
			//	cout << "adding to front!" << endl;
				//pointList.pop_front();
				pointList.push_front( pair<V2d,bool>( goalPos + rayDir * startRadius, true ) );
			}
			if( pointList.back().second == true )
			{
				//pointList.pop_back();
			//	cout << "popping from back!" << endl;
				pointList.push_back( pair<V2d,bool>( rayEnd, false ) );
			}
		}

		assert( pointList.size() % 2 == 0 );

		//true then false

		//always an even number of them
	}

	if( allInfo.empty() )
	{
		return NULL;
	}

	int totalPoints = 0;
	for(list<list<pair<V2d,bool>>>::iterator it = allInfo.begin(); it != allInfo.end(); ++it )
	{
		list<pair<V2d,bool>> &pointList = (*it);
		totalPoints += pointList.size();
	}

	cout << "number of quads: " << totalPoints / 2 << endl;
	VertexArray *VA = new VertexArray( sf::Quads, (totalPoints / 2) * 4 );
	VertexArray &va = *VA;
	int extra = 0;
	for(list<list<pair<V2d,bool>>>::iterator it2 = allInfo.begin(); it2 != allInfo.end(); ++it2 )
	{
		list<pair<V2d,bool>> &pointList = (*it2);
		for(list<pair<V2d,bool>>::iterator it = pointList.begin(); it != pointList.end(); ++it )
		{
			V2d startPoint = (*it).first;
			++it;
			V2d endPoint = (*it).first;

			V2d along = normalize( endPoint - startPoint );
			V2d other( along.y, -along.x );

			V2d startLeft = startPoint - other * width / 2.0 + along * 16.0;
			V2d startRight = startPoint + other * width / 2.0 + along * 16.0;
			V2d endLeft = endPoint - other * width / 2.0 - along * 16.0;
			V2d endRight = endPoint + other * width / 2.0 - along * 16.0;

			va[extra + 0].color = Color::Red;
			va[extra + 1].color = Color::Red;
			va[extra + 2].color = Color::Red;
			va[extra + 3].color = Color::Red;

			va[extra + 0].position = Vector2f( startLeft.x, startLeft.y );
			va[extra + 1].position = Vector2f( startRight.x, startRight.y );
			va[extra + 2].position = Vector2f( endRight.x, endRight.y );
			va[extra + 3].position = Vector2f( endLeft.x, endLeft.y );

			extra += 4;
		}
	}

	return VA;
}

sf::VertexArray * GameSession::SetupBorderTris( int bgLayer, Edge *startEdge, Tileset *ts )
{

	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	assert( qt != NULL );

	int tw = 32;
	int th = 128;
	//double an = th * PI / 6;
	double an = PI / 12.0;
	
	int numTotalTris = 0;
	Edge *te = startEdge;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		double c = cross( nextAlong, along );
		if( c > 0 )
		{
			V2d endVec = normalize( te->v0 - te->v1 );
			V2d startVec = normalize( e1->v1 - te->v1 );

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

			/*double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;*/

			if( endAngle > startAngle )
			{
				startAngle += 2 * PI;
			}

			

			double angleDiff = (startAngle - endAngle);

			int numTris = angleDiff / an;
			//go counter clockwise


			
			
				
			/*if( numTris == 0 )
			{
				numTris = 1;
			}*/

			numTotalTris += numTris;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	if( numTotalTris == 0 )
	{
		//cout << "no tris here" << endl;
		return NULL;
	}
	else
	{
		//cout << "numtotaltris: " << numTotalTris << endl;
	}

	VertexArray *currVA = new VertexArray( sf::Quads, numTotalTris * 4 );
	

	IntRect sub = ts->GetSubRect( 0 );

	VertexArray &va = *currVA;

	int extra = 0;
	te = startEdge;
	int varietyCounter = 0;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		double c = cross( nextAlong, along );
		if( c > 0 )
		{
			V2d endVec = normalize( te->v0 - te->v1 );
			endVec = -V2d( endVec.y, -endVec.x );
			endVec = -te->Normal();
			V2d startVec = normalize( e1->v1 - te->v1 );
			startVec = -V2d( startVec.y, -startVec.x );
			startVec = -e1->Normal();

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

			/*double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;*/

			if( endAngle > startAngle )
			{
				startAngle += 2 * PI;
			}

			double angleDiff = (startAngle - endAngle);
			int numTris = angleDiff / an;
			//cout << "angleDiff: " << angleDiff << endl;
			
			
			//double triWidth = (startAngle - endAngle) / numTris;
			
			V2d centerPos( te->v1.x, te->v1.y );
			for( int i = 0; i < numTris; ++i )
			{
				double currAngle = startAngle - (double)i/numTris * angleDiff;
				double nextAngle = startAngle - (double)(i+1)/numTris * angleDiff;
				V2d currVec = V2d( cos( currAngle), -sin( currAngle ) );

				V2d currNorm( currVec.y, -currVec.x );

				V2d nextVec = V2d( cos( nextAngle ), -sin( nextAngle ) );

				//length( //length( inside - insideNext);

				V2d inside = centerPos + currVec * 48.0 - currNorm * (double)tw / 2.0;
				V2d insideNext = centerPos + nextVec * 48.0 + currNorm * (double)tw / 2.0;
				
				
				V2d outLeft = centerPos - currNorm * (double)tw / 2.0 - currVec * 16.0;
				V2d outRight = centerPos + currNorm * (double)tw / 2.0 - currVec * 16.0;

				//va[extra + i*3 + 0].position = Vector2f( centerPos.x, centerPos.y );
				va[extra + i*4 + 0].position = Vector2f( outLeft.x, outLeft.y );
				va[extra + i*4 + 1].position = Vector2f( outRight.x, outRight.y );
				va[extra + i*4 + 2].position = Vector2f( insideNext.x, insideNext.y );
				va[extra + i*4 + 3].position = Vector2f( inside.x, inside.y );

				//int blah = 20;
				//va[extra + i*3 + 0].color = Color( i * 20, i * 20, i * 20 );//Color::Red;
				//va[extra + i*3 + 1].color = Color( i * 20, i * 20, i * 20 );
				//va[extra + i*3 + 2].color = Color( i * 20, i * 20, i * 20 );

				va[extra + i*4 + 0].texCoords = Vector2f( sub.left, sub.top );
				va[extra + i*4 + 1].texCoords = Vector2f( sub.left + tw, sub.top );
				va[extra + i*4 + 2].texCoords = Vector2f( sub.left + tw, sub.top + sub.height );
				va[extra + i*4 + 3].texCoords = Vector2f( sub.left, sub.top + sub.height );
			}

			extra += numTris * 4;
		}
	}
	while( te != startEdge );

	return currVA;
}

sf::VertexArray * GameSession::SetupTransitions( int bgLayer, Edge *startEdge, Tileset *ts )
{
	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	double tw = 256;//64;//8;
	double th = 256;

	int out = 40;
	int in = th - out;
	assert( qt != NULL );


	int numtotalTris = 0;
	Edge *te = startEdge;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		V2d nextNorm = e1->Normal();
		double c = cross( nextAlong, along );
		bool viable = true;

		V2d jutDir = normalize( normalize( te->v1 - te->v0 ) + normalize( te->v1 - e1->v1 ) ) / 2.0;
		V2d jutPoint = te->v1 + jutDir * (th - out );

		//double rayTest = (th - out);

		rcEdge = NULL;
		rayIgnoreEdge = NULL;
		rayStart = te->v1 + jutDir * 1.0;
		rayEnd = jutPoint;//te->v0 + (double)i * quadWidth * along - other * in;
		RayCast( this, qt->startNode, rayStart, rayEnd );

		//start ray
		if( rcEdge != NULL )
		{
			//cout << "viable is now false" << endl;
			viable = false;
		}
		else
		{
			//rcEdge = NULL;
			//rayIgnoreEdge = te;
			//rayIgnoreEdge = NULL;
			rayStart = te->v1 - eNorm * .01;
			rayEnd = te->v1 - eNorm * (th-out);//te->v0 + (double)i * quadWidth * along - other * in;
			RayCast( this, qt->startNode, rayStart, rayEnd );


			//start ray
			if( rcEdge != NULL )
			{
				viable = false;
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			}
			else
			{
				//rayIgnoreEdge = te;
				//rayIgnoreEdge = NULL;
				rayStart = te->v1 - nextNorm * .01;
				rayEnd = te->v1 - nextNorm * (th-out);
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					viable = false;
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
				}
			}
		}


		if( c > 0 && viable )
		{
			numtotalTris+=2;
			//V2d endVec = normalize( te->v0 - te->v1 );
			//V2d startVec = normalize( e1->v1 - te->v1 );

			//double startAngle = atan2( -startVec.y, startVec.x );
			//if( startAngle < 0 )
			//{
			//	startAngle += 2 * PI;
			//}
			//double endAngle = atan2( -endVec.y, endVec.x );
			//if( endAngle < 0 )
			//{
			//	endAngle += 2 * PI;
			//}

			///*double temp = startAngle;
			//startAngle = endAngle;
			//endAngle = temp;*/

			//if( endAngle > startAngle )
			//{
			//	startAngle += 2 * PI;
			//}

			//

			//double angleDiff = (startAngle - endAngle);

			//int numTris = angleDiff / an;
			////go counter clockwise


			//
			//
			//	
			///*if( numTris == 0 )
			//{
			//	numTris = 1;
			//}*/

			//numTotalTris += numTris;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	if( numtotalTris == 0 )
	{
		return NULL;
	}

	VertexArray *currVA = new VertexArray( sf::Triangles, numtotalTris * 3 );
	VertexArray &va = *currVA;

	//..IntRect sub = ts->GetSubRect( 0 );

	te = startEdge;
	int extra = 0;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		double c = cross( nextAlong, along );
		V2d nextNorm = e1->Normal();
		V2d point = e1->v0;
		int i = 0;
		V2d jutDir = normalize( ( normalize( te->v1 - te->v0 ) + normalize( te->v1 - e1->v1 ) ) / 2.0 );
		//cout << "length jut dir: " << length( jutDir ) << endl;
		V2d jutPoint = te->v1 + jutDir * (th - out );
		bool viable = true;

		rcEdge = NULL;
		rayIgnoreEdge = NULL;
		rayStart = te->v1 + jutDir * 1.0;
		rayEnd = jutPoint;//te->v0 + (double)i * quadWidth * along - other * in;
		RayCast( this, qt->startNode, rayStart, rayEnd );

		//start ray
		if( rcEdge != NULL )
		{
			//cout << "viable is now false" << endl;
			viable = false;
		}
		else
		{
			rcEdge = NULL;
			rayIgnoreEdge = te;
			rayStart = te->v1 - eNorm * .01;
			rayEnd = te->v1 - eNorm * (th-out);//te->v0 + (double)i * quadWidth * along - other * in;
			RayCast( this, qt->startNode, rayStart, rayEnd );


			//start ray
			if( rcEdge != NULL )
			{
				viable = false;
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			}
			else
			{
				rayIgnoreEdge = te;
				rayStart = te->v1 - nextNorm * .01;
				rayEnd = te->v1 - nextNorm * (th-out);
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					viable = false;
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
				}
			}
		}


		if( c > 0 && viable )
		{
			/*GameSession::IsFlatGround( sf::Vector2<double> &normal )
			GameSession::IsSlopedGround( sf::Vector2<double> &normal )
			GameSession::IsWall( sf::Vector2<double> &normal )*/

			
			int valid = -1;
			valid = IsFlatGround( eNorm );
			if( valid == -1 )
			{
				valid = IsSlopedGround( eNorm );
				if( valid == -1 )
				{
					valid = IsWall( eNorm );

					if( valid == -1 )
					{
						valid = IsSteepGround( eNorm );
					}
				}

			}

			if( valid == -1 )
			{
				cout << "wat: " << eNorm.x << ", " << eNorm.y << endl;
			}
			assert( valid != -1 );

			int otherValid = -1;
			otherValid = IsFlatGround( nextNorm );
			if( otherValid == -1 )
			{
				otherValid = IsSlopedGround( nextNorm );
				if( otherValid == -1 )
				{
					otherValid = IsWall( nextNorm );

					if( otherValid == -1 )
					{
						otherValid = IsSteepGround( nextNorm );
					}
				}
			}

			if( otherValid == -1 )
			{
				cout << "watother: " << eNorm.x << ", " << eNorm.y << endl;
			}
			assert( otherValid != -1 );
			
			
			//int valid = ValidEdge( eNorm );
				//add (worldNum * 5) to realIndex to get the correct borders
			

			int realIndex = valid * 32;
			int realOther = otherValid * 32;

			//cout << "valid: " << valid << ", otherValid: " << otherValid << endl;
			//cout << "norm: " << eNorm.x << ", " << eNorm.y << endl;
			//cout << "nextNorm: " << nextNorm.x << ", " << nextNorm.y << endl;
			IntRect sub = ts->GetSubRect( realIndex );
			IntRect subOther = ts->GetSubRect( realOther );

			

			V2d currNormOpp = -te->Normal();
			V2d nextNormOpp = -e1->Normal();

			V2d currInPoint = point + currNormOpp * (th - out);
			V2d nextInPoint = point + nextNormOpp * (th - out);

			double baseLength = length( jutPoint - currInPoint );
			int mid = floor( baseLength + .5 );
			//cout << "mid: " << mid << endl;
			mid = min( mid, (int)tw );
			//assert( mid <= 128 );
			//cout << "mid: " << mid << endl;
			//cout << "jut length: " << length( jutPoint - point ) << endl;

			va[extra + 0].position = Vector2f( point.x, point.y );
			va[extra + 1].position = Vector2f( currInPoint.x, currInPoint.y );
			va[extra + 2].position = Vector2f( jutPoint.x, jutPoint.y );

			/*va[extra + i*3 + 0].color = Color::Red;
			va[extra + i*3 + 1].color = Color::Green;
			va[extra + i*3 + 2].color = Color::Blue;*/

			va[extra + 0].texCoords = Vector2f( sub.left + mid / 2, sub.top + out);
			va[extra + 1].texCoords = Vector2f( sub.left, th + sub.top );
			va[extra + 2].texCoords = Vector2f( sub.left + mid, sub.top + th );

			extra += 3;

			va[extra + 0].position = Vector2f( point.x, point.y );
			va[extra + 1].position = Vector2f( jutPoint.x, jutPoint.y );
			va[extra + 2].position = Vector2f( nextInPoint.x, nextInPoint.y );

			/*va[extra + i*3 + 0].color = Color::Red;
			va[extra + i*3 + 1].color = Color::Blue;
			va[extra + i*3 + 2].color = Color::Green;*/


			/*va[extra + i*3 + 0].color = Color::Red;
			va[extra + i*3 + 1].color = Color::Red;
			va[extra + i*3 + 2].color = Color::Red;*/

			va[extra + 0].texCoords = Vector2f( subOther.left + mid / 2, subOther.top + out );
			va[extra + 1].texCoords = Vector2f( subOther.left, th + subOther.top );
			va[extra + 2].texCoords = Vector2f( subOther.left + mid, th + subOther.top );

			//va[extra + i*4 + 0].position = Vector2f( outLeft.x, outLeft.y );

			//V2d half = normalize( ( currNorm + nextNorm ) / 2.0 );
			//V2d oppHalf = -half;

			//double outDist = 16.0;
			//double inDist = 128.0 - outDist;//48.0;

			//V2d out = te->v1 + half * outDist;
			//V2d in = te->v1 + oppHalf * inDist;

			//V2d otherHalf( half.y, -half.x );

			//double hw = tw / 2.0;
			//V2d outLeft = out - otherHalf * hw;
			//V2d outRight = out + otherHalf * hw;
			//V2d inLeft = in - otherHalf * hw;
			//V2d inRight = in + otherHalf * hw;

			//va[extra + i*4 + 0].position = Vector2f( outLeft.x, outLeft.y );
			//va[extra + i*4 + 1].position = Vector2f( outRight.x, outRight.y );
			//va[extra + i*4 + 2].position = Vector2f( inRight.x, inRight.y );
			//va[extra + i*4 + 3].position = Vector2f( inLeft.x, inLeft.y );

			////int blah = 20;
			////va[extra + i*3 + 0].color = Color( i * 20, i * 20, i * 20 );//Color::Red;
			////va[extra + i*3 + 1].color = Color( i * 20, i * 20, i * 20 );
			////va[extra + i*3 + 2].color = Color( i * 20, i * 20, i * 20 );

			//va[extra + i*4 + 0].texCoords = Vector2f( sub.left, sub.top );
			//va[extra + i*4 + 1].texCoords = Vector2f( sub.left + tw, sub.top );
			//va[extra + i*4 + 2].texCoords = Vector2f( sub.left + tw, sub.top + sub.height );
			//va[extra + i*4 + 3].texCoords = Vector2f( sub.left, sub.top + sub.height );

			extra += 3;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	return currVA;
}

sf::VertexArray * GameSession::SetupDecor0( std::vector<p2t::Triangle*> &tris, Tileset *ts )
{
	for( vector<p2t::Triangle*>::iterator it = tris.begin(); it != tris.end(); ++it )
	{
		//random point stuff. do this after you get the enemies working
	}
}

sf::VertexArray *GameSession::SetupBushes( int bgLayer, std::vector<p2t::Triangle*> &tris, Tileset *ts )
{
	int numBushes = 0;
	
	int trisSize = tris.size();
	for( int i = 0; i < trisSize; ++i )
	{
		numBushes++;
	}

	VertexArray *va = new VertexArray( sf::Quads, numBushes * 4 );
	VertexArray &VA = *va;

	Vector2f testPos;
	Vector2f p0, p1, p2;
	Vector2f avg;

	IntRect subRect = ts->GetSubRect( 0 );
	for( int i = 0; i < numBushes; ++i )
	{
		p0.x = tris[i]->GetPoint( 0 )->x;
		p0.y = tris[i]->GetPoint( 0 )->y;

		p1.x = tris[i]->GetPoint( 1 )->x;
		p1.y = tris[i]->GetPoint( 1 )->y;

		p2.x = tris[i]->GetPoint( 2 )->x;
		p2.y = tris[i]->GetPoint( 2 )->y;

		avg = ( p0 + p1 + p2 ) / 3.f;

		VA[i*4+0].position = Vector2f( avg.x - subRect.width / 2, avg.y - subRect.height / 2 );
		VA[i*4+1].position = Vector2f( avg.x + subRect.width / 2, avg.y - subRect.height / 2 );
		VA[i*4+2].position = Vector2f( avg.x + subRect.width / 2, avg.y + subRect.height / 2 );
		VA[i*4+3].position = Vector2f( avg.x - subRect.width / 2, avg.y + subRect.height / 2 );

		VA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		VA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		VA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		VA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}
	return va;
}


int GameSession::IsFlatGround( sf::Vector2<double> &normal )
{
	if( normal.x == 0 )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int GameSession::IsSlopedGround( sf::Vector2<double> &normal )
{
	//.4 is the current steepthresh value
	double steepThresh = .4;
	if( abs( normal.y ) > steepThresh  )
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int GameSession::IsSteepGround(  sf::Vector2<double> &normal )
{
	double steepThresh = .4;
	double wallThresh = .9999;
	if( abs( normal.y ) <= steepThresh && abs( normal.x ) < wallThresh )
	{
		if( normal.y > 0 )
		{
			return 3;
		}
		else if( normal.y < 0 )
		{
			return 2;
		}
	}
	else
	{
		return -1;
	}
}

int GameSession::IsWall( sf::Vector2<double> &normal )
{
	double wallThresh = .9999;
	if( abs( normal.x ) >= wallThresh )
	{
		return 4;
	}
	else
	{
		return -1;
	}
}

//save state to enter clone world
void GameSession::SaveState()
{
	stored.activeEnemyList = activeEnemyList;
	cloneInactiveEnemyList = NULL;

	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{
		currEnemy->SaveState();
		currEnemy = currEnemy->next;
	}
}

//reset from clone world
void GameSession::LoadState()
{
	Enemy *test = cloneInactiveEnemyList;
	int listSize = 0;
	while( test != NULL )
	{
		listSize++;
		test = test->next;
	}

	cout << "there are " << listSize << " enemies killed during the last clone process" << endl;


	//enemies killed while in the clone world
	Enemy *deadEnemy = cloneInactiveEnemyList;
	while( deadEnemy != NULL )
	{
		
		Enemy *next = deadEnemy->next;
		if( deadEnemy->spawnedByClone )
		{
			deadEnemy->Reset();
			//cout << "resetting dead enemy: " << deadEnemy << endl;
		}
		else
		{
			deadEnemy->LoadState();
			//cout << "loading dead enemy: " << deadEnemy << endl;
		}
		deadEnemy = next;
	}

	//enemies that are still alive
	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{		
		Enemy *next = currEnemy->next;
		if( currEnemy->spawnedByClone )
		{
			//cout << "resetting enemy: " << currEnemy << endl;
			currEnemy->Reset();
		}
		else
		{
			currEnemy->LoadState();
			//cout << "loading enemy: " << currEnemy << endl;
		}

		currEnemy = next;
	}

	//restore them all to their original state and then reset the list pointer

	//cloneInactiveEnemyList = NULL;
	activeEnemyList = stored.activeEnemyList;
}

void GameSession::Pause( int frames )
{
	pauseFrames = frames;
}

void GameSession::Fade( bool in, int frames, sf::Color c)
{
	if( in )
	{
		fadeLength = frames;
		fadingIn = true;
		fadingOut = false;
	}
	else
	{
		fadeLength = frames;
		fadingIn = false;
		fadingOut = true;
	}
	fadeRect.setFillColor( Color( c.r, c.g, c.b, 255 ) );
	
	fadeFrame = 0;
}

void GameSession::UpdateFade()
{
	if( !fadingIn && !fadingOut )
		return;
	//cout << "fade frame: " << fadeFrame << endl;

	++fadeFrame;

	if( fadeFrame > fadeLength )
	{
		fadingIn = false;
		fadingOut = false;
		return;
	}

	if( fadingIn )
	{
		fadeAlpha = floor( (255.0 - 255.0 * fadeFrame / (double)fadeLength) + .5 );
	}
	else if( fadingOut )
	{
		fadeAlpha = floor( 255.0 * fadeFrame / (double)fadeLength + .5 );
	}

	Color oldColor = fadeRect.getFillColor();
	fadeRect.setFillColor( Color( oldColor.r, oldColor.g, oldColor.b, fadeAlpha ) );
}

void GameSession::DrawFade( sf::RenderTarget *target )
{
	if( fadeAlpha > 0 )
	{
		target->draw( fadeRect );
	}
}

void GameSession::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	if( rayMode == "border_quads" )
	{
		if( rayIgnoreEdge != NULL )
		{
		double d0 = dot(normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ),
			normalize( rayIgnoreEdge->edge0->v1 - rayIgnoreEdge->edge0->v0 ));
		double c0 = cross( normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ),
			normalize( rayIgnoreEdge->edge1->v0 - rayIgnoreEdge->edge0->v0 ) );


		double d1 = dot( normalize( rayIgnoreEdge->edge1->v1 - rayIgnoreEdge->edge1->v0 ), 
			normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ) );
		double c1 = cross( normalize( rayIgnoreEdge->edge1->v1 - rayIgnoreEdge->edge1->v0 ), 
			normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ) );
		if( edge == rayIgnoreEdge->edge1 && ( d1 >= 0  || c1 > 0 ) )
			return;
		if( edge == rayIgnoreEdge->edge0 && ( d0 >= 0 || c0 > 0 ) )
		{
			return;
		}

		if( edge != rayIgnoreEdge && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - rayStart ) < 
			length( rcEdge->GetPoint( rcQuantity ) - rayStart ) ) )
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
		}
		else
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
	}
	else if( rayMode == "energy_flow" )
	{
		if( edge->edgeType == Edge::CLOSED_GATE )
		{
			return;
		}

		if( edge != rayIgnoreEdge && edge != rayIgnoreEdge1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - rayStart ) < 
			length( rcEdge->GetPoint( rcQuantity ) - rayStart ) ) )
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
	}
	//if( rayPortion > 1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - position ) < length( rcEdge->GetPoint( rcQuantity ) - position ) ) )
	
//	{
//		rcEdge = edge;
//		rcQuantity = edgeQuantity;
//	}
}

void GameSession::AllocateEffect()
{
	if( inactiveEffects == NULL )
	{
		inactiveEffects = new BasicEffect( this );
		inactiveEffects->prev = NULL;
		inactiveEffects->next = NULL;
	}
	else
	{
		BasicEffect *b = new BasicEffect( this ) ;
		b->next = inactiveEffects;
		inactiveEffects->prev = b;
		inactiveEffects = b;
	}
}

void GameSession::AllocateLight()
{
	if( inactiveLights == NULL )
	{
		inactiveLights = new Light( this, Vector2i( 0, 0 ), Color( 255, 255, 255, 255 ), 1, 1 );
		inactiveLights->prev = NULL;
		inactiveLights->next = NULL;
	}
	else
	{
		Light *light= new Light( this, Vector2i( 0, 0 ), Color( 255, 255, 255, 255 ), 1, 1 );
		light->next = inactiveLights;
		inactiveLights->prev = light;
		inactiveLights = light;
	}
}

BasicEffect * GameSession::ActivateEffect( EffectLayer layer, Tileset *ts, V2d pos, bool pauseImmune, double angle, int frameCount,
	int animationFactor, bool right )
{
	if( inactiveEffects == NULL )
	{
		return NULL;
	}
	else
	{
		//return NULL;

		BasicEffect *b = inactiveEffects;

		if( inactiveEffects->next == NULL )
		{
			inactiveEffects = NULL;
		}
		else
		{
			inactiveEffects = (BasicEffect*)(inactiveEffects->next);
			inactiveEffects->prev = NULL;
		}

		//assert( ts != NULL );
		b->Init( ts, pos, angle, frameCount, animationFactor, right );
		b->prev = NULL;
		b->next = NULL;
		b->pauseImmune = pauseImmune;
		b->layer = layer;

		AddEffect( layer, b );
		//AddEnemy( b );
		
		//cout << "activating: " << b << " blah: " << b->prev << endl;
		return b;
	}
}

void GameSession::RemoveEffect( EffectLayer layer, Enemy *e )
{
	Enemy *& fxList = effectLists[layer];
	assert( fxList != NULL );
	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if( prev == NULL && next == NULL )
	{
		fxList = NULL;
	}
	else
	{
		if( e == fxList )
		{
			assert( next != NULL );
			
			next->prev = NULL;
			
			fxList = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}

	//if( e->type != e->BASICEFFECT )
	//{
	//	/*if( e->hasMonitor )
	//	{
	//		cout << "adding monitor!" << endl;
	//		e->monitor->position = e->position;
	//		AddEnemy( e->monitor );
	//	}*/

	//	cout << "adding an inactive enemy!" << endl;
	//	//cout << "secret count: " << CountActiveEnemies() << endl;
	//	if( inactiveEnemyList == NULL )
	//	{
	//		inactiveEnemyList = e;
	//		e->next = NULL;
	//		e->prev = NULL;
	//	}
	//	else
	//	{
	//		//cout << "creating more dead clone enemies" << endl;
	//		e->next = inactiveEnemyList;
	//		inactiveEnemyList->prev = e;
	//		inactiveEnemyList = e;
	//	}
	//}

	//might need to give enemies a second next/prev pair for clone power?
	//totally does >.> CLONE POWER
	if( player->record > 0 )
	{
		if( cloneInactiveEnemyList == NULL )
		{
			cloneInactiveEnemyList = e;
			e->next = NULL;
			e->prev = NULL;
			//cout << "creating first dead clone enemy" << endl;

			/*int listSize = 0;
			Enemy *ba = cloneInactiveEnemyList;
			while( ba != NULL )
			{
				listSize++;
				ba = ba->next;
			}

			cout << "size of dead list after first add: " << listSize << endl;*/
		}
		else
		{
			//cout << "creating more dead clone enemies" << endl;
			e->next = cloneInactiveEnemyList;
			cloneInactiveEnemyList->prev = e;
			cloneInactiveEnemyList = e;
		}
	}
}

void GameSession::DeactivateEffect( BasicEffect *b )
{
	//cout << "deactivate " << b << endl;
	RemoveEffect( b->layer, b );
	//RemoveEnemy( b );

	if( player->record == 0 )
	{
		if( inactiveEffects == NULL )
		{
			inactiveEffects = b;
			b->next = NULL;
			b->prev = NULL;
		}
		else
		{
			b->next = inactiveEffects;
			inactiveEffects->prev = b;
			inactiveEffects = b;
		}
	}
}

void GameSession::ResetEnemies()
{
	rResetEnemies( enemyTree->startNode );

	/*Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		Enemy *next = curr->next;
		if( curr->type == Enemy::BASICEFFECT )
		{
			DeactivateEffect( (BasicEffect*)curr );
		}
		curr = next;
	}
	activeEnemyList = NULL;*/
	activeEnemyList = NULL;
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		Enemy *curr = effectLists[i];
		while( curr != NULL )
		{
			Enemy *next = curr->next;
			assert( curr->type == Enemy::BASICEFFECT );
			DeactivateEffect( (BasicEffect*)curr );

			curr = next;
		}
		effectLists[i] = NULL;
		
	}


	if( b_bird != NULL ) b_bird->Reset();

	if( b_crawler != NULL ) b_crawler->Reset();

	if( b_coyote != NULL ) b_coyote->Reset();
}

void GameSession::ResetPlants()
{
	rResetPlants( envPlantTree->startNode );

	activeEnvPlants = NULL;
}

void GameSession::rResetPlants( QNode *node )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		for( int i = 0; i < n->objCount; ++i )
		{			
			EnvPlant *ev = (EnvPlant*)(n->entrants[i]);
			//cout << "reset1" << endl;
			ev->Reset();
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		ParentNode *n = (ParentNode*)node;
		for( int i = 0; i < 4; ++i )
		{
			rResetPlants( n->children[i] );
		}

		for( list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it )
		{
			EnvPlant *ev = (EnvPlant *)(*it);
			//cout << "reset2" << endl;
			ev->Reset();
		}
		
	}
}

void GameSession::ResetInactiveEnemies()
{
	Enemy *e = inactiveEnemyList;

	e = inactiveEnemyList;
	while( e != NULL )
	{
		//cout << "reset inactive enemy" << endl;
		Enemy *temp = e->next;

		e->Reset();
		e = temp;
	}

	inactiveEnemyList = NULL;
}

void GameSession::rResetEnemies( QNode *node )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		//cout << "\t\tstarting leaf reset: " << endl;
		for( int i = 0; i < n->objCount; ++i )
		{			
			//cout << "\t\t\tresetting enemy " << i << endl;
			Enemy * e = (Enemy*)(n->entrants[i]);
			e->Reset();
			//cout << e->type << endl;
			
			//((Enemy*)node)->Reset();		
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		ParentNode *n = (ParentNode*)node;
		//cout << "start parent reset: " << endl;
		for( int i = 0; i < 4; ++i )
		{
		//	cout << "\tresetting child: " << i << endl;
			rResetEnemies( n->children[i] );
		}

		for( list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it )
		{
			Enemy * e = (Enemy*)(*it);
			e->Reset();
		}
		
	}
}

void GameSession::LevelSpecifics()
{
	if( fileName == "test3" )
	{
		//startSeq = new GameStartSeq( this );
		//activeSequence = startSeq;
		//GameStartMovie();
		//cout << "doing stuff here" << endl;
	}
	else
	{
	//	player->velocity.x = 60;
	}
}

Light * GameSession::ActivateLight( int radius,  int brightness, const Color color )
{
	if( inactiveLights == NULL )
	{
		return NULL;
	}
	else
	{
		Light *l = inactiveLights;

		if( inactiveEffects->next == NULL )
		{
			inactiveEffects = NULL;
		}
		else
		{
			inactiveLights = (Light*)(inactiveLights->next);
			inactiveLights->prev = NULL;
		}

		//assert( ts != NULL );
		l->next = NULL;
		
		if( activeLights != NULL )
		{
			activeLights->prev = l;
			l->next = activeLights;
			activeLights = l;
		}
		else
		{
			activeLights = l;
		}

		l->radius = radius;
		l->brightness = brightness;
		l->color = color;
		//cout << "activating: " << b << " blah: " << b->prev << endl;
		return l;
	}
}

void GameSession::DeactivateLight( Light *light )
{
	Light *prev = light->prev;
	Light *next = light->next;

	if( prev == NULL && next == NULL )
	{
		activeLights = NULL;
	}
	else
	{
		if( light == activeLights )
		{
			next->prev = NULL;
			activeLights = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}


	if( inactiveLights == NULL )
	{
		inactiveLights = light;
		light->next = NULL;
	}
	else
	{
		light->next = inactiveLights;
		inactiveLights->prev = light;
		inactiveLights = light;
	}	
}

PowerBar::PowerBar()
{
	pointsPerDot = 2;
	dotsPerLine = 6;
	dotWidth = 9;
	dotHeight = 9;
	linesPerBar = 60;

	pointsPerLayer = 2 * 6 * 60;//3 * 6 * 60//240 * 10;
	maxLayer = 1;//6;
	points = pointsPerLayer;//pointsPerLayer * ( maxLayer + 1 );
	layer = maxLayer;//maxLayer;
	
	minUse = 1;
	
	//panelTex.loadFromFile( "lifebar.png" );
	panelTex.loadFromFile( "powerbarmockup.png" );
	panelSprite.setTexture( panelTex );
	//panelSprite.setScale( 10, 10 );
	panelSprite.setPosition( 0, 280 );
	panelSprite.setColor( Color( 255, 255, 255, 150 ) );

	//powerRect.setPosition( 42, 108 );
	//powerRect.setSize( sf::Vector2f( 4 * 4, 59 * 4 ) );
	//powerRect.setFillColor( Color::Green );
	//powerRect.

	maxRecover = 75;
	maxRecoverLayer = 0;
}

void PowerBar::Reset()
{
	points = pointsPerLayer;
	layer = maxLayer;
}

void PowerBar::Draw( sf::RenderTarget *target )
{
	int fullLines = points / pointsPerDot / dotsPerLine;
	int partial = points % ( dotsPerLine * pointsPerDot );//pointsPerLayer - fullLines * pointsPerDot * dotsPerLine;
	//cout << "fullLines: " << fullLines << endl;
	//cout << "partial: " << partial << endl;
	//Color c;
	/*switch( layer )
	{
	case 0:
		c = Color( 0, 0xee, 0xff );
		//c = Color( 0x00eeff );
		break;
	case 1:
		//c = Color( 0x0066cc );
		c = Color( 0, 0x66, 0xcc );
		break;
	case 2:
		c = Color( 0, 0xcc, 0x44 );
		break;
	case 3:
		c = Color( 0xff, 0xf0, 0 );
		break;
	case 4:
		c = Color( 0xff, 0xbb, 0 );
		break;
	case 5:
		c = Color( 0xff, 0x22, 0 );
		break;
	case 6:
		c = Color( 0xff, 0, 0xff );
		break;
	case 7:
		c = Color( 0xff, 0xff, 0xff );
		break;
	}*/
	//c = Color( 0, 0xee, 0xff );


	sf::RectangleShape rs;


	//primary portion
	rs.setPosition( 4, 180 + 600 );
	rs.setFillColor( COLOR_TEAL );
	rs.setSize( Vector2f( 60, -fullLines * dotHeight ) );
	target->draw( rs );

	//secondary portion
	rs.setPosition( rs.getGlobalBounds().left, rs.getGlobalBounds().top - dotHeight );
	rs.setSize( Vector2f( (partial / pointsPerDot) * dotWidth, dotHeight ) );
	target->draw( rs );

	//tertiary portion
	int singleDot = partial % pointsPerDot;
	if( singleDot == 1 )
	{
		rs.setPosition( rs.getGlobalBounds().left + rs.getLocalBounds().width,
			rs.getPosition().y );
		rs.setSize( Vector2f( dotWidth, dotHeight ) );
		rs.setFillColor( COLOR_BLUE );
		target->draw( rs );
	}

	//draw full tanks
	int tankWidth = 32;
	int tankHeight = 32;
	int tankSpacing = 20;
	rs.setFillColor( COLOR_TEAL );
	rs.setSize( Vector2f( tankWidth, tankHeight ) );
	rs.setPosition( 0, 180 + 600 - tankHeight );
	for( int i = 0; i < layer; ++i )
	{
		//target->draw( rs );
		//rs.setPosition( rs.getPosition().x, rs.getPosition().y - tankHeight - tankSpacing );
	}

	//only need this until I get a background

	//rs.setFillColor( Color( 100, 100, 100 ) );
	//target->draw( rs );

	/*rs.setFillColor( COLOR_TEAL );
	//in progress tank
	int off = ceil( (points / (double)pointsPerLayer) * 32.0);
	//cout << "off: " << off << endl;
	rs.setPosition( rs.getPosition().x, rs.getPosition().y + tankHeight );
	rs.setSize( Vector2f( tankWidth, -off ) );
	target->draw( rs );*/
	

	//0x99a9b9
	
	//cout << "points: " << points << endl;
	double diffz = (double)points / (double)pointsPerLayer;
	assert( diffz <= 1 );
	diffz = 1 - diffz;
	diffz *= 60 * 4;

	
	//rs.setPosition( 42, 108 + diffz );
	//rs.setPosition( 0, 108 + diffz );
	//rs.setSize( sf::Vector2f( 4 * 4, 60 * 4 - diffz ) );
	//rs.setPosition( 0, 200 );
	//rs.setSize( Vector2f( 100, 500 ) );
	

	

	//target->draw( panelSprite );
	//target->draw( rs );
}

bool PowerBar::Damage( int power )
{
	//cout << "points: " << points << ", power: " << power << endl;
	
	//cout << "newpoints: " << points << endl;
	while( power > 0 )
	{
		points -= power;
		if( points <= 0 )
		{
			power = -points;
			if( layer > 0 )
			{
				layer--;
				points = pointsPerLayer;
				//cout << "layer switch: " << points << endl;
			}
			else
			{
				points = 0;
				//cout << "DONE DEAD: " << points << endl;
				return false;
			}
		}
		else
		{
			break;
		}
	}

	return true;
}

bool PowerBar::Use( int power )
{
	if( layer == 0 )
	{
		if( points - power <= 0 )
		{
			return false;
		}
		else
		{
			points -= power;
		}
	}
	else
	{
		points -= power;
		if( points <= 0 )
		{
			points = pointsPerLayer + points;
			layer--;
		}
	}
	return true;
	/*if( layer == 0 )
	{
		if( points - power < minUse )
		{
			return false;
		}
		else
		{
			points -= power;
		}
	}
	else
	{
		points -= power;
		if( points <= 0 )
		{
			points = pointsPerLayer + points;
			layer--;
		}
	}
	return true;*/
}

void PowerBar::Recover( int power )
{
	if( layer == maxRecoverLayer )
	{
		if( points + power > maxRecover )
		{
			points = maxRecover;
		}
		else
		{
			points += power;
		}
	}
	else
	{
		if( points + power > pointsPerLayer )
		{
			layer++;
			points = points + power - pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
}

void PowerBar::Charge( int power )
{
	if( layer == maxLayer )
	{
		if( points + power > pointsPerLayer )
		{
			points = pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
	else
	{
		if( points + power > pointsPerLayer )
		{
			layer++;
			points = points + power - pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
}

void Grass::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool Grass::IsTouchingBox( const Rect<double> &r )
{
	return isQuadTouchingQuad( V2d( r.left, r.top ), V2d( r.left + r.width, r.top ), 
		V2d( r.left + r.width, r.top + r.height ), V2d( r.left, r.top + r.height ),
		A, B, C, D );


	/*double left = min( edge->v0.x, edge->v1.x );
	double right = max( edge->v0.x, edge->v1.x );
	double top = min( edge->v0.y, edge->v1.y );
	double bottom = max( edge->v0.y, edge->v1.y );

	Rect<double> er( left, top, right - left, bottom - top );

	if( er.intersects( r ) )
	{
		return true;
	}*/
}

 //groundLeft,airLeft,airRight,groundRight
EnvPlant::EnvPlant(sf::Vector2<double>&a, V2d &b, V2d &c, V2d &d, int vi, VertexArray *v, Tileset *t )
	:A(a),B(b),C(c),D(d), vaIndex( vi ), va( v ), frame( 0 ), activated( false ), next( NULL ), ts( t ),
	idleLength( 4 ), idleFactor( 3 )
{
	particle = new AirParticleEffect( ( b + c ) / 2.0 );
	disperseLength = particle->maxDurationToLive + particle->emitDuration;
	disperseFactor = 1;
	SetupQuad();
}

void EnvPlant::SetupQuad()
{
	VertexArray &eva = *va;
	eva[vaIndex+0].position = Vector2f( A.x, A.y );
	eva[vaIndex+1].position = Vector2f( B.x, B.y );
	eva[vaIndex+2].position = Vector2f( C.x, C.y );
	eva[vaIndex+3].position = Vector2f( D.x, D.y );
}

void EnvPlant::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool EnvPlant::IsTouchingBox( const Rect<double> &r )
{
	return isQuadTouchingQuad( V2d( r.left, r.top ), V2d( r.left + r.width, r.top ), 
		V2d( r.left + r.width, r.top + r.height ), V2d( r.left, r.top + r.height ),
		A, B, C, D );
}

void EnvPlant::Reset()
{
	
	//cout << "resetting plant!" << endl;
	next = NULL;
	activated = false;
	frame = 0;

	SetupQuad();

	particle->Reset();
}

//GameSession::GameStartSeq::GameStartSeq( GameSession *own )
//	:stormVA( sf::Quads, 6 * 3 * 4 ) 
//{
//	owner = own;
//	shipTex.loadFromFile( "ship.png" );
//	shipSprite.setTexture( shipTex );
//	shipSprite.setOrigin( shipSprite.getLocalBounds().width / 2, shipSprite.getLocalBounds().height / 2 );
//
//	stormTex.loadFromFile( "stormclouds.png" );
//	stormSprite.setTexture( stormTex );
//	
//	//shipSprite.setPosition( 250, 250 );
//	startPos = Vector2f( owner->player->position.x, owner->player->position.y );
//	frameCount = 1;//180;
//	frame = 0;
//
//	int count = 6;
//	for( int i = 0; i < count; ++i )
//	{
//		Vector2f topLeft( startPos.x - 480, startPos.y - 270 );
//		topLeft.y -= 540;
//
//		topLeft.x += i * 960;
//
//		stormVA[i*4].position = topLeft;
//		stormVA[i*4].texCoords = Vector2f( 0, 0 );
//
//		stormVA[i*4+1].position = topLeft + Vector2f( 0, 540 );
//		stormVA[i*4+1].texCoords = Vector2f( 0, 540 );
//
//		stormVA[i*4+2].position = topLeft + Vector2f( 960, 540 );
//		stormVA[i*4+2].texCoords = Vector2f( 960, 540 );
//
//		stormVA[i*4+3].position = topLeft + Vector2f( 960, 0 );
//		stormVA[i*4+3].texCoords = Vector2f( 960, 0 );
//
//		
//		
//
//
//		topLeft.y += 440 + 540;
//
//		stormVA[i*4 + 4 * count].position = topLeft;
//		stormVA[i*4 + 4 * count].texCoords = Vector2f( 0, 0 );
//
//		stormVA[i*4+1+4 * count].position = topLeft + Vector2f( 0, 540 );
//		stormVA[i*4+1+4 * count].texCoords = Vector2f( 0, 540 );
//
//		stormVA[i*4+2+4 * count].position = topLeft + Vector2f( 960, 540 );
//		stormVA[i*4+2+4 * count].texCoords = Vector2f( 960, 540 );
//
//		stormVA[i*4+3+4 * count].position = topLeft + Vector2f( 960, 0 );
//		stormVA[i*4+3+4 * count].texCoords = Vector2f( 960, 0 );
//
//		topLeft.y += 540;
//		stormVA[i*4 + 4 * count * 2].position = topLeft;
//		stormVA[i*4 + 4 * count * 2].texCoords = Vector2f( 0, 0 );
//
//		stormVA[i*4+1 + 4 * count * 2].position = topLeft + Vector2f( 0, 540 );
//		stormVA[i*4+1 + 4 * count * 2].texCoords = Vector2f( 0, 540 );
//
//		stormVA[i*4+2 + 4 * count * 2].position = topLeft + Vector2f( 960, 540 );
//		stormVA[i*4+2 + 4 * count * 2].texCoords = Vector2f( 960, 540 );
//
//		stormVA[i*4+3 + 4 * count * 2].position = topLeft + Vector2f( 960, 0 );
//		stormVA[i*4+3 + 4 * count * 2].texCoords = Vector2f( 960, 0 );
//	}
//}
//
//bool GameSession::GameStartSeq::Update()
//{
//	if( frame < frameCount )
//	{
//		
//		V2d vel( 60, 0 );
//		//if( frame > 60 )
//			//vel.y = -20;
//
//		shipSprite.setPosition( startPos.x + frame * vel.x, startPos.y + frame * vel.y );
//		++frame;
//
//		return true;
//	}
//	else 
//		return false;
//}
//
//void GameSession::GameStartSeq::Draw( sf::RenderTarget *target )
//{
//	target->setView( owner->bgView );
//	target->draw( owner->background );
//	target->setView( owner->view );
//
//	target->setView( owner->uiView );
//	owner->powerBar.Draw( target );
//
//	target->setView( owner->view );
//	/*sf::RectangleShape rs( Vector2f( 960 * 4, 540 ) );
//	rs.setPosition( Vector2f( startPos.x - 480, startPos.y - 270 ) );
//	rs.setFillColor( Color::Black );
//	target->draw( rs );*/
//
//
//	//target->draw( stormVA, &stormTex );
//
//	//target->draw( shipSprite );
//
//}

void GameSession::SetParMountains( sf::RenderTarget *target )
{
	View vah = view;
	double zoomFactor = 6.0;
	double yChange = 100;
	double zoom = view.getSize().x / 960.0;
	double addZoom = (zoom - 1) / zoomFactor;
	double newZoom = 1 + addZoom;

	vah.setSize( 960 * newZoom, 540 * newZoom );
	//vah.setSize( 960 * zoom, 540 * zoom );
	vah.setCenter( vah.getCenter().x / zoomFactor, vah.getCenter().y / zoomFactor );

	sf::RectangleShape rs;
	rs.setSize( Vector2f( vah.getSize().x, 512 ) );
	rs.setFillColor( Color::Red );
	rs.setPosition( vah.getCenter().x - vah.getSize().x / 2, - 512 + yChange );

	mountainShader.setParameter( "Resolution", 1920, 1080 );
	mountainShader.setParameter( "zoom", newZoom );
	mountainShader.setParameter( "size", 1920, 1024 );
	
	Vector2f trueBotLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y / 2 );
	Vector2i tempPos = preScreenTex->mapCoordsToPixel( trueBotLeft );
	preScreenTex->setView( vah );
	trueBotLeft = preScreenTex->mapPixelToCoords( tempPos );
	trueBotLeft.y -= yChange;


	mountainShader.setParameter( "topLeft", trueBotLeft );

	preScreenTex->draw( rs, &mountainShader );

	preScreenTex->setView( view );
}

void GameSession::SetParMountains1( sf::RenderTarget *target )
{
	View vah = view;
	double zoomFactor = 4;
	double yChange = 200;
	double zoom = view.getSize().x / 960.0;
	double addZoom = (zoom - 1) / zoomFactor;
	double newZoom = 1 + addZoom;

	//vah.setSize( 960 * newZoom, 540 * newZoom );
	vah.setSize( 960 * newZoom, 540 * newZoom );
	vah.setCenter( vah.getCenter().x / zoomFactor, vah.getCenter().y / zoomFactor );
	
	sf::RectangleShape rs;
	rs.setSize( Vector2f( vah.getSize().x, 406 ) );
	//rs.setFillColor( Color::Red );
	rs.setPosition( vah.getCenter().x - vah.getSize().x / 2, -406 + yChange );//- 512 );

	mountainShader1.setParameter( "Resolution", 1920, 1080 );
	mountainShader1.setParameter( "zoom", newZoom );
	mountainShader1.setParameter( "size", 1920, 812 );
	
	
	Vector2f trueBotLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y / 2 );
	Vector2i tempPos = preScreenTex->mapCoordsToPixel( trueBotLeft );
	preScreenTex->setView( vah );
	trueBotLeft = preScreenTex->mapPixelToCoords( tempPos );
	trueBotLeft.y -= yChange;

	mountainShader1.setParameter( "topLeft", trueBotLeft );

	preScreenTex->draw( rs, &mountainShader1 );

	preScreenTex->setView( view );
}

void GameSession::SetParOnTop( sf::RenderTarget *target )
{
	//closeBack0.setPosition( -960, -400 );
	//closeBack0.setTextureRect( IntRect( 0, 0, 1920, 400 ) );
	//closeBack0.setColor( Color::Red );
	
	sf::RectangleShape rs;
	rs.setSize( Vector2f( view.getSize().x, 370 / 2 ) );
	rs.setFillColor( Color::White );
	rs.setPosition( view.getCenter().x - view.getSize().x / 2, - 370 / 2 );

	onTopShader.setParameter( "Resolution", 1920, 1080 );
	onTopShader.setParameter( "zoom", cam.GetZoom() );
	onTopShader.setParameter( "topLeft", view.getCenter().x - view.getSize().x / 2,
		view.getCenter().y + view.getSize().y / 2 );

	preScreenTex->draw( rs, &onTopShader );
	//preScreenTex->draw( rs );


	/*int tilesWide = 3;
	int totalWidth = 1920 * tilesWide;
	int camLeft = view.getCenter().x - view.getSize().x / 2;
	int camRight = view.getCenter().x + view.getSize().x / 2;
	int diff = camLeft / totalWidth;

	sf::RectangleShape r0( Vector2f( 1920, 400 ) );

	r0.setPosition( diff * totalWidth, -400 );
	r0.setFillColor( Color::Red );

	preScreenTex->draw( r0 );

	r0.setPosition( 1920 + diff * totalWidth, -400 );
	r0.setFillColor( Color::Green );

	preScreenTex->draw( r0 );

	r0.setPosition( 1920 * 2 + diff * totalWidth, -400 );
	r0.setFillColor( Color::Blue );

	
	preScreenTex->draw( r0 );*/


	
	/*int tilesWide = 6;
	int zoom = 1;
	int height = 400;
	int width = 1920;
	int totalWidth = width * tilesWide;
	for( int i = 0; i < tilesWide; ++i )
	{
		onTopPar[i*4].color = Color::Blue;
		onTopPar[i*4+1].color= Color::Red;
		onTopPar[i*4+2].color= Color::Red;
		onTopPar[i*4+3].color= Color::Blue;
	}

	Vector2f delta;
	delta.x = cam.pos.x / zoom;
	delta.y = cam.pos.y / zoom;

	cout << "blah: " << (int)cam.pos.x % totalWidth << endl;
	for( int i = 0; i < tilesWide; ++i )
	{
		int x;
		if( cam.pos.x >= 0 )
		{
			x = i * width + cam.pos.x - ;//- (int)cam.pos.x % totalWidth;

			onTopPar[i*4].position = Vector2f( x, -height );
			onTopPar[i*4+1].position = Vector2f( x + width, -height );
			onTopPar[i*4+2].position = Vector2f( x + width, 0 );
			onTopPar[i*4+3].position = Vector2f( x, 0 );
		}
		else
		{
			x = i * width + cam.pos.x - (int)cam.pos.x % totalWidth;
			onTopPar[i*4].position = Vector2f( x, -height );
			onTopPar[i*4+1].position = Vector2f( x + width, -height );
			onTopPar[i*4+2].position = Vector2f( x + width, 0 );
			onTopPar[i*4+3].position = Vector2f( x, 0 );
		}

		
		
	}

	target->draw( onTopPar );*/
	
}

bool GameSession::SetGroundPar()
{	
	Color undertransColor( 255, 50, 255, 255 );
	Color altTransColor( 0, 255, 255, 255 );
	int widthFactor = 1;
	int yView = view.getCenter().y / widthFactor;
	cloudView.setCenter( 960, 540 + yView );
	int cloudBot = cloudView.getCenter().y + cloudView.getSize().y / 2;

	//cout << "yView << " << yView << endl;
	int tileHeight = 1080 / 2;//540;
	int transTileHeight = 750;//650 / 2;

	if( yView > 1080 + transTileHeight || yView < -tileHeight )
	{
	//	return false;
	}
	Vector2f offset( 0, -transTileHeight );
	
	int width = 1920 * widthFactor;
	bool flipped = false;
	int a = ((int)view.getCenter().x) % width;
	double ratio = a / (double)width;
	if( ratio < 0 )
		ratio = 1 + ratio;

	int b = ((int)view.getCenter().x) % (width * 2);
	double ratiob = b / (double)(width);
	if( ratiob < 0 )
		ratiob = 2 + ratiob;

	if( ratiob > ratio + .001 )
	{
	//	cout << "flipped ratiob: " << ratiob << ", oldratio: " << ratio << endl;
		flipped = true;
	}
	

	int i = 0;
	if( flipped )
	{
		i = 1;
		
	}
	//preScreenTex->setView( cloudView );
	float screenBottom = view.getCenter().y + view.getSize().y / 2;
	int transBot;
	
	int groundBottom = 1080;
	int groundTop = groundBottom - tileHeight;
	preScreenTex->setView( view );
	

	transBot = groundBottom + transTileHeight + 1;

	if( screenBottom >= 0 )
	{
		//cout << "undergroundPos: " << 
		Vector2i po = preScreenTex->mapCoordsToPixel( Vector2f( 0, 0 ) );
		preScreenTex->setView( cloudView );
		Vector2f la = preScreenTex->mapPixelToCoords( po );
	//	cout << "under: " << la.y << endl;
		transBot = -offset.y + la.y + 1;
	//	Vector2f pix = preScreenTex->mapPixelToCoords( Vector2i( 1920, 1080 ) ).y;
		//underground is visible
		//transBot =  //1080 - screenBottom;;//cloudBot;//cloudBot; //
		//transBot = view.getCenter().y / 2 / cam.GetZoom();
	//	cout << "transbot: " << transBot << ", center: " << view.getCenter().y << endl;
	}
	//else
	{
		//underground isn't visible
	//	cout << "transtop: " << groundBottom << endl;
	//	cout << "transbot no underground: " << transBot << endl;
	}
	int transTop = groundBottom;
	//ratio = 1 - ratio;
	
	ratio = 1 - ratio;
	//cout << "ratio: " << ratio << endl;

	groundPar[i*4].position = Vector2f( 0, groundTop ) + offset;
	groundPar[i*4+1].position = Vector2f( 1920 * ratio, groundTop) + offset;
	groundPar[i*4+2].position = Vector2f( 1920 * ratio, groundBottom ) + offset;
	groundPar[i*4+3].position = Vector2f( 0, groundBottom ) + offset;

	

	groundPar[i*4].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * i );
	groundPar[i*4 + 1].texCoords = Vector2f( 1920, tileHeight * i );
	groundPar[i*4 + 2].texCoords = Vector2f( 1920, tileHeight * (i + 1) );
	groundPar[i*4 + 3].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * (i + 1) );

	//int what = transTileHeight;//tileHeight / 2 + 100;

//	int bottom = transTileHeight + (1080);
	
	//preScreenTex->setView( view );
	if( screenBottom >= 0 )
	{
	//	bottom = preScreenTex->mapCoordsToPixel( Vector2f( 0, 0 ) ).y;//transTileHeight + 1080 - screenBottom;
	}
	/*underTransPar[i*4].position = Vector2f( 0, transTop  ) + offset;
	underTransPar[i*4+1].position = Vector2f( 1920 * ratio, transTop ) + offset;
	underTransPar[i*4+2].position = Vector2f( 1920 * ratio, transBot ) + offset;
	underTransPar[i*4+3].position = Vector2f( 0, transBot ) + offset;

	underTransPar[i*4].color = undertransColor;
	underTransPar[i*4 + 1].color = undertransColor;
	underTransPar[i*4 + 2].color = altTransColor;
	underTransPar[i*4 + 3].color = altTransColor;*/

	if( flipped )
	{
		i = 0;
	}
	else
	{
		i = 1;
	}



	groundPar[i*4].position = Vector2f( 1920 * ratio , groundTop ) + offset;
	groundPar[i*4+ 1].position = Vector2f( 1920, groundTop ) + offset;
	groundPar[i*4+2].position = Vector2f( 1920, groundBottom ) + offset;
	groundPar[i*4+3].position = Vector2f( 1920 * ratio , groundBottom ) + offset;

	groundPar[i*4].texCoords = Vector2f( 0, tileHeight * i );
	groundPar[i*4+1].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * i );
	groundPar[i*4+2].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * (i+1) );
	groundPar[i*4+3].texCoords = Vector2f( 0, tileHeight * (i+1) );

	/*underTransPar[i*4].position = Vector2f( 1920 * ratio , transTop ) + offset;
	underTransPar[i*4+ 1].position = Vector2f( 1920, transTop ) + offset;
	underTransPar[i*4+2].position = Vector2f( 1920, transBot ) + offset;
	underTransPar[i*4+3].position = Vector2f( 1920 * ratio , transBot ) + offset;

	underTransPar[i*4].color = undertransColor;
	underTransPar[i*4 + 1].color = undertransColor;
	underTransPar[i*4 + 2].color = altTransColor;
	underTransPar[i*4 + 3].color = altTransColor;*/

	
	
	preScreenTex->setView( cloudView );

	return true;
}

void GameSession::SetupClouds()
{
	clouds[0].setTexture( *cloudTileset->texture );//"cloud01.png
	clouds[1].setTexture( *cloudTileset->texture );
	clouds[2].setTexture( *cloudTileset->texture );
	clouds[3].setTexture( *cloudTileset->texture );
	clouds[4].setTexture( *cloudTileset->texture );

	for( int i = 0; i < NUM_CLOUDS; ++i )
	{
		//clouds[i].setOrigin( clouds[i].getLocalBounds().width / 2, clouds[i].getLocalBounds().height / 2 );
	}

	clouds[0].setPosition( 0, 0 );
	clouds[1].setPosition( 100, 100 );
	clouds[2].setPosition( 200, 300 );
	clouds[3].setPosition( 300, 500 );
	clouds[4].setPosition( 400, 700 );
}

void GameSession::SetCloudParAndDraw()
{
	int depth = 8;
	Vector2f orig( originalPos.x, originalPos.y );
	int screenWidthFactor = 3;


	for( int i = 0; i < NUM_CLOUDS; ++i )
	{
		if( view.getCenter().x < 0 )
		{
			int x = (int)((orig.x + view.getCenter().x) / depth - .5);
			clouds[i].setPosition( -( x % ( 1920 * screenWidthFactor ) 
				+ ( (1920 * screenWidthFactor) / 2 )), clouds[i].getPosition().y );
		//	cout << "neg: " << clouds[i].getPosition().x << endl;
		}
		else
		{
			int x = (int)((orig.x + view.getCenter().x) / depth + .5);
			clouds[i].setPosition( -(x % ( 1920 * screenWidthFactor ) 
				- ( (1920 * screenWidthFactor) / 2 )), clouds[i].getPosition().y );
		//	cout << "pos: " << clouds[i].getPosition().x << endl;
		}

		if( view.getCenter().y < 0 )
		{
			clouds[i].setPosition( clouds[i].getPosition().x, -((int)((orig.y + view.getCenter().y) / depth - .5) % ( -1080 * 3 ) ));// - ( 1080 * 3 / 2 )));
		}
		else
		{
			clouds[i].setPosition( clouds[i].getPosition().x, -((int)((orig.y + view.getCenter().y) / depth + .5) % ( 1080 * 3 ) ));// - ( 1080 * 3 / 2 )));
		}
		//cout << "cloudpos: " << clouds[i].getPosition().x << ", " << clouds[i].getPosition().y << endl;
		
		preScreenTex->draw( clouds[i] );
	}

	//float depth = 3;
	//parTest.setPosition( orig / depth + ( cam.pos - orig ) / depth );
	//float scale = 1 + ( 1 - 1 / ( cam.GetZoom() * depth ) );
	//parTest.setScale( scale, scale );
}

void GameSession::SetUndergroundParAndDraw()
{
	preScreenTex->setView( view );

	underShader.setParameter( "u_texture", *GetTileset( "underground01.png" , 128, 128 )->texture );
	underShader.setParameter( "u_normals", *GetTileset( "underground01_NORMALS.png", 128, 128 )->texture );
	//underShader.setParameter( "u_pattern", *GetTileset( "terrainworld1_PATTERN.png", 16, 16 )->texture );

	underShader.setParameter( "AmbientColor", 1, 1, 1, 1 );
	underShader.setParameter( "Resolution", 1920, 1080 );//window->getSize().x, window->getSize().y);
	underShader.setParameter( "zoom", cam.GetZoom() );
	underShader.setParameter( "topLeft", view.getCenter().x - view.getSize().x / 2, 
		view.getCenter().y + view.getSize().y / 2 );

	lightsAtOnce = 0;
	tempLightLimit = 0;//9; //only for now

	sf::Rect<double> r( view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2, view.getSize().x, view.getSize().y );
	
	queryMode = "lights"; 
	lightTree->Query( this, r );

	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f,  -1 + vi.y / 1080.f, .015 );
	//polyShader.setParameter( "stuff", 10, 10, 10 );
	
/*	Vector3f pos0( vi0.x / 1920.f, (1080 - vi0.y) / 1080.f, .015 ); 
	pos0.y = 1 - pos0.y;
	Vector3f pos1( vi1.x / 1920.f, (1080 - vi1.y) / 1080.f, .015 ); 
	pos1.y = 1 - pos1.y;
	Vector3f pos2( vi2.x / 1920.f, (1080 - vi2.y) / 1080.f, .015 ); 
	pos2.y = 1 - pos2.y;*/
	
	bool on[9];
	for( int i = 0; i < 9; ++i )
	{
		on[i] = false;
	}

	float windowx = 1920;//window->getSize().x;
	float windowy = 1080;//window->getSize().y;
	
	if( lightsAtOnce > 0 )
	{
		float depth0 = touchedLights[0]->depth;
		Vector2i vi0 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[0]->pos.x, touchedLights[0]->pos.y ) );
		

		Vector3f pos0( vi0.x / windowx, -1 + vi0.y / windowy, depth0 ); 
		//Vector3f pos0( vi0.x / (float)window->getSize().x, ((float)window->getSize().y - vi0.y) / (float)window->getSize().y, depth0 ); 
		Color c0 = touchedLights[0]->color;
		
		//underShader.setParameter( "On0", true );
		on[0] = true;
		underShader.setParameter( "LightPos0", pos0 );//Vector3f( 0, -300, .075 ) );
		underShader.setParameter( "LightColor0", c0.r / 255.0, c0.g / 255.0, c0.b / 255.0, 1 );
		underShader.setParameter( "Radius0", touchedLights[0]->radius );
		underShader.setParameter( "Brightness0", touchedLights[0]->brightness);
		
	}
	if( lightsAtOnce > 1 )
	{
		float depth1 = touchedLights[1]->depth;
		Vector2i vi1 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[1]->pos.x, touchedLights[1]->pos.y ) ); 
		Vector3f pos1( vi1.x / windowx, -1 + vi1.y / windowy, depth1 ); 
		//Vector3f pos1( vi1.x / (float)window->getSize().x, ((float)window->getSize().y - vi1.y) / (float)window->getSize().y, depth1 ); 
		Color c1 = touchedLights[1]->color;
		
		on[1] = true;
		//underShader.setParameter( "On1", true );
		underShader.setParameter( "LightPos1", pos1 );//Vector3f( 0, -300, .075 ) );
		underShader.setParameter( "LightColor1", c1.r / 255.0, c1.g / 255.0, c1.b / 255.0, 1 );
		underShader.setParameter( "Radius1", touchedLights[1]->radius );
		underShader.setParameter( "Brightness1", touchedLights[1]->brightness);
	}
	if( lightsAtOnce > 2 )
	{
		float depth2 = touchedLights[2]->depth;
		Vector2i vi2 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[2]->pos.x, touchedLights[2]->pos.y ) );
		Vector3f pos2( vi2.x / windowx, -1 + vi2.y / windowy, depth2 ); 
		//Vector3f pos2( vi2.x / (float)window->getSize().x, ((float)window->getSize().y - vi2.y) / (float)window->getSize().y, depth2 ); 
		Color c2 = touchedLights[2]->color;
		
		on[2] = true;
		//underShader.setParameter( "On2", true );
		underShader.setParameter( "LightPos2", pos2 );//Vector3f( 0, -300, .075 ) );
		underShader.setParameter( "LightColor2", c2.r / 255.0, c2.g / 255.0, c2.b / 255.0, 1 );
		underShader.setParameter( "Radius2", touchedLights[2]->radius );
		underShader.setParameter( "Brightness2", touchedLights[2]->brightness);
	}
	if( lightsAtOnce > 3 )
	{
		float depth3 = touchedLights[3]->depth;
		Vector2i vi3 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[3]->pos.x, touchedLights[3]->pos.y ) );
		Vector3f pos3( vi3.x / windowx, -1 + vi3.y / windowy, depth3 ); 
		//Vector3f pos3( vi3.x / (float)window->getSize().x, ((float)window->getSize().y - vi3.y) / (float)window->getSize().y, depth3 ); 
		Color c3 = touchedLights[3]->color;
		
		on[3] = true;
		//underShader.setParameter( "On3", true );
		underShader.setParameter( "LightPos3", pos3 );
		underShader.setParameter( "LightColor3", c3.r / 255.0, c3.g / 255.0, c3.b / 255.0, 1 );
		underShader.setParameter( "Radius3", touchedLights[3]->radius );
		underShader.setParameter( "Brightness3", touchedLights[3]->brightness);
	}
	if( lightsAtOnce > 4 )
	{
		float depth4 = touchedLights[4]->depth;
		Vector2i vi4 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[4]->pos.x, touchedLights[4]->pos.y ) );
		Vector3f pos4( vi4.x / windowx, -1 + vi4.y / windowy, depth4 ); 
		//Vector3f pos4( vi4.x / (float)window->getSize().x, ((float)window->getSize().y - vi4.y) / (float)window->getSize().y, depth4 ); 
		Color c4 = touchedLights[4]->color;
		
		
		on[4] = true;
		underShader.setParameter( "LightPos4", pos4 );
		underShader.setParameter( "LightColor4", c4.r / 255.0, c4.g / 255.0, c4.b / 255.0, 1 );
		underShader.setParameter( "Radius4", touchedLights[4]->radius );
		underShader.setParameter( "Brightness4", touchedLights[4]->brightness);
	}
	if( lightsAtOnce > 5 )
	{
		float depth5 = touchedLights[5]->depth;
		Vector2i vi5 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[5]->pos.x, touchedLights[5]->pos.y ) );
		Vector3f pos5( vi5.x / windowx, -1 + vi5.y / windowy, depth5 ); 
		//Vector3f pos5( vi5.x / (float)window->getSize().x, ((float)window->getSize().y - vi5.y) / (float)window->getSize().y, depth5 ); 
		Color c5 = touchedLights[5]->color;
		
		
		on[5] = true;
		underShader.setParameter( "LightPos5", pos5 );
		underShader.setParameter( "LightColor5", c5.r / 255.0, c5.g / 255.0, c5.b / 255.0, 1 );
		underShader.setParameter( "Radius5", touchedLights[5]->radius );
		underShader.setParameter( "Brightness5", touchedLights[5]->brightness);
	}
	if( lightsAtOnce > 6 )
	{
		float depth6 = touchedLights[6]->depth;
		Vector2i vi6 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[6]->pos.x, touchedLights[6]->pos.y ) );
		Vector3f pos6( vi6.x / windowx, -1 + vi6.y / windowy, depth6 ); 
		//Vector3f pos6( vi6.x / (float)window->getSize().x, ((float)window->getSize().y - vi6.y) / (float)window->getSize().y, depth6 ); 
		Color c6 = touchedLights[6]->color;
		
		on[6] = true;
		underShader.setParameter( "LightPos6", pos6 );
		underShader.setParameter( "LightColor6", c6.r / 255.0, c6.g / 255.0, c6.b / 255.0, 1 );
		underShader.setParameter( "Radius6", touchedLights[0]->radius );
		underShader.setParameter( "Brightness6", touchedLights[0]->brightness);
	}
	if( lightsAtOnce > 7 )
	{
		float depth7 = touchedLights[7]->depth;
		Vector2i vi7 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[7]->pos.x, touchedLights[7]->pos.y ) );
		Vector3f pos7( vi7.x / windowx, -1 + vi7.y / windowy, depth7 ); 
		//Vector3f pos7( vi7.x / (float)window->getSize().x, ((float)window->getSize().y - vi7.y) / (float)window->getSize().y, depth7 ); 
		Color c7 = touchedLights[7]->color;
		
		on[7] = true;
		underShader.setParameter( "LightPos7", pos7 );
		underShader.setParameter( "LightColor7", c7.r / 255.0, c7.g / 255.0, c7.b / 255.0, 1 );
		underShader.setParameter( "Radius7", touchedLights[7]->radius );
		underShader.setParameter( "Brightness7", touchedLights[7]->brightness);
	}
	if( lightsAtOnce > 8 )
	{
		float depth8 = touchedLights[8]->depth;
		Vector2i vi8 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[8]->pos.x, touchedLights[8]->pos.y ) );
		Vector3f pos8( vi8.x / windowx, -1 + vi8.y / windowy, depth8 ); 
		//Vector3f pos8( vi8.x / (float)window->getSize().x, ((float)window->getSize().y - vi8.y) / (float)window->getSize().y, depth8 ); 
		Color c8 = touchedLights[8]->color;
		
		on[8] = true;
		underShader.setParameter( "LightPos8", pos8 );
		underShader.setParameter( "LightColor8", c8.r / 255.0, c8.g / 255.0, c8.b / 255.0, 1 );
		underShader.setParameter( "Radius8", touchedLights[8]->radius );
		underShader.setParameter( "Brightness8", touchedLights[8]->brightness);
	}

	underShader.setParameter( "On0", on[0] );
	underShader.setParameter( "On1", on[1] );
	underShader.setParameter( "On2", on[2] );
	underShader.setParameter( "On3", on[3] );
	underShader.setParameter( "On4", on[4] );
	underShader.setParameter( "On5", on[5] );
	underShader.setParameter( "On6", on[6] );
	underShader.setParameter( "On7", on[7] );
	underShader.setParameter( "On8", on[8] );

	Color c = player->testLight->color;
	Vector2i vip = preScreenTex->mapCoordsToPixel( Vector2f( player->testLight->pos.x, player->testLight->pos.y ) );
	Vector3f posp( vip.x / windowx, -1 + vip.y / windowy, player->testLight->depth ); 
	underShader.setParameter( "LightPosPlayer", posp );
	underShader.setParameter( "LightColorPlayer", c.r / 255.0, c.g / 255.0, c.b / 255.0, 1 );
	underShader.setParameter( "RadiusPlayer", player->testLight->radius );
	underShader.setParameter( "BrightnessPlayer", player->testLight->brightness );

	/*undergroundPar[0].color = Color::Red;
	undergroundPar[1].color = Color::Red;
	undergroundPar[2].color = Color::Red;
	undergroundPar[3].color = Color::Red;*/

	Vector2f center = view.getCenter();

	float top = center.y - view.getSize().y / 2;
	float left = center.x - view.getSize().x / 2;
	float bottom = center.y + view.getSize().y / 2;
	float right = center.x + view.getSize().x / 2;
	
	
	//cout << preScreenTex->getView().getCenter().x << ", " << preScreenTex->getView().getCenter().y << endl;
//	cout << "zoom: " << cam.GetZoom() << ", dist: " << -center.y * cam.GetZoom() /  4  << endl;
	
	//int distFromTop = 
	//cout << "distfrom: " << distFromTop << endl;
	//if( distFromTop < 0 )
	//	distFromTop = 0;
	//if( distFromTop > 1080 )
	//{
		/*undergroundPar[0].position = Vector2f( 0, 0 );
		undergroundPar[1].position = Vector2f( 0, 0 );
		undergroundPar[2].position = Vector2f( 0, 0 );
		undergroundPar[3].position = Vector2f( 0, 0 );*/
	//}
	//else
	int blah = 0;
	//cout << "blah: " << blah << endl;
	if( bottom < blah )
	{
	}
	else
	{
		
		if( top < blah )
		{
			top = blah;
		}
		//preScreenTex->setView( view );
		//top = 0;
		undergroundPar[0].position = Vector2f( left, top );
		undergroundPar[1].position = Vector2f( right, top );
		undergroundPar[2].position = Vector2f( right, bottom );
		undergroundPar[3].position = Vector2f( left, bottom );
		preScreenTex->draw( undergroundPar, &underShader );
	}
	
		
	//else
	{
		
		//cout << "NOT normal" << endl;
	}

	//cloudView.setCenter( cloudView.getCenter().x, center.y );

	
}

//only activates zones if they're inactive. 
void GameSession::ActivateZone( Zone *z )
{
	if( z == NULL )
		return;
	//assert( z != NULL );
	//cout << "ACTIVATE ZONE!!!" << endl;
	if( !z->active )
	{
		for( list<Enemy*>::iterator it = z->spawnEnemies.begin(); it != z->spawnEnemies.end(); ++it )
		{
			assert( (*it)->spawned == false );

			(*it)->Init();
			(*it)->spawned = true;
			AddEnemy( (*it) );
		}

		
		z->active = true;


		

		if( activatedZoneList == NULL )
		{
			activatedZoneList = z;
			z->activeNext = NULL;
		}
		else
		{
			z->activeNext = activatedZoneList;
			activatedZoneList = z;
		}
	}

	//z->SetShadowColor( Color( 0, 0, 255, 10 ) );
	currentZone = z;
	keyMarker->SetStartKeys( currentZone->requiredKeys );
	int soundIndex = SoundType::S_KEY_ENTER_0 + ( currentZone->requiredKeys );
	soundNodeList->ActivateSound( gameSoundBuffers[soundIndex] );
}

void GameSession::UnlockGate( Gate *g )
{
	g->SetLocked( false );

	cout << "adding gate to unlock list: " << g << endl;
	if( unlockedGateList == NULL )
	{
		unlockedGateList = g;
		g->activeNext = NULL;
	}
	else
	{
		g->activeNext = unlockedGateList;
		unlockedGateList = g;
	}

	//if( player->rightWire != NULL && player->rightWire->a

	if( currentZone != NULL )
	{
		list<Edge*> &gList = currentZone->gates;
		for( list<Edge*>::iterator it = gList.begin(); it != gList.end(); ++it )
		{
			Gate *gg = (Gate*)(*it)->info;
			if( gg == g || gg->gState == Gate::OPEN || gg->gState == Gate::DISSOLVE )
				continue;

			gg->gState = Gate::LOCKFOREVER;
			//g->SetLocked();
		}
	}
}

void GameSession::LockGate( Gate *g )
{
	//inefficient but i can adjust it later using prev pointers
	g->SetLocked( true );

	//Enemy *prev = e->prev;
	//Enemy *next = e->next;
	assert( unlockedGateList != NULL );
	if( unlockedGateList->activeNext == NULL )
	{
		unlockedGateList = NULL;
	}
	else
	{
		Gate *gate = unlockedGateList;
		if( g == unlockedGateList )
		{
			unlockedGateList = unlockedGateList->activeNext;
			//g->activeNext = unlockedGateList->activeNext;
			//unlockedGateList = g;
			//break;
		} 
		else
		while( gate != NULL )
		{
			//do i need this? i feel like i need this
			if( gate->activeNext == g )
			{
				Gate *gate2 = gate->activeNext->activeNext;
				g->activeNext = NULL;
				gate->activeNext = gate2;
				break;
			}
			gate = gate->activeNext;
		}
	}
	
}

void GameSession::TriggerBarrier( Barrier *b )
{
	PoiInfo *poi = b->poi;
	string name = poi->name;

	if( name == "birdfighttrigger" )
	{
		assert( b_bird != NULL );
		
		b_bird->spawned = true;
		AddEnemy( b_bird );
	}
	else if( name == "crawlerfighttrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		activeSequence = b_crawler->crawlerFightSeq;
		activeSequence->frame = 0;

		assert( b_crawler != NULL );
		b_crawler->spawned = true;
		AddEnemy( b_crawler );

	}
	else if( name == "meetcoyotetrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		powerWheel->Hide( true, 60 );
		activeSequence = b_coyote->meetCoyoteSeq;
		activeSequence->frame = 0;

		assert( b_coyote != NULL );
		b_coyote->spawned = true;
		AddEnemy( b_coyote );
	}
	else if( name == "coyotefighttrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		powerWheel->Hide( true, 60 );
		activeSequence = b_coyote->coyoteFightSeq;
		activeSequence->frame = 0;
	}
}

Critical::Critical( V2d &pointA, V2d &pointB )
	:bar( sf::Quads, 4 )
{
	for( int i = 0; i < Gate::GateType::Count; ++i )
	{
		hadKey[i] = false;
	}
	//hadBlueKey = false;

	anchorA = pointA;
	anchorB = pointB;
	
	V2d dir( anchorB - anchorA );
	double len = length( dir );
	dir = normalize( dir );
	
	pos = anchorA + dir * len / 2.0; //+ ( anchorB - anchorA ) / 2.0;
	radius = 100;

	next = NULL;
	prev = NULL;

	box.rw = radius;
	box.rh = radius;
	box.globalPosition = pos;
	box.isCircle = true;	
	
	active = true;

	double width = 5;
	V2d along = normalize( anchorB - anchorA );
	V2d other( along.y, -along.x );
	
	V2d leftv0 = anchorA - other * width;
	V2d rightv0 = anchorA + other * width;

	V2d leftv1 = anchorB - other * width;
	V2d rightv1 = anchorB + other * width;

	
	Color c = Color::Black;
	bar[0].color = c;
	bar[1].color = c;
	bar[2].color = c;
	bar[3].color = c;

	bar[0].position = Vector2f( leftv0.x, leftv0.y );
	bar[1].position = Vector2f( leftv1.x, leftv1.y );
	bar[2].position = Vector2f( rightv1.x, rightv1.y );
	bar[3].position = Vector2f( rightv0.x, rightv0.y );
}

void Critical::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Critical::IsTouchingBox( const sf::Rect<double> &r )
{
	sf::Rect<double> circleBox( pos.x - radius, pos.y - radius, radius * 2, radius * 2 );
	return circleBox.intersects( r );
}

void Critical::Draw( RenderTarget *target )
{
	if( active )
	{
		CircleShape cs( radius );
		cs.setFillColor( Color::Magenta );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( pos.x, pos.y );

		
		target->draw( bar );
		target->draw( cs );
	}
	else
	{
		CircleShape cs( radius );
		cs.setFillColor( Color::Black );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( pos.x, pos.y );

		target->draw( bar );
		target->draw( cs );
	}
}