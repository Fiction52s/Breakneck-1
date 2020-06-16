#include "Session.h"
#include "Actor.h"
#include "MainMenu.h"
#include "MapHeader.h"
#include "EditorTerrain.h"
#include "EditorDecorInfo.h"
#include "EditorRail.h"
#include "EditorActors.h"
#include "EditSession.h"
#include "GameSession.h"
#include "Background.h"
#include "HitboxManager.h"
#include "Enemy_BasicEffect.h"
#include "GateMarker.h"
#include "KeyMarker.h"
#include "Gate.h"
#include "Minimap.h"
#include "EnvPlant.h"
#include "HUD.h"
#include "Fader.h"

//#include "Enemy_Shard.h"


//enemy stuff:
#include "SoundManager.h"




#include "ActorParams.h"

#include "EnemiesW1.h"



using namespace sf;
using namespace std;


template <typename X>ActorParams * MakeParams(ActorType *at, int level)
{
	return new X(at, level);
}


template<typename X> ActorParams *LoadParams(
	ActorType *at, std::ifstream &is)
{
	ActorParams *ap = new X(at, is);
	ap->CreateMyEnemy();
	return ap;
}

template<typename X> Enemy *CreateEnemy(ActorParams* ap)
{
	return new X(ap);
}

template <typename X> void SetParamsType(ParamsInfo *pi)
{
	pi->pMaker = MakeParams<X>;
	pi->pLoader = LoadParams<X>;
	/*pi->pCanBeGrounded = CanBeGrounded<X>;
	pi->pCanBeAerial = CanBeAerial<X>;
	pi->pCanBeRailGrounded = CanBeRailGrounded<X>;*/
}

void Session::SetupEnemyTypes()
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

void Session::SetupTimeBubbles()
{
	if (fBubbleFrame != NULL)
		return;

	if (parentGame != NULL)
	{
		fBubbleFrame = parentGame->fBubbleFrame;
		fBubblePos = parentGame->fBubblePos;
		fBubbleRadiusSize = parentGame->fBubbleRadiusSize;
		return;
	}

	int numBubbleInfo = Actor::MAX_BUBBLES * MAX_PLAYERS;
	fBubbleFrame = new float[numBubbleInfo];
	for (int i = 0; i < numBubbleInfo; ++i)
	{
		fBubbleFrame[i] = 0;
	}
	fBubblePos = new sf::Vector2f[numBubbleInfo];
	fBubbleRadiusSize = new float[numBubbleInfo];

	//int count = 0;
	Actor *tempPlayer = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (tempPlayer = GetPlayer(i))
		{
			tempPlayer->fBubbleFrame = (fBubbleFrame + i * Actor::MAX_BUBBLES);
			tempPlayer->fBubblePos = (fBubblePos + i * Actor::MAX_BUBBLES);
			tempPlayer->fBubbleRadiusSize = (fBubbleRadiusSize + i * Actor::MAX_BUBBLES);
			//++count;
		}
	}
}

void Session::SetupSoundLists()
{
	if (parentGame != NULL)
	{
		soundNodeList = parentGame->soundNodeList;
		pauseSoundNodeList = parentGame->pauseSoundNodeList;

		soundNodeList->Clear();
		pauseSoundNodeList->Clear();
	}
	else if (soundNodeList == NULL)
	{
		soundNodeList = new SoundNodeList(10);
		pauseSoundNodeList = new SoundNodeList(10);
	}
	else
	{
		soundNodeList->Clear();
		pauseSoundNodeList->Clear();
	}
}

void Session::SetupSoundManager()
{
	if (parentGame != NULL)
	{
		soundManager = parentGame->soundManager;
	}
	else if (soundManager == NULL)
	{
		soundManager = new SoundManager;
	}
}

void Session::SetParentGame(GameSession *game)
{
	parentGame = game;
	SetParentTilesetManager(game);
}

void Session::SetPlayerOptionField(int pIndex)
{
	GetPlayer(0)->SetAllUpgrades(playerOptionsField);
}

void Session::SetupHitboxManager()
{
	if (parentGame != NULL)
	{
		hitboxManager = parentGame->hitboxManager;
	}
	else if (hitboxManager == NULL)
	{
		hitboxManager = new HitboxManager;
	}
		

	//dont clear this because it will probably contain all the player hitboxes
	//else
	//hitboxManager->
}

void Session::SetupEnemyType(ParamsInfo &pi)
{
	if (types[pi.name] == NULL)
	{
		types[pi.name] = new ActorType(pi);
	}
}

