//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Physics.h"
#include "Action.h"
#include <set>
#include "MainMenu.h"
#include "Background.h"
#include "Parallax.h"
#include "Enemy_Shard.h"
#include "ActorParams.h"
#include "EditorBG.h"
#include "EditorRail.h"
#include "EditorGraph.h"


#include "clipper.hpp"
//using namespace ClipperLib;
//#include "TerrainRender.h"

using namespace std;
using namespace sf;

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

#define cout std::cout

const double EditSession::PRIMARY_LIMIT = .999;
const double EditSession::SLIVER_LIMIT = PI / 10.0;
double EditSession::zoomMultiple = 1;
EditSession * EditSession::currSession = NULL;

template <typename X>ActorParams * MakeParamsGrounded(ActorType *at)
{
	EditSession *edit = EditSession::GetSession();
	if (edit->enemyEdgePolygon != NULL)
	{
		return new X(at, edit->enemyEdgePolygon,
			edit->enemyEdgeIndex,
			edit->enemyEdgeQuantity);
	}
	else
	{
		return NULL;
	}
}

template <typename X>ActorParams * MakeParamsRailed(ActorType *at)
{
	EditSession *edit = EditSession::GetSession();
	if (edit->enemyEdgeRail != NULL)
	{
		return new X(at, edit->enemyEdgeRail,
			edit->enemyEdgeIndex,
			edit->enemyEdgeQuantity);
	}
	else
	{
		return NULL;
	}
}

template <typename X>ActorParams * MakeParamsAerial(ActorType *at)
{
	EditSession *edit = EditSession::GetSession();
	return new X(at, sf::Vector2i(edit->worldPos));
}

template<typename X> ActorParams *LoadParams(
	ActorType *at, std::ifstream &is)
{
	return new X(at, is);
}

EditSession *EditSession::GetSession()
{
	return currSession;
}

EditSession::EditSession( MainMenu *p_mainMenu )
	:w( p_mainMenu->window ), fullBounds( sf::Quads, 16 ), mainMenu( p_mainMenu ), arial( p_mainMenu->arial )
{
	currSession = this;
	for (int i = 0; i < MAX_TERRAINTEX_PER_WORLD * 9; ++i)
	{
		terrainTextures[i] = NULL;
	}

	minZoom = .25;
	maxZoom = 65536;

	copiedBrush = NULL;
	mapHeader.ver1 = 1;
	mapHeader.ver2 = 5;
	mapHeader.description = "no description";
	mapHeader.collectionName = "default";
	mapHeader.gameMode = MapHeader::T_STANDARD;//"default";
	//arialFont.loadFromFile( "Breakneck_Font_01.ttf" );
	cursorLocationText.setFont( mainMenu->arial );
	cursorLocationText.setCharacterSize( 16 );
	cursorLocationText.setFillColor( Color::White );
	cursorLocationText.setPosition( 0, 0 );
	
	scaleSprite.setPosition(0, 80);
	scaleSpriteBGRect.setPosition(0, 80);
	scaleSpriteBGRect.setFillColor(Color( 255, 255, 255, 200 ));
	scaleSpriteBGRect.setSize(Vector2f( 80, 100 ));

	

	scaleText.setFont(mainMenu->arial);
	scaleText.setCharacterSize(32);
	scaleText.setFillColor(Color::White);
	scaleText.setPosition(5, 30);

	Tileset *ts_kinScale = p_mainMenu->tilesetManager.GetTileset("Kin/stand_64x64.png", 64, 64);
	scaleSprite.setTexture(*ts_kinScale->texture);
	scaleSprite.setTextureRect(ts_kinScale->GetSubRect(0));

	PoiParams::font = &mainMenu->arial;
	
	mapPreviewTex = MainMenu::mapPreviewTexture;

	background = new EditorBG();

	for( int i = 0; i < 16; ++i )
	{
		fullBounds[i].color = COLOR_ORANGE;
		fullBounds[i].position = Vector2f( 0, 0 );
	}
	grabbedObject = NULL;
	zoomMultiple = 1;
	editMouseDownBox = false;
	editMouseDownMove = false;
	editMoveThresh = 5;
	editStartMove = false;
	//adding 5 for random distance buffer

	preScreenTex = mainMenu->preScreenTexture;
	
	//minAngle = .99;
	showPoints = false;
	messagePopup = NULL;
	errorPopup = NULL;
	confirm = NULL;
	progressBrush = new Brush();
	selectedBrush = new Brush();
	enemyQuad.setFillColor( Color( 0, 255, 0, 100 ) );
	moveActive = false;

	ActorGroup *playerGroup = new ActorGroup( "player" );
	groups["player"] = playerGroup;
	
	ParamsInfo playerPI("player", NULL,NULL, NULL,
		Vector2i(), Vector2i(22, 42), false, false, false, false, 1,
		GetTileset("Kin/jump_64x64.png", 64, 64));

	playerType = new ActorType(playerPI);
	types["player"] = playerType;

	player.reset( new PlayerParams( playerType, Vector2i( 0, 0 ) ) );
	groups["player"]->actors.push_back( player );
	

	grassSize = 128;//64;
	grassSpacing = -60;//-40;//-20;//-10;

	//extras

	//monitor,level, path, loop, 

	AddGeneralEnemies();
	AddW1Enemies();
	AddW2Enemies();
	AddW3Enemies();
	AddW4Enemies();
	AddW5Enemies();
	AddW6Enemies();
}

