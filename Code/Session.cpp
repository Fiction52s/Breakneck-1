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
#include "AbsorbParticles.h"
#include "Barrier.h"
#include "ShardSequence.h"
#include "Sequence.h"
#include "ParticleEffects.h"
#include "DeathSequence.h"
#include "TopClouds.h"
#include "StorySequence.h"
#include "GoalFlow.h"
#include "GoalExplosion.h"
#include "InputVisualizer.h"
//#include "Enemy_Shard.h"
#include "EnvEffects.h"
#include "MusicPlayer.h"

//enemy stuff:
#include "SoundManager.h"
#include "GGPO.h"



#include "ActorParams.h"

#include "ShipPickup.h"
#include "EnemiesW1.h"
#include "EnemiesW2.h"
#include "EnemiesW3.h"
#include "EnemiesW4.h"
#include "EnemiesW5.h"
#include "EnemiesW6.h"
#include "EnemiesW7.h"
#include "EnemiesW8.h"

#include "ScoreDisplay.h"

#include "GameMode.h"



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
	//if (fBubbleFrame != NULL)
	//	return;

	//if (parentGame != NULL)
	//{
	//	fBubbleFrame = parentGame->fBubbleFrame;
	//	fBubblePos = parentGame->fBubblePos;
	//	fBubbleRadiusSize = parentGame->fBubbleRadiusSize;
	//	return;
	//}

	//int numBubbleInfo = Actor::MAX_BUBBLES * MAX_PLAYERS;
	//fBubbleFrame = new float[numBubbleInfo];
	//for (int i = 0; i < MAX_TOTAL_BUBBLES; ++i)
	//{
	//	fBubbleFrame[i] = 0;
	//}
	//fBubblePos = new sf::Vector2f[numBubbleInfo];
	//fBubbleRadiusSize = new float[numBubbleInfo];

	////int count = 0;
	//Actor *tempPlayer = NULL;
	//for (int i = 0; i < MAX_PLAYERS; ++i)
	//{
	//	if (tempPlayer = GetPlayer(i))
	//	{
	//		tempPlayer->fBubbleFrame = (fBubbleFrame + i * Actor::MAX_BUBBLES);
	//		tempPlayer->fBubblePos = (fBubblePos + i * Actor::MAX_BUBBLES);
	//		tempPlayer->fBubbleRadiusSize = (fBubbleRadiusSize + i * Actor::MAX_BUBBLES);
	//		//++count;
	//	}
	//}
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
	GetPlayer(pIndex)->SetAllUpgrades(playerOptionsField);
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