void Session::AddGeneralEnemies()
{
	AddExtraEnemy("zoneproperties", NULL, SetParamsType<ZonePropertiesParams>, Vector2i(0, 0),
		Vector2i(128, 128), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Editor/zoneproperties_128x128.png"));

	AddExtraEnemy("shard", CreateEnemy<Shard>, SetParamsType<ShardParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1 );

	AddExtraEnemy("healthfly", CreateEnemy<FlyChain>, SetParamsType<FlyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	AddExtraEnemy("camerashot", NULL, SetParamsType<CameraShotParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddExtraEnemy("key", NULL, SetParamsType<KeyParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false, 1, GetSizedTileset("Enemies/bouncefloater_128x128.png"));

	AddExtraEnemy("poi", NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
	false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/rail_32x32.png"), 1);

	AddExtraEnemy("xbarrier", NULL, SetParamsType<XBarrierParams>,Vector2i(0, 0), Vector2i(64, 64),false, false, false, false, true, false, false, 1,
		GetSizedTileset("Enemies/blocker_w1_192x192.png"));

	AddExtraEnemy("shippickup", NULL, SetParamsType<ShipPickupParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false, 1,
		GetSizedTileset("Ship/shipleave_128x128.png"));

	AddExtraEnemy("nexus", NULL, SetParamsType<NexusParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false );

	AddExtraEnemy("groundtrigger", NULL, SetParamsType<GroundTriggerParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, true, 1,
		GetSizedTileset("Ship/shipleave_128x128.png" ));

	AddExtraEnemy("airtrigger", NULL, SetParamsType<AirTriggerParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Enemies/jayshield_128x128.png"));

	AddExtraEnemy("ship", NULL, SetParamsType<BasicAirEnemyParams>, Vector2i(0, 0), Vector2i(864, 400), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Ship/ship_864x400.png"));

	
	//ignore flowerpods until I figure out how I want to change them.

	/*AddExtraEnemy("flowerpod", CreateEnemy<FlowerPod>, SetParamsType<FlowerPodParams>,
	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false, 1 );*/


	/*AddExtraEnemy("flowerpod", NULL, SetParamsType<FlowerPodParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Momenta/momentaflower_128x128.png", 128, 128);*/

	/*AddExtraEnemy("flowerpod", CreateEnemy<FlowerPod>, SetParamsType<FlowerPodParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false, 1 );*/

	/*AddExtraEnemy("poi", NULL, LoadParams<PoiParams>, MakeParamsGrounded<PoiParams>, MakeParamsAerial<PoiParams>,
		Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false);

	AddExtraEnemy("xbarrier", NULL, LoadParams<XBarrierParams>, NULL, MakeParamsAerial<XBarrierParams>,
		Vector2i(0, 0), Vector2i(64, 64),
		false, false, false, false, 1,
		GetTileset("Enemies/blocker_w1_192x192.png", 192, 192));

	AddExtraEnemy("camerashot", NULL, LoadParams<CameraShotParams>, NULL, MakeParamsAerial<CameraShotParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddExtraEnemy("key", NULL, LoadParams<KeyParams>, NULL, MakeParamsAerial<KeyParams>,
		Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false);

	AddExtraEnemy("shippickup", NULL, LoadParams<ShipPickupParams>, MakeParamsGrounded<ShipPickupParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Ship/shipleave_128x128.png", 128, 128));

	AddExtraEnemy("shard", NULL, LoadParams<ShardParams>, NULL, MakeParamsAerial<ShardParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 3,
		GetTileset("Shard/shards_w1_192x192.png", 192, 192));

	AddExtraEnemy("ship", NULL, LoadParams<BasicAirEnemyParams>, NULL, MakeParamsAerial<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(864, 400), false, false, false, false, 1,
		GetTileset("Ship/ship_864x400.png", 864, 400));

	AddExtraEnemy("healthfly", NULL, LoadParams<BasicAirEnemyParams>, NULL, MakeParamsAerial<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1,
		GetTileset("Enemies/healthfly_64x64.png", 64, 64));

	AddExtraEnemy("extrascene", NULL, LoadParams<ExtraSceneParams>, NULL, MakeParamsAerial<ExtraSceneParams>,
		Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, 1,
		GetTileset("Enemies/bouncefloater_128x128.png", 128, 128));

	AddExtraEnemy("racefighttarget", NULL, LoadParams<RaceFightTargetParams>, NULL, MakeParamsAerial<RaceFightTargetParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddExtraEnemy("groundtrigger", NULL, LoadParams<GroundTriggerParams>, MakeParamsGrounded<GroundTriggerParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Ship/shipleave_128x128.png", 128, 128));

	AddExtraEnemy("airtrigger", NULL, LoadParams<AirTriggerParams>, NULL, MakeParamsAerial<AirTriggerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	AddExtraEnemy("flowerpod", NULL, LoadParams<FlowerPodParams>, MakeParamsGrounded<FlowerPodParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Momenta/momentaflower_128x128.png", 128, 128));

	AddExtraEnemy("nexus", NULL, LoadParams<NexusParams>, MakeParamsGrounded<NexusParams>, NULL,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/
}

void Session::AddW1Enemies()
{
	AddBasicGroundWorldEnemy("goal", 1, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddBasicGroundWorldEnemy("crawler", 1, CreateEnemy<Crawler>, Vector2i(0, 0), Vector2i(100, 100), true, true, false, false, 3);
	
	AddBasicAerialWorldEnemy("booster", 1, CreateEnemy<Booster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3 );
		//GetTileset("Goal/goal_w01_a_288x320.png", 288, 320));

	AddWorldEnemy("blocker", 1, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	AddBasicAerialWorldEnemy("patroller", 1, CreateEnemy<Patroller>, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3 );

	

	//AddBasicAerialWorldEnemy("airdasher", 1, CreateEnemy<Airdasher>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3 );

	AddBasicGroundWorldEnemy("shroom", 1, CreateEnemy<Shroom>, Vector2i(0, 0), Vector2i(50, 50), true, true, false, false, 3);

	AddBasicGroundWorldEnemy("basicturret", 1, CreateEnemy<BasicTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3 );

	AddBasicAerialWorldEnemy("splitcomboer", 1, CreateEnemy<SplitComboer>, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3 );
	
	AddBasicAerialWorldEnemy("comboer", 1, CreateEnemy<Comboer>, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3 );

	AddWorldEnemy("spring", 1, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false, 1 );

	/*AddWorldEnemy("shard", 1, CreateEnemy<Shard>, SetParamsType<ShardParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);*/

	/*

	/*AddBasicAerialWorldEnemy("jugglercatcher", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	GetTileset("Enemies/jugglercatcher_128x128.png", 128, 128));*/

	/*AddBasicGroundWorldEnemy("shroom", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/shroom_192x192.png", 192, 192));

	AddBasicGroundWorldEnemy("basicturret", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/basicturret_128x80.png", 128, 80));

	AddBasicAerialWorldEnemy("airdasher", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/dasher_208x144.png", 208, 144));

	AddWorldEnemy("bosscrawler", 1, NULL, LoadParams<BossCrawlerParams>, MakeParamsGrounded<BossCrawlerParams>, NULL,
		Vector2i(0, 0), Vector2i(128, 144), false, false, false, false, 1,
		GetTileset("Bosses/Crawler/crawler_queen_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("booster", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
		GetTileset("Enemies/Booster_512x512.png", 512, 512));

	AddWorldEnemy("spring", 1, NULL, LoadParams<SpringParams>, NULL, MakeParamsAerial<SpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_256x256.png", 256, 256));*/
}

void Session::AddW2Enemies()
{
	//AddBasicGroundWorldEnemy("greengoal", 2, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Goal/goal_w02_a_288x256.png", 288, 256));

	//AddWorldEnemy("greenblocker", 2, LoadParams<BlockerParams>, NULL, MakeParamsAerial<BlockerParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/blocker_w2_192x192.png", 192, 192));

	//AddWorldEnemy("downgravityjuggler", 2, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	//AddWorldEnemy("upgravityjuggler", 2, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/jayshield_128x128.png", 128, 128));


	//AddBasicAerialWorldEnemy("airdashjuggler", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128));

	///*AddBasicAerialWorldEnemy("gravdowncomboer", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	//GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	///*AddBasicAerialWorldEnemy("gravupcomboer", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	//GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	//AddBasicAerialWorldEnemy("bat", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	//	GetTileset("Enemies/bat_144x176.png", 144, 176));

	//AddBasicGroundWorldEnemy("curveturret", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/curveturret_144x96.png", 144, 96));

	//AddBasicGroundWorldEnemy("poisonfrog", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/frog_80x80.png", 80, 80));

	//AddBasicGroundWorldEnemy("stagbeetle", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/stag_idle_256x176.png", 256, 176));

	//AddBasicGroundWorldEnemy("gravityfaller", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/gravity_faller_128x128.png", 128, 128));

	//AddBasicAerialWorldEnemy("gravityincreaser", 2, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/grav_increase_256x256.png", 256, 256));

	//AddBasicAerialWorldEnemy("gravitydecreaser", 2, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/grav_decrease_256x256.png", 256, 256));

	//AddWorldEnemy("gravityspring", 2, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddWorldEnemy("bossbird", 2, LoadParams<BossBirdParams>, NULL, MakeParamsAerial<BossBirdParams>,
	//	Vector2i(0, 0), Vector2i(64, 64), false, false, false, false);
}

void Session::AddW3Enemies()
{
	//AddBasicAerialWorldEnemy("bouncefloater", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/bouncefloater_128x128.png", 128, 128));

	///*AddBasicAerialWorldEnemy("bouncecomboer", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	//GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	//AddWorldEnemy("bouncejuggler", 3, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/jayshield_128x128.png", 128, 128));

	//AddWorldEnemy("bouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddWorldEnemy("airbouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	//AddBasicAerialWorldEnemy("upbouncebooster", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/Booster_512x512.png", 512, 512));

	//AddBasicAerialWorldEnemy("omnibouncebooster", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/Booster_512x512.png", 512, 512));
	///*AddWorldEnemy("redirectspring", 3, LoadParams<BounceSpringParams>, NULL, MakeParamsAerial<BounceSpringParams>,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddWorldEnemy("reflectspring", 3, LoadParams<BounceSpringParams>, NULL, MakeParamsAerial<BounceSpringParams>,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));*/

	////AddWorldEnemy("bouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	////	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddBasicAerialWorldEnemy("pulser", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3);

	//AddBasicGroundWorldEnemy("badger", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/badger_192x128.png", 192, 128));

	//AddBasicGroundWorldEnemy("roadrunner", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/roadrunner_256x256.png", 256, 256));

	//AddBasicAerialWorldEnemy("owl", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);


	//AddBasicGroundWorldEnemy("cactus", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	///*AddWorldEnemy("cactus", 3, LoadParams<CactusParams>, MakeParamsGrounded<CactusParams>, NULL,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/

	//AddWorldEnemy("bosscoyote", 3, LoadParams<BossCoyoteParams>, NULL, MakeParamsAerial<BossCoyoteParams>,
	//	Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
}

void Session::AddW4Enemies()
{
	//AddWorldEnemy("rail", 4, LoadParams<RailParams>, NULL, MakeParamsAerial<RailParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false,
	//	3, GetTileset("Enemies/rail_64x64.png", 64, 64));

	//AddWorldEnemy("grindrail", 4, LoadParams<RailParams>, NULL, MakeParamsAerial<RailParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false,
	//	3, GetTileset("Enemies/rail_64x64.png", 64, 64));

	////AddBasicAerialWorldEnemy("teleporter", 4, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	////	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddWorldEnemy("teleporter", 4, LoadParams<TeleporterParams>, NULL, MakeParamsAerial<TeleporterParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	//AddWorldEnemy("onewayteleporter", 4, LoadParams<TeleporterParams>, NULL, MakeParamsAerial<TeleporterParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	//AddWorldEnemy("grindjugglercw", 4, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128));

	//AddWorldEnemy("grindjugglerccw", 4, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128));

	//AddWorldEnemy("groundedgrindjugglercw", 4, LoadParams<GroundedJugglerParams>, MakeParamsGrounded<GroundedJugglerParams>, NULL,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	//AddWorldEnemy("groundedgrindjugglerccw", 4, LoadParams<GroundedJugglerParams>, MakeParamsGrounded<GroundedJugglerParams>, NULL,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	//AddBasicRailWorldEnemy("railtest", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/shroom_192x192.png", 192, 192));

	//AddBasicGroundWorldEnemy("spider", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/crawler_160x160.png", 160, 160));
	////AddWorldEnemy("spider", 4, LoadParams<SpiderParams>, MakeParamsGrounded<SpiderParams>, NULL,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddBasicAerialWorldEnemy("turtle", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/turtle_80x64.png", 80, 64));

	//AddBasicGroundWorldEnemy("cheetah", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/shroom_192x192.png", 192, 192));

	////AddWorldEnemy("cheetah", 4, LoadParams<CheetahParams>, MakeParamsGrounded<CheetahParams>, NULL,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddWorldEnemy("coral", 4, LoadParams<CoralParams>, NULL, MakeParamsAerial<CoralParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddWorldEnemy("bosstiger", 4, LoadParams<BossTigerParams>, NULL, MakeParamsAerial<BossTigerParams>,
	//	Vector2i(0, 0), Vector2i(64, 64), false, false, false, false);


}

void Session::AddW5Enemies()
{
	//AddWorldEnemy("hungrycomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128));

	//AddWorldEnemy("hungryreturncomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 2);

	//AddWorldEnemy("relativecomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	//AddWorldEnemy("relativecomboerdetach", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	//AddBasicAerialWorldEnemy("swarm", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/swarm_pod_128x128.png", 128, 128));

	////AddWorldEnemy("swarm", 5, LoadParams<SwarmParams>, NULL, MakeParamsAerial<SwarmParams>,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddBasicAerialWorldEnemy("shark", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/shark_circle_256x256.png", 256, 256));

	///*AddWorldEnemy("shark", 5, LoadParams<SharkParams>, NULL, MakeParamsAerial<SharkParams>,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/


	//AddBasicGroundWorldEnemy("growingtree", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/sprout_160x160.png", 160, 160));

	////AddWorldEnemy("overgrowth", 5, LoadParams<OvergrowthParams>, MakeParamsGrounded<OvergrowthParams>, NULL,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddBasicAerialWorldEnemy("ghost", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/plasmid_192x192.png", 192, 192));

	////AddWorldEnemy("ghost", 5, LoadParams<GhostParams>, NULL, MakeParamsAerial<GhostParams>,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddWorldEnemy("bossgator", 5, LoadParams<BossGatorParams>, NULL, MakeParamsAerial<BossGatorParams>,
	//	Vector2i(0, 0), Vector2i(32, 128), false, false, false, false);
}

void Session::AddW6Enemies()
{
	//w6

	/*AddWorldEnemy("swinglaunchercw", 6, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));
	AddWorldEnemy("swinglauncherccw", 6, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
		GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("wiretarget", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddWorldEnemy("wirejuggler", 6, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	AddBasicAerialWorldEnemy("specter", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/specter_256x256.png", 256, 256));

	AddWorldEnemy("narwhal", 6, LoadParams<NarwhalParams>, NULL, MakeParamsAerial<NarwhalParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddWorldEnemy("copycat", 6, LoadParams<CopycatParams>, NULL, MakeParamsAerial<CopycatParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddBasicAerialWorldEnemy("gorilla", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
		GetTileset("Enemies/gorilla_320x256.png", 320, 256));

	AddWorldEnemy("bossskeleton", 6, LoadParams<BossSkeletonParams>, NULL, MakeParamsAerial<BossSkeletonParams>,
		Vector2i(0, 0), Vector2i(32, 128), false, false, false, false);*/
}

void Session::AddBasicGroundWorldEnemy(const std::string &name, int w, EnemyCreator *p_enemyCreator,
	Vector2i &off, Vector2i &size, bool w_mon,
	bool w_level, bool w_path, bool w_loop, int p_numLevels, Tileset *ts, int tileIndex)
{
	AddWorldEnemy(name, w, p_enemyCreator, SetParamsType<BasicGroundEnemyParams>, off, size,
		w_mon, w_level, w_path, w_loop, false, true, false,  p_numLevels, ts, tileIndex);
}

void Session::AddBasicRailWorldEnemy(
	const std::string &name, 
	int w, 
	EnemyCreator *p_enemyCreator,
	Vector2i &off, 
	Vector2i &size, 
	bool w_mon,
	bool w_level, 
	bool w_path, 
	bool w_loop, 
	int p_numLevels, 
	Tileset *ts, 
	int tileIndex)
{
	AddWorldEnemy(name, w, p_enemyCreator, SetParamsType<BasicRailEnemyParams>, off, size,
		w_mon, w_level, w_path, w_loop, false, false, true, p_numLevels, ts, tileIndex);
}

void Session::AddBasicAerialWorldEnemy(
	const std::string &name, 
	int w,
	EnemyCreator *p_enemyCreator,
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
	AddWorldEnemy(name, w, p_enemyCreator, SetParamsType<BasicAirEnemyParams>, off, size, w_mon, w_level,
		w_path, w_loop, true, false, false, p_numLevels, ts, tileIndex);
}

void Session::AddWorldEnemy(const std::string &name,
	int w,
	EnemyCreator *p_enemyCreator,
	ParamsCreator *p_paramsCreator,
	sf::Vector2i &off,
	sf::Vector2i &size,
	bool w_mon,
	bool w_level,
	bool w_path,
	bool w_loop,
	bool p_canBeAerial,
	bool p_canBeGrounded,
	bool p_canBeRailGrounded,
	int p_numLevels,
	Tileset *ts,
	int tileIndex)
{
	auto & names = worldEnemyNames[w - 1];
	for (auto it = names.begin(); it != names.end(); ++it)
	{
		if ((*it).name == name)
		{
			return; //already added
		}
	}

	worldEnemyNames[w - 1].push_back(ParamsInfo(name, p_enemyCreator, p_paramsCreator, off, size,
		w_mon, w_level, w_path, w_loop, p_canBeAerial, p_canBeGrounded,
		p_canBeRailGrounded, p_numLevels, w, ts, tileIndex));
}

void Session::AddExtraEnemy(const std::string &name, 
	EnemyCreator *p_enemyCreator,
	ParamsCreator *p_paramsCreator,
	Vector2i &off, 
	Vector2i &size, 
	bool w_mon,
	bool w_level, 
	bool w_path, 
	bool w_loop, 
	bool p_canBeAerial,
	bool p_canBeGrounded,
	bool p_canBeRailGrounded,
	int p_numLevels, 
	Tileset *ts, 
	int tileIndex)
{
	for (auto it = extraEnemyNames.begin(); it != extraEnemyNames.end(); ++it)
	{
		if ((*it).name == name)
		{
			return; //already added
		}
	}


	extraEnemyNames.push_back(ParamsInfo(name, p_enemyCreator, p_paramsCreator, off, size,
		w_mon, w_level, w_path, w_loop, p_canBeAerial, p_canBeGrounded,
		p_canBeRailGrounded, p_numLevels, 0, ts, tileIndex));
}



int Session::GetPauseFrames()
{
	return pauseFrames;
}

BasicEffect * Session::ActivateEffect(EffectLayer layer, Tileset *ts, V2d pos, bool pauseImmune,
	double angle, int frameCount, int animationFactor, bool right, int startFrame, float depth)
{
	if (inactiveEffects == NULL)
	{
		return NULL;
	}
	else
	{
		BasicEffect *b = inactiveEffects;

		if (inactiveEffects->next == NULL)
		{
			inactiveEffects = NULL;
		}
		else
		{
			inactiveEffects = (BasicEffect*)(inactiveEffects->next);
			inactiveEffects->prev = NULL;
		}

		//assert( ts != NULL );
		b->startFrame = startFrame;
		b->Init(ts, pos, angle, frameCount, animationFactor, right, depth);
		b->prev = NULL;
		b->next = NULL;
		b->pauseImmune = pauseImmune;
		b->layer = layer;


		Enemy *& fxList = effectListVec[layer];
		if (fxList != NULL)
		{
			fxList->prev = b;
			b->next = fxList;
			fxList = b;
		}
		else
		{
			fxList = b;
		}
		return b;
	}
}

void Session::DeactivateEffect(BasicEffect *b)
{
	Enemy *& fxList = effectListVec[b->layer];
	assert(fxList != NULL);
	Enemy *prev = b->prev;
	Enemy *next = b->next;

	if (prev == NULL && next == NULL)
	{
		fxList = NULL;
	}
	else
	{
		if (b == fxList)
		{
			assert(next != NULL);

			next->prev = NULL;

			fxList = next;
		}
		else
		{
			if (prev != NULL)
			{
				prev->next = next;
			}

			if (next != NULL)
			{
				next->prev = prev;
			}
		}

	}

	if (inactiveEffects == NULL)
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

void Session::DrawEffects(EffectLayer layer, sf::RenderTarget *target)
{
	sf::View oldView = target->getView();
	if (layer == UI_FRONT)
	{
		target->setView(uiView);
	}
	Enemy *currentEffect = effectListVec[layer];
	while (currentEffect != NULL)
	{
		currentEffect->Draw(target);
		currentEffect = currentEffect->next;
	}

	if (layer == UI_FRONT)
	{
		target->setView(oldView);
	}
}


sf::SoundBuffer *Session::GetSound(const std::string &name)
{
	return soundManager->GetSound(name);
}

SoundNode *Session::ActivateSoundAtPos(V2d &pos, SoundBuffer *buffer, bool loop)
{
	sf::Rect<double> soundRect = screenRect;
	double soundExtra = 300;
	soundRect.left -= soundExtra;
	soundRect.width += 2 * soundExtra;
	soundRect.top -= soundExtra;
	soundRect.height += 2 * soundExtra;

	if (soundRect.contains(pos))
	{
		return soundNodeList->ActivateSound(buffer, loop);
	}
	else
	{
		return NULL;
	}
}

SoundNode *Session::ActivateSound(sf::SoundBuffer *buffer, bool loop)
{
	return soundNodeList->ActivateSound(buffer, loop);
}

SoundNode *Session::ActivatePauseSound(sf::SoundBuffer *buffer, bool loop)
{
	return pauseSoundNodeList->ActivateSound(buffer, loop);
}

void Session::AllocateEffects()
{
	effectListVec.resize(EffectLayer::Count);

	allEffectVec.resize(MAX_EFFECTS);
	BasicEffect *b;
	for (int i = 0; i < MAX_EFFECTS; ++i)
	{
		b = &allEffectVec[i];
		if (i == 0)
		{
			inactiveEffects = b;
		}
		else
		{
			b->next = inactiveEffects;
			inactiveEffects->prev = b;
			inactiveEffects = b;
		}
	}
}

void Session::ClearEffects()
{
	for (int i = 0; i < EffectLayer::Count; ++i)
	{
		Enemy *curr = effectListVec[i];
		while (curr != NULL)
		{
			Enemy *next = curr->next;
			assert(curr->type == EnemyType::EN_BASICEFFECT);
			DeactivateEffect((BasicEffect*)curr);

			curr = next;
		}
		effectListVec[i] = NULL;
	}
}

void Session::UpdateEffects( bool pauseImmuneOnly )
{
	Enemy *curr;
	Enemy *next;

	BasicEffect *currEffect;
	for (int i = 0; i < EffectLayer::Count; ++i)
	{
		curr = NULL;
		if (!effectListVec.empty())
		{
			curr = effectListVec[i];
		}

		while (curr != NULL)
		{
			next = curr->next;

			if (pauseImmuneOnly)
			{
				currEffect = (BasicEffect*)curr;
				if (currEffect->pauseImmune)
				{
					curr->UpdatePrePhysics();
					if (!curr->dead)
						curr->UpdatePostPhysics();
				}
			}
			else
			{
				curr->UpdatePrePhysics();
				if (!curr->dead)
					curr->UpdatePostPhysics();
			}

			curr = next;
		}
	}
}


bool Session::IsSessTypeGame()
{
	return sessType == SESS_GAME;
}

bool Session::IsSessTypeEdit()
{
	return sessType == SESS_EDIT;
}

void Session::Pause(int frames)
{
	pauseFrames = frames;
	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = players[i];
		if (p != NULL)
		{
			p->ClearPauseBufferedActions();
		}
	}
}

void Session::PlayerConfirmEnemyNoKill(Enemy *en, int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->ConfirmEnemyNoKill(en);
	}
}

void Session::PlayerConfirmEnemyKill(Enemy *en, int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->ConfirmEnemyKill(en);
	}
}

V2d Session::GetPlayerKnockbackDirFromVel(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		return p->GetKnockbackDirFromVel();
	}
	else
	{
		return V2d();
	}
}

V2d Session::GetPlayerTrueVel(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		return p->GetTrueVel();
	}
	else
	{
		return V2d();
	}
}

V2d Session::GetPlayerPos(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->position;
	}
	else
	{
		return V2d();
	}
}

void Session::PlayerApplyHit(HitboxInfo *hi, int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->ApplyHit(hi);
	}
}