void EditSession::AddGeneralEnemies()
{
	AddExtraEnemy("poi", LoadParams<PoiParams>, MakeParamsGrounded<PoiParams>, MakeParamsAerial<PoiParams>,
		Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false);

	AddExtraEnemy("xbarrier", LoadParams<XBarrierParams>, NULL, MakeParamsAerial<XBarrierParams>,
		Vector2i(0, 0), Vector2i(64, 64),
		false, false, false, false, 1,
		GetTileset("Enemies/blocker_w1_192x192.png", 192, 192));

	AddExtraEnemy("camerashot", LoadParams<CameraShotParams>, NULL, MakeParamsAerial<CameraShotParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddExtraEnemy("key", LoadParams<KeyParams>, NULL, MakeParamsAerial<KeyParams>,
		Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false);

	AddExtraEnemy("shippickup", LoadParams<ShipPickupParams>, MakeParamsGrounded<ShipPickupParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Ship/shipleave_128x128.png", 128, 128));

	//AddExtraEnemy("dudgoal", LoadParams<ShipPickupParams>, MakeParamsGrounded<ShipPickupParams>, NULL,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Goal/goal_w01_a_288x320.png", 288, 320));

	AddExtraEnemy("shard", LoadParams<ShardParams>, NULL, MakeParamsAerial<ShardParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 3,
		GetTileset("Shard/shards_w1_192x192.png", 192, 192));

	AddExtraEnemy("ship", LoadParams<BasicAirEnemyParams>, NULL, MakeParamsAerial<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(864, 400), false, false, false, false, 1,
		GetTileset("Ship/ship_864x400.png", 864, 400));

	AddExtraEnemy("healthfly", LoadParams<BasicAirEnemyParams>, NULL, MakeParamsAerial<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1,
		GetTileset("Enemies/healthfly_64x64.png", 64, 64));

	AddExtraEnemy("extrascene", LoadParams<ExtraSceneParams>, NULL, MakeParamsAerial<ExtraSceneParams>,
		Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, 1,
		GetTileset("Enemies/bouncefloater_128x128.png", 128, 128));

	AddExtraEnemy("racefighttarget", LoadParams<RaceFightTargetParams>, NULL, MakeParamsAerial<RaceFightTargetParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddExtraEnemy("groundtrigger", LoadParams<GroundTriggerParams>, MakeParamsGrounded<GroundTriggerParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Ship/shipleave_128x128.png", 128, 128));

	AddExtraEnemy("airtrigger", LoadParams<AirTriggerParams>, NULL, MakeParamsAerial<AirTriggerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddExtraEnemy("flowerpod", LoadParams<FlowerPodParams>, MakeParamsGrounded<FlowerPodParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Momenta/momentaflower_128x128.png", 128, 128));

	AddExtraEnemy("nexus", LoadParams<NexusParams>, MakeParamsGrounded<NexusParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);
}

void EditSession::AddW1Enemies()
{
	AddBasicGroundWorldEnemy("goal", 1, Vector2i(0, -32), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Goal/goal_w01_a_288x320.png", 288, 320));

	AddWorldEnemy("blocker", 1, LoadParams<BlockerParams>, NULL, MakeParamsAerial<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/blocker_w1_192x192.png", 192, 192));

	AddBasicAerialWorldEnemy("patroller", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
		GetTileset("Enemies/patroller_icon_256x256.png", 256, 256));

	/*AddBasicAerialWorldEnemy("comboer", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	AddBasicAerialWorldEnemy("comboer", 1, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddBasicAerialWorldEnemy("splitcomboer", 1, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128),1);



	/*AddBasicAerialWorldEnemy("jugglercatcher", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	GetTileset("Enemies/jugglercatcher_128x128.png", 128, 128));*/


	AddBasicGroundWorldEnemy("crawler", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/crawler_160x160.png", 160, 160));

	AddBasicGroundWorldEnemy("shroom", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/shroom_192x192.png", 192, 192));

	AddBasicGroundWorldEnemy("basicturret", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/basicturret_128x80.png", 128, 80));

	AddBasicAerialWorldEnemy("airdasher", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/dasher_208x144.png", 208, 144));

	AddWorldEnemy("bosscrawler", 1, LoadParams<BossCrawlerParams>, MakeParamsGrounded<BossCrawlerParams>, NULL,
		Vector2i(0, 0), Vector2i(128, 144), false, false, false, false, 1,
		GetTileset("Bosses/Crawler/crawler_queen_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("booster", 1, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/Booster_512x512.png", 512, 512));

	AddWorldEnemy("spring", 1, LoadParams<SpringParams>, NULL, MakeParamsAerial<SpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_256x256.png", 256, 256));
}

void EditSession::AddW2Enemies()
{
	AddBasicGroundWorldEnemy("greengoal", 2, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Goal/goal_w02_a_288x256.png", 288, 256));

	AddWorldEnemy("greenblocker", 2, LoadParams<BlockerParams>, NULL, MakeParamsAerial<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/blocker_w2_192x192.png", 192, 192));

	AddWorldEnemy("downgravityjuggler", 2, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddWorldEnemy("upgravityjuggler", 2, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));


	AddBasicAerialWorldEnemy("airdashjuggler", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	/*AddBasicAerialWorldEnemy("gravdowncomboer", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	/*AddBasicAerialWorldEnemy("gravupcomboer", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	AddBasicAerialWorldEnemy("bat", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
		GetTileset("Enemies/bat_144x176.png", 144, 176));

	AddBasicGroundWorldEnemy("curveturret", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/curveturret_144x96.png", 144, 96));

	AddBasicGroundWorldEnemy("poisonfrog", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/frog_80x80.png", 80, 80));

	AddBasicGroundWorldEnemy("stagbeetle", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/stag_idle_256x176.png", 256, 176));

	AddBasicGroundWorldEnemy("gravityfaller", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/gravity_faller_128x128.png", 128, 128));

	AddBasicAerialWorldEnemy("gravityincreaser", 2, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/grav_increase_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("gravitydecreaser", 2, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/grav_decrease_256x256.png", 256, 256));

	AddWorldEnemy("gravityspring", 2, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddWorldEnemy("bossbird", 2, LoadParams<BossBirdParams>, NULL, MakeParamsAerial<BossBirdParams>,
		Vector2i(0, 0), Vector2i(64, 64), false, false, false, false);
}

void EditSession::AddW3Enemies()
{
	AddBasicAerialWorldEnemy("bouncefloater", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/bouncefloater_128x128.png", 128, 128));

	/*AddBasicAerialWorldEnemy("bouncecomboer", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	AddWorldEnemy("bouncejuggler", 3, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddWorldEnemy("bouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddWorldEnemy("airbouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	AddBasicAerialWorldEnemy("upbouncebooster", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/Booster_512x512.png", 512, 512));

	AddBasicAerialWorldEnemy("omnibouncebooster", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/Booster_512x512.png", 512, 512));
	/*AddWorldEnemy("redirectspring", 3, LoadParams<BounceSpringParams>, NULL, MakeParamsAerial<BounceSpringParams>,
	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddWorldEnemy("reflectspring", 3, LoadParams<BounceSpringParams>, NULL, MakeParamsAerial<BounceSpringParams>,
	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));*/

	//AddWorldEnemy("bouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("pulser", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3);

	AddBasicGroundWorldEnemy("badger", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/badger_192x128.png", 192, 128));

	AddBasicGroundWorldEnemy("roadrunner", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/roadrunner_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("owl", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);


	AddBasicGroundWorldEnemy("cactus", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	/*AddWorldEnemy("cactus", 3, LoadParams<CactusParams>, MakeParamsGrounded<CactusParams>, NULL,
	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/

	AddWorldEnemy("bosscoyote", 3, LoadParams<BossCoyoteParams>, NULL, MakeParamsAerial<BossCoyoteParams>,
		Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
}

void EditSession::AddW4Enemies()
{
	AddWorldEnemy("rail", 4, LoadParams<RailParams>, NULL, MakeParamsAerial<RailParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false,
		3, GetTileset("Enemies/rail_64x64.png", 64, 64));

	AddWorldEnemy("grindrail", 4, LoadParams<RailParams>, NULL, MakeParamsAerial<RailParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 
		3, GetTileset("Enemies/rail_64x64.png", 64, 64));

	//AddBasicAerialWorldEnemy("teleporter", 4, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddWorldEnemy("teleporter", 4, LoadParams<TeleporterParams>, NULL, MakeParamsAerial<TeleporterParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	AddWorldEnemy("onewayteleporter", 4, LoadParams<TeleporterParams>, NULL, MakeParamsAerial<TeleporterParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	AddWorldEnemy("grindjugglercw", 4, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddWorldEnemy("grindjugglerccw", 4, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddWorldEnemy("groundedgrindjugglercw", 4, LoadParams<GroundedJugglerParams>, MakeParamsGrounded<GroundedJugglerParams>, NULL,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	AddWorldEnemy("groundedgrindjugglerccw", 4, LoadParams<GroundedJugglerParams>, MakeParamsGrounded<GroundedJugglerParams>, NULL,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	AddBasicRailWorldEnemy("railtest", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/shroom_192x192.png", 192, 192));

	AddBasicGroundWorldEnemy("spider", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/crawler_160x160.png", 160, 160));
	//AddWorldEnemy("spider", 4, LoadParams<SpiderParams>, MakeParamsGrounded<SpiderParams>, NULL,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddBasicAerialWorldEnemy("turtle", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/turtle_80x64.png", 80, 64));

	AddBasicGroundWorldEnemy("cheetah", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/shroom_192x192.png", 192, 192));

	//AddWorldEnemy("cheetah", 4, LoadParams<CheetahParams>, MakeParamsGrounded<CheetahParams>, NULL,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddWorldEnemy("coral", 4, LoadParams<CoralParams>, NULL, MakeParamsAerial<CoralParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddWorldEnemy("bosstiger", 4, LoadParams<BossTigerParams>, NULL, MakeParamsAerial<BossTigerParams>,
		Vector2i(0, 0), Vector2i(64, 64), false, false, false, false);

	
}

void EditSession::AddW5Enemies()
{
	AddWorldEnemy("hungrycomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddWorldEnemy("hungryreturncomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128),2);

	AddWorldEnemy("relativecomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	AddWorldEnemy("relativecomboerdetach", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128),1);

	AddBasicAerialWorldEnemy("swarm", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/swarm_pod_128x128.png", 128, 128));

	//AddWorldEnemy("swarm", 5, LoadParams<SwarmParams>, NULL, MakeParamsAerial<SwarmParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddBasicAerialWorldEnemy("shark", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/shark_circle_256x256.png", 256, 256));

	/*AddWorldEnemy("shark", 5, LoadParams<SharkParams>, NULL, MakeParamsAerial<SharkParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/


	AddBasicGroundWorldEnemy("growingtree", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/sprout_160x160.png", 160, 160));

	//AddWorldEnemy("overgrowth", 5, LoadParams<OvergrowthParams>, MakeParamsGrounded<OvergrowthParams>, NULL,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddBasicAerialWorldEnemy("ghost", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/plasmid_192x192.png", 192, 192));

	//AddWorldEnemy("ghost", 5, LoadParams<GhostParams>, NULL, MakeParamsAerial<GhostParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddWorldEnemy("bossgator", 5, LoadParams<BossGatorParams>, NULL, MakeParamsAerial<BossGatorParams>,
		Vector2i(0, 0), Vector2i(32, 128), false, false, false, false);


	//name == "hungrycomboer" || name == "hungryattackcomboer"
}

void EditSession::AddW6Enemies()
{
	//w6

	AddWorldEnemy("swinglaunchercw", 6, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));
	AddWorldEnemy("swinglauncherccw", 6, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("wiretarget", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddWorldEnemy("wirejuggler", 6, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128),1);

	AddBasicAerialWorldEnemy("specter", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/specter_256x256.png", 256, 256));

	AddWorldEnemy("narwhal", 6, LoadParams<NarwhalParams>, NULL, MakeParamsAerial<NarwhalParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddWorldEnemy("copycat", 6, LoadParams<CopycatParams>, NULL, MakeParamsAerial<CopycatParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddBasicAerialWorldEnemy("gorilla", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/gorilla_320x256.png", 320, 256));

	AddWorldEnemy("bossskeleton", 6, LoadParams<BossSkeletonParams>, NULL, MakeParamsAerial<BossSkeletonParams>,
		Vector2i(0, 0), Vector2i(32, 128), false, false, false, false);
}

void EditSession::AddBasicGroundWorldEnemy(const std::string &name, int w,
	Vector2i &off, Vector2i &size, bool w_mon,
	bool w_level, bool w_path, bool w_loop, int p_numLevels, Tileset *ts, int tileIndex)
{
	worldEnemyNames[w - 1].push_back(ParamsInfo(name, LoadParams<BasicGroundEnemyParams>, MakeParamsGrounded<BasicGroundEnemyParams>, NULL, off, size,
		w_mon, w_level, w_path, w_loop, p_numLevels, ts, tileIndex));
}

void EditSession::AddBasicRailWorldEnemy(const std::string &name, int w,
	Vector2i &off, Vector2i &size, bool w_mon,
	bool w_level, bool w_path, bool w_loop, int p_numLevels, Tileset *ts, int tileIndex)
{
	worldEnemyNames[w - 1].push_back(ParamsInfo(name, LoadParams<BasicRailEnemyParams>, NULL, NULL, off, size,
		w_mon, w_level, w_path, w_loop, p_numLevels, ts, tileIndex));
	worldEnemyNames[w - 1].back().pmRail = MakeParamsRailed<BasicRailEnemyParams>;
}

void EditSession::AddBasicAerialWorldEnemy(const std::string &name, int w,
	sf::Vector2i &off,
	sf::Vector2i &size,
	bool w_mon,
	bool w_level,
	bool w_path,
	bool w_loop,
	int p_numLevels,
	Tileset *ts,
	int tileIndex)
{
	worldEnemyNames[w - 1].push_back(ParamsInfo(name, LoadParams<BasicAirEnemyParams>, NULL, MakeParamsAerial<BasicAirEnemyParams>, off, size,
		w_mon, w_level, w_path, w_loop, p_numLevels, ts, tileIndex));
}

void EditSession::AddWorldEnemy( const std::string &name, int w, ParamsLoader *pLoader, 
	ParamsMaker* pmGround, ParamsMaker *pmAir,
	Vector2i &off, Vector2i &size, bool w_mon, 
	bool w_level, bool w_path, bool w_loop, int p_numLevels, Tileset *ts, int tileIndex )
{
	worldEnemyNames[w - 1].push_back(ParamsInfo( name, pLoader, pmGround, pmAir, off, size, 
		w_mon, w_level, w_path, w_loop, p_numLevels, ts, tileIndex ));
}

void EditSession::AddExtraEnemy(const std::string &name, ParamsLoader *pLoader, 
	ParamsMaker *pmGround, ParamsMaker *pmAir,
	Vector2i &off, Vector2i &size, bool w_mon,
	bool w_level, bool w_path, bool w_loop, int p_numLevels, Tileset *ts, int tileIndex)
{
	extraEnemyNames.push_back(ParamsInfo(name, pLoader, pmGround, pmAir, off, size,
		w_mon, w_level, w_path, w_loop, p_numLevels, ts, tileIndex));
}

TerrainPolygon *EditSession::GetPolygon(int index, int &edgeIndex )
{
	TerrainPolygon* terrain = NULL;
	if (index == -1)
	{
		terrain = inversePolygon.get();
	}
	else
	{
		int testIndex = 0;
		list<PolyPtr>::iterator it = polygons.begin();
		if (inversePolygon != NULL)
			++it;

		for (; it != polygons.end(); ++it)
		{
			if (testIndex == index)
			{
				terrain = (*it).get();
				break;
			}
			testIndex++;
		}
	}

	if (terrain == NULL)
		assert(0 && "failure terrain indexing goal");

	if (edgeIndex == terrain->numPoints - 1)
		edgeIndex = 0;
	else
		edgeIndex++;

	return terrain;
}

TerrainRail *EditSession::GetRail(int index, int &edgeIndex)
{
	TerrainRail* rail = NULL;

	int pSize = polygons.size();
	if (inversePolygon != NULL)
	{
		pSize--;
	}

	int realIndex = index - pSize;

	int testIndex = 0;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if (testIndex == realIndex)
		{
			rail = (*it).get();
			break;
		}
		++testIndex;
	}

	if (rail == NULL)
		assert(0 && "failure rail indexing");

	return rail;
	//if (edgeIndex == rails->numPoints - 1)
	//	edgeIndex = 0;
	//else
	//	edgeIndex++;

	//return terrain;
}

EditSession::~EditSession()
{
	delete graph;


	delete background;

	polygonInProgress.reset();

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		(*it).reset();
	}

	for (int i = 0; i < 2; ++i)
	{
		auto & polyList = GetCorrectPolygonList(i);
		for (auto it = polyList.begin(); it != polyList.end(); ++it)
		{
			(*it).reset();
		}
	}
	


	delete progressBrush;
	delete selectedBrush;

	for (auto it = allPopups.begin(); it != allPopups.end(); ++it)
	{
		delete (*it);
	}


	for (auto it = types.begin(); it != types.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete(*it).second;
	}

	delete[] decorTileIndexes;

	for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
	{
		if (terrainTextures[i] != NULL)
			delete terrainTextures[i];
	}
}

void EditSession::SnapPointToGraph(Vector2f &p, int gridSize )
{
	V2d pCopy(p);
	SnapPointToGraph(pCopy, gridSize);
	p = Vector2f(pCopy);
}

void EditSession::SnapPointToGraph(V2d &p, int gridSize)
{
	int adjX, adjY;

	p.x /= gridSize;
	p.y /= gridSize;

	if (p.x > 0)
		p.x += .5f;
	else if (p.x < 0)
		p.x -= .5f;

	if (p.y > 0)
		p.y += .5f;
	else if (p.y < 0)
		p.y -= .5f;

	adjX = ((int)p.x) * gridSize;
	adjY = ((int)p.y) * gridSize;

	p = V2d(adjX, adjY);
}

bool EditSession::IsKeyPressed(int k)
{
	return mainMenu->IsKeyPressed(k);
}

bool EditSession::IsMousePressed(int m)
{
	return mainMenu->IsMousePressed(m);
}

void EditSession::Draw()
{
	preScreenTex->clear();
	preScreenTex->setView(view);

	background->Draw(preScreenTex);

	preScreenTex->draw(border, 8, sf::Lines);

	DrawDecorBehind();

	DrawGates();

	DrawPolygons();

	DrawRails();
	//DrawPolygonInProgress();

	DrawDecorBetween();
	DrawActors();
	

	if (mode == PASTE)
	{
		copiedBrush->Draw(preScreenTex);
	}
}

void EditSession::UpdateFullBounds()
{
	int boundRectWidth = 5 * zoomMultiple;
		//top rect
	fullBounds[0].position = Vector2f( leftBound, topBound - boundRectWidth );
	fullBounds[1].position = Vector2f( leftBound + boundWidth, topBound - boundRectWidth );
	fullBounds[2].position = Vector2f( leftBound + boundWidth, topBound + boundRectWidth );
	fullBounds[3].position = Vector2f( leftBound, topBound + boundRectWidth );

	//right rect
	fullBounds[4].position = Vector2f( ( leftBound + boundWidth ) - boundRectWidth, topBound );
	fullBounds[5].position = Vector2f( ( leftBound + boundWidth ) + boundRectWidth, topBound );
	fullBounds[6].position = Vector2f( ( leftBound + boundWidth ) + boundRectWidth, topBound + boundHeight );
	fullBounds[7].position = Vector2f( ( leftBound + boundWidth ) - boundRectWidth, topBound + boundHeight );

	//bottom rect
	fullBounds[8].position = Vector2f(0, 0);//Vector2f( leftBound, ( topBound + boundHeight ) - boundRectWidth );
	fullBounds[9].position = Vector2f(0, 0);//Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) - boundRectWidth );
	fullBounds[10].position = Vector2f(0, 0);//Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) + boundRectWidth );
	fullBounds[11].position = Vector2f(0, 0);//Vector2f( leftBound, ( topBound + boundHeight ) + boundRectWidth );

	//left rect
	fullBounds[12].position = Vector2f( leftBound - boundRectWidth, topBound );
	fullBounds[13].position = Vector2f( leftBound + boundRectWidth, topBound );
	fullBounds[14].position = Vector2f( leftBound + boundRectWidth, topBound + boundHeight );
	fullBounds[15].position = Vector2f( leftBound - boundRectWidth, topBound + boundHeight );
}

bool EditSession::ReadDecor(std::ifstream &is)
{
	int numDecorImages;
	is >> numDecorImages;

	for (int i = 0; i < numDecorImages; ++i)
	{
		string dName;
		is >> dName;
		int dLayer;
		is >> dLayer;

		Vector2f dPos;
		is >> dPos.x;
		is >> dPos.y;

		float dRot;
		is >> dRot;

		Vector2f dScale;
		is >> dScale.x;
		is >> dScale.y;

		int dTile;
		is >> dTile;

		Sprite dSpr;
		dSpr.setScale(dScale);
		dSpr.setRotation(dRot);
		dSpr.setPosition(dPos);

		//string fullDName = dName + string(".png");
		Tileset *ts = decorTSMap[dName];
		dSpr.setTexture(*ts->texture);
		dSpr.setTextureRect(ts->GetSubRect(dTile));
		dSpr.setOrigin(dSpr.getLocalBounds().width / 2, dSpr.getLocalBounds().height / 2);
		dSpr.setColor(Color(255, 255, 255, 100));
		//dSpr.setTexture do this after dinner


		EditorDecorPtr dec(new EditorDecorInfo(dSpr, dLayer, dName, dTile));
		if (dLayer > 0)
		{
			dec->myList = &decorImagesBehindTerrain;
			//decorImagesBehindTerrain.sort(CompareDecorInfo);
			//decorImagesBehindTerrain.push_back(dec);
		}
		else if (dLayer < 0)
		{
			dec->myList = &decorImagesFrontTerrain;
			//decorImagesFrontTerrain.push_back(dec);
		}
		else if (dLayer == 0)
		{
			dec->myList = &decorImagesBetween;
			//decorImagesBetween.push_back(dec);
		}

		CreateDecorImage(dec);
	}

	return true;
}

bool EditSession::ReadTerrain(ifstream &is )
{
	string hasBorderPolyStr;
	is >> hasBorderPolyStr;
	bool hasBorderPoly;
	bool hasReadBorderPoly;
	if (hasBorderPolyStr == "borderpoly")
	{
		hasBorderPoly = true;
		hasReadBorderPoly = false;
	}
	else if (hasBorderPolyStr == "no_borderpoly")
	{
		hasBorderPoly = false;
		hasReadBorderPoly = true;
	}
	else
	{
		cout << hasBorderPolyStr << endl;
		assert(0 && "what is this string?");
	}
	int numPoints = mapHeader.numVertices;

	while (numPoints > 0)
	{
		PolyPtr poly(new TerrainPolygon(&grassTex));


		int matWorld;
		int matVariation;
		is >> matWorld;
		is >> matVariation;

		poly->terrainWorldType = (TerrainPolygon::TerrainWorldType)matWorld;
		poly->terrainVariation = matVariation;

		if (!hasReadBorderPoly)
		{
			poly->inverse = true;
			inversePolygon = poly;
			hasReadBorderPoly = true;
		}

		polygons.push_back(poly);

		int polyPoints;
		is >> polyPoints;


		numPoints -= polyPoints;
		int x, y;
		for (int i = 0; i < polyPoints; ++i)
		{
			is >> x;
			is >> y;
			//is >> special;
			poly->AddPoint(new TerrainPoint(Vector2i(x, y), false));
		}


		int edgesWithSegments;
		is >> edgesWithSegments;


		list<GrassSeg> segments;
		for (int i = 0; i < edgesWithSegments; ++i)
		{
			int edgeIndex;
			is >> edgeIndex;

			int numSegments;
			is >> numSegments;

			for (int j = 0; j < numSegments; ++j)
			{
				int index;
				is >> index;
				int reps;
				is >> reps;
				segments.push_back(GrassSeg(edgeIndex, index, reps));

			}
		}

		poly->Finalize();

		int grassIndex = 0;
		VertexArray &grassVa = *poly->grassVA;
		int numEdges = poly->numPoints;
		int *indexArray = new int[numEdges];
		int edgeIndex = 0;

		int iai = 0;

		for (TerrainPoint *polyCurr = poly->pointStart; polyCurr != NULL; polyCurr = polyCurr->next)
		{
			indexArray[edgeIndex] = grassIndex;

			Vector2i next;

			TerrainPoint *temp = polyCurr->next;
			if (temp == NULL)
			{
				next = poly->pointStart->pos;
			}
			else
			{
				next = temp->pos;
			}

			V2d v0(polyCurr->pos.x, polyCurr->pos.y);
			V2d v1(next.x, next.y);

			//double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );
			bool rem;
			int num = poly->GetNumGrass(polyCurr, rem);//floor( remainder ) + 1;

			grassIndex += num;

			++edgeIndex;
		}

		for (list<GrassSeg>::iterator it = segments.begin(); it != segments.end(); ++it)
		{
			int vaIndex = indexArray[(*it).edgeIndex];

			for (int extra = 0; extra <= (*it).reps; ++extra)
			{
				grassVa[(vaIndex + (*it).index + extra) * 4].color.a = 255;
				grassVa[(vaIndex + (*it).index + extra) * 4 + 1].color.a = 255;
				grassVa[(vaIndex + (*it).index + extra) * 4 + 2].color.a = 255;
				grassVa[(vaIndex + (*it).index + extra) * 4 + 3].color.a = 255;
			}
		}

		delete[] indexArray;
	}
	return true;
}

bool EditSession::ReadBGTerrain(std::ifstream &is)
{
	int bgPlatformNum0;
	is >> bgPlatformNum0;
	for (int i = 0; i < bgPlatformNum0; ++i)
	{
		PolyPtr poly(new TerrainPolygon(&grassTex));
		polygons.push_back(poly);

		int matWorld;
		int matVariation;
		is >> matWorld;
		is >> matVariation;

		poly->terrainWorldType = (TerrainPolygon::TerrainWorldType)matWorld;
		poly->terrainVariation = matVariation;

		int polyPoints;
		is >> polyPoints;

		for (int j = 0; j < polyPoints; ++j)
		{
			int x, y, special;
			is >> x;
			is >> y;
			poly->AddPoint(new TerrainPoint(Vector2i(x, y), false));
		}

		poly->Finalize();
		poly->SetLayer(1);
		//no grass for now
	}
	return true;
}

bool EditSession::ReadRails(std::ifstream &is)
{
	int numRails;
	is >> numRails;
	for (int i = 0; i < numRails; ++i)
	{
		RailPtr rail(new TerrainRail());
		rails.push_back(rail);

		rail->Load(is);
	}
	return true;
}

bool EditSession::ReadSpecialTerrain(std::ifstream &is)
{
	int specialPolyNum;
	is >> specialPolyNum;

	for (int i = 0; i < specialPolyNum; ++i)
	{
		PolyPtr poly(new TerrainPolygon(&grassTex));

		int matWorld;
		int matVariation;
		is >> matWorld;
		is >> matVariation;

		poly->SetMaterialType(matWorld, matVariation);
		int polyPoints;
		is >> polyPoints;

		GetCorrectPolygonList(poly.get()).push_back(poly);

		for (int j = 0; j < polyPoints; ++j)
		{
			int x, y, special;
			is >> x;
			is >> y;
			poly->AddPoint(new TerrainPoint(Vector2i(x, y), false));
		}

		poly->Finalize();
	}

	return true;
}

bool EditSession::ReadActors(std::ifstream &is)
{
	//enemies here
	int numGroups;
	is >> numGroups;
	cout << "num groups " << numGroups << endl;
	for (int i = 0; i < numGroups; ++i)
	{
		string groupName;
		is >> groupName;

		int numActors;
		is >> numActors;

		ActorGroup *gr = new ActorGroup(groupName);
		groups[groupName] = gr;

		for (int j = 0; j < numActors; ++j)
		{
			string typeName;
			is >> typeName;

			//ActorParams *a; //= new ActorParams;
			ActorPtr a(NULL);



			ActorType *at = NULL;
			cout << "typename: " << typeName << endl;
			if (types.count(typeName) == 0)
			{
				cout << "TYPENAME: " << typeName << endl;
				assert(false && "bad typename");
			}
			else
			{
				at = types[typeName];
			}

			at->LoadEnemy(is, a);

			gr->actors.push_back(a);
			a->group = gr;
		}
	}

	return true;
}

bool EditSession::ReadGates(std::ifstream &is)
{
	int numGates;
	is >> numGates;
	cout << "numgates: " << numGates << endl;
	for (int i = 0; i < numGates; ++i)
	{
		int gType;
		int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
		int numKeysRequired = -1;

		is >> gType;
		//is >> numKeysRequired;
		is >> poly0Index;
		is >> vertexIndex0;
		is >> poly1Index;
		is >> vertexIndex1;

		int testIndex = 0;
		PolyPtr terrain0(NULL);
		PolyPtr terrain1(NULL);
		bool first = true;

		if (poly0Index == -1)
		{
			terrain0 = inversePolygon;
		}
		if (poly1Index == -1)
		{
			terrain1 = inversePolygon;
		}
		for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
		{
			if ((*it)->inverse) continue;

			if (terrain0 != NULL && terrain1 != NULL)
				break;

			if (testIndex == poly0Index && terrain0 == NULL)
			{
				terrain0 = (*it);

				if (first)
					first = false;
				else
					break;
			}
			if (testIndex == poly1Index && terrain1 == NULL)
			{
				terrain1 = (*it);

				if (first)
					first = false;
				else
					break;
			}
			testIndex++;
		}
		if (terrain0 == NULL || terrain1 == NULL)
		{
			int zzzerwr = 56;
		}
		//PolyPtr poly(  new TerrainPolygon( &grassTex ) );
		GateInfoPtr gi(new GateInfo);
		//GateInfo *gi = new GateInfo;
		gi->numKeysRequired = numKeysRequired;
		gi->poly0 = terrain0;
		gi->poly1 = terrain1;
		gi->vertexIndex0 = vertexIndex0;
		gi->vertexIndex1 = vertexIndex1;
		gi->type = (Gate::GateType)gType;
		gi->edit = this;

		if (gType == Gate::SHARD)
		{
			int sw, si;
			is >> sw;
			is >> si;
			gi->SetShard(sw, si);
		}

		int index = 0;
		for (TerrainPoint *curr = gi->poly0->pointStart; curr != NULL; curr = curr->next)
		{
			if (index == vertexIndex0)
			{
				gi->point0 = curr;
				curr->gate = gi;
				break;
			}
			++index;
		}

		index = 0;
		//cout << "poly1: " << gi->poly1 << endl;
		for (TerrainPoint *curr = gi->poly1->pointStart; curr != NULL; curr = curr->next)
		{
			if (index == vertexIndex1)
			{
				gi->point1 = curr;
				curr->gate = gi;
				break;
			}
			++index;
		}

		gi->UpdateLine();
		gates.push_back(gi);
	}
	return true;
}

bool EditSession::ReadPlayer(ifstream &is)
{
	is >> player->position.x;
	is >> player->position.y;

	player->image.setPosition(player->position.x, player->position.y);
	player->SetBoundingQuad();

	return true;
}

bool EditSession::ReadHeader(std::ifstream &is)
{
	MapHeader *mh = MapSelectionMenu::ReadMapHeader(is);

	mapHeader = *mh;

	envName = mh->envName;

	mapHeader.numVertices = mh->numVertices;

	envWorldType = mh->envWorldType;

	leftBound = mh->leftBounds;
	topBound = mh->topBounds;
	boundWidth = mh->boundsWidth;
	boundHeight = mh->boundsHeight;

	drainSeconds = mh->drainSeconds;

	Background::SetupFullBG(envName, *this, background->currBackground, background->scrollingBackgrounds);


	bossType = mh->bossFightType;

	delete mh;
	mh = NULL;

	UpdateFullBounds();

	return true;
}

bool EditSession::OpenFile()
{
	ifstream is;
	is.open( currentFile );

	if( is.is_open() )
	{
		ReadHeader(is);
		ReadDecor(is);
		ReadPlayer(is);
		ReadTerrain(is);
		ReadSpecialTerrain(is);
		ReadBGTerrain(is);
		ReadRails(is);
		ReadActors(is);
		ReadGates(is);

		is.close();
	}
	else
	{

		//new file
		cout << "filename: " << currentFile << endl;
		assert( false && "error getting file to edit " );

		return false;
	}

	//grassTex.loadFromFile( "Resources/Env/placeholdergrass_22x22.png" );
	grassTex.loadFromFile("Resources/Env/grass_128x128.png");

	return true;
	
}

void EditSession::WriteMapHeader(ofstream &of)
{
	mapHeader.leftBounds = leftBound;
	mapHeader.topBounds = topBound;
	mapHeader.boundsWidth = boundWidth;
	mapHeader.boundsHeight = boundHeight;
	mapHeader.bossFightType = bossType;

	mapHeader.shardNameList.clear();
	ShardParams *sp = NULL;
	int numShards = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		std::list<ActorPtr> &aList = (*it).second->actors;
		for (auto ait = aList.begin(); ait != aList.end(); ++ait)
		{
			if ((*ait)->type->info.name == "shard")
			{
				numShards++;
				sp = (ShardParams*)(ait->get());
				mapHeader.shardNameList.push_back(sp->shardStr);
			}
		}
	}

	mapHeader.numShards = numShards;

	mapHeader.drainSeconds = drainSeconds;

	mapHeader.envName = envName;

	mapHeader.envWorldType = envWorldType;

	mapHeader.Save(of);
}

void EditSession::WriteDecor(ofstream &of)
{
	int totalDecor = 0;
	totalDecor += decorImagesBehindTerrain.size() + decorImagesBetween.size() + decorImagesFrontTerrain.size();

	of << totalDecor << endl;

	for (auto it = decorImagesBehindTerrain.begin(); it != decorImagesBehindTerrain.end(); ++it)
	{
		(*it)->WriteFile(of);
	}

	for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
	{
		(*it)->WriteFile(of);
	}

	for (auto it = decorImagesFrontTerrain.begin(); it != decorImagesFrontTerrain.end(); ++it)
	{
		(*it)->WriteFile(of);
	}
}

void EditSession::WriteInversePoly(std::ofstream &of)
{
	if (inversePolygon != NULL)
	{
		of << "borderpoly" << endl;
		inversePolygon->writeIndex = -1;//writeIndex;

		inversePolygon->WriteFile(of);
	}
	else
	{
		of << "no_borderpoly" << endl;
	}
}

void EditSession::WriteSpecialPolygons(std::ofstream &of)
{
	int numSpecialPolys = waterPolygons.size();
	of << numSpecialPolys << endl;

	for (auto it = waterPolygons.begin(); it != waterPolygons.end(); ++it)
	{
		(*it)->WriteFile(of);
	}
}

void EditSession::WritePolygons(std::ofstream &of, int bgPlatCount0)
{
	int writeIndex = 0;

	cout << "writing to file with : " << polygons.size() << " polygons" << endl;
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		if ((*it)->inverse) continue;

		//cout << "layerrr: " << (*it)->layer << ", pathsize: " << (*it)->path.size() << endl;
		if ((*it)->layer == 0 && (*it)->path.size() < 2)
		{
			cout << "writing polygon of write index: " << writeIndex << endl;
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			(*it)->WriteFile(of);
		}
	}

	tempWriteIndex = writeIndex;


	WriteSpecialPolygons(of);
	//of << "0" << endl; //writing the number of moving platforms. remove this when possible


	//write moving platorms
	/*writeIndex = 0;
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		if ((*it)->layer == 0 && (*it)->path.size() >= 2)
		{
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " "
				<< (*it)->terrainVariation << endl;

			of << (*it)->numPoints << endl;

			for (TerrainPoint *pcurr = (*it)->pointStart; pcurr != NULL; pcurr = pcurr->next)
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl;
			}


			of << (*it)->path.size() - 1 << endl;

			list<Vector2i>::iterator pathit = (*it)->path.begin();
			++pathit;

			for (; pathit != (*it)->path.end(); ++pathit)
			{
				of << (*pathit).x << " " << (*pathit).y << endl;
			}
		}
	}*/

	of << bgPlatCount0 << endl;

	writeIndex = 0;
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		if ((*it)->inverse) continue;

		if ((*it)->layer == 1)// && (*it)->path.size() < 2 )
		{
			//writeindex doesnt matter much for these for now
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " "
				<< (*it)->terrainVariation << endl;

			of << (*it)->numPoints << endl;

			for (TerrainPoint *pcurr = (*it)->pointStart; pcurr != NULL; pcurr = pcurr->next)
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl; // << " " << (int)(*it2).special << endl;
			}

			//WriteGrass( (*it), of );
		}
	}
}

void EditSession::WriteActors(ofstream &of)
{
	//minus 1 because of the player group
	of << groups.size() - 1 << endl;
	//write the stuff for goals and remove them from the enemy stuff

	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		ActorGroup *ag = (*it).second;
		if (ag->name == "player")
			continue;

		//cout << "group size: " << ag->actors.size() << endl;
		of << ag->name << " " << ag->actors.size() << endl;
		for (list<ActorPtr>::iterator it = ag->actors.begin(); it != ag->actors.end(); ++it)
		{
			if ((*it)->type == types["poi"])
			{
				(*it)->WriteFile(of);
			}
		}
	}

	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		ActorGroup *ag = (*it).second;
		if (ag->name == "player")
			continue;

		//cout << "group size: " << ag->actors.size() << endl;
		//of << ag->name << " " << ag->actors.size() << endl;
		for (list<ActorPtr>::iterator it = ag->actors.begin(); it != ag->actors.end(); ++it)
		{
			if ((*it)->type != types["poi"])
			{
				(*it)->WriteFile(of);
			}
		}
		//(*it).second->WriteFile( of );

	}
}

void EditSession::WriteGates(ofstream &of)
{
	of << gates.size() << endl;
	for (list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->WriteFile(of);
	}
}

void EditSession::WriteRails(ofstream &of)
{
	int writeIndex = tempWriteIndex;

	of << rails.size() << endl;

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->writeIndex = writeIndex;
		++writeIndex;

		(*it)->WriteFile(of);
		
	}
}

void EditSession::WriteFile(string fileName)
{
	bool hasGoal = false;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		ActorGroup *group = (*it).second;
		for( list<ActorPtr>::iterator it2 = group->actors.begin(); it2 != group->actors.end(); ++it2 )
		{
			if( (*it2)->type->IsGoalType() )
			{
				hasGoal = true;
				break;
			}
		}
	}

	/*if( !hasGoal )
	{
		MessagePop( "Map not saved because no goal is in place. \nPlease add it from the CREATE ENEMIES mode." );
		cout << "you need to place a goal in the map. file not written to!. add a popup to this alert later"
			<< endl;
		return;
	}*/

	ofstream of;
	of.open(fileName);

	WriteMapHeader(of);

	int pointCount = 0;
	int bgPlatCount0 = 0;

	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if ((*it)->layer == 0)
		{
			pointCount += (*it)->numPoints;
		}
		else if ((*it)->layer == 1)
		{
			bgPlatCount0++;
		}
	}
	of << pointCount << endl;

	WriteDecor(of);

	of << player->position.x << " " << player->position.y << endl;

	WriteInversePoly(of);

	WritePolygons(of, bgPlatCount0);

	WriteRails(of);
	//going to use this for number of rails
	//of << "0" << endl; //writing the number of static lights for consistency. Remove this when possible.

	WriteActors(of);

	WriteGates(of);
	

	CreatePreview(Vector2i( 1920 / 2 - 48, 1080 / 2 - 48 ));
	//CreatePreview(Vector2i(960 * 1.25f, 540 * ));

	//enemies here


}



void EditSession::WriteGrass( PolyPtr poly, ofstream &of )
{
	int edgesWithSegments = 0;

	VertexArray &grassVa = *poly->grassVA;

	int edgeIndex = 0;
	int i = 0;
	list<list<GrassSeg>> grassListList;
	for( TerrainPoint *curr = poly->pointStart;  curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = poly->pointStart->pos;
		}
		else
		{
			next = temp->pos;
		}

		V2d v0( curr->pos.x, curr->pos.y );
		V2d v1( next.x, next.y );

		bool rem;
		int num = poly->GetNumGrass(curr, rem);//floor( remainder ) + 1;

		grassListList.push_back( list<GrassSeg>() );

		list<GrassSeg> &grassList = grassListList.back();
		
		GrassSeg *gPtr = NULL;
		bool hasGrass = false;
		for( int j = 0; j < num; ++j )
		{
			//V2d pos = v0 + (v1 - v0) * ((double)(j )/ num);

			if( grassVa[i*4].color.a == 255 || grassVa[i*4].color.a == 254 )
			{
				hasGrass = true;
				if( gPtr == NULL )//|| (j == num - 1 && rem ))
				{
					grassList.push_back( GrassSeg( edgeIndex, j, 0 ) );
					gPtr = &grassList.back();
				}
				else
				{
					grassList.back().reps++;
				}
			}
			else
			{
				if( gPtr != NULL )
					gPtr = NULL;
			}
			
			++i;
		}

		if( hasGrass )
		{
			++edgesWithSegments;
		}

		++edgeIndex;

	}

	//cout << "saving edges with segments: " << edgesWithSegments << endl;
	of << edgesWithSegments << endl;

	for( list<list<GrassSeg>>::iterator it = grassListList.begin(); it != grassListList.end(); ++it )
	{
		int numSegments = (*it).size();

		if( numSegments > 0 )
		{
			int edgeIndex = (*it).front().edgeIndex;
			of << edgeIndex << " " << numSegments << endl;

			for( list<GrassSeg>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 )
			{
				of << (*it2).index << " "<< (*it2).reps << endl;
				//cout << "index: " << (*it2).index << ", reps: " << (*it2).reps << endl;
			}
		}
		
		
	}
}

bool EditSession::PointOnLine( V2d &pos, V2d &p0, V2d &p1, double width)
{
	V2d dir = normalize( p1 - p0 );
	double len = length( p1 - p0 );
	double d = dot( pos - p0, dir );

	if( d >= width && d <= len - width )
	{
		double c = cross( pos - p0, dir );	
		//cout << "c: " << c << endl;
		if( abs( c ) < 1.0 )
		{
			//cout << "return true?>?? " << endl;
			return true;
		}
	}
	return false;
}


void EditSession::TryPlaceGatePoint(V2d &pos)
{
	modifyGate = NULL;
	bool found = false;

	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end() && !found; ++it)
	{
		//extended aabb 
		TerrainPoint *closePoint = (*it)->GetClosePoint(8 * zoomMultiple, pos);

		if (closePoint != NULL)
		{
			if (gatePoints == 0)
			{
				for (list<GateInfoPtr>::iterator git = gates.begin(); git != gates.end() && !found; ++git)
				{
					if ((*git)->point0 == closePoint || (*git)->point1 == closePoint)
					{
						GateInfoPtr gi = (*git);

						modifyGate = gi;


						found = true;
					}
				}

				if (!found)
				{
					found = true;
					gatePoints = 1;
					testGateInfo.poly0 = (*it);
					testGateInfo.point0 = closePoint;
					testGateInfo.vertexIndex0 = (*it)->GetPointIndex(closePoint);
				}
			}
			else
			{
				found = true;
				gatePoints = 2;

				testGateInfo.poly1 = (*it);
				testGateInfo.point1 = closePoint;
				testGateInfo.vertexIndex1 = (*it)->GetPointIndex(closePoint);
				//view.setCenter(testGateInfo.point1->pos.x, testGateInfo.point1->pos.y);
				//preScreenTex->setView(view);
			}
		}
	}

	if (!found && gatePoints == 0 )
	{
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it)->ContainsPoint(pos))
			{
				modifyGate = (*it);
			}
		}
	}
}

//returns true if attach is successful
ActorParams * EditSession::AttachActorToPolygon( ActorPtr actor, TerrainPolygon *poly )
{
	TerrainPoint *next;
	V2d currPos, nextPos;
	V2d aCurr, aNext;
	V2d actorPos;
	for( TerrainPoint *p = poly->pointStart; p != NULL; p = p->next )
	{
		if( p == poly->pointEnd )
		{
			next = poly->pointStart;
		}
		else
		{
			next = p->next;
		}

		currPos.x = p->pos.x;
		currPos.y = p->pos.y;

		nextPos.x = next->pos.x;
		nextPos.y = next->pos.y;

		assert( actor->groundInfo != NULL );
		double actorQuant = actor->groundInfo->groundQuantity;
		aCurr.x = actor->groundInfo->edgeStart->pos.x;
		aCurr.y = actor->groundInfo->edgeStart->pos.y;

		TerrainPoint *nextActorPoint = actor->groundInfo->edgeStart->next;
		if( actor->groundInfo->edgeStart == actor->groundInfo->ground->pointEnd )
		{
			nextActorPoint = actor->groundInfo->ground->pointStart;
		}
		aNext.x = nextActorPoint->pos.x;
		aNext.y = nextActorPoint->pos.y;

		actorPos = aCurr + normalize(aNext - aCurr) * actorQuant;//V2d( actor->image.getPosition() );//
		bool onLine = PointOnLine( actorPos, currPos, nextPos );

		double finalQuant = dot( actorPos - currPos, normalize( nextPos - currPos ) );
		
		
		if( onLine )
		{
			cout << "actorPos: " << actorPos.x << ", " << actorPos.y << ", currPos: "
				<< currPos.x << ", " << currPos.y << endl;
			GroundInfo gi;
			gi.edgeStart = p;
			gi.ground = poly;
			cout << "finalQuant: " << finalQuant << endl;
			gi.groundQuantity = finalQuant;

			//might need to make sure it CAN be grounded
			assert( actor->groundInfo != NULL ); 

			ActorParams *newActor = actor->Copy();
			newActor->groundInfo = NULL;
			newActor->AnchorToGround(gi);

			assert(gi.edgeStart != NULL);
			assert(newActor != NULL);



			return newActor;
		}
	}

	return NULL;
	//return false;
}

void EditSession::UndoMostRecentAction()
{
	if (doneActionStack.size() > 0)
	{
		Action *action = doneActionStack.back();
		doneActionStack.pop_back();

		action->Undo();

		undoneActionStack.push_back(action);

		//clearing this so things don't get messing with deleted items being selected etc
		selectedBrush->SetSelected(false);
		selectedBrush->Clear();
	}
}

void EditSession::RedoMostRecentUndoneAction()
{
	if (undoneActionStack.size() > 0)
	{
		Action *action = undoneActionStack.back();
		undoneActionStack.pop_back();

		action->Perform();

		doneActionStack.push_back(action);

		selectedBrush->SetSelected(false);
		selectedBrush->Clear();
	}
}

void EditSession::AttachActorsToPolygon( list<ActorPtr> &actors, TerrainPolygon *poly )
{
}

double GetClockwiseAngleDifference(const V2d &A, const V2d &B)
{
	double angleA = atan2(-A.y, A.x);
	if (angleA < 0)
	{
		angleA += PI * 2;
	}
	double angleB = atan2(-B.y, B.x);
	double xxx = atan2(0, 1.0);
	double yyy = atan2(0, -1.0);
	double ggg = atan2(-1.0, 0);
	double fff = atan2(1.0, 0);
	if (angleB < 0)
	{
		angleB += PI * 2;
	}

	if (angleA > angleB)
	{
		return angleA - angleB;
	}
	else if (angleA < angleB)
	{
		return ((2 * PI) - angleB) + angleA;
		/*if (angleA == 0)
		{
			return angleB;
		}
		else
		{
			return angleB + ((PI * 2) - angleA);
		}*/
		

	}

	/*if (angle < 0)
	{
		angle += PI * 2;
	}*/
}

bool IsWithinOne(sf::Vector2i &a, sf::Vector2i &b)
{
	return (abs(a.x - b.x) <= 1 && abs(a.y - b.y) <= 1);
}

LineIntersection EditSession::SegmentIntersect( Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{
	LineIntersection li = lineIntersection( V2d( a.x, a.y ), V2d( b.x, b.y ), 
				V2d( c.x, c.y ), V2d( d.x, d.y ) );
	if( !li.parallel )
	{
		double e1Left = min( a.x, b.x );
		double e1Right = max( a.x, b.x );
		double e1Top = min( a.y, b.y );
		double e1Bottom = max( a.y, b.y );

		double e2Left = min( c.x, d.x );
		double e2Right = max( c.x, d.x );
		double e2Top = min( c.y, d.y );
		double e2Bottom = max( c.y, d.y );
		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right && li.position.x >= e1Left && li.position.y >= e1Top && li.position.y <= e1Bottom)
			{
				if( li.position.x <= e2Right && li.position.x >= e2Left && li.position.y >= e2Top && li.position.y <= e2Bottom)
				{
					//cout << "seg intersect!!!!!!" << endl;
					//assert( 0 );
					return li;
				}
			}
		}
	}
	else
	{
		/*V2d dir0 = normalize(V2d(b) - V2d(a));
		V2d dir1 = normalize(V2d(d) - V2d(c));
		if ( abs( dot( dir1, dir0 ) ) == 1 )
		{
			double dc = dot(V2d(c) - V2d(a), dir0);
			double dd = dot(V2d(d) - V2d(a), dir0);
			double da = 0;
			double db = length(V2d(b) - V2d(a));

			if (dc >= da && dc <= db )
			{
				li.parallel = false;
				li.position = V2d(c);
			}
			if (dd >= da && dd <= db)
			{
				li.parallel = false;
				li.position = V2d(d);
			}
		}*/
	}
	//cout << "return false" << endl;
	li.parallel = true;
	return li;
}

bool EditSession::QuadPolygonIntersect( TerrainPolygon* poly, Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{

	//TerrainPolygon *quadPoly = new TerrainPolygon( poly->grassTex );
	//PolyPtr quadPoly( new TerrainPolygon( poly->grassTex ) );
	TerrainPolygon quadPoly( poly->grassTex );
	quadPoly.AddPoint( new TerrainPoint( a, false ) );
	quadPoly.AddPoint( new TerrainPoint( b, false ) );
	quadPoly.AddPoint( new TerrainPoint( c, false ) );
	quadPoly.AddPoint( new TerrainPoint( d, false ) );
	quadPoly.UpdateBounds();

	//PolyPtr blah( &quadPoly );

	//cout << "quad bottom: " << quadPoly.bottom << endl;
	//cout << "poly top: " << poly->top << endl;
	
	bool touching = poly->IsTouching( &quadPoly );

	//delete quadPoly;

	return touching;

	/*int qLeft = min( a.x, min( b.x, min( c.x, d.x ) ) );
	int qRight = max( a.x, min( b.x, min( c.x, d.x ) ) );
	int qTop = min( a.y, min( b.y, min( c.y, d.y ) ) );
	int qBot = max( a.y, min( b.y, min( c.y, d.y ) ) );

	if( poly->left >= qLeft && poly->right <= qRight && poly->top >= qTop && poly->bottom <= qBot )
	{
		return true;
	}

	IntRect ri( qLeft, qTop, qRight - qLeft, qBot - qTop );
	IntRect riPoly( poly->left, poly->top, poly->right - poly->left, poly->bottom - poly->top );

	if( !ri.intersects( riPoly ) )
	{
		return false;
	}

	bool containsA = poly->ContainsPoint( Vector2f( a.x, a.y ) );
	bool containsB = poly->ContainsPoint( Vector2f( b.x, b.y ) );
	bool containsC = poly->ContainsPoint( Vector2f( c.x, c.y ) );
	bool containsD = poly->ContainsPoint( Vector2f( d.x, d.y ) );

	if( containsA || containsB || containsC || containsD )
	{
		return true;
	}

	for( PointList::iterator it = poly->points.begin(); it != poly->points.end(); ++it )
	{
		Vector2i &p = (*it).pos;
		
	}

	return false;*/
}

LineIntersection EditSession::LimitSegmentIntersect( Vector2i a, Vector2i b, Vector2i c, Vector2i d, bool firstLimitOnly )
{
	LineIntersection li = lineIntersection( V2d( a.x, a.y ), V2d( b.x, b.y ), 
				V2d( c.x, c.y ), V2d( d.x, d.y ) );
	if( !li.parallel )
	{
		double e1Left = min( a.x, b.x );
		double e1Right = max( a.x, b.x );
		double e1Top = min( a.y, b.y );
		double e1Bottom = max( a.y, b.y );

		double e2Left = min( c.x, d.x );
		double e2Right = max( c.x, d.x );
		double e2Top = min( c.y, d.y );
		double e2Bottom = max( c.y, d.y );
		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right && li.position.x >= e1Left && li.position.y >= e1Top && li.position.y <= e1Bottom)
			{
				if( li.position.x <= e2Right && li.position.x >= e2Left && li.position.y >= e2Top && li.position.y <= e2Bottom)
				{
					V2d &pos = li.position;
					if( ( length( li.position - V2d( a.x, a.y ) ) > 1.0 ) && ( firstLimitOnly || length( li.position - V2d( b.x, b.y ) ) > 1.0 ) 
						&&  ( firstLimitOnly || ( ( length( li.position - V2d( c.x, c.y ) ) > 0 &&  length( li.position - V2d( d.x, d.y ) ) > 1.0 ))) )
					{
						return li;
					}
					//cout << "seg intersect!!!!!!" << endl;
					//assert( 0 );
					
				}
			}
		}
	}
	//cout << "return false" << endl;
	li.parallel = true;
	return li;
}

int EditSession::Run( const boost::filesystem::path &p_filePath, Vector2f cameraPos, Vector2f cameraSize )
{
	testGateInfo.edit = EditSession::GetSession();
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVis = mainMenu->GetMouseVisible();

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(true);

	sf::View oldPreTexView = preScreenTex->getView();//mainMenu->preScreenTexture->
	sf::View oldWindowView = w->getView();

	currTool = TOOL_ADD;
	//bosstype = 0;
	currentFile = p_filePath.string();
	currentPath = p_filePath;


	tempActor = NULL;
	int width = 1920;
	int height = 1080;
	uiView = View( sf::Vector2f( width / 2, height / 2), sf::Vector2f( width, height ) );
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	w->setView( v );

	shardSelectPopup = CreatePopupPanel("shardselector");

	confirm = CreatePopupPanel( "confirmation" );
	validityRadius = 4;


	modifyGate = NULL;


	showGrass = false;
	showGraph = false;

	justCompletedPolyWithClick = false;

	trackingEnemy = NULL;
	showPanel = NULL;

	sf::Texture playerZoomIconTex;
	playerZoomIconTex.loadFromFile( "Resources/Editor/playerzoomicon.png" );
	sf::Sprite playerZoomIcon( playerZoomIconTex );
	
	playerZoomIcon.setOrigin( playerZoomIcon.getLocalBounds().width / 2, playerZoomIcon.getLocalBounds().height / 2 );	

	SetupEnemyTypes();

	InitDecorPanel();

	mapOptionsPanel = CreateOptionsPanel("map");
	terrainOptionsPanel = CreateOptionsPanel("terrain");
	railOptionsPanel = CreateOptionsPanel("rail");

	messagePopup = CreatePopupPanel( "message" );
	errorPopup = CreatePopupPanel( "error" );
	bgPopup = CreatePopupPanel("bg");

	GridSelector *bgSel = bgPopup->AddGridSelector(
		"terraintypes", Vector2i(20, 20), 6, 7, 1920/8, 1080/8, false, true);

	Tileset *bgTS;
	string path = "BGInfo/";
	string bgName; //"w1_0";
	string png = ".png";
	string numStr;
	string fullName;
	for (int w = 0; w < 2; ++w)
	{
		for (int i = 0; i < 6; ++i)
		{
			numStr = to_string(i + 1);
			bgName = "w" + to_string(w + 1) + "_0";// +to_string(i + 1);
			fullName = path + bgName + numStr + png;
			bgTS = GetTileset(fullName, 1920, 1080);
			if (bgTS == NULL)
			{
				continue;
			}
			Sprite bgSpr(*bgTS->texture);
			bgSpr.setScale(.125, .125);
			bgSel->Set(i, w, bgSpr, bgName + numStr);
		}
	}

	enemySelectPanel = new Panel( "enemyselection", 200, 200, this );
	allPopups.push_back(enemySelectPanel);
	int gridSizeX = 80;
	int gridSizeY = 80;

	GridSelector *gs = NULL;
	int counter = 0;
	for (int i = 0; i < 4; ++i)
	{
		gs = enemySelectPanel->AddGridSelector(to_string(i), Vector2i(0, 0), 15, 10, gridSizeX,
			gridSizeY, false, true);
		gs->active = false;
		enemyGrid[i] = gs;

		counter = 0;
		for (int j = 0; j < 8; ++j)
		{
			auto &wen = worldEnemyNames[j];
			counter = 0;
			for (auto it = wen.begin(); it != wen.end(); ++it)
			{
				if ((*it).numLevels >= i+1)
				{
					SetEnemyGridIndex(gs, counter, j, (*it).name);
				}
				++counter;
			}
		}

		counter = 0;
		int row = 8;
		for (auto it = extraEnemyNames.begin(); it != extraEnemyNames.end(); ++it)
		{
			if ((*it).numLevels >= i + 1)
			{
				SetEnemyGridIndex(gs, counter, row, (*it).name);
			}
			++counter;
			if (counter == gs->xSize)
			{
				counter = 0;
				++row;
			}
		}

	}

	SetActiveEnemyGrid(0);

	gateSelectorPopup = CreatePopupPanel( "gateselector" );
	GridSelector *gateSel = gateSelectorPopup->AddGridSelector( "gatetypes", Vector2i( 20, 20 ), 7, 4, 32, 32, false, true );

	sf::Texture whiteTex; //temp
	whiteTex.loadFromFile( "Resources/Editor/whitesquare.png" );
	Sprite tempSq;
	tempSq.setTexture( whiteTex );

	tempSq.setColor( Color::Black );
	gateSel->Set( 0, 0, tempSq, "black" );

	tempSq.setColor( Color( 100, 100, 100 ) );
	gateSel->Set( 1, 0, tempSq, "keygate" );

	tempSq.setColor( Color::Red );
	gateSel->Set( 2, 0, tempSq, "secret" );

	tempSq.setColor( Color( 0, 255, 40 ) );
	gateSel->Set( 3, 0, tempSq, "birdfight" );

	tempSq.setColor( Color::Blue );
	gateSel->Set( 4, 0, tempSq, "crawlerunlock" );

	tempSq.setColor( Color::Cyan );
	gateSel->Set( 5, 0 , tempSq, "nexus1unlock" );

	tempSq.setColor(Color(100, 255, 10));
	gateSel->Set(6, 0, tempSq, "shard");

	gateSelectorPopup->AddButton( "deletegate", Vector2i( 20, 300 ), Vector2f( 80, 40 ), "delete" );

	SetupTerrainTypeSelector();

	currTerrainWorld = 0;
	currTerrainVar = 0;
	currTerrainTypeSpr.setPosition(0, 160);
	UpdateCurrTerrainType();

	returnVal = 0;
	Color testColor( 0x75, 0x70, 0x90 );
	view = View( cameraPos, cameraSize );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setSize( 1920, 1080 );

	preScreenTex->setView( view );

	OpenFile();

	//Vector2f vs(  );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setCenter( (float)player->position.x, (float)player->position.y );

	//mode = "neutral";
	quit = false;
	polygonInProgress.reset( new TerrainPolygon(&grassTex ) );
	railInProgress.reset(new TerrainRail());
	//inversePolygon.reset( NULL );

	zoomMultiple = 2;

	view.setSize(Vector2f(960 * (zoomMultiple), 540 * (zoomMultiple)));
	preScreenTex->setView(view);

	UpdateFullBounds();
	
	panning = false;
	minimumEdgeLength = 8;//8;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	
	border[0] = sf::Vertex(sf::Vector2<float>(-max, -max), borderColor);
	border[1] = sf::Vertex(sf::Vector2<float>(-max, max), borderColor);
	border[2] = sf::Vertex(sf::Vector2<float>(-max, max), borderColor);
	border[3] = sf::Vertex(sf::Vector2<float>(max, max), borderColor);
	border[4] = sf::Vertex(sf::Vector2<float>(max, max), borderColor);
	border[5] = sf::Vertex(sf::Vector2<float>(max, -max), borderColor);
	border[6] = sf::Vertex(sf::Vector2<float>(max, -max), borderColor);
	border[7] = sf::Vertex(sf::Vector2<float>(-max, -max), borderColor);

	sf::Texture guiMenuCubeTexture;
	guiMenuCubeTexture.loadFromFile( "Resources/Editor/guioptions.png" );
	guiMenuSprite.setTexture( guiMenuCubeTexture );
	guiMenuSprite.setOrigin( guiMenuSprite.getLocalBounds().width / 2, guiMenuSprite.getLocalBounds().height / 2 );

	graphColor = Color( 200, 50, 50, 100 );
	numGraphLines = 30;
	graphLinesVA = new VertexArray(sf::Lines, numGraphLines * 8);

	graph = new EditorGraph;

	SetupGraph();
	

	bool s = IsKeyPressed( sf::Keyboard::T );

	mode = EDIT;
	stored = mode;
	bool canCreatePoint = true;

	menuCircleDist = 100;
	menuCircleRadius = 50;

	circleTopPos =  V2d( 0, -1 ) * menuCircleDist;

	circleUpperRightPos = V2d( sqrt( 3.0 ) / 2, -.5 ) * menuCircleDist;
	circleLowerRightPos = V2d( sqrt( 3.0 ) / 2, .5 ) * menuCircleDist;

	circleUpperLeftPos = V2d( -sqrt( 3.0 ) / 2, -.5 ) * menuCircleDist;
	circleLowerLeftPos = V2d( -sqrt( 3.0 ) / 2, .5 ) * menuCircleDist;

	circleBottomPos = V2d( 0, 1 ) * menuCircleDist;

	menuSelection = "";

	borderMove = 100;

	Vector2f uiMouse;

	while( !quit )
	{
		pixelPos = GetPixelPos();

		worldPos = V2d(preScreenTex->mapPixelToCoords(pixelPos));
		worldPosGround = ConvertPointToGround( Vector2i( worldPos.x, worldPos.y ) );
		worldPosRail = ConvertPointToRail(Vector2i(worldPos));

		preScreenTex->setView( uiView );
		uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		uiMousePos = uiMouse;
		
		preScreenTex->setView( view );
		
		testPoint.x = worldPos.x;
		testPoint.y = worldPos.y;

		if (mode == CREATE_PATROL_PATH || mode == SET_DIRECTION)
		{
			if (showPanel != NULL)
			{

			}
			else
			{
				V2d pathBack(patrolPath.back());
				V2d temp = V2d(testPoint.x, testPoint.y) - pathBack;

				if (IsKeyPressed(Keyboard::LShift))
				{
					double angle = atan2(-temp.y, temp.x);
					if (angle < 0)
					{
						angle += PI * 2.0;
					}
					double len = length(temp);
					double mult = angle / (PI / 4.0);
					double dec = mult - floor(mult);
					int iMult = mult;
					if (dec >= .5)
					{
						iMult++;
					}

					angle = iMult * PI / 4.0;
					V2d testVec(len, 0);
					RotateCCW(testVec, angle);
					testPoint = Vector2f(pathBack + testVec);
					temp = testVec;
				}
			}
		}

		HandleEvents();

		if( quit )
			break;

		showGraph = false;

		if (IsKeyPressed(Keyboard::Num5))
		{
			Vector2f halfSize(scaleSprite.getGlobalBounds().width / 2.f, 
				scaleSprite.getGlobalBounds().height / 2.f);
			scaleSprite.setPosition(Vector2f(pixelPos) - halfSize );
			scaleSpriteBGRect.setPosition(Vector2f(pixelPos) - halfSize );
		}
		else
		{
			scaleSprite.setPosition(0, 80);
			scaleSpriteBGRect.setPosition(0, 80);
		}

		//--------

		UpdateMode();
		

		UpdatePanning();
		
		UpdatePolyShaders();
		
		Draw();
		
		DrawMode();		

		TempMoveSelectedBrush();

		DrawGraph();

		DrawDecorFront();

		if( zoomMultiple > 7 )
		{
			playerZoomIcon.setPosition( player->position.x, player->position.y );
			playerZoomIcon.setScale( zoomMultiple * 1.8, zoomMultiple * 1.8 );
			preScreenTex->draw( playerZoomIcon );
		}

		preScreenTex->draw( fullBounds );

		if (mode == CREATE_IMAGES)
		{
			if( showPanel == NULL )
				preScreenTex->draw(tempDecorSprite);
		}

		DrawUI();

		preScreenTex->setView( view );

		Display();
	}
	
	preScreenTex->setView(oldPreTexView);
	w->setView(oldWindowView);

	mainMenu->SetMouseGrabbed(oldMouseGrabbed);
	mainMenu->SetMouseVisible(oldMouseVis);

	return returnVal;
}

//THIS IS ALSO DEFINED IN ACTORPARAMS NEED TO GET RID OF THE DUPLICATE
//helper function to assign monitor types

void EditSession::ButtonCallback( Button *b, const std::string & e )
{
	//cout << "start of callback!: " << groups["--"]->actors.size() << endl;
	Panel *p = b->owner;
	
	if (p == editDecorPanel)
	{
		if (b->name == "ok")
		{
			showPanel = NULL;
		}
	}
	else if (p->name == "airtrigger_options")
	{
		if (b->name == "ok")
		{
			RegularOKButton();
		}
		else if (b->name == "createrect")
		{
			if (mode == EDIT)
			{
				ISelectable *select = selectedBrush->objects.front().get();
				AirTriggerParams *airTrigger = (AirTriggerParams*)select;
				rectCreatingTrigger = airTrigger;
			}
			else if (mode == CREATE_ENEMY)
			{
				rectCreatingTrigger = (AirTriggerParams*)tempActor;
			}

			showPanel = NULL;
			
			mode = CREATE_RECT;
			drawingCreateRect = false;
		}
	}
	else if (p->name == "camerashot_options")
	{
		if (b->name == "ok")
		{
			RegularOKButton();
		}
		else if (b->name == "setzoom")
		{
			if (mode == EDIT)
			{
				ISelectable *select = selectedBrush->objects.front().get();
				CameraShotParams *camShot = (CameraShotParams*)select;
				currentCameraShot = camShot;
			}
			else if (mode == CREATE_ENEMY)
			{
				currentCameraShot = (CameraShotParams*)tempActor;
			}

			showPanel = NULL;

			mode = SET_CAM_ZOOM;
		}
	}
	else if( p->name == "map_options" )
	{
		if (b->name == "ok")
		{
			int minEdgeSize;

			stringstream ss;
			//string s = p->textBoxes["minedgesize"]->text.getString().toAnsiString();
			string s = p->textBoxes["minedgesize"]->text.getString().toAnsiString();
			string drainStr = p->textBoxes["draintime"]->text.getString().toAnsiString();
			string bossTypeStr = p->textBoxes["bosstype"]->text.getString().toAnsiString();
			ss << s;

			ss >> minEdgeSize;

			if (ss.fail())
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert(false);
			}

			if (minEdgeSize < 8)
			{
				minimumEdgeLength = 8;

				p->textBoxes["minedgesize"]->text.setString("8");

				MessagePop("minimum edge length too low.\n Set to minimum of 8");
				//assert( false && "made min edge length too small!" );
			}
			else
			{
				minimumEdgeLength = minEdgeSize;
			}
			//stringstream ss2;
			ss.clear();

			ss << drainStr;

			int dSecs;
			ss >> dSecs;

			if (!ss.fail())
			{
				drainSeconds = dSecs;
			}

			ss.clear();

			ss << bossTypeStr;

			int bType;
			ss >> bType;

			if (!ss.fail())
			{
				bossType = bType;
			}


			showPanel = NULL;
		}
		else if (b->name == "envtype")
		{
			GridSelectPop("bg");
		}
	}
	else if( p->name == "terrain_options" )
	{
		if (b->name == "ok")
		{
			showPanel = NULL;
		}
	}
	else if (p->name == "rail_options")
	{
		ISelectable *select = selectedBrush->objects.front().get();
		TerrainRail *tr = (TerrainRail*)select;
		if (b->name == "ok")
		{
			tr->SetParams(railOptionsPanel);
			showPanel = NULL;
		}
		else if (b->name == "reverse")
		{
			tr->SwitchDirection();
			//reverse single rail
		}
	}
	else if( p->name == "error_popup" )
	{
		if (b->name == "ok")
		{
			showPanel = NULL;
		}
	}
	else if( p->name == "confirmation_popup" )
	{
		if( b->name == "confirmOK" )
		{
			confirmChoice = ConfirmChoices::CONFIRM;
		}
		else if( b->name == "cancel" )
		{
			confirmChoice = ConfirmChoices::CANCEL;
		}
	}
	else if( p == gateSelectorPopup )
	{
		tempGridResult = "delete";
	}
	else if (p == shardSelectPopup)
	{
		if (b->name == "ok")
		{
			tempGridResult = "shardclose";
		}
	}
	else
	{
		if (b->name == "ok")
		{
			RegularOKButton();
		}
		else if (b->name == "createpath" || b->name == "createrail")
		{
			RegularCreatePathButton();
		}
		else if (b->name == "setdirection")
		{
			RegularCreatePathButton();
			mode = SET_DIRECTION;
		}
	}
}

void EditSession::TextBoxCallback( TextBox *tb, const std::string & e )
{
	//to be able to show previews in real time
	Panel *p = tb->owner;
	if( p->name == "curveturret_options" )
	{
		if( tb->name == "xgravfactor" || tb->name == "ygravfactor"
			|| tb->name == "bulletspeed" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				CurveTurretParams *curveTurret = (CurveTurretParams*)select;
				curveTurret->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				CurveTurretParams *curveTurret = (CurveTurretParams*)tempActor;
				curveTurret->SetParams();
			}
		}
	}
	else if( p->name == "poisonfrog_options" )
	{
		if( tb->name == "xstrength" || tb->name == "ystrength" 
			|| tb->name == "gravfactor" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoisonFrogParams *poisonFrog = (PoisonFrogParams*)select;
				poisonFrog->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				PoisonFrogParams *poisonFrog = (PoisonFrogParams*)tempActor;
				poisonFrog->SetParams();
			}
		}
	}
	else if( p->name == "poi_options" )
	{
		if( tb->name == "camzoom" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoiParams *poi = (PoiParams*)select;
				poi->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				PoiParams *poi = (PoiParams*)tempActor;
				poi->SetParams();
			}
		}
	}
	else if (p == editDecorPanel)
	{
		SetDecorParams();
	}
}

void EditSession::GridSelectorCallback( GridSelector *gs, const std::string & p_name )
{
	cout << "grid selector callback!" << endl;
	string name = p_name;
	Panel *panel = gs->owner;
	if( panel == enemySelectPanel )
	{
		if( name != "not set" )
		{
			trackingEnemy = types[name];

			enemySprite = trackingEnemy->GetSprite(false);

			enemyQuad.setSize( Vector2f( trackingEnemy->info.size.x, trackingEnemy->info.size.y) );

			showPanel = NULL;

			cout << "set your cursor as the image" << endl;
		}
		else
		{
			cout << "not set" << endl;
		}
	}
	else if( panel == gateSelectorPopup || panel == terrainSelectorPopup )
	{
		cout << "callback!" << endl;
		if( name != "not set" )
		{
			cout << "real result: " << name << endl;
			tempGridResult = name;
			tempGridX = gs->selectedX;
			tempGridY = gs->selectedY;
			//showPanel = NULL;
		}
		else
		{
		//	cout << "not set" << endl;
		}
	}
	else if (panel == decorPanel)
	{
		if (name != "not set")
		{
			//cout << "real result: " << name << endl;
			currDecorName = name;
			ts_currDecor = decorTSMap[currDecorName];
			int ind = gs->selectedY * gs->xSize + gs->selectedX;
			currDecorTile = decorTileIndexes[ind];


			tempDecorSprite.setTexture(*ts_currDecor->texture);
			tempDecorSprite.setTextureRect(ts_currDecor->GetSubRect(currDecorTile));

			string layerStr = decorPanel->textBoxes["layer"]->text.getString().toAnsiString();
			stringstream tempSS;

			tempSS << layerStr;

			int cdLayer;
			tempSS >> cdLayer;

			if (!tempSS.fail())
			{
				currDecorLayer = cdLayer;
			}
			else
			{
				decorPanel->textBoxes["layer"]->text.setString("0");
			}

			showPanel = NULL;
			//ts_currDecor = GetTileset( currDecorName + string(".png"),  )
			//tempDecorSprite.setTexture(ts_currDecor)
			/*tempGridResult = name;
			tempGridX = gs->selectedX;
			tempGridY = gs->selectedY;*/
		}
		else
		{
			//	cout << "not set" << endl;
		}
	}
	else if (panel == bgPopup)
	{
		if (name != "not set")
		{
			if (Background::SetupFullBG(name, *this, background->currBackground, background->scrollingBackgrounds))
			{
				tempGridResult = name;
				envName = name;
			}
		}
	}
	else if (panel->name == "shard_options" )
	{
		int world = gs->selectedX / 11;
		int realX = gs->selectedX % 11;
		int realY = gs->selectedY;

		ShardParams *shard = NULL;
		if (mode == EDIT)
		{
			ISelectable *select = selectedBrush->objects.front().get();
			shard = (ShardParams*)select;
		}
		else if (mode == CREATE_ENEMY)
		{
			shard = (ShardParams*)tempActor;
		}
		else
		{
			assert(0);
		}
		shard->SetShard(world, realX, realY);

		panel->labels["shardtype"]->setString(name);
	}
	else if (panel->name == "shardselector")
	{
		tempGridResult = name;
		tempGridX = gs->selectedX;
		tempGridY = gs->selectedY;
		panel->labels["shardtype"]->setString(name);
	}
}

void EditSession::LoadDecorImages()
{
	ifstream is;
	is.open("Resources/decor.txt");
	if (is.is_open())
	{
		string name;
		int width;
		int height;
		int tile;
		while (!is.eof())
		{
			is >> name;

			is >> width;
			is >> height;

			is >> tile;

			string fullName = name + string(".png");
			
			Tileset *ts = GetTileset(fullName, width, height);
			assert(ts != NULL);
			decorTSMap[name] = ts;
			decorTileIndexMap[name].push_back(tile);
		}
	}
	else
	{
		assert(0);
	}
}

void EditSession::CheckBoxCallback( CheckBox *cb, const std::string & e )
{
	//cout << cb->name << " was " << e << endl;
	Panel *p = cb->owner;
	if( p->name == "curveturret_options" )
	{
		if( cb->name == "relativegrav" )
		{
			//cout << "BLAHBADIOHFWEIHEGHWEAOHGEAWHGEWAHG" << endl;
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				CurveTurretParams *curveTurret = (CurveTurretParams*)select;
				curveTurret->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				CurveTurretParams *curveTurret = (CurveTurretParams*)tempActor;
				curveTurret->SetParams();
			}
		}
	}
	else if( p->name == "poi_options" )
	{
		if( cb->name == "camprops" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoiParams *poi = (PoiParams*)select;
				poi->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				PoiParams *poi = (PoiParams*)tempActor;
				poi->SetParams();
			}
		}
	}
}