void Session::RegisterGeneralEnemies()
{
	ParamsInfo basePI("multiplayerbase", CreateEnemy<MultiplayerBase>, NULL,
		Vector2i(), Vector2i(32, 32), false, false, false, false, true,
		false, false, 1, 1);

	types["multiplayerbase"] = new ActorType(basePI);

	AddExtraEnemy("multiplayerprogresstarget", CreateEnemy<MultiplayerProgressTarget>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(), Vector2i(32, 32), false, false, false, false, true, 
		false, false, 1);


	AddExtraEnemy("zoneproperties", NULL, SetParamsType<ZonePropertiesParams>, Vector2i(0, 0),
		Vector2i(128, 128), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Editor/zoneproperties_128x128.png"));

	AddExtraEnemy("shard", CreateEnemy<Shard>, SetParamsType<ShardParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1 );

	AddExtraEnemy("healthfly", CreateEnemy<FlyChain>, SetParamsType<FlyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	AddExtraEnemy("camerashot", NULL, SetParamsType<CameraShotParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1,
		GetTileset("Editor/camera_128x128.png", 128, 128));

	AddExtraEnemy("key", NULL, SetParamsType<KeyParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false, 1, GetSizedTileset("Enemies/bouncefloater_128x128.png"));

	AddExtraEnemy("poi", NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
	false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/rail_32x32.png"), 1);

	AddExtraEnemy("xbarrier", NULL, SetParamsType<XBarrierParams>,Vector2i(0, 0), Vector2i(64, 64),false, false, false, false, true, false, false, 1,
		GetSizedTileset("Enemies/blocker_w1_192x192.png"));
	AddExtraEnemy("xbarrierwarp", NULL, SetParamsType<XBarrierParams>, Vector2i(0, 0), Vector2i(64, 64), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Enemies/Ball_64x64.png"));

	AddExtraEnemy("shippickup", CreateEnemy<ShipPickup>, SetParamsType<ShipPickupParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false, 1);

	

	//AddExtraEnemy("groundtrigger", NULL, SetParamsType<GroundTriggerParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, true, 1,
	//	GetSizedTileset("Ship/shipleave_128x128.png" ));

	AddExtraEnemy("airtrigger", NULL, SetParamsType<AirTriggerParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Enemies/jayshield_128x128.png"));

	AddExtraEnemy("ship", NULL, SetParamsType<BasicAirEnemyParams>, Vector2i(0, 0), Vector2i(864, 400), false, false, false, false, true, false, false, 1,
		GetSizedTileset("Ship/ship_864x400.png"));

	AddExtraEnemy("flowerpod", CreateEnemy<FlowerPod>, SetParamsType<FlowerPodParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false, 1);
	
	AddExtraEnemy("groundedwarper", CreateEnemy<GroundedWarper>, SetParamsType<GroundedWarperParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false,
		true, false );

	

	/*AddExtraEnemy("groundedwarper", CreateEnemy<GroundedWarper>, SetParamsType<GroundedWarperParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false,
		true, false);*/

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

void Session::RegisterW1Enemies()
{

	AddWorldEnemy("crawlernode", 2, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/crawlernode_32x32.png"));

	AddBasicAerialWorldEnemy("movementtester", 1, CreateEnemy<MovementTester>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);

	AddBasicGroundWorldEnemy("goal", 1, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddBasicGroundWorldEnemy("crawler", 1, CreateEnemy<Crawler>, Vector2i(0, 0), Vector2i(100, 100), true, true, false, false, 3);

	AddBasicGroundWorldEnemy("crawlerqueen", 1, CreateEnemy<CrawlerQueen>, Vector2i(0, 0), Vector2i(100, 100), false, true, false, false, 2);
	
	AddBasicAerialWorldEnemy("booster", 1, CreateEnemy<Booster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3 );
		//GetTileset("Goal/goal_w01_a_288x320.png", 288, 320));

	AddWorldEnemy("blocker", 1, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	AddBasicAerialWorldEnemy("patroller", 1, CreateEnemy<Patroller>, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3 );

	AddBasicGroundWorldEnemy("nexus1", 1, CreateEnemy<Nexus>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddBasicAerialWorldEnemy("airdasher", 1, CreateEnemy<Airdasher>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3 );

	AddBasicGroundWorldEnemy("shroom", 1, CreateEnemy<Shroom>, Vector2i(0, 0), Vector2i(50, 50), true, true, false, false, 3);

	AddBasicGroundWorldEnemy("basicturret", 1, CreateEnemy<BasicTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3 );

	AddBasicAerialWorldEnemy("splitcomboer", 1, CreateEnemy<SplitComboer>, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3 );
	
	AddBasicAerialWorldEnemy("comboer", 1, CreateEnemy<Comboer>, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3 );

	AddWorldEnemy("spring", 1, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false, 1 );

	AddBasicAerialWorldEnemy("bluekeyfly", 1, CreateEnemy<KeyFly>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);

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

void Session::RegisterW2Enemies()
{
	AddBasicAerialWorldEnemy("bird", 2, CreateEnemy<Bird>, Vector2i(0, 0), Vector2i(200, 200), false, true, false, false, 3);

	AddBasicAerialWorldEnemy("glidetarget", 2, CreateEnemy<GlideTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);

	AddBasicGroundWorldEnemy("greengoal", 2, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);
	//AddBasicGroundWorldEnemy("greengoal", 2, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Goal/goal_w02_a_288x256.png", 288, 256));

	//AddWorldEnemy("greenblocker", 2, LoadParams<BlockerParams>, NULL, MakeParamsAerial<BlockerParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/blocker_w2_192x192.png", 192, 192));

	AddBasicGroundWorldEnemy("nexus2",2, CreateEnemy<Nexus>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddWorldEnemy("downgravityjuggler", 2, CreateEnemy<GravityJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3 );

	AddWorldEnemy("upgravityjuggler", 2, CreateEnemy<GravityJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	//AddWorldEnemy("upgravityjuggler", 2, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/jayshield_128x128.png", 128, 128));


	//AddBasicAerialWorldEnemy("airdashjuggler", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128));

	AddWorldEnemy("birdnode", 2, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/birdnode_32x32.png"));

	///*AddBasicAerialWorldEnemy("gravdowncomboer", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	//GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	///*AddBasicAerialWorldEnemy("gravupcomboer", 2, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3,
	//GetTileset("Enemies/comboer_128x128.png", 128, 128));*/

	AddBasicAerialWorldEnemy("bat", 2, CreateEnemy<Bat>, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3);

	AddBasicGroundWorldEnemy("curveturret", 2, CreateEnemy<CurveTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	AddBasicGroundWorldEnemy("poisonfrog", 2, CreateEnemy<PoisonFrog>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	AddBasicGroundWorldEnemy("stagbeetle", 2, CreateEnemy<StagBeetle>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3 );

	AddBasicGroundWorldEnemy("gravityfaller", 2, CreateEnemy<GravityFaller>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	AddWorldEnemy("gravityincreaser", 2, CreateEnemy<GravityModifier>, SetParamsType<GravityModifierParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	AddWorldEnemy("gravitydecreaser", 2, CreateEnemy<GravityModifier>, SetParamsType<GravityModifierParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	//AddBasicAerialWorldEnemy("gravityincreaser", 2, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/grav_increase_256x256.png", 256, 256));

	//AddBasicAerialWorldEnemy("gravitydecreaser", 2, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/grav_decrease_256x256.png", 256, 256));

	AddWorldEnemy("glidespring", 2, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);

	//AddWorldEnemy("bossbird", 2, LoadParams<BossBirdParams>, NULL, MakeParamsAerial<BossBirdParams>,
	//	Vector2i(0, 0), Vector2i(64, 64), false, false, false, false);
}

void Session::RegisterW3Enemies()
{
	//AddBasicAerialWorldEnemy("bouncefloater", 3, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3,
	//	GetTileset("Enemies/bouncefloater_128x128.png", 128, 128));

	//AddBasicGroundWorldEnemy("yellowgoal", 3, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddBasicGroundWorldEnemy("coyote", 3, CreateEnemy<Coyote>, Vector2i(0, 0), Vector2i(80, 80), false, false, false, false);

	AddWorldEnemy("coyotenode", 3, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/coyotenode_32x32.png"));

	AddBasicAerialWorldEnemy("ball", 3, CreateEnemy<Ball>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);

	AddWorldEnemy("bouncejuggler", 3, CreateEnemy<BounceJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	AddWorldEnemy("bouncespring", 3, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1 );
	//AddWorldEnemy("airbouncespring", 3, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);

	//AddWorldEnemy("airbouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	AddBasicAerialWorldEnemy("upbouncebooster", 3, CreateEnemy<BounceBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3 );

	AddBasicAerialWorldEnemy("omnibouncebooster", 3, CreateEnemy<BounceBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3 );

	//*AddWorldEnemy("redirectspring", 3, LoadParams<BounceSpringParams>, NULL, MakeParamsAerial<BounceSpringParams>,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddWorldEnemy("reflectspring", 3, LoadParams<BounceSpringParams>, NULL, MakeParamsAerial<BounceSpringParams>,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));*/

	////AddWorldEnemy("bouncespring", 3, LoadParams<GravitySpringParams>, NULL, MakeParamsAerial<GravitySpringParams>,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	////	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256));

	//AddBasicAerialWorldEnemy("pulser", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3);

	AddBasicGroundWorldEnemy("badger", 3, CreateEnemy<Badger>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3 );

	//AddBasicGroundWorldEnemy("roadrunner", 3, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/roadrunner_256x256.png", 256, 256));

	AddBasicAerialWorldEnemy("owl", 3, CreateEnemy<Owl>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);


	AddBasicGroundWorldEnemy("cactus", 3, CreateEnemy<Cactus>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	///*AddWorldEnemy("cactus", 3, LoadParams<CactusParams>, MakeParamsGrounded<CactusParams>, NULL,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/

	//AddWorldEnemy("bosscoyote", 3, LoadParams<BossCoyoteParams>, NULL, MakeParamsAerial<BossCoyoteParams>,
	//	Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
}

void Session::RegisterW4Enemies()
{
	//AddBasicGroundWorldEnemy("orangegoal", 4, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddBasicGroundWorldEnemy("tiger", 4, CreateEnemy<Tiger>, Vector2i(0, 0), Vector2i(80, 80), false, true, false, false, 2);
	//AddWorldEnemy("rail", 4, LoadParams<RailParams>, NULL, MakeParamsAerial<RailParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false,
	//	3, GetTileset("Enemies/rail_64x64.png", 64, 64));

	//AddWorldEnemy("grindrail", 4, LoadParams<RailParams>, NULL, MakeParamsAerial<RailParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false,
	//	3, GetTileset("Enemies/rail_64x64.png", 64, 64));

	AddWorldEnemy("tigernode", 4, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/tigernode_32x32.png"));

	AddBasicAerialWorldEnemy("teleporter", 4, CreateEnemy<Spring>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);

	//AddWorldEnemy("onewayteleporter", 4, LoadParams<TeleporterParams>, NULL, MakeParamsAerial<TeleporterParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1,
	//	GetTileset("Enemies/spring_idle_2_256x256.png", 256, 256), 1);

	AddWorldEnemy("grindjugglercw", 4, CreateEnemy<GrindJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	AddWorldEnemy("grindjugglerccw", 4, CreateEnemy<GrindJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	AddWorldEnemy("groundedgrindjugglercw", 4, CreateEnemy<GroundedGrindJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, false, true, false, 3);

	AddWorldEnemy("groundedgrindjugglerccw", 4, CreateEnemy<GroundedGrindJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, false, true, false, 3);

	//AddBasicRailWorldEnemy("railtest", 4, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/shroom_192x192.png", 192, 192));

	AddBasicGroundWorldEnemy("spider", 4, CreateEnemy<Spider>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	AddBasicAerialWorldEnemy("turtle", 4, CreateEnemy<Turtle>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	AddBasicGroundWorldEnemy("cheetah", 4, CreateEnemy<Cheetah>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//AddWorldEnemy("coral", 4, LoadParams<CoralParams>, NULL, MakeParamsAerial<CoralParams>,
	//	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddWorldEnemy("bosstiger", 4, LoadParams<BossTigerParams>, NULL, MakeParamsAerial<BossTigerParams>,
	//	Vector2i(0, 0), Vector2i(64, 64), false, false, false, false);


}

void Session::RegisterW5Enemies()
{
	//AddBasicGroundWorldEnemy("redgoal", 5, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddWorldEnemy("gatornode", 5, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/gatornode_32x32.png"));

	AddBasicAerialWorldEnemy("gator", 5, CreateEnemy<Gator>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);

	AddWorldEnemy("hungrycomboer", 5, CreateEnemy<HungryComboer>, SetParamsType<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	AddWorldEnemy("hungryreturncomboer", 5, CreateEnemy<HungryComboer>, SetParamsType<JugglerParams>,
		Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	//AddWorldEnemy("hungryreturncomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 2);

	//AddWorldEnemy("relativecomboer", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	//AddWorldEnemy("relativecomboerdetach", 5, LoadParams<JugglerParams>, NULL, MakeParamsAerial<JugglerParams>,
	//	Vector2i(0, 0), Vector2i(128, 128), true, true, true, false, 3,
	//	GetTileset("Enemies/comboer_128x128.png", 128, 128), 1);

	AddBasicAerialWorldEnemy("swarm", 5, CreateEnemy<Swarm>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//AddBasicAerialWorldEnemy("shark", 5, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
	//	GetTileset("Enemies/shark_circle_256x256.png", 256, 256));

	///*AddWorldEnemy("shark", 5, LoadParams<SharkParams>, NULL, MakeParamsAerial<SharkParams>,
	//Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);*/


	AddBasicGroundWorldEnemy("growingtree", 5, CreateEnemy<GrowingTree>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	////AddWorldEnemy("overgrowth", 5, LoadParams<OvergrowthParams>, MakeParamsGrounded<OvergrowthParams>, NULL,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	AddBasicAerialWorldEnemy("ghost", 5, CreateEnemy<Ghost>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	////AddWorldEnemy("ghost", 5, LoadParams<GhostParams>, NULL, MakeParamsAerial<GhostParams>,
	////	Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddWorldEnemy("bossgator", 5, LoadParams<BossGatorParams>, NULL, MakeParamsAerial<BossGatorParams>,
	//	Vector2i(0, 0), Vector2i(32, 128), false, false, false, false);
}

void Session::RegisterW6Enemies()
{
	//AddBasicGroundWorldEnemy("magentagoal", 6, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddWorldEnemy("skeletonnode", 6, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/skeletonnode_32x32.png"));

	AddBasicAerialWorldEnemy("dimensioneye", 6, CreateEnemy<DimensionEye>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);
	AddBasicAerialWorldEnemy("dimensionexit", 6, CreateEnemy<DimensionExit>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);

	AddBasicGroundWorldEnemy("skeleton", 6, CreateEnemy<Skeleton>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
	//w6
	AddBasicGroundWorldEnemy("coyotehelper", 6, CreateEnemy<CoyoteHelper>, Vector2i(0, 0), Vector2i(80, 80), false, false, false, false);

	AddBasicAerialWorldEnemy("gorilla", 6, CreateEnemy<Gorilla>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//AddWorldEnemy("glidespring", 2, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddBasicAerialWorldEnemy("specter", 6, CreateEnemy<Specter>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	AddWorldEnemy("swinglaunchercw", 6, CreateEnemy<SwingLauncher>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddWorldEnemy("swinglauncherccw", 6, CreateEnemy<SwingLauncher>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);

	AddBasicAerialWorldEnemy("freeflighttarget", 6, CreateEnemy<FreeFlightTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);

	//AddBasicAerialWorldEnemy("specter", 6, CreateEnemy<Specter>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	/*AddBasicAerialWorldEnemy("wiretarget", 6, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3,
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

void Session::RegisterW7Enemies()
{
	//AddBasicGroundWorldEnemy("greygoal", 7, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);

	AddWorldEnemy("greyskeletonnode", 7, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, GetSizedTileset("Enemies/skeleton2node_32x32.png"));

	AddBasicAerialWorldEnemy("greyskeleton", 7, CreateEnemy<GreySkeleton>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
}

void Session::RegisterW8Enemies()
{
	//AddBasicGroundWorldEnemy("blackgoal", 8, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);
}

void Session::RegisterAllEnemies()
{
	RegisterGeneralEnemies();
	RegisterW1Enemies();
	RegisterW2Enemies();
	RegisterW3Enemies();
	RegisterW4Enemies();
	RegisterW5Enemies();
	RegisterW6Enemies();
	RegisterW7Enemies();
	RegisterW8Enemies();
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
	effectListVec.resize(EffectLayer::EFFECTLAYER_Count);

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
	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
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
	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
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

void Session::PlayerApplyHit( int index, HitboxInfo *hi, Actor *attacker, int res, V2d &pos )
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->ApplyHit(hi, attacker, (Actor::HitResult)res, pos );
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

void Session::PlayerHitNexus( Nexus *nex, int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		p->hitNexus = nex;
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
		cout << "making big bullet VA" << endl;
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
	if (totalNumberBullets > 0 )
	{
		target->draw(bigBulletVA, totalNumberBullets * 4, sf::Quads, ts_basicBullets->texture);
	}
}

Session::Session( SessionType p_sessType, const boost::filesystem::path &p_filePath)
	:playerOptionsField(PLAYER_OPTION_BIT_COUNT)
{
	flowHandler.sess = this;
	mainMenu = MainMenu::GetInstance();
	preScreenTex = MainMenu::preScreenTexture;
	minimapTex = MainMenu::minimapTexture;
	postProcessTex2 = MainMenu::postProcessTexture2;
	fader = mainMenu->fader;
	swiper = mainMenu->swiper;
	ggpo = NULL;

	bossNodeVectorMap.resize(BossFightType_Count);

	playerSimState = new PState;

	gameMode = NULL;

	timeSyncFrames = 0;

	frameRateDisplay.InitText(mainMenu->arial);
	runningTimerDisplay.InitText(mainMenu->arial);

	totalGameFrames = 0;
	numGates = 0;
	drain = true;
	goalDestroyed = false;
	playerAndEnemiesFrozen = false;

	currSuperPlayer = NULL;
	superSequence = NULL;
	inputVis = NULL;
	scoreDisplay = NULL;
	rain = NULL;
	goalFlow = NULL;
	goalPulse = NULL;
	currStorySequence = NULL;
	shardsCapturedField = NULL;
	preLevelScene = NULL;
	postLevelScene = NULL;
	activeSequence = NULL;

	shipEnterScene = NULL;
	shipExitScene = NULL;

	originalMusic = NULL;
	hud = NULL;
	gateMarkers = NULL;
	topClouds = NULL;

	getShardSeq = NULL;
	deathSeq = NULL;
	shardPop = NULL;

	zoneTree = NULL;
	currentZoneNode = NULL;
	zoneTreeStart = NULL;
	zoneTreeEnd = NULL;
	currentZone = NULL;
	originalZone = NULL;
	activatedZoneList = NULL;

	absorbParticles = NULL;
	absorbDarkParticles = NULL;
	absorbShardParticles = NULL;

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
	
	railDrawTree = NULL;
	flyTerrainTree = NULL;
	terrainTree = NULL;
	specialTerrainTree = NULL;
	railEdgeTree = NULL;
	barrierTree = NULL;
	borderTree = NULL;
	grassTree = NULL;
	activeItemTree = NULL;
	gateTree = NULL;
	enemyTree = NULL;

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
	showNetStats = false;
}



Session::~Session()
{
	delete playerSimState;
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
		bigBulletVA = NULL;
	}

	if (parentGame == NULL && absorbParticles != NULL)
	{
		delete absorbParticles;
		absorbParticles = NULL;
	}

	if (parentGame == NULL && absorbDarkParticles != NULL)
	{
		delete absorbDarkParticles;
		absorbDarkParticles = NULL;
	}

	if (parentGame == NULL && absorbShardParticles != NULL)
	{
		delete absorbShardParticles;
		absorbShardParticles = NULL;
	}


	if (parentGame == NULL && hud != NULL)
	{
		delete hud;
		hud = NULL;
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
	
	if (railDrawTree != NULL)
	{
		delete railDrawTree;
		railDrawTree = NULL;
	}

	if (terrainTree != NULL)
	{
		delete terrainTree;
		terrainTree = NULL;
	}
	
	if (enemyTree!= NULL)
	{
		delete enemyTree;
		enemyTree = NULL;
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

	CleanupGlobalBorders();

	CleanupBarriers(); //has to be before background deletion for warp barriers

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

	if (getShardSeq != NULL)
	{
		delete getShardSeq;
		getShardSeq = NULL;
	}

	if (shardPop != NULL)
	{
		delete shardPop;
		shardPop = NULL;
	}

	if (parentGame == NULL && shardsCapturedField != NULL)
	{
		delete shardsCapturedField;
		shardsCapturedField = NULL;
	}

	if ( parentGame == NULL && deathSeq != NULL)
	{
		delete deathSeq;
		deathSeq = NULL;
	}

	if (parentGame == NULL && inputVis != NULL)
	{
		delete inputVis;
		inputVis = NULL;
	}

	CleanupPreLevelScene();
	CleanupPostLevelScene();

	CleanupShipEntrance();
	CleanupShipExit();

	CleanupZones();
	CleanupGates();

	CleanupCameraShots();
	CleanupPoi();
	CleanupBossNodes();
	

	CleanupTopClouds();

	CleanupGoalFlow();
	CleanupGoalPulse();

	CleanupRain();

	CleanupGateMarkers();

	CleanupScoreDisplay();

	if (gameMode != NULL)
	{
		delete gameMode;
		gameMode = NULL;
	}

	CleanupSuperSequence();
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


	if (matWorld + 1 >= 9)
	{
		ss << "_32x32.png";
	}
	else
	{
		ss << "_512x512.png";
	}
	

	string matFile = ss.str();

	Tileset *ts_mat = GetSizedTileset(matFile);

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

bool Session::ReadPlayersStartPos(std::ifstream &is)
{
	int numPlayers = mapHeader->GetNumPlayers();

	for (int i = 0; i < numPlayers; ++i)
	{
		is >> playerOrigPos[i].x;
		is >> playerOrigPos[i].y;
	}
	

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
		if (groups[groupName] != NULL)
		{
			delete groups[groupName]; //because its not getting made in the editor startup too.
		}
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

		ReadPlayersStartPos(is);

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

		/*if (!cutPlayerInput)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->prevInput = GetCurrInput(i);
			}
		}*/

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
		player->prevInput = player->currInput;
		player->currInput = currInput;
		//player->prevInput = prevInput;

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
	currentZoneNode = NULL;
	currentZone = NULL;

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

		AdventureHUD *ah = GetAdventureHUD();
		if( ah != NULL ) ah->keyMarker->Reset();
		
		
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
		if( gateMarkers != NULL )
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

		if (gateMarkers != NULL)
		{
			gateMarkers->SetToZone(currentZone);
		}
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

	CleanupGateMarkers();

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

void Session::QueryRailDrawTree(sf::Rect<double> &rect)
{
	railDrawList = NULL;
	queryMode = QUERY_RAIL;
	railDrawTree->Query(this, screenRect);
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

void Session::DrawSpecialMapTerrain(sf::RenderTarget *target)
{
	PolyPtr poly = specialPieceList;
	while (poly != NULL)
	{
		//Color oldColor = poly->fillCol;
		//poly->SetTerrainColor(c);
		poly->MiniDraw(target);
		//poly->SetTerrainColor(oldColor);

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
		hud = parentGame->hud;
	}
	else
	{
		if (hud == NULL)
		{
			hud = gameMode->CreateHUD();
		}
	}
}

void Session::DrawHUD(sf::RenderTarget *target)
{
	if (hud != NULL)
	{
		sf::View oldView = target->getView();
		target->setView(uiView);
		hud->Draw(target);
		target->setView(oldView);
	}
}

void Session::UpdateHUD()
{
	if (hud != NULL)
		hud->Update();
}

void Session::HitlagUpdate()
{
	//UpdateControllers();
	//UpdateAllPlayersInput();

	//UpdatePlayersInHitlag();

	if (activeSequence == superSequence)
	{
		ActiveSequenceUpdate();
		currSuperPlayer->CheckBirdCommands();
	}

	UpdateEffects(true);

	cam.UpdateRumble();

	fader->Update();
	swiper->Update();
	mainMenu->UpdateEffects();

	pauseFrames--;

	//cout << "hitlag update. pause frames is now: " << pauseFrames << endl;

	
}

void Session::SetupAbsorbParticles()
{
	if (parentGame != NULL)
	{
		absorbParticles = parentGame->absorbParticles;
		absorbDarkParticles = parentGame->absorbDarkParticles;
		absorbShardParticles = parentGame->absorbShardParticles;
	}
	else if (absorbParticles == NULL)
	{
		absorbParticles = new AbsorbParticles(this, AbsorbParticles::ENERGY);
		absorbDarkParticles = new AbsorbParticles(this, AbsorbParticles::DARK);
		absorbShardParticles = new AbsorbParticles(this, AbsorbParticles::SHARD);
	}
}

void Session::ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
	V2d &pos, float startAngle)
{
	switch (absorbType)
	{
	case AbsorbParticles::ENERGY:
		absorbParticles->Activate(p, storedHits, pos, startAngle);
		break;
	case AbsorbParticles::DARK:
		absorbDarkParticles->Activate(p, storedHits, pos, startAngle);
		break;
	case AbsorbParticles::SHARD:
		absorbShardParticles->Activate(p, storedHits, pos, startAngle);
		break;
	}
}

void Session::CollectKey()
{
	GetPlayer(0)->numKeysHeld++;
	if (hud != NULL && hud->hType == HUD::ADVENTURE)
	{
		AdventureHUD *ah = (AdventureHUD*)hud;
		ah->keyMarker->UpdateKeyNumbers();
	}
}

void Session::ResetAbsorbParticles()
{
	absorbParticles->Reset();
	absorbDarkParticles->Reset();
	absorbShardParticles->Reset();
}

void Session::DrawEnemies(sf::RenderTarget *target)
{
	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		if (current->type != EnemyType::EN_BASICEFFECT && (pauseFrames < 2 || current->receivedHit == NULL))
		{
			current->Draw(preScreenTex);
		}
		current = current->next;
	}

	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->CheckedZoneDraw(target, FloatRect(screenRect));
	}
}

void Session::DrawHitEnemies(sf::RenderTarget *target)
{
	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		if ((pauseFrames >= 2 && current->receivedHit != NULL))
		{
			current->Draw(target);
		}
		current = current->next;
	}
}

void Session::ResetZones()
{
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Reset();
	}
}

void Session::UpdateZones()
{
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Update();
	}
}

void Session::SetNumGates(int nGates)
{
	gates.reserve(nGates);
	numGates = nGates;
}

void Session::LockGate(Gate *g)
{
	//inefficient but i can adjust it later using prev pointers
	g->SetLocked(true);
}

void Session::UnlockGate(Gate *g)
{
	g->SetLocked(false);
}

void Session::DrawGates(sf::RenderTarget *target)
{
	//put this query within the frame update, not the draw call
	testGateCount = 0;
	queryMode = QUERY_GATE;
	gateList = NULL;
	gateTree->Query(this, screenRect);

	while (gateList != NULL)
	{
		gateList->Draw(target);
		Gate *next = gateList->next;//(Gate*)gateList->edgeA->edge1;
		gateList = next;
	}
}

void Session::ResetGates()
{
	for (int i = 0; i < numGates; ++i)
	{
		gates[i]->Reset();
	}
}

void Session::ResetBarriers()
{
	for (auto it = barriers.begin(); it != barriers.end(); ++it)
	{
		(*it)->Reset();
	}
}

void Session::AddBarrier(XBarrierParams *xbp, bool warp )
{
	const std::string &xbpName = xbp->GetName();
	Barrier *b = new Barrier(xbpName, true, xbp->GetIntPos().x, xbp->hasEdge, NULL, warp );
	barrierMap[xbpName] = b;
	barriers.push_back(b);
}

void Session::Fade(bool in, int frames, sf::Color c, bool skipKin)
{
	fader->Fade(in, frames, c, skipKin);
}

void Session::CrossFade(int fadeOutFrames,
	int pauseFrames, int fadeInFrames,
	sf::Color c, bool skipKin)
{
	fader->CrossFade(fadeOutFrames, pauseFrames, fadeInFrames, c, skipKin);
}

void Session::ClearFade()
{
	fader->Clear();
}

bool Session::IsFading()
{
	return fader->IsFading();
}

void Session::TotalDissolveGates(int gCat)
{
	Gate *g;
	for (int i = 0; i < numGates; ++i)
	{
		g = gates[i];
		g->TotalDissolve();
	}
}

void Session::ReverseDissolveGates(int gCat)
{
	Gate *g;
	for (int i = 0; i < numGates; ++i)
	{
		g = gates[i];
		g->ReverseDissolve();
	}
}

void Session::SetDrainOn(bool d)
{
	drain = d;
}

void Session::RemoveAllEnemies()
{
	Enemy *curr = activeEnemyList;
	while (curr != NULL)
	{
		Enemy *next = curr->next;

		if (curr->type != EnemyType::EN_GOAL && curr->type != EnemyType::EN_NEXUS)
		{
			RemoveEnemy(curr);
			//curr->health = 0;
		}
		curr = next;
	}
}

void Session::FreezePlayerAndEnemies(bool freeze)
{
	playerAndEnemiesFrozen = freeze;
}

int Session::GetGameSessionState()
{
	return gameState;
}

void Session::SetGameSessionState(int s)
{
	gameState = (GameState)s;
}

void Session::SetGlobalBorders()
{
	//borders not allowed to intersect w/ gates
	V2d topLeft(mapHeader->leftBounds, mapHeader->topBounds);
	V2d topRight(mapHeader->leftBounds + mapHeader->boundsWidth, mapHeader->topBounds);
	V2d bottomRight(mapHeader->leftBounds + mapHeader->boundsWidth, mapHeader->topBounds + mapHeader->boundsHeight);
	V2d bottomLeft(mapHeader->leftBounds, mapHeader->topBounds + mapHeader->boundsHeight);

	Edge *left = new Edge;
	left->v0 = topLeft;
	left->v1 = bottomLeft;
	left->edgeType = Edge::BORDER;

	Edge *right = new Edge;
	right->v0 = bottomRight;
	right->v1 = topRight;
	right->edgeType = Edge::BORDER;
	//cout << "making new edge at x value: " << right->v0.x << endl;

	Edge *top = new Edge;
	top->v0 = topRight;
	top->v1 = topLeft;
	top->edgeType = Edge::BORDER;

	Edge *bot = new Edge;
	bot->v0 = bottomLeft;
	bot->v1 = bottomRight;
	bot->edgeType = Edge::BORDER;

	left->edge0 = top;
	left->edge1 = bot;

	top->edge0 = right;
	top->edge1 = left;

	right->edge0 = bot;
	right->edge1 = top;

	bot->edge0 = left;
	bot->edge1 = right;

	terrainTree->Insert(left);
	terrainTree->Insert(right);
	terrainTree->Insert(top);
	//terrainTree->Insert(bot); //this is a problem because it lets the goal flow leak out. can probably fix it in goal flow rather than doing it here.

	globalBorderEdges.push_back(left);
	globalBorderEdges.push_back(right);
	globalBorderEdges.push_back(top);
	globalBorderEdges.push_back(bot);
	//

}

void Session::SetupGlobalBorderQuads(bool *blackBorder, bool &topBorderOn)
{
	double extraBorder = 100;
	if (inversePolygon != NULL)
	{
		IntRect inverseAABB = inversePolygon->GetAABB();

		int trueTop = mapHeader->topBounds;
		int possibleTop = inverseAABB.top; //- extraBorder;
		if (possibleTop > trueTop)
			trueTop = possibleTop;
		else
		{
			topBorderOn = true;
		}
		mapHeader->topBounds = trueTop - extraBorder / 2;
		int inversePolyBottom = inverseAABB.top + inverseAABB.height;
		mapHeader->boundsHeight = (inversePolyBottom + extraBorder) - trueTop;

		int inversePolyRight = (inverseAABB.left + inverseAABB.width);
		blackBorder[0] = inverseAABB.left < mapHeader->leftBounds; //inverse is further left than border
		blackBorder[1] = inversePolyRight >(mapHeader->leftBounds + mapHeader->boundsWidth); //inverse is further right than border

		int leftB = mapHeader->leftBounds;
		int rightB = mapHeader->leftBounds + mapHeader->boundsWidth;
		if (!blackBorder[0])
		{
			mapHeader->leftBounds = inverseAABB.left - extraBorder;
			mapHeader->boundsWidth = rightB - mapHeader->leftBounds;
		}
		if (!blackBorder[1])
		{
			mapHeader->boundsWidth = (inversePolyRight + extraBorder) - mapHeader->leftBounds;
		}
		else
		{
			//cout << "creating black border at " << (mapHeader->leftBounds + mapHeader->boundsWidth) << endl;
		}
	}
	else
	{
		blackBorder[0] = true;
		blackBorder[1] = true;

		int maxY;
		int minX;
		int maxX;
		bool adjust = false;

		if (IsSessTypeGame() && !allPolysVec.empty())
		{
			adjust = true;
			auto it = allPolysVec.begin();

			IntRect polyAABB = (*it)->GetAABB();
			maxY = polyAABB.top + polyAABB.height;
			minX = polyAABB.left;
			maxX = polyAABB.left + polyAABB.width;

			++it;
			int temp;
			for (; it != allPolysVec.end(); ++it)
			{
				polyAABB = (*it)->GetAABB();
				temp = polyAABB.top + polyAABB.height;
				if (temp > maxY)
				{
					maxY = temp;
				}
				temp = polyAABB.left;
				if (temp < minX)
				{
					minX = temp;
				}
				temp = polyAABB.left + polyAABB.width;
				if (temp > maxX)
				{
					maxX = temp;
				}
			}
		}
		else if (IsSessTypeEdit())
		{
			EditSession *edit = EditSession::GetSession();

			auto &polyList = edit->polygons;

			if (!polyList.empty())
			{
				adjust = true;
				auto it = polyList.begin();

				IntRect polyAABB = (*it)->GetAABB();
				maxY = polyAABB.top + polyAABB.height;
				minX = polyAABB.left;
				maxX = polyAABB.left + polyAABB.width;

				++it;
				int temp;
				for (; it != polyList.end(); ++it)
				{
					polyAABB = (*it)->GetAABB();
					temp = polyAABB.top + polyAABB.height;
					if (temp > maxY)
					{
						maxY = temp;
					}
					temp = polyAABB.left;
					if (temp < minX)
					{
						minX = temp;
					}
					temp = polyAABB.left + polyAABB.width;
					if (temp > maxX)
					{
						maxX = temp;
					}
				}
			}
		}

		if (adjust)
		{
			mapHeader->boundsHeight = maxY - mapHeader->topBounds - extraBorder;
			int oldRight = mapHeader->leftBounds + mapHeader->boundsWidth;
			int oldLeft = mapHeader->leftBounds;
			if (minX > oldLeft)
			{
				mapHeader->leftBounds = minX;
			}
			if (maxX <= oldRight)
			{
				mapHeader->boundsWidth = maxX - mapHeader->leftBounds;
			}
		}
	}

	SetGlobalBorders();

	int quadWidth = 200;
	int extra = 1000;

	int top = mapHeader->topBounds;
	int lBound = mapHeader->leftBounds;
	int rBound = mapHeader->leftBounds + mapHeader->boundsWidth;
	int height = mapHeader->boundsHeight;

	SetRectCenter(blackBorderQuads, quadWidth, height, Vector2f(lBound + quadWidth / 2,
		top + height / 2));
	SetRectCenter(blackBorderQuads + 4, quadWidth, height, Vector2f(rBound - quadWidth / 2,
		top + height / 2));

	/*SetRectCenter(blackBorderQuads + 8, extra, height, Vector2f(lBound - extra / 2,
		top + height / 2));
	SetRectCenter(blackBorderQuads + 12, extra, height, Vector2f(rBound + extra / 2,
		top + height / 2));*/
	SetRectColor(blackBorderQuads, Color(Color::Black));
	SetRectColor(blackBorderQuads + 4, Color(Color::Black));
	//SetRectColor(blackBorderQuads + 8, Color(Color::Black));
	//SetRectColor(blackBorderQuads + 12, Color(Color::Black));

	if (blackBorder[0])
	{
		blackBorderQuads[1].color.a = 0;
		blackBorderQuads[2].color.a = 0;
	}
	else
	{
		SetRectColor(blackBorderQuads, Color(Color::Transparent));
		//SetRectColor(blackBorderQuads + 8, Color(Color::Transparent));
	}
	if (blackBorder[1])
	{
		blackBorderQuads[4].color.a = 0;
		blackBorderQuads[7].color.a = 0;
	}
	else
	{
		SetRectColor(blackBorderQuads + 4, Color(Color::Transparent));
		//SetRectColor(blackBorderQuads + 12, Color(Color::Transparent));
	}
}

void Session::CleanupGlobalBorders()
{
	for (auto it = globalBorderEdges.begin(); it != globalBorderEdges.end(); ++it)
	{
		delete (*it);
	}
	globalBorderEdges.clear();
}

void Session::CleanupBarriers()
{
	for (auto it = barriers.begin();
		it != barriers.end(); ++it)
	{
		delete (*it);
	}
	barriers.clear();
}

void Session::QueryToSpawnEnemies()
{
	queryMode = QUERY_ENEMY;
	sf::Rect<double> spawnRect = screenRect;
	double spawnExtra = 600;//800
	spawnRect.left -= spawnExtra;
	spawnRect.width += 2 * spawnExtra;
	spawnRect.top -= spawnExtra;
	spawnRect.height += 2 * spawnExtra;

	tempSpawnRect = spawnRect;
	enemyTree->Query(this, spawnRect);
}

void Session::DrawBlackBorderQuads(sf::RenderTarget *target)
{
	bool narrowMap = mapHeader->boundsWidth < 1920 * 2;

	if (cam.manual || narrowMap)
	{
		target->draw(blackBorderQuads, 16, sf::Quads);
	}
	else
	{
		target->draw(blackBorderQuads, 8, sf::Quads);
	}
}

void Session::TryCreateShardResources()
{
	if (shardPop == NULL)
	{
		shardPop = new ShardPopup;
	}

	if (getShardSeq == NULL)
	{
		getShardSeq = new GetShardSequence;
		getShardSeq->Init();
	}
}

void Session::SetupShardsCapturedField()
{
	if (parentGame != NULL)
	{
		shardsCapturedField = parentGame->shardsCapturedField;
	}
	else if (shardsCapturedField == NULL)
		shardsCapturedField = new BitField(32 * 5);
	else
	{
		shardsCapturedField->Reset();
	}
}

void Session::SetActiveSequence(Sequence *activeSeq)
{
	activeSequence = activeSeq;

	if (activeSequence == preLevelScene)
	{
		FreezePlayerAndEnemies(true);
		SetPlayerInputOn(false);
	}

	activeSequence->StartRunning();
}

void Session::ActiveSequenceUpdate()
{
	if (activeSequence != NULL)// && activeSequence == startSeq )
	{
		GameState oldState = gameState;
		if (!activeSequence->Update())
		{
			if (activeSequence->nextSeq != NULL)
			{
				activeSequence->nextSeq->Reset();
				SetActiveSequence(activeSequence->nextSeq);
			}
			else
			{
				if (activeSequence == preLevelScene)
				{
					FreezePlayerAndEnemies(false);
					SetPlayerInputOn(true);
					if (shipEnterScene != NULL)
					{
						shipEnterScene->Reset();
						SetActiveSequence(shipEnterScene);
						gameState = RUN;
						return;
					}
				}


				if (activeSequence == postLevelScene)
				{
					goalDestroyed = true;
				}

				gameState = RUN;
				activeSequence = NULL;
			}
		}
		else
		{
			if (gameState != oldState)
			{
				switchGameState = true;
				return;
				//goto starttest;
			}
		}
	}
}

void Session::DrawActiveSequence(EffectLayer layer, sf::RenderTarget *target)
{
	if (activeSequence != NULL)
	{
		sf::View oldView = target->getView();
		if (layer == UI_FRONT)
		{
			target->setView(uiView);
		}

		activeSequence->Draw(target, layer);

		if (layer == UI_FRONT)
		{
			target->setView(oldView);
		}
	}
}

bool Session::IsShardCaptured(int s)
{
	return shardsCapturedField->GetBit(s);
}

void Session::AddEmitter(ShapeEmitter *emit,
	EffectLayer layer)
{
	ShapeEmitter *&currList = emitterLists[layer];
	if (currList == NULL)
	{
		currList = emit;
		emit->next = NULL;
	}
	else
	{
		emit->next = currList;
		currList = emit;
	}
}


void Session::DrawEmitters(EffectLayer layer, sf::RenderTarget *target)
{
	ShapeEmitter *curr = emitterLists[layer];
	while (curr != NULL)
	{
		curr->Draw(target);
		curr = curr->next;
	}
}

void Session::UpdateEmitters()
{
	ShapeEmitter *prev = NULL;
	ShapeEmitter *curr;
	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
	{
		curr = emitterLists[i];
		prev = NULL;
		while (curr != NULL)
		{
			if (curr->IsDone())
			{
				if (curr == emitterLists[i])
				{
					emitterLists[i] = curr->next;
				}
				else
				{
					prev->next = curr->next;
				}
				curr = curr->next;
			}
			else
			{
				curr->Update();

				prev = curr;
				curr = curr->next;
			}
		}
	}
}

void Session::ClearEmitters()
{
	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
	{
		emitterLists[i] = NULL;
	}
}

void Session::SetupDeathSequence()
{
	if (parentGame != NULL)
	{
		deathSeq = parentGame->deathSeq;
	}
	else if (deathSeq == NULL)
	{
		deathSeq = new DeathSequence;
		deathSeq->Init();
	}
}

void Session::DrawDyingPlayers(sf::RenderTarget *target)
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DeathDraw(target);
		}
	}
}

void Session::DrawKinOverFader(sf::RenderTarget *target)
{
	Actor *p = NULL;
	if ((fader->fadeSkipKin && fader->fadeAlpha > 0) || (swiper->skipKin && swiper->IsSwiping()))//IsFading()) //adjust later?
	{
		DrawEffects(EffectLayer::IN_FRONT, preScreenTex);
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->Draw(preScreenTex);
			}
		}
	}
}

void Session::UpdateBarriers()
{
	for (auto it = barriers.begin();
		it != barriers.end(); ++it)
	{
		bool trig = (*it)->Update();
		if (trig)
		{
			TriggerBarrier((*it));
		}
	}
}

void Session::TriggerBarrier(Barrier *b)
{
	if (b->triggerSeq != NULL)
	{
		SetActiveSequence(b->triggerSeq);
	}
	else if (b->myBonus != NULL)
	{
		if (b->warpSeq != NULL)
		{
			b->SetWarpSeq();
		}
		else
		{
			//curent never happens
			GameSession *game = GameSession::GetSession();
			if (game != NULL)
			{
				game->SetBonus(b->myBonus, V2d(0, 0));
			}
		}
	}
	else
	{
		b->Trigger();
	}
	

	/*if (b->triggerSeq != NULL)
	{
		SetActiveSequence(b->triggerSeq);
	}
	else
	{
		b->Trigger();
	}*/
}

void Session::SetupBarrierScenes()
{
	//happens after all enemies have been loaded
	for (auto it = barriers.begin(); it != barriers.end(); ++it)
	{
		(*it)->SetScene();
	}
}

void Session::SetupEnemyZoneSprites()
{
	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->SetZoneSpritePosition();
	}
}

void Session::CleanupCameraShots()
{
	for (auto it = cameraShotMap.begin(); it != cameraShotMap.end(); ++it)
	{
		delete (*it).second;
	}
	cameraShotMap.clear();
}

void Session::AddCameraShot(CameraShotParams *csp)
{
	const std::string &cName = csp->GetName();
	
	if (cameraShotMap.count(cName) > 0)
	{
		//already loaded by another sequence
		return;
		//assert(false);
	}

	CameraShot *shot = new CameraShot(csp->GetName(), csp->GetFloatPos(), csp->zoom);

	cameraShotMap[cName] = shot;
}

void Session::AddPoi(PoiParams *pp)
{
	PoiInfo *pi = NULL;
	if (pp->posInfo.IsAerial())
	{
		pi = new PoiInfo(pp->name, pp->GetIntPos());
	}
	else
	{
		pi = new PoiInfo(pp->name, pp->posInfo.GetEdge()->poly, pp->posInfo.GetEdgeIndex(), pp->posInfo.GetQuant() );

	}
	poiMap[pp->name] = pi;
}

void Session::AddBossNode( const std::string &nodeTypeName, PoiParams *pp)
{
	PoiInfo *pi = NULL;
	if (pp->posInfo.IsAerial())
	{
		pi = new PoiInfo(pp->name, pp->GetIntPos());
	}
	else
	{
		pi = new PoiInfo(pp->name, pp->posInfo.GetEdge()->poly, pp->posInfo.GetEdgeIndex(), pp->posInfo.GetQuant());
	}

	int ftIndex = -1;
	if (nodeTypeName == "crawlernode")
	{
		ftIndex = FT_CRAWLER;
	}
	if (nodeTypeName == "birdnode")
	{
		ftIndex = FT_BIRD;
	}
	else if (nodeTypeName == "coyotenode")
	{
		ftIndex = FT_COYOTE;
	}
	else if (nodeTypeName == "tigernode")
	{
		ftIndex = FT_TIGER;
	}
	else if (nodeTypeName == "gatornode")
	{
		ftIndex = FT_GATOR;
	}
	else if (nodeTypeName == "skeletonnode")
	{
		ftIndex = FT_SKELETON;
	}
	else if (nodeTypeName == "greyskeletonnode")
	{
		ftIndex = FT_SKELETON2;
	}
	else if (nodeTypeName == "bearnode")
	{
		ftIndex = FT_BEAR;
	}

	bossNodeVectorMap[ftIndex][pp->name].push_back(pi);
}

std::map<std::string, std::vector<PoiInfo*>> & Session::GetBossNodeVectorMap( int i)
{
	return bossNodeVectorMap[i];
}

std::vector<PoiInfo*> & Session::GetBossNodeVector(int i, const std::string &name)
{
	auto &m = GetBossNodeVectorMap(i);
	assert(m.count(name) > 0);
	return m[name];
}

void Session::CleanupBossNodes()
{
	for (auto vit = bossNodeVectorMap.begin(); vit != bossNodeVectorMap.end(); ++vit)
	{
		for (auto mit = (*vit).begin(); mit != (*vit).end(); ++mit)
		{
			auto &poiVec = (*mit).second;
			for (auto pit = poiVec.begin(); pit != poiVec.end(); ++pit)
			{
				delete (*pit);
			}
		}
		(*vit).clear();
	}
	//bossNodeVectorMap.clear();
}


void Session::CleanupPoi()
{
	for (auto it = poiMap.begin(); it != poiMap.end(); ++it)
	{
		delete (*it).second;
	}
	poiMap.clear();
}

void Session::CleanupTopClouds()
{
	if (topClouds != NULL)
	{
		delete topClouds;
		topClouds = NULL;
	}
}

void Session::DrawTopClouds(sf::RenderTarget *target)
{
	if (topClouds != NULL)
		topClouds->Draw(target);
}

void Session::UpdateTopClouds()
{
	if (topClouds != NULL)
		topClouds->Update();
}

void Session::CleanupShipEntrance()
{
	if (shipEnterScene != NULL)
	{
		delete shipEnterScene;
		shipEnterScene = NULL;
	}
}

void Session::CleanupShipExit()
{
	if (shipExitScene != NULL)
	{
		delete shipExitScene;
		shipExitScene = NULL;
	}
}

void Session::UpdateEnemiesPrePhysics()
{
	Actor *player = GetPlayer(0);
	if (player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT)
	{
		return;
	}

	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		current->UpdatePrePhysics();
		current = current->next;
	}
}

void Session::UpdateEnemiesPreFrameCalculations()
{
	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		current->UpdatePreFrameCalculations();
		current = current->next;
	}
}

void Session::UpdatePhysics()
{
	Actor *p = NULL;
	Actor *player = GetPlayer(0);
	if (player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT)
	{
		return;
	}

	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->physicsOver = false;
	}


	for (substep = 0; substep < NUM_MAX_STEPS; ++substep)
	{
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				if (substep == 0 || p->highAccuracyHitboxes)
					p->UpdatePhysics();
			}
		}

		Enemy *current = activeEnemyList;
		while (current != NULL)
		{
			current->UpdatePhysics(substep);
			current = current->next;
		}
	}
}

void Session::UpdateEnemiesPostPhysics()
{
	Actor *player = GetPlayer(0);
	if (player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT)
	{
		return;
	}

	int keyLength = 30;//16 * 5;
	keyFrame = totalGameFrames % keyLength;


	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		Enemy *temp = current->next; //need this in case enemy is removed during its update

		current->UpdatePostPhysics();

		if (current->hasMonitor)
		{
			float halftot = keyLength / 2;
			float fac;
			if (keyFrame < keyLength / 2)
			{
				fac = keyFrame / (halftot - 1);
			}
			else
			{
				fac = 1.f - (keyFrame - halftot) / (halftot - 1);
			}
			//cout << "fac: " << fac << endl;
			current->keyShader.setUniform("prop", fac);
		}

		current = temp;
	}

	/*for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
	(*it)->CheckedZoneUpdate(FloatRect(screenRect));
	}*/
}

void Session::QuerySpecialTerrainTree(sf::Rect<double>&rect)
{
	if (specialTerrainTree != NULL)
	{
		specialPieceList = NULL;
		queryMode = QUERY_SPECIALTERRAIN;
		specialTerrainTree->Query(this, rect);
	}
}

void Session::QueryFlyTerrainTree(sf::Rect<double>&rect)
{
	if (flyTerrainTree != NULL)
	{
		flyTerrainList = NULL;
		queryMode = QUERY_FLYTERRAIN;
		flyTerrainTree->Query(this, rect);
	}
}

void Session::DrawStoryLayer(EffectLayer ef, sf::RenderTarget *target)
{
	if (currStorySequence != NULL)
	{
		sf::View oldV = target->getView();
		target->setView(uiView);
		currStorySequence->DrawLayer(target, ef);
		target->setView(oldV);
	}
}

void Session::DrawGateMarkers(sf::RenderTarget *target)
{
	if (gateMarkers != NULL)
		gateMarkers->Draw(target);
}

void Session::LayeredDraw(EffectLayer ef, sf::RenderTarget *target)
{
	DrawDecor(ef, target);
	DrawStoryLayer(ef, target);
	DrawActiveSequence(ef, target);
	DrawEffects(ef, target);
	DrawEmitters(ef, target);
}

typedef pair<V2d, V2d> pairV2d;
void Session::SetupGoalFlow()
{
	//is still created in a bonus level

	CleanupGoalFlow();

	int bgLayer = 0;
	QuadTree *qt = terrainTree;

	assert(qt != NULL);

	double angle = 0;
	double divs = 64;
	double moveAngle = (2 * PI) / divs;
	double tau = 2 * PI;
	double startRadius = 50;
	bool insideTerrain = false;
	bool knowInside = false;
	double rayLen = 100;
	double width = 16;

	list<list<pair<V2d, bool>>> allInfo;
	double rayCheck = 0;

	for (int i = 0; i < divs; ++i)
	{
		rayIgnoreEdge1 = NULL;
		rayIgnoreEdge = NULL;

		allInfo.push_back(list<pair<V2d, bool>>());
		list<pair<V2d, bool>> &pointList = allInfo.back();

		double angle = (tau / divs) * i;
		V2d rayDir(cos(angle), sin(angle));

		rayStart = goalPos + rayDir * startRadius;
		rayEnd = rayStart + rayDir * rayLen;

		bool rayOkay = rayEnd.x >= mapHeader->leftBounds && rayEnd.y >= mapHeader->topBounds
			&& rayEnd.x <= mapHeader->leftBounds + mapHeader->boundsWidth
			&& rayEnd.y <= mapHeader->topBounds + mapHeader->boundsHeight;


		Edge *cEdge = NULL;
		//list<pair<V2d, bool>> pointList; //if true, then its facing the ray


		while (rayOkay)
		{
			//cout << "ray start: " << rayStart.x << ", " << rayStart.y << endl;
			rcEdge = NULL;

			RayCast(&flowHandler, qt->startNode, rayStart, rayEnd);
			//rayStart = v0 + along * quant;
			//rayIgnoreEdge = te;
			//V2d goalDir = normalize( rayStart - goalPos );
			//rayEnd = rayStart + goalDir * maxRayLength;//rayStart - norm * maxRayLength;


			//start ray
			if (rcEdge != NULL)
			{
				//cout << "point list size: " << pointList.size() << endl;
				if (rcEdge->IsInvisibleWall() || rcEdge->edgeType == Edge::CLOSED_GATE)
				{
					//	cout << "secret break" << endl;
					break;
				}

				rayIgnoreEdge1 = rayIgnoreEdge;
				rayIgnoreEdge = rcEdge;

				V2d rn = rcEdge->Normal();
				double d = dot(rn, rayDir);
				V2d hitPoint = rcEdge->GetPosition(rcQuantity);
				if (d > 0)
				{
					if (pointList.size() > 0 && pointList.back().second == false)
					{
						pointList.pop_back();
						cout << "goal lines failing here: " << i << " although nothing seems wrong"
							<< endl;
						//assert( 0 );
					}
					else
					{
						//pointList.pop_back
						pointList.push_back(pair<V2d, bool>(hitPoint, false)); //not facing the ray, so im inside
																			   //cout << "adding false: " << hitPoint.x << ", " << hitPoint.y << "    " << pointList.size() << endl;
					}
				}
				else if (d < 0)
				{
					if (pointList.size() > 0 && pointList.back().second == true)
					{
						pointList.pop_back();
						//cout << "failing here111 " << i << endl;
						//assert( 0 ); //commented out this assert for testing
					}
					else
					{

						pointList.push_back(pair<V2d, bool>(hitPoint, true)); // facing the ray, so im outside
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

			double oldRayCheck = rayCheck;
			rayCheck = length((goalPos + rayDir * startRadius) - rayEnd);

			if (rayCheck == oldRayCheck)
			{
				rayOkay = false;
			}
			else
			{
				rayOkay = rayCheck <= 10000;
			}
			//cout << "rayLen: " << rayLen << endl;

			//rayOkay = rayEnd.x >= leftBounds && rayEnd.y >= topBounds && rayEnd.x <= leftBounds + boundsWidth 
			//	&& rayEnd.y <= topBounds + boundsHeight;
		}

		if (pointList.size() > 0)
		{
			if (pointList.front().second == false)
			{
				//	cout << "adding to front!" << endl;
				//pointList.pop_front();
				pointList.push_front(pair<V2d, bool>(goalPos + rayDir * startRadius, true));
			}
			if (pointList.back().second == true)
			{
				//pointList.pop_back();
				//	cout << "popping from back!" << endl;
				pointList.push_back(pair<V2d, bool>(rayEnd, false));
			}
		}

		assert(pointList.size() % 2 == 0);

		//true then false

		//always an even number of them
	}

	if (allInfo.empty())
	{
		return;
	}

	goalFlow = new GoalFlow(Vector2f(goalPos), allInfo);
}

void Session::CleanupGoalFlow()
{
	if (goalFlow != NULL)
	{
		delete goalFlow;
		goalFlow = NULL;
	}
}

void Session::FlowHandler::HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion)
{
	if (edge->edgeType == Edge::CLOSED_GATE || edge->edgeType == Edge::OPEN_GATE)
	{
		return;
	}

	if (edge != sess->rayIgnoreEdge && edge != sess->rayIgnoreEdge1
		&& (sess->rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - sess->rayStart) <
			length(sess->rcEdge->GetPosition(sess->rcQuantity) - sess->rayStart)))
	{
		sess->rcEdge = edge;
		sess->rcQuantity = edgeQuantity;
	}
}

void Session::DrawGoalFlow(sf::RenderTarget *target)
{
	if (goalFlow != NULL)
	{
		goalFlow->Draw(target);
	}
}

void Session::UpdateGoalFlow()
{
	if (goalFlow != NULL)
	{
		//Vector2f topLeft(screenRect.left, screenRect.top + screenRect.height); //actually bottom left
		Vector2f topLeft(view.getCenter().x - view.getSize().x / 2,
			view.getCenter().y + view.getSize().y / 2);
		float zoom = view.getSize().x / 960.f;
		//goalFlow->Update(cam.GetZoom(), topLeft);
		goalFlow->Update(zoom, topLeft);
	}
}

void Session::CleanupGoalPulse()
{
	if (parentGame == NULL && goalPulse != NULL)
	{
		delete goalPulse;
		goalPulse = NULL;
	}
}

void Session::UpdateGoalPulse()
{
	if (goalPulse != NULL)
	{
		goalPulse->Update();
	}
}

void Session::SetupGoalPulse()
{
	if (parentGame != NULL && parentGame->hasGoal )
	{
		goalPulse = parentGame->goalPulse;
	}
	else if (goalPulse == NULL)
	{
		goalPulse = new GoalPulse;// , Vector2f(goalPos.x, goalPos.y));
	}

	goalPulse->SetPosition(Vector2f(goalPos));
}


void Session::DrawGoalPulse(sf::RenderTarget *target)
{
	if (goalPulse != NULL)
	{
		goalPulse->Draw(target);
	}
}

void Session::CleanupRain()
{
	if (rain != NULL)
	{
		delete rain;
		rain = NULL;
	}
}

void Session::SetupRain()
{
	CleanupRain();
	rain = new Rain;
}
void Session::UpdateRain()
{
	if (rain != NULL)
		rain->Update();
}

void Session::DrawRain(sf::RenderTarget *target)
{
	if (rain != NULL)
		rain->Draw(target);
}

void Session::DrawFrameRate(sf::RenderTarget *target)
{
	if (frameRateDisplay.showFrameRate)
	{
		target->draw(frameRateDisplay.frameRateText);
	}
}

void Session::DrawRunningTimer(sf::RenderTarget *target)
{
	if (runningTimerDisplay.showRunningTimer && (scoreDisplay == NULL || !scoreDisplay->active) )
	{
		target->draw(runningTimerDisplay.runningTimerText);
	}
}

void Session::DrawInputVis(sf::RenderTarget *target)
{
	if (inputVis != NULL)
		inputVis->Draw(target);
}

void Session::UpdateInputVis()
{
	if (inputVis != NULL)
		inputVis->Update(GetPlayer(0)->currInput);
}

Session::FrameRateDisplay::FrameRateDisplay()
{
	frameRateCounterWait = 20;
	Reset();
	showFrameRate = false;

	
}

void Session::FrameRateDisplay::InitText( sf::Font &f )
{
	frameRateText.setFont(f);
	frameRateText.setString("00");
	frameRateText.setCharacterSize(30);
	frameRateText.setFillColor(Color::Red);
	frameRateText.setPosition(0, 1040);
}



void Session::FrameRateDisplay::Update(double frameTime)
{
	if (showFrameRate)
	{
		if (frameRateCounter == frameRateCounterWait)
		{
			double blah = 1.0 / frameTime;
			frameRateTimeTotal += blah;
			frameRateText.setString(to_string(frameRateTimeTotal / (frameRateCounterWait + 1)));
			frameRateCounter = 0;
			frameRateTimeTotal = 0;
		}
		else
		{
			double blah = 1.0 / frameTime;
			frameRateTimeTotal += blah;
			++frameRateCounter;
		}
	}
}

void Session::FrameRateDisplay::Reset()
{
	frameRateCounter = 0;
	frameRateTimeTotal = 0;
}

void Session::DrawScoreDisplay(sf::RenderTarget *target)
{
	if( scoreDisplay != NULL )
		scoreDisplay->Draw(target);
}

void Session::SetupInputVis()
{
	if (parentGame != NULL)
	{
		inputVis = parentGame->inputVis;
	}
	else
	{
		if (inputVis == NULL)
		{
			inputVis = new InputVisualizer;
		}
	}
}


//rendertexture? what was that used for?
void Session::DrawGame(sf::RenderTarget *target)//sf::RenderTarget *target)
{
	target->setView(view);

	if (background != NULL)
		background->Draw(target);

	//UpdateEnvShaders(); //move this into the update loop

	DrawTopClouds(target);

	DrawBlackBorderQuads(target);

	LayeredDraw(EffectLayer::BEHIND_TERRAIN, target);

	DrawZones(target);

	DrawSpecialTerrain(target);

	DrawFlyTerrain(target);

	DrawTerrain(target);

	DrawGoalFlow(target);

	DrawHUD(target);

	DrawGates(target);
	DrawRails(target);

	LayeredDraw(EffectLayer::BEHIND_ENEMIES, target);

	DrawEnemies(target);

	LayeredDraw(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, target);

	DrawGoalPulse(target);
	DrawPlayerWires(target);

	DrawHitEnemies(target); //whited out hit enemies

	absorbParticles->Draw(target);
	absorbDarkParticles->Draw(target);

	DrawPlayers(target);

	

	DrawPlayerShields(target);

	absorbShardParticles->Draw(target);

	DrawReplayGhosts(target);

	LayeredDraw(EffectLayer::IN_FRONT, target);

	DrawBullets(target);

	DrawRain(target);

	//DrawActiveEnvPlants();

	DebugDraw(target);

	//target->draw(testSimCircle);

	//DrawShockwaves(target); //not operational atm

	target->setView(uiView);


	DrawRaceFightScore(target);

	DrawScoreDisplay(target);

	DrawFrameRate(target);

	DrawRunningTimer(target);

	DrawInputVis(target);

	DrawGateMarkers(target);

	target->setView(view);

	DrawDyingPlayers(target);

	//UpdateTimeSlowShader();

	target->setView(uiView);

	LayeredDraw(EffectLayer::UI_FRONT, target);

	fader->Draw(target);
	swiper->Draw(target);

	mainMenu->DrawEffects(target);

	target->setView(view); //sets it back to normal for any world -> pixel calcs
	DrawKinOverFader(target);
}


Session::RunningTimerDisplay::RunningTimerDisplay()
{
	showRunningTimer = true;
}

void Session::RunningTimerDisplay::InitText(sf::Font &f)
{
	runningTimerText.setFont(f);
	runningTimerText.setString("---- : --");
	runningTimerText.setCharacterSize(30);
	runningTimerText.setFillColor(Color::Red);
	runningTimerText.setPosition(0, 0);
	/*runningTimerText.setOrigin(runningTimerText.getLocalBounds().left +
		runningTimerText.getLocalBounds().width, 0);
	runningTimerText.setPosition(1920 - 30, 10);*/
}


void Session::RunningTimerDisplay::Reset()
{
	
}

void Session::UpdateRunningTimerText()
{
	if (runningTimerDisplay.showRunningTimer && (scoreDisplay == NULL || !scoreDisplay->active))
	{
		int tFrames = totalGameFrames;
		if (totalFramesBeforeGoal >= 0)
		{
			tFrames = totalFramesBeforeGoal;
		}
		runningTimerDisplay.runningTimerText.setString(GetTimeStr(tFrames));

	}
}

void Session::UpdateSoundNodeLists()
{
	soundNodeList->Update();
}

void Session::UpdateScoreDisplay()
{
	if (scoreDisplay != NULL)
	{
		scoreDisplay->Update();
	}
}

void Session::ActiveStorySequenceUpdate()
{
	if (currStorySequence != NULL)
	{
		if (!currStorySequence->Update(GetPrevInput(0), GetCurrInput(0)))
		{
			currStorySequence->EndSequence();
			currStorySequence = NULL;
		}
		else
		{
		}
	}
}


bool Session::RunGameModeUpdate()
{
	collider.ClearDebug();

	//double testMult = accumulator / TIMESTEP;
	//if (testMult > 10)
	//{
	//	cout << "accumulator mult: " << testMult << "\n";
	//}
	

	while (accumulator >= TIMESTEP)
	{	
		if (!OneFrameModeUpdate())
		{
			break;
		}

		if (!RunPreUpdate())
			break;


		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPreFrameCalculations();
		}

		UpdateControllers();

		RepPlayerUpdateInput();

		RecPlayerRecordFrame();

		UpdateAllPlayersInput();

		if (pauseFrames > 0)
		{
			HitlagUpdate(); //the full update while in hitlag
			accumulator -= TIMESTEP;
			if (pauseFrames == 0)
			{
				currSuperPlayer = NULL;
			}
			continue;
		}

		//int pIndex = 0;
		//ForwardSimulatePlayer(pIndex, 30);//GetPlayer(pIndex)->hitstunFrames);
		//testSimCircle.setFillColor(Color::Red);
		//testSimCircle.setRadius(20);
		//testSimCircle.setOrigin(testSimCircle.getLocalBounds().width / 2, testSimCircle.getLocalBounds().height / 2);
		//testSimCircle.setPosition(Vector2f(GetPlayer(pIndex)->position));
		//RevertSimulatedPlayer(pIndex);

		/*if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPreFrameCalculations();
		}

		UpdateControllers();

		RepPlayerUpdateInput();

		RecPlayerRecordFrame();

		UpdateAllPlayersInput();*/

		ActiveSequenceUpdate();
		if (switchGameState)
			break;

		if (!playerAndEnemiesFrozen)
		{
			UpdatePlayersPrePhysics();
		}
		

		int bonusRes = TryToActivateBonus();
		if (bonusRes == GameSession::GR_BONUS_RESPAWN)
		{
			return false;
		}
		else if (bonusRes == GameSession::GR_EXITLEVEL)
		{
			return false;
		}


		ActiveStorySequenceUpdate();

		UpdateInputVis();

		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPrePhysics();

			UpdatePhysics();
		}

		//RecordReplayEnemies();

		if (!playerAndEnemiesFrozen)
		{
			UpdatePlayersPostPhysics();
		}

		RecGhostRecordFrame();

		UpdateReplayGhostSprites();

		if (gameMode->CheckVictoryConditions())
		{
			gameMode->EndGame();
			break;
		}

		if (!RunPostUpdate())
		{
			break;
		}

		UpdatePlayerWireQuads();

		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPostPhysics();
		}

		UpdateGates();

		absorbParticles->Update();
		absorbDarkParticles->Update();
		absorbShardParticles->Update();

		UpdateEffects();
		UpdateEmitters();

		mainMenu->musicPlayer->Update();

		UpdateHUD();

		UpdateScoreDisplay();

		UpdateSoundNodeLists();

		UpdateGoalPulse();

		UpdateRain();

		UpdateBarriers();

		UpdateCamera();

		if (gateMarkers != NULL)
			gateMarkers->Update(&cam);

		fader->Update();
		swiper->Update();

		if (IsSessTypeGame())
		{
			background->Update(view.getCenter());
		}
		UpdateTopClouds();

		mainMenu->UpdateEffects();

		UpdateRaceFightScore();

		UpdateGoalFlow();

		QueryToSpawnEnemies();

		UpdateEnvPlants();

		QueryBorderTree(screenRect);

		QuerySpecialTerrainTree(screenRect);

		QueryRailDrawTree(screenRect);

		QueryFlyTerrainTree(screenRect);

		UpdateDecorSprites();
		UpdateDecorLayers();

		if (UpdateRunModeBackAndStartButtons())
		{

		}

		UpdateZones();

		UpdateEnvShaders(); //havent tested at this position. should work fine.

		totalGameFrames++;

		accumulator -= TIMESTEP;

		//if (debugScreenRecorder != NULL)
		//{
		//	break; //for recording stuff
		//}
	}

	if (switchGameState && gameState != FROZEN)
	{
		return false;
	}

	//if (debugScreenRecorder != NULL)
	//	if (IsKeyPressed(Keyboard::R))
	//	{
	//		debugScreenRecorder->StartRecording();
	//		//player->maxFallSpeedSlo += maxFallSpeedFactor;
	//		//cout << "maxFallSpeed : " << player->maxFallSpeed << endl;
	//	}
	return true;
}

bool Session::FrozenGameModeUpdate()
{
	while (accumulator >= TIMESTEP)
	{
		if (!OneFrameModeUpdate())
		{
			break;
		}

		UpdateControllers();

		ActiveSequenceUpdate();

		fader->Update();
		swiper->Update();

		if (gameState != FROZEN)
		{
			break;
		}

		accumulator -= TIMESTEP;
	}

	if (gameState != FROZEN)
	{
		return false;
	}

	return true;
}

void Session::DrawGameSequence(sf::RenderTarget *target)
{
	if (activeSequence != NULL)
	{
		//preScreenTex->setView(uiView);
		for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
		{
			activeSequence->Draw(preScreenTex, (EffectLayer)i);
		}
	}

	preScreenTex->setView(uiView);


	fader->Draw(preScreenTex);
	swiper->Draw(preScreenTex);

	mainMenu->DrawEffects(preScreenTex);

	DrawFrameRate(preScreenTex);
}

bool Session::SequenceGameModeUpdate()
{
	while (accumulator >= TIMESTEP)
	{
		if (!OneFrameModeUpdate())
		{
			return true;
		}

		UpdateControllers();

		ActiveSequenceUpdate();

		mainMenu->musicPlayer->Update();

		fader->Update();
		swiper->Update();
		mainMenu->UpdateEffects();
		UpdateEmitters();

		accumulator -= TIMESTEP;

		if (goalDestroyed)
		{
			SequenceGameModeRespondToGoalDestroyed();
			break;
		}
	}

	if (switchGameState)
	{
		return false;
	}

	return true;
}

void Session::CleanupGateMarkers()
{
	if (gateMarkers != NULL)
	{
		delete gateMarkers;
		gateMarkers = NULL;
	}
}

void Session::ResetEnemies()
{
	if (enemyTree != NULL)
	{
		rResetEnemies(enemyTree->startNode);
	}
	

	activeEnemyList = NULL;
	activeEnemyListTail = NULL;

	ClearEffects();
}

//please get rid of this soon lol
void Session::rResetEnemies(QNode *node)
{
	if (node->leaf)
	{
		LeafNode *n = (LeafNode*)node;

		//cout << "\t\tstarting leaf reset: " << endl;
		for (int i = 0; i < n->objCount; ++i)
		{
			//cout << "\t\t\tresetting enemy " << i << endl;
			Enemy * e = (Enemy*)(n->entrants[i]);

			if (e->spawned)
			{
				e->InitOnRespawn();
			}
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
		for (int i = 0; i < 4; ++i)
		{
			//	cout << "\tresetting child: " << i << endl;
			rResetEnemies(n->children[i]);
		}

		for (list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it)
		{
			Enemy * e = (Enemy*)(*it);

			if (e->spawned)
			{
				e->InitOnRespawn();
			}

			e->Reset();
		}

	}
}


bool Session::PlayerIsMovingLeft(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		return p->IsMovingLeft();
	}
	else
	{
		return false;
	}
}

bool Session::PlayerIsMovingRight(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		return p->IsMovingRight();
	}
	else
	{
		return false;
	}
}

bool Session::PlayerIsFacingRight(int index)
{
	Actor *p = GetPlayer(index);
	if (p != NULL)
	{
		return p->facingRight;
	}
	else
	{
		return false;
	}
}

bool Session::GGPORunGameModeUpdate()
{
	//if (players[0]->currInput.leftStickPad != players[1]->currInput.leftStickPad)
	//{
	//	cout << "p0:: L: " << (int)players[0]->currInput.LLeft() << ", R: " << (int)players[0]->currInput.LRight() << ", p1:: L: " << (int)players[1]->currInput.LLeft() << ", " << (int)players[1]->currInput.LRight() << endl;
	//	cout << "different inputs" << endl;
	//}


	collider.ClearDebug();

	if (!OneFrameModeUpdate())
	{
		return true;
	}

	if (!RunPreUpdate())
		return true;

	if (pauseFrames > 0)
	{
		HitlagUpdate(); //the full update while in hitlag
		ggpo_advance_frame(ggpo);
		return true;
	}

	//RepPlayerUpdateInput();

	//RecPlayerRecordFrame();

	

	ActiveSequenceUpdate();
	if (switchGameState)
		return true;

	UpdatePlayersPrePhysics();

	TryToActivateBonus();

	ActiveStorySequenceUpdate();

	UpdateInputVis();
	if (!playerAndEnemiesFrozen)
	{
		UpdateEnemiesPrePhysics();

		UpdatePhysics();
	}

	if (!playerAndEnemiesFrozen)
	{
		UpdatePlayersPostPhysics();
	}

	RecGhostRecordFrame();

	UpdateReplayGhostSprites();

	/*if (gameMode->CheckVictoryConditions())
	{
		ggpo_advance_frame(ggpo);
		gameMode->EndGame();

		return true;
	}*/

	if (!RunPostUpdate())
	{
		return true;
	}

	UpdatePlayerWireQuads();

	if (!playerAndEnemiesFrozen)
	{
		UpdateEnemiesPostPhysics();
	}

	UpdateGates();

	absorbParticles->Update();
	absorbDarkParticles->Update();
	absorbShardParticles->Update();

	UpdateEffects();
	UpdateEmitters();

	mainMenu->musicPlayer->Update();

	UpdateHUD();

	UpdateScoreDisplay();

	UpdateSoundNodeLists();

	UpdateGoalPulse();

	UpdateRain();

	UpdateBarriers();

	UpdateCamera();

	if (gateMarkers != NULL)
		gateMarkers->Update(&cam);

	fader->Update();
	swiper->Update();

	if (IsSessTypeGame())
	{
		background->Update(view.getCenter());
	}
	UpdateTopClouds();

	mainMenu->UpdateEffects();

	UpdateRaceFightScore();

	UpdateGoalFlow();

	QueryToSpawnEnemies();

	UpdateEnvPlants();

	QueryBorderTree(screenRect);

	QuerySpecialTerrainTree(screenRect);

	QueryFlyTerrainTree(screenRect);

	UpdateDecorSprites();
	UpdateDecorLayers();

	if (UpdateRunModeBackAndStartButtons())
	{

	}

	UpdateZones();

	UpdateEnvShaders(); //havent tested at this position. should work fine.

	totalGameFrames++;

	if (gameMode->CheckVictoryConditions())
	{
		//ggpo_advance_frame(ggpo);
		gameMode->EndGame();

		//return true;
	}

	ggpo_advance_frame(ggpo);

	

	return true;
}


//static int frameCC = 0;
//
//int r = rand();
//if (r % 2 == 0)
//{
//	input = 41;
//}
//else
//{
//	input = 0;
//}
//
//if (frameCC < 20)
//{
//
//	input = -1;
//	switch (frameCC)
//	{
//	case 0:
//	case 1:
//	case 8:
//	case 11:
//	case 12:
//	case 14:
//	case 16:
//	case 17:
//		input = 0;
//		break;
//
//	}
//
//	if (input == -1)
//		input = 41;
//}
void Session::GGPORunFrame()
{
	int disconnect_flags;
	int compressedInputs[GGPO_MAX_PLAYERS] = { 0 };

	//UpdateControllers();

	bool gccEnabled = mainMenu->gccDriverEnabled;

	if (gccEnabled)
		gcControllers = mainMenu->gccDriver->getState();


	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		GameController &con = GetController(i);
		if (gccEnabled)
			con.gcController = gcControllers[i];

		con.UpdateState();

		GetCurrInput(i) = con.GetState();
		GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
	}
	


	assert(ngs->local_player_handle != GGPO_INVALID_HANDLE);
	//int input = GetCurrInput(ngs->local_player_handle - 1).GetCompressedState();
	int input = GetCurrInput(0).GetCompressedState();
	//input = rand();
	GGPOErrorCode result = ggpo_add_local_input(ggpo, ngs->local_player_handle, &input, sizeof(input));
	//cout << "local player handle: " << ngs->local_player_handle << "\n";

	//static ControllerState lastCurr;

	if (GGPO_SUCCEEDED(result))
	{
		result = ggpo_synchronize_input(ggpo, (void*)compressedInputs, sizeof(int) * GGPO_MAX_PLAYERS, &disconnect_flags);
		if (GGPO_SUCCEEDED(result))
		{
			//GetPrevInput(1) = lastCurr;
			
			for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
			{
				GetCurrInput(i).SetFromCompressedState(compressedInputs[i]);
			}

			//lastCurr = GetCurrInput(1);

			UpdateAllPlayersInput();
			GGPORunGameModeUpdate();
			//frameCC++;
			//accumulator -= TIMESTEP;
			
		}
		
	}

	

	//draw after then loop again
}

int Session::GetSaveDataSize()
{
	int totalSize = sizeof(SaveGameState);

	totalSize += gameMode->GetNumStoredBytes();

	return totalSize;
}

bool Session::SaveState(unsigned char **buffer,
	int *len, int *checksum, int frame)
{
	/*int unusedState = 0;
	for (int i = 0; i < 10; ++i)
	{

	}*/
	players[0]->PopulateState(&currSaveState->states[0]);
	players[1]->PopulateState(&currSaveState->states[1]);
	currSaveState->totalGameFrames = totalGameFrames;
	currSaveState->activeEnemyList = activeEnemyList;
	currSaveState->activeEnemyListTail = activeEnemyListTail;
	currSaveState->inactiveEnemyList = inactiveEnemyList;
	currSaveState->pauseFrames = pauseFrames;
	currSaveState->currSuperPlayer = currSuperPlayer;
	*len = GetSaveDataSize();
	*buffer = (unsigned char *)malloc(*len);
	memset(*buffer, 0, *len);

	if (!*buffer) {
		return false;
	}
	memcpy(*buffer, currSaveState, sizeof(SaveGameState));


	unsigned char *tempBuf = *buffer;
	tempBuf += sizeof(SaveGameState);
	gameMode->StoreBytes(tempBuf);
	
	ReachEnemyBaseMode *rebm = (ReachEnemyBaseMode*)gameMode;
	//*checksum = fletcher32_checksum((short *)*buffer, *len / 2);
	int pSize = sizeof(PState);
	int offset = 0;//sizeof(SaveGameState) + sizeof(ReachEnemyBaseMode::ReachEnemyBaseModeData);//64;//sizeof(SaveGameState);
	int fletchLen = *len;//sizeof(BaseData); //*len;//sizeof(SaveGameState);//*len;//pSize;//(*len) - offset;//16;//464;//640;//sizeof(Test);//64;// pSize / 2;//pSize;//*len;//*len;//*len;//*len;//64;// pSize / 2;;//*len;//8;//(*len) - offset;
	*checksum = fletcher32_checksum((short *)((*buffer)+offset), fletchLen/2);// currSaveState->states[1].hitlagFrames;//
	return true;
}

bool Session::LoadState(unsigned char *bytes, int len)
{
	int saveSize = sizeof(SaveGameState);
	memcpy(currSaveState, bytes, saveSize);

	bytes += saveSize;
	gameMode->SetFromBytes(bytes);

	//cout << "rollback. setting frame to: " << currSaveState->totalGameFrames << " from " << totalGameFrames << "\n";
	totalGameFrames = currSaveState->totalGameFrames;
	activeEnemyList = currSaveState->activeEnemyList;
	inactiveEnemyList = currSaveState->inactiveEnemyList;
	activeEnemyListTail = currSaveState->activeEnemyListTail;
	pauseFrames = currSaveState->pauseFrames;
	currSuperPlayer = currSaveState->currSuperPlayer;
	players[0]->PopulateFromState(&currSaveState->states[0]);
	players[1]->PopulateFromState(&currSaveState->states[1]);

	return true;
}

void Session::CleanupPreLevelScene()
{
	if (preLevelScene != NULL)
	{
		delete preLevelScene;
		preLevelScene = NULL;
	}
}

void Session::CleanupPostLevelScene()
{
	if (postLevelScene != NULL)
	{
		delete postLevelScene;
		postLevelScene = NULL;
	}
}

void Session::SetupScoreDisplay()
{
	if (parentGame != NULL)
	{
		scoreDisplay = parentGame->scoreDisplay;
		scoreDisplay->Reset();
	}
	else if (scoreDisplay == NULL)
		scoreDisplay = new ScoreDisplay(Vector2f(1920, 0), mainMenu->arial);
	else
	{
		scoreDisplay->Reset();
	}
}

void Session::CleanupScoreDisplay()
{
	if (parentGame == NULL && scoreDisplay != NULL)
	{
		delete scoreDisplay;
		scoreDisplay = NULL;
	}
}

void Session::EndLevel()
{
	if (postLevelScene != NULL)
	{
		SetActiveSequence(postLevelScene);
	}
	else
	{
		if (IsSessTypeGame())
		{
			GameSession *game = GameSession::GetSession();
			game->QuitGame();
			/*quit = true;
			returnVal = resType;*/
			//goalDestroyed = true;
		}
		else
		{
			EditSession *edit = EditSession::GetSession();
			edit->EndTestMode();
		}
	}
}

int Session::GetGameMode()
{
	return mapHeader->gameMode;
}

bool Session::IsMapVersionNewerThanOrEqualTo(int ver1, int ver2)
{
	return (mapHeader->ver1 >= ver1 && mapHeader->ver2 >= ver2);
}

AdventureHUD *Session::GetAdventureHUD()
{
	if (hud != NULL && hud->hType == HUD::ADVENTURE)
	{
		return (AdventureHUD*)hud;
	}

	return NULL;
}

void Session::SetupSuperSequence()
{
	if (superSequence == NULL)
	{
		superSequence = new SuperSequence;
		superSequence->Init();
	}
}

void Session::CleanupSuperSequence()
{
	if (superSequence != NULL)
	{
		delete superSequence;
		superSequence = NULL;
	}
}

void Session::DrawPlayerShields(sf::RenderTarget *target)
{
	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawShield(target);
		}
	}
}

void Session::SetupGameMode()
{
	switch (mapHeader->gameMode)
	{
	case MapHeader::T_BASIC:
		gameMode = new BasicMode;
		break;
	case MapHeader::T_REACHENEMYBASE:
		gameMode = new ReachEnemyBaseMode;
		break;
	case MapHeader::T_FIGHT:
		gameMode = new FightMode;
		break;
	}
}

void Session::ForwardSimulatePlayer(int index, int frames)
{
	Actor *p = GetPlayer(index);
	assert(p != NULL);

	p->PopulateState(playerSimState);
	p->simulationMode = true;
	for (int i = 0; i < frames; ++i)
	{
		p->UpdatePrePhysics();
		p->physicsOver = false;
		for (substep = 0; substep < NUM_MAX_STEPS; ++substep)
		{
			if (substep == 0 || p->highAccuracyHitboxes)
				p->UpdatePhysics();
		}
		p->UpdatePostPhysics();
		UpdatePlayerInput(index);
	}
	p->simulationMode = false;
}

void Session::RevertSimulatedPlayer(int index)
{
	Actor *p = GetPlayer(index);
	assert(p != NULL);

	p->PopulateFromState(playerSimState);
}

Enemy * Session::GetEnemy(int enType)
{
	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		if ((*it)->type == enType)
		{
			return (*it);
		}
	}
	return NULL;
}

GroundedWarper *Session::GetWarper(const std::string levelWarp)
{
	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		if ((*it)->type == EN_GROUNDEDWARPER)
		{
			GroundedWarper *gw = (GroundedWarper*)(*it);
			if (levelWarp == gw->bonusName)
			{
				return gw;
			}
		}
	}

	return NULL;
}

int Session::GetPlayerEnemiesKilledLastFrame(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->enemiesKilledLastFrame;
	}
	else
	{
		return 0;
	}
}

void Session::PlayerRestoreDoubleJump(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->RestoreDoubleJump();
	}
}

void Session::PlayerRestoreAirDash(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->RestoreAirDash();
	}
}

void Session::DrawQueriedTerrain(sf::RenderTarget *target)
{
	PolyPtr poly = polyQueryList;
	while (poly != NULL)
	{
		poly->Draw(preScreenTex);
		poly = poly->queryNext;
	}
}

void Session::DrawQueriedSpecialTerrain(sf::RenderTarget *target)
{
	PolyPtr sp = specialPieceList;
	while (sp != NULL)
	{
		sp->Draw(preScreenTex);
		sp = sp->queryNext;
	}
}

void Session::DrawQueriedRails(sf::RenderTarget *target)
{
	RailPtr r = railDrawList;
	while (r != NULL)
	{
		railDrawList->Draw(target);
		RailPtr next = railDrawList->queryNext;
		railDrawList->queryNext = NULL;
		railDrawList = next;
	}
}