void Session::PlayerAddActiveComboObj(ComboObject *co, int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->AddActiveComboObj(co);
	}
}

void Session::PlayerRemoveActiveComboer(ComboObject *co, int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->RemoveActiveComboObj(co);
	}
}

void Session::AddEnemy(Enemy *e)
{
	//do not spawn shards that are already captured in the file.
	

	//cout << "spawning enemy! of type: " << e->type << endl;
	if (e->spawned)
	{
		assert(e->spawned == false);
	}

	e->Reset();
	e->spawned = true;
	e->Init();

	//^^note remove this later
	//debugging only

	//debug test
	Enemy *c = activeEnemyList;
	while (c != NULL)
	{
		assert(c != e);
		c = c->next;
	}

	if (activeEnemyList == NULL)
	{
		activeEnemyList = e;
		activeEnemyListTail = e;
	}
	else
	{
		activeEnemyListTail->next = e;
		e->prev = activeEnemyListTail;
		activeEnemyListTail = e;
	}
}

void Session::RemoveEnemy(Enemy *e)
{
	if (activeEnemyList == NULL)
	{
		return;
	}

	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if (prev == NULL && next == NULL)
	{
		activeEnemyList = NULL;
		activeEnemyListTail = NULL;
	}
	else
	{
		if (e == activeEnemyListTail)
		{
			assert(prev != NULL);

			prev->next = NULL;

			activeEnemyListTail = prev;
		}
		else if (e == activeEnemyList)
		{
			assert(next != NULL);

			next->prev = NULL;
			activeEnemyList = next;
		}
		else
		{
			if (next != NULL)
			{
				next->prev = prev;
			}

			if (prev != NULL)
			{
				prev->next = next;
			}
		}
	}
}