void EditSession::ClearUndoneActions()
{
	for( list<Action*>::iterator it = undoneActionStack.begin(); it != undoneActionStack.end(); ++it )
	{
		delete (*it);
	}
	undoneActionStack.clear();
}

bool EditSession::TrySnapPosToPoint(sf::Vector2f &p, double radius, PolyPtr &poly, TerrainPoint *&point)
{
	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = Vector2f(closePoint->pos);
			poly = (*it);
			point = closePoint;
			return true;
		}
	}

	return false;
}

bool EditSession::TrySnapPosToPoint(V2d &p, double radius, PolyPtr &poly, TerrainPoint *&point)
{
	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = V2d(closePoint->pos);
			poly = (*it);
			point = closePoint;
			return true;
		}
	}

	return false;
}

bool EditSession::IsSpecialTerrainMode()
{
	if (currTerrainWorld >= 8)
	{
		return true;
	}

	return false;
}

void EditSession::InitDecorPanel()
{
	int w = 8;
	int h = 5;
	int sw = 128;
	int sh = 128;
	LoadDecorImages();
	decorPanel = new Panel("decorpanel", 650, 800, this);
	allPopups.push_back(decorPanel);
	GridSelector *gs = decorPanel->AddGridSelector("decorselector", Vector2i(0, 0), w, h, sw, sh, false, true );
	decorPanel->AddTextBox("layer", Vector2i( 0, 800), 100, 3, "0");
	decorTileIndexes = new int[w*h];
	

	//decorPanel->textBoxes["layer"]->text

	gs->active = true;

	int ind = 0;
	int x, y;
	for (auto it = decorTSMap.begin(); it != decorTSMap.end(); ++it)
	{
		for (auto tit = decorTileIndexMap[(*it).first].begin(); 
			tit != decorTileIndexMap[(*it).first].end(); ++tit)
		{
			x = ind % w;
			y = ind / w;
			Tileset *ts = (*it).second;
			Sprite s(*ts->texture);
			s.setTextureRect(ts->GetSubRect((*tit)));
			decorTileIndexes[ind] = (*tit);
			float texX = ts->texture->getSize().x;
			float texY = ts->texture->getSize().y;
			s.setScale(((float)sw) / ts->tileWidth, ((float)sh) / ts->tileHeight);
			//s.setTextureRect(IntRect(0, 0, sw, sh));
			//gs->Set(x, y, s, (*it).first);
			gs->Set(x, y, s, (*it).first);

			++ind;
		}
	}

	editDecorPanel = new Panel("editdecorpanel", 500, 500, this);
	allPopups.push_back(editDecorPanel);
	editDecorPanel->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");

	editDecorPanel->AddTextBox("xpos", Vector2i(20, 20), 200, 20, "x");
	editDecorPanel->AddTextBox("ypos", Vector2i( 200, 20), 200, 20, "y");
	editDecorPanel->AddTextBox("rotation", Vector2i(20, 100), 200, 20, "r");
	editDecorPanel->AddTextBox("xscale", Vector2i(20, 180), 200, 20, "x");
	editDecorPanel->AddTextBox("yscale", Vector2i(200, 180), 200, 20, "y");
	editDecorPanel->AddTextBox("layer", Vector2i(20, 280), 200, 20, "l");
	//editDecorPanel->AddTextBox("rotation", Vector2i(200, 100), 200, 20, "r");
	//editDecorPanel->AddTextBox("strength", Vector2i(20, 200), 200, 3, "");
}

int EditSession::CountSelectedPoints()
{
	int count = 0;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		count += (*it).second.size();
	}
	return count;
}

void EditSession::ClearSelectedPoints()
{
	for( PointMap::iterator pmit = selectedPoints.begin();
		pmit != selectedPoints.end(); ++pmit )
	{
		list<PointMoveInfo> & pList = (*pmit).second;
		for( list<PointMoveInfo>::iterator pit = pList.begin();
			pit != pList.end(); ++pit )
		{
			(*pit).point->selected = false;
		}
	}

	for (auto pmit = selectedRailPoints.begin();
		pmit != selectedRailPoints.end(); ++pmit)
	{
		list<PointMoveInfo> & pList = (*pmit).second;
		for (list<PointMoveInfo>::iterator pit = pList.begin();
			pit != pList.end(); ++pit)
		{
			(*pit).point->selected = false;
		}
	}

	selectedPoints.clear();
	selectedRailPoints.clear();
}

void EditSession::RemovePointFromPolygonInProgress()
{
	if (polygonInProgress->numPoints > 0)
	{
		polygonInProgress->RemovePoint(polygonInProgress->pointEnd);
	}
}

void EditSession::RemovePointFromRailInProgress()
{
	if (railInProgress->numPoints > 0)
	{
		railInProgress->RemovePoint(railInProgress->pointEnd);
	}
}

void EditSession::TryAttachActors(
	std::list<PolyPtr> &origPolys,
	std::list<PolyPtr> &newPolys,
	Brush *b)
{
	for (auto it = origPolys.begin(); it != origPolys.end(); ++it)
	{
		for (auto mit = (*it)->enemies.begin(); mit != (*it)->enemies.end(); ++mit)
		{
			for (auto bit = (*mit).second.begin(); bit != (*mit).second.end(); ++bit)
			{
				for (auto rit = newPolys.begin();
					rit != newPolys.end(); ++rit)
				{
					ActorParams *ac = AttachActorToPolygon((*bit), (*rit).get());
					if (ac != NULL)
					{
						ActorPtr newActor(ac);
						b->AddObject(newActor);
					}
				}
			}
		}
	}
}

void EditSession::TryKeepGates( list<GateInfoPtr> &gateInfoList, list<PolyPtr> &newPolys, Brush *b )
{
	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		TerrainPoint *test = NULL;
		TerrainPoint * outTest = NULL;

		TerrainPoint *p0 = NULL;
		TerrainPoint *p1 = NULL;
		TerrainPoint *testPoint = NULL;


		PolyPtr poly0;
		PolyPtr poly1;

		for (auto rit = newPolys.begin();
			rit != newPolys.end(); ++rit)
		{
			if (p0 != NULL && p1 != NULL)
				break;

			if (p0 == NULL)
			{
				testPoint = (*rit)->GetSamePoint((*it)->point0->pos);
				if (testPoint != NULL)
				{
					p0 = testPoint;
					poly0 = (*rit);
				}
			}

			if (p1 == NULL)
			{
				testPoint = (*rit)->GetSamePoint((*it)->point1->pos);
				if (testPoint != NULL)
				{
					p1 = testPoint;
					poly1 = (*rit);
				}
			}
		}


		if (p0 == NULL && p1 == NULL)
			continue;

		GateInfoPtr gi(new GateInfo(*((*it).get())));
		gi->edit = NULL;

		if (p0 != NULL)
		{
			gi->point0 = p0;
			gi->poly0 = poly0;
			p0->gate = gi;
		}
		else
		{
			gi->point0 = (*it)->point0;
			gi->poly0 = (*it)->poly0;
			gi->point0->gate = gi;
		}
		if (p1 != NULL)
		{
			gi->point1 = p1;
			gi->poly1 = poly1;
			p1->gate = gi;

		}
		else
		{
			gi->point1 = (*it)->point1;
			gi->poly1 = (*it)->poly1;
			gi->point1->gate = gi;
		}

		b->AddObject(gi);
	}
}

void EditSession::TryRemoveSelectedPoints()
{
	//need to make this into an undoable action soon

	int removeSuccess = IsRemovePointsOkay();

	if (removeSuccess == 1)
	{
		Brush orig;
		Brush result;

		list<GateInfoPtr> gateInfoList;
		list<PolyPtr> affectedPolys;
		list<PolyPtr> newPolys;
		for (PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
		{
			PolyPtr tp = (*it).first;
			affectedPolys.push_back(tp);

			PolyPtr newPoly(new TerrainPolygon(*tp, true, true));
			newPoly->RemoveSelectedPoints();
			newPoly->RemoveSlivers();
			newPoly->AlignExtremes();
			newPoly->Finalize();

			newPolys.push_back(newPoly);

			result.AddObject(newPoly);
		}

		AddFullPolysToBrush(affectedPolys, gateInfoList, &orig);

		TryAttachActors(affectedPolys, newPolys, &result);

		TryKeepGates(gateInfoList, newPolys, &result);
		selectedPoints.clear();

		Action * action = new ReplaceBrushAction(&orig, &result);

		action->Perform();
		doneActionStack.push_back(action);

		ClearUndoneActions();
	}
	else if (removeSuccess == 0)
	{
		MessagePop("problem removing points");
	}
}

bool EditSession::PointSelectActor( V2d &pos )
{
	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		for (list<ActorPtr>::iterator ait = (*it).second->actors.begin();
			ait != (*it).second->actors.end(); ++ait)
		{
			if ((*ait)->ContainsPoint(Vector2f(pos)))
			{
				SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*ait));

				if (sp->selected)
				{

				}
				else
				{
					if (!HoldingShift())
					{
						selectedBrush->SetSelected(false);
						selectedBrush->Clear();
					}

					sp->SetSelected(true);
					grabbedObject = sp;
					selectedBrush->AddObject(sp);
				}
				return true;
			}
		}
	}

	return false;
}

bool EditSession::PointSelectDecor(V2d &pos)
{
	for (auto it = decorImagesBehindTerrain.begin();
		it != decorImagesBehindTerrain.end(); ++it)
	{

	}

	for (auto it = decorImagesBetween.begin();
		it != decorImagesBetween.end(); ++it)
	{
		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));
		if ((*it)->ContainsPoint(Vector2f(worldPos.x, worldPos.y)))
		{
			SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

			if (sp->selected)
			{

			}
			else
			{
				if (!HoldingShift())
				{
					selectedBrush->SetSelected(false);
					selectedBrush->Clear();
				}

				sp->SetSelected(true);

				grabbedObject = sp;
				selectedBrush->AddObject(sp);
			}

			return true;
		}
	}

	for (auto it = decorImagesFrontTerrain.begin();
		it != decorImagesFrontTerrain.end(); ++it)
	{

	}

	return false;
}

bool EditSession::AnchorSelectedAerialEnemy()
{
	bool singleActor = selectedBrush->objects.size() == 1
		&& selectedPoints.size() == 0
		&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
	if (singleActor)
	{
		ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>(selectedBrush->objects.front());
		if (actor->groundInfo != NULL)
		{
			Action *gAction = new GroundAction(actor);
			gAction->performed = true;

			if (moveAction != NULL)
			{
				moveAction->subActions.push_back(gAction);
				doneActionStack.push_back(moveAction);
			}
			else
			{
				Vector2i delta = Vector2i(worldPos.x, worldPos.y) - editMouseOrigPos;
				Action *action = new MoveBrushAction(selectedBrush, delta, false, PointVectorMap(), RailPointMap());

				action->Perform();

				moveAction = new CompoundAction;
				moveAction->subActions.push_back(action);
				moveAction->subActions.push_back(gAction);
				doneActionStack.push_back(moveAction);
			}

			return true;
		}
	}
	return false;
}

void EditSession::TryMoveSelectedBrush()
{
	bool validMove = false;

	//check if valid
	if (selectedBrush->CanApply())
	{
		validMove = true;
	}

	if (validMove)
	{
		ClearUndoneActions();
	}
	else
	{
		Action * action = doneActionStack.back();
		doneActionStack.pop_back();

		action->Undo();

		delete action;
	}
}

void EditSession::SetupTerrainTypeSelector()
{
	terrainSelectorPopup = CreatePopupPanel("terrainselector");
	GridSelector *terrainSel = terrainSelectorPopup->AddGridSelector(
		"terraintypes", Vector2i(20, 20), TERRAIN_WORLDS, MAX_TERRAINTEX_PER_WORLD, 64, 64, false, true);

	for (int worldI = 0; worldI < TERRAIN_WORLDS; ++worldI)
	{
		int ind;
		for (int i = 0; i < MAX_TERRAINTEX_PER_WORLD; ++i)
		{
			ind = worldI * MAX_TERRAINTEX_PER_WORLD + i;
			stringstream ss;
			ss << "Resources/Terrain/" << "terrain_" << (worldI + 1) << "_0" << (i + 1) << "_512x512.png";
			terrainTextures[ind] = new Texture;
			if (!terrainTextures[ind]->loadFromFile(ss.str()))
			{
				delete terrainTextures[ind];
				terrainTextures[ind] = NULL;
				break;
			}

			terrainSel->Set(worldI, i, Sprite(*terrainTextures[ind], sf::IntRect(0, 0, 64, 64)),
				"xx");

			if (!polyShaders[ind].loadFromFile("Resources/Shader/mat_shader2.frag", sf::Shader::Fragment))
			{
				cout << "MATERIAL SHADER NOT LOADING CORRECTLY EDITOR" << endl;
				assert(0 && "polygon shader not loaded editor");
			}

			polyShaders[ind].setUniform("u_texture", *terrainTextures[ind]);
			polyShaders[ind].setUniform("Resolution", Vector2f(1920, 1080));
			polyShaders[ind].setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));
			polyShaders[ind].setUniform("skyColor", ColorGL(Color::White));
		}
	}
}

void EditSession::SetEnemyGridIndex( GridSelector *gs, int x, int y, const std::string &eName)
{
	gs->Set(x, y, types[eName]->GetSprite(gs->tileSizeX, gs->tileSizeY), eName);
}

void EditSession::SetActiveEnemyGrid(int index)
{
	for (int i = 0; i < 4; ++i)
	{
		enemyGrid[i]->active = false;
	}
	enemyGrid[index]->active = true;
	enemySelectLevel = index+1;
}

