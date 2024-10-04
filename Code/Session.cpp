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
#include "PowerSequence.h"
#include "Sequence.h"
#include "ParticleEffects.h"
#include "DeathSequence.h"
#include "TopClouds.h"
#include "StorySequence.h"
#include "GoalFlow.h"
#include "GoalExplosion.h"
#include "InputVisualizer.h"
#include "PlayerRecord.h"
//#include "Enemy_Shard.h"
#include "EnvEffects.h"
#include "MusicPlayer.h"

#include "ShardMenu.h"
#include "LogMenu.h"

//enemy stuff:
#include "SoundManager.h"
#include "GGPO.h"
#include "NetplayManager.h"

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

#include "steam\steam_api.h"
#include "ggpo\network\udp_msg.h"
#include "ControlProfile.h"

#include "ReplayHUD.h"
#include "AlertBox.h"
#include "NameTag.h"

#include "PracticeInviteDisplay.h"
#include "BasicTextMenu.h"
#include <assert.h>
#include "AdventureManager.h"
#include "Config.h"

#include "AdventureScoreDisplay.h"
#include "RushManager.h"
#include "RushScoreDisplay.h"
#include "ShipTravelSequence.h"
#include "EnvParticleSystem.h"

//#include "ggpo\backends\backend.h"

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

		if (soundNodeList != NULL)
		{
			soundNodeList->Clear();
		}
		
		if (pauseSoundNodeList != NULL)
		{
			pauseSoundNodeList->Clear();
		}
	}
	else if (soundNodeList == NULL)
	{
		//parallel sessions shouldn't use sounds at all. making a pool for each session overloads the system resources.
		if (IsParallelSession())
		{
			soundNodeList = NULL;
			pauseSoundNodeList = NULL;
		}
		else
		{
			//want plenty of available sounds but don't overload the system. used to be 100 and 100 and that was too much
			soundNodeList = new SoundNodeList(20);
			pauseSoundNodeList = new SoundNodeList(20);
		}
		
	}
	else
	{
		if (soundNodeList != NULL)
		{
			soundNodeList->Clear();
		}

		if (pauseSoundNodeList != NULL)
		{
			pauseSoundNodeList->Clear();
		}
	}
}

void Session::SetParentGame(GameSession *game)
{
	parentGame = game;
	SetParentTilesetManager(game);
}

void Session::SetPlayerOptionField(int pIndex)
{
	GetPlayer(pIndex)->SetAllOptions(defaultStartingPlayerOptionsField);
}

void Session::SetupEnemyType(ParamsInfo &pi, bool unlisted )
{
	if (types[pi.name] == NULL)
	{
		types[pi.name] = new ActorType(pi, unlisted );
		orderedTypeList.push_back(types[pi.name]);
	}
}