//-------------------
void Session::KillAllEnemies()
{
	Enemy *curr = activeEnemyList;
	while (curr != NULL)
	{
		Enemy *next = curr->next;

		if (curr->type != EnemyType::EN_GOAL && curr->type != EnemyType::EN_NEXUS)
		{
			curr->DirectKill();
			//curr->health = 0;
		}
		curr = next;
	}
}

void Session::PlayerHitNexus(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->hitNexus = true;
	}
}

void Session::PlayerHitGoal(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->hitGoal = true;
	}
}

Session *Session::GetSession()
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
		return edit;

	GameSession *game = GameSession::GetSession();
	if (game != NULL)
		return game;
	//add GameSession in here eventually

	return NULL;
}

//bool Session::IsSessionTypeEdit()
//{
//}
//
//bool Session::IsSessionTypeGame()
//{
//	GameSession *gs = GameSession::GetSession();
//	return gs != NULL;
//}

void Session::CreateBulletQuads()
{
	if (totalNumberBullets > 0)
	{
		bigBulletVA = new Vertex[totalNumberBullets * 4];
		for (int i = 0; i < totalNumberBullets * 4; ++i)
		{
			bigBulletVA[i].position = Vector2f(0, 0);
		}
		ts_basicBullets = GetTileset("Enemies/bullet_64x64.png", 64, 64);
	}
	else
	{
		
	}
}

void Session::DrawBullets(sf::RenderTarget *target)
{
	if (ts_basicBullets != NULL)
	{
		target->draw(bigBulletVA, totalNumberBullets * 4, sf::Quads, ts_basicBullets->texture);
	}
}

Session::Session( SessionType p_sessType, const boost::filesystem::path &p_filePath)
	:playerOptionsField(PLAYER_OPTION_BIT_COUNT)
{
	mainMenu = MainMenu::GetInstance();
	preScreenTex = MainMenu::preScreenTexture;
	minimapTex = MainMenu::minimapTexture;
	postProcessTex2 = MainMenu::postProcessTexture2;
	fader = mainMenu->fader;
	swiper = mainMenu->swiper;

	numGates = 0;

	adventureHUD = NULL;

	zoneTree = NULL;
	currentZoneNode = NULL;
	zoneTreeStart = NULL;
	zoneTreeEnd = NULL;
	currentZone = NULL;
	originalZone = NULL;
	activatedZoneList = NULL;

	parentGame = NULL;

	sessType = p_sessType;
	cutPlayerInput = false;
	
	assert(mainMenu != NULL);

	window = mainMenu->window;

	filePath = p_filePath;
	filePathStr = filePath.string();

	players.resize(MAX_PLAYERS);

	soundManager = NULL;
	soundNodeList = NULL;
	pauseSoundNodeList = NULL;
	
	flyTerrainTree = NULL;
	terrainTree = NULL;
	specialTerrainTree = NULL;
	railEdgeTree = NULL;
	barrierTree = NULL;
	borderTree = NULL;
	grassTree = NULL;
	activeItemTree = NULL;
	gateTree = NULL;

	staticItemTree = NULL;

	polyShaders = NULL;
	background = NULL;
	hitboxManager = NULL;
	inactiveEffects = NULL;

	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	inactiveEnemyList = NULL;

	ts_basicBullets = NULL;
	bigBulletVA = NULL;

	uiView = View(sf::Vector2f(960, 540), sf::Vector2f(1920, 1080));

	totalNumberBullets = 0;
	keyFrame = 0;

	mapHeader = NULL;

	showDebugDraw = false;
}



Session::~Session()
{
	//new stuff
	if ( parentGame == NULL && soundManager != NULL)
	{
		delete soundManager;
		soundManager = NULL;
	}

	if (parentGame == NULL && soundNodeList != NULL)
	{
		delete soundNodeList;
		soundNodeList = NULL;
	}

	if (parentGame == NULL && pauseSoundNodeList != NULL)
	{
		delete pauseSoundNodeList;
		pauseSoundNodeList = NULL;
	}

	if (bigBulletVA != NULL)
	{
		delete [] bigBulletVA;
	}


	if (parentGame == NULL && adventureHUD != NULL)
	{
		delete adventureHUD;
		adventureHUD = NULL;
	}
	/*if (parentGame == NULL && mini != NULL)
	{
		delete mini;
		mini = NULL;
	}*/

	//---------------

	if (parentGame == NULL)
	{
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			delete players[i];
			players[i] = NULL;
		}
	}
	

	if (terrainTree != NULL)
	{
		delete terrainTree;
		terrainTree = NULL;
	}
		

	if (specialTerrainTree != NULL)
	{
		delete specialTerrainTree;
		specialTerrainTree = NULL;
	}
		

	if (railEdgeTree != NULL)
	{
		delete railEdgeTree;
		railEdgeTree = NULL;
	}
	
	if (flyTerrainTree != NULL)
	{
		delete flyTerrainTree;
		flyTerrainTree = NULL;
	}

	if (borderTree != NULL)
	{
		delete borderTree;
		borderTree = NULL;
	}
		
	if (barrierTree != NULL)
	{
		delete barrierTree;
		barrierTree = NULL;
	}

	if (staticItemTree != NULL)
	{
		delete staticItemTree;
		staticItemTree = NULL;
	}

	if (gateTree != NULL)
	{
		delete gateTree;
		gateTree = NULL;
	}
		

	if (grassTree != NULL)
	{
		delete grassTree;
		grassTree = NULL;
	}
		

	if (activeItemTree != NULL)
	{
		delete activeItemTree;
		activeItemTree = NULL;
	}


	if (mapHeader != NULL)
	{
		delete mapHeader;
		mapHeader = NULL;
	}
		

	if (polyShaders != NULL)
	{
		delete[] polyShaders;
		polyShaders = NULL;
	}
		

	for (auto it = terrainDecorInfoMap.begin(); it != terrainDecorInfoMap.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}

	if (background != NULL)
	{
		delete background;
		background = NULL;
	}	

	if ( parentGame == NULL && hitboxManager != NULL)
	{
		delete hitboxManager;
		hitboxManager = NULL;
	}
		

	for (auto it = types.begin(); it != types.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete(*it).second;
	}

	CleanupZones();
	CleanupGates();
}

void Session::UpdateDecorLayers()
{
	for (auto mit = decorLayerMap.begin(); mit != decorLayerMap.end();
		++mit)
	{
		(*mit).second->Update();
	}
}

void Session::UpdateDecorSprites()
{
	//update relevant terrain pieces, need to do a check for camera etc.
}

void Session::SetPlayersGameMode()
{
	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->SetGameMode();
	}
}

void Session::DebugDraw(sf::RenderTarget *target)
{
	if (showDebugDraw)
	{
		DebugDrawActors(target);
	}
}

void Session::DebugDrawActors(sf::RenderTarget *target)
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DebugDraw(target);
		}
	}

	Enemy *currEnemy = activeEnemyList;
	while (currEnemy != NULL)
	{
		currEnemy->DebugDraw(target);
		currEnemy = currEnemy->next;
	}
}

void Session::AllocatePolyShaders(int numPolyTypes)
{
	if (polyShaders != NULL)
	{
		delete[] polyShaders;
		polyShaders = NULL;
	}

	numPolyShaders = numPolyTypes;
	polyShaders = new Shader[numPolyShaders];
	//polyShaders.resize(numPolyTypes);
	ts_polyShaders.resize(numPolyTypes);
}

void Session::DrawPlayerWires( RenderTarget *target )
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawWires(target);
		}
	}
}

void Session::SetPlayerInputOn(bool on )
{
	cutPlayerInput = !on;
	//if (!cutPlayerInput)
	//{
	//	//GetPrevInput(0) = ControllerState();
	//	//GetCurrInput(0) = ControllerState()
	//}
}

void Session::DrawPlayers(sf::RenderTarget *target)
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->Draw(target);
		}
	}
}

void Session::UpdatePlayerWireQuads()
{
	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->UpdateWireQuads();
		}
	}
}

//void Session::TestLoad()
//{
//	hitboxManager = new HitboxManager;
//	soundManager = new SoundManager;
//	soundNodeList = new SoundNodeList(10);
//	pauseSoundNodeList = new SoundNodeList(10);
//}

bool Session::ReadDecorImagesFile()
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

			ProcessDecorFromFile(name, tile);
		}

		is.close();
	}
	else
	{
		return false;
	}

	return true;
}

bool Session::LoadPolyShader(int index, int matWorld, int matVariation)
{
	stringstream ss;
	ss << "Terrain/terrain_";

	ss << matWorld + 1 << "_";
	if (matVariation < 10)
	{
		ss << "0" << matVariation + 1;
	}
	else
	{
		ss << matVariation + 1;
	}

	ss << "_512x512.png";

	string matFile = ss.str();

	Tileset *ts_mat = GetTileset(matFile, 512, 512);

	if (ts_mat == NULL)
		return false;

	if (!polyShaders[index].loadFromFile("Resources/Shader/mat_shader2.frag", sf::Shader::Fragment))
	{
		cout << "MATERIAL SHADER NOT LOADING CORRECTLY:" << matFile << endl;
		return false;
	}

	ts_polyShaders[index] = ts_mat;
	polyShaders[index].setUniform("u_texture", *ts_mat->texture);
	polyShaders[index].setUniform("Resolution", Vector2f(1920, 1080));
	polyShaders[index].setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));
	polyShaders[index].setUniform("skyColor", ColorGL(Color::White));

	ReadDecorInfoFile(matWorld, matVariation);
	return true;
}