void EditSession::SetupEnemyTypes()
{
	for (int i = 0; i < 8; ++i)
	{
		auto &wen = worldEnemyNames[i];
		for (auto it = wen.begin(); it != wen.end(); ++it)
		{
			SetupEnemyType((*it));
		}
	}

	for (auto it = extraEnemyNames.begin(); it != extraEnemyNames.end(); ++it)
	{
		SetupEnemyType((*it));
	}
}

void EditSession::SetupEnemyType(ParamsInfo &pi)
{
	types[pi.name] = new ActorType(pi);
}

void EditSession::RegularOKButton()
{
	if (mode == EDIT)
	{
		ISelectable *select = selectedBrush->objects.front().get();
		ActorParams *ap = (ActorParams*)select;
		ap->SetParams();
	}
	else if (mode == CREATE_ENEMY)
	{
		ActorPtr ac(tempActor);
		ac->SetParams();
		ac->group = groups["--"];

		CreateActor(ac);

		tempActor = NULL;
	}
	showPanel = NULL;
}

void EditSession::RegularCreatePathButton()
{
	mode = CREATE_PATROL_PATH;
	showPanel = NULL;
	Vector2i front = patrolPath.front();
	patrolPath.clear();
	patrolPath.push_back(front);
	patrolPathLengthSize = 0;
}

void EditSession::SelectPoint(PolyPtr poly,
	TerrainPoint *point)
{
	if (!point->selected)
	{
		selectedPoints[poly].push_back(PointMoveInfo(point));
		point->selected = true;
	}
}

void EditSession::DeselectPoint(PolyPtr poly,
	TerrainPoint *point)
{
	if (point->selected)
	{
		point->selected = false;
		auto & infoList = selectedPoints[poly];
		for (auto it = infoList.begin(); it != infoList.end(); ++it)
		{
			if ((*it).point == point)
			{
				infoList.erase(it);
				break;
			}
		}
	}
}

void EditSession::SelectPoint(TerrainRail *rail,
	TerrainPoint *point)
{
	if (!point->selected)
	{
		selectedRailPoints[rail].push_back(PointMoveInfo(point));
		point->selected = true;
	}
}

void EditSession::DeselectPoint(TerrainRail *rail,
	TerrainPoint *point)
{
	if (point->selected)
	{
		point->selected = false;
		auto & infoList = selectedRailPoints[rail];
		for (auto it = infoList.begin(); it != infoList.end(); ++it)
		{
			if ((*it).point == point)
			{
				infoList.erase(it);
				break;
			}
		}
	}
}

void EditSession::MoveSelectedPoints( V2d worldPos )//sf::Vector2i delta )
{
	
	bool validMove = true;

	//num polys

	int numSelectedPolys = selectedPoints.size();
	Vector2i** allDeltas = new Vector2i*[numSelectedPolys];
	int allDeltaIndex = 0;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		TerrainPolygon &poly = *((*it).first);
		selectedBrush->RemoveObject((*it).first);

		int polySize = poly.numPoints;
		Vector2i *deltas = new Vector2i[polySize];
		allDeltas[allDeltaIndex] = deltas;
		int deltaIndex = 0;
						

		double prim_limit = PRIMARY_LIMIT;
		if( IsKeyPressed( Keyboard::LShift ) )
		{
			prim_limit = .99;
		}

		
		//if( false)
		for( TerrainPoint *curr = poly.pointStart; curr != NULL ; curr = curr->next )
		{
			deltas[deltaIndex] = Vector2i( 0, 0 );
			++deltaIndex;
		}
		++allDeltaIndex;
	}

	if( validMove )
	{
		allDeltaIndex = 0;
		for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
		{
			PolyPtr poly = (*it).first;
			bool affected = false;

			TerrainPoint *points = poly->pointStart;
			int deltaIndex = 0;
			for( TerrainPoint *curr = points; curr != NULL; curr = curr->next )
			{
				TerrainPoint *prev;
				if( curr == poly->pointStart )
				{
					prev = poly->pointEnd;
				}
				else
				{
					prev = curr->prev;
				}


				if( curr->selected ) //selected
				{					
								
					Vector2i delta = allDeltas[allDeltaIndex][deltaIndex];

					curr->pos += pointGrabDelta - delta;

					if( curr->gate != NULL )
					{
						curr->gate->UpdateLine();
					}

					poly->UpdateLineColor(poly->lines, prev, poly->GetPointIndex(prev) * 2);
					poly->UpdateLineColor(poly->lines, curr, poly->GetPointIndex(curr) * 2);

					if( poly->enemies.count( curr ) > 0 )
					{
						list<ActorPtr> &enemies = poly->enemies[curr];
						for( list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
						{
							//(*ait)->UpdateGroundedSprite();
											
						}
						//revquant is the quantity from the edge's v1
						//double revQuant = 
					}

					affected = true;
				}

				++deltaIndex;
			}

			poly->UpdateBounds();

			if( affected )
			{
				poly->movingPointMode = true;

				for( map<TerrainPoint*,list<ActorPtr>>::iterator mit = poly->enemies.begin();
					mit != poly->enemies.end(); ++mit )
				{
					list<ActorPtr> &enemies = (*mit).second;//(*it)->enemies[curr];
					for( list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
					{
						(*ait)->UpdateGroundedSprite();
						(*ait)->SetBoundingQuad();
					}
					//revquant is the quantity from the edge's v1
					//double revQuant = 	
				}
			}

			++allDeltaIndex;			
		}	
	}
	else
	{
		//cout << "NOT VALID move" << endl;
	}

	for( int i = 0; i < numSelectedPolys; ++i )
	{
		delete [] allDeltas[i];
	}
	delete [] allDeltas;
}

void EditSession::MoveSelectedRailPoints(V2d worldPos)
{
	/*pointGrabDelta = Vector2i(worldPos.x, worldPos.y) - pointGrabPos;

	Vector2i oldPointGrabPos = pointGrabPos;
	pointGrabPos = Vector2i(worldPos.x, worldPos.y);*/// - Vector2i( pointGrabDelta.x % 32, pointGrabDelta.y % 32 );
	bool validMove = true;
	//num polys

	int numSelectedRails = selectedRailPoints.size();
	Vector2i** allDeltas = new Vector2i*[numSelectedRails];
	int allDeltaIndex = 0;
	for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
	{
		TerrainRail &rail = *((*it).first);

		int railSize = rail.numPoints;
		Vector2i *deltas = new Vector2i[railSize];
		allDeltas[allDeltaIndex] = deltas;
		int deltaIndex = 0;

		double prim_limit = PRIMARY_LIMIT;
		if (IsKeyPressed(Keyboard::LShift))
		{
			prim_limit = .99;
		}


		for (TerrainPoint *curr = rail.pointStart; curr != NULL; curr = curr->next)
		{
			deltas[deltaIndex] = Vector2i(0, 0);

			if (!curr->selected)
			{
				++deltaIndex;
				continue;
			}

			Vector2i diff;

			TerrainPoint *prev, *next;

			prev = curr->prev;
			next = curr->next;

			V2d prevPos, nextPos, newVec, normVec;
			V2d pos(curr->pos.x + pointGrabDelta.x, curr->pos.y + pointGrabDelta.y);
			V2d extreme(0, 0);
			Vector2i vec;

			if (prev != NULL)
			{
				prevPos = V2d(prev->pos.x, prev->pos.y);
				vec = curr->pos - prev->pos;
				normVec = normalize(V2d(vec.x, vec.y));
				newVec = normalize(pos - V2d(prev->pos.x, prev->pos.y));

				if (!prev->selected)
				{
					if (normVec.x == 0 || normVec.y == 0)
					{
						if (newVec.x > prim_limit)
							extreme.x = 1;
						else if (newVec.x < -prim_limit)
							extreme.x = -1;
						if (newVec.y > prim_limit)
							extreme.y = 1;
						else if (newVec.y < -prim_limit)
							extreme.y = -1;

						if (extreme.x != 0)
						{
							pointGrabPos.y = oldPointGrabPos.y;
							pointGrabDelta.y = 0;
						}

						if (extreme.y != 0)
						{
							pointGrabPos.x = oldPointGrabPos.x;
							pointGrabDelta.x = 0;
						}
					}
					else
					{
						if (normVec.x > prim_limit)
							extreme.x = 1;
						else if (normVec.x < -prim_limit)
							extreme.x = -1;
						if (normVec.y > prim_limit)
							extreme.y = 1;
						else if (normVec.y < -prim_limit)
							extreme.y = -1;

						if (extreme.x != 0)
						{
							//int diff = ;
							diff.y = curr->pos.y - prev->pos.y;

							//(*it2).pos.y = (*prev).pos.y;
							cout << "lining up x: " << diff.y << endl;
						}

						if (extreme.y != 0)
						{
							diff.x = curr->pos.x - prev->pos.x;

							cout << "lining up y: " << diff.x << endl;
						}
					}
				}
			}
			if (next != NULL)
			{
				nextPos = V2d(next->pos.x, next->pos.y);

				if (!next->selected)
				{
					vec = curr->pos - next->pos;
					normVec = normalize(V2d(vec.x, vec.y));

					extreme = V2d(0, 0);

					newVec = normalize(pos - V2d((*next).pos.x, (*next).pos.y));

					if (normVec.x == 0 || normVec.y == 0)
					{
						if (newVec.x > prim_limit)
							extreme.x = 1;
						else if (newVec.x < -prim_limit)
							extreme.x = -1;
						if (newVec.y > prim_limit)
							extreme.y = 1;
						else if (newVec.y < -prim_limit)
							extreme.y = -1;

						if (extreme.x != 0)
						{
							pointGrabPos.y = oldPointGrabPos.y;
							pointGrabDelta.y = 0;
						}

						if (extreme.y != 0)
						{
							pointGrabPos.x = oldPointGrabPos.x;
							pointGrabDelta.x = 0;
						}
					}
					else
					{
						if (normVec.x > prim_limit)
							extreme.x = 1;
						else if (normVec.x < -prim_limit)
							extreme.x = -1;
						if (normVec.y > prim_limit)
							extreme.y = 1;
						else if (normVec.y < -prim_limit)
							extreme.y = -1;

						if (extreme.x != 0)
						{
							//int diff = ;
							//diff.y = curr->pos.y - next->pos.y;

							//(*it2).pos.y = (*prev).pos.y;
							cout << "lining up x222: " << diff.y << endl;
						}

						if (extreme.y != 0)
						{
							//diff.x = curr->pos.x - next->pos.x;

							cout << "lining up y222: " << diff.x << endl;
						}
					}
				}
			}

			if (!(diff.x == 0 && diff.y == 0))
			{
				cout << "allindex: " << allDeltaIndex << ", deltaIndex: " << deltaIndex << endl;
				cout << "diff: " << diff.x << ", " << diff.y << endl;
			}
			deltas[deltaIndex] = diff;

			++deltaIndex;
		}
		++allDeltaIndex;
	}

	if (validMove)
	{
		allDeltaIndex = 0;
		for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
		{
			TerrainRail *rail = (*it).first;
			bool affected = false;

			TerrainPoint *points = rail->pointStart;
			int deltaIndex = 0;
			for (TerrainPoint *curr = points; curr != NULL; curr = curr->next)
			{
				if (curr->selected) //selected
				{
					Vector2i delta = allDeltas[allDeltaIndex][deltaIndex];
					Vector2i testDiff = pointGrabDelta - delta;
					curr->pos += pointGrabDelta - delta;
					cout << "moving point: " << testDiff.x << ", " << testDiff.y << endl;
					//rail->UpdateLineColor(rail->lines, prev, rail->GetPointIndex(prev) * 2);
					//rail->UpdateLineColor(rail->lines, curr, rail->GetPointIndex(curr) * 2);

					if (rail->enemies.count(curr) > 0)
					{
						list<ActorPtr> &enemies = rail->enemies[curr];
						for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
						{
							//(*ait)->UpdateGroundedSprite();

						}
						//revquant is the quantity from the edge's v1
						//double revQuant = 
					}

					affected = true;
				}

				++deltaIndex;
			}

			rail->UpdateBounds();

			if (affected)
			{
				rail->movingPointMode = true;

				for (auto mit = rail->enemies.begin();
					mit != rail->enemies.end(); ++mit)
				{
					list<ActorPtr> &enemies = (*mit).second;
					for (auto ait = enemies.begin(); ait != enemies.end(); ++ait)
					{
						(*ait)->UpdateGroundedSprite();
						(*ait)->SetBoundingQuad();
					}
				}
			}

			++allDeltaIndex;
		}
	}
	else
	{
		//cout << "NOT VALID move" << endl;
	}

	for (int i = 0; i < numSelectedRails; ++i)
	{
		delete[] allDeltas[i];
	}
	delete[] allDeltas;
}


bool EditSession::IsGateAttachedToAffectedPoints(
	GateInfoPtr gi, PointVectorMap &pm,
	bool &a)
{
	PolyPtr poly;
	for (auto pit = pm.begin(); pit != pm.end(); ++pit)
	{
		poly = (*pit).first;
		if (gi->poly0 == poly || gi->poly1 == poly)
		{
			if (gi->poly0 == poly)
			{
				a = true;
			}
			else
			{
				a = false;
			}
			return true;
		}
	}

	return false;
}

bool EditSession::IsGateAttachedToAffectedPoints(
	GateInfoPtr gi, PointMap &pm,
	bool &a)
{
	PolyPtr poly;
	for (auto pit = pm.begin(); pit != pm.end(); ++pit)
	{
		poly = (*pit).first;
		if (gi->poly0 == poly || gi->poly1 == poly)
		{
			if (gi->poly0 == poly)
			{
				a = true;
			}
			else
			{
				a = false;
			}
			return true;
		}
	}

	return false;
}

bool EditSession::IsGateAttachedToAffectedPoly(
	GateInfoPtr gi, Brush *b,
	bool &a)
{
	for (auto bit = b->objects.begin(); bit != b->objects.end(); ++bit)
	{
		if ((*bit)->selectableType == ISelectable::ISelectableType::TERRAIN)
		{
			PolyPtr p = boost::static_pointer_cast<TerrainPolygon>((*bit));
			if (gi->poly0 == p || gi->poly1 == p)
			{
				if (gi->poly0 == p)
				{
					a = true;
				}
				else
				{
					a = false;
				}

				return true;
			}
		}
	}

	return false;
}

bool EditSession::BlahTest( GateInfoPtr gi, bool polyMove, bool a, CompoundAction *testAction)
{
	return true;

}

void EditSession::PerformMovePointsAction()
{
	Vector2i delta = Vector2i(worldPos.x, worldPos.y) - editMouseOrigPos;
	//here the delta being subtracted is the points original positionv
	
	PointVectorMap pm;
	
	for (PointMap::iterator mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit)
	{
		PolyPtr poly = (*mit).first;
		selectedBrush->RemoveObject(poly);
		auto &pmVec = pm[poly];
		pmVec.reserve(poly->numPoints);

		for (TerrainPoint *curr = poly->pointStart; curr != NULL; curr = curr->next)
		{
			PointMoveInfo pi(curr);
			
			if (curr->selected)
			{
				pi.delta = delta;
				pi.origPos = pi.point->pos - delta;
				pi.moveIntent = true;
			}

			pmVec.push_back(pi);
		}
	}

	/*for (PointMap::iterator mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit)
	{
		list<PointMoveInfo> &pList = (*mit).second;
		for (list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it)
		{
			(*it).origPos = (*it).delta;
			(*it).delta = (*it).point->pos - (*it).delta;
		}
	}*/

	for (auto mit = selectedRailPoints.begin(); mit != selectedRailPoints.end(); ++mit)
	{
		list<PointMoveInfo> &pList = (*mit).second;
		for (list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it)
		{
			(*it).delta = (*it).point->pos - (*it).delta;
		}
	}
	
	MoveBrushAction *action = new MoveBrushAction(selectedBrush, delta, false, pm, selectedRailPoints);
	action->Perform();

	CompoundAction *testAction = NULL;
	if (moveAction != NULL)
	{
		testAction = moveAction;
	}
	else
	{
		testAction = new CompoundAction;
	}
	testAction->subActions.push_back(action);

	
	if (action->moveValid)
	{
		int gateActionsAdded = 0;
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			bool gateAttachedToAffectedPoly = false;
			PolyPtr poly;
			bool a = true;
			bool polyMove = true;

			if (IsGateAttachedToAffectedPoints((*it), selectedPoints, a))
			{
				polyMove = false;
				gateAttachedToAffectedPoly = true;
			}

			if (IsGateAttachedToAffectedPoly((*it), selectedBrush, a))
			{
				//should this even happen if the points are moving?
				gateAttachedToAffectedPoly = true;
			}

			/*for (auto pit = selectedPoints.begin(); pit != selectedPoints.end(); ++pit)
			{
				poly = (*pit).first;
				if ((*it)->poly0 == poly || (*it)->poly1 == poly)
				{
					if ((*it)->poly0 == poly)
					{
						a = true;
					}
					else
					{
						a = false;
					}
					polyMove = false;
					gateAttachedToAffectedPoly = true;
					break;
				}
			}*/


			/*for (auto bit = selectedBrush->objects.begin(); bit != selectedBrush->objects.end(); ++bit)
			{
				if ((*bit)->selectableType == ISelectable::ISelectableType::TERRAIN)
				{
					PolyPtr p = boost::static_pointer_cast<TerrainPolygon>((*bit));
					if ((*it)->poly0 == p || (*it)->poly1 == p)
					{
						if ((*it)->poly0 == p)
						{
							a = true;
						}
						else
						{
							a = false;
						}

						gateAttachedToAffectedPoly = true;
						break;
					}
				}
			}*/

			if (gateAttachedToAffectedPoly)
			{
				GateInfo *gi = (*it).get();
				Vector2i adjust;
				Vector2i pA, pB;

				GateAdjustOption gaOption;
				if (polyMove)
				{
					if (a)
					{
						gaOption = GATEADJUST_A;
					}
					else
					{
						gaOption = GATEADJUST_B;
					}
				}
				else
				{
					if (a)
					{
						gaOption = GATEADJUST_POINT_B;
					}
					else
					{
						gaOption = GATEADJUST_POINT_A;
					}
				}
				
				if (GetPrimaryAdjustment(gi->point0->pos, gi->point1->pos, adjust))
				{

					if (!TryGateAdjustAction(gaOption, gi, adjust, testAction))
					{
						testAction->Undo();

						if (testAction == moveAction)
						{
							moveAction = NULL;
						}

						delete testAction;
						return;
					}
					else
					{
						gateActionsAdded++;
					}
				}
				else
				{
					//action->Perform();
					//doneActionStack.push_back(action);
				}
			}



		}

		testAction->performed = true;
		doneActionStack.push_back(testAction);
		/*if (gateActionsAdded > 0)
		{
			testAction->performed = true;
			doneActionStack.push_back(testAction);
		}
		else
		{
			if (moveAction != NULL)
			{
				moveAction->subActions.push_back(action);
				doneActionStack.push_back(moveAction);
			}
			else
			{
				doneActionStack.push_back(action);
			}
		}*/
	}
	else
	{
		testAction->Undo();

		if (testAction == moveAction)
		{
			moveAction = NULL;
		}

		delete testAction;
	}
}

bool EditSession::PolyContainsPolys(TerrainPolygon *p, TerrainPolygon *ignore)
{
	auto &testPolygons = GetCorrectPolygonList(p);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if ((*it).get() == ignore)
			continue;

		if (p->Contains((*it).get()))
		{
			return true;
		}
	}
}

bool EditSession::PolyIsContainedByPolys(TerrainPolygon *p, TerrainPolygon *ignore)
{
	auto &testPolygons = GetCorrectPolygonList(p);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if ((*it).get() == ignore)
			continue;

		if ((*it)->Contains(p))
		{
			return true;
		}
	}
}

bool EditSession::PolyIsTouchingEnemiesOrBeingTouched( TerrainPolygon *p, TerrainPolygon *ignore)
{
	//this also tests for your own enemies to check for validity.
	auto &testPolygons = GetCorrectPolygonList(p);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if ((*it).get() == ignore)
			continue;

		if ((*it)->IsTouchingEnemiesFromPoly(p))
		{
			return true;
		}

		if (p->IsTouchingEnemiesFromPoly((*it).get()))
		{
			return true;
		}
	}


	return false;
}

bool EditSession::GateIsTouchingEnemies(GateInfo *gi)
{
	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		auto actorList = (*it).second->actors;
		for (auto ait = actorList.begin(); ait != actorList.end(); ++ait)
		{
			if (gi->IsTouchingEnemy((*ait).get()))
			{
				return true;
			}
		}
	}
	return false;
}

bool EditSession::PolyIntersectsGates(TerrainPolygon *poly)
{
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if (poly->IntersectsGate((*it).get()))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::GateIntersectsPolys(GateInfo *gi)
{
	auto &testPolygons = GetCorrectPolygonList(0);

	for (auto pit = testPolygons.begin(); pit != testPolygons.end(); ++pit)
	{
		if ((*pit)->IntersectsGate(gi))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::GateIntersectsGates(GateInfo *gi)
{
	Vector2i myPoint0, myPoint1, otherPoint0, otherPoint1;
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it).get() == gi)
		{
			continue;
		}

		myPoint0 = gi->point0->pos;
		myPoint1 = gi->point1->pos;

		otherPoint0 = (*it)->point0->pos;
		otherPoint1 = (*it)->point1->pos;

		LineIntersection li = EditSession::SegmentIntersect(myPoint0, myPoint1, otherPoint0, otherPoint1);
		if (!li.parallel)
		{
			return true;
		}
	}
	return false;
}

bool EditSession::PolyGatesIntersectOthers(TerrainPolygon *poly)
{
	auto &testPolygons = GetCorrectPolygonList(poly);

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it)->poly0.get() == poly || (*it)->poly1.get() == poly)
		{	
			for (auto pit = testPolygons.begin(); pit != testPolygons.end(); ++pit)
			{
				if ((*pit).get() == poly)
					continue;

				if ((*pit)->IntersectsGate((*it).get()))
				{
					return true;
				}		
			}
		}
	}

	return false;
}

bool EditSession::IsGateInProgressValid(PolyPtr startPoly,
	TerrainPoint *startPoint)
{
	if (gateInProgressTestPoly == NULL)
	{
		return false;
	}
	else
	{
		GateInfo tempGate;
		tempGate.poly0 = startPoly;
		tempGate.point0 = startPoint;
		tempGate.poly1 = gateInProgressTestPoly;
		tempGate.point1 = gateInProgressTestPoint;
		return IsGateValid(&tempGate);

	}
}

bool EditSession::IsGateValid(GateInfo *gi)
{
	if (gi->point0 == gi->point1)
		return false;

	if (GateMakesSliverAngles(gi))
	{
		return false;
	}

	if (GateIntersectsPolys(gi))
	{
		return false;
	}

	if (GateIntersectsGates(gi))
	{
		return false;
	}

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it)->point0 == gi->point0 || (*it)->point0 == gi->point1
			|| (*it)->point1 == gi->point0 || (*it)->point1 == gi->point1)
		{
			return false;
		}
	}

	if (GateIsTouchingEnemies(gi))
	{
		return false;
	}

	Vector2f center(Vector2f(gi->point0->pos + gi->point1->pos) / 2.f);
	if (gi->poly0 == gi->poly1)
	{
		if (gi->poly0->ContainsPoint(center))
		{
			return false;
		}
	}

	//check slivers

	//check intersections

	//make sure you cant go within a single poly.
	return true;
}

bool EditSession::GateMakesSliverAngles(GateInfo *gi)
{
	TerrainPolygon *poly0 = gi->poly0.get();
	TerrainPolygon *poly1 = gi->poly1.get();

	TerrainPoint *p0 = gi->point0;
	TerrainPoint *p1 = gi->point1;

	TerrainPoint *prev = p0->prev;
	if (prev == NULL)
		prev = poly0->pointEnd;

	TerrainPoint *next = p0->next;
	if (next == NULL)
		next = poly0->pointStart;

	if (IsSliver(prev, p0, p1))
	{
		return true;
	}

	if (IsSliver(p1, p0, next))
	{
		return true;
	}

	prev = p1->prev;
	if (prev == NULL)
	{
		prev = poly1->pointEnd;
	}

	next = p1->next;
	if (next == NULL)
	{
		next = poly1->pointStart;
	}

	if (IsSliver(prev, p1, p0))
	{
		return true;
	}

	if (IsSliver(p0, p1, next))
	{
		return true;
	}

	return false;
}


bool EditSession::IsSliver( TerrainPoint *prev, TerrainPoint *curr, TerrainPoint *next)
{
	V2d pos(curr->pos.x, curr->pos.y);
	V2d prevPos(prev->pos.x, prev->pos.y);
	V2d nextPos(next->pos.x, next->pos.y);
	V2d dirA = normalize(prevPos - pos);
	V2d dirB = normalize(nextPos - pos);

	double diff = GetVectorAngleDiffCCW(dirA, dirB);
	double diffCW = GetVectorAngleDiffCW(dirA, dirB);
	if (diff < SLIVER_LIMIT)
	{
		return true;
	}
	else if (diffCW < SLIVER_LIMIT)
	{
		return true;
	}

	return false;
}

bool EditSession::PolyGatesMakeSliverAngles(TerrainPolygon *poly)
{
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it)->poly0.get() == poly || (*it)->poly1.get() == poly)
		{
			if (GateMakesSliverAngles((*it).get()))
			{
				return true;
			}
		}
	}

	return false;
}

bool EditSession::IsCloseToPrimary(sf::Vector2i &p0,
	sf::Vector2i &p1, sf::Vector2i &prim)
{
	Vector2i diff = p1 - p0;
	prim = Vector2i(0, 0);

	if (diff.x == 0 || diff.y == 0)
		return false;

	V2d diffDir = normalize(V2d(diff));

	if (diffDir.x > PRIMARY_LIMIT)
		prim.x = 1;
	else if (diffDir.x < -PRIMARY_LIMIT)
		prim.x = -1;
	if (diffDir.y > PRIMARY_LIMIT)
		prim.y = 1;
	else if (diffDir.y < -PRIMARY_LIMIT)
		prim.y = -1;

	if (prim.x != 0 || prim.y != 0)
	{
		return true;
	}

	return false;
}

bool EditSession::GetPrimaryAdjustment(sf::Vector2i &p0,
	sf::Vector2i &p1, sf::Vector2i &adjust)
{
	adjust = Vector2i(0, 0);
	Vector2i prim;
	if (IsCloseToPrimary(p0, p1, prim))
	{
		if (prim.x != 0 )
		{
			adjust.y = p1.y - p0.y;
		}
		else if (prim.y != 0)
		{
			adjust.x = p1.x - p0.x;
		}
		return true;
	}
	return false;
}

bool EditSession::TryGateAdjustAction( GateAdjustOption option,
	GateInfo *gi, Vector2i &adjust, CompoundAction *compound)
{
	Action *adjustAction = NULL;
	bool success = false;
	switch (option)
	{
	case GATEADJUST_A:
		assert(gi->poly0 != gi->poly1);
		success = TryGateAdjustActionPoly( gi, adjust, true, gi->poly0,compound);
		break;
	case GATEADJUST_B:
		assert(gi->poly0 != gi->poly1);
		success = TryGateAdjustActionPoly( gi, -adjust, false, gi->poly1,compound);
		break;
	case GATEADJUST_MIDDLE:
		break;
	case GATEADJUST_POINT_A:
	{
		success = TryGateAdjustActionPoint(gi, adjust, true,compound);
		break;
	}
	case GATEADJUST_POINT_B:
	{
		success = TryGateAdjustActionPoint(gi, -adjust, false,compound);
		break;
	}
	case GATEADJUST_POINT_MIDDLE:
		break;
	}
	

	return success;
}

bool EditSession::TryGateAdjustActionPoly( GateInfo *gi, sf::Vector2i &adjust, bool a, PolyPtr p, CompoundAction *compound)
{
	Brush b;
	b.AddObject(p);

	MoveBrushAction *action = new MoveBrushAction(&b, adjust, true, PointVectorMap(), RailPointMap());
	action->Perform();

	if (action->moveValid)
	{
		compound->subActions.push_back(action);

		Brush attachedPolys;
		PolyPtr p0, p1;
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it).get() == gi)
				continue;

			p0 = (*it)->poly0;
			p1 = (*it)->poly1;

			if ( p0 == p && p1 == p )
			{
				continue;
			}
			else if (p0 == p)
			{
				attachedPolys.AddObject(p1);
			}
			else if( p1 == p )
			{
				attachedPolys.AddObject(p0);
			}
		}


		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it).get() == gi)
				continue;

			bool gateAttachedToAffectedPoly = false;
			PolyPtr poly;
			bool a = true;
			if (IsGateAttachedToAffectedPoly((*it),&attachedPolys,a))
			{
				GateInfo *gi = (*it).get();
				Vector2i adjust;
				Vector2i pA, pB;

				GateAdjustOption gaOption;

				if (a)
				{
					gaOption = GATEADJUST_B;
				}
				else
				{
					gaOption = GATEADJUST_A;
				}


				if (GetPrimaryAdjustment(gi->point0->pos, gi->point1->pos, adjust))
				{
					if (!TryGateAdjustAction(gaOption, gi, adjust, compound))
					{
						return false;
					}
				}
			}
		}
		return true;


		compound->subActions.push_back(action);
	}
	else
	{
		action->Undo();
		delete action;
		return false;
	}

	//check if its okay!

	return true;
}

bool EditSession::TryGateAdjustActionPoint( GateInfo *gi, Vector2i &adjust, bool a, CompoundAction *compound)
{
	PointVectorMap pmap;
	PolyPtr poly;
	TerrainPoint *point;

	if (a)
	{
		poly = gi->poly0;
		point = gi->point0;
	}
	else
	{
		poly = gi->poly1;
		point = gi->point1;
	}

	auto &pVec = pmap[poly];

	pVec.reserve(poly->numPoints);
	for (TerrainPoint *curr = poly->pointStart; curr != NULL; curr = curr->next)
	{
		PointMoveInfo pi(curr);
		if (curr == point)
		{
			pi.delta = adjust;
			pi.moveIntent = true;
			//SelectPoint(poly, curr);
		}

		pVec.push_back(pi);
	}
	
	MoveBrushAction * action = new MoveBrushAction(selectedBrush, Vector2i(), true, pmap, RailPointMap());
	action->Perform();

	//action->CheckValidPointMove();

	if (action->moveValid)
	{
		compound->subActions.push_back(action);

		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it).get() == gi)
				continue;

			bool gateAttachedToAffectedPoly = false;
			PolyPtr poly;
			bool a = true;
			if (IsGateAttachedToAffectedPoints((*it), pmap, a))
			{
				GateInfo *gi = (*it).get();
				Vector2i adjust;
				Vector2i pA, pB;

				GateAdjustOption gaOption;

				if (a)
				{
					gaOption = GATEADJUST_POINT_B;
				}
				else
				{
					gaOption = GATEADJUST_POINT_A;
				}


				if (GetPrimaryAdjustment(gi->point0->pos, gi->point1->pos, adjust))
				{
					if (!TryGateAdjustAction(gaOption, gi, adjust, compound))
					{
						return false;
					}
				}
			}
		}
		return true;
	}
	else
	{
		action->Undo();
		delete action;
		return false;
	}
}

