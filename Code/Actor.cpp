#include "Actor.h"
#include "GameSession.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>
#include "PowerOrbs.h"
#include "Sequence.h"
#include <list>
#include <fstream>
#include "MainMenu.h" //just for glsl color macro
#include "PlayerRecord.h"
#include "VisualEffects.h"
#include "Enemy.h"
#include "Enemy_Booster.h"
#include "HitboxManager.h"
#include "ImageText.h"
#include "KeyMarker.h"
#include "ScoreDisplay.h"
#include "WorldMap.h"
#include "SaveFile.h"
#include "PauseMenu.h"
#include "KinMenu.h"
#include "GroundTrigger.h"
#include "Enemy_Comboer.h"
#include "Enemy_Spring.h"
#include "AirTrigger.h"
#include "Nexus.h"
#include "HUD.h"
#include "VisualEffects.h"
#include "MapHeader.h"
#include "TouchGrass.h"
#include "Enemy_GravityModifier.h"
#include "ParticleEffects.h"
#include "Enemy_HealthFly.h"
#include "StorySequence.h"
#include "Enemy_BounceBooster.h"
#include "Enemy_Teleporter.h"
#include "Wire.h"
#include "Enemy_SwingLauncher.h"
#include "Grass.h"
#include "EnvPlant.h"
#include "Barrier.h"
#include "AbsorbParticles.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "MovingGeo.h"
#include "GateMarker.h"

using namespace sf;
using namespace std;

#define COLOR_TEAL Color( 0, 0xee, 0xff )

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

//#define SETUP_ACTION_FUNCS(VAR) SetupFuncsForAction(##VAR,&Actor::##VAR_Start,&Actor::X_End,&Actor::X_Change,&Actor::X_Update,&Actor::X_UpdateSprite,&Actor::X_TransitionToAction,&Actor::X_TimeIndFrameInc,&Actor::X_TimeDepFrameInc,&Actor::X_GetActionLength,&Actor::X_GetTileset);

void KeyExplodeUpdater::OnDeactivate(EffectInstance *ei)
{
	actor->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
		actor->ts_keyExplode, V2d(ei->pos), true, 0, 6, 3, true);
}

//#define cout std::cout

void Actor::SetSession(Session *p_sess,
	GameSession *game,
	EditSession *edit)
{
	sess = p_sess;
	owner = game;
	editOwner = edit;
}

void Actor::SetupDrain()
{
	float numSecondsToDrain = sess->mapHeader->drainSeconds;
	float drainPerSecond = totalHealth / numSecondsToDrain;
	float drainPerFrame = drainPerSecond / 60.f;
	float drainFrames = 1.f;
	if (drainPerFrame < 1.f)
	{
		drainFrames = floor(1.f / drainPerFrame);
	}
	drainCounterMax = drainFrames;
	if (drainPerFrame < 1.0)
		drainPerFrame = 1.0;
	drainAmount = drainPerFrame;
}


sf::SoundBuffer * Actor::GetSound(const std::string &name)
{
	return sess->soundManager->GetSound(name);
}

map<int, list<CollisionBox>> & Actor::GetHitboxList(const string & str)
{
	return sess->hitboxManager->GetHitboxList(str);
}

void Actor::UpdatePowersMenu()
{
	if (owner != NULL)
	{
		owner->mainMenu->pauseMenu->kinMenu->UpdatePowers(this);
	}
}

QuadTree *Actor::GetTerrainTree()
{
	return sess->terrainTree;
	/*if (owner != NULL)
		return owner->terrainTree;
	else if( editOwner != NULL )
	{
		return editOwner->terrainTree;
	}*/
}

QuadTree *Actor::GetSpecialTerrainTree()
{
	return sess->specialTerrainTree;
}

QuadTree *Actor::GetRailEdgeTree()
{
	return sess->railEdgeTree;
	/*if (owner != NULL)
		return owner->railEdgeTree;
	else if (editOwner != NULL)
	{
		return editOwner->railEdgeTree;
	}*/
}

QuadTree *Actor::GetBarrierTree()
{
	return sess->barrierTree;
	/*if (owner != NULL)
		return owner->barrierTree;
	else if (editOwner != NULL)
	{
		return editOwner->barrierTree;
	}*/
}

QuadTree *Actor::GetBorderTree()
{
	return sess->borderTree;
	/*if (owner != NULL)
		return owner->borderTree;
	else if (editOwner != NULL)
	{
		return editOwner->borderTree;
	}*/
}

int Actor::GetTotalGameFrames()
{
	return sess->totalGameFrames;
	/*if (owner != NULL)
		return owner->totalGameFrames;
	else
		return editOwner->totalGameFrames;*/
}

void Actor::SetToOriginalPos()
{
	if (owner != NULL)
	{
		position = V2d(owner->playerOrigPos);
	}
	else if (editOwner != NULL)
	{
		position = editOwner->GetPlayerSpawnPos();
	}
}

SoundNode * Actor::ActivateSound(SoundType st, bool loop )
{
	SoundBuffer *sb = soundBuffers[st];

	if (sb == NULL)
		return NULL;

	return sess->soundNodeList->ActivateSound(sb, loop);
}

void Actor::DeactivateSound(SoundNode *sn)
{
	if( owner != NULL )
		owner->soundNodeList->DeactivateSound(sn);
}

BasicEffect * Actor::ActivateEffect(
	EffectLayer layer,
	Tileset *ts,
	sf::Vector2<double> pos,
	bool pauseImmune,
	double angle,
	int frameCount,
	int animationFactor,
	bool right,
	int startFrame,
	float depth)
{
	return sess->ActivateEffect(layer, ts, pos, pauseImmune, angle, frameCount, animationFactor,
		right, startFrame, depth);
}

GameController &Actor::GetController(int index)
{
	if (owner != NULL)
	{
		return owner->GetController(actorIndex);
	}
	else if (editOwner != NULL)
	{
		return editOwner->mainMenu->GetController(actorIndex);
	}
	else
	{
		assert(0);
		return owner->GetController(actorIndex);
	}
}

void Actor::SetCurrHitboxes(CollisionBody *cBody,
	int p_frame)
{
	if (cBody != NULL)
	{
		currHitboxes = cBody;
		currHitboxFrame = p_frame;
	}
}

Collider &Actor::GetCollider()
{
	return sess->collider;
}

void Actor::SetupFXTilesets()
{
	string folder = "Kin/FX/";

	ts_fairSwordLightning[0] = sess->GetSizedTileset( folder, "fair_sword_lightninga_256x256.png", swordSkin);
	ts_fairSwordLightning[1] = sess->GetSizedTileset( folder, "fair_sword_lightninga_256x256.png", swordSkin);
	ts_fairSwordLightning[2] = sess->GetSizedTileset( folder, "fair_sword_lightninga_256x256.png", swordSkin);

	ts_dairSwordLightning[0] = sess->GetSizedTileset(folder,"dair_sword_lightninga_256x256.png", swordSkin);
	ts_dairSwordLightning[1] = sess->GetSizedTileset(folder,"dair_sword_lightninga_256x256.png", swordSkin);
	ts_dairSwordLightning[2] = sess->GetSizedTileset(folder,"dair_sword_lightninga_256x256.png", swordSkin);

	ts_uairSwordLightning[0] = sess->GetSizedTileset(folder,"uair_sword_lightninga_256x256.png", swordSkin);
	ts_uairSwordLightning[1] = sess->GetSizedTileset(folder,"uair_sword_lightninga_256x256.png", swordSkin);
	ts_uairSwordLightning[2] = sess->GetSizedTileset(folder,"uair_sword_lightninga_256x256.png", swordSkin);

	ts_bounceBoost = sess->GetSizedTileset(folder, "bounceboost_256x192.png", skin);

	ts_fx_hurtSpack = sess->GetSizedTileset(folder, "hurt_spack_128x160.png");

	ts_fx_dashStart = sess->GetSizedTileset(folder, "fx_dashstart_160x160.png");
	ts_fx_dashRepeat = sess->GetSizedTileset(folder, "fx_dashrepeat_192x128.png");

	ts_fx_land[0] = sess->GetSizedTileset(folder, "land_a_128x128.png");
	ts_fx_land[1] = sess->GetSizedTileset(folder,"land_b_192x208.png");
	ts_fx_land[2] = sess->GetSizedTileset(folder,"land_c_256x256.png");
	ts_fx_runStart[0] = sess->GetSizedTileset(folder,"runstart_a_128x128.png");
	ts_fx_runStart[1] = sess->GetSizedTileset(folder,"runstart_b_224x224.png");
	ts_fx_runStart[2] = sess->GetSizedTileset(folder,"runstart_c_224x224.png");

	ts_fx_sprint = sess->GetSizedTileset(folder, "fx_sprint_176x176.png");
	ts_fx_run = sess->GetSizedTileset(folder, "fx_run_144x128.png");
	ts_fx_bigRunRepeat = sess->GetSizedTileset(folder, "fx_bigrunrepeat_176x112.png");

	ts_fx_jump[0] = sess->GetSizedTileset(folder,"jump_a_128x80.png");
	ts_fx_jump[1] = sess->GetSizedTileset(folder,"jump_b_160x192.png");
	ts_fx_jump[2] = sess->GetSizedTileset(folder,"jump_c_160x192.png");

	ts_fx_wallJump[0] = sess->GetSizedTileset(folder,"walljump_a_160x160.png");
	ts_fx_wallJump[1] = sess->GetSizedTileset(folder,"walljump_b_224x224.png");
	ts_fx_wallJump[2] = sess->GetSizedTileset(folder,"walljump_c_224x224.png");
	ts_fx_double = sess->GetSizedTileset(folder, "fx_doublejump_196x160.png");
	ts_fx_gravReverse = sess->GetSizedTileset(folder, "fx_grav_reverse_128x128.png");

	ts_fx_chargeBlue0 = sess->GetSizedTileset(folder,"elec_01_128x128.png");
	ts_fx_chargeBlue1 = sess->GetSizedTileset(folder,"elec_03_128x128.png");
	ts_fx_chargeBlue2 = sess->GetSizedTileset(folder,"elec_04_128x128.png");
	ts_fx_chargePurple = sess->GetSizedTileset(folder, "elec_02_128x128.png");


	ts_fx_rightWire = sess->GetSizedTileset(folder, "wire_boost_r_64x64.png");
	ts_fx_leftWire = sess->GetSizedTileset(folder, "wire_boost_b_64x64.png");
	ts_fx_doubleWire = sess->GetSizedTileset(folder, "wire_boost_m_64x64.png");

	ts_fx_airdashDiagonal = sess->GetSizedTileset(folder, "fx_airdash_diagonal_1_128x128.png");
	ts_fx_airdashUp = sess->GetSizedTileset(folder, "fx_airdash_128x128.png");
	ts_fx_airdashHover = sess->GetSizedTileset(folder, "fx_airdash_hold_1_96x80.png");

	ts_fx_death_1a = sess->GetSizedTileset(folder,"death_fx_1a_256x256.png");
	ts_fx_death_1b = sess->GetSizedTileset(folder,"death_fx_1b_128x80.png");
	ts_fx_death_1c = sess->GetSizedTileset(folder,"death_fx_1c_128x128.png");
	ts_fx_death_1d = sess->GetSizedTileset(folder,"death_fx_1d_48x48.png");
	ts_fx_death_1e = sess->GetSizedTileset(folder,"death_fx_1e_160x160.png");
	ts_fx_death_1f = sess->GetSizedTileset(folder,"death_fx_1f_160x160.png");

	ts_fx_gateEnter = sess->GetSizedTileset(folder, "gateenter_256x320.png");

	for (int i = 0; i < 7; ++i)
	{
		smallLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 4, 1.f);
	}

	smallLightningPool[0]->ts = sess->GetSizedTileset(folder,"elec_01_96x96.png");
	smallLightningPool[1]->ts = sess->GetSizedTileset(folder,"elec_02_96x96.png");
	smallLightningPool[2]->ts = sess->GetSizedTileset(folder,"elec_03_96x96.png");
	smallLightningPool[3]->ts = sess->GetSizedTileset(folder,"elec_04_96x96.png");
	smallLightningPool[4]->ts = sess->GetSizedTileset(folder,"elec_05_96x96.png");
	smallLightningPool[5]->ts = sess->GetSizedTileset(folder,"elec_06_96x96.png");
	smallLightningPool[6]->ts = sess->GetSizedTileset(folder,"elec_07_96x96.png");

	for (int i = 0; i < 3; ++i)
	{
		fairLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 20, 1.f);
		fairLightningPool[i]->ts = sess->GetSizedTileset(folder, "fair_sword_lightninga_256x256.png");
		dairLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 20, 1.f);
		dairLightningPool[i]->ts = sess->GetSizedTileset(folder, "dair_sword_lightninga_256x256.png");
		uairLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 20, 1.f);
		uairLightningPool[i]->ts = sess->GetSizedTileset(folder, "uair_sword_lightninga_256x256.png");
	}

	gateBlackFXPool = new EffectPool(EffectType::FX_RELATIVE, 2, 1.f);
	gateBlackFXPool->ts = sess->GetSizedTileset(folder, "keydrain_160x160.png");

	ts_key = sess->GetSizedTileset("FX/key_128x128.png");

	keyExplodePool = new EffectPool(EffectType::FX_REGULAR, 32, 1.f);
	keyExplodePool->ts = ts_key;
	keyExplodeUpdater = new KeyExplodeUpdater(this);
	keyExplodePool->SetUpdater(keyExplodeUpdater);
	ts_keyExplode = sess->GetSizedTileset("FX/keyexplode_128x128.png");

	keyExplodeRingGroup = new MovingGeoGroup;
	keyExplodeRingGroup->AddGeo(new MovingRing(32, 20, 300, 12, 12, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	keyExplodeRingGroup->Init();
}

void Actor::SetupSwordTilesets()
{
	string folder = "Sword/";

	ts_fairSword[0] = sess->GetSizedTileset(folder, "fair_sworda_256x256.png", swordSkin);
	ts_fairSword[1] = sess->GetSizedTileset(folder, "fair_swordb_288x288.png", swordSkin);
	ts_fairSword[2] = sess->GetSizedTileset(folder, "fair_swordc_384x384.png", swordSkin);

	ts_dairSword[0] = sess->GetSizedTileset(folder, "dair_sworda_256x256.png", swordSkin);
	ts_dairSword[1] = sess->GetSizedTileset(folder, "dair_swordb_288x288.png", swordSkin);
	ts_dairSword[2] = sess->GetSizedTileset(folder, "dair_swordc_384x384.png", swordSkin);

	ts_uairSword[0] = sess->GetSizedTileset(folder, "uair_sworda_256x256.png", swordSkin);
	ts_uairSword[1] = sess->GetSizedTileset(folder, "uair_swordb_288x288.png", swordSkin);
	ts_uairSword[2] = sess->GetSizedTileset(folder, "uair_swordc_320x320.png", swordSkin);

	ts_grindLungeSword[0] = sess->GetSizedTileset(folder, "grind_lunge_sworda_160x160.png", swordSkin);
	ts_grindLungeSword[1] = sess->GetSizedTileset(folder, "grind_lunge_swordb_192x192.png", swordSkin);
	ts_grindLungeSword[2] = sess->GetSizedTileset(folder, "grind_lunge_swordc_224x208.png", swordSkin);

	ts_standingNSword[0] = sess->GetSizedTileset(folder, "stand_sworda_208x208.png", swordSkin);
	ts_standingNSword[1] = sess->GetSizedTileset(folder, "stand_swordb_304x176.png", swordSkin);
	ts_standingNSword[2] = sess->GetSizedTileset(folder, "stand_swordc_304x192.png", swordSkin);

	ts_wallAttackSword[0] = sess->GetSizedTileset(folder, "wall_sworda_144x256.png", swordSkin);
	ts_wallAttackSword[1] = sess->GetSizedTileset(folder, "wall_swordb_240x352.png", swordSkin);
	ts_wallAttackSword[2] = sess->GetSizedTileset(folder, "wall_swordc_298x400.png", swordSkin);

	ts_steepSlideAttackSword[0] = sess->GetSizedTileset(folder, "steep_att_sworda_480x176.png", swordSkin);
	ts_steepSlideAttackSword[1] = sess->GetSizedTileset(folder, "steep_att_swordb_352x192.png", swordSkin);
	ts_steepSlideAttackSword[2] = sess->GetSizedTileset(folder, "steep_att_swordc_560x256.png", swordSkin);

	ts_steepClimbAttackSword[0] = sess->GetSizedTileset(folder, "climb_att_sworda_352x128.png", swordSkin);
	ts_steepClimbAttackSword[1] = sess->GetSizedTileset(folder, "climb_att_swordb_416x320.png", swordSkin);
	ts_steepClimbAttackSword[2] = sess->GetSizedTileset(folder, "climb_att_swordc_496x208.png", swordSkin);

	ts_diagUpSword[0] = sess->GetSizedTileset(folder, "airdash_u_sword_144x208.png", swordSkin);
	ts_diagUpSword[1] = sess->GetSizedTileset(folder, "airdash_u_sword_b_224x240.png", swordSkin);
	ts_diagUpSword[2] = sess->GetSizedTileset(folder, "airdash_u_sword_p_320x384.png", swordSkin);

	ts_diagDownSword[0] = sess->GetSizedTileset(folder, "airdash_sword_128x208.png", swordSkin);
	ts_diagDownSword[1] = sess->GetSizedTileset(folder, "airdash_sword_b_224x240.png", swordSkin);
	ts_diagDownSword[2] = sess->GetSizedTileset(folder, "airdash_sword_p_320x384.png", swordSkin);
}

void Actor::SetupExtraTilesets()
{
	string folder = "Kin/";

	ts_scorpRun = sess->GetSizedTileset( folder, "scorp_run_192x128.png");
	ts_scorpSlide = sess->GetSizedTileset( folder, "scorp_slide_160x96.png");
	ts_scorpSteepSlide = sess->GetSizedTileset( folder, "scorp_steep_slide_224x128.png");
	ts_scorpStart = sess->GetSizedTileset( folder, "scorp_start_256x256.png");
	ts_scorpStand = sess->GetSizedTileset( folder, "scorp_stand_224x128.png");
	ts_scorpJump = sess->GetSizedTileset( folder, "scorp_jump_192x144.png");
	ts_scorpDash = sess->GetSizedTileset( folder, "scorp_dash_192x80.png");
	ts_scorpSprint = sess->GetSizedTileset( folder, "scorp_sprint_192x96.png");
	ts_scorpClimb = sess->GetSizedTileset( folder, "scorp_climb_256x128.png");
	ts_bubble = sess->GetSizedTileset( folder, "time_bubble_128x128.png");

	ts_airBounceFlame = sess->GetSizedTileset( folder, "bouncejumpflame_128x128.png", skin);
	ts_runBounceFlame = sess->GetSizedTileset( folder, "bouncerunflame_128x96.png", skin);

	ts_dodecaSmall = sess->GetSizedTileset( folder, "dodecasmall_180x180.png", skin);
	ts_dodecaBig = sess->GetSizedTileset( folder, "dodecabig_360x360.png", skin);
	tsgsdodeca = sess->GetSizedTileset( folder, "dodeca_64x64.png");
	tsgstriblue = sess->GetSizedTileset( folder, "triblue_64x64.png");
	tsgstricym = sess->GetSizedTileset( folder, "tricym_128x128.png");
	tsgstrigreen = sess->GetSizedTileset( folder, "trigreen_64x64.png");
	tsgstrioran = sess->GetSizedTileset( folder, "trioran_128x128.png");
	tsgstripurp = sess->GetSizedTileset( folder, "tripurp_128x128.png");
	tsgstrirgb = sess->GetSizedTileset( folder, "trirgb_128x128.png");

	if (owner != NULL)
	{
		ts_exitAura = owner->mainMenu->tilesetManager.GetTileset("Kin/exitaura_256x256.png", 256, 256);
		exitAuraSprite.setTexture(*ts_exitAura->texture);
	}
}

int Actor::GetActionLength(int a)
{
	if (getActionLengthFuncs[a] != NULL)
	{
		return (this->*getActionLengthFuncs[a])();
	}

	return 0;
}

void Actor::ActionTimeIndFrameInc()
{
	if (timeIndFrameIncFuncs[action] != NULL)
	{
		(this->*timeIndFrameIncFuncs[action])();
	}
}

void Actor::ActionTimeDepFrameInc()
{
	if (timeDepFrameIncFuncs[action] != NULL)
	{
		(this->*timeDepFrameIncFuncs[action])();
	}
}

void Actor::SetupActionTilesets()
{
	for (int i = 0; i < Count; ++i)
	{
		tileset[i] = NULL;
	}

	Tileset *temp;
	for (int i = 0; i < Count; ++i)
	{
		if (getTilesetFuncs[i] != NULL)
		{
			temp = (this->*getTilesetFuncs[i])();
			if (temp != NULL)
			{
				tileset[i] = temp;
			}
		}
	}
}

void Actor::SetupTilesets()
{
	SetupFXTilesets();
	SetupSwordTilesets();
	SetupExtraTilesets();
	SetupActionTilesets();
}

void Actor::Init()
{
	if (owner != NULL)
	{
		SetAction(INTROBOOST);//INTRO
		frame = 0;
	}
	else
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::SetupFuncsForAction(
	int a,
	void(Actor::* start)(),
	void(Actor::* end) (),
	void(Actor::* change)(),
	void(Actor::* update)(),
	void(Actor::* updateSprite)(),
	void(Actor::* transitionToAction)(int),
	void(Actor::* timeIndInc)(),
	void(Actor::* timeDepInc)(),
	int(Actor::* getActionLength)(),
	Tileset*(Actor::* getTileset)() )
{
	startActionFuncs[a] = start;
	endActionFuncs[a] = end;
	changeActionFuncs[a] = change;
	updateActionFuncs[a] = update;
	updateSpriteFuncs[a] = updateSprite;
	transitionFuncs[a] = transitionToAction;
	timeIndFrameIncFuncs[a] = timeIndInc;
	timeDepFrameIncFuncs[a] = timeDepInc;
	getActionLengthFuncs[a] = getActionLength;
	getTilesetFuncs[a] = getTileset;
}

Tileset *Actor::GetActionTileset(const std::string &fn)
{
	return sess->GetSizedTileset(actionFolder, fn, skin);
}

void Actor::SetupActionFunctions()
{
	startActionFuncs.resize(Count);
	endActionFuncs.resize(Count);
	changeActionFuncs.resize(Count);
	updateActionFuncs.resize(Count);
	updateSpriteFuncs.resize(Count);
	transitionFuncs.resize(Count);
	timeIndFrameIncFuncs.resize(Count);
	timeDepFrameIncFuncs.resize(Count);
	getActionLengthFuncs.resize(Count);
	getTilesetFuncs.resize(Count);

	SetupFuncsForAction(AIRDASH,
		&Actor::AIRDASH_Start,
		&Actor::AIRDASH_End,
		&Actor::AIRDASH_Change,
		&Actor::AIRDASH_Update,
		&Actor::AIRDASH_UpdateSprite,
		&Actor::AIRDASH_TransitionToAction,
		&Actor::AIRDASH_TimeIndFrameInc,
		&Actor::AIRDASH_TimeDepFrameInc,
		&Actor::AIRDASH_GetActionLength,
		&Actor::AIRDASH_GetTileset);

	SetupFuncsForAction(AIRHITSTUN,
		&Actor::AIRHITSTUN_Start,
		&Actor::AIRHITSTUN_End,
		&Actor::AIRHITSTUN_Change,
		&Actor::AIRHITSTUN_Update,
		&Actor::AIRHITSTUN_UpdateSprite,
		&Actor::AIRHITSTUN_TransitionToAction,
		&Actor::AIRHITSTUN_TimeIndFrameInc,
		&Actor::AIRHITSTUN_TimeDepFrameInc,
		&Actor::AIRHITSTUN_GetActionLength,
		&Actor::AIRHITSTUN_GetTileset);

	SetupFuncsForAction(AUTORUN,
		&Actor::AUTORUN_Start,
		&Actor::AUTORUN_End,
		&Actor::AUTORUN_Change,
		&Actor::AUTORUN_Update,
		&Actor::AUTORUN_UpdateSprite,
		&Actor::AUTORUN_TransitionToAction,
		&Actor::AUTORUN_TimeIndFrameInc,
		&Actor::AUTORUN_TimeDepFrameInc,
		&Actor::AUTORUN_GetActionLength,
		&Actor::AUTORUN_GetTileset);

	SetupFuncsForAction(BACKWARDSDOUBLE,
		&Actor::BACKWARDSDOUBLE_Start,
		&Actor::BACKWARDSDOUBLE_End,
		&Actor::BACKWARDSDOUBLE_Change,
		&Actor::BACKWARDSDOUBLE_Update,
		&Actor::BACKWARDSDOUBLE_UpdateSprite,
		&Actor::BACKWARDSDOUBLE_TransitionToAction,
		&Actor::BACKWARDSDOUBLE_TimeIndFrameInc,
		&Actor::BACKWARDSDOUBLE_TimeDepFrameInc,
		&Actor::BACKWARDSDOUBLE_GetActionLength,
		&Actor::BACKWARDSDOUBLE_GetTileset);

	SetupFuncsForAction(BOUNCEAIR,
		&Actor::BOUNCEAIR_Start,
		&Actor::BOUNCEAIR_End,
		&Actor::BOUNCEAIR_Change,
		&Actor::BOUNCEAIR_Update,
		&Actor::BOUNCEAIR_UpdateSprite,
		&Actor::BOUNCEAIR_TransitionToAction,
		&Actor::BOUNCEAIR_TimeIndFrameInc,
		&Actor::BOUNCEAIR_TimeDepFrameInc,
		&Actor::BOUNCEAIR_GetActionLength,
		&Actor::BOUNCEAIR_GetTileset);

	SetupFuncsForAction(BOUNCEGROUND,
		&Actor::BOUNCEGROUND_Start,
		&Actor::BOUNCEGROUND_End,
		&Actor::BOUNCEGROUND_Change,
		&Actor::BOUNCEGROUND_Update,
		&Actor::BOUNCEGROUND_UpdateSprite,
		&Actor::BOUNCEGROUND_TransitionToAction,
		&Actor::BOUNCEGROUND_TimeIndFrameInc,
		&Actor::BOUNCEGROUND_TimeDepFrameInc,
		&Actor::BOUNCEGROUND_GetActionLength,
		&Actor::BOUNCEGROUND_GetTileset);

	SetupFuncsForAction(BOUNCEGROUNDEDWALL,
		&Actor::BOUNCEGROUNDEDWALL_Start,
		&Actor::BOUNCEGROUNDEDWALL_End,
		&Actor::BOUNCEGROUNDEDWALL_Change,
		&Actor::BOUNCEGROUNDEDWALL_Update,
		&Actor::BOUNCEGROUNDEDWALL_UpdateSprite,
		&Actor::BOUNCEGROUNDEDWALL_TransitionToAction,
		&Actor::BOUNCEGROUNDEDWALL_TimeIndFrameInc,
		&Actor::BOUNCEGROUNDEDWALL_TimeDepFrameInc,
		&Actor::BOUNCEGROUNDEDWALL_GetActionLength,
		&Actor::BOUNCEGROUNDEDWALL_GetTileset);

	SetupFuncsForAction(BRAKE,
		&Actor::BRAKE_Start,
		&Actor::BRAKE_End,
		&Actor::BRAKE_Change,
		&Actor::BRAKE_Update,
		&Actor::BRAKE_UpdateSprite,
		&Actor::BRAKE_TransitionToAction,
		&Actor::BRAKE_TimeIndFrameInc,
		&Actor::BRAKE_TimeDepFrameInc,
		&Actor::BRAKE_GetActionLength,
		&Actor::BRAKE_GetTileset);

	SetupFuncsForAction(DAIR,
		&Actor::DAIR_Start,
		&Actor::DAIR_End,
		&Actor::DAIR_Change,
		&Actor::DAIR_Update,
		&Actor::DAIR_UpdateSprite,
		&Actor::DAIR_TransitionToAction,
		&Actor::DAIR_TimeIndFrameInc,
		&Actor::DAIR_TimeDepFrameInc,
		&Actor::DAIR_GetActionLength,
		&Actor::DAIR_GetTileset);

	SetupFuncsForAction(DASH,
		&Actor::DASH_Start,
		&Actor::DASH_End,
		&Actor::DASH_Change,
		&Actor::DASH_Update,
		&Actor::DASH_UpdateSprite,
		&Actor::DASH_TransitionToAction,
		&Actor::DASH_TimeIndFrameInc,
		&Actor::DASH_TimeDepFrameInc,
		&Actor::DASH_GetActionLength,
		&Actor::DASH_GetTileset);

	SetupFuncsForAction(DASHATTACK,
		&Actor::DASHATTACK_Start,
		&Actor::DASHATTACK_End,
		&Actor::DASHATTACK_Change,
		&Actor::DASHATTACK_Update,
		&Actor::DASHATTACK_UpdateSprite,
		&Actor::DASHATTACK_TransitionToAction,
		&Actor::DASHATTACK_TimeIndFrameInc,
		&Actor::DASHATTACK_TimeDepFrameInc,
		&Actor::DASHATTACK_GetActionLength,
		&Actor::DASHATTACK_GetTileset);

	SetupFuncsForAction(DEATH,
		&Actor::DEATH_Start,
		&Actor::DEATH_End,
		&Actor::DEATH_Change,
		&Actor::DEATH_Update,
		&Actor::DEATH_UpdateSprite,
		&Actor::DEATH_TransitionToAction,
		&Actor::DEATH_TimeIndFrameInc,
		&Actor::DEATH_TimeDepFrameInc,
		&Actor::DEATH_GetActionLength,
		&Actor::DEATH_GetTileset);

	SetupFuncsForAction(DIAGDOWNATTACK,
		&Actor::DIAGDOWNATTACK_Start,
		&Actor::DIAGDOWNATTACK_End,
		&Actor::DIAGDOWNATTACK_Change,
		&Actor::DIAGDOWNATTACK_Update,
		&Actor::DIAGDOWNATTACK_UpdateSprite,
		&Actor::DIAGDOWNATTACK_TransitionToAction,
		&Actor::DIAGDOWNATTACK_TimeIndFrameInc,
		&Actor::DIAGDOWNATTACK_TimeDepFrameInc,
		&Actor::DIAGDOWNATTACK_GetActionLength,
		&Actor::DIAGDOWNATTACK_GetTileset);

	SetupFuncsForAction(DIAGUPATTACK,
		&Actor::DIAGUPATTACK_Start,
		&Actor::DIAGUPATTACK_End,
		&Actor::DIAGUPATTACK_Change,
		&Actor::DIAGUPATTACK_Update,
		&Actor::DIAGUPATTACK_UpdateSprite,
		&Actor::DIAGUPATTACK_TransitionToAction,
		&Actor::DIAGUPATTACK_TimeIndFrameInc,
		&Actor::DIAGUPATTACK_TimeDepFrameInc,
		&Actor::DIAGUPATTACK_GetActionLength,
		&Actor::DIAGUPATTACK_GetTileset);

	SetupFuncsForAction(DOUBLE,
		&Actor::DOUBLE_Start,
		&Actor::DOUBLE_End,
		&Actor::DOUBLE_Change,
		&Actor::DOUBLE_Update,
		&Actor::DOUBLE_UpdateSprite,
		&Actor::DOUBLE_TransitionToAction,
		&Actor::DOUBLE_TimeIndFrameInc,
		&Actor::DOUBLE_TimeDepFrameInc,
		&Actor::DOUBLE_GetActionLength,
		&Actor::DOUBLE_GetTileset);

	SetupFuncsForAction(ENTERNEXUS1,
		&Actor::ENTERNEXUS1_Start,
		&Actor::ENTERNEXUS1_End,
		&Actor::ENTERNEXUS1_Change,
		&Actor::ENTERNEXUS1_Update,
		&Actor::ENTERNEXUS1_UpdateSprite,
		&Actor::ENTERNEXUS1_TransitionToAction,
		&Actor::ENTERNEXUS1_TimeIndFrameInc,
		&Actor::ENTERNEXUS1_TimeDepFrameInc,
		&Actor::ENTERNEXUS1_GetActionLength,
		&Actor::ENTERNEXUS1_GetTileset);

	SetupFuncsForAction(EXIT,
		&Actor::EXIT_Start,
		&Actor::EXIT_End,
		&Actor::EXIT_Change,
		&Actor::EXIT_Update,
		&Actor::EXIT_UpdateSprite,
		&Actor::EXIT_TransitionToAction,
		&Actor::EXIT_TimeIndFrameInc,
		&Actor::EXIT_TimeDepFrameInc,
		&Actor::EXIT_GetActionLength,
		&Actor::EXIT_GetTileset);

	SetupFuncsForAction(EXITBOOST,
		&Actor::EXITBOOST_Start,
		&Actor::EXITBOOST_End,
		&Actor::EXITBOOST_Change,
		&Actor::EXITBOOST_Update,
		&Actor::EXITBOOST_UpdateSprite,
		&Actor::EXITBOOST_TransitionToAction,
		&Actor::EXITBOOST_TimeIndFrameInc,
		&Actor::EXITBOOST_TimeDepFrameInc,
		&Actor::EXITBOOST_GetActionLength,
		&Actor::EXITBOOST_GetTileset);

	SetupFuncsForAction(EXITWAIT,
		&Actor::EXITWAIT_Start,
		&Actor::EXITWAIT_End,
		&Actor::EXITWAIT_Change,
		&Actor::EXITWAIT_Update,
		&Actor::EXITWAIT_UpdateSprite,
		&Actor::EXITWAIT_TransitionToAction,
		&Actor::EXITWAIT_TimeIndFrameInc,
		&Actor::EXITWAIT_TimeDepFrameInc,
		&Actor::EXITWAIT_GetActionLength,
		&Actor::EXITWAIT_GetTileset);

	SetupFuncsForAction(FAIR,
		&Actor::FAIR_Start,
		&Actor::FAIR_End,
		&Actor::FAIR_Change,
		&Actor::FAIR_Update,
		&Actor::FAIR_UpdateSprite,
		&Actor::FAIR_TransitionToAction,
		&Actor::FAIR_TimeIndFrameInc,
		&Actor::FAIR_TimeDepFrameInc,
		&Actor::FAIR_GetActionLength,
		&Actor::FAIR_GetTileset);

	SetupFuncsForAction(GETPOWER_AIRDASH_FLIP,
		&Actor::GETPOWER_AIRDASH_FLIP_Start,
		&Actor::GETPOWER_AIRDASH_FLIP_End,
		&Actor::GETPOWER_AIRDASH_FLIP_Change,
		&Actor::GETPOWER_AIRDASH_FLIP_Update,
		&Actor::GETPOWER_AIRDASH_FLIP_UpdateSprite,
		&Actor::GETPOWER_AIRDASH_FLIP_TransitionToAction,
		&Actor::GETPOWER_AIRDASH_FLIP_TimeIndFrameInc,
		&Actor::GETPOWER_AIRDASH_FLIP_TimeDepFrameInc,
		&Actor::GETPOWER_AIRDASH_FLIP_GetActionLength,
		&Actor::GETPOWER_AIRDASH_FLIP_GetTileset);

	SetupFuncsForAction(GETPOWER_AIRDASH_MEDITATE,
		&Actor::GETPOWER_AIRDASH_MEDITATE_Start,
		&Actor::GETPOWER_AIRDASH_MEDITATE_End,
		&Actor::GETPOWER_AIRDASH_MEDITATE_Change,
		&Actor::GETPOWER_AIRDASH_MEDITATE_Update,
		&Actor::GETPOWER_AIRDASH_MEDITATE_UpdateSprite,
		&Actor::GETPOWER_AIRDASH_MEDITATE_TransitionToAction,
		&Actor::GETPOWER_AIRDASH_MEDITATE_TimeIndFrameInc,
		&Actor::GETPOWER_AIRDASH_MEDITATE_TimeDepFrameInc,
		&Actor::GETPOWER_AIRDASH_MEDITATE_GetActionLength,
		&Actor::GETPOWER_AIRDASH_MEDITATE_GetTileset);

	SetupFuncsForAction(GETSHARD,
		&Actor::GETSHARD_Start,
		&Actor::GETSHARD_End,
		&Actor::GETSHARD_Change,
		&Actor::GETSHARD_Update,
		&Actor::GETSHARD_UpdateSprite,
		&Actor::GETSHARD_TransitionToAction,
		&Actor::GETSHARD_TimeIndFrameInc,
		&Actor::GETSHARD_TimeDepFrameInc,
		&Actor::GETSHARD_GetActionLength,
		&Actor::GETSHARD_GetTileset);

	SetupFuncsForAction(GLIDE,
		&Actor::GLIDE_Start,
		&Actor::GLIDE_End,
		&Actor::GLIDE_Change,
		&Actor::GLIDE_Update,
		&Actor::GLIDE_UpdateSprite,
		&Actor::GLIDE_TransitionToAction,
		&Actor::GLIDE_TimeIndFrameInc,
		&Actor::GLIDE_TimeDepFrameInc,
		&Actor::GLIDE_GetActionLength,
		&Actor::GLIDE_GetTileset);

	SetupFuncsForAction(GOALKILL,
		&Actor::GOALKILL_Start,
		&Actor::GOALKILL_End,
		&Actor::GOALKILL_Change,
		&Actor::GOALKILL_Update,
		&Actor::GOALKILL_UpdateSprite,
		&Actor::GOALKILL_TransitionToAction,
		&Actor::GOALKILL_TimeIndFrameInc,
		&Actor::GOALKILL_TimeDepFrameInc,
		&Actor::GOALKILL_GetActionLength,
		&Actor::GOALKILL_GetTileset);

	SetupFuncsForAction(GOALKILLWAIT,
		&Actor::GOALKILLWAIT_Start,
		&Actor::GOALKILLWAIT_End,
		&Actor::GOALKILLWAIT_Change,
		&Actor::GOALKILLWAIT_Update,
		&Actor::GOALKILLWAIT_UpdateSprite,
		&Actor::GOALKILLWAIT_TransitionToAction,
		&Actor::GOALKILLWAIT_TimeIndFrameInc,
		&Actor::GOALKILLWAIT_TimeDepFrameInc,
		&Actor::GOALKILLWAIT_GetActionLength,
		&Actor::GOALKILLWAIT_GetTileset);

	SetupFuncsForAction(GRABSHIP,
		&Actor::GRABSHIP_Start,
		&Actor::GRABSHIP_End,
		&Actor::GRABSHIP_Change,
		&Actor::GRABSHIP_Update,
		&Actor::GRABSHIP_UpdateSprite,
		&Actor::GRABSHIP_TransitionToAction,
		&Actor::GRABSHIP_TimeIndFrameInc,
		&Actor::GRABSHIP_TimeDepFrameInc,
		&Actor::GRABSHIP_GetActionLength,
		&Actor::GRABSHIP_GetTileset);

	SetupFuncsForAction(GRAVREVERSE,
		&Actor::GRAVREVERSE_Start,
		&Actor::GRAVREVERSE_End,
		&Actor::GRAVREVERSE_Change,
		&Actor::GRAVREVERSE_Update,
		&Actor::GRAVREVERSE_UpdateSprite,
		&Actor::GRAVREVERSE_TransitionToAction,
		&Actor::GRAVREVERSE_TimeIndFrameInc,
		&Actor::GRAVREVERSE_TimeDepFrameInc,
		&Actor::GRAVREVERSE_GetActionLength,
		&Actor::GRAVREVERSE_GetTileset);

	SetupFuncsForAction(GRINDATTACK,
		&Actor::GRINDATTACK_Start,
		&Actor::GRINDATTACK_End,
		&Actor::GRINDATTACK_Change,
		&Actor::GRINDATTACK_Update,
		&Actor::GRINDATTACK_UpdateSprite,
		&Actor::GRINDATTACK_TransitionToAction,
		&Actor::GRINDATTACK_TimeIndFrameInc,
		&Actor::GRINDATTACK_TimeDepFrameInc,
		&Actor::GRINDATTACK_GetActionLength,
		&Actor::GRINDATTACK_GetTileset);

	SetupFuncsForAction(GRINDBALL,
		&Actor::GRINDBALL_Start,
		&Actor::GRINDBALL_End,
		&Actor::GRINDBALL_Change,
		&Actor::GRINDBALL_Update,
		&Actor::GRINDBALL_UpdateSprite,
		&Actor::GRINDBALL_TransitionToAction,
		&Actor::GRINDBALL_TimeIndFrameInc,
		&Actor::GRINDBALL_TimeDepFrameInc,
		&Actor::GRINDBALL_GetActionLength,
		&Actor::GRINDBALL_GetTileset);

	SetupFuncsForAction(GRINDLUNGE,
		&Actor::GRINDLUNGE_Start,
		&Actor::GRINDLUNGE_End,
		&Actor::GRINDLUNGE_Change,
		&Actor::GRINDLUNGE_Update,
		&Actor::GRINDLUNGE_UpdateSprite,
		&Actor::GRINDLUNGE_TransitionToAction,
		&Actor::GRINDLUNGE_TimeIndFrameInc,
		&Actor::GRINDLUNGE_TimeDepFrameInc,
		&Actor::GRINDLUNGE_GetActionLength,
		&Actor::GRINDLUNGE_GetTileset);

	SetupFuncsForAction(GRINDSLASH,
		&Actor::GRINDSLASH_Start,
		&Actor::GRINDSLASH_End,
		&Actor::GRINDSLASH_Change,
		&Actor::GRINDSLASH_Update,
		&Actor::GRINDSLASH_UpdateSprite,
		&Actor::GRINDSLASH_TransitionToAction,
		&Actor::GRINDSLASH_TimeIndFrameInc,
		&Actor::GRINDSLASH_TimeDepFrameInc,
		&Actor::GRINDSLASH_GetActionLength,
		&Actor::GRINDSLASH_GetTileset);

	SetupFuncsForAction(GROUNDHITSTUN,
		&Actor::GROUNDHITSTUN_Start,
		&Actor::GROUNDHITSTUN_End,
		&Actor::GROUNDHITSTUN_Change,
		&Actor::GROUNDHITSTUN_Update,
		&Actor::GROUNDHITSTUN_UpdateSprite,
		&Actor::GROUNDHITSTUN_TransitionToAction,
		&Actor::GROUNDHITSTUN_TimeIndFrameInc,
		&Actor::GROUNDHITSTUN_TimeDepFrameInc,
		&Actor::GROUNDHITSTUN_GetActionLength,
		&Actor::GROUNDHITSTUN_GetTileset);

	SetupFuncsForAction(INTRO,
		&Actor::INTRO_Start,
		&Actor::INTRO_End,
		&Actor::INTRO_Change,
		&Actor::INTRO_Update,
		&Actor::INTRO_UpdateSprite,
		&Actor::INTRO_TransitionToAction,
		&Actor::INTRO_TimeIndFrameInc,
		&Actor::INTRO_TimeDepFrameInc,
		&Actor::INTRO_GetActionLength,
		&Actor::INTRO_GetTileset);

	SetupFuncsForAction(INTROBOOST,
		&Actor::INTROBOOST_Start,
		&Actor::INTROBOOST_End,
		&Actor::INTROBOOST_Change,
		&Actor::INTROBOOST_Update,
		&Actor::INTROBOOST_UpdateSprite,
		&Actor::INTROBOOST_TransitionToAction,
		&Actor::INTROBOOST_TimeIndFrameInc,
		&Actor::INTROBOOST_TimeDepFrameInc,
		&Actor::INTROBOOST_GetActionLength,
		&Actor::INTROBOOST_GetTileset);

	SetupFuncsForAction(JUMP,
		&Actor::JUMP_Start,
		&Actor::JUMP_End,
		&Actor::JUMP_Change,
		&Actor::JUMP_Update,
		&Actor::JUMP_UpdateSprite,
		&Actor::JUMP_TransitionToAction,
		&Actor::JUMP_TimeIndFrameInc,
		&Actor::JUMP_TimeDepFrameInc,
		&Actor::JUMP_GetActionLength,
		&Actor::JUMP_GetTileset);

	SetupFuncsForAction(JUMPSQUAT,
		&Actor::JUMPSQUAT_Start,
		&Actor::JUMPSQUAT_End,
		&Actor::JUMPSQUAT_Change,
		&Actor::JUMPSQUAT_Update,
		&Actor::JUMPSQUAT_UpdateSprite,
		&Actor::JUMPSQUAT_TransitionToAction,
		&Actor::JUMPSQUAT_TimeIndFrameInc,
		&Actor::JUMPSQUAT_TimeDepFrameInc,
		&Actor::JUMPSQUAT_GetActionLength,
		&Actor::JUMPSQUAT_GetTileset);

	SetupFuncsForAction(GLIDE,
		&Actor::GLIDE_Start,
		&Actor::GLIDE_End,
		&Actor::GLIDE_Change,
		&Actor::GLIDE_Update,
		&Actor::GLIDE_UpdateSprite,
		&Actor::GLIDE_TransitionToAction,
		&Actor::GLIDE_TimeIndFrameInc,
		&Actor::GLIDE_TimeDepFrameInc,
		&Actor::GLIDE_GetActionLength,
		&Actor::GLIDE_GetTileset);

	SetupFuncsForAction(LAND,
		&Actor::LAND_Start,
		&Actor::LAND_End,
		&Actor::LAND_Change,
		&Actor::LAND_Update,
		&Actor::LAND_UpdateSprite,
		&Actor::LAND_TransitionToAction,
		&Actor::LAND_TimeIndFrameInc,
		&Actor::LAND_TimeDepFrameInc,
		&Actor::LAND_GetActionLength,
		&Actor::LAND_GetTileset);

	SetupFuncsForAction(LAND2,
		&Actor::LAND2_Start,
		&Actor::LAND2_End,
		&Actor::LAND2_Change,
		&Actor::LAND2_Update,
		&Actor::LAND2_UpdateSprite,
		&Actor::LAND2_TransitionToAction,
		&Actor::LAND2_TimeIndFrameInc,
		&Actor::LAND2_TimeDepFrameInc,
		&Actor::LAND2_GetActionLength,
		&Actor::LAND2_GetTileset);

	SetupFuncsForAction(NEXUSKILL,
		&Actor::NEXUSKILL_Start,
		&Actor::NEXUSKILL_End,
		&Actor::NEXUSKILL_Change,
		&Actor::NEXUSKILL_Update,
		&Actor::NEXUSKILL_UpdateSprite,
		&Actor::NEXUSKILL_TransitionToAction,
		&Actor::NEXUSKILL_TimeIndFrameInc,
		&Actor::NEXUSKILL_TimeDepFrameInc,
		&Actor::NEXUSKILL_GetActionLength,
		&Actor::NEXUSKILL_GetTileset);

	SetupFuncsForAction(RAILDASH,
		&Actor::RAILDASH_Start,
		&Actor::RAILDASH_End,
		&Actor::RAILDASH_Change,
		&Actor::RAILDASH_Update,
		&Actor::RAILDASH_UpdateSprite,
		&Actor::RAILDASH_TransitionToAction,
		&Actor::RAILDASH_TimeIndFrameInc,
		&Actor::RAILDASH_TimeDepFrameInc,
		&Actor::RAILDASH_GetActionLength,
		&Actor::RAILDASH_GetTileset);

	SetupFuncsForAction(RAILGRIND,
		&Actor::RAILGRIND_Start,
		&Actor::RAILGRIND_End,
		&Actor::RAILGRIND_Change,
		&Actor::RAILGRIND_Update,
		&Actor::RAILGRIND_UpdateSprite,
		&Actor::RAILGRIND_TransitionToAction,
		&Actor::RAILGRIND_TimeIndFrameInc,
		&Actor::RAILGRIND_TimeDepFrameInc,
		&Actor::RAILGRIND_GetActionLength,
		&Actor::RAILGRIND_GetTileset);

	SetupFuncsForAction(RAILSLIDE,
		&Actor::RAILSLIDE_Start,
		&Actor::RAILSLIDE_End,
		&Actor::RAILSLIDE_Change,
		&Actor::RAILSLIDE_Update,
		&Actor::RAILSLIDE_UpdateSprite,
		&Actor::RAILSLIDE_TransitionToAction,
		&Actor::RAILSLIDE_TimeIndFrameInc,
		&Actor::RAILSLIDE_TimeDepFrameInc,
		&Actor::RAILSLIDE_GetActionLength,
		&Actor::RAILSLIDE_GetTileset);

	SetupFuncsForAction(RIDESHIP,
		&Actor::RIDESHIP_Start,
		&Actor::RIDESHIP_End,
		&Actor::RIDESHIP_Change,
		&Actor::RIDESHIP_Update,
		&Actor::RIDESHIP_UpdateSprite,
		&Actor::RIDESHIP_TransitionToAction,
		&Actor::RIDESHIP_TimeIndFrameInc,
		&Actor::RIDESHIP_TimeDepFrameInc,
		&Actor::RIDESHIP_GetActionLength,
		&Actor::RIDESHIP_GetTileset);

	SetupFuncsForAction(RUN,
		&Actor::RUN_Start,
		&Actor::RUN_End,
		&Actor::RUN_Change,
		&Actor::RUN_Update,
		&Actor::RUN_UpdateSprite,
		&Actor::RUN_TransitionToAction,
		&Actor::RUN_TimeIndFrameInc,
		&Actor::RUN_TimeDepFrameInc,
		&Actor::RUN_GetActionLength,
		&Actor::RUN_GetTileset);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_DODGEBACK,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Start,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_End,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Change,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Update,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_GetActionLength,
		&Actor::AIRDASH_GetTileset);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_LAND,
		&Actor::SEQ_CRAWLERFIGHT_LAND_Start,
		&Actor::SEQ_CRAWLERFIGHT_LAND_End,
		&Actor::SEQ_CRAWLERFIGHT_LAND_Change,
		&Actor::SEQ_CRAWLERFIGHT_LAND_Update,
		&Actor::SEQ_CRAWLERFIGHT_LAND_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_LAND_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_LAND_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_LAND_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_LAND_GetActionLength,
		&Actor::AIRDASH_GetTileset);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_STAND,
		&Actor::SEQ_CRAWLERFIGHT_STAND_Start,
		&Actor::SEQ_CRAWLERFIGHT_STAND_End,
		&Actor::SEQ_CRAWLERFIGHT_STAND_Change,
		&Actor::SEQ_CRAWLERFIGHT_STAND_Update,
		&Actor::SEQ_CRAWLERFIGHT_STAND_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_STAND_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_STAND_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STAND_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STAND_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_STAND_GetTileset);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_STRAIGHTFALL,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Start,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_End,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Change,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Update,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetTileset);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Start,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_End,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Change,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Update,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetTileset);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Start,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_End,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Change,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Update,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetTileset);

	SetupFuncsForAction(SEQ_ENTERCORE1,
		&Actor::SEQ_ENTERCORE1_Start,
		&Actor::SEQ_ENTERCORE1_End,
		&Actor::SEQ_ENTERCORE1_Change,
		&Actor::SEQ_ENTERCORE1_Update,
		&Actor::SEQ_ENTERCORE1_UpdateSprite,
		&Actor::SEQ_ENTERCORE1_TransitionToAction,
		&Actor::SEQ_ENTERCORE1_TimeIndFrameInc,
		&Actor::SEQ_ENTERCORE1_TimeDepFrameInc,
		&Actor::SEQ_ENTERCORE1_GetActionLength,
		&Actor::SEQ_ENTERCORE1_GetTileset);

	SetupFuncsForAction(SEQ_FADE_INTO_NEXUS,
		&Actor::SEQ_FADE_INTO_NEXUS_Start,
		&Actor::SEQ_FADE_INTO_NEXUS_End,
		&Actor::SEQ_FADE_INTO_NEXUS_Change,
		&Actor::SEQ_FADE_INTO_NEXUS_Update,
		&Actor::SEQ_FADE_INTO_NEXUS_UpdateSprite,
		&Actor::SEQ_FADE_INTO_NEXUS_TransitionToAction,
		&Actor::SEQ_FADE_INTO_NEXUS_TimeIndFrameInc,
		&Actor::SEQ_FADE_INTO_NEXUS_TimeDepFrameInc,
		&Actor::SEQ_FADE_INTO_NEXUS_GetActionLength,
		&Actor::SEQ_FADE_INTO_NEXUS_GetTileset);

	SetupFuncsForAction(SEQ_FLOAT_TO_NEXUS_OPENING,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Start,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_End,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Change,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Update,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_UpdateSprite,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_TransitionToAction,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_TimeIndFrameInc,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_TimeDepFrameInc,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_GetActionLength,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_GetTileset);

	SetupFuncsForAction(SEQ_KINFALL,
		&Actor::SEQ_KINFALL_Start,
		&Actor::SEQ_KINFALL_End,
		&Actor::SEQ_KINFALL_Change,
		&Actor::SEQ_KINFALL_Update,
		&Actor::SEQ_KINFALL_UpdateSprite,
		&Actor::SEQ_KINFALL_TransitionToAction,
		&Actor::SEQ_KINFALL_TimeIndFrameInc,
		&Actor::SEQ_KINFALL_TimeDepFrameInc,
		&Actor::SEQ_KINFALL_GetActionLength,
		&Actor::SEQ_KINFALL_GetTileset);

	SetupFuncsForAction(SEQ_KINSTAND,
		&Actor::SEQ_KINSTAND_Start,
		&Actor::SEQ_KINSTAND_End,
		&Actor::SEQ_KINSTAND_Change,
		&Actor::SEQ_KINSTAND_Update,
		&Actor::SEQ_KINSTAND_UpdateSprite,
		&Actor::SEQ_KINSTAND_TransitionToAction,
		&Actor::SEQ_KINSTAND_TimeIndFrameInc,
		&Actor::SEQ_KINSTAND_TimeDepFrameInc,
		&Actor::SEQ_KINSTAND_GetActionLength,
		&Actor::SEQ_KINSTAND_GetTileset);

	SetupFuncsForAction(SEQ_KINTHROWN,
		&Actor::SEQ_KINTHROWN_Start,
		&Actor::SEQ_KINTHROWN_End,
		&Actor::SEQ_KINTHROWN_Change,
		&Actor::SEQ_KINTHROWN_Update,
		&Actor::SEQ_KINTHROWN_UpdateSprite,
		&Actor::SEQ_KINTHROWN_TransitionToAction,
		&Actor::SEQ_KINTHROWN_TimeIndFrameInc,
		&Actor::SEQ_KINTHROWN_TimeDepFrameInc,
		&Actor::SEQ_KINTHROWN_GetActionLength,
		&Actor::SEQ_KINTHROWN_GetTileset);

	SetupFuncsForAction(SEQ_KNEEL,
		&Actor::SEQ_KNEEL_Start,
		&Actor::SEQ_KNEEL_End,
		&Actor::SEQ_KNEEL_Change,
		&Actor::SEQ_KNEEL_Update,
		&Actor::SEQ_KNEEL_UpdateSprite,
		&Actor::SEQ_KNEEL_TransitionToAction,
		&Actor::SEQ_KNEEL_TimeIndFrameInc,
		&Actor::SEQ_KNEEL_TimeDepFrameInc,
		&Actor::SEQ_KNEEL_GetActionLength,
		&Actor::SEQ_KNEEL_GetTileset);

	SetupFuncsForAction(SEQ_KNEEL_TO_MEDITATE,
		&Actor::SEQ_KNEEL_TO_MEDITATE_Start,
		&Actor::SEQ_KNEEL_TO_MEDITATE_End,
		&Actor::SEQ_KNEEL_TO_MEDITATE_Change,
		&Actor::SEQ_KNEEL_TO_MEDITATE_Update,
		&Actor::SEQ_KNEEL_TO_MEDITATE_UpdateSprite,
		&Actor::SEQ_KNEEL_TO_MEDITATE_TransitionToAction,
		&Actor::SEQ_KNEEL_TO_MEDITATE_TimeIndFrameInc,
		&Actor::SEQ_KNEEL_TO_MEDITATE_TimeDepFrameInc,
		&Actor::SEQ_KNEEL_TO_MEDITATE_GetActionLength,
		&Actor::SEQ_KNEEL_TO_MEDITATE_GetTileset);

	SetupFuncsForAction(SEQ_LOOKUP,
		&Actor::SEQ_LOOKUP_Start,
		&Actor::SEQ_LOOKUP_End,
		&Actor::SEQ_LOOKUP_Change,
		&Actor::SEQ_LOOKUP_Update,
		&Actor::SEQ_LOOKUP_UpdateSprite,
		&Actor::SEQ_LOOKUP_TransitionToAction,
		&Actor::SEQ_LOOKUP_TimeIndFrameInc,
		&Actor::SEQ_LOOKUP_TimeDepFrameInc,
		&Actor::SEQ_LOOKUP_GetActionLength,
		&Actor::SEQ_LOOKUP_GetTileset);

	SetupFuncsForAction(SEQ_LOOKUPDISAPPEAR,
		&Actor::SEQ_LOOKUPDISAPPEAR_Start,
		&Actor::SEQ_LOOKUPDISAPPEAR_End,
		&Actor::SEQ_LOOKUPDISAPPEAR_Change,
		&Actor::SEQ_LOOKUPDISAPPEAR_Update,
		&Actor::SEQ_LOOKUPDISAPPEAR_UpdateSprite,
		&Actor::SEQ_LOOKUPDISAPPEAR_TransitionToAction,
		&Actor::SEQ_LOOKUPDISAPPEAR_TimeIndFrameInc,
		&Actor::SEQ_LOOKUPDISAPPEAR_TimeDepFrameInc,
		&Actor::SEQ_LOOKUPDISAPPEAR_GetActionLength,
		&Actor::SEQ_LOOKUPDISAPPEAR_GetTileset);

	SetupFuncsForAction(SEQ_MASKOFF,
		&Actor::SEQ_MASKOFF_Start,
		&Actor::SEQ_MASKOFF_End,
		&Actor::SEQ_MASKOFF_Change,
		&Actor::SEQ_MASKOFF_Update,
		&Actor::SEQ_MASKOFF_UpdateSprite,
		&Actor::SEQ_MASKOFF_TransitionToAction,
		&Actor::SEQ_MASKOFF_TimeIndFrameInc,
		&Actor::SEQ_MASKOFF_TimeDepFrameInc,
		&Actor::SEQ_MASKOFF_GetActionLength,
		&Actor::SEQ_MASKOFF_GetTileset);

	SetupFuncsForAction(SEQ_MEDITATE,
		&Actor::SEQ_MEDITATE_Start,
		&Actor::SEQ_MEDITATE_End,
		&Actor::SEQ_MEDITATE_Change,
		&Actor::SEQ_MEDITATE_Update,
		&Actor::SEQ_MEDITATE_UpdateSprite,
		&Actor::SEQ_MEDITATE_TransitionToAction,
		&Actor::SEQ_MEDITATE_TimeIndFrameInc,
		&Actor::SEQ_MEDITATE_TimeDepFrameInc,
		&Actor::SEQ_MEDITATE_GetActionLength,
		&Actor::SEQ_MEDITATE_GetTileset);

	SetupFuncsForAction(SEQ_MEDITATE_MASKON,
		&Actor::SEQ_MEDITATE_MASKON_Start,
		&Actor::SEQ_MEDITATE_MASKON_End,
		&Actor::SEQ_MEDITATE_MASKON_Change,
		&Actor::SEQ_MEDITATE_MASKON_Update,
		&Actor::SEQ_MEDITATE_MASKON_UpdateSprite,
		&Actor::SEQ_MEDITATE_MASKON_TransitionToAction,
		&Actor::SEQ_MEDITATE_MASKON_TimeIndFrameInc,
		&Actor::SEQ_MEDITATE_MASKON_TimeDepFrameInc,
		&Actor::SEQ_MEDITATE_MASKON_GetActionLength,
		&Actor::SEQ_MEDITATE_MASKON_GetTileset);

	SetupFuncsForAction(SEQ_TURNFACE,
		&Actor::SEQ_TURNFACE_Start,
		&Actor::SEQ_TURNFACE_End,
		&Actor::SEQ_TURNFACE_Change,
		&Actor::SEQ_TURNFACE_Update,
		&Actor::SEQ_TURNFACE_UpdateSprite,
		&Actor::SEQ_TURNFACE_TransitionToAction,
		&Actor::SEQ_TURNFACE_TimeIndFrameInc,
		&Actor::SEQ_TURNFACE_TimeDepFrameInc,
		&Actor::SEQ_TURNFACE_GetActionLength,
		&Actor::SEQ_TURNFACE_GetTileset);

	SetupFuncsForAction(SEQ_WAIT,
		&Actor::SEQ_WAIT_Start,
		&Actor::SEQ_WAIT_End,
		&Actor::SEQ_WAIT_Change,
		&Actor::SEQ_WAIT_Update,
		&Actor::SEQ_WAIT_UpdateSprite,
		&Actor::SEQ_WAIT_TransitionToAction,
		&Actor::SEQ_WAIT_TimeIndFrameInc,
		&Actor::SEQ_WAIT_TimeDepFrameInc,
		&Actor::SEQ_WAIT_GetActionLength,
		&Actor::SEQ_WAIT_GetTileset);

	SetupFuncsForAction(SKYDIVE,
		&Actor::SKYDIVE_Start,
		&Actor::SKYDIVE_End,
		&Actor::SKYDIVE_Change,
		&Actor::SKYDIVE_Update,
		&Actor::SKYDIVE_UpdateSprite,
		&Actor::SKYDIVE_TransitionToAction,
		&Actor::SKYDIVE_TimeIndFrameInc,
		&Actor::SKYDIVE_TimeDepFrameInc,
		&Actor::SKYDIVE_GetActionLength,
		&Actor::SKYDIVE_GetTileset);

	SetupFuncsForAction(SKYDIVETOFALL,
		&Actor::SKYDIVETOFALL_Start,
		&Actor::SKYDIVETOFALL_End,
		&Actor::SKYDIVETOFALL_Change,
		&Actor::SKYDIVETOFALL_Update,
		&Actor::SKYDIVETOFALL_UpdateSprite,
		&Actor::SKYDIVETOFALL_TransitionToAction,
		&Actor::SKYDIVETOFALL_TimeIndFrameInc,
		&Actor::SKYDIVETOFALL_TimeDepFrameInc,
		&Actor::SKYDIVETOFALL_GetActionLength,
		&Actor::SKYDIVETOFALL_GetTileset);

	SetupFuncsForAction(SLIDE,
		&Actor::SLIDE_Start,
		&Actor::SLIDE_End,
		&Actor::SLIDE_Change,
		&Actor::SLIDE_Update,
		&Actor::SLIDE_UpdateSprite,
		&Actor::SLIDE_TransitionToAction,
		&Actor::SLIDE_TimeIndFrameInc,
		&Actor::SLIDE_TimeDepFrameInc,
		&Actor::SLIDE_GetActionLength,
		&Actor::SLIDE_GetTileset);

	SetupFuncsForAction(SPAWNWAIT,
		&Actor::SPAWNWAIT_Start,
		&Actor::SPAWNWAIT_End,
		&Actor::SPAWNWAIT_Change,
		&Actor::SPAWNWAIT_Update,
		&Actor::SPAWNWAIT_UpdateSprite,
		&Actor::SPAWNWAIT_TransitionToAction,
		&Actor::SPAWNWAIT_TimeIndFrameInc,
		&Actor::SPAWNWAIT_TimeDepFrameInc,
		&Actor::SPAWNWAIT_GetActionLength,
		&Actor::SPAWNWAIT_GetTileset);

	SetupFuncsForAction(SPRINGSTUN,
		&Actor::SPRINGSTUN_Start,
		&Actor::SPRINGSTUN_End,
		&Actor::SPRINGSTUN_Change,
		&Actor::SPRINGSTUN_Update,
		&Actor::SPRINGSTUN_UpdateSprite,
		&Actor::SPRINGSTUN_TransitionToAction,
		&Actor::SPRINGSTUN_TimeIndFrameInc,
		&Actor::SPRINGSTUN_TimeDepFrameInc,
		&Actor::SPRINGSTUN_GetActionLength,
		&Actor::SPRINGSTUN_GetTileset);

	SetupFuncsForAction(SPRINGSTUNAIRBOUNCE,
		&Actor::SPRINGSTUNAIRBOUNCE_Start,
		&Actor::SPRINGSTUNAIRBOUNCE_End,
		&Actor::SPRINGSTUNAIRBOUNCE_Change,
		&Actor::SPRINGSTUNAIRBOUNCE_Update,
		&Actor::SPRINGSTUNAIRBOUNCE_UpdateSprite,
		&Actor::SPRINGSTUNAIRBOUNCE_TransitionToAction,
		&Actor::SPRINGSTUNAIRBOUNCE_TimeIndFrameInc,
		&Actor::SPRINGSTUNAIRBOUNCE_TimeDepFrameInc,
		&Actor::SPRINGSTUNAIRBOUNCE_GetActionLength,
		&Actor::SPRINGSTUNAIRBOUNCE_GetTileset);

	SetupFuncsForAction(SPRINGSTUNBOUNCE,
		&Actor::SPRINGSTUNBOUNCE_Start,
		&Actor::SPRINGSTUNBOUNCE_End,
		&Actor::SPRINGSTUNBOUNCE_Change,
		&Actor::SPRINGSTUNBOUNCE_Update,
		&Actor::SPRINGSTUNBOUNCE_UpdateSprite,
		&Actor::SPRINGSTUNBOUNCE_TransitionToAction,
		&Actor::SPRINGSTUNBOUNCE_TimeIndFrameInc,
		&Actor::SPRINGSTUNBOUNCE_TimeDepFrameInc,
		&Actor::SPRINGSTUNBOUNCE_GetActionLength,
		&Actor::SPRINGSTUNBOUNCE_GetTileset);

	SetupFuncsForAction(SPRINGSTUNGLIDE,
		&Actor::SPRINGSTUNGLIDE_Start,
		&Actor::SPRINGSTUNGLIDE_End,
		&Actor::SPRINGSTUNGLIDE_Change,
		&Actor::SPRINGSTUNGLIDE_Update,
		&Actor::SPRINGSTUNGLIDE_UpdateSprite,
		&Actor::SPRINGSTUNGLIDE_TransitionToAction,
		&Actor::SPRINGSTUNGLIDE_TimeIndFrameInc,
		&Actor::SPRINGSTUNGLIDE_TimeDepFrameInc,
		&Actor::SPRINGSTUNGLIDE_GetActionLength,
		&Actor::SPRINGSTUNGLIDE_GetTileset);

	SetupFuncsForAction(SPRINGSTUNTELEPORT,
		&Actor::SPRINGSTUNTELEPORT_Start,
		&Actor::SPRINGSTUNTELEPORT_End,
		&Actor::SPRINGSTUNTELEPORT_Change,
		&Actor::SPRINGSTUNTELEPORT_Update,
		&Actor::SPRINGSTUNTELEPORT_UpdateSprite,
		&Actor::SPRINGSTUNTELEPORT_TransitionToAction,
		&Actor::SPRINGSTUNTELEPORT_TimeIndFrameInc,
		&Actor::SPRINGSTUNTELEPORT_TimeDepFrameInc,
		&Actor::SPRINGSTUNTELEPORT_GetActionLength,
		&Actor::SPRINGSTUNTELEPORT_GetTileset);

	SetupFuncsForAction(SPRINT,
		&Actor::SPRINT_Start,
		&Actor::SPRINT_End,
		&Actor::SPRINT_Change,
		&Actor::SPRINT_Update,
		&Actor::SPRINT_UpdateSprite,
		&Actor::SPRINT_TransitionToAction,
		&Actor::SPRINT_TimeIndFrameInc,
		&Actor::SPRINT_TimeDepFrameInc,
		&Actor::SPRINT_GetActionLength,
		&Actor::SPRINT_GetTileset);

	SetupFuncsForAction(STAND,
		&Actor::STAND_Start,
		&Actor::STAND_End,
		&Actor::STAND_Change,
		&Actor::STAND_Update,
		&Actor::STAND_UpdateSprite,
		&Actor::STAND_TransitionToAction,
		&Actor::STAND_TimeIndFrameInc,
		&Actor::STAND_TimeDepFrameInc,
		&Actor::STAND_GetActionLength,
		&Actor::STAND_GetTileset);

	SetupFuncsForAction(STANDN,
		&Actor::STANDN_Start,
		&Actor::STANDN_End,
		&Actor::STANDN_Change,
		&Actor::STANDN_Update,
		&Actor::STANDN_UpdateSprite,
		&Actor::STANDN_TransitionToAction,
		&Actor::STANDN_TimeIndFrameInc,
		&Actor::STANDN_TimeDepFrameInc,
		&Actor::STANDN_GetActionLength,
		&Actor::STANDN_GetTileset);

	SetupFuncsForAction(STEEPCLIMB,
		&Actor::STEEPCLIMB_Start,
		&Actor::STEEPCLIMB_End,
		&Actor::STEEPCLIMB_Change,
		&Actor::STEEPCLIMB_Update,
		&Actor::STEEPCLIMB_UpdateSprite,
		&Actor::STEEPCLIMB_TransitionToAction,
		&Actor::STEEPCLIMB_TimeIndFrameInc,
		&Actor::STEEPCLIMB_TimeDepFrameInc,
		&Actor::STEEPCLIMB_GetActionLength,
		&Actor::STEEPCLIMB_GetTileset);

	SetupFuncsForAction(STEEPCLIMBATTACK,
		&Actor::STEEPCLIMBATTACK_Start,
		&Actor::STEEPCLIMBATTACK_End,
		&Actor::STEEPCLIMBATTACK_Change,
		&Actor::STEEPCLIMBATTACK_Update,
		&Actor::STEEPCLIMBATTACK_UpdateSprite,
		&Actor::STEEPCLIMBATTACK_TransitionToAction,
		&Actor::STEEPCLIMBATTACK_TimeIndFrameInc,
		&Actor::STEEPCLIMBATTACK_TimeDepFrameInc,
		&Actor::STEEPCLIMBATTACK_GetActionLength,
		&Actor::STEEPCLIMBATTACK_GetTileset);

	SetupFuncsForAction(STEEPCLING,
		&Actor::STEEPCLING_Start,
		&Actor::STEEPCLING_End,
		&Actor::STEEPCLING_Change,
		&Actor::STEEPCLING_Update,
		&Actor::STEEPCLING_UpdateSprite,
		&Actor::STEEPCLING_TransitionToAction,
		&Actor::STEEPCLING_TimeIndFrameInc,
		&Actor::STEEPCLING_TimeDepFrameInc,
		&Actor::STEEPCLING_GetActionLength,
		&Actor::STEEPCLING_GetTileset);

	SetupFuncsForAction(STEEPSLIDE,
		&Actor::STEEPSLIDE_Start,
		&Actor::STEEPSLIDE_End,
		&Actor::STEEPSLIDE_Change,
		&Actor::STEEPSLIDE_Update,
		&Actor::STEEPSLIDE_UpdateSprite,
		&Actor::STEEPSLIDE_TransitionToAction,
		&Actor::STEEPSLIDE_TimeIndFrameInc,
		&Actor::STEEPSLIDE_TimeDepFrameInc,
		&Actor::STEEPSLIDE_GetActionLength,
		&Actor::STEEPSLIDE_GetTileset);

	SetupFuncsForAction(STEEPSLIDEATTACK,
		&Actor::STEEPSLIDEATTACK_Start,
		&Actor::STEEPSLIDEATTACK_End,
		&Actor::STEEPSLIDEATTACK_Change,
		&Actor::STEEPSLIDEATTACK_Update,
		&Actor::STEEPSLIDEATTACK_UpdateSprite,
		&Actor::STEEPSLIDEATTACK_TransitionToAction,
		&Actor::STEEPSLIDEATTACK_TimeIndFrameInc,
		&Actor::STEEPSLIDEATTACK_TimeDepFrameInc,
		&Actor::STEEPSLIDEATTACK_GetActionLength,
		&Actor::STEEPSLIDEATTACK_GetTileset);

	SetupFuncsForAction(SWINGSTUN,
		&Actor::SWINGSTUN_Start,
		&Actor::SWINGSTUN_End,
		&Actor::SWINGSTUN_Change,
		&Actor::SWINGSTUN_Update,
		&Actor::SWINGSTUN_UpdateSprite,
		&Actor::SWINGSTUN_TransitionToAction,
		&Actor::SWINGSTUN_TimeIndFrameInc,
		&Actor::SWINGSTUN_TimeDepFrameInc,
		&Actor::SWINGSTUN_GetActionLength,
		&Actor::SWINGSTUN_GetTileset);

	SetupFuncsForAction(UAIR,
		&Actor::UAIR_Start,
		&Actor::UAIR_End,
		&Actor::UAIR_Change,
		&Actor::UAIR_Update,
		&Actor::UAIR_UpdateSprite,
		&Actor::UAIR_TransitionToAction,
		&Actor::UAIR_TimeIndFrameInc,
		&Actor::UAIR_TimeDepFrameInc,
		&Actor::UAIR_GetActionLength,
		&Actor::UAIR_GetTileset);

	SetupFuncsForAction(WAITFORSHIP,
		&Actor::WAITFORSHIP_Start,
		&Actor::WAITFORSHIP_End,
		&Actor::WAITFORSHIP_Change,
		&Actor::WAITFORSHIP_Update,
		&Actor::WAITFORSHIP_UpdateSprite,
		&Actor::WAITFORSHIP_TransitionToAction,
		&Actor::WAITFORSHIP_TimeIndFrameInc,
		&Actor::WAITFORSHIP_TimeDepFrameInc,
		&Actor::WAITFORSHIP_GetActionLength,
		&Actor::WAITFORSHIP_GetTileset);

	SetupFuncsForAction(WALLATTACK,
		&Actor::WALLATTACK_Start,
		&Actor::WALLATTACK_End,
		&Actor::WALLATTACK_Change,
		&Actor::WALLATTACK_Update,
		&Actor::WALLATTACK_UpdateSprite,
		&Actor::WALLATTACK_TransitionToAction,
		&Actor::WALLATTACK_TimeIndFrameInc,
		&Actor::WALLATTACK_TimeDepFrameInc,
		&Actor::WALLATTACK_GetActionLength,
		&Actor::WALLATTACK_GetTileset);

	SetupFuncsForAction(WALLCLING,
		&Actor::WALLCLING_Start,
		&Actor::WALLCLING_End,
		&Actor::WALLCLING_Change,
		&Actor::WALLCLING_Update,
		&Actor::WALLCLING_UpdateSprite,
		&Actor::WALLCLING_TransitionToAction,
		&Actor::WALLCLING_TimeIndFrameInc,
		&Actor::WALLCLING_TimeDepFrameInc,
		&Actor::WALLCLING_GetActionLength,
		&Actor::WALLCLING_GetTileset);

	SetupFuncsForAction(WALLJUMP,
		&Actor::WALLJUMP_Start,
		&Actor::WALLJUMP_End,
		&Actor::WALLJUMP_Change,
		&Actor::WALLJUMP_Update,
		&Actor::WALLJUMP_UpdateSprite,
		&Actor::WALLJUMP_TransitionToAction,
		&Actor::WALLJUMP_TimeIndFrameInc,
		&Actor::WALLJUMP_TimeDepFrameInc,
		&Actor::WALLJUMP_GetActionLength,
		&Actor::WALLJUMP_GetTileset);

	SetupFuncsForAction(WIREHOLD,
		&Actor::WIREHOLD_Start,
		&Actor::WIREHOLD_End,
		&Actor::WIREHOLD_Change,
		&Actor::WIREHOLD_Update,
		&Actor::WIREHOLD_UpdateSprite,
		&Actor::WIREHOLD_TransitionToAction,
		&Actor::WIREHOLD_TimeIndFrameInc,
		&Actor::WIREHOLD_TimeDepFrameInc,
		&Actor::WIREHOLD_GetActionLength,
		&Actor::WIREHOLD_GetTileset);
}

void Actor::StartAction()
{
	if (startActionFuncs[action] != NULL)
	{
		(this->*startActionFuncs[action])();
	}
}

void Actor::EndAction()
{
	if (endActionFuncs[action] != NULL)
	{
		(this->*endActionFuncs[action])();
	}
}

void Actor::ChangeAction()
{
	if (changeActionFuncs[action] != NULL)
	{
		(this->*changeActionFuncs[action])();
	}
}

void Actor::UpdateAction()
{
	if (updateActionFuncs[action] != NULL)
	{
		(this->*updateActionFuncs[action])();
	}
}

void Actor::UpdateActionSprite()
{
	if (updateSpriteFuncs[action] != NULL)
	{
		(this->*updateSpriteFuncs[action])();
	}
}

Actor::Actor( GameSession *gs, EditSession *es, int p_actorIndex )
	:dead( false ), actorIndex( p_actorIndex ), bHasUpgradeField(Session::PLAYER_OPTION_BIT_COUNT),
	bStartHasUpgradeField(Session::PLAYER_OPTION_BIT_COUNT)
	{
	flyCounter = 0;
	action = -1; //for init
	SetupActionFunctions();

	numKeysHeld = 0;
	SetSession(Session::GetSession(), gs, es);

	fBubblePos = NULL;
	fBubbleRadiusSize = NULL;
	fBubbleFrame = NULL;


	kinMode = K_NORMAL;
	autoRunStopEdge = NULL;
	extraDoubleJump = false;
	stunBufferedJump = false;
	stunBufferedDash = false;
	stunBufferedAttack = Action::Count;

	extraGravityModifier = 1.0;
	airTrigBehavior = AT_NONE;
	rpu = new RisingParticleUpdater( this );

	totalHealth = 3600;
	storedTrigger = NULL;
	steepClimbBoostStart = 10;
	SetDirtyAura(false);
	activeComboObjList = NULL;

	boostGrassAccel = .25;
	jumpGrassExtra = 15;

	cout << "Start player" << endl;


	//glideEmitter = new GlideEmitter(owner);
	//glideEmitter->CreateParticles();
	//glideEmitter->Reset();
	//glideEmitter->SetOn(false);
	//owner->AddEmitter(glideEmitter, EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);

	repeatingSound = NULL;
	currBooster = NULL;
	oldBooster = NULL;

	currBounceBooster = NULL;
	oldBounceBooster = NULL;

	gravResetFrames = 0;

	currModifier = NULL;
	oldModifier = NULL;
	currWall = NULL;
	currHurtboxes = NULL;
	currHitboxes = NULL;

	standNDashBoostCooldown = 10;
	standNDashBoostCurr = 0;
	ClearPauseBufferedActions();
	standNDashBoostQuant = 3;
	dairBoostVel = 4;
	fairAirDashBoostQuant = 2;
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i] = new MotionGhostEffect(80);
	}
		
	//preload them
	sess->GetTileset("Kin/exitenergy_0_512x512.png", 512, 512);
	sess->GetTileset("Kin/exitenergy_2_512x512.png", 512, 512);
	sess->GetTileset("Kin/exitenergy_1_512x512.png", 512, 512);
		
			
	currLockedFairFX = NULL;
	currLockedDairFX = NULL;
	currLockedUairFX = NULL;
	gateBlackFX = NULL;

	

	motionGhostBuffer = new VertexBuf(80, sf::Quads);
	motionGhostBufferBlue = new VertexBuf(80, sf::Quads);
	motionGhostBufferPurple = new VertexBuf(80, sf::Quads);

	kinRing = new KinRing(this);
	kinMask = new KinMask(this);
	/*if (owner != NULL)
	{
		
	}
	else
	{
		kinRing = NULL;
		kinMask = NULL;
	}*/
		

	//risingAuraPool = new EffectPool(EffectType::FX_RELATIVE, 100, 1.f);
	//risingAuraPool->ts = GetTileset("Kin/rising_8x8.png", 8, 8);

	maxMotionGhosts = 80;
	memset(tileset, 0, sizeof(tileset));
	sf::Color startChanges[] = {
		sf::Color(0x14, 0x59, 0x22),
		sf::Color(0x08, 0x40, 0x12),
		sf::Color(0x08, 0x2a, 0x4d),
		sf::Color(0x05, 0x1a, 0x26),
		sf::Color(0x4d, 0x2c, 0x28),
		sf::Color(0x12, 0x3f, 0x4d),
		sf::Color(0x04, 0x23, 0x26),
		sf::Color(0x6b, 0xff, 0xff),
		sf::Color(0x66, 0xdd, 0xff)
	};
	sf::Color endChanges[] = {
		sf::Color(0x71, 0x0f, 0x0f),
		sf::Color(0x04, 0x08, 0x08),
		sf::Color(0x12, 0x3b, 0x2e),
		sf::Color(0x01, 0x2a, 0x0a),
		sf::Color(0x59, 0x55, 0x47),
		sf::Color(0xb2, 0x91, 0x0c),
		sf::Color(0x65, 0x3f, 0x07),
		sf::Color(0x6b, 0xfd, 0x30),
		sf::Color(0x53, 0xf9, 0xf9)
	};

	skin = NULL;
	swordSkin = NULL;
	actionFolder = "Kin/";
		
	team = (Team)actorIndex; //debug

	SetupTilesets();

	/*if (actorIndex == 0)
	{
		SetupTilesets(NULL, NULL);
	}
	else if (actorIndex == 1)
	{
		Skin *swordSkin = new Skin(startChanges, endChanges, 9, 1);
		Skin *skin = new Skin(startChanges, endChanges, 9, 1);
		SetupTilesets(skin, swordSkin);
		delete skin;
		delete swordSkin;
	}*/
		
	//SetupTilesets(NULL,NULL);

		

	prevRail = NULL;

	maxFramesSinceGrindAttempt = 30;
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	canRailGrind = false;
	canRailSlide = false;

	regrindOffMax = 3;
	regrindOffCount = 3;

	currSpring = NULL;
	currBooster = NULL;
	currTeleporter = NULL;
	currSwingLauncher = NULL;

	currBounceBooster = NULL;
	oldBounceBooster = NULL;

	railTest.setSize(Vector2f(64, 64));
	railTest.setFillColor(Color( COLOR_ORANGE.r, COLOR_ORANGE.g, COLOR_ORANGE.b, 80 ));
	railTest.setOrigin(railTest.getLocalBounds().width / 2, railTest.getLocalBounds().height / 2);

	ts_dirtyAura = sess->GetTileset("Kin/dark_aura_w1_384x384.png", 384, 384);
	dirtyAuraSprite.setTexture(*ts_dirtyAura->texture);
	//dirtyAuraSprite.setpo
	//dirtyAuraSprite.setOrigin( )

	//framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	jumpGrassCount = 0;
	gravityGrassCount = 0;
	bounceGrassCount = 0;
	boostGrassCount = 0;

	scorpOn = false;
	framesSinceRightWireBoost = 0;
	framesSinceLeftWireBoost = 0;
	framesSinceDoubleWireBoost = 0;

	glideTurnFactor = 0;

	singleWireBoostTiming = 4;
	doubleWireBoostTiming = 4;

	glideTurnAccel = .01;
	maxGlideTurnFactor = .08;

	//(0x14, 0x59, 0x22) = Kin Green
	// gonna try to make this red in his airdash animation
	spriteAction = FAIR;
	currTileIndex = 0;

	framesNotGrinding = 0;
	bufferedAttack = JUMP;
	oldBounceEdge = NULL;
	//seq = SEQ_NOTHING;
	enemiesKilledThisFrame = 0;
	enemiesKilledLastFrame = 0;
	GameController &cont = GetController(actorIndex);
	toggleBounceInput = cont.keySettings.toggleBounce;
	toggleTimeSlowInput = cont.keySettings.toggleTimeSlow;
	toggleGrindInput = cont.keySettings.toggleGrind;
	speedParticleRate = 10; //20
	speedParticleCounter = 1;
	hitGoal = false;
	hitNexus = false;
	ground = NULL;
	//re = new RotaryParticleEffect( this );
	//re1 = new RotaryParticleEffect( this );
	//pTrail = new ParticleTrail( this );
	//re1->angle += PI;
	//ae = new AirParticleEffect( position );

	level1SpeedThresh = 25;	//30;//22;//22;//32;
	level2SpeedThresh = 45; 
	speedChangeUp = .5;//03;//.5;
	speedChangeDown = .03;//.005;//.07;


	grindLungeSpeed0 = 15.0;
	grindLungeSpeed1 = 17.0;//20.0;
	grindLungeSpeed2 = 22.0;//28.0;
	//grindLungeExtraMax = 10.0;

	speedLevel = 0;
	currentSpeedBar = 0;

	motionGhostSpacing = 1;
	ghostSpacingCounter = 0;

	drainCounterMax = 10;
	drainAmount = 1;
	drainCounter = 0;
	//currentCheckPoint = NULL;
	flashFrames = 0;
	hitEnemyDuringPhyiscs = false;

	lastWire = 0;
	inBubble = false;
	oldInBubble = false;
		
		

	gateTouched = NULL;

	//activeEdges = new Edge*[16]; //this can probably be really small I don't think it matters. 
	//numActiveEdges = 0;
	assert( Shader::isAvailable() && "help me" );
	if (!sh.loadFromFile("Resources/Shader/player_shader.frag", sf::Shader::Fragment))
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{

		cout << "PLAYER SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "player shader not loaded" );
	}
	Tileset *ts_auraTest = sess->GetSizedTileset("FX/aura1_64x64.png");
	Tileset *ts_auraTest2 = sess->GetSizedTileset("FX/aura2_64x64.png");
	Color auraColor(Color::Cyan);
	//auraColor.a = 200;
	sh.setUniform("u_auraColor", ColorGL( auraColor ));
	ts_auraTest2->texture->setRepeated(true);
	ts_auraTest->texture->setRepeated(true);
	sh.setUniform("u_auraTex", *(ts_auraTest->texture));
	sh.setUniform("u_auraTex2", *(ts_auraTest2->texture));

	if (!despFaceShader.loadFromFile("Resources/Shader/colorswap_shader.frag", sf::Shader::Fragment))
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "desp face SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "desp shader not loaded" );
	}
	//uniform vec4 toColor;
	//uniform vec4 fromColor;
	Color c( 0x66, 0xee, 0xff );
	despFaceShader.setUniform( "fromColor", ColorGL( c ) );

	if (!motionGhostShader.loadFromFile("Resources/Shader/motionghost_shader.frag", sf::Shader::Fragment))
	{
		cout << "motion ghost SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "desp shader not loaded");
	}
		
	Color mgc = Color::Cyan;
	mgc.a = 25;
	//motionGhostShader.setUniform("energyColor", ColorGL(mgc));
		
	motionGhostShader.setUniform("u_texture", sf::Shader::CurrentTexture);


	if (!playerDespShader.loadFromFile("Resources/Shader/playerdesperation.frag", sf::Shader::Fragment))
		//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "desp player SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "desp shader not loaded");
	}
	playerDespShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	if (!playerSuperShader.loadFromFile("Resources/Shader/playersuper_shader.frag", sf::Shader::Fragment))
		//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "super player SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "super shader not loaded");
	}
	playerSuperShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	//swordShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	/*if( !timeSlowShader.loadFromFile( "Shader/timeslow_shader.frag", sf::Shader::Fragment ) )
	{
		cout << "TIME SLOW SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "time slow shader not loaded" );
	}*/
	int sizeofsoundbuf = sizeof(soundBuffers);
	memset(soundBuffers, 0, sizeofsoundbuf );
		
	soundBuffers[S_HITCEILING] = GetSound("Kin/ceiling");
	soundBuffers[S_CLIMB_STEP1] = GetSound("Kin/climb_01a");
	soundBuffers[S_CLIMB_STEP2] = GetSound("Kin/climb_02a");
	soundBuffers[S_DAIR] = GetSound("Kin/dair");
	soundBuffers[S_DOUBLE] = GetSound("Kin/doublejump");
	soundBuffers[S_DOUBLEBACK] = GetSound("Kin/doublejump_back");
	soundBuffers[S_FAIR1] = GetSound("Kin/fair");
	soundBuffers[S_JUMP] = GetSound("Kin/jump");
	soundBuffers[S_LAND] = GetSound("Kin/land");
	soundBuffers[S_RUN_STEP1] = GetSound( "Kin/run_01a" );
	soundBuffers[S_RUN_STEP2] = GetSound( "Kin/run_01b" );
	soundBuffers[S_SLIDE] = GetSound("Kin/slide");
	soundBuffers[S_SPRINT_STEP1] = GetSound( "Kin/sprint_01a" );
	soundBuffers[S_SPRINT_STEP2] = GetSound( "Kin/sprint_01b" );
	soundBuffers[S_STANDATTACK] = GetSound("Kin/stand");
	soundBuffers[S_STEEPSLIDE] = GetSound("Kin/steep");
	soundBuffers[S_STEEPSLIDEATTACK] = GetSound("Kin/steep_att");
	soundBuffers[S_UAIR] = GetSound("Kin/uair");
	soundBuffers[S_WALLATTACK] = GetSound("Kin/wall_att");
	soundBuffers[S_WALLJUMP] = GetSound("Kin/walljump");
	soundBuffers[S_WALLSLIDE] = GetSound("Kin/wallslide");

	soundBuffers[S_GOALKILLSLASH1] = GetSound("Kin/goal_kill_01");
	soundBuffers[S_GOALKILLSLASH2] = GetSound("Kin/goal_kill_02");
	soundBuffers[S_GOALKILLSLASH3] = GetSound("Kin/goal_kill_03");
	soundBuffers[S_GOALKILLSLASH4] = GetSound("Kin/goal_kill_04");


	/*soundBuffers[S_DASH_START] = GetSound( "Kin/dash_02" );
	soundBuffers[S_HIT] = GetSound( "kin_hitspack_short" );
	soundBuffers[S_HURT] = GetSound( "Kin/hit_1b" );
	soundBuffers[S_HIT_AND_KILL] = GetSound( "Kin/kin_hitspack" );
	soundBuffers[S_HIT_AND_KILL_KEY] = GetSound( "Kin/key_kill" );
		
		
	soundBuffers[S_GRAVREVERSE] = GetSound( "Kin/gravreverse" );
	soundBuffers[S_BOUNCEJUMP] = GetSound( "Kin/bounce" );
		
	soundBuffers[S_TIMESLOW] = GetSound( "Kin/time_slow_1" );
	soundBuffers[S_ENTER] = GetSound( "Kin/enter" );
	soundBuffers[S_EXIT] = GetSound( "Kin/exit" );

	soundBuffers[S_DIAGUPATTACK] = soundBuffers[S_FAIR1];
	soundBuffers[S_DIAGDOWNATTACK] = soundBuffers[S_FAIR1];*/


	currHitboxInfo = new HitboxInfo();
	currHitboxInfo->damage = 20;
	currHitboxInfo->drainX = .5;
	currHitboxInfo->drainY = .5;
	currHitboxInfo->hitlagFrames = 0;
	currHitboxInfo->hitstunFrames = 30;
	currHitboxInfo->knockback = 0;
	currHitboxInfo->freezeDuringStun = true;
		
	framesGrinding = 0;
	maxDespFrames = 60 * 5;
	despCounter = 0;

	maxSuperFrames = 60 * 5;
	

	

		

	holdJump = false;
	steepJump = false;

	bounceBoostSpeed = 6.0;//8.0;//.5;//1;//6.0;//5.0;//4.7;

	offsetX = 0;
	sprite = new Sprite;
	if( actorIndex == 1 )
	{
		//sprite->setColor( Color( 255, 0, 0 ) );
	}

	velocity = Vector2<double>( 0, 0 );
		
	CollisionBox cb;
	cb.isCircle = true;
	cb.offset.x = 32;
	cb.offset.y = -8;
	//cb.offsetAngle = 0;
	cb.rw = 64;
	cb.rh = 64;

	//setup hitboxes
	//for( int j = 4; j < 10; ++j )

	/*diagDownSwordOffset[0] = Vector2f(32, 24);
	diagDownSwordOffset[1] = Vector2f(32, 24);
	diagDownSwordOffset[2] = Vector2f(32, 24);*/
	diagDownSwordOffset[0] = Vector2f(32, 24);
	diagDownSwordOffset[1] = Vector2f(16, 32);
	diagDownSwordOffset[2] = Vector2f(16, 64);
	//Vector2f(16, 32)
	//Vector2f(16, 64)

	Vector2i offsets[3];//( 0, 0 );
	offsets[0] = Vector2i(40, -32);
		

	/*diagUpSwordOffset[0] = Vector2f(40, -32);
	diagUpSwordOffset[1] = Vector2f(40, -32);
	diagUpSwordOffset[2] = Vector2f(40, -32);*/

	diagUpSwordOffset[0] = Vector2f(40, -32);
	diagUpSwordOffset[1] = Vector2f(16, -40);
	diagUpSwordOffset[2] = Vector2f(32, -48);
	/*offsets[1] = Vector2i(16, -40);
	offsets[2] = Vector2i(32, -48);*/

	standSwordOffset[0] = Vector2f(64, 64);//Vector2f(0, -64);
	standSwordOffset[1] = Vector2f(64, 32);//Vector2f(0, -64);
	standSwordOffset[2] = Vector2f(64, 16);//Vector2f(0, -64);

	climbAttackOffset[0] = Vector2f(0, -32);
	climbAttackOffset[1] = Vector2f(0, -128);
	climbAttackOffset[2] = Vector2f(0, -72);

	slideAttackOffset[0] = Vector2f(0, -56);
	slideAttackOffset[1] = Vector2f(0, -64);
	slideAttackOffset[2] = Vector2f(0, -96);

	if (true)
	{


		std::map<int, std::list<CollisionBox>> & fairAList =
			GetHitboxList("fairahitboxes");

		std::map<int, std::list<CollisionBox>> & fairBList =
			GetHitboxList("fairbhitboxes");

		std::map<int, std::list<CollisionBox>> & fairCList =
			GetHitboxList("fairchitboxes");

		std::map<int, std::list<CollisionBox>> & dairAList =
			GetHitboxList("dairahitboxes");

		std::map<int, std::list<CollisionBox>> & dairBList =
			GetHitboxList("dairbhitboxes");

		std::map<int, std::list<CollisionBox>> & dairCList =
			GetHitboxList("dairchitboxes");

		std::map<int, std::list<CollisionBox>> & uairAList =
			GetHitboxList("uairahitboxes");

		std::map<int, std::list<CollisionBox>> & uairBList =
			GetHitboxList("uairbhitboxes");

		std::map<int, std::list<CollisionBox>> & uairCList =
			GetHitboxList("uairchitboxes");

		std::map<int, std::list<CollisionBox>> & adUpAList =
			GetHitboxList("airdashupahitboxes");

		std::map<int, std::list<CollisionBox>> & adUpBList =
			GetHitboxList("airdashupbhitboxes");

		std::map<int, std::list<CollisionBox>> & adUpCList =
			GetHitboxList("airdashupchitboxes");

		std::map<int, std::list<CollisionBox>> & adDownAList =
			GetHitboxList("airdashdownahitboxes");

		std::map<int, std::list<CollisionBox>> & adDownBList =
			GetHitboxList("airdashdownbhitboxes");

		std::map<int, std::list<CollisionBox>> & adDownCList =
			GetHitboxList("airdashdownchitboxes");

		std::map<int, std::list<CollisionBox>> & standAList =
			GetHitboxList("standahitboxes");

		std::map<int, std::list<CollisionBox>> & standBList =
			GetHitboxList("standbhitboxes");

		std::map<int, std::list<CollisionBox>> & standCList =
			GetHitboxList("standchitboxes");



		std::map<int, std::list<CollisionBox>> & wallAList =
			GetHitboxList("wallahitboxes");

		std::map<int, std::list<CollisionBox>> & wallBList =
			GetHitboxList("wallbhitboxes");

		std::map<int, std::list<CollisionBox>> & wallCList =
			GetHitboxList("wallchitboxes");

		std::map<int, std::list<CollisionBox>> & climbAList =
			GetHitboxList("climbahitboxes");

		std::map<int, std::list<CollisionBox>> & climbBList =
			GetHitboxList("climbbhitboxes");

		std::map<int, std::list<CollisionBox>> & climbCList =
			GetHitboxList("climbchitboxes");

		std::map<int, std::list<CollisionBox>> & slideAList =
			GetHitboxList("slideahitboxes");

		std::map<int, std::list<CollisionBox>> & slideBList =
			GetHitboxList("slidebhitboxes");

		std::map<int, std::list<CollisionBox>> & slideCList =
			GetHitboxList("slidechitboxes");



		fairHitboxes[0] = new CollisionBody(16, fairAList, currHitboxInfo);
		uairHitboxes[0] = new CollisionBody(16, uairAList, currHitboxInfo);
		dairHitboxes[0] = new CollisionBody(16, dairAList, currHitboxInfo);

		fairHitboxes[1] = new CollisionBody(16, fairBList, currHitboxInfo);
		uairHitboxes[1] = new CollisionBody(16, uairBList, currHitboxInfo);
		dairHitboxes[1] = new CollisionBody(16, dairBList, currHitboxInfo);

		fairHitboxes[2] = new CollisionBody(16, fairCList, currHitboxInfo);
		uairHitboxes[2] = new CollisionBody(16, uairCList, currHitboxInfo);
		dairHitboxes[2] = new CollisionBody(16, dairCList, currHitboxInfo);


		standHitboxes[0] = new CollisionBody(8, standAList, currHitboxInfo);
		standHitboxes[1] = new CollisionBody(8, standBList, currHitboxInfo);
		standHitboxes[2] = new CollisionBody(8, standCList, currHitboxInfo);

		////dashHitboxes[0] = NULL;
		wallHitboxes[0] = new CollisionBody(8, wallAList, currHitboxInfo);

		wallHitboxes[1] = new CollisionBody(8, wallBList, currHitboxInfo);

		wallHitboxes[2] = new CollisionBody(8, wallCList, currHitboxInfo);


		steepClimbHitboxes[0] = new CollisionBody(8, climbAList, currHitboxInfo);
		steepClimbHitboxes[1] = new CollisionBody(8, climbBList, currHitboxInfo);
		steepClimbHitboxes[2] = new CollisionBody(8, climbCList, currHitboxInfo);

		steepSlideHitboxes[0] = new CollisionBody(8, slideAList, currHitboxInfo);
		steepSlideHitboxes[1] = new CollisionBody(8, slideBList, currHitboxInfo);
		steepSlideHitboxes[2] = new CollisionBody(8, slideCList, currHitboxInfo);

		diagUpHitboxes[0] = new CollisionBody(12, adUpAList, currHitboxInfo);
		diagDownHitboxes[0] = new CollisionBody(12, adDownAList, currHitboxInfo);

		diagUpHitboxes[1] = new CollisionBody(12, adUpBList, currHitboxInfo);
		diagDownHitboxes[1] = new CollisionBody(12, adDownBList, currHitboxInfo);

		diagUpHitboxes[2] = new CollisionBody(12, adUpCList, currHitboxInfo);
		diagDownHitboxes[2] = new CollisionBody(12, adDownCList, currHitboxInfo);



		for (int i = 0; i < 3; ++i)
		{
			diagDownHitboxes[i]->OffsetAllFrames(diagDownSwordOffset[i]);
			diagUpHitboxes[i]->OffsetAllFrames(diagUpSwordOffset[i]);
		}

		for (int i = 0; i < 3; ++i)
		{
			Vector2f testOffset = standSwordOffset[i];
			testOffset.y -= ts_standingNSword[i]->tileHeight / 2.0;
			//standHitboxes[i]->OffsetAllFrames(standSwordOffset[i]);
			standHitboxes[i]->OffsetAllFrames(testOffset);
		}

		for (int i = 0; i < 3; ++i)
		{
			Vector2f testOffset = -slideAttackOffset[i];
			testOffset.y -= ts_steepSlideAttackSword[i]->tileHeight / 2.0;
			steepSlideHitboxes[i]->OffsetAllFrames(testOffset);
		}

		for (int i = 0; i < 3; ++i)
		{
			Vector2f testOffset = -climbAttackOffset[i];
			testOffset.y -= ts_steepClimbAttackSword[i]->tileHeight / 2.0;
			steepClimbHitboxes[i]->OffsetAllFrames(testOffset);
		}

		shockwaveHitboxes = NULL;
		grindHitboxes[0] = NULL;

		cb.rw = 90;
		cb.rh = 90;
		cb.offset.x = 0;
		cb.offset.y = 0;
		grindHitboxes[0] = new CollisionBody( CollisionBox::Hit);
		grindHitboxes[0]->BasicSetup();
		grindHitboxes[0]->AddCollisionBox(0, cb);
		//up
	}
	else
	{
		fairHitboxes[0] = NULL;
		uairHitboxes[0] = NULL;
		dairHitboxes[0] = NULL;

		fairHitboxes[1] = NULL;
		uairHitboxes[1] = NULL;
		dairHitboxes[1] = NULL;

		fairHitboxes[2] = NULL;
		uairHitboxes[2] = NULL;
		dairHitboxes[2] = NULL;


		standHitboxes[0] = NULL;
		standHitboxes[1] = NULL;
		standHitboxes[2] = NULL;

		wallHitboxes[0] = NULL;

		wallHitboxes[1] = NULL;

		wallHitboxes[2] = NULL;


		steepClimbHitboxes[0] = NULL;
		steepClimbHitboxes[1] = NULL;
		steepClimbHitboxes[2] = NULL;

		steepSlideHitboxes[0] = NULL;
		steepSlideHitboxes[1] = NULL;
		steepSlideHitboxes[2] = NULL;

		diagUpHitboxes[0] = NULL;
		diagDownHitboxes[0] = NULL;

		diagUpHitboxes[1] = NULL;
		diagDownHitboxes[1] = NULL;

		diagUpHitboxes[2] = NULL;
		diagDownHitboxes[2] = NULL;

		shockwaveHitboxes = NULL;
		grindHitboxes[0] = NULL;
	}

		
	//grindHitboxes[0] = new list<CollisionBox>;
	//grindHitboxes[0]->push_back( cb );
		

	queryMode = "";
	wallThresh = .9999;
	//tileset setup
		
		

	BounceFlameOff();


	airBounceFlameFrames = 20 * 3;
	runBounceFlameFrames = 21 * 3;
	maxBBoostCount = GetActionLength(DASH);
		 	

	gsdodeca.setTexture( *tsgsdodeca->texture);
	gstriblue.setTexture( *tsgstriblue->texture);
	gstricym.setTexture( *tsgstricym->texture);
	gstrigreen.setTexture( *tsgstrigreen->texture);
	gstrioran.setTexture( *tsgstrioran->texture);
	gstripurp.setTexture( *tsgstripurp->texture);
	gstrirgb.setTexture( *tsgstrirgb->texture);



	if (!swordShaders[0].loadFromFile("Resources/Shader/colorswap_shader.frag", sf::Shader::Fragment))
	{
		cout << "SWORD SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "sword shader not loaded" );
	}
	swordShaders[0].setUniform( "fromColor", ColorGL(COLOR_TEAL) );
	swordShaders[0].setUniform("u_texture", sf::Shader::CurrentTexture);
	//swordShaders[1] = swordShaders[0];
	if (!swordShaders[1].loadFromFile("Resources/Shader/colorswap_shader.frag", sf::Shader::Fragment))
	{
		cout << "SWORD SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "sword shader not loaded" );
	}
	swordShaders[1].setUniform( "fromColor", ColorGL(Color( 43, 167, 255 )) );
	swordShaders[1].setUniform("u_texture", sf::Shader::CurrentTexture);
	//swordShaders[2] = swordShaders[0];
	if (!swordShaders[2].loadFromFile("Resources/Shader/colorswap_shader.frag", sf::Shader::Fragment))
	{
		cout << "SWORD SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "sword shader not loaded" );
	}
	swordShaders[2].setUniform( "fromColor", ColorGL(Color( 140, 146, 255 )) );
	swordShaders[2].setUniform("u_texture", sf::Shader::CurrentTexture);

	grindActionLength = 32;
	//SetAction( SPAWNWAIT );

	//if (owner != NULL)
	//{
	//	SetAction(INTROBOOST);//INTRO
	//	frame = 0;
	//}
	//else
	//{
	//	SetAction(JUMP);
	//	frame = 1;
	//}
		
	Init();

	baseSlowMultiple = 1;
	timeSlowStrength = 5 * baseSlowMultiple;
	slowMultiple = baseSlowMultiple;
	slowCounter = 1;

	reversed = false;

	grindActionCurrent = 0;

	framesInAir = 0;
	wallJumpFrameCounter = 0;
	wallJumpMovementLimit = 12; //10 frames

	steepThresh = .4; // go between 0 and 1

	steepSlideGravFactor = .25;//.25;//.4;
	steepSlideFastGravFactor = .3;//.5;

	wallJumpStrength.x = 10;
	wallJumpStrength.y = 20;
	clingSpeed = 3;

	slopeTooSteepLaunchLimitX = .1;
		
	steepClimbGravFactor = .6;//.31;
	steepClimbUpFactor = .31;
	steepClimbDownFactor = .8;
	steepClingSpeedLimit = 2.0;
	//steepClimbFastFactor = .7;//.2;
	framesSinceClimbBoost = 0;
	climbBoostLimit = 25;//22;//15;
		

		
		
	hasDoubleJump = true;
		

	ground = NULL;

	groundSpeed = 0;
	maxNormalRun = 60; //adjust up w/ more power?
	
	facingRight = true;
	collision = false;
	
	airAccel = 1.5;
		
	gravity = 1;//1.9; // 1 
	wallClimbGravityFactor = .7;
	wallClimbGravityOn = false;
	jumpStrength = 21.5;//18;//25;//27.5; // 2 
	doubleJumpStrength = 20;//17;//23;//26.5;
	backDoubleJumpStrength = 22;
	dashSpeed = 9;//12; // 3

	dashSpeed0 = 9;
	dashSpeed1 = 10.5;
	dashSpeed2 = 14;

	airDashSpeed0 = dashSpeed0;
	airDashSpeed1 = dashSpeed1;
	airDashSpeed2 = dashSpeed2;

	airDashSpeed = dashSpeed;

	maxVelocity = 60;
	double maxSpeed = maxVelocity;
	double maxXSpeed = maxVelocity;
	maxGroundSpeed = maxSpeed;
	maxAirXSpeed = maxSpeed;
	maxFallSpeedSlow = 30;//30;//100; // 4
	maxFallSpeedFast = maxSpeed;

	scorpAdditionalAccel = .2;
	scorpAdditionalCapMax = 20.0;//12.0;
	scorpAdditionalCap = 0.0;

	offSlopeByWallThresh = dashSpeed;//18;
	slopeLaunchMinSpeed = 5;//dashSpeed * .7;
	steepClimbSpeedThresh = dashSpeed - 1;
	slideGravFactor = .25;//.25;//.3;//.45;

		
	maxRunInit = 4;
	maxAirXControl = 6;//maxRunInit;
	airSlow = .3;//.7;//.3;

	groundOffsetX = 0;

	grindEdge = NULL;
	grindQuantity = 0;
	grindSpeed = 0;

		
		

	//max ground speed should probably start around 60-80 and then get powerups to rise to 100
	//for world 1 lets do the lowest number for the beta
		


	runAccelInit = .5;
		
	runAccel = .03;
	sprintAccel = .08;//.3;//.85;

	holdDashAccel = .05;
	bounceFlameAccel0 = .08;//.15;//.8;
	bounceFlameAccel1 = .1; //this was the original
	bounceFlameAccel2 = .15;//.18;

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

	//hurtboxMap[STAND] = new CollisionBody(1);
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.isCircle = false;
	hurtBody.rw = 7;//10;
	hurtBody.rh = 15;//normalHeight - 5;//normalHeight;
	//hurtboxMap[STAND]->AddCollisionBox(0, hurtBody);

		


	currHitboxes = NULL;
	//currHitboxInfo = NULL;
		
		 
	wireChargeInfo = new HitboxInfo();
	wireChargeInfo->damage = 20;
	wireChargeInfo->drainX = .5;
	wireChargeInfo->drainY = .5;
	wireChargeInfo->hitlagFrames = 0;
	wireChargeInfo->hitstunFrames = 30;//30;
	wireChargeInfo->knockback = 0;
	wireChargeInfo->freezeDuringStun = true;

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
	bounceGrounded = false;

		

	minRailGrindSpeed[0] = dashSpeed0;
	minRailGrindSpeed[1] = dashSpeed1;
	minRailGrindSpeed[2] = dashSpeed2;

	touchEdgeWithLeftWire= false;
	touchEdgeWithRightWire= false;

	bubbleSprite.setTexture( *ts_bubble->texture );

	currBubble = 0;
	bubbleRadius = 160;

	bubbleRadius0 = 160;
	bubbleRadius1 = 180;
	bubbleRadius2 = 200;
		

	bubbleLifeSpan = 240;

	bHasUpgradeField.Reset();
	bStartHasUpgradeField.Reset();

	bool isCampaign = false;
	if (owner != NULL)
	{
		SaveFile *currProgress = owner->GetCurrentProgress();
		if (currProgress != NULL )
		{
			isCampaign = true;
			//set this up better later
			if( currProgress->HasUpgrade(UPGRADE_POWER_AIRDASH) )
				SetStartUpgrade(UPGRADE_POWER_AIRDASH, true);
		}
	}

	if (!isCampaign)
	{
		SetAllUpgrades(sess->playerOptionsField);
	}

	//for (int i = 0; i < UPGRADE_Count; ++i)
	//{
	//	SetStartUpgrade(i, true);
	//}

	//SetStartUpgrade(UPGRADE_POWER_AIRDASH, false);

	SetupTimeBubbles();


	cout << "end player" << endl;
}

Actor::~Actor()
{
	//delete skin;
	//delete swordSkin;
	/*if (glideEmitter != NULL)
	{
		delete glideEmitter;
	}*/

	delete rpu;

	if( kinMask != NULL)
		delete kinMask;
	for (int i = 0; i < 7; ++i)
	{
		delete smallLightningPool[i];
	}

	//delete risingAuraPool;
	for (int i = 0; i < 3; ++i)
	{
		delete motionGhostsEffects[i];
	}

	//delete kinRing;
//	delete dustParticles;

	for (int i = 0; i < 3; ++i)
	{
		delete fairLightningPool[i];
		delete uairLightningPool[i];
		delete dairLightningPool[i];
	}
	delete gateBlackFXPool;
	delete wireChargeInfo;

	delete keyExplodePool;
	delete keyExplodeUpdater;
	delete keyExplodeRingGroup;
	
	delete sprite;

	delete motionGhostBuffer;
	delete motionGhostBufferBlue;
	delete motionGhostBufferPurple;

	delete currHitboxInfo;

	/*if (owner != NULL)*/
	{
		for (int i = 0; i < 3; ++i)
		{
			delete fairHitboxes[i];
			delete uairHitboxes[i];
			delete dairHitboxes[i];
			delete standHitboxes[i];
			delete wallHitboxes[i];
			//		delete dashHitboxes[i];

				//	delete wallHitboxes[i];
			delete steepClimbHitboxes[i];
			delete steepSlideHitboxes[i];
			delete diagUpHitboxes[i];
			delete diagDownHitboxes[i];
			//delete grindHitboxes[i];
		}

		delete grindHitboxes[0];

	}

	
	//delete shockwaveHitboxes;

	if (rightWire != NULL)
		delete rightWire;
	if (leftWire != NULL)
		delete leftWire;

	delete[] bubblePos;
	
	delete[] bubbleFramesToLive;
	delete[] bubbleRadiusSize;
	

	for (int i = 0; i < maxBubbles; ++i)
	{
		delete bubbleHitboxes[i];
	}
	delete[] bubbleHitboxes;

	//eventually delete everything here lol
}

void Actor::SetGameMode()
{
	int mapType;
	if (sess->mapHeader == NULL)
	{
		//blank editor files only:
		mapType = MapHeader::MapType::T_STANDARD;
	}
	else
	{
		mapType = sess->mapHeader->gameMode;
	}

	if (mapType == MapHeader::MapType::T_RACEFIGHT)
	{
		maxBubbles = 2;
		invincibleFrames = 180;
	}
	else
	{
		maxBubbles = MAX_BUBBLES;
		invincibleFrames = 0;
	}
}

void Actor::SetupTimeBubbles()
{
	maxBubbles = MAX_BUBBLES;

	bubbleHitboxes = new CollisionBody*[MAX_BUBBLES];

	CollisionBox genericBox;
	genericBox.isCircle = true;
	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		bubbleHitboxes[i] = new CollisionBody(CollisionBox::Hit);
		bubbleHitboxes[i]->BasicSetup();
		bubbleHitboxes[i]->AddCollisionBox(0, genericBox);
	}

	bubblePos = new V2d[MAX_BUBBLES];
	bubbleFramesToLive = new int[MAX_BUBBLES];
	bubbleRadiusSize = new int[MAX_BUBBLES];

	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		bubbleFramesToLive[i] = 0;

		if (fBubblePos != NULL)
		{
			fBubbleFrame[i] = 0;
		}
	}
}

void Actor::ActionEnded()
{
	if (frame >= GetActionLength(action))
	{
		EndAction();
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

bool Actor::SteepSlideAttack()
{
	if (pauseBufferedAttack == Action::STEEPSLIDEATTACK)
	{
		SetAction(pauseBufferedAttack);
		frame = 0;
		return true;
	}

	bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
	bool rightStickSwing = (currInput.RDown() && !prevInput.RDown())
		|| (currInput.RLeft() && !prevInput.RLeft())
		|| (currInput.RUp() && !prevInput.RUp())
		|| (currInput.RRight() && !prevInput.RRight());

	if (normalSwing || rightStickSwing )
	{
		SetAction(STEEPSLIDEATTACK);
		frame = 0;
		return true;
	}

	return false;
}

bool Actor::SteepClimbAttack()
{
	if (pauseBufferedAttack == Action::STEEPCLIMBATTACK)
	{
		SetAction(pauseBufferedAttack);
		frame = 0;
		return true;
	}

	bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
	bool rightStickSwing = (currInput.RDown() && !prevInput.RDown())
		|| (currInput.RLeft() && !prevInput.RLeft())
		|| (currInput.RUp() && !prevInput.RUp())
		|| (currInput.RRight() && !prevInput.RRight());

	if (normalSwing || rightStickSwing)
	{
		SetAction(STEEPCLIMBATTACK);
		frame = 0;
		return true;
	}

	return false;
}

bool Actor::AirAttack()
{
	if (pauseBufferedAttack != Action::Count)
	{
		SetAction(pauseBufferedAttack);
		frame = 0;
		return true;
	}

	if (stunBufferedAttack != Action::Count)
	{
		SetAction(stunBufferedAttack);
		frame = 0;
		return true;
	}

	bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
	bool rightStickSwing = (currInput.RDown() && !prevInput.RDown())
		|| (currInput.RLeft() && !prevInput.RLeft())
		|| (currInput.RUp() && !prevInput.RUp())
		|| (currInput.RRight() && !prevInput.RRight());


	if( normalSwing || rightStickSwing )
	{
		if( action == AIRDASH )
		{
			if (normalSwing)
			{
				if (currInput.LUp())
				{
					if (currInput.LRight() || currInput.LLeft())
					{
						SetAction(DIAGUPATTACK);
						frame = 0;
						return true;
					}
				}
				else if (currInput.LDown())
				{
					if (currInput.LRight() || currInput.LLeft())
					{
						SetAction(DIAGDOWNATTACK);
						frame = 0;
						return true;
					}
				}
			}
			else
			{
				return false;
				/*bool sideInput = currInput.RRight() || currInput.RLeft();
				if (currInput.RUp())
				{
					SetAction(DIAGUPATTACK);
					frame = 0;
					return true;
				}
				else if (currInput.RDown())
				{
					SetAction(DIAGDOWNATTACK);
					frame = 0;
					return true;
				}*/
			}
		}
		
		if (normalSwing)
		{
			if ((currInput.LUp()))
			{
				if (action == UAIR)
				{
					if (currLockedUairFX != NULL && action != UAIR)
					{
						currLockedUairFX->ClearLockPos();
						currLockedUairFX = NULL;
					}
				}
				SetAction(UAIR);
				frame = 0;
			}
			else if (currInput.LDown())
			{
				if (action == DAIR)
				{
					if (currLockedDairFX != NULL && action != DAIR)
					{
						currLockedDairFX->ClearLockPos();
						currLockedDairFX = NULL;
					}
				}
				SetAction(DAIR);
				frame = 0;
			}
			else
			{
				if (action == FAIR)
				{
					if (currLockedFairFX != NULL && action != FAIR)
					{
						currLockedFairFX->ClearLockPos();
						currLockedFairFX = NULL;
					}
				}
				SetAction(FAIR);
				frame = 0;
			}
		}
		else
		{
			if ((currInput.RUp()) )
			{
				if (action == UAIR)
				{
					if (currLockedUairFX != NULL && action != UAIR)
					{
						currLockedUairFX->ClearLockPos();
						currLockedUairFX = NULL;
					}
				}
				SetAction(UAIR);
				frame = 0;
			}
			else if (currInput.RDown() )
			{
				if (action == DAIR)
				{
					if (currLockedDairFX != NULL && action != DAIR)
					{
						currLockedDairFX->ClearLockPos();
						currLockedDairFX = NULL;
					}
				}
				SetAction(DAIR);
				frame = 0;
			}
			else
			{
				if (action == FAIR)
				{
					if (currLockedFairFX != NULL && action != FAIR)
					{
						currLockedFairFX->ClearLockPos();
						currLockedFairFX = NULL;
					}
				}
				SetAction(FAIR);
				frame = 0;
			}

		}
		
		return true;
	}
	return false;
}

void Actor::CreateGateExplosion()
{
	Vector2f floatPos(position);
	keyExplodeRingGroup->SetBase(floatPos);
	keyExplodeRingGroup->Reset();
	keyExplodeRingGroup->Start();
	sess->cam.SetRumble(3, 3, 20);//5
	sess->Pause(4);//5
}

void Actor::CreateKeyExplosion()
{
	//this gets rid of any keys that are still around, instead of
	//letting the player collect them after going through the door
	


	EffectInstance params;
	Transform tr = sf::Transform::Identity;
	//params.SetParams(Vector2f(position), tr, 16, 4, 0);
	//params.SetVelocityParams(Vector2f(0, -1), Vector2f(), 10);

	Transform posTransform;
	Vector2f pos(0, -10);
	Vector2f adjustedPoint;

	Vector2f floatPos(position);
	for (int i = 0; i < numKeysHeld; ++i)
	{
		adjustedPoint = posTransform.transformPoint(pos);
		params.SetParams(floatPos + adjustedPoint, tr, 16, 4, 0);
		params.SetVelocityParams(normalize(adjustedPoint) * 4.f, Vector2f(), 10.f);

		keyExplodePool->ActivateEffect(&params);

		posTransform.rotate(360.f / numKeysHeld);
	}

	CreateGateExplosion();
}

void Actor::CreateAttackLightning()
{
	if ( frame != 0 || slowCounter != 1 
		|| ( action != FAIR && action != DAIR && action != UAIR ) )
		return;

	RelEffectInstance params;
	Transform tr = sf::Transform::Identity;
	if (!facingRight)
	{
		tr.scale(Vector2f(-1, 1));
	}
	params.SetParams(Vector2f(0, 0), tr, 23, 1, 0, &spriteCenter );
	//fair should be 25 but meh

	if (!facingRight)
	{
		tr.scale(-1, 1);
	}

	switch (action)
	{
	case FAIR:
		currLockedFairFX = (RelEffectInstance*)fairLightningPool[0]->ActivateEffect(&params);
		break;
	case DAIR:
		currLockedDairFX = (RelEffectInstance*)dairLightningPool[0]->ActivateEffect(&params);
		break;
	case UAIR:
		currLockedUairFX = (RelEffectInstance*)uairLightningPool[0]->ActivateEffect(&params);
		break;
	}
}

void Actor::AddActiveComboObj(ComboObject *c)
{
	if (c->active)
		return;


	ComboObject *testCurr = activeComboObjList;
	while (testCurr != NULL)
	{
		/*if (testCurr == c)
		{
			int xxx = 5;
		}*/

		testCurr = testCurr->nextComboObj;//activeComboObjList->nextComboObj;
	}

	c->active = true;
	if (activeComboObjList == NULL)
	{
		activeComboObjList = c;
	}
	else
	{
		c->nextComboObj = activeComboObjList;
		activeComboObjList = c;
	}
}

void Actor::RemoveActiveComboObj(ComboObject *c)
{
	if (!c->active)
		return;

	if (activeComboObjList == NULL)
		return;
	//assert(activeComboObjList != NULL);

	c->active = false;
	if (c == activeComboObjList)
	{
		activeComboObjList = activeComboObjList->nextComboObj;
	}

	ComboObject *curr = activeComboObjList;
	ComboObject *prev = NULL;
	while (curr != NULL)
	{
		if (curr == c)
		{
			prev->nextComboObj = curr->nextComboObj;
			break;
		}

		prev = curr;
		curr = curr->nextComboObj;
	}
}

void Actor::DebugDrawComboObj(sf::RenderTarget *target)
{
	ComboObject *curr = activeComboObjList;
	while (curr != NULL)
	{
		curr->Draw(target);
		curr = curr->nextComboObj;
	}
}

void Actor::Respawn()
{
	flyCounter = 0;
	kinMode = K_NORMAL;
	action = -1;
	framesStanding = 0;
	keyExplodeRingGroup->Reset();
	numKeysHeld = 0;
	//glideEmitter->Reset();
	//owner->AddEmitter(glideEmitter, EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);
	autoRunStopEdge = NULL;
	stunBufferedJump = false;
	stunBufferedAttack = Action::Count;
	stunBufferedDash = false;
	extraDoubleJump = false;

	if( kinMask != NULL )
		kinMask->Reset();
	SetDirtyAura(false);

	glideTurnFactor = 0;
	storedTrigger = NULL;
	airTrigBehavior = AT_NONE;
	currAirTrigger = NULL;


	bHasUpgradeField.Set(bStartHasUpgradeField);

	activeComboObjList = NULL;

	gravResetFrames = 0;

	currBBoostCounter = 0;
	repeatingSound = NULL;
	currBooster = NULL;
	currSpring = NULL;
	currTeleporter = NULL;
	currSwingLauncher = NULL;
	oldBooster = NULL;

	currBounceBooster = NULL;
	oldBounceBooster = NULL;

	currModifier = NULL;
	oldModifier = NULL;
	extraGravityModifier = 1.0;
	
	slowMultiple = baseSlowMultiple;
	slowCounter = 1;

	currWall = NULL;
	wallClimbGravityOn = false;
	currHurtboxes = NULL;
	currHitboxes = NULL;

	standNDashBoostCurr = 0;
	ClearPauseBufferedActions();
	currLockedFairFX = NULL;
	currLockedDairFX = NULL;
	currLockedUairFX = NULL;

	for (int i = 0; i < 3; ++i)
	{
		fairLightningPool[i]->Reset();
		dairLightningPool[i]->Reset();
		uairLightningPool[i]->Reset();
	}

	gateBlackFXPool->Reset();

	jumpGrassCount = 0;
	gravityGrassCount = 0;
	bounceGrassCount = 0;
	boostGrassCount = 0;
	regrindOffCount = 3;
	scorpAdditionalCap = 0.0;
	prevRail = NULL;
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	canRailSlide = false;
	canRailGrind = false;
	scorpOn = false;

	framesSinceRightWireBoost = 0;
	framesSinceLeftWireBoost = 0;
	framesSinceDoubleWireBoost = 0;

	speedParticleCounter = 0;
	framesNotGrinding = 0;
	bufferedAttack = Actor::JUMP;

	//clean up the checkpoint code at some point

	hitGoal = false;
	hitNexus = false;

	SetToOriginalPos();

	enemiesKilledThisFrame = 0;
	gateTouched = NULL;


	if( owner != NULL && owner->shipEnterScene == NULL )
	{
		SetAction(INTROBOOST);
		frame = 0;
		//ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/enter_fx_320x320.png", 320, 320), position, false, 0, 6, 2, true);
	}
	else if (owner == NULL && editOwner != NULL)
	{
		SetAction(JUMP);
		frame = 1;
	}

	velocity.x = 0;
	velocity.y = 0;
	reversed = false;
	b.offset.y = 0;
	b.rh = normalHeight;
	facingRight = true;
	offsetX = 0;
	prevInput = ControllerState();
	currInput = ControllerState();
	ground = NULL;
	grindEdge = NULL;
	bounceEdge = NULL;
	dead = false;

	receivedHit = NULL;
	speedParticleCounter = 1;
	speedLevel = 0;
	currentSpeedBar = 0;//60;

	bounceFlameOn = false;
	scorpOn = false;

	if( HasUpgrade( UPGRADE_POWER_LWIRE ) )
	{
		leftWire->Reset();
	}
	if(HasUpgrade(UPGRADE_POWER_RWIRE))
	{
		rightWire->Reset();
	}
	
	//powerBar.Reset();
	lastWire = 0;

	flashFrames = 0;
	
	hasDoubleJump = true;
	hasAirDash = true;
	hasGravReverse = true;

	for( int i = 0; i < maxBubbles; ++i )
	{
		bubbleFramesToLive[i] = 0;
		
	}

	if (fBubblePos != NULL)
	{
		for (int i = 0; i < 5; ++i)
		{
			fBubbleFrame[i] = 0;
		}
	}
	

	//for( int i = 0; i < maxMotionGhosts; ++i )
	//{
	//	motionGhosts[i].setPosition( position.x, position.y );
	//}

	if( owner != NULL && owner->raceFight != NULL )
	{
		invincibleFrames = 180;
	}

	//kinFace.setTextureRect(ts_kinFace->GetSubRect(expr + 6));
	//kinFaceBG.setTextureRect(ts_kinFace->GetSubRect(0));

	if( kinRing != NULL && kinRing->powerRing != NULL )
		kinRing->powerRing->ResetFull(); //only means anything for single player
}

double Actor::GetBounceFlameAccel()
{
	double bounceFlameAccel = 0;
	switch( speedLevel )
	{
	case 0:
		
		bounceFlameAccel = bounceFlameAccel0;
		//cout << "zero: " << bounceFlameAccel << endl;
		break;
	case 1:
		bounceFlameAccel = bounceFlameAccel1;
		//cout << "one: " << bounceFlameAccel << endl;
		break;
	case 2:
		
		bounceFlameAccel = bounceFlameAccel2;
		//cout << "two: " << bounceFlameAccel << endl;
		break;
	}
	assert( bounceFlameAccel != 0 );

	return bounceFlameAccel;
}

void Actor::CheckForAirTrigger()
{
	if (owner == NULL)
		return;

	currAirTrigger = NULL;
	queryMode = "airtrigger";
	Rect<double> r(position.x - b.rw + b.offset.x, position.y - b.rh + b.offset.y, 2 * b.rw, 2 * b.rh);
	owner->airTriggerTree->Query(this, r);
	if (currAirTrigger != NULL)
	{
		switch (currAirTrigger->triggerType)
		{
		case AirTrigger::AUTORUNRIGHT:
		case AirTrigger::AUTORUNRIGHTAIRDASH:
			airTrigBehavior = AT_AUTORUNRIGHT;
			break;
		}
		currAirTrigger = NULL;
	}
}

void Actor::HandleAirTrigger()
{
	if (currAirTrigger != NULL)
	{
		if (airTrigBehavior == AT_AUTORUNRIGHT)
		{
			if (ground != NULL)
			{
				owner->adventureHUD->Hide(60);
				SetAction(AUTORUN);
				frame = 0;
				maxAutoRunSpeed = 25;
				facingRight = true;
				airTrigBehavior = AT_NONE;
			}
		}
	}
}

void Actor::KinModeUpdate()
{
	if( kinMode == K_DESPERATION )
	{
		

		int transFrames = 8;
		
		

		Color blah[8];// = { Color( 0x00, 0xff, 0xff ), Color(0x00, 0xbb, 0xff ) };
		blah[0] = Color( 0x00, 0xff, 0xff );
		blah[1] = Color( 0x00, 0xbb, 0xff );
		int cIndex = 2;
		if( HasUpgrade( UPGRADE_POWER_AIRDASH ) )
		{
			blah[cIndex] = Color( 0x00, 0x55, 0xff );
			cIndex++;
		}
		if(HasUpgrade(UPGRADE_POWER_GRAV))
		{
			blah[cIndex] = Color( 0x00, 0xff, 0x88 );
			cIndex++;
		}
		if(HasUpgrade(UPGRADE_POWER_BOUNCE))
		{
			blah[cIndex] = Color( 0xff, 0xff, 0x33 );
			cIndex++;
		}

		if(HasUpgrade(UPGRADE_POWER_GRIND))
		{
			blah[cIndex] = Color( 0xff, 0x88, 0x00 );
			cIndex++;
		}

		if(HasUpgrade(UPGRADE_POWER_TIME ))
		{
			blah[cIndex] = Color( 0xff, 0x00, 0x00 );
			cIndex++;
		}

		if(HasUpgrade(UPGRADE_POWER_LWIRE) || HasUpgrade(UPGRADE_POWER_RWIRE))
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
		//sh.setUniform( "despColor", ColorGL(currentDespColor) );
		despFaceShader.setUniform( "toColor", ColorGL(currentDespColor) );
		playerDespShader.setUniform("toColor", ColorGL(currentDespColor));

		despCounter++;
		if( kinRing != NULL && despCounter == maxDespFrames )
		{
			SetKinMode(K_NORMAL);
			if (kinRing->powerRing->IsEmpty())
			{

				SetAction(DEATH);
				rightWire->Reset();
				leftWire->Reset();
				slowCounter = 1;
				frame = 0;

				sess->deathSeq->Reset();
				sess->SetActiveSequence(sess->deathSeq);

				for (int i = 0; i < 3; ++i)
				{
					fairLightningPool[i]->Reset();
					dairLightningPool[i]->Reset();
					uairLightningPool[i]->Reset();
				}
			}
			else
			{
				kinRing->powerRing->mode == PowerRing::NORMAL;
			}
		}
	}
	else if (kinMode == K_SUPER)
	{
		Color superColor(230, 242, 21, 255);
		playerSuperShader.setUniform("u_auraColor", ColorGL(superColor));
		++superFrame;

		if (superFrame == maxSuperFrames)
		{
			if (kinRing != NULL && kinRing->powerRing->IsEmpty())
			{
				SetKinMode(K_DESPERATION);
			}
			else
			{
				SetKinMode(K_NORMAL);
			}
		}
	}
}

void Actor::ReverseVerticalInputsWhenOnCeiling()
{
	if (reversed)
	{
		bool up = currInput.LUp();
		bool down = currInput.LDown();

		if (up) currInput.leftStickPad -= 1;
		if (down) currInput.leftStickPad -= 2;

		if (up) currInput.leftStickPad += 2;
		if (down) currInput.leftStickPad += 1;
	}
}

void Actor::ProcessReceivedHit()
{
	if (receivedHit != NULL)
	{
		assert(action != DEATH);

		hitlagFrames = receivedHit->hitlagFrames;
		hitstunFrames = receivedHit->hitstunFrames;
		setHitstunFrames = hitstunFrames;
		invincibleFrames = receivedHit->hitstunFrames + 20;//25;//receivedHit->damage;

		ActivateEffect(EffectLayer::IN_FRONT, ts_fx_hurtSpack, position, true, 0, 12, 1, facingRight);
		sess->Pause(hitlagFrames);

		ActivateSound(S_HURT);

		if (kinRing != NULL)
			kinRing->powerRing->Drain(receivedHit->damage);

		int dmgRet = 0;//owner->powerRing->Drain(receivedHit->damage);
					   //bool dmgSuccess = owner->powerWheel->Damage( receivedHit->damage );

		if (ground != NULL)
		{
			if (reversed)
				reversed = false;
			ground = NULL;
			SetAction(JUMP);
			frame = 1;
		}

		if (true)
		{
			bool onRail = IsOnRailAction(action) || (grindEdge != NULL && action == JUMPSQUAT);
			if (grindEdge != NULL && !onRail)
			{
				//do something different for grind ball? you don't wanna be hit out at a sensitive moment
				//owner->powerWheel->Damage( receivedHit->damage ); //double damage for now bleh
				//grindSpeed *= .8;

				V2d op = position;

				V2d grindNorm = grindEdge->Normal();

				if (grindNorm.y < 0)
				{
					double extra = 0;
					if (grindNorm.x > 0)
					{
						offsetX = b.rw;
						extra = .1;
					}
					else if (grindNorm.x < 0)
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

					if (!CheckStandUp())
					{
						position = op;

						if (kinRing != NULL)
							kinRing->powerRing->Drain(receivedHit->damage);

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
						edgeQuantity = grindQuantity;
						groundSpeed = grindSpeed;

						hurtBody.isCircle = false;
						hurtBody.rw = 7;
						hurtBody.rh = normalHeight;

						SetAction(GROUNDHITSTUN);
						frame = 0;

						if (receivedHit->knockback > 0)
						{
							groundSpeed = receivedHit->kbDir.x * receivedHit->knockback;
						}
						else
						{
							groundSpeed *= (1 - receivedHit->drainX) * abs(grindNorm.y) + (1 - receivedHit->drainY) * abs(grindNorm.x);
						}

						if (toggleGrindInput)
						{
							currInput.Y = false;
						}

						grindEdge = NULL;
						reversed = false;
					}

				}
				else
				{

					if (grindNorm.x > 0)
					{
						position.x += b.rw + .1;
					}
					else if (grindNorm.x < 0)
					{
						position.x += -b.rw - .1;
					}

					if (grindNorm.y > 0)
						position.y += normalHeight + .1;

					if (!CheckStandUp())
					{
						position = op;

						//owner->powerWheel->Damage( receivedHit->damage );
						if (kinRing != NULL)
							kinRing->powerRing->Drain(receivedHit->damage);

						//apply extra damage since you cant stand up
					}
					else
					{
						//abs( e0n.x ) < wallThresh )

						if (!HasUpgrade(UPGRADE_POWER_GRAV) || (abs(grindNorm.x) >= wallThresh || !hasGravReverse) || grindEdge->IsInvisibleWall())
						{
							framesNotGrinding = 0;
							if (reversed)
							{
								velocity = normalize(grindEdge->v1 - grindEdge->v0) * -grindSpeed;
							}
							else
							{
								velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
							}


							//SetAction( JUMP );
							SetAction(AIRHITSTUN);
							frame = 0;
							if (receivedHit->knockback > 0)
							{
								velocity = receivedHit->knockback * receivedHit->kbDir;
							}
							else
							{
								velocity.x *= (1 - receivedHit->drainX);
								velocity.y *= (1 - receivedHit->drainY);
							}

							if (toggleGrindInput)
							{
								currInput.Y = false;
							}

							hurtBody.isCircle = false;
							hurtBody.rw = 7;
							hurtBody.rh = normalHeight;

							//	frame = 0;
							ground = NULL;
							grindEdge = NULL;
							reversed = false;
						}
						else
						{
							//	velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
							if (grindNorm.x > 0)
							{
								offsetX = b.rw;
							}
							else if (grindNorm.x < 0)
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
							groundSpeed = -grindSpeed;
							edgeQuantity = grindQuantity;
							grindEdge = NULL;
							reversed = true;
							hasGravReverse = false;

							hurtBody.isCircle = false;
							hurtBody.rw = 7;
							hurtBody.rh = normalHeight;

							SetAction(GROUNDHITSTUN);
							frame = 0;

							if (toggleGrindInput)
							{
								currInput.Y = false;
							}

							if (receivedHit->knockback > 0)
							{
								groundSpeed = receivedHit->kbDir.x * receivedHit->knockback;
							}
							else
							{
								groundSpeed *= (1 - receivedHit->drainX) * abs(grindNorm.y) + (1 - receivedHit->drainY) * abs(grindNorm.x);
							}

							frame = 0;
							framesNotGrinding = 0;

							double angle = GroundedAngle();



							ActivateEffect(EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight);
							ActivateSound(S_GRAVREVERSE);
						}
					}
				}

			}
			else if (ground == NULL || onRail)
			{
				ground = NULL;
				grindEdge = NULL;
				bounceEdge = NULL;
				SetAction(AIRHITSTUN);
				frame = 0;
				if (receivedHit->knockback > 0)
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
				SetAction(GROUNDHITSTUN);
				frame = 0;

				if (receivedHit->knockback > 0)
				{
					groundSpeed = receivedHit->kbDir.x * receivedHit->knockback;
				}
				else
				{
					groundSpeed *= (1 - receivedHit->drainX) * abs(currNormal.y) + (1 - receivedHit->drainY) * abs(currNormal.x);
				}

				//dot( receivedHit->kbDir, normalize( ground->v1 - ground->v0 ) ) * receivedHit->knockback;
			}
			bounceEdge = NULL;
		}

		if (dmgRet > 0 && kinMode != K_DESPERATION )
		{
			SetKinMode(K_NORMAL);
			//action = DEATH;
			//frame = 0;
		}




		receivedHit = NULL;
	}
}

void Actor::SetKinMode(Mode m)
{
	kinMode = m;
	switch (m)
	{
	case K_NORMAL:
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
		break;
	case K_SUPER:
		superFrame = 0;
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
		break;
	case K_DESPERATION:
		despCounter = 0;
		SetExpr(KinMask::Expr::Expr_DESP);
		break;
	}
}

void Actor::UpdateDrain()
{
	//change this soon. just so i can get the minimap running
	if ( kinRing != NULL && kinRing->powerRing != NULL && action != DEATH && sess->adventureHUD->IsShown()
		&& (sess->currentZone == NULL || sess->currentZone->zType != Zone::MOMENTA))
	{
		if (sess->drain && kinMode == K_NORMAL
			&& !IsIntroAction(action) && !IsGoalKillAction(action) && !IsExitAction(action) && !IsSequenceAction(action)
			&& sess->activeSequence == NULL)
		{
			drainCounter++;
			if (drainCounter == drainCounterMax)
			{
				int res = kinRing->powerRing->Drain(drainAmount);//powerWheel->Use( 1 );	
																 //cout << "drain by " << drainAmount << endl;
				if (res > 0)
				{
					SetKinMode(K_DESPERATION);
				}
				drainCounter = 0;
			}
		}
	}
}

void Actor::ProcessGravityGrass()
{
	if (ground != NULL && reversed && !HasUpgrade(UPGRADE_POWER_GRAV) && gravityGrassCount == 0)
	{
		//testgrasscount is from the previous frame. if you're not touching anything in your current spot.
		//need to delay a frame so that the player can see themselves not being in the grass
		//before they fall
		if (bounceFlameOn)
			airBounceFrame = 13 * 3;
		//so you dont jump straight up on a nearly vertical edge
		double blah = .5;

		V2d dir(0, 0);

		dir.y = .2;
		V2d along = normalize(ground->v1 - ground->v0);
		V2d trueNormal = along;
		if (groundSpeed > 0)
			trueNormal = -trueNormal;

		trueNormal = normalize(trueNormal + dir);
		velocity = abs(groundSpeed) * trueNormal;

		ground = NULL;
		frame = 1; //so it doesnt use the jump frame when just dropping
		reversed = false;
		framesInAir = 0;
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::UpdateCanStandUp()
{
	canStandUp = true;
	if (b.rh < normalHeight)
	{
		canStandUp = CheckStandUp();
		if (canStandUp)
		{
			b.rh = normalHeight;
			b.offset.y = 0;
		}
	}
}

void Actor::UpdateBounceFlameOn()
{
	justToggledBounce = false;
	if (bounceFlameOn)
	{
		if (toggleBounceInput)
		{
			if (currInput.X && !prevInput.X)
			{
				BounceFlameOff();
				bounceGrounded = false;
				justToggledBounce = true;
			}
		}
		else
		{
			//assert( !toggleBounceInput );
			if (!currInput.X)
			{
				bounceFlameOn = false;
				oldBounceEdge = NULL;
				bounceGrounded = false;
				scorpOn = false;
			}
		}
	}
}

void Actor::HitstunBufferedChangeAction()
{
	if (action == AIRHITSTUN)
	{
		if (hitstunFrames == 0)
		{
			if (!TryDoubleJump())
			{
				if (!AirAttack())
				{
					if (!TryAirDash())
					{
						SetAction(JUMP);
						frame = 1;
						holdJump = false;
					}
				}
			}
		}

	}
	else if (action == GROUNDHITSTUN)
	{
		if (hitstunFrames == 0)
		{
			if (stunBufferedJump)
			{
				if (stunBufferedAttack != Action::Count)
				{
					SetAction(JUMPSQUAT);
					frame = 0;

					if (currInput.LUp())
					{
						bufferedAttack = UAIR;
					}
					else if (currInput.LDown())
					{
						bufferedAttack = DAIR;
					}
					else
					{
						bufferedAttack = FAIR;
					}
				}
				else
				{
					SetAction(JUMPSQUAT);
					frame = 0;
				}

			}
			else if (!TryGroundAttack())
			{
				if (stunBufferedDash)
				{
					SetAction(DASH);
					frame = 0;
				}
				else
				{
					SetAction(LAND);
					frame = 0;
				}
				//if( !)
			}

			//prevInput = ControllerState();
		}

	}
}

void Actor::UpdateWireStates()
{
	if (HasUpgrade(UPGRADE_POWER_LWIRE) && ((action != GRINDBALL && action != GRINDATTACK) || leftWire->state == Wire::RETRACTING))
	{
		leftWire->ClearDebug();
		leftWire->storedPlayerPos = leftWire->storedPlayerPos = leftWire->GetPlayerPos();//leftWire->GetOriginPos(true);
																						 //leftWire->UpdateAnchors2( V2d( 0, 0 ) );
		leftWire->UpdateState(touchEdgeWithLeftWire);
	}

	if (HasUpgrade(UPGRADE_POWER_RWIRE) && ((action != GRINDBALL && action != GRINDATTACK) || rightWire->state == Wire::RETRACTING))
	{
		rightWire->ClearDebug();
		rightWire->storedPlayerPos = leftWire->storedPlayerPos = leftWire->GetPlayerPos();
		rightWire->UpdateState(touchEdgeWithRightWire);
	}
}

void Actor::ProcessBooster()
{
	if (currBooster != NULL && oldBooster == NULL && action != AIRDASH && currBooster->Boost())
	{
		if (ground == NULL && bounceEdge == NULL && grindEdge == NULL)
		{
			SetBoostVelocity();
			//velocity = normalize(velocity) * (length(velocity) + currBooster->strength);
		}
		else if (grindEdge != NULL)
		{
			if (grindSpeed > 0)
			{
				grindSpeed += currBooster->strength;
			}
			else if (grindSpeed < 0)
			{
				grindSpeed -= currBooster->strength;
			}

			if (action == RAILGRIND || action == RAILSLIDE)
			{
				velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
			}
		}
		else if (ground != NULL)
		{
			if (groundSpeed > 0)
			{
				groundSpeed += currBooster->strength;
			}
			else if (groundSpeed < 0)
			{
				groundSpeed -= currBooster->strength;
			}
		}
		//currBooster = NULL;
		//boostUsed = false;
	}
}

void Actor::ProcessBoostGrass()
{
	if (ground != NULL && grassBoosted)
	{
		if (groundSpeed > 0)
		{
			groundSpeed += boostGrassAccel;
		}
		else if (groundSpeed < 0)
		{
			groundSpeed -= boostGrassAccel;
		}
	}
}

void Actor::LimitMaxSpeeds()
{
	double maxReal = maxVelocity + scorpAdditionalCap;
	if (ground == NULL && bounceEdge == NULL && grindEdge == NULL
		&& action != ENTERNEXUS1
		&& action != SPRINGSTUN
		&& action != GLIDE
		&& action != SPRINGSTUNGLIDE
		&& action != SPRINGSTUNBOUNCE
		&& action != SPRINGSTUNAIRBOUNCE
		&& action != SPRINGSTUNTELEPORT)
	{
		if (action != AIRDASH && !(rightWire->state == Wire::PULLING && leftWire->state == Wire::PULLING) && action != GRINDLUNGE && action != RAILDASH && action != GETSHARD)
		{
			velocity = AddGravity(velocity);
		}

		if (velocity.x > maxReal)
			velocity.x = maxReal;
		else if (velocity.x < -maxReal)
			velocity.x = -maxReal;

		if (velocity.y > maxReal)
			velocity.y = maxReal;
		else if (velocity.y < -maxReal)
			velocity.y = -maxReal;
	}
	else
	{
		if (groundSpeed > maxReal)
		{
			groundSpeed = maxReal;
		}
		else if (groundSpeed < -maxReal)
		{
			groundSpeed = -maxReal;
		}
	}
}

void Actor::UpdateBubbles()
{
	for (int i = 0; i < maxBubbles; ++i)
	{
		if (bubbleFramesToLive[i] > 0)
		{
			bubbleFramesToLive[i]--;
			if (fBubblePos != NULL)
			{
				fBubbleFrame[i] = bubbleFramesToLive[i];
			}
		}
	}


	bool bubbleCreated = false;
	oldInBubble = inBubble;
	inBubble = false;

	//int mBubbles = maxBubbles;

	if (HasUpgrade(UPGRADE_POWER_TIME))
	{
		//calculate this all the time so I can give myself infinite airdash
		for (int i = 0; i < maxBubbles; ++i)
		{
			if (bubbleFramesToLive[i] > 0)
			{
				//if( IsQuadTouchingCircle( hurtB
				if (length(position - bubblePos[i]) < bubbleRadiusSize[i])
				{
					inBubble = true;
					break;
				}
			}
		}
	}

	bool isInOwnBubble = inBubble;
	bool isBeingSlowed = IsBeingSlowed();
	if (isBeingSlowed)
	{
		inBubble = true;
	}

	if (toggleTimeSlowInput && !inBubble && oldInBubble)
	{
		currInput.leftShoulder = false;
	}



	if (!inBubble && action == AIRDASH && airDashStall)
	{
		SetAction(JUMP);
		frame = 1;
		holdJump = false;
	}

	int tempSlowCounter = slowCounter;
	if (CanCreateTimeBubble() && HasUpgrade(UPGRADE_POWER_TIME) && currInput.leftShoulder)
	{
		if (!prevInput.leftShoulder && !inBubble)
		{
			if (bubbleFramesToLive[currBubble] == 0)
			{
				inBubble = true;
				//bubbleFramesToLive[currBubble] = bubbleLifeSpan;
				bubbleFramesToLive[currBubble] = bubbleLifeSpan;

				bubbleRadiusSize[currBubble] = GetBubbleRadius();

				if (fBubblePos != NULL)
				{
					fBubbleFrame[currBubble] = bubbleLifeSpan;
					fBubbleRadiusSize[currBubble] = bubbleRadiusSize[currBubble];
				}

				bubblePos[currBubble] = position;

				CollisionBox &bHitbox = bubbleHitboxes[currBubble]->GetCollisionBoxes(0).front();
				bHitbox.globalPosition = position;
				bHitbox.rw = bubbleRadiusSize[currBubble];
				bHitbox.rh = bHitbox.rw;

				if (fBubblePos != NULL)
				{
					fBubblePos[currBubble].x = position.x;
					fBubblePos[currBubble].y = position.y;
				}

				++currBubble;
				if (currBubble == maxBubbles)
				{
					currBubble = 0;
				}

				bubbleCreated = true;
				ActivateSound(S_TIMESLOW);
			}
		}

		if (inBubble)
		{
			if (slowMultiple == baseSlowMultiple)
			{
				//cout << "a" << endl;
				slowCounter = 1;
				slowMultiple = timeSlowStrength;
			}
		}
		else
		{
			//cout << "b" << endl;
			slowCounter = 1;
			slowMultiple = baseSlowMultiple;
		}
	}
	else
	{
		//cout << "C " << endl;
		//slowCounter = 1;
		slowMultiple = baseSlowMultiple;

		//changed when doing editor stuff. before it would always set to one
		if (slowCounter > baseSlowMultiple)
			slowCounter = 1;
	}

	if (isBeingSlowed && !isInOwnBubble)
	{
		if (currInput.leftShoulder)
		{
			slowCounter = 1;
			slowMultiple = baseSlowMultiple;
		}
		else
		{
			slowCounter = tempSlowCounter;
			slowMultiple = timeSlowStrength;
		}
	}
}

void Actor::UpdateRegrindOffCounter()
{
	if (action != RAILGRIND)
	{
		if (regrindOffCount < regrindOffMax)
		{
			regrindOffCount++;
		}
	}
}

void Actor::UpdateKnockbackDirectionAndHitboxType()
{
	currHitboxInfo->hType = HitboxInfo::NORMAL;

	V2d trueNorm;
	V2d along;
	if (ground != NULL)
	{
		GroundedAngleAttack(trueNorm);
		along = V2d(-trueNorm.y, trueNorm.x);
	}

	switch (action)
	{
	case FAIR:
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(1, 0);//HitboxInfo::HitDirection::RIGHT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(-1, 0);
		}
		break;
	case DAIR:
		currHitboxInfo->hDir = V2d(0, 1);
		break;
	case UAIR:
		currHitboxInfo->hDir = V2d(0, -1);
		break;
	case STANDN:
		if ((!reversed && facingRight) || (reversed && !facingRight))
		{
			currHitboxInfo->hDir = along;
		}
		else
		{
			currHitboxInfo->hDir = -along;
		}
		break;
	case STEEPCLIMBATTACK:
		if (reversed)
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = -along;//V2d(-1, 1);//HitboxInfo::HitDirection::DOWNLEFT;
			}
			else
			{
				currHitboxInfo->hDir = along;//V2d(1, 1);//HitboxInfo::HitDirection::DOWNRIGHT;
			}
		}
		else
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = along;//V2d(1, -1);//HitboxInfo::HitDirection::UPRIGHT;
			}
			else
			{
				currHitboxInfo->hDir = -along;//V2d(-1, -1);//HitboxInfo::HitDirection::UPLEFT;
			}
		}
		break;
	case STEEPSLIDEATTACK:
		if (reversed)
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = -along;//V2d(-1, -1);//HitboxInfo::HitDirection::UPLEFT;
			}
			else
			{
				currHitboxInfo->hDir = along;//V2d(1, -1);//HitboxInfo::HitDirection::UPRIGHT;
			}
		}
		else
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = along;//V2d(1, 1);//HitboxInfo::HitDirection::DOWNRIGHT;
			}
			else
			{
				currHitboxInfo->hDir = -along;//V2d(-1, 1);//HitboxInfo::HitDirection::DOWNLEFT;
			}
		}
		break;
	case WALLATTACK:
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(-1, 0);//HitboxInfo::HitDirection::LEFT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(1, 0);//HitboxInfo::HitDirection::RIGHT;
		}
		break;
	case DIAGUPATTACK:
		currHitboxInfo->hType = HitboxInfo::BLUE;
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(1, -1);//HitboxInfo::HitDirection::UPRIGHT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(-1, -1);//HitboxInfo::HitDirection::UPLEFT;
		}
		break;
	case DIAGDOWNATTACK:
		currHitboxInfo->hType = HitboxInfo::BLUE;
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(1, 1);//HitboxInfo::HitDirection::DOWNRIGHT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(-1, 1);//HitboxInfo::HitDirection::DOWNLEFT;
		}
		break;
	default:
		currHitboxInfo->hDir = V2d(0, 0);//HitboxInfo::HitDirection::NONE;
		break;
	}
}

void Actor::UpdatePrePhysics()
{
	ProcessGravityGrass();
	CheckForAirTrigger();
	HandleAirTrigger();

	UpdateDrain();
	
	enemiesKilledLastFrame = enemiesKilledThisFrame;
	enemiesKilledThisFrame = 0;

	//kinRing->powerRing->Drain(1000000);
	//DesperationUpdate();

	ReverseVerticalInputsWhenOnCeiling();

	ActionEnded();

	if (action == DEATH)
	{
		UpdateAction();
		return;
	}

	if( IsIntroAction( action ) || (IsGoalKillAction(action) && action != GOALKILLWAIT) || action == EXIT 
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || IsSequenceAction( action ) || action == EXITBOOST )
	{
		if( action == WAITFORSHIP )
		{ 
			HandleWaitingScoreDisplay();

			if (!owner->scoreDisplay->active && owner->activeSequence == NULL)
			{
				owner->SetActiveSequence(owner->shipExitScene);
				owner->shipExitScene->Reset();
				//owner->SetActiveSequence(owner->shipExitSeq);
				//owner->shipExitSeq->Reset();
			}
		}
		if( action == INTRO && frame == 0 )
		{
			ActivateSound( S_ENTER );
		}
		else if( action == EXIT && frame == 30 )
		{
			ActivateSound( S_EXIT );
		}
		else if (action == EXITWAIT)
		{
			if (frame == 0)
			{
				ActivateEffect(EffectLayer::IN_FRONT, sess->GetTileset("Kin/exitenergy_0_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}
			else if (frame == 6 * 2)
			{
				ActivateEffect(EffectLayer::IN_FRONT, sess->GetTileset("Kin/exitenergy_1_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}
			else if (frame == 6 * 4)
			{
				ActivateEffect(EffectLayer::IN_FRONT, sess->GetTileset("Kin/exitenergy_2_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}	
		}
		else if (action == SEQ_KINFALL)
		{
			velocity = AddGravity(velocity);
		}
		return;
	}
	else if( action == GOALKILLWAIT )
	{
		HandleWaitingScoreDisplay();
		
		if( !owner->scoreDisplay->active )
		{
			
			//SetAction(EXIT);

			if (owner->resType == GameSession::GameResultType::GR_WINCONTINUE)
			{
				SetAction(EXITBOOST);
				owner->Fade(false, 30, Color::Black, true);
			}
			else
			{
				SetAction(EXIT);
				owner->Fade(false, 30, Color::Black, true);
			}

			//position = V2d(owner->goalNodePos.x, owner->goalNodePos.y -80.f);
			frame = 0;
		}
		return;
	}

	if( ground != NULL )
		currNormal = ground->Normal();
	
	ProcessReceivedHit();
	
	UpdateCanStandUp();

	UpdateBounceFlameOn();
	
	HitstunBufferedChangeAction();
	
	ChangeAction();
	
	currHitboxes = NULL;

	UpdateAction();

	UpdateWireStates();
	
	ProcessBooster();

	ProcessBoostGrass();

	LimitMaxSpeeds();

	WireMovement();

	UpdateBubbles();

	UpdateRegrindOffCounter();

	UpdateKnockbackDirectionAndHitboxType();

	ClearPauseBufferedActions();

	touchedJumpGrass = false;
	grassBoosted = false;
	oldVelocity.x = velocity.x;
	oldVelocity.y = velocity.y;
	touchEdgeWithLeftWire = false;
	touchEdgeWithRightWire = false;
	oldAction = action;
	collision = false;
	groundedWallBounce = false;
	oldBooster = currBooster;
	oldModifier = currModifier;
	oldBounceBooster = currBounceBooster;
	highAccuracyHitboxes = true;
	wallNormal.x = 0;
	wallNormal.y = 0;
	hitEnemyDuringPhyiscs = false;
}

double Actor::GetNumSteps()
{
	if (highAccuracyHitboxes)
	{
		return NUM_MAX_STEPS;
	}
	else
	{ 
		return NUM_STEPS;
	}
}



void Actor::StartSeqKinThrown( V2d &pos, V2d &vel )
{
	SetAction(SEQ_KINTHROWN);
	frame = 0;
	ground = NULL;
	position = pos;
	velocity = vel;

}

void Actor::SeqKneel()
{
	SetAction(SEQ_KNEEL);
	frame = 0;
}

void Actor::SeqMeditateMaskOn()
{
	SetAction(SEQ_KNEEL_TO_MEDITATE);
	frame = 0;
}

void Actor::SeqMaskOffMeditate()
{
	SetAction(SEQ_MASKOFF);
	frame = 0;
}

void Actor::SeqGetAirdash()
{
	SetAction(GETPOWER_AIRDASH_FLIP);
	frame = 0;
}

void Actor::TransitionAction(int a)
{
	if (action == -1)
		return;
	if (transitionFuncs[action] != NULL)
	{
		(this->*transitionFuncs[action])(a);
	}
}

void Actor::SetAction( int a )
{
	//standNDashBoost = (action == STANDN && a == DASH && currAttackHit );

	/*if (action == AIRHITSTUN || action == GROUNDHITSTUN )
	{
		stunBufferedJump = false;
		stunBufferedDash = false;
		stunBufferedAttack = Action::Count;
	}*/

	TransitionAction(a);

	action = a;
	frame = 0;

	if (repeatingSound != NULL)
	{
		DeactivateSound(repeatingSound);
		repeatingSound = NULL;
	}

	StartAction();
		
	if( slowCounter > 1 )
	{
		slowCounter = 1;
	}
}

bool Actor::TryClimbBoost( V2d &gNorm)
{
	bool pressB = currInput.B && !prevInput.B;
	if (pressB)
	{
		double sp = 13;//10;//5;//20;//5;//jumpStrength + 1;//28.0;
		double fac = min(((double)framesSinceClimbBoost) / climbBoostLimit, 1.0);

		double extra = 5.0;
		if (currInput.LUp())
		{
			//cout << "boost but better" << endl;
			extra = 5.5;
		}

		extra = extra * fac;

		//cout << "frames: " << framesSinceClimbBoos
		//cout << "fac: " << fac << " extra: " << extra << endl;
		if (gNorm.x > 0)//&& currInput.LLeft() )
		{
			groundSpeed = std::min(groundSpeed - extra, -sp);
		}
		else if (gNorm.x < 0)// && currInput.LRight() )
		{
			groundSpeed = std::max(groundSpeed + extra, sp);
		}

		framesSinceClimbBoost = 0;
		return true;
	}

	return false;
}

void Actor::WireMovement()
{
	Wire *wire = rightWire;

	double accel = .15;//.2;//.15;//.15;
	double triggerSpeed = 17;
	double doubleWirePull = 1.0;//2.0

	doubleWireBoost = false; //just for now temp
	rightWireBoost = false;
	leftWireBoost = false;

	if (framesInAir > 1)
	{
		if (rightWire->state == Wire::PULLING && leftWire->state == Wire::PULLING)
		{
			bool canDoubleWireBoostParticle = false;
			if (framesSinceDoubleWireBoost >= doubleWireBoostTiming)
			{
				canDoubleWireBoostParticle = true;
				framesSinceDoubleWireBoost = 0;
			}

			lastWire = 0;
			V2d rwPos = rightWire->storedPlayerPos;
			V2d lwPos = rightWire->storedPlayerPos;
			V2d newVel1, newVel2;
			V2d wirePoint = wire->anchor.pos;
			if (wire->numPoints > 0)
				wirePoint = wire->points[wire->numPoints - 1].pos;

			V2d wireDir1 = normalize(wirePoint - rwPos);
			V2d tes = normalize(rwPos - wirePoint);
			double temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			V2d old = velocity;
			//velocity = dot( velocity, tes ) * tes;


			V2d future = rwPos + velocity;

			V2d seg = wirePoint - rwPos;
			double segLength = length(seg);
			V2d diff = wirePoint - future;

			//wire->segmentLength -= 10;
			if (length(diff) > wire->segmentLength)
			{
				future += normalize(diff) * (length(diff) - (wire->segmentLength));
				newVel1 = future - rwPos;
			}


			wire = leftWire;

			wirePoint = wire->anchor.pos;
			if (wire->numPoints > 0)
				wirePoint = wire->points[wire->numPoints - 1].pos;

			V2d wireDir2 = normalize(wirePoint - lwPos);
			tes = normalize(lwPos - wirePoint);
			temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			old = velocity;
			//velocity = dot( velocity, tes ) * tes;


			future = lwPos + velocity;

			seg = wirePoint - lwPos;
			segLength = length(seg);
			diff = wirePoint - future;

			//wire->segmentLength -= 10;
			if (length(diff) > wire->segmentLength)
			{
				future += normalize(diff) * (length(diff) - (wire->segmentLength));
				newVel2 = future - lwPos;
			}

			V2d totalVelDir = normalize((newVel1 + newVel2));//normalize( wireDir1 + wireDir2 );
															 //velocity = dot( (newVel1 + newVel2)/ 2.0, totalVelDir ) * normalize( totalVelDir );

			totalVelDir = normalize(wireDir1 + wireDir2);

			double dott = dot(-wireDir1, wireDir2);
			bool opposite = (dott > .95);

			V2d otherDir(totalVelDir.y, -totalVelDir.x);
			double dotvel = dot(velocity, otherDir);
			//correction for momentum



			V2d wdirs = (wireDir1 + wireDir2) / 2.0;


			if (opposite)
				wdirs = wireDir1;




			V2d owdirs(wdirs.y, -wdirs.x);

			doubleWireBoostDir = -owdirs;

			V2d inputDir;
			if (currInput.LLeft())
			{
				inputDir.x = -1;
			}
			else if (currInput.LRight())
			{
				inputDir.x = 1;
			}
			if (currInput.LUp())
			{
				inputDir.y = -1;
			}
			else if (currInput.LDown())
			{
				inputDir.y = 1;
			}

			dotvel = -dot(inputDir, owdirs);
			double v = .5;//.73;//.8;//.5;//1.0;
			if (dotvel > 0)
			{
				//cout << "a" << endl;
				if (canDoubleWireBoostParticle)
				{
					doubleWireBoost = true;
				}
				double q = dot(velocity, normalize(-owdirs));

				if (q >= 0 && q < 40)
				{
					velocity += -owdirs * v / (double)slowMultiple;
				}
				else
				{
					velocity += -owdirs * (v * 2) / (double)slowMultiple;
				}

				doubleWireBoostDir = -doubleWireBoostDir;
			}
			else if (dotvel < 0)
			{
				if (canDoubleWireBoostParticle)
				{
					doubleWireBoost = true;
				}
				//cout << "b" << endl;
				double q = dot(velocity, normalize(owdirs));
				if (q >= 0 && q < 40)
				{
					velocity += owdirs * v / (double)slowMultiple;
				}
				else
				{
					velocity += owdirs * (v * 2) / (double)slowMultiple;
				}

			}
			else
			{
			}

			if (!opposite)
			{
				V2d totalAcc;
				totalAcc.x = totalVelDir.x * doubleWirePull / (double)slowMultiple;
				if (totalVelDir.y < 0)
					totalAcc.y = totalVelDir.y * (doubleWirePull) / (double)slowMultiple;
				//totalAcc.y = totalVelDir.y * ( doubleWirePull + 1 )/ (double)slowMultiple;
				else
					totalAcc.y = totalVelDir.y * (doubleWirePull) / (double)slowMultiple;

				double beforeAlongAmount = dot(velocity, totalVelDir);

				if (beforeAlongAmount >= 20)
				{
					totalAcc *= .5;
				}
				else if (beforeAlongAmount >= 40)
				{
					totalAcc = V2d(0, 0);
				}
				//if( length( velocity ) > 20.0 )
				//{
				//	totalAcc *= .5;
				//}
				//totalVel *= dot( totalVelDir, rightWire->
				velocity += totalAcc;
			}

			/*if( opposite && !currInput.LLeft() && !currInput.LDown() && !currInput.LUp() && !currInput.LRight() )
			{
			double wdirsVel = dot( velocity, wdirs );
			double owdirsVel = dot( velocity, owdirs );
			}*/

			if (opposite)
			{
				dotvel = dot(inputDir, wireDir1);
				double towardsExtra = .5;//.7;//1.0;
				if (dotvel > 0)
				{
					//cout << "a" << endl;\



					velocity += wireDir1 * towardsExtra / (double)slowMultiple;
				}
				else if (dotvel < 0)
				{
					//cout << "b" << endl;
					velocity += wireDir2 * towardsExtra / (double)slowMultiple;
				}
				else
				{
				}
			}
			else
			{
				//totalVelDir
				if (dot(wireDir1, wireDir2) > .99)
					velocity = (velocity + AddGravity(velocity)) / 2.0;
			}
			//removing the max velocity cap now that it doesnt pull you in a straight direction
			//double afterAlongAmount = dot( velocity, totalVelDir );
			//double maxAlong = 100;//45.0;

			//if( afterAlongAmount > maxAlong )
			//{
			//	velocity -= ( afterAlongAmount - maxAlong ) * totalVelDir;
			//}

			//if( length( velocity ) > afterAlongAmount )
			//{
			//	velocity = normalize( velocity ) * afterAlongAmount;
			//}

			//velocity = ( dot( velocity, totalVelDir ) + 4.0 ) * totalVelDir; //+ V2d( 0, gravity / slowMultiple ) ;
			///velocity += totalVelDir * doubleWirePull / (double)slowMultiple;
		}
		else if (rightWire->state == Wire::PULLING)
		{

			//lastWire = 1;
			V2d wPos = rightWire->storedPlayerPos;
			if (position != rightWire->storedPlayerPos)
			{
				//cout << "wPos: " << wPos.x << ", " << wPos.y << endl;
				//cout << "pp: " << position.x << ", " << position.y << endl;
				//assert( 0 && "what" );
			}
			V2d wirePoint = wire->anchor.pos;
			if (wire->numPoints > 0)
				wirePoint = wire->points[wire->numPoints - 1].pos;

			V2d tes = normalize(wPos - wirePoint);
			double temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			double val = dot(velocity, normalize(wirePoint - wPos));
			V2d otherTes;
			//if( val > 0 )
			{
				otherTes = val * normalize(wirePoint - wPos);
			}

			//otherTes = V2d( 0, 0 );

			V2d old = velocity;

			//if( normalize( wirePoint - wPos ).y > 0 )
			//{
			//	V2d g = AddGravity( V2d( 0, 0 ) );
			//	velocity -= g;//V2d( 0, gravity );
			//}


			double speed = dot(velocity, tes);


			if (speed > triggerSpeed)
			{
				//rightWireBoost = true;
				speed += accel / (double)slowMultiple;
			}
			else if (speed < -triggerSpeed)
			{
				//rightWireBoost = true;
				speed -= accel / (double)slowMultiple;
			}
			else
			{


			}

			V2d wireDir = normalize(wirePoint - wPos);
			double otherAccel = .5 / (double)slowMultiple;

			V2d vec45(1, 1);
			vec45 = normalize(vec45);
			double xLimit = vec45.x;

			rightWireBoostDir = -V2d(wireDir.y, -wireDir.x);

			if (abs(wireDir.x) < xLimit)
			{
				if (wireDir.y < 0)
				{
					if (currInput.LLeft())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
					else if (currInput.LRight())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.y > 0)
				{
					if (currInput.LLeft())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LRight())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
				}
			}
			else
			{
				if (wireDir.x > 0)
				{
					if (currInput.LUp())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
					else if (currInput.LDown())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.x < 0)
				{
					if (currInput.LUp())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LDown())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
				}


			}



			if (rightWireBoost && framesSinceRightWireBoost >= singleWireBoostTiming && slowCounter == 1)
			{
				framesSinceRightWireBoost = 0;
			}
			else
			{
				rightWireBoost = false;
			}

			velocity = speed * tes;
			velocity += otherTes;
			

			
			//velocity += otherTes;


			V2d future = wPos + velocity;

			//if (rightWire->anchor.enemy != NULL)
			//{
			//	future += rightWire->anchorVel;
				//velocity += rightWire->anchorVel;
			//}

			V2d seg = wirePoint - wPos;
			double segLength = length(seg);
			V2d diff = wirePoint - future;


			//wire->segmentLength -= 10;
			//cout << "ws: " << wire->segmentLength << ", segg: " << segLength << ", get: " << wire->GetSegmentLength() << endl;
			if (length(diff) > wire->segmentLength)
			{
				double pullVel = length(diff) - wire->segmentLength;
				V2d pullDir = normalize(diff);
				//cout << "lengthdiff: " << length( diff ) << ", : seglength: " << wire->segmentLength << endl;
				future += pullDir * pullVel;
				//cout << "fut: " << length( future - wirePoint ) << ", seglength: " << wire->segmentLength << endl;
				velocity = future - wPos;

				//cout << "pullVel: " << pullVel << endl;

			}

			//velocity = V2d( 0, 0 );

			//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
		}
		else if (leftWire->state == Wire::PULLING)
		{
			//lastWire = 2;
			wire = leftWire;
			V2d wPos = leftWire->storedPlayerPos;
			V2d wirePoint = wire->anchor.pos;
			if (wire->numPoints > 0)
				wirePoint = wire->points[wire->numPoints - 1].pos;

			V2d tes = normalize(wPos - wirePoint);
			double temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			double val = dot(velocity, normalize(wirePoint - wPos));
			V2d otherTes;
			//if( val > 0 )
			{
				otherTes = val * normalize(wirePoint - wPos);
			}



			V2d old = velocity;

			if (normalize(wirePoint - wPos).y > 0)
			{
				//	velocity -= V2d( 0, gravity );
			}

			//double accel = .3;
			double speed = dot(velocity, tes);

			if (speed > triggerSpeed)
			{

				speed += accel / (double)slowMultiple;
			}
			else if (speed < -triggerSpeed)
			{
				speed -= accel / (double)slowMultiple;
			}
			else
			{


			}

			V2d wireDir = normalize(wirePoint - wPos);
			double otherAccel = .5 / (double)slowMultiple;
			V2d vec45(1, 1);
			vec45 = normalize(vec45);
			double xLimit = vec45.x;
			leftWireBoostDir = -V2d(wireDir.y, -wireDir.x);
			if (abs(wireDir.x) < xLimit)
			{
				if (wireDir.y < 0)
				{
					if (currInput.LLeft())
					{
						leftWireBoost = true;
						leftWireBoostDir = -leftWireBoostDir;
						speed -= otherAccel;
					}
					else if (currInput.LRight())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.y > 0)
				{
					if (currInput.LLeft())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LRight())
					{
						leftWireBoost = true;
						speed -= otherAccel;
						leftWireBoostDir = -leftWireBoostDir;
					}
				}
			}
			else
			{
				if (wireDir.x > 0)
				{
					if (currInput.LUp())
					{
						leftWireBoost = true;
						speed -= otherAccel;
						leftWireBoostDir = -leftWireBoostDir;
					}
					else if (currInput.LDown())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.x < 0)
				{
					if (currInput.LUp())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LDown())
					{
						leftWireBoost = true;
						speed -= otherAccel;
						leftWireBoostDir = -leftWireBoostDir;
					}
				}


			}


			if (leftWireBoost && framesSinceLeftWireBoost >= singleWireBoostTiming)
			{
				framesSinceLeftWireBoost = 0;
			}
			else
			{
				leftWireBoost = false;
			}

			velocity = speed * tes;
			velocity += otherTes;
			//velocity += otherTes;


			V2d future = wPos + velocity;

			V2d seg = wirePoint - wPos;
			double segLength = length(seg);
			V2d diff = wirePoint - future;

			//wire->segmentLength -= 10;
			if (length(diff) > wire->segmentLength)
			{
				future += normalize(diff) * (length(diff) - (wire->segmentLength));
				velocity = future - wPos;
			}
		}
	}

	if (ground != NULL)
	{
		lastWire = 0;
	}
}

float Actor::GetSpeedBarPart()
{
	float quant = 0;
	if (speedLevel == 0)
	{
		quant = (float)(currentSpeedBar / level1SpeedThresh);
	}
	else if (speedLevel == 1)
	{
		quant = (float)((currentSpeedBar - level1SpeedThresh) / (level2SpeedThresh - level1SpeedThresh));
	}
	else
	{
		quant = (float)((currentSpeedBar - level2SpeedThresh) / (maxGroundSpeed - level2SpeedThresh));
	}

	return quant;
	/*if (currentSpeedBar >= level2SpeedThresh)
	{
		speedLevel = 2;
	}
	else if (currentSpeedBar >= level1SpeedThresh)
	{
		speedLevel = 1;
	}
	else
	{
		speedLevel = 0;
	}*/
}

void Actor::RailGrindMovement()
{
	V2d along = normalize(grindEdge->v1 - grindEdge->v0);

	Rail *rail = (Rail*)grindEdge->info;

	if (true )//rail->accelerate)
	{
		double ffac = 1.0;
		double startAccel = .3;
		V2d grn = grindEdge->Normal();

		bool ceil = false;

		if (grn.y > 0)
		{
			along = -along;
			grn = -grn;
			ceil = true;
		}

		double accel = 0;
		if (GameSession::IsSteepGround(grn) > 0)
		{
			double fac = GetGravity() *steepSlideGravFactor;//gravity * 2.0 / 3.0;
			accel = dot(V2d(0, fac), along) / slowMultiple;
			//cout << "steep accel: " << accel << endl;
		}
		else if (grn.x != 0)
		{
			accel = dot(V2d(0, slideGravFactor * GetGravity()), along) / slowMultiple;
			//cout << "normal accel: " << accel << endl;
		}

		if (ceil)
			accel = -accel;

		if (accel != 0 && abs(grindSpeed + accel) > abs(grindSpeed))
			grindSpeed += accel;

	}

	velocity = along * grindSpeed;
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

	if (velocity.x > 0)
	{
		facingRight = true;
	}
	else if (velocity.x < 0)
	{
		facingRight = false;
	}

	//cout << "grindSpeed: " << grindSpeed << endl;
}

bool Actor::CanRailSlide()
{
	bool isAttack = IsAttackAction(action);
	bool aerialRailSlideTest = ground == NULL && grindEdge == NULL && bounceEdge == NULL && action != RAILDASH &&
		action != RAILSLIDE && velocity.y >= 0 && action != AIRDASH && !isAttack;

	bool groundRailSlideTest = ground != NULL && grindEdge == NULL && action != DASH && !isAttack;

	if (aerialRailSlideTest || groundRailSlideTest)
	{
		return true;
	}

	return false;
}

bool Actor::CanRailGrind()
{
	if (HasUpgrade(UPGRADE_POWER_GRIND))
	{
		if (!currInput.Y && prevInput.Y)
		{
			framesSinceGrindAttempt = 0;
		}

		if (ground == NULL && grindEdge == NULL && bounceEdge == NULL && action != RAILDASH)
		{
			if (currInput.Y || framesSinceGrindAttempt < maxFramesSinceGrindAttempt)
			{
				return true;
			}
		}
		else
		{
			framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
		}
	}

	return false;
}

bool Actor::IsRailSlideFacingRight()
{
	assert(grindEdge != NULL);

	V2d grindNorm = grindEdge->Normal();
	bool r = grindSpeed > 0;

	if ((action == RAILGRIND || action == RAILSLIDE) && grindNorm.y > 0)
	{
		grindNorm = -grindNorm;
		r = !r;
	}

	return r;
}

void Actor::SetAllUpgrades(BitField &b)
{
	bStartHasUpgradeField.Set(b);
	bHasUpgradeField.Set(b);
}

bool Actor::HasUpgrade(int index)
{
	return bHasUpgradeField.GetBit(index);
}

void Actor::SetUpgrade(int upgrade, bool on)
{
	bHasUpgradeField.SetBit(upgrade, on);
	UpdatePowersMenu();
}

void Actor::SetStartUpgrade(int upgrade, bool on)
{
	bStartHasUpgradeField.SetBit(upgrade, on);
	SetUpgrade(upgrade, on);
}


void Actor::ReverseSteepSlideJump()
{
	SetAction(JUMP);
	frame = 1;

	V2d along = ground->Along();
	V2d norm = ground->Normal();
	V2d alongSpeed = along * -groundSpeed;

	if (norm.x > 0)
	{
		alongSpeed.x += 5;
	}
	else if (norm.x < 0)
	{
		alongSpeed.x -= 5;
	}

	alongSpeed.y -= 8;

	reversed = false;
	ground = NULL;

	velocity = alongSpeed;
}

void Actor::HandleWaitingScoreDisplay()
{
	if (owner->scoreDisplay->waiting)
	{
		ControllerState &unfilteredCurr = owner->GetCurrInputUnfiltered(0);
		ControllerState &unfiltetedPrev = owner->GetPrevInputUnfiltered(0);
		bool a = unfilteredCurr.A && !unfiltetedPrev.A;
		bool x = unfilteredCurr.X && !unfiltetedPrev.X;
		bool b = unfilteredCurr.B && !unfiltetedPrev.B;
		if (a || x)
		{
			SaveFile *currFile = owner->mainMenu->GetCurrentProgress();
			bool levValid = owner->level != NULL && !currFile->IsLevelLastInSector( owner->level );
			if (a && owner->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && levValid)
			{
				owner->resType = GameSession::GameResultType::GR_WINCONTINUE;
			}
			else if (x)
			{
				owner->resType = GameSession::GameResultType::GR_WIN;
			}

			owner->scoreDisplay->Deactivate();
		}
		else if (b)
		{
			if ( owner->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
			{
				SaveFile *currFile = owner->mainMenu->GetCurrentProgress();
				owner->mainMenu->worldMap->CompleteCurrentMap( owner->level, owner->totalFramesBeforeGoal);
				currFile->Save();
			}
			owner->NextFrameRestartLevel();
			return;
		}
	}
}

void Actor::EndLevelWithoutGoal()
{
	if (owner != NULL)
	{
		owner->scoreDisplay->Activate();
		SetAction(Actor::GOALKILLWAIT);
		frame = 0;
	}
	else
	{
		editOwner->EndTestMode();
	}
	
}

void Actor::SetStandInPlacePos(Edge *g, double q,
	bool fr)
{
	facingRight = fr;
	SetGroundedPos(g, q);
	StandInPlace();
}

void Actor::SetGroundedPos(Edge *g, double q, double xoff)
{
	ground = g;
	edgeQuantity = q;

	hasDoubleJump = true;
	hasAirDash = true;
	
	offsetX = xoff;

	V2d norm = ground->Normal();
	if (norm.y <= 0)
	{
		reversed = false;
	}
	else
	{
		reversed = true;
	}
}

void Actor::SetAirPos(V2d &pos, bool fr)
{
	ground = NULL;
	bounceEdge = NULL;
	grindEdge = NULL;
	action = Actor::JUMP;
	frame = 1;
	position = pos;
	facingRight = fr;
}

void Actor::SetGroundedPos(Edge *g, double q)
{
	ground = g;
	edgeQuantity = q;

	hasDoubleJump = true;
	hasAirDash = true;

	V2d norm = ground->Normal();
	if (norm.x > 0)
	{
		offsetX = b.rw;
	}
	else if (norm.x < 0)
	{
		offsetX = -b.rw;
	}
	else
	{
		offsetX = 0;
	}

	if (norm.y <= 0)
	{
		reversed = false;
	}
	else
	{
		reversed = true;
	}
}

void Actor::SetStoryRun(bool fr, double maxAutoRun, Edge * g,
	double q)
{
	SetGroundedPos(g, q);
	SetAutoRun(fr, maxAutoRun);
}

void Actor::SetStoryRun(bool fr, double maxAutoRun, Edge * g,
	double q, Edge *end, double eq )
{
	SetGroundedPos(g, q);
	SetAutoRun(fr, maxAutoRun);
	SetAutoRunStopper(end, eq);
}

void Actor::SetAutoRunStopper(Edge *g, double q)
{
	autoRunStopEdge = g;
	autoRunStopQuant = q;
}

bool Actor::IsAutoRunning()
{
	return action == AUTORUN;
}

void Actor::TurnFace()
{
	assert(ground != NULL);
	SetAction(SEQ_TURNFACE);
	frame = 0;
}

void Actor::StandInPlace()
{
	assert(ground != NULL);
	SetAction(STAND);
	frame = 0;
	groundSpeed = 0;
}

void Actor::WaitInPlace()
{
	sess->SetPlayerInputOn(false);
	SetAction(STAND);
	//SetAction(SEQ_WAIT);
	frame = 0;
	physicsOver = true;
	groundSpeed = 0;
}

void Actor::Wait()
{
	SetAction(SEQ_WAIT);
	frame = 0;
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
	col = false;
	queryMode = "checkwall";
	tempVel = vel;

	//sf::Rect<double> r( 
	Rect<double> r( position.x + tempVel.x + b.offset.x - b.rw, position.y + tempVel.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );


	GetTerrainTree()->Query( this, r );
	
	
	sess->barrierTree->Query(this, r);
	


	if( !col )
	{
		return false;
	}

	bool wally = false;
	if( minContact.edge != NULL && !minContact.edge->IsInvisibleWall() )
	{
		double quant = minContact.edge->GetQuantity( test.position );

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


		if( approxEquals(en.x,1) || approxEquals(en.x,-1) )
		{
			return true;
		}

		
		if( en.y > 0 && abs( en.x ) > .8 )
		{
			//cout << "here" << endl;
			return true;
		}

		if( (one && en.x < 0 && en.y > 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x > 0 )
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
		GetTerrainTree()->Query( this, r );

		
		sess->barrierTree->Query(this, r);

		possibleEdgeCount = 0;

		/*if( checkValid )
			cout << "check valid" << endl;
		else
		{
			cout << "cant stand up" << endl;
		}*/

		/*if( checkValid )
		{
			cout << "canstand" << endl;
		}
		else
		{
			cout << "cannot stand" << endl;
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

	if (action == SPRINGSTUNTELEPORT)
		return false;
	
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
	//double oldQuant = edgeQuantity;
	//double oldOffsetX = offsetX;
	double oldGs = groundSpeed;
	
//	Query( this, owner->testTree, r );

	//cout << "Start resolve" << endl;
	GetTerrainTree()->Query( this, r );

	
	sess->barrierTree->Query(this, r);


	testr = false;

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

	if ( (owner != NULL && owner->hasAnyGrass) || editOwner != NULL )
	{
		queryMode = "grass";
		jumpGrassCount = 0;
		gravityGrassCount = 0;
		bounceGrassCount = 0;
		boostGrassCount = 0;
		sess->grassTree->Query(this, r);
	}

	
	

	//queryMode = "item";
	//owner->itemTree->Query( this, r );
	
	if (owner != NULL)
	{
		queryMode = "envplant";
		owner->envPlantTree->Query(this, r);

		Rect<double> staticItemRect(position.x - 400, position.y - 400, 800, 800);//arbitrary decent sized area around kin
		owner->staticItemTree->Query(NULL, staticItemRect);
	}

	canRailGrind = CanRailGrind();
	canRailSlide = CanRailSlide();


	//if (ground == NULL && bounceEdge == NULL && grindEdge == NULL && (canRailGrind || canRailSlide ) )
	//if( owner != NULL && owner->totalRails > 0 && (canRailGrind || canRailSlide) )
	if( canRailGrind || canRailSlide )
	{
		queryMode = "rail";
		sess->railEdgeTree->Query(this, r);
	}


	currBooster = NULL;
	currBounceBooster = NULL;
	currModifier = NULL;

	double activeExtra = 500;
	sf::Rect<double> activeR = r;
	activeR.left -= activeExtra;
	activeR.top -= activeExtra;
	activeR.width += activeExtra * 2;
	activeR.height += activeExtra * 2;

	queryMode = "activeitem";
	sess->activeItemTree->Query(this, r);//activeR);
	
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
	currWall = NULL;
	if( grindEdge != NULL )
	{
		movement = grindSpeed / (double)slowMultiple / GetNumSteps();
	}
	else if( ground != NULL )
	{
		movement = groundSpeed / (double)slowMultiple / GetNumSteps();
	}
	else
	{
		movementVec = velocity / (double)slowMultiple / GetNumSteps();
	}

	movement = -movement;

		
	while( (ground != NULL && !approxEquals( movement, 0 )))// || ( ground == NULL && length( movementVec ) > 0 ) )
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
				bool unlockedGateJumpOff = false;
				if( e0->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)e0->info;
					if( CanUnlockGate( g ) )
					{
						UnlockGate( g );

						if( e0 == g->edgeA )
						{
							gateTouched = g->edgeB;
						
						}
						else
						{
							gateTouched = g->edgeA;
						}
					}

					if (!g->locked)
					{
						unlockedGateJumpOff = true;
					}
				}

				//gNormal.x = -gNormal.x;
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				V2d nextNorm = e0n;
				double yDist = abs( gNormal.x ) * -groundSpeed;

				//bool jumpOff = (currInput.LUp() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x <= 0 );
				//jumpOff = jumpOff || abs( e0n.x ) >= wallThresh || nextNorm.y >= 0;
				
				bool nextSteep = e0n.y > -steepThresh;
				bool currSteep = gNormal.y > -steepThresh;
				bool movingDown = gNormal.x < 0;
				bool movingUp = gNormal.x > 0;
				bool nextMovingDown = e0n.x < 0;
				bool nextMovingUp = e0n.x > 0;

				//if( !jumpOff )
				//{

				bool jumpOff = false;
				
				if( nextNorm.y >= 0 || abs( e0n.x ) >= wallThresh || e0->IsInvisibleWall())
				{
					jumpOff = true;
				}
				else if( currSteep )
				{
					if( movingUp )
					{
						if( nextMovingUp )
						{
							jumpOff = false;
						}
						else
						{
							jumpOff = true;
						}
					}
					else
					{
						jumpOff = false;
					}
				}
				else
				{
					if( movingUp )
					{
						if( nextSteep && nextMovingDown )
						{
							jumpOff = true;
						}
						else if( nextNorm.x <= 0 && currInput.LUp() && yDist < -slopeLaunchMinSpeed )
						{
							jumpOff = true;
						}
						else
						{
							jumpOff = false;
						}
					}
					else
					{
						if( gNormal.x == 0 && nextMovingDown && nextSteep )
						{
							jumpOff = true;
						}
						else
						{
							jumpOff = false;
						}
					}
				}


				//}
				
				if( jumpOff || unlockedGateJumpOff)
				{	
					reversed = false;
					velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;

					movementVec.y += .1;
					if( movementVec.x <= .1 )
					{
						movementVec.x = .1;
					}

					//cout << "airborne 2" << endl;
					leftGround = true;
					SetAction( JUMP );
					frame = 1;
					
					ground = NULL;

					leftGroundExtra = movementVec;
					return leftGroundExtra;
				}
				else if( nextSteep && nextMovingUp )
				{
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
				else
				{
					ground = next;
					q = length( ground->v1 - ground->v0 );	
				}

				//rightWire->UpdateAnchors( V2d( 0, 0 ) );
				//leftWire->UpdateAnchors( V2d( 0, 0 ) );

				if( false )
				if( nextNorm.y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && /*!currInput.LLeft() &&*/ gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x <= 0 ) )
				{
					cout << "e0n: " << e0n.x << ", " << e0n.y << endl;
					if( e0n.x > 0 && nextSteep )
					{
						cout << "c" << endl;
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

						movementVec.y += .1;
						if( movementVec.x >= -.1 )
						{
							movementVec.x = -.1;
						}
						//leftGroundExtra.y = .01;
						//leftGroundExtra.x = .01;

						cout << "airborne 2" << endl;
						leftGround = true;
						SetAction( JUMP );
						frame = 1;
						//rightWire->UpdateAnchors( V2d( 0, 0 ) );
						//leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;

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

									movementVec.y += .1;
									if( movementVec.x <= .1 )
									{
										movementVec.x = .1;
									}	

									cout << "airborne 1" << endl;
									leftGroundExtra.y = .01;
									leftGroundExtra.x = .01;

									leftGround = true;
									SetAction( JUMP );
									frame = 1;
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									holdJump = false;

									return leftGroundExtra;
								}
								else
								{
									facingRight = true;
									if (!IsGroundAttackAction(action ) )
									{

										SetAction(STEEPSLIDE);
										frame = 0;
									}
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = next;
									q = length( ground->v1 - ground->v0 );	
								}
							}
							else if( e0n.x > 0 )
							{
								facingRight = false;
								SetAction(STEEPCLIMB);
								frame = 0;
								//rightWire->UpdateAnchors( V2d( 0, 0 ) );
								//leftWire->UpdateAnchors( V2d( 0, 0 ) );
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

					movementVec.y += .1;
					if( movementVec.x <= .1 )
					{
						movementVec.x = .1;
					}

					leftGround = true;
					SetAction( JUMP );
					frame = 1;
					ground = NULL;
					holdJump = false;
					leftGroundExtra = movementVec;
					return leftGroundExtra;

					//break;
				}
			}
			else if( transferRight )
			{
				//cout << "transferright" << endl;

				bool jumpOff = false;
				bool unlockedGateJumpOff = false;
				if( e1->edgeType == Edge::CLOSED_GATE )
				{
					Gate * g = (Gate*)e1->info;
					if( CanUnlockGate( g ) )
					{
						UnlockGate( g );

						if( e1 == g->edgeA )
						{
							gateTouched = g->edgeB;
						}
						else
						{
							gateTouched = g->edgeA;
						}
					}

					if (!g->locked)
					{
						unlockedGateJumpOff = true;
					}
				}


				Edge *next = ground->edge1;
				V2d nextNorm = e1n;
				double yDist = abs( gNormal.x ) * -groundSpeed;

				bool nextSteep = e1n.y > -steepThresh;
				bool currSteep = gNormal.y > -steepThresh;
				bool movingDown = gNormal.x > 0;
				bool movingUp = gNormal.x < 0;
				bool nextMovingDown = e1n.x > 0;
				bool nextMovingUp = e1n.x < 0;

				
				
				if( nextNorm.y >= 0 || abs( e1n.x ) >= wallThresh || e1->IsInvisibleWall())
				{
					jumpOff = true;
				}
				else if( currSteep )
				{
					if( movingUp )
					{
						if( nextMovingUp )
						{
							jumpOff = false;
						}
						else
						{
							jumpOff = true;
						}
					}
					else
					{
						jumpOff = false;
					}
				}
				else
				{
					if( movingUp )
					{
						if( nextSteep && nextMovingDown )
						{
							jumpOff = true;
						}
						else if( nextNorm.x >= 0 && currInput.LUp() && yDist < -slopeLaunchMinSpeed )
						{
							jumpOff = true;
						}
						else
						{
							jumpOff = false;
						}
					}
					else
					{
						if( gNormal.x == 0 && nextMovingDown && nextSteep )
						{
							jumpOff = true;
						}
						else
						{
							jumpOff = false;
						}
						//jumpOff = false;
					}
				}

				if( jumpOff || unlockedGateJumpOff)
				{
					reversed = false;
						
					velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;

					movementVec.y += .1;
					if( movementVec.x >= -.1 )
					{
						movementVec.x = -.1;
					}

					leftGround = true;
					SetAction( JUMP );
					frame = 1;
					//rightWire->UpdateAnchors( V2d( 0, 0 ) );
					//leftWire->UpdateAnchors( V2d( 0, 0 ) );
					ground = NULL;
					holdJump = false;
					leftGroundExtra = movementVec;
					//leftGroundExtra.y = .01;
					//leftGroundExtra.x = -.01;
					return leftGroundExtra;
				}
				else if( nextSteep && nextMovingUp )
				{
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
				else
				{
					ground = next;
					q = 0;
				}

				//rightWire->UpdateAnchors( V2d( 0, 0 ) );
				//leftWire->UpdateAnchors( V2d( 0, 0 ) );

				if( false )
				if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && /*!currInput.LRight() && */gNormal.x < 0 && yDist > slopeLaunchMinSpeed && nextNorm.x >= 0 ) )
				{
					cout << "e1n: " << e1n.x << ", " << e1n.y << endl;
					if( e1n.x < 0 && e1n.y > -steepThresh )
					{
						cout << "dd" << endl;
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
					else if( e1n.x > 0 && e1n.y > -steepThresh )
					{
						reversed = false;
						cout << "b" << endl;
						cout << "gNormal: " << gNormal.x << ", " << gNormal.y << endl;
						velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						movementVec = normalize( ground->v1 - ground->v0 ) * extra;

						movementVec.y += .1;
						if( movementVec.x <= .1 )
						{
							movementVec.x = .1;
						}

						leftGround = true;
						SetAction( JUMP );
						frame = 1;
						//rightWire->UpdateAnchors( V2d( 0, 0 ) );
						//leftWire->UpdateAnchors( V2d( 0, 0 ) );
						ground = NULL;
						holdJump = false;
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

									movementVec.y += .1;
									if( movementVec.x >= -.1 )
									{
										movementVec.x = -.1;
									}

									cout << "airborne 3" << endl;
									leftGround = true;
									SetAction( JUMP );
									frame = 1;
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;

									holdJump = false;

									leftGroundExtra = movementVec;
									//leftGroundExtra.y = .01;
									//leftGroundExtra.x = -.01;
									return leftGroundExtra;

								}
								else
								{
									facingRight = false;
									if (!IsGroundAttackAction(action))
									{

										SetAction(STEEPSLIDE);
										frame = 0;
									}
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = next;
									q = 0;
								}
							}
							else if( e1n.x < 0 )
							{
								//cout << "setting to climb??" << endl;
								facingRight = true;
								SetAction(STEEPCLIMB);
								frame = 0;
								//rightWire->UpdateAnchors( V2d( 0, 0 ) );
								//leftWire->UpdateAnchors( V2d( 0, 0 ) );
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
					//remember this is modified
					velocity = normalize( V2d( 0, 0 ) + normalize(ground->v1 - ground->v0 ) ) * -groundSpeed;
						
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;



					//cout  <<  "reverse right:" << movementVec.x << ", " << movementVec.y << endl;
					movementVec.y += .1;//.01;
					if( movementVec.x >= -.1 )
					{
						movementVec.x = -.1;
					}
					cout << "airborne 4: " << velocity.x << ", " << velocity.y << endl;
					SetAction( JUMP );
					frame = 1;
					//rightWire->UpdateAnchors( V2d( 0, 0 ) );
					//leftWire->UpdateAnchors( V2d( 0, 0 ) );
					leftGround = true;
					reversed = false;
					ground = NULL;
					holdJump = false;

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
						V2d eNorm = minContact.normal;
						m = -m;
						//cout << "eNorm: " << eNorm.x << ", " << eNorm.y << ", m: " << m << endl;
						if( eNorm.y > 0 )
						{
							//this could be a problem later hopefully i solved it!
							bool speedTransfer = (eNorm.x < 0 && eNorm.y < steepThresh 
								&& groundSpeed > 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y < steepThresh 
									&& groundSpeed < 0 && groundSpeed <= steepClimbSpeedThresh);

							if( minContact.position.y <= position.y - b.rh + 5 && !speedTransfer )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									//cout << "a" << endl;
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									//cout << "b" << endl;
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

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

							//cout << "d" << endl;
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
						
					}

					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );

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
					//cout << "reverse secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					if( groundSpeed > 0 )
					{
						double yDist = abs( gNormal.x ) * -groundSpeed;
						Edge *next = ground->edge0;
						V2d nextNorm = e0n;
						if( !e0->IsInvisibleWall() && nextNorm.y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x < gNormal.x ) )
						{
							if( e0n.x > 0 && e0n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
							{

								if( e0->edgeType == Edge::CLOSED_GATE )
								{
									cout << "OPENING GATE HERE I THOUGHT THIS WASNT NECESSARY A" << endl;
									Gate *g = (Gate*)e0->info;

									if( CanUnlockGate( g ) )
									{
										UnlockGate( g );

										if( e0 == g->edgeA )
										{
											gateTouched = g->edgeB;
										}
										else
										{
											gateTouched = g->edgeA;
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
						else if( abs( e0n.x ) >= wallThresh && !e0->IsInvisibleWall())
						{
							if( e0->edgeType == Edge::CLOSED_GATE )
							{
								Gate *g = (Gate*)e0->info;

								if( CanUnlockGate( g ) )
								{
									UnlockGate( g );

									if( e0 == g->edgeA )
									{
										gateTouched = g->edgeB;
									}
									else
									{
										gateTouched = g->edgeA;
										
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
						}
					}
					else if( groundSpeed < 0 )
					{
						//cout << "right"<< endl;
						Edge *next = ground->edge1;
						V2d nextNorm = e1n;
						double yDist = abs( gNormal.x ) * -groundSpeed;
						if( !e1->IsInvisibleWall() && nextNorm.y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && nextNorm.x > 0 ) )
						{

							if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh )
							{
								if( e1->edgeType == Edge::CLOSED_GATE )
								{
								//	cout << "OPENING GATE HERE I THOUGHT THIS WASNT NECESSARY B" << endl;
									Gate *g = (Gate*)e1->info;

									if( CanUnlockGate( g ) )
									{
										UnlockGate( g );

										if( e1 == g->edgeA )
										{
											gateTouched = g->edgeB;
										}
										else
										{
											gateTouched = g->edgeA;
											
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
						else if(!e1->IsInvisibleWall() && abs( e1n.x ) >= wallThresh )
						{
							//attemping to fix reverse secret issues on gates
							if( e1->edgeType == Edge::CLOSED_GATE )
							{
								Gate *g = (Gate*)e1->info;

								if( CanUnlockGate( g ) )
								{
									UnlockGate( g );

									if( e1 == g->edgeA )
									{
										gateTouched = g->edgeB;
									}
									else
									{
										gateTouched = g->edgeA;
										
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
						}

					}
					else
					{
						break;
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
						//honestly no idea why I had this in the first place?
						if ( minContact.edge->IsInvisibleWall() &&
							minContact.edge->Normal().y == 1.0 )
						{
							velocity = normalize(ground->v1 - ground->v0) * -groundSpeed;
							velocity.y = 0;
							movementVec = normalize(ground->v1 - ground->v0) * extra;

							leftGround = true;
							reversed = false;
							ground = NULL;
							break;
							//return V2d( 0, 0 );
						}
						else
						{

							V2d eNorm = minContact.normal;//minContact.edge->Normal();
							eNorm = -eNorm;


							//cout<< "blah" << endl;
							if (eNorm.y < 0)
							{
								bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x > 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh);


								if (minContact.position.y <= position.y + minContact.resolution.y - b.rh + b.offset.y + 5 && !speedTransfer)
								{
									double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									double sum = position.x + minContact.resolution.x;
									double diff = sum - minContact.position.x;

									if ((test < -b.rw && !approxEquals(test, -b.rw)) || (test > b.rw && !approxEquals(test, b.rw)))
									{
										//this is for corner border cases
										cout << "REVERSED CORNER BORDER CASE: " << test << endl;

										q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);

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

										q = ground->GetQuantity(minContact.position);

										V2d eNorm = minContact.edge->Normal();
										offsetX = position.x + minContact.resolution.x - minContact.position.x;
										offsetX = -offsetX;
									}
								}
								else
								{
									q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);
									groundSpeed = 0;
									edgeQuantity = q;
									offsetX = -offsetX;
									break;
								}
							}
							else
							{
								if (bounceFlameOn && abs(groundSpeed) > 1)
								{
									storedBounceGroundSpeed = groundSpeed * slowMultiple;
									groundedWallBounce = true;
								}
								q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);
								groundSpeed = 0;
								offsetX = -offsetX;
								edgeQuantity = q;
								break;
							}
						}
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
						
					}
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					
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

	/*if (ground == NULL)
	{
		cout << "check if this is a bug" << endl;
	}*/

	return leftGroundExtra;
}

bool Actor::ExitGrind(bool jump)
{
	
	V2d op = position;

	V2d grindNorm = grindEdge->Normal();

	if (grindNorm.y < 0)
	{
		double extra = 0;
		if (grindNorm.x > 0)
		{
			offsetX = b.rw;
			extra = .1;
		}
		else if (grindNorm.x < 0)
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

		if (!CheckStandUp())
		{
			position = op;
		}
		else
		{
			if (grindSpeed > 0)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}

			framesNotGrinding = 0;
			hasAirDash = true;
			hasGravReverse = true;
			hasDoubleJump = true;
			lastWire = 0;

			if (!jump)
			{
				ground = grindEdge;
				edgeQuantity = grindQuantity;
				SetAction(LAND);
				frame = 0;
				groundSpeed = grindSpeed;

				if (currInput.LRight())
				{
					facingRight = true;
					if (groundSpeed < 0)
					{
						groundSpeed = 0;
					}
				}
				else if (currInput.LLeft())
				{
					facingRight = false;
					if (groundSpeed > 0)
					{
						groundSpeed = 0;
					}
				}
			}
			else
			{
				ground = grindEdge;
				edgeQuantity = grindQuantity;
				groundSpeed = grindSpeed;
				SetAction(JUMPSQUAT);
				frame = 0;
			}


			grindEdge = NULL;
			reversed = false;
		}

	}
	else if (grindNorm.y > 0)
	{
		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}

		if (grindNorm.y > 0)
			position.y += normalHeight + .1;

		if (!CheckStandUp())
		{
			position = op;
		}
		else
		{
			if (!HasUpgrade( UPGRADE_POWER_GRAV ) || (abs(grindNorm.x) >= wallThresh) || jump || grindEdge->IsInvisibleWall())//|| !hasGravReverse ) )
			{
				if (grindSpeed < 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}


				framesNotGrinding = 0;
				if (reversed)
				{
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * -grindSpeed;
				}
				else
				{
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
				}

				SetAction(JUMP);
				frame = 1;
				ground = NULL;
				grindEdge = NULL;
				reversed = false;
			}
			else
			{
				if (grindNorm.x > 0)
				{
					offsetX = b.rw;
				}
				else if (grindNorm.x < 0)
				{
					offsetX = -b.rw;
				}
				else
				{
					offsetX = 0;
				}

				if (grindSpeed < 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}

				hasAirDash = true;
				hasGravReverse = true;
				hasDoubleJump = true;
				lastWire = 0;

				ground = grindEdge;
				groundSpeed = -grindSpeed;
				edgeQuantity = grindQuantity;
				reversed = true;
				grindEdge = NULL;

				SetAction(LAND2);
				framesNotGrinding = 0;
				frame = 0;


				double angle = GroundedAngle();

				ActivateEffect(EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight);
				ActivateSound(S_GRAVREVERSE);
			}
		}
	}
	else
	{
		framesInAir = 0;
		SetAction(DOUBLE);
		frame = 0;
		grindEdge = NULL;
		ground = NULL;

		//TODO: this might glitch grind areas? test it with the range of your get out of grind query
		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}
	}

	return true;
}

bool Actor::TrySlideBrakeOrStand()
{
	if (!(currInput.LLeft() || currInput.LRight()))//&& t )
	{
		if (currInput.LDown() || currInput.LUp())
		{
			action = SLIDE;
			frame = 0;
		}
		else
		{
			double absGroundSpeed = abs(groundSpeed);
			double dSpeed = 22;//GetDashSpeed();
			if (( action == BRAKE && absGroundSpeed < 2) ||
				( action != STAND && action != BRAKE && absGroundSpeed < dSpeed ) )
			{
				SetAction(STAND);
				frame = 0;
			}
			else if( action != BRAKE && action != STAND )
			{
				SetAction(BRAKE);
				frame = 0;
			}
		}
		return true;
	}
	return false;
}

bool Actor::BasicSteepAction(V2d &gNorm)
{
	if (reversed)
	{
		if (-gNorm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{
			if ((groundSpeed > 0 && gNorm.x < 0) || (groundSpeed < 0 && gNorm.x > 0))
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				return true;
			}
			else
			{
				if (gNorm.x < 0)
					facingRight = false;
				else
					facingRight = true;
				SetAction(STEEPSLIDE);
				frame = 0;
				return true;
			}
		}
	}
	else
	{
		if (gNorm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{
			if ((groundSpeed > 0 && gNorm.x < 0) || (groundSpeed < 0 && gNorm.x > 0))
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				return true;
			}
			else
			{
				if (gNorm.x > 0)
					facingRight = true;
				else
					facingRight = false;

				SetAction(STEEPSLIDE);
				frame = 0;
				return true;
			}
		}
	}

	return false;
}

void Actor::SetSprintStartFrame()
{
	if (action == RUN)
	{
		frame = frame / 4;

		if (frame < 3)
		{
			frame = frame + 1;
		}
		else if (frame == 8)
		{
			frame = 7;
		}

		else if (frame == 9)
		{
			frame = 0;
		}
		frame = frame * 4;
	}	
	else
	{
		frame = 0;
	}
}

bool Actor::TrySprintOrRun(V2d &gNorm)
{
	bool isLandingAction = action == LAND || action == LAND2;
	if (currInput.LLeft())
	{
		if ((currInput.LDown() && gNorm.x < 0) || (currInput.LUp() && gNorm.x > 0))
		{
			if (action != SPRINT)
			{
				SetAction(SPRINT);
				SetSprintStartFrame();
			}
		}
		else
		{
			if (action != RUN)
			{
				SetAction(RUN);
				frame = 0;
			}
		}

		if (facingRight )
		{
			if (!isLandingAction)
			{
				groundSpeed = 0;
			}
			facingRight = false;
		}
		return true;
	}
	else if (currInput.LRight())
	{
		if ((currInput.LDown() && gNorm.x > 0) || (currInput.LUp() && gNorm.x < 0))
		{
			if (action != SPRINT)
			{
				SetAction(SPRINT);
				SetSprintStartFrame();
			}
		}
		else
		{
			if (action != RUN)
			{
				SetAction(RUN);
				frame = 0;
			}
		}

		if (!facingRight)
		{
			if (!isLandingAction)
			{
				groundSpeed = 0;
			}

			facingRight = true;
		}

		return true;
	}

	return false;
}

double Actor::GetDashSpeed()
{
	
	switch( speedLevel )
	{
	case 0:
		return dashSpeed0;
		break;
	case 1:
		return dashSpeed1;
		break;
	case 2:
	{
		double sbp = GetSpeedBarPart();
		if (sbp > .8)
			sbp = 1.0;
		return dashSpeed2 + 4.0 * sbp;
		break;
	}
		
	}
}

bool Actor::TryGrind()
{
	if (HasUpgrade( UPGRADE_POWER_GRIND ) && currInput.Y && !prevInput.Y)
	{
		SetActionGrind();
		BounceFlameOff();
		return true;
	}

	return false;
}

bool Actor::TryJumpSquat()
{
	if (currInput.A && !prevInput.A)
	{
		SetAction(JUMPSQUAT);
		frame = 0;
		return true;
	}

	return false;
}

bool Actor::TryDash()
{
	bool landAction = action == LAND || action == LAND2;
	if (currInput.B && (!prevInput.B || landAction ))
	{
		SetAction(DASH);
		frame = 0;
		return true;
	}
	
	return false;
}

bool Actor::BasicGroundAction( V2d &gNorm)
{
	CheckBounceFlame();

	//button-based inputs

	if (TryGrind()) return true;

	if (TryJumpSquat()) return true;

	if (TryGroundAttack()) return true;
	
	if (TryDash()) return true;

	//control only

	if (BasicSteepAction( gNorm )) return true;

	if (TrySprintOrRun( gNorm )) return true;

	if (TrySlideBrakeOrStand()) return true;


	assert(0);
	return false;
}

bool Actor::BasicAirAction()
{
	CheckBounceFlame();

	if (TryDoubleJump()) return true;

	if (TryAirDash()) return true;

	//if (TryGlide()) return true;

	if (TryWallJump()) return true;

	if( AirAttack()) return true;

	return false;
}

bool Actor::GlideAction()
{
	CheckBounceFlame();

	if (TryDoubleJump()) return true;

	if (TryAirDash()) return true;

	//if (TryGlide()) return true;

	if (TryWallJump()) return true;

	if (AirAttack())
	{
		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}
		return true;
	}

	return false;
}

bool Actor::BasicAirAttackAction()
{
	CheckBounceFlame();

	if (currAttackHit)
	{
		if (TryAirDash()) return true;

		if (TryDoubleJump()) return true;

		if (AirAttack()) return true;
	}
	
	return false;
}

void Actor::CheckBounceFlame()
{
	if (HasUpgrade( UPGRADE_POWER_BOUNCE ) )
	{
		if (currInput.X && !bounceFlameOn && !justToggledBounce)
		{
			BounceFlameOn();
			oldBounceEdge = NULL;
		}
		else if (!currInput.X && bounceFlameOn)
		{
			BounceFlameOff();
		}
	}
}

bool Actor::TryWallJump()
{
	bool wj = false;
	if (CheckWall(false))
	{
		if (!currInput.LDown() && currInput.LRight() && !prevInput.LRight())
		{
			SetAction(WALLJUMP);
			frame = 0;
			facingRight = true;
			wj = true;
		}
	}

	if (CheckWall(true))
	{
		if (!currInput.LDown() && currInput.LLeft() && !prevInput.LLeft())
		{
			SetAction(WALLJUMP);
			frame = 0;
			facingRight = false;
			wj = true;
		}
	}

	if (wj)
	{
		
	}
	

	return false;
}

bool Actor::EnemyIsFar(V2d &enemyPos)
{
	double len = length(position - enemyPos);
	bool isFar;
	isFar = (len > MAX_VELOCITY * 2);

	if (!isFar)
		return false;
	else
	{
		ComboObject *curr = activeComboObjList;
		while (curr != NULL)
		{
			len = length(curr->GetComboPos() - enemyPos); //doesnt account for origin of shape
			isFar = (len > MAX_VELOCITY * 2);
			if (!isFar)
				return false;
			curr = curr->nextComboObj;
		}
	}

	return true;
}

ComboObject * Actor::IntersectMyComboHitboxes(Enemy *e, CollisionBody *cb,
	int cbFrame)
{
	if (cb == NULL || activeComboObjList == NULL)
		return NULL;

	ComboObject *curr = activeComboObjList;
	while (curr != NULL)
	{
		if (e != curr->enemy)
		{
			if (curr->enemyHitBody.Intersects(curr->enemyHitboxFrame, cb, cbFrame))
			{
				return curr;
			}
		}
		curr = curr->nextComboObj;
	}

	return NULL;
}

Wire * Actor::IntersectMyWireHitboxes(Enemy *e, CollisionBody *cb,
	int cbFrame)
{
	if (cb == NULL)
		return NULL;

	bool canHitRight = e->CanBeHitByWireTip(true);
	bool canHitLeft = e->CanBeHitByWireTip(false);

	CollisionBox *rightBox = NULL;
	CollisionBox *leftBox = NULL;
	if (rightWire != NULL && canHitRight )
	{
		rightBox = rightWire->GetTipHitbox();
	}
	if (leftWire != NULL && canHitLeft)
	{
		leftBox = leftWire->GetTipHitbox();
	}

	if (rightBox != NULL)
	{
		if (cb->Intersects(cbFrame, rightBox))
		{
			return rightWire;
		}
	}
	if (leftBox != NULL)
	{
		if (cb->Intersects(cbFrame, leftBox))
		{
			return leftWire;
		}
	}

	return false;
}

bool Actor::IntersectMyHurtboxes(CollisionBox &cb)
{
	if (IsIntangible())
		return false;

	return cb.Intersects(hurtBody);
}

bool Actor::IntersectMyHurtboxes(CollisionBody *cb, int cbFrame )
{
	if (IsIntangible())
		return false;

	//just for the demo. more detailed hurtboxes later
	if (cb == NULL)
		return false;

	return cb->Intersects( cbFrame, &hurtBody); 
	//if ( cb == NULL || currHurtboxes == NULL)
	//	return false;

	//return currHurtboxes->Intersects(currHurtboxFrame, cb, cbFrame);
}

bool Actor::IntersectMyHitboxes(CollisionBody *cb,
	int cbFrame)
{
	if (cb == NULL || currHitboxes == NULL )
		return false;

	return currHitboxes->Intersects(currHitboxFrame, cb, cbFrame);
}

bool Actor::IntersectMySlowboxes(CollisionBody *cb, int cbFrame )
{
	if (cb == NULL )
		return false;

	CollisionBody *bubbleBody;
	for (int i = 0; i < maxBubbles; ++i)
	{
		bubbleBody = GetBubbleHitbox(i);
		if (bubbleBody != NULL)
		{
			if (bubbleBody->Intersects(0, cb, cbFrame))
			{
				return true;
			}
		}
	}
	return false;
}

double Actor::GetAirDashSpeed()
{
	//return 45;
	switch( speedLevel )
	{
	case 0:
		return airDashSpeed0;
		break;
	case 1:
		return airDashSpeed1;
		break;
	case 2:
		return airDashSpeed2;
		break;
	}

	
}

int Actor::GetBubbleRadius()
{
	switch( speedLevel )
	{
	case 0:
		return bubbleRadius0;
		break;
	case 1:
		return bubbleRadius1;
		break;
	case 2:
		return bubbleRadius2;
		break;
	}
}

double Actor::GetFullSprintAccel( bool downSlope, sf::Vector2<double> &gNorm )
{
	double extraSprintAccel;

	extraSprintAccel = abs(dot( gNorm, V2d( 1, 0 ) ));


	if( downSlope )
	{
	}
	else
	{
		extraSprintAccel = min( .3, extraSprintAccel );
	}
	extraSprintAccel *= .09;

	return sprintAccel + extraSprintAccel;
}

double Actor::GetMinRailGrindSpeed()
{
	return minRailGrindSpeed[speedLevel];
}

//eventually need to change resolve physics so that the player can't miss going by enemies. i understand the need now
//for universal substeps. guess box2d makes more sense now doesn't it XD


void Actor::GroundExtraAccel()
{
	if( bounceFlameOn )
	{
		double bounceFlameAccel = GetBounceFlameAccel();
		if( groundSpeed > 0 )
			groundSpeed += bounceFlameAccel / slowMultiple;
		else if( groundSpeed < 0 )
			groundSpeed -= bounceFlameAccel / slowMultiple;
	}
	else if( currInput.B )
	{
		/*if( groundSpeed > 0 )
			groundSpeed += holdDashAccel / slowMultiple;
		else if( groundSpeed < 0 )
			groundSpeed -= holdDashAccel / slowMultiple;*/
	}
}


void Actor::UpdateWirePhysics()
{
	if (rightWire != NULL)
		rightWire->UpdateChargesPhysics();
	if (leftWire != NULL)
		leftWire->UpdateChargesPhysics();

	if (rightWire != NULL)
	{
		rightWire->UpdateEnemyAnchor();
	}

	if (leftWire != NULL)
	{
		leftWire->UpdateEnemyAnchor();
	}
}

void Actor::UpdateGrindPhysics(double movement)
{
	Edge *e0 = grindEdge->edge0;
	Edge *e1 = grindEdge->edge1;
	V2d e0n = e0->Normal();
	V2d e1n = e1->Normal();

	double q = grindQuantity;
	double hitBorderSpeed = GetDashSpeed() / 2;

	while (!approxEquals(movement, 0))
	{
		//cout << "movement: " << movement << endl;
		double gLen = length(grindEdge->v1 - grindEdge->v0);
		if (movement > 0)
		{
			double extra = q + movement - gLen;
			V2d gPoint = grindEdge->GetPosition(q + movement);
			if (owner != NULL && !owner->IsWithinCurrentBounds(gPoint))
			{
				grindSpeed = max(-grindSpeed, -hitBorderSpeed);
				//grindSpeed = -grindSpeed;
				return;
			}

			if (extra > 0)
			{
				movement -= gLen - q;


				V2d v0 = grindEdge->v0;
				V2d v1 = grindEdge->v1;


				if (e1->edgeType == Edge::CLOSED_GATE)
				{
					Gate *gg = (Gate*)e1->info;
					if (gg->gState == Gate::SOFT || gg->gState == Gate::SOFTEN)
					{
						if (CanUnlockGate(gg))
						{
							//cout << "unlock gate" << endl;
							UnlockGate(gg);

							if (e1 == gg->edgeA)
							{
								gateTouched = gg->edgeB;

							}
							else
							{
								gateTouched = gg->edgeA;

							}

							e1 = grindEdge->edge1;
						}
					}
				}
				grindEdge = e1;

				if (GameSession::IsWall(grindEdge->Normal()) == -1)
				{
					if (HasUpgrade( UPGRADE_POWER_GRAV ) || grindEdge->Normal().y < 0)
					{
						hasDoubleJump = true;
						hasAirDash = true;
						hasGravReverse = true;
						lastWire = 0;
					}
				}
				q = 0;
			}
			else
			{
				q += movement;
				movement = 0;
			}
		}
		else if (movement < 0)
		{
			double extra = q + movement;

			V2d gPoint = grindEdge->GetPosition(q + movement);
			if (owner != NULL && !owner->IsWithinCurrentBounds(gPoint))
			{
				grindSpeed = min(-grindSpeed, hitBorderSpeed);

				return;
			}

			if (extra < 0)
			{
				movement -= movement - extra;

				V2d v0 = grindEdge->v0;
				sf::Rect<double> r(v0.x - 1, v0.y - 1, 2, 2);


				//CheckStandUp();
				//if( )

				if (e0->edgeType == Edge::CLOSED_GATE)
				{
					Gate *gg = (Gate*)e0->info;
					if (gg->gState == Gate::SOFT || gg->gState == Gate::SOFTEN)
					{
						if (CanUnlockGate(gg))
						{
							//cout << "unlock gate" << endl;
							UnlockGate(gg);

							if (e0 == gg->edgeA)
							{
								gateTouched = gg->edgeB;
							}
							else
							{
								gateTouched = gg->edgeA;

							}

							e0 = grindEdge->edge0;
						}
					}
				}
				grindEdge = e0;
				q = length(grindEdge->v1 - grindEdge->v0);

				if (GameSession::IsWall(grindEdge->Normal()) == -1)
				{
					if (HasUpgrade(UPGRADE_POWER_GRAV) || grindEdge->Normal().y < 0)
					{
						hasDoubleJump = true;
						hasAirDash = true;
						hasGravReverse = true;
						lastWire = 0;
					}
				}
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
}

void Actor::HandleBounceGrass()
{
	if (minContact.normal.y < 0)
	{
		velocity.y = -25;
	}
	else if (minContact.normal.y > 0)
	{
		velocity.y = 25;
	}

	hasDoubleJump = true;
	hasAirDash = true;

	if (action == AIRDASH || IsSpringAction(action))
	{
		SetAction(JUMP);
		frame = 1;
	}
}

bool Actor::UpdateGrindRailPhysics(double movement)
{
	Edge *e0 = grindEdge->edge0;
	Edge *e1 = grindEdge->edge1;
	//V2d e0n = e0->Normal();
	//V2d e1n = e1->Normal();

	double q = grindQuantity;
	while (!approxEquals(movement, 0))
	{
		//cout << "movement: " << movement << endl;
		double gLen = length(grindEdge->v1 - grindEdge->v0);
		if (movement > 0)
		{
			double extra = q + movement - gLen;
			if (extra > 0)
			{
				movement -= gLen - q;



				V2d v0 = grindEdge->v0;
				V2d v1 = grindEdge->v1;

				if (e1 != NULL)
				{
					grindEdge = e1;
					q = 0;
				}
				else
				{
					SetAction(JUMP);
					frame = 1;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesNotGrinding = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

					PhysicsResponse();
					return true;
				}
			}
			else
			{
				q += movement;
				V2d posOld = position;
				bool col = ResolvePhysics(normalize(grindEdge->v1 - grindEdge->v0) * movement);
				if (col)
				{
					position = posOld;
					SetAction(JUMP);
					frame = 1;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesNotGrinding = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
				}
				movement = 0;
			}
		}
		else if (movement < 0)
		{
			double extra = q + movement;
			if (extra < 0)
			{
				movement -= movement - extra;

				V2d v0 = grindEdge->v0;
				sf::Rect<double> r(v0.x - 1, v0.y - 1, 2, 2);

				if (e0 != NULL)
				{
					grindEdge = e0;
					q = length(grindEdge->v1 - grindEdge->v0);
				}
				else
				{
					SetAction(JUMP);
					frame = 1;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesSinceGrindAttempt = 0;
					framesNotGrinding = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

					PhysicsResponse();
					return true;

				}
			}
			else
			{
				q += movement;
				V2d posOld = position;
				bool col = ResolvePhysics(normalize(grindEdge->v1 - grindEdge->v0) * movement);
				if (col)
				{
					position = posOld;
					SetAction(JUMP);
					frame = 1;
					framesNotGrinding = 0;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
				}
				movement = 0;
			}
		}
	}

	if (action == RAILGRIND || action == RAILSLIDE)
	{
		grindQuantity = q;

		PhysicsResponse();
		return true;
	}

	return false;
}

bool Actor::TryUnlockOnTransfer( Edge * e)
{
	if (e->edgeType == Edge::CLOSED_GATE)
	{
		Gate * g = (Gate*)e->info;

		if (CanUnlockGate(g))
		{
			UnlockGate(g);

			if (e == g->edgeA)
			{
				gateTouched = g->edgeB;
			}
			else
			{
				gateTouched = g->edgeA;

			}

			return true;
		}
	}

	return false;
}

void Actor::LeaveGroundTransfer(bool right, V2d leaveExtra )
{
	velocity = normalize(ground->v1 - ground->v0) * groundSpeed + leaveExtra;

	movementVec = normalize(ground->v1 - ground->v0) * extra;

	movementVec.y -= .01;
	if (right)
	{
		if (movementVec.x <= .01)
		{
			movementVec.x = .01;
		}
	}
	else
	{
		if (movementVec.x >= -.01)
		{
			movementVec.x = -.01;
		}
	}
	

	leftGround = true;
	ground = NULL;
	SetAction(JUMP);
	frame = 1;
	holdJump = false;
}

bool Actor::UpdateAutoRunPhysics( double q, double m )
{
	if (IsAutoRunning())
	{
		if (autoRunStopEdge != NULL && autoRunStopEdge == ground)
		{
			if (facingRight)
			{
				if (autoRunStopQuant <= q && autoRunStopQuant >= q - m)
				{
					WaitInPlace();
					return true;
				}
			}
			else
			{
				if (autoRunStopQuant >= q && autoRunStopQuant <= q - m)
				{
					WaitInPlace();
					return true;
				}
			}
		}
	}

	return false;
}

void Actor::UpdatePhysics()
{
	if( IsIntroAction(action) || IsGoalKillAction(action) || action == EXIT
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || action == EXITBOOST
		|| action == DEATH || hitEnemyDuringPhyiscs)
		return;	
	
	UpdateWirePhysics();

	double temp_groundSpeed = groundSpeed / slowMultiple;
	V2d temp_velocity = velocity / (double)slowMultiple;
	double temp_grindSpeed = grindSpeed / slowMultiple;

	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	movementVec = V2d(0, 0);
	V2d lastExtra( 100000, 100000 );
	
	if( grindEdge != NULL )
	{
		if( reversed )
		{
			reversed = false;
			grindSpeed = -grindSpeed;
		}
		movement = temp_grindSpeed / GetNumSteps();
	}
	else if( ground != NULL )
	{

		movement = temp_groundSpeed / GetNumSteps();
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
		movementVec = temp_velocity / GetNumSteps();
	}

	if( physicsOver )
	{
		//still need to do hitbox/hurtbox responses if hes not moving
		return;
	}
	
	if ( grindEdge == NULL && movement == 0 && movementVec.x == 0 && movementVec.y == 0)
	{
		ResolvePhysics(V2d(0, 0));
		PhysicsResponse();
		return;
	}

	if( grindEdge != NULL && ( action == GRINDBALL || action == GRINDATTACK ))
	{
		UpdateGrindPhysics(movement);
		return;
	}
	else if (grindEdge != NULL && ( action == RAILGRIND || action == RAILSLIDE ))
	{
		if (UpdateGrindRailPhysics(movement))
		{
			return;
		}
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
		trueFramesInAir = framesInAir;
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
			else if( movement < 0 )
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}
			else
			{
				
			}


			extra = 0;
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

			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;

			//on reverse doesnt need to fly up off of edges
			if( transferLeft )
			{
				TryUnlockOnTransfer(e0);

				Edge *next = ground->edge0;
				double yDist = abs( gNormal.x ) * groundSpeed;
				if( next->Normal().y < 0 && abs( e0n.x ) < wallThresh 
					&& !(currInput.LUp() && gNormal.x > 0 
						&& yDist < -slopeLaunchMinSpeed && next->Normal().x <= 0 ) )
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
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
					else if( gNormal.x > 0 && gNormal.y > -steepThresh )
					{
						ground = next;
						q = length( ground->v1 - ground->v0 );	
					}
					else
					{
						if( e0n.y > -steepThresh )
						{
							if( e0n.x < 0 )
							{
								if( gNormal.x >= -slopeTooSteepLaunchLimitX )
								{
									LeaveGroundTransfer(false);
								}
								else
								{
									facingRight = false;
									if (!IsGroundAttackAction(action))
									{

										SetAction(STEEPSLIDE);
										frame = 0;
									}
									ground = next;
									q = length( ground->v1 - ground->v0 );	
								}
							}
							else if( e0n.x > 0 )
							{
								facingRight = false;
								SetAction(STEEPCLIMB);
								frame = 0;
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
					LeaveGroundTransfer(false);
				}
			}
			else if( transferRight )
			{
				TryUnlockOnTransfer(e1);

				double yDist = abs( gNormal.x ) * groundSpeed;
				Edge *next = ground->edge1;
				V2d nextNorm = next->Normal();
				if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh 
					&& !(currInput.LUp() && gNormal.x < 0 
					&& yDist > slopeLaunchMinSpeed && nextNorm.x >= 0 ) )
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
						}
					}
					else if( gNormal.x < 0 && gNormal.y > -steepThresh )
					{
						ground = next;
						q = 0;
					}
					else
					{

						if( e1n.y > -steepThresh)
						{
							if( e1n.x > 0 )
							{
								if( gNormal.x <= slopeTooSteepLaunchLimitX )
								{
									LeaveGroundTransfer(true);
								}
								else
								{
									facingRight = true;
									if (!IsGroundAttackAction(action))
									{

										SetAction(STEEPSLIDE);
										frame = 0;
									}
									ground = next;
									q = 0;
								}
							}
							else if( e1n.x < 0 )
							{
								facingRight = true;
								SetAction(STEEPCLIMB);
								frame = 0;
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
					LeaveGroundTransfer(true);
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
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
					/*if ( m > 0 && offsetX + m < 0)
					{
						m = -offsetX;
						offsetX = 0;
					}
					else if( m < 0 && offsetX + m > 0 )
					{
						m = -offsetX;
						offsetX = 0;
					}
					else
					{
						offsetX += m;
					}				*/	
				}

				if(!approxEquals( m, 0 ) )
				{
					V2d oldPos = position;
					bool hit = ResolvePhysics( V2d( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) 
						|| ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.edge->Normal();

						if( eNorm.y < 0 )
						{
							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);
							if( minContact.position.y >= position.y + b.rh - 5 && !speedTransfer)
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

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
					}

					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
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

						//cout << "checking for airborne" << endl;
						
						
						double yDist = abs( gNormal.x ) * groundSpeed;
						if( gNormal.x < 0 
							&& e1n.y < 0 
							&& abs( e1n.x ) < wallThresh 
							&& e1n.x >= 0 
							&& yDist > slopeLaunchMinSpeed
							&& currInput.LUp() )
						{
							LeaveGroundTransfer(true, V2d(0, -gravity * 2));
							break;
						}
						else
						{
							if( gNormal.x < 0 && gNormal.y > -steepThresh && e1n.x >= 0
								&& abs( e1n.x ) < wallThresh && groundSpeed > 5 )
							{
								LeaveGroundTransfer(true, V2d(0, -gravity * 2));
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
						double yDist = abs( gNormal.x ) * -groundSpeed;
						if( gNormal.x > 0 
							&& e0n.y < 0 
							&& abs( e0n.x ) < wallThresh 
							&& e0n.x <= 0 
							&& yDist > slopeLaunchMinSpeed 
							&& currInput.LUp() )
						{
							LeaveGroundTransfer(false, V2d(0, -gravity * 2));
							break;
						}
						else
						{
							if( gNormal.x > 0 && gNormal.y > -steepThresh && e0n.x <= 0
								&& abs( e0n.x ) < wallThresh && groundSpeed < -5 )
							{
								LeaveGroundTransfer(false, V2d(0, -gravity * 2));
								break;
							}
							else
							{
								q = 0;
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
				
				if (UpdateAutoRunPhysics(q, m))
					return;

				if( approxEquals( m, 0 ) )
				{
					if( groundSpeed > 0 )
					{
						
						Edge *next = ground->edge1;
						double yDist = abs( gNormal.x ) * groundSpeed;
						if( next->Normal().y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && next->Normal().x >= 0 ) )
						{
							if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
							{
								if (TryUnlockOnTransfer(e1))
									break;
								groundSpeed = 0;
								break;
							}
							else
							{
								break;
							}
					
						}
						else if( abs( e1n.x ) >= wallThresh )
						{
							
							if (TryUnlockOnTransfer(e1))
							{
								break;
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
										UnlockGate( g );

										if( e0 == g->edgeA )
										{
											gateTouched = g->edgeB;
										}
										else
										{
											gateTouched = g->edgeA;
											
										}

										break;
									}

								}

								groundSpeed = 0;
								break;
							}
							else
							{
								//cout << "possible bug. solved secret??" << endl;
								//ground = next;
								//q = length( ground->v1 - ground->v0 );	
								break;
							}
						}
						else if( abs( e0n.x ) >= wallThresh )
						{
							if (TryUnlockOnTransfer(e0))
							{
								break;
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
						}
					}
					else
					{
						break; //recently added for one last bug removal
					}
				}

				//only want this to fire if secret doesn't happen
				else//if( !approxEquals( m, 0 ) )
				{	
					bool down = true;
					V2d oldPos = position;
					
					V2d resMove = normalize( ground->v1 - ground->v0 ) * m;
					bool hit = ResolvePhysics( resMove );


					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) 
						|| ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						if( down)
						{
							V2d eNorm = minContact.normal;
							
							if( minContact.position == minContact.edge->v0 )
							{

							}
							//minContact.edge->Normal();
							if( minContact.position.y > position.y + b.offset.y + b.rh - 5 
								&& minContact.edge->Normal().y >= 0 )
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
								bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh 
									&& groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh 
										&& groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);

								if( minContact.position.y >= position.y + minContact.resolution.y + b.rh + b.offset.y - 5  
									&& !speedTransfer)
								{
									double test = position.x + minContact.resolution.x - minContact.position.x;
									if( (test < -b.rw && !approxEquals(test,-b.rw))
										|| (test > b.rw && !approxEquals(test,b.rw)) )
									{
										//corner border case. hope it doesn't cause problems
										cout << "CORNER BORDER CASE: " << test << endl;
										q = ground->GetQuantity( ground->GetPosition( q ) + minContact.resolution);
										groundSpeed = 0;
										edgeQuantity = q;
										break;
									}
									else
									{	
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );

										V2d eNorm = minContact.normal;

										//hopefully this doesn't cause any bugs. 
										//if it does i know exactly where to find it

										//CHANGED OFFSET
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
									q = ground->GetQuantity( ground->GetPosition( q ) + minContact.resolution);
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
								if( testVel.y < -offSlopeByWallThresh && eNorm.y == 0 && !bounceFlameOn && !minContact.edge->IsInvisibleWall() )
								{
									assert( abs(eNorm.x ) > wallThresh );
							//		cout << "testVel: " << testVel.x << ", " << testVel.y << endl;
									velocity = testVel;
						
									movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
									leftGround = true;
									ground = NULL;
									SetAction( JUMP );
									holdJump = false;
									frame = 1;
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
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


									q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);

									groundSpeed = 0;
									edgeQuantity = q;
								}
							}
						}
						else
						{
							q = ground->GetQuantity( ground->GetPosition( q ) + minContact.resolution);
							groundSpeed = 0;
							edgeQuantity = q;
							break;
						}
						
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
					}

					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
				}	
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
					currWall = minContact.edge;
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

				//might still need some more work
				extraVel = dot( normalize( velocity ), extraDir ) * length( minContact.resolution ) * extraDir;
				
				if( length(extraVel) < .01 )
					extraVel = V2d( 0, 0 );

				//extraVel = V2d( 0, 0 );
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

			if( leftWire->state == Wire::PULLING || leftWire->state == Wire::HIT )
			{
				touchEdgeWithLeftWire = tempCollision;
				if( action == WALLCLING )
				{
					touchEdgeWithLeftWire = true;
				}
			}
			

			if( rightWire->state == Wire::PULLING || rightWire->state == Wire::HIT )
			{
				touchEdgeWithRightWire = tempCollision;
				if( action == WALLCLING )
				{
					touchEdgeWithRightWire = true;
				}
			}

			bool bounceOkay = true;
			
			trueFramesInAir = framesInAir;

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
					SetAction( JUMP );
					holdJump = false;
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

			
			if (tempCollision && bounceGrassCount > 0)
			{
				HandleBounceGrass();
			}
			else if (tempCollision && action == SPRINGSTUNBOUNCE)
			{
				V2d norm = minContact.normal;
				springVel = norm * length(springVel);

				if (springVel.x > 0)
				{
					facingRight = true;
				}
				else if( springVel.x < 0 )
				{
					facingRight = false;
				}
			}
			else if( ( action == BOUNCEAIR || action == BOUNCEGROUND || bounceFlameOn ) && tempCollision && bounceOkay )
			{
				prevRail = NULL;
				//this condition might only work when not reversed? does it matter?
				if( bounceEdge == NULL )//|| ( bounceEdge != NULL && minContact.edge->Normal().y < 0 && bounceEdge->Normal().y >= 0 ) )
				{
					bounceEdge = minContact.edge;
					bounceNorm = minContact.normal;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt; //turn off grind attempter
				

					V2d oldv0 = bounceEdge->v0;
					V2d oldv1 = bounceEdge->v1;

					bounceQuant = bounceEdge->GetQuantity( minContact.position );

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

					V2d alongVel = V2d(-minContact.normal.y, minContact.normal.x);

					//double groundLength = length(ground->v1 - ground->v0);

					V2d bn = bounceEdge->Normal();

					V2d testVel = velocity;

					
					/*if (testVel.y > 20)
					{
						testVel.y *= .7;
					}
					else if (testVel.y < -30)
					{
						testVel.y *= .5;
					}*/
					
					groundSpeed = CalcLandingSpeed(testVel, alongVel, bn);
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
						SetAction( JUMP );
						holdJump = false;
						frame = 1;
						break;
					}
					
				}
				
				
			//	cout << "offset now!: " << offsetX << endl;
				//groundSpeed = 0;
			//	cout << "bouncing" << endl;
			}
			//else if( ((action == JUMP && !holdJump) || framesInAir > maxJumpHeightFrame ) && tempCollision && minContact.edge->Normal().y < 0 && abs( minContact.edge->Normal().x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			else if( ((action == JUMP && /*!holdJump*/false) || ( framesInAir > maxJumpHeightFrame || velocity.y > -8 || !holdJump ) || action == WALLCLING || action == WALLATTACK ) && tempCollision && minContact.normal.y < 0 && abs( minContact.normal.x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			{
				//	minContact.position += minContact.movingPlat->vel;//normalize( minContact.edge->v1 - minContact.edge->v0 ) * dot( minContact.movingPlat->vel, normalize( minContact.edge->v1 - minContact.edge->v0 ) );
				prevRail = NULL;

				//b.rh = dashHeight;
				//cout << "edge: " << minContact.edge->v0.x << ", " << minContact.edge->v0.y << ", v1: " << minContact.edge->v1.x << ", " << minContact.edge->v1.y << endl;
				//cout << "pos: " << position.x << ", " << position.y << ", minpos: " << minContact.position.x << ", " << minContact.position.y << endl;
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;

				//cout << "offsetX: " << offsetX << endl;

				//if( offsetX > b.rw + .00001 || offsetX < -b.rw - .00001 ) //to prevent glitchy stuff
				////if( false )
				//{
				//	cout << "prevented glitchy offset: " << offsetX << endl;
				//	assert( 0 );
				if( false )
				{
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
				framesSinceGrindAttempt = maxFramesSinceGrindAttempt; //turn off grind attempter
				
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );

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

				gNorm = ground->Normal();

				groundSpeed = CalcLandingSpeed( testVel, alongVel, gNorm );

				//normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				//cout << "setting groundSpeed: " << groundSpeed << endl;
				//V2d gNorm = ground->Normal();//minContact.normal;//ground->Normal();
				

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
			else if( (HasUpgrade(UPGRADE_POWER_GRAV) || gravityGrassCount > 0 )
				&& tempCollision && ((currInput.B && currInput.LUp())|| (HasUpgrade(UPGRADE_POWER_GRIND) && currInput.Y ))
				&& minContact.normal.y > 0 
				&& abs( minContact.normal.x ) < wallThresh 
				&& minContact.position.y <= position.y - b.rh + b.offset.y + 1
				&& !minContact.edge->IsInvisibleWall() )
			{
				prevRail = NULL;

				if( b.rh < normalHeight )
				{
					b.offset.y = -(normalHeight - b.rh);
				}

				if( minContact.edge->Normal().y <= 0 )
				{
					if( minContact.position == minContact.edge->v0 ) 
					{
						if( minContact.edge->edge0->Normal().y >= 0 )
						{
							minContact.edge = minContact.edge->edge0;
						}
					}
				}

				hasGravReverse = false;
				hasAirDash = true;
				hasDoubleJump = true;
				reversed = true;
				lastWire = 0;

				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;

				edgeQuantity = minContact.edge->GetQuantity( minContact.position );

				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = 0;

				V2d gno = ground->Normal();


				double angle = atan2( gno.x, -gno.y );
				
				if( -gno.y > -steepThresh )
				{
					groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );
				}
				else
				{
					groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );
				}

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

				ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
				ActivateSound( S_GRAVREVERSE );
			}
			else if( tempCollision && HasUpgrade(UPGRADE_POWER_GRIND) /*&& action == AIRDASH*/ && currInput.Y && velocity.y != 0 && abs( minContact.normal.x ) >= wallThresh && !minContact.edge->IsInvisibleWall()  )
			{
				prevRail = NULL;
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
			}
			else if( tempCollision )
			{
				velocity = newVel;
			}
			else
			{
				//cout << "no temp collision" << endl;
			}

			if( length( extraVel ) > 0 )
			{
				movementVec = stealVec + extraVel;
			}

			else
			{
				movementVec = stealVec;
			}


		}
	}
	while( (ground != NULL && !approxEquals( movement, 0 ) ) || ( ground == NULL && length( movementVec ) > 0 ) );


	PhysicsResponse();

	//cout << "post vel: " << velocity.x << ", " << velocity.y << endl;

	if ( reversed && ( action == STANDN || action == STEEPCLIMBATTACK || action == STEEPSLIDEATTACK ) && currHitboxes != NULL)
	{
		//auto it = currHitboxes->begin();
		//sf::IntRect aabb;
		//aabb.left = (*it).
		//for ( it != currHitboxes->end(); ++it)
		//{
		//	
		//	/*Grass *g = owner->explodingGravityGrass;
		//	while (g != NULL)
		//	{
		//		if ( g->visible && !g->exploding )
		//		{
		//			
		//		}
		//	}*/
		//}

		//queryMode = "gravitygrass";
		//owner->grassTree->Query( this, )
	}
}

bool Actor::CheckSwing()
{
	return CheckNormalSwing() || CheckRightStickSwing();
}

bool Actor::CheckNormalSwingHeld()
{
	return currInput.rightShoulder;
}

bool Actor::CheckSwingHeld()
{
	return CheckNormalSwingHeld() || CheckRightStickSwingHeld();
}

bool Actor::CheckNormalSwing()
{
	bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
	return normalSwing;
}

bool Actor::CheckRightStickSwingHeld()
{
	return currInput.RDown() || currInput.RLeft() || currInput.RUp() || currInput.RRight();
}

bool Actor::CheckRightStickSwing()
{
	bool rightStickSwing = (currInput.RDown() && !prevInput.RDown())
		|| (currInput.RLeft() && !prevInput.RLeft())
		|| (currInput.RUp() && !prevInput.RUp())
		|| (currInput.RRight() && !prevInput.RRight());
	return rightStickSwing;
}

bool Actor::TryGroundAttack()
{
	bool normalSwing = CheckNormalSwing();
	bool rightStickSwing = false;//CheckRightStickSwing();

	if ( normalSwing || rightStickSwing || pauseBufferedAttack == Action::STANDN
		|| stunBufferedAttack == Action::STANDN )
	{
		
		if (!rightStickSwing)
		{
			if (currInput.LLeft())
			{
				facingRight = false;
			}
			else if (currInput.LRight())
			{
				facingRight = true;
			}
		}
		else
		{
			//this is probably buggy on ceiling?
			if (currInput.RLeft())
			{
				facingRight = false;
			}
			else if (currInput.RRight())
			{
				facingRight = true;
			}
		}

		if (stunBufferedAttack == Action::STANDN)
		{
			V2d gn = ground->Normal();
			if (TerrainPolygon::IsSteepGround(gn) )
			{
				if (facingRight)
				{
					if (gn.x > 0)
					{
						stunBufferedAttack = Action::STEEPSLIDEATTACK;
					}
					else
					{
						stunBufferedAttack = Action::STEEPCLIMBATTACK;
					}
				}
				else
				{
					if (gn.x > 0)
					{
						stunBufferedAttack = Action::STEEPCLIMBATTACK;
					}
					else
					{
						stunBufferedAttack = Action::STEEPSLIDEATTACK;
					}
				}
				
			}

			SetAction(stunBufferedAttack);
			frame = 0;
		}
		else
		{
			SetAction(STANDN);
			frame = 0;
		}
		

		return true;
	}

	return false;
	
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

void Actor::HandleTouchedGate()
{
	Edge *edge = gateTouched;
	Gate *g = (Gate*)gateTouched->info;


	V2d A(b.globalPosition.x - b.rw, b.globalPosition.y - b.rh);
	V2d B(b.globalPosition.x + b.rw, b.globalPosition.y - b.rh);
	V2d C(b.globalPosition.x + b.rw, b.globalPosition.y + b.rh);
	V2d D(b.globalPosition.x - b.rw, b.globalPosition.y + b.rh);
	V2d nEdge = edge->Normal();//normalize( edge->v1 - edge->v0 );
	double ang = atan2(nEdge.x, -nEdge.y);

	double crossA = dot(A - edge->v0, nEdge);
	double crossB = dot(B - edge->v0, nEdge);
	double crossC = dot(C - edge->v0, nEdge);
	double crossD = dot(D - edge->v0, nEdge);

	//double crossCenter = cross(b.globalPosition - edge->v0, nEdge);
	double alongAmount = dot(b.globalPosition - edge->v0, normalize(edge->v1 - edge->v0));
	alongAmount /= length(edge->v1 - edge->v0);
	//alongAmount = 1.0 - alongAmount;
	//alongAmount = 1.0 - alongAmount;
	V2d alongPos = edge->v1 + normalize(edge->v0 - edge->v1) * alongAmount * edge->GetLength();

	double thresh = .01;
	bool activate = crossA > thresh && crossB > thresh && crossC > thresh && crossD > thresh;


	g->SetLocked(true);

	if (grindEdge != NULL)
	{
		Edge *otherEdge;
		if (edge == g->edgeA)
			otherEdge = g->edgeB;
		else
			otherEdge = g->edgeA;

		if (grindEdge == edge->edge0 || grindEdge == edge->edge1)
		{
			//this could be flawed. needs more testing

			activate = true;
		}
	}

	g->SetLocked(false);

	if (activate)
	{

		RelEffectInstance params;
		Transform tr = sf::Transform::Identity;
		int s = 1;
		if (!facingRight)
		{
			tr.scale(Vector2f(-s, s));
		}
		else
		{
			tr.scale(Vector2f(s, s));
		}
		params.SetParams(Vector2f(0, 0), tr, 8, 2, 0, &spriteCenter);
		//fair should be 25 but meh

		gateBlackFX = (RelEffectInstance*)gateBlackFXPool->ActivateEffect(&params);

		//lock all the gates from this zone now that I chose one


		g->PassThrough(alongAmount);

		if (g->IsZoneType())
		{

			//owner->SuppressEnemyKeys(g);

			Zone *oldZone;
			Zone *newZone;
			if (edge == g->edgeA)
			{
				oldZone = g->zoneB;
				newZone = g->zoneA;
			}
			else
			{
				oldZone = g->zoneA;
				newZone = g->zoneB;
			}

			if (oldZone != newZone)
			{

				bool twoWay = g->IsTwoWay();
				if (!twoWay) //for secret gates
				{
					if (oldZone != NULL && oldZone->active)
					{
						oldZone->ReformAllGates(g);
					}

					sess->adventureHUD->keyMarker->Reset();
				}
				sess->ActivateZone(newZone);
				
				if (!twoWay)
				{
					CreateKeyExplosion();
					sess->absorbDarkParticles->KillAllActive();
					numKeysHeld = 0;
				}
				else
				{ 
					CreateGateExplosion();
				}

				RestoreAirDash();
				RestoreDoubleJump();
			}
		}

		//V2d gEnterPos = alongPos;// +nEdge;// *32.0;

		V2d enterPos = edge->v1 + normalize(edge->v0 - edge->v1) * (1.0-alongAmount) * edge->GetLength()
			+ nEdge * 0.0; //could change this for offset along the normal

		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
			ts_fx_gateEnter, enterPos, false, ang, 8, 3, true);

		//set gate action to disperse
		//maybe have another gate action when you're on the gate and its not sure whether to blow up or not
		//it only enters this state if you already unlock it though
		gateTouched = NULL;
	}
	else if (crossA < 0 && crossB < 0 && crossC < 0 && crossD < 0)
	{
		//cout << "went back" << endl;
		gateTouched = NULL;
		sess->LockGate(g);

	}
	else
	{
		//cout << "not clear" << endl;
	}
}

void Actor::PhysicsResponse()
{
	V2d gn;
	//Edge *e;

	

	if ((action == RAILGRIND || action == RAILSLIDE ) && collision )
	{
		grindEdge = NULL;
		SetAction(JUMP);
		frame = 1;
		framesNotGrinding = 0;
		
		regrindOffCount = 0;
		framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	}
	else if (action == SPRINGSTUNAIRBOUNCE && collision)
	{
		SetAction(JUMP);
		frame = 1;
	}

	wallClimbGravityOn = false;
	if( grindEdge != NULL )
	{
		//e = grindEdge;
		framesInAir = 0;

		V2d oldv0 = grindEdge->v0;
		V2d oldv1 = grindEdge->v1;


		V2d grindPoint = grindEdge->GetPosition( grindQuantity );

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
			scorpOn = false;
			//oldBounceEdge = NULL;
			//BounceFlameOff();
			//bounceFlameOn = false;

			SetAction(BOUNCEGROUND);
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
					BounceFlameOn();
					runBounceFrame = 4 * 3;
					
					SetAction(LAND);
					frame = 0;
					//bounceEdge = NULL;
					ground = bounceEdge;
					edgeQuantity = bounceQuant;
					bounceEdge = NULL;
					//oldBounceEdge = NULL;
				}
			}

			if( bn.y != 0 )
			{
				if( bounceEdge != NULL )
				{
					V2d oldv0 = bounceEdge->v0;
					V2d oldv1 = bounceEdge->v1;

					
					position = bounceEdge->GetPosition( bounceQuant );

				}
				else
				{
					V2d oldv0 = ground->v0;
					V2d oldv1 = ground->v1;


					position = ground->GetPosition( bounceQuant );
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
		if (SpringLaunch()) return;

		if (TeleporterLaunch())return;

		if (SwingLaunch())return;

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

				UnlockGate( g );

				SetAction( JUMP );
				frame = 1;
				
				framesInAir = 0;
				holdJump = false;
				velocity = groundSpeed * normalize( ground->v1 - ground->v0 );
				//velocity = V2d( 0, 0 );
				leaveGround = true;
				ground = NULL;
				reversed = false;
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
		
		gn = ground->Normal();
		if( collision )
		{
			if( action == AIRHITSTUN )
			{
				//cout << "setting to ground hitstun!!!" << endl;
				SetAction(GROUNDHITSTUN);
				frame = 0;
			}
			/*else if (action == SEQ_KINFALL)
			{
				SetAction(SEQ_KINSTAND);
				frame = 0;
				groundSpeed = 0;
				physicsOver = true;
			}*/
			else if( action != GROUNDHITSTUN && action != LAND2 && action != LAND 
				&& action != SEQ_CRAWLERFIGHT_STRAIGHTFALL
				&& action != SEQ_CRAWLERFIGHT_LAND 
				&& action != SEQ_CRAWLERFIGHT_DODGEBACK && action != GRAVREVERSE
				&& action != JUMPSQUAT )
			{
			//	cout << "Action: " << action << endl;
				if( currInput.LLeft() || currInput.LRight() )
				{
					SetAction(LAND2);
					ActivateSound(S_LAND);
					//rightWire->UpdateAnchors(V2d( 0, 0 ));
					//leftWire->UpdateAnchors(V2d( 0, 0 ));
					frame = 0;
				}
				else
				{
					if( reversed )//&& trueFramesInAir > 1 )
					{
						//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
						//cout << "trueframes in air: " << trueFramesInAir << endl;
						//cout << "THIS GRAV REVERSE" << endl;
						//cout << "frames in air: " << framesInAir << endl;
						//cout << "frame: " << frame << endl;
						SetAction(GRAVREVERSE);

						if( currInput.LLeft() || currInput.LRight() )
						{
							storedReverseSpeed = 0;
						}
						else
						{
							storedReverseSpeed = -groundSpeed;
						}
							
						//if( groundSpeed != 0 )
						//{
						//storedReverseSpeed = -groundSpeed;
						//}
						//groundSpeed = 0;
							
					}
					else
					{
						SetAction(LAND);
						ActivateSound(S_LAND);
					}
					//rightWire->UpdateAnchors(V2d( 0, 0 ));
					//leftWire->UpdateAnchors(V2d( 0, 0 ));
					frame = 0;
					//cout << "blahaaa" << endl;
					//cout << "blahbbb" << endl;
					//cout << "l" << endl;
					//cout << "action = 5" << endl;
					//if( framesInAir > 0 )
					//{
						
					//}
				}
			}
			else if( action == SEQ_CRAWLERFIGHT_STRAIGHTFALL || action == SEQ_CRAWLERFIGHT_DODGEBACK )
			{
				//cout << "action = 41" << endl;
				SetAction(SEQ_CRAWLERFIGHT_LAND);
				frame = 0;
				groundSpeed = 0;
			}
		}
		framesInAir = 0;
		
		Vector2<double> groundPoint = ground->GetPosition( edgeQuantity );
		
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
				SetAction(LAND2);
				frame = 0;
			}
		}
		else
		{
			
			if( ( action == STEEPCLIMB || action == STEEPSLIDE ) && (gn.y <= -steepThresh || !approxEquals( abs( offsetX ), b.rw ) ) )
			{
				//cout << "here no: " << action << ", " << offsetX << endl;
				SetAction(LAND2);
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
		if (SpringLaunch()) return;

		if (TeleporterLaunch())return;

		if (SwingLaunch())return;

		if( action == GROUNDHITSTUN )
		{
			SetAction(AIRHITSTUN);
			frame = 0;
		}

		

		if( action != AIRHITSTUN && action != AIRDASH )
		{
			//oldAction = action;
			if( collision && action != WALLATTACK && action != WALLCLING )
			{
				if( length( wallNormal ) > 0 
					&& (currWall == NULL || !currWall->IsInvisibleWall())
					&& oldVelocity.y >= 0 /*&& rightWire->state != Wire::PULLING
					&& leftWire->state != Wire::PULLING*/ )
				{
					if( wallNormal.x > 0)
					{
						if( currInput.LLeft() && !currInput.LDown() )
						{
							//cout << "setting to wallcling" << endl;
							facingRight = true;
							SetAction(WALLCLING);
							repeatingSound = ActivateSound(S_WALLSLIDE, true);
							frame = 0;
						}
					}
					else
					{
						if( currInput.LRight() && !currInput.LDown() )
						{
							//cout << "setting to wallcling" << endl;
							facingRight = false;
							SetAction(WALLCLING);
							repeatingSound = ActivateSound(S_WALLSLIDE, true);
							
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
							//stopWallClinging = true;
						}
					}
					else
					{
						if( !currInput.LRight() || currInput.LDown() )
						{
							//stopWallClinging = true;
						}
					}
				}
				else
				{
					//cout << "zzz: " << (int)collision << endl;//wallNormal.x << ", " << wallNormal.y << endl;
					stopWallClinging = true;
					
				}

				if( stopWallClinging )
				{
					//cout << "stop wall clinging" << endl;
					SetAction( JUMP );
					frame = 1;
					holdJump = false;
				}
			}

			if( leftGround )
			{
				SetAction( JUMP );
				frame = 1;
				holdJump = false;
			}
		}
		else if (action != AIRHITSTUN && action != WALLATTACK && action != AIRDASH)
		{
			if (collision)
			{
				if (length(wallNormal) > 0 && oldVelocity.y <= 0)
				{
					wallClimbGravityOn = true;
				}
			}
		}

		if (collision && minContact.normal.y > 0 && !reversed && action != WALLCLING 
			&& rightWire->state != Wire::PULLING && leftWire->state != Wire::PULLING
			&& action != SEQ_KINFALL )
			//&& hitCeilingCounter == 0 )
		{
			//hitCeilingCounter = hitCeilingLockoutFrames;
			ActivateSound(S_HITCEILING);

			if (action == SEQ_KINTHROWN)
			{
				SetAction(SEQ_KINFALL);
				frame = 1;
				velocity = V2d(0, 0);
				physicsOver = true;
			}
		}
	}

	

	if( groundedWallBounce )
	{
		SetAction(BOUNCEGROUNDEDWALL);
		frame = 0;

		//already bounced by here i think
		if( storedBounceGroundSpeed < 0 )
		{
			//cout << "facing right: " << groundSpeed << endl;
			facingRight = true;
		}
		else
		{
			//cout << "facing left: " << groundSpeed << endl;
			facingRight = false;
		}
	}

	UpdateHitboxes();

	if( gateTouched != NULL )
	{
		HandleTouchedGate();
	}
	
	if( owner != NULL && owner->raceFight != NULL )
	{
		Actor *pTarget = NULL;
		int target = 0;
		if( actorIndex == 0 )
		{
			target = 1;
		}
		else if( actorIndex == 1 )
		{
			target = 0;
		}
		pTarget = owner->GetPlayer( target );

		if( IHitPlayer( target ) )
		{
			pTarget->ApplyHit( currHitboxInfo );
			//ConfirmHit( ,2, 5, .8, 6 );

			if( owner->raceFight != NULL )
				owner->raceFight->PlayerHitByPlayer( actorIndex, target );
		}
	}
}

void Actor::SetGroundedSpriteTransform()
{
	double angle = GroundedAngle();
	SetGroundedSpriteTransform(ground, angle );
}

void Actor::SetGroundedSpriteTransform( Edge * e, double angle )
{
	V2d groundP = e->GetPosition(edgeQuantity);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	double factor;
	if (reversed)
	{
		if (angle < 0)
			angle += 2 * PI;
		factor = 1.0 - abs(((angle-PI) / (PI / 6)));
	}
	else
	{
		factor = 1.0 - abs((angle / (PI / 6)));
	}
	
	//cout << "factor: " << factor << "angle: " << angle << endl;
		//if (factor < .9)
		//	factor = 0;
	V2d along = e->Along();
	if (reversed)
	{
		along = -along;
	}
	Vector2f testOffset(along * offsetX * factor);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
	{
		sprite->setPosition(groundP.x + offsetX, groundP.y);
	}
	else
		sprite->setPosition(Vector2f(groundP) + testOffset);
}

void Actor::SetGroundedSpritePos( Edge * e, double angle )
{
	V2d groundP = e->GetPosition(edgeQuantity);

	double factor;
	if (reversed)
	{
		if (angle < 0)
			angle += 2 * PI;
		factor = 1.0 - abs(((angle - PI) / (PI / 6)));
	}
	else
	{
		factor = 1.0 - abs((angle / (PI / 6)));
	}

	//aligns based on slopes
	V2d along = e->Along();
	if (reversed)
	{
		along = -along;
	}
	Vector2f testOffset(along * offsetX * factor);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
	{
		sprite->setPosition(groundP.x + offsetX, groundP.y);
	}
	else
		sprite->setPosition(Vector2f(groundP) + testOffset);
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
				//cout << "BLAH BLAH" << endl;
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
		vector<CollisionBox> *cList = &(currHitboxes->GetCollisionBoxes(currHitboxFrame));
		if( cList != NULL )
		for( auto it = cList->begin(); it != cList->end(); ++it )
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

			(*it).flipHorizontal = ((!facingRight && !reversed) 
				|| (facingRight && reversed));
				
				//offX = -offX;

			//if( reversed )
			//	offY = -offY;

			V2d pos = position;
			if( grindEdge != NULL )
			{
				pos = grindEdge->GetPosition( grindQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
			}
			else if( ground != NULL )
			{
			//	V2d gn = ground->Normal();
				pos = V2d( sprite->getPosition().x, sprite->getPosition().y );

				//pos = position;
				//pos += gd * offX + gn * -offY + gn * (double)sprite->getLocalBounds().height / 2.0;
				//pos += gd * offX + gn * -offY; //+ V2d( offsetX, 0 );
			}
			else
			{
				//pos += V2d( offX, offY );// + V2d( offsetX, 0 );
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
	else if( action != GRINDBALL && action != GRINDATTACK )
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
		
		
		//hurtBody.offset = b.offset;
	}

	
	if( grindEdge != NULL )
	{
		hurtBody.globalPosition = grindEdge->GetPosition( grindQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
		hurtBody.globalAngle = angle;
	}
	else if( ground != NULL )
	{
		if( gn.x == 0 )
		{
			if (action == DASH)
			{
				int xxxx = 6;
			}
			hurtBody.globalPosition = ground->GetPosition(edgeQuantity) + V2d( offsetX, 0 ) + hurtBody.offset + gn * hurtBody.rh ;//+ V2d( 0, -hurtBody.rh );
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
			hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + V2d( 0, -1 ) * (double)(b.rh) + V2d( xoff, 0 );// + hurtBody.rh );

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
			hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + V2d( 0, 1 ) * (double)(b.rh) + V2d( xoff, 0 );// + hurtBody.rh );

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
			hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + gn * (double)(b.rh);// + hurtBody.rh );
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
	//	hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + gn * (double)( b.rh + 5 );
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

void Actor::ClearSpecialTerrainCounts()
{
	for (int i = 0; i < SPECIAL_TERRAIN_Count; ++i)
	{
		specialTerrainCount[i] = 0;
	}
}

void Actor::HandleSpecialTerrain()
{
	for (int i = 0; i < SPECIAL_TERRAIN_Count; ++i)
	{
		if (specialTerrainCount[i] > 0)
		{
			HandleSpecialTerrain(i);
		}
	}
}

void Actor::HandleSpecialTerrain(int stType)
{
	switch (stType)
	{
	case SPECIAL_TERRAIN_WATER:
		RestoreAirDash();
		RestoreDoubleJump();
		break;
	case SPECIAL_TERRAIN_GLIDEWATER:
	{
		if (action != SPRINGSTUNGLIDE)
		{
			V2d vel = GetTrueVel();

			if (ground != NULL)
			{
				ground = NULL;
				framesInAir = 0;
				vel.y -= 5;
			}


			vel = normalize(vel) * 15.0;

			springVel = vel;

			springExtra = V2d(0, 0);

			if (vel.x < 0)
				facingRight = false;
			else if (vel.x > 0)
				facingRight = true;

			action = SPRINGSTUNGLIDE;
			holdJump = false;
			holdDouble = false;
			hasDoubleJump = true;
			hasAirDash = true;
			rightWire->Reset();
			leftWire->Reset();
			frame = 0;
			UpdateHitboxes();
			ground = NULL;
			wallNormal = V2d(0, 0);
			velocity = V2d(0, 0);
			currWall = NULL;
		}
		springStunFrames = 10;///currSwingLauncher->stunFrames;
		break;
	}
	}
}

void Actor::UpdateSmallLightning()
{
	int smallLightningCounter = -1;
	switch (speedLevel)
	{
	case 0:
		smallLightningCounter = 30;
		break;
	case 1:
		smallLightningCounter = 20;
		break;
	case 2:
		smallLightningCounter = 10;
		break;
	}

	bool dontActivateLightningAction = action == SEQ_MEDITATE_MASKON ||
		action == SEQ_MASKOFF || action == SEQ_MEDITATE;

	if (!IsIntroAction(action) && GetTotalGameFrames() % smallLightningCounter == 0
		&& !dontActivateLightningAction)
	{
		RelEffectInstance params;
		//EffectInstance params;
		Transform tr = sf::Transform::Identity;
		//params.SetParams(Vector2f(position.x, position.y - 100) , tr, 7, 1, 0);
		Vector2f randPos(rand() % 100 - 50, rand() % 100 - 50);

		params.SetParams(randPos, tr, 7, 1, 0, &spriteCenter);

		int r = rand() % 7;

		smallLightningPool[r]->ActivateEffect(&params);
	}

	for (int i = 0; i < 7; ++i)
	{
		smallLightningPool[i]->Update();
	}
}

void Actor::UpdateRisingAura()
{
	//this is turned off atm
	if (!IsIntroAction(action) && GetTotalGameFrames() % 30 == 0)
	{
		RelEffectInstance params;
		//EffectInstance params;
		Transform tr = sf::Transform::Identity;
		Vector2f randPos(rand() % 20 - 10, rand() % 20 - 10);

		params.SetParams(randPos, tr, 1, 60, 0, &spriteCenter, 40);
		//EffectInstance *ei = risingAuraPool->ActivateEffect(&params);
		//ei->SetVelocityParams(Vector2f(0, 0), Vector2f(0, -.02), 5 );
	}
}

void Actor::UpdateLockedFX()
{
	if (currLockedFairFX != NULL && action != FAIR)
	{
		currLockedFairFX->ClearLockPos();
		currLockedFairFX = NULL;
	}
	if (currLockedDairFX != NULL && action != DAIR)
	{
		currLockedDairFX->ClearLockPos();
		currLockedDairFX = NULL;
	}
	if (currLockedUairFX != NULL && action != UAIR)
	{
		currLockedUairFX->ClearLockPos();
		currLockedUairFX = NULL;
	}
}

void Actor::ProcessSpecialTerrain()
{
	ClearSpecialTerrainCounts();
	queryMode = "specialterrain";
	Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
	GetSpecialTerrainTree()->Query(this, r);

	HandleSpecialTerrain();
}

void Actor::UpdateScorpCap()
{
	if (scorpOn)
	{
		scorpAdditionalCap += scorpAdditionalAccel;
		if (scorpAdditionalCap > scorpAdditionalCapMax)
			scorpAdditionalCap = scorpAdditionalCapMax;
	}
	else
	{
		scorpAdditionalCap -= scorpAdditionalAccel;
		if (scorpAdditionalCap < 0)
			scorpAdditionalCap = 0;
	}
}

void Actor::ProcessHitGoal()
{
	if (hitGoal)// && action != GOALKILL && action != EXIT && action != GOALKILLWAIT && action != EXITWAIT)
	{
		if (owner != NULL)
		{
			owner->totalFramesBeforeGoal = owner->totalGameFrames;
			SetAction(GOALKILL);
			SetKinMode(K_NORMAL);
			hitGoal = false;

			if (owner->parentGame == NULL)
			{
				if (owner->recPlayer != NULL)
				{
					owner->recPlayer->RecordFrame();
					owner->recPlayer->StopRecording();
					owner->recPlayer->WriteToFile("testreplay.brep");
				}

				if (owner->recGhost != NULL)
				{
					owner->recGhost->StopRecording();
					owner->recGhost->WriteToFile("Recordings/Ghost/testghost.bghst");
				}
			}

			frame = 0;
			position = owner->goalNodePos;
			owner->cam.Ease(Vector2f(owner->goalNodePosFinal), 1, 60, CubicBezier());
			rightWire->Reset();
			leftWire->Reset();
			SetKinMode(K_NORMAL);
		}
		else if (editOwner != NULL)
		{
			editOwner->EndTestMode();
		}
	}
	else if (hitNexus)
	{
		owner->totalFramesBeforeGoal = owner->totalGameFrames;
		SetAction(NEXUSKILL);
		SetKinMode(K_NORMAL);
		hitNexus = false;
		if (owner->parentGame == NULL)
		{
			if (owner->recPlayer != NULL)
			{
				owner->recPlayer->RecordFrame();
				owner->recPlayer->StopRecording();
				owner->recPlayer->WriteToFile("testreplay.brep");
			}

			if (owner->recGhost != NULL)
			{
				owner->recGhost->StopRecording();
				owner->recGhost->WriteToFile("Recordings/Ghost/testghost.bghst");
			}
		}
		frame = 0;
		position = owner->goalNodePos;
		rightWire->Reset();
		leftWire->Reset();
	}
}

bool Actor::CareAboutSpeedAction()
{
	return action != DEATH && action != EXIT && !IsGoalKillAction(action) && action != RIDESHIP && action != GRINDBALL
		&& action != GRINDATTACK;
}

void Actor::UpdateSpeedBar()
{
	V2d trueVel;
	double speed;
	if (ground != NULL) //ground
	{
		trueVel = normalize(ground->v1 - ground->v0) * groundSpeed;
		//speed = abs(groundSpeed);
	}
	else //air
	{
		trueVel = velocity;
		//speed = length( velocity );
	}

	trueVel.y = min(40.0, trueVel.y); //falling can only help your speed so much

	speed = length(trueVel);

	if (CareAboutSpeedAction())
	{
		if (speed > currentSpeedBar)
		{
			currentSpeedBar += speedChangeUp;
			if (currentSpeedBar > speed)
				currentSpeedBar = speed;//currentSpeedBar * (1.0 -fUp) + speed * fUp;
		}
		else if (speed < currentSpeedBar)
		{
			currentSpeedBar -= speedChangeDown;
			if (currentSpeedBar < speed)
			{
				currentSpeedBar = speed;
			}
			//currentSpeedBar = currentSpeedBar * (1.0 -fDown) + speed * fDown;
		}

		if (currentSpeedBar >= level2SpeedThresh)
		{
			speedLevel = 2;
		}
		else if (currentSpeedBar >= level1SpeedThresh)
		{
			speedLevel = 1;
		}
		else
		{
			speedLevel = 0;
		}

	}
}

void Actor::UpdateMotionGhosts()
{
	//shouldn't be max motion ghosts cuz thats not efficient
	for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhostBuffer->SetRotation(i, sprite->getRotation() / 180.f * PI);
		motionGhostBufferBlue->SetRotation(i, sprite->getRotation() / 180.f * PI);
		motionGhostBufferPurple->SetRotation(i, sprite->getRotation() / 180.f * PI);
	}

	V2d motionGhostDir;
	double motionMagnitude = 0;
	if (ground != NULL)
	{
		motionGhostDir = -normalize(normalize(ground->v1 - ground->v0) * groundSpeed);
		if (reversed)
			motionGhostDir = -motionGhostDir;
		motionMagnitude = abs(groundSpeed);
	}
	else
	{
		motionGhostDir = -normalize(velocity);
		motionMagnitude = length(velocity);
	}

	float dist = motionGhostSpacing;

	int showMotionGhosts = min(motionMagnitude / 1.0, 80.0);

	if (speedLevel == 0)
	{
		showMotionGhosts = min(showMotionGhosts * 2.0, 80.0);
	}
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetSpread(showMotionGhosts, Vector2f(motionGhostDir.x, motionGhostDir.y), sprite->getRotation() / 180.f * PI);
		motionGhostsEffects[i]->SetRootPos(Vector2f(spriteCenter.x, spriteCenter.y));
	}
}

void Actor::UpdateSpeedParticles()
{
	if (speedParticleCounter == speedParticleRate && CareAboutSpeedAction())
	{
		if (speedLevel == 1)
		{
			Tileset *tset = NULL;
			int randTex = rand() % 3;
			if (randTex == 0)
				tset = ts_fx_chargeBlue0;
			else if (randTex == 1)
				tset = ts_fx_chargeBlue1;
			else
				tset = ts_fx_chargeBlue2;

			int rx = 30;
			int ry = 30;

			if (ground != NULL)
			{

				double angle = GroundedAngle();
				V2d groundPos = ground->GetPosition(edgeQuantity);

				V2d truePos = groundPos + currNormal * normalHeight;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);



				ActivateEffect(EffectLayer::IN_FRONT, tset, truePos, false, angle, 6, 3, facingRight);
			}
			else
			{
				V2d truePos = position;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);
				double angle = atan2(currNormal.x, currNormal.y);
				ActivateEffect(EffectLayer::IN_FRONT, tset, truePos, false, angle, 6, 3, facingRight);
			}

		}
		else if (speedLevel == 2)
		{
			int rx = 30;
			int ry = 30;
			if (ground != NULL)
			{
				V2d groundPos = ground->GetPosition(edgeQuantity);
				V2d truePos = groundPos + currNormal * normalHeight;//.0;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);
				double angle = GroundedAngle();

				ActivateEffect(EffectLayer::IN_FRONT, ts_fx_chargePurple, truePos, false, angle, 6, 3, facingRight);
			}
			else
			{
				V2d truePos = position;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);
				double angle = atan2(currNormal.x, currNormal.y);
				ActivateEffect(EffectLayer::IN_FRONT, ts_fx_chargePurple, truePos, false, angle, 6, 3, facingRight);
			}
		}

		speedParticleCounter = 0;
	}
}
void Actor::UpdateAttackLightning()
{
	CreateAttackLightning();

	for (int i = 0; i < 3; ++i)
	{
		fairLightningPool[i]->Update();
		dairLightningPool[i]->Update();
		uairLightningPool[i]->Update();
	}
}

void Actor::UpdatePlayerShader()
{
	if (kinMode == K_DESPERATION )
	{
		//cout << "sending this parameter! "<< endl;
		sh.setUniform("despFrame", (float)despCounter);
	}
	else
	{

		sh.setUniform("despFrame", (float)-1);
	}

	
}

void Actor::UpdateDashBooster()
{
	//happens even when in time slow
	if (action == DASH)
	{
		if (currBBoostCounter < maxBBoostCount)
		{
			currBBoostCounter++;
		}
	}
}

void Actor::SlowDependentFrameIncrement()
{
	if (slowCounter == slowMultiple)
	{
		ActionTimeDepFrameInc();

		if (wallJumpFrameCounter < wallJumpMovementLimit)
			wallJumpFrameCounter++;
		//cout << "++frames in air: "<< framesInAir << " to " << (framesInAir+1) << endl;
		framesInAir++;
		framesSinceDouble++;

		if (gravResetFrames > 0)
		{
			gravResetFrames--;
			if (gravResetFrames == 0)
			{
				extraGravityModifier = 1.0;
			}
		}


		if (action == GRINDBALL || action == GRINDATTACK || action == RAILGRIND)
		{
			framesGrinding++;
			framesNotGrinding = 0;
		}
		else
		{
			framesGrinding = 0;
			framesNotGrinding++;
		}

		if (action == BOUNCEAIR && oldBounceEdge != NULL)
		{
			framesSinceBounce++;
		}

		++framesSinceRightWireBoost;
		++framesSinceLeftWireBoost;
		++framesSinceDoubleWireBoost;

		++frame;



		if (springStunFrames > 0)
			--springStunFrames;
		//cout << "frame: " << frame << endl;

		++framesSinceClimbBoost;
		++speedParticleCounter;


		slowCounter = 1;

		if (!IsIntroAction(action))
			if (invincibleFrames > 0)
				--invincibleFrames;

		if (flashFrames > 0)
			--flashFrames;
	}
	else
		slowCounter++;
}

void Actor::UpdateBounceFlameCounters()
{
	if (bounceFlameOn)
	{
		if (ground == NULL)
		{
			airBounceFrame++;
			if (airBounceFrame == airBounceFlameFrames)
			{
				airBounceFrame = 13 * 3;
			}
		}
		else
		{
			runBounceFrame++;
			if (runBounceFrame == runBounceFlameFrames)
			{
				runBounceFrame = 8 * 3;
			}
		}

		++framesFlameOn;
	}
}

void Actor::TryEndLevel()
{
	if (action == EXITWAIT && frame == GetActionLength(EXITWAIT))
	{
		owner->EndLevel();
		//owner->goalDestroyed = true;	
	}
	else if (action == EXITBOOST && frame == GetActionLength(EXITBOOST))
	{
		owner->EndLevel();
		//owner->goalDestroyed = true;
	}
}

void Actor::UpdatePostPhysics()
{
	KinModeUpdate();

	keyExplodePool->Update();
	if (!keyExplodeRingGroup->Update())
	{
		//keyExplodeRingGroup->Reset();
	}

	UpdateSmallLightning();
	UpdateRisingAura();

	UpdateLockedFX();	
	QueryTouchGrass();

	ProcessSpecialTerrain();

	UpdateScorpCap();
	
	if( ground != NULL )
	{
		currNormal = ground->Normal();
	}

	if( action == DEATH )
	{
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

		//if( frame % 1 == 0 )
		//{
		//	double startRadius = 64;
		//	double endRadius = 500;

		//	double part = frame / 88.f;

		//	double currRadius = startRadius * ( 1.f - part ) + endRadius * ( part );

		//	int randAng = (rand() % 360);
		//	double randAngle = randAng / 180.0 * PI;
		//	//randAngle += PI / 2.0;
		//	V2d pos( sin( randAngle ) * currRadius, -cos( randAngle ) * currRadius );

		//	pos += position;
		//	double fxAngle = randAngle - PI / 3.5;
		//	//cout << "randAngle: " << randAngle << endl;

		//	//cout << "randang: " << randAng << endl;
		//	ActivateEffect( EffectLayer::IN_FRONT, ts_fx_death_1c, pos, false, fxAngle, 12, 2, true );
		//}

		if( kinMask != NULL)
			kinMask->Update( speedLevel, kinMode == K_DESPERATION );


		++frame;
		return;
	}

	ProcessHitGoal();

	
	UpdateSprite();
	
	//risingAuraPool->Update();
	UpdateAttackLightning();
	
	gateBlackFXPool->Update();

	UpdateSpeedBar();

	UpdateMotionGhosts();

	UpdateSpeedParticles();

	UpdatePlayerShader();

	rotaryAngle = sprite->getRotation() / 180 * PI;

	UpdateDashBooster();

	SlowDependentFrameIncrement();

	ActionTimeIndFrameInc();

	if (framesSinceGrindAttempt < maxFramesSinceGrindAttempt)
	{
		++framesSinceGrindAttempt;
	}

	if( standNDashBoostCurr > 0 )
		standNDashBoostCurr--;

	UpdateBounceFlameCounters();
	

	UpdateHitboxes();

	//pTrail->Update( position );

	if( kinMask != NULL)
		kinMask->Update(speedLevel, kinMode == K_DESPERATION );

	if( ground != NULL ) //doesn't work when grinding or bouncing yet
	{
		velocity = normalize( ground->v1 - ground->v0) * groundSpeed;
	}

	if (kinRing != NULL)
		kinRing->Update();

	TryEndLevel();
}

void Actor::BounceFlameOn()
{
	framesFlameOn = 0;
	bounceFlameOn = true;
	scorpOn = true;
	runBounceFrame = 0;
	airBounceFrame = 0;
}

void Actor::BounceFlameOff()
{
	scorpOn = false;
	framesFlameOn = 0;
	bounceFlameOn = false;
	oldBounceEdge = NULL;
	bounceEdge = NULL;
	boostBounce = false;
	bounceGrounded = false;
}

bool Actor::IsBeingSlowed()
{
	if( owner != NULL && owner->raceFight != NULL )
	{
		Actor *other;
		if( actorIndex == 0 )
		{
			other = owner->GetPlayer( 1 );
		}
		else
		{
			other = owner->GetPlayer( 0 );
		}

		bool found = false;
		for( int i = 0; i < other->maxBubbles; ++i )
		{
			if( other->bubbleFramesToLive[i] > 0 )
			{
				if( length( position - other->bubblePos[i] ) <= other->bubbleRadius )
				{
					return true;
				}
			}
		}
		return false;
	}
	else
	{
		//except for gator fight
		return false;
	}
}

bool Actor::CanShootWire()
{
	return !IsSpringAction(action);
}

bool Actor::CanCreateTimeBubble()
{
	return action != SPRINGSTUNTELEPORT;
}

void Actor::HandleGroundTrigger(GroundTrigger *trigger)
{
	storedTrigger = trigger;
	switch (trigger->trigType)
	{
	case TRIGGER_SHIPPICKUP:
	{
		ShipPickupPoint(trigger->currPosInfo.GetQuant(), trigger->facingRight);
		break;
	}
	case TRIGGER_HOUSEFAMILY:
	{
		SetKinMode(K_NORMAL);
		SetExpr(KinMask::Expr_NEUTRAL);
		assert(ground != NULL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;
		//facingRight = trigger->facingRight;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}
		TurnFace();
		owner->SetStorySeq(trigger->storySeq);
		//owner->state = GameSession::STORY;
		//owner->state = GameSession::SEQUENCE;
		//physicsOver = true;
		//owner->activeSequence = trigger->gameSequence;

		break;
	}
	case TRIGGER_GETAIRDASH:
	{
		SetKinMode(K_NORMAL);
		SetExpr(KinMask::Expr_NEUTRAL);
		assert(ground != NULL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;
		//facingRight = trigger->facingRight;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		//SetAction(GETPOWER_AIRDASH_MEDITATE);
		//frame = 0;
		physicsOver = true;
		sess->activeSequence = trigger->gameSequence;
		//owner->currStorySequence = trigger->storySeq;
		//owner->state = GameSession::STORY;
		break;
	}
	case TRIGGER_DESTROYNEXUS1:
	{
		SetKinMode(K_NORMAL);
		//SetExpr(Expr_NEUTRAL);
		//assert(ground != NULL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		//owner->Fade(false, 30, Color::Black);

		sess->activeSequence = trigger->gameSequence;

		SetAction(SEQ_ENTERCORE1);
		frame = 0;
		physicsOver = true;

		//owner->currStorySequence = trigger->storySeq;
		//owner->state = GameSession::STORY;
		break;
	}
	case TRIGGER_CRAWLERATTACK:
	{
		SetKinMode(K_NORMAL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		SetAction(SEQ_LOOKUP);
		frame = 0;
		physicsOver = true;

		sess->activeSequence = trigger->gameSequence;
		break;
	}
	case TRIGGER_TEXTTEST:
	{
		SetKinMode(K_NORMAL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		WaitInPlace();
		physicsOver = true;

		sess->activeSequence = trigger->gameSequence;
		//owner->SetStorySeq(trigger->storySeq);
		break;
	}
	}
}

double Actor::GetGravity()
{
	return gravity * extraGravityModifier;
}

sf::Vector2<double> Actor::AddGravity( sf::Vector2<double> vel )
{
	double normalGravity;
	if( vel.y >= maxFallSpeedSlow )
	{
		normalGravity = gravity * .4 / slowMultiple;
	}
	//else if (abs( vel.y ) < 8 )
	/*else if (abs(vel.y) < 8)
	{
		normalGravity = gravity * .9 / slowMultiple;
	}*/
	else if( vel.y < 0 )
	{
		normalGravity = gravity * 1.2 / slowMultiple;
	}
	else
	{
		normalGravity = gravity / slowMultiple;
	}

	if (wallClimbGravityOn)
	{
		normalGravity *= wallClimbGravityFactor;
	}

	normalGravity *= extraGravityModifier;

	vel += V2d(0, normalGravity );

	return vel;
}

//void Actor::StartSeq( SeqType s )
//{
//	switch( s )
//	{
//	case SEQ_CRAWLER_FIGHT:
//		{
//			PoiInfo *pi = owner->poiMap["crawlerfighttrigger"];
//			V2d startFall = pi->pos;
//
//			action = JUMP;
//			frame = 1;
//			velocity = V2d( 0, 0 );
//			cutInput = true;
//			owner->cam.manual = true;
//		}
//		break;
//	}
//}
//
//void Actor::UpdateSeq()
//{
//	switch( seq )
//	{
//	case SEQ_CRAWLER_FIGHT:
//		{
//
//		}
//		break;
//	}
//}

bool Actor::SwingLaunch()
{
	/*if (currSwingLauncher != NULL)
	{
		currSwingLauncher->Launch();

		oldSwingLauncher = currSwingLauncher;

		position = currSwingLauncher->position;
		V2d dir = currSwingLauncher->dir;

		double s = currSwingLauncher->speed;

		springVel = currSwingLauncher->dir * s;

		springExtra = V2d(0, 0);

		springStunFrames = currSwingLauncher->stunFrames;

		ground = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		action = SWINGSTUN;

		currSwingLauncher = NULL;

		holdJump = false;
		holdDouble = false;
		hasDoubleJump = true;
		hasAirDash = true;
		rightWire->Reset();
		leftWire->Reset();
		frame = 0;
		UpdateHitboxes();
		ground = NULL;
		wallNormal = V2d(0, 0);
		velocity = V2d(0, 0);
		currWall = NULL;
		return true;
	}*/

	return false;
}

bool Actor::TeleporterLaunch()
{
	/*if (currTeleporter != NULL)
	{
		if (!currTeleporter->TryTeleport())
		{
			currTeleporter = NULL;
			return false;
		}

		oldTeleporter = currTeleporter;
		position = currTeleporter->position;
		V2d dir = currTeleporter->dir;

		double s = currTeleporter->speed;

		springVel = currTeleporter->dir * s;

		springExtra = V2d(0, 0);

		springStunFrames = currTeleporter->stunFrames;

		ground = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		action = SPRINGSTUNTELEPORT;
		teleportSpringDest = currTeleporter->dest;
		teleportSpringVel = velocity;

		currTeleporter = NULL;

		holdJump = false;
		holdDouble = false;
		hasDoubleJump = true;
		hasAirDash = true;
		rightWire->Reset();
		leftWire->Reset();
		frame = 0;
		UpdateHitboxes();
		ground = NULL;
		wallNormal = V2d(0, 0);
		velocity = V2d(0, 0);
		currWall = NULL;
		return true;
	}*/

	return false;
}

bool Actor::SpringLaunch()
{
	if (currSpring != NULL)
	{
		currSpring->Launch();
		position = currSpring->GetPosition();
		V2d sprDir = currSpring->dir;
		
		double s = currSpring->speed;

		if (currSpring->springType == Spring::BOUNCE)
		{
			V2d testVel(velocity.x, velocity.y);
			double along = dot(testVel, sprDir);
			if (along > s)
			{
				s = along;
			}
		}

		springVel = currSpring->dir * s;

		springExtra = V2d( 0, 0 );

		if (currSpring->springType != Spring::TELEPORT)
		{
			if (springVel.x > .01)
			{
				facingRight = true;
			}
			else if (springVel.x < -.01)
			{
				facingRight = false;
			}
		}
		

		springStunFrames = currSpring->stunFrames;

		if (currSpring->springType == Spring::GREEN )
		{
			action = SPRINGSTUNGLIDE;
		}
		//else if (currSpring->springType == Spring::REDIRECT)
		//{
		//	//action = SPRINGSTUN;
		//	//springStunFrames = 15;
		//	if (ground != NULL)
		//	{
		//		//springVel = currSpring->dir * //abs(groundSpeed);
		//	}
		//	else
		//	{
		//		//springVel = 
		//		//springVel = currSpring->dir * length(velocity);
		//	}
		//	
		//}
		else if (currSpring->springType == Spring::BOUNCE)
		{
			action = SPRINGSTUNBOUNCE;
			
		}
		else if (currSpring->springType == Spring::AIRBOUNCE)
		{
			action = SPRINGSTUNAIRBOUNCE;
		}
		else if (currSpring->springType == Spring::TELEPORT)
		{
			ground = NULL;
			bounceEdge = NULL;
			grindEdge = NULL;
			action = SPRINGSTUNTELEPORT;
			teleportSpringDest = currSpring->dest;
			teleportSpringVel = velocity;
			//springStunFrames = 0;
			//position = currSpring->dest;
		}
		//else if (currSpring->springType == Spring::REFLECT)
		//{
		//	
		//	if (ground != NULL)
		//	{

		//	}
		//	else
		//	{
		//		double len = length(velocity);
		//		if (len < 25)
		//		{
		//			len = 25;
		//			//springVel = currSpring->dir * len;
		//		}
		//		//else
		//		{

		//			action = SPRINGSTUN;
		//			springStunFrames = 15;

		//			V2d v = normalize(velocity);
		//			cout << "v: " << v.x << ", " << v.y << endl;
		//			v = -v;

		//			V2d dir = currSpring->dir;

		//			if (dot(v, currSpring->dir) > 0 )
		//			{
		//				dir = -dir;
		//			}
		//			

		//			double a = GetVectorAngleDiffCCW(v, dir);
		//			//V2d di = currSpring->dir;
		//			RotateCCW(dir, a);
		//			//V2d reflectDir = d;//(cross(v, currSpring->dir), dot(v, currSpring->dir) );
		//			//cout << "v: " << v.x << ", " << v.y << endl;
		//			//cout << "reflectDir: " << reflectDir.x << ", " << reflectDir.y << endl;
		//			springVel = dir * len;
		//		}
		//	}

		//}
		else
		{
			action = SPRINGSTUN;
		}

		currSpring = NULL;
		
		holdJump = false;
		holdDouble = false;
		hasDoubleJump = true;
		hasAirDash = true;
		rightWire->Reset();
		leftWire->Reset();
		frame = 0;
		UpdateHitboxes();
		ground = NULL;
		wallNormal = V2d(0, 0);
		velocity = V2d(0, 0);
		currWall = NULL;
		return true;
	}

	return false;
}

void Actor::SetBoostVelocity()
{
	velocity = normalize(velocity) * (length(velocity) + currBooster->strength);
}

void Actor::SetBounceBoostVelocity()
{
	double s = currBounceBooster->strength;
	//velocity.y = min(s, velocity.y);

	if (currBounceBooster->upOnly)
	{
		velocity.y = -s;
	}
	else
	{
		V2d dir = normalize(position - currBounceBooster->GetPosition());
		velocity = dir * s;
		velocity.x *= .6;

		if (velocity.y > 0)
			velocity.y *= .5;
	}
	
}

bool Actor::IsGoalKillAction(int a)
{
	return (a == GOALKILL || a == GOALKILL1 || a == GOALKILL2 || a == GOALKILL3 || a == GOALKILL4 || a == GOALKILLWAIT
		|| a == NEXUSKILL || a == SEQ_FLOAT_TO_NEXUS_OPENING || a == SEQ_FADE_INTO_NEXUS);
}

void Actor::QueryTouchGrass()
{
	Rect<double> queryR = hurtBody.GetAABB();//(position.x - b.rw + b.offset.x, position.y - b.rh + b.offset.y, 2 * b.rw, 2 * b.rh);
	Rect<double> queryRExtended;
	if (currHitboxes != NULL)
	{
		queryR = currHitboxes->GetAABB(currHitboxFrame);
	}

	queryRExtended = queryR;
	double extra = 300;
	queryRExtended.left -= extra;
	queryRExtended.top -= extra;
	queryRExtended.width += extra * 2;
	queryRExtended.height += extra * 2;

	possibleEdgeCount = 0;

	polyQueryList = NULL;
	queryMode = "touchgrasspoly";
	GetBorderTree()->Query(this, queryRExtended);

	queryMode = "touchgrass";
	PolyPtr tempT = polyQueryList;
	while (tempT != NULL)
	{
		tempT->QueryTouchGrass(this, queryR);
		tempT = tempT->queryNext;
	}
}

bool Actor::IsIntroAction(int a)
{
	return a == INTRO || a == SPAWNWAIT || a == INTROBOOST;
}

bool Actor::IsSequenceAction(int a)
{
	return a == SEQ_ENTERCORE1 || action == SEQ_LOOKUP || action == SEQ_KINTHROWN
		|| action == SEQ_KINFALL;
}

bool Actor::IsExitAction(int a)
{
	return a == EXIT || a == EXITWAIT || a == WAITFORSHIP || a == GRABSHIP || a == EXITBOOST;
}

void Actor::SeqAfterCrawlerFight()
{
	PoiInfo *kp = owner->poiMap["kinaftercrawlerfight"];
	assert(kp != NULL);
	ground = kp->edge;
	edgeQuantity = kp->edgeQuantity;
	facingRight = true;
	offsetX = 0;
	SetAction(Actor::STAND);
	frame = 0;
}

void Actor::AddToFlyCounter(int count)
{
	flyCounter += count;
	if (flyCounter > 100)
	{
		SetKinMode(K_SUPER);
		flyCounter = flyCounter % 100;
	}

	sess->adventureHUD->flyCountText.setString("x" + to_string(flyCounter));
}

void Actor::HandleEntrant( QuadTreeEntrant *qte )
{
	

	assert( queryMode != "" );
	if( queryMode == "resolve" )
	{
		Edge *e = (Edge*)qte;

		if (e->edgeType == Edge::OPEN_GATE)
		{
			return;
		}
		else if (e->edgeType == Edge::BARRIER)
		{
			Barrier *b = (Barrier*)(e->info);
			if (!b->edgeActive)
			{
				return;
			}
		}

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

		Contact *c = GetCollider().collideEdge( position + b.offset , b, e, tempVel, V2d( 0, 0 ) );

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

					UnlockGate( g );

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
					col = true;
					
				}
			}
		}
		
	}
	else if( queryMode == "check" )
	{
		Edge *e = (Edge*)qte;

		if (e->edgeType == Edge::OPEN_GATE)
		{
			return;
		}
		//cout << "checking: " << e << endl;
		if( (grindEdge == NULL && ground == e) || grindEdge == e )
			return;

		//Edge *testEdge = ground;
		
			
		


		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight );
		//Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		if ( action != GRINDBALL && action != GRINDATTACK )
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

		//cout << "hit edge: " << e->Normal().x << ", " << e->Normal().y << endl;
		/*if( grindEdge != NULL && (e == grindEdge->edge0 || e== grindEdge->edge1 ) )
		{
			
		}*/
		//cout << "valid is false" << endl;
		checkValid = false;

		//}
	}
	else if( queryMode == "checkwall" )
	{
		Edge *e = (Edge*)qte;
		if( ground == e )
			return;

		if( e->edgeType == Edge::OPEN_GATE )
		{
			return;
		}

		Contact *c = GetCollider().collideEdge( position + tempVel , b, e, tempVel, V2d( 0, 0 ) );
		
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
						col = true;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					col = true;
					
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
			if (g->grassType == Grass::JUMP)
			{
				++jumpGrassCount;
				touchedJumpGrass = true;
			}
			else if (g->grassType == Grass::GRAVITY)
			{
				++gravityGrassCount; //if gravity type
			}
			else if( g->grassType == Grass::BOUNCE )
			{
				++bounceGrassCount;
			}
			else if (g->grassType == Grass::BOOST)
			{
				++boostGrassCount;
				grassBoosted = true;
			}
			
		}
	}
	else if( queryMode == "envplant" )
	{
		EnvPlant *ev = (EnvPlant*)qte;
		
		if( !ev->activated )
		{
			ev->activated = true;
			if( ground != NULL )
			{
				ev->particle->dir = ground->Normal();//normalize( normalize( ground->v1 - ground->v0 ) * groundSpeed );
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
	else if (queryMode == "rail")
	{
		Edge *e = (Edge*)qte;
		RailPtr rail = e->rail;

		if ((rail->requirePower && !canRailGrind) || IsInHistunAction(action))
		{
			return;
		}

		bool canGrabRail = (rail->requirePower && canRailGrind) || (!rail->requirePower && canRailSlide);

		if ((rail != prevRail || regrindOffCount == regrindOffMax) && canGrabRail)
		{
			V2d r;
			V2d eFocus;
			bool ceiling = false;
			V2d en = e->Normal();
			if (en.y > 0)
			{
				r = e->v0 - e->v1;
				eFocus = e->v1;
				ceiling = true;
			}
			else
			{
				r = e->v1 - e->v0;
				eFocus = e->v0;
			}
		
			V2d along = normalize(r);
			double lenR = length(r);
			double q = dot(position - eFocus, along);

			if (ground != NULL)
			{
				V2d gAlong = normalize(ground->v1 - ground->v0);
				V2d rAlong = along;
				double diff = GetVectorAngleDiffCW(rAlong, gAlong);
				diff -= PI;

				bool groundTransferOkay = (diff < 0 && groundSpeed > 0) || (diff > 0 && groundSpeed < 0);
				if (!groundTransferOkay)
					return;
			}


			double c = cross(position - e->v0, along);
			double preC = cross((position - tempVel) - e->v0, along);

			double alongQuantVel = dot(velocity, along);

			bool cStuff = (c >= 0 && preC <= 0) || (c <= 0 && preC >= 0);

			if ( cStuff && q >= 0 && q <= lenR )//&& alongQuantVel != 0)
			{
				V2d rn(along.y, -along.x);
			
				
				double railSpeed;
				double minRailCurr = GetMinRailGrindSpeed();

				if (ground == NULL)
				{


					double landingSpeed = CalcLandingSpeed(velocity, along, rn, true);
					if (landingSpeed == 0)
					{
						if (facingRight)
						{
							railSpeed = minRailCurr;
						}
						else
						{
							railSpeed = -minRailCurr;
						}
					}
					else if (landingSpeed > 0)
					{
						railSpeed = max(landingSpeed, minRailCurr);
					}
					else if (landingSpeed < 0)
					{
						railSpeed = min(landingSpeed, -minRailCurr);
					}
				}
				else
				{
					railSpeed = groundSpeed;
				}

			

				if (canRailGrind)
				{
					SetAction(RAILGRIND);
				}
				else
				{
					SetAction(RAILSLIDE);
				}

				hasAirDash = true;
				hasDoubleJump = true;
				frame = 0;
				framesGrinding = 0;
				grindEdge = e;
				prevRail = (RailPtr)grindEdge->info;
				ground = NULL;
				bounceEdge = NULL;

				LineIntersection li = lineIntersection(position, position - tempVel, grindEdge->v0, grindEdge->v1);
				if (!li.parallel)
				{
					V2d p = li.position;
					grindQuantity = grindEdge->GetQuantity(p);
				}
				else
				{
					assert(0);
				}

				if (ceiling) //ceiling rail
				{
					grindSpeed = -railSpeed;
				}
				else
				{
					grindSpeed = railSpeed;
				}
			}
		}
	}
	else if (queryMode == "activeitem")
	{
		Enemy *en = (Enemy*)qte;
		if (en->type == EnemyType::EN_BOOSTER)
		{
			Booster *boost = (Booster*)qte;

			if (currBooster == NULL)
			{
				if (boost->hitBody.Intersects(boost->currHitboxFrame, &hurtBody) && boost->IsBoostable())
				{
					currBooster = boost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_BOUNCEBOOSTER)
		{
			//BounceBooster *boost = (BounceBooster*)qte;

			//if (currBounceBooster == NULL)
			//{
			//	if (boost->hitBody->Intersects(boost->currHitboxFrame, &hurtBody) && boost->IsBoostable())
			//	{
			//		currBounceBooster = boost;
			//	}
			//}
			//else
			//{
			//	//some replacement formula later
			//}
		}
		else if (en->type == EnemyType::EN_GRAVITYMODIFIER)
		{
			//GravityModifier *mod = (GravityModifier*)qte;

			//if (currModifier == NULL)
			//{
			//	if (mod->hitBody->Intersects(mod->currHitboxFrame, &hurtBody) && mod->IsModifiable())
			//	{
			//		currModifier = mod;
			//	}
			//}
			//else
			//{
			//	//some replacement formula later
			//}
		}
		else if (en->type == EnemyType::EN_GRAVITYGRASS)
		{

		}
		else if (en->type == EnemyType::EN_SPRING)
		{
			Spring *spr = (Spring*)qte;
			if (currSpring == NULL)
			{
				if (spr->hitBody.Intersects(spr->currHitboxFrame, &hurtBody) && spr->action == Spring::IDLE)
				{
					currSpring = spr;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_TELEPORTER)
		{
			Teleporter *tele = (Teleporter*)qte;
			if (currTeleporter == NULL)
			{
				if (tele->hitBody->Intersects(tele->currHitboxFrame, &hurtBody) && tele->action == Teleporter::IDLE)
				{
					currTeleporter = tele;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_SWINGLAUNCHER)
		{
			/*SwingLauncher *sw = (SwingLauncher*)qte;
			if (currSwingLauncher == NULL)
			{
				if (sw->hitBody->Intersects(sw->currHitboxFrame, &hurtBody) && sw->action == SwingLauncher::IDLE)
				{
					currSwingLauncher = sw;
				}
			}*/
		}
		else if (en->type == EnemyType::EN_HEALTHFLY)
		{
			/*HealthFly *hf = (HealthFly*)qte;
			if (hf->IsCollectible() &&
				hf->hitBody.Intersects(hf->currHitboxFrame, &hurtBody ) )
			{
				if( kinRing != NULL)
					kinRing->powerRing->Fill(hf->GetHealAmount());
				hf->Collect();
				AddToFlyCounter(hf->GetCounterAmount());
				
			}*/
		}
	}
	else if (queryMode == "airtrigger")
	{
		AirTrigger *at = (AirTrigger*)qte;
		currAirTrigger = at;
	}
	else if (queryMode == "touchgrass")
	{
		TouchGrass *tGrass = (TouchGrass*)qte;
		if (currHitboxes != NULL)
		{
			if (tGrass->Intersects(currHitboxes, currHitboxFrame))
			{
				tGrass->Destroy( this );
			}
		}
		else
		{
			if (tGrass->hurtBody.Intersects(0, &hurtBody))
			{
				tGrass->Touch(this);
			}
		}
	}
	else if (queryMode == "touchgrasspoly")
	{
		PolyPtr poly = (PolyPtr)qte;
		poly->queryNext = NULL;
		if (polyQueryList == NULL)
			polyQueryList = poly;
		else
		{
			poly->queryNext = polyQueryList;
			polyQueryList = poly;
		}
			

		/*TerrainPiece *tPiece = (TerrainPiece*)qte;
		tPiece->next = NULL;
		if (polyQueryList == NULL)
		{
			polyQueryList = tPiece;
		}
		else
		{
			tPiece->next = polyQueryList;
			polyQueryList = tPiece;
		}*/
	}
	else if (queryMode == "specialterrain")
	{
		PolyPtr poly = (PolyPtr)qte;
		Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
		{
			if (poly->IsInsideArea(position))
			{
				specialTerrainCount[poly->terrainVariation]++;//stp->specialType]++;
			}
		}

	}
	++possibleEdgeCount; //not needed
}

double Actor::CalcLandingSpeed( V2d &testVel,
		V2d &alongVel, V2d &gNorm, bool rail )
{
	//cout << "vel: " << velocity.x << ", " << velocity.y << " test: " << testVel.x << ", " << testVel.y;
	double gSpeed = 0; //groundSpeed;
	bool noLeftRight = !(currInput.LLeft() || currInput.LRight());

	double alongSpeed = dot(testVel, alongVel);

	if ((currInput.LDown() || rail ) && noLeftRight)
	{
		gSpeed = alongSpeed;
	}
	else if ((currInput.LUp() || rail ) && noLeftRight)
	{
		gSpeed = alongSpeed;
	}
	else if( ( currInput.LLeft() && testVel.x < 0 ) || ( currInput.LRight() && testVel.x > 0 ))// || currInput.LDown() || currInput.LUp() )
	{
		double res = dot( testVel, alongVel );

		if( gNorm.y <= -steepThresh )
		{
			if( testVel.x > 0 && gNorm.x < 0 )
			{
				V2d straight( 1, 0 );
				double temp = dot(testVel, straight);
				res = max( res, temp );
			}
			else if( testVel.x < 0 && gNorm.x > 0 )
			{
				V2d straight( 1, 0 );
				double temp = dot(testVel, straight);
				res = min( res, temp );
			}
		}
		gSpeed = res;
					
		//groundSpeed = 
	}
	else
	{
		if( gNorm.y > -steepThresh )
		{
			gSpeed = dot( testVel, alongVel );
		}
		else
		{
			gSpeed = 0;
		}
	}

	//cout << " gspeed: " << gSpeed << endl;
	
	return gSpeed;
}

//double Actor::CalcRailLandingSpeed(V2d &testVel,
//	V2d &alongDir, V2d &railNorm)
//{
//	//cout << "vel: " << velocity.x << ", " << velocity.y << " test: " << testVel.x << ", " << testVel.y;
//	//double rSpeed = 0; //groundSpeed;
//
//	//if (currInput.LLeft() || currInput.LRight() || currInput.LDown() || currInput.LUp())
//	//{
//	//	double res = dot(testVel, alongDir);
//
//	//	if (railNorm.y <= -steepThresh) //not steep
//	//	{
//	//		if (testVel.x > 0 && railNorm.x < 0)
//	//		{
//	//			V2d straight(1, 0);
//	//			res = max(res, dot(testVel, straight));
//	//		}
//	//		else if (testVel.x < 0 && railNorm.x > 0)
//	//		{
//	//			V2d straight(-1, 0);
//	//			res = min(res, dot(testVel, straight));
//	//		}
//	//	}
//	//	rSpeed = res;
//	//}
//	//else
//	//{
//	//	if (railNorm.y > -steepThresh)
//	//	{
//	//		rSpeed = dot(testVel, alongDir);
//	//	}
//	//	else
//	//	{
//	//		rSpeed = 0;
//	//	}
//	//}
//
//	//return rSpeed;
//}

bool Actor::IsIntangible()
{
	return kinMode == K_SUPER || invincibleFrames > 0;
}

void Actor::ApplyHit( HitboxInfo *info )
{
	if (info == NULL)
		return;
	//use the first hit you got. no stacking hits for now
	if (invincibleFrames == 0)
	{
		if (receivedHit == NULL || info->damage > receivedHit->damage)
		{
			receivedHit = info;
		}
	}
}

CollisionBody * Actor::GetBubbleHitbox(int index)
{
	if (bubbleFramesToLive[index] > 0)
	{
		return bubbleHitboxes[index];
	}
	return NULL;
}

void Actor::MiniDraw(sf::RenderTarget *target)
{
	keyExplodeRingGroup->Draw(target);
}

void Actor::Draw( sf::RenderTarget *target )
{
	//dustParticles->Draw(target);
	if (action == DEATH || action == EXITWAIT || action == SPAWNWAIT /*|| (action == INTRO && frame < 11 )*/ || action == SEQ_LOOKUPDISAPPEAR || action == SPRINGSTUNTELEPORT )
	{
		return;
	}
	//risingAuraPool->Draw(target);
	
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
	
	//auras were here before
	
	
	//testAura3->Draw(target);

	//if (action == SEQ_KNEEL || action == SEQ_KNEEL_TO_MEDITATE
	//	|| action == SEQ_MEDITATE_MASKON)
	if (showDirtyAura)
	{
		target->draw(dirtyAuraSprite);
	}


	if( bounceFlameOn && action != EXIT && !IsGoalKillAction(action) && action != BOUNCEGROUNDEDWALL && action != GRINDBALL 
		&& action != RAILGRIND )
	{
		target->draw( scorpSprite );
	}

	if (canRailGrind)
	{
		railTest.setPosition(position.x, position.y);
		//target->draw(railTest);
	}

	V2d motionGhostDir;
	double motionMagnitude = 0;
	if (ground != NULL)
	{
		motionGhostDir = -normalize(normalize( ground->v1 - ground->v0 ) * groundSpeed );
		if (reversed)
			motionGhostDir = -motionGhostDir;
		motionMagnitude = abs(groundSpeed);
	}
	else
	{
		motionGhostDir = -normalize(velocity);
		motionMagnitude = length(velocity);
	}

	//int testf = 20;

	V2d motionNormal(motionGhostDir.y, -motionGhostDir.x);

	float dist = motionGhostSpacing;

	//RotateCW( diff,);
	int showMotionGhosts = min( motionMagnitude / 1.0, 79.0 );

	//if (showMotionGhosts == 0)
	//	showMotionGhosts = 1;

	Vector2f sprPos(spriteCenter.x, spriteCenter.y );//sprite->getPosition() + diff;
	Vector2f tempPos;
	int testq = 100;
	int ttest;// = 20;

	bool fr = facingRight;
	if (ground != NULL && reversed )
	{
		fr = !fr;
	}

	for (int i = 0; i < showMotionGhosts; ++i)
	{
		if (i < 20)
		{
			ttest = 0;
		}
		else if (i < 40)
		{
			ttest = 10;
		}
		else if (i < 80)
		{
			ttest = 20;
		}
		tempPos = Vector2f(sprPos.x + motionGhostDir.x * (i * dist), sprPos.y + motionGhostDir.y * (i * dist));
		if( ttest != 0 )
			tempPos += Vector2f(motionNormal * (double)(rand() % ttest - ttest / 2) );
		//motionGhosts[i].setPosition( tempPos );

		int tf = 10;
		Vector2f ff(rand() % tf - tf / 2, rand() % tf - tf / 2);
		Vector2f ff1(rand() % tf - tf / 2, rand() % tf - tf / 2);

		motionGhostBuffer->SetPosition(i, tempPos);
		motionGhostBufferBlue->SetPosition(i, tempPos + ff);
		motionGhostBufferPurple->SetPosition(i, tempPos + ff1);

		float x = 1.f;
		if (!fr)
			x = -x;
		float y = 1.f;
		//if (reversed)
		//	y = -y;

		if (i >= 10)
		{
			//float blah = ((rand() % testq) - testq / 2) / ((float)testq / 2);
			float blah = ((rand() % testq)) / ((float)testq);
			blah /= 4.f;//2.f;
			//blah = -blah;
			//motionGhosts[i].setScale( Vector2f(1.f + blah, 1.f + blah));
			motionGhostBuffer->SetScale( i, Vector2f(x + blah, y + blah) );
			motionGhostBufferBlue->SetScale(i, Vector2f(x + blah, y + blah));
			motionGhostBufferPurple->SetScale(i, Vector2f(x + blah, y + blah));
		}
		else
		{
			motionGhostBuffer->SetScale(i, Vector2f( x, y ));
			motionGhostBufferBlue->SetScale(i, Vector2f(x, y));
			motionGhostBufferPurple->SetScale(i, Vector2f(x, y));
		}
	}

	//motionGhostShader.setUniform("textureSize", Vector2f( sprite->getTextureRect().width,
	//	sprite->getTextureRect().height ) );

	motionGhostBuffer->SetNumActiveMembers(showMotionGhosts);
	motionGhostBufferBlue->SetNumActiveMembers(showMotionGhosts);
	motionGhostBufferPurple->SetNumActiveMembers(showMotionGhosts);
	
	if( showMotionGhosts > 0 )
	{

		CubicBezier cb(.11, 1.01, .4, .96);//(.1, .82, .49, .86);//(.29, .71, .49, .86);//(.11, 1.01, .4, .96);
		float start = 128.f;
		for (int i = 0; i < showMotionGhosts; ++i)
		{
			//motionGhosts[i] = *sprite;
			float factor = (float)i / showMotionGhosts;
			float a = cb.GetValue(factor);
			int alpha = max(start - a * start, 0.f);
			//motionGhosts[i].setColor();
			Color tColor = Color::Cyan;

			float startF = 64;

			tColor.a = max(startF - a * startF, 0.f);

			Color bColor = Color::Blue;
			bColor.a = max(startF - a * startF, 0.f);

			//b500f7
			Color pColor = Color(0xb5, 0x00, 0xf7);//Color(255, 0, 255);
			//6100bd
			pColor.a = max(startF - a * startF, 0.f);
			//motionGhostBuffer->SetColor(i, sf::Color(255, 255, 255, alpha));
			//motionGhostBufferBlue->SetColor(i, sf::Color(255, 255, 255, alpha));
			//motionGhostBufferPurple->SetColor(i, sf::Color(255, 255, 255, alpha));
			motionGhostBuffer->SetColor(i, tColor);
			motionGhostBufferBlue->SetColor(i, bColor);
			motionGhostBufferPurple->SetColor(i, pColor);
		}

		//swordShader.setUniform( "isTealAlready", 0 );
		//
		//for( int i = 0; i < showMotionGhosts; ++i )
	//	{
			//float opac = .5 * ((MAX_MOTION_GHOSTS - i ) / (float)MAX_MOTION_GHOSTS);
			//swordShader.setUniform( "opacity", opac );
			//cout << "setting : " << i << endl;
			//motionGhosts[i].setColor( Color( 50, 50, 255, 50 ) );
			//motionGhosts[i].setColor( Color( 50, 50, 255, 100 * ( 1 - (double)i / showMotionGhosts ) ) );
			//target->draw( fairSword1, &swordShader );
			//target->draw(motionGhosts[i], &motionGhostShader);//&motionGhostShader );// , &swordShader );
		//}
		
		//motionGhostBuffer->Draw(target, &motionGhostShader );
		//motionGhostBuffer->Draw(target, motionGhostShader);
		//target->draw()
		

		

		
		/*{
			motionGhostBuffer->UpdateVertices();
			motionGhostBuffer->Draw(target, &motionGhostShader);
		}

		if (speedLevel > 0)
		{
			motionGhostBufferBlue->UpdateVertices();
			motionGhostBufferBlue->Draw(target, &motionGhostShader);
		}

		if (speedLevel > 1)
		{
			motionGhostBufferPurple->UpdateVertices();
			motionGhostBufferPurple->Draw(target, &motionGhostShader);
		}*/

		for (int i = speedLevel; i >= 0; --i)
		{
			motionGhostsEffects[i]->SetShader(&motionGhostShader);
			motionGhostsEffects[i]->ApplyUpdates();
			motionGhostsEffects[i]->Draw(target);
		}
		
	}
	
	if (showExitAura)
	{
		target->draw(exitAuraSprite);
		//if we add another "extra aura" should just make a single sprite for them, combining
		//this and dirtyaura
	}

	{

		
	}
	if (action == GRINDBALL || action == GRINDATTACK || action == RAILGRIND)
	{
		target->draw(*sprite, &sh);
		target->draw( gsdodeca );
		target->draw( gstriblue );
		target->draw( gstricym );
		target->draw( gstrigreen );
		target->draw( gstrioran );
		target->draw( gstripurp );
		target->draw( gstrirgb );
		target->draw( *sprite );
	}
	else
	{
		if (owner != NULL)
			flashFrames = owner->pauseFrames;
		else
			flashFrames = 0;
		if (kinMode == K_DESPERATION)
		{
			target->draw(*sprite, &playerDespShader);
		}
		else if (kinMode == K_SUPER)
		{
			target->draw(*sprite, &playerSuperShader);
		}
		else
		{
			target->draw(*sprite, &sh);
		}
		
		if (showSword)
		{
			sf::Shader &swordSh = swordShaders[speedLevel];
			//swordShader.setUniform( "isTealAlready", 1 );
			switch (action)
			{
			case FAIR:
			{
				if (flashFrames > 0)
				{
					target->draw(fairSword, &swordSh);
					//cout << "shader!" << endl;
				}
				else
				{
					target->draw(fairSword);
				}
				break;
			}
			case DAIR:
			{
				if (flashFrames > 0)
					target->draw(dairSword, &swordSh);
				else
					target->draw(dairSword);
				break;
			}
			case UAIR:
			{
				if (flashFrames > 0)
					target->draw(uairSword, &swordSh);
				else
					target->draw(uairSword);
				break;
			}
			case STANDN:
			{
				if (flashFrames > 0)
				{
					target->draw(standingNSword, &swordSh);
					//cout << "Standn" << endl;
				}
				else
					target->draw(standingNSword);
				break;
			}
			case WALLATTACK:
				if (flashFrames > 0)
					target->draw(wallAttackSword, &swordSh);
				else
					target->draw(wallAttackSword);
				break;
				break;
			case STEEPCLIMBATTACK:
				if (flashFrames > 0)
					target->draw(steepClimbAttackSword, &swordSh);
				else
					target->draw(steepClimbAttackSword);
				break;
				break;
			case STEEPSLIDEATTACK:
				if (flashFrames > 0)
					target->draw(steepSlideAttackSword, &swordSh);
				else
					target->draw(steepSlideAttackSword);
				break;
				break;
			case GRINDSLASH:
			{

				if (flashFrames > 0)
					target->draw(grindLungeSword, &swordSh);
				else
					target->draw(grindLungeSword);
				break;

			}
			case DIAGUPATTACK:
			{
				if (flashFrames > 0)
				{
					target->draw(diagUpAttackSword, &swordSh);
					//cout << "shader!" << endl;
				}
				else
				{
					target->draw(diagUpAttackSword);
				}
				break;
			}
			case DIAGDOWNATTACK:
			{
				if (flashFrames > 0)
				{
					target->draw(diagDownAttackSword, &swordSh);
					//cout << "shader!" << endl;
				}
				else
				{
					target->draw(diagDownAttackSword);
				}
				break;
			}
			break;
			}
		}
	}
	
	for( int i = 0; i < maxBubbles; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			bubbleSprite.setTextureRect(ts_bubble->GetSubRect(0));//bubbleFramesToLive[i] % 11 ) );
			//this isnt going to be final anyway
			//cout << "drawing " << i << ": " << bubbleRadiusSize[i] << endl;
			double fac = ((double)bubbleRadiusSize[i]) / 128.0;
			//cout << "fac: " << fac << endl;
			bubbleSprite.setScale( fac * 2, fac * 2 );
			//bubbleSprite.setScale( 2, 2 );
			bubbleSprite.setOrigin( bubbleSprite.getLocalBounds().width / 2, bubbleSprite.getLocalBounds().height / 2 );
			bubbleSprite.setPosition( bubblePos[i].x, bubblePos[i].y );
			bubbleSprite.setColor( Color( 255, 255, 255, 100 ) );
			//CircleShape cs;
		//	cs.setFillColor( sf::Color::Transparent );
			//cs.setRadius( 

			target->draw( bubbleSprite );// &timeSlowShader );
		}
	}
	
	for (int i = 0; i < 3; ++i)
	{
		fairLightningPool[i]->Draw(target);
		dairLightningPool[i]->Draw(target);
		uairLightningPool[i]->Draw(target);
	}

	gateBlackFXPool->Draw(target);

	for (int i = 0; i < 7; ++i)
	{
		smallLightningPool[i]->Draw(target);
	}

	keyExplodeRingGroup->Draw(target);
	keyExplodePool->Draw(target);
}

void Actor::DeathDraw(sf::RenderTarget *target)
{
	if (action == DEATH)
	{
		target->draw(*sprite);// , &sh);
	}
}

void Actor::DrawMapWires(sf::RenderTarget *target)
{
	if ((action != Actor::GRINDBALL && action != Actor::GRINDATTACK))
	{
		rightWire->DrawMinimap(target);
		leftWire->DrawMinimap(target);
	}
}

void Actor::GrabShipWire()
{
	SetAction(GRABSHIP);
	ground = NULL;
	frame = 0;
}



void Actor::ShipPickupPoint( double eq, bool fr )
{
	if( action != WAITFORSHIP && action != GRABSHIP )
	{
		owner->totalFramesBeforeGoal = owner->totalGameFrames;
		SetKinMode(K_NORMAL);
		SetExpr(KinMask::Expr_NEUTRAL);
		owner->scoreDisplay->Activate();
		SetAction(WAITFORSHIP);
		frame = 0;
		assert( ground != NULL );
		edgeQuantity = eq;
		groundSpeed = 0;
		facingRight = fr;

		if( ground->Normal().y == -1 )
		{
			offsetX = 0;
		}
	}
}

void Actor::EnterNexus( int nexusIndex, sf::Vector2<double> &pos )
{
	if( nexusIndex == 0 )
	{
		SetAction(ENTERNEXUS1);
		frame = 0;
		position = pos;
		velocity = V2d( 0, 0 );
	}
	
}

void Actor::DodecaLateDraw(sf::RenderTarget *target)
{
	int dodecaFactor = 1;
	for( int i = 0; i < maxBubbles; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			int trueFrame = bubbleLifeSpan - ( bubbleFramesToLive[i] - 1);
			if( trueFrame / dodecaFactor - 9 < 12 )
			{
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
}

void Actor::DebugDraw( RenderTarget *target )
{
	if( currHitboxes != NULL )
	{
		currHitboxes->DebugDraw( currHitboxFrame, target);
	}

	//hurtBody.DebugDraw(target)

	b.DebugDraw( CollisionBox::Physics, target);

	leftWire->DebugDraw( target );
	rightWire->DebugDraw( target );

	DebugDrawComboObj(target);

}

void Actor::SetFakeCurrInput( ControllerState &state )
{
	currInput = state;
}

int Actor::GetJumpFrame()
{
	sf::IntRect ir;
	int tFrame = -1;
	

	if (velocity.y < -15)
	{
		tFrame = 1;
	}
	else if (velocity.y < -6)
	{
		tFrame = 2;
	}
	else if (velocity.y < 2)
	{
		tFrame = 3;
	}
	else if (velocity.y < 7)
	{
		tFrame = 4;
	}
	else if (velocity.y < 9)
	{
		tFrame = 5;
	}
	else if (velocity.y < 12)
	{
		tFrame = 6;
	}
	else if (velocity.y < 16)
	{
		tFrame = 7;
	}
	else if (velocity.y < 20)
	{
		tFrame = 8;
	}
	else if (velocity.y < 26)
	{
		tFrame = 9;
	}
	else if (velocity.y < 30)
	{
		tFrame = 10;
	}
	else if (velocity.y < 35)
	{
		tFrame = 11;
	}
	else if (velocity.y < 37)
	{
		tFrame = 12;
	}
	else if (velocity.y < 40)
	{
		tFrame = 13;
	}
	else
	{
		tFrame = 14;
	}

	//if (tFrame = -1)
	//	tFrame = 14;

	//assert(tFrame != -1);
	return tFrame;
}

bool Actor::IsMovingRight()
{
	if (ground == NULL && grindEdge == NULL && bounceEdge == NULL)
	{
		if (velocity.x > 0)
		{
			return true;
		}
	}
	else if (ground != NULL)
	{
		if ((!reversed && groundSpeed > 0) || (reversed && groundSpeed < 0))
		{
			return true;
		}
	}
	
	return false;
	//fill out grind and bounce later
}

bool Actor::IsMovingLeft()
{
	if (ground == NULL && grindEdge == NULL && bounceEdge == NULL)
	{
		if (velocity.x < 0)
		{
			return true;
		}
	}
	else if (ground != NULL)
	{
		if ((!reversed && groundSpeed < 0) || (reversed && groundSpeed > 0))
		{
			return true;
		}
	}

	return false;
	//fill out grind and bounce later
}

void Actor::SetDirtyAura(bool on)
{
	showDirtyAura = on;
}

void Actor::UpdateSprite()
{
	scorpSet = false;
	showExitAura = false;

	if (ground != NULL)
	{
		EffectInstance params;
		params.SetParams(Vector2f(ground->GetPosition(edgeQuantity)),
			Transform(Transform::Identity), 1, 20, 0);

		Vector2f v(0, -.2);
		Transform tr;

		int r = (rand() % 20) - 10;
		tr.rotate(r);
		
		
		params.SetVelocityParams(tr.transformPoint( v ), Vector2f(0, 0), 1 );
		//dustParticles->ActivateEffect(&params);
		float zz = .2;
		v = Vector2f(normalize(ground->v1 - ground->v0)) * zz;//Vector2f(zz, 0);
		
		params.SetVelocityParams(tr.transformPoint(v), Vector2f(0, 0), 1);
		//dustParticles->ActivateEffect(&params);
		v = -Vector2f(normalize(ground->v1 - ground->v0)) * zz;
		params.SetVelocityParams(tr.transformPoint(v), Vector2f(0, 0), 1);
		//dustParticles->ActivateEffect(&params);
		
	}

	//dustParticles->Update(&rpu);


	//V2d gn( 0, 0 );
	if( ground != NULL )
	{
		currNormal = ground->Normal();
	}
	else
	{
		bool r = rightWire->state == Wire::PULLING;
		bool l = leftWire->state == Wire::PULLING;
		if( r && l && doubleWireBoost )
		{
			
			//create double wire boost
		}
		else if( r && rightWireBoost )
		{
			ActivateEffect( EffectLayer::BEHIND_ENEMIES, ts_fx_rightWire, position, false, 
				atan2( rightWireBoostDir.y, rightWireBoostDir.x ), 8, 2, true );
			//create right wire boost
		}
		else if( l && leftWireBoost )
		{
			ActivateEffect( EffectLayer::BEHIND_ENEMIES, ts_fx_leftWire, position, false, 
				atan2( leftWireBoostDir.y, leftWireBoostDir.x ), 8, 2, true );
			//create left wire boost
		}
	}

	

	UpdateActionSprite();
	

	Vector2f oldOrigin = sprite->getOrigin();
	Vector2f center(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	Vector2f diff = center - oldOrigin;
	RotateCW(diff, sprite->getRotation() / 180.f * PI);
	spriteCenter = V2d(sprite->getPosition() + diff);

	
	//Vector2f extraParticle0(rand() % t - t / 2, rand() % t - t / 2);
	Vector2f extraParticle0;
	int t = 2;
	Vector2f extraParticle1(rand() % t - t / 2, rand() % t - t / 2);
	t = 10;
	Vector2f extraParticle2(rand() % t - t / 2, rand() % t - t / 2);

	bool fr = facingRight;
	if (reversed)
		fr = !fr;

	Transform tr;
	tr.rotate(sprite->getRotation());
	if (!fr)
	{
		tr.scale(Vector2f(-1, 1));
	}
	
	if( scorpOn && !scorpSet )
	{
		if( ground != NULL )
		{
			
		}
		else
		{
		}
	}

	if( false )//bounceFlameOn )
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
	/*if( e->hasMonitor && !e->suppressMonitor )
	{
		ActivateSound( soundBuffers[S_HIT_AND_KILL_KEY] );
	}
	else
	{
		ActivateSound( soundBuffers[S_HIT_AND_KILL] );
	}*/
	enemiesKilledThisFrame++;


	//for the growing tree
	//wrong
}

void Actor::ConfirmEnemyNoKill( Enemy *e )
{
	//cout << "hit sound" << endl;
	ActivateSound( S_HIT );
}

void Actor::ConfirmHit( Enemy *e )
{

	HitParams &hitParams = e->hitParams;
	//owner->cam.SetRumble(3, 3, 5);

	if (!hitParams.canBeHit)
		return;

	if (ground == NULL && velocity.y > 0 && action == DAIR )
	{
		velocity.y = 4;//.5;
	}

	Color c;
	switch(e->world )
	{
	case 1:
		c = COLOR_BLUE;
		break;
	case 2:
		c = COLOR_GREEN;
		break;
	case 3:
		c = COLOR_YELLOW;
		break;
	case 4:
		c = COLOR_ORANGE;
		break;
	case 5:
		c = COLOR_RED;
		break;
	case 6:
		c = COLOR_MAGENTA;
		break;
	case 7:
		c = Color::Black;
		break;
	}

	currentSpeedBar += hitParams.speedBar;
	hitEnemyDuringPhyiscs = true;
	currAttackHit = true;
	if( bounceFlameOn )
	{
		bounceAttackHit = true;
	}
	else
	{
		bounceAttackHit = false;
	}


	flashColor = c;	
	//flashFrames = hitParams->flashFrames + 1;
	for( int i = 0; i < 3; ++i )
	{
		swordShaders[i].setUniform( "toColor", ColorGL( flashColor ) );
	}

	//owner->powerWheel->Charge( charge );

	if( kinRing != NULL )
		kinRing->powerRing->Fill(hitParams.charge);
	
	if (kinMode == K_DESPERATION)
	{
		SetKinMode(K_NORMAL);
	}
	
	
	hasDoubleJump = true;
	hasAirDash = true;
	/*switch (action)
	{
	case UAIR:
		if( velocity.y <= 0 )
			hasDoubleJump = true;
		break;
	case DAIR:
		dairBoostedDouble = true;
		break;
	}*/

	double slowDownFall = 14;
	if (velocity.y > slowDownFall)
	{
		velocity.y = slowDownFall;
	}
}

//void Actor::ConfirmWireHit( bool right )
//{
//
//}


void Actor::UnlockGate(Gate *g)
{
	owner->UnlockGate(g);
	
	if (rightWire != NULL)
		rightWire->Retract();

	if (leftWire != NULL)
		leftWire->Retract();
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
	V2d gn;
	if( ground != NULL )
	{
		gn = ground->Normal();
		//return 0;
	}
	else if( grindEdge != NULL )
	{
		gn = grindEdge->Normal();
		if (gn.y > 0)
		{
			gn = -gn;
		}
	}
	else
	{
		return 0;
	}
	
	//cout << "gn: " << gn.x << ", " << gn.y << endl;

	double angle = 0;
	
	bool extraCase = false;
	
	if (ground != NULL)
	{
		if (!reversed)
		{
			V2d e0n = ground->edge0->Normal();
			V2d e1n = ground->edge1->Normal();
			bool a = (offsetX > 0 && approxEquals(edgeQuantity, 0) && e0n.x < 0);
			bool b = (offsetX < 0 && approxEquals(edgeQuantity, length(ground->v1 - ground->v0)) && e1n.x > 0);
			extraCase = a || b;
			//cout << "extra: " << extraCase << endl;
		}
		else
		{
			V2d e0n = ground->edge0->Normal();
			V2d e1n = ground->edge1->Normal();
			bool a = (offsetX > 0 && approxEquals(edgeQuantity, 0) && e0n.x < 0);
			bool b = (offsetX < 0 && approxEquals(edgeQuantity, length(ground->v1 - ground->v0)) && e1n.x > 0);
			extraCase = a || b;
			//cout << "extraCSe : " << a <<", " << b << ", edge: " << edgeQuantity << ", " << length( ground->v1 - ground->v0 )
			//	 << ", " << approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) << endl;
		}
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
	//wtf is the extra case??? its bugging
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
	extraCase = false;
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

void Actor::SetSpriteTexture( int a )
{
	spriteAction = a;
	Tileset *ts = tileset[a];
	sprite->setTexture( *ts->texture );
	//orbSprite.setTexture( )
	motionGhostBuffer->SetTileset(ts);
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetTileset(ts);
	}
	
	motionGhostBufferBlue->SetTileset(ts);
	motionGhostBufferPurple->SetTileset(ts);

	//const auto &size = ts->texture->getSize();

}

void Actor::SetAerialScorpSprite()
{
	assert(scorpOn);
	int tFrame = GetJumpFrame();
	scorpSprite.setTexture(*ts_scorpJump->texture);

	SetSpriteTile(&scorpSprite, ts_scorpJump, tFrame, facingRight);

	scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
		scorpSprite.getLocalBounds().height / 2);
	scorpSprite.setPosition(position.x, position.y);
	scorpSprite.setRotation(sprite->getRotation());
	scorpSet = true;
}

void Actor::SetSpriteTile( int tileIndex, bool noFlipX, bool noFlipY )
{
	currTileIndex = tileIndex;

	Tileset *ts = tileset[spriteAction];
	IntRect ir = ts->GetSubRect( currTileIndex );

	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	if (kinMode == K_NORMAL)
	{
		sh.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
			(ir.left + ir.width) / width, (ir.top + ir.height) / height));
		static float testCounter = 0;
		sh.setUniform("u_slide", testCounter);
		testCounter += .01f;
	}
	else if( kinMode == K_SUPER )
	{
		playerSuperShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
			(ir.left + ir.width) / width, (ir.top + ir.height) / height));
	}

	
	
	/*if (testCounter > 1.f)
	{
		testCounter -= 1.f;
	}*/


	if( !noFlipX )
	{
		flipTileX = true;
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	else
	{
		flipTileX = false;
	}

	if( !noFlipY )
	{
		flipTileY = true;
		ir.top += ir.height;
		ir.height = -ir.height;
	}
	else
	{
		flipTileY = false;
	}

	


	CubicBezier cb(.11, 1.01, .4, .96);
	Color cols[3] = { Color::Cyan, Color::Blue, Color::Magenta };//Color(100, 0, 255) };
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetTileset(tileset[spriteAction]);
		motionGhostsEffects[i]->SetTile(currTileIndex);
		motionGhostsEffects[i]->SetFacing(facingRight, reversed);
		motionGhostsEffects[i]->SetDistInBetween(1.f);
		motionGhostsEffects[i]->SetScaleParams(CubicBezier(), .25 + i * .15, 0, 10);
		motionGhostsEffects[i]->SetVibrateParams(CubicBezier(), 20 + i * 4, 10);
		Color t(cols[i]);
		t.a = 100;
		Color b(cols[i]);//(Color::Blue);
		b.a = 10;

		motionGhostsEffects[i]->SetColorGradient(t, b, cb);
	}
	
	//t.r = rand() % 255;
	//t.g = rand() % 255;
	//t.b = rand() % 255;

	//b.r = rand() % 255;
	//b.g = rand() % 255;
	//b.b = rand() % 255;

	//t.a = rand() % 255;//80;
	//b.a = rand() % 255;//20;
	
	
	
	//	CubicBezier());
	

	for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhostBuffer->SetTile(i, currTileIndex);
		motionGhostBufferBlue->SetTile(i, currTileIndex);
		motionGhostBufferPurple->SetTile(i, currTileIndex);
	}
	
	sprite->setTextureRect( ir );
}

void Actor::SetSpriteTile( sf::Sprite *spr, 
		Tileset *tss, int tileIndex, bool noFlipX, bool noFlipY )
{
	//if (spr == sprite)
	//{
	//	//inefficient
	//	for (int i = 0; i < maxMotionGhosts; ++i)
	//	{
	//		motionGhostBuffer->SetTile( i, tileIndex);
	//	}
	//	
	//}
	IntRect ir = tss->GetSubRect( tileIndex );
	if( !noFlipX )
	{
		flipTileX = true;
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	else
	{
		flipTileX = false;
	}

	if( !noFlipY )
	{
		flipTileY = true;
		ir.top += ir.height;
		ir.height = -ir.height;
	}
	else
	{
		flipTileY = false;
	}


	spr->setTextureRect( ir );

	int t = 4;
	//Vector2f extraParticle0(rand() % t - t / 2, rand() % t - t / 2);
	Vector2f extraParticle0(0, 0);
	Vector2f extraParticle1(rand() % t - t / 2, rand() % t - t / 2);
	Vector2f extraParticle2(rand() % t - t / 2, rand() % t - t / 2);

	bool fr = facingRight;
	if (reversed)
		fr = !fr;

	Transform tr;
	tr.rotate(sprite->getRotation());
	if (!fr)
	{
		tr.scale(Vector2f(-1, 1));
	}
}

void Actor::SetupAction(int a)
{
}

void Actor::AirMovement()
{
	if( leftWire->state == Wire::PULLING || rightWire->state == Wire::PULLING )
	{
	}
	else
	{
		double dSpeed = GetDashSpeed();
		if( currInput.LLeft() )
		{
			if( velocity.x > dSpeed )
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
			if( velocity.x < -dSpeed )
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
			if( velocity.x > dSpeed )
			{
				velocity.x -= airSlow;
				if( velocity.x < dSpeed ) 
				{
					velocity.x = dSpeed;
				}
			}
			else if( velocity.x > 0 )
			{
				velocity.x += -airSlow;
				if( velocity.x < 0 ) velocity.x = 0;
				else if( velocity.x >= -dSpeed )
				{
					velocity.x = 0;
				}
				//velocity.x = 0;
			}
			else if( velocity.x < -dSpeed )
			{
				velocity.x += airSlow;
				if( velocity.x > -dSpeed ) 
				{
					velocity.x = -dSpeed;
				}
			}
			else if( velocity.x < 0 )
			{
				velocity.x += airSlow;
				if( velocity.x > 0 ) velocity.x = 0;
				else if( velocity.x >= -dSpeed )
				{
					velocity.x = 0;
				}
			}
		}
	}
}

void Actor::DrawWires(sf::RenderTarget *target)
{
	if (HasUpgrade(UPGRADE_POWER_LWIRE) &&
		((action != Actor::GRINDBALL && action != Actor::GRINDATTACK)
			|| leftWire->state == Wire::RETRACTING))
	{
		leftWire->Draw(target);
	}
	if (HasUpgrade(UPGRADE_POWER_RWIRE) &&
		((action != Actor::GRINDBALL && action != Actor::GRINDATTACK)
			|| rightWire->state == Wire::RETRACTING))
	{
		rightWire->Draw(target);
	}
}

void Actor::UpdateWireQuads()
{
	if (HasUpgrade(UPGRADE_POWER_LWIRE))
		leftWire->UpdateQuads();

	if (HasUpgrade(UPGRADE_POWER_RWIRE))
		rightWire->UpdateQuads();
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

	offset = Vector2i( 0, 4.9 );

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

void Actor::SetAutoRun(bool fr, double maxAutoRun)
{
	SetAction(AUTORUN);
	frame = 0;
	autoRunStopEdge = NULL;

	maxAutoRunSpeed = maxAutoRun;
	facingRight = fr;
}

void Actor::BrakeMovement()
{
	double brakeAmount = 1.0;
	double dSpeed = dashSpeed;
	if (groundSpeed > 0)
	{
		groundSpeed -= brakeAmount;
	}
	else if (groundSpeed < 0)
	{
		groundSpeed += brakeAmount;
	}
}

void Actor::RunMovement()
{
	if (action != AUTORUN)
	{
		if (currInput.LLeft())
			facingRight = false;
		else if (currInput.LRight())
			facingRight = true;
	}
	

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
		
		GroundExtraAccel();

		if (action == AUTORUN)
		{
			if (-groundSpeed > maxAutoRunSpeed)
			{
				groundSpeed = -maxAutoRunSpeed;
			}
		}
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

		GroundExtraAccel();

		if (action == AUTORUN)
		{
			if (groundSpeed > maxAutoRunSpeed)
			{
				groundSpeed = maxAutoRunSpeed;
			}
		}
	}
}

void Actor::AttackMovement()
{
	double dSpeed = GetDashSpeed();
	if( currInput.LLeft() )
	{
		if( groundSpeed > 0 )
		{
			if( currInput.B )
			{
				groundSpeed = -dSpeed;
			}
			else
			{
				groundSpeed = 0;
			}
		}
		else
		{
			if( groundSpeed > -dSpeed && currInput.B )
			{
				groundSpeed = -dSpeed;
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
				groundSpeed = dSpeed;
			}
			else
			{
				groundSpeed = 0;
			}
		}
		else
		{
			if( groundSpeed < dSpeed && currInput.B )
			{
				groundSpeed = dSpeed;
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
	BounceFlameOff();

	//double gSpeed = groundSpeed;
	//if( reversed )
	//	gSpeed = -gSpeed;
	//groundSpeed = 10;
	double dSpeed = GetDashSpeed();
	if( groundSpeed == 0 )
	{
		if( facingRight )
		{

			//cout << "dashspeed" << endl;
			grindSpeed = dSpeed;
			//if( reversed )
			//	grindSpeed = -dashSpeed;
		}
		else
		{
			grindSpeed = -dSpeed;
			//if( reversed )
			//	grindSpeed = dashSpeed;
		}
	}
	else if( groundSpeed > 0 )
	{
		grindSpeed = std::min( maxGroundSpeed + scorpAdditionalCap, std::max( groundSpeed, dSpeed ) );
	}
	else
	{
		grindSpeed = std::max( -maxGroundSpeed - scorpAdditionalCap, std::min( groundSpeed, -dSpeed ) );
	}
	


	framesGrinding = 0;
	if( rightWire != NULL )
		rightWire->Retract();

	if( leftWire != NULL )
		leftWire->Retract();
	//rightWire->Reset();
	//leftWire->Reset();
	SetAction(GRINDBALL);
	grindEdge = ground;
	frame = 0;
	grindQuantity = edgeQuantity;


	if (grindHitboxes[0] != NULL)
	{
		double grindHitRadius[] = { 90, 100, 110 };
		CollisionBox &gh = grindHitboxes[0]->GetCollisionBoxes(0).front();
		gh.rw = gh.rh = grindHitRadius[speedLevel];
	}
	
	hurtBody.isCircle = true;
	double grindHurtRadius = 40;
	hurtBody.rw = grindHurtRadius;
	hurtBody.rh = grindHurtRadius;

		
	ground = NULL;
	bounceEdge = NULL;

	if( reversed )
	{
	//	grindSpeed = -grindSpeed;
	}
}

bool Actor::CanUnlockGate( Gate *g )
{
	return g->CanUnlock();
}

void Actor::SetExpr( int ex )
{
	if (kinMask != NULL)
	{
		kinMask->SetExpr((KinMask::Expr)ex);
	}
}

void Actor::ExecuteDoubleJump()
{
	framesSinceDouble = 0;

	//add direction later
	ActivateEffect(EffectLayer::IN_FRONT, ts_fx_double,
		V2d(position.x, position.y - 20), false, 0, 14, 2, facingRight);

	//velocity = groundSpeed * normalize(ground->v1 - ground->v0 );
	if (velocity.y > 0)
		velocity.y = 0;

	if (action == BACKWARDSDOUBLE)
	{
		velocity.y = -backDoubleJumpStrength;
	}
	else
	{
		velocity.y = -doubleJumpStrength;
	}

	extraDoubleJump = false;
	hasDoubleJump = false;

	if (currInput.LLeft())
	{
		if (velocity.x > -maxRunInit)
		{
			velocity.x = -maxRunInit;
		}
	}
	else if (currInput.LRight())
	{
		if (velocity.x < maxRunInit)
		{
			velocity.x = maxRunInit;
		}
	}
	else
	{
		velocity.x = 0;
	}

	if (aerialHitCancelDouble)
	{
		if (cancelAttack == FAIR)
		{
			if (facingRight && currInput.LLeft()
				|| (!facingRight && currInput.LRight()))
			{
				if (facingRight)
				{
					velocity.x = -GetDashSpeed();
				}
				else
				{
					velocity.x = GetDashSpeed();
				}
				facingRight = !facingRight;
			}
		}
		else if (cancelAttack == DAIR || cancelAttack == DIAGDOWNATTACK)
		{
			velocity.y -= dairBoostVel;
			if (cancelAttack == DIAGDOWNATTACK)
			{
				if (facingRight && currInput.LLeft()
					|| (!facingRight && currInput.LRight()))
				{
					if (facingRight)
					{
						velocity.x = -GetDashSpeed();
					}
					else
					{
						velocity.x = GetDashSpeed();
					}
				}
			}
		}
	}

	if( action == DOUBLE)
	{
		ActivateSound(S_DOUBLE);
	}
	else if( action == BACKWARDSDOUBLE)
	{
		ActivateSound(S_DOUBLEBACK);
	}
}

void Actor::ExecuteWallJump()
{
	wallJumpFrameCounter = 0;
	double strengthX = wallJumpStrength.x;
	double strengthY = wallJumpStrength.y;

	if (touchedJumpGrass)
	{
		strengthY += 10;
	}

	if (facingRight)
	{
		velocity.x = strengthX;
	}
	else
	{
		velocity.x = -strengthX;
	}

	double movingVertStrength = strengthY * .5; //for when you're moving up already
	velocity.y = min(velocity.y - movingVertStrength, -strengthY);


	ActivateSound(S_WALLJUMP);

	V2d fxPos = position;
	if (facingRight)
	{
		fxPos += V2d(0, 0);
	}
	else
	{
		fxPos += V2d(0, 0);
	}

	switch (speedLevel)
	{
	case 0:
		ActivateEffect(EffectLayer::IN_FRONT, ts_fx_wallJump[0], fxPos, false, 0, 7, 3, facingRight);
		break;
	case 1:
		ActivateEffect(EffectLayer::IN_FRONT, ts_fx_wallJump[1], fxPos, false, 0, 7, 3, facingRight);
		break;
	case 2:
		ActivateEffect(EffectLayer::IN_FRONT, ts_fx_wallJump[2], fxPos, false, 0, 7, 3, facingRight);
		break;
	}
}

int Actor::GetDoubleJump()
{
	if( (facingRight && currInput.LLeft()) || ( !facingRight && currInput.LRight() ) )
	{
		if (action == FAIR)
		{
			return DOUBLE;
		}
		else
		{
			return BACKWARDSDOUBLE;
		}
	}
	else
	{
		return DOUBLE;
	}
}

bool Actor::CanDoubleJump()
{
	//cout << "prevInput.A: " << (int)(prevInput.A) << endl;
	return ( (hasDoubleJump || extraDoubleJump ) && 
		((currInput.A && !prevInput.A) || pauseBufferedJump || stunBufferedJump )  && !IsSingleWirePulling() );
}

bool Actor::IsDoubleWirePulling()
{
	return ( rightWire->state == Wire::PULLING && leftWire->state == Wire::PULLING );
}

bool Actor::TryDoubleJump()
{
	if( CanDoubleJump() )
	{
		
		aerialHitCancelDouble = IsAttackAction(action);
		if (aerialHitCancelDouble)
		{
			cancelAttack = action;
		}
		dairBoostedDouble = (action == DAIR || action == UAIR || action == DIAGDOWNATTACK || action == DIAGUPATTACK );
		SetAction( GetDoubleJump() );

		if (currInput.rightShoulder && !pauseBufferedJump && !stunBufferedJump)
		{
			if (currInput.LUp())
			{
				doubleJumpBufferedAttack = UAIR; //none
			}
			else if (currInput.LDown())
			{
				doubleJumpBufferedAttack = DAIR;
			}
			else
			{
				doubleJumpBufferedAttack = FAIR;
			}
		}
		else
		{
			doubleJumpBufferedAttack = DOUBLE;
		}

		return true;
	}
	
	return false;
}

bool Actor::TryAirDash()
{
	if (HasUpgrade(UPGRADE_POWER_AIRDASH) && !IsSingleWirePulling())
	{
		if ((hasAirDash || inBubble) && ((!prevInput.B && currInput.B) || pauseBufferedDash
			|| stunBufferedDash ))
		{
			hasFairAirDashBoost = (action == FAIR);
			SetAction( AIRDASH );
			return true;
		}
	}

	return false;
}

bool Actor::TryGlide()
{
	bool ad = (HasUpgrade(UPGRADE_POWER_AIRDASH) && !hasAirDash) 
		|| !HasUpgrade(UPGRADE_POWER_AIRDASH);
	if (!prevInput.B && currInput.B)
	{
		SetAction(GLIDE);
		frame = 0;
		return true;
	}
	return false;
}
//you can pull with both or neither to return false. pulling with a single wire will return true;
bool Actor::IsSingleWirePulling()
{
	return ( ( rightWire->state == Wire::PULLING || leftWire->state == Wire::PULLING )
		&& !IsDoubleWirePulling() );
}

bool Actor::IHitPlayer( int otherPlayerIndex )
{
	Actor *player = owner->GetPlayer( otherPlayerIndex );
	
	if( !player->IsIntangible() )
	{
		if( currHitboxes != NULL )
		{
			bool hit = false;

			vector<CollisionBox> *cList = &(currHitboxes->GetCollisionBoxes(currHitboxFrame));
			if( cList != NULL )
			for( auto it = cList->begin(); it != cList->end(); ++it )
			{
				if( player->hurtBody.Intersects( (*it) ) )
				{
					hit = true;
					break;
				}
			}

			if( hit )
			{
				//receivedHit = player->currHitboxInfo;
				return true;
			}
		}
		//if( player->position.x < position.x )
		//{
		//	hitboxInfo->kbDir.x = -abs( hitboxInfo->kbDir.x );
		//	//cout << "left" << endl;
		//}
		//else if( player->position.x > position.x )
		//{
		//	//cout << "right" << endl;
		//	hitboxInfo->kbDir.x = abs( hitboxInfo->kbDir.x );
		//}
		//else
		//{
		//	//dont change it
		//}
		//attackFrame = 0;
		//player->ApplyHit( hitboxInfo );
		//return true;
	}
	
	return false;
}

void Actor::ClearPauseBufferedActions()
{
	pauseBufferedJump = false;
	pauseBufferedAttack = Action::Count;
	pauseBufferedDash = false;
}

bool Actor::IsAttackAction( int a )
{
	return (a == FAIR || a == DAIR || a == UAIR || a == STANDN || a == DIAGDOWNATTACK
		|| a == DIAGUPATTACK || a == WALLATTACK || a == STEEPCLIMBATTACK || a == STEEPSLIDEATTACK );
}

bool Actor::IsGroundAttackAction(int a)
{
	return (a == STANDN || a == STEEPCLIMBATTACK || a == STEEPSLIDEATTACK);
}

bool Actor::IsSpringAction(int a)
{
	return a == SPRINGSTUN || a == SPRINGSTUNGLIDE || a == SPRINGSTUNBOUNCE || a == SPRINGSTUNAIRBOUNCE
		|| a == SPRINGSTUNTELEPORT || a == SPRINGSTUNAIRBOUNCE;
}

bool Actor::IsOnRailAction(int a)
{
	return a == RAILGRIND || a == RAILSLIDE;
}

bool Actor::IsInHistunAction( int a )
{
	return a == GROUNDHITSTUN || a == AIRHITSTUN;
}

V2d Actor::GetKnockbackDirFromVel()
{
	if (ground != NULL)
	{
		return normalize(-groundSpeed * (ground->v1 - ground->v0));
	}
	else
	{
		return normalize(-velocity);
	}
}

V2d Actor::GetTrueVel()
{
	if (grindEdge != NULL)
	{
		return grindEdge->Along() * grindSpeed;
	}
	else if (ground != NULL)
	{
		return ground->Along() * groundSpeed;
	}
	else
	{
		return velocity;
	}
}

void Actor::RestoreDoubleJump()
{
	if( !hasDoubleJump )
		extraDoubleJump = true;
	//hasDoubleJump = true;
}

void Actor::RestoreAirDash()
{
	hasAirDash = true;
}

void Actor::UpdateInHitlag()
{
	if (IsAttackAction(action) && currAttackHit )
	{
		if (pauseBufferedAttack == Action::Count)
		{
			if (currInput.rightShoulder && !prevInput.rightShoulder)
			{
				if (ground == NULL)
				{
					if (currInput.LUp())
					{
						pauseBufferedAttack = UAIR;
					}
					else if (currInput.LDown())
					{
						pauseBufferedAttack = DAIR;
					}
					else
					{
						pauseBufferedAttack = FAIR;
					}
				}
				else
				{
					if (action == STEEPCLIMBATTACK)
						pauseBufferedAttack = STEEPCLIMBATTACK;
					else if (action == STEEPSLIDEATTACK)
					{
						pauseBufferedAttack = STEEPSLIDEATTACK;
					}
					else
					{
						pauseBufferedAttack = STANDN;
					}
				}
			}
		}

		if (!pauseBufferedJump && currInput.A && !prevInput.A )
		{
			pauseBufferedJump = true;
		}

		if (!pauseBufferedDash && currInput.B && !prevInput.B)
		{
			if( ground != NULL || ( ground == NULL && hasAirDash ) )
				pauseBufferedDash = true;
		}
	}	

	--flashFrames;
}

 pair<bool, bool> Actor::PlayerHitMe( int otherPlayerIndex )
{
	Actor *player = NULL;
	switch( otherPlayerIndex )
	{
	case 0:
		player = owner->GetPlayer( 0 );
		break;
	case 1:
		player = owner->GetPlayer( 0 );
		break;
	}

	if( player->currHitboxes != NULL )
	{
		bool hit = false;


		/*for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}*/
		
		//receivedHit = player->currHitboxInfo;
		return pair<bool, bool>(true,false);
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

 void Actor::CollectFly(HealthFly *hf)
 {
	 if (kinRing != NULL)
		 kinRing->powerRing->Fill(hf->GetHealAmount());
	 hf->Collect();
	 AddToFlyCounter(hf->GetCounterAmount());
 }


MotionGhostEffect::MotionGhostEffect( int maxGhosts )
	:shader( NULL ), ts( NULL )
{
	motionGhostBuffer = new VertexBuf( maxGhosts, sf::Quads);
}

MotionGhostEffect::~MotionGhostEffect()
{
	delete motionGhostBuffer;
}

void MotionGhostEffect::SetDistInBetween(float dist)
{
	distInBetween = dist;
}

void MotionGhostEffect::SetSpread(int p_numGhosts, Vector2f &p_dir, float p_angle )
{
	motionGhostBuffer->SetNumActiveMembers(p_numGhosts);
	dir = p_dir;
	angle = p_angle;
}

void MotionGhostEffect::SetRootPos(Vector2f &pos)
{
	rootPos = pos;
}

void MotionGhostEffect::SetColorGradient(sf::Color &c0,
	sf::Color &c1, CubicBezier &bez)
{
	rootColor = c0;
	tailColor = c1;
	colorBez = bez;
}

void MotionGhostEffect::SetColor(sf::Color &c)
{
	rootColor = c;
	tailColor = c;
	colorBez = CubicBezier();
}

void MotionGhostEffect::ApplyUpdates()
{
	int numActiveMembers = motionGhostBuffer->numActiveMembers;

	

	for (int i = 0; i < numActiveMembers; ++i)
	{
		motionGhostBuffer->SetRotation(i, angle);
		motionGhostBuffer->SetTile(i, tileIndex);
	}

	//int showMotionGhosts = min((int)round(motionMagnitude), motionGhostBuffer->numMembers );
	//motionGhostBuffer->SetNumActiveMembers(showMotionGhosts);

	Vector2f motionNormal(dir.y, -dir.x);

	
	//float scaleAmountDown = scaleBez.GetValue(pGhosts) * maxScaleDown;

	Vector2f tempPos;
	float pGhosts;
	float vibrateAmount;
	int iVibrateAmount;
	float scaleAmountUp;
	Color currColor;
	int cr, cg, cb, ca;
	float ta;
	float recip;
	float vGhosts;
	float sGhosts;
	for (int i = 0; i < numActiveMembers; ++i)
	{
		if (numActiveMembers == 1)
		{
			pGhosts = 0;
			vGhosts = 0;
			sGhosts = 0;
		}
		else
		{
			pGhosts = i / (float)(numActiveMembers - 1);
			if (i > startVibrateGhost)
			{
				vGhosts = i / (float)(numActiveMembers - startVibrateGhost - 1);
			}
			else
			{
				vGhosts = 0;
			}

			if (i > startScaleGhost)
			{
				sGhosts = i / (float)(numActiveMembers - startScaleGhost - 1);
			}
			else
			{
				sGhosts = 0;
			}

			
			
		}
		
		
		ta = colorBez.GetValue(pGhosts);
		recip = 1.f - ta;
		cr = round(rootColor.r * recip + tailColor.r * ta);
		cr = max(cr, 0);
		cr = min(cr, 255);

		cg = round(rootColor.g * recip + tailColor.g * ta);
		cg = max(cg, 0);
		cg = min(cg, 255);

		cb = round(rootColor.b * recip + tailColor.b * ta);
		cb = max(cb, 0);
		cb = min(cb, 255);

		ca = round(rootColor.a * recip + tailColor.a * ta);
		ca = max(ca, 0);
		ca = min(ca, 255);

		 vibrateAmount = vibrateBez.GetValue(vGhosts) * maxVibrate;

		iVibrateAmount = round(vibrateAmount);
		if (iVibrateAmount % 2 == 1)
			iVibrateAmount++;

		scaleAmountUp = scaleBez.GetValue(sGhosts) * maxScaleUp;


		tempPos = Vector2f(rootPos.x + dir.x * (i * distInBetween), rootPos.y + dir.y * (i * distInBetween));
		if (iVibrateAmount != 0)
			tempPos += Vector2f(motionNormal * (float)(rand() % iVibrateAmount - iVibrateAmount / 2));
		//motionGhosts[i].setPosition( tempPos );

		/*int tf = 10;
		Vector2f ff(rand() % tf - tf / 2, rand() % tf - tf / 2);
		Vector2f ff1(rand() % tf - tf / 2, rand() % tf - tf / 2);*/
		motionGhostBuffer->SetColor( i, sf::Color(cr, cg, cb, ca));
		motionGhostBuffer->SetPosition(i, tempPos);
		//motionGhostBufferBlue->SetPosition(i, tempPos + ff);
		//motionGhostBufferPurple->SetPosition(i, tempPos + ff1);

		float x = 1.f;

		if (!facingRight )//|| (facingRight && reversed) )
			x = -x;

		if (reversed)
			x = -x;
		float y = 1.f;
		
		//if (reversed)
		//	y = -y;

		//motionGhostBuffer->SetScale(i, Vector2f(x, y));

		if (i >= startScaleGhost )
		{
			//float blah = ((rand() % testq) - testq / 2) / ((float)testq / 2);
			float blah = ((rand() % 100)) / 100.f;
			blah /= 4.f;//2.f;
						//blah = -blah;
						//motionGhosts[i].setScale( Vector2f(1.f + blah, 1.f + blah));
			motionGhostBuffer->SetScale(i, Vector2f(x + blah, y + blah));
			/*motionGhostBufferBlue->SetScale(i, Vector2f(x + blah, y + blah));
			motionGhostBufferPurple->SetScale(i, Vector2f(x + blah, y + blah));*/
		}
		else
		{
			motionGhostBuffer->SetScale(i, Vector2f(x, y));
			/*motionGhostBufferBlue->SetScale(i, Vector2f(x, y));
			motionGhostBufferPurple->SetScale(i, Vector2f(x, y));*/
		}
	}
	motionGhostBuffer->UpdateVertices();
}

void MotionGhostEffect::SetVibrateParams(CubicBezier &vBez, float p_maxVibrate, int startGhost)
{
	vibrateBez = vBez;
	maxVibrate = p_maxVibrate;
	startVibrateGhost = startGhost;
}

void MotionGhostEffect::SetScaleParams(CubicBezier &sBez, float p_maxScaleUp, float p_maxScaleDown, int p_startGhost )
{
	scaleBez = sBez;
	maxScaleUp = p_maxScaleUp;
	maxScaleDown = p_maxScaleDown;
	startScaleGhost = p_startGhost;
}

void MotionGhostEffect::SetFacing(bool p_facingRight, bool p_reversed)
{
	facingRight = p_facingRight;
	reversed = p_reversed;
}

void MotionGhostEffect::Draw(sf::RenderTarget *target)
{
	if (shader != NULL)
	{
		motionGhostBuffer->Draw(target, shader);
	}
	else
	{
		motionGhostBuffer->Draw(target);
	}
}

void MotionGhostEffect::SetShader(sf::Shader *pShad)
{
	shader = pShad;
}

void MotionGhostEffect::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
	motionGhostBuffer->ts = p_ts;
}
void MotionGhostEffect::SetTile(int p_tileIndex)
{
	tileIndex = p_tileIndex;
}