bool Session::ReadDecorInfoFile(int tWorld, int tVar)
{
	stringstream ss;
	ifstream is;

	ss << "Resources/Terrain/Decor/" << "terraindecor_"
		<< (tWorld + 1) << "_0" << (tVar + 1) << ".txt";
	is.open(ss.str());

	if (is.is_open())
	{
		list<pair<string, int>> loadedDecorList;
		while (!is.eof())
		{
			string dStr;
			is >> dStr;

			if (dStr == "")
			{
				break;
			}

			int frequencyPercent;
			is >> frequencyPercent;

			loadedDecorList.push_back(pair<string, int>(dStr, frequencyPercent));
		}
		is.close();

		TerrainDecorInfo *tdInfo = new TerrainDecorInfo(loadedDecorList.size());
		terrainDecorInfoMap[make_pair(tWorld, tVar)] = tdInfo;

		int dI = 0;
		for (auto it = loadedDecorList.begin(); it != loadedDecorList.end(); ++it)
		{
			tdInfo->decors[dI] = TerrainPolygon::GetDecorType((*it).first);
			tdInfo->percents[dI] = (*it).second;
			++dI;
		}

		return true;
	}
	else
	{
		return false;
		//not found, thats fine.
	}
}



bool Session::ReadHeader(std::ifstream &is)
{
	if (mapHeader != NULL)
	{
		delete mapHeader;
		mapHeader = NULL;
	}
		

	mapHeader = new MapHeader;
	mapHeader->Load(is);
	if (mapHeader == NULL)
		return false;

	ProcessHeader();

	return true;
}

bool Session::ReadDecor(std::ifstream &is)
{
	//decorTSMap.clear();

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

		//Sprite dSpr;
		//dSpr.setScale(dScale);
		//dSpr.setRotation(dRot);
		//dSpr.setPosition(dPos);

		//Tileset *d_ts = ;
		//dSpr.setTexture(*d_ts->texture);
		//dSpr.setTextureRect(d_ts->GetSubRect(dTile));
		//dSpr.setOrigin(dSpr.getLocalBounds().width / 2, dSpr.getLocalBounds().height / 2);

		ProcessDecorSpr( dName, decorTSMap[dName], dTile, dLayer, dPos, dRot, dScale );
	}

	ProcessAllDecorSpr();

	return true;
}

bool Session::ReadPlayerStartPos(std::ifstream &is)
{
	is >> playerOrigPos.x;
	is >> playerOrigPos.y;

	ProcessPlayerStartPos();

	return true;
}

bool Session::ReadPlayerOptions(std::ifstream &is)
{
	if ((mapHeader->ver1 == 2 && mapHeader->ver2 >= 1) || mapHeader->ver1 > 2)
	{
		return playerOptionsField.Load(is);
	}
	else
	{
		playerOptionsField.Reset();
	}

	return true;
}

bool Session::ReadTerrainGrass(std::ifstream &is, PolyPtr poly)
{
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

	poly->ProcessGrass(segments);

	return true;
}

bool Session::ReadTerrain(std::ifstream &is)
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

	int numPoints = mapHeader->numVertices;

	while (numPoints > 0)
	{
		PolyPtr poly(new TerrainPolygon());

		if (!hasReadBorderPoly)
		{
			poly->inverse = true;
			hasReadBorderPoly = true;
		}

		poly->Load(is);
		numPoints -= poly->GetNumPoints();

		ProcessTerrain(poly);
		//poly->Finalize(); //need this before readTerrainGrass

		poly->LoadGrass(is);
	}

	ProcessAllTerrain();
	return true;
}

bool Session::ReadSpecialTerrain(std::ifstream &is)
{
	int specialPolyNum;
	is >> specialPolyNum;

	for (int i = 0; i < specialPolyNum; ++i)
	{
		PolyPtr poly(new TerrainPolygon());

		poly->Load(is);

		poly->Finalize();

		ProcessSpecialTerrain(poly);
	}

	return true;
}

bool Session::ReadBGTerrain(std::ifstream &is)
{
	int bgPlatformNum0;
	is >> bgPlatformNum0;
	for (int i = 0; i < bgPlatformNum0; ++i)
	{
		PolyPtr poly(new TerrainPolygon());
		poly->Load(is);
		poly->Finalize();
		poly->SetLayer(1);

		ProcessBGTerrain(poly);
		//no grass for now
	}
	return true;
}

bool Session::ReadRails(std::ifstream &is)
{
	int numRails;
	is >> numRails;

	for (int i = 0; i < numRails; ++i)
	{
		RailPtr rail(new TerrainRail());
		rail->Load(is);
		ProcessRail(rail);
	}
	return true;
}

bool Session::ReadActors(std::ifstream &is)
{
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete(*it).second;
	}
	groups.clear();

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


			ProcessActor(a);
		}
	}

	ProcessAllActors();

	return true;
}

bool Session::ReadGates(std::ifstream &is)
{
	int numGates;
	is >> numGates;

	SetNumGates(numGates);

	int gCat;
	int gVar;
	int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
	int shardWorld = -1;
	int shardIndex = -1;
	int numToOpen;
	for (int i = 0; i < numGates; ++i)
	{
		shardWorld = -1;
		shardIndex = -1;


		if (mapHeader->ver1 == 1) //version of map before the gate updates
		{
			is >> gCat;

			is >> poly0Index;
			is >> vertexIndex0;
			is >> poly1Index;
			is >> vertexIndex1;

			if (gCat == Gate::SHARD)
			{
				is >> shardWorld;
				is >> shardIndex;
			}

			continue;
		}

		is >> gCat;
		is >> gVar;

		if (gCat == Gate::KEY || gCat == Gate::PICKUP)
		{
			is >> numToOpen;
		}

		is >> poly0Index;
		is >> vertexIndex0;
		is >> poly1Index;
		is >> vertexIndex1;

		if (gCat == Gate::SHARD)
		{
			is >> shardWorld;
			is >> shardIndex;
		}

		ProcessGate( gCat, gVar, numToOpen, poly0Index, vertexIndex0, poly1Index, vertexIndex1, shardWorld,
			shardIndex);
	
	}
	return true;
}


bool Session::ReadFile()
{
	ifstream is;
	is.open(filePathStr);

	if (is.is_open())
	{
		ReadHeader(is);

		ReadDecor(is);

		ReadPlayerStartPos(is);

		ReadPlayerOptions(is);

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
		cout << "file read failed: " << filePathStr << endl;
		assert(false && "error getting file to edit ");

		return false;
	}

	return true;
}

ControllerState &Session::GetPrevInput(int index)
{
	return mainMenu->GetPrevInput(index);
}

ControllerState &Session::GetCurrInput(int index)
{
	return mainMenu->GetCurrInput(index);
}

ControllerState &Session::GetPrevInputUnfiltered(int index)
{
	return mainMenu->GetPrevInputUnfiltered(index);
}

ControllerState &Session::GetCurrInputUnfiltered(int index)
{
	return mainMenu->GetCurrInputUnfiltered(index);
}

GameController &Session::GetController(int index)
{
	return mainMenu->GetController(index);
}

Actor *Session::GetPlayer(int index)
{
	assert(index >= 0 && index < MAX_PLAYERS);
	return players[index];
}

void Session::UpdateControllersOneFrameMode()
{
	bool gccEnabled = mainMenu->gccDriverEnabled;

	if (gccEnabled)
		gcControllers = mainMenu->gccDriver->getState();

	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		//GetPrevInput(i) = GetCurrInput(i);
		//GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);

		/*p = GetPlayer(i);
		if (p != NULL)
		{
			p->prevInput = GetCurrInput(i);
		}*/

		GameController &con = GetController(i);
		if (gccEnabled)
			con.gcController = gcControllers[i];

		con.UpdateState();

		GetCurrInput(i) = con.GetState();
		GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
	}
}

void Session::UpdateControllers()
{
	bool gccEnabled = mainMenu->gccDriverEnabled;

	if (gccEnabled)
		gcControllers = mainMenu->gccDriver->getState();


	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		GetPrevInput(i) = GetCurrInput(i);
		GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);

		if (!cutPlayerInput)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->prevInput = GetCurrInput(i);
			}
		}

		GameController &con = GetController(i);
		if (gccEnabled)
			con.gcController = gcControllers[i];

		con.UpdateState();

		GetCurrInput(i) = con.GetState();
		GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
	}
}

void Session::UpdatePlayerInput(int index)
{
	Actor *player = GetPlayer(index);
	if (player == NULL)
		return;

	ControllerState &pCurr = player->currInput;
	GameController &controller = GetController(index);
	ControllerState &currInput = GetCurrInput(index);
	ControllerState &prevInput = GetPrevInput(index);

	bool alreadyBounce = pCurr.X;
	bool alreadyGrind = pCurr.Y;
	bool alreadyTimeSlow = pCurr.leftShoulder;

	if (cutPlayerInput)
	{
		player->currInput = ControllerState();
	}
	else
	{
		player->currInput = currInput;

		if (controller.keySettings.toggleBounce)
		{
			if (currInput.X && !prevInput.X)
			{
				pCurr.X = !alreadyBounce;
			}
			else
			{
				pCurr.X = alreadyBounce;
			}
		}
		if (controller.keySettings.toggleGrind)
		{
			if (currInput.Y && !prevInput.Y)
			{
				pCurr.Y = !alreadyGrind;
			}
			else
			{
				pCurr.Y = alreadyGrind;
			}
		}
		if (controller.keySettings.toggleTimeSlow)
		{
			if (currInput.leftShoulder && !prevInput.leftShoulder)
			{
				pCurr.leftShoulder = !alreadyTimeSlow;

			}
			else
			{
				pCurr.leftShoulder = alreadyTimeSlow;
			}
		}
	}
}

void Session::UpdateAllPlayersInput()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		UpdatePlayerInput(i);
	}
}

bool Session::IsKeyPressed(int k)
{
	return mainMenu->IsKeyPressed(k);
}

bool Session::IsMousePressed(int m)
{
	return mainMenu->IsMousePressed(m);
}