void EditSession::GetNearPrimaryGateList(PointMap &pmap, list<GateInfoPtr> & gList)
{
	//this doesnt work because the gate hasnt been assigned to the points yet.
	Vector2i prim;
	for (auto it = pmap.begin(); it != pmap.end(); ++it)
	{
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			GateInfo *gi = (*pit).point->gate.get();
			if (gi != NULL)
			{
				if (IsCloseToPrimary(gi->point0->pos, gi->point1->pos, prim))
				{
					gList.push_back((*pit).point->gate);
				}
			}
		}
	}
}


bool EditSession::IsPolygonExternallyValid( TerrainPolygon *poly, TerrainPolygon *ignore )
{
	list<PolyPtr> intersections;
	GetIntersectingPolys(poly, intersections);
	if (intersections.size() > 0)
		return false;

	if (PolyContainsPolys(poly, ignore ))
	{
		return false;
	}

	if (PolyIsContainedByPolys(poly, ignore ))
	{
		return false;
	}

	if (PolyIsTouchingEnemiesOrBeingTouched(poly, ignore ))
	{
		return false;
	}

	if (PolyIntersectsGates(poly))
	{
		return false;
	}

	if (PolyGatesIntersectOthers(poly))
	{
		return false;
	}

	if (PolyGatesMakeSliverAngles(poly))
	{
		return false;
	}

	//list<GateInfoPtr> gList;
	//GetNearPrimaryGateList( )


	return true;
}

bool EditSession::IsPolygonValid( TerrainPolygon *poly, TerrainPolygon *ignore )
{
	bool a = IsPolygonExternallyValid(poly, ignore);
	bool b = poly->IsInternallyValid();

	return a && b;
}

bool EditSession::ConfirmationPop( const std::string &question )
{

	confirm->labels["question"]->setString( question );

	confirmChoice = ConfirmChoices::NONE;

	w->setView( v );
	
	//preScreenTex->setView( uiView );	
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );

	sf::Event ev;
	while( confirmChoice == ConfirmChoices::NONE )
	{
		Vector2i pPos = GetPixelPos();
		Vector2f uiMouse = preScreenTex->mapPixelToCoords(pPos);
		w->clear();
		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						confirm->Update( true, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					confirm->Update( false, uiMouse.x, uiMouse.y );
					break;
				}
			case Event::MouseWheelMoved:
				{
					break;
				}
			case Event::KeyPressed:
				{
					confirm->SendKey( ev.key.code, ev.key.shift );
					break;
				}
			case Event::KeyReleased:
				{
					break;
				}
			case Event::LostFocus:
				{
					break;
				}
			case Event::GainedFocus:
				{
					break;
				}
			}
					break;	
		}
		//cout << "drawing confirm" << endl;

		w->setView( v );

		w->draw( preTexSprite );

		w->setView( uiView );

		confirm->Draw( w );

		w->setView( v );

		w->display();
		//preScreenTex->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
	//preScreenTex->setView( view );

	if( confirmChoice == ConfirmChoices::CONFIRM )
	{
		return true;
	}
	else if( confirmChoice == ConfirmChoices::CANCEL )
	{
		return false;
	}
}

void EditSession::MessagePop( const std::string &message )
{
	messagePopup->labels["message"]->setString( message );
	bool closePopup = false;
	w->setView( v );
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );

	sf::Event ev;
	while( !closePopup )
	{
		Vector2i pPos = GetPixelPos();
		Vector2f uiMouse = preScreenTex->mapPixelToCoords(pPos);
		w->clear();

		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						//if( uiMouse.x < messagePopup->pos.x 
						//messagePopup->Update( true, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					closePopup = true;
					//messagePopup->Update( false, uiMouse.x, uiMouse.y );
					break;
				}
			case Event::MouseWheelMoved:
				{
					break;
				}
			case Event::KeyPressed:
				{
					closePopup = true;
					//messagePopup->SendKey( ev.key.code, ev.key.shift );
					break;
				}
			case Event::KeyReleased:
				{
					break;
				}
			case Event::LostFocus:
				{
					break;
				}
			case Event::GainedFocus:
				{
					break;
				}
			}
			break;	
		}

		w->setView( v );

		w->draw( preTexSprite );

		w->setView( uiView );

		messagePopup->Draw( w );

		w->setView( v );

		w->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
	//preScreenTex->setView( view );
}

void EditSession::ErrorPop( const std::string &error )
{

}


void EditSession::GridSelectPop( const std::string &type )
{
	Panel *panel = NULL;
	if( type == "gateselect" )
	{
		panel = gateSelectorPopup;
	}
	else if( type == "terraintypeselect" )
	{
		panel = terrainSelectorPopup;
	}
	else if (type == "bg")
	{
		panel = bgPopup;
	}
	else if (type == "shardselector")
	{
		panel = shardSelectPopup;//types["shard"]->panel;
	}

	assert( panel != NULL );
	int selectedIndex = -1;
	tempGridResult = "nothing";
	bool closePopup = false;
	w->setView( v );
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	


	preScreenTex->setView( uiView );

	Vector2i pPos = Vector2i(960, 540) - Vector2i( panel->size.x / 2, panel->size.y / 2 );//sf::Mouse::getPosition( *w );
	pPos.x *= 1920 / w->getSize().x;
	pPos.y *= 1920 / w->getSize().y;

	Vector2f uiMouse = preScreenTex->mapPixelToCoords(pPos);



	panel->pos.x = uiMouse.x;
	panel->pos.y = uiMouse.y;

	sf::Event ev;
	while( !closePopup )
	{
		pPos = GetPixelPos();
		uiMouse = preScreenTex->mapPixelToCoords(pPos);
		w->clear();

		bool shardClose = tempGridResult == "shardclose";
		if (panel != shardSelectPopup)
		{
			if (tempGridResult != "nothing")
			{
				return;
			}
		}
		else
		{
			if (tempGridResult == "shardclose")
			{
				return;
			}
		}

		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						cout << "are we here: " << uiMouse.x << ", " << uiMouse.y << endl;
						panel->Update( true, uiMouse.x, uiMouse.y );
						//if you click outside of the box, delete the gate
						
						//if( uiMouse.x < messagePopup->pos.x 
						//messagePopup->Update( true, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					//closePopup = true;
					if( ev.mouseButton.button == Mouse::Left )
					{
						cout << "are we real: " << uiMouse.x << ", " << uiMouse.y << endl;
						panel->Update( false, uiMouse.x, uiMouse.y );
					}
					break;
				}
			case Event::MouseWheelMoved:
				{
					break;
				}
			case Event::KeyPressed:
				{
					//closePopup = true;
					//messagePopup->SendKey( ev.key.code, ev.key.shift );
					break;
				}
			case Event::KeyReleased:
				{
					break;
				}
			case Event::LostFocus:
				{
					break;
				}
			case Event::GainedFocus:
				{
					break;
				}
			}
			break;	
		}

		w->setView( v );

		w->draw( preTexSprite );

		w->setView( uiView );

		//messagePopup->Draw( w );
		panel->Draw( w );

		w->setView( v );

		w->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
}
Panel * EditSession::CreatePopupPanel( const std::string &type )
{
	Panel *p = NULL;
	if( type == "message" )
	{
		p = new Panel( "message_popup", 400, 100, this );
		p->pos.x = 300;
		p->pos.y = 300;
		//p->AddButton( "ok", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "message", Vector2i( 10, 10 ), 12, "_EMPTY\n_MESSAGE_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
		//p->
	}
	else if( type == "error" )
	{
		p = new Panel( "error_popup", 400, 100, this );
		//p->AddButton( "ok", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "message", Vector2i( 25, 50 ), 12, "_EMPTY_ERROR_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
	}
	else if( type == "confirmation" )
	{
		p = new Panel( "confirmation_popup", 400, 100, this );
		p->AddButton( "confirmOK", Vector2i( 50, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddButton( "cancel", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "Cancel" );
		p->AddLabel( "question", Vector2i( 10, 10 ), 12, "_EMPTY\n_QUESTION_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
	}
	else if( type == "gateselector" )
	{
		p = new Panel( "gate_popup", 200, 500, this );
	}
	else if( type == "terrainselector" )
	{
		p = new Panel( "terrain_popup", 100, 100, this );
	}
	else if (type == "bg")
	{
		p = new Panel("bg_popup", 1500, 600, this);
	}
	else if (type == "shardselector")
	{
		p = new Panel("shardselector", 700, 1080, this);
		p->AddLabel("shardtype", Vector2i(20, 900), 24, "SHARD_W1_TEACH_JUMP");
		CreateShardGridSelector(p, Vector2i(0, 0));
		p->AddButton("ok", Vector2i(100, 1000), Vector2f(100, 50), "OK");
	}

	if( p != NULL )
		allPopups.push_back(p);

	return p;
}

//-1 means you denied it, 0 means it didnt work, and 1 means it will work
int EditSession::IsRemovePointsOkay()
{
	bool terrainOkay = true;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		PolyPtr tp = (*it).first;
		bool res = tp->IsRemovePointsOkayTerrain( this );
		if( !res )
		{
			terrainOkay = false;
			break;
		}
	}

	if( !terrainOkay )
	{
		return 0;
	}

	return 1;
}

Panel * EditSession::CreateOptionsPanel( const std::string &name )
{
	Panel *p = NULL;
	
	if( name == "map" )
	{
		p = new Panel( "map_options", 200, 800, this );
		p->AddButton( "ok", Vector2i( 100, 600 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		p->AddLabel("draintime_label", Vector2i(20, 200), 20, "drain seconds:");
		p->AddTextBox("draintime", Vector2i(20, 250), 200, 20, "60");
		p->AddTextBox("bosstype", Vector2i(20, 300), 200, 20, "0");
		p->AddButton( "envtype", Vector2i(20, 400), Vector2f(100, 50), "Choose BG");
		//p->AddTextBox("envtype", Vector2i(20, 400), 200, 20, "w1_01");
	}
	else if( name == "terrain" )
	{
		p = new Panel( "terrain_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		//p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		//p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		p->AddButton( "create_path", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "Create Path" );
	}
	else if (name == "rail")
	{
		p = new Panel("rail_options", 200, 600, this);
		p->AddButton("ok", Vector2i(100, 500), Vector2f(100, 50), "OK");
		p->AddLabel( "requirepower", Vector2i( 20, 50), 20, "require power:" );
		p->AddCheckBox("requirepower", Vector2i(20, 100));
		p->AddLabel("accelerate", Vector2i(20, 150), 20, "accelerate:");
		p->AddCheckBox("accelerate", Vector2i(20, 200));
		p->AddButton("reverse", Vector2i(20, 300), Vector2f(100, 50), "Reverse Dir");
		p->AddTextBox("level", Vector2i(20, 400), 200, 20, "");
	}
	if( p != NULL )
		allPopups.push_back(p);

	return p;
}

void EditSession::SetPanelDefault( ActorType *type )
{
}

void EditSession::SetEnemyEditPanel()
{
	//eventually set this up so that I can give the same parameters to multiple copies of the same enemy?
	//need to be able to apply paths simultaneously to multiples also
	ISelectable *sp = selectedBrush->objects.front().get();
	assert( sp->selectableType == ISelectable::ACTOR );
	ActorParams *ap = (ActorParams*)sp;
	
	ActorType *type = ap->type;
	string name = type->info.name;

	Panel *p = type->panel;

	ap->SetPanelInfo();

	showPanel = p;
}

void EditSession::SetDecorEditPanel()
{
	ISelectable *sp = selectedBrush->objects.front().get();
	assert(sp->selectableType == ISelectable::IMAGE);
	EditorDecorInfo *di = (EditorDecorInfo*)sp;

	editDecorPanel->textBoxes["xpos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().x));
	editDecorPanel->textBoxes["ypos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().y));

	editDecorPanel->textBoxes["rotation"]->text.setString(boost::lexical_cast<string>(di->spr.getRotation()));
	editDecorPanel->textBoxes["xscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().x));
	editDecorPanel->textBoxes["yscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().y));
	
	editDecorPanel->textBoxes["layer"]->text.setString(boost::lexical_cast<string>(di->layer));
}

void EditSession::SetDecorParams()
{
	ISelectable *sp = selectedBrush->objects.front().get();
	assert(sp->selectableType == ISelectable::IMAGE);
	EditorDecorInfo *di = (EditorDecorInfo*)sp;
	
	string xposStr = editDecorPanel->textBoxes["xpos"]->text.getString().toAnsiString();
	string yposStr = editDecorPanel->textBoxes["ypos"]->text.getString().toAnsiString();
	string rotStr = editDecorPanel->textBoxes["rotation"]->text.getString().toAnsiString();
	string xscaleStr = editDecorPanel->textBoxes["xscale"]->text.getString().toAnsiString();
	string yscaleStr = editDecorPanel->textBoxes["yscale"]->text.getString().toAnsiString();
	string layerStr = editDecorPanel->textBoxes["layer"]->text.getString().toAnsiString();

	stringstream ss;
	ss << xposStr;

	float posx;
	ss >> posx;

	if (!ss.fail())
	{
		di->spr.setPosition(posx, di->spr.getPosition().y);
	}

	ss.clear();

	ss << yposStr;

	float posy;
	ss >> posy;

	if (!ss.fail())
	{
		di->spr.setPosition(di->spr.getPosition().x, posy );
	}

	ss.clear();

	ss << rotStr;

	float rot;
	ss >> rot;

	if (!ss.fail())
	{
		di->spr.setRotation(rot);
	}

	ss.clear();

	ss << xscaleStr;

	float xScale;
	ss >> xScale;

	if (!ss.fail())
	{
		di->spr.setScale( xScale, di->spr.getScale().y );
	}

	ss.clear();

	ss << yscaleStr;

	float yScale;
	ss >> yScale;

	if (!ss.fail())
	{
		di->spr.setScale(di->spr.getScale().x, yScale );
	}

	ss.clear();

	ss << layerStr;

	int layer;
	ss >> layer;
	if (!ss.fail())
	{
		di->layer = layer;
	}
}

bool EditSession::CanCreateGate( GateInfo &testGate )
{
	//if (testGate.poly0 == NULL || testGate.poly1 == NULL)
	//	return false;
	//this function can later be moved into IsGateValid and cleaned up

	Vector2i v0 = testGate.point0->pos;
	Vector2i v1 = testGate.point1->pos;

	//no duplicate points
	for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
	{
		if( v0 == (*it)->point0->pos || v0 == (*it)->point1->pos || v1 == (*it)->point0->pos || v1 == (*it)->point1->pos )
		{
			return false;
		}
	}

	if( testGate.poly0 == testGate.poly1 )
	{
		if( testGate.vertexIndex0 + 1 == testGate.vertexIndex1 
			|| testGate.vertexIndex0 - 1 == testGate.vertexIndex1 
			|| testGate.vertexIndex0 == 0 && testGate.vertexIndex1 == testGate.poly1->numPoints
			|| testGate.vertexIndex1 == 0 && testGate.vertexIndex0 == testGate.poly1->numPoints )
		{
			return false;
		}
	}
	
	//get aabb, check intersection with polygons. check line intersections with those polygons

	int left = min( v0.x, v1.x );
	int right = max( v0.x, v1.x );
	int top = min( v0.y, v1.y );
	int bot = max( v0.y, v1.y );

	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//aabb collide
		if( left <= (*it)->right && right >= (*it)->left && top <= (*it)->bottom && bot >= (*it)->top )
		{
			TerrainPoint *prev;
			for( TerrainPoint *pcurr = (*it)->pointStart; pcurr != NULL; pcurr = pcurr->next )
			{
				if( pcurr == (*it)->pointStart )
				{
					prev = (*it)->pointEnd;
				}
				else
				{
				prev = pcurr->prev;
				}

				Vector2i prevPos = prev->pos;
				Vector2i pos = pcurr->pos;

				LineIntersection li = LimitSegmentIntersect(prevPos, pos, v0, v1);

				if (!li.parallel)
				{
					return false;
				}
			}
		}
	}

	//make sure its not within a single polygon and cutting through the middle of it.
	Vector2f center(Vector2f(v0 + v1) / 2.f);
	if (testGate.poly0 == testGate.poly1)
	{
		if (testGate.poly0->ContainsPoint(center))
		{
			return false;
		}
	}


	return true;
}



void EditSession::CreateActor(ActorPtr actor)
{
	Brush b;
	SelectPtr select = boost::dynamic_pointer_cast<ISelectable>(actor);
	b.AddObject(select);
	Action * action = new ApplyBrushAction(&b);
	action->Perform();
	doneActionStack.push_back(action);
}

void EditSession::CreateDecorImage(EditorDecorPtr dec)
{
	Brush b;
	SelectPtr select = boost::dynamic_pointer_cast<ISelectable>(dec);
	b.AddObject(select);
	Action * action = new ApplyBrushAction(&b);
	action->Perform();
	doneActionStack.push_back(action);
}

void EditSession::CreatePreview(Vector2i imageSize)
{
	int extraBound = 0;
	int left;
	int top;
	int right;
	int bot;
	cout << "CREATING PREVIEW" << endl;

	if (inversePolygon != NULL)
	{
		left = inversePolygon->left;
		top = inversePolygon->top;
		right = inversePolygon->right;
		bot = inversePolygon->bottom;

		extraBound = 200;
	}
	else
	{
		int pLeft;
		int pTop;
		int pRight;
		int pBot;
		for (list<boost::shared_ptr<TerrainPolygon>>::iterator it
			= polygons.begin(); it != polygons.end(); ++it)
		{
			if (polygons.front() == (*it))
			{
				pLeft = (*it)->left;
				pTop = (*it)->top;
				pRight = (*it)->right;
				pBot = (*it)->bottom;
			}
			else
			{
				pLeft = min((*it)->left, pLeft);
				pRight = max((*it)->right, pRight);
				pTop = min((*it)->top, pTop);
				pBot = max((*it)->bottom, pBot );
			}
		}

		left = pLeft;
		top = topBound;
		right = pRight;
		bot = pBot;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto it2 = (*it).second->actors.begin(); it2 != (*it).second->actors.end(); ++it2)
		{
			if ((*it2)->type->info.name == "poi" )
			{
				boost::shared_ptr<PoiParams> pp = boost::static_pointer_cast<PoiParams>((*it2));
				if (pp->name == "stormceiling")
				{
					top = pp->position.y;
				}
			}
		}
	}

		
	if (left < leftBound)
	{
		left = leftBound;
	}
	else
	{
		left -= extraBound;
	}

	if (top < topBound)
	{
		top = topBound;
	}
	else
	{
		top -= extraBound;
	}
	int bRight = leftBound + boundWidth;
	if (right > bRight)
	{
		right = bRight;
	}
	else
	{
		right += extraBound;
	}

		
	bot += extraBound;

	int width = right - left;
	int height = bot - top;

	Vector2f middle( left + width / 2, top + height / 2 );

	int remX = (right - left) % imageSize.x;
	int remY = (bot - top) % imageSize.y;

	double idealXYRatio = (double)imageSize.x/imageSize.y;
	double realXYRatio = (double)width/(double)height;

	double facX = (right - left) / (double)imageSize.x;
	double facY = (bot - top) / (double)imageSize.y;

	if( realXYRatio > idealXYRatio )
	{
		//wider than it should be
			

		height = ceil(height * (realXYRatio / idealXYRatio));

		if( height % imageSize.y == 1 )
			height--;
		else if( height % imageSize.y == imageSize.y - 1 )
			height++;
	}
	else if( realXYRatio < idealXYRatio )
	{
		//taller than it should be

		width = ceil( width * (idealXYRatio / realXYRatio) );

		if( width % imageSize.x == 1 )
			width--;
		if( width % imageSize.x == imageSize.x - 1 )
			width++;
	}
	else
	{
		//its exactly right
	}

	sf::View pView;
	pView.setCenter( middle );
	pView.setSize(Vector2f(width, -height));// *1.05f );

	Vector2f vSize = pView.getSize();
	float zoom = vSize.x / 960;
	Vector2f botLeft(pView.getCenter().x - vSize.x / 2, pView.getCenter().y + vSize.y / 2);
	for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
	{
		if (terrainTextures[i] != NULL)
		{
			polyShaders[i].setUniform("zoom", zoom);
			polyShaders[i].setUniform("topLeft", botLeft);
			//just need to change the name topleft  to botleft eventually
		}

	}

	mapPreviewTex->clear(Color::Black);

	mapPreviewTex->setView( pView );


	CircleShape cs;
	cs.setRadius( 10.f * ( (float)width / 1920 ) );
	cs.setFillColor( Color::Red );
	cs.setOrigin( cs.getLocalBounds().width / 2, 
		cs.getLocalBounds().height / 2 );

	CircleShape goalCS;
	goalCS.setRadius(16.f * ((float)width / 1920));
	goalCS.setFillColor(Color::Magenta);
	goalCS.setOrigin(cs.getLocalBounds().width / 2,
		cs.getLocalBounds().height / 2);

	//mapPreviewTex->draw(*tempva);ke
	//delete tempva;

	for( list<boost::shared_ptr<TerrainPolygon>>::iterator it
		= polygons.begin(); it != polygons.end(); ++it )
	{
		//if( (*it)->IsTouching( inversePolygon.get() ) )
		//	continue;

		(*it)->Draw( false, 1, mapPreviewTex, false, NULL );
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->Draw(1, false, mapPreviewTex);
	}

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->DrawPreview(mapPreviewTex);
	}

	sf::RectangleShape borderRect;
	borderRect.setFillColor(Color( 30, 30, 30));
	borderRect.setSize(Vector2f(1000000, bot - top));
	borderRect.setPosition(left, top);
	borderRect.setOrigin(borderRect.getLocalBounds().width, 0);
	mapPreviewTex->draw(borderRect);

	borderRect.setOrigin(0, 0);
	borderRect.setPosition(right, top);
	mapPreviewTex->draw(borderRect);
	//sf::Vertex borderRect[4];
	//SetRectColor(borderRect, Color::Cyan);
		
		

	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		for( list<ActorPtr>::iterator it2 = (*it).second->actors.begin();
			it2 != (*it).second->actors.end(); ++it2 )
		{
			if ((*it2)->type->IsGoalType())
			{
				goalCS.setPosition((*it2)->position.x, (*it2)->position.y);
				mapPreviewTex->draw(goalCS);
			}
			else
			{
				cs.setPosition((*it2)->position.x, (*it2)->position.y);
				mapPreviewTex->draw(cs);
			}
				
		}

			
		//(*it).second->DrawPreview( mapPreviewTex );
	}

	cs.setPosition(player->position.x, player->position.y );
	cs.setFillColor(Color::Green);
	mapPreviewTex->draw(cs);

	sf::RectangleShape rs;
	rs.setPosition(pView.getCenter().x - pView.getSize().x / 2, top);// pView.getCenter().y);
	rs.setSize(Vector2f(pView.getSize().x, top - (pView.getCenter().y - pView.getSize().y / 2 )));
	rs.setFillColor(Color::Cyan);
	mapPreviewTex->draw(rs);
	//this rectangle shape is just a placeholder, because eventually we will texture stuff.

		

	Image img = mapPreviewTex->getTexture().copyToImage();
		
	std::stringstream ssPrev;
	ssPrev << currentPath.parent_path().relative_path().string() << "/Previews/" << currentPath.stem().string() << "_preview_" << imageSize.x << "x" << imageSize.y << ".png";
	std::string previewFile = ssPrev.str();
	img.saveToFile( previewFile );
	//currentFile
}

GroundInfo EditSession::ConvertPointToGround( sf::Vector2i testPoint )
{
	GroundInfo gi;
	double testRadius = 200;
	//PolyPtr poly = NULL;
	gi.ground = NULL;
	gi.railGround = NULL;

	bool contains;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y));

		if (contains )//(contains && !(*it)->inverse) || (!contains && (*it)->inverse))
		{
			TerrainPoint *prev = (*it)->pointEnd;
			TerrainPoint *curr = (*it)->pointStart;

			double minDistance = 10000000;
			TerrainPoint *storedEdge = NULL;
			TerrainPolygon *storedPoly = NULL;
			double storedQuantity = 0;
							
			V2d closestPoint;

			for( ; curr != NULL; curr = curr->next )
			{
				double dist = abs(
					cross( 
					V2d( testPoint.x - prev->pos.x, testPoint.y - prev->pos.y ), 
					normalize( V2d( curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y ) ) ) );
				double testQuantity =  dot( 
						V2d( testPoint.x - prev->pos.x, testPoint.y - prev->pos.y ), 
						normalize( V2d( curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y ) ) );

				V2d pr( prev->pos.x, prev->pos.y );
				V2d cu( curr->pos.x, curr->pos.y );
				V2d te( testPoint.x, testPoint.y );
									
				V2d newPoint( pr.x + (cu.x - pr.x) * (testQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
						(testQuantity / length( cu - pr ) ) );

				if( dist < testRadius && testQuantity >= 0 && testQuantity <= length( cu - pr ) 
					&& length( newPoint - te ) < length( closestPoint - te ) )
				{
					minDistance = dist;

					storedPoly = (*it).get();
					storedEdge = prev;
					storedQuantity = testQuantity;

					//storedIndex = edgeIndex;
					double l = length( cu - pr );
										
					
					closestPoint = newPoint;
				}
				else
				{
				}

				prev = curr;
			}

			
			gi.edgeStart = storedEdge;
			gi.groundQuantity = storedQuantity;
			gi.ground = storedPoly;

			break;
		}

		if( testPoint.x >= (*it)->left - testRadius 
			&& testPoint.x <= (*it)->right + testRadius
			&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
		{

		}
	}

	return gi;
}

GroundInfo EditSession::ConvertPointToRail(sf::Vector2i testPoint)
{
	GroundInfo gi;
	gi.railGround = NULL;
	gi.ground = NULL;

	double testRadius = 200;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if (testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
			&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius)
		{
			TerrainPoint *curr = (*it)->pointStart;
			TerrainPoint *next = NULL;

			bool contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y), 32);

			if (contains)
			{
				

				V2d closestPoint;


				//prev is starting at 0. start normally at 1
				int edgeIndex = 0;
				double minDistance = 10000000;

				TerrainPoint *storedEdge = NULL;
				TerrainRail *storedRail = NULL;
				double storedQuantity = 0;

				for (; curr != NULL; curr = curr->next)
				{
					if (curr == (*it)->pointEnd)
					{
						break;
					}
					else
					{
						next = curr->next;
					}

					double dist = abs(
						cross(
							V2d(testPoint.x - curr->pos.x, testPoint.y - curr->pos.y),
							normalize(V2d(next->pos.x - curr->pos.x, next->pos.y - curr->pos.y))));
					double testQuantity = dot(
						V2d(testPoint.x - curr->pos.x, testPoint.y - curr->pos.y),
						normalize(V2d(next->pos.x - curr->pos.x, next->pos.y - curr->pos.y)));

					V2d pr(curr->pos.x, curr->pos.y);
					V2d cu(next->pos.x, next->pos.y);
					V2d te(testPoint.x, testPoint.y);

					V2d newPoint(pr.x + (cu.x - pr.x) * (testQuantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
						(testQuantity / length(cu - pr)));
					double edgeLength = length(cu - pr);
					double newDist = length(newPoint - te);
					double closestDist = length(closestPoint - te);

					//int hw = trackingEnemy->info.size.x / 2;
					//int hh = trackingEnemy->info.size.y / 2;
					if (dist < 100 && testQuantity >= 0 && testQuantity <= edgeLength
						&& newDist < closestDist)
					{
						minDistance = dist;

						storedRail = (*it).get();
						storedEdge = curr;
						storedQuantity = testQuantity;
						
						closestPoint = newPoint;
					}

					++edgeIndex;
				}

				if (storedRail != NULL )
				{
					gi.railGround = storedRail;
					gi.edgeStart = storedEdge;
					gi.groundQuantity = storedQuantity;

					break;
				}
			}
		}
	}

	return gi;
}

list<PolyPtr> & EditSession::GetCorrectPolygonList(TerrainPolygon *t)
{
	if (t->IsSpecialPoly())
	{
		return waterPolygons;
	}
	else 
	{
		return polygons;
	}
}

list<PolyPtr> & EditSession::GetCorrectPolygonList(int ind)
{
	switch (ind)
	{
	case 0:
		return polygons;
	case 1:
		return waterPolygons;
	default:
		assert(0);
		return polygons;
	}
}

list<PolyPtr> & EditSession::GetCorrectPolygonList()
{
	if (IsSpecialTerrainMode())
	{
		return waterPolygons;
	}
	else
	{
		return polygons;
	}
}

void EditSession::GetIntersectingPolys(
	TerrainPolygon *p,
	std::list<PolyPtr> & intersections)
{
	auto &testPolygons = GetCorrectPolygonList( p );

	for( auto it = testPolygons.begin(); it != testPolygons.end(); ++it )
	{
		if (p->IsTouching((*it).get()))
		{
			intersections.push_back((*it));
		}
	}
}

bool EditSession::ExecuteTerrainCompletion()
{
	if( polygonInProgress->numPoints > 2 )
	{
		if (!polygonInProgress->IsCompletionValid())
		{
			return false;
		}

		polygonInProgress->SetMaterialType(currTerrainWorld,
			currTerrainVar);
		
		auto &testPolygons = GetCorrectPolygonList(polygonInProgress.get());
		list<PolyPtr>::iterator it = testPolygons.begin();
		bool added = false;
									
		bool recursionDone = false;
		PolyPtr currentBrush = polygonInProgress;

		list<PolyPtr> intersectingPolys;

		polygonInProgress->UpdateBounds();



		bool applyOkay = true;
		for(; it != testPolygons.end(); ++it )
		{
			if ((*it)->Contains(polygonInProgress.get()))
			{
				applyOkay = false;
				polygonInProgress->ClearPoints();
				break;
			}

			if( polygonInProgress->LinesIntersect( (*it).get() ) )
			{
				//not too close and I intersect, so I can add
				intersectingPolys.push_back( (*it) );
			}
		}

		if( !applyOkay )
		{
			//MessagePop( "polygon is invalid!!! new message" );
		}
		else
		{
			bool empty = intersectingPolys.empty();

			if( empty && IsKeyPressed(Keyboard::LAlt))
			{
				polygonInProgress->inverse = true;
			}
			
			if( empty )
			{
				if (polygonInProgress->inverse)
				{
					SetInversePoly();
				}
				else
				{
					polygonInProgress->FixWinding();
					polygonInProgress->RemoveSlivers();
					polygonInProgress->AlignExtremes();
					polygonInProgress->Finalize();

					SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>(polygonInProgress);

					progressBrush->Clear();


					progressBrush->AddObject(sp);

					Action *action = new ApplyBrushAction(progressBrush);
					action->Perform();
					doneActionStack.push_back(action);

					ClearUndoneActions();

					PolyPtr newPoly(new TerrainPolygon(&grassTex));
					polygonInProgress = newPoly;
				}
			}
			else
			{
				//add each of the intersecting polygons onto the polygonInProgress,
				//then do a replacebrushaction

				//polygonInProgress->Finalize();
				//polygonInProgress->FixWinding();

				//hold shift ATM to activate subtraction
				Action *action = ChooseAddOrSub(intersectingPolys);

				action->Perform();
				doneActionStack.push_back(action);

				ClearUndoneActions();
			}

			return true;
		}
	}
	else if( polygonInProgress->numPoints <= 2 && polygonInProgress->numPoints > 0  )
	{
		cout << "cant finalize. cant make polygon" << endl;
		polygonInProgress->ClearPoints();
		return false;
	}

	return false;
}

void EditSession::ExecuteRailCompletion()
{
	if (railInProgress->numPoints > 2)
	{
		//test final line
		bool valid = true;
		if (!valid)
		{
			//MessagePop( "unable to complete polygon" );
			//popupPanel = messagePopup;
			//return;
		}

		list<RailPtr>::iterator it = rails.begin();
		bool added = false;

		bool recursionDone = false;
		RailPtr currentBrush = railInProgress;

		railInProgress->UpdateBounds();
		bool applyOkay = true;
		if (!applyOkay)
		{
			MessagePop("polygon is invalid!!! new message");
		}
		else
		{
			bool empty = true;
			//eventually be able to combine rails by putting your start/end points at their starts/ends
			if (empty)
			{
				railInProgress->Finalize();

				SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>(railInProgress);

				progressBrush->Clear();

				progressBrush->AddObject(sp);

				Action *action = new ApplyBrushAction(progressBrush);
				action->Perform();
				doneActionStack.push_back(action);

				ClearUndoneActions();

				RailPtr newRail(new TerrainRail());
				railInProgress = newRail;
			}
			else
			{
				//eventually combine rails here

				//Action *action = ChooseAddOrSub(intersectingPolys);

				//action->Perform();
				//doneActionStack.push_back(action);

				//ClearUndoneActions();
			}
		}
	}
	else if (railInProgress->numPoints <= 2 && railInProgress->numPoints > 0)
	{
		cout << "cant finalize. cant make polygon" << endl;
		railInProgress->ClearPoints();
	}
}

void EditSession::SetInversePoly()
{
	polygonInProgress->FixWinding();
	//polygonInProgress->Finalize();

	polygonInProgress->FinalizeInverse();
	

	Brush orig;
	if( inversePolygon != NULL )
	{
		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( inversePolygon );
		orig.AddObject( sp );
	}

	SelectPtr sp = boost::dynamic_pointer_cast< ISelectable>( polygonInProgress );

	progressBrush->Clear();
	progressBrush->AddObject( sp );

	Action * action = new ReplaceBrushAction( &orig, progressBrush );
	action->Perform();
	doneActionStack.push_back( action );

	ClearUndoneActions();

	PolyPtr newPoly( new TerrainPolygon(&grassTex) );
	polygonInProgress = newPoly;

	progressBrush->Clear();
	//progressBrush->AddObject( sp );
									
	//Action *action = new ApplyBrushAction( progressBrush );
	//action->Perform();
	//doneActionStack.push_back( action );

	//ClearUndoneActions();

	//PolyPtr newPoly( new TerrainPolygon(&grassTex) );
	//polygonInProgress = newPoly;
}

bool EditSession::HoldingShift()
{
	return ((IsKeyPressed(Keyboard::LShift) ||
		IsKeyPressed(Keyboard::RShift)));
}

bool EditSession::HoldingControl()
{
	return ((IsKeyPressed(Keyboard::LControl) ||
		IsKeyPressed(Keyboard::RControl)));
}

void EditSession::CreateShardGridSelector( Panel *p, sf::Vector2i &pos )
{
	int xSize = 11;
	int ySize = 2;

	GridSelector *gs = p->AddGridSelector("shardselector", pos, xSize, ySize * 7, 64, 64, true, true);
	Sprite spr;


	ts_shards[0] = GetTileset("Shard/shards_w1_48x48.png", 48, 48);
	ts_shards[1] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
	ts_shards[2] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
	ts_shards[3] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
	ts_shards[4] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
	ts_shards[5] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
	ts_shards[6] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);


	Tileset *ts_currShards;
	int sInd = 0;

	for (int w = 0; w < 7; ++w)
	{
		ts_currShards = ts_shards[w];
		spr.setTexture(*ts_currShards->texture);
		for (int y = 0; y < ySize; ++y)
		{
			for (int x = 0; x < xSize; ++x)
			{
				sInd = y * xSize + x;
				spr.setTextureRect(ts_currShards->GetSubRect(sInd));
				int shardT = (sInd + (xSize * ySize) * w);
				if (shardT >= SHARD_Count)
				{
					gs->Set(x, y + ySize * w, spr, "---"); //need a way to set the names later
				}
				else
				{
					gs->Set(x, y + ySize * w, spr, Shard::GetShardString((ShardType)shardT));
				}

			}
		}
	}
}

void EditSession::GetShardWorldAndIndex(int selX, int selY,
	int &w, int &li)
{
	int world = selX / 11;
	int realX = selX % 11;
	int realY = selY;

	w = world;
	li = realX + realY * 11;
}

Action * EditSession::ChooseAddOrSub( list<PolyPtr> &intersectingPolys)
{
	if (!(IsKeyPressed(Keyboard::LShift) ||
		IsKeyPressed(Keyboard::RShift)))
	{
		return ExecuteTerrainAdd( intersectingPolys);
	}
	else
	{
		return ExecuteTerrainSubtract(intersectingPolys);
	}
}

void EditSession::PasteTerrain(Brush *b)
{
	CompoundAction *compoundAction = new CompoundAction;
	Brush applyBrush;
	for (auto bit = b->objects.begin(); bit != b->objects.end(); ++bit)
	{
		if ((*bit)->selectableType == ISelectable::TERRAIN)
		{
			TerrainPolygon *tp = (TerrainPolygon*)((*bit).get());
			polygonInProgress.reset(tp->Copy());
			list<PolyPtr> intersectingPolys;
			for (auto it = polygons.begin(); it != polygons.end(); ++it)
			{
				if (polygonInProgress->LinesIntersect((*it).get()))
				{
					//not too close and I intersect, so I can add
					intersectingPolys.push_back((*it));
				}
			}

			if (intersectingPolys.empty())
			{
				applyBrush.AddObject((*bit));
			}
			else
			{
				Action * a = ChooseAddOrSub(intersectingPolys);
				compoundAction->subActions.push_back(a);
			}
		}
		else if ((*bit)->selectableType == ISelectable::ACTOR)
		{
			ActorPtr a = boost::dynamic_pointer_cast<ActorParams>((*bit));
			ActorPtr ap(a->Copy());
			if (ap->groundInfo != NULL)
			{
				ap->AnchorToGround(*ap->groundInfo);
			}
			applyBrush.AddObject(ap);
		}
	}

	if (applyBrush.objects.size() > 0)
	{
		Action *ac = new ApplyBrushAction(&applyBrush);
		compoundAction->subActions.push_back(ac);

		PolyPtr newPoly(new TerrainPolygon(&grassTex));
		polygonInProgress = newPoly;
	}

	compoundAction->Perform();
	doneActionStack.push_back(compoundAction);
	ClearUndoneActions();

	copiedBrush = copiedBrush->Copy();
}

void EditSession::AddFullPolysToBrush(
	std::list<PolyPtr> & polyList,
	std::list<GateInfoPtr> &gateInfoList,
	Brush *b)
{
	for (list<PolyPtr>::iterator it = polyList.begin(); it != polyList.end(); ++it)
	{
		b->AddObject((*it));
		(*it)->AddGatesToBrush(b, gateInfoList);
		(*it)->AddEnemiesToBrush(b);
	}
}

Action* EditSession::ExecuteTerrainAdd( list<PolyPtr> &intersectingPolys)
{
	Brush orig;
	Brush resultBrush;

	list<GateInfoPtr> gateInfoList;

	AddFullPolysToBrush(intersectingPolys, gateInfoList, &orig);

	bool inverse = false;
	int otherSize = intersectingPolys.size();


	for (auto it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it)
	{
		if ((*it)->inverse)
		{
			inverse = true;
			break;
		}
	}

	if (inverse)
		otherSize--;

	ClipperLib::Clipper c;
	TerrainPoint *curr;

	ClipperLib::Paths inProgress(1), other(otherSize), otherInverse(1), solution, inverseSolution;

	curr = polygonInProgress->pointStart;

	polygonInProgress->CopyPointsToClipperPath(inProgress[0]);

	PolyPtr outPoly(new TerrainPolygon(&grassTex));

	ClipperLib::Path clipperIntersections;
	//if I want to speed this up later, only care about these points when they are on the OUTSIDE of the target polys
	//for add and on the INSIDE for subtract
	polygonInProgress->CopyPointsToClipperPath(clipperIntersections);

	list<TerrainPoint*> newPoints;

	if (otherSize > 0)
	{
		//setup intersected polys
		int otherIndex = 0;
		for (auto it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it)
		{
			if ((*it)->inverse)
			{
				continue;
			}

			(*it)->CopyPointsToClipperPath(other[otherIndex]);
			++otherIndex;
		}

		//add
		c.AddPaths(other, ClipperLib::PolyType::ptSubject, true);

		c.AddPaths(inProgress, ClipperLib::PolyType::ptClip, true);
		c.Execute(ClipperLib::ClipType::ctUnion, solution);
		
		ClipperLib::Path &intersectPath = c.GetIntersectPath();
		clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
		clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());

		if (!inverse)
		{
			outPoly->AddPointsFromClipperPath(solution[0], clipperIntersections, newPoints);
			outPoly->RemoveClusters(newPoints);
		}
		else
		{
			inversePolygon->CopyPointsToClipperPath(otherInverse[0]);

			c.Clear();

			c.AddPaths(solution, ClipperLib::PolyType::ptClip, true);
		}
	}
	else
	{
		inversePolygon->CopyPointsToClipperPath(otherInverse[0]);
		c.AddPaths(inProgress, ClipperLib::PolyType::ptClip, true);

	}

	//clip 
	if (inverse)
	{
		c.AddPaths(otherInverse, ClipperLib::PolyType::ptSubject, true);

		c.Execute(ClipperLib::ClipType::ctDifference, inverseSolution);

		ClipperLib::Path &intersectPath = c.GetIntersectPath();

		clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
		clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());

		outPoly->AddPointsFromClipperPath(inverseSolution[0], clipperIntersections, newPoints);
		outPoly->RemoveClusters(newPoints);
	}

	//outPoly->RemoveClusters(allIntersections);

	outPoly->SetMaterialType(currTerrainWorld, currTerrainVar );//poly->terrainWorldType,
									   //poly->terrainVariation);
	outPoly->RemoveSlivers();
	outPoly->AlignExtremes();

	outPoly->inverse = inverse;
	outPoly->Finalize();

	
	resultBrush.AddObject(outPoly);

	list<PolyPtr> attachList;
	attachList.push_back(outPoly);

	TryAttachActors(intersectingPolys, attachList, &resultBrush);
	TryKeepGates(gateInfoList, attachList, &resultBrush);

	Action * action = new ReplaceBrushAction(&orig, &resultBrush);

	polygonInProgress->ClearPoints();

	return action;
}