void Session::RegisterGeneralEnemies()
{
	bool isEdit = IsSessTypeEdit();

	Tileset *ts_zoneProperties = NULL;
	Tileset *ts_camShot = NULL;
	Tileset *ts_poi = NULL;
	Tileset *ts_xBarrier = NULL;
	Tileset *ts_xBarrierWarp = NULL;
	Tileset *ts_ship = NULL;

	if (isEdit)
	{
		ts_zoneProperties = GetSizedTileset("Editor/zoneproperties_128x128.png");
		ts_camShot = GetTileset("Editor/camera_128x128.png", 128, 128);
		ts_poi = GetSizedTileset("Editor/pointofinterest_32x32.png");
		ts_xBarrier = GetSizedTileset("Enemies/General/blocker_w1_192x192.png");
		ts_xBarrierWarp = GetSizedTileset("Enemies/Multiplayer/target_224x224.png");
		ts_ship = GetSizedTileset("Ship/ship_open_864x410.png");
	}


	int normalRow = 0;
	int objectRow = 1;
	int powerRow = 2;
	int utilityRow = 3;
	int multiRow = 4;

	//normal gameplay
	AddExtraEnemy("Goal", "goal", normalRow, CreateEnemy<Goal>, SetParamsType<BasicGroundEnemyParams>, Vector2i(0, -32), Vector2i(200, 200),
		false, false, false, false, false, true, false);
	AddExtraEnemy("Key", "key", normalRow, CreateEnemy<KeyFly>, SetParamsType<BasicAirEnemyParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false);
	AddExtraEnemy("Booster", "booster", normalRow, CreateEnemy<Booster>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);
	AddExtraEnemy("Currency Item", "currencyitem", normalRow, CreateEnemy<CurrencyItemChain>, SetParamsType<CurrencyItemParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);
	AddExtraEnemy("Blocker", "blocker", normalRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	//objects
	AddExtraEnemy("Shard", "shard", objectRow, CreateEnemy<Shard>, SetParamsType<ShardParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddExtraEnemy("Log", "log", objectRow, CreateEnemy<LogItem>, SetParamsType<LogParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddExtraEnemy("Ship Pickup", "shippickup", objectRow, CreateEnemy<ShipPickup>, SetParamsType<ShipPickupParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false, true, false, 1);
	AddExtraEnemy("Entrance Ship", "ship", objectRow, NULL, SetParamsType<BasicAirEnemyParams>, Vector2i(0, 0), Vector2i(864, 400), false, false, false, false, true, false, false, 1,
		ts_ship);
	AddExtraEnemy("Tutorial Object", "tutorialobject", objectRow, CreateEnemy<TutorialObject>, SetParamsType<TutorialObjectParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddExtraEnemy("Family Picture (Inspect)", "inspectfamilypicture", objectRow, CreateEnemy<InspectObject>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddExtraEnemy("Cockpit (Inspect)", "inspectcockpit", objectRow, CreateEnemy<InspectObject>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	//AddExtraEnemy("Ship Travel", "shiptravel", objectRow, NULL, SetParamsType<BasicAirEnemyParams>, Vector2i(0, 0), Vector2i(864, 400), false, false, false, false, true, false, false, 1,
	//	GetSizedTileset( "Ship/ShipTest/travel1_1725x921.png" ));


	AddExtraEnemy("Airdash", "airdashpoweritem", powerRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false );
	AddExtraEnemy("Gravity\nReverse", "gravpoweritem", powerRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	AddExtraEnemy("Bounce\nScorpion", "bouncepoweritem", powerRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	AddExtraEnemy("Grind\nWheel", "grindpoweritem", powerRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	AddExtraEnemy("Time Slow\nBubbles", "timeslowpoweritem", powerRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	AddExtraEnemy("Double\nWires", "doublewirepoweritem", powerRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	/*AddExtraEnemy("Red Wire", "redwirepoweritem", objectRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);
	AddExtraEnemy("Blue Wire", "bluewirepoweritem", objectRow, CreateEnemy<PowerItem>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false);*/

	
	//utility
	

	AddExtraEnemy("Zone Properties", "zoneproperties", utilityRow, NULL, SetParamsType<ZonePropertiesParams>, Vector2i(0, 0),
		Vector2i(128, 128), false, false, false, false, true, false, false, 1,
		ts_zoneProperties);
	AddExtraEnemy("Sequence Camera", "camerashot", utilityRow, NULL, SetParamsType<CameraShotParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1,
		ts_camShot );
	AddExtraEnemy("Point of Interest", "poi", utilityRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_poi);

	AddExtraEnemy("X Barrier", "xbarrier", utilityRow, NULL, SetParamsType<XBarrierParams>, Vector2i(0, 0), Vector2i(64, 64), false, false, false, false, true, false, false, 1,
		ts_xBarrier );
	AddExtraEnemy("X Warp Barrier", "xbarrierwarp", utilityRow, NULL, SetParamsType<XBarrierParams>, Vector2i(0, 0), Vector2i(64, 64), false, false, false, false, true, false, false, 1,
		ts_xBarrierWarp );
	//AddExtraEnemy("Air Trigger", "airtrigger", utilityRow, NULL, SetParamsType<AirTriggerParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1,
	//	GetSizedTileset("Enemies/jayshield_128x128.png"));
	AddExtraEnemy("Grounded Warper", "groundedwarper", utilityRow, CreateEnemy<GroundedWarper>, SetParamsType<GroundedWarperParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, false,
		true, false);

	//multiplayer
	AddExtraEnemy("Multiplayer Progress Target", "multiplayerprogresstarget", multiRow, CreateEnemy<MultiplayerProgressTarget>, SetParamsType<BasicAirEnemyParams>,
		Vector2i(), Vector2i(32, 32), false, false, false, false, true,
		false, false, 1);
	
}

void Session::RegisterW1Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int comboerRow = 2;
	int targetRow = 3;
	int storyRow = 4;

	//unlisted
	AddUnlistedEnemy("queenfloatingbomb", CreateEnemy<QueenFloatingBomb>);

	//enemy
	
	AddBasicGroundWorldEnemy("Crawler", "crawler", 1, enemyRow, CreateEnemy<Crawler>, Vector2i(0, 0), Vector2i(100, 100), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Patroller", "patroller", 1, enemyRow, CreateEnemy<Patroller>, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3);
	AddBasicAerialWorldEnemy("Bee", "airdasher", 1, enemyRow, CreateEnemy<Airdasher>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Jelly Shroom", "shroom", 1, enemyRow, CreateEnemy<Shroom>, Vector2i(0, 0), Vector2i(50, 50), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Basic Turret", "basicturret", 1, enemyRow, CreateEnemy<BasicTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	
	//item
	AddWorldEnemy("Launcher", "spring", 1, itemRow, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false, 1);

	//comboer
	AddBasicAerialWorldEnemy("Basic\nComboer", "comboer", 1, comboerRow, CreateEnemy<Comboer>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	//AddBasicAerialWorldEnemy("Split\nComboer", "splitcomboer", 1, comboerRow, CreateEnemy<SplitComboer>, Vector2i(0, 0), Vector2i(32, 32), false, true, true, true, 3);

	//target
	AddBasicAerialWorldEnemy("Blue\nComboer\nTarget", "bluecomboertarget", 1, targetRow, CreateEnemy<ComboerTarget>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, 1);
	AddWorldEnemy("Blue\nBlocker", "blueblocker", 1, targetRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Regenerating\nTarget", "regentarget", 1, targetRow, CreateEnemy<RegenTarget>,
		Vector2i(0, 0), Vector2i(32, 32), true, false, false, false);

	bool isEdit = IsSessTypeEdit();

	Tileset *ts_crawlerNode = NULL;

	if (isEdit)
	{
		ts_crawlerNode = GetSizedTileset("Editor/crawlernode_32x32.png");
	}

	//story
	AddWorldEnemy("Crawler Boss Node", "crawlernode", 1, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_crawlerNode);
	AddBasicAerialWorldEnemy("Movement Tester", "movementtester", 1, storyRow, CreateEnemy<MovementTester>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);
	AddBasicGroundWorldEnemy("Crawler Queen Boss", "crawlerqueen", 1, storyRow, CreateEnemy<CrawlerQueen>, Vector2i(0, 0), Vector2i(100, 100), false, true, false, false, 2);
	AddBasicGroundWorldEnemy("Sequence Crawler Queen", "sequencecrawler", 1, storyRow, CreateEnemy<SequenceCrawler>, Vector2i(0, 0), Vector2i(100, 100), false, true, false, false, 1);
	AddBasicGroundWorldEnemy("Blue Nexus", "nexus1", 1, storyRow, CreateEnemy<Nexus>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);
}

void Session::RegisterW2Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int comboerRow = 2;
	int targetRow = 3;
	int storyRow = 4;

	//unlisted

	//enemies
	//AddBasicGroundWorldEnemy("Green\nGoal", "greengoal", 2, enemyRow, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);
	AddBasicAerialWorldEnemy("Bat", "bat", 2, enemyRow, CreateEnemy<Bat>, Vector2i(0, 0), Vector2i(32, 32), true, true, true, true, 3);
	AddBasicGroundWorldEnemy("Lob\nTurret", "lobturret", 2, enemyRow, CreateEnemy<LobTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Poison\nFrog", "poisonfrog", 2, enemyRow, CreateEnemy<PoisonFrog>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Stag", "stagbeetle", 2, enemyRow, CreateEnemy<StagBeetle>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Gravity\nFaller", "gravityfaller", 2, enemyRow, CreateEnemy<GravityFaller>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//items
	AddWorldEnemy("Glide\nLauncher", "glidespring", 2, itemRow, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Gravity/nIncreaser", "gravityincreaser", 2, itemRow, CreateEnemy<GravityModifier>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Gravity/nDecreaser", "gravitydecreaser", 2, itemRow, CreateEnemy<GravityModifier>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddWorldEnemy("Gravity\nIncreaser", "gravityincreaser", 2, itemRow, CreateEnemy<GravityModifier>, SetParamsType<GravityModifierParams>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, true, false, false);
	//AddWorldEnemy("Gravity\nDecreaser", "gravitydecreaser", 2, itemRow, CreateEnemy<GravityModifier>, SetParamsType<GravityModifierParams>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, true, false, false);

	//comboers

	AddWorldEnemy("Gravity\nJuggler", "downgravityjuggler", 2, comboerRow, CreateEnemy<GravityJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddWorldEnemy("Up Gravity\nJuggler", "upgravityjuggler", 2, comboerRow, CreateEnemy<GravityJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	
	//targets
	AddBasicAerialWorldEnemy("Green\nComboer\nTarget", "greencomboertarget", 2, targetRow, CreateEnemy<ComboerTarget>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, 1);
	AddBasicAerialWorldEnemy("Glide\nTarget", "glidetarget", 2, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1);
	AddWorldEnemy("Green\nBlocker", "greenblocker", 2, targetRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	//story

	bool isEdit = IsSessTypeEdit();

	Tileset *ts_birdNode = NULL;

	if (isEdit)
	{
		ts_birdNode = GetSizedTileset("Editor/birdnode_32x32.png");
	}

	AddBasicAerialWorldEnemy("Bird Boss", "bird", 2, storyRow, CreateEnemy<Bird>, Vector2i(0, 0), Vector2i(200, 200), false, true, false, false, 3);
	AddBasicGroundWorldEnemy("Sequence Bird", "sequencebird", 2, storyRow, CreateEnemy<SequenceBird>, Vector2i(0, 0), Vector2i(200, 200), false, true, false, false, 1);
	AddWorldEnemy("Bird Boss Node", "birdnode", 2, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_birdNode);
	AddBasicGroundWorldEnemy("Green Nexus", "nexus2", 2, storyRow, CreateEnemy<Nexus>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false);

	//AddBasicGroundWorldEnemy("reversepoisonfrog", 2, CreateEnemy<PoisonFrog>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicGroundWorldEnemy("reversestagbeetle", 2, CreateEnemy<StagBeetle>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
}

void Session::RegisterW3Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int comboerRow = 2;
	int targetRow = 3;
	int storyRow = 4;

	//unlisted
	AddUnlistedWorldEnemy("babyscorpion", 3, CreateEnemy<BabyScorpion>, NULL, Vector2i(), Vector2i(), false, true, false, false, true, false, false, 3);

	//enemies
	AddBasicGroundWorldEnemy("Badger", "badger", 3, enemyRow, CreateEnemy<Badger>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Roadrunner", "roadrunner", 3, enemyRow, CreateEnemy<Roadrunner>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Owl", "owl", 3, enemyRow, CreateEnemy<Owl>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Air Mine", "explodingbarrel", 3, enemyRow, CreateEnemy<ExplodingBarrel>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Firefly", "firefly", 3, enemyRow, CreateEnemy<Firefly>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Shotgun\nTurret", "shotgunturret", 3, enemyRow, CreateEnemy<ShotgunTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//items
	AddWorldEnemy("Aim Launcher", "aimlauncher", 3, itemRow, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddWorldEnemy("Air Bounce\nLauncher", "airbouncelauncher", 3, itemRow, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Up\nBounce\nBooster", "upbouncebooster", 3, itemRow, CreateEnemy<BounceBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Omni\nBounce\nBooster", "omnibouncebooster", 3, itemRow, CreateEnemy<BounceBooster>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Bounce\nFloater", "bouncefloater", 3, itemRow, CreateEnemy<BounceFloater>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//comboers
	AddWorldEnemy("Ball", "ball", 3, comboerRow, CreateEnemy<Ball>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Bounce\Comboer", "bouncecomboer", 3, comboerRow, CreateEnemy<BounceJuggler>, Vector2i(0, 0), Vector2i(128, 128), false, true, false, false, 3);
	//AddWorldEnemy("Bounce\Comboer", "bouncecomboer", 3, comboerRow, CreateEnemy<BounceJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	//targets
	AddBasicAerialWorldEnemy("Yellow\nComboer\nTarget", "yellowcomboertarget", 3, targetRow, CreateEnemy<ComboerTarget>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, 1);
	AddBasicAerialWorldEnemy("Bounce\nLaunch\nTarget", "bouncetarget", 3, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1);
	AddWorldEnemy("Yellow\nBlocker", "yellowblocker", 3, targetRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	//AddBasicGroundWorldEnemy("cactus", 3, CreateEnemy<Cactus>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	bool isEdit = IsSessTypeEdit();

	Tileset *ts_coyNode = NULL;

	if (isEdit)
	{
		ts_coyNode = GetSizedTileset("Editor/coyotenode_32x32.png");
	}

	//story
	AddBasicGroundWorldEnemy("Coyote Boss", "coyote", 3, storyRow, CreateEnemy<Coyote>, Vector2i(0, 0), Vector2i(80, 80), false, false, false, false);
	AddBasicGroundWorldEnemy("Sequence Coyote", "sequencecoyote", 3, storyRow, CreateEnemy<SequenceCoyote>, Vector2i(0, 0), Vector2i(80, 80), false, false, false, false);
	AddWorldEnemy("Coyote Boss Node", "coyotenode", 3, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_coyNode);
}

void Session::RegisterW4Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int comboerRow = 2;
	int targetRow = 3;
	int storyRow = 4;

	//unlisted
	AddUnlistedWorldEnemy("tigertarget", 4, CreateEnemy<TigerTarget>, NULL, Vector2i(), Vector2i(), false, true, false, false, true, false, false, 3);
	AddUnlistedWorldEnemy("tigerspinturret", 4, CreateEnemy<TigerSpinTurret>, NULL, Vector2i(), Vector2i(), false, true, false, false, true, false, false, 3);

	//enemies
	//AddBasicGroundWorldEnemy("Spider", "spider", 4, enemyRow, CreateEnemy<Spider>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicGroundWorldEnemy("Widow", "widow", 4, enemyRow, CreateEnemy<Widow>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Turtle", "turtle", 4, enemyRow, CreateEnemy<Turtle>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Pufferfish", "pufferfish", 4, enemyRow, CreateEnemy<Pufferfish>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Cheetah", "cheetah", 4, enemyRow, CreateEnemy<Cheetah>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Palm Turret", "palmturret", 4, enemyRow, CreateEnemy<PalmTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Falcon", "falcon", 4, enemyRow, CreateEnemy<Falcon>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//items
	//CreateEnemy<Spring>, SetParamsType<SpringParams>
	AddWorldEnemy("Grind\nLauncher", "grindlauncher", 4, itemRow, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	//AddWorldEnemy("Teleport\nLauncher", "teleporter", 4, itemRow, CreateEnemy<Teleporter>, SetParamsType<TeleporterParams>, Vector2i(0, 0), Vector2i(32, 32),
	//	false, false, false, false, true, false, false, 1);
	//AddWorldEnemy("2-way Teleport\nLauncher", "doubleteleporter", 4, itemRow, CreateEnemy<Teleporter>, SetParamsType<TeleporterParams>, Vector2i(0, 0), Vector2i(32, 32),
	//	false, false, false, false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Momentum\nBooster", "momentumbooster", 4, itemRow, CreateEnemy<MomentumBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Phase\nSwitch", "phaseswitch", 4, itemRow, CreateEnemy<PhaseSwitch>, Vector2i(0, 0), Vector2i(100, 100), true, false, false, false, 1);

	//comboers
	//AddBasicAerialWorldEnemy("Grind\nComboer", "grindcomboer", 4, comboerRow, CreateEnemy<GrindJuggler>, Vector2i(0, 0), Vector2i(128, 128), true, false, false, false);
	//AddWorldEnemy("Grind\nJuggler", "grindjuggler", 4, comboerRow, CreateEnemy<GrindJuggler>, SetParamsType<GrindJugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddWorldEnemy("Grind\Comboer", "grindcomboer", 4, comboerRow, CreateEnemy<GrindJuggler>, SetParamsType<GrindJugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddWorldEnemy("Grounded\nGrind\nJuggler", "groundedgrindjuggler", 4, comboerRow, CreateEnemy<GroundedGrindJuggler>, SetParamsType<GroundedGrindJugglerParams>, Vector2i(0, 0), Vector2i(128, 128), 
		true, true, false, false, false, true, false, 3);

	//targets
	AddBasicAerialWorldEnemy("Orange\nComboer\nTarget", "orangecomboertarget", 4, targetRow, CreateEnemy<ComboerTarget>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, 1);
	AddBasicAerialWorldEnemy("Scorpion\nTarget", "scorpiontarget", 4, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Grind\nTarget", "grindtarget", 4, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1);

	AddWorldEnemy("Orange\nBlocker", "orangeblocker", 4, targetRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	bool isEdit = IsSessTypeEdit();

	Tileset *ts_tigerNode = NULL;

	if (isEdit)
	{
		ts_tigerNode = GetSizedTileset("Editor/tigernode_32x32.png");
	}

	//story
	AddBasicGroundWorldEnemy("Boss Tiger", "tiger", 4, storyRow, CreateEnemy<Tiger>, Vector2i(0, 0), Vector2i(80, 80), false, true, false, false, 2);
	AddBasicGroundWorldEnemy("Sequence Tiger", "sequencetiger", 4, storyRow, CreateEnemy<SequenceTiger>, Vector2i(0, 0), Vector2i(80, 80), false, true, false, false, 1);
	AddWorldEnemy("Boss Tiger Node", "tigernode", 4, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_tigerNode);
}

void Session::RegisterW5Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int comboerRow = 2;
	int targetRow = 3;
	int storyRow = 4;

	//unlisted

	//enemies
	AddBasicAerialWorldEnemy("Swarm", "swarm", 5, enemyRow, CreateEnemy<Swarm>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicAerialWorldEnemy("Shark", "shark", 5, enemyRow, CreateEnemy<Shark>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Growing Tree", "growingtree", 5, enemyRow, CreateEnemy<GrowingTree>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Plasmid", "ghost", 5, enemyRow, CreateEnemy<Ghost>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Lizard", "lizard", 5, enemyRow, CreateEnemy<Lizard>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Parrot", "parrot", 5, enemyRow, CreateEnemy<Parrot>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//items
	AddWorldEnemy("Homing\nLauncher", "hominglauncher", 5, itemRow, CreateEnemy<AimLauncher>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Time Slow\nBooster", "timebooster", 5, itemRow, CreateEnemy<TimeBooster>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Homing\nBooster", "homingbooster", 5, itemRow, CreateEnemy<HomingBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Anti\nTime Slow\nBooster", "antitimeslowbooster", 5, itemRow, CreateEnemy<AntiTimeSlowBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	//AddWorldEnemy("limitedrelativecomboer", 5, CreateEnemy<RelativeComboer>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	//comboers
	//AddWorldEnemy("Hungry\nComboer", "hungrycomboer", 5, comboerRow, CreateEnemy<HungryComboer>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Hungry\nComboer", "hungrycomboer", 5, comboerRow, CreateEnemy<HungryComboer>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3);
	//AddWorldEnemy("Hungry\nReturn\nComboer", "hungryreturncomboer", 5, comboerRow, CreateEnemy<HungryComboer>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);


	AddWorldEnemy("Relative\nComboer", "relativecomboer", 5, comboerRow, CreateEnemy<RelativeComboer>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddWorldEnemy("Relative\nDetach\nComboer", "relativecomboerdetach", 5, comboerRow, CreateEnemy<RelativeComboer>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	//targets
	AddBasicAerialWorldEnemy("Red\nComboer\nTarget", "redcomboertarget", 5, targetRow, CreateEnemy<ComboerTarget>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, 1);
	AddBasicAerialWorldEnemy("Homing\nTarget", "homingtarget", 5, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1 );
	AddWorldEnemy("Red\nBlocker", "redblocker", 5, targetRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	bool isEdit = IsSessTypeEdit();

	Tileset *ts_gatorNode = NULL;

	if (isEdit)
	{
		ts_gatorNode = GetSizedTileset("Editor/gatornode_32x32.png");
	}

	//story
	AddWorldEnemy("Gator Boss Node", "gatornode", 5, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_gatorNode);
	AddBasicAerialWorldEnemy("Gator Boss", "gator", 5, storyRow, CreateEnemy<Gator>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
	AddBasicAerialWorldEnemy("Sequence Gator", "sequencegator", 5, storyRow, CreateEnemy<SequenceGator>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
}

void Session::RegisterW6Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int comboerRow = 2;
	int targetRow = 3;
	int storyRow = 4;

	//unlisted

	//enemies
	AddBasicAerialWorldEnemy("Gorilla", "gorilla", 6, enemyRow, CreateEnemy<Gorilla>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Specter", "specter", 6, enemyRow, CreateEnemy<Specter>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Laser Wolf", "laserwolf", 6, enemyRow, CreateEnemy<LaserWolf>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicAerialWorldEnemy("ChessX", "chessx", 6, enemyRow, CreateEnemy<Chess>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicAerialWorldEnemy("ChessY", "chessy", 6, enemyRow, CreateEnemy<Chess>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicAerialWorldEnemy("Chess Diag Down Right", "chessdiagdownright", 6, enemyRow, CreateEnemy<Chess>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicAerialWorldEnemy("chess Diag Up Right", "chessdiagupright", 6, enemyRow, CreateEnemy<Chess>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	
	//AddBasicAerialWorldEnemy("Laser Jays", "laserjays", 6, enemyRow, CreateEnemy<LaserJays>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicAerialWorldEnemy("Tethered Rusher", "tetheredrusher", 6, enemyRow, CreateEnemy<TetheredRusher>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Dragon", "dragon", 6, enemyRow, CreateEnemy<Dragon>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicGroundWorldEnemy("Predict Turret", "predictturret", 6, enemyRow, CreateEnemy<PredictTurret>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Skunk", "skunk", 6, enemyRow, CreateEnemy<Skunk>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	//AddBasicGroundWorldEnemy("Copycat", "copycat", 6, enemyRow, CreateEnemy<Copycat>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);


	//items
	AddBasicAerialWorldEnemy("Free\nFlight\nBooster", "freeflightbooster", 6, itemRow, CreateEnemy<FreeFlightBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddWorldEnemy("Swing\nLauncher", "swinglauncher", 6, itemRow, CreateEnemy<SwingLauncher>, SetParamsType<SwingLauncherParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	//AddWorldEnemy("Swing Launcher CCW", "swinglauncherccw", 6, itemRow, CreateEnemy<SwingLauncher>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	
	//comboers
	//AddWorldEnemy("Blue\nWire\nJuggler", "bluewirejuggler", 6, comboerRow, CreateEnemy<WireJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Red\nWire\nJuggler", "redwirejuggler", 6, comboerRow, CreateEnemy<WireJuggler>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Blue\nWire\nJuggler", "bluewirejuggler", 6, comboerRow, CreateEnemy<WireJuggler>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, 3);
	//AddWorldEnemy("Red\nWire\nJuggler", "redwirejuggler", 6, comboerRow, CreateEnemy<WireJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	//AddWorldEnemy("Magenta\nWire\nJuggler", "magentawirejuggler", 6, comboerRow, CreateEnemy<WireJuggler>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);
	AddWorldEnemy("Wire Ball", "wireball", 6, comboerRow, CreateEnemy<WireBall>, SetParamsType<JugglerParams>, Vector2i(0, 0), Vector2i(128, 128), true, true, false, false, true, false, false, 3);

	//targets
	AddBasicAerialWorldEnemy("Magenta\nComboer\nTarget", "magentacomboertarget", 6, targetRow, CreateEnemy<ComboerTarget>, Vector2i(0, 0), Vector2i(32, 32), true, false, false, false, 1);
	AddBasicAerialWorldEnemy("Free\nFlight\nTarget", "freeflighttarget", 6, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Swing\nTarget", "swingtarget", 6, targetRow, CreateEnemy<SpecialTarget>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 1);
	AddWorldEnemy("Magenta\nBlocker", "magentablocker", 6, targetRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	bool isEdit = IsSessTypeEdit();

	Tileset *ts_skeleNode = NULL;

	if (isEdit)
	{
		ts_skeleNode = GetSizedTileset("Editor/skeletonnode_32x32.png");
	}

	//story
	AddWorldEnemy("Skeleton Boss Node", "skeletonnode", 6, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_skeleNode);
	AddBasicGroundWorldEnemy("Skeleton Boss", "skeleton", 6, storyRow, CreateEnemy<Skeleton>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
	AddBasicGroundWorldEnemy("Sequence Skeleton", "sequenceskeleton", 6, storyRow, CreateEnemy<SequenceSkeleton>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
	AddBasicGroundWorldEnemy("Coyote Helper", "coyotehelper", 6, storyRow, CreateEnemy<CoyoteHelper>, Vector2i(0, 0), Vector2i(80, 80), false, false, false, false);
	AddWorldEnemy("Scorpion Launcher", "scorpionlauncher", 6, storyRow, CreateEnemy<ScorpionLauncher>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, false, false, 1);
}

void Session::RegisterW7Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int storyRow = 2;

	//unlisted

	//enemies
	AddBasicAerialWorldEnemy("Trailer", "trailer", 7, enemyRow, CreateEnemy<Trailer>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicGroundWorldEnemy("Copycat", "copycat", 7, enemyRow, CreateEnemy<Copycat>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);
	AddBasicAerialWorldEnemy("Future Checker", "futurechecker", 7, enemyRow, CreateEnemy<FutureChecker>, Vector2i(0, 0), Vector2i(32, 32), true, true, false, false, 3);

	//items
	AddBasicAerialWorldEnemy("Dimension Eye", "dimensioneye", 7, itemRow, CreateEnemy<DimensionEye>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);
	AddBasicAerialWorldEnemy("Dimension Exit", "dimensionexit", 7, itemRow, CreateEnemy<DimensionExit>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, 1);
	AddWorldEnemy("Annihilation Launcher", "annihilationspring", 7, itemRow, CreateEnemy<Spring>, SetParamsType<SpringParams>, Vector2i(0, 0), Vector2i(32, 32), false, false, false, false, true, false, false, 1);
	AddBasicAerialWorldEnemy("Sword Projectile Booster", "swordprojectilebooster", 7, itemRow, CreateEnemy<SwordProjectileBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Enemy Sword Projectile Booster", "enemyswordprojectilebooster", 7, itemRow, CreateEnemy<SwordProjectileBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Rewind Booster", "rewindbooster", 7, itemRow, CreateEnemy<RewindBooster>, Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Attract Juggler", "attractjuggler", 7, itemRow, CreateEnemy<AttractJuggler>, Vector2i(0, 0), Vector2i(128, 128), false, true, false, false, 3);
	AddBasicAerialWorldEnemy("Remote Comboer", "remotecomboer", 7, itemRow, CreateEnemy<RemoteComboer>, Vector2i(0, 0), Vector2i(128, 128), false, true, false, false, 3);
	
	bool isEdit = IsSessTypeEdit();

	Tileset *ts_greyNode = NULL;

	if (isEdit)
	{
		ts_greyNode = GetSizedTileset("Editor/skeleton2node_32x32.png");
	}


	//story
	AddWorldEnemy("Grey Skeleton Boss Node", "greyskeletonnode", 7, storyRow, NULL, SetParamsType<PoiParams>, Vector2i(0, 0), Vector2i(32, 32),
		false, false, false, false, true, true, false, 1, ts_greyNode );
	AddBasicAerialWorldEnemy("Grey Skeleton Boss", "greyskeleton", 7, storyRow, CreateEnemy<GreySkeleton>, Vector2i(0, 0), Vector2i(200, 200), false, false, false, false);
}

void Session::RegisterW8Enemies()
{
	int enemyRow = 0;
	int itemRow = 1;
	int storyRow = 2;

	//unlisted

	//enemies

	//items
	AddWorldEnemy("Black Blocker", "blackblocker", 8, itemRow, CreateEnemy<BlockerChain>, SetParamsType<BlockerParams>,
		Vector2i(0, 0), Vector2i(32, 32), false, true, false, false, true, false, false, 3);

	//story

	
	//AddBasicGroundWorldEnemy("blackgoal", 8, CreateEnemy<Goal>, Vector2i(0, -32), Vector2i(200, 200), false, false, false, false, 1);
}

void Session::RegisterAllEnemies()
{
	RegisterGeneralEnemies();
	RegisterW1Enemies();
	/*RegisterW2Enemies();
	RegisterW3Enemies();
	RegisterW4Enemies();
	RegisterW5Enemies();
	RegisterW6Enemies();
	RegisterW7Enemies();
	RegisterW8Enemies();*/
}

void Session::AddUnlistedEnemy(const std::string &name, EnemyCreator *p_enemyCreator)
{
	ParamsInfo testPI(name, name, 0, p_enemyCreator, NULL,
		Vector2i(), Vector2i(), false, false, false, false, false,
		false, false);

	SetupEnemyType(testPI, true);
}


void Session::AddBasicGroundWorldEnemy(
	const std::string &displayName,
	const std::string &name,
	int w,
	int row,
	EnemyCreator *p_enemyCreator,
	Vector2i &off, Vector2i &size, bool w_mon,
	bool w_level, bool w_path, bool w_loop, int p_numLevels, Tileset *ts, int tileIndex)
{
	AddWorldEnemy( displayName, name, w, row, p_enemyCreator, SetParamsType<BasicGroundEnemyParams>, off, size,
		w_mon, w_level, w_path, w_loop, false, true, false,  p_numLevels, ts, tileIndex);
}

void Session::AddBasicRailWorldEnemy(
	const std::string &displayName,
	const std::string &name,
	int w,
	int row,
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
	AddWorldEnemy( displayName, name, w, row, p_enemyCreator, SetParamsType<BasicRailEnemyParams>, off, size,
		w_mon, w_level, w_path, w_loop, false, false, true, p_numLevels, ts, tileIndex);
}

void Session::AddBasicAerialWorldEnemy(
	const std::string &displayName,
	const std::string &name, 
	int w,
	int row,
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
	AddWorldEnemy( displayName, name, w, row, p_enemyCreator, SetParamsType<BasicAirEnemyParams>, off, size, w_mon, w_level,
		w_path, w_loop, true, false, false, p_numLevels, ts, tileIndex);
}

void Session::AddUnlistedWorldEnemy(
	const std::string &name,
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

	ParamsInfo pInfo(name, name, -1, p_enemyCreator, p_paramsCreator, off, size,
		w_mon, w_level, w_path, w_loop, p_canBeAerial, p_canBeGrounded,
		p_canBeRailGrounded, p_numLevels, w, ts, tileIndex);

	SetupEnemyType(pInfo, true);
}

void Session::AddWorldEnemy(
	const std::string &displayName,
	const std::string &name,
	int w,
	int row,
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
	//use world and row to calculate position and check for repetitions

	auto & names = worldEnemyNames[w - 1];
	for (auto it = names.begin(); it != names.end(); ++it)
	{
		if ((*it).name == name)
		{
			return; //already added
		}
	}

	worldEnemyNames[w - 1].push_back(ParamsInfo( displayName, name, row, p_enemyCreator, p_paramsCreator, off, size,
		w_mon, w_level, w_path, w_loop, p_canBeAerial, p_canBeGrounded,
		p_canBeRailGrounded, p_numLevels, w, ts, tileIndex));
}

void Session::AddExtraEnemy(
	const std::string &displayName,
	const std::string &name,
	int row,
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

	extraEnemyNames.push_back(ParamsInfo(displayName, name, row, p_enemyCreator, p_paramsCreator, off, size,
		w_mon, w_level, w_path, w_loop, p_canBeAerial, p_canBeGrounded,
		p_canBeRailGrounded, p_numLevels, 0, ts, tileIndex));
}



int Session::GetPauseFrames()
{
	return pauseFrames;
}

BasicEffect * Session::ActivateEffect(int p_drawLayer, Tileset *ts, V2d pos, bool pauseImmune,
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
		b->drawLayer = p_drawLayer;


		Enemy *& fxList = effectListVec[p_drawLayer];
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
	Enemy *& fxList = effectListVec[b->drawLayer];
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

void Session::DrawEffects(int p_drawLayer, sf::RenderTarget *target)
{
	sf::View oldView = target->getView();
	if (p_drawLayer == DrawLayer::UI_FRONT)
	{
		target->setView(uiView);
	}
	Enemy *currentEffect = effectListVec[p_drawLayer];
	while (currentEffect != NULL)
	{
		currentEffect->Draw( Enemy::ENEMYDRAWLAYER_DEFAULT, target);
		currentEffect = currentEffect->next;
	}

	//envParticleSystem->Draw()

	if (p_drawLayer == DrawLayer::UI_FRONT)
	{
		target->setView(oldView);
	}

	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);

		if (p != NULL)
		{
			p->LayeredDrawEffects(p_drawLayer, target);
		}
	}
}


SoundInfo *Session::GetSound(const std::string &name)
{
	return mainMenu->soundManager.GetSound(name);
}

SoundNode *Session::ActivateSoundAtPos(V2d &pos, SoundInfo *si, bool loop)
{
	/*if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	{
		return NULL;
	}*/
	if (soundNodeList == NULL)
		return NULL;

	if (IsParallelSession())
		return NULL;

	sf::Rect<double> soundRect = screenRect;
	double soundExtra = 300;
	soundRect.left -= soundExtra;
	soundRect.width += 2 * soundExtra;
	soundRect.top -= soundExtra;
	soundRect.height += 2 * soundExtra;

	if (soundRect.contains(pos))
	{
		return soundNodeList->ActivateSound(si, loop);
	}
	else
	{
		return NULL;
	}
}

SoundNode *Session::ActivateSound(SoundInfo *si, bool loop)
{
	/*if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	{
		return NULL;
	}*/
	if (soundNodeList == NULL)
		return NULL;

	if (IsParallelSession())
		return NULL;

	return soundNodeList->ActivateSound(si, loop);
}

SoundNode *Session::ActivatePauseSound(SoundInfo *si, bool loop)
{
	//if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	//{
	//	return NULL;
	//}
	if (pauseSoundNodeList == NULL)
	{
		return NULL;
	}

	if (IsParallelSession())
		return NULL;

	return pauseSoundNodeList->ActivateSound(si, loop);
}

void Session::AllocateEffects()
{
	effectListVec.resize(DrawLayer::DrawLayer_Count);

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
	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
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
	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
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
	e->active = true;
	e->Init();

	//^^note remove this later
	//debugging only

	AddEnemyWithoutResetting(e);
	
}

//kinda buggy on its own rn
void Session::AddEnemyWithoutResetting(Enemy *e)
{
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


	e->HandleRemove();
	e->active = false;
	

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

		if (curr->type != EnemyType::EN_GOAL && curr->type != EnemyType::EN_NEXUS
			&& curr->type != EnemyType::EN_SHIPPICKUP )
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
		p->HitGoal();
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
		ts_basicBullets = GetSizedTileset("Enemies/General/bullet_80x80.png");
		ts_basicBulletExplode = GetSizedTileset("Enemies/General/bullet_explode_128x128.png");
	}
	else
	{
		
	}
}

void Session::DrawBullets(sf::RenderTarget *target)
{
	//added the bigBulletVA != NULL here for when I create
	//enemies with bullets for the logmenu, but don't use them.
	if (totalNumberBullets > 0 )
	{
		target->draw(bigBulletVA, totalNumberBullets * 4, sf::Quads, ts_basicBullets->texture);
	}
}

Session::Session( SessionType p_sessType, const boost::filesystem::path &p_filePath)
	:defaultStartingPlayerOptionsField(PLAYER_OPTION_BIT_COUNT),
	currPlayerOptionsField(Session::PLAYER_OPTION_BIT_COUNT), currLogField( LogDetailedInfo::MAX_LOGS),
	originalProgressionPlayerOptionsField( PLAYER_OPTION_BIT_COUNT ),
	originalProgressionLogField( LogDetailedInfo::MAX_LOGS )
{
	originalProgressionModeOn = false;
	skipOneReplayFrame = false;
	currWorldDependentTilesetWorldIndex = -1;
	ts_key = NULL;
	ts_goal = NULL;
	ts_goalCrack = NULL;
	ts_goalExplode = NULL;

	currShaderDrawLayer = DrawLayer::INVALID; //just needs to be invalid completely so I know to update initially

	turnTimerOnCounter = -1;
	timerOn = false;

	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
	{
		emitterLists[i] = NULL;
	}

	phaseOn = false;
	usedWarp = false;
	goal = NULL;
	shipPickup = NULL;

	activePlayerReplayManagers.reserve(10);

	nextFrameRestartGame = false;

	onlinePauseMenuOn = false;

	matchPlacings.resize(4);
	controllerStates.resize(4);
	controlProfiles.resize(4);

	ts_key = NULL;

	currSaveState = new SaveGameState;

	alertBox = new AlertBox;

	ngs = NULL;
	ggpoPlayers = NULL;

	ggpoCompressedInputs = new COMPRESSED_INPUT_TYPE[GGPO_MAX_PLAYERS];

	myBestReplayManager = NULL;
	playerRecordingManager = NULL;

	parallelSessionIndex = -1;
	randomState = 0;
	ggpoReady = false;

	desyncCheckerActive = false;

	frameConfirmed = false;

	flowHandler.sess = this;
	mainMenu = MainMenu::GetInstance();
	preScreenTex = MainMenu::preScreenTexture;
	extraScreenTex = MainMenu::extraScreenTexture;
	minimapTex = MainMenu::minimapTexture;
	postProcessTex2 = MainMenu::postProcessTexture2;
	pauseTex = MainMenu::pauseTexture;
	mapTex = MainMenu::mapTexture;
	fader = mainMenu->fader;
	swiper = mainMenu->swiper;
	ggpo = NULL;
	waterShaderCounter = 0.f;

	bossNodeVectorMap.resize(BossFightType_Count);

	firstUpdateHasHappened = false;

	specialTempTilesetManager = NULL;
	specialTempSoundManager = NULL;

	for (int i = 0; i < 4; ++i)
	{
		SetControllerStates(i, CONTROLLERS.GetStateQueue( CTYPE_XBOX, i ));//mainMenu->GetController(i)); //placeholder until things are cleaned up more
		controlProfiles[i] = NULL;
		//controllers[i] = NULL;
	}



	gameMode = NULL;

	timeSyncFrames = 0;

	simulationMode = false;

	frameRateDisplay.InitText(mainMenu->arial);
	runningTimerDisplay.InitText(mainMenu->arial);

	totalGameFramesIncludingRespawns = 0;
	totalGameFrames = 0;
	numGates = 0;
	drain = true;
	goalDestroyed = false;
	playerAndEnemiesFrozen = false;
	playerFrozen = false;

	netplayManager = NULL;

	currSuperPlayer = NULL;
	superSequence = NULL;
	inputVis = NULL;
	scoreDisplay = NULL;
	rain = NULL;
	goalFlow = NULL;
	goalPulse = NULL;
	currStorySequence = NULL;
	preLevelScene = NULL;
	postLevelScene = NULL;
	ClearActiveSequences();

	envParticleSystem = NULL;

	pokeTriangleScreenGroup = NULL;

	shipEnterScene = NULL;
	shipExitScene = NULL;
	shipTravelSequence = NULL;

	originalMusic = NULL;
	hud = NULL;
	gateMarkers = NULL;
	topClouds = NULL;
	deathSeq = NULL;
	shardPop = NULL;
	logPop = NULL;
	powerPop = NULL;

	shardMenu = NULL;
	logMenu = NULL;

	zoneTree = NULL;
	zoneTreeStart = NULL;
	zoneTreeEnd = NULL;
	currentZone = NULL;
	originalZone = NULL;

	absorbParticles = NULL;
	absorbDarkParticles = NULL;

	parentGame = NULL;

	sessType = p_sessType;
	cutPlayerInput = false;
	
	assert(mainMenu != NULL);

	window = mainMenu->window;

	filePath = p_filePath;
	filePathStr = filePath.string();

	players.resize(MAX_PLAYERS);

	soundNodeList = NULL;
	pauseSoundNodeList = NULL;
	
	railDrawTree = NULL;
	itemTerrainTree = NULL;
	terrainTree = NULL;
	specialTerrainTree = NULL;
	specterTree = NULL;
	railEdgeTree = NULL;
	barrierTree = NULL;
	borderTree = NULL;
	grassTree = NULL;
	activeItemTree = NULL;
	gateTree = NULL;
	enemyTree = NULL;

	staticItemTree = NULL;

	//polyShaders = NULL;
	
	background = NULL;
	ownsBG = true;
	inactiveEffects = NULL;

	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	inactiveEnemyList = NULL;

	ts_basicBullets = NULL;
	bigBulletVA = NULL;

	uiView = View(sf::Vector2f(960, 540), sf::Vector2f(1920, 1080));

	numSimulatedFramesRequired = 0;
	totalNumberBullets = 0;
	keyFrame = 0;

	mapHeader = NULL;

	showDebugDraw = false;
	showNetStats = false;
}


Session::~Session()
{
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

	if (absorbParticles != NULL)
	{
		delete absorbParticles;
		absorbParticles = NULL;
	}

	if (absorbDarkParticles != NULL)
	{
		delete absorbDarkParticles;
		absorbDarkParticles = NULL;
	}

	if (!IsAdventureSession() && mainMenu->rushManager == NULL ) //fix the conditional for this soon
	{
		if (hud != NULL)
		{
			delete hud;
			hud = NULL;
		}
	}

	/*if (parentGame == NULL && hud != NULL)
	{
		delete hud;
		hud = NULL;
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

	if (specterTree != NULL)
	{
		delete specterTree;
		specterTree = NULL;
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
	
	if (itemTerrainTree != NULL)
	{
		delete itemTerrainTree;
		itemTerrainTree = NULL;
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

	if (envParticleSystem != NULL)
	{
		delete envParticleSystem;
		envParticleSystem = NULL;
	}
		

	/*if (polyShaders != NULL)
	{
		delete[] polyShaders;
		polyShaders = NULL;
	}*/

	
		

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

	CleanupBackground();

	for (auto it = types.begin(); it != types.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete(*it).second;
	}

	if (shardPop != NULL)
	{
		delete shardPop;
		shardPop = NULL;
	}

	if (logPop != NULL)
	{
		delete logPop;
		logPop = NULL;
	}

	if (powerPop != NULL)
	{
		delete powerPop;
		powerPop = NULL;
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
	CleanupShipTravel();

	CleanupZones();
	CleanupGates();

	CleanupCameraShots();
	CleanupPoi();
	CleanupBossNodes();

	CleanupPokeTriangleScreenGroup();

	CleanupTopClouds();

	CleanupGoalFlow();
	CleanupGoalPulse();

	ClearEmitters();
	CleanupEnvParticleSystem();

	CleanupRain();

	CleanupGateMarkers();

	CleanupScoreDisplay();

	CleanupGameMode();

	CleanupSuperSequence();

	if (envParticleSystem != NULL)
	{
		delete envParticleSystem;
		envParticleSystem = NULL;
	}

	delete[] ggpoCompressedInputs;
	ggpoCompressedInputs = NULL;

	delete alertBox;

	delete currSaveState;
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

void Session::DrawPlayerWires( RenderTarget *target )
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->DrawParallelWires(preScreenTex);
	}


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
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->DrawParallelPlayers(preScreenTex);
	}

	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;

		for (int i = 0; i < pm->MAX_PARALLEL_SESSIONS; ++i)
		{
			if (pm->parallelGames[i] != NULL)
			{
				if (pm->parallelGames[i]->GetPlayer(0)->practiceDesyncDetected)
				{
					sf::CircleShape cs;
					cs.setFillColor(Color::Red);
					cs.setRadius(10);
					cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
					cs.setPosition(Vector2f(pm->parallelGames[i]->GetPlayer(0)->practiceDesyncPosition));
					target->draw(cs);
				}
			}
		}

	}

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





bool Session::ReadDecorInfoFile(int tWorld, int tVar)
{
	stringstream ss;
	ifstream is;

	//ss << "Resources/Terrain/Decor/" << "terraindecor_"
	//	<< (tWorld + 1) << "_0" << (tVar + 1) << ".txt";

	//path depreciated

	//had format like
	
	/*terraindecor_1_01.txt
	
	D_W1_VEINS1 100
	D_W1_VEINS2 100
	D_W1_VEINS3 100
	D_W1_VEINS4 100
	D_W1_VEINS5 100
	D_W1_VEINS6 100
	D_W1_ROCK_1 100
	D_W1_ROCK_2 100
	D_W1_ROCK_3 100
	D_W1_GRASSYROCK 100
	D_W1_BUSH_NORMAL 100
	D_W1_PLANTROCK 100
	*/
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

		ProcessDecorSpr( dName, dTile, dLayer, dPos, dRot, dScale );
	}

	ProcessAllDecorSpr();

	return true;
}

bool Session::ReadPlayersStartPos(std::ifstream &is)
{
	int numPlayers = mapHeader->numPlayerSpawns;

	for (int i = 0; i < numPlayers; ++i)
	{
		is >> playerOrigPos[i].x;
		is >> playerOrigPos[i].y;
	}
	

	return true;
}

bool Session::ReadPlayerOptions(std::ifstream &is)
{
	if (mapHeader->ver1 >= 9)
	{
		defaultStartingPlayerOptionsField.Load(is);
	}
	else if ((mapHeader->ver1 == 2 && mapHeader->ver2 >= 1) || mapHeader->ver1 > 2)
	{
		defaultStartingPlayerOptionsField.Reset();
		BitField oldOptions(256);
		oldOptions.Load(is);
		for (int i = 0; i < 256; ++i)
		{
			defaultStartingPlayerOptionsField.SetBit(i, oldOptions.GetBit(i));
		}
	}
	else
	{
		defaultStartingPlayerOptionsField.Reset();
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

		poly->polyIndex = specialPolyNum;

		poly->Load(is);

		poly->Finalize();

		ProcessSpecialTerrain(poly);
	}

	ProcessAllSpecialTerrain();

	return true;
}

bool Session::ReadLayeredTerrain(std::ifstream &is)
{
	int layeredPolyNum;
	is >> layeredPolyNum;
	for (int i = 0; i < layeredPolyNum; ++i)
	{
		PolyPtr poly(new TerrainPolygon());
		poly->Load(is);
		poly->Finalize();

		ProcessLayeredTerrain(poly);
	}

	ProcessAllLayeredTerrain();
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
	int numToOpen = -1;

	int seconds = -1;
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

		if (gCat == Gate::NUMBER_KEY || gCat == Gate::ALLKEY || gCat == Gate::PICKUP)
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
		else if (gCat == Gate::TIME_GLOBAL || gCat == Gate::TIME_ROOM)
		{
			is >> seconds;
		}

		ProcessGate( gCat, gVar, numToOpen, poly0Index, vertexIndex0, poly1Index, vertexIndex1, shardWorld,
			shardIndex, seconds);
	
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

		ReadLayeredTerrain(is);

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

ControllerState Session::GetPrevInput(int index)
{
	if (controllerStates[index] != NULL)
	{
		return controllerStates[index]->GetPrevState();
	}
	else
	{
		return ControllerState();
	}

	//return //mainMenu->GetPrevInput(index);
}

//eventually fix things, since there is currently no filtering
ControllerState Session::GetCurrInput(int index)
{
	if (controllerStates[index] != NULL)
	{
		return controllerStates[index]->GetCurrState();
	}
	else
	{
		return ControllerState();
	}
}

ControllerState Session::GetPrevInputFiltered(int index)
{
	return filteredPrevInput[index];
	/*if (controllerStates[index] != NULL)
	{
		assert(controlProfiles[index] != NULL);

		ControllerState prev = controllerStates[index]->GetPrevState();

		controlProfiles[index]->FilterState(prev);
		return prev;
	}
	else
	{
		return ControllerState();
	}*/
}

ControllerState Session::GetCurrInputFiltered(int index, Actor *p )
{
	ControllerDualStateQueue *currStates = controllerStates[index];
	if ( currStates != NULL)
	{
		ControlProfile *currProf = controlProfiles[index];
		assert(currProf != NULL);

		ControllerState curr;
		if (currStates->GetControllerType() == CTYPE_KEYBOARD)
		{
			if (p == NULL)
			{
				CONTROLLERS.UpdateFilteredKeyboardState(currProf, curr, filteredPrevInput[index]);
			}
			else
			{
				CONTROLLERS.UpdateFilteredKeyboardState(currProf, curr, p->prevInput);
			}
		}
		else
		{
			curr = currStates->GetCurrState();
			currProf->FilterState(curr);
		}
		return curr;
	}
	else
	{
		return ControllerState();
	}
}

void Session::SetControllerStates(int index, ControllerDualStateQueue *conStates)
{
	controllerStates[index] = conStates;
	//controllers[index] = c;
}

GameController * Session::GetController(int index)
{
	if (controllerStates[index] == NULL)
	{
		return NULL;
	}
	else
	{
		return controllerStates[index]->con;
	}
	//changed because I want to assign controllers at the freeplay menu
	//return con//mainMenu->GetController(index);
}

Actor *Session::GetPlayer(int index)
{
	if (index < 0)
		return NULL; //for savestate stuff

	assert(index >= 0 && index < MAX_PLAYERS);
	return players[index];
}

void Session::UpdateControllersOneFrameMode()
{
	CONTROLLERS.Update();

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

		//GameController *con = GetController(i);

		//GetCurrInput(i) = //con->GetState();
		//GetCurrInputUnfiltered(i) = con->GetUnfilteredState();
	}
}

void Session::UpdateControllers()
{
	if (IsSessTypeGame())
	{
		//pretty sure I only need this for pause menu and replay controls so gamesession only
		for (int i = 0; i < 4; ++i)
		{
			filteredPrevInput[i] = GetCurrInputFiltered(i);
		}
	}

	CONTROLLERS.Update();
	
	
	//Actor *p = NULL;
	//for (int i = 0; i < 4; ++i)
	//{
	//	GetPrevInput(i) = GetCurrInput(i);
	//	GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);
	//	/*if (!cutPlayerInput)
	//	{
	//		p = GetPlayer(i);
	//		if (p != NULL)
	//		{
	//			p->prevInput = GetCurrInput(i);
	//		}
	//	}*/

	//	GameController *con = GetController(i);

	//	GetCurrInput(i) = con->GetState();
	//	GetCurrInputUnfiltered(i) = con->GetUnfilteredState();
	//}
}

void Session::UpdatePlayerInput(int index)
{
	int playerInd = index;


	if (IsSessTypeEdit() && index > 0)
	{ 
		return;
	}

	if(IsParallelSession() )//&& gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
	{
		playerInd = 0;
	}

	Actor *player = GetPlayer(playerInd);

	if (player == NULL)
		return;
	
	bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();

	if (ggpoNetplay)
	{
		player->prevInput = player->currInput;

		player->currInput.SetFromCompressedState(ggpoCompressedInputs[index]);//8
		//cout << "setting player: " << playerInd << "in universe " << parallelSessionIndex << " to input " << ggpoCompressedInputs[index] << "\n";
	}
	else if (netplayManager != NULL && netplayManager->IsPracticeMode() && IsParallelSession())
	{
		assert(netplayManager->practicePlayers[parallelSessionIndex].HasInputs() > 0 );

		player->prevInput = player->currInput;

		//cout << "X: " << (int)(player->currInput.X) << "\n";

		PracticeInputMsg test = netplayManager->practicePlayers[parallelSessionIndex].GetNextMsg();

		player->currInput.SetFromCompressedState(netplayManager->practicePlayers[parallelSessionIndex].AdvanceInput());

		player->practiceDesyncPosition = test.desyncCheckPos;
		if (player->practiceDesyncPosition != player->position)
		{
			cout << "desynced. player pos: " << player->position.x << ", " << player->position.y << ", desync check: "
				<< test.desyncCheckPos.x << ", " << test.desyncCheckPos.y << "\n";
			cout << "player action: " << player->action << ", desync check: " << test.desyncCheckAction << "\n";
			player->practiceDesyncDetected = true;
		}
		else
		{
			//player->practiceDesyncDetected = true;
		}

		/*if (player->position != test.desyncCheckPos)
		{
			
		}*/

		//if (netplayManager->practicePlayers[0].HasNextInput())
		//{
		//	
		//}
		//else
		//{
		//	int xx = 56;
		//	//cout << "doesn't have input\n"
		//}

		
		//practice mode inputs for parallel sessions go here!
	}
	else if (player->simulationMode)
	{
		//make sure this takes priority over the replay
		player->prevInput = player->currInput;
	}
	else if (IsReplayOn())
	{
		if (!skipOneReplayFrame)
		{
			player->prevInput = player->currInput;

			assert(!activePlayerReplayManagers.empty());

			activePlayerReplayManagers[0]->GetReplayer(playerInd)->replayPlayer->UpdateInput(player->currInput);
		}
		//repPlayer->UpdateInput(player->currInput);//controllerStates[0]);
	}
	else
	{
		player->prevInput = player->currInput;

		if (cutPlayerInput)
		{
			player->currInput = ControllerState();
		}
		else
		{
			player->currInput = GetCurrInputFiltered(index, player);
		}

		if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession() )//&& playerInd == 0 )
		{
			//netplayManager->SendPracticeInitMessageToAllNewPeers();

			////sends the start to message to any new peers that join
			//PracticeStartMsg psm;
			//psm.skinIndex = GetPlayerNormalSkin(player->actorIndex);
			//psm.SetUpgradeField(player->bStartHasUpgradeField);
			//psm.startFrame = totalGameFrames;
			//psm.wantsToPlay = netplayManager->wantsToPracticeRace;
			//netplayManager->SendPracticeStartMessageToAllNewPeers(psm);

			PracticeInputMsg pm;
			pm.frame = totalGameFrames;
			pm.input = player->currInput.GetCompressedState();
			pm.desyncCheckPos = player->position;
			netplayManager->SendPracticeInputMessageToAllPeers(pm);
		}

		RecPlayerRecordFrame(playerInd);
	}	
	//player->prevInput = prevInput;

	/*if (controller->keySettings.toggleBounce)
	{
		if (currSessInput.X && !prevSessInput.X)
		{
			pCurr.X = !alreadyBounce;
		}
		else
		{
			pCurr.X = alreadyBounce;
		}
	}
	if (controller->keySettings.toggleGrind)
	{
		if (currSessInput.Y && !prevSessInput.Y)
		{
			pCurr.Y = !alreadyGrind;
		}
		else
		{
			pCurr.Y = alreadyGrind;
		}
	}
	if (controller->keySettings.toggleTimeSlow)
	{
		if (currSessInput.leftShoulder && !prevSessInput.leftShoulder)
		{
			pCurr.leftShoulder = !alreadyTimeSlow;
		}
		else
		{
			pCurr.leftShoulder = alreadyTimeSlow;
		}
	}*/
}

void Session::RunFrameForParallelPractice()
{
	if (!IsParallelSession() && gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
	{
		ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
		ppm->UpdateLobbyHUD();

		ParallelMode *pm = (ParallelMode*)gameMode;
		//ppm->ClearUpdateFlags();

		for (int i = 0; i < pm->MAX_PARALLEL_SESSIONS; ++i)
		{
			if (pm->parallelGames[i] != NULL)
			{
				PracticePlayer &prac = netplayManager->practicePlayers[i];
				if (prac.needsSessionRestart )
				{

					prac.needsSessionRestart = false;
					prac.sequenceConfirmMap.clear();
					prac.stateChangeMap.clear();
					//prac.hasSequenceConfirmReady = false;

					pm->parallelGames[i]->currLogField.Set(prac.logField);
					pm->parallelGames[i]->currPlayerOptionsField.Set(prac.playerOptionField);
					pm->parallelGames[i]->originalProgressionModeOn = prac.origProgression;

					if (prac.syncStateBufSize > 0)
					{
						//fresh player mid-game
						pm->parallelGames[i]->RestartLevel();
						pm->parallelGames[i]->LoadState(prac.syncStateBuf, prac.syncStateBufSize);

						prac.ClearSyncStateBuf();
					}
					else
					{
						//fresh player at beginning of level
						pm->parallelGames[i]->RestartLevel();
					}


					if (pm->parallelGames[i]->gameState == FROZEN || pm->parallelGames[i]->gameState == Session::PRACTICE_INVITE
						|| pm->parallelGames[i]->gameState == PAUSE )
					{
						//allows the player and nametag to remain visible if you enter a map while someone is frozen
						pm->parallelGames[i]->GetPlayer(0)->UpdateActionSprite();
						pm->parallelGames[i]->GetPlayer(0)->nameTag->SetPos(Vector2f(pm->parallelGames[i]->GetPlayer(0)->position));
					}
				}
			}
		}

		//can smooth this out later transitioning from frozen to run etc.
		int numFramesToRun = 0;
		for (int i = 0; i < pm->MAX_PARALLEL_SESSIONS; ++i)
		{
			if (pm->parallelGames[i] != NULL)
			{
				if (pm->parallelGames[i]->gameState == Session::FROZEN)
				{
					pm->parallelGames[i]->OnlineFrozenGameModeUpdate();

					/*if (netplayManager->practicePlayers[i].HasSequenceConfirms())
					{
						assert(pm->parallelGames[i]->gameState == Session::FROZEN);

						cout << "sequence confirm ready. confirming for session: " << i << "\n";

					}*/
				}
				else if (netplayManager->practicePlayers[i].HasStateChange())
				{
					pm->parallelGames[i]->OnlineRunGameModeUpdate();
				}
				else
				{
					numFramesToRun = netplayManager->practicePlayers[i].HasInputs();

					if (numFramesToRun > 0)
					{
						numFramesToRun = netplayManager->practicePlayers[i].HasInputs();
					}

					numFramesToRun = min(numFramesToRun, PracticePlayer::MAX_SIM_FRAMES);

					for (int j = 0; j < numFramesToRun; ++j)
					{
						//pm->parallelGames[i]->UpdatePlayerInput(i);
						pm->parallelGames[i]->OnlineRunGameModeUpdate();
					}
				}
			}
		}

		
		//for (int i = 0; i < pm->MAX_PARALLEL_SESSIONS; ++i)
		//{
		//	if (pm->parallelGames[i] != NULL)
		//	{
		//		numFramesToRun = netplayManager->practicePlayers[i].HasInputs();
		//		numFramesToRun = min(numFramesToRun, PracticePlayer::MAX_SIM_FRAMES);
		//		for( int j = 0; j < numFramesToRun; ++j )
		//		{
		//			//cout << "run parallel frame" << endl;
		//			pm->parallelGames[i]->UpdatePlayerInput(i);
		//			pm->parallelGames[i]->OnlineRunGameModeUpdate();
		//			switch (pm->parallelGames[i]->gameState)
		//			{
		//			case GameState::RUN:
		//				UpdatePlayerInput(parallelSessionIndex);
		//				OnlineRunGameModeUpdate();
		//				break;
		//			case GameState::FROZEN:
		//				OnlineFrozenGameModeUpdate();
		//				break;
		//			}

		//			pm->parallelGames[i]->RunPracticeModeUpdate();
		//			//ppm->updatePracticeSessions[i] = true;

		//			//ppm->parallelGames[i]->OnlineRunGameModeUpdate();//RunMainLoopOnce();
		//		}
		//	}
		//}
		//assert(netplayManager->practicePlayers[parallelSessionIndex].HasNextInput());
	}
}

void Session::UpdateAllPlayersInput()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		UpdatePlayerInput(i);
	}

	//only happens for parallel race. in parallel practice, 
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && !IsParallelSession() )//IsParallelGameModeType() && !IsParallelSession() )
	{
		ParallelMode *pm = (ParallelMode*)gameMode;

		/*for (int i = 0; i < ParallelMode::MAX_PARALLEL_SESSIONS; ++i)
		{
			if (pm->parallelGames[i] != NULL && !netplayManager->practicePlayers[i].HasStateChange())
			{
				pm->parallelGames[i]->UpdatePlayerInput(i + 1);
			}
		}*/

		pm->UpdateParallelPlayerInputs();
	}
}

bool Session::OneFrameModeUpdate()
{
	bool skipInput = false;
	
	//turned off for the beta
	
	//fix this so you can only tas in the editor right after testing this.


#ifdef _DEBUG
	skipInput = CONTROLLERS.KeyboardButtonHeld(Keyboard::PageUp);
#else
	skipInput = CONTROLLERS.KeyboardButtonHeld(Keyboard::PageUp) && IsSessTypeEdit();
#endif
	bool replayMode = IsReplayHUDOn();
	//and hit nexus or start ship sequence
	if (IsReplayOn())
	{
		activePlayerReplayManagers[0]->replayHUD->Update();

		if (replayMode)
		{
			skipInput = activePlayerReplayManagers[0]->replayHUD->IsGoingToNextFrame();
		}
	}

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
		bool stopSkippingInput = false;

#ifdef _DEBUG
		stopSkippingInput = CONTROLLERS.KeyboardButtonHeld(Keyboard::PageDown);
#else
		stopSkippingInput = CONTROLLERS.KeyboardButtonHeld(Keyboard::PageDown) && IsSessTypeEdit();
#endif
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

		if (replayMode)
		{	
			skipped = skipInput;
			if (skipInput)
			{
				accumulator = 0;//TIMESTEP;
				return true;
			}
		}
		else
		{
			if (!skipped && skipInput)
			{
				skipped = true;
				//accumulator = 0;
				return true;
			}

			if (skipped && !skipInput)
			{
				skipped = false;
			}
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
	/*if (zoneTree != NULL)
	{
		delete zoneTree;
		zoneTree = NULL;
	}*/
	zoneTree = NULL;
	currentZone = NULL;

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		delete (*it);
	}
	zones.clear();

	CleanupGateMarkers();
}

void Session::DrawZones(sf::RenderTarget *target)
{
	for (auto it = zones.begin(); it != zones.end(); ++it)
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

		TerrainPolygon *tp = new TerrainPolygon;
		V2d v0 = curr->v0;
		V2d v1 = curr->v1;
		list<Edge*> currGates;
		list<Gate*> ignoreGates;

		currGates.push_back(curr);



		tp->AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

		curr = curr->edge1;
		while (true)
		{
			if (curr->v0 == g->edgeA->v0)//curr == g->edgeA )
			{
				//we found a zone!

				if (!tp->IsClockwise())
				{
					//cout << "found a zone aaa!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;


					for (auto zit = zones.begin(); zit != zones.end() && okayZone; ++zit)
					{
						for (auto cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit)
						{
							for (auto git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git)
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
						z->zoneIndex = zones.size();
						zones.push_back(z);
						//	cout << "creating a zone with " << currGates.size() << " gatesAAA" << endl;
						//	cout << "actually creating a new zone   1! with " << z->gates.size() << endl;
					}
					else
					{
						delete tp;
						tp = NULL;
					}


				}
				else
				{
					delete tp;
					tp = NULL;
					//cout << "woulda been a zone" << endl;
				}

				break;
			}
			else if (curr == g->edgeB)
			{
				delete tp;
				tp = NULL;
				//currGates.push_back( curr );
				//cout << "not a zone even" << endl;
				break;
			}


			tp->AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

			if (curr->IsLockedGateEdge())
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

					tp->RemoveLastPoint();
					//TerrainPoint *tempPoint = NULL;
					//TerrainPoint *tempPoint = tp.pointEnd;
					//tp.RemovePoint( tempPoint );
					//delete tempPoint;
					//cout << "removing from a( " << g << " ) start: " << tp.numPoints << endl;

					while (true)
					{
						if (cc->IsLockedGateEdge())
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
							tp->RemoveLastPoint();

							//if( tp.pointStart == tp.pointEnd )
							if (tp->GetNumPoints() == 0)
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
						delete tp;
						tp = NULL;
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


		TerrainPolygon *tpb = new TerrainPolygon;

		tpb->AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

		curr = curr->edge1;
		while (true)
		{
			if (curr->v0 == g->edgeB->v0)//curr == g->edgeB )
			{
				//we found a zone!

				if (!tpb->IsClockwise())
				{
					//cout << "found a zone bbb!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;
					for (auto zit = zones.begin(); zit != zones.end() && okayZone; ++zit)
					{
						for (auto cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit)
						{
							for (auto git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git)
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
						z->zoneIndex = zones.size();

						zones.push_back(z);
						//cout << "actually creating a new zone   2! with " << z->gates.size() << endl;
					}
					else
					{
						delete tpb;
						tpb = NULL;
					}
					

				}
				else
				{
					delete tpb;
					tpb = NULL;
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


			tpb->AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

			if (curr->IsLockedGateEdge())
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

					tpb->RemoveLastPoint();

					Edge *cc = curr->edge0;

					while (true)
					{
						if (cc->IsLockedGateEdge())
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
							tpb->RemoveLastPoint();
							//if( tpb.pointStart == tpb.pointEnd )
							if (tpb->GetNumPoints() == 0)
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
						delete tpb;
						tpb = NULL;
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
		for (auto it = zones.begin(); it != zones.end(); ++it)
		{
			//cout << "setting gates in zone: " << (*it) << " which has " << (*it)->gates.size() << " gates " << endl;
			//cout << i << ", it gates: " << (*it)->gates.size() << endl;
			for (auto eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit)
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


	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		for (auto it2 = zones.begin(); it2 != zones.end(); ++it2)
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

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		for (auto it2 = (*it)->subZones.begin();
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
			bool foundZone = false;

			for (auto zit = zones.begin(); zit != zones.end(); ++zit)
			{
				if ((*zit)->ContainsPointMostSpecific((g->edgeA->v0 + g->edgeA->v1) / 2.0) != NULL)
				{
					g->zoneA = (*zit);
					g->zoneB = (*zit);
					foundZone = true;
					break;
				}
			}

			if (!foundZone)
			{
				outsideGates.push_back(g->edgeB);
				numOutsideGates++;
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

		TerrainPolygon *tp = new TerrainPolygon;
		Edge *startEdge;

		startEdge = globalBorderEdges.front();


		
		Edge *curr = startEdge;

		tp->AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

		curr = curr->edge1;

		while (curr != startEdge)
		{
			tp->AddPoint(Vector2i(curr->v0.x, curr->v0.y), false);

			curr = curr->edge1;
		}

		tp->FixWinding();

		Zone *z = new Zone(tp);
		z->gates = outsideGates;
		z->zoneIndex = zones.size();
		zones.push_back(z);

		for (auto it = outsideGates.begin(); it != outsideGates.end(); ++it)
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


		for (auto it2 = zones.begin(); it2 != zones.end(); ++it2)
		{
			if (z == (*it2))
				continue;

			if (z->ContainsZone((*it2)))
			{
				//cout << "contains zone!" << endl;
				z->subZones.push_back((*it2));
			}
		}



		for (auto it2 = z->subZones.begin();
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

int Session::SetupZones()
{
	originalZone = NULL;


	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		(*it)->zone = NULL;
	}

	if (zones.size() == 0)
	{
		return 0;
	}
		

	//add enemies to the correct zone.
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		for (auto zit = zones.begin(); zit != zones.end(); ++zit)
		{
			bool hasPoint = (*zit)->ContainsPoint((*it)->GetPosition());
			if (hasPoint)
			{
				bool mostSpecific = true;
				for (auto zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2)
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

	int mapTotalNumKeys = 0; //useful for stats later?
	for (auto zit = zones.begin(); zit != zones.end(); ++zit)
	{
		(*zit)->totalNumKeys = 0;
		//int numTotalKeys = 0;
		for (auto it = (*zit)->allEnemies.begin(); it != (*zit)->allEnemies.end(); ++it)
		{
			if ((*it)->hasMonitor)
			{
				++(*zit)->totalNumKeys;
				++mapTotalNumKeys;
			}
		}
	}

	//set key number objects correctly
	for (auto it = zoneObjects.begin(); it != zoneObjects.end(); ++it)
	{
		Zone *assignZone = NULL;
		V2d cPos((*it)->pos.x, (*it)->pos.y);
		for (auto zit = zones.begin(); zit != zones.end(); ++zit)
		{
			bool hasPoint = (*zit)->ContainsPoint(cPos);
			if (hasPoint)
			{
				bool mostSpecific = true;
				for (auto zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2)
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

	V2d playerPos(playerOrigPos[0]);
	for (auto zit = zones.begin(); zit != zones.end(); ++zit)
	{
		bool hasPoint = (*zit)->ContainsPoint(playerPos);//GetPlayer(0)->position);
		if (hasPoint)
		{
			bool mostSpecific = true;
			for (auto zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2)
			{
				if ((*zit2)->ContainsPoint(playerPos))//GetPlayer(0)->position))
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



	//original pos
	if (originalZone != NULL)
	{
		//cout << "setting original zone to active: " << originalZone << endl;
		ActivateZone(originalZone, true);
		
	}



	//std::vector<Zone*> zoneVec(zones.size());
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->hasGoal = false;
	}

	bool foundGoal = false;
	Zone *goalZone = NULL;
	for (auto it = zones.begin(); it != zones.end(); ++it)
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
		return -1;
		//assert(foundGoal);
	}

	Gate *g;
	for (auto it = zones.begin(); it != zones.end(); ++it)
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
				//assert(0);
			}
		}
	}

	/*if (zoneTree != NULL)
	{
		delete zoneTree;
		zoneTree = NULL;
	}*/
	zoneTree = NULL;

	zoneTreeStart = originalZone;
	zoneTreeEnd = goalZone;
	zoneTree = new ZoneNode;
	//zoneTree->parent = NULL;

	//this is testing a black gate bug
	if (zoneTreeStart == NULL)
	{
		cout << "unreachable goal? Black gate bug fix." << endl;
		for (auto it = zones.begin(); it != zones.end(); ++it)
		{
			(*it)->Init();
		}
		return 0;
	}

	zoneTree->SetZone(zoneTreeStart);

	//using shouldreform to test the secret gate stuff
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->SetShouldReform(true);
	}

	zoneTree->SetChildrenShouldNotReform();

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		if ((*it)->ShouldReform())
		{
			for (auto git = (*it)->gates.begin(); git != (*it)->gates.end(); ++git)
			{
				g = (Gate*)(*git)->info;

				if (g->zoneA == g->zoneB)
				{
					//a gate thats fully within a zone should simply
					//disappear upon entering
					continue;
				}

				g->SetToTwoWay();
				g->Init();
			}
			(*it)->SetZoneType(Zone::SECRET);
		}
	}

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Init();
	}

	if (originalZone != NULL)
	{
		SetKeyMarkerToCurrentZone(); //moved this to account for some gates getting set to secret
	}

	return 0;
}

void Session::WarpToZone(Zone *z)
{
	if (z == NULL)
		return;

	bool didZoneActivateForFirstTime = z->Activate(false);

	if (currentZone != NULL && z->zType != Zone::SECRET && currentZone->zType != Zone::SECRET)
	{
		currentZone->SetClosing(z->openFrames); //this is how it worked before, but openFrames is constant
												//probably meant this or something like it:
												//currentZone->SetClosing(z->data.frame);

		//bool foundNode = false;
		//ZoneNode *currZoneNode = currentZone->myNode;
		//for (auto it = currZoneNode->children.begin();
		//	it != currZoneNode->children.end(); ++it)
		//{
		//	if ((*it)->myZone == z)
		//	{
		//		foundNode = true;
		//		break;
		//	}
		//}

		////zones were accessed out of order somehow
		//if (!foundNode)
		//{
		//	assert(foundNode);
		//}

		KillAllEnemies();

		phaseOn = false; //for phase terrain to reset each gate makes the most sense to me.

		Actor *p = NULL;
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->ResetBoosterEffects();
			}
		}

		currentZone = z;

		CloseOffLimitZones();
	}
	else
	{
		Zone *oldZone = currentZone;
		currentZone = z;

		if (oldZone == NULL) //for starting the map
		{

		}
	}

	Gate *g;
	for (auto it = currentZone->gates.begin(); it != currentZone->gates.end(); ++it)
	{
		g = (Gate*)(*it)->info;
		if (g->category == Gate::ALLKEY)
		{
			g->SetNumToOpen(currentZone->totalNumKeys);
		}
	}

	if (gateMarkers != NULL)
	{
		gateMarkers->SetToZone(currentZone);
	}
}


void Session::ActivateZone(Zone * z, bool instant)
{
	if (z == NULL)
		return;

	bool didZoneActivateForFirstTime = z->Activate( instant );

	if (currentZone != NULL && z->zType != Zone::SECRET && currentZone->zType != Zone::SECRET)
	{
		currentZone->SetClosing(z->openFrames); //this is how it worked before, but openFrames is constant
		//probably meant this or something like it:
		//currentZone->SetClosing(z->data.frame);

		bool foundNode = false;
		ZoneNode *currZoneNode = currentZone->myNode;
		for (auto it = currZoneNode->children.begin();
			it != currZoneNode->children.end(); ++it)
		{
			if ((*it)->myZone == z)
			{
				foundNode = true;
				break;
			}
		}

		//zones were accessed out of order somehow
		if (!foundNode)
		{
			assert(foundNode);
		}

		KillAllEnemies();

		phaseOn = false; //for phase terrain to reset each gate makes the most sense to me.

		Actor *p = NULL;
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->ResetBoosterEffects();
			}
		}

		currentZone = z;

		CloseOffLimitZones();
	}
	else
	{
		Zone *oldZone = currentZone;
		currentZone = z;

		if (oldZone == NULL) //for starting the map
		{

		}
	}

	Gate *g;
	for (auto it = currentZone->gates.begin(); it != currentZone->gates.end(); ++it)
	{
		g = (Gate*)(*it)->info;
		if (g->category == Gate::ALLKEY)
		{
			g->SetNumToOpen(currentZone->totalNumKeys);
		}
	}

	if (gateMarkers != NULL)
	{
		gateMarkers->SetToZone(currentZone);
	}

	if (!instant)
	{
		//int soundIndex = SoundType::S_KEY_ENTER_0 + (currentZone->requiredKeys);
		//ActivateSound(gameSoundBuffers[soundIndex]);
	}
}

void Session::CloseOffLimitZones()
{
	if (currentZone == NULL)
	{
		return;
	}

	/*if (currentZoneNode == NULL)
		return;*/

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->SetShouldReform(true);
	}

	currentZone->myNode->SetChildrenShouldNotReform();

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->CloseOffIfLimited();
	}
}

void Session::SetupGateMarkers()
{
	//doesnt care about parentGame

	

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

void Session::SimulateGGPOGameFrame()
{
	simulationMode = true;

	RunGGPOModeUpdate();
	//GGPORunGameModeUpdate();

	simulationMode = false;

	if (IsParallelGameModeType() && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->SimulateParallelGGPOGameFrames();
	}

	if (!IsParallelSession())
	{
		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
		{
			if (gameMode->CheckVictoryConditions())
			{
				//ggpo_advance_frame(ggpo);
				gameMode->EndGame();

				//return true;
			}
		}


		ggpo_advance_frame(ggpo); //only update this once
	}
}

void Session::SimulateGameFrame()
{
	//simulationMode = true;
	RunGameModeUpdate();
	//simulationMode = false;
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
	case QUERY_ITEMTERRAIN:
		TryAddItemPolyToQueryList(qte);
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

	if (!e->SpawnableByCamera())
	{
		return;
	}

	bool a = e->spawnRect.intersects(tempSpawnRect);
	bool b = (e->zone == NULL || e->zone->IsActive());

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

void Session::TryAddItemPolyToQueryList(QuadTreeEntrant *qte)
{
	PolyPtr p = (PolyPtr)qte;

	if (itemTerrainList == NULL)
	{
		itemTerrainList = p;
		itemTerrainList->queryNext = NULL;
	}
	else
	{
		PolyPtr tva = p;
		PolyPtr temp = itemTerrainList;
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
			tva->queryNext = itemTerrainList;
			itemTerrainList = tva;
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
	railDrawTree->Query(this, rect);
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
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		(*it)->CheckedMiniDraw(target, rect);
	}
}

void Session::EnemiesCheckPauseMapDraw(sf::RenderTarget *target,
	sf::FloatRect &rect)
{
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		if ((*it)->IsGoalType())
		{
			continue;
		}
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

void Session::DrawPlayersToMap(sf::RenderTarget *target, bool drawKin, bool drawNameTags, bool p_isMinimap, float scale )
{
	if( gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession() )
	{
		const ConfigData &cd = mainMenu->config->GetData();
		if ((cd.parallelPracticeShowKinsOnPauseMap && p_isMinimap) || ( cd.parallelPracticeShowKinsOnPauseMap && !p_isMinimap) )
		{
			ParallelMode *pm = (ParallelMode*)gameMode;
			//pm->DrawParallelPlayersToMap(target, drawKin, drawNameTags, scale);
			for (int i = 0; i < ParallelMode::MAX_PARALLEL_SESSIONS; ++i)
			{
				if (pm->parallelGames[i] != NULL)
				{
					if (netplayManager->practicePlayers[i].isConnectedTo)
					{
						pm->parallelGames[i]->DrawPlayersToMap(target, drawKin, drawNameTags, scale);
					}
				}
			}
		}
	}


	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->MapDraw(target, drawKin, drawNameTags, scale);
		}
	}
}

void Session::SetupHUD()
{
	if (IsParallelSession())
	{
		hud = NULL;
	}
	else
	{
		if (parentGame != NULL)
		{
			hud = parentGame->hud;
		}
		else
		{
			if (IsAdventureSession())
			{
				hud = mainMenu->adventureManager->adventureHUD;
				hud->SetSession(this);
				GetPlayer(0)->kinMask = mainMenu->adventureManager->adventureHUD->kinMask;
			}
			else if (IsRushSession())
			{
				hud = mainMenu->rushManager->adventureHUD;
				hud->SetSession(this);
				GetPlayer(0)->kinMask = mainMenu->rushManager->adventureHUD->kinMask;
			}
			else if (hud == NULL)//&& !IsParallelSession() )
			{
				cout << "creating HUD because none is available" << endl;
				hud = gameMode->CreateHUD();
				hud->SetSession(this);

				if (hud->hType == HUD::HUDType::ADVENTURE)
				{
					AdventureHUD *ah = (AdventureHUD*)hud;
					GetPlayer(0)->kinMask = ah->kinMask;
				}
				//mainMenu->adventureManager->adventureHUD->kinMask;
			}
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

		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
		{
			if (mainMenu->config->GetData().parallelPracticeShowLobby)
			{
				ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
				ppm->DrawLobbyHUD(target);
			}
		}

		target->setView(oldView);
	}
}

void Session::UpdateHUD()
{
	if (hud != NULL && !IsParallelSession() )
	{
		hud->Update();
	}
	
	if (!timerOn)
	{
		if (turnTimerOnCounter == 0)
		{
			timerOn = true;
			turnTimerOnCounter = -1;
		}
		else
		{
			--turnTimerOnCounter;
		}
	}
}

void Session::HitlagUpdate()
{
	//UpdateControllers();
	//UpdateAllPlayersInput();

	//UpdatePlayersInHitlag();

	if ( activeSequences[0] != NULL && activeSequences[0] == superSequence)
	{
		ActiveSequencesUpdate();
		currSuperPlayer->CheckBirdCommands();
	}

	UpdateEffects(true);

	cam.UpdateRumble();

	SteamAPI_RunCallbacks();

	if (!IsParallelSession())
	{
		fader->Update();
		swiper->Update();

		mainMenu->UpdateEffects();

		pokeTriangleScreenGroup->Update();
	}

	pauseFrames--;

	//cout << "hitlag update. pause frames is now: " << pauseFrames << endl;

	
}

void Session::SetupAbsorbParticles()
{
	//every session would probably need its own absorb particles since they have different numbers of enemies
	/*if (parentGame != NULL)
	{
		absorbParticles = parentGame->absorbParticles;
		absorbDarkParticles = parentGame->absorbDarkParticles;
	}
	else*/
	{
		if (absorbParticles != NULL)
		{
			delete absorbParticles;
			delete absorbDarkParticles;
		}

		absorbParticles = new AbsorbParticles(this, AbsorbParticles::ENERGY);
		absorbDarkParticles = new AbsorbParticles(this, AbsorbParticles::DARK);
	}
	
}

void Session::ActivateEnergyAbsorbParticles(Actor *p, int storedHits,
	V2d &pos, float startAngle)
{
	absorbParticles->Activate(p, storedHits, pos, startAngle);
}

void Session::ActivateDarkAbsorbParticles(Actor *p, int storedHits,
	V2d &pos, float startAngle)
{
	absorbDarkParticles->Activate(p, storedHits, pos, startAngle);
	CollectKey();
}

void Session::CollectKey()
{
	GetPlayer(0)->numKeysHeld++;
	if (hud != NULL && hud->hType == HUD::ADVENTURE && !IsParallelSession() )
	{
		AdventureHUD *ah = (AdventureHUD*)hud;
		ah->UpdateKeyNumbers();
	}
}

void Session::ResetAbsorbParticles()
{
	if (absorbParticles == NULL)
		return;
	absorbParticles->Reset();
	absorbDarkParticles->Reset();
}

void Session::DrawEnemies(sf::RenderTarget *target)
{
	Enemy *current = NULL;//activeEnemyList;

	for (int i = 0; i < Enemy::ENEMYDRAWLAYER_Count; ++i)
	{
		current = activeEnemyList;
		while (current != NULL)
		{
			if (current->type != EnemyType::EN_BASICEFFECT && (pauseFrames < 2 || current->receivedHit.IsEmpty()))
			{
				current->Draw( i, preScreenTex);
			}
			current = current->next;
		}
	}
	

	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		(*it)->CheckedZoneDraw(target, FloatRect(screenRect));
	}
}

void Session::DrawHitEnemies(sf::RenderTarget *target)
{
	Enemy *current = NULL;
	for (int i = 0; i < Enemy::ENEMYDRAWLAYER_Count; ++i)
	{
		current = activeEnemyList;
		while (current != NULL)
		{
			if ((pauseFrames >= 2 && !current->receivedHit.IsEmpty()))
			{
				current->Draw(i, target);
			}
			current = current->next;
		}
	}
}

void Session::ResetZones()
{
	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Reset();
	}
}

void Session::UpdateZones()
{
	for (auto it = zones.begin(); it != zones.end(); ++it)
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

	g->OpenSecretTimeGate();
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

void Session::SetPlayerOption(int optionType, bool isOn, int playerIndex)
{
	currPlayerOptionsField.SetBit(optionType, true);
	GetPlayer(playerIndex)->SetStartOption(optionType, true);
}

void Session::UnlockLog(int logType, int playerIndex )
{
	//nothing
}


void Session::Fade(bool in, int frames, sf::Color c, bool skipKin, int p_drawLayer)
{
	if (IsParallelSession())
	{
		return;
	}
	fader->Fade(in, frames, c, skipKin, p_drawLayer);
}

void Session::CrossFade(int fadeOutFrames,
	int pauseFrames, int fadeInFrames,
	sf::Color c, bool skipKin)
{
	if (IsParallelSession())
	{
		return;
	}
	fader->CrossFade(fadeOutFrames, pauseFrames, fadeInFrames, c, skipKin);
}

void Session::ClearFade()
{
	if (IsParallelSession())
	{
		return;
	}

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
	playerFrozen = freeze;
}

void Session::FreezePlayer(bool freeze)
{
	playerFrozen = freeze;
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
	left->edgeIndex = 0;

	Edge *right = new Edge;
	right->v0 = bottomRight;
	right->v1 = topRight;
	right->edgeType = Edge::BORDER;
	right->edgeIndex = 1;
	//cout << "making new edge at x value: " << right->v0.x << endl;

	Edge *top = new Edge;
	top->v0 = topRight;
	top->v1 = topLeft;
	top->edgeType = Edge::BORDER;
	top->edgeIndex = 2;

	Edge *bot = new Edge;
	bot->v0 = bottomLeft;
	bot->v1 = bottomRight;
	bot->edgeType = Edge::BORDER;
	bot->edgeIndex = 3;

	left->edge0 = top;
	left->edge1 = bot;

	top->edge0 = right;
	top->edge1 = left;

	right->edge0 = bot;
	right->edge1 = top;

	bot->edge0 = left;
	bot->edge1 = right;

	left->CalcAABB();
	right->CalcAABB();
	top->CalcAABB();
	bot->CalcAABB();

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

void Session::AdjustBoundsFromTerrain()
{
	int maxY;
	int minX;
	int maxX;
	bool adjust = false;
	double extraBorder = 100;

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

		auto &polyList = edit->polygons[TerrainPolygon::CATEGORY_NORMAL];

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
		maxY += extraBorder;
		mapHeader->boundsHeight = maxY - mapHeader->topBounds;
		int oldRight = mapHeader->leftBounds + mapHeader->boundsWidth;
		int oldLeft = mapHeader->leftBounds;

		if (minX >= oldLeft && inversePolygon != NULL)
		{
			mapHeader->leftBounds = minX - extraBorder;
			mapHeader->boundsWidth = oldRight - mapHeader->leftBounds;
		}
		else if (minX > oldLeft)
		{
			mapHeader->leftBounds = minX;
			mapHeader->boundsWidth = oldRight - mapHeader->leftBounds;
		}
		
		if (maxX <= oldRight && inversePolygon != NULL)
		{
			mapHeader->boundsWidth = (maxX + extraBorder) - mapHeader->leftBounds;
		}
		else if (maxX < oldRight)
		{
			mapHeader->boundsWidth = maxX - mapHeader->leftBounds;
		}
		
	}
}

//for editor to adjust when making new terrain
void Session::AdjustBoundsHeightFromTerrain()
{
	int maxY;
	int minX;
	int maxX;
	bool adjust = false;
	double extraBorder = 100;

	if (IsSessTypeEdit())
	{
		EditSession *edit = EditSession::GetSession();

		auto &polyList = edit->polygons[TerrainPolygon::CATEGORY_NORMAL];

		if (!polyList.empty())
		{
			adjust = true;
			auto it = polyList.begin();

			IntRect polyAABB = (*it)->GetAABB();
			maxY = polyAABB.top + polyAABB.height;

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
			}
		}
	}

	if (adjust)
	{
		maxY += extraBorder;
		mapHeader->boundsHeight = maxY - mapHeader->topBounds;
	}
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
	}

	AdjustBoundsFromTerrain();

	SetGlobalBorders();

	int quadWidth = 200;

	int extraHeight = 1000;
	int extraWidth = 2000;

	int top = mapHeader->topBounds - extraHeight;
	int lBound = mapHeader->leftBounds;
	int rBound = mapHeader->leftBounds + mapHeader->boundsWidth;
	int height = mapHeader->boundsHeight + extraHeight;

	Vector2f leftCenter(lBound + quadWidth / 2,
		top + height / 2);
	Vector2f rightCenter(rBound - quadWidth / 2,
		top + height / 2);

	

	SetRectCenter(blackBorderQuads, quadWidth, height, leftCenter);
	SetRectCenter(blackBorderQuads + 8, extraWidth, height, leftCenter + Vector2f( -quadWidth / 2 - extraWidth/2,0 ));


	SetRectCenter(blackBorderQuads + 4, quadWidth, height, rightCenter);
	SetRectCenter(blackBorderQuads + 12, extraWidth, height, rightCenter + Vector2f(quadWidth / 2 + extraWidth/2, 0));

	/*SetRectCenter(blackBorderQuads + 8, extra, height, Vector2f(lBound - extra / 2,
		top + height / 2));
	SetRectCenter(blackBorderQuads + 12, extra, height, Vector2f(rBound + extra / 2,
		top + height / 2));*/
	SetRectColor(blackBorderQuads, Color(Color::Black));
	SetRectColor(blackBorderQuads + 4, Color(Color::Black));
	SetRectColor(blackBorderQuads + 8, Color(Color::Black));
	SetRectColor(blackBorderQuads + 12, Color(Color::Black));
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

	if (mapHeader->specialMapType == MapHeader::MAPTYPE_NORMAL)
	{
		target->draw(blackBorderQuads, 16, sf::Quads);
	}
	/*if (cam.manual || narrowMap)
	{
		target->draw(blackBorderQuads, 16, sf::Quads);
	}
	else
	{
		target->draw(blackBorderQuads, 8, sf::Quads);
	}*/
}

void Session::TryCreateShardResources()
{
	if (shardPop == NULL)
	{
		shardPop = new ShardPopup;
	}
}

void Session::TryCreateLogResources()
{
	if (logPop == NULL)
	{
		logPop = new LogPopup;
	}
}

void Session::TryCreatePowerItemResources()
{
	if (powerPop == NULL)
	{
		powerPop = new PowerPopup;
	}
}

void Session::SetActiveSequence(Sequence *newSeq, int activeSeqSlot )
{
	Sequence *&activeSeq = activeSequences[activeSeqSlot];
	if (newSeq == NULL)
	{
		if (activeSeq == NULL)
		{
			//already NULL
			return;
		}

		activeSeq->ReturnToGame();
		activeSeq = NULL;
		return;
	}

	if (newSeq->sequenceID == -1)
	{
		assert(0);
		return;
	}

	activeSeq = newSeq;

	

	if (activeSeq == preLevelScene)
	{
		FreezePlayer(true);
		//FreezePlayerAndEnemies(true);
		SetPlayerInputOn(false);
	}

	activeSeq->StartRunning();
}

void Session::ActiveSequencesUpdate()
{
	Sequence *activeSeq = NULL;
	for (int i = 0; i < MAX_SIMULTANEOUS_SEQUENCES; ++i)
	{
		activeSeq = activeSequences[i];
		if (activeSeq != NULL)// && activeSequence == startSeq )
		{
			GameState oldState = gameState;
			Sequence *oldSeq = activeSeq;
			if (!activeSeq->Update())
			{
				if (activeSeq->nextSeq != NULL)
				{
					activeSeq->nextSeq->Reset();
					SetActiveSequence(activeSeq->nextSeq, i );
				}
				else
				{
					if (activeSeq == preLevelScene)
					{
						//FreezePlayerAndEnemies(false);
						FreezePlayer(false);
						SetPlayerInputOn(true);
						if (shipEnterScene != NULL)
						{
							shipEnterScene->Reset();
							SetActiveSequence(shipEnterScene);
							gameState = RUN;
							return;
						}
						else if (shipTravelSequence != NULL)
						{
							shipTravelSequence->Reset();
							SetActiveSequence(shipTravelSequence, 1);
							gameState = RUN;
							return;
						}
					}


					if (activeSeq == postLevelScene)
					{
						goalDestroyed = true;
					}

					//if (gameState == SEQUENCE) //if this sets it to run when its frozen, sometimes you can get a weird bug in multiplayer endings.
					//{
					//	
					//}

					if (gameState != RUN)
					{
						gameState = RUN;
						switchGameState = true; //turned this on so the while loop will know to exit early and not run more frames in the wrong gameState
					}

					//testing for sequence after ship
					if (activeSequences[i] == oldSeq)
					{
						activeSequences[i] = NULL;
					}
					
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
}

void Session::DrawActiveSequences(int p_drawLayer, sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_SIMULTANEOUS_SEQUENCES; ++i)
	{
		if (activeSequences[i] != NULL)
		{
			sf::View oldView = target->getView();
			if (p_drawLayer == DrawLayer::UI_FRONT)
			{
				target->setView(uiView);
			}

			activeSequences[i]->LayeredDraw(p_drawLayer, target);

			if (p_drawLayer == DrawLayer::UI_FRONT)
			{
				target->setView(oldView);
			}
		}
	}
	
}

bool Session::HasLog(int logIndex)
{
	if (IsReplayOn())
	{
		return activePlayerReplayManagers[0]->header.IsLogCaptured(logIndex);
	}

	return currLogField.GetBit(logIndex);
}

void Session::AddEmitter(ShapeEmitter *emit )
{
	if (emit->data.active)
	{
		//cout << "shouldn't add emitter that is already active again" << "\n";
		//assert(0);

		//RemoveEmitter(emit, layer);
		//emit->Reset();

		return;
	}

	emit->data.active = true;

	ShapeEmitter *&currList = emitterLists[emit->drawLayer];

	ShapeEmitter *c = currList;

	//just double checking for debug purposes
	while (c != NULL)
	{
		assert(c != emit);
		c = c->next;
	}

	if (currList == NULL)
	{
		emit->prev = NULL;
		emit->next = NULL;

		currList = emit;
	}
	else
	{
		emit->prev = NULL;
		emit->next = currList;
		currList->prev = emit;
		
		currList = emit;
	}
}

void Session::RemoveEmitter(ShapeEmitter *emit )
{
	if (!emit->data.active)
		return;

	assert(emit->data.active);

	ShapeEmitter *&currList = emitterLists[emit->drawLayer];

	if (currList == NULL)
	{
		assert(false);
		return;
	}

	emit->data.active = false;

	ShapeEmitter *prev = emit->prev;
	ShapeEmitter *next = emit->next;

	if (prev == NULL && next == NULL)
	{
		currList = NULL;
	}
	else
	{
		if (emit == currList)
		{
			assert(next != NULL);

			next->prev = NULL;
			currList = next;
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

void Session::DrawEmitters(int p_drawLayer, sf::RenderTarget *target)
{
	ShapeEmitter *curr = emitterLists[p_drawLayer];
	while (curr != NULL)
	{
		curr->Draw(target);
		curr = curr->next;
	}
}

void Session::UpdateEmitters()
{
	ShapeEmitter *curr = NULL;
	ShapeEmitter *next = NULL;
	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
	{
		curr = emitterLists[i];
		while (curr != NULL)
		{
			next = curr->next;
			if (curr->IsDone())
			{
				RemoveEmitter(curr);
				/*if (curr == emitterLists[i])
				{
					emitterLists[i] = curr->next;
				}
				else
				{
					prev->next = curr->next;
				}*/
				curr = next;
			}
			else
			{
				curr->Update();
				curr = next;
			}
		}
	}
}

void Session::ClearEmitters()
{
	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
	{
		ShapeEmitter *curr = emitterLists[i];

		while (curr != NULL)
		{
			ShapeEmitter *next = curr->next;
			curr->Reset();
			curr = next;
		}

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
		DrawEffects(DrawLayer::IN_FRONT, preScreenTex);
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
		b->Trigger(); //of course it should trigger here, right?
		cam.SetManual(true);
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
				game->SetBonus(b->myBonus, V2d(0,0));
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
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
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

std::vector<PoiInfo*> * Session::GetBossNodeVector(int i, const std::string &name)
{
	auto &m = GetBossNodeVectorMap(i);
	if (m.count(name) > 0)
	{
		return &(m[name]);
	}
	else
	{
		return NULL;
	}
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

void Session::CleanupShipTravel()
{
	if (shipTravelSequence != NULL)
	{
		delete shipTravelSequence;
		shipTravelSequence = NULL;
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
	if ((player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT) && !playerFrozen)
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

void Session::InitPreFrameCalculations()
{
	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->InitPreFrameCalculations();
	}
}

void Session::UpdatePreFrameCalculations()
{
	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->UpdatePreFrameCalculations();
	}
}

void Session::UpdatePhysics()
{
	Actor *p = NULL;
	Actor *player = GetPlayer(0);
	if ((player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT) && !playerFrozen)
	{
		return;
	}

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->physicsOver = false;
	}



	/*int numSteps;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			numSteps = p->GetNumSteps();
			for (substep = 0; substep < numSteps; ++substep)
			{
				p->UpdatePhysics();
			}
		}
	}*/

	//1 substep of players, then 1 substep of enemies. how it should be.
	for (substep = 0; substep < NUM_MAX_STEPS; ++substep)
	{
		if (!playerFrozen)
		{
			for (int i = 0; i < 4; ++i)
			{
				p = GetPlayer(i);

				if (p != NULL)
				{
					//players always have high accuracy movements in normal physics
					p->UpdatePhysics();
					/*numSteps = p->GetNumSteps();
					for (substep = 0; substep < numSteps; ++substep)
					{
						p->UpdatePhysics();
					}*/
				}
			}
		}


		if (currentZone != NULL)
		{
			Gate *g = NULL;
			for (auto it = currentZone->gates.begin(); it != currentZone->gates.end(); ++it)
			{
				 g = (*it)->GetGate();
				 if (g->IsSoft())
				 {
					 g->SetLocked(true);
				 }
			}
		}

		Enemy *current = activeEnemyList;
		while (current != NULL)
		{
			current->UpdatePhysics(substep);
			current = current->next;
		}

		if (currentZone != NULL)
		{
			Gate *g = NULL;
			for (auto it = currentZone->gates.begin(); it != currentZone->gates.end(); ++it)
			{
				g = (*it)->GetGate();
				if (g->IsSoft())
				{
					g->SetLocked(false);
				}
			}
		}

	}
}

void Session::UpdateEnemiesPostPhysics()
{
	Actor *player = GetPlayer(0);
	if ((player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT) && !playerFrozen)
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

void Session::QueryItemTerrainTree(sf::Rect<double>&rect)
{
	if (itemTerrainTree != NULL)
	{
		itemTerrainList = NULL;
		queryMode = QUERY_ITEMTERRAIN;
		itemTerrainTree->Query(this, rect);
	}
}

int Session::GetNumTotalEnergyParticles(int absorbType)
{
	int total = 0;
	switch (absorbType)
	{
	case AbsorbParticles::ENERGY:
	{
		for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
		{
			total += (*it)->GetTotalEnergyAbsorbParticles();
		}
		break;
	}
	case AbsorbParticles::DARK:
	{
		for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
		{
			total += (*it)->GetNumDarkAbsorbParticles();	
		}
		break;
	}
	}

	return total;
}

void Session::DrawStoryLayer(int p_drawLayer, sf::RenderTarget *target)
{
	if (currStorySequence != NULL)
	{
		sf::View oldV = target->getView();
		target->setView(uiView);
		currStorySequence->LayeredDraw( p_drawLayer, target );
		target->setView(oldV);
	}
}

void Session::DrawGateMarkers(sf::RenderTarget *target)
{
	if (gateMarkers != NULL)
		gateMarkers->Draw(target);
}

void Session::LayeredDraw(int p_drawLayer, sf::RenderTarget *target)
{
	View oldView = target->getView();
	target->setView(uiView);
	fader->Draw(p_drawLayer, target);
	target->setView(oldView);
	
	if (background != NULL)
	{
		background->LayeredDraw(p_drawLayer, target);
	}

	DrawTerrain(p_drawLayer, target);

	DrawDecor(p_drawLayer, target);
	DrawStoryLayer(p_drawLayer, target);
	DrawActiveSequences(p_drawLayer, target);
	DrawEffects(p_drawLayer, target);
	DrawEmitters(p_drawLayer, target);
	//swiper->Draw(target);
}

typedef pair<V2d, V2d> pairV2d;
void Session::SetupGoalFlow()
{
	if (goal == NULL)
		return;

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

	V2d goalPos = GetLevelFinisherPos();

	for (int i = 0; i < divs; ++i)
	{
		rayIgnoreEdge1 = NULL;
		rayIgnoreEdge = NULL;

		allInfo.push_back(list<pair<V2d, bool>>());
		list<pair<V2d, bool>> &pointList = allInfo.back();

		double angle = (tau / divs) * i;
		V2d rayDir(cos(angle), sin(angle));

		flowHandler.rayCastInfo.rayStart = goalPos + rayDir * startRadius;
		flowHandler.rayCastInfo.rayEnd = flowHandler.rayCastInfo.rayStart + rayDir * rayLen;

		bool rayOkay = flowHandler.rayCastInfo.rayEnd.x >= mapHeader->leftBounds && flowHandler.rayCastInfo.rayEnd.y >= mapHeader->topBounds
			&& flowHandler.rayCastInfo.rayEnd.x <= mapHeader->leftBounds + mapHeader->boundsWidth
			&& flowHandler.rayCastInfo.rayEnd.y <= mapHeader->topBounds + mapHeader->boundsHeight;


		Edge *cEdge = NULL;
		//list<pair<V2d, bool>> pointList; //if true, then its facing the ray


		while (rayOkay)
		{
			//cout << "ray start: " << rayStart.x << ", " << rayStart.y << endl;
			flowHandler.rayCastInfo.rcEdge = NULL;

			RayCast(&flowHandler, qt->startNode, flowHandler.rayCastInfo.rayStart, flowHandler.rayCastInfo.rayEnd);
			//rayStart = v0 + along * quant;
			//rayIgnoreEdge = te;
			//V2d goalDir = normalize( rayStart - goalPos );
			//rayEnd = rayStart + goalDir * maxRayLength;//rayStart - norm * maxRayLength;


			//start ray
			if (flowHandler.rayCastInfo.rcEdge != NULL)
			{
				//cout << "point list size: " << pointList.size() << endl;
				if (flowHandler.rayCastInfo.rcEdge->IsInvisibleWall() || flowHandler.rayCastInfo.rcEdge->IsLockedGateEdge())
				{
					//	cout << "secret break" << endl;
					break;
				}

				rayIgnoreEdge1 = rayIgnoreEdge;
				rayIgnoreEdge = flowHandler.rayCastInfo.rcEdge;

				V2d rn = flowHandler.rayCastInfo.rcEdge->Normal();
				double d = dot(rn, rayDir);
				V2d hitPoint = flowHandler.rayCastInfo.rcEdge->GetPosition(flowHandler.rayCastInfo.rcQuant);
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
				flowHandler.rayCastInfo.rayStart = hitPoint;
				flowHandler.rayCastInfo.rayEnd = hitPoint + rayDir * rayLen;



				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );

			}
			else
			{
				flowHandler.rayCastInfo.rayStart = flowHandler.rayCastInfo.rayEnd;
				flowHandler.rayCastInfo.rayEnd = flowHandler.rayCastInfo.rayStart + rayDir * rayLen;
			}

			double oldRayCheck = rayCheck;
			rayCheck = length((goalPos + rayDir * startRadius) - flowHandler.rayCastInfo.rayEnd);

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
				pointList.push_back(pair<V2d, bool>(flowHandler.rayCastInfo.rayEnd, false));
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
	goalFlow->SetWorld(mapHeader->envWorldType);
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
	if (edge->IsGateEdge()
		|| edge->edgeType == Edge::BORDER || edge->edgeType == Edge::BARRIER )
	{
		return;
	}

	if (edge != sess->rayIgnoreEdge && edge != sess->rayIgnoreEdge1
		&& (rayCastInfo.rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - rayCastInfo.rayStart) <
			length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant) - rayCastInfo.rayStart)))
	{
		rayCastInfo.rcEdge = edge;
		rayCastInfo.rcQuant = edgeQuantity;
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
		Vector2f vSize = view.getSize();

		float zoom = vSize.x / 960.f;

		Vector2f botLeft(view.getCenter().x - view.getSize().x / 2,
			view.getCenter().y + view.getSize().y / 2);

		float camAngle = (float)(view.getRotation() * PI / 180.0);

		Vector2f botLeftTest(-vSize.x / 2, vSize.y / 2);
		RotateCW(botLeftTest, camAngle);

		botLeftTest += view.getCenter();

		botLeft = botLeftTest;

		goalFlow->Update(zoom, botLeft, camAngle );
	}
}

void Session::SetupEnvParticleSystem()
{
	assert(envParticleSystem == NULL);
	envParticleSystem = new EnvParticleSystem;
}

void Session::CleanupEnvParticleSystem()
{
	if (envParticleSystem != NULL)
	{
		delete envParticleSystem;
		envParticleSystem = NULL;
	}
}

void Session::UpdateEnvParticleSystem()
{
	if (envParticleSystem != NULL)
	{
		envParticleSystem->Update();
	}
}

void Session::CleanupGoalPulse()
{
	//cleaned up in case you have a map with no goal having a bonus with a goal etc
	if (goalPulse == NULL)
	{
		return;
	}

	if (parentGame == NULL)
	{
		delete goalPulse;
		goalPulse = NULL;
	}

	/*bool isGoalPulseOwner = true;
	GameSession *tempParent = parentGame;
	while (tempParent != NULL)
	{
		if (tempParent->goalPulse != NULL)
		{
			isGoalPulseOwner = false;
			break;
		}
		else
		{
			tempParent = tempParent->parentGame;
		}
	}

	if (isGoalPulseOwner)
	{
		delete goalPulse;
		goalPulse = NULL;
	}*/
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
	assert(goalPulse == NULL);

	if (parentGame != NULL)
	{
		goalPulse = parentGame->goalPulse;
	}
	else
	{
		goalPulse = new GoalPulse;
	}

	//GameSession *tempParent = parentGame;
	//while (tempParent != NULL)
	//{
	//	if (tempParent->goalPulse != NULL)
	//	{
	//		goalPulse = tempParent->goalPulse;
	//		break;
	//	}
	//	else
	//	{
	//		tempParent = tempParent->parentGame;
	//	}
	//}

	//if (goalPulse == NULL)
	//{
	//	goalPulse = new GoalPulse;// , Vector2f(goalPos.x, goalPos.y));
	//}

	

	//goalPulse->SetPosition(Vector2f(goalPos));
}


void Session::DrawGoalPulse(sf::RenderTarget *target)
{
	if (goalPulse != NULL)
	{
		goalPulse->Draw(target);
	}
}

void Session::CleanupMyBestPlayerReplayManager()
{
	if (parentGame == NULL && myBestReplayManager != NULL)
	{
		delete myBestReplayManager;
		myBestReplayManager = NULL;
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
	if (runningTimerDisplay.showRunningTimer && (scoreDisplay == NULL || !scoreDisplay->IsActive()) )
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

bool Session::IsAdventureSession()
{
	return mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && mainMenu->adventureManager != NULL;
}

bool Session::IsRushSession()
{
	return mainMenu->gameRunType == MainMenu::GRT_RUSH && mainMenu->rushManager != NULL;
}

void Session::ActivateScoreDisplay(int hideHUDFrames )
{
	if (IsAdventureSession())
	{
		mainMenu->adventureManager->adventureScoreDisplay->Activate();
	}
	else if (IsRushSession())
	{
		mainMenu->rushManager->rushScoreDisplay->Activate();
	}
	else
	{
		scoreDisplay->Activate();
	}
	
	HideHUD(hideHUDFrames);
}

void Session::DrawScoreDisplay(sf::RenderTarget *target)
{
	if (IsAdventureSession())
	{
		mainMenu->adventureManager->adventureScoreDisplay->Draw(target);
	}
	else if (IsRushSession())
	{
		mainMenu->rushManager->rushScoreDisplay->Draw(target);
	}
	else if (scoreDisplay != NULL)
	{
		scoreDisplay->Draw(target);
	}
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

	if (!firstUpdateHasHappened)
	{
		//cout << "first update draw" << endl;
		LayeredDraw(DrawLayer::IN_FRONT, target);

		//target->setView(view);
		DrawKinOverFader(target);

		return;
	}

	if (background != NULL)
	{
		background->DrawBackLayer(target);
	}


	for (int i = DrawLayer::BG_10; i <= DrawLayer::BG_1; ++i)
	{
		LayeredDraw(i, target);
	}
		//background->Draw(target);

	//UpdateEnvShaders(); //move this into the update loop

	DrawTopClouds(target);

	DrawBlackBorderQuads(target);

	LayeredDraw(DrawLayer::BEHIND_TERRAIN, target);

	DrawZones(target);

	DrawSpecialTerrain(target);

	DrawItemTerrain(target);

	DrawTerrain( DrawLayer::TERRAIN, target);

	DrawGoalFlow(target);

	DrawPracticeSessions(target, view );

	DrawHUD(target);

	

	//DrawBossHUD(target);

	DrawGates(target);
	DrawRails(target);

	LayeredDraw(DrawLayer::BEHIND_ENEMIES, target);

	DrawReplayGhosts(target);

	DrawEnemies(target);

	LayeredDraw(DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, target);

	DrawGoalPulse(target);
	DrawPlayerWires(target);

	DrawHitEnemies(target); //whited out hit enemies

	absorbParticles->Draw(target);
	absorbDarkParticles->Draw(target);

	DrawPlayersHomingBall(target);

	DrawPlayers(target);

	DrawPlayerShields(target);

	LayeredDraw(DrawLayer::IN_FRONT, target);

	DrawBullets(target);

	DrawRain(target);

	//DrawActiveEnvPlants();

	DebugDraw(target);

	//target->draw(testSimCircle);

	//DrawShockwaves(target); //not operational atm

	target->setView(uiView);

	pokeTriangleScreenGroup->Draw(target);

	DrawScoreDisplay(target);

	alertBox->Draw(target);

	//DrawLeaderboard(target);

	DrawFrameRate(target);

	DrawRunningTimer(target);

	DrawInputVis(target);

	DrawGateMarkers(target);

	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession() && gameState == PRACTICE_INVITE)
	{
		ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
		ppm->DrawInviteDisplay(target);
	}

	//left = 300;
	//right = -300;
	//top = 300;//150;
	//bottom = -300;

	//camera debug grid
	/*sf::Vertex testGrid[4];
	testGrid[0].position = Vector2f(960 - cam.maxOffset.x * 2, 0);
	testGrid[1].position = Vector2f(960 - cam.maxOffset.x * 2, 1080);
	testGrid[2].position = Vector2f(0, 540);
	testGrid[3].position = Vector2f(1920, 540);
	
	testGrid[0].color = Color::Red;
	testGrid[1].color = Color::Red;
	testGrid[2].color = Color::Red;
	testGrid[3].color = Color::Red;
	target->draw(testGrid, 2, sf::Lines);*/


	target->setView(view);

	DrawDyingPlayers(target);

	UpdateNameTagsPixelPos(target);

	for (int i = DrawLayer::FG_1; i <= DrawLayer::FG_10; ++i)
	{
		LayeredDraw(i, target);
	}

	//UpdateTimeSlowShader();

	target->setView(uiView);

	LayeredDraw(DrawLayer::UI_FRONT, target);

	LayeredDraw(DrawLayer::IN_FRONT_OF_UI, target);

	DrawNameTags(target);

	if (!activePlayerReplayManagers.empty())
	{
		activePlayerReplayManagers[0]->replayHUD->Draw(target);
	}

	if (gameState == GameState::RUN && onlinePauseMenuOn )
	{
		assert(gameMode->onlinePauseMenu != NULL);
		gameMode->onlinePauseMenu->Draw(target);
	}

	mainMenu->DrawEffects(target);

	target->setView(view); //sets it back to normal for any world -> pixel calcs
	DrawKinOverFader(target);
}

void Session::DrawPracticeGame(sf::RenderTarget *target)
{
	//target->setView(view);

	DrawPlayerWires(target);

	DrawPlayersHomingBall(target);

	DrawPlayers(target);

	DrawPlayerShields(target);

	DrawDyingPlayers(target);

	//target->setView(view); //sets it back to normal for any world -> pixel calcs
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
	if (runningTimerDisplay.showRunningTimer && (scoreDisplay == NULL || !scoreDisplay->IsActive()))
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
	if (soundNodeList != NULL && !IsParallelSession() )
	{
		soundNodeList->Update();
	}
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
	collider.ClearDebug(); //not sure if this is exactly where this should be, but its a bit irrelevant

	//double testMult = accumulator / TIMESTEP;
	//if (testMult > 10)
	//{
	//	cout << "accumulator mult: " << testMult << "\n";
	//}
	

	while (accumulator >= TIMESTEP)
	{
		currShaderDrawLayer = DrawLayer::INVALID;

		if (!firstUpdateHasHappened)
		{
			firstUpdateHasHappened = true;
		}

		if (!OneFrameModeUpdate())
		{
			UpdateControllers();

			if (UpdateRunModeBackAndStartButtons())
			{

			}

			if (IsReplayOn() && IsSessTypeGame())
			{
				//this was added so hopefully pausing a replay doesn't completely blow up parallel play
				//we'll see if it causes any issues
				RunFrameForParallelPractice();
				SteamAPI_RunCallbacks();
			}
			break;
		}

		if (!RunPreUpdate())
			break;


		if (gameMode->CheckVictoryConditions())
		{
			gameMode->EndGame();
			break;
		}

		if (!playerAndEnemiesFrozen)
		{
			if (!playerFrozen)
			{
				InitPreFrameCalculations();
			}

			UpdateEnemiesPreFrameCalculations();

			if (!playerFrozen)
			{
				UpdatePreFrameCalculations();
			}
			
		}

		UpdateControllers();


		// UpdateAllPlayersInput(); //moving this below ActiveSequenceUpdate, because the gamestate changing can make this replay an extra frame of inputs
		//it basically gets called twice in the same time that the actual players update once which makes no sense


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


		UpdateAllPlayersInput();*/

		//please....hoping to fix the bug from below and doesn't mess up sequence stuff...
		//UpdateAllPlayersInput(); //when its here, logs and state changes fail, because the input is getting incremented but shouldn't be if the
		//state is going to change

		skipOneReplayFrame = false;

		if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession())//&& playerInd == 0 )
		{
			netplayManager->SendPracticeInitMessageToAllNewPeers();

			SendPracticeStartMessageToAllNewPeers();
		}

		ActiveSequencesUpdate();
		if (switchGameState)
		{
			if (IsReplayOn())
			{
				skipOneReplayFrame = true;
			}
			if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
			{
				PracticeStateChangeMsg pm;
				pm.state = (int)gameState;
				pm.frame = totalGameFrames;
				netplayManager->SendPracticeStateChangeMessageToAllPeers(pm);
			}
			break;
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

		//old spot causes 1 frame discrepancy when loading in for the first time
		//UpdateAllPlayersInput();

		UpdateAllPlayersInput();

		RunFrameForParallelPractice();

		if (!playerAndEnemiesFrozen)
		{
			if (!playerFrozen)
			{
				UpdatePlayersPrePhysics();
			}
		}
		
		//old pos below update inputs
		/*int bonusRes = TryToActivateBonus();
		if (bonusRes == GameSession::GR_BONUS_RESPAWN)
		{
			return false;
		}
		else if (bonusRes == GameSession::GR_EXITLEVEL)
		{
			return false;
		}*/


		ActiveStorySequenceUpdate();

		UpdateInputVis();

		
		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPrePhysics();
			UpdatePhysics();
		}

		/*if (!playerAndEnemiesFrozen)
		{
			if (!playerFrozen)
			{
				UpdatePlayersPostPhysics();
			}
		}

		RecGhostRecordFrame();

		UpdateReplayGhostSprites();

		if (!RunPostUpdate())
		{
			break;
		}

		UpdatePlayerWireQuads();

		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPostPhysics();
		}*/

		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPostPhysics();
		}

		if (!playerAndEnemiesFrozen)
		{
			if (!playerFrozen)
			{
				UpdatePlayersPostPhysics();
			}
		}
		UpdatePlayerWireQuads();

		RecGhostRecordFrame();
		UpdateReplayGhostSprites();

		if (!RunPostUpdate())
		{
			break;
		}

		UpdateGates();

		UpdateTerrainStates();
		UpdateRailStates();

		absorbParticles->Update();
		absorbDarkParticles->Update();

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

		UpdateRunningTimerText();

		alertBox->Update();

		if (gateMarkers != NULL)
			gateMarkers->Update(&cam);

		SteamAPI_RunCallbacks();

		if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession())
		{
			netplayManager->Update();
		}

		fader->Update();
		swiper->Update();

		pokeTriangleScreenGroup->Update();

		if (IsSessTypeGame() )
		{
			if( background != NULL )
				background->Update(view.getCenter());
		}
		UpdateTopClouds();

		mainMenu->UpdateEffects();

		UpdateGoalFlow();

		UpdateEnvParticleSystem();

		//careful because minimap also needs to query
		//to draw things.
		//if something is behind the minimap,
		//the minimaps query might interfere w/ one of these.

		QueryToSpawnEnemies();

		UpdateEnvPlants();

		QueryBorderTree(screenRect);

		QuerySpecialTerrainTree(screenRect);

		//QueryRailDrawTree(screenRect);

		QueryItemTerrainTree(screenRect);

		UpdateDecorSprites();
		UpdateDecorLayers();

		if (UpdateRunModeBackAndStartButtons())
		{

		}

		UpdateZones();

		UpdateEnvShaders( DrawLayer::TERRAIN, true); //havent tested at this position. should work fine.

		totalGameFrames++;
		totalGameFramesIncludingRespawns++;

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
	return true;
}

bool Session::TrySendPracticeSequenceConfirmMessage()
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
	{
		PracticeSequenceConfirmMsg pscm;
		pscm.frame = totalGameFrames;

		netplayManager->SendPracticeSequenceConfirmMessageToAllPeers(pscm);

		return true;
	}

	return false;
}

bool Session::OnlineFrozenGameModeUpdate()
{
	switchGameState = false;

	if (!OneFrameModeUpdate())
	{
		if (!IsParallelSession())
		{
			UpdateControllers();
		}
		
		return true;
	}

	//AddDesyncCheckInfo(); //netplay only

	//ProcessDesyncMessageQueue(); //netplay only

	if (!IsParallelSession())
	{
		UpdateControllers();
	}

	ActiveSequencesUpdate();
	if (switchGameState)
	{
		return false;
	}

	//SteamAPI_RunCallbacks();
	fader->Update();
	swiper->Update();

	pokeTriangleScreenGroup->Update();

	//ggpo_advance_frame(ggpo); //I think you get a weird ggpo assert on framecount without this. game freezes on game ending

	if (gameState != FROZEN)
	{
		return false;
	}

	return true;
}


bool Session::PopupGameModeUpdate()
{
	return true;
}

bool Session::LeaderboardGameModeUpdate()
{
	return true;
}

bool Session::FrozenGameModeUpdate()
{
	while (accumulator >= TIMESTEP)
	{
		currShaderDrawLayer = DrawLayer::INVALID;

		if (!OneFrameModeUpdate())
		{
			UpdateControllers();
			break;
		}

		UpdateControllers();

		RunFrameForParallelPractice();

		ActiveSequencesUpdate();

		SteamAPI_RunCallbacks();

		if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession())
		{
			netplayManager->SendPracticeInitMessageToAllNewPeers();

			SendPracticeStartMessageToAllNewPeers();

			netplayManager->Update();
		}

		fader->Update();
		swiper->Update();

		

		if (cam.manual)
		{
			UpdateCamera();

			UpdateEnvShaders( DrawLayer::TERRAIN, true);
		}

		if (background != NULL)
			background->Update(view.getCenter());

		pokeTriangleScreenGroup->Update();

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
	Sequence *activeSeq = NULL;
	for (int i = 0; i < MAX_SIMULTANEOUS_SEQUENCES; ++i)
	{
		activeSeq = activeSequences[i];

		if (activeSeq != NULL)
		{
			//preScreenTex->setView(uiView);
			for (int j = 0; j < DrawLayer::DrawLayer_Count; ++j)
			{
				View oldView = target->getView();
				target->setView(uiView);
				fader->Draw(j, target);
				target->setView(oldView);
				//swiper->Draw(i, preScreenTex);
				activeSeq->LayeredDraw(j, target);
			}
		}
	}

	

	target->setView(uiView);
	//fader draw was here before

	mainMenu->DrawEffects(target);

	DrawFrameRate(target);
}

bool Session::SequenceGameModeUpdate()
{
	while (accumulator >= TIMESTEP)
	{
		currShaderDrawLayer = DrawLayer::INVALID;

		if (!OneFrameModeUpdate())
		{
			UpdateControllers();
			return true;
		}

		UpdateControllers();

		//UpdateAllPlayersInput(); //added this recently. hopefully doesn't break netplay.

		ActiveSequencesUpdate();

		mainMenu->musicPlayer->Update();

		if (!IsSessTypeEdit())
			SteamAPI_RunCallbacks();

		fader->Update();
		swiper->Update();
		mainMenu->UpdateEffects();
		UpdateEmitters();

		//UpdateEnvShaders(); //havent tested at this position. should work fine.

		//totalGameFrames++;

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

		int extraChildrenSize = n->extraChildren.size();
		for (int i = 0; i < extraChildrenSize; ++i )
		{
			Enemy * e = (Enemy*)(n->extraChildren[i]);

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

void Session::InitGGPO()
{
	desyncCheckerActive = true;


	ggpoReady = false;
	timeSyncFrames = 0;
	//srand(400);
	
	//WSADATA wd = { 0 };
	//WSAStartup(MAKEWORD(2, 2), &wd);

	GGPOSessionCallbacks cb = { 0 };
	cb.begin_game = begin_game_callback;
	cb.advance_frame = advance_frame_callback;
	cb.load_game_state = load_game_state_callback;
	cb.save_game_state = save_game_state_callback;
	cb.free_buffer = free_buffer;
	cb.on_event = on_event_callback;
	cb.log_game_state = log_game_state;
	cb.confirm_frame = confirm_frame;

	assert(ngs == NULL);

	ngs = new GGPONonGameState;
	ggpoPlayers = new GGPOPlayer[4];

	GGPOErrorCode result;

	int frameDelay = 2;

	int numPlayers = matchParams.numPlayers;

	if (numPlayers == -1)
	{
		assert(0);
	}
	//ifstream is;
	//is.open("Resources/ggpotest.txt");
	//is >> frameDelay;
	//is >> ipStr;

	//int sync;
	//is >> sync;


	//int offset = 1, local_player = 0;
	ngs->num_players = numPlayers; //used to be hardcoded to 2

	cout << "initializing ggpo" << endl;

	//bool sync = true;

	if (netplayManager->isSyncTest)
	{
		result = ggpo_start_synctest(&ggpo, &cb, "breakneck_synctest", numPlayers,
			sizeof(int), 1);
	}
	else
	{
		result = ggpo_start_session(&ggpo, &cb, "breakneck", numPlayers, COMPRESSED_INPUT_SIZE );//, localPort);
	}

	//ggpo_log(ggpo, "test\n");
	//result = ggpo_start_session(&ggpo, &cb, "vectorwar", num_players, sizeof(int), localport);
	ggpo_set_disconnect_timeout(ggpo, 0); //3000
	ggpo_set_disconnect_notify_start(ggpo, 1000);
	//int myIndex = 0;
	//int otherIndex = 1;

	/*for (int i = 0; i < netplayManager->numPlayers; ++i)
	{

	}*/

	//this needs to be gone over for more than 2 players etc

	if ( !netplayManager->IsHost() )//!netplayManager->Is() )
	{
		if (gameModeType != MatchParams::GAME_MODE_PARALLEL_RACE)
		{
			//myIndex = 1;
			//otherIndex = 0;
		}
	}

	int playerIndex = netplayManager->playerIndex;

	int normalSkins[4] = { Actor::SKIN_NORMAL, Actor::SKIN_RED, Actor::SKIN_GOLD, Actor::SKIN_BONFIRE };

	for (int i = 0; i < numPlayers; ++i)
	{
		int normalSkin = Actor::SKIN_NORMAL;
		int realIndex = i;

		normalSkin = normalSkins[realIndex];

		netplayManager->netplayPlayers[i].skinIndex = normalSkin;
	}

	netplayManager->netplayPlayers[playerIndex].skinIndex = normalSkins[playerIndex];
	
	//myIndex and otherIndex have nothing to do with playerIndex (which is determined by lobby order currently)

	
	int myIndex = playerIndex;


	

	if (IsParallelGameModeType())
	{
		myIndex = 0;
	}

	cout << "connections:" << endl;
	for (int i = 0; i < 4; ++i)
	{
		cout << i << ": " << netplayManager->netplayPlayers[i].connection << endl;
	}

	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == myIndex)
		{
			ggpoPlayers[i].size = sizeof(ggpoPlayers[i]);
			ggpoPlayers[i].player_num = i + 1;
			ggpoPlayers[i].type = GGPO_PLAYERTYPE_LOCAL;
			cout << "setting local player at: " << i << "\n";
		}
		else
		{
			ggpoPlayers[i].size = sizeof(ggpoPlayers[i]);
			ggpoPlayers[i].player_num = i + 1;
			ggpoPlayers[i].type = GGPO_PLAYERTYPE_REMOTE;


			if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
			{
				//because everyone thinks they're player 0
				int realIndex = i;
				if (realIndex <= playerIndex)
				{
					--realIndex;
				}

				cout << "setting remote player connection to: " << netplayManager->netplayPlayers[realIndex].connection << "\n";
				ggpoPlayers[i].u.remote.connection = netplayManager->netplayPlayers[realIndex].connection;
			}
			else
			{
				cout << "setting remote player but not for parallel race. gamemodetype is: " << gameModeType << "\n";
				ggpoPlayers[i].u.remote.connection = netplayManager->netplayPlayers[i].connection;
			}
			
		}
	}
	//	local_player = myIndex;

	//int ipLen = ipStr.length();
	//for (int i = 0; i < ipLen; ++i)
	//{
	//	ggpoPlayers[otherIndex].u.remote.ip_address[i] = ipStr[i];
	//}
	//ggpoPlayers[otherIndex].u.remote.ip_address[ipLen] = '\0';

	////ggpoPlayers[otherIndex].u.remote.ip_address = ipStr.c_str();
	//ggpoPlayers[otherIndex].u.remote.port = otherPort;


	int i;
	for (i = 0; i < numPlayers; i++) {
		GGPOPlayerHandle handle;
		result = ggpo_add_player(ggpo, ggpoPlayers + i, &handle);
		ngs->playerInfo[i].handle = handle;
		ngs->playerInfo[i].type = ggpoPlayers[i].type;
		if (ggpoPlayers[i].type == GGPO_PLAYERTYPE_LOCAL) {
			ngs->playerInfo[i].connect_progress = 100;
			ngs->local_player_handle = handle;
			ngs->SetConnectState(handle, Connecting);
			ggpo_set_frame_delay(ggpo, handle, frameDelay);
		}
		else {
			ngs->playerInfo[i].connect_progress = 0;
		}
	}


	totalGameFramesIncludingRespawns = 0; //important to reset this whenever you are starting a new ggpo session to avoid desync checker bugs

	if (IsParallelGameModeType())
	{
		if (!IsParallelSession())
		{
			//allows you to respawn with netplay info in mind
			ParallelMode *pm = (ParallelMode*)gameMode;
			pm->SetParallelGGPOSessions(ggpo);

			pm->ResetParallelTotalFramesIncludingRespawns();
			
			Actor *p = GetPlayer(0);
			p->Respawn();

			pm->RespawnParallelPlayers();
		}
	}

	//assert(netplayManager != NULL);
	//cout << "test queue 3: " << (sf::Uint32)netplayManager << "\n";
	//for (auto it = netplayManager->ggpoMessageQueue.begin(); it != netplayManager->ggpoMessageQueue.end(); ++it)
	//{
	//	cout << (*it) << "\n";
	//}
}

void Session::UpdateJustGGPO()
{
	/*assert(netplayManager != NULL);
	cout << "test queue 4: " << (sf::Uint32)netplayManager << "\n";
	for (auto it = netplayManager->ggpoMessageQueue.begin(); it != netplayManager->ggpoMessageQueue.end(); ++it)
	{
		cout << (*it) << "\n";
	}*/

	ggpo_idle(ggpo, 5);

	//ggpo_advance_frame(ggpo);
}

void Session::CleanupGGPO()
{
	if (ngs != NULL)
	{
		delete ngs;
		ngs = NULL;
	}

	if (ggpoPlayers != NULL)
	{
		delete[] ggpoPlayers;
		ggpoPlayers = NULL;
	}
}

GameSession *Session::GetTopParentGame()
{
	if (parentGame == NULL)
	{
		assert(IsSessTypeGame());
		return (GameSession*)this;
	}
	else
	{
		return parentGame->GetTopParentGame();
	}
}

void Session::AddDesyncCheckInfo()
{
	//cout << "add desync check info: " << totalGameFrames << endl;
	if (!desyncCheckerActive)
		return;

	bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();
	assert(ggpoNetplay);

	int totalOnlineFrames = totalGameFramesIncludingRespawns;//ggpo->GetFrameCount();

	if (netplayManager != NULL)
	{
		Actor *p = NULL;
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayerFromNetplayPlayerIndex(i);

			if (p != NULL)
			{
				DesyncCheckInfo dci;
				dci.pos = p->position;
				dci.action = p->action;
				dci.actionFrame = p->frame;
				dci.gameFrame = totalOnlineFrames;//ggpo->//p->sess->totalGameFrames;//totalGameFrames;

				if (!netplayManager->IsHost())
				{
					/*cout << "add desync info for frame: " << totalGameFrames << ", pos: " << dci.pos.x << ", " << dci.pos.y << ", action: " << dci.action
						<< ", actionframe: " << dci.actionFrame << endl;*/
				}
				
				
				netplayManager->AddDesyncCheckInfo(i, dci);
			}

		}

		/*if (netplayManager->IsHost())
		{
			cout << "adding desync checker info for frame: " << totalOnlineFrames << "\n";
		}*/
		

		if (!netplayManager->IsHost() && frameConfirmed )
		{
			//if (totalGameFrames % 2 == 0 && totalGameFrames > 0)
			{
				netplayManager->SendDesyncCheckToHost(totalOnlineFrames);
			}	
		}
	}
}

bool Session::OnlineRunGameModeUpdate()
{
	switchGameState = false;

	currShaderDrawLayer = DrawLayer::INVALID;

	if (!firstUpdateHasHappened)
	{
		firstUpdateHasHappened = true;
	}

	collider.ClearDebug();

	if (!OneFrameModeUpdate())
	{
		//UpdateControllers();
		return true;
	}

	//here rn because runpreupdate can restart the game, and you need to send the desync info before that happens
	//just testing for now
	if (ggpo != NULL)
	{
		AddDesyncCheckInfo(); //netplay only

		ProcessDesyncMessageQueue(); //netplay only
	}

	if (!RunPreUpdate())
		return true;

	if (!playerAndEnemiesFrozen)
	{
		if (!playerFrozen)
		{
			InitPreFrameCalculations();
		}

		UpdateEnemiesPreFrameCalculations();

		if (!playerFrozen)
		{
			UpdatePreFrameCalculations();
		}
	}

	if (pauseFrames > 0)
	{
		HitlagUpdate(); //the full update while in hitlag

		if (frameConfirmed) //doubt this check needs to be here
		{
			//cout << "unintended behavior" << "\n";
			AddDesyncCheckInfo(); //netplay only

			ProcessDesyncMessageQueue(); //netplay only
		}

		if( ggpo != NULL )
			ggpo_advance_frame(ggpo);
		return true;
	}	

	/*if (UpdateRunModeBackAndStartButtons())
	{

	}*/

	ActiveSequencesUpdate();
	if (switchGameState)
	{
		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
		{
			if (!netplayManager->practicePlayers[parallelSessionIndex].HasStateChange())
			{
				assert(netplayManager->practicePlayers[parallelSessionIndex].HasStateChange());
			}
			
			netplayManager->practicePlayers[parallelSessionIndex].ConsumeStateChange();
		}

		//good chance this will be a problem at some point since I moved ggpo_advance_frame out of the normal function for parallel races
		//cout << "switch game state" << endl;
		//if( ggpo != NULL )
		//	ggpo_advance_frame(ggpo);
		return true;
	}

	//original location
	//if (ggpo != NULL)
	//{
	//	AddDesyncCheckInfo(); //netplay only

	//	ProcessDesyncMessageQueue(); //netplay only
	//}

	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	{
		UpdatePlayerInput(parallelSessionIndex);
	}

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

	if (!IsParallelSession())
	{
		fader->Update();
		swiper->Update();
		pokeTriangleScreenGroup->Update();
	}

	if (IsSessTypeGame())
	{
		if( background != NULL )
			background->Update(view.getCenter());
	}
	UpdateTopClouds();

	mainMenu->UpdateEffects();

	UpdateGoalFlow();

	UpdateEnvParticleSystem();

	QueryToSpawnEnemies();

	UpdateEnvPlants();

	QueryBorderTree(screenRect);

	QuerySpecialTerrainTree(screenRect);

	QueryItemTerrainTree(screenRect);

	UpdateDecorSprites();
	UpdateDecorLayers();

	//orig location
	if (UpdateRunModeBackAndStartButtons())
	{

	}

	UpdateZones();

	UpdateEnvShaders( DrawLayer::TERRAIN, true); //havent tested at this position. should work fine.

	//cout << "incrementing total frames from " << totalGameFramesIncludingRespawns << " to " << totalGameFramesIncludingRespawns + 1 << "\n";
	totalGameFrames++;
	totalGameFramesIncludingRespawns++;

	if (gameModeType != MatchParams::GAME_MODE_PARALLEL_RACE)
	{
		if (gameMode->CheckVictoryConditions())
		{
			//ggpo_advance_frame(ggpo);
			gameMode->EndGame();

			//return true;
		}
	}
	

	/*if (gameModeType != MatchParams::GAME_MODE_PARALLEL_RACE )
	{
		ggpo_advance_frame(ggpo);
	}
	else if( isParallelSession )
	{
		ggpo_advance_frame(ggpo);
	}*/

	return true;
}


//bool Session::OnlinePauseMenuModeUpdate()
//{
//
//}

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
void Session::CopyGGPOInputsToParallelSessions()
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->SetParalellGGPOInputs(ggpoCompressedInputs);

		/*for (int i = 0; i < 3; ++i)
		{
			if (prm->parallelGames[i] != NULL)
			{
				for (int pIndex = 0; pIndex < GGPO_MAX_PLAYERS; ++pIndex)
				{
					prm->parallelGames[i]->ggpoCompressedInputs[pIndex] = ggpoCompressedInputs[pIndex];
				}

			}
		}*/
	}
}

void Session::GGPORunFrame()
{
	assert(!IsParallelSession());
	//cout << "ggpo run frame " << endl;
	int disconnect_flags;

	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		ggpoCompressedInputs[i] = 0;
	}

	

	UpdateControllers();

	//CONTROLLERS.Update(); //using updatecontrollers again for online pause menu

	//just turned this off
	//Actor *p = NULL;
	//for (int i = 0; i < 4; ++i)
	//{
	//	GameController *con = GetController(i);

	//	GetCurrInput(i) = con->GetState();
	//	//GetCurrInputUnfiltered(i) = con->GetUnfilteredState();
	//}

	assert(ngs->local_player_handle != GGPO_INVALID_HANDLE);
	
	/*if (ngs->local_player_handle == 0)
	{
		cout << "handle is 0" << endl;
		assert(0);
	}*/

	int pIndex = ngs->local_player_handle - 1;

	if (netplayManager->isSyncTest)
	{
		//cout << "pIndex: " << pIndex << endl;
		if (pIndex < 0)
		{
			pIndex = 0;
		}
	}

	ControllerState testInput;
	Actor *player = GetPlayer(pIndex);

	if (onlinePauseMenuOn)
	{
		testInput = ControllerState();

		if (onlinePauseMenuOn)
		{
			//ControllerState pauseInput = GetCurrInputFiltered(0);
			//ControllerState prevPauseInput = GetPrevInputFiltered(0);
			assert(gameMode->onlinePauseMenu != NULL);

			int res = gameMode->onlinePauseMenu->Update();

			switch (gameModeType)
			{
			case MatchParams::GAME_MODE_FIGHT:
			{
				if (res == 0)
				{
					onlinePauseMenuOn = false;
				}
				else if (res == 1)
				{
					if (IsSessTypeGame())
					{
						GameSession *game = GameSession::GetSession();
						game->resType = GameSession::GR_EXITGAME;
						game->QuitGame();
					}
					//quit
				}
				break;
			}
			case MatchParams::GAME_MODE_PARALLEL_RACE:
			{
				if (res == 0)
				{
					onlinePauseMenuOn = false;
				}
				else if (res == 1)
				{
					testInput.respawnTest = true;
				}
				else if (res == 2)
				{
					if (IsSessTypeGame())
					{
						GameSession *game = GameSession::GetSession();
						game->resType = GameSession::GR_EXITGAME;
						game->QuitGame();
					}
					//quit
				}
				break;
			}
			}
		}
	}
	else
	{
		testInput = GetCurrInputFiltered(0, player);
	}

	COMPRESSED_INPUT_TYPE input = testInput.GetCompressedState();
	
	GGPOErrorCode result = ggpo_add_local_input(ggpo, ngs->local_player_handle, &input, COMPRESSED_INPUT_SIZE);

	//cout << "ggpo run frame: " << result << endl;
	//cout << "local player handle: " << ngs->local_player_handle << "\n";

	//static ControllerState lastCurr;
	
	


	if (GGPO_SUCCEEDED(result))
	{
		//cout << "putting in input " << pIndex << ": " << input << "\n";

		frameConfirmed = false; //to make sure to only send desync checks on confirmed frames
		result = ggpo_synchronize_input(ggpo, (void*)ggpoCompressedInputs, COMPRESSED_INPUT_SIZE * GGPO_MAX_PLAYERS, &disconnect_flags);
		if (GGPO_SUCCEEDED(result))
		{
			//this was used recently
			/*for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
			{
				GetCurrInput(i).SetFromCompressedState(compressedInputs[i]);
			}*/

			CopyGGPOInputsToParallelSessions();


			UpdateAllPlayersInput();

			//assert(gameState == GameState::RUN);

			RunGGPOModeUpdate();

			if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
			{
				ParallelRaceMode *prm = (ParallelRaceMode*)gameMode;
				prm->RunParallelGGPOModeUpdates();
			}

			if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
			{
				if (gameMode->CheckVictoryConditions())
				{
					//ggpo_advance_frame(ggpo);
					gameMode->EndGame();

					//return true;
				}
			}

			ggpo_advance_frame(ggpo);
			
		}
		
	}

	

	//draw after then loop again
}

int Session::GetNumStoredBytes()
{
	int totalSize = 0;

	totalSize += sizeof(SaveGameState);

	for( auto it = gates.begin(); it != gates.end(); ++it )
	{
		totalSize += (*it)->GetNumStoredBytes();
	}

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		totalSize += (*it)->GetNumStoredBytes();
	}

	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			totalSize += p->GetNumStoredBytes();
		}
	}

	totalSize += gameMode->GetNumStoredBytes();

	int enemySize = 0;
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		enemySize += (*it)->GetNumStoredBytes();
	}

	totalSize += enemySize;

	totalSize += absorbParticles->GetNumStoredBytes();
	totalSize += absorbDarkParticles->GetNumStoredBytes();

	for (auto it = allSequencesVec.begin(); it != allSequencesVec.end(); ++it)
	{
		totalSize += (*it)->GetNumStoredBytes();
	}

	for (auto it = allEmittersVec.begin(); it != allEmittersVec.end(); ++it)
	{
		totalSize += (*it)->GetNumStoredBytes();
	}

	//totalSize += deathSeq->GetNumStoredBytes();

	return totalSize;
}

void Session::StoreBytes(unsigned char *bytes)
{
	currSaveState->totalGameFramesIncludingRespawns = totalGameFramesIncludingRespawns;
	currSaveState->totalGameFrames = totalGameFrames;
	currSaveState->activeEnemyListID = GetEnemyID(activeEnemyList);
	currSaveState->activeEnemyListTailID = GetEnemyID(activeEnemyListTail);
	currSaveState->inactiveEnemyListID = GetEnemyID(inactiveEnemyList);
	
	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
	{
		currSaveState->emitterListIDs[i] = GetEmitterID(emitterLists[i]);
	}

	currSaveState->currentZoneID = GetZoneID(currentZone);

	currSaveState->turnTimerOnCounter = turnTimerOnCounter;

	currSaveState->timerOn = timerOn;

	

	currSaveState->phaseOn = phaseOn;
	currSaveState->usedWarp = usedWarp;
	currSaveState->pauseFrames = pauseFrames;
	currSaveState->currSuperPlayerIndex = GetPlayerIndex(currSuperPlayer);
	currSaveState->gameState = gameState;
	currSaveState->nextFrameRestartGame = nextFrameRestartGame;
	for (int i = 0; i < MAX_SIMULTANEOUS_SEQUENCES; ++i)
	{
		currSaveState->activeSequenceID[i] = GetSequenceID(activeSequences[i]);
	}
	
	currSaveState->randomState = randomState;
	currSaveState->cam = cam;



	int saveSize = sizeof(SaveGameState);

	memcpy(bytes, currSaveState, saveSize);
	bytes += saveSize;

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			
			p->StoreBytes(bytes);

			bytes += p->GetNumStoredBytes();
		}
	}

	gameMode->StoreBytes(bytes);
	bytes += gameMode->GetNumStoredBytes();

	int totalEnemySize = 0;
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		totalEnemySize += (*it)->GetNumStoredBytes();
		bytes += (*it)->GetNumStoredBytes();
	}

	absorbParticles->StoreBytes(bytes);
	bytes += absorbParticles->GetNumStoredBytes();

	absorbDarkParticles->StoreBytes(bytes);
	bytes += absorbDarkParticles->GetNumStoredBytes();

	for (auto it = allSequencesVec.begin(); it != allSequencesVec.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	for (auto it = allEmittersVec.begin(); it != allEmittersVec.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	//deathSeq->StoreBytes(bytes);
	//bytes += deathSeq->GetNumStoredBytes();

	/*if (IsParallelSession())
	{
		currSaveState->states[0].Print();
	}*/
}

void Session::SetFromBytes(unsigned char *bytes)
{
	int saveSize = sizeof(SaveGameState);
	memcpy(currSaveState, bytes, saveSize);
	bytes += saveSize;

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	Actor *p = NULL;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->SetFromBytes(bytes);
			bytes += p->GetNumStoredBytes();
		}
	}

	gameMode->SetFromBytes(bytes);
	bytes += gameMode->GetNumStoredBytes();

	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	absorbParticles->SetFromBytes(bytes);
	bytes += absorbParticles->GetNumStoredBytes();

	absorbDarkParticles->SetFromBytes(bytes);
	bytes += absorbDarkParticles->GetNumStoredBytes();

	for (auto it = allSequencesVec.begin(); it != allSequencesVec.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	for (auto it = allEmittersVec.begin(); it != allEmittersVec.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}

	//deathSeq->SetFromBytes(bytes);
	//bytes += deathSeq->GetNumStoredBytes();

	totalGameFramesIncludingRespawns = currSaveState->totalGameFramesIncludingRespawns;
	totalGameFrames = currSaveState->totalGameFrames;
	activeEnemyList = GetEnemyFromID( currSaveState->activeEnemyListID );
	inactiveEnemyList = GetEnemyFromID( currSaveState->inactiveEnemyListID );
	activeEnemyListTail = GetEnemyFromID(currSaveState->activeEnemyListTailID );

	for (int i = 0; i < DrawLayer::DrawLayer_Count; ++i)
	{
		emitterLists[i] = GetEmitterFromID(currSaveState->emitterListIDs[i]);
	}

	currentZone = GetZoneFromID(currSaveState->currentZoneID);

	turnTimerOnCounter = currSaveState->turnTimerOnCounter;

	timerOn = currSaveState->timerOn;

	phaseOn = currSaveState->phaseOn;
	usedWarp = currSaveState->usedWarp;
	pauseFrames = currSaveState->pauseFrames;
	currSuperPlayer = GetPlayer(currSaveState->currSuperPlayerIndex);
	randomState = currSaveState->randomState;

	//when sent through the network, I shouldn't be storing/sending the sess anyway.
	//this is a workaround making sure I don't use the pointer from the other side
	Session *origCamSess = cam.sess;
	GameSession *origCamGame = cam.game;

	cam = currSaveState->cam;

	cam.sess = origCamSess;
	cam.game = origCamGame;


	gameState = (GameState)currSaveState->gameState;
	nextFrameRestartGame = currSaveState->nextFrameRestartGame;
	for (int i = 0; i < MAX_SIMULTANEOUS_SEQUENCES; ++i)
	{
		activeSequences[i] = GetSequenceFromID(currSaveState->activeSequenceID[i]);
	}
	
}

bool Session::SaveState(unsigned char **buffer,
	int *len, int *checksum, int frame)
{
	//cout << "save state: " << totalGameFrames << endl;

	*len = GetNumStoredBytes();
	*buffer = (unsigned char *)malloc(*len);
	memset(*buffer, 0, *len);

	if (!*buffer) {
		return false;
	}

	StoreBytes(*buffer);

	int pSize = sizeof(PState);
	int offset = 0;
	int fletchLen = *len;
	*checksum = fletcher32_checksum((short *)((*buffer)+offset), fletchLen/2);
	return true;
}

bool Session::LoadState(unsigned char *bytes, int len)
{
	//cout << "loading state: " << currSaveState->totalGameFrames << endl;

	int oldTotalGameFrames = totalGameFramesIncludingRespawns;//totalGameFrames;

	SetFromBytes(bytes);

	int rollbackFrames = oldTotalGameFrames - currSaveState->totalGameFramesIncludingRespawns;

	/*cout << "load state: " << totalGameFrames << endl;

	if (gameState != (GameState)currSaveState->gameState)
	{
		cout << "ROLLING BACK GAME STATE BUG??: " << gameState << ", new: " << currSaveState->gameState << endl;
	}
	if (activeSequence != currSaveState->activeSequence)
	{
		cout << "ROLLING BACK SEQUENCE: current: " << activeSequence << ", new: " << currSaveState->activeSequence << endl;
	}*/
	
	if ( netplayManager != NULL)
	{
		bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();
		//assert(ggpoNetplay);

		if (ggpoNetplay)
		{
			cout << "rollback of " << rollbackFrames << " from " << oldTotalGameFrames << " back to " << totalGameFramesIncludingRespawns << "\n";
			//rollback the desync checker system also
			netplayManager->RemoveDesyncCheckInfos(rollbackFrames);
		}
		else
		{
			cout << "loading state for practice mode on frame: " << totalGameFramesIncludingRespawns << "\n";
		}
	}

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
	else if (IsAdventureSession())
	{
		assert(scoreDisplay == NULL);
		scoreDisplay = mainMenu->adventureManager->adventureScoreDisplay;
	}
	else if (IsRushSession())
	{
		assert(scoreDisplay == NULL);
		scoreDisplay = mainMenu->rushManager->rushScoreDisplay;
	}
	else if (scoreDisplay == NULL)
	{
		scoreDisplay = new DefaultScoreDisplay(Vector2f(1920, 0), mainMenu->arial);
	}
	else
	{
		scoreDisplay->Reset();
	}
}

void Session::CleanupScoreDisplay()
{
	if (IsAdventureSession())
	{

	}
	else if (IsRushSession())
	{

	}
	else if ( parentGame == NULL && scoreDisplay != NULL)
	{
		delete scoreDisplay;	
	}

	scoreDisplay = NULL;
}

void Session::EndLevel()
{
	if (postLevelScene != NULL)
	{
		postLevelScene->Reset();
		SetActiveSequence(postLevelScene);
	}
	else
	{
		EndLevelNoScene();
	}
}

void Session::EndLevelNoScene()
{
	if (IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();

		if ( IsRushSession() && mainMenu->rushManager->TryToGoToNextLevel(game))
		{
			//currently jumps immediately to the bonus, so scenes only work for the last one in a world
			//sess->EndLevel();
			return;
		}

	
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

void Session::RecPlayerRecordFrame( int index )
{
	if (playerRecordingManager != NULL)
	{
		playerRecordingManager->RecordReplayFrame(index);
	}
}

void Session::RecGhostRecordFrame()
{
	if (playerRecordingManager != NULL)
	{
		playerRecordingManager->RecordGhostFrames();
	}
}

bool Session::IsMapVersionNewerThanOrEqualTo(int ver1, int ver2)
{
	return (mapHeader->ver1 > ver1 || ( mapHeader->ver1 == ver1 && mapHeader->ver2 >= ver2));
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
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->DrawParallelPlayerShields(preScreenTex);
	}

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


void Session::DrawPlayersHomingBall(sf::RenderTarget *target)
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->DrawParallelPlayerHomingBalls(preScreenTex);
	}

	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawHomingBall(target);
		}
	}
}

void Session::UpdateNameTagsPixelPos(sf::RenderTarget *target)
{
	for (auto it = activePlayerReplayManagers.begin(); it != activePlayerReplayManagers.end(); ++it)
	{
		(*it)->UpdateGhostNameTagsPixelPos(target);
	}

	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->nameTag->UpdatePixelPos(target);
		}
	}

	if (IsParallelGameModeType() && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->UpdateParallelNameTagsPixelPos(target);
	}
}

void Session::DrawNameTags(sf::RenderTarget *target)
{
	//draw ghost name tags first!

	for (auto it = activePlayerReplayManagers.begin(); it != activePlayerReplayManagers.end(); ++it)
	{
		(*it)->DrawGhostNameTags(target);
	}

	if (IsParallelGameModeType() && !IsParallelSession())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;

		for (int i = 0; i < ParallelMode::MAX_PARALLEL_SESSIONS; ++i)
		{
			if (pm->parallelGames[i] != NULL)
			{
				if (netplayManager->practicePlayers[i].isConnectedTo)
				{
					pm->parallelGames[i]->DrawNameTags(target);
				}
			}
		}
	}

	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawNameTag(target);
		}
	}
}

void Session::SetupGameMode()
{
	CleanupGameMode();

	switch (gameModeType)
	{
	case MatchParams::GAME_MODE_BASIC:
		gameMode = new BasicMode;
		break;
	case MatchParams::GAME_MODE_REACHENEMYBASE:
		gameMode = new ReachEnemyBaseMode;
		break;
	case MatchParams::GAME_MODE_FIGHT:
		gameMode = new FightMode;
		break;
	case MatchParams::GAME_MODE_RACE:
		gameMode = new RaceMode;
		break;
	case MatchParams::GAME_MODE_PARALLEL_RACE:
	{
		ParallelRaceMode *prm = new ParallelRaceMode;
		if (!IsParallelSession())
		{
			prm->CreateParallelSessions();
		}
		gameMode = prm;
		break;
	}
	case MatchParams::GAME_MODE_COOP:
		gameMode = new CoopMode;
		break;
	case MatchParams::GAME_MODE_EXPLORE:
		gameMode = new ExploreMode(matchParams.numPlayers);
		break;
	case MatchParams::GAME_MODE_PARALLEL_PRACTICE:
	{
		ParallelPracticeMode *ppm = new ParallelPracticeMode;
		if (!IsParallelSession())
		{
			ppm->CreateParallelSessions();
		}
		gameMode = ppm;
		break;
	}
		
	}
}

V2d Session::GetFuturePlayerPos(int futureFrames, int index )
{
	Actor *p = GetPlayer(index);
	assert(p != NULL);
	assert(futureFrames > 0 && futureFrames < p->numCalculatedFuturePositions- 1);

	return p->futurePositions[futureFrames - 1];
}

void Session::PlayerMustSimulateAtLeast(int f, int index )
{
	Actor *p = GetPlayer(index);
	assert(p != NULL);
	if (p->currFrameSimulationFrames < f)
	{
		p->currFrameSimulationFrames = f;
	}
}


Enemy * Session::GetEnemy(int enType)
{
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
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
	for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
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

void Session::PlayerRestoreAirOptions(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->RestoreAirOptions();
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
		poly->Draw(target); //preScreenTex
		poly = poly->queryNext;
	}
}

void Session::DrawQueriedSpecialTerrain(sf::RenderTarget *target)
{
	PolyPtr sp = specialPieceList;
	while (sp != NULL)
	{
		sp->Draw(target); //preScreenTex
		sp = sp->queryNext;
	}
}

void Session::DrawQueriedRails(sf::RenderTarget *target)
{
	RailPtr r = railDrawList;
	RailPtr next;
	while (r != NULL)
	{
		r->Draw(target);
		next = r->queryNext;
		r->queryNext = NULL;
		r = next;
	}
}

void Session::DrawQueriedRailsToMinimap(sf::RenderTarget *target)
{
	RailPtr r = railDrawList;
	RailPtr next;
	while (r != NULL)
	{
		r->DrawForMinimap(target);
		next = r->queryNext;
		r->queryNext = NULL;
		r = next;
	}
}

V2d Session::CalcBounceReflectionVel(V2d norm, V2d &vel)
{

	/*V2d norm = Normal();
	double reflX = cross(-dir, norm);
	double reflY = dot(-dir, norm);
	V2d edgeDir = Along();

	return normalize(reflX * edgeDir + reflY * norm);*/

	double lenVel = length(vel);

	V2d dir = normalize(vel);
	double reflX = cross(-dir, norm);
	double reflY = dot(-dir, norm);
	
	V2d along(-norm.y, norm.x);

	V2d reflectionDir = normalize(reflX * along + reflY * norm);

	//V2d edgeDir = Along();
	// e->GetReflectionDir(normalize(vel));

	//V2d edgeDir = e->Along();

	double diffCW = GetVectorAngleDiffCW(reflectionDir, along);

	//these next few lines make sure that you cant
	//run up the steep slope and then bounce at such a shallow angle
	//that you push through the terrain
	double thresh = .1; //if the angle is too close to the edgeDir
	if (diffCW > 0 && diffCW < thresh)
	{
		RotateCCW(reflectionDir, thresh - diffCW);
	}

	double diffCCW = GetVectorAngleDiffCCW(reflectionDir, -along);

	if (diffCCW > 0 && diffCCW < thresh)
	{
		RotateCW(reflectionDir, thresh - diffCCW);
	}

	return reflectionDir * lenVel;
}

bool Session::IsWithinBounds(V2d &p)
{
	return (((p.x >= mapHeader->leftBounds)
		&& (p.y >= mapHeader->topBounds)
		&& (p.x <= mapHeader->leftBounds + mapHeader->boundsWidth)
		&& (p.y <= mapHeader->topBounds + mapHeader->boundsHeight)));
}

bool Session::IsWithinBarrierBounds(V2d &p)
{
	for (auto it = barriers.begin(); it != barriers.end(); ++it)
	{
		if (!(*it)->IsPointWithinBarrier(p))
		{
			return false;
		}
	}

	return true;
}

bool Session::IsWithinCurrentBounds(V2d &p)
{
	return (IsWithinBounds(p) && IsWithinBarrierBounds(p));
}

void Session::UpdateTerrainStates()
{

}

void Session::UpdateRailStates()
{

}

//void Session::DrawBossHUD(sf::RenderTarget *target)
//{
//	sf::View oldView = target->getView();
//	target->setView(uiView);
//
//	for (auto it = activeBosses.begin(); it != activeBosses.end(); ++it)
//	{
//		(*it)->DrawHealth(target);
//	}
//
//	target->setView(oldView);
//}

void Session::SetCurrentBoss(Boss *b)
{
	//activeBosses not really used for anything atm
	activeBosses.clear();
	activeBosses.push_back(b);

	if (hud != NULL && hud->hType == HUD::ADVENTURE && !IsParallelSession())
	{
		AdventureHUD *ah = (AdventureHUD*)hud;
		ah->SetBossHealthBar(b->healthBar);
	}
}

void Session::RemoveBoss(Boss *b)
{
	activeBosses.remove(b);

	if (hud != NULL && hud->hType == HUD::ADVENTURE && !IsParallelSession())
	{
		AdventureHUD *ah = (AdventureHUD*)hud;
		ah->bossHealthBar = NULL;
	}
}

void Session::SetKeyMarkerToCurrentZone()
{
	AdventureHUD *ah = (AdventureHUD*)hud;
	
	if (IsParallelSession())
		return;

	if (ah == NULL)
		return;

	ah->numActiveKeyMarkers = 0;

	bool hasEnemyGate = false;
	bool hasKeyGate = false;
	Gate *g;

	

	
	


	for (auto it = currentZone->gates.begin(); it != currentZone->gates.end(); ++it)
	{
		g = (Gate*)(*it)->info;

		if (g->IsLockedForever())
		{
			continue;
		}

		if (!hasKeyGate)
		{
			if (g->category == Gate::ALLKEY || g->category == Gate::NUMBER_KEY)
			{
				hasKeyGate = true;
				
			}
		}
		if (!hasEnemyGate)
		{
			if (g->category == Gate::ENEMY)
			{
				hasEnemyGate = true;
			}
		}
		
	}

	if (hasKeyGate && currentZone->totalNumKeys > 0 )
	{
		bool showMaxKeys = true;
		int keysRequired = -1;
		for (auto it = currentZone->gates.begin(); it != currentZone->gates.end(); ++it)
		{
			g = (Gate*)(*it)->info;

			if (g->IsLockedForever())
			{
				continue;
			}

			if (g->category == Gate::NUMBER_KEY )//&& g->numToOpen < currentZone->totalNumKeys )
			{
				if (keysRequired >= 0)
				{
					if (g->numToOpen != keysRequired)
					{
						showMaxKeys = false;
						break;
					}
				}
				else
				{
					keysRequired = g->numToOpen;
				}
				break;
			}
			else if (g->category == Gate::ALLKEY)
			{
				if (keysRequired >= 0)
				{
					if (currentZone->totalNumKeys != keysRequired)
					{
						showMaxKeys = false;
						break;
					}
				}
				else
				{
					keysRequired = currentZone->totalNumKeys;
				}
			}
		}

		ah->numActiveKeyMarkers++;
		if (showMaxKeys)
		{
			assert(keysRequired >= 0);

			ah->keyMarkers[0]->ShowMaxKeys(keysRequired);
		}
		else
		{
			ah->keyMarkers[0]->HideMaxKeys();
		}
		
		ah->keyMarkers[0]->SetMarkerType(KeyMarker::KEY);
	}

	if (hasEnemyGate && currentZone->allEnemies.size() > 0 )
	{
		ah->numActiveKeyMarkers++;
		if (hasKeyGate)
		{
			ah->keyMarkers[1]->SetMarkerType(KeyMarker::ENEMY);
		}
		else
		{
			ah->keyMarkers[0]->SetMarkerType(KeyMarker::ENEMY);
		}
	}

	for (int i = 0; i < ah->keyMarkers.size(); ++i)
	{
		ah->keyMarkers[i]->Reset();
	}

	if (ah->numActiveKeyMarkers > 0)
	{
		ah->Show(); //the positions are wrong without this
	}
	
	//ah->keyMarker->Reset();
}

void Session::TransitionMusic(MusicInfo *mi,
	int transFrames)
{
	mainMenu->musicPlayer->TransitionMusic(mi, transFrames);
}

void Session::SetupPokeTriangleScreenGroup()
{
	if (parentGame != NULL )
	{
		pokeTriangleScreenGroup = parentGame->pokeTriangleScreenGroup;
	}
	else
	{
		if (pokeTriangleScreenGroup == NULL)
		{
			pokeTriangleScreenGroup = new PokeTriangleScreenGeoGroup;
		}
	}
}

void Session::CleanupPokeTriangleScreenGroup()
{
	if (parentGame != NULL)
	{

	}
	else
	{
		if (pokeTriangleScreenGroup != NULL)
		{
			delete pokeTriangleScreenGroup;
			pokeTriangleScreenGroup = NULL;
		}
	}
}

void Session::HandleMessage(HSteamNetConnection connection, SteamNetworkingMessage_t *steamMsg)
{
	assert(netplayManager != NULL);

	UdpMsg *msg = (UdpMsg*)steamMsg->GetData();

	bool handled = false;

	switch (msg->hdr.type)
	{
	case UdpMsg::Game_Desync_Check:
	{
		assert(netplayManager->IsHost());
		if (netplayManager->IsHost())
		{
			//cout << "pushing to desync queue" << endl;
			netplayManager->desyncMessageQueue.push_back(steamMsg);
			//steamMsg->Release();
			handled = true;
			break;
		}
		else
		{
			steamMsg->Release();
			handled = true;
			break;
		}
	}
	}

	if (!handled)
	{
		netplayManager->HandleMessage(connection, steamMsg);
	}	
}

bool Session::IsGGPOReady()
{
	return ggpoReady;
}

void Session::ProcessDesyncMessageQueue()
{
	if (netplayManager == NULL)
	{
		return;
	}

	if (!desyncCheckerActive)
	{
		for (auto it = netplayManager->desyncMessageQueue.begin(); it != netplayManager->desyncMessageQueue.end(); ++it)
		{
			(*it)->Release();
		}
		netplayManager->desyncMessageQueue.clear();
		return;
	}
	

	//real code below. temporarily turned off.

	for (auto it = netplayManager->desyncMessageQueue.begin(); it != netplayManager->desyncMessageQueue.end(); ++it)
	{
		//cout << "processing desync check" << endl;

		/*UdpMsg *msg = (UdpMsg*)(*it)->GetData();

		int targetPlayerIndex = -1;
		for (int i = 0; i < 4; ++i)
		{
			if (i == netplayManager->playerIndex)
				continue;

			if ( netplayManager->netplayPlayers[i].connection == (*it)->GetConnection())
			{
				targetPlayerIndex = i;
				break;
			}
		}

		assert(targetPlayerIndex >= 0);

		return netplayPlayers[targetPlayerIndex].GetDesyncCheckInfo(framesAgo);*/

		/*int sessTotalFrames = totalGameFrames;
		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
		{
			int parIndex = -1;
			for (int i = 1; i < GGPO_MAX_PLAYERS; ++i)
			{
				assert(ggpoPlayers[i].type == GGPO_PLAYERTYPE_REMOTE);
				if (ggpoPlayers[i].u.remote.connection == (*it)->GetConnection())
				{
					parIndex = i;
					break;
				}
			}

			assert(parIndex >= 0);

			ParallelMode *pm = (ParallelMode*)gameMode;
			sessTotalFrames = pm->parallelGames[parIndex-1]->totalGameFrames;

			cout << "switching out " << totalGameFrames << " for " << sessTotalFrames << "\n";
		}*/

		int sessTotalFrames = totalGameFramesIncludingRespawns;//ggpo->GetFrameCount();

		UdpMsg *msg = (UdpMsg*)(*it)->GetData();
		//int frameDifference = totalGameFrames - msg->u.desync_info.frame_number;
		int frameDifference = sessTotalFrames - msg->u.desync_info.frame_number;

		if (frameDifference >= 0)
		{
			//cout << "frameDifference: " << frameDifference << endl;
			const DesyncCheckInfo & dci = netplayManager->GetDesyncCheckInfo((*it), frameDifference);
			if (msg->u.desync_info.x == dci.pos.x && msg->u.desync_info.y == dci.pos.y && msg->u.desync_info.player_action == dci.action
				&& msg->u.desync_info.player_action_frame == dci.actionFrame )
			//	&& msg->u.desync_info.health == dci.health )
			{
				//cout << "no desync comparing: " << sessTotalFrames << " and " << msg->u.desync_info.frame_number << ", action: " << dci.action << "\n";
				//no desync!
				/*cout << "no desync comparing: " << totalGameFrames << " and " << msg->u.desync_info.frame_number << "\n";
				cout << "my action: " << dci.action << ", their action: " << msg->u.desync_info.player_action << "\n";
				cout << "my action frame: " << dci.actionFrame << ", their action frame: " << msg->u.desync_info.player_action_frame << "\n";
				cout << "my pos: " << dci.pos.x << ", " << dci.pos.y << ", their pos: " << msg->u.desync_info.x << ", " << msg->u.desync_info.y << endl;*/
			}
			else
			{
				//cout << "DESYNC DETECTED comparing " << totalGameFrames << " and " << msg->u.desync_info.frame_number << "\n";
				cout << "DESYNC DETECTED comparing " << sessTotalFrames << " and " << msg->u.desync_info.frame_number << "\n";
				cout << "my action: " << dci.action << ", their action: " << msg->u.desync_info.player_action << "\n";
				cout << "my action frame: " << dci.actionFrame << ", their action frame: " << msg->u.desync_info.player_action_frame << "\n";
				cout << "my pos: " << dci.pos.x << ", " << dci.pos.y << ", their pos: " << msg->u.desync_info.x << ", " << msg->u.desync_info.y << endl;

				netplayManager->desyncDetected = true;

				//netplayManager->DumpDesyncInfo();
			}
		}
		else
		{
			cout << "weird desync check message from the future. sessTotalFrames: " << sessTotalFrames << ", msgnumber: " << msg->u.desync_info.frame_number << endl;
			
			//sync message from the future who cares for now LOL
		}

		(*it)->Release();
	}

	netplayManager->desyncMessageQueue.clear();
	
}

void Session::ConfirmFrame(int frameCheck)
{
	assert(frameConfirmed == false);
	frameConfirmed = true;
	//cout << "confirm frame: " << frameCheck << endl;
}

int Session::GetPlayerNormalSkin(int index)
{
	if (IsReplayOn())
	{
		if (index == 0)
		{
			return activePlayerReplayManagers[0]->GetReplayer(0)->skinIndex;
		}
		else
		{
			return matchParams.playerSkins[index];
		}
	}
	else
	{
		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
		{
			assert(netplayManager != NULL);
			return netplayManager->practicePlayers[parallelSessionIndex].skinIndex;
		}
		else if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && netplayManager != NULL )
		{
			int trueIndex = -1;
			int parIndex = parallelSessionIndex;
			int playerIndex = netplayManager->playerIndex;

			if (parIndex == -1)
			{
				trueIndex = playerIndex;
			}
			else if (parIndex < playerIndex)
			{
				trueIndex = parIndex;
			}
			else
			{
				trueIndex = parIndex + 1;
			}

			return matchParams.playerSkins[trueIndex];
		}

		return matchParams.playerSkins[index];
	}
	
	/*if (IsSessTypeEdit())
	{
		return Actor::SKIN_NORMAL + index;
	}
	else
	{
		return Actor::SKIN_NORMAL;
	}*/
}

int Session::GetRand()
{
	randomState = randomState * 1664525 + 1013904223;
	return randomState >> 24;
}

void Session::SeedRand(uint32 r)
{
	randomState = r;
}

void Session::RunGGPOModeUpdate()
{
	switch (gameState)
	{
	case GameState::RUN:
		OnlineRunGameModeUpdate();
		break;
	case GameState::FROZEN:
		OnlineFrozenGameModeUpdate();
		break;
	}
}

void Session::RunPracticeModeUpdate()
{
	switch (gameState)
	{
	case GameState::RUN:
		UpdatePlayerInput(parallelSessionIndex);
		OnlineRunGameModeUpdate();
		break;
	case GameState::FROZEN:
		OnlineFrozenGameModeUpdate();
		break;
	}
}

void Session::SetMatchPlacings(int p1Placing, int p2Placing, int p3Placing, int p4Placing)
{
	matchPlacings[0] = p1Placing;
	matchPlacings[1] = p2Placing;
	matchPlacings[2] = p3Placing;
	matchPlacings[3] = p4Placing;
}

sf::IntRect Session::GetButtonIconTile(int cIndex, ControllerSettings::ButtonType button )
{
	return mainMenu->GetButtonIconTile(button, controlProfiles[cIndex] );
}

Tileset * Session::GetButtonIconTileset(int cIndex)
{
	return mainMenu->GetButtonIconTileset(controllerStates[cIndex]->GetControllerType());
}

bool Session::IsReplayOn()
{
	return (activePlayerReplayManagers.size() > 0 && activePlayerReplayManagers[0]->IsReplayOn(0));
	//return (playerReplayManager != NULL && playerReplayManager->IsReplayOn(0));
}

bool Session::IsReplayHUDOn()
{
	return IsReplayOn() && activePlayerReplayManagers[0]->IsReplayHUDOn(0);
}

void Session::DrawLeaderboard(sf::RenderTarget *target)
{

}

void Session::StartAlertBox(const std::string &msg)
{
	alertBox->Start(msg);
}

void Session::CleanupBackground()
{
	if ((IsSessTypeGame() && (background != NULL && ownsBG))
		|| (IsSessTypeEdit() && background != NULL ))
	{
		delete background;
		background = NULL;
		ownsBG = true;
	}
}

bool Session::IsParallelGameModeType()
{
	return gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE || gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE;
}

void Session::SetView(const sf::View &p_view)
{
	preScreenTex->setView(p_view);
	extraScreenTex->setView(p_view);
}

const BitField & Session::GetPracticePlayerOptionField()
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	{
		assert(netplayManager != NULL);
		return netplayManager->practicePlayers[parallelSessionIndex].playerOptionField;
	}
	else
	{
		assert(0);
		return defaultStartingPlayerOptionsField;
	}
}

Edge *Session::GetEdge(EdgeInfo * ei)
{
	switch (ei->eiType)
	{
	case EdgeInfo::ETI_EMPTY:
		return NULL;
	case EdgeInfo::ETI_POLY:
		return allPolysVec[ei->ownerIndex]->GetEdge(ei->edgeIndex);
	case EdgeInfo::ETI_RAIL:
		return allRailsVec[ei->ownerIndex]->GetEdge(ei->edgeIndex);
	case EdgeInfo::ETI_GATE:
	{
		Gate *g = gates[ei->ownerIndex];
		if (ei->edgeIndex == 0)
		{
			return g->edgeA;
		}
		else if (ei->edgeIndex == 1)
		{
			return g->edgeB;
		}
		else
		{
			assert(0);
			return  NULL;
		}
	}
	default:
		assert(0);
		return NULL;
	}
}

PolyPtr Session::GetPolyFromID(int id)
{
	if (id < 0)
		return NULL;
	else return allPolysVec[id];
}

RailPtr Session::GetRailFromID(int id)
{
	if (id < 0)
		return NULL;
	else return allRailsVec[id];
}

Enemy *Session::GetEnemyFromID(int index)
{
	if (index < 0)
		return NULL;
	else
	{
		return allEnemiesVec[index];
	}
}

int Session::GetEnemyID(Enemy *e)
{
	if (e == NULL)
		return -1;
	else
	{
		return e->enemyIndex;
	}
}

ShapeEmitter *Session::GetEmitterFromID(int id)
{
	if (id < 0)
		return NULL;
	else
	{
		return allEmittersVec[id];
	}
}

int Session::GetEmitterID(ShapeEmitter *emit)
{
	if (emit == NULL)
	{
		return -1;
	}
	else
	{
		return emit->emitterID;
	}
}

int Session::GetRailID(TerrainRail *tr)
{
	if (tr == NULL)
		return -1;
	else
		return tr->railIndex;
}

int Session::GetPolyID(PolyPtr poly)
{
	if (poly == NULL)
		return -1;
	else
		return poly->polyIndex;
}

int Session::GetSpecialPolyID(PolyPtr poly)
{
	if (poly == NULL)
		return -1;
	else
		return poly->polyIndex;
}

PolyPtr Session::GetSpecialPolyFromID(int id)
{
	if (id < 0)
		return NULL;
	else
		return allSpecialPolysVec[id];
}

int Session::GetPlayerIndex(Actor *p)
{
	if (p == NULL)
		return -1;
	else
		return p->actorIndex;
}

Zone *Session::GetZoneFromID(int id)
{
	if (id < 0)
	{
		return NULL;
	}
	else
	{
		return zones[id];
	}
}

int Session::GetZoneID(Zone *z)
{
	if (z == NULL)
		return -1;
	else
	{
		return z->zoneIndex;
	}
}

int Session::GetComboObjectID(ComboObject *cb)
{
	if (cb == NULL)
		return -1;
	else
		return cb->comboObjectID;
}

ComboObject * Session::GetComboObjectFromID(int id)
{
	if (id < 0)
		return NULL;
	else
		return allComboObjectsVec[id];
}

Sequence *Session::GetSequenceFromID(int id)
{
	if (id < 0)
		return NULL;
	else
		return allSequencesVec[id];
}

int Session::GetSequenceID(Sequence *seq)
{
	if (seq == NULL)
		return -1;
	else
		return seq->sequenceID;
}

int Session::GetGateID(Gate *g)
{
	if (g == NULL)
		return -1;
	else
	{
		return g->gateIndex;
	}
}

Gate *Session::GetGateFromID(int id)
{
	if (id < 0)
		return NULL;
	else
		return gates[id];
}

bool Session::HasPracticeSequenceConfirm()
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession() )
	{
		assert(netplayManager != NULL);
		return netplayManager->practicePlayers[parallelSessionIndex].HasSequenceConfirms();
	}

	return false;
}

void Session::ConsumePracticeSequenceConfirm()
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	{
		assert(netplayManager != NULL);
		netplayManager->practicePlayers[parallelSessionIndex].ConsumeSequenceConfirm();
	}
}

void Session::CleanupGameMode()
{
	if (gameMode != NULL)
	{
		delete gameMode;
		gameMode = NULL;
	}
}

void Session::UpdateWorldDependentTileset( int worldIndex)
{
	if(IsAdventureSession())
	{
		mainMenu->adventureManager->UpdateWorldDependentTileset(worldIndex);
		ts_key = mainMenu->adventureManager->ts_key;
		ts_keyExplode = mainMenu->adventureManager->ts_keyExplode;
		ts_goal = mainMenu->adventureManager->ts_goal;
		ts_goalCrack = mainMenu->adventureManager->ts_goalCrack;
		ts_goalExplode = mainMenu->adventureManager->ts_goalExplode;
		return;
	}
	else if (IsRushSession())
	{
		//mainMenu->rushManager->UpdateWorldDependentTileset(worldIndex);
		ts_key = mainMenu->rushManager->ts_key;
		ts_keyExplode = mainMenu->rushManager->ts_keyExplode;
		ts_goal = mainMenu->rushManager->ts_goal;
		ts_goalCrack = mainMenu->rushManager->ts_goalCrack;
		ts_goalExplode = mainMenu->rushManager->ts_goalExplode;
		return;
	}


	currWorldDependentTilesetWorldIndex = worldIndex;
	if (ts_key != NULL)
	{
		DestroyTileset(ts_key);
		ts_key = NULL;

		DestroyTileset(ts_keyExplode);
		ts_keyExplode = NULL;
	}

	int w = worldIndex + 1;

	if (worldIndex < 8)
	{
		stringstream ss;
		ss << "Enemies/General/Keys/key_w" << w << "_128x128.png";
		stringstream ssExplode;
		ssExplode << "Enemies/General/Keys/keyexplode_w" << w << "_128x128.png";
		ts_key = GetSizedTileset(ss.str());
		ts_keyExplode = GetSizedTileset(ssExplode.str());
	}
	else
	{
		ts_key = GetSizedTileset("Enemies/General/Keys/key_w1_128x128.png");
		ts_keyExplode = GetSizedTileset("Enemies/General/Keys/keyexplode_w1_128x128.png");
	}

	if (ts_goal != NULL)
	{
		DestroyTileset(ts_goal);
		ts_goal = NULL;
	}

	if (ts_goalCrack != NULL)
	{
		DestroyTileset(ts_goalCrack);
		ts_goalCrack = NULL;
	}
	
	if (ts_goalExplode != NULL)
	{
		DestroyTileset(ts_goalExplode);
		ts_goalExplode = NULL;
	}
	

	//if (worldIndex < 1)
	{
		stringstream ss;
		ss << "Enemies/General/Goal/goal_w" << w << "_a_512x512.png";
		stringstream ssCrack;
		ssCrack << "Enemies/General/Goal/goal_w" << w << "_b_512x512.png";
		stringstream ssExplode;
		ssExplode << "Enemies/General/Goal/goal_w" << w << "_c_512x512.png";
		ts_goal = GetSizedTileset(ss.str());
		ts_goalCrack = GetSizedTileset(ssCrack.str());
		ts_goalExplode = GetSizedTileset(ssExplode.str());
	}
	/*else
	{
		ts_goal = GetSizedTileset("Goal/goal_w_1_a_512x512.png");
		ts_goalExplode = GetSizedTileset("Goal/goal_w_1_b_512x512.png");
	}*/


}

Actor *Session::GetPlayerFromNetplayPlayerIndex(int index)
{
	Actor *p = NULL;

	if (netplayManager != NULL)
	{
		int playerParIndex = -1;
		int myPlayerIndex = netplayManager->playerIndex;

		if (matchParams.gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
		{
			if (index == myPlayerIndex)
			{
				playerParIndex = 0;
			}
			else if (index > myPlayerIndex)
			{
				playerParIndex = index;
			}
			else
			{
				playerParIndex = index + 1;
			}

			if (playerParIndex == 0)
			{
				p = GetPlayer(0);
			}
			else
			{
				ParallelRaceMode *prm = (ParallelRaceMode*)gameMode;

				if (prm->parallelGames[playerParIndex - 1] != NULL)
				{
					p = prm->parallelGames[playerParIndex - 1]->GetPlayer(0);
				}
			}
		}
		else
		{
			p = GetPlayer(index);
		}
	}

	return p;
}

std::string Session::GetMapPreviewPath()
{
	return filePath.parent_path().string() + "\\" + filePath.stem().string() + ".png";
}

void Session::HideHUD(int frames)
{
	if (!IsParallelSession())
	{
		hud->Hide(frames);
	}
	
	timerOn = false;
}

void Session::ShowHUD(int frames)
{
	if (!IsParallelSession())
	{
		hud->Show(frames);
	}
	
	if (frames == 0)
	{
		timerOn = true;
	}
	else
	{
		turnTimerOnCounter = frames;
	}
}

void Session::DrawReplayGhosts(sf::RenderTarget *target)
{
	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void Session::UpdateReplayGhostSprites()
{
	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->UpdateReplaySprite();
	}
}

void Session::ClearReplayGhosts()
{
	replayGhosts.clear();
}

void Session::StartGoalPulse(sf::Vector2f pos)
{
	assert(goalPulse != NULL);

	goalPulse->SetPosition(pos);
	goalPulse->StartPulse();
}

void Session::SendPracticeStartMessageToAllNewPeers()
{
	//sends the start to message to any new peers that join
	PracticeStartMsg psm;
	psm.skinIndex = GetPlayerNormalSkin(0);
	psm.SetPlayerOptionField(currPlayerOptionsField);//GetPlayer(0)->bStartHasUpgradeField);	
	psm.SetLogField(currLogField);
	psm.startFrame = totalGameFrames;
	psm.wantsToPlay = netplayManager->wantsToPracticeRace;
	psm.origProgression = originalProgressionModeOn;
	netplayManager->SendPracticeStartMessageToAllNewPeers(psm);
}

void Session::DrawPracticeSessions(sf::RenderTarget *target, sf::View practiceView )
{
	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
	{
		//extraScreenTex->clear(Color::Transparent);
		extraScreenTex->setView(practiceView);
		ParallelMode *pm = (ParallelMode*)gameMode;

		for (int i = 0; i < ParallelMode::MAX_PARALLEL_SESSIONS; ++i)
		{
			if (pm->parallelGames[i] != NULL)
			{
				if (netplayManager->practicePlayers[i].isConnectedTo)
				{
					pm->parallelGames[i]->DrawPracticeGame(extraScreenTex);
				}
			}
		}

		extraScreenTex->display();
		const Texture &extraTex = extraScreenTex->getTexture();
		Sprite extraSprite(extraTex);

		int alpha = 255;//150;

		extraSprite.setColor(Color(255, 255, 255, 150));

		extraSprite.setPosition(-960 / 2, -540 / 2);
		extraSprite.setScale(.5, .5);
		extraSprite.setTexture(extraTex);

		View oldView = target->getView();

		target->setView(window->getView());

		//preScreenTex->draw(extraSprite);
		target->draw(extraSprite);

		target->setView(oldView);
	}
}

bool Session::HasLevelFinisher()
{
	if (goal != NULL || shipPickup != NULL)
		return true;

	return false;
}

V2d Session::GetLevelFinisherPos()
{
	if (goal != NULL)
	{
		return goal->GetPosition();
	}
	else if (shipPickup != NULL)
	{
		return shipPickup->GetPosition();
	}

	assert(0);

	return V2d(0, 0);
}

void Session::ClearActiveSequences()
{
	for (int i = 0; i < MAX_SIMULTANEOUS_SEQUENCES; ++i)
	{
		activeSequences[i] = NULL;
	}
}