bool Session::OneFrameModeUpdate()
{
	bool skipInput = IsKeyPressed(sf::Keyboard::PageUp);
	//if (!oneFrameMode) && GetPrevInput(0).PLeft())
	//{
	//	skipInput = true;
	//}

	if (skipInput && !oneFrameMode)
	{
		oneFrameMode = true;
	}

	if (oneFrameMode)
	{
		bool tookScreenShot = false;
		bool screenShot = false;

		//UpdateControllersOneFrameMode();

		//GetController(0).UpdateState();
		//ControllerState &testState = GetController(0).GetState();
		/*if (GetCurrInput(0).PLeft())
		{
			skipInput = true;
		}*/

		/*vector<GCC::GCController> controllers;
		if (mainMenu->gccDriverEnabled)
			controllers = mainMenu->gccDriver->getState();

		for (int i = 0; i < 4; ++i)
		{
			GameController &c = GetController(i);
			if (mainMenu->gccDriverEnabled)
				c.gcController = controllers[i];
			c.UpdateState();
		}*/

		bool stopSkippingInput = IsKeyPressed(sf::Keyboard::PageDown);
		/*if (GetCurrInput(0).PRight())
		{
			stopSkippingInput = true;
		}*/


		screenShot = false;//IsKeyPressed( sf::Keyboard::F );// && !tookScreenShot;

		if (screenShot)
		{
			//cout << "TOOK A SCREENSHOT" << endl;
			//tookScreenShot = true;
			//Image im = window->capture();

			// time_t now = time(0);
			// char* dt = ctime(&now);
			//im.saveToFile( "screenshot.png" );//+ string(dt) + ".png" );
		}
		else
		{
			if (skipInput)
			{
				tookScreenShot = false;
			}
		}

		if (!skipped && skipInput)
		{
			skipped = true;
			accumulator = 0;
			return true;
		}

		if (skipped && !skipInput)
		{
			skipped = false;
		}

		if (stopSkippingInput)
		{
			oneFrameMode = false;
			currentTime = gameClock.getElapsedTime().asSeconds();
			accumulator = 0;
			return true;
		}

		return false;
		//window->clear();
	}

	return true;
}

void Session::CleanupZones()
{
	if (zoneTree != NULL)
	{
		delete zoneTree;
		zoneTree = NULL;
	}

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		delete (*it);
	}
	zones.clear();
}