list<TerrainPoint*> InsertTemporaryPoints( TerrainPolygon *poly, list<Inter> &inters )
{
	list<TerrainPoint*> addedPoints;
	TerrainPoint *tp;
	//TerrainPoint *prev;
	TerrainPoint *next;

	map<TerrainPoint*, list<V2d>> interMap;
	for( list<Inter>::iterator it = inters.begin(); it != inters.end(); ++it )
	{
		interMap[(*it).point].push_back( (*it).position );
	}

	for( map<TerrainPoint*, list<V2d>>::iterator it = interMap.begin(); it != interMap.end(); ++it )
	{
		tp = (*it).first;
		next = tp->next;

	//	//if( prev == NULL )
	//	//	prev = poly->pointEnd;
		//if( next == NULL )
		//	next = poly->pointStart;
		list<V2d> &points = (*it).second;

		int size = (*it).second.size();
		if( size >= 2 ) //has enough intersections on one line
		{
			
			list<V2d>::iterator vit = points.begin();
			V2d prev = (*vit);
			++vit;
			for( ; vit != points.end(); ++vit )
			{
				V2d midPoint = ( (*vit) + prev ) / 2.0;
				if( midPoint.x > 0 )
					midPoint.x += .5;
				else if( midPoint.x < 0 )
					midPoint.x -= .5;

				if( midPoint.y > 0 )
					midPoint.y += .5;
				else if( midPoint.y < 0 )
					midPoint.y -= .5;
				

				TerrainPoint *newPoint = new TerrainPoint( Vector2i( midPoint.x, midPoint.y ), false );
				addedPoints.push_back( newPoint );

				//cout << "inserting new point between: 1: " << prev.x << ", " << prev.y <<
				//	" and: " << (*vit).x << ", " << (*vit).y << endl;
				//cout << "midPoint: " << midPoint.x << ", "
				//	 << midPoint.y << endl;
				poly->InsertPoint( newPoint, tp );
				tp = newPoint;

				prev = (*vit);
			}
		}

	}
	return addedPoints;
	//for( list<Inter>::iterator it = inters.begin(); it != inters.end(); ++it )
	//{
	//	tp = (*it).first;
	//	//prev = tp->prev;
	//	next = tp->next;

	//	//if( prev == NULL )
	//	//	prev = poly->pointEnd;
	//	if( next == NULL )
	//		next = poly->pointStart;

	//	TerrainPoint *newPoint = new TerrainPoint( Vector2i( (*it).second.x + .5,
	//		(*it).second.y + .5 ), false );
	//	cout << "inserting new point!" << endl;
	//	newPoint->prev = tp;
	//	newPoint->next = next;
	//	tp->next = newPoint;
	//	next->prev = newPoint;
	//}
}

void RemoveTemporaryPoints( TerrainPolygon *poly, list<TerrainPoint*> &addedPoints )
{
	TerrainPoint *tp;
	TerrainPoint *prev;
	TerrainPoint *next;
	TerrainPoint *nextnext;

	for( list<TerrainPoint*>::iterator it = addedPoints.begin(); it != addedPoints.end(); ++it )
	{
		poly->RemovePoint( (*it) );
	}
}

Action* EditSession::ExecuteTerrainSubtract( list<PolyPtr> &intersectingPolys)
{
	bool inverse = false;
	int otherSize = intersectingPolys.size();
	for (auto it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it)
	{
		if ((*it)->inverse)
		{
			inverse = true;
			break;
		}
	}

	if (inverse)
		otherSize--;

	ClipperLib::Clipper c;
	TerrainPoint *curr;
	TerrainPolygon *testOutPoly;

	ClipperLib::Paths inProgress(1), other(otherSize), otherInverse(1), solution, inverseSolution;

	curr = polygonInProgress->pointStart;

	polygonInProgress->CopyPointsToClipperPath(inProgress[0]);

	Brush orig;
	Brush resultBrush;

	resultBrush.Clear();

	list<TerrainPolygon*> results;
	list<PolyPtr> resultsPtr;

	//add original stuff to the original brush
	list<GateInfoPtr> gateInfoList;

	ClipperLib::Path clipperIntersections;

	list<TerrainPoint*> newPoints;

	AddFullPolysToBrush(intersectingPolys, gateInfoList, &orig);

	if (otherSize > 0)
	{
		//setup intersected polys
		ClipperLib::Path otherPath;
		for (auto it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it)
		{
			if ((*it)->inverse)
			{
				continue;
			}

			
			c.Clear();
			solution.clear();
			otherPath.clear();

			(*it)->CopyPointsToClipperPath(otherPath);
			c.AddPath( otherPath, ClipperLib::PolyType::ptSubject, true);
			c.AddPaths(inProgress, ClipperLib::PolyType::ptClip, true);
			c.Execute(ClipperLib::ClipType::ctDifference, solution);

			ClipperLib::Path &intersectPath = c.GetIntersectPath();

			clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
			clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());

			/*if (!inverse)
			{
				outPoly->AddPointsFromClipperPath(solution[0], clipperIntersections, newPoints);*/

			for (auto sit = solution.begin(); sit != solution.end(); ++sit)
			{
				TerrainPolygon *newPoly = new TerrainPolygon(&grassTex);
				newPoly->AddPointsFromClipperPath((*sit), clipperIntersections, newPoints );
				newPoly->RemoveClusters(newPoints);
				newPoly->SetMaterialType((*it)->terrainWorldType, (*it)->terrainVariation);
				results.push_back(newPoly);
			}
		}		
	}

	//clip 
	if (inverse)
	{
		c.Clear();
		inversePolygon->CopyPointsToClipperPath(otherInverse[0]);

		c.AddPaths(otherInverse, ClipperLib::PolyType::ptSubject, true);
		c.AddPaths(inProgress, ClipperLib::PolyType::ptClip, true);

		c.Execute(ClipperLib::ClipType::ctUnion, inverseSolution);

		ClipperLib::Path &intersectPath = c.GetIntersectPath();

		clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
		clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());
		//for (auto it = testList.begin(); it != testList.end(); ++it)
		//{
		//	allIntersections.push_back((*it));$
		//	//cout << "intersection4: " << (*it).X << ", " << (*it).Y << endl;
		//}

		for (auto it = inverseSolution.begin(); it != inverseSolution.end(); ++it)
		{
			TerrainPolygon *newPoly = new TerrainPolygon(&grassTex);

			newPoly->AddPointsFromClipperPath((*it), clipperIntersections, newPoints);
			newPoly->RemoveClusters(newPoints);
			newPoly->SetMaterialType(inversePolygon->terrainWorldType, 
				inversePolygon->terrainVariation );

			results.push_back(newPoly);
		}

		//figure out which polygon should be the new inverse polygon
		bool isOuter;
		for (auto it = results.begin(); it != results.end(); ++it)
		{
			isOuter = true;
			for (auto it2 = results.begin(); it2 != results.end(); ++it2)
			{
				if ((*it) == (*it2))
					continue;

				if (!(*it)->Contains((*it2)))
				{
					isOuter = false;
				}
			}

			if (isOuter)
			{
				(*it)->inverse = true;
				break;
			}
		}
	}


	/*for (auto it = allIntersections.begin(); it != allIntersections.end(); ++it)
	{
		cout << "intersection2: " << (*it).X << ", " << (*it).Y << endl;
	}*/

	for (auto it = results.begin(); it != results.end(); ++it)
	{
		PolyPtr p((*it));
		resultsPtr.push_back(p);
		resultBrush.AddObject(p);
	}

	TryAttachActors(intersectingPolys, resultsPtr, &resultBrush);
	TryKeepGates(gateInfoList, resultsPtr, &resultBrush );

	for (auto it = resultsPtr.begin(); it != resultsPtr.end(); ++it)
	{
		(*it)->RemoveSlivers();
		(*it)->AlignExtremes();
		(*it)->Finalize();
	}


	Action * action = new ReplaceBrushAction(&orig, &resultBrush);

	polygonInProgress->ClearPoints();

	return action;
}

bool EditSession::PointSelectTerrain(V2d &pos)
{
	bool pointSelectKeyHeld = IsKeyPressed(Keyboard::B);

	if (pointSelectKeyHeld)
	{
		if (PointSelectPolyPoint(worldPos))
		{
			return true;
		}
	}
	else
	{
		if (PointSelectPoly(worldPos))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectPolyPoint( V2d &pos )
{
	bool shift = IsKeyPressed(Keyboard::LShift) || IsKeyPressed(Keyboard::RShift);

	auto & currPolyList = GetCorrectPolygonList();

	TerrainPoint *foundPoint = NULL;
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		foundPoint = (*it)->GetClosePoint( 8 * zoomMultiple, pos);
		if (foundPoint != NULL)
		{
			if (shift && foundPoint->selected )
			{
				DeselectPoint((*it), foundPoint);
			}
			else
			{	
				if (!foundPoint->selected)
				{
					if (!shift)
						ClearSelectedPoints();

					SelectPoint((*it), foundPoint);
				}			
			}
			return true;
		}
	}
	return false;
}

bool EditSession::PointSelectRail(V2d &pos)
{
	bool pointSelectKeyHeld = IsKeyPressed(Keyboard::B);

	if (pointSelectKeyHeld)
	{
		if (PointSelectRailPoint(worldPos))
		{
			return true;
		}
	}
	else
	{
		if (PointSelectGeneralRail(worldPos))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectGeneralRail(V2d &pos)
{
	for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
	{
		bool sel = (*it)->ContainsPoint(Vector2f(pos));
		if (sel)
		{
			SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

			if (sp->selected)
			{

			}
			else
			{
				if (!HoldingShift())
				{
					selectedBrush->SetSelected(false);
					selectedBrush->Clear();
				}

				sp->SetSelected(true);

				grabbedObject = sp;
				selectedBrush->AddObject(sp);
			}

			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectRailPoint(V2d &pos)
{
	bool shift = HoldingShift();

	TerrainPoint *foundPoint = NULL;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		foundPoint = (*it)->GetClosePoint(8 * zoomMultiple, pos);
		if (foundPoint != NULL)
		{
			if (shift && foundPoint->selected)
			{
				DeselectPoint((*it).get(), foundPoint);
			}
			else
			{
				if (!foundPoint->selected)
				{
					if (!shift)
						ClearSelectedPoints();

					SelectPoint((*it).get(), foundPoint);
				}
			}
			return true;
		}
	}
	return false;
}

bool EditSession::PointSelectPoly(V2d &pos)
{
	auto & currPolyList = GetCorrectPolygonList();
	for (list<PolyPtr>::iterator it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		//bool pressF1 = IsKeyPressed(Keyboard::F1);
		//if ((pressF1 && !(*it)->inverse) || !pressF1 && (*it)->inverse)
		//	continue;

		bool sel = (*it)->ContainsPoint(Vector2f(pos));
		/*if ((*it)->inverse)
		{
			sel = !sel;
		}*/

		if (sel)
		{
			SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

			if (sp->selected)
			{

			}
			else
			{
				if (!HoldingShift())
				{
					selectedBrush->SetSelected(false);
					selectedBrush->Clear();
				}

				sp->SetSelected(true);

				grabbedObject = sp;
				selectedBrush->AddObject(sp);
			}

			return true;
		}
	}

	return false;
}

bool EditSession::BoxSelectPoints(sf::IntRect &r,
	double radius)
{
	auto & currPolyList = GetCorrectPolygonList();

	bool specialMode = IsSpecialTerrainMode();

	bool found = false;
	for (list<PolyPtr>::iterator it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		IntRect adjustedR(r.left, r.top, r.width, r.height);
		//IntRect adjustedR(r.left - radius, r.top, r.width, r.height);
		//why was this here with only the left coordinate changed?

		//aabb w/ polygon
		if ((*it)->Intersects(adjustedR))
		{
			TerrainPoint *curr = (*it)->pointStart;
			while (curr != NULL)
			{
				if (IsQuadTouchingCircle(V2d(r.left, r.top),
					V2d(r.left + r.width, r.top),
					V2d(r.left + r.width, r.top + r.height),
					V2d(r.left, r.top + r.height),
					V2d(curr->pos.x, curr->pos.y), radius)
					|| adjustedR.contains(curr->pos))
				{
					SelectPoint((*it), curr);
					found = true;
				}
				curr = curr->next;
			}
		}
	}

	if (!specialMode)
	{
		for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
		{
			//IntRect adjustedR(r.left - radius, r.top, r.width, r.height);
			IntRect adjustedR(r.left, r.top, r.width, r.height);

			if ((*it)->Intersects(adjustedR))
			{
				TerrainPoint *curr = (*it)->pointStart;
				while (curr != NULL)
				{
					if (IsQuadTouchingCircle(V2d(r.left, r.top),
						V2d(r.left + r.width, r.top),
						V2d(r.left + r.width, r.top + r.height),
						V2d(r.left, r.top + r.height),
						V2d(curr->pos.x, curr->pos.y), radius)
						|| adjustedR.contains(curr->pos))
					{
						SelectPoint((*it).get(), curr);
						found = true;
					}
					curr = curr->next;
				}
			}
		}
	}

	return found;
}

bool EditSession::BoxSelectActors(sf::IntRect &rect)
{
	bool found = false;
	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		for (list<ActorPtr>::iterator ait = (*it).second->actors.begin();
			ait != (*it).second->actors.end(); ++ait)
		{
			if ((*ait)->Intersects(rect))
			{
				SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*ait));

				if (HoldingShift())
				{
					if (sp->selected)
					{
						sp->SetSelected(false);
						selectedBrush->RemoveObject(sp); //might be slow?
					}
					else
					{
						sp->SetSelected(true);
						selectedBrush->AddObject(sp);
					}
				}
				else
				{
					sp->SetSelected(true);
					selectedBrush->AddObject(sp);
				}


				found = true;
			}
		}
	}

	return found;
}

bool EditSession::BoxSelectDecor(sf::IntRect &rect)
{
	bool found = false;
	for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
	{
		if ((*it)->Intersects(rect))
		{
			SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

			if (HoldingShift())
			{
				if (sp->selected)
				{
					sp->SetSelected(false);
					selectedBrush->RemoveObject(sp); //might be slow?
				}
				else
				{
					sp->SetSelected(true);
					selectedBrush->AddObject(sp);
				}
			}
			else
			{
				sp->SetSelected(true);
				selectedBrush->AddObject(sp);
			}


			found = true;
		}
	}

	return found;
}

bool EditSession::BoxSelectPolys(sf::IntRect &rect)
{
	bool found = false;

	auto & currPolyList = GetCorrectPolygonList();

	for (list<PolyPtr>::iterator it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		if ((*it)->Intersects(rect))
		{

			SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

			if (HoldingShift())
			{
				if (sp->selected)
				{
					sp->SetSelected(false);
					selectedBrush->RemoveObject(sp);
				}
				else
				{
					sp->SetSelected(true);
					selectedBrush->AddObject(sp);
				}
			}
			else
			{
				sp->SetSelected(true);
				selectedBrush->AddObject(sp);
			}

			found = true;
		}
	}

	return true;
}

bool EditSession::BoxSelectRails(sf::IntRect &rect)
{
	bool found = false;
	for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
	{
		if ((*it)->Intersects(rect))
		{
			SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

			if (HoldingShift())
			{
				if (sp->selected)
				{
					sp->SetSelected(false);
					selectedBrush->RemoveObject(sp);
				}
				else
				{
					sp->SetSelected(true);
					selectedBrush->AddObject(sp);
				}
			}
			else
			{
				sp->SetSelected(true);
				selectedBrush->AddObject(sp);
			}

			found = true;
		}
	}

	return true;
}

void EditSession::TryBoxSelect()
{
	Vector2i currPos(worldPos.x, worldPos.y);

	int left = std::min(editMouseOrigPos.x, currPos.x);
	int right = std::max(editMouseOrigPos.x, currPos.x);
	int top = std::min(editMouseOrigPos.y, currPos.y);
	int bot = std::max(editMouseOrigPos.y, currPos.y);


	sf::Rect<int> r(left, top, right - left, bot - top);
	//check this rectangle for the intersections, but do that next

	bool selectionEmpty = true;

	bool specialTerrain = IsSpecialTerrainMode();

	if (!HoldingShift())
	{
		//clear everything
		selectedBrush->SetSelected(false);
		selectedBrush->Clear();
	}

	if ( !specialTerrain && BoxSelectActors(r))
	{
		selectionEmpty = false;
	}

	if ( !specialTerrain && BoxSelectDecor(r))
	{
		selectionEmpty = false;
	}

	bool pointSelectButtonHeld = IsKeyPressed(Keyboard::B);
	if (pointSelectButtonHeld) //always use point selection for now
	{
		if (HoldingShift())
		{
			//ClearSelectedPoints();
		}

		BoxSelectPoints(r, 8 * zoomMultiple);
	}
	else if (!showPoints)//polygon selection. don't use it for a little bit
	{
		if (BoxSelectPolys(r))
		{
			selectionEmpty = false;
		}

		if (!specialTerrain && BoxSelectRails(r))
		{
			selectionEmpty = false;
		}
	}

	if (selectionEmpty)
	{
		selectedBrush->SetSelected(false);
		selectedBrush->Clear();
	}
}

double EditSession::GetZoomedMinEdgeLength()
{
	return minimumEdgeLength * std::max(zoomMultiple, 1.0);
}

void EditSession::UpdateGrass()
{
	if (showGrass)
	{
		for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
		{
			if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
			{
				boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
				d->UpdateGrass();
			}
		}
	}
}

void EditSession::ModifyGrass()
{
	if (showGrass && IsMousePressed(Mouse::Left))
	{
		for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
		{
			if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
			{
				boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
				d->SwitchGrass(worldPos);
			}
		}
	}
}

void EditSession::SetMode(Emode m)
{
	Emode oldMode = mode;
	mode = m;
	
	switch (mode)
	{

	}
}

bool EditSession::IsOnlyPlayerSelected()
{
	if (selectedBrush->objects.size() == 1)
	{
		SelectPtr test = boost::dynamic_pointer_cast<ISelectable>(player);

		if (test == selectedBrush->objects.front())
		{
			return true;
		}
	}

	return false;
}

void EditSession::RemoveSelectedObjects()
{
	//get list of selected ACTORs only
	SelectList actorsList;
	for (auto it = selectedBrush->objects.begin();
		it != selectedBrush->objects.end(); ++it)
	{
		if ((*it)->selectableType == ISelectable::ACTOR)
		{
			actorsList.push_back((*it));
		}
	}

	//get list of additional ACTORs from selected terrain
	SelectList addedActorsList;
	list<GateInfoPtr> tempGateList;
	for (auto it = selectedBrush->objects.begin();
		it != selectedBrush->objects.end(); ++it)
	{
		if ((*it)->selectableType == ISelectable::TERRAIN)
		{
			PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));

			for (auto objs = poly->enemies.begin(); objs != poly->enemies.end(); ++objs)
			{
				list<ActorPtr> &ap = (*objs).second;
				for (auto api = ap.begin(); api != ap.end(); ++api)
				{
					bool alreadyHere = false;
					for (auto ita = actorsList.begin();
						ita != actorsList.end(); ++ita)
					{
						if ((*api) == (*ita))
						{
							alreadyHere = true;
						}
					}
					if (!alreadyHere)
					{
						addedActorsList.push_back((*api));
					}
				}
			}

			poly->AddGatesToList(tempGateList);
		}
	}

	for (auto it = addedActorsList.begin(); it != addedActorsList.end(); ++it)
	{
		selectedBrush->AddObject((*it));
	}

	for (auto it = tempGateList.begin(); it != tempGateList.end(); ++it)
	{
		selectedBrush->AddObject((*it));
	}



	Action *remove = new RemoveBrushAction(selectedBrush);
	remove->Perform();

	doneActionStack.push_back(remove);

	ClearUndoneActions();

	selectedBrush->SetSelected(false);
	selectedBrush->Clear();
}

void EditSession::TryRemoveSelectedObjects()
{
	bool perform = true;

	if (IsOnlyPlayerSelected())
	{
		perform = false;
	}

	if (perform)
	{
		RemoveSelectedObjects();
	}
}

bool EditSession::IsSingleActorSelected()
{
	return (selectedBrush->objects.size() == 1
		&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR);
}

void EditSession::MoveSelectedActor( Vector2i &delta )
{
	//if (IsSingleActorSelected() )
	{
		ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>(selectedBrush->objects.front());
		if (actor->type->CanBeGrounded())
		{
			if (worldPosGround.ground != NULL)
			{
				if (actor->groundInfo != NULL)
				{
					actor->UnAnchor(actor);
				}

				actor->AnchorToGround(worldPosGround);
				worldPosGround.ground->enemies[worldPosGround.edgeStart].push_back(actor);
				worldPosGround.ground->UpdateBounds();
			}
			else
			{
				if (actor->groundInfo != NULL)
				{
					actor->UnAnchor(actor);
				}

				selectedBrush->Move(delta);
			}
		}
		else if (actor->type->CanBeRailGrounded())
		{
			if (worldPosRail.railGround != NULL)
			{
				if (actor->groundInfo != NULL)
				{
					actor->UnAnchor(actor);
				}

				actor->AnchorToRail(worldPosRail);
				worldPosRail.railGround->enemies[worldPosGround.edgeStart].push_back(actor);
				worldPosRail.railGround->UpdateBounds();
			}
			else
			{
				if (actor->groundInfo != NULL)
				{
					actor->UnAnchor(actor);
				}

				selectedBrush->Move(delta);
			}
		}
		else
		{
			selectedBrush->Move(delta);
		}
	}
}

