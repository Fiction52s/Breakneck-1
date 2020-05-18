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

	AddBasicGroundWorldEnemy("shroom", 1, CreateEnemy<Shroom>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
		/*
	AddBasicAerialWorldEnemy("patroller", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
		GetTileset("Enemies/patroller_icon_256x256.png", 256, 256));*/

	/*AddBasicAerialWorldEnemy("comboer", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddBasicAerialWorldEnemy("splitcomboer", 1, NULL, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3,
		GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);*/



	/*AddBasicAerialWorldEnemy("jugglercatcher", 1, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	GetTileset("Enemies/jugglercatcher_128x128.png", 128, 128));*/


	
		//GetTileset("Enemies/crawler_160x160.png", 160, 160));

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
		curr = effectListVec[i];

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
	if (e->type == EnemyType::EN_SHARD)
	{
		if (IsSessTypeGame())
		{
			GameSession *game = GameSession::GetSession();
			Shard *sh = (Shard*)e;
			if (game->IsShardCaptured(sh->shardType))
				return;
		}
	}

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
{
	parentGame = NULL;

	sessType = p_sessType;
	cutPlayerInput = false;
	mainMenu = MainMenu::GetInstance();
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
		

	mapHeader = mainMenu->ReadMapHeader(is);
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

		ReadTerrainGrass(is, poly);

		
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

	int gType;
	int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
	int shardWorld = -1;
	int shardIndex = -1;
	for (int i = 0; i < numGates; ++i)
	{
		shardWorld = -1;
		shardIndex = -1;

		is >> gType;
		is >> poly0Index;
		is >> vertexIndex0;
		is >> poly1Index;
		is >> vertexIndex1;

		if (gType == Gate::SHARD)
		{
			is >> shardWorld;
			is >> shardIndex;
		}

		ProcessGate( gType, poly0Index, vertexIndex0, poly1Index, vertexIndex1, shardWorld,
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

		p = GetPlayer(i);
		if (p != NULL)
		{
			p->prevInput = GetCurrInput(i);
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