void Session::DrawZones(sf::RenderTarget *target)
{
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void Session::CreateZones()
{
	for (int i = 0; i < numGates; ++i)
	{
		Gate *g = gates[i];
		if (!g->IsZoneType())
		{
			//UnlockGate(g);
		}
	}
	//OpenGates(Gate::CRAWLER_UNLOCK);
	//no gates, no zones!


	for (int i = 0; i < numGates; ++i)
	{
		Gate *g = gates[i];

		if (!g->IsZoneType())
		{
			continue;
		}
		//cout << "gate index: " << i << ", a: " << g->edgeA->v0.x << ", " << g->edgeA->v0.y << ", b: "
		//	<< g->edgeA->v1.x << ", " << g->edgeA->v1.y << endl;

		Edge *curr = g->edgeA;

		TerrainPolygon tp;
		V2d v0 = curr->v0;
		V2d v1 = curr->v1;
		list<Edge*> currGates;
		list<Gate*> ignoreGates;

		currGates.push_back(curr);


		tp.AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

		curr = curr->edge1;
		while (true)
		{
			if (curr->v0 == g->edgeA->v0)//curr == g->edgeA )
			{
				//we found a zone!

				if (!tp.IsClockwise())
				{
					//cout << "found a zone aaa!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;


					for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end() && okayZone; ++zit)
					{
						for (list<Edge*>::iterator cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit)
						{
							for (list<Edge*>::iterator git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git)
							{
								if ((*cit) == (*git))
								{
									okayZone = false;
								}
							}
							//for( list<Gate*>::iterator git =

						}
					}

					if (okayZone)
					{
						Zone *z = new Zone(tp);
						z->gates = currGates;
						zones.push_back(z);
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
			else if (curr == g->edgeB)
			{
				//currGates.push_back( curr );
				//cout << "not a zone even" << endl;
				break;
			}


			tp.AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

			if (curr->edgeType == Edge::CLOSED_GATE)
			{
				Gate *thisGate = (Gate*)curr->info;
				//this loop is so both sides of a gate can't be hit in the same zone
				bool okayGate = true;
				bool quitLoop = false;
				for (list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it)
				{
					Gate *otherGate = (Gate*)(*it)->info;


					if (otherGate == thisGate)
					{
						//currGates.erase( it );
						okayGate = false;
						break;
					}
				}

				if (!okayGate)
				{
					currGates.push_back(curr);
					Edge *cc = curr->edge0;

					tp.RemoveLastPoint();
					//TerrainPoint *tempPoint = NULL;
					//TerrainPoint *tempPoint = tp.pointEnd;
					//tp.RemovePoint( tempPoint );
					//delete tempPoint;
					//cout << "removing from a( " << g << " ) start: " << tp.numPoints << endl;

					while (true)
					{
						if (cc->edgeType == Edge::CLOSED_GATE)
						{
							Gate *ccGate = (Gate*)cc->info;
							if (ccGate == thisGate)
								break;
							else
							{
								bool foundIgnore = false;
								for (list<Gate*>::iterator it = ignoreGates.begin(); it != ignoreGates.end(); ++it)
								{
									if ((*it) == ccGate)
									{
										foundIgnore = true;
										break;
									}
								}

								if (foundIgnore)
								{
									Edge *temp = cc->edge1;
									ccGate->SetLocked(false);
									cc = temp->edge0;
									ccGate->SetLocked(true);
									continue;
								}
							}
						}


						if (true)
							//if( tp.pointStart != NULL )
						{
							tp.RemoveLastPoint();

							//if( tp.pointStart == tp.pointEnd )
							if (tp.GetNumPoints() == 0)
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

					if (quitLoop)
					{
						//cout << "quitloop a" << endl;
						break;
					}

					Edge *pr = cc->edge0;
					thisGate->SetLocked(false);
					curr = pr->edge1->edge1;
					ignoreGates.push_back(thisGate);
					thisGate->SetLocked(true);

					//cout << "GATE IS NOT OKAY A: " << tp.numPoints << endl;
					//break;
					continue;
				}
				else
				{
					//cout << "found another gate AA: " <<  << endl;
				}

				//cout << "found another gate AA" << endl;
				currGates.push_back(curr);
			}
			else
			{

			}
			curr = curr->edge1;
		}


		currGates.clear();
		ignoreGates.clear();

		curr = g->edgeB;


		currGates.push_back(curr);


		TerrainPolygon tpb;

		tpb.AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

		curr = curr->edge1;
		while (true)
		{
			if (curr->v0 == g->edgeB->v0)//curr == g->edgeB )
			{
				//we found a zone!

				if (!tpb.IsClockwise())
				{
					//cout << "found a zone bbb!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;
					for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end() && okayZone; ++zit)
					{
						for (list<Edge*>::iterator cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit)
						{
							for (list<Edge*>::iterator git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git)
							{
								if ((*cit) == (*git))
								{
									okayZone = false;
								}
							}
							//for( list<Gate*>::iterator git =

						}
					}

					if (okayZone)
					{
						Zone *z = new Zone(tpb);
						//cout << "creating a zone with " << currGates.size() << " gatesBBB" << endl;
						z->gates = currGates;
						zones.push_back(z);
						//cout << "actually creating a new zone   2! with " << z->gates.size() << endl;
					}


				}
				else
				{
					//cout << "woulda been a zone" << endl;
				}

				break;
			}
			else if (curr == g->edgeA)
			{
				//currGates.push_back( curr );
				//cout << "not a zone even b" << endl;
				break;
			}


			tpb.AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

			if (curr->edgeType == Edge::CLOSED_GATE)
			{
				bool quitLoop = false;
				bool okayGate = true;
				Gate *thisGate = (Gate*)curr->info;
				for (list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it)
				{
					Gate *otherGate = (Gate*)(*it)->info;


					if (otherGate == thisGate)
					{
						okayGate = false;
						break;
					}
				}

				if (!okayGate)
				{
					currGates.push_back(curr);

					tpb.RemoveLastPoint();

					Edge *cc = curr->edge0;

					while (true)
					{
						if (cc->edgeType == Edge::CLOSED_GATE)
						{

							Gate *ccGate = (Gate*)cc->info;
							if (ccGate == thisGate)
								break;
							else
							{
								bool foundIgnore = false;
								for (list<Gate*>::iterator it = ignoreGates.begin(); it != ignoreGates.end(); ++it)
								{
									if ((*it) == ccGate)
									{
										foundIgnore = true;
										break;
									}
								}

								if (foundIgnore)
								{
									Edge *temp = cc->edge1;
									ccGate->SetLocked(false);
									cc = temp->edge0;
									ccGate->SetLocked(true);
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

						if (true)
						{
							tpb.RemoveLastPoint();
							//if( tpb.pointStart == tpb.pointEnd )
							if (tpb.GetNumPoints() == 0)
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

					if (quitLoop)
					{
						//cout << "quitloop b" << endl;
						break;
					}

					Edge *pr = cc->edge0;
					thisGate->SetLocked(false);
					curr = pr->edge1->edge1;
					ignoreGates.push_back(thisGate);
					thisGate->SetLocked(true);
					continue;
				}
				else
				{
				}


				currGates.push_back(curr);
			}

			curr = curr->edge1;
		}
	}



	for (int i = 0; i < numGates; ++i)
	{
		if (!gates[i]->IsZoneType())
		{
			continue;
		}
		//gates[i]->SetLocked( true );
		for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
		{
			//cout << "setting gates in zone: " << (*it) << " which has " << (*it)->gates.size() << " gates " << endl;
			//cout << i << ", it gates: " << (*it)->gates.size() << endl;
			for (list<Edge*>::iterator eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit)
			{
				if (gates[i]->edgeA == (*eit))
				{
					//	cout << "gate: " << gates[i] << ", gate zone a: " << (*it ) << endl;
					gates[i]->zoneA = (*it);
					//done++;
				}
				else if (gates[i]->edgeB == (*eit))
				{
					//	cout << "gate: " << gates[i] << ", gate zone B: " << (*it ) << endl;
					//cout << "gate zone B: " << (*it ) << endl;
					gates[i]->zoneB = (*it);
					//done++;
				}
			}
		}
	}


	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		for (list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2)
		{
			if ((*it) == (*it2))
				continue;

			if ((*it)->ContainsZone((*it2)))
			{
				//cout << "contains zone!" << endl;
				(*it)->subZones.push_back((*it2));
			}
		}
	}

	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		for (list<Zone*>::iterator it2 = (*it)->subZones.begin();
			it2 != (*it)->subZones.end(); ++it2)
		{
			if ((*it)->ContainsZoneMostSpecific((*it2)))
			{
				(*it2)->parentZone = (*it);
			}
		}
	}
	//list<Gate*> ;
	list<Edge*> outsideGates;

	int numOutsideGates = 0;
	for (int i = 0; i < numGates; ++i)
	{
		Gate *g = gates[i];
		if (!g->IsZoneType())
		{
			continue;
		}
		if (g->zoneA == NULL && g->zoneB == NULL)
		{
			for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
			{
				if ((*zit)->ContainsPoint(g->edgeA->v0))
				{
					g->zoneA = (*zit);
					g->zoneB = (*zit);
				}
			}
		}
		else if (g->zoneA == NULL)
		{
			if (g->zoneB->parentZone != NULL)
			{
				g->zoneA = g->zoneB->parentZone;
				g->zoneA->gates.push_back(g->edgeA);
			}
			else
			{
				outsideGates.push_back(g->edgeA);
				numOutsideGates++;
			}
		}
		else if (g->zoneB == NULL)
		{
			if (g->zoneA->parentZone != NULL)
			{
				g->zoneB = g->zoneA->parentZone;
				g->zoneB->gates.push_back(g->edgeB);
			}
			else
			{
				outsideGates.push_back(g->edgeB);
				numOutsideGates++;
			}
		}
	}

	if (numOutsideGates > 0)
	{
		//assert(inverseEdgeTree != NULL);

		TerrainPolygon tp;
		Edge *startEdge = allPolysVec[0]->GetEdge(0);
		Edge *curr = startEdge;

		tp.AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

		curr = curr->edge1;

		while (curr != startEdge)
		{
			tp.AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

			curr = curr->edge1;
		}

		tp.FixWinding();

		Zone *z = new Zone(tp);
		z->gates = outsideGates;
		zones.push_back(z);

		for (list<Edge*>::iterator it = outsideGates.begin(); it != outsideGates.end(); ++it)
		{
			Gate *g = (Gate*)(*it)->info;
			if (g->zoneA == NULL)
			{
				g->zoneA = z;
			}

			if (g->zoneB == NULL)
			{
				g->zoneB = z;
			}

		}


		for (list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2)
		{
			if (z == (*it2))
				continue;

			if (z->ContainsZone((*it2)))
			{
				//cout << "contains zone!" << endl;
				z->subZones.push_back((*it2));
			}
		}



		for (list<Zone*>::iterator it2 = z->subZones.begin();
			it2 != z->subZones.end(); ++it2)
		{
			if (z->ContainsZoneMostSpecific((*it2)))
			{
				(*it2)->parentZone = z;
			}
		}

		//TerrainPolygon tp( NULL );

	}

	for (int i = 0; i < numGates; ++i)
	{
		Gate *g = gates[i];
		if (!g->IsZoneType())
		{
			//LockGate(g);
		}
	}
}

void Session::SetupZones()
{
	if (zones.size() == 0)
		return;
	//cout << "setupzones" << endl;
	//setup subzones
	//for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	//{
	//	for( list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2 )
	//	{
	//		if( (*it) == (*it2) ) 
	//			continue;

	//		if( (*it)->ContainsZone( (*it2) ) )
	//		{
	//			//cout << "contains zone!" << endl;
	//			(*it)->subZones.push_back( (*it2) );
	//		}
	//	}
	//}

	//for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	//{
	//	for (list<Zone*>::iterator it2 = (*it)->subZones.begin();
	//		it2 != (*it)->subZones.end(); ++it2)
	//	{
	//		if ((*it)->ContainsZoneMostSpecific((*it2)))
	//		{
	//			(*it2)->parentZone = (*it);
	//		}
	//	}
	//}


	//	cout << "1" << endl;
	//add enemies to the correct zone.
	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
		{
			bool hasPoint = (*zit)->ContainsPoint((*it)->GetPosition());
			if (hasPoint)
			{
				bool mostSpecific = true;
				for (list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2)
				{
					if ((*zit2)->ContainsPoint((*it)->GetPosition()))
					{
						mostSpecific = false;
						break;
					}
				}

				if (mostSpecific)
				{
					(*it)->SetZone((*zit));
				}
			}
		}

		if ((*it)->zone != NULL)
			(*it)->zone->allEnemies.push_back((*it));
	}

	for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
	{
		int numTotalKeys = 0;
		for (auto it = (*zit)->allEnemies.begin(); it != (*zit)->allEnemies.end(); ++it)
		{
			if ((*it)->hasMonitor)
			{
				++numTotalKeys;
			}
		}
	}

	//set key number objects correctly
	for (auto it = zoneObjects.begin(); it != zoneObjects.end(); ++it)
	{
		Zone *assignZone = NULL;
		V2d cPos((*it)->pos.x, (*it)->pos.y);
		for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
		{
			bool hasPoint = (*zit)->ContainsPoint(cPos);
			if (hasPoint)
			{
				bool mostSpecific = true;
				for (list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2)
				{
					if ((*zit2)->ContainsPoint(cPos))
					{
						mostSpecific = false;
						break;
					}
				}

				if (mostSpecific)
				{
					assignZone = (*zit);
				}
			}
		}

		if (assignZone != NULL)
		{
			if ((*it)->zoneType > 0)
				assignZone->SetZoneType((*it)->zoneType);
		}

		delete (*it);
	}

	zoneObjects.clear();



	cout << "2" << endl;
	//which zone is the player in?
	for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
	{
		//Vector2i truePos = Vector2i( player->position.x, player->position.y );
		bool hasPoint = (*zit)->ContainsPoint(GetPlayer(0)->position);
		if (hasPoint)
		{
			bool mostSpecific = true;
			for (list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2)
			{
				if ((*zit2)->ContainsPoint(GetPlayer(0)->position))
				{
					mostSpecific = false;
					break;
				}
			}

			if (mostSpecific)
			{
				originalZone = (*zit);
			}
		}
	}




	if (originalZone != NULL)
	{
		//cout << "setting original zone to active: " << originalZone << endl;
		ActivateZone(originalZone, true);
		adventureHUD->keyMarker->Reset();
	}



	//std::vector<Zone*> zoneVec(zones.size());
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->hasGoal = false;
	}

	bool foundGoal = false;
	Zone *goalZone = NULL;
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		for (auto ait = (*it)->allEnemies.begin(); ait != (*it)->allEnemies.end(); ++ait)
		{
			if ((*ait)->IsGoalType())
			{
				(*it)->hasGoal = true;
				foundGoal = true;
				goalZone = (*it);
				break;
			}
		}
		if (foundGoal)
			break;
	}

	if (!foundGoal)
	{
		assert(foundGoal);
	}

	Gate *g;
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		for (auto git = (*it)->gates.begin(); git != (*it)->gates.end(); ++git)
		{
			g = (Gate*)(*git)->info;
			if (g->zoneA == (*it))
			{
				(*it)->connectedSet.insert(g->zoneB);
			}
			else if (g->zoneB == (*it))
			{
				(*it)->connectedSet.insert(g->zoneA);
			}
			else
			{
				assert(0);
			}
		}
	}

	if (zoneTree != NULL)
	{
		delete zoneTree;
		zoneTree = NULL;
	}

	zoneTreeStart = originalZone;
	zoneTreeEnd = goalZone;
	zoneTree = new ZoneNode;
	zoneTree->parent = NULL;
	zoneTree->SetZone(zoneTreeStart);
	currentZoneNode = zoneTree;

	//std::vector<bool> hasNodeVec(zones.size());

	//using shouldreform to test the secret gate stuff
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->shouldReform = true;
	}

	zoneTree->SetChildrenShouldNotReform();

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		if ((*it)->shouldReform)
		{
			for (auto git = (*it)->gates.begin(); git != (*it)->gates.end(); ++git)
			{
				g = (Gate*)(*git)->info;
				g->category = Gate::SECRET;
				g->Init();
			}
			(*it)->SetZoneType(Zone::SECRET);
		}
	}

	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Init();
	}

	if (originalZone != NULL)
	{
		//CloseOffLimitZones();
		gateMarkers->SetToZone(currentZone);
	}
}

void Session::ActivateZone(Zone * z, bool instant)
{
	if (z == NULL)
		return;
	//assert( z != NULL );
	//cout << "ACTIVATE ZONE!!!" << endl;
	if (!z->active)
	{
		if (instant)
		{
			z->action = Zone::OPEN;
		}
		else
		{
			z->action = Zone::OPENING;
			z->frame = 0;
		}

		for (list<Enemy*>::iterator it = z->spawnEnemies.begin(); it != z->spawnEnemies.end(); ++it)
		{
			assert((*it)->spawned == false);

			(*it)->Init();
			(*it)->spawned = true;
			AddEnemy((*it));
		}

		z->active = true;
		z->visited = true;

		if (activatedZoneList == NULL)
		{
			activatedZoneList = z;
			z->activeNext = NULL;
		}
		else
		{
			z->activeNext = activatedZoneList;
			activatedZoneList = z;
		}

		//z->ReformHigherIndexGates();
	}
	else
	{
		z->reexplored = true;
		{
			//z->frame = 0;
			//z->action = Zone::OPEN;

			/*if (z->action == Zone::CLOSING)
			{
			z->frame = z->openFrames - z->frame;
			z->action = Zone::OPENING;
			}
			else
			{
			z->frame = 0;
			z->action = Zone::OPEN;
			}*/

		}
		//already activated
		//assert(0);
	}


	if (currentZone != NULL && z->zType != Zone::SECRET && currentZone->zType != Zone::SECRET)
	{
		if (currentZone->action == Zone::OPENING)
		{
			currentZone->frame = z->openFrames - currentZone->frame;
		}
		else
		{
			currentZone->frame = 0;
		}
		currentZone->action = Zone::CLOSING;

		currentZone->active = false;

		bool foundNode = false;
		for (auto it = currentZoneNode->children.begin();
			it != currentZoneNode->children.end(); ++it)
		{
			if ((*it)->myZone == z)
			{
				currentZoneNode = (*it);
				foundNode = true;
				break;
			}
		}

		if (!foundNode)
		{
			assert(foundNode);
		}

		KillAllEnemies();

		currentZone = z;

		CloseOffLimitZones();

		gateMarkers->SetToZone(currentZone);
	}
	else
	{

		Zone *oldZone = currentZone;
		currentZone = z;

		if (oldZone == NULL) //for starting the map
		{

		}
	}

	if (!instant)
	{
		//int soundIndex = SoundType::S_KEY_ENTER_0 + (currentZone->requiredKeys);
		//ActivateSound(gameSoundBuffers[soundIndex]);
	}
}