void EditSession::StartTerrainMove()
{
	editStartMove = true;
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;

	for (PointMap::iterator mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit)
	{
		list<PointMoveInfo> &pList = (*mit).second;
		for (list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it)
		{
			(*it).delta = (*it).point->pos;
		}
	}

	for (auto mit = selectedRailPoints.begin(); mit != selectedRailPoints.end(); ++mit)
	{
		list<PointMoveInfo> &pList = (*mit).second;
		for (list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it)
		{
			(*it).delta = (*it).point->pos;
		}
	}

	moveAction = selectedBrush->UnAnchor();
	if (moveAction != NULL)
		moveAction->Perform();

	selectedBrush->Move(delta);

	pointGrabDelta = Vector2i(worldPos.x, worldPos.y) - pointGrabPos;
	oldPointGrabPos = pointGrabPos;
	pointGrabPos = Vector2i(worldPos.x, worldPos.y);

	MoveSelectedPoints(worldPos);
	MoveSelectedRailPoints(worldPos);

	editMouseGrabPos = pos;
}

void EditSession::ContinueTerrainMove()
{
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;

	if (IsSingleActorSelected() && selectedPoints.empty())
	{
		MoveSelectedActor(delta);
	}
	else
	{
		selectedBrush->Move(delta);

		pointGrabDelta = Vector2i(worldPos.x, worldPos.y) - pointGrabPos;
		oldPointGrabPos = pointGrabPos;
		pointGrabPos = Vector2i(worldPos.x, worldPos.y);

		MoveSelectedPoints(worldPos);
		MoveSelectedRailPoints(worldPos);
	}

	editMouseGrabPos = Vector2i(worldPos);
}

void EditSession::TryTerrainMove()
{
	//this secondary calculation makes the move not count for some window
	if ((editMouseDownMove && !editStartMove ))// && length(V2d(editMouseGrabPos.x, editMouseGrabPos.y) - worldPos) > editMoveThresh * zoomMultiple))
	{
		StartTerrainMove();
	}
	else if (editMouseDownMove && editStartMove)
	{
		ContinueTerrainMove();
	}
	else if (editMouseDownBox)
	{
		//stuff
	}
}

void EditSession::PreventNearPrimaryAnglesOnPolygonInProgress()
{
	if (polygonInProgress->numPoints > 0)
	{
		V2d backPoint = V2d(polygonInProgress->pointEnd->pos.x, polygonInProgress->pointEnd->pos.y);
		V2d tPoint(testPoint.x, testPoint.y);
		V2d extreme(0, 0);
		V2d vec = tPoint - backPoint;
		V2d normVec = normalize(vec);

		if (normVec.x > PRIMARY_LIMIT)
			extreme.x = 1;
		else if (normVec.x < -PRIMARY_LIMIT)
			extreme.x = -1;
		if (normVec.y > PRIMARY_LIMIT)
			extreme.y = 1;
		else if (normVec.y < -PRIMARY_LIMIT)
			extreme.y = -1;

		if (!(extreme.x == 0 && extreme.y == 0))
		{
			testPoint = Vector2f(backPoint + extreme * length(vec));
		}
	}
}

void EditSession::PreventNearPrimaryAnglesOnRailInProgress()
{
	if (railInProgress->numPoints > 0)
	{
		V2d backPoint = V2d(railInProgress->pointEnd->pos.x, railInProgress->pointEnd->pos.y);
		V2d tPoint(testPoint.x, testPoint.y);
		V2d extreme(0, 0);
		V2d vec = tPoint - backPoint;
		V2d normVec = normalize(vec);

		if (normVec.x > PRIMARY_LIMIT)
			extreme.x = 1;
		else if (normVec.x < -PRIMARY_LIMIT)
			extreme.x = -1;
		if (normVec.y > PRIMARY_LIMIT)
			extreme.y = 1;
		else if (normVec.y < -PRIMARY_LIMIT)
			extreme.y = -1;

		if (!(extreme.x == 0 && extreme.y == 0))
		{
			testPoint = Vector2f(backPoint + extreme * length(vec));
		}
	}
}

void EditSession::TryAddPointToPolygonInProgress()
{
	if (!panning && IsMousePressed(Mouse::Left))
	{
		Vector2i worldi(round(testPoint.x), round(testPoint.y));

		bool validPoint = polygonInProgress->IsValidInProgressPoint(worldi);//true;

		//test validity later
		if (validPoint)
		{
			if ( polygonInProgress->numPoints >= 3 && polygonInProgress->IsCloseToFirstPoint(GetZoomedPointSize(), V2d(worldi)))
			{
				if (ExecuteTerrainCompletion())
				{
					justCompletedPolyWithClick = true;
				}
				//complete polygon
			}
			else
			{
				polygonInProgress->AddPoint(new TerrainPoint(worldi, false));
			}
		}
	}
}

void EditSession::TryAddPointToRailInProgress()
{
	if (!panning && IsMousePressed(Mouse::Left))
	{
		bool validPoint = true;

		//test validity later
		if (validPoint)
		{
			Vector2i worldi(testPoint.x, testPoint.y);

			if (railInProgress->numPoints == 0 || (railInProgress->numPoints > 0 &&
				length(V2d(testPoint.x, testPoint.y)
					- Vector2<double>(railInProgress->pointEnd->pos.x,
						railInProgress->pointEnd->pos.y)) >= minimumEdgeLength * std::max(zoomMultiple, 1.0)))
			{
				railInProgress->AddPoint(new TerrainPoint(worldi, false));
			}
		}
	}
}

void EditSession::SetSelectedTerrainLayer(int layer)
{
	assert(layer == 0 || layer == 1);

	if (selectedBrush != NULL)
	{
		SelectList &sl = selectedBrush->objects;
		for (SelectList::iterator it = sl.begin(); it != sl.end(); ++it)
		{
			if ((*it)->selectableType == ISelectable::TERRAIN)
			{
				SelectPtr select = (*it);
				PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>(select);
				poly->SetLayer(layer);
			}
		}
	}
}

void EditSession::MoveTopBorder(int amount)
{
	topBound += amount;
	boundHeight -= amount;
	UpdateFullBounds();
}

void EditSession::MoveLeftBorder(int amount)
{
	leftBound += amount;
	boundWidth -= amount;
	UpdateFullBounds();
}

void EditSession::MoveRightBorder(int amount)
{
	boundWidth += amount;
	UpdateFullBounds();
}

void EditSession::ShowGrass(bool s)
{
	showGrass = s;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	{
		if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
		{
			boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
			d->ShowGrass(s);
		}
	}
}

void EditSession::TryPlaceTrackingEnemy()
{
	if (showPanel == NULL && trackingEnemy != NULL)
	{
		bool placementOkay = true;

		//air enemy
		if (enemyQuad.getLocalBounds().width == 0)
		{

		}
		else
		{
			sf::Transform tf = enemyQuad.getTransform();

			Vector2f fa = tf.transformPoint(enemyQuad.getPoint(0));
			Vector2f fb = tf.transformPoint(enemyQuad.getPoint(1));
			Vector2f fc = tf.transformPoint(enemyQuad.getPoint(2));
			Vector2f fd = tf.transformPoint(enemyQuad.getPoint(3));
			V2d a(fa.x, fa.y);
			V2d b(fb.x, fb.y);
			V2d c(fc.x, fc.y);
			V2d d(fd.x, fd.y);

			for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end() && placementOkay; ++it)
			{
				ActorGroup *ag = (*it).second;
				for (list<ActorPtr>::iterator git = ag->actors.begin(); git != ag->actors.end(); ++git)
				{
					ActorParams *params = (*git).get();
					V2d pa(params->boundingQuad[0].position.x, params->boundingQuad[0].position.y);
					V2d pb(params->boundingQuad[1].position.x, params->boundingQuad[1].position.y);
					V2d pc(params->boundingQuad[2].position.x, params->boundingQuad[2].position.y);
					V2d pd(params->boundingQuad[3].position.x, params->boundingQuad[3].position.y);
					//isQuadTouchingQuad( 

					cout << "a: " << a.x << ", " << a.y << ", b: " << b.x << ", " << b.y <<
						", " << c.x << ", " << c.y << ", " << d.x << ", " << d.y << endl;
					cout << "pa: " << pa.x << ", " << pa.y << ", pb: " << pb.x << ", " << pb.y <<
						", " << pc.x << ", " << pc.y << ", " << pd.x << ", " << pd.y << endl;

					cout << "testing vs: " << params->type->info.size.y << endl;
					if (isQuadTouchingQuad(pa, pb, pc, pd, a, b, c, d))
					{
						cout << "IS TOUCHING" << endl;
						placementOkay = false;
						break;
					}

				}

			}

		}

		if (!placementOkay)
		{
			MessagePop("can't place on top of another actor");
		}
		else
		{
			trackingEnemy->PlaceEnemy();
		}
	}
}

void EditSession::ModifyZoom(double factor)
{
	double old = zoomMultiple;

	zoomMultiple *= factor;

	if (zoomMultiple < minZoom)
		zoomMultiple = minZoom;
	else if (zoomMultiple > maxZoom)
		zoomMultiple = maxZoom;
	else if (abs(zoomMultiple - 1.0) < .1)
	{
		zoomMultiple = 1;
	}

	if (old != zoomMultiple)
	{
		UpdateFullBounds();

		Vector2<double> ff = Vector2<double>(view.getCenter().x, view.getCenter().y);//worldPos - ( - (  .5f * view.getSize() ) );
		view.setSize(Vector2f(960 * (zoomMultiple), 540 * (zoomMultiple)));
		preScreenTex->setView(view);
		Vector2f newWorldPosTemp = preScreenTex->mapPixelToCoords(GetPixelPos());
		Vector2<double> newWorldPos(newWorldPosTemp.x, newWorldPosTemp.y);
		Vector2<double> tempCenter = ff + (worldPos - newWorldPos);
		view.setCenter(tempCenter.x, tempCenter.y);
		preScreenTex->setView(view);
	}
}

Vector2i EditSession::GetPixelPos()
{
	Vector2i pPos = Mouse::getPosition(*w);
	pPos.x *= 1920.f / w->getSize().x;
	pPos.y *= 1080.f / w->getSize().y;

	return pPos;
}

void EditSession::UpdateCurrTerrainType()
{
	int ind = currTerrainWorld * MAX_TERRAINTEX_PER_WORLD + currTerrainVar;
	currTerrainTypeSpr.setTexture(*terrainTextures[ind]);
	currTerrainTypeSpr.setTextureRect(IntRect(0, 0, 64, 64));
}

void EditSession::AnchorTrackingEnemyOnTerrain()
{
	if (trackingEnemy != NULL)
	{
		string name = trackingEnemy->info.name;

		if (trackingEnemy->CanBeGrounded())
		{
			enemyEdgePolygon = NULL;

			double testRadius = 200;

			for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
			{
				if (testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
					&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius)
				{
					TerrainPoint *prev = (*it)->pointEnd;
					TerrainPoint *curr = (*it)->pointStart;

					bool contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y));

					//if ((contains && !(*it)->inverse) || (!contains && (*it)->inverse))
					{
						//prev is starting at 0. start normally at 1
						int edgeIndex = 0;
						double minDistance = 10000000;
						int storedIndex = -1;
						double storedQuantity;

						V2d closestPoint;

						for (; curr != NULL; curr = curr->next)
						{
							double dist = abs(
								cross(
									V2d(testPoint.x - prev->pos.x, testPoint.y - prev->pos.y),
									normalize(V2d(curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y))));
							double testQuantity = dot(
								V2d(testPoint.x - prev->pos.x, testPoint.y - prev->pos.y),
								normalize(V2d(curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y)));

							V2d pr(prev->pos.x, prev->pos.y);
							V2d cu(curr->pos.x, curr->pos.y);
							V2d te(testPoint.x, testPoint.y);

							V2d newPoint(pr.x + (cu.x - pr.x) * (testQuantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
								(testQuantity / length(cu - pr)));


							int hw = trackingEnemy->info.size.x / 2;
							int hh = trackingEnemy->info.size.y / 2;
							if (dist < 100 && testQuantity >= 0 && testQuantity <= length(cu - pr) && testQuantity >= hw && testQuantity <= length(cu - pr) - hw
								&& length(newPoint - te) < length(closestPoint - te))
							{
								minDistance = dist;
								storedIndex = edgeIndex;
								double l = length(cu - pr);

								storedQuantity = testQuantity;
								closestPoint = newPoint;

								if (name != "poi")
								{
									enemySprite.setOrigin(enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height);
									enemyQuad.setOrigin(enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height);
								}

								enemySprite.setPosition(closestPoint.x, closestPoint.y);
								enemySprite.setRotation(atan2((cu - pr).y, (cu - pr).x) / PI * 180);


								enemyQuad.setRotation(enemySprite.getRotation());
								enemyQuad.setPosition(enemySprite.getPosition());
							}

							prev = curr;
							++edgeIndex;
						}

						if (storedIndex >= 0)
						{
							enemyEdgeIndex = storedIndex;

							enemyEdgeQuantity = storedQuantity;

							enemyEdgePolygon = (*it).get();

							break;
						}
					}
				}
			}

		}
		else if (trackingEnemy->CanBeRailGrounded())
		{
			enemyEdgeRail = NULL;

			double testRadius = 200;
			for (auto it = rails.begin(); it != rails.end(); ++it)
			{
				if (testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
					&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius)
				{
					TerrainPoint *curr = (*it)->pointStart;
					TerrainPoint *next = NULL;

					bool contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y), 32);

					if (contains)
					{
						//prev is starting at 0. start normally at 1
						int edgeIndex = 0;
						double minDistance = 10000000;
						int storedIndex = -1;
						double storedQuantity;

						V2d closestPoint;

						for (; curr != NULL; curr = curr->next)
						{
							if (curr == (*it)->pointEnd)
							{
								break;
							}
							else
							{
								next = curr->next;
							}

							double dist = abs(
								cross(
									V2d(testPoint.x - curr->pos.x, testPoint.y - curr->pos.y),
									normalize(V2d(next->pos.x - curr->pos.x, next->pos.y - curr->pos.y))));
							double testQuantity = dot(
								V2d(testPoint.x - curr->pos.x, testPoint.y - curr->pos.y),
								normalize(V2d(next->pos.x - curr->pos.x, next->pos.y - curr->pos.y)));

							V2d pr(curr->pos.x, curr->pos.y);
							V2d cu(next->pos.x, next->pos.y);
							V2d te(testPoint.x, testPoint.y);

							V2d newPoint(pr.x + (cu.x - pr.x) * (testQuantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
								(testQuantity / length(cu - pr)));


							int hw = trackingEnemy->info.size.x / 2;
							int hh = trackingEnemy->info.size.y / 2;
							if (dist < 100 && testQuantity >= 0 && testQuantity <= length(cu - pr) && testQuantity >= hw && testQuantity <= length(cu - pr) - hw
								&& length(newPoint - te) < length(closestPoint - te))
							{
								minDistance = dist;
								storedIndex = edgeIndex;
								double l = length(cu - pr);

								storedQuantity = testQuantity;
								closestPoint = newPoint;

								if (name != "poi")
								{
							//		enemySprite.setOrigin(enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height);
						//			enemyQuad.setOrigin(enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height);
								}

								enemySprite.setPosition(closestPoint.x, closestPoint.y);
								//enemySprite.setRotation(atan2((cu - pr).y, (cu - pr).x) / PI * 180);


								//enemyQuad.setRotation(enemySprite.getRotation());
								enemyQuad.setPosition(enemySprite.getPosition());
							}

							++edgeIndex;
						}

						if (storedIndex >= 0)
						{
							enemyEdgeIndex = storedIndex;

							enemyEdgeQuantity = storedQuantity;

							enemyEdgeRail = (*it).get();

							break;
						}
						

						
					}
				}

			}
		}
	}
}

void EditSession::MoveTrackingEnemy()
{
	if (trackingEnemy != NULL && showPanel == NULL)
	{
		enemySprite.setOrigin(enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height / 2);
		enemySprite.setRotation(0);

		Vector2f p = preScreenTex->mapPixelToCoords(pixelPos);

		enemySprite.setPosition(p);

		enemyQuad.setOrigin(enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height / 2);
		enemyQuad.setRotation(0);
		enemyQuad.setPosition(enemySprite.getPosition());

		AnchorTrackingEnemyOnTerrain();
	}
}

void EditSession::TryAddToPatrolPath()
{
	V2d pathBack(patrolPath.back());
	V2d temp = V2d(testPoint.x, testPoint.y) - pathBack;

	if (!panning && IsMousePressed(Mouse::Left))
	{
		//double test = 100;
		//worldPos before testPoint

		double tempQuant = length(temp);
		if (tempQuant >= minimumPathEdgeLength * std::max(zoomMultiple, 1.0)
			&& tempQuant > patrolPathLengthSize / 2)
		{

			if (patrolPathLengthSize > 0)
			{
				V2d temp1 = V2d(patrolPath.back().x, patrolPath.back().y);
				temp = normalize(V2d(testPoint.x, testPoint.y) - temp1)
					* (double)patrolPathLengthSize + temp1;
				Vector2i worldi(temp.x, temp.y);
				patrolPath.push_back(worldi);
			}
			else
			{
				Vector2i worldi(testPoint.x, testPoint.y);
				patrolPath.push_back(worldi);
			}
		}
	}
}

void EditSession::SetEnemyLevel()
{
	if (IsMousePressed(Mouse::Left))
	{
		for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			list<ActorPtr> &actors = it->second->actors;
			for (list<ActorPtr>::iterator it2 = actors.begin(); it2 != actors.end(); ++it2)
			{
				sf::FloatRect bounds = (*it2)->image.getGlobalBounds();
				if (bounds.contains(Vector2f(worldPos.x, worldPos.y)))
				{
					(*it2)->SetLevel(setLevelCurrent);
				}
			}
		}
	}
}

void EditSession::UpdatePanning()
{
	V2d tempWorldPos = V2d(preScreenTex->mapPixelToCoords(pixelPos));
	if (panning)
	{
		Vector2<double> temp = panAnchor - tempWorldPos;
		view.move(Vector2f(temp.x, temp.y));
	}
}

void EditSession::UpdatePolyShaders()
{
	Vector2f vSize = view.getSize();
	float zoom = vSize.x / 960;
	Vector2f botLeft(view.getCenter().x - vSize.x / 2, view.getCenter().y + vSize.y / 2);
	for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
	{
		if (terrainTextures[i] != NULL)
		{
			polyShaders[i].setUniform("zoom", zoom);
			polyShaders[i].setUniform("topLeft", botLeft);
			//just need to change the name topleft  to botleft eventually
		}
	}
}

void EditSession::TempMoveSelectedBrush()
{
	if (mode == EDIT)
	{
		if (moveActive)
		{
			Vector2i currMouse(worldPos.x, worldPos.y);
			Vector2i delta = currMouse - pointMouseDown;
			pointMouseDown = currMouse;

			selectedBrush->Move(delta);
			selectedBrush->Draw(preScreenTex);
		}
	}
}

void EditSession::DrawGraph()
{
	if (showGraph)
	{
		graph->SetCenterAbsolute(view.getCenter());
		graph->Draw(preScreenTex);
	}
	/*if (showGraph)
	{
		VertexArray &graphLines = *graphLinesVA;
		Vector2f adjustment;
		for (int i = 0; i < numGraphLines * 8; ++i)
		{
			int adjX, adjY;
			float x = view.getCenter().x;
			float y = view.getCenter().y;

			x /= 32;
			y /= 32;

			if (x > 0)
				x += .5f;
			else if (x < 0)
				x -= .5f;

			if (y > 0)
				y += .5f;
			else if (y < 0)
				y -= .5f;

			adjX = ((int)x) * 32;
			adjY = ((int)y) * 32;

			adjustment = Vector2f(adjX, adjY);

			graphLines[i].position += adjustment;
		}

		preScreenTex->draw(graphLines);

		for (int i = 0; i < numGraphLines * 8; ++i)
		{
			graphLines[i].position -= adjustment;
		}
	}*/
}

void EditSession::SetupGraph()
{
	VertexArray &graphLines = *graphLinesVA;

	int graphSep = 32;
	int graphMax = graphSep * numGraphLines;
	int temp = -graphMax;

	//horiz
	for (int i = 0; i < numGraphLines * 4; i += 2)
	{
		graphLines[i] = sf::Vertex(sf::Vector2<float>(-graphMax, temp), graphColor);
		graphLines[i + 1] = sf::Vertex(sf::Vector2<float>(graphMax, temp), graphColor);
		temp += graphSep;
	}

	//vert
	temp = -graphMax;
	for (int i = numGraphLines * 4; i < numGraphLines * 8; i += 2)
	{
		graphLines[i] = sf::Vertex(sf::Vector2<float>(temp, -graphMax), graphColor);
		graphLines[i + 1] = sf::Vertex(sf::Vector2<float>(temp, graphMax), graphColor);
		temp += graphSep;
	}
}

void EditSession::DrawPolygons()
{
	for (int i = 0; i < 2; ++i)
	{
		auto & currPolyList = GetCorrectPolygonList(i);
		for (list<PolyPtr>::iterator it = currPolyList.begin(); it != currPolyList.end(); ++it)
		{
			(*it)->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
		}
	}
}

void EditSession::DrawRails()
{
	for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->Draw(zoomMultiple, showPoints, preScreenTex);
	}
}

void EditSession::DrawPolygonInProgress()
{
	int progressSize = polygonInProgress->numPoints;
	if (progressSize > 0)
	{
		Vector2i backPoint = polygonInProgress->pointEnd->pos;
		Vector2i worldi = Vector2i(round(testPoint.x), round(testPoint.y));


		Color validColor = Color::Green;
		Color invalidColor = Color::Red;
		Color colorSelection;

		bool valid = polygonInProgress->IsValidInProgressPoint(worldi);
		//cout << "draw testpoint: " << testPoint.x << ", " << testPoint.y << endl;
		if (valid)
		{
			colorSelection = validColor;
		}
		else
		{
			colorSelection = invalidColor;
		}

		int pSize = progressSize;
		if (progressSize == 1)
		{
			pSize += 1;
		}
		else
		{
			pSize += 2;
		}

		VertexArray v(sf::LinesStrip, pSize);

		int i = 0;

		if (progressSize > 1)
		{
			for (TerrainPoint *curr = polygonInProgress->pointStart; curr != NULL; curr = curr->next)
			{
				v[i] = Vertex(Vector2f(curr->pos.x, curr->pos.y), validColor);
				++i;
			}
		}

		v[i] = Vertex(Vector2f(backPoint), colorSelection);
		v[i + 1] = Vertex(Vector2f(worldi), colorSelection);

		
		preScreenTex->draw(v);

		CircleShape cs;
		cs.setRadius(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Green);


		for (TerrainPoint *progressCurr = polygonInProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next)
		{
			cs.setPosition(progressCurr->pos.x, progressCurr->pos.y);
			preScreenTex->draw(cs);
		}
	}
}

void EditSession::DrawRailInProgress()
{
	int progressSize = railInProgress->numPoints;
	if (progressSize > 0)
	{
		Vector2i backPoint = railInProgress->pointEnd->pos;

		Color validColor = Color::Yellow;
		Color invalidColor = Color::Red;
		Color colorSelection;
		if (true)
		{
			colorSelection = validColor;
		}

		{
			sf::Vertex activePreview[2] =
			{
				sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection),
				sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
			};


			preScreenTex->draw(activePreview, 2, sf::Lines);
		}

		if (progressSize > 1)
		{
			VertexArray v(sf::LinesStrip, progressSize);
			int i = 0;
			for (TerrainPoint *curr = railInProgress->pointStart; curr != NULL; curr = curr->next)
			{
				v[i] = Vertex(Vector2f(curr->pos.x, curr->pos.y));
				++i;
			}
			preScreenTex->draw(v);
		}

		CircleShape cs;
		cs.setRadius(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Red);


		for (TerrainPoint *progressCurr = railInProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next)
		{
			cs.setPosition(progressCurr->pos.x, progressCurr->pos.y);
			preScreenTex->draw(cs);
		}
	}
}

void EditSession::DrawActors()
{
	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		(*it).second->Draw(preScreenTex);
	}
}

void EditSession::DrawGates()
{
	for (list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void EditSession::DrawDecorBehind()
{
	for (auto it = decorImagesBehindTerrain.begin(); it != decorImagesBehindTerrain.end(); ++it)
	{
		preScreenTex->draw((*it)->spr);
	}
}

void EditSession::DrawDecorBetween()
{
	for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void EditSession::DrawBoxSelection()
{
	if (editMouseDownBox)
	{
		Vector2i currPos(worldPos.x, worldPos.y);

		int left = std::min(editMouseOrigPos.x, currPos.x);
		int right = std::max(editMouseOrigPos.x, currPos.x);
		int top = std::min(editMouseOrigPos.y, currPos.y);
		int bot = std::max(editMouseOrigPos.y, currPos.y);

		sf::RectangleShape rs(Vector2f(right - left, bot - top));
		rs.setFillColor(Color(200, 200, 200, 80));
		rs.setPosition(left, top);
		preScreenTex->draw(rs);
	}
}

void EditSession::DrawTrackingEnemy()
{
	if (trackingEnemy != NULL)
	{
		if (tempActor != NULL)
		{
			tempActor->Draw(preScreenTex);
		}
		else
		{
			preScreenTex->draw(enemySprite);
		}
		preScreenTex->draw(enemyQuad);
	}
}

void EditSession::DrawPatrolPathInProgress()
{
	int pathSize = patrolPath.size();
	if (pathSize > 0)
	{
		Vector2i backPoint = patrolPath.back();

		Color validColor = Color::Green;
		Color invalidColor = Color::Red;
		Color colorSelection;
		if (true)
		{
			colorSelection = validColor;
		}
		sf::Vertex activePreview[2] =
		{
			sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection),
			sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
		};
		preScreenTex->draw(activePreview, 2, sf::Lines);

		if (pathSize > 1)
		{
			VertexArray v(sf::LinesStrip, pathSize);
			int i = 0;
			for (list<sf::Vector2i>::iterator it = patrolPath.begin();
				it != patrolPath.end(); ++it)
			{
				v[i] = Vertex(Vector2f((*it).x, (*it).y));
				++i;
			}
			preScreenTex->draw(v);
		}
	}

	if (pathSize > 0) //always
	{
		CircleShape cs;
		cs.setRadius(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Green);


		for (list<Vector2i>::iterator it = patrolPath.begin(); it != patrolPath.end(); ++it)
		{
			cs.setPosition((*it).x, (*it).y);
			preScreenTex->draw(cs);
		}
	}
}

double EditSession::GetZoomedPointSize()
{
	return POINT_SIZE * zoomMultiple;
}

void EditSession::DrawGateInProgress()
{
	if (gatePoints > 0)
	{
		CircleShape cs(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setPosition(testGateInfo.point0->pos.x, testGateInfo.point0->pos.y);
		cs.setFillColor(COLOR_TEAL);

		V2d origin(testGateInfo.point0->pos.x, testGateInfo.point0->pos.y);

		V2d pointB;
		if (gatePoints > 1)
		{
			pointB = V2d(testGateInfo.point1->pos.x, testGateInfo.point1->pos.y);
		}
		else
		{
			pointB = worldPos;
		}


		V2d axis = normalize(worldPos - origin);
		V2d other(axis.y, -axis.x);


		double width = 4.0 * zoomMultiple;
		V2d closeA = origin + other * width;
		V2d closeB = origin - other * width;
		V2d farA = pointB + other * width;
		V2d farB = pointB - other * width;

		Color c;
		if (gatePoints == 1)
		{
			if (IsGateInProgressValid(testGateInfo.poly0, testGateInfo.point0))
			{
				c = Color::White;
			}
			else
			{
				c = Color::Red;
			}
		}

		sf::Vertex quad[4] = {
			sf::Vertex(Vector2f(closeA.x, closeA.y), c),
			sf::Vertex(Vector2f(farA.x, farA.y),c),
			sf::Vertex(Vector2f(farB.x, farB.y), c),
			sf::Vertex(Vector2f(closeB.x , closeB.y), c)
		};

		preScreenTex->draw(quad, 4, sf::Quads);
		preScreenTex->draw(cs);
	}
}

void EditSession::DrawDecorFront()
{
	for (auto it = decorImagesFrontTerrain.begin(); it != decorImagesFrontTerrain.end(); ++it)
	{
		preScreenTex->draw((*it)->spr);
	}
}

void EditSession::DrawMode()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
	{
		DrawPolygonInProgress();
		break;
	}
	case CREATE_RAILS:
	{
		DrawRailInProgress();
		break;
	}
	case EDIT:
	{
		DrawBoxSelection();

		break;
	}
	case CREATE_ENEMY:
	{
		DrawTrackingEnemy();
		break;
	}
	case CREATE_RECT:
	{
		rectCreatingTrigger->Draw(preScreenTex);
		break;
	}
	case SET_CAM_ZOOM:
	{
		currentCameraShot->Draw(preScreenTex);
		break;
	}
	case CREATE_PATROL_PATH:
	{
		DrawTrackingEnemy();
		DrawPatrolPathInProgress();
		break;
	}
	case SET_DIRECTION:
	{
		DrawTrackingEnemy();
		DrawPatrolPathInProgress();
		break;
	}
	case SELECT_MODE:
	{
		break;
	}
	case CREATE_GATES:
	{
		DrawGateInProgress();
		break;
	}
	}
}

void EditSession::DrawModeUI()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
	{
		break;
	}
	case CREATE_ENEMY:
	{
		break;
	}
	case SELECT_MODE:
	{
		preScreenTex->draw(guiMenuSprite);


		Color c;


		CircleShape cs;
		cs.setRadius(menuCircleRadius);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);


		cs.setFillColor(COLOR_BLUE);
		cs.setPosition((menuDownPos + circleUpperRightPos).x, (menuDownPos + circleUpperRightPos).y);
		preScreenTex->draw(cs);

		sf::Text textblue;
		textblue.setCharacterSize(14);
		textblue.setFont(arial);
		textblue.setString("CREATE\nTERRAIN");
		textblue.setFillColor(sf::Color::White);
		textblue.setOrigin(textblue.getLocalBounds().width / 2, textblue.getLocalBounds().height / 2);
		textblue.setPosition((menuDownPos + circleUpperRightPos).x, (menuDownPos + circleUpperRightPos).y);
		preScreenTex->draw(textblue);


		cs.setFillColor(COLOR_GREEN);
		cs.setPosition((menuDownPos + circleLowerRightPos).x, (menuDownPos + circleLowerRightPos).y);
		preScreenTex->draw(cs);

		sf::Text textgreen;
		textgreen.setCharacterSize(14);
		textgreen.setFont(arial);
		textgreen.setString("MAP\nOPTIONS");
		textgreen.setFillColor(sf::Color::White);
		textgreen.setOrigin(textgreen.getLocalBounds().width / 2, textgreen.getLocalBounds().height / 2);
		textgreen.setPosition((menuDownPos + circleLowerRightPos).x, (menuDownPos + circleLowerRightPos).y);
		preScreenTex->draw(textgreen);


		cs.setFillColor(COLOR_YELLOW);
		cs.setPosition((menuDownPos + circleBottomPos).x, (menuDownPos + circleBottomPos).y);
		preScreenTex->draw(cs);

		cs.setFillColor(COLOR_ORANGE);
		cs.setPosition((menuDownPos + circleLowerLeftPos).x, (menuDownPos + circleLowerLeftPos).y);
		preScreenTex->draw(cs);

		sf::Text textorange;
		textorange.setString("CREATE\nLIGHTS");
		textorange.setFont(arial);
		textorange.setCharacterSize(14);
		textorange.setFillColor(sf::Color::White);
		textorange.setOrigin(textorange.getLocalBounds().width / 2, textorange.getLocalBounds().height / 2);
		textorange.setPosition((menuDownPos + circleLowerLeftPos).x, (menuDownPos + circleLowerLeftPos).y);
		preScreenTex->draw(textorange);

		cs.setFillColor(COLOR_RED);
		cs.setPosition((menuDownPos + circleUpperLeftPos).x, (menuDownPos + circleUpperLeftPos).y);
		preScreenTex->draw(cs);

		sf::Text textred;
		textred.setString("CREATE\nENEMIES");
		textred.setFont(arial);
		textred.setCharacterSize(14);
		textred.setFillColor(sf::Color::White);
		textred.setOrigin(textred.getLocalBounds().width / 2, textred.getLocalBounds().height / 2);
		textred.setPosition((menuDownPos + circleUpperLeftPos).x, (menuDownPos + circleUpperLeftPos).y);
		preScreenTex->draw(textred);

		cs.setFillColor(COLOR_MAGENTA);
		cs.setPosition((menuDownPos + circleTopPos).x, (menuDownPos + circleTopPos).y);
		preScreenTex->draw(cs);

		sf::Text textmag;

		bool singleObj, singleActor, singleImage, singleRail, onlyPoly; 
		
		singleObj = selectedBrush->objects.size() == 1 && selectedPoints.size() == 0;

		singleActor = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
		singleImage = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
		singleRail = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::RAIL;
		onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

		if (menuDownStored == EditSession::EDIT && singleActor)
		{
			textmag.setString("EDIT\nENEMY");
		}
		else if (menuDownStored == EditSession::EDIT && singleImage)
		{
			textmag.setString("EDIT\nIMAGE");
		}
		else if (menuDownStored == EditSession::EDIT && onlyPoly)
		{
			textmag.setString("TERRAIN\nOPTIONS");
		}
		else if(menuDownStored == EditSession::EDIT && singleRail)
		{
			textmag.setString("RAIL\nOPTIONS");
		}
		else
		{
			textmag.setString("EDIT");
		}

		textmag.setFont(arial);
		textmag.setCharacterSize(14);
		textmag.setFillColor(sf::Color::White);
		textmag.setOrigin(textmag.getLocalBounds().width / 2, textmag.getLocalBounds().height / 2);
		textmag.setPosition((menuDownPos + circleTopPos).x, (menuDownPos + circleTopPos).y);
		preScreenTex->draw(textmag);

		break;
	}
	case EDIT:
	{

		break;
	}
	case CREATE_IMAGES:

		break;
	}
}