void Session::CloseOffLimitZones()
{
	if (currentZoneNode == NULL)
		return;

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->shouldReform = true;
	}

	currentZoneNode->SetChildrenShouldNotReform();

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		if ((*it)->zType != Zone::SECRET && !(*it)->visited && (*it)->shouldReform)
		{
			(*it)->visited = true;
			(*it)->ReformAllGates();
		}
	}
}

void Session::SetupGateMarkers()
{
	//doesnt care about parentGame

	if (gateMarkers != NULL)
	{
		delete gateMarkers;
		gateMarkers = NULL;
	}

	int maxGates = 0;
	int numGatesInZone;
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		numGatesInZone = (*it)->gates.size();
		if (numGatesInZone > maxGates)
		{
			maxGates = numGatesInZone;
		}
	}

	if (maxGates > 0)
	{
		gateMarkers = new GateMarkerGroup(maxGates);
	}

	//gateMarkers = new GateMarkerGroup()
}

void Session::CleanupGates()
{
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		delete (*it);
	}
	gates.clear();
}

void Session::UpdateGates()
{
	for (int i = 0; i < numGates; ++i)
	{
		gates[i]->Update();
	}
}

void Session::UpdatePlayersInHitlag()
{
	Actor *pTemp = NULL;
	for (int i = 0; i < 4; ++i)
	{
		pTemp = GetPlayer(i);
		if (pTemp != NULL)
		{
			pTemp->UpdateInHitlag();
		}
	}
}

void Session::UpdatePlayersPrePhysics()
{
	Actor *pTemp = NULL;
	for (int i = 0; i < 4; ++i)
	{
		pTemp = GetPlayer(i);
		if (pTemp != NULL)
			pTemp->UpdatePrePhysics();
	}
}

void Session::UpdatePlayersPostPhysics()
{
	Actor *pTemp = NULL;
	for (int i = 0; i < 4; ++i)
	{
		pTemp = GetPlayer(i);
		if (pTemp != NULL)
			pTemp ->UpdatePostPhysics();
	}
}

void Session::HandleEntrant(QuadTreeEntrant *qte)
{
	switch (queryMode)
	{
	case QUERY_ENEMY:
		TrySpawnEnemy(qte);
		break;
	case QUERY_BORDER:
		TryAddPolyToQueryList(qte);
		break;
	case QUERY_SPECIALTERRAIN:
		TryAddSpecialPolyToQueryList(qte);
		break;
	case QUERY_FLYTERRAIN:
		TryAddFlyPolyToQueryList(qte);
		break;
	case QUERY_INVERSEBORDER:
		SetQueriedInverseEdge(qte);
		break;
	case QUERY_GATE:
		TryAddGateToQueryList(qte);
		break;
	case QUERY_ENVPLANT:
		TryActivateQueriedEnvPlant(qte);
		break;
	case QUERY_RAIL:
		TryAddRailToQueryList(qte);
		break;
	}
}

void Session::TrySpawnEnemy(QuadTreeEntrant *qte)
{
	Enemy *e = (Enemy*)qte;

	if (e->spawned)
		return;

	bool a = e->spawnRect.intersects(tempSpawnRect);
	bool b = (e->zone == NULL || e->zone->active);

	if (a && b)
	{
		AddEnemy(e);
	}
}

void Session::TryAddPolyToQueryList(QuadTreeEntrant *qte)
{
	PolyPtr p = (PolyPtr)qte;
	if (polyQueryList == NULL)
	{
		polyQueryList = p;
		polyQueryList->queryNext = NULL;
		numBorders++;
	}
	else
	{
		PolyPtr poly = p;
		PolyPtr temp = polyQueryList;
		bool okay = true;
		while (temp != NULL)
		{
			if (temp == poly)
			{
				okay = false;
				break;
			}
			temp = temp->queryNext;
		}

		if (okay)
		{
			poly->queryNext = polyQueryList;
			polyQueryList = poly;
			numBorders++;
		}
	}
}

void Session::TryAddSpecialPolyToQueryList(QuadTreeEntrant *qte)
{
	PolyPtr p = (PolyPtr)qte;

	if (specialPieceList == NULL)
	{
		specialPieceList = p;
		specialPieceList->queryNext = NULL;
	}
	else
	{
		PolyPtr tva = p;
		PolyPtr temp = specialPieceList;
		bool okay = true;
		while (temp != NULL)
		{
			if (temp == tva)
			{
				okay = false;
				break;
			}
			temp = temp->queryNext;
		}

		if (okay)
		{
			tva->queryNext = specialPieceList;
			specialPieceList = tva;
		}
	}
}

void Session::TryAddFlyPolyToQueryList(QuadTreeEntrant *qte)
{
	PolyPtr p = (PolyPtr)qte;

	if (flyTerrainList == NULL)
	{
		flyTerrainList = p;
		flyTerrainList->queryNext = NULL;
	}
	else
	{
		PolyPtr tva = p;
		PolyPtr temp = flyTerrainList;
		bool okay = true;
		while (temp != NULL)
		{
			if (temp == tva)
			{
				okay = false;
				break;
			}
			temp = temp->queryNext;
		}

		if (okay)
		{
			tva->queryNext = flyTerrainList;
			flyTerrainList = tva;
		}
	}
}

void Session::TryAddGateToQueryList(QuadTreeEntrant *qte)
{
	Gate *g = (Gate*)qte;
	//possible to add the same gate twice? fix.
	if (gateList == NULL)
	{
		gateList = g;
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

void Session::TryAddRailToQueryList(QuadTreeEntrant *qte)
{
	RailPtr r = (RailPtr)qte;
	if (railDrawList == NULL)
	{
		railDrawList = r;
		r->queryNext = NULL;
	}
	else
	{
		r->queryNext = railDrawList;
		railDrawList = r;
	}
}

void Session::SetQueriedInverseEdge(QuadTreeEntrant *qte)
{
	Edge *e = (Edge*)qte;

	if (inverseEdgeList == NULL)
	{
		inverseEdgeList = e;
		inverseEdgeList->info = NULL;
		//numBorders++;
	}
	else
	{
		Edge *tva = e;
		Edge *temp = inverseEdgeList;
		bool okay = true;
		while (temp != NULL)
		{
			if (temp == tva)
			{
				okay = false;
				break;
			}
			temp = (Edge*)temp->info;//->edge1;
		}

		if (okay)
		{
			tva->info = (void*)inverseEdgeList;
			inverseEdgeList = tva;
		}
	}
}

void Session::TryActivateQueriedEnvPlant(QuadTreeEntrant *qte)
{
	EnvPlant *ep = (EnvPlant*)qte;
	if (!ep->activated)
	{
		int idleLength = ep->idleLength;
		int idleFactor = ep->idleFactor;

		IntRect sub = ep->ts->GetSubRect((totalGameFrames % (idleLength * idleFactor)) / idleFactor);
		VertexArray &eva = *ep->va;
		eva[ep->vaIndex + 0].texCoords = Vector2f(sub.left, sub.top);
		eva[ep->vaIndex + 1].texCoords = Vector2f(sub.left + sub.width, sub.top);
		eva[ep->vaIndex + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
		eva[ep->vaIndex + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);
	}
}

void Session::QueryBorderTree(sf::Rect<double> &rect)
{
	polyQueryList = NULL;
	queryMode = QUERY_BORDER;
	numBorders = 0;
	borderTree->Query(this, rect);
}

void Session::QueryGateTree(sf::Rect<double>&rect)
{
	testGateCount = 0;
	queryMode = QUERY_GATE;
	gateList = NULL;
	gateTree->Query(this, rect);
}

void Session::EnemiesCheckedMiniDraw(RenderTarget *target,
	sf::FloatRect &rect)
{
	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->CheckedMiniDraw(target, rect);
	}
}

void Session::DrawAllMapWires(
	sf::RenderTarget *target)
{
	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawMapWires(target);
		}
	}
}

void Session::DrawColoredMapTerrain(sf::RenderTarget *target, sf::Color &c)
{
	//must be already filled from a query
	PolyPtr poly = polyQueryList;
	while (poly != NULL)
	{
		Color oldColor = poly->fillCol;
		poly->SetTerrainColor(c);
		poly->MiniDraw(target);
		poly->SetTerrainColor(oldColor);

		poly = poly->queryNext;
	}
}

void Session::DrawPlayersMini(sf::RenderTarget *target)
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->MiniDraw(target);
		}
	}
}

void Session::SetupHUD()
{
	if (parentGame != NULL)
	{
		adventureHUD = parentGame->adventureHUD;
	}
	else if (mapHeader->gameMode == MapHeader::MapType::T_STANDARD && adventureHUD == NULL)
		adventureHUD = new AdventureHUD;
}

void Session::DrawHUD(sf::RenderTarget *target)
{
	if (adventureHUD != NULL)
	{
		sf::View oldView = target->getView();
		target->setView(uiView);
		adventureHUD->Draw(target);
		target->setView(oldView);
	}
}

void Session::UpdateHUD()
{
	if (adventureHUD != NULL)
		adventureHUD->Update();
}

void Session::HitlagUpdate()
{
	UpdateControllers();
	UpdateAllPlayersInput();

	UpdatePlayersInHitlag();

	UpdateEffects(true);

	cam.UpdateRumble();

	fader->Update();
	swiper->Update();
	mainMenu->UpdateEffects();

	pauseFrames--;

	accumulator -= TIMESTEP;
}