void EditSession::DrawUI()
{
	preScreenTex->setView(uiView);
	stringstream cursorPosSS;
	stringstream scaleTextSS;
	if (mode == CREATE_PATROL_PATH || mode == SET_DIRECTION)
	{
		V2d temp = V2d(testPoint.x, testPoint.y) - Vector2<double>(patrolPath.back().x,
			patrolPath.back().y);
		cursorPosSS << (int)temp.x << ", " << (int)temp.y;
	}
	else
	{
		cursorPosSS << (int)worldPos.x << ", " << (int)worldPos.y;
	}
	cursorLocationText.setString(cursorPosSS.str());

	Vector2f size = uiView.getSize();
	float sca = view.getSize().x / 960.f / 2.f;
	scaleSprite.setScale(1.f / sca, 1.f / sca);
	scaleTextSS << "scale: x" << scaleSprite.getScale().x;
	scaleSpriteBGRect.setSize(Vector2f(scaleSprite.getGlobalBounds().width,
		scaleSprite.getGlobalBounds().height));
	scaleText.setString(scaleTextSS.str());

	if (mode == CREATE_TERRAIN || mode == EDIT || mode == SELECT_MODE)
	{
		preScreenTex->draw(currTerrainTypeSpr);
	}
	

	preScreenTex->draw(scaleSpriteBGRect);
	preScreenTex->draw(scaleSprite);
	preScreenTex->draw(cursorLocationText);
	preScreenTex->draw(scaleText);

	DrawModeUI();

	if (showPanel != NULL)
	{
		showPanel->Draw(preScreenTex);
	}
}

void EditSession::Display()
{
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();

	Sprite preTexSprite(preTex);
	preTexSprite.setPosition(-960 / 2, -540 / 2);
	preTexSprite.setScale(.5, .5);
	w->clear();
	w->draw(preTexSprite);
	w->display();
}

void EditSession::HandleEvents()
{
	while (w->pollEvent(ev))
	{
		if (ev.type == Event::KeyPressed)
		{
			if (ev.key.code == Keyboard::F5)
			{
				background->FlipShown();
				continue;
			}
		}

		switch (mode)
		{
		case CREATE_TERRAIN:
		{
			CreateTerrainModeHandleEvent();
			break;
		}
		case CREATE_RAILS:
		{
			CreateRailsModeHandleEvent();
			break;
		}
		case EDIT:
		{
			EditModeHandleEvent();
			break;
		}
		case PASTE:
		{
			PasteModeHandleEvent();
			break;
		}
		case CREATE_ENEMY:
		{
			CreateEnemyModeHandleEvent();
			break;
		}
		case PAUSED:
		{
			PausedModeHandleEvent();
			break;
		}
		case SELECT_MODE:
		{
			SelectModeHandleEvent();
			break;
		}
		case CREATE_PATROL_PATH:
		{
			CreatePatrolPathModeHandleEvent();
			break;
		}
		case CREATE_RECT:
		{
			CreateRectModeHandleEvent();
			break;
		}
		case SET_CAM_ZOOM:
		{
			SetCamZoomModeHandleEvent();
			break;
		}
		case SET_DIRECTION:
		{
			SetDirectionModeHandleEvent();
			break;
		}
		case CREATE_GATES:
		{
			CreateGatesModeHandleEvent();
			break;
		}
		case CREATE_IMAGES:
		{
			CreateImagesHandleEvent();
			break;
		}
		case SET_LEVEL:
		{
			SetLevelModeHandleEvent();
			break;
		}


		}
		//ones that aren't specific to mode

		if (mode != PAUSED && mode != SELECT_MODE)
		{
			switch (ev.type)
			{
			case Event::MouseButtonPressed:
			{
				if (ev.mouseButton.button == Mouse::Button::Middle)
				{
					panning = true;
					panAnchor = worldPos;
					cout << "setting panAnchor: " << panAnchor.x << " , " << panAnchor.y << endl;
				}
				else if (ev.mouseButton.button == Mouse::Button::Right)
				{
					menuDownStored = mode;
					mode = SELECT_MODE;
					menuDownPos = V2d(uiMousePos.x, uiMousePos.y);
					guiMenuSprite.setPosition(uiMousePos.x, uiMousePos.y);
				}
				break;
			}
			case Event::MouseButtonReleased:
			{
				if (ev.mouseButton.button == Mouse::Button::Middle)
				{
					panning = false;
				}
				break;
			}
			case Event::MouseWheelMoved:
			{
				if (ev.mouseWheel.delta > 0)
				{
					ModifyZoom(.5);
				}
				else if (ev.mouseWheel.delta < 0)
				{
					ModifyZoom(2);
				}
				break;
			}
			case Event::KeyPressed:
			{
				if (ev.key.code == Keyboard::S && ev.key.control)
				{
					polygonInProgress->ClearPoints();
					cout << "writing to file: " << currentFile << endl;
					WriteFile(currentFile);
				}
				else if (ev.key.code == Keyboard::T && showPanel == NULL)
				{
					quit = true;
				}
				else if (ev.key.code == Keyboard::Escape)
				{
					quit = true;
					returnVal = 1;
				}
				else if (ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash)
				{
					if (showPanel != NULL)
						break;

					if (ev.key.code == sf::Keyboard::Equal)
					{
						ModifyZoom(.5);
					}
					else if (ev.key.code == sf::Keyboard::Dash)
					{
						ModifyZoom(2);
					}
					break;
				}
				break;
			}
			case Event::KeyReleased:
			{
				break;
			}
			case Event::LostFocus:
			{
				stored = mode;
				mode = PAUSED;
				break;
			}
			case Event::GainedFocus:
			{
				mode = stored;
				break;
			}
			}
		}

	}
}

void EditSession::CreateTerrainModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
				break;
			}
		}

		justCompletedPolyWithClick = false; //just make this always false here.

		break;
	}
	case Event::MouseButtonReleased:
	{
		if (showPanel != NULL)
		{
			showPanel->Update(false, uiMousePos.x, uiMousePos.y);
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (showPanel != NULL)
		{
			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::Space)
		{
			ExecuteTerrainCompletion();
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			RemovePointFromPolygonInProgress();
		}
		else if (ev.key.code == sf::Keyboard::E)
		{
			GridSelectPop("terraintypeselect");
			currTerrainWorld = tempGridX;
			currTerrainVar = tempGridY;
			UpdateCurrTerrainType();
		}
		else if (ev.key.code == sf::Keyboard::R)
		{
			mode = CREATE_RAILS;
			railInProgress->CopyOtherPoints(polygonInProgress->pointStart,
				polygonInProgress->pointEnd);
			//railInProgress->CopyPoints(polygonInProgress->pointStart, polygonInProgress->pointEnd);
			polygonInProgress->ClearPoints();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}

		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::CreateRailsModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
				break;
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (showPanel != NULL)
		{
			showPanel->Update(false, uiMousePos.x, uiMousePos.y);
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (showPanel != NULL)
		{
			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::Space)
		{
			ExecuteRailCompletion();
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			RemovePointFromRailInProgress();
		}
		else if (ev.key.code == sf::Keyboard::E)
		{
		}
		else if (ev.key.code == sf::Keyboard::R)
		{
			mode = CREATE_TERRAIN;
			railInProgress->ClearPoints();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}

		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::EditModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
				break;
			}

			if (showGrass)
				break;

			bool emptysp = true;
			bool specialTerrain = IsSpecialTerrainMode();

			if (!(editMouseDownMove || editMouseDownBox))
			{
				if (emptysp && !specialTerrain && PointSelectActor(worldPos))
				{
					emptysp = false;
				}

				if (emptysp && !specialTerrain && PointSelectRail(worldPos))
				{
					emptysp = false;
				}

				if (emptysp && PointSelectTerrain(worldPos))
				{
					emptysp = false;
				}

				if (emptysp && !specialTerrain && PointSelectDecor(worldPos))
				{
					emptysp = false;
				}

				editMouseGrabPos = Vector2i(worldPos.x, worldPos.y);
				pointGrabPos = Vector2i(worldPos.x, worldPos.y);
				editMouseOrigPos = editMouseGrabPos;

				if (emptysp)
				{
					editMouseDownMove = false;
					editMouseDownBox = true;
					editStartMove = false;
				}
				else
				{
					editMouseDownMove = true;
					editStartMove = false;
					editMouseDownBox = false;
				}
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(false, uiMousePos.x, uiMousePos.y);
				break;
			}

			if (editStartMove)
			{
				bool done = false;
				if (AnchorSelectedAerialEnemy())
				{
					done = true;
				}

				if (!done)
				{
					PerformMovePointsAction();
				}

				TryMoveSelectedBrush();
			}
			else if (editMouseDownBox)
			{
				TryBoxSelect();
			}

			editMouseDownBox = false;
			editMouseDownMove = false;
			editStartMove = false;

			UpdateGrass();
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::MouseMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (showPanel != NULL)
		{
			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::Tilde)
		{
			mode = SET_LEVEL;
			setLevelCurrent = 1;
			showPanel = NULL;
		}

		if (ev.key.code == Keyboard::I && ev.key.control)
		{
			mode = CREATE_IMAGES;
			currImageTool = ITOOL_EDIT;
			showPanel = decorPanel;
		}
		if (ev.key.code == Keyboard::C && ev.key.control)
		{
			copiedBrush = selectedBrush->Copy();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		else if (ev.key.code == Keyboard::V && ev.key.control)
		{
			if (copiedBrush != NULL)
			{
				Vector2i pos = Vector2i(worldPos.x, worldPos.y);
				copiedBrush->Move(pos - copiedBrush->GetCenter());
				editMouseGrabPos = pos;
				mode = PASTE;

				selectedBrush->SetSelected(false);
				selectedBrush->Clear();
			}
		}
		else if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			if (CountSelectedPoints() > 0)
			{
				TryRemoveSelectedPoints();
			}
			else
			{
				TryRemoveSelectedObjects();
			}
		}
		else if (ev.key.code == Keyboard::R)
		{
			ShowGrass(true);
		}
		else if (ev.key.code == Keyboard::P)
		{
			SetSelectedTerrainLayer(1);
		}
		else if (ev.key.code == Keyboard::O)
		{
			SetSelectedTerrainLayer(0);
		}
		else if (ev.key.code == Keyboard::E)
		{
			GridSelectPop("terraintypeselect");

			if (selectedBrush->objects.size() > 0)
			{
				Action * action = new ModifyTerrainTypeAction(
					selectedBrush, tempGridX, tempGridY);
				action->Perform();
				doneActionStack.push_back(action);
			}

			currTerrainWorld = tempGridX;
			currTerrainVar = tempGridY;
			UpdateCurrTerrainType();
		}
		else if (ev.key.code == Keyboard::I)
		{
			if (ev.key.shift)
			{
				MoveTopBorder(borderMove);
			}
			else
			{
				MoveTopBorder(-borderMove);
			}
		}
		else if (ev.key.code == Keyboard::J)
		{
			if (ev.key.shift)
			{
				MoveLeftBorder(borderMove);
			}
			else
			{
				MoveLeftBorder(-borderMove);
			}
		}
		else if (ev.key.code == Keyboard::L)
		{
			if (ev.key.shift)
			{
				MoveRightBorder(-borderMove);
			}
			else
			{
				MoveRightBorder(borderMove);
			}
		}
		break;
	}
	case Event::KeyReleased:
	{
		if (ev.key.code == Keyboard::R)
		{
			ShowGrass(false);
		}
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::PasteModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == sf::Mouse::Button::Left)
		{
			PasteTerrain(copiedBrush);
			if (!HoldingControl())
			{
				mode = EDIT;
			}
		}
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::X)
		{
			mode = EDIT;
		}
		break;
	}
	}
}

void EditSession::CreateEnemyModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			TryPlaceTrackingEnemy();

			if (showPanel != NULL)
			{
				showPanel->Update(false, uiMousePos.x, uiMousePos.y);
			}
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (showPanel != NULL)
		{
			if (showPanel == enemySelectPanel)
			{
				if (ev.key.code == Keyboard::Num1)
				{
					SetActiveEnemyGrid(0);
				}
				else if (ev.key.code == Keyboard::Num2)
				{
					SetActiveEnemyGrid(1);
				}
				else if (ev.key.code == Keyboard::Num3)
				{
					SetActiveEnemyGrid(2);
				}
				else if (ev.key.code == Keyboard::Num4)
				{
					SetActiveEnemyGrid(3);
				}
			}

			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			if (trackingEnemy != NULL)
			{
				trackingEnemy = NULL;
				showPanel = enemySelectPanel;
			}
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::PausedModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		break;
	}
	case Event::MouseButtonReleased:
	{
		break;
	}
	case Event::GainedFocus:
	{
		mode = stored;
		break;
	}
	}
}

void EditSession::SelectModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		break;
	}
	case Event::MouseButtonReleased:
	{
		V2d releasePos(uiMousePos.x, uiMousePos.y);

		V2d worldTop = menuDownPos + circleTopPos;
		V2d worldUpperLeft = menuDownPos + circleUpperLeftPos;
		V2d worldUpperRight = menuDownPos + circleUpperRightPos;
		V2d worldLowerRight = menuDownPos + circleLowerRightPos;
		V2d worldLowerLeft = menuDownPos + circleLowerLeftPos;
		V2d worldBottom = menuDownPos + circleBottomPos;


		if (length(releasePos - worldTop) < menuCircleRadius)
		{
			menuSelection = "top";
		}
		else if (length(releasePos - worldUpperLeft) < menuCircleRadius)
		{
			menuSelection = "upperleft";
		}
		else if (length(releasePos - worldUpperRight) < menuCircleRadius)
		{
			menuSelection = "upperright";
		}
		else if (length(releasePos - worldLowerLeft) < menuCircleRadius)
		{
			menuSelection = "lowerleft";
		}
		else if (length(releasePos - worldLowerRight) < menuCircleRadius)
		{
			menuSelection = "lowerright";
		}
		else if (length(releasePos - worldBottom) < menuCircleRadius)
		{
			menuSelection = "bottom";
		}
		else
		{
			mode = menuDownStored;
			menuSelection = "none";
		}

		if (menuDownStored == EDIT && menuSelection != "none" && menuSelection != "top")
		{
			if (menuDownStored == EDIT)
			{
				selectedBrush->SetSelected(false);
				selectedBrush->Clear();
			}
		}
		else if (menuDownStored == CREATE_TERRAIN && menuSelection != "none")
		{
			polygonInProgress->ClearPoints();
		}

		if (menuSelection == "top")
		{
			bool singleObject = selectedBrush->objects.size() == 1
				&& selectedPoints.size() == 0;
			bool singleActor = singleObject
				&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
			bool singleImage = singleObject
				&& selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
			bool singleRail = singleObject
				&& selectedBrush->objects.front()->selectableType == ISelectable::RAIL;

			//bool singlePoly = selectedBrush->objects.size() == 1 
			//	&& selectedPoints.size() == 0
			//	&& selectedBrush->objects.front()->selectableType == ISelectable::TERRAIN;

			bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

			if (menuDownStored == EDIT && onlyPoly)
			{
				showPanel = terrainOptionsPanel;
				mode = menuDownStored;
			}
			else if (menuDownStored == EDIT && singleActor)
			{
				SetEnemyEditPanel();
				mode = menuDownStored;
			}
			else if (menuDownStored == EDIT && singleImage)
			{
				showPanel = editDecorPanel;
				SetDecorEditPanel();
				mode = menuDownStored;
			}
			else if (menuDownStored == EDIT && singleRail)
			{
				showPanel = railOptionsPanel;

				ISelectable *select = selectedBrush->objects.front().get();
				TerrainRail *tr = (TerrainRail*)select;
				tr->UpdatePanel(railOptionsPanel);
				
				mode = menuDownStored;
			}
			else
			{
				mode = EDIT;
				showPanel = NULL;
			}
		}
		else if (menuSelection == "upperleft")
		{
			showPoints = false;
			mode = CREATE_ENEMY;
			trackingEnemy = NULL;
			showPanel = enemySelectPanel;
		}
		else if (menuSelection == "upperright")
		{
			showPoints = false;
			justCompletedPolyWithClick = false;
			mode = CREATE_TERRAIN;
			showPanel = NULL;
		}
		else if (menuSelection == "lowerleft")
		{
			mode = CREATE_GATES;
			gatePoints = 0;
			showPanel = NULL;
			showPoints = true;
		}
		else if (menuSelection == "lowerright")
		{
			showPanel = mapOptionsPanel;
			mapOptionsPanel->textBoxes["draintime"]->text.setString(to_string(drainSeconds));
			mapOptionsPanel->textBoxes["bosstype"]->text.setString(to_string(bossType));
			mode = menuDownStored;
		}
		else if (menuSelection == "bottom")
		{
		}


		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::CreatePatrolPathModeHandleEvent()
{
	minimumPathEdgeLength = 16;
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		break;
	}
	case Event::MouseButtonReleased:
	{
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if ((ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete) && patrolPath.size() > 1)
		{
			patrolPath.pop_back();
		}
		else if (ev.key.code == Keyboard::Space)
		{
			if (selectedBrush->objects.size() == 1) //EDIT
			{
				ISelectable *select = selectedBrush->objects.front().get();
				ActorParams *actor = (ActorParams*)select;
				showPanel = actor->type->panel;
				actor->SetPath(patrolPath);
				mode = EDIT;
			}
			else
			{
				showPanel = trackingEnemy->panel;
				tempActor->SetPath(patrolPath);
				mode = CREATE_ENEMY;
			}
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::CreateRectModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				//cout << "edit mouse update" << endl;
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
				break;
			}

			if (!drawingCreateRect)
			{
				drawingCreateRect = true;
				createRectStartPoint = Vector2i(worldPos);
				createRectCurrPoint = Vector2i(worldPos);
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (drawingCreateRect)
		{
			drawingCreateRect = false;

			createRectCurrPoint = Vector2i(worldPos);

			Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
			float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
			float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
			rectCreatingTrigger->SetRect(width, height, rc);

			if (trackingEnemy != NULL)
			{
				enemySprite.setPosition(Vector2f(rc));
				enemyQuad.setPosition(enemySprite.getPosition());
			}
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::Space)
		{
			if (selectedBrush->objects.size() == 1) //EDIT
			{
				ISelectable *select = selectedBrush->objects.front().get();
				AirTriggerParams *actor = (AirTriggerParams*)select;
				showPanel = actor->type->panel;
				mode = EDIT;
			}
			else
			{
				showPanel = trackingEnemy->panel;
				mode = CREATE_ENEMY;
			}
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::SetCamZoomModeHandleEvent()
{
	//minimumPathEdgeLength = 16;

	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			//Vector2i worldi(testPoint.x, testPoint.y);
			//patrolPath.push_back(worldi);

			currentCameraShot->SetZoom(Vector2i(testPoint));

			if (tempActor != NULL)
			{
				mode = CREATE_ENEMY;
			}
			else
			{
				mode = EDIT;
			}

			showPanel = currentCameraShot->type->panel;
		}
		break;
	}
	}

}

void EditSession::SetDirectionModeHandleEvent()
{
	minimumPathEdgeLength = 16;

	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			Vector2i worldi(testPoint.x, testPoint.y);
			patrolPath.push_back(worldi);

			ActorParams *actor;
			if (tempActor != NULL)
			{
				actor = tempActor;
				mode = CREATE_ENEMY;
			}
			else
			{
				ISelectable *select = selectedBrush->objects.front().get();
				actor = (ActorParams*)select;
				mode = EDIT;
			}

			showPanel = actor->type->panel;
			actor->SetPath(patrolPath);
		}
		break;
	}
	}

}

void EditSession::CreateGatesModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			TryPlaceGatePoint(worldPos);
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::MouseMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		else if (ev.key.code == sf::Keyboard::X)
		{
			gatePoints = 0;
			//modifyGate = NULL;
		}
		break;
	}
	case Event::KeyReleased:
	{

		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
}

void EditSession::CreateImagesHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
			}
			else
			{
				EditorDecorPtr dec(new EditorDecorInfo(tempDecorSprite, currDecorLayer, currDecorName, currDecorTile));
				if (currDecorLayer > 0)
				{
					dec->myList = &decorImagesBehindTerrain;
				}
				else if (currDecorLayer < 0)
				{
					dec->myList = &decorImagesFrontTerrain;
				}
				else if (currDecorLayer == 0)
				{
					dec->myList = &decorImagesBetween;
				}
				CreateDecorImage(dec);
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(false, uiMousePos.x, uiMousePos.y);
			}
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (showPanel != NULL)
		{
			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			showPanel = decorPanel;
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		break;
	}
	}
}

void EditSession::SetLevelModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
	}
	case Event::MouseButtonReleased:
	{
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == sf::Keyboard::Num1)
		{
			setLevelCurrent = 1;
		}
		else if (ev.key.code == sf::Keyboard::Num2)
		{
			setLevelCurrent = 2;
		}
		else if (ev.key.code == sf::Keyboard::Num3)
		{
			setLevelCurrent = 3;
		}
		else if (ev.key.code == sf::Keyboard::Num4)
		{
			setLevelCurrent = 4;
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::UpdateMode()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
	{
		CreateTerrainModeUpdate();
		break;
	}
	case CREATE_RAILS:
	{
		CreateRailsModeUpdate();
		break;
	}
	case EDIT:
	{
		EditModeUpdate();
		break;
	}
	case PASTE:
	{
		PasteModeUpdate();
		break;
	}
	case CREATE_ENEMY:
	{
		CreateEnemyModeUpdate();
		break;
	}
	case CREATE_RECT:
	{
		CreateRectModeUpdate();
		break;
	}
	case SET_CAM_ZOOM:
	{
		SetCamZoomModeUpdate();
		break;
	}
	case CREATE_PATROL_PATH:
	{
		CreatePatrolPathModeUpdate();
		break;
	}
	case SET_DIRECTION:
	{
		SetDirectionModeUpdate();
		break;
	}
	case CREATE_GATES:
	{
		CreateGatesModeUpdate();
		break;
	}
	case CREATE_IMAGES:
	{
		CreateImagesModeUpdate();
		break;
	}
	case SET_LEVEL:
	{
		SetLevelModeUpdate();
		break;
	}
	}
}

void EditSession::CreateTerrainModeUpdate()
{
	if (showPanel != NULL)
		return;

	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(testPoint, graph->graphSpacing);
		showGraph = true;
	}
	else if (IsKeyPressed(Keyboard::F))
	{
		PolyPtr p;
		TerrainPoint *pPoint;
		TrySnapPosToPoint(testPoint, 8 * zoomMultiple, p, pPoint );
		showPoints = true;
	}
	else
	{
		showPoints = false;
	}

	PreventNearPrimaryAnglesOnPolygonInProgress();

	if (!justCompletedPolyWithClick)
	{
		TryAddPointToPolygonInProgress();
	}
}

void EditSession::CreateRailsModeUpdate()
{
	if (showPanel != NULL)
		return;

	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(testPoint, graph->graphSpacing);
		showGraph = true;
	}
	else if (IsKeyPressed(Keyboard::F))
	{
		PolyPtr p;
		TerrainPoint *pPoint;
		TrySnapPosToPoint(testPoint, 8 * zoomMultiple, p, pPoint );
		showPoints = true;
	}
	else
	{
		showPoints = false;
	}

	PreventNearPrimaryAnglesOnRailInProgress();

	TryAddPointToRailInProgress();
}

void EditSession::EditModeUpdate()
{
	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(worldPos, graph->graphSpacing);
		showGraph = true;
	}

	if (IsKeyPressed(Keyboard::B))
	{
		showPoints = true;
	}
	else
	{
		ClearSelectedPoints();
		showPoints = false;
	}

	TryTerrainMove();

	ModifyGrass();
}

void EditSession::PasteModeUpdate()
{
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;
	copiedBrush->Move(delta);
	editMouseGrabPos = pos;
}

void EditSession::CreateEnemyModeUpdate()
{
	MoveTrackingEnemy();
}

void EditSession::CreatePatrolPathModeUpdate()
{
	if (showPanel != NULL)
		return;

	TryAddToPatrolPath();
}

void EditSession::CreateRectModeUpdate()
{
	if (showPanel != NULL)
		return;


	if (!panning && IsMousePressed(Mouse::Left))
	{
		createRectCurrPoint = Vector2i(worldPos);

		Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
		float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
		float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
		rectCreatingTrigger->SetRect(width, height, rc);

		if (trackingEnemy != NULL)
		{
			enemySprite.setPosition(Vector2f(rc));
			enemyQuad.setPosition(enemySprite.getPosition());
		}
	}
}

void EditSession::SetCamZoomModeUpdate()
{
	if (showPanel != NULL)
		return;

	currentCameraShot->SetZoom(Vector2i(testPoint));
	/*if (!panning && IsMousePressed(Mouse::Left))
	{
		createRectCurrPoint = Vector2i(worldPos);

		Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
		float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
		float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
		rectCreatingTrigger->SetRect(width, height, rc);

		if (trackingEnemy != NULL)
		{
			enemySprite.setPosition(Vector2f(rc));
			enemyQuad.setPosition(enemySprite.getPosition());
		}
	}*/
}

void EditSession::SetDirectionModeUpdate()
{
	if (showPanel != NULL)
		return;
}

void EditSession::CreateGatesModeUpdate()
{
	PolyPtr p = NULL;
	TerrainPoint *pPoint = NULL;
	if (gatePoints == 1)
	{
		TrySnapPosToPoint(worldPos, 8 * zoomMultiple, p, pPoint );
	}
	gateInProgressTestPoly = p;
	gateInProgressTestPoint = pPoint;

	if (modifyGate != NULL)
	{
		GridSelectPop("gateselect");

		string gateResult = tempGridResult;

		if (gateResult == "delete")
		{
			Action * action = new DeleteGateAction(modifyGate);
			action->Perform();
			doneActionStack.push_back(action);

			modifyGate = NULL;
		}
		else
		{

			Action * action = new ModifyGateAction(modifyGate, gateResult);
			action->Perform();

			if (gateResult == "shard")
			{
				GridSelectPop("shardselector");

				int sw, si;
				GetShardWorldAndIndex(tempGridX, tempGridY, sw, si);
				modifyGate->SetShard(sw, si);
			}


			doneActionStack.push_back(action);
			modifyGate = NULL;
		}
		return;
	}

	//DrawGateInProgress();

	if (gatePoints > 1)
	{
		bool result = IsGateValid(&testGateInfo);//CanCreateGate(testGateInfo);

		if (result)
		{
			GridSelectPop("gateselect");
			string gateResult = tempGridResult;

			if (gateResult == "delete")
			{
			}
			else
			{

				if (gateResult == "shard")
				{
					GridSelectPop("shardselector");
					int sw, si;
					GetShardWorldAndIndex(tempGridX, tempGridY, sw, si);
					testGateInfo.SetShard(sw, si);
				}

				Vector2i adjust(0, 0);
				
				Action *action = new CreateGateAction(testGateInfo, gateResult);
				action->Perform();

				if (GetPrimaryAdjustment(testGateInfo.point0->pos, testGateInfo.point1->pos, adjust))
				{
					CompoundAction *testAction = new CompoundAction;
					testAction->subActions.push_back(action);

					if (!TryGateAdjustAction(GATEADJUST_POINT_B, &testGateInfo, adjust, testAction))
					{
						action->Undo();
						delete action;
						gatePoints = 0;
						return;
					}
					else
					{
						testAction->performed = true;
						doneActionStack.push_back(testAction);
					}
				}
				else
				{
					doneActionStack.push_back(action);
				}
			}
			gatePoints = 0;
		}
		else
		{
			gatePoints = 1;
			testGateInfo.poly1 = NULL;
			testGateInfo.point1 = NULL;
			//MessagePop("gate would intersect some terrain");
		}

		
	}
}

void EditSession::CreateImagesModeUpdate()
{
	if (showPanel == NULL)
	{
		Vector2f p = preScreenTex->mapPixelToCoords(pixelPos);
		tempDecorSprite.setPosition(p);

		tempDecorSprite.setOrigin(tempDecorSprite.getLocalBounds().width / 2, tempDecorSprite.getLocalBounds().height / 2);
		tempDecorSprite.setRotation(0);
	}
}

void EditSession::SetLevelModeUpdate()
{
	SetEnemyLevel();
}