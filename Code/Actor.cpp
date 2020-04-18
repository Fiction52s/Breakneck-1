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
#include "Rail.h"
#include "Aura.h"
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
#include "TerrainPiece.h"
#include "Grass.h"
#include "EnvPlant.h"
#include "SpecialTerrain.h"
#include "Barrier.h"
#include "AbsorbParticles.h"
#include "EditSession.h"

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

Tileset * Actor::GetTileset(const std::string & s, int tileWidth, int tileHeight, int altColorIndex)
{
	if (owner != NULL)
	{
		return owner->GetTileset(s, tileWidth, tileHeight, altColorIndex);
	}
	else if (editOwner != NULL)
	{
		return editOwner->GetTileset(s, tileWidth, tileHeight, altColorIndex);
	}
	else
		return NULL;
}


Tileset * Actor::GetTileset(const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numColorChanges,
	sf::Color *startColorBuf, sf::Color *endColorBuf)
{
	if (owner != NULL)
	{
		return owner->GetTileset(s, tileWidth, tileHeight, altColorIndex, numColorChanges, startColorBuf, endColorBuf);
	}
	else if (editOwner != NULL)
	{
		return editOwner->GetTileset(s, tileWidth, tileHeight, altColorIndex, numColorChanges, startColorBuf, endColorBuf);
	}
	else
		return NULL;
}

Tileset * Actor::GetTileset(const std::string & s, int tileWidth, int tileHeight, Skin *skin)
{
	return sess->GetTileset(s, tileWidth, tileHeight, skin);
}

sf::SoundBuffer * Actor::GetSound(const std::string &name)
{
	if (owner != NULL)
	{
		return owner->soundManager->GetSound(name);
	}
	else
		return NULL;
}

map<int, list<CollisionBox>> & Actor::GetHitboxList(const string & str)
{
	if (owner != NULL)
	{
		return owner->hitboxManager->GetHitboxList(str);
	}
	else
	{
		return editOwner->hitboxManager->GetHitboxList(str);
	}
	
}

void Actor::UpdatePowers()
{
	if (owner != NULL)
	{
		owner->mainMenu->pauseMenu->kinMenu->UpdatePowers(this);
	}
}

QuadTree *Actor::GetTerrainTree()
{
	if (owner != NULL)
		return owner->terrainTree;
	else if( editOwner != NULL )
	{
		return editOwner->terrainTree;
	}
}

QuadTree *Actor::GetSpecialTerrainTree()
{
	if (owner != NULL)
		return owner->specialTerrainTree;
	else if (editOwner != NULL)
	{
		return editOwner->specialTerrainTree;
	}
}

QuadTree *Actor::GetRailEdgeTree()
{
	if (owner != NULL)
		return owner->railEdgeTree;
	else if (editOwner != NULL)
	{
		return editOwner->railEdgeTree;
	}
}

QuadTree *Actor::GetBarrierTree()
{
	if (owner != NULL)
		return owner->barrierTree;
	else if (editOwner != NULL)
	{
		return editOwner->barrierTree;
	}
}

QuadTree *Actor::GetBorderTree()
{
	if (owner != NULL)
		return owner->borderTree;
	else if (editOwner != NULL)
	{
		return editOwner->borderTree;
	}
}

int Actor::GetTotalGameFrames()
{
	if (owner != NULL)
		return owner->totalGameFrames;
	else
		return editOwner->totalGameFrames;
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
	SoundBuffer *sb = soundBuffers[S_ENTER];

	if (sb == NULL)
		return NULL;

	if (owner != NULL)
	{
		return owner->soundNodeList->ActivateSound(sb, loop);
	}
	else
	{

	}

	return NULL;
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
	if (owner != NULL)
	{
		return owner->ActivateEffect(layer, ts, pos, pauseImmune, angle, frameCount, animationFactor,
			right, startFrame, depth);
	}
	else
	{
		return NULL;
	}
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
	if (owner != NULL)
	{
		return owner->coll;
	}
	else if (editOwner != NULL)
	{
		return editOwner->collider;
	}

}

void Actor::SetupTilesets( Skin *skin, Skin *swordSkin )
{
	ts_scorpRun = GetTileset("Kin/scorp_run_192x128.png", 192, 128);
	ts_scorpSlide = GetTileset("Kin/scorp_slide_160x96.png", 160, 96);
	ts_scorpSteepSlide = GetTileset("Kin/scorp_steep_slide_224x128.png", 224, 128);
	ts_scorpStart = GetTileset("Kin/scorp_start_256x256.png", 256, 256);
	ts_scorpStand = GetTileset("Kin/scorp_stand_224x128.png", 224, 128);
	ts_scorpJump = GetTileset("Kin/scorp_jump_192x144.png", 192, 144);
	ts_scorpDash = GetTileset("Kin/scorp_dash_192x80.png", 192, 80);
	ts_scorpSprint = GetTileset("Kin/scorp_sprint_192x96.png", 192, 96);
	ts_scorpClimb = GetTileset("Kin/scorp_climb_256x128.png", 256, 128);
	ts_bubble = GetTileset("Kin/time_bubble_128x128.png", 128, 128);

	ts_airBounceFlame = GetTileset("Kin/bouncejumpflame.png", 128, 128, skin);
	ts_runBounceFlame = GetTileset("Kin/bouncerunflame.png", 128, 96, skin);
	ts_bounceBoost = GetTileset("FX/bounceboost_256x192.png", 256, 192, skin);

	ts_dodecaSmall = GetTileset("Kin/dodecasmall.png", 180, 180, skin);
	ts_dodecaBig = GetTileset("Kin/dodecabig.png", 360, 360, skin);
	tsgsdodeca = GetTileset("Kin/dodeca_64x64.png", 64, 64);
	tsgstriblue = GetTileset("Kin/triblue.png", 64, 64);
	tsgstricym = GetTileset("Kin/tricym_128x128.png", 128, 128);
	tsgstrigreen = GetTileset("Kin/trigreen.png", 64, 64);
	tsgstrioran = GetTileset("Kin/trioran_128x128.png", 128, 128);
	tsgstripurp = GetTileset("Kin/tripurp_128x128.png", 128, 128);
	tsgstrirgb = GetTileset("Kin/trirgb_128x128.png", 128, 128);

	tileset[STAND] = GetTileset("Kin/stand_64x64.png", 64, 64, skin);
	tileset[WALLATTACK] = GetTileset("Kin/wall_att_64x128.png", 64, 128, skin);
	tileset[DAIR] = GetTileset("Kin/dair_80x80.png", 80, 80, skin);
	tileset[DASH] = GetTileset("Kin/dash_96x48.png", 96, 48, skin);
	tileset[DOUBLE] = GetTileset("Kin/double_64x64.png", 64, 64, skin);
	tileset[BACKWARDSDOUBLE] = GetTileset("Kin/double_back_96x96.png", 96, 96, skin);
	tileset[FAIR] = GetTileset("Kin/fair_80x80.png", 80, 80, skin);
	tileset[DIAGUPATTACK] = GetTileset("Kin/airdash_attack_up_96x80.png", 96, 80, skin);
	tileset[DIAGDOWNATTACK] = GetTileset("Kin/airdash_attack_down_64x64.png", 64, 64, skin);
	tileset[JUMP] = GetTileset("Kin/jump_64x64.png", 64, 64, skin);
	tileset[LAND] = GetTileset("Kin/land_64x64.png", 64, 64, skin);
	tileset[LAND2] = GetTileset("Kin/land_64x64.png", 64, 64, skin);
	tileset[RUN] = GetTileset("Kin/run_64x64.png", 64, 64, skin);
	tileset[SPRINGSTUN] = GetTileset("Kin/launch_96x64.png", 96, 64, skin);
	tileset[SPRINGSTUNGLIDE] = tileset[SPRINGSTUN];

	tileset[SPRINGSTUNAIRBOUNCE] = tileset[SPRINGSTUN];
	tileset[SPRINGSTUNBOUNCE] = tileset[SPRINGSTUN];
	tileset[SPRINGSTUNTELEPORT] = tileset[SPRINGSTUN];

	tileset[SWINGSTUN] = tileset[SPRINGSTUN];

	tileset[GLIDE] = tileset[SPRINGSTUN];
	tileset[SLIDE] = GetTileset("Kin/slide_64x64.png", 64, 64, skin);
	tileset[SPRINT] = GetTileset("Kin/sprint_80x48.png", 80, 48, skin);	
	//tileset[DASHATTACK] = GetTileset("dash_attack_128x96.png", 128, 96);
	tileset[STANDN] = GetTileset("Kin/standn_96x64.png", 96, 64, skin);
	tileset[UAIR] = GetTileset("Kin/uair_96x96.png", 96, 96, skin);
	tileset[WALLCLING] = GetTileset("Kin/wallcling_64x64.png", 64, 64, skin);
	tileset[WALLJUMP] = GetTileset("Kin/walljump_64x64.png", 64, 64, skin);
	tileset[GRINDBALL] = GetTileset("Kin/grindball_64x64.png", 64, 64, skin);
	tileset[GRINDLUNGE] = GetTileset("Kin/airdash_80x80.png", 80, 80, skin);
	tileset[GRINDSLASH] = GetTileset("Kin/grind_lunge_96x128.png", 96, 128, skin);
	tileset[GRINDATTACK] = GetTileset("Kin/grindball_64x64.png", 64, 64, skin);
	tileset[STEEPSLIDE] = GetTileset("Kin/steepslide_64x64.png", 64, 64, skin);
	tileset[STEEPCLIMBATTACK] = GetTileset("Kin/climb_att_128x64.png", 128, 64, skin);
	tileset[STEEPSLIDEATTACK] = GetTileset("Kin/steep_att_128x64.png", 128, 64, skin);
	tileset[AIRDASH] = GetTileset("Kin/airdash_80x80.png", 80, 80, skin);
	tileset[STEEPCLIMB] = GetTileset("Kin/steepclimb_96x32.png", 96, 32, skin);
	tileset[AIRHITSTUN] = GetTileset("Kin/hurt_64x64.png", 64, 64, skin);
	tileset[GROUNDHITSTUN] = GetTileset("Kin/hurt_64x64.png", 64, 64, skin);
	tileset[WIREHOLD] = tileset[STEEPSLIDE];//GetTileset("Kin/steepslide_80x48.png", 80, 48, skin);
	tileset[BOUNCEAIR] = GetTileset("Kin/bounce_224x224.png", 224, 224, skin);
	tileset[BOUNCEGROUND] = GetTileset("Kin/bounce_224x224.png", 224, 224, skin);
	tileset[BOUNCEGROUNDEDWALL] = GetTileset("Kin/bounce_wall_224x224.png", 224, 224, skin);
	tileset[DEATH] = GetTileset("Kin/death_128x96.png", 128, 96, skin);
	tileset[JUMPSQUAT] = GetTileset("Kin/jump_64x64.png", 64, 64, skin);
	tileset[INTRO] = GetTileset("Kin/enter_64x64.png", 64, 64, skin);
	
	if (owner != NULL)
	{
		ts_exitAura = owner->mainMenu->tilesetManager.GetTileset("Kin/exitaura_256x256.png", 256, 256);
		exitAuraSprite.setTexture(*ts_exitAura->texture);
	}

	tileset[EXIT] = GetTileset("Kin/exit_64x128.png", 64, 128, skin);
	tileset[EXITBOOST] = GetTileset("Kin/exit_96x128.png", 96, 128, skin);// kin_exit_128x128.png", 128, 128, skin);
	tileset[INTROBOOST] = tileset[EXITBOOST];
	tileset[EXITWAIT] = NULL;
	tileset[GRAVREVERSE] = GetTileset("Kin/grav_64x64.png", 64, 64, skin);
	tileset[RIDESHIP] = GetTileset("Kin/dive_64x64.png", 64, 64, skin);
	tileset[SKYDIVE] = GetTileset("Kin/walljump_64x64.png", 64, 64, skin);
	tileset[SKYDIVETOFALL] = GetTileset("Kin/intro_0_160x80.png", 160, 80, skin);
	tileset[WAITFORSHIP] = GetTileset("Kin/shipjump_160x96.png", 160, 96, skin);
	tileset[GRABSHIP] = GetTileset("Kin/shipjump_160x96.png", 160,96, skin);
	tileset[ENTERNEXUS1] = GetTileset("Kin/intro_0_160x80.png", 160, 80, skin);

	tileset[GETPOWER_AIRDASH_MEDITATE] = GetTileset("Kin/w1_airdashget_128x128.png", 128, 128, skin);
	tileset[GETPOWER_AIRDASH_FLIP] = GetTileset("Kin/w1_airdashget_128x128.png", 128, 128, skin);

	tileset[SEQ_LOOKUP] = GetTileset("Kin/kin_cover_64x64.png", 64, 64, skin);

	tileset[SEQ_KINTHROWN] = tileset[AIRHITSTUN];

	tileset[SEQ_KNEEL] = GetTileset("Kin/kin_meditate_64x96.png", 64, 96, skin);
	tileset[SEQ_KNEEL_TO_MEDITATE] = tileset[SEQ_KNEEL];
	tileset[SEQ_MEDITATE_MASKON] = tileset[SEQ_KNEEL];
	tileset[SEQ_MASKOFF] = tileset[SEQ_KNEEL];
	tileset[SEQ_MEDITATE] = tileset[SEQ_KNEEL];

	tileset[SEQ_FLOAT_TO_NEXUS_OPENING] = GetTileset("Kin/nexus_enter_384x256.png", 384, 256, skin);
	tileset[SEQ_FADE_INTO_NEXUS] = tileset[SEQ_FLOAT_TO_NEXUS_OPENING];//tileset[AIRDASH];

	tileset[SEQ_CRAWLERFIGHT_STAND] = GetTileset("Kin/stand_64x64.png", 64, 64, skin);
	tileset[SEQ_WAIT] = GetTileset("Kin/jump_64x64.png", 64, 64, skin);
	tileset[SEQ_CRAWLERFIGHT_DODGEBACK] = GetTileset("Kin/jump_64x64.png", 64, 64, skin);
	tileset[SEQ_CRAWLERFIGHT_STRAIGHTFALL] = GetTileset("Kin/jump_64x64.png", 64, 64, skin);
	tileset[SEQ_CRAWLERFIGHT_LAND] = GetTileset("Kin/land_64x64.png", 64, 64, skin);
	tileset[SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY] = GetTileset("Kin/run_64x64.png", 64, 64, skin);
	tileset[SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED] = GetTileset("Kin/slide_64x64.png", 64, 64, skin);

	tileset[SEQ_TURNFACE] = GetTileset("Kin/shipjump_160x96.png", 160, 96, skin);

	tileset[GETSHARD] = tileset[DEATH];

	ts_fairSword[0] = GetTileset("Sword/fair_sworda_256x256.png", 256, 256, swordSkin);
	ts_fairSword[1] = GetTileset("Sword/fair_swordb_288x288.png", 288, 288, swordSkin);//ts_fairSword[0];//GetTileset("fair_swordb_256x256.png", 256, 256, swordSkin);
	ts_fairSword[2] = GetTileset("Sword/fair_swordc_384x384.png", 384, 384, swordSkin);//ts_fairSword[0];//GetTileset("fair_swordc_384x384.png", 384, 384, swordSkin);

	ts_fairSwordLightning[0] = GetTileset("FX/fair_sword_lightninga_256x256.png", 256, 256, swordSkin);
	ts_fairSwordLightning[1] = GetTileset("FX/fair_sword_lightninga_256x256.png", 256, 256, swordSkin);
	ts_fairSwordLightning[2] = GetTileset("FX/fair_sword_lightninga_256x256.png", 256, 256, swordSkin);

	ts_dairSword[0] = GetTileset("Sword/dair_sworda_256x256.png", 256, 256, swordSkin);
	ts_dairSword[1] = GetTileset("Sword/dair_swordb_288x288.png", 288, 288, swordSkin);
	ts_dairSword[2] = GetTileset("Sword/dair_swordc_384x384.png", 384, 384, swordSkin);

	ts_dairSwordLightning[0] = GetTileset("FX/dair_sword_lightninga_256x256.png", 256, 256, swordSkin);
	ts_dairSwordLightning[1] = GetTileset("FX/dair_sword_lightninga_256x256.png", 256, 256, swordSkin);
	ts_dairSwordLightning[2] = GetTileset("FX/dair_sword_lightninga_256x256.png", 256, 256, swordSkin);

	ts_uairSword[0] = GetTileset("Sword/uair_sworda_256x256.png", 256, 256, swordSkin);
	ts_uairSword[1] = GetTileset("Sword/uair_swordb_288x288.png", 288, 288, swordSkin);
	ts_uairSword[2] = GetTileset("Sword/uair_swordc_320x320.png", 320, 320, swordSkin);

	ts_uairSwordLightning[0] = GetTileset("FX/uair_sword_lightninga_256x256.png", 256, 256, swordSkin);
	ts_uairSwordLightning[1] = GetTileset("FX/uair_sword_lightninga_256x256.png", 256, 256, swordSkin);
	ts_uairSwordLightning[2] = GetTileset("FX/uair_sword_lightninga_256x256.png", 256, 256, swordSkin);

	ts_grindLungeSword[0] = GetTileset("Sword/grind_lunge_sworda_160x160.png", 160, 160, swordSkin);
	ts_grindLungeSword[1] = GetTileset("Sword/grind_lunge_swordb_192x192.png", 192, 192, swordSkin);
	ts_grindLungeSword[2] = GetTileset("Sword/grind_lunge_swordc_224x208.png", 224, 208, swordSkin);

	ts_standingNSword[0] = GetTileset("Sword/stand_sworda_208x208.png", 208, 208, swordSkin);
	ts_standingNSword[1] = GetTileset("Sword/stand_swordb_304x176.png", 304, 176, swordSkin);
	ts_standingNSword[2] = GetTileset("Sword/stand_swordc_304x192.png", 304, 192, swordSkin);

	/*ts_dashAttackSword[0] = GetTileset("dash_sworda_256x256.png", 256, 256, swordSkin);
	ts_dashAttackSword[1] = GetTileset("dash_swordb_256x256.png", 256, 256, swordSkin);
	ts_dashAttackSword[2] = GetTileset("dash_swordc_256x304.png", 256, 304, swordSkin);*/

	ts_wallAttackSword[0] = GetTileset("Sword/wall_sworda_144x256.png", 144, 256, swordSkin);
	ts_wallAttackSword[1] = GetTileset("Sword/wall_swordb_240x352.png", 240, 352, swordSkin);
	ts_wallAttackSword[2] = GetTileset("Sword/wall_swordc_298x400.png", 298, 400, swordSkin);

	ts_steepSlideAttackSword[0] = GetTileset("Sword/steep_att_sworda_480x176.png", 480, 176, swordSkin);
	ts_steepSlideAttackSword[1] = GetTileset("Sword/steep_att_swordb_352x192.png", 352, 192, swordSkin);
	ts_steepSlideAttackSword[2] = GetTileset("Sword/steep_att_swordc_560x256.png", 560, 256, swordSkin);

	ts_steepClimbAttackSword[0] = GetTileset("Sword/climb_att_sworda_352x128.png", 352, 128, swordSkin);
	ts_steepClimbAttackSword[1] = GetTileset("Sword/climb_att_swordb_416x320.png", 416, 320, swordSkin);
	ts_steepClimbAttackSword[2] = GetTileset("Sword/climb_att_swordc_496x208.png", 496, 208, swordSkin);

	ts_diagUpSword[0] = GetTileset("Sword/airdash_u_sword_144x208.png", 144, 208, swordSkin);
	ts_diagUpSword[1] = GetTileset("Sword/airdash_u_sword_b_224x240.png", 224, 240, swordSkin);
	ts_diagUpSword[2] = GetTileset("Sword/airdash_u_sword_p_320x384.png", 320, 384, swordSkin);

	ts_diagDownSword[0] = GetTileset("Sword/airdash_sword_128x208.png", 128, 208, swordSkin);
	ts_diagDownSword[1] = GetTileset("Sword/airdash_sword_b_224x240.png", 224, 240, swordSkin);
	ts_diagDownSword[2] = GetTileset("Sword/airdash_sword_p_320x384.png", 320, 384, swordSkin);

	ts_fx_hurtSpack = GetTileset("FX/hurt_spack_128x160.png", 128, 160);

	ts_fx_dashStart = GetTileset("FX/fx_dashstart_160x160.png", 160, 160);
	ts_fx_dashRepeat = GetTileset("FX/fx_dashrepeat_192x128.png", 192, 128);

	ts_fx_land[0] = GetTileset("FX/land_a_128x128.png", 128, 128);
	ts_fx_land[1] = GetTileset("FX/land_b_192x208.png", 192, 208);
	ts_fx_land[2] = GetTileset("FX/land_c_256x256.png", 256, 256);
	ts_fx_runStart[0] = GetTileset("FX/runstart_a_128x128.png", 128, 128);
	ts_fx_runStart[1] = GetTileset("FX/runstart_b_224x224.png", 224, 224);
	ts_fx_runStart[2] = GetTileset("FX/runstart_c_224x224.png", 224, 224);

	ts_fx_sprint = GetTileset("FX/fx_sprint_176x176.png", 176, 176);
	ts_fx_run = GetTileset("FX/fx_run_144x128.png", 144, 128);
	ts_fx_bigRunRepeat = GetTileset("FX/fx_bigrunrepeat.png", 176, 112);

	ts_fx_jump[0] = GetTileset("FX/jump_a_128x80.png", 128, 80);
	ts_fx_jump[1] = GetTileset("FX/jump_b_160x192.png", 160, 192);
	ts_fx_jump[2] = GetTileset("FX/jump_c_160x192.png", 160, 192);

	ts_fx_wallJump[0] = GetTileset("FX/walljump_a_160x160.png", 160, 160);
	ts_fx_wallJump[1] = GetTileset("FX/walljump_b_224x224.png", 224, 224);
	ts_fx_wallJump[2] = GetTileset("FX/walljump_c_224x224.png", 224, 224);
	ts_fx_double = GetTileset("FX/fx_doublejump_196x160.png", 196, 160);
	ts_fx_gravReverse = GetTileset("FX/fx_grav_reverse_128x128.png", 128, 128);

	ts_fx_chargeBlue0 = GetTileset("FX/elec_01_128x128.png", 128, 128);
	ts_fx_chargeBlue1 = GetTileset("FX/elec_03_128x128.png", 128, 128);
	ts_fx_chargeBlue2 = GetTileset("FX/elec_04_128x128.png", 128, 128);
	ts_fx_chargePurple = GetTileset("FX/elec_02_128x128.png", 128, 128);


	ts_fx_rightWire = GetTileset("FX/wire_boost_r_64x64.png", 64, 64);
	ts_fx_leftWire = GetTileset("FX/wire_boost_b_64x64.png", 64, 64);
	ts_fx_doubleWire = GetTileset("FX/wire_boost_m_64x64.png", 64, 64);

	ts_fx_airdashDiagonal = GetTileset("FX/fx_airdash_diagonal_1_128x128.png", 128, 128);
	ts_fx_airdashUp = GetTileset("FX/fx_airdash_128x128.png", 128, 128);
	ts_fx_airdashHover = GetTileset("FX/fx_airdash_hold_1_96x80.png", 96, 80);

	ts_fx_death_1a = GetTileset("FX/death_fx_1a_256x256.png", 256, 256);
	ts_fx_death_1b = GetTileset("FX/death_fx_1b_128x80.png", 128, 80);
	ts_fx_death_1c = GetTileset("FX/death_fx_1c_128x128.png", 128, 128);
	ts_fx_death_1d = GetTileset("FX/death_fx_1d_48x48.png", 48, 48);
	ts_fx_death_1e = GetTileset("FX/death_fx_1e_160x160.png", 160, 160);
	ts_fx_death_1f = GetTileset("FX/death_fx_1f_160x160.png", 160, 160);

	tileset[GOALKILL] = GetTileset("Kin/goal_w01_killa_384x256.png", 384, 256);
	tileset[GOALKILL1] = GetTileset("Kin/goal_w01_killb_384x256.png", 384, 256);
	tileset[GOALKILL2] = GetTileset("Kin/goal_w01_killc_384x256.png", 384, 256);
	tileset[GOALKILL3] = GetTileset("Kin/goal_w01_killd_384x256.png", 384, 256);
	tileset[GOALKILL4] = GetTileset("Kin/goal_w01_kille_384x256.png", 384, 256);
}

Actor::Actor( GameSession *gs, EditSession *es, int p_actorIndex )
	:owner( gs ), editOwner(es), dead( false ), actorIndex( p_actorIndex )
	{

	sess = Session::GetSession();

	usingAura = false;
	fBubblePos = NULL;
	fBubbleRadiusSize = NULL;
	fBubbleFrame = NULL;


	autoRunStopEdge = NULL;
	extraDoubleJump = false;
	stunBufferedJump = false;
	stunBufferedDash = false;
	stunBufferedAttack = Action::Count;

	extraGravityModifier = 1.0;
	airTrigBehavior = AT_NONE;
	rpu = new RisingParticleUpdater( this );
	//hitCeilingLockoutFrames = 20;
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
		GetTileset("Kin/exitenergy_0_512x512.png", 512, 512);
		GetTileset("Kin/exitenergy_2_512x512.png", 512, 512);
		GetTileset("Kin/exitenergy_1_512x512.png", 512, 512);
		
			
		currLockedFairFX = NULL;
		currLockedDairFX = NULL;
		currLockedUairFX = NULL;
		gateBlackFX = NULL;

		for (int i = 0; i < 7; ++i)
		{
			smallLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 4, 1.f);
		}

		smallLightningPool[0]->ts = GetTileset("FX/elec_01_96x96.png", 96, 96);
		smallLightningPool[1]->ts = GetTileset("FX/elec_02_96x96.png", 96, 96);
		smallLightningPool[2]->ts = GetTileset("FX/elec_03_96x96.png", 96, 96);
		smallLightningPool[3]->ts = GetTileset("FX/elec_04_96x96.png", 96, 96);
		smallLightningPool[4]->ts = GetTileset("FX/elec_05_96x96.png", 96, 96);
		smallLightningPool[5]->ts = GetTileset("FX/elec_06_96x96.png", 96, 96);
		smallLightningPool[6]->ts = GetTileset("FX/elec_07_96x96.png", 96, 96);

		motionGhostBuffer = new VertexBuf(80, sf::Quads);
		motionGhostBufferBlue = new VertexBuf(80, sf::Quads);
		motionGhostBufferPurple = new VertexBuf(80, sf::Quads);
		

		for (int i = 0; i < 3; ++i)
		{
			fairLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 20, 1.f);
			fairLightningPool[i]->ts = GetTileset("FX/fair_sword_lightninga_256x256.png", 256, 256);
			dairLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 20, 1.f);
			dairLightningPool[i]->ts = GetTileset("FX/dair_sword_lightninga_256x256.png", 256, 256);
			uairLightningPool[i] = new EffectPool(EffectType::FX_RELATIVE, 20, 1.f);
			uairLightningPool[i]->ts = GetTileset("FX/uair_sword_lightninga_256x256.png", 256, 256);
		}

		gateBlackFXPool = new EffectPool(EffectType::FX_RELATIVE, 2, 1.f);
		gateBlackFXPool->ts = GetTileset("FX/keydrain_160x160.png", 160, 160);

		if (owner != NULL)
		{
			kinRing = new KinRing(this);
			kinMask = new KinMask(this);
		}
		else
		{
			kinRing = NULL;
			kinMask = NULL;
		}
		

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

		
		
		team = (Team)actorIndex; //debug

		SetupTilesets(NULL, NULL);

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

		ts_dirtyAura = GetTileset("Kin/dark_aura_w1_384x384.png", 384, 384);
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
		followerPos = V2d( 0, 0 );
		followerVel = V2d( 0, 0 );
		followerFac = 1.0 / 60.0;
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
		test = false;

		lastWire = 0;
		inBubble = false;
		oldInBubble = false;

		numKeys = 0;
		
		

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
		percentCloneChanged = 0;
		percentCloneRate = .01;
		changingClone = false;

		desperationMode = false;
		maxDespFrames = 60 * 5;
		despCounter = 0;

		

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
		
		CollisionBox cb(CollisionBox::BoxType::Hit );
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
			grindHitboxes[0] = new CollisionBody(1);
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
		actionLength[WALLATTACK] = 8 * 2;
		actionLength[DAIR] = 16;
		actionLength[DASH] = 45 + 10;
		maxBBoostCount = actionLength[DASH];
		actionLength[DOUBLE] = 28 + 10;
		actionLength[BACKWARDSDOUBLE] = 40;//28 + 10;
		actionLength[FAIR] = 8 * 2;
		actionLength[DIAGUPATTACK] = 11 * 2;
		actionLength[DIAGDOWNATTACK] = 11 * 2;
		actionLength[JUMP] = 2;
		actionLength[SEQ_WAIT] = 2;
		actionLength[SEQ_CRAWLERFIGHT_DODGEBACK] = 2;
		actionLength[SEQ_CRAWLERFIGHT_STRAIGHTFALL] = 2;
		actionLength[LAND] = 1;
		actionLength[SEQ_CRAWLERFIGHT_LAND] = 1;
		actionLength[LAND2] = 1;
		actionLength[RUN] = 10 * 4;
		actionLength[AUTORUN] = actionLength[RUN];
		actionLength[SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY] = 10 * 4;
		actionLength[SLIDE] = 1;
		actionLength[SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED] = 1;
		actionLength[SPRINT] = 8 * 4;
		actionLength[STAND] = 20 * 8;
		actionLength[SEQ_ENTERCORE1] = 60;
		actionLength[SPRINGSTUN] = 8;
		actionLength[SPRINGSTUNGLIDE] = 8;

		actionLength[SPRINGSTUNAIRBOUNCE] = 30;
		actionLength[SPRINGSTUNBOUNCE] = 8;
		actionLength[SPRINGSTUNTELEPORT] = 8;

		actionLength[SWINGSTUN] = 8;

		actionLength[GLIDE] = 8;
		actionLength[SEQ_CRAWLERFIGHT_STAND] = 20 * 8;//240;//20 * 8;
		actionLength[DASHATTACK] = 8 * 2;
		actionLength[STANDN] = 8 * 2;
		actionLength[UAIR] = 16;
		actionLength[GRINDATTACK] = 1;
		actionLength[STEEPSLIDE] = 1;
		actionLength[WALLCLING] = 1;
		actionLength[WALLJUMP] = 9 * 2;
		actionLength[GRINDBALL] = 1;
		actionLength[GRINDLUNGE] = 20;
		actionLength[GRINDSLASH] = 16;
		actionLength[STEEPCLIMBATTACK] = 4 * 4;
		actionLength[SKYDIVETOFALL] = 10 * 4;
		actionLength[WAITFORSHIP] = 60 * 1;
		actionLength[GRABSHIP] = 10 * 5 + 20;
		actionLength[GETPOWER_AIRDASH_MEDITATE] = 300;
		actionLength[RIDESHIP] = 1;
		actionLength[SKYDIVE] = 9 * 2;
		actionLength[EXIT] = 29 * 2; //16 * 7
		actionLength[EXITBOOST] = 79 * 2;//71 * 2;

		actionLength[EXITWAIT] = 6 * 3 * 2;
		actionLength[GRAVREVERSE] = 20;
		actionLength[JUMPSQUAT] = 3;
		actionLength[INTRO] = 18 * 2;
		actionLength[INTROBOOST] = 22 * 2;//40 * 2;
		actionLength[AIRDASH] = 33;//27;
		actionLength[STEEPSLIDEATTACK] = 16;
		actionLength[AIRHITSTUN] = 1;
		actionLength[STEEPCLIMB] = 8 * 4;
		actionLength[GROUNDHITSTUN] = 1;
		actionLength[WIREHOLD] = 1;
		actionLength[BOUNCEAIR] = 1;
		actionLength[BOUNCEGROUND] = 15;
		actionLength[BOUNCEGROUNDEDWALL] = 30;
		actionLength[DEATH] = 44 * 2;
		actionLength[GETPOWER_AIRDASH_FLIP] = 133 * 2;
		
		actionLength[ENTERNEXUS1] = 10 * 4;
		
		actionLength[SPAWNWAIT] = 120;
		actionLength[RAILDASH] = 20;
		actionLength[RAILSLIDE] = 1;


		actionLength[GOALKILL] = 72 * 2;
		actionLength[GOALKILLWAIT] = 2;

		actionLength[NEXUSKILL] = 63 * 2;//actionLength[GOALKILL];

		actionLength[SEQ_LOOKUP] = 1;
		actionLength[SEQ_LOOKUPDISAPPEAR] = 1;
		
		actionLength[SEQ_KINTHROWN] = 1;
		actionLength[SEQ_KINSTAND] = actionLength[STAND];

		actionLength[SEQ_KNEEL] = 1;

		actionLength[SEQ_KNEEL_TO_MEDITATE] = 7 * 3;
		actionLength[SEQ_MEDITATE_MASKON] = 1;
		actionLength[SEQ_MASKOFF] = 24 * 3;
		actionLength[SEQ_MEDITATE] = 1;

		actionLength[SEQ_FLOAT_TO_NEXUS_OPENING] = 3 * 10;
		actionLength[SEQ_FADE_INTO_NEXUS] = 8 * 10;

		actionLength[GETSHARD] = 2;//44 * 2;
		}
		 	

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

		cout << "Start aura" << endl;
		//sh.setUniform( "u_texture", *tileset[action]->texture ); 

		//LoadAllAuras();
		
		cout << "end aura" << endl;

		
		

		grindActionLength = 32;
		//SetActionExpr( SPAWNWAIT );

		if (owner != NULL)
		{
			SetActionExpr(INTROBOOST);//INTRO
			frame = 0;
		}
		else
		{
			SetActionExpr(JUMP);
			frame = 1;
		}
		
		
		

		
		
		timeSlowStrength = 5;
		slowMultiple = 1;
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
		
		
		steepClimbGravFactor = .31;//.2;//.31;//.7;
		steepClimbFastFactor = .2;
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
		b.type = CollisionBox::BoxType::Physics;

		
		b.type = b.Physics;

		//hurtboxMap[STAND] = new CollisionBody(1);
		hurtBody.offset.x = 0;
		hurtBody.offset.y = 0;
		hurtBody.isCircle = false;
		hurtBody.rw = 7;//10;
		hurtBody.rh = 15;//normalHeight - 5;//normalHeight;
		hurtBody.type = CollisionBox::BoxType::Hurt;
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

		record = false;
		blah = false;

		touchEdgeWithLeftWire= false;
		touchEdgeWithRightWire= false;
		ghostFrame = 0;

		recordedGhosts = 0;

		bubbleSprite.setTexture( *ts_bubble->texture );

		currBubble = 0;
		bubbleRadius = 160;

		bubbleRadius0 = 160;
		bubbleRadius1 = 180;
		bubbleRadius2 = 200;
		

		bubbleLifeSpan = 240;
		
		int maxAura = 128 * 128;//64 * 64;//1000 * 1000;//300 * 300;//64 * 64;
		testAura = new Aura(this, 1, maxAura, 0);
		testAura1 = new Aura(this, 1, maxAura, 1);
		testAura2 = new Aura(this, 1, maxAura, 2);
		testAura3 = new Aura(this, 1, maxAura, 3);

		hasPowerAirDash = true;//false;//true;
		hasPowerGravReverse = false;
		hasPowerBounce = false;
		hasPowerGrindBall = false;
		hasPowerTimeSlow = false;
		hasPowerLeftWire = false;
		hasPowerRightWire = false;
		hasPowerClones = 0;

		if (true)
		{
			hasPowerAirDash = true;
			hasPowerGravReverse = true;
			hasPowerBounce = true;
			hasPowerGrindBall = true;
			hasPowerTimeSlow = true;
			hasPowerLeftWire = true;
			hasPowerRightWire = true;
		}

		if (owner != NULL)
		{
			SaveFile *currProgress = owner->GetCurrentProgress();
			if (currProgress != NULL && currProgress->HasPowerUnlocked(POWER_AIRDASH))//currProgress->ShardIsCaptured( ShardType::SHARD_W1_GET_AIRDASH ))
			{
				hasPowerAirDash = true;
			}
		}

		startHasPowerAirDash = hasPowerAirDash;
		startHasPowerGravReverse =	hasPowerGravReverse;
		startHasPowerBounce = hasPowerBounce;
		startHasPowerGrindBall = hasPowerGrindBall;
		startHasPowerTimeSlow = hasPowerTimeSlow;
		startHasPowerLeftWire = hasPowerLeftWire;
		startHasPowerRightWire = hasPowerRightWire;

		//do this a little later.
		UpdatePowers();


		//only set these parameters again if u get a power or lose one.
		//sh.setUniform( "hasPowerAirDash", hasPowerAirDash );
		//sh.setUniform( "hasPowerGravReverse", hasPowerGravReverse );
		//sh.setUniform( "hasPowerBounce", hasPowerBounce );
		//sh.setUniform( "hasPowerGrindBall", hasPowerGrindBall );
		//sh.setUniform( "hasPowerTimeSlow", hasPowerTimeSlow );
		//sh.setUniform( "hasPowerLeftWire", hasPowerLeftWire );
		//sh.setUniform( "hasPowerRightWire", hasPowerRightWire );

		/*Color basicArmor( 0x14, 0x59, 0x22 );
		Color basicArmorDark( 0x08, 0x40, 0x12 );

		if( actorIndex == 1 )
		{
			sf::Uint8 basicRed = basicArmor.r;
			sf::Uint8 basicGreen = basicArmor.g;
			sf::Uint8 basicBlue = basicArmor.b;

			basicArmor.r = basicGreen;
			basicArmor.b = basicRed;
			basicArmor.g = basicBlue;

			sf::Uint8 basicRedDark = basicArmorDark.r;
			sf::Uint8 basicGreenDark = basicArmorDark.g;
			sf::Uint8 basicBlueDark = basicArmorDark.b;

			basicArmorDark.r = basicGreenDark;
			basicArmorDark.b = basicRedDark;
			basicArmorDark.g = basicBlueDark;

		}*/
			
		//sh.setUniform( "armorColor", basicArmor );
		//sh.setUniform( "armorColorDark", basicArmorDark );
		
		//sh.setUniform( "hasPowerClones", hasPowerClones > 0 );

		//for( int i = 0; i < MAX_MOTION_GHOSTS; ++i )
		//{
		//	motionGhosts[i] = 
		//}

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

	if (usingAura)
	{
		list<Vector2f> *currP = NULL;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < Action::Count; ++j)
			{
				currP = auraPoints[i][j];
				if (currP != NULL)
				{
					delete[] currP;
				}
			}
		}
	}

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
	
	delete sprite;

	delete testAura;
	delete testAura1;
	delete testAura2;
	delete testAura3;

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

	for (int i = 0; i < MAX_GHOSTS; ++i)
	{
		if (ghosts[i] != NULL)
			delete ghosts[i];
	}
	//eventually delete everything here lol
}

void Actor::ActionEnded()
{
	//cout << "length: " << actionLength[action] << endl;
	if( frame >= actionLength[action] )
	{
		switch (action)
		{
		case SEQ_KINSTAND:
		case STAND:
			frame = 0;
			break;
		case AUTORUN:
		case RUN:
			frame = 0;
			break;
		case SEQ_KINFALL:
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
			SetActionExpr(JUMP);
			frame = 1;
			holdJump = false;
			break;
		case GRAVREVERSE:
			SetAction(STAND);
			frame = 0;
			break;
		case SPRINGSTUN:
			//action = JUMP;
			frame = 0;
			
			//assert(ground == NULL);
			break;
		case SPRINGSTUNAIRBOUNCE:
			frame = actionLength[SPRINGSTUNAIRBOUNCE] - 1;
			break;
		case SPRINGSTUNBOUNCE:
		case SPRINGSTUNGLIDE:
		case SPRINGSTUNTELEPORT:
			//action = JUMP;
			frame = 0;

			//assert(ground == NULL);
			break;
		case SWINGSTUN:
			frame = 0;
			break;
		case GLIDE:
			//action = JUMP;
			frame = 0;

			//assert(ground == NULL);
			break;
		case STANDN:

			if (currInput.LLeft() || currInput.LRight())
			{
				if (currInput.B)
				{
					SetActionExpr(DASH);
					//action = DASH;
					//re->Reset();
					//re1->Reset();
				}
				else
				{
					SetActionExpr(RUN);
				}
				facingRight = currInput.LRight();
			}
			else
			{
				SetActionExpr(STAND);
			}
			frame = 0;

			
			break;
		case FAIR:
		{
			//currLockedFairFX->ClearLockPos();
			//currLockedFairFX = NULL;
			

			SetActionExpr(JUMP);
			frame = 1;
			break;

			}
			
		case DIAGUPATTACK:
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case DIAGDOWNATTACK:
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
			//currLockedDairFX->ClearLockPos();
			//currLockedDairFX = NULL;
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case UAIR:
			//currLockedUairFX->ClearLockPos();
			//currLockedUairFX = NULL;
			SetActionExpr( JUMP );
			frame = 1;
			break;
		case DASH:
			//dashStartSound.stop();
			/*if (groundSpeed > 0 && abs(ground->Normal().y) == 1 )
			{
				groundSpeed += 5.0;
			}
			else if( groundSpeed < 0 && abs( ground->Normal().y ) == 1 )
			{
				groundSpeed -= 5.0;
			}*/

			



			SetActionExpr( STAND );
			frame = 0;
			break;
		case BACKWARDSDOUBLE:
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
		case RAILGRIND:
		case RAILSLIDE:
			frame = 0;
			break;
		case RAILDASH:
			SetActionExpr(JUMP);
			frame = 1;
			break;
		case GRINDLUNGE:
			SetAction(JUMP);
			frame = 1;
			break;
		case GRINDSLASH:
			SetAction(JUMP);
			frame = 1;
			break;
		case GRINDATTACK:
			SetAction(GRINDATTACK);
			frame = 0;
			break;
		case AIRDASH:
			{
				//cout << "inBubble: " << inBubble << endl;
				if( inBubble )//|| rightWire->state == Wire::PULLING )
				{
					//5 is here to give you extra frames to airdash
					frame = actionLength[AIRDASH] - 1;
					++framesExtendingAirdash;
					airDashStall = true;
				}
				else
				{
					SetActionExpr( JUMP );
					frame = 1;
					holdJump = false;
				}
			break;
			}
		case STEEPCLIMB:
			frame = 0;
			break;
		case AIRHITSTUN:


			if (stunBufferedAttack == Action::Count)
			{
				if (currInput.rightShoulder && !prevInput.rightShoulder)
				{
					if (currInput.LUp())
					{
						stunBufferedAttack = UAIR;
					}
					else if (currInput.LDown())
					{
						stunBufferedAttack = DAIR;
					}
					else
					{
						stunBufferedAttack = FAIR;
					}
				}
			}

			if (!stunBufferedJump && currInput.A && !prevInput.A)
			{
				stunBufferedJump = true;
			}

			if (!stunBufferedDash && currInput.B && !prevInput.B)
			{
				stunBufferedDash = true;
			}
			

			frame = 0;
			if (hitstunFrames <= setHitstunFrames / 2)
			{
				//AirAttack();
			}
			break;
		case GROUNDHITSTUN:

			if (stunBufferedAttack == Action::Count)
			{
				if (currInput.rightShoulder && !prevInput.rightShoulder)
				{
					stunBufferedAttack = STANDN;
				}
			}

			if (!stunBufferedJump && currInput.A && !prevInput.A)
			{
				stunBufferedJump = true;
			}

			if (!stunBufferedDash && currInput.B && !prevInput.B)
			{
				stunBufferedDash = true;
			}

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
			SetAction(STAND);
			frame = 0;
			break;
		case INTRO:
			//cout << "intro over" << endl;
			SetAction(JUMP);
			frame = 1;
			break;
		case INTROBOOST:
			SetAction(JUMP);
			frame = 1;
			break;
		case EXIT:
			SetAction(EXITWAIT);
			frame = 0;
			break;
		case EXITBOOST:
			
			break;
		case SPAWNWAIT:
			SetAction(INTRO);
			frame = 0;
			break;
		case GOALKILL:
			facingRight = true;
			SetAction(GOALKILLWAIT);
			frame = 0;
			owner->scoreDisplay->Activate();
			break;
		case GOALKILLWAIT:
			//action = EXIT;
			frame = 0;
			break;
		case NEXUSKILL:
			SetAction(SEQ_FLOAT_TO_NEXUS_OPENING);
			frame = 0;
			//grab the nexus and start its sequence.
			break;
		case DEATH:
			frame = 0;
			break;
		case WAITFORSHIP:
			frame = 0;
			break;
		case SEQ_CRAWLERFIGHT_STRAIGHTFALL:
			frame = 1;
			break;
		case SEQ_CRAWLERFIGHT_LAND:
			frame = 0;
			SetAction(SEQ_CRAWLERFIGHT_STAND);
			break;
		case SEQ_CRAWLERFIGHT_STAND:
			//action = SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY;
			frame = 0;
			break;
		case SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY:
			//action = SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED;
			//groundSpeed = 0;
			frame = 0;
			break;
		case SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED:
			frame = 0;
			break;
		case SEQ_CRAWLERFIGHT_DODGEBACK:
			frame = 1;
			break;
		case GETPOWER_AIRDASH_MEDITATE:
			SetAction(GETPOWER_AIRDASH_FLIP);
			frame = 0;
			break;
		case GETPOWER_AIRDASH_FLIP:
			hasPowerAirDash = true;
			SetAction(STAND);
			frame = 0;
			break;
		case SEQ_WAIT:
			frame = 0;
			break;
		case SEQ_ENTERCORE1:
			frame = actionLength[SEQ_ENTERCORE1] - 1;
			break;
		case SEQ_LOOKUP:
			frame = 0;
			break;
		case SEQ_KINTHROWN:
			frame = actionLength[SEQ_KINTHROWN] = 1;
			break;
		case SEQ_KNEEL:
			frame = 0;
			break;
		case SEQ_KNEEL_TO_MEDITATE:
			action = SEQ_MEDITATE_MASKON;
			frame = 0;
			break;
		case SEQ_MEDITATE_MASKON:
			frame = 0;
			break;
		case SEQ_MASKOFF:
			action = SEQ_MEDITATE;
			frame = 0;
			break;
		case SEQ_MEDITATE:
			frame = 0;
			break;
		case SEQ_FLOAT_TO_NEXUS_OPENING:
			action = SEQ_FADE_INTO_NEXUS;
			frame = 0;
			break;
		case SEQ_FADE_INTO_NEXUS:
			frame = actionLength[SEQ_FADE_INTO_NEXUS] - 1;
			owner->SetActiveSequence(owner->nexus->insideSeq);
			break;
		case SEQ_TURNFACE:
			frame = 0;
			break;
		case GETSHARD:
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

	hasPowerAirDash = startHasPowerAirDash;
	hasPowerGravReverse = startHasPowerGravReverse;
	hasPowerBounce = startHasPowerBounce;
	hasPowerGrindBall = startHasPowerGrindBall;
	hasPowerTimeSlow = startHasPowerTimeSlow;
	hasPowerLeftWire = startHasPowerLeftWire;
	hasPowerRightWire = startHasPowerRightWire;

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
	
	slowMultiple = 1;
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

	followerPos = position;
	followerVel = V2d( 0, 0 );
	enemiesKilledThisFrame = 0;
	gateTouched = NULL;


	if( owner != NULL && !owner->hasShipEntrance )
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
	
	record = 0;
	recordedGhosts = 0;
	blah = false;
	receivedHit = NULL;
	speedParticleCounter = 1;
	speedLevel = 0;
	currentSpeedBar = 0;//60;

	bounceFlameOn = false;
	scorpOn = false;

	if( hasPowerLeftWire )
	{
		leftWire->Reset();
	}
	if( hasPowerRightWire )
	{
		rightWire->Reset();
	}
	
	//powerBar.Reset();
	lastWire = 0;
	desperationMode = false;

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

	SetExpr( Actor::Expr::Expr_NEUTRAL );

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

void Actor::HandleAirTrigger()
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

void Actor::UpdatePrePhysics()
{

	if ( owner != NULL && owner->stormCeilingOn)
	{
		//if (ground == NULL && grindEdge == NULL && bounceEdge == NULL)
		//{
		//	if (position.y < owner->stormCeilingHeight)
		//	{
		//		/*hitlagFrames = 10;
		//		hitstunFrames = 60;
		//		invincibleFrames = 0;

		//		
		//		owner->Pause(6);

		//		

		//		SetExpr(Expr::Expr_HURT);*/

		//		
		//	}

		//	if (dmgRet > 0 && !desperationMode)
		//	{
		//		desperationMode = true;
		//		despCounter = 0;
		//		//action = DEATH;
		//		//frame = 0;
		//	}




		//	receivedHit = NULL;
		//	}
		//}
	}

	//for gravity grass
	if (ground != NULL && reversed && !hasPowerGravReverse && gravityGrassCount == 0 )
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

	HandleAirTrigger();
	//cout << "Start frame" << endl;
	if (airTrigBehavior == AT_AUTORUNRIGHT)
	{ 
		if (ground != NULL)
		{
			owner->adventureHUD->Hide(60);
			SetAction(AUTORUN);
			frame = 0;
			maxAutoRunSpeed = 25;
			/*if (groundSpeed > 0)
			{
				
			}
			else
			{
				groundSpeed = 10;
			}*/
			facingRight = true;
			airTrigBehavior = AT_NONE;
		}
	}
	//cout << "JFRAME BEHI: " << frame << endl;

	if ( kinRing != NULL && kinRing->powerRing != NULL && action != DEATH && owner->adventureHUD->IsShown() 
		&& ( owner->currentZone == NULL || owner->currentZone->zType != Zone::MOMENTA))
	{
		if (owner->drain && !desperationMode 
			&& !IsIntroAction( action ) && !IsGoalKillAction( action ) && !IsExitAction( action ) && !IsSequenceAction( action )
			&& owner->activeSequence == NULL )
		{
			drainCounter++;
			if (drainCounter == drainCounterMax)
			{
				int res = kinRing->powerRing->Drain(drainAmount);//powerWheel->Use( 1 );	
				//cout << "drain by " << drainAmount << endl;
				if (res > 0)
				{
					desperationMode = true;
					despCounter = 0;
				}
				drainCounter = 0;
			}
		}
	}
	

	enemiesKilledLastFrame = enemiesKilledThisFrame;
	enemiesKilledThisFrame = 0;
	

	//cout << "action: " << action << endl;
	
	if (currAirTrigger != NULL)
	{
		HandleAirTrigger();
	}

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
		//sh.setUniform( "despColor", ColorGL(currentDespColor) );
		despFaceShader.setUniform( "toColor", ColorGL(currentDespColor) );
		playerDespShader.setUniform("toColor", ColorGL(currentDespColor));
		//sh.setUniform( "auraColor", ColorGL(auraColor) );
		//currentDespColor


		//cout << "desperation: " << despCounter << endl;
		despCounter++;
		if( kinRing != NULL && despCounter == maxDespFrames )
		{
			desperationMode = false;
			if (kinRing->powerRing->IsEmpty())
			{

				SetAction(DEATH);
				rightWire->Reset();
				leftWire->Reset();
				slowCounter = 1;
				frame = 0;

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

	if( kinRing != NULL && hasPowerClones &&  ( (currInput.RUp() && !prevInput.RUp()) || ( currInput.rightPress && !prevInput.rightPress ) ) )
	{
		if( record == 0 )
		{
			//SaveState();
			//owner->SaveState();
			recordedGhosts = 1;
			ghosts[record]->currFrame = 0;
			ghostFrame = 0;
			//owner->powerWheel->Use( 20 );
			kinRing->powerRing->Drain(20);
			record++;
			changingClone = true;
			percentCloneChanged = 0;
			sess->Pause( 60 );
			//percentCloneRate = .01;
		}
		else
		{
			if( recordedGhosts < MAX_GHOSTS )
			{
				cout << "creating ghost: " << recordedGhosts + 1 << ", of " << MAX_GHOSTS << endl;
				//LoadState();
				//owner->LoadState();
				recordedGhosts++;
				ghosts[record-1]->totalRecorded = ghosts[record-1]->currFrame;
				ghosts[record]->currFrame = 0;
				ghostFrame = 1;
				record++;
				changingClone = true;
				sess->Pause( 60 );
				percentCloneChanged = 0;
			}
			
			
		}

		//record = true;
		blah = false;
	}
		

		
	if( kinRing != NULL && record > 0 && ( ( currInput.RDown() && !prevInput.RDown() ) || ghosts[record-1]->currFrame == PlayerGhost::MAX_FRAMES - 1 ) )
	{
		//record = false;
		ghosts[record-1]->totalRecorded = ghosts[record-1]->currFrame;
		record = 0;
		//LoadState();
		//owner->LoadState(); 
		blah = true;
		ghostFrame = 1;
		//cout << "recordedGhosts: " << recordedGhosts << endl;
		//owner->powerBar.Charge( 20 );
		kinRing->powerRing->Fill(20);
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

	if( IsIntroAction( action ) || (IsGoalKillAction(action) && action != GOALKILLWAIT) || action == EXIT 
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || IsSequenceAction( action ) || action == EXITBOOST )
	{
		/*if (action == SPAWNWAIT && frame == actionLength[SPAWNWAIT] - 6)
		{
			ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/enter_fx_320x320.png", 320, 320), spriteCenter, false, 0, 6, 2, true);
		}*/
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
				ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/exitenergy_0_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}
			else if (frame == 6 * 2)
			{
				ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/exitenergy_1_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}
			else if (frame == 6 * 4)
			{
				ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/exitenergy_2_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
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
	/*else if (action == GRABSHIP)
	{

		if (currInput.A && !prevInput.A && owner->scoreDisplay->waiting)
		{
			owner->scoreDisplay->Deactivate();
		}
	}*/

	/*if( bounceAttackHit && enemiesKilledLastFrame > 0 )
	{
		action = BOUNCESWORDBOOST;
		frame = 0;
		if( facingRight )
			bounceSwordBoostDir = normalize( V2d( 1, 1 ) );
		else
			bounceSwordBoostDir = normalize( V2d( -1, 1 ) );
		break;
	}*/

	V2d gNorm;
	if( ground != NULL )
		gNorm = ground->Normal();

	if( receivedHit != NULL && action != DEATH )
	{
		hitlagFrames = receivedHit->hitlagFrames;
		hitstunFrames = receivedHit->hitstunFrames;
		setHitstunFrames = hitstunFrames;
		invincibleFrames = receivedHit->hitstunFrames + 20;//25;//receivedHit->damage;
		
		ActivateEffect( EffectLayer::IN_FRONT, ts_fx_hurtSpack, position, true, 0, 12, 1, facingRight );
		sess->Pause( hitlagFrames );

		ActivateSound( S_HURT );

		if( kinRing != NULL )
			kinRing->powerRing->Drain(receivedHit->damage);

		//kinMask->SetExpr( KinMask::Expr::Expr_HURT );
		//expr = Expr::Expr_HURT;
		
		//cout << "damaging player with: " << receivedHit->damage << endl;
		int dmgRet = 0;//owner->powerRing->Drain(receivedHit->damage);
		//bool dmgSuccess = owner->powerWheel->Damage( receivedHit->damage );

		if (ground != NULL)
		{
			if (reversed)
				reversed = false;
			ground = NULL;
			SetAction(JUMP);
			frame = 1;
			//velocity = V2d(0, 0);
			//velocity = normalize( ground->v)
		}

		if( true )
		{
			bool onRail = IsOnRailAction(action) || (grindEdge != NULL && action == JUMPSQUAT);
			if( grindEdge != NULL && !onRail)
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
						
						if( kinRing != NULL )
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

						//owner->powerWheel->Damage( receivedHit->damage );
						if( kinRing != NULL )
							kinRing->powerRing->Drain(receivedHit->damage);
						
						//apply extra damage since you cant stand up
					}
					else
					{
						//abs( e0n.x ) < wallThresh )

						if( !hasPowerGravReverse || ( abs( grindNorm.x ) >= wallThresh || !hasGravReverse ) || grindEdge->IsInvisibleWall() )
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
							SetAction(AIRHITSTUN);
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



							ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
							ActivateSound( S_GRAVREVERSE );
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
				SetAction(GROUNDHITSTUN);
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
		
		if( dmgRet > 0 && !desperationMode )
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
	justToggledBounce = false;
	if( bounceFlameOn )
	{
		if( toggleBounceInput )
		{
			if( currInput.X && !prevInput.X )
			{
				BounceFlameOff();
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
				oldBounceEdge = NULL;
				bounceGrounded = false;
				scorpOn = false;
			}
		}
	}
	
	
	if( action == AIRHITSTUN )
	{
		if( hitstunFrames == 0 )
		{
			if (!TryDoubleJump())
			{
				if (!AirAttack())
				{
					if (!TryAirDash())
					{
						SetActionExpr(JUMP);
						frame = 1;
						holdJump = false;
					}
				}
			}

			
			
			//prevInput = ControllerState();
		}
		
	}
	else if( action == GROUNDHITSTUN )
	{
		if( hitstunFrames == 0 )
		{
			if (stunBufferedJump)
			{
				if (stunBufferedAttack != Action::Count )
				{	
					SetActionExpr(JUMPSQUAT);
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
					SetActionExpr(JUMPSQUAT);
					frame = 0;
				}
				
			}
			else if (!GroundAttack())
			{
				if (stunBufferedDash )
				{
					SetActionExpr(DASH);
					frame = 0;
				}
				else
				{
					SetActionExpr(LAND);
					frame = 0;
				}
				//if( !)
			}
			
			//prevInput = ControllerState();
		}
		
	}

	

	switch( action )
	{
	case STAND:
		{
		if (BasicGroundAction(gNorm))
			break;


			if (TrySprint(gNorm))
			{
				break;
			}
			else if (currInput.LLeft() || currInput.LRight())
			{
				SetActionExpr(RUN);
				frame = 0;
				//facingRight = currInput.LRight();
				break;
			}
			else
			{
				if (currInput.LDown() || currInput.LUp())
				{
					SetActionExpr(SLIDE);
					frame = 0;
					break;
				}
			}
			break;
		}
	case RUN:
		{
			if (BasicGroundAction(gNorm))
			break;


			if (TrySlide() )
			{
				break;
			}
			else
			{
				if (TrySprint(gNorm)) break;
			}
			
			break;
		}
	case JUMP:
		{
			if( bufferedAttack != JUMP )
			{
				SetAction( bufferedAttack );
				bufferedAttack = JUMP;
				frame = 0;
				break;
			}

			BasicAirAction();

			break;
		}
	case JUMPSQUAT:
		{
			if( currInput.rightShoulder && !prevInput.rightShoulder && bufferedAttack == JUMP )
			{
				if( currInput.LUp() )
				{
					bufferedAttack = UAIR; //none
				}
				else if( currInput.LDown() )
				{
					bufferedAttack = DAIR; 
				}
				else
				{
					bufferedAttack = FAIR; 
				}
				//bufferedAttack = true;
			}
			
			if( frame == actionLength[JUMPSQUAT] - 1 )
			{
				SetActionExpr( JUMP );
				frame = 0;
				groundSpeed = storedGroundSpeed;
			}

			break;
		}
	case BACKWARDSDOUBLE:
	case DOUBLE:
		{
			if ((frame == 1 || (frame == 0 && slowCounter > 1)) && doubleJumpBufferedAttack != DOUBLE)
			{
				SetAction(doubleJumpBufferedAttack);
				doubleJumpBufferedAttack = DOUBLE;
				frame = 0;
				break;
			}
			BasicAirAction();

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

			if (hasPowerBounce && currInput.X && !bounceFlameOn)
			{
				//bounceGrounded = true;
				BounceFlameOn();
				oldBounceEdge = NULL;
				//break;
			}
			else if (!(hasPowerBounce && currInput.X) && bounceFlameOn)
			{
				//bounceGrounded = false;
				BounceFlameOff();
			}

			if (currInput.A && !prevInput.A)
			{
				SetActionExpr(JUMPSQUAT);
				frame = 0;
				////runTappingSound.stop();
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
						
						SetAction(STEEPCLIMB);

						if (SteepClimbAttack())
						{

						}
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						SetAction(STEEPSLIDE);
						if (SteepSlideAttack())
						{

						}
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
						//action = DASH;
						//frame = 0;
						SetActionExpr(DASH);

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
					else if( currInput.LDown() || currInput.LUp() )
					{
						SetAction(SLIDE);
						frame = 0;
					}
					else
					{
						SetActionExpr( STAND );
						frame = 0;
					}

					if (GroundAttack())
					{
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
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						SetAction(STEEPCLIMB);

						if (SteepClimbAttack())
						{

						}
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
						
						SetAction(STEEPSLIDE);

						if (SteepSlideAttack())
						{

						}

						frame = 0;
						break;
					}
					
				}
				else
				{
					//if( currInput.A && !prevInput.A )
					//{
					//	SetActionExpr( JUMPSQUAT );
					//	frame = 0;
					//	////runTappingSound.stop();
					//	break;
					//}

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
						
						SetActionExpr(DASH);
						//action = DASH;
						//frame = 0;
						/*re->Reset();
						re1->Reset();*/

						
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
						frame = 0;
						facingRight = currInput.LRight();
					}
					else if( currInput.LDown() || currInput.LUp() )
					{
						SetAction(SLIDE);
						frame = 0;
					}
					else
					{
						SetActionExpr( STAND );
						frame = 0;
					}

					if (GroundAttack())
						break;
				}
			}
		

			break;
		}
	case GRAVREVERSE:
		{
		//buffered grind ball works

			if( hasPowerGrindBall && currInput.Y )//&& !prevInput.Y )
			{
				//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
				if( !reversed || ( hasPowerGravReverse && reversed ) )
				{
					groundSpeed = storedReverseSpeed;
					SetActionGrind();
					break;
				}
			}

			//groundSpeed = 0;
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
							
						SetAction(STEEPCLIMB);

						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						SetAction(STEEPSLIDE);
						frame = 0;
						break;
					}
					
				}
				else
				{

					if( ( currInput.B && !( reversed && (!currInput.LLeft() && !currInput.LRight() ) ) ) || !canStandUp )
					{
						cout << "storedreversesddddpeed: " << storedReverseSpeed << endl;
						groundSpeed = storedReverseSpeed;
						//action = DASH;
						//frame = 0;
						SetActionExpr(DASH);

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
					{
						cout << "storedreversespeed: " << storedReverseSpeed << endl;
						groundSpeed = storedReverseSpeed;
						SetActionExpr( RUN );
						frame = 0;
					}
					else if(GroundAttack())
					{
						break;
					}
					else if( !currInput.LDown() )
					{
						SetAction(STAND);
						frame = 0;
						/*bool okay = false;
						if( frame > 0 && !prevInput.LUp() )
						{
							action = SLIDE;
							frame = 0;
						}*/
					}
					else
					{
						/*SetActionExpr( STAND );
						frame = 0;*/
					}
				}
			}
			else
			{
				assert( 0 && "should be reversed here" );
			}

			
			SetCurrHitboxes(shockwaveHitboxes, frame);
			

			break;
		}
	case SPRINGSTUN:
	{
		if (springStunFrames == 0)
		{
			SetAction(JUMP);
			frame = 1;
		}
		break;
	}
	case SPRINGSTUNTELEPORT:
	{
		if (springStunFrames == 0)
		{
			//oldTeleporter->ReceiveRecover();
			SetAction(JUMP);
			frame = 1;
			position = teleportSpringDest;
			velocity = teleportSpringVel;
		}
		break;
	}
	case SWINGSTUN:
	{
		if (!GlideAction())
		{
			if (springStunFrames == 0)
			{
				SetAction(JUMP);
				frame = 1;
				if (velocity.x < 0)
				{
					facingRight = false;
				}
				else if (velocity.x > 0)
				{
					facingRight = true;
				}
			}
		}
		else
		{
			springStunFrames = 0;
		}
		break;
	}
	case SPRINGSTUNAIRBOUNCE:
	{
		if (!GlideAction())
		{
			if (springStunFrames == 0)
			{
				SetAction(JUMP);
				frame = 1;
				if (velocity.x < 0)
				{
					facingRight = false;
				}
				else if (velocity.x > 0)
				{
					facingRight = true;
				}
			}
		}
		else
		{
			springStunFrames = 0;
		}
		break;
	}
	case SPRINGSTUNBOUNCE:
		if (!GlideAction())
		{
			if (springStunFrames == 0)
			{
				SetAction(JUMP);
				frame = 1;
				if (velocity.x < 0)
				{
					facingRight = false;
				}
				else if (velocity.x > 0)
				{
					facingRight = true;
				}
			}
		}
		else
		{
			springStunFrames = 0;
		}
		break;
	case SPRINGSTUNGLIDE:
	{
		if (!GlideAction())
		{
			if (springStunFrames == 0)
			{
				SetAction(JUMP);
				frame = 1;
				if (velocity.x < 0)
				{
					facingRight = false;
				}
				else if (velocity.x > 0)
				{
					facingRight = true;
				}
			}
		}
		else
		{
			springStunFrames = 0;
		}
		break;
	}
	case GLIDE:
	{
		GlideAction();
		break;
	}
	case WALLCLING:
		{
			if( !currInput.LDown() && ( (facingRight && currInput.LRight()) || (!facingRight && currInput.LLeft() ) ) )
			{
				SetAction(WALLJUMP);
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
			else if( TryDoubleJump() )
			{
				break;
			}
			else if( currInput.LDown() )
			{
				SetActionExpr( JUMP );
				frame = 1;
				holdJump = false;
			}
			else //if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
				bool rightStickSwing = (currInput.RDown() && !prevInput.RDown())
					|| (currInput.RLeft() && !prevInput.RLeft())
					|| (currInput.RUp() && !prevInput.RUp())
					|| (currInput.RRight() && !prevInput.RRight());

				if (normalSwing || (rightStickSwing && (currInput.RLeft() || currInput.RRight())))
				{
					SetActionExpr(WALLATTACK);
					frame = 0;
				}
				/*else if (!normalSwing && (rightStickSwing && (currInput.RUp() || currInput.RDown())))
				{
					AirAttack();
				}*/
			}


			break;
		}
	case WALLJUMP:
		{
			if ((frame == 1 || (frame == 0 && slowCounter > 1)) && wallJumpBufferedAttack != WALLJUMP)
			{
				SetAction(wallJumpBufferedAttack);
				wallJumpBufferedAttack = WALLJUMP;
				frame = 0;
				break;
			}

			BasicAirAction();
			break;
		}
	case WALLATTACK:
		{
			if( !BasicAirAttackAction() && frame > 6 )
			{
				if( currInput.LDown() )
				{
					SetAction(JUMP);
					frame = 1;
					break;
				}
				else
				{
					if( !facingRight )
					{
						if( currInput.LLeft() )
						{
							SetAction(WALLJUMP);
							frame = 0;
							break;
						}
					}
					else
					{
						if( currInput.LRight() )
						{
							SetAction(WALLJUMP);
							frame = 0;
							break;
						}
					}
				}
			}
			break;
		}
	case FAIR:
		{
			BasicAirAttackAction();
			break;
		}
	case DAIR:
		{
			BasicAirAttackAction();
			break;
		}
	case UAIR:
		{
			BasicAirAttackAction();
			break;
		}
	case DIAGUPATTACK:
		{
			BasicAirAttackAction();
			break;
		}
	case DIAGDOWNATTACK:
		{
			BasicAirAttackAction();
			break;
		}
	case DASH:
		{
			
			//don't break becaus eyou can cancel this
			CheckBounceFlame();

			if (TryGrind())break;

			

			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMPSQUAT );
				frame = 0;
				break;
			}
			
			if (GroundAttack())
			{
				break;
			}

			if (BasicSteepAction(gNorm))
				break;

			if( canStandUp )
			{
				if( !currInput.B )
				{
					if (currBBoostCounter > maxBBoostCount - 20 )
					{
						double dashFactor = 3.0;
						double ag = abs(groundSpeed);

						if (ag > 30)
						{
							dashFactor = 3.0;
						}
						else
						{
							dashFactor = 2.0;
						}

						double bboost = GetDashSpeed() / dashFactor;

						if (groundSpeed > 0)
						{
							groundSpeed += bboost;
						}
						else
						{
							groundSpeed -= bboost;
						}
						//currBBoostCounter = 0;
					}




					if( currInput.LLeft() || currInput.LRight() )
					{
						SetActionExpr( RUN );
						frame = 0;
					}
					else if( currInput.LDown() || currInput.LUp() )
					{
						SetAction(SLIDE);
						frame = 0;
					}
					else
					{
						SetAction(STAND);
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
				BounceFlameOn();
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				BounceFlameOff();
			}

			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				BounceFlameOff();
				SetActionGrind();
				break;
			}

			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMPSQUAT );
				frame = 0;
				break;
			}
			else if( GroundAttack() )
			{
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
						
						SetAction(STEEPCLIMB);

						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						
						SetAction(STEEPSLIDE);
						frame = 0;
						break;
					}
					
				}
				else
				{
					//you can't dash on the ceiling with no horizontal input. probably a weakness
					if( ( currInput.B && !prevInput.B/*&& !( reversed && (!currInput.LLeft() && !currInput.LRight() ) )*/ ) || !canStandUp )
					{
						//action = DASH;
						/*re->Reset();
						re1->Reset();*/
						//frame = 0;
						SetActionExpr(DASH);

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
					else if( currInput.LDown() || currInput.LUp() )
					{
						SetAction(SLIDE);
						frame = 0;
					}
					else
					{
						SetAction(STAND);
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
						SetAction(STEEPCLIMB);
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						SetAction(STEEPSLIDE);
						frame = 0;
						break;
					}
					
				}
				else
				{
					if( (currInput.B && !prevInput.B) || !canStandUp )
					{
						//cout << "start dash" << endl;
						//action = DASH;
						/*re->Reset();
						re1->Reset();*/
						//frame = 0;
						SetActionExpr(DASH);

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
					else if( currInput.LDown() || currInput.LUp() )
					{
						SetAction(SLIDE);
						frame = 0;
					}
					else
					{
						SetAction(STAND);
						frame = 0;
					}
				}
			}

			break;
		}
	case SPRINT:
		{
		if (BasicGroundAction(gNorm))
			break;

			if(!( currInput.LLeft() || currInput.LRight() ))
			{
				if( currInput.LDown() || currInput.LUp() )
				{
					SetAction(SLIDE);
					frame = 0;
					break;
				}
				else
				{
					SetActionExpr(STAND);
					frame = 0;
					break;
				}
				/*else if( currInput.LUp() && ( (gNorm.x < 0 && facingRight) || (gNorm.x > 0 && !facingRight) ) )
				{
					break;
				}
				else
				{
					SetAction(STAND);
					frame = 0;
				}*/
			
				
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
			if( currAttackHit )//&& frame > 0 )
			{
				if( hasPowerBounce && currInput.X && !bounceFlameOn )
				{
					BounceFlameOn();
				}
				else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
				{
					//bounceGrounded = false;
					BounceFlameOff();
				}

				if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
				{
					BounceFlameOff();
					SetActionGrind();
					//dashStartSound.setLoop( false );
					////runTappingSound.stop();
					break;
				}

				//if( reversed )
				//{
				//	if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				//	{
				//		if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
				//		{
				//			SetAction(STEEPCLIMB);
				//			frame = 0;
				//			break;
				//		}
				//		else
				//		{
				//			SetAction(STEEPSLIDE);
				//			frame = 0;
				//			break;
				//		}
				//	}
				//}
				//else
				//{
				//	if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				//	{
				//		if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
				//		{
				//			//cout << "steep clzzzimb" << endl;
				//			SetAction(STEEPCLIMB);
				//			frame = 0;
				//			break;
				//		}
				//		else
				//		{
				//			SetAction(STEEPSLIDE);
				//			frame = 0;
				//			break;
				//		}
				//	}
				//}

				if( (currInput.A && !prevInput.A) || pauseBufferedJump )
				{
					SetActionExpr( JUMPSQUAT );
					frame = 0;
					break;
				}

				if (GroundAttack())
				{
					break;
				}

				if ( pauseBufferedDash || (currInput.B && !prevInput.B ))
				{
					if (standNDashBoostCurr == 0)
					{
						standNDashBoost = true;
						standNDashBoostCurr = standNDashBoostCooldown;
					}
					SetActionExpr(DASH);
					frame = 0;
					break;
				}
			}
			break;
		}
	case STEEPSLIDEATTACK:
		{
			if( currAttackHit )//&& frame > 0 )
			{
				if( hasPowerBounce && currInput.X && !bounceFlameOn )
				{
					BounceFlameOn();
				}
				else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
				{
					//bounceGrounded = false;
					BounceFlameOff();
				}

				if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
				{
					SetActionGrind();
					break;
				}

				if( currInput.A && !prevInput.A || pauseBufferedJump )
				{
					SetActionExpr( JUMPSQUAT );
					frame = 0;
					break;
				}

				if (SteepSlideAttack())
					break;

				if( currInput.B && !prevInput.B )
				//if( currInput.A && !prevInput.A )
				{
					if( gNorm.x < 0 && currInput.LRight() )
					{
						SetAction(STEEPCLIMB);
						facingRight = true;
						groundSpeed = steepClimbBoostStart;
						frame = 0;
					}
					else if( gNorm.x > 0 && currInput.LLeft() )
					{
						SetAction(STEEPCLIMB);
						facingRight = false;
						groundSpeed = -steepClimbBoostStart;
						frame = 0;
					}
					break;
				}

				//if( reversed )
				//{
				//	if( -gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				//	{
				//		SetAction(LAND2);
				//		frame = 0;
				//	}
				//}
				//else
				//{
				//	if( gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				//	{
				//		cout << "is it really this wtf" << endl;
				//		SetAction(LAND2);
				//		frame = 0;
				//		//not steep
				//	}
				//	else
				//	{
				//		//is steep
				//		if( ( gNorm.x < 0 && groundSpeed > 0 ) || (gNorm.x > 0 && groundSpeed < 0 ) )
				//		{
				//			SetAction(STEEPCLIMB);
				//			frame = 1;
				//		}
				//	}
				//}
			}
			break;
		}
	case STEEPCLIMBATTACK:
		{
			if( currAttackHit )//&& frame > 0 )
			{
				if( hasPowerBounce && currInput.X && !bounceFlameOn )
				{
					BounceFlameOn();
				}
				else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
				{
					//bounceGrounded = false;
					BounceFlameOff();
				}

				if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
				{
					SetActionGrind();
					break;
				}

				if( currInput.A && !prevInput.A || pauseBufferedJump )
				{
					SetActionExpr( JUMPSQUAT );
					frame = 0;
					break;
				}

				if (SteepClimbAttack())
				{
					break;
				}

				//if( currInput.B && !prevInput.B )
				////if( currInput.A && !prevInput.A )
				//{
				//	if( gNorm.x < 0 && currInput.LRight() )
				//	{
				//		action = STEEPCLIMB;
				//		facingRight = true;
				//		groundSpeed = 10;
				//		frame = 0;
				//	}
				//	else if( gNorm.x > 0 && currInput.LLeft() )
				//	{
				//		action = STEEPCLIMB;
				//		facingRight = false;
				//		groundSpeed = -10;
				//		frame = 0;
				//	}
				//	break;
				//}

				//if( reversed )
				//{
				//	if( -gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				//	{
				//		SetAction(LAND2);
				//		frame = 0;
				//	}
				//}
				//else
				//{
				//	if( gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				//	{
				//		cout << "is it really this wtf" << endl;
				//		SetAction(LAND2);
				//		frame = 0;
				//		//not steep
				//	}
				//	else
				//	{
				//		//is steep
				//		if( ( gNorm.x < 0 && groundSpeed < 0 ) || (gNorm.x > 0 && groundSpeed > 0 ) )
				//		{
				//			SetAction(STEEPSLIDE);
				//			frame = 0;
				//			//frame = 1;
				//		}
				//	}
				//}
			}
			
			break;
		}
	case GRINDBALL:
		{
			framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
			bool j = currInput.A && !prevInput.A;
			//bool isntWall = grindEdge->Normal().y != 0;
			if( !currInput.Y || j )//&& grindEdge->Normal().y < 0 )
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
						if( grindSpeed > 0 )
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

						if( !j )
						{
							ground = grindEdge;
							edgeQuantity = grindQuantity;
							SetAction(LAND);
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
						}
						else
						{
							ground = grindEdge;
							edgeQuantity = grindQuantity;
							groundSpeed = grindSpeed;
							SetActionExpr(JUMPSQUAT);
							frame = 0;
						}


						grindEdge = NULL;
						reversed = false;
					}

				}
				else if( grindNorm.y > 0 )
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
						if( !hasPowerGravReverse || ( abs( grindNorm.x ) >= wallThresh ) || j || grindEdge->IsInvisibleWall() )//|| !hasGravReverse ) )
						{
							if( grindSpeed < 0 )
							{
								facingRight = true;
							}
							else
							{
								facingRight = false;
							}


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
							frame = 1;
							ground = NULL;
							grindEdge = NULL;
							reversed = false;
						}
						else
						{
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

							if( grindSpeed < 0 )
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

							ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
							ActivateSound( S_GRAVREVERSE );
						}
					}
				}	
				else
				{
					framesInAir = 0;
					SetActionExpr(JUMP);
					frame = 1;
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
				//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			}
			else if( currInput.B && !prevInput.B )
			{
				V2d op = position;
				
				//V2d op = position;

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
						SetAction(GRINDLUNGE);
						frame = 0;

						V2d grindNorm = grindEdge->Normal();
						V2d gDir = normalize( grindEdge->v1 - grindEdge->v0 );
						lungeNormal = grindNorm;

						double lungeSpeed;
						if( speedLevel == 0 )
						{
							lungeSpeed = grindLungeSpeed0;
						}
						else if( speedLevel == 1 )
						{
							lungeSpeed = grindLungeSpeed1;
						}
						else if( speedLevel == 2 )
						{
							lungeSpeed = grindLungeSpeed2;
						}
						//double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
						//double extra = f * grindLungeExtraMax;
						
						velocity = lungeNormal * lungeSpeed;//( grindLungeSpeed + extra );

						/*double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
						double extra = f * grindLungeExtraMax;
						
						velocity = lungeNormal * ( grindLungeSpeed + extra );*/

						/*if( currInput.A )
						{
							velocity += gDir * grindSpeed;
							V2d normV = normalize( velocity );
							lungeNormal = V2d( normV.y, -normV.x );
						}*/
						
						//grindEdge = NULL;

						facingRight = (grindNorm.x > 0);

						grindEdge = NULL;
						ground = NULL;
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
						SetAction(GRINDLUNGE);
						frame = 0;

						V2d grindNorm = grindEdge->Normal();
						V2d gDir = normalize( grindEdge->v1 - grindEdge->v0 );

						lungeNormal = grindNorm;
						double lungeSpeed;
						if( speedLevel == 0 )
						{
							lungeSpeed = grindLungeSpeed0;
						}
						else if( speedLevel == 1 )
						{
							lungeSpeed = grindLungeSpeed1;
						}
						else if( speedLevel == 2 )
						{
							lungeSpeed = grindLungeSpeed2;
						}
						//double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
						//double extra = f * grindLungeExtraMax;
						
						velocity = lungeNormal * lungeSpeed;//( grindLungeSpeed + extra );

						facingRight = (grindNorm.x > 0);

						grindEdge = NULL;
						ground = NULL;
					}
				}		
				//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			}
			else if( currInput.rightShoulder && !prevInput.rightShoulder )
			{
				SetAction(GRINDATTACK);
				frame = 0;
			}
			

			if( action != GRINDBALL && action != GRINDATTACK )
			{
				hurtBody.isCircle = false;
				hurtBody.rw = 7;
				hurtBody.rh = normalHeight;
			}
				
			break;
		}
	case RAILGRIND:
	{
		Rail *rail = (Rail*)grindEdge->info;
		if (currInput.A && !prevInput.A)
		{
			if (GameSession::IsWall(grindEdge->Normal()) < 0 ) //not wall
			{
				SetActionExpr(JUMPSQUAT);
				//frame = 0;
			}
			else
			{
				hasDoubleJump = true;
				TryDoubleJump();
				grindEdge = NULL;
			}
			//if( abs( grindEdge->Normal().y ) )
			
			frame = 0;
			
			regrindOffCount = 0;
			break;
		}
		else if (currInput.B && !prevInput.B)
		{
			SetAction(RAILDASH);
			frame = 0;
			grindEdge = NULL;
		}
		if ( currInput.Y && !prevInput.Y && framesGrinding > 1)
		{
			SetAction(JUMP);
			grindEdge = NULL;
			frame = 1;
			regrindOffCount = 0;
			break;
		}
		break;
	}
	case RAILSLIDE:
	{
		Rail *rail = (Rail*)grindEdge->info;
		if (currInput.A && !prevInput.A)
		{
			
			facingRight = IsRailSlideFacingRight();

			if (GameSession::IsWall(grindEdge->Normal()) < 0) //not wall
			{
				SetActionExpr(JUMPSQUAT);
				//frame = 0;
			}
			else
			{
				hasDoubleJump = true;
				TryDoubleJump();
				grindEdge = NULL;
			}

			
			

			//if( abs( grindEdge->Normal().y ) )

			frame = 0;

			regrindOffCount = 0;
			break;
		}

		if (TryAirDash())
		{
			facingRight = IsRailSlideFacingRight();
			grindEdge = NULL;
			hasDoubleJump = true;
			break;
		}

		if (AirAttack())
		{
			facingRight = IsRailSlideFacingRight();
			grindEdge = NULL;
			hasDoubleJump = true;
			hasAirDash = true;
			
			break;
		}
		break;
	}
	case RAILDASH:
	{
		if (!BasicAirAction())
		{
			if (!currInput.B)
			{
				SetAction(JUMP);
				frame = 1;
			}
		}
		break;
	}
	case GRINDATTACK:
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
						if( grindSpeed > 0 )
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
						ground = grindEdge;
						edgeQuantity = grindQuantity;
						SetAction(LAND);
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

						if( !hasPowerGravReverse || ( abs( grindNorm.x ) >= wallThresh ) || grindEdge->IsInvisibleWall())//|| !hasGravReverse ) )
						{
							if( grindSpeed < 0 )
							{
								facingRight = true;
							}
							else
							{
								facingRight = false;
							}


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
							grindEdge = NULL;
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

							if( grindSpeed < 0 )
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
							grindEdge = NULL;
							reversed = true;
							//hasGravReverse = false;

								
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

							SetAction(LAND2);
							frame = 0;
							framesNotGrinding = 0;

							double angle = GroundedAngle();

							ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
							ActivateSound( S_GRAVREVERSE );
						}
					}
				}		
				//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			}
			else if( currInput.B && !prevInput.B )
			{
				V2d op = position;
				
				//V2d op = position;

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
						SetAction(GRINDLUNGE);
						frame = 0;

						V2d grindNorm = grindEdge->Normal();
						V2d gDir = normalize( grindEdge->v1 - grindEdge->v0 );
						lungeNormal = grindNorm;

						double lungeSpeed;
						if( speedLevel == 0 )
						{
							lungeSpeed = grindLungeSpeed0;
						}
						else if( speedLevel == 1 )
						{
							lungeSpeed = grindLungeSpeed1;
						}
						else if( speedLevel == 2 )
						{
							lungeSpeed = grindLungeSpeed2;
						}
						//double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
						//double extra = f * grindLungeExtraMax;
						
						velocity = lungeNormal * lungeSpeed;//( grindLungeSpeed + extra );

						/*double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
						double extra = f * grindLungeExtraMax;
						
						velocity = lungeNormal * ( grindLungeSpeed + extra );*/

						/*if( currInput.A )
						{
							velocity += gDir * grindSpeed;
							V2d normV = normalize( velocity );
							lungeNormal = V2d( normV.y, -normV.x );
						}*/
						
						//grindEdge = NULL;

						facingRight = (grindNorm.x > 0);

						grindEdge = NULL;
						ground = NULL;
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
						SetAction(GRINDLUNGE);
						frame = 0;

						V2d grindNorm = grindEdge->Normal();
						V2d gDir = normalize( grindEdge->v1 - grindEdge->v0 );

						lungeNormal = grindNorm;
						double lungeSpeed;
						if( speedLevel == 0 )
						{
							lungeSpeed = grindLungeSpeed0;
						}
						else if( speedLevel == 1 )
						{
							lungeSpeed = grindLungeSpeed1;
						}
						else if( speedLevel == 2 )
						{
							lungeSpeed = grindLungeSpeed2;
						}
						//double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
						//double extra = f * grindLungeExtraMax;
						
						velocity = lungeNormal * lungeSpeed;//( grindLungeSpeed + extra );

						facingRight = (grindNorm.x > 0);

						grindEdge = NULL;
						ground = NULL;
					}
				}		
				//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			}
			else if( !currInput.rightShoulder )
			{
				SetAction(GRINDBALL);
				frame = 0;
			}
			
			if( action != GRINDBALL && action != GRINDATTACK )
			{
				hurtBody.isCircle = false;
				hurtBody.rw = 7;
				hurtBody.rh = normalHeight;
			}	

			break;
		}
		
	case GRINDLUNGE:
		{
			if (currInput.rightShoulder && !prevInput.rightShoulder)
			{
				SetAction(GRINDSLASH);
				frame = 0;
			}
			else if( !BasicAirAction() && !currInput.B )
			{
				SetAction(JUMP);
				frame = 1;
			}
			break;
		}
	case GRINDSLASH:
		{
			BasicAirAttackAction();
			break;
		}
	case STEEPSLIDE:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				BounceFlameOn();
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				BounceFlameOff();
			}

			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				break;
			}

			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMPSQUAT );
				frame = 0;
				break;
			}

			if (reversed)
			{
				if (-gNorm.y <= -steepThresh || !(approxEquals(offsetX, b.rw) || approxEquals(offsetX, -b.rw)))
				{
					SetAction(LAND2);
					frame = 0;
					if (GroundAttack())
					{

					}
					break;
				}
			}
			else
			{
				if (gNorm.y <= -steepThresh || !(approxEquals(offsetX, b.rw) || approxEquals(offsetX, -b.rw)))
				{
					cout << "is it really this wtf" << endl;
					SetAction(LAND2);
					frame = 0;
					if (GroundAttack())
					{

					}
					break;
					//not steep
				}
				else
				{
					//is steep
					if ((gNorm.x < 0 && groundSpeed > 0) || (gNorm.x > 0 && groundSpeed < 0))
					{
						SetAction(STEEPCLIMB);
						frame = 1;
						if (SteepClimbAttack())
						{
							
						}
						break;
					}
				}
			}

			if (SteepSlideAttack())
			{
				break;
			}

			if( currInput.B && !prevInput.B )
			//if( currInput.A && !prevInput.A )
			{
				if( gNorm.x < 0 && (currInput.LRight() || currInput.LUp() ) )
				{
					SetAction(STEEPCLIMB);
					facingRight = true;
					groundSpeed = steepClimbBoostStart;
					frame = 0;
				}
				else if( gNorm.x > 0 && (currInput.LLeft() || currInput.LUp()) )
				{
					SetAction(STEEPCLIMB);
					facingRight = false;
					groundSpeed = -steepClimbBoostStart;
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

			
			
			break;
		}
	case AIRDASH:
		{
			if (!BasicAirAction())
			{
				if (!currInput.B)//|| ( oldInBubble && !inBubble ) )
				{
					SetActionExpr(JUMP);
					frame = 1;
					holdJump = false;

					if (rightWire->state == Wire::PULLING || leftWire->state == Wire::PULLING)
					{
					}
					else
					{
						//velocity = V2d( 0, 0 );
					}
					break;
				}
			}
			break;
		}
	case STEEPCLIMB:
		{
			if( hasPowerBounce && currInput.X && !bounceFlameOn )
			{
				BounceFlameOn();
			}
			else if( !(hasPowerBounce && currInput.X) && bounceFlameOn )
			{
				//bounceGrounded = false;
				BounceFlameOff();
			}


			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				break;
			}

			if( currInput.A && !prevInput.A )
			{
				SetActionExpr( JUMPSQUAT );
				frame = 0;
				break;
			}

			if (reversed)
			{
				if (-gNorm.y <= -steepThresh || !(approxEquals(offsetX, b.rw) || approxEquals(offsetX, -b.rw)))
				{
					SetAction(LAND2);
					frame = 0;
					if (GroundAttack())
					{

					}
					break;
				}

				if (gNorm.x > 0 && groundSpeed >= 0)
				{
					SetAction(STEEPSLIDE);
					frame = 0;
					facingRight = true;
					if (SteepSlideAttack())
					{

					}
					break;
				}
				else if (gNorm.x < 0 && groundSpeed <= 0)
				{
					SetAction(STEEPSLIDE);
					frame = 0;
					facingRight = false;
					if (SteepSlideAttack())
					{

					}
					break;
				}
			}
			else
			{
				if (gNorm.y <= -steepThresh || !(approxEquals(offsetX, b.rw) || approxEquals(offsetX, -b.rw)))
				{
					//cout << "blahzzz" << endl;
					SetAction(LAND2);
					frame = 0;
					if (GroundAttack())
					{

					}
					break;
					//not steep
				}

				if (gNorm.x > 0 && groundSpeed >= 0)
				{
					SetAction(STEEPSLIDE);
					frame = 0;
					facingRight = true;
					if (SteepSlideAttack())
					{
						
					}
					break;
				}
				else if (gNorm.x < 0 && groundSpeed <= 0)
				{
					SetAction(STEEPSLIDE);
					frame = 0;
					facingRight = false;
					if (SteepSlideAttack())
					{

					}
					break;
				}
			}

			if (SteepClimbAttack())
			{
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
	case BOUNCEAIR:
		{
			if( !currInput.X )
			{
				SetActionExpr( JUMP );
				frame = 1;
				BounceFlameOff();
			}

			if( TryAirDash() ) break;

			if( AirAttack() )
			{
				BounceFlameOn();
				//bounceFlameOn = true;
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
				BounceFlameOff();
				holdJump = false;
				break;
			}

			
			
			V2d bn = bounceNorm;//bounceEdge->Normal();
			bool framesDone = frame == actionLength[BOUNCEGROUND] - 1;
			if( boostBounce || (framesDone && bn.y >= 0 ) )
			{
				ActivateSound( S_BOUNCEJUMP );
				framesInAir = 0;
				SetAction(BOUNCEAIR);
				oldBounceEdge = bounceEdge;
				oldBounceNorm  = bounceNorm;
				frame = 0;
				

				int option = 0; //0 is ground, 1 is wall, 2 is ceiling

				bool boostNow = boostBounce && framesSinceBounce > 8;

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
				double extraBUp = .2;
				double extraBDown = .2;
				double extraBThresh = .8; //works on almost everything
				double dSpeed = GetDashSpeed();
				if( bn.y < 0 )
				{
					//cout << "prevel: " << velocity.x << ", " << velocity.y << endl;
					if( bn.y > -steepThresh )
					{
						//if( bn.x > 0  && storedBounceVel.x < 0 )
						//{
						//	//cout << "A" << endl;
						//	velocity = V2d( abs(storedBounceVel.x), -abs(storedBounceVel.y) );
						//}
						//else if( bn.x < 0 && storedBounceVel.x > 0 )
						//{
						////	cout << "B" << endl;
						//	velocity = V2d( -abs(storedBounceVel.x), -abs(storedBounceVel.y) );
						//}
						//else
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

						double velStrength = length( velocity );
						V2d vDir = normalize( velocity );
						if( abs( vDir.y ) < extraBThresh )
						{
							if( currInput.LUp() )
							{
								vDir = normalize( vDir + V2d( 0, -extraBUp ) );
							}
							else if( currInput.LDown() )
							{
								vDir = normalize( vDir + V2d( 0, extraBDown ) );
							}
							//velocity = vDir * velStrength;
						}

						if( boostNow )
						{
							//double fac = max( 6.0, .3 * velocity.y ); //bounceBoostSpeed;
							velocity += vDir * bounceBoostSpeed / (double)slowMultiple;
						

							boostBounce = false;
						}
						else if( boostBounce )
						{
							boostBounce = false;
						}
						//if( 
						//bounceNorm.y = -1;
					}
					else
					{
						//cout << "DD" << endl;
						
						
						if( storedBounceVel.x > 0 && currInput.LLeft() )
						{
							storedBounceVel.x = -dSpeed;
						}
						else if( storedBounceVel.x < 0 && currInput.LRight() )
						{
							storedBounceVel.x = dSpeed;
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
						
						if( boostNow )
						{
							double fac = max( 6.0, .25 * abs(velocity.y) ); //bounceBoostSpeed;
							//cout << "fac: " << fac << ", vy: "<< velocity.y << endl;
							velocity += normalize( velocity ) * fac / (double)slowMultiple;
							boostBounce = false;
						}
						else if( boostBounce )
						{
							boostBounce = false;
						}

					}
				}
				else if( bn.y > 0 )
				{
					if( -bn.y > -steepThresh )
					{
						//if( bn.x > 0 && storedBounceVel.x < 0 )
						//{
						////	cout << "C" << endl;
						//	velocity = V2d( abs(storedBounceVel.x), storedBounceVel.y );
						//}
						//else if( bn.x < 0 && storedBounceVel.x > 0 )
						//{
						////	cout << "D" << endl;
						//	velocity = V2d( -abs(storedBounceVel.x), storedBounceVel.y );
						//}
						//else
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

					double velStrength = length( velocity );
					V2d vDir = normalize( velocity );
					if( abs( vDir.y ) < extraBThresh )
					{
						if( currInput.LUp() )
						{
							vDir = normalize( vDir + V2d( 0, -extraBUp ) );
						}
						else if( currInput.LDown() )
						{
							vDir = normalize( vDir + V2d( 0, extraBDown ) );
						}
						//velocity = vDir * velStrength;
					}

					if( boostNow )
					{
						//double fac = max( 6.0, .3 * velocity.y ); //bounceBoostSpeed;
						//velocity += normalize( velocity ) * bounceBoostSpeed / (double)slowMultiple;
						velocity += vDir * bounceBoostSpeed / (double)slowMultiple;

						boostBounce = false;
					}
					else if( boostBounce )
					{
						boostBounce = false;
					}
				}
				else
				{
				//	cout << "F" << endl;
					velocity = V2d( -storedBounceVel.x, storedBounceVel.y ); 

					double velStrength = length( velocity );
					V2d vDir = normalize( velocity );
					if( abs( vDir.y ) < extraBThresh )
					{
						if( currInput.LUp() )
						{
							vDir = normalize( vDir + V2d( 0, -extraBUp ) );
						}
						else if( currInput.LDown() )
						{
							vDir = normalize( vDir + V2d( 0, extraBDown ) );
						}
						velocity = vDir * velStrength;
					}

					if( boostNow )
					{
						velocity += vDir * bounceBoostSpeed / (double)slowMultiple;

						
						/*if( currInput.LUp() )
						{
							velocity += V2d( 0, -1 ) * extraBUp;
						}
						else if( currInput.LDown() )
						{
							velocity += V2d( 0, 1 ) * extraBDown;
						}*/

						boostBounce = false;
					}
					else if( boostBounce )
					{
						boostBounce = false;
					}
				}

				//velocity += V2d( 0, -gravity * slowMultiple );
				if( facingRight && velocity.x < 0 )
					facingRight = false;
				else if( !facingRight && velocity.x > 0 )
					facingRight = true;

				/*double lenVel = length( storedBounceVel );
				double reflX = cross( normalize( -storedBounceVel ), bn );
				double reflY = dot( normalize( -storedBounceVel ), bn );
				V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
				velocity = normalize( reflX * edgeDir + reflY * bn ) * lenVel;*/

				
				framesSinceBounce = 0;

				//velocity = length( storedBounceVel ) * bounceEdge->Normal();
				//ground = NULL;
				bounceEdge = NULL;

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

					if( bn.y > -steepThresh )
					{
						
					}
					else
					{
					}
					SetAction(LAND);
					frame = 0;

					bounceFlameOn = true;
					scorpOn = true;

					V2d testVel = storedBounceVel;

				
					if( testVel.y > 20 )
					{
						testVel.y *= .7;
					}
					else if( testVel.y < -30 )
					{
				
						testVel.y *= .5;
					}

					groundSpeed = CalcLandingSpeed( testVel, alongVel, bn );

					/*if( currInput.LLeft() || currInput.LRight() || currInput.LDown() || currInput.LUp() )
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
					}*/

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
					SetAction(JUMP);
					frame = 1;
					velocity = storedBounceVel;
					bounceEdge = NULL;
				}
			}
			break;
		}
	case BOUNCEGROUNDEDWALL:
		{
			if( hasPowerGrindBall && currInput.Y && !prevInput.Y )
			{
				SetActionGrind();
				BounceFlameOff();
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

			if (GroundAttack())
			{
				break;
			}
			break;
		}
	case AUTORUN:
	{
		break;
	}
	
	case DEATH:
		{

			break;
		}
	case GETSHARD:
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
			RunMovement( );
		break;
		}
	case JUMP:
		{
		if( frame == 0 && slowCounter == 1 )
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

					dir.y = .2;
					V2d along = normalize(ground->v1 - ground->v0 );
					V2d trueNormal = along;
					if( groundSpeed > 0 )
						trueNormal = -trueNormal;

					trueNormal = normalize( trueNormal + dir );
					velocity = abs(groundSpeed) * trueNormal;

					ground = NULL;
					frame = 1; //so it doesnt use the jump frame when just dropping
					reversed = false;
					framesInAir = 0;

				}
				else
				{
					double dSpeed = GetDashSpeed();

					if( bounceFlameOn )
						airBounceFrame = 13 * 3;

					double blah = .25;

					V2d dir( 0, 0 );
					
					V2d trueNormal = normalize(dir + normalize(ground->v1 - ground->v0 ));
					velocity = groundSpeed * trueNormal;
					if( velocity.y < 0 )
					{
						velocity.y *= .7;
					}

					if( currInput.B )
					{
						if( currInput.LRight() && velocity.x < dSpeed )
						{
							velocity.x = dSpeed;
						}
						else if( currInput.LLeft() && velocity.x > -dSpeed)
						{							
							velocity.x = -dSpeed;
						}
						else
						{
							velocity.x = (velocity.x + groundSpeed) / 2.0;	
						}
					}
					else
					{
						velocity.x = (velocity.x + groundSpeed) / 2.0;
					}

					if( velocity.y > 0 )
					{
						//min jump velocity for jumping off of edges.
						if( abs(velocity.x) < dSpeed && length( velocity ) >= dSpeed )
						{
							if( velocity.x > 0 )
							{
								velocity.x = dSpeed;
							}
							else
							{
								velocity.x = -dSpeed;
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
					
					switch (speedLevel)
					{
					case 0:
						ActivateEffect(EffectLayer::IN_FRONT, ts_fx_jump[0], fxPos, false, ang, 6, 4, facingRight);
						break;
					case 1:
						ActivateEffect(EffectLayer::IN_FRONT, ts_fx_jump[1], fxPos, false, ang, 6, 4, facingRight);
						break;
					case 2:
						ActivateEffect(EffectLayer::IN_FRONT, ts_fx_jump[2], fxPos, false, ang, 6, 4, facingRight);
						break;
					}

					//ActivateEffect( EffectLayer::IN_FRONT, ts_fx_jump, fxPos , false, ang, 6, 4, facingRight );

					ground = NULL;
					holdJump = true;

					framesInAir = 0;

					if (touchedJumpGrass)
					{
						velocity.y -= jumpGrassExtra;
					}
					//steepJump = false;
				}
				
			}
			else if (grindEdge != NULL )
			{
				//assert(0);
				//V2d ev0, ev1;
				Edge tempEdge(*grindEdge);
				/*if (tempEdge.Normal().y > 0)
				{
					V2d temp = tempEdge.v0;
					tempEdge.v0 = tempEdge.v1;
					tempEdge.v1 = temp;
				}*/

				double dSpeed = GetDashSpeed();

				if (bounceFlameOn)
					airBounceFrame = 13 * 3;

				double blah = .25;

				V2d dir(0, 0);

				V2d trueNormal = normalize(dir + normalize(tempEdge.v1 - tempEdge.v0));
				velocity = grindSpeed * trueNormal;
				if (velocity.y < 0)
				{
					velocity.y *= .7;
				}

				/*if (currInput.B)
				{
					if (currInput.LRight())
					{
						if (velocity.x < dSpeed)
							velocity.x = dSpeed;
					}
					else if (currInput.LLeft())
					{
						if (velocity.x > -dSpeed)
							velocity.x = -dSpeed;
					}
				}*/
				if (velocity.y > 0)
				{
					//min jump velocity for jumping off of edges.
					if (abs(velocity.x) < dSpeed && length(velocity) >= dSpeed)
					{
						//		cout << "here: " << velocity.x << endl;
						if (velocity.x > 0)
						{
							velocity.x = dSpeed;
						}
						else
						{
							velocity.x = -dSpeed;
						}
					}

					velocity.y = 0;
				}

				if (steepJump)
				{
					velocity.y -= jumpStrength * .75;
				}
				else
				{
					velocity.y -= jumpStrength;
				}
				holdJump = true;

				framesInAir = 0;

				
				grindEdge = NULL;
				
			}
			else
			{
				assert(0);
			}

			
		}
		else if( frame > 0 )
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
			if( frame == 0 && slowCounter == 1 )
			{
				ActivateSound(S_JUMP);
				storedGroundSpeed = groundSpeed;
				/*if( reversed )
					storedGroundSpeed = -storedGroundSpeed;*/

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
				ExecuteWallJump();
			}
			else if( frame >= wallJumpMovementLimit )
			{
				AirMovement();
			}
			break;
		}
	case WALLATTACK:
		{
			int f = frame / 2;
			if (f < 8)
			{
				SetCurrHitboxes(wallHitboxes[speedLevel], frame / 2);
			}
			

			if( frame == 0 )
			{
				currAttackHit = false;
				ActivateSound( S_WALLATTACK);
			//	fairSound.play();
			}

			if( velocity.y > clingSpeed )
			{
				//cout << "running wallcling" << endl;
				velocity.y = clingSpeed;
			}

			double constantWallCling = 5;
			if (facingRight)
			{
				velocity.x = -constantWallCling;
			}
			else
			{
				velocity.x = constantWallCling;
			}
			//AirMovement();

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

			
			SetCurrHitboxes(fairHitboxes[speedLevel], frame);

			if( frame == 0 && slowCounter == 1)
			{
				ActivateSound( S_FAIR1 );
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

			SetCurrHitboxes(dairHitboxes[speedLevel], frame);

			if( frame == 0 && slowCounter == 1 )
			{

				ActivateSound(S_DAIR);
				if( speedLevel == 0 ) 
				{
					
				}
				/*else if (speedLevel == 1)
				{
					ActivateSound(soundBuffers[S_DAIR_B]);
				}*/
				
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

			SetCurrHitboxes(uairHitboxes[speedLevel], frame);

			if( frame == 0 && slowCounter == 1)
			{
				ActivateSound( S_UAIR );
				currAttackHit = false;
			}

			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{	
				AirMovement();
			}
			break;
		}
	case DIAGUPATTACK:
		{
			SetCurrHitboxes(diagUpHitboxes[speedLevel], frame / 2);

			if( frame == 0 && slowCounter == 1)
			{
				ActivateSound( S_DIAGUPATTACK );
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
	case DIAGDOWNATTACK:
		{
			SetCurrHitboxes(diagDownHitboxes[speedLevel], frame / 2);

			if( frame == 0 && slowCounter == 1)
			{
				ActivateSound( S_DIAGDOWNATTACK );
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
	case DASH:
		{
			double dSpeed = GetDashSpeed();
			b.rh = dashHeight;
			b.offset.y = (normalHeight - dashHeight);
			if( reversed )
				b.offset.y = -b.offset.y;
			if( currInput.LLeft() && facingRight )
			{
				facingRight = false;
				groundSpeed = -dSpeed;
				frame = 0;
			}
			else if( currInput.LRight() && !facingRight )
			{
				facingRight = true;
				groundSpeed = dSpeed;
				frame = 0;
			}
			else if( !facingRight )
			{
				if( groundSpeed > -dSpeed )
					groundSpeed = -dSpeed;
			}
			else
			{
				if( groundSpeed < dSpeed )
					groundSpeed = dSpeed;
			}

			
			
			double sprFactor = 2.0; //dash must be slower accel on slopes
			if( currInput.LDown() && (( facingRight && gNorm.x > 0 ) || ( !facingRight && gNorm.x < 0 ) ) )
			{
				double sprAccel = GetFullSprintAccel(true, gNorm) / sprFactor;
				if( facingRight )
				{
					groundSpeed += sprAccel / slowMultiple;
				}
				else 
				{
					groundSpeed -= sprAccel / slowMultiple;
				}
			}
			else if( currInput.LUp() && (( facingRight && gNorm.x > 0 ) || ( !facingRight && gNorm.x < 0 ) ) )
			{
				double sprAccel = GetFullSprintAccel(false, gNorm) / sprFactor;

				if( facingRight )
				{
					groundSpeed += sprAccel / slowMultiple; 
				}
				else 
				{	
					groundSpeed -= sprAccel / slowMultiple; 
				}
			}

			GroundExtraAccel();

			if (standNDashBoost)
			{
				if (groundSpeed > 0)
				{
					groundSpeed += standNDashBoostQuant;
				}
				else
				{
					groundSpeed -= standNDashBoostQuant;
				}
				standNDashBoost = false;
			}

			break;
		}
	case BACKWARDSDOUBLE:
	case DOUBLE:
		{
			if( action == DOUBLE )
			{
				b.rh = doubleJumpHeight;
			}
			
		//	b.offset.y = -5;
			if( frame == 0 )
			{
				ExecuteDoubleJump();
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
			double gAngle = GroundedAngle();
			//double fac = gravity * 2.0 / 3;
			if( gAngle != 0 && gAngle != PI )
			{
				if( reversed )
				{
					double accel = dot( V2d( 0, slideGravFactor * GetGravity()), normalize( ground->v1 - ground->v0 )) / slowMultiple;
					groundSpeed += accel;
					
				}
				else
				{
					double accel = dot( V2d( 0, slideGravFactor * GetGravity()), normalize( ground->v1 - ground->v0 )) / slowMultiple;
					groundSpeed += accel;
					//cout << "accel slide: \n" << accel;
				}
			}

			//GroundExtraAccel();
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

			if( currInput.LLeft() )
				facingRight = false;
			else if( currInput.LRight() )				
				facingRight = true;

			double accel = 0;
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
						

						if( gNorm.x > 0 )
						{
							//up a slope
							double sprAccel = GetFullSprintAccel(false, gNorm);

							//GroundExtraAccel();

							accel = sprAccel / slowMultiple;
							groundSpeed -= accel;
							
						}
						else
						{	
							//GroundExtraAccel();

							double sprAccel = GetFullSprintAccel(true, gNorm);

							accel = sprAccel / slowMultiple;
							groundSpeed -= accel;

						
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
							//GroundExtraAccel();

							double sprAccel = GetFullSprintAccel(false, gNorm);

							accel = sprAccel / slowMultiple; 
							groundSpeed += accel;
						}
						else
						{	

							//GroundExtraAccel();

							double sprAccel = GetFullSprintAccel(true, gNorm);

							accel = sprAccel / slowMultiple;
							groundSpeed += accel;
							//down a slope
						}
					}
				}
				facingRight = true;
			}

			//cout << "sprint accel: " << accel << "\n";
			break;
		}
	case STANDN:
		{
			SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

			if( frame == 0 && slowCounter == 1 )
			{
				ActivateSound( S_STANDATTACK);
				currAttackHit = false;
			}

			AttackMovement();
			break;

		}
	case STEEPCLIMBATTACK:
		{
			SetCurrHitboxes(steepClimbHitboxes[speedLevel], frame/2);

			if( frame == 0 )
			{
				currAttackHit = false;
			}


			bool boost = TryClimbBoost(gNorm);

			float factor = steepClimbGravFactor;//.7 ;
			if( currInput.LUp() )
			{
				//cout << "speeding up climb!" << endl;
				factor = steepClimbFastFactor;//.5;
			}

			if( reversed )
			{
				groundSpeed += dot( V2d( 0, GetGravity() * factor), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				groundSpeed += dot( V2d( 0, GetGravity() * factor), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}

			//AttackMovement();
			break;

		}
	case STEEPSLIDEATTACK:
		{
			if (frame / 2 > 7)
			{
			assert(0);
			}
			SetCurrHitboxes(steepSlideHitboxes[speedLevel], frame / 2);

			if( frame == 0 )
			{
				currAttackHit = false;
			}

			double fac = GetGravity() * steepSlideGravFactor;//gravity * 2.0 / 3.0;

			if( currInput.LDown() )
			{
				//cout << "fast slide" << endl;
				fac = GetGravity() * steepSlideFastGravFactor;
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
	case GRINDBALL:
		{
			if (grindSpeed > 0)
			{
				grindSpeed = std::min(maxGroundSpeed + scorpAdditionalCap,grindSpeed);
			}
			else
			{
				grindSpeed = std::max(-maxGroundSpeed - scorpAdditionalCap, grindSpeed);
			}

			velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			//framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
			//framesGrinding++;
			break;
		}
	case RAILSLIDE:
	case RAILGRIND:
	{
		RailGrindMovement();
		//framesGrinding++;
		break;
	}
	case GRINDATTACK:
		{
			
			framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
			double dSpeed = GetDashSpeed();
			double adSpeed = abs( dSpeed );
			double tot = ( abs( grindSpeed ) - adSpeed ) / ( maxGroundSpeed - adSpeed );

			double fac = .2;

			if( tot > .5 )
				fac = .5;

			if( grindSpeed > dSpeed )
			{
				grindSpeed -= fac * slowMultiple;
				if( grindSpeed < dSpeed )
					grindSpeed = dSpeed;
			}
			else if( grindSpeed < -dSpeed )
			{
				grindSpeed += fac * slowMultiple;
				if( grindSpeed > -dSpeed )
					grindSpeed = -dSpeed;
			}

			
			velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;

			//if( grindHitboxes.count( frame ) > 0 )
			if( abs( grindSpeed ) > dSpeed )
			{
				SetCurrHitboxes(grindHitboxes[0], 0);
			}
			break;
		}
	case GRINDSLASH:
		{
			SetCurrHitboxes(uairHitboxes[0], frame);
		}
		break;
	case GRAVREVERSE:
		break;

	case SPRINGSTUN:
	case SPRINGSTUNTELEPORT:
	{		
		velocity = springVel + springExtra;
		break;
	}
	case SWINGSTUN:
	{
		double rad = oldSwingLauncher->swingRadius;
		double speed = oldSwingLauncher->speed;

		V2d anchor = oldSwingLauncher->anchor;
		V2d future = position + normalize(springVel) * speed;//velocity;

		//V2d seg = wirePoint - rwPos;
		//double segLength = length(seg);
		V2d diff = anchor - future;

		if (length(diff) > rad)
		{
			future += normalize(diff) * (length(diff) - rad);
			springVel = future - position;
		}

		velocity = springVel; //* speed;
		break;
	}
	case SPRINGSTUNAIRBOUNCE:
	{
		V2d inputDir(0, 0);

		if (frame > 10)
		{


			if (currInput.LUp())
			{
				inputDir.y = -1;
			}
			else if (currInput.LDown())
			{
				inputDir.y = 1;
			}

			if (currInput.LRight())
			{
				inputDir.x = 1;
			}
			else if (currInput.LLeft())
			{
				inputDir.x = -1;
			}


			//bool inputSwitch = true;
			if (inputDir.x != 0 || inputDir.y != 0)
			{
				inputDir = normalize(inputDir);
				springVel = inputDir * length(springVel);
			}
		}
		

		velocity = springVel + springExtra;

		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}

		break;
	}
	case SPRINGSTUNBOUNCE:
	{
		double bounceTurnFactor = .012;
		if (currInput.LUp())
		{
			glideTurnFactor = bounceTurnFactor;
			/*if (bounceTurnFactor < 0)
			{
				bounceTurnFactor = 0;
			}
			glideTurnFactor += glideTurnAccel;
			if (glideTurnFactor > maxGlideTurnFactor)
			{
				glideTurnFactor = maxGlideTurnFactor;
			}*/
			//RotateCCW(springVel, glideTurnFactor);
		}
		else if (currInput.LDown())
		{
			/*if (glideTurnFactor > 0)
			{
				glideTurnFactor = 0;
			}
			glideTurnFactor -= glideTurnAccel;
			if (glideTurnFactor < -maxGlideTurnFactor)
			{
				glideTurnFactor = -maxGlideTurnFactor;
			}*/
			glideTurnFactor = -bounceTurnFactor;
			//RotateCCW(springVel, glideTurnFactor);
			//grav = AddGravity(V2d(0, 0));
		}
		else
		{
			glideTurnFactor = 0;
		}


		if (facingRight)
		{
			RotateCCW(springVel, glideTurnFactor);
		}
		else
		{
			RotateCCW(springVel, -glideTurnFactor);
		}

		velocity = springVel + springExtra;

		break;
	}
	case SPRINGSTUNGLIDE:
	{
		if (currInput.LUp())
		{
			if (glideTurnFactor < 0)
			{
				glideTurnFactor = 0;
			}
			glideTurnFactor += glideTurnAccel;
			if (glideTurnFactor > maxGlideTurnFactor)
			{
				glideTurnFactor = maxGlideTurnFactor;
			}
			//RotateCCW(springVel, glideTurnFactor);
		}
		else if (currInput.LDown())
		{
			if (glideTurnFactor > 0)
			{
				glideTurnFactor = 0;
			}
			glideTurnFactor -= glideTurnAccel;
			if (glideTurnFactor < -maxGlideTurnFactor)
			{
				glideTurnFactor = -maxGlideTurnFactor;
			}
			//RotateCCW(springVel, glideTurnFactor);
			//grav = AddGravity(V2d(0, 0));
		}
		else
		{
			glideTurnFactor = 0;
		}


		if (facingRight)
		{
			RotateCCW(springVel, glideTurnFactor);
		}
		else
		{
			RotateCCW(springVel, -glideTurnFactor);
		}

		//springExtra = AddGravity(springExtra);
		//if (springVel.y > 0)
		if (false)
		{
			double gFac = 1.0;
			V2d nsv = normalize(springVel);
			springVel = nsv * (length(springVel) + gFac * nsv.y);//AddGravity(springVel);
		}


		//springExtra = tempVel - springVel;
		velocity = springVel + springExtra;

		break;
	}
	case GLIDE:
	{
		if (currInput.LUp())
		{
			if (glideTurnFactor < 0)
			{
				glideTurnFactor = 0;
			}
			glideTurnFactor += glideTurnAccel;
			if (glideTurnFactor > maxGlideTurnFactor)
			{
				glideTurnFactor = maxGlideTurnFactor;
			}
			//RotateCCW(springVel, glideTurnFactor);
		}
		else if (currInput.LDown())
		{
			if (glideTurnFactor > 0)
			{
				glideTurnFactor = 0;
			}
			glideTurnFactor -= glideTurnAccel;
			if (glideTurnFactor < -maxGlideTurnFactor)
			{
				glideTurnFactor = -maxGlideTurnFactor;
			}
			//RotateCCW(springVel, glideTurnFactor);
			//grav = AddGravity(V2d(0, 0));
		}
		else
		{
			glideTurnFactor = 0;
		}


		if (facingRight)
		{
			RotateCCW(velocity, glideTurnFactor);
		}
		else
		{
			RotateCCW(velocity, -glideTurnFactor);
		}

		//springExtra = AddGravity(springExtra);
		//if (false)
		{
			double gFac = .2;//1.0;
			V2d nsv = normalize(velocity);
			double len = length(velocity) + gFac * nsv.y;
			if (len < 10)
			{
				len = 10;
			}
			velocity = nsv * len;//AddGravity(springVel);
		}


		//springExtra = tempVel - springVel;
		//velocity = velocity + springExtra;

		break;
	}
	case STEEPSLIDE:
		{
			//if( groundSpeed > 0 )
			double fac = GetGravity() * steepSlideGravFactor;//gravity * 2.0 / 3.0;

			if( currInput.LDown() )
			{
				//cout << "fast slide" << endl;
				fac = GetGravity() * steepSlideFastGravFactor;
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
			double aSpeed = GetAirDashSpeed();


			if( IsSingleWirePulling() )
			{
				if( frame == 0 )
				{
					hasAirDash = false;
					startAirDashVel = velocity;//V2d( velocity.x, 0 );//velocity;//
				}
			}
			else
			{
				bool isDoubleWiring = IsDoubleWirePulling();

				//some old code in here for an alternate airdash when double wiring. ignore for now
				if( frame == 0 )
				{
					dWireAirDash = V2d( 0, 0 );
					dWireAirDashOld = V2d( 0, 0 );

					hasAirDash = false;

					if (hasFairAirDashBoost)
					{
						if (velocity.x > 0)
						{
							velocity.x += fairAirDashBoostQuant;
						}
						else if (velocity.x < 0)
						{
							velocity.x -= fairAirDashBoostQuant;
						}
					}

					startAirDashVel = V2d( velocity.x, 0 );//velocity;//
					if( (velocity.y > 0 && currInput.LDown()) || ( velocity.y < 0 && currInput.LUp() ) )
					{
						if( abs( velocity.y ) > aSpeed )
						{
							if( velocity.y < 0 )
							{
								extraAirDashY = velocity.y + aSpeed;// / 2;
							}
							else
							{
								extraAirDashY = velocity.y - aSpeed;// / 2;
							}
						}
						else
						{
							extraAirDashY = velocity.y;//0;
						}

						if( extraAirDashY > 0 )
						{
							extraAirDashY = .1;
							//extraAirDashY = min( extraAirDashY, 5.0 );
							//extraAirDashY *= 1.8;
						}
					}
					else
					{
						extraAirDashY = 0;
					}
				}
				//V2d oldvel = velocity;

				dWireAirDash = V2d( 0, 0 );
				if( false )//isDoubleWiring )
				{
					if( currInput.LUp() )
					{
						dWireAirDash += V2d( 0, -aSpeed );
					}
					if( currInput.LLeft() )
					{
						dWireAirDash += V2d( -aSpeed, 0 );
					}
					if( currInput.LRight() )
					{
						dWireAirDash += V2d( aSpeed, 0);
					}
					if( currInput.LDown() )
					{
						dWireAirDash += V2d( 0, aSpeed );
					}
				}
				else
				{
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
					
					if( extraAirDashY > 0 )
						extraAirDashY = 0;

					double mod = 1.0;

					if( extraGravityModifier < 1.0 )
					{
						mod = extraGravityModifier * 1.5;
						mod = min(1.0, mod);
						mod = max(.5, mod);
					}
					else
					{
						mod = extraGravityModifier * .75;
						mod = max(1.0, mod);
						mod = min(2.0, mod);
					}
					//extragravitymodifier must not be 0
					velocity.y = -aSpeed / mod + extraAirDashY;

					if( extraAirDashY < 0 )
					{
						extraAirDashY = AddGravity( V2d( 0, extraAirDashY ) ).y;
						//extraAirDashY += gravity / slowMultiple;
						if( extraAirDashY > 0 )
							extraAirDashY = 0;
					}
				}
				else if( currInput.LDown() )
				{
					if( !(currInput.LLeft() || currInput.LRight() ) && abs(startAirDashVel.x) >= keepHorizontalLimit )
					{	
						velocity.x = startAirDashVel.x * removeSpeedFactor;
						//cout << "velocity.x: " << velocity.x << endl;
					}

					if( extraAirDashY < 0 )
						extraAirDashY = 0;

					velocity.y = aSpeed + extraAirDashY;

					if( extraAirDashY > 0 )
					{
						extraAirDashY = AddGravity( V2d( 0, extraAirDashY ) ).y;
						//extraAirDashY += gravity / slowMultiple;
					}
				}
				else
				{
					extraAirDashY = 0;
				}


				if( currInput.LLeft() )
				{
					if( startAirDashVel.x > 0 )
					{
						startAirDashVel.x = 0;
						velocity.x = -aSpeed;
					}
					else
					{
						velocity.x = min( startAirDashVel.x, -aSpeed );
					}
					facingRight = false;

				}
				else if (currInput.LRight())
				{
					if (startAirDashVel.x < 0)
					{
						startAirDashVel.x = 0;
						velocity.x = aSpeed;
					}
					else
					{
						velocity.x = max(startAirDashVel.x, aSpeed);
					}
					facingRight = true;
				}

				if (velocity.x == 0 && velocity.y == 0)
				{
					/*if( isDoubleWiring )
					{
						velocity = oldvel;
					}
					else*/
					{
						startAirDashVel = V2d(0, 0);
						extraAirDashY = 0;
						velocity = AddGravity(velocity);
					}

				}

				}

				velocity -= dWireAirDashOld;
				velocity += dWireAirDash;

				dWireAirDashOld = dWireAirDash;
			}

			if (currBooster != NULL && oldBooster == NULL && currBooster->Boost() )
			{
				SetBoostVelocity();
				
				startAirDashVel.x = velocity.x;
				extraAirDashY = velocity.y;
				if (extraAirDashY > aSpeed)
				{
					extraAirDashY = extraAirDashY - aSpeed;
				}
				else if (extraAirDashY < -aSpeed)
				{
					extraAirDashY = extraAirDashY + aSpeed;
				}
			}
			//else if (currBounceBooster != NULL && oldBounceBooster == NULL && currBounceBooster->Boost())
			//{
			//	SetBounceBoostVelocity();

			//	startAirDashVel.x = velocity.x;//normalize(velocity).x * ( length( velocity ) + currBooster->strength);
			//	extraAirDashY = velocity.y; //- aSpeed;
			//	if (extraAirDashY > aSpeed)
			//	{
			//		extraAirDashY = extraAirDashY - aSpeed;
			//	}
			//	else if (extraAirDashY < -aSpeed)
			//	{
			//		extraAirDashY = extraAirDashY + aSpeed;
			//	}
			//}
			break;
		}
	case STEEPCLIMB:
	{
		//if( groundSpeed > 0 )

		bool boost = TryClimbBoost(gNorm);

		//the factor is just to make you climb a little farther
		float factor = steepClimbGravFactor;//.7 ;
		if (currInput.LUp())
		{
			//cout << "speeding up climb!" << endl;
			factor = steepClimbFastFactor;//.5;
		}

		if (reversed)
		{
			groundSpeed += dot(V2d(0, GetGravity() * factor), normalize(ground->v1 - ground->v0)) / slowMultiple;
		}
		else
		{
			groundSpeed += dot(V2d(0, GetGravity() * factor), normalize(ground->v1 - ground->v0)) / slowMultiple;
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
		int slowDown = 1;
		if (groundSpeed > 0)
		{
			groundSpeed -= slowDown;
			if (groundSpeed < 0)
			{
				groundSpeed = 0;
			}
		}
		else if (groundSpeed < 0)
		{
			groundSpeed += slowDown;
			if (groundSpeed > 0)
			{
				groundSpeed = 0;
			}
		}
		
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
				//ActivateSound( soundBuffers[S_BOUNCEJUMP] );
				boostBounce = true;

				
				V2d bouncePos = bounceEdge->GetPoint( bounceQuant );
				V2d bn = bounceEdge->Normal();
				double angle = atan2( bn.x, -bn.y );
				bouncePos += bn * 80.0;
				ActivateEffect( EffectLayer::IN_FRONT, ts_bounceBoost, bouncePos, false, angle, 30, 1, facingRight );
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
				
				groundSpeed = -storedBounceGroundSpeed / (double)slowMultiple;
				//cout << "set ground speed to: " << groundSpeed << endl;
			}
			break;
		}
	case AUTORUN:
	{
		RunMovement();
		break;
	}
	case DEATH:
		{
			velocity.x = 0;
			velocity.y = 0;
			groundSpeed = 0;
			break;
		}
	case SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY:
		{
			//cout << "frame: " << frame << endl;
			RunMovement( );
			break;
		}
	case SEQ_CRAWLERFIGHT_DODGEBACK:
		if( frame == 0 )
		{
			//cout << "leaving ground" << endl;
			ground = NULL;
			velocity = V2d( -10, -20);
			
		}
		//cout << "frame: " << frame << endl;
		//velocity = V2d( -30, -100 );
		break;
	case GETSHARD:
	{
		if (frame == 0)
		{
			if (ground != NULL)
			{
				ground = NULL;
				framesInAir = 0;
				hasAirDash = true;
				hasDoubleJump = true;
			}
		}

		velocity = V2d(0, 0);
		break;
	}
	}
	
	//if( action != GRINDBALL && action != GROUNDHITSTUN && action != AIRHITSTUN )

	//if( action != GRINDBALL && action != GRINDATTACK )
	//{
	//	//for camera smoothing
	//	framesNotGrinding++;
	//}
	
	
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
	}

	Wire::WireState oldLeftWireState = leftWire->state;
	Wire::WireState oldRightWireState = rightWire->state;


	if( hasPowerLeftWire && ( (action != GRINDBALL && action != GRINDATTACK ) || leftWire->state == Wire::RETRACTING ) )
	{
		leftWire->ClearDebug();
		leftWire->storedPlayerPos = leftWire->storedPlayerPos = leftWire->GetPlayerPos();//leftWire->GetOriginPos(true);
		//leftWire->UpdateAnchors2( V2d( 0, 0 ) );
		leftWire->UpdateState( touchEdgeWithLeftWire );
	}

	if( hasPowerRightWire && ((action != GRINDBALL && action != GRINDATTACK ) || rightWire->state == Wire::RETRACTING ) )
	{
		rightWire->ClearDebug();
		rightWire->storedPlayerPos = leftWire->storedPlayerPos = leftWire->GetPlayerPos();
		//rightWire->UpdateAnchors2( V2d( 0, 0 ) );
		rightWire->UpdateState( touchEdgeWithRightWire );
	}
	
	if (currBooster != NULL && oldBooster == NULL && action != AIRDASH && currBooster->Boost())
	{	
		if (ground == NULL && bounceEdge == NULL && grindEdge == NULL  )
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

	//if (currBounceBooster != NULL && oldBounceBooster == NULL && action != AIRDASH && currBounceBooster->Boost())
	//{
	//	if (ground == NULL && bounceEdge == NULL && grindEdge == NULL)
	//	{
	//		SetBounceBoostVelocity();
	//	}
	//	else if (grindEdge != NULL)
	//	{
	//		/*if (grindSpeed > 0)
	//		{
	//			grindSpeed += currBounceBooster->strength;
	//		}
	//		else if (grindSpeed < 0)
	//		{
	//			grindSpeed -= currBounceBooster->strength;
	//		}

	//		if (action == RAILGRIND)
	//		{
	//			velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
	//		}*/
	//	}
	//	else if (ground != NULL)
	//	{
	//		/*if (groundSpeed > 0)
	//		{
	//			groundSpeed += currBounceBooster->strength;
	//		}
	//		else if (groundSpeed < 0)
	//		{
	//			groundSpeed -= currBounceBooster->strength;
	//		}*/
	//	}
	//}

	/*if (currModifier != NULL && oldModifier == NULL && currModifier->Modify())
	{
		extraGravityModifier = currModifier->gravFactor;
		gravResetFrames = currModifier->duration;
	}*/


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


	double maxReal = maxVelocity + scorpAdditionalCap;
	if (ground == NULL && bounceEdge == NULL && grindEdge == NULL && action != DEATH
		&& action != ENTERNEXUS1 && action != SPRINGSTUN && action != GLIDE && action != SPRINGSTUNGLIDE
		&& action != SPRINGSTUNBOUNCE && action != SPRINGSTUNAIRBOUNCE && action != SPRINGSTUNTELEPORT )
	{
		if (action != AIRDASH && !(rightWire->state == Wire::PULLING && leftWire->state == Wire::PULLING) && action != GRINDLUNGE && action != RAILDASH && action != GETSHARD )
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

	//rightWire->UpdateEnemyAnchor();
	//leftWire->UpdateEnemyAnchor();

	WireMovement();

	
	

	for( int i = 0; i < maxBubbles; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			bubbleFramesToLive[i]--;
			if (fBubblePos != NULL)
			{
				fBubbleFrame[i] = bubbleFramesToLive[i];
			}
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

	//int mBubbles = maxBubbles;

	if( hasPowerTimeSlow )
	{
		//calculate this all the time so I can give myself infinite airdash
		for( int i = 0; i < maxBubbles; ++i )
		{
			if( bubbleFramesToLive[i] > 0 )
			{
				//if( IsQuadTouchingCircle( hurtB
				if( length( position - bubblePos[i] ) < bubbleRadiusSize[i] )
				{
					inBubble = true;
					break;
				}
			}
		}
	}

	bool isInOwnBubble = inBubble;
	bool isBeingSlowed = IsBeingSlowed();
	if( isBeingSlowed )
	{
		inBubble = true;
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
		holdJump = false;
	}

	int tempSlowCounter = slowCounter;
	if( ( CanCreateTimeBubble() && hasPowerTimeSlow && currInput.leftShoulder ) || cloneBubbleCreated )
	{
		if( (!prevInput.leftShoulder  && !inBubble) || cloneBubbleCreated )
		{
			if( bubbleFramesToLive[currBubble] == 0 )
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
				

				if( !cloneBubbleCreated )
				{
					bubblePos[currBubble] = position;

					CollisionBox &bHitbox = bubbleHitboxes[currBubble]->GetCollisionBoxes(0)->front();
					bHitbox.globalPosition = position;
					bHitbox.rw = bubbleRadiusSize[currBubble];
					bHitbox.rh = bHitbox.rw;

					if (fBubblePos != NULL)
					{
						fBubblePos[currBubble].x = position.x;
						fBubblePos[currBubble].y = position.y;
					}
				}
				else
				{
					bubblePos[currBubble] = cloneBubbleCreatedPos;
					if (fBubblePos != NULL)
					{
						fBubblePos[currBubble].x = cloneBubbleCreatedPos.x;
						fBubblePos[currBubble].y = cloneBubbleCreatedPos.y;
					}
				}
				

				++currBubble;
				if( currBubble == maxBubbles )
				{
					currBubble = 0;
				}

				bubbleCreated = true;
				ActivateSound( S_TIMESLOW );
			}			
		}

		if( inBubble )
		{
			if( slowMultiple == 1 )
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
			slowMultiple = 1;
		}
	}
	else
	{
		//cout << "C " << endl;
		slowCounter = 1;
		slowMultiple = 1;
	}

	if( isBeingSlowed && !isInOwnBubble )
	{
		if( currInput.leftShoulder )
		{
			slowCounter = 1;
			slowMultiple = 1;
		}
		else
		{
			slowCounter = tempSlowCounter;
			slowMultiple = timeSlowStrength;
		}
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
	
	
	/*if (hasPowerGrindBall)
	{
		if (!currInput.Y && prevInput.Y)
		{
			framesSinceGrindAttempt = 0;
		}

		if (ground == NULL && grindEdge == NULL && bounceEdge == NULL && action != RAILDASH)
		{
			if (currInput.Y || framesSinceGrindAttempt < maxFramesSinceGrindAttempt)
			{
				canRailGrind = true;
			}
		}
		else
		{
			framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
		}
	}
	

	
	bool aerialRailSlideTest = ground == NULL && grindEdge == NULL && bounceEdge == NULL && action != RAILDASH &&
		action != RAILSLIDE && velocity.y >= 0 && action != AIRDASH && !IsAttackAction(action);

	if ( aerialRailSlideTest )
	{
		canRailSlide = true;
	}*/
	

	touchedJumpGrass = false;
	grassBoosted = false;
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

	if (action != RAILGRIND)
	{
		if (regrindOffCount < regrindOffMax)
		{
			regrindOffCount++;
		}
	}

	/*if( ground != NULL )
	{
		cout << "groundspeed: " << groundSpeed << endl;
	}
	else
	{
		cout << "vel: " << velocity.x << ", " << velocity.y << endl;
	}*/
	//if( ground == NULL )
	//cout << "final vel: " << velocity.x << ", " << velocity.y << endl;
	//cout << "before position: " << position.x << ", " << position.y << endl;
	/*if (grindEdge != NULL)
	{
		cout << "grindspeed: " << grindSpeed << endl;
	}
	else if (ground != NULL)
	{
		cout << "speed: " << groundSpeed << endl;
	}
	else
	{
		cout << "vel: " << velocity.x << ", " << velocity.y << endl;
	}*/

	oldBooster = currBooster;
	oldModifier = currModifier;
	oldBounceBooster = currBounceBooster;

	highAccuracyHitboxes = true;

	wallNormal.x = 0;
	wallNormal.y = 0;

	currHitboxInfo->hType = HitboxInfo::NORMAL;

	V2d trueNorm;
	V2d along;
	if (ground != NULL)
	{
		GroundedAngleAttack(trueNorm);
		along = V2d(-trueNorm.y, trueNorm.x);
	}
	
	
	//sets directionality of attacks
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
		if ((!reversed && facingRight )|| (reversed && !facingRight ))
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

	ClearPauseBufferedActions();
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

void Actor::LoadAllAuras()
{
	usingAura = true;

	ifstream is;
	is.open("Resources/Kin/kinaura", ios::in|ios::binary );

	if (is.is_open())
	{
		sf::Uint32 totalSize;
		is.read((char*)&totalSize, sizeof(sf::Uint32));

		sf::Uint32 *iList = new sf::Uint32[totalSize];

		is.read((char*)iList, totalSize * sizeof( sf::Uint32) );

		int currIndex = 0;
		sf::Uint32 size;

		sf::Uint32 numT;

		list<Vector2f> *fListArr;
		list<Vector2f> *listPtr;

		Vector2f curr;
		//autorun thing is temporary, should be < Count when fixed
		for (int j = 0; j < Action::Count; ++j)  //AUTORUN
		{
			numT = iList[currIndex++];

			for (int f = 0; f < 3; ++f)
			{
				fListArr = new list<Vector2f>[numT];
				auraPoints[f][j] = fListArr;
					
				for (int t = 0; t < numT; ++t)
				{
					listPtr = &(fListArr[t]);
					size = iList[currIndex++];
					for (int s = 0; s < size; ++s)
					{
						curr.x = iList[currIndex++];
						curr.y = iList[currIndex++];
						listPtr->push_back(curr);
					}
				}
			}
		}

		delete[] iList;
		
		
		cout << "finished reading auras" << endl;
	}
	else
	{
		int totalCounter = 0;
		int totalSize = 0;
		int numTA[Action::Count];
		int sizeExtra = 0;
		int numT;

		for (int j = 0; j < Action::Count; ++j)
		{
			numT = CreateAura(auraPoints[0][j], tileset[j], 0);
			numTA[j] = numT;
			for (int z = 0; z < numT; ++z)
			{
				totalSize += auraPoints[0][j][z].size();
			}
			
			//++totalSize; //num tiles

			CreateAura(auraPoints[1][j], tileset[j], 0, 0, 2);
			for (int z = 0; z < numT; ++z)
			{
				totalSize += auraPoints[1][j][z].size();
			}
			//++totalSize;

			CreateAura(auraPoints[2][j], tileset[j], 0, 0, 4);
			for (int z = 0; z < numT; ++z)
			{
				totalSize += auraPoints[2][j][z].size();
			}

			sizeExtra += numT * 3;
		}
		totalSize *= 2; //because there are 2 position values for each value
		totalSize += Action::Count; //number of tiles

		totalSize += sizeExtra;
		//totalSize +=  //for sizes
		totalSize++; //putting total size at the front

		ofstream os;
		os.open("Resources/Kin/kinaura", ios::out | ios::binary);

		cout << "no aura file found. generating one now." << endl;
		if (!os.is_open())
		{
			cout << "couldn't open aura file for writing" << endl;
			assert(0);
			return;
		}

		sf::Uint32 *viList = new sf::Uint32[totalSize];
		list<Vector2f>::iterator begin;
		list<Vector2f>::iterator end;
		sf::Uint32 size;
		list<Vector2f> *aList;

		viList[totalCounter++] = totalSize - 1; //doesn't count itself

		for (int j = 0; j < Action::Count; ++j)
		{
			numT = numTA[j];
			viList[totalCounter++] = numT;
			for (int f = 0; f < 3; ++f)
			{
				aList = auraPoints[f][j];
				for (int s = 0; s < numT; ++s)
				{
					begin = aList[s].begin();
					end = aList[s].end();
					size = aList[s].size();
					
					viList[totalCounter++] = size;

					for (auto it = begin; it != end; ++it)
					{
						viList[totalCounter++] = (*it).x;
						viList[totalCounter++] = (*it).y;
					}
				}
			}
		}

		os.write((char*)viList, sizeof(sf::Uint32) * totalSize);
		os.close();
		cout << "finished creating aura file" << endl;

		delete[] viList;
	}
	
}

void Actor::InitAfterEnemies()
{
	if( owner != NULL && owner->raceFight != NULL )
	{
		maxBubbles = 2;
	}
	else
	{
		maxBubbles = 5;
	}

	bubbleHitboxes = new CollisionBody*[maxBubbles];
	CollisionBox genericBox;
	genericBox.isCircle = true;
	for (int i = 0; i < maxBubbles; ++i)
	{
		bubbleHitboxes[i] = new CollisionBody(1);
		bubbleHitboxes[i]->AddCollisionBox( 0, genericBox);
	}
	//memset(bubbleHitboxes, 0, sizeof(bubbleHitboxes));

	bubblePos = new V2d[maxBubbles];
	bubbleFramesToLive = new int[maxBubbles];
	bubbleRadiusSize = new int[maxBubbles];

	for( int i = 0; i < maxBubbles; ++i )
	{
		bubbleFramesToLive[i] = 0;

		if (fBubblePos != NULL)
		{
			fBubbleFrame[i] = 0;
		}
	}

	if( owner != NULL && owner->raceFight != NULL )
	{
		invincibleFrames = 180;
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

void Actor::StartAction(Action a)
{
	SetAction(a);
	frame = 0;
}

void Actor::SetAction( Action a )
{
	standNDashBoost = (action == STANDN && a == DASH && currAttackHit );

	if (action == AIRHITSTUN || action == GROUNDHITSTUN )
	{
		stunBufferedJump = false;
		stunBufferedDash = false;
		stunBufferedAttack = Action::Count;
	}
	

	action = a;

	if (repeatingSound != NULL)
	{
		DeactivateSound(repeatingSound);
		repeatingSound = NULL;
	}

	if (action == GRAVREVERSE || action == LAND || action == LAND2)
	{
		currBBoostCounter = 0;
	}

	if (action == WALLJUMP)
	{
		if (currInput.rightShoulder && !prevInput.rightShoulder)
		{
			if (currInput.LUp())
			{
				wallJumpBufferedAttack = UAIR; //none
			}
			else if (currInput.LDown())
			{
				wallJumpBufferedAttack = DAIR;
			}
			else
			{
				wallJumpBufferedAttack = FAIR;
			}
		}
		else
		{
			wallJumpBufferedAttack = WALLJUMP;
		}
	}

	switch (action)
	{
	case STEEPSLIDE:
	{
		repeatingSound = ActivateSound(S_STEEPSLIDE, true);
		break;
	}
	case SLIDE:
	{
		repeatingSound = ActivateSound(S_SLIDE, true);
		break;
	}
	/*case UAIR:
	{
		if (currLockedUairFX != NULL && a != UAIR)
		{
			currLockedUairFX->ClearLockPos();
			currLockedUairFX = NULL;
		}
		break;
	}
	case FAIR:
	{
		if (currLockedFairFX != NULL && a != FAIR)
		{
			currLockedFairFX->ClearLockPos();
			currLockedFairFX = NULL;
		}
		break;
	}
	case DAIR:
	{
		if (currLockedDairFX != NULL && a != UAIR)
		{
			currLockedDairFX->ClearLockPos();
			currLockedDairFX = NULL;
		}
		break;
	}
	*/

		
	}

	//shouldnt this be slow counter?
	/*if( slowMultiple > 1 )
	{
		slowMultiple = 1;
	}*/

	

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
	if (hasPowerGrindBall)
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

bool Actor::HasPower(int index)
{
	switch (index)
	{
	case 0:
		return hasPowerAirDash;
	case 1:
		return hasPowerGravReverse;
	case 2:
		return hasPowerBounce;
	case 3:
		return hasPowerGrindBall;
	case 4:
		return hasPowerTimeSlow;
	case 5:
		return hasPowerLeftWire || hasPowerRightWire;
	}
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
			bool levValid = owner->level != NULL && !owner->level->IsLastInSector();
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
				SaveFile *currFile = owner->GetCurrentProgress();
				owner->mainMenu->worldMap->CompleteCurrentMap(currFile, owner->totalFramesBeforeGoal);
				currFile->Save();
			}
			owner->NextFrameRestartLevel();
			return;
		}
	}
}

void Actor::EndLevelWithoutGoal()
{
	owner->scoreDisplay->Activate();
	SetAction(Actor::GOALKILLWAIT);
	frame = 0;
}

void Actor::SetStandInPlacePos(Edge *g, double q,
	bool fr)
{
	facingRight = fr;
	SetGroundedPos(g, q);
	StandInPlace();
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
	owner->SetPlayerInputOn(false);
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
	
	if( owner != NULL )
		owner->barrierTree->Query(this, r);
	


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

		if( owner != NULL )
			owner->barrierTree->Query(this, r);

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

	if( owner != NULL )
		owner->barrierTree->Query(this, r);


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

	if ( owner != NULL && owner->hasAnyGrass)
	{
		queryMode = "grass";
		jumpGrassCount = 0;
		gravityGrassCount = 0;
		bounceGrassCount = 0;
		boostGrassCount = 0;
		owner->grassTree->Query(this, r);
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
	if( owner != NULL && owner->totalRails > 0 && (canRailGrind || canRailSlide) )
	{
		queryMode = "rail";
		owner->railEdgeTree->Query(this, r);
	}


	currBooster = NULL;
	currBounceBooster = NULL;
	currModifier = NULL;

	if (owner != NULL)
	{
		queryMode = "activeitem";
		owner->activeItemTree->Query(this, r);
	}
	
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
					SetActionExpr( JUMP );
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
						SetActionExpr( JUMP );
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
									SetActionExpr( JUMP );
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
					SetActionExpr( JUMP );
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
					SetActionExpr( JUMP );
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
						SetActionExpr( JUMP );
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
									SetActionExpr( JUMP );
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
					SetActionExpr( JUMP );
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

										q = ground->GetQuantity(ground->GetPoint(q) + minContact.resolution);

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
									q = ground->GetQuantity(ground->GetPoint(q) + minContact.resolution);
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
								q = ground->GetQuantity(ground->GetPoint(q) + minContact.resolution);
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
				SetActionExpr(JUMPSQUAT);
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
			if (!hasPowerGravReverse || (abs(grindNorm.x) >= wallThresh) || jump || grindEdge->IsInvisibleWall())//|| !hasGravReverse ) )
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

				SetActionExpr(JUMP);
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
		SetActionExpr(DOUBLE);
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

bool Actor::TrySlide()
{
	//bool t = (!currInput.LUp() && ((gNorm.x > 0 && facingRight) || (gNorm.x < 0 && !facingRight)));
	//if (!(currInput.LLeft() || currInput.LRight()))//&& t )
	//{
	//	if (currInput.LDown() || currInput.LUp())
	//	{
	//		SetAction(SLIDE);
	//		frame = 0;

	//	}
	//	else if (currInput.LUp())
	//	{
	//		//stay running
	//	}
	//	else
	//	{
	//		SetActionExpr(STAND);
	//		frame = 0;
	//	}
	//	////runTappingSound.stop();
	//	return true;

	//}
	if (!(currInput.LLeft() || currInput.LRight()))//&& t )
	{
		if (currInput.LDown() || currInput.LUp())
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

bool Actor::TrySprint(V2d &gNorm)
{
	if (facingRight && currInput.LLeft())
	{
		if ((currInput.LDown() && gNorm.x < 0) || (currInput.LUp() && gNorm.x > 0))
		{
			SetActionExpr(SPRINT);
		}
		else
		{
			SetActionExpr(RUN);
		}

		groundSpeed = 0;
		facingRight = false;
		frame = 0;
		//runTappingSound.stop();
		return true;
	}
	else if (!facingRight && currInput.LRight())
	{
		if ((currInput.LDown() && gNorm.x > 0) || (currInput.LUp() && gNorm.x < 0))
		{
			SetActionExpr(SPRINT);
		}
		else
		{
			SetActionExpr(RUN);
		}

		groundSpeed = 0;
		facingRight = true;
		frame = 0;
		//runTappingSound.stop();
		return true;
	}
	else if ((currInput.LDown() && ((gNorm.x > 0 && facingRight && currInput.LRight()) || (gNorm.x < 0 && !facingRight && currInput.LLeft())))
		|| (currInput.LUp() && ((gNorm.x < 0 && facingRight && currInput.LRight()) || (gNorm.x > 0 && !facingRight && currInput.LLeft()))))
	{
		bool oldActionRun = (action == RUN);
		SetActionExpr(SPRINT);

		if (oldActionRun)
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
	if (hasPowerGrindBall && currInput.Y && !prevInput.Y)
	{
		SetActionGrind();
		BounceFlameOff();
		return true;
	}

	return false;
}

bool Actor::BasicGroundAction( V2d &gNorm)
{
	CheckBounceFlame();

	if (TryGrind()) return true;


	//bool canJump = (currInput.A && !prevInput.A);
	//bool canDash = (currInput.B && !prevInput.B);


	if(currInput.A && !prevInput.A)
	{
		SetActionExpr(JUMPSQUAT);
		frame = 0;
		return true;
	}
	else if (GroundAttack())
	{
		return true;
	}
	else if (currInput.B && !prevInput.B)
	{
		SetActionExpr(DASH);
		frame = 0;
		return true;
	}

	if (BasicSteepAction( gNorm )) return true;


	
	

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
	if (hasPowerBounce)
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
			if (curr->enemyHitBody->Intersects(curr->enemyHitboxFrame, cb, cbFrame))
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

bool Actor::IntersectMyHurtboxes(CollisionBody *cb, int cbFrame )
{
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

//int blah = 0;
void Actor::UpdatePhysics()
{
	if( IsIntroAction(action) || IsGoalKillAction(action) || action == EXIT
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || action == EXITBOOST)
		return;	

	if( action == DEATH )
	{
		return;
	}

	if( test )
		return;
	
	if( rightWire != NULL )
		rightWire->UpdateChargesPhysics();
	if( leftWire != NULL )
		leftWire->UpdateChargesPhysics();

	if (rightWire != NULL)
	{
		rightWire->UpdateEnemyAnchor();
	}
	
	if (leftWire != NULL)
	{
		leftWire->UpdateEnemyAnchor();
	}
	

	double temp_groundSpeed = groundSpeed / slowMultiple;
	V2d temp_velocity = velocity / (double)slowMultiple;
	//cout << "temp velocity: " << temp_velocity.x << ", " << temp_velocity.y << endl;
	double temp_grindSpeed = grindSpeed / slowMultiple;

	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec(0, 0);
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
		//cout << "movelength: " << moveLength << endl;
		//cout << "movevec: " << movementVec.x << ", " << movementVec.y << endl;
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
		Edge *e0 = grindEdge->edge0;
		Edge *e1 = grindEdge->edge1;
		V2d e0n = e0->Normal();
		V2d e1n = e1->Normal();
		
		double q = grindQuantity;
		double hitBorderSpeed = GetDashSpeed() / 2;

		while( !approxEquals(movement, 0 ) )
		{
			//cout << "movement: " << movement << endl;
			double gLen = length( grindEdge->v1 - grindEdge->v0 );
			if( movement > 0 )
			{
				double extra = q + movement - gLen;
				V2d gPoint = grindEdge->GetPoint(q + movement);
				if ( owner != NULL && !owner->IsWithinCurrentBounds(gPoint))
				{
					grindSpeed = max(-grindSpeed, -hitBorderSpeed);
					//grindSpeed = -grindSpeed;
					return;
				}

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
								UnlockGate( gg );

								if( e1 == gg->edgeA )
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

					if( GameSession::IsWall( grindEdge->Normal() ) == -1 )
					{
						if( hasPowerGravReverse || grindEdge->Normal().y < 0 )
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
			else if( movement < 0 )
			{
				double extra = q + movement;

				V2d gPoint = grindEdge->GetPoint(q + movement);
				if (owner != NULL && !owner->IsWithinCurrentBounds(gPoint))
				{
					grindSpeed = min( -grindSpeed, hitBorderSpeed);
					
					return;
				}

				if( extra < 0 )
				{
					movement -= movement - extra;

					V2d v0 = grindEdge->v0;
					sf::Rect<double> r( v0.x - 1, v0.y - 1, 2, 2 );

					
					//CheckStandUp();
					//if( )

					if( e0->edgeType == Edge::CLOSED_GATE )
					{
						Gate *gg = (Gate*)e0->info;
						if( gg->gState == Gate::SOFT || gg->gState == Gate::SOFTEN )
						{
							if( CanUnlockGate( gg ) )
							{
								//cout << "unlock gate" << endl;
								UnlockGate( gg );

								if( e0 == gg->edgeA )
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
					q = length( grindEdge->v1 - grindEdge->v0 );

					if( GameSession::IsWall( grindEdge->Normal() ) == -1 )
					{
						if( hasPowerGravReverse || grindEdge->Normal().y < 0 )
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
		return;
	}
	else if (grindEdge != NULL && ( action == RAILGRIND || action == RAILSLIDE ))
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
						return;
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
						return;
						
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

		if (action == RAILGRIND || action == RAILSLIDE )
		{
			grindQuantity = q;

			PhysicsResponse();
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
						UnlockGate( g );

						if( e0 == g->edgeA )
						{
							gateTouched = g->edgeB;
						}
						else
						{
							gateTouched = g->edgeA;
							
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
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
									holdJump = false;
								}
								else
								{
								//	cout << "this steep" << endl;
									facingRight = false;
									if (!IsGroundAttackAction(action))
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
							//	cout << "this steepclimb" << endl;
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
					holdJump = false;
					//rightWire->UpdateAnchors( V2d( 0, 0 ) );
					//leftWire->UpdateAnchors( V2d( 0, 0 ) );
					ground = NULL;
				}
			}
			else if( transferRight )
			{
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
				}
			//	cout << "transferRight!" << endl;
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
					//		cout << "steep transfer right" << endl;
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
									holdJump = false;
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
									ground = NULL;
								}
								else
								{
						//			cout << "this steep 1" << endl;
									//cout << "slidin" << endl;
									facingRight = true;
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
						//		cout << "this here??" << endl;
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
					SetActionExpr( JUMP );
					frame = 1;
					holdJump = false;
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

							velocity = normalize(ground->v1 - ground->v0) * groundSpeed + V2d(0, -gravity * 2);
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;

							movementVec.y -= .01;
							if( movementVec.x <= .01 )
							{
								movementVec.x = .01;
							}
							cout << "real slope jump A" << endl;
							leftGround = true;
							SetActionExpr( JUMP );
							holdJump = false;
							frame = 1;
							ground = NULL;
							break;
						}
						else
						{
							if( gNormal.x < 0 && gNormal.y > -steepThresh && e1n.x >= 0
								&& abs( e1n.x ) < wallThresh && groundSpeed > 5 )
							{
								velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed + V2d(0, -gravity * 2);;
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
								holdJump = false;
								frame = 1;
								//rightWire->UpdateAnchors( V2d( 0, 0 ) );
								//leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = NULL;
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

							velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed + V2d(0, -gravity * 2);
							movementVec = normalize( ground->v1 - ground->v0 ) * extra;

							movementVec.y -= .01;
							if( movementVec.x <= .01 )
							{
								movementVec.x = .01;
							}
							cout << "real slope jump B" << endl;
							leftGround = true;
							SetActionExpr( JUMP );
							holdJump = false;
							frame = 1;
							//rightWire->UpdateAnchors( V2d( 0, 0 ) );
							//leftWire->UpdateAnchors( V2d( 0, 0 ) );
							ground = NULL;
							break;
						}
						else
						{
							if( gNormal.x > 0 && gNormal.y > -steepThresh && e0n.x <= 0
								&& abs( e0n.x ) < wallThresh && groundSpeed < -5 )
							{
								velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed + V2d(0, -gravity * 2);
								movementVec = normalize( ground->v1 - ground->v0 ) * extra;

								movementVec.y -= .01;
								if( movementVec.x <= .01 )
								{
									movementVec.x = .01;
								}
								cout << "real slope jump C" << endl;
								leftGround = true;
								SetActionExpr( JUMP );
								holdJump = false;
								frame = 1;
								//rightWire->UpdateAnchors( V2d( 0, 0 ) );
								//leftWire->UpdateAnchors( V2d( 0, 0 ) );
								ground = NULL;
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
				
				if (IsAutoRunning())
				{
					if ( autoRunStopEdge != NULL && autoRunStopEdge == ground)
					{
						if (facingRight)
						{
							if (autoRunStopQuant <= q && autoRunStopQuant >= q - m)
							{
								//StandInPlace();
								WaitInPlace();
								return;
							}
						}
						else
						{
							if (autoRunStopQuant >= q && autoRunStopQuant <= q - m)
							{
								//StandInPlace();
								WaitInPlace();
								return;
							}
						}
					}
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
									UnlockGate( g );

									if( e1 == g->edgeA )
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
							if( e0->edgeType == Edge::CLOSED_GATE )
								{
							//		cout << "similar secret but not reversed D" << endl;
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
										q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
										groundSpeed = 0;
										edgeQuantity = q;
										break;
									}
									else
									{	
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );

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
									q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
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
									SetActionExpr( JUMP );
									holdJump = false;
									frame = 1;
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
								}
								else
								{

									//cout << "action: "<< (int)action << endl;
									//cout << "a bounceFlameOn: " << (int)bounceFlameOn << "gspeed: " << groundSpeed << endl;
									if( bounceFlameOn && abs( groundSpeed ) > 1)
									{
										if( action != STEEPCLIMB )
										{
											storedBounceGroundSpeed = groundSpeed * slowMultiple;
											groundedWallBounce = true;
										}
										
									}


									q = ground->GetQuantity(ground->GetPoint(q) + minContact.resolution);

									groundSpeed = 0;
									edgeQuantity = q;
								}
							}
						}
						else
						{
							q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
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

			/*if (groundSpeed < 0 && cantPushPastQuant >= 0 )
			{
				if (q < cantPushPastQuant + 1.0)
				{
					q = cantPushPastQuant + 1.0;
				}
			}*/

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
				//cout << "setting steal vec: " << stealVec.x << ", " << stealVec.y << endl;
				//	<< moveLength << endl;
				movementVec = velDir * maxMovement;
			}

			V2d newVel( 0, 0 );
			V2d oldPos = position;

			//cout << "movement: " << movementVec.x << ", " << movementVec.y << endl;
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
				//cout << "movmeent vec: " << movementVec.x << ", " << movementVec.y << endl;
				//cout << "contact res: " << minContact.resolution.x << ", " << minContact.resolution.y << endl;
				
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
				
				//cout << "extraVel: " << extraVel.x << ", " << extraVel.y << endl;
				if( length(extraVel) < .01 )
					extraVel = V2d( 0, 0 );

				//extraVel = V2d( 0, 0 );
				newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );
				
				if( length( stealVec ) > 0 )
				{
					stealVec = length( stealVec ) * normalize( extraVel );
					//cout << "modify steal: " << stealVec.x << ", " << stealVec.y << endl;
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
					SetActionExpr( JUMP );
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
				if (minContact.normal.y < 0)
				{
					velocity.y = -25;
				}
				else if( minContact.normal.y > 0 )
				{
					velocity.y = 25;
				}
				
				hasDoubleJump = true;
				hasAirDash = true;

				if (action == AIRDASH || IsSpringAction(action) )
				{
					SetAction(JUMP);
					frame = 1;
				}
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
						SetActionExpr( JUMP );
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
			else if( (hasPowerGravReverse || gravityGrassCount > 0 ) 
				&& tempCollision && ((currInput.B && currInput.LUp())|| (hasPowerGrindBall && currInput.Y ))
				&& minContact.normal.y > 0 
				&& abs( minContact.normal.x ) < wallThresh 
				&& minContact.position.y <= position.y - b.rh + b.offset.y + 1
				&& !minContact.edge->IsInvisibleWall() )
			{
				prevRail = NULL;
				//cout << "vel: " << velocity.x << ", " << velocity.y << endl;
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

				edgeQuantity = minContact.edge->GetQuantity( minContact.position );

				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = 0;
				//groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				V2d gno = ground->Normal();


				double angle = atan2( gno.x, -gno.y );


				//cout << "frames in air: " << framesInAir << ", holdJump: " << (int)holdJump << endl;
				if( trueFramesInAir < 10 && holdJump && -gno.y > -steepThresh )
				{
					//cout << "adjusted y vel from: " << velocity.y;
					//velocity.y *= .7;
					//cout << ", new: " << velocity.y << endl;
					
				}

				
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
				ActivateEffect( EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight );
				ActivateSound( S_GRAVREVERSE );
				//}
			}
			else if( tempCollision && hasPowerGrindBall /*&& action == AIRDASH*/ && currInput.Y && velocity.y != 0 && abs( minContact.normal.x ) >= wallThresh && !minContact.edge->IsInvisibleWall()  )
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
				velocity = newVel;
			}
			else
			{
				//cout << "no temp collision" << endl;
			}

			//cout << "steal: " << stealVec.x << ", " << stealVec.y << endl;
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

bool Actor::GroundAttack()
{
	bool normalSwing = CheckNormalSwing();
	bool rightStickSwing = CheckRightStickSwing();

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
			if (currInput.RLeft())
			{
				facingRight = false;
			}
			else if (currInput.RRight())
			{
				facingRight = true;
			}
		}


		SetAction(STANDN);
		frame = 0;

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


		V2d grindPoint = grindEdge->GetPoint( grindQuantity );

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

					
					position = bounceEdge->GetPoint( bounceQuant );

				}
				else
				{
					V2d oldv0 = ground->v0;
					V2d oldv1 = ground->v1;


					position = ground->GetPoint( bounceQuant );
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

				SetActionExpr( JUMP );
				frame = 1;
				
				framesInAir = 0;
				holdJump = false;
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
		
		Vector2<double> groundPoint = ground->GetPoint( edgeQuantity );
		
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
					SetActionExpr( JUMP );
					frame = 1;
					holdJump = false;
				}
			}

			if( leftGround )
			{
				SetActionExpr( JUMP );
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

	//rightWire->UpdateAnchors(V2d( 0, 0 ));
	//leftWire->UpdateAnchors(V2d( 0, 0 ));

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
		V2d nEdge = edge->Normal();//normalize( edge->v1 - edge->v0 );
		double ang = atan2(nEdge.x, -nEdge.y);

		double crossA = dot( A - edge->v0, nEdge );
		double crossB = dot( B - edge->v0, nEdge );
		double crossC = dot( C - edge->v0, nEdge );
		double crossD = dot( D - edge->v0, nEdge );

		//double crossCenter = cross(b.globalPosition - edge->v0, nEdge);
		double alongAmount = dot(b.globalPosition - edge->v0, normalize(edge->v1 - edge->v0));
		alongAmount /= length(edge->v1 - edge->v0);
		alongAmount = 1.0 - alongAmount;
		V2d alongPos = edge->v1 + normalize(edge->v0 - edge->v1) * alongAmount * edge->GetLength();

		double thresh = .01;
		bool activate = crossA > thresh && crossB > thresh && crossC > thresh && crossD > thresh;
		 

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
			//activate = true;
		}
		else
		{
			//cout << "groundSpeed: " << groundSpeed << ", " << 
		}
		g->SetLocked( false );

		if( activate )
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
			
			if (g->IsZoneType())
			{

				owner->SuppressEnemyKeys(g);

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

				if (!g->IsTwoWay()) //for secret gates
				{
					if (oldZone != NULL && oldZone->active)
					{
						oldZone->ReformAllGates(g);
					}

					owner->keyMarker->SetStartKeysZone(newZone);
				}

				owner->ActivateZone(newZone);

			}
			
			if( g->IsReformingType())
			{
				owner->LockGate( g );
				
				g->gState = Gate::REFORM;
				g->frame = 0;
				float aa = alongAmount;
				g->centerShader.setUniform("breakPosQuant", aa);
			}
			else
			{
				g->gState = Gate::DISSOLVE;
				g->frame = 0;
				float aa = alongAmount;
				g->centerShader.setUniform("breakPosQuant", aa);
			}

			V2d gEnterPos = alongPos + nEdge;// *32.0;
			Tileset *ts_gateEnter = GetTileset("FX/gateenter_256x320.png",256, 320);
			ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				ts_gateEnter, gEnterPos, false, ang, 8, 3, true);

			//set gate action to disperse
			//maybe have another gate action when you're on the gate and its not sure whether to blow up or not
			//it only enters this state if you already unlock it though
			gateTouched = NULL;
		}
		else if( crossA < 0 && crossB < 0 && crossC < 0 && crossD < 0 )
		{
			gateTouched = NULL;
			owner->LockGate( g );
			//cout << "went back" << endl;
		}
		else
		{
			//cout << "not clear" << endl;
		}
	}

	//only for motion ghosts
	//UpdateSprite();
	
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

	
	/*if( ghostSpacingCounter == motionGhostSpacing )
	{
		ghostSpacingCounter = 0;
		for( int i = maxMotionGhosts -1; i > 0; --i )
		{
			motionGhosts[i] = motionGhosts[i-1];
		}
		motionGhosts[0] = *sprite;
	}
	else
	{
		ghostSpacingCounter++;
	}*/
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
		list<CollisionBox> *cList = (currHitboxes->GetCollisionBoxes(currHitboxFrame));
		if( cList != NULL )
		for( list<CollisionBox>::iterator it = cList->begin(); it != cList->end(); ++it )
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
				pos = grindEdge->GetPoint( grindQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
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
		hurtBody.globalPosition = grindEdge->GetPoint( grindQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
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
			hurtBody.globalPosition = ground->GetPoint(edgeQuantity) + V2d( offsetX, 0 ) + hurtBody.offset + gn * hurtBody.rh ;//+ V2d( 0, -hurtBody.rh );
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

void Actor::UpdatePostPhysics()
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

	if (!IsIntroAction(action) && GetTotalGameFrames() % smallLightningCounter == 0 && !dontActivateLightningAction)
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

	if (!IsIntroAction(action) && GetTotalGameFrames() % 30 == 0)
	{
		RelEffectInstance params;
		//EffectInstance params;
		Transform tr = sf::Transform::Identity;
		Vector2f randPos(rand() % 20 - 10, rand() % 20 - 10);

		params.SetParams(randPos, tr, 1, 60, 0, &spriteCenter, 40 );
		//EffectInstance *ei = risingAuraPool->ActivateEffect(&params);
		//ei->SetVelocityParams(Vector2f(0, 0), Vector2f(0, -.02), 5 );
	}

	if (currLockedFairFX != NULL && action != FAIR )
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

	
	QueryTouchGrass();

	ClearSpecialTerrainCounts();
	queryMode = "specialterrain";
	Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
	GetSpecialTerrainTree()->Query(this, r);

	HandleSpecialTerrain();
	
	for (int i = 0; i < 7; ++i)
	{
		smallLightningPool[i]->Update();
	}
	
	//testPool->ActivateEffect( )

	//rightWire->UpdateAnchors2(V2d( 0, 0 ));
	//leftWire->UpdateAnchors2( V2d( 0, 0 ) );
	updateAura = true;

	//cout << "action: " << action << endl;
	test = false;

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
		//sh.setUniform( "On0", false );
		//sh.setUniform( "On1", false );
		//sh.setUniform( "On2", false );
		//sh.setUniform( "despFrame", (float)-1 );

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
			ActivateEffect( EffectLayer::IN_FRONT, ts_fx_death_1c, pos, false, fxAngle, 12, 2, true );
		}
		//if( slowCounter == slowMultiple )
		//{
		int total = actionLength[DEATH];
		//int faceDeathAnimLength = 11;
		//int an = 6;
		//int extra = 22;

		//if (frame == 0)
		//{
		//	kinMask->SetExpr(KinMask::Expr_DEATHYELL);
		//}
		//else if (frame == extra)
		//{
		//	kinMask->SetExpr(KinMask::Expr_DEATH);
		//}

		//if( frame < faceDeathAnimLength * an + extra && frame >= extra )
		//{
		//	int f = (frame - extra) / an;
		//	expr = (Expr)(f);
		//}
		//else
		//{
		//	expr = (Expr)0;
		//}
	

		////cout << "death: " << expr + 11 << endl;
		//kinFace.setTextureRect( ts_kinFace->GetSubRect( expr + 11 ) );
		//kinFaceBG.setTextureRect(ts_kinFace->GetSubRect(5));
		if( kinMask != NULL)
			kinMask->Update( speedLevel, desperationMode );


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

	if( hitGoal )// && action != GOALKILL && action != EXIT && action != GOALKILLWAIT && action != EXITWAIT)
	{
		
		if (owner != NULL)
		{


			owner->totalFramesBeforeGoal = owner->totalGameFrames;
			SetActionExpr(GOALKILL);
			desperationMode = false;
			hitGoal = false;
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

			frame = 0;
			position = owner->goalNodePos;
			owner->cam.Ease(Vector2f(owner->goalNodePosFinal), 1, 60, CubicBezier());
			rightWire->Reset();
			leftWire->Reset();
			desperationMode = false;
		}
		else if (editOwner != NULL )
		{
			editOwner->EndTestMode();
		}
	}
	else if (hitNexus)
	{
		owner->totalFramesBeforeGoal = owner->totalGameFrames;
		SetActionExpr(NEXUSKILL);
		desperationMode = false;
		hitNexus = false;
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

		frame = 0;
		position = owner->goalNodePos;
		rightWire->Reset();
		leftWire->Reset();
		desperationMode = false;
	}

	
	UpdateSprite();
	
	//risingAuraPool->Update();
	CreateAttackLightning();

	for (int i = 0; i < 3; ++i)
	{
		fairLightningPool[i]->Update();
		dairLightningPool[i]->Update();
		uairLightningPool[i]->Update();
	}
	
	gateBlackFXPool->Update();
	//if (updateAura)
	if( usingAura )
	{
		testAura->Update();
		testAura1->Update();
		testAura2->Update();
		testAura3->Update();
	}


	
	
	
	//shouldn't be max motion ghosts cuz thats not efficient
	for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhostBuffer->SetRotation( i, sprite->getRotation() / 180.f * PI);
		motionGhostBufferBlue->SetRotation(i, sprite->getRotation() / 180.f * PI);
		motionGhostBufferPurple->SetRotation(i, sprite->getRotation() / 180.f * PI);
		/*Vector2f scale = motionGhostBuffer->GetMemberInfo(i).scale;
		if (facingRight || (reversed && !facingRight) )
		{
			motionGhostBuffer->SetScale( i, Vector2f( abs(scale.x), scale.y ) );
		}
		else
		{
			motionGhostBuffer->SetScale(i, Vector2f(-abs( scale.x ), scale.y));
		}*/
		//motionGhostBuffer->setpo
	}
	
	/*for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhosts[i] = *sprite;
	}*/

	/*CubicBezier cb(.11, 1.01, .4, .96);
	float start = 255.0;
	for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhosts[i] = *sprite;
		float factor = (float)i / maxMotionGhosts;
		float a = cb.GetValue(factor);
		int alpha = max( start - a * start, 0.f );
		motionGhosts[i].setColor(sf::Color(255, 255, 255, alpha ) );
	}*/

	

	switch( action )
	{
	case AIRDASH:
		{
			bool horizontal = abs( velocity.y ) < 7; //for hover
			bool stopped = horizontal && velocity.x == 0;
			if( frame % 1 == 0 && !stopped )
			{
				//ActivateEffect( ts_fx_airdashSmall, V2d( position.x, position.y + 0 ), false, 0, 12, 4, facingRight );
			}

			if( ( frame + framesExtendingAirdash ) % 4 == 0 && slowCounter == 1 )
			{
				if( stopped )
				{
					//cout << "frame: " << frame << endl;
					//if( frame % 4 == 1 )
						ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_airdashHover, V2d( position.x, position.y + 70 ), 
							false, 0, 12, 1, facingRight );
				}
				else if( horizontal )
				{
					//cout << "STUFF???" << endl;
					if( velocity.x > 0 )
					{
						ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_airdashUp, V2d( position.x - 64, position.y - 18 ), false, PI / 2.0, 15, 3, true );
					}
					else
					{
						ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_airdashUp, V2d( position.x + 64, position.y - 18 ), false, -PI / 2.0, 15, 3, true );
					}
				}
				else if( velocity.x == 0 && velocity.y < 0 )
				{
					ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashUp, V2d( position.x, position.y + 64 ), false, 0, 15, 3, facingRight );
				}
				else if( velocity.x == 0 && velocity.y > 0 )
				{
					ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashUp, V2d( position.x, position.y + 0 ), false, PI, 15, 3, facingRight );
				}
				else if( velocity.x > 0 && velocity.y > 0 )
				{
					V2d pos = V2d( position.x - 40, position.y - 60 );
					ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, pos, false, PI, 15, 3, true );//facingRight );
				}
				else if( velocity.x < 0 && velocity.y > 0 )
				{
					ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, V2d( position.x + 40, position.y - 60 ), false, PI, 15, 3, false );//facingRight );
				}
				else if( velocity.x < 0 && velocity.y < 0 )
				{
					ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, V2d( position.x + 54, position.y + 60 ), false, 0, 15, 3, true );
				}
				else if( velocity.x > 0 && velocity.y < 0 )
				{
					ActivateEffect( EffectLayer::IN_FRONT, ts_fx_airdashDiagonal, V2d( position.x - 54, position.y + 60 ), false, 0, 15, 3, false );
				}
				
				//cout << "airdash fx" << endl;
				
			}

			
		}
		break;
	}

	V2d trueVel;
	double speed;
	if( ground != NULL ) //ground
	{
		trueVel = normalize( ground->v1 - ground->v0 ) * groundSpeed;
		//speed = abs(groundSpeed);
	}
	else //air
	{
		trueVel = velocity;
		//speed = length( velocity );
	}

	trueVel.y = min( 40.0, trueVel.y ); //falling can only help your speed so much
		
	speed = length( trueVel );

	bool careAboutSpeedAction = action != DEATH && action != EXIT && !IsGoalKillAction(action) && action != RIDESHIP && action != GRINDBALL
		&& action != GRINDATTACK;
	if (careAboutSpeedAction)
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

	if( speedParticleCounter == speedParticleRate && careAboutSpeedAction)
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

				

				ActivateEffect( EffectLayer::IN_FRONT, tset, truePos, false, angle, 6, 3, facingRight );
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
				ActivateEffect( EffectLayer::IN_FRONT, tset, truePos, false, angle, 6, 3, facingRight );
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
				
				ActivateEffect( EffectLayer::IN_FRONT, ts_fx_chargePurple, truePos, false, angle, 6, 3, facingRight );
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
				ActivateEffect( EffectLayer::IN_FRONT, ts_fx_chargePurple, truePos, false, angle, 6, 3, facingRight );
			}
		}

		speedParticleCounter = 0;
	}
	
	

	//Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );


	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
	

	if( desperationMode )
	{
		//cout << "sending this parameter! "<< endl;
		sh.setUniform( "despFrame", (float)despCounter );
	}
	else
	{
		
		sh.setUniform( "despFrame", (float)-1 );
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


	//happens even when in time slow
	if (action == DASH)
	{
		if (currBBoostCounter < maxBBoostCount)
		{
			currBBoostCounter++;
		}
	}

	if( slowCounter == slowMultiple )
	{
		if( wallJumpFrameCounter < wallJumpMovementLimit )
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
			
		//if (hitCeilingCounter > 0)
		//	--hitCeilingCounter;

		if( action == BOUNCEAIR && oldBounceEdge != NULL )
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

		if( !IsIntroAction(action) )
			if( invincibleFrames > 0 )
				--invincibleFrames;

		if( flashFrames > 0 )
			--flashFrames;
	}
	else
		slowCounter++;

	if (framesSinceGrindAttempt < maxFramesSinceGrindAttempt)
	{
		++framesSinceGrindAttempt;
	}

	if( standNDashBoostCurr > 0 )
		standNDashBoostCurr--;

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

		++framesFlameOn;
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

	//pTrail->Update( position );

	

	

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

	if( kinMask != NULL)
		kinMask->Update(speedLevel, desperationMode);

	//if( expr == Expr_NEUTRAL || expr == Expr_SPEED1 || expr == Expr_SPEED2 )
	//{
	//	switch( speedLevel )
	//	{
	//	case 0:
	//		expr = Expr_NEUTRAL;
	//		break;
	//	case 1:
	//		expr = Expr_SPEED1;
	//		break;
	//	case 2:
	//		expr = Expr_SPEED2;
	//		break;
	//	}
	//	//despCounter == maxDespFrames )
	//	if( desperationMode )
	//	{
	//		expr = Expr_DESP;
	//	}
	//}

	//if (action != DEATH)
	//{
		//kinFace.setTextureRect(ts_kinFace->GetSubRect(expr + 6));
		//kinFaceBG.setTextureRect(ts_kinFace->GetSubRect(expr));
	//}
	


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

	if (action == EXITWAIT && frame == actionLength[EXITWAIT])
	{
		owner->EndLevel();
		//owner->goalDestroyed = true;	
	}
	else if (action == EXITBOOST && frame == actionLength[EXITBOOST])
	{
		owner->EndLevel();
		//owner->goalDestroyed = true;
	}
	

	if (kinRing != NULL)
		kinRing->Update();

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

	/*if( updateAura )
		testAura->Update();*/
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
		ShipPickupPoint(trigger->edgeQuantity, trigger->facingRight);
		break;
	}
	case TRIGGER_HOUSEFAMILY:
	{
		desperationMode = false;
		SetExpr(Expr_NEUTRAL);
		assert(ground != NULL);
		edgeQuantity = trigger->edgeQuantity;
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
		desperationMode = false;
		SetExpr(Expr_NEUTRAL);
		assert(ground != NULL);
		edgeQuantity = trigger->edgeQuantity;
		groundSpeed = 0;
		//facingRight = trigger->facingRight;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		//SetAction(GETPOWER_AIRDASH_MEDITATE);
		//frame = 0;
		physicsOver = true;
		owner->activeSequence = trigger->gameSequence;
		//owner->currStorySequence = trigger->storySeq;
		//owner->state = GameSession::STORY;
		break;
	}
	case TRIGGER_DESTROYNEXUS1:
	{
		desperationMode = false;
		//SetExpr(Expr_NEUTRAL);
		//assert(ground != NULL);
		edgeQuantity = trigger->edgeQuantity;
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		//owner->Fade(false, 30, Color::Black);

		owner->activeSequence = trigger->gameSequence;

		SetAction(SEQ_ENTERCORE1);
		frame = 0;
		physicsOver = true;

		//owner->currStorySequence = trigger->storySeq;
		//owner->state = GameSession::STORY;
		break;
	}
	case TRIGGER_CRAWLERATTACK:
	{
		desperationMode = false;
		edgeQuantity = trigger->edgeQuantity;
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		SetAction(SEQ_LOOKUP);
		frame = 0;
		physicsOver = true;

		owner->activeSequence = trigger->gameSequence;
		break;
	}
	case TRIGGER_TEXTTEST:
	{
		desperationMode = false;
		edgeQuantity = trigger->edgeQuantity;
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		WaitInPlace();
		physicsOver = true;

		owner->activeSequence = trigger->gameSequence;
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
		position = currSpring->position;
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
		V2d dir = normalize(position - currBounceBooster->position);
		velocity = dir * s;
		velocity.x *= .6;

		if (velocity.y > 0)
			velocity.y *= .5;
	}
	
}

bool Actor::IsGoalKillAction(Action a)
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

bool Actor::IsIntroAction(Action a)
{
	return a == INTRO || a == SPAWNWAIT || a == INTROBOOST;
}

bool Actor::IsSequenceAction(Action a)
{
	return a == SEQ_ENTERCORE1 || action == SEQ_LOOKUP || action == SEQ_KINTHROWN
		|| action == SEQ_KINFALL;
}

bool Actor::IsExitAction(Action a)
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
		Rail *rail = (Rail*)e->info;

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
				prevRail = (Rail*)grindEdge->info;
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
		if (en->type == EnemyType::EN_BOOSTER )
		{
			//Booster *boost = (Booster*)qte;
			//
			//if (currBooster == NULL)
			//{
			//	if (boost->hitBody->Intersects(boost->currHitboxFrame, &hurtBody) && boost->IsBoostable() )
			//	{
			//		currBooster = boost;
			//	}
			//}
			//else
			//{
			//	//some replacement formula later
			//}
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
				if (spr->hitBody->Intersects(spr->currHitboxFrame, &hurtBody) && spr->action == Spring::IDLE)
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
			if (hf->CanCollect() && hf->hitBody->Intersects(hf->currHitboxFrame, &hurtBody))
			{
				if( kinRing != NULL)
					kinRing->powerRing->Fill(hf->healAmount);
				hf->Collect();
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
			if (tGrass->hurtBody->Intersects(0, &hurtBody))
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
		SpecialTerrainPiece *stp = (SpecialTerrainPiece*)qte;
		Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
		//if (stp->IsTouchingBox(r))
		{
			if (stp->IsInsideArea(position))
			{
				specialTerrainCount[stp->specialType]++;
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

void Actor::ApplyHit( HitboxInfo *info )
{
	if (info == NULL)
		return;
	//use the first hit you got. no stacking hits for now
	if( invincibleFrames == 0 )
	{
		if( receivedHit == NULL || info->damage > receivedHit->damage )
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

void Actor::Draw( sf::RenderTarget *target )
{
	//dustParticles->Draw(target);
	if (action == EXITWAIT || action == SPAWNWAIT /*|| (action == INTRO && frame < 11 )*/ || action == SEQ_LOOKUPDISAPPEAR || action == SPRINGSTUNTELEPORT )
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


	if( bounceFlameOn && action != DEATH && action != EXIT && !IsGoalKillAction(action) && action != BOUNCEGROUNDEDWALL && action != GRINDBALL 
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

	if (action != DEATH)
	{
		if (usingAura)
		{
			if (speedLevel > 1)
			{
				testAura2->Draw(target);
			}
			if (speedLevel > 0)
			{
				testAura1->Draw(target);
			}

			testAura->Draw(target);
		}
	}
	
	if (showExitAura)
	{
		target->draw(exitAuraSprite);
		//if we add another "extra aura" should just make a single sprite for them, combining
		//this and dirtyaura
	}

	{

		//RayCast( this, owner->testTree, position, V2d( position.x - 100, position.y ) );
		
		

		//Vector2i vi = Mouse::getPosition();
		////Vector2i vi = owner->window->mapCoordsToPixel( Vector2f( position.x, position.y ) );//sf::Vector2f( 0, -300 ) );

		//Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
		////Vector3f blahblah( vi.x / (float)owner->window->getSize().x, 
		////	(owner->window->getSize().y - vi.y) / (float)owner->window->getSize().x, .015 );

		//
		//
		//if( action != DEATH && action != SPAWNWAIT && action != GOALKILL && action != GOALKILLWAIT )
		////if( action == RUN )
		//{
		//	//sh.setUniform( "u_texture",( *GetTileset( "run.png" , 128, 64 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//	//sh.setUniform( "u_normals", *GetTileset( "run_normal.png", 128, 64 )->texture );
		//	/*Sprite spr;
		//	
		//	spr.setTexture( *GetTileset( "testrocks.png", 300, 225)->texture );
		//	spr.setOrigin( spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2 );
		//	if( !facingRight )
		//	{
		//		sf::IntRect r = spr.getTextureRect();
		//		spr.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
		//	}
		//	spr.setPosition( sprite->getPosition() );
		//	
		//	// global positions first. then zooming

		//	
		//	sh.setUniform( "u_texture",( *GetTileset( "testrocks.png" , 300, 225 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//	sh.setUniform( "u_normals", *GetTileset( "testrocksnormal.png", 300, 225 )->texture );
		//	sh.setUniform( "Resolution", owner->window->getSize().x, owner->window->getSize().y );
		//	//sh.setUniform( "LightPos", blahblah );//Vector3f( 0, -300, .075 ) );
		//	//sh.setUniform( "LightColor", 1, .8, .6, 1 );
		//	sh.setUniform( "AmbientColor", .6, .6, 1, .8 );
		//	//sh.setUniform( "Falloff", Vector3f( .4, 3, 20 ) );
		//	sh.setUniform( "right", (facingRight && !reversed) || (facingRight && reversed ) );
		//	sh.setUniform( "zoom", owner->cam.GetZoom() );
		//	//cout << "right: " << (float)facingRight << endl;

		//	CircleShape cs;
		//	cs.setFillColor( Color::Magenta );
		//	cs.setRadius( 40 );
		//	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		//	cs.setPosition( 0, -300 );*/
		//	

		//	//target->draw( spr, &sh );






		//	//sh.setUniform( "u_texture",( *GetTileset( "run2.png" , 80, 48 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//	//sh.setUniform( "u_normals", *GetTileset( "run_NORMALS.png", 80, 48 )->texture );


		//	//sh.setUniform( "u_texture",( *GetTileset( "run.png" , 128, 64 )->texture ) ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//	//sh.setUniform( "u_normals", *GetTileset( "run_normal.png", 128, 64 )->texture );
		//	//cout << "action: " << action << endl;
		//	//sh.setUniform( "u_texture", *tileset[action]->texture ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//	//sh.setUniform( "u_normals", *normal[action]->texture );
		//	
		//}
		//else
		//{
		//	target->draw( *sprite );
		//}
		

		
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
		if (desperationMode && action != DEATH)
		{
			target->draw(*sprite, &playerDespShader);
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

	/*Color followerCol = Color::Red;
	sf::Vertex follower[2] = 
	{
		Vertex( Vector2f( position.x, position.y ), followerCol ),
		Vertex( Vector2f( followerPos.x, followerPos.y ), followerCol )
	};
	target->draw( follower, 2, sf::Lines );*/
	
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
		desperationMode = false;
		SetExpr(Expr_NEUTRAL);
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

	//hurtBody.DebugDraw(target);



	/*if (currHurtboxes != NULL)
	{
		currHurtboxes->DebugDraw( currHurtboxFrame, target);
	}*/

	b.DebugDraw(target);




	/*sf::CircleShape cs;
	cs.setOutlineThickness( 10 );
	cs.setOutlineColor( Color::Red );
	cs.setFillColor( Color::Transparent );
	cs.setRadius( 160 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );*/
	//target->draw( cs );
	

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
		params.SetParams(Vector2f(ground->GetPoint(edgeQuantity)),
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


	V2d gn( 0, 0 );
	if( ground != NULL )
	{
		gn = ground->Normal();
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

	

	switch( action )
	{
	case SEQ_CRAWLERFIGHT_STAND:
	case SEQ_ENTERCORE1:
	case SEQ_KINSTAND:
	case STAND:
		{	
			SetSpriteTexture( STAND );

			//the %20 is for seq
			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			int f = (frame / 8) % 20;
			SetSpriteTile( f, r );
			assert( ground != NULL );
		
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			sprite->setRotation( angle / PI * 180 );

			if( scorpOn )
			{
				scorpSprite.setTexture( *ts_scorpStand->texture );
				
				SetSpriteTile( &scorpSprite, ts_scorpStand, f, r );
				
				scorpSprite.setOrigin( scorpSprite.getLocalBounds().width / 2,
					scorpSprite.getLocalBounds().height / 2 + 10 );
				scorpSprite.setPosition( position.x, position.y );
				scorpSprite.setRotation( sprite->getRotation() );
				scorpSet = true;
			}


			break;
		}
	case SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY:
	case AUTORUN:
	case RUN:
		{	
			V2d pp = ground->GetPoint( edgeQuantity );

			double angle = GroundedAngle();


			//V2d along = normalize( ground->v1 - ground->v0 );
			SetSpriteTexture(RUN);

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			int f = (frame / 2) % 10;
			SetSpriteTile( f, r );
		
			assert( ground != NULL );
		
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d along;
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
			{
				sprite->setPosition( pp.x + offsetX, pp.y );
				if( !reversed )
				{
					along = V2d( 1, 0 );
				}
				else
				{
					along = V2d( -1, 0 );
				}
			}
			else
			{
				sprite->setPosition( pp.x, pp.y );
				along = normalize( ground->v1 - ground->v0 );
			}

			V2d gn( along.y, -along.x );

			bool fr = facingRight;
			if( reversed )
				fr = !fr;

			double xExtraStartRun = -48.0;//0.0;//5.0
			if( !fr )
				xExtraStartRun = -xExtraStartRun;

			//this seems pretty odd. need a thing for its first repetition
			//so i dont need to check the controller here
			if( frame == 0 && slowCounter == 1 && ( 
					( currInput.LLeft() && !prevInput.LLeft() ) 
				||  ( currInput.LRight() && !prevInput.LRight() ) )  )
			{
				switch (speedLevel)
				{
				case 0:
					ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart[0],
						pp + gn * 40.0 + along * xExtraStartRun, false, angle, 6, 3, fr);
					break;
				case 1:
					ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart[1],
						pp + gn * 40.0 + along * xExtraStartRun, false, angle, 6, 3, fr);
					break;
				case 2:
					ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart[2],
						pp + gn * 40.0 + along * xExtraStartRun, false, angle, 6, 3, fr);
					break;
				}
			}
		
			double xExtraStart = -48.0;
			if( !facingRight )
				xExtraStart = -xExtraStart;
			if( reversed )
				xExtraStart = -xExtraStart;

		
			if( frame == 3 * 4 && slowCounter == 1 )
			{
				ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_run,
					pp + gn * 48.0 + along * xExtraStart, false, angle, 8, 3, fr );
				ActivateSound( S_RUN_STEP1 );
			}
			else if( frame == 8 * 4 && slowCounter == 1 )
			{
				ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_run,
					pp + gn * 48.0 + along * xExtraStart, false, angle, 8, 3, fr );
				ActivateSound( S_RUN_STEP2 );
			}


			if( frame % 5 == 0 && abs( groundSpeed ) > 0 )
			{
				//ActivateEffect( ts_fx_bigRunRepeat, pp + gn * 56.0, false, angle, 24, 1, facingRight );
			}
			
			if( scorpOn )
			{
				scorpSprite.setTexture( *ts_scorpRun->texture );
				
				SetSpriteTile( &scorpSprite, ts_scorpRun, f/2, fr );
				
				scorpSprite.setOrigin( scorpSprite.getLocalBounds().width / 2,
					scorpSprite.getLocalBounds().height / 2 + 20 );
				scorpSprite.setPosition( position.x, position.y );
				scorpSprite.setRotation( sprite->getRotation() );
				scorpSet = true;
			}

			
			updateAura = false;
			break;
		}
	case SPRINT:
		{	
			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			int tFrame = frame / 2;
			SetSpriteTile( tFrame, r );
			
			assert( ground != NULL );
			
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d pp = ground->GetPoint( edgeQuantity );

			V2d along;
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
			{
				sprite->setPosition( pp.x + offsetX, pp.y );
				if( !reversed )
				{
					along = V2d( 1, 0 );
				}
				else
				{
					along = V2d( -1, 0 );
				}
			}
			else
			{
				sprite->setPosition( pp.x, pp.y );
				along = normalize( ground->v1 - ground->v0 );
			}

			V2d gn( along.y, -along.x );
			

			double xExtraStart = -48.0;
			if( !facingRight )
				xExtraStart = -xExtraStart;
			if( reversed )
				xExtraStart = -xExtraStart;

			if( frame == 2 * 4 && slowCounter == 1 )
			{
				ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_sprint,
					pp + gn * 48.0 + along * xExtraStart, false, angle, 10, 2, facingRight );
				ActivateSound( S_SPRINT_STEP1 );
			}
			else if( frame == 6 * 4 && slowCounter == 1 )
			{
				ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_sprint,
					pp + gn * 48.0 + along * xExtraStart, false, angle, 10, 2, facingRight );
				ActivateSound( S_SPRINT_STEP2 );
			}

			if( scorpOn )
			{
				scorpSprite.setTexture( *ts_scorpSprint->texture );
				
				SetSpriteTile( &scorpSprite, ts_scorpSprint, tFrame/2, r );
				
				scorpSprite.setOrigin( scorpSprite.getLocalBounds().width / 2,
					scorpSprite.getLocalBounds().height / 2 + 20 );
				scorpSprite.setPosition( position.x, position.y );
				scorpSprite.setRotation( sprite->getRotation() );
				scorpSet = true;
			}

			break;
		}
	case SEQ_CRAWLERFIGHT_STRAIGHTFALL:
	case SEQ_CRAWLERFIGHT_DODGEBACK:
	case SEQ_KINFALL:
	case JUMP:
		{
			sf::IntRect ir;
			int tFrame = GetJumpFrame();

			SetSpriteTexture(JUMP);
			
			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( tFrame, r );		

			if( frame > 0 )
			{
				sprite->setRotation( 0 );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );


			if (scorpOn)
				SetAerialScorpSprite();
			break;
		}
	case JUMPSQUAT:
		{
			SetSpriteTexture( action );
			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 0, r );		

			double angle = GroundedAngle();

			//some special stuff for jumpsquat
			if( reversed )
			{
				if( -gn.y > -steepThresh )
				{
					angle = PI;
				}
			}
			else
			{
				if( gn.y > -steepThresh )
				{
					angle = 0;
				}
			}



			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			

			V2d pp;
			if (ground != NULL)
				pp = ground->GetPoint(edgeQuantity);
			else if (grindEdge != NULL)
				pp = grindEdge->GetPoint(grindQuantity);
			else
			{
				assert(0);
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			
			break;
		}
	case SEQ_CRAWLERFIGHT_LAND:
	case LAND: 
		{
			SetSpriteTexture( LAND );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 0, r );


			double angle = GroundedAngle();
		

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
		
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			if( frame == 0 && slowCounter == 1 )
			{
				V2d fxPos;
				if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				{
					fxPos = V2d( pp.x + offsetX, pp.y );
					fxPos += V2d( 0, -1 ) * 48.0;
				}
				else
				{
					fxPos = pp;
					fxPos += gn * 48.0;
				}

				//cout << "activating" << endl;
				switch (speedLevel)
				{
				case 0:
					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_land[0], fxPos, false, angle, 8, 2, facingRight);
					break;
				case 1:
					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_land[1], fxPos, false, angle, 8, 2, facingRight);
					break;
				case 2:
					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_land[2], fxPos, false, angle, 9, 2, facingRight);
					break;
				}
				
			}
			break;
		}
	case LAND2: 
		{
			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 1, r );
		
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
		
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			break;
		}
	case WALLCLING:
		{
			SetSpriteTexture( action );
			
			SetSpriteTile( 0, facingRight );
			
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case WALLJUMP:
		{
			if( frame == 0 && slowCounter == 1 )
			{
				

				
				
				//cout << "ACTIVATING WALLJUMP" << endl;
			}

			SetSpriteTexture( action );
			
			SetSpriteTile( frame / 2, facingRight );
			
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED:
	case SLIDE:
		{
			SetSpriteTexture( SLIDE );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 0, r );
		
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
		
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
					sprite->setPosition( pp.x + offsetX, pp.y );
				else
					sprite->setPosition( pp.x, pp.y );

			if( scorpOn )
			{
				scorpSprite.setTexture( *ts_scorpSlide->texture );
				
				SetSpriteTile( &scorpSprite, ts_scorpSlide, 0, r );
				
				scorpSprite.setOrigin( scorpSprite.getLocalBounds().width / 2,
					scorpSprite.getLocalBounds().height / 2 + 15 );
				scorpSprite.setPosition( position.x, position.y );
				scorpSprite.setRotation( sprite->getRotation() );
				scorpSet = true;
			}
			break;
		}
	case STANDN:
		{
			int startFrame = 0;
			showSword = true;

			Tileset *curr_ts = ts_standingNSword[speedLevel];

			if( showSword )
			{
				standingNSword.setTexture( *curr_ts->texture );
			}
			//Vector2i offset( 24, -16 );
			//Vector2i offset( 24, 0 );
			//Vector2i offset( 32, 0 );
			//Vector2i offset( 0, -16 );
			Vector2f offset = standSwordOffset[speedLevel];

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( frame / 2, r );

			if( showSword )
			{
				if( r )
				{
					standingNSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
				}
				else
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / 2 - startFrame );
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

			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			//V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y ) + V2d( offset.x * cos( angle ) + offset.y * sin( angle ), 
			//offset.x * -sin( angle ) +  offset.y * cos( angle ) );
			V2d pos = V2d( sprite->getPosition().x, sprite->getPosition().y );
			V2d truDir( -trueNormal.y, trueNormal.x );//normalize( ground->v1 - ground->v0 );

			pos += truDir * (double)offset.x;
			pos += -trueNormal * (double)offset.y;
			

			standingNSword.setPosition( pos.x, pos.y );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = standingNSword;
			}

			break;
		}
	case STEEPCLIMBATTACK:
		{
			int startFrame = 0;
			showSword = true;//frame / 2 >= startFrame && frame / 2 <= 7;
			Tileset *curr_ts = ts_steepClimbAttackSword[speedLevel];
			int animFactor = 2;

			if( showSword )
			{
				steepClimbAttackSword.setTexture( *curr_ts->texture );
			}

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			int tFrame = frame / animFactor;
			SetSpriteTile( tFrame, r );

			Vector2f offset = climbAttackOffset[speedLevel];

			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword )
			{
				if( r )
				{
					steepClimbAttackSword.setTextureRect( curr_ts->GetSubRect( frame / animFactor - startFrame ) );
				}
				else
				{	
					sf::IntRect irSword = curr_ts->GetSubRect( frame / animFactor - startFrame );
					steepClimbAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				
				}

				steepClimbAttackSword.setTexture( *curr_ts->texture );
				steepClimbAttackSword.setOrigin( steepClimbAttackSword.getLocalBounds().width / 2, steepClimbAttackSword.getLocalBounds().height);
				steepClimbAttackSword.setRotation( angle / PI * 180 );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			V2d pos = V2d( sprite->getPosition().x, sprite->getPosition().y );
			V2d truDir( -trueNormal.y, trueNormal.x );//normalize( ground->v1 - ground->v0 );

			pos += trueNormal * (double)offset.y;
			pos += truDir * (double)offset.x;

			steepClimbAttackSword.setPosition( pos.x, pos.y );

			if (scorpOn)
			{
				scorpSprite.setTexture(*ts_scorpClimb->texture);

				SetSpriteTile(&scorpSprite, ts_scorpClimb, 0, r);

				scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
					scorpSprite.getLocalBounds().height / 2);
				scorpSprite.setPosition(position.x, position.y);
				scorpSprite.setRotation(sprite->getRotation());
				scorpSet = true;
			}

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

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( frame / 2, r );
			

			Vector2f offset = slideAttackOffset[speedLevel];

			V2d trueNormal;
			double angle = GroundedAngleAttack( trueNormal );

			if( showSword )
			{
				if( r )
				{
					steepSlideAttackSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
				}
				else
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / 3 - startFrame );
					steepSlideAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;	
				}

				steepSlideAttackSword.setTexture( *curr_ts->texture );
				steepSlideAttackSword.setOrigin( steepSlideAttackSword.getLocalBounds().width / 2, steepSlideAttackSword.getLocalBounds().height);
				steepSlideAttackSword.setRotation( angle / PI * 180 );
			}
			

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );

			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			V2d pos = V2d( sprite->getPosition().x, sprite->getPosition().y );
			V2d truDir( -trueNormal.y, trueNormal.x );//normalize( ground->v1 - ground->v0 );

			pos += trueNormal * (double)offset.y;
			pos += truDir * (double)offset.x;

			steepSlideAttackSword.setPosition( pos.x, pos.y );

			if (scorpOn)
			{
				scorpSprite.setTexture(*ts_scorpSteepSlide->texture);

				SetSpriteTile(&scorpSprite, ts_scorpSteepSlide, 0, r);

				if (r)
				{
					scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 20,
						scorpSprite.getLocalBounds().height / 2 + 20);
				}
				else
				{
					scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 20,
						scorpSprite.getLocalBounds().height / 2 + 20);
				}

				scorpSprite.setPosition(position.x, position.y);
				scorpSprite.setRotation(sprite->getRotation());
				scorpSet = true;
			}

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

			SetSpriteTexture( action );

			SetSpriteTile( frame / 2, facingRight );

			//Vector2i offset( 32, -16 );
			Vector2i offset( -8, -8 );

			if( showSword )
			{
				if( facingRight )
				{
					wallAttackSword.setTextureRect( curr_ts->GetSubRect( frame / 2 - startFrame ) );
				}
				else
				{
					offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame / 2 - startFrame );
					//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
					wallAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );	
				}

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

			//Vector2i offset( 32, -16 );
			Vector2i offset( 0, 0 );

			SetSpriteTexture( action );

			SetSpriteTile( frame, facingRight );

			if( showSword )
			{
				if( facingRight )
				{
					fairSword.setTextureRect( curr_ts->GetSubRect( frame - startFrame ) );
				}
				else
				{
					offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame - startFrame );
					//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
					fairSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );	
				}

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
			
			if (scorpOn)
				SetAerialScorpSprite();

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
			offsetArr[0] = Vector2i( 0, 0 );
			offsetArr[1] = Vector2i(0, 0);//Vector2i( 0, 48 );
			offsetArr[2] = Vector2i( 0, 0 );

			Vector2i offset = offsetArr[speedLevel];

			SetSpriteTexture( action );

			SetSpriteTile( frame, facingRight );

			if( showSword )
			{
				if( facingRight )
				{
					dairSword.setTextureRect( curr_ts->GetSubRect( frame - startFrame ) );
				}
				else
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame - startFrame );
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

			if (scorpOn)
				SetAerialScorpSprite();
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

			SetSpriteTexture( action );

			SetSpriteTile( frame, facingRight );
			

			Vector2i offset( 0, 0 );
			//Vector2i offset( 8, -24 );

			if( showSword )
			{
				if( facingRight )
				{
					uairSword.setTextureRect( curr_ts->GetSubRect( frame - startFrame ) );
				}
				else
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame - startFrame );
					uairSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );

					offset.x = -offset.x;
				}

				uairSword.setOrigin( uairSword.getLocalBounds().width / 2, uairSword.getLocalBounds().height / 2 );
				uairSword.setPosition( position.x + offset.x, position.y + offset.y );
				uairSword.setRotation( 0 );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if (scorpOn)
				SetAerialScorpSprite();
			break;
		}
	case DIAGUPATTACK:
		{
			Tileset *curr_ts = ts_diagUpSword[speedLevel];
			//cout << "fair frame : " << frame / 2 << endl;
			int startFrame = 0;
			showSword = frame/2 < 11;//frame >= startFrame && frame / 2 <= 9;

			if( showSword )
			{
				diagUpAttackSword.setTexture( *curr_ts->texture );
			}


			SetSpriteTexture( action );

			SetSpriteTile( frame/2, facingRight );

			//Vector2i offset( 32, -16 );
			//Vector2i offset( 0, 0 );

			if( showSword )
			{
				//Vector2i offsets[3];//( 0, 0 );
				//offsets[0] = Vector2i( 40, -32 );
				//offsets[1] = Vector2i( 16, -40 );
				//offsets[2] = Vector2i( 32, -48 );

				Vector2f offset = diagUpSwordOffset[speedLevel];

				if( facingRight )
				{
					diagUpAttackSword.setTextureRect( curr_ts->GetSubRect( frame /2 - startFrame ) );
				}
				else
				{
					offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame/2 - startFrame );
					diagUpAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );	
				}

				diagUpAttackSword.setOrigin( diagUpAttackSword.getLocalBounds().width / 2, diagUpAttackSword.getLocalBounds().height / 2 );
				diagUpAttackSword.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = diagUpAttackSword;
			}
			
			if (scorpOn)
				SetAerialScorpSprite();

			break;
		}
	case DIAGDOWNATTACK:
		{
			Tileset *curr_ts = ts_diagDownSword[speedLevel];
			//cout << "fair frame : " << frame / 2 << endl;
			int startFrame = 0;
			showSword = frame/2 < 11;//frame >= startFrame && frame / 2 <= 9;

			if( showSword )
			{
				diagDownAttackSword.setTexture( *curr_ts->texture );
			}

			
			SetSpriteTexture( action );

			SetSpriteTile( frame / 2, facingRight );

			//Vector2i offset( 32, -16 );
			

			if( showSword )
			{

				//Vector2i offsets[3];//( 0, 0 );
				//offsets[0] = Vector2i( 32, 24 );
				//offsets[1] = Vector2i( 16, 32 );
				//offsets[2] = Vector2i( 16, 64 );

				Vector2f offset = diagDownSwordOffset[speedLevel];

				if( facingRight )
				{
					diagDownAttackSword.setTextureRect( curr_ts->GetSubRect( frame/2 - startFrame ) );
				}
				else
				{
				
					offset.x = -offset.x;

					sf::IntRect irSword = curr_ts->GetSubRect( frame/2 - startFrame );
					//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
					diagDownAttackSword.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );	
				}

				diagDownAttackSword.setOrigin( diagDownAttackSword.getLocalBounds().width / 2, diagDownAttackSword.getLocalBounds().height / 2 );
				diagDownAttackSword.setPosition( position.x + offset.x, position.y + offset.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if( record > 0 )
			{
				PlayerGhost::P & p = ghosts[record-1]->states[ghosts[record-1]->currFrame];
				p.showSword = showSword;
				p.swordSprite1 = diagDownAttackSword;
			}
			
			if (scorpOn)
				SetAerialScorpSprite();

			break;
		}
	case BACKWARDSDOUBLE:
		{
			int fr = frame;

			SetSpriteTexture( action );
			SetSpriteTile( fr / 1, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if (scorpOn)
				SetAerialScorpSprite();
			break;
		}
	case DOUBLE:
		{
	
			int fr = frame;
			if ( frame > 27 )
			{
				fr = 27;
			}

			SetSpriteTexture( action );

			SetSpriteTile( fr / 1, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			if (scorpOn)
				SetAerialScorpSprite();
			break;
		}
	case SPRINGSTUN:
	{
		SetSpriteTexture(action);

		SetSpriteTile(0, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		
		V2d sVel = springVel + springExtra;
		if (facingRight)
		{
			double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
			sprite->setRotation(a);
		}
		else
		{
			double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
			sprite->setRotation(-a + 180);
		}

		if (scorpOn)
			SetAerialScorpSprite();
		break;
	}
	case SPRINGSTUNBOUNCE:
	case SPRINGSTUNGLIDE:
	case SPRINGSTUNAIRBOUNCE:
	case SPRINGSTUNTELEPORT:
	{
		SetSpriteTexture(action);

		SetSpriteTile(0, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);

		V2d sVel = springVel + springExtra;
		if (facingRight)
		{
			double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
			sprite->setRotation(a);
		}
		else
		{
			double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
			sprite->setRotation(-a + 180);
		}

		if (scorpOn)
			SetAerialScorpSprite();
		break;
	}
	case SWINGSTUN:
	{
		SetSpriteTexture(action);

		SetSpriteTile(0, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		break;
	}
	case GLIDE:
	{
		SetSpriteTexture(action);

		SetSpriteTile(0, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);

		V2d sVel = velocity;//springVel + springExtra;
		if (facingRight)
		{
			double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
			sprite->setRotation(a);
		}
		else
		{
			double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
			sprite->setRotation(-a + 180);
		}

		if (scorpOn)
			SetAerialScorpSprite();
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

			SetSpriteTexture( action );

			

			//3-8 is the cycle
			sf::IntRect ir;
			int checkFrame = -1;

			if( frame / 2 < 1 )
			{
				checkFrame = frame / 2;
			}
			else if( frame < actionLength[DASH] - 1 )
			{
				checkFrame = 1 + ( (frame/2 - 1) % 5 );
			}
			else
			{
				checkFrame = 6;
			}

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( checkFrame, r );

			
			double angle = GroundedAngle();
			

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			
			V2d pp = ground->GetPoint( edgeQuantity );

			V2d along = normalize( ground->v1 - ground->v0 );
			double xExtraRepeat = 64;
			double xExtraStart = 80;

			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				xExtraRepeat = -xExtraRepeat;
				xExtraStart = -xExtraStart;
			}

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				pp.x += offsetX;

			sprite->setPosition( pp.x, pp.y );

			bool fr = facingRight;
			if( reversed )
				fr = !fr;
			if( frame == 0 && currInput.B && !prevInput.B )
			{
				
				ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashStart, 
					pp + gn * 64.0 + along * xExtraStart , false, angle, 9, 3, fr );
				ActivateSound( S_DASH_START );
			}
			else if( frame % 5 == 0 )
			{
				ActivateEffect( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashRepeat, 
					pp + gn * 32.0 + along * xExtraRepeat, false, angle, 12, 3, fr );
			}

			if( scorpOn )
			{
				scorpSprite.setTexture( *ts_scorpDash->texture );
				
				SetSpriteTile( &scorpSprite, ts_scorpDash, checkFrame, r );
				
				scorpSprite.setOrigin( scorpSprite.getLocalBounds().width / 2,
					scorpSprite.getLocalBounds().height / 2 + 10 );
				scorpSprite.setPosition( position.x, position.y );
				scorpSprite.setRotation( sprite->getRotation() );
				scorpSet = true;
			}

			break;
		}
	case GRINDATTACK:
		{
		}
	case RAILSLIDE:
	case RAILGRIND:
	case GRINDBALL:
		{
			assert( grindEdge != NULL );
			
			SetSpriteTexture( GRINDBALL );

			V2d grindNorm = grindEdge->Normal();
			bool r = grindSpeed > 0;

			if ((action == RAILGRIND || action == RAILSLIDE ) && grindNorm.y > 0)
			{
				grindNorm = -grindNorm;
				r = !r;
			}

			SetSpriteTile( 0, r );
			
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

			gsdodeca.setTextureRect( tsgsdodeca->GetSubRect( (grindActionInt * 5) % grindActionLength   ) );
			gstriblue.setTextureRect( tsgstriblue->GetSubRect( (grindActionInt * 5) % grindActionLength ) );
			gstricym.setTextureRect( tsgstricym->GetSubRect( grindActionInt % grindActionLength ) ); //broken?
			gstrigreen.setTextureRect( tsgstrigreen->GetSubRect( (grindActionInt * 5) % grindActionLength ) );
			gstrioran.setTextureRect( tsgstrioran->GetSubRect( grindActionInt% grindActionLength ));
			gstripurp.setTextureRect( tsgstripurp->GetSubRect( grindActionInt% grindActionLength ) );
			gstrirgb.setTextureRect( tsgstrirgb->GetSubRect( grindActionInt% grindActionLength ) );

			

			double angle = 0;
			angle = atan2( grindNorm.x, -grindNorm.y );
			if( !approxEquals( abs(offsetX), b.rw ) )
			{

			}
			else
			{
				//angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
				//angle = asin( dot( grindNorm, V2d( 1, 0 ) ) ); 
				
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
			sprite->setRotation( angle / PI * 180 );
		//	sprite->setRotation( 0 );
			
			V2d oldv0 = grindEdge->v0;
			V2d oldv1 = grindEdge->v1;

			V2d pp = grindEdge->GetPoint( grindQuantity );
			
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
	case RAILDASH:
	{
		//double angle = 0;
		//angle = atan2(grindNorm.x, -grindNorm.y);
		//if (!approxEquals(abs(offsetX), b.rw))
		//{

		//}
		//else
		//{
		//	//angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
		//	//angle = asin( dot( grindNorm, V2d( 1, 0 ) ) ); 

		//}
		//sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		//sprite->setRotation(angle / PI * 180);

		sprite->setPosition(position.x, position.y);



		break;
	}
	case GRINDLUNGE:
		{
			SetSpriteTexture( GRINDLUNGE );

			SetSpriteTile( 1, facingRight );

			

			double angle = atan2( lungeNormal.x, -lungeNormal.y );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
			sprite->setRotation( angle / PI * 180 );
			sprite->setPosition( position.x, position.y );
			//float angle = atan2( 

			if (scorpOn)
				SetAerialScorpSprite();
			break;
		}
	case GRINDSLASH:
		{
			//sprite->setTexture( *(tileset[GRINDLUNGE]->texture) );

			//IntRect ir = tileset[GRINDLUNGE]->GetSubRect( 1 );

			Tileset *curr_ts = ts_grindLungeSword[speedLevel];
			int startFrame = 0;
			showSword = frame < 15;
			//showSword = frame / 3 >= startFrame && frame / 3 <= 5;

			if( showSword )
			{
				grindLungeSword.setTexture( *curr_ts->texture );
			}

			SetSpriteTexture( GRINDSLASH );

			SetSpriteTile( frame, facingRight );

			Vector2i offset( 0, 0 );
			switch (speedLevel)
			{
			case 0:
			case 1:
				offset = Vector2i(16, 24);
				break;
			case 2:
				offset = Vector2i(32, 32);
				break;
			}
			//Vector2i offset( 8, -24 );

			if( showSword )
			{
				if( facingRight )
				{
					grindLungeSword.setTextureRect( curr_ts->GetSubRect( frame / 3 - startFrame ) );
				}
				else
				{
					sf::IntRect irSword = curr_ts->GetSubRect( frame / 3 - startFrame );
					grindLungeSword.setTextureRect( sf::IntRect( irSword.left + irSword.width,
						irSword.top, -irSword.width, irSword.height ) );
				

					offset.x = -offset.x;
				}

				grindLungeSword.setOrigin(grindLungeSword.getLocalBounds().width / 2, grindLungeSword.getLocalBounds().height / 2 );
				grindLungeSword.setPosition( position.x + offset.x, position.y + offset.y );
				grindLungeSword.setRotation( sprite->getRotation() );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			//sprite->setRotation( 0 );
			if (scorpOn)
				SetAerialScorpSprite();
			//sprite->setPosition( position.x, position.y );
		}
		break;
	case STEEPSLIDE:
		{

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 0, r );

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
			
			V2d pp = ground->GetPoint( edgeQuantity );

			sprite->setPosition( pp.x, pp.y );
			//if( angle == 0 )
			//	sprite->setPosition( pp.x + offsetX, pp.y );
			//else
			//	sprite->setPosition( pp.x, pp.y );

			if (scorpOn)
			{
				scorpSprite.setTexture(*ts_scorpSteepSlide->texture);

				SetSpriteTile(&scorpSprite, ts_scorpSteepSlide, 0, r);

				if (r)
				{
					scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 20,
						scorpSprite.getLocalBounds().height / 2 + 20);
				}
				else
				{
					scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 20,
						scorpSprite.getLocalBounds().height / 2 + 20);
				}
				
				scorpSprite.setPosition(position.x, position.y);
				scorpSprite.setRotation(sprite->getRotation());
				scorpSet = true;
			}
			break;
		}
	case AIRDASH:
		{
			SetSpriteTexture( action );

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

			SetSpriteTile( f, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			//if (scorpOn)
			//	SetAerialScorpSprite();
			break;
		}
	case GRAVREVERSE:
		{
			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 0, r );

			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );

			V2d pp = ground->GetPoint(edgeQuantity);

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			break;
		}
	case STEEPCLIMB:
		{

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			int tFrame = frame / 4;
			SetSpriteTile( tFrame, r );

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
			
			V2d pp = ground->GetPoint( edgeQuantity );

			sprite->setPosition( pp.x, pp.y );


			if (frame == 0 * 4 && slowCounter == 1)
			{
				ActivateSound(S_CLIMB_STEP1);
			}
			else if (frame == 4 * 4 && slowCounter == 1)
			{
				ActivateSound(S_CLIMB_STEP1);
			}

			if (scorpOn)
			{
				scorpSprite.setTexture(*ts_scorpClimb->texture);

				SetSpriteTile(&scorpSprite, ts_scorpClimb, tFrame, r);

				if (r)
				{
					scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 30,
						scorpSprite.getLocalBounds().height / 2 + 25);
				}
				else
				{
					scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 30,
						scorpSprite.getLocalBounds().height / 2 + 25);
				}
			
				scorpSprite.setPosition(position.x, position.y);
				scorpSprite.setRotation(sprite->getRotation());
				scorpSet = true;
			}
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

			SetSpriteTexture( action );

			SetSpriteTile( 0, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case GROUNDHITSTUN:
		{

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 1, r );

			
			double angle = GroundedAngle();

		

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

			sprite->setRotation( angle / PI * 180 );
			
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
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
				int xThresh = 10;
				int yThresh = 10;
				if( velocity.y > yThresh )
				{
					if( abs( velocity.x ) < xThresh )//10 just for testing
					{
						bounceFrame = 5;
					}
					else
					{
						bounceFrame = 8;
					}
				}
				else if( velocity.y < -yThresh )
				{
					if( abs( velocity.x ) < xThresh )//10 just for testing
					{
						bounceFrame = 1;
					}
					else
					{
						bounceFrame = 7;
					}
				}
				else
				{
					if( abs( velocity.x ) > xThresh )
					{
						bounceFrame = 3;
					}
					else
					{
						bounceFrame = 6;
					}
				}



				V2d bn = oldBounceNorm;//oldBounceEdge->Normal();
			//	if( bn.y <= 0 && bn.y > -steepThresh )
			//	{
			//		bounceFrame = 7;
			//		if( facingRight )
			//		{
			//			//facingRight = false;

			}
			else
			{
				bounceFrame = 6;
			}
			//	}
			//	else if( bn.y >= 0 && -bn.y > -steepThresh )
			//	{
			//		
			//		bounceFrame = 8;
			//	}
			//	else if( bn.y == 0 )
			//	{
			//		bounceFrame = 3;
			//	}
			//	else if( bn.y < 0 )
			//	{
			//		bounceFrame = 1;
			//	}
			//	else if( bn.y > 0 )
			//	{
			//		bounceFrame = 5;
			//	}
			//}
			//else
			//{
			//	bounceFrame = 6;
			//}
			
			SetSpriteTexture( action );

			SetSpriteTile( bounceFrame, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );

			scorpSet = true;
			break;
		}
	case BOUNCEGROUND:
		{
			int bounceFrame = 0;
			V2d bn = bounceNorm;//bounceEdge->Normal();

			bool bounceFacingRight = facingRight;

			if( bn.y <= 0 && bn.y > -steepThresh )
			{
				//if( storedBounceVel.y > 0 ) //falling
				//{

				//}
				//else
				//{

				//}
				//if( (bn.x > 0 && storedBounceVel.x >= 0) || (bn.x < 0 && storedBounceVel.x <= 0 ) )
				//	bounceFrame = 0;
				//else
				//	bounceFrame = 2;

				bounceFrame = 2;

				bounceFacingRight = (bn.x > 0 );
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				/*if( (bn.x > 0 && storedBounceVel.x >= 0) || (bn.x < 0 && storedBounceVel.x <= 0 ) )
					bounceFrame = 4;
				else
					bounceFrame = 2;*/
				bounceFrame = 2;
				bounceFacingRight = (bn.x > 0 );
				//facingRight = !facingRight;
			}
			else if( bn.y == 0 )
			{
				bounceFrame = 2;
			//	facingRight = !facingRight;
			}
			else if( bn.y < 0 )
			{
				bounceFrame = 0;//8
			}
			else if( bn.y > 0 )
			{
				bounceFrame = 4;
			}


			SetSpriteTexture( action );

			//bool r = (bounceFacingRight && !reversed ) || (!bounceFacingRight && reversed );
			SetSpriteTile( bounceFrame, bounceFacingRight );

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
					sprite->setOrigin( 110, sprite->getLocalBounds().height / 2);
				}
				else
				{
					sprite->setOrigin( sprite->getLocalBounds().width-110, sprite->getLocalBounds().height / 2);
				}
			}
			else if( bn.y <= 0 && bn.y > -steepThresh )
			{
				if( bounceFacingRight )
				{
					sprite->setOrigin( 110, sprite->getLocalBounds().height / 2);
					//sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
				}
				else
				{
					//sprite->setOrigin( , sprite->getLocalBounds().height / 2);
					sprite->setOrigin( sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
				}
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				if( bounceFacingRight )//bounceFrame == 4 )
				{
					sprite->setOrigin( 110, sprite->getLocalBounds().height / 2);
					//sprite->setOrigin( sprite->getLocalBounds().width / 2, 0);
					//sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
				}
				else
				{
					sprite->setOrigin( sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
					//sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
				}	
			}
			else if( bn.y < 0 )
			{
				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - 120);
			}
			else if( bn.y > 0 )
			{
				//cout << "this one" << endl;
				sprite->setOrigin( sprite->getLocalBounds().width / 2, 80 );
			}

			
			sprite->setPosition( position.x, position.y );	

			scorpSet = true;
			break;
		}
	case BOUNCEGROUNDEDWALL:
		{
			SetSpriteTexture( action );

			int tFrame = -1;
			if( frame < 6 )
			{
				tFrame = 0;
			}
			else if( frame < 20 )
			{
				tFrame = 1;
			}
			else
			{
				tFrame = 2;
			}

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( tFrame, r );

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

			int yOffset = -75;
			if( frame < 6 )
			{
				if( ( facingRight && !reversed ) || (!facingRight && reversed ) )
				{
					sprite->setOrigin( sprite->getLocalBounds().width / 2 - 3, sprite->getLocalBounds().height + yOffset);
				}
				else
				{
					sprite->setOrigin( sprite->getLocalBounds().width / 2 + 3, sprite->getLocalBounds().height + yOffset);
				}
				
				angle = 0;
				if( reversed )
					angle = PI;
			}
			else
			{
				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height + yOffset);
			}
			//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
			
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

			scorpSet = true;

			break;
		}
	case INTRO:
		{
			if (frame == 0 && slowCounter == 1)
			{
				ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/enter_fx_320x320.png", 320, 320), position, false, 0, 22, 2, true);
			}
			else if (frame/2 >= 5)
			{
				SetSpriteTexture(action);
				SetSpriteTile((frame - 5) / 2, facingRight);

				sprite->setOrigin(sprite->getLocalBounds().width / 2,
					sprite->getLocalBounds().height / 2);
				sprite->setPosition(position.x, position.y);
				sprite->setRotation(0);
			}
			break;
		}
	case INTROBOOST:
	{
		if (frame == 0 && slowCounter == 1)
		{
			ActivateEffect(EffectLayer::IN_FRONT, ts_exitAura, position, false, 0, 8, 2, true, 55);
			ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/enter_fx_320x320.png", 320, 320), position, false, 0, 19, 2, true);
			//owner->cam.SetManual(true);
		}
		else if (frame == 20)
		{
			//owner->cam.SetManual(false);
		}
		SetSpriteTexture(action);
		SetSpriteTile((frame/2 )+110, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		sprite->setRotation(0);
		break;
	}
	case EXIT:
		{
			SetSpriteTexture( action );

			SetSpriteTile( frame / 2, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );//position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case EXITBOOST:
	{
		SetSpriteTexture(action);

		SetSpriteTile(frame / 2, facingRight);
		//cout << "f: " << frame / 2 << endl;
		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);//position.x, position.y );
		sprite->setRotation(0);

		int aF = frame / 2 - 55;
		if (aF < 61 && aF >= 0)
		{
			showExitAura = true;
			exitAuraSprite.setTextureRect(ts_exitAura->GetSubRect(aF));
			exitAuraSprite.setOrigin(exitAuraSprite.getLocalBounds().width / 2,
				exitAuraSprite.getLocalBounds().height / 2);
			exitAuraSprite.setPosition(sprite->getPosition());
		}

		if (frame == 59 * 2)
		{
			owner->mainMenu->ActivateIndEffect(
				owner->mainMenu->tilesetManager.GetTileset("Kin/exitenergy_0_512x512.png", 512, 512), V2d( 960, 540 ), false, 0, 4, 2, true,2);
		}
		else if (frame == 63 * 2)
		{
			owner->mainMenu->ActivateIndEffect(
				owner->mainMenu->tilesetManager.GetTileset("Kin/exitenergy_1_512x512.png", 512, 512), V2d(960, 540), false, 0, 6, 2, true);
		}
		/*else if (frame == 6 * 4 + 55 * 2)
		{
			owner->mainMenu->ActivateIndEffect(EffectLayer::IN_FRONT,
				GetTileset("Kin/exitenergy_2_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
		}*/
		break;
	}
	case NEXUSKILL:
	case GOALKILL:
		{
			assert(slowCounter == 1);
			if (slowCounter == 1)
			{
				if (frame == 20)
				{
					ActivateSound(S_GOALKILLSLASH1);
				}
				else if (frame == 36)
				{
					ActivateSound(S_GOALKILLSLASH2);
				}
				else if (frame == 60)
				{
					ActivateSound(S_GOALKILLSLASH3);
				}
				else if (frame == 84)
				{
					ActivateSound(S_GOALKILLSLASH4);
				}
			}


			int tsIndex = (frame / 2) / 16;
			switch (tsIndex)
			{
			case 0:
				SetSpriteTexture(GOALKILL);
				break;
			case 1:
				SetSpriteTexture(GOALKILL1);
				break;
			case 2:
				SetSpriteTexture(GOALKILL2);
				break;
			case 3:
				SetSpriteTexture(GOALKILL3);
				break;
			case 4:
				SetSpriteTexture(GOALKILL4);
				break;
			default:
				assert(0);
				break;
			}
			//radius is 24. 100 pixel offset

			int realFrame = (frame / 2 ) % 16;
			//cout << "goalkill index: " << tsIndex << ", realFrame: " << realFrame << ", frame: " << frame << endl;
			
			SetSpriteTile(realFrame, facingRight);
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 + 24 );

			if (action == GOALKILL) //move this laster ewww
			{
				CubicBezier cb(.61, .3, .4, 1);//0, 0, 1, 1 );
				

				//cb.GetValue()
				
				V2d start = owner->goalNodePos;
				
				V2d end = owner->goalNodePosFinal;
				int st = 48 * 2;
				if (frame >= st)
				{
					double a = (frame - st) / (double)(actionLength[GOALKILL] - (st+1));
					double t = cb.GetValue(a);
					V2d newPos = start + (end - start) * t;
					position = newPos;
				}

				//72 * 2
				
				/*float fff = 78.f / (actionLength[GOALKILL] - st);
				if (frame >= st)
				{
					position.y -= fff;
				}*/
			}
			
			//sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );//- 24.f );
			sprite->setPosition(Vector2f(position));
			sprite->setRotation( 0 );
			break;
		}
	case GOALKILLWAIT:
		{
			SetSpriteTexture(GOALKILL4);
			SetSpriteTile(7, facingRight);
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 + 24);
			sprite->setPosition(Vector2f(position));
			//sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );
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
			int tFrame = ( frame - 90 ) / 5;
			
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

			SetSpriteTexture( action );
			SetSpriteTile( tFrame, facingRight );

			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
		}
		break;
	case SKYDIVE:
		{
			SetSpriteTexture( action );

			SetSpriteTile( 0, facingRight );
		
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
		break;
		}
	case SKYDIVETOFALL:
		{
			SetSpriteTexture( action );

			SetSpriteTile( 0, facingRight );
		
			sprite->setOrigin( sprite->getLocalBounds().width / 2,
				sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case WAITFORSHIP:
		{

			SetSpriteTexture( action );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( 0, r );

			if( ground != NULL )
			{
				double angle = GroundedAngle();

				sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

				V2d pp = ground->GetPoint( edgeQuantity );
			
				if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
					sprite->setPosition( pp.x + offsetX, pp.y );
				else
					sprite->setPosition( pp.x, pp.y );
				sprite->setRotation( angle / PI * 180 );


				//cout << "angle: " << angle / PI * 180  << endl;
			}
		}
		break;
	case GRABSHIP:
		{
			//cout << "grabship: " << frame << endl;
			if( frame / 5 < 8 )
			{
				SetSpriteTexture( action );

				//bool r = (facingRight && !reversed ) || (!facingRight && reversed );
				SetSpriteTile( 1 + frame / 5, true );
			
				sprite->setOrigin( sprite->getLocalBounds().width / 2,
					sprite->getLocalBounds().height / 2 );
			}
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
		}
		break;
	case GETPOWER_AIRDASH_MEDITATE:
		{

			SetSpriteTexture( action );

			int f = min( frame / 3, 2 );

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( f, r );

			assert( ground != NULL );
			
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			sprite->setRotation( angle / PI * 180 );
			
			break;
		}
		
	case GETPOWER_AIRDASH_FLIP:
		{
			SetSpriteTexture( action );

			//int f = min( frame / 2, 11 );
			int f = frame / 2;

			bool r = (facingRight && !reversed ) || (!facingRight && reversed );
			SetSpriteTile( f, r );
			
			assert( ground != NULL );
		
			
			double angle = GroundedAngle();

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - 16);

			V2d pp = ground->GetPoint( edgeQuantity );
			
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			sprite->setRotation( angle / PI * 180 );
			
			break;
		}
	case ENTERNEXUS1:
	{
		SetSpriteTexture(action);

		SetSpriteTile(frame / 4, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		sprite->setRotation(0);
		break;
	}
	case SEQ_WAIT:
		{
		sprite->setTexture( *(tileset[JUMP]->texture));
		{
		sf::IntRect ir;

		ir = tileset[JUMP]->GetSubRect( 3 );
		

		sprite->setRotation( 0 );

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
	case SEQ_LOOKUP:
	{
		SetSpriteTexture(action);

		bool r = (facingRight && !reversed) || (!facingRight && reversed);
		SetSpriteTile(0, r);


		double angle = GroundedAngle();



		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

		sprite->setRotation(angle / PI * 180);

		V2d pp = ground->GetPoint(edgeQuantity);

		if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
			sprite->setPosition(pp.x + offsetX, pp.y);
		else
			sprite->setPosition(pp.x, pp.y);
		break;
	}
	case SEQ_KINTHROWN:
	{
		SetSpriteTexture(action);

		SetSpriteTile(0, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		sprite->setRotation(0);
		break;
	}
	case SEQ_KNEEL:
	{
		SetSpriteTexture(action);
		bool r = (facingRight && !reversed) || (!facingRight && reversed);
		SetSpriteTile(0, r);

		double angle = 0;//GroundedAngle()


		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height-16);
		sprite->setRotation(angle / PI * 180);

		V2d pp;
		if (ground != NULL)
			pp = ground->GetPoint(edgeQuantity);
		else
		{
			assert(0);
		}

		sprite->setPosition(pp.x, pp.y);	

		//dirtyAuraSprite.setTextureRect(ts_dirtyAura->GetSubRect((owner->totalGameFrames % (15 * 3) / 3)));
		//dirtyAuraSprite.setOrigin(dirtyAuraSprite.getLocalBounds().width / 2, 
		//	dirtyAuraSprite.getLocalBounds().height / 2);
		//dirtyAuraSprite.setPosition(Vector2f(position));
		break;
	}
	case SEQ_KNEEL_TO_MEDITATE:
	{
		SetSpriteTexture(action);
		bool r = (facingRight && !reversed) || (!facingRight && reversed);

		int f = frame / 3 + 1;
		SetSpriteTile( f, r);

		double angle = 0;//GroundedAngle()


		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height-16);
		sprite->setRotation(angle / PI * 180);

		V2d pp;
		if (ground != NULL)
			pp = ground->GetPoint(edgeQuantity);
		else
		{
			assert(0);
		}

		sprite->setPosition(pp.x, pp.y);
		break;
	}
	case SEQ_MEDITATE_MASKON:
	{
		SetSpriteTexture(action);
		bool r = (facingRight && !reversed) || (!facingRight && reversed);

		int f = 8;
		SetSpriteTile(f, r);

		double angle = 0;//GroundedAngle()


		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height-16);
		sprite->setRotation(angle / PI * 180);

		V2d pp;
		if (ground != NULL)
			pp = ground->GetPoint(edgeQuantity);
		else
		{
			assert(0);
		}

		sprite->setPosition(pp.x, pp.y);
		break;
	}
	case SEQ_MASKOFF:
	{
		SetSpriteTexture(action);
		bool r = (facingRight && !reversed) || (!facingRight && reversed);

		int f = frame / 3 + 11;
		SetSpriteTile(f, r);

		double angle = 0;//GroundedAngle()


		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height-16);
		sprite->setRotation(angle / PI * 180);

		V2d pp;
		if (ground != NULL)
			pp = ground->GetPoint(edgeQuantity);
		else
		{
			assert(0);
		}

		sprite->setPosition(pp.x, pp.y);
		break;
	}
	case SEQ_MEDITATE:
	{
		SetSpriteTexture(action);
		bool r = (facingRight && !reversed) || (!facingRight && reversed);

		int f = 39;
		SetSpriteTile(f, r);

		double angle = 0;//GroundedAngle()


		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height-16);
		sprite->setRotation(angle / PI * 180);

		V2d pp;
		if (ground != NULL)
			pp = ground->GetPoint(edgeQuantity);
		else
		{
			assert(0);
		}

		sprite->setPosition(pp.x, pp.y);
		break;
	}
	case SEQ_FLOAT_TO_NEXUS_OPENING:
	{
		SetSpriteTexture(action);
		SetSpriteTile(frame / 10, facingRight);
		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2 + 24);
		sprite->setPosition(Vector2f(position));
		//sprite->setPosition(owner->goalNodePos.x, owner->goalNodePos.y - 24.f);//- 24.f );
		//sprite->setPosition(Vector2f(position));
		sprite->setRotation(0);
		break;
	}
	case SEQ_FADE_INTO_NEXUS:
	{
		SetSpriteTexture(action);
		SetSpriteTile(frame / 10 + 3, facingRight);
		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2 + 24);
		sprite->setPosition(Vector2f(position));
		//sprite->setPosition(owner->goalNodePos.x, owner->goalNodePos.y - 24.f);//- 24.f );
		//sprite->setPosition(Vector2f(position));
		sprite->setRotation(0);
		break;
	}
	case SEQ_TURNFACE:
	{

		SetSpriteTexture(action);

		bool r = (facingRight && !reversed) || (!facingRight && reversed);
		SetSpriteTile(0, r);

		if (ground != NULL)
		{
			double angle = GroundedAngle();

			sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			
			V2d pp = ground->GetPoint(edgeQuantity);

			if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
				sprite->setPosition(pp.x + offsetX, pp.y);
			else
				sprite->setPosition(pp.x, pp.y);
			sprite->setRotation(angle / PI * 180);


			//cout << "angle: " << angle / PI * 180  << endl;
		}
		break;
	}
	case GETSHARD:
	{
		SetSpriteTexture(action);
		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height/2);
		sprite->setPosition(position.x, position.y);
		SetSpriteTile(0, facingRight);
		break;
	}
	}

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

	Transform tr1 = tr;
	//tr1.scale(1.5, 1.5);
	//Vector2f oldOrigin = sprite->getOrigin();
	//Vector2f center(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	//Vector2f diff = center - oldOrigin;

	//Vector2f center(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	Aura::NormalParams np;
	//sf::FloatRect gb = sprite->getGlobalBounds();
	//sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	
	np.centerPos = sprite->getPosition() + diff;//sprite->getPosition() + center;//Vector2f(gn.x, gn.y) * (sprite->getLocalBounds().height / 2);
	//sprite->setOrigin(oldOrigin);
	
	if (usingAura)
	{
		if (auraPoints[0][spriteAction] != NULL)
		{
			testAura->ActivateParticles(auraPoints[0][spriteAction][currTileIndex], tr, Vector2f(spriteCenter), &np, 0);
			//testAura1->ActivateParticles(auraPoints[spriteAction][currTileIndex], tr1, Vector2f( spriteCenter ) + extraParticle0, &np);
			//testAura2->ActivateParticles(auraPoints[spriteAction][currTileIndex], tr, sprite->getOrigin() + extraParticle2, &np);
			//testAura3->ActivateParticles(auraPoints[spriteAction][currTileIndex], tr, sprite->getOrigin(), &np);
		}

		if (auraPoints[1][spriteAction] != NULL)
		{
			testAura1->ActivateParticles(auraPoints[1][spriteAction][currTileIndex], tr, Vector2f(spriteCenter) + extraParticle1, &np, 3);
		}

		if (auraPoints[2][spriteAction] != NULL)
		{
			//np.thickness = 10.f;
			//tr.scale(2, 2);
			testAura2->ActivateParticles(auraPoints[2][spriteAction][currTileIndex], tr, Vector2f(spriteCenter) + extraParticle2, &np, 1);
		}
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

void Actor::ConfirmHit( EnemyParams *hitParams )
{
	//owner->cam.SetRumble(3, 3, 5);

	if (!hitParams->canBeHit)
		return;

	if (ground == NULL && velocity.y > 0 && action == DAIR )
	{
		velocity.y = 4;//.5;
	}

	Color c;
	switch(hitParams->worldIndex )
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

	currentSpeedBar += hitParams->speedBar;
	test = true;
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
		kinRing->powerRing->Fill(hitParams->charge);
	
	desperationMode = false;
	
	
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
	/*if( bounceFlameOn )
	{
		if( ground == NULL )
		{
			velocity = -velocity;
		}
	}*/
	//owner->GetPlayer( 0 )->test = true;
	//desperationMode = false;
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

void Actor::SetSpriteTexture( Action a )
{
	spriteAction = a;
	sprite->setTexture( *tileset[a]->texture );
	//orbSprite.setTexture( )
	motionGhostBuffer->SetTileset(tileset[a]);
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetTileset(tileset[a]);
	}
	
	motionGhostBufferBlue->SetTileset(tileset[a]);
	motionGhostBufferPurple->SetTileset(tileset[a]);
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

	IntRect ir = tileset[spriteAction]->GetSubRect( currTileIndex );
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


	Vector2f oldOrigin = sprite->getOrigin();
	Vector2f center(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	Vector2f diff = center - oldOrigin;

	//Vector2f center(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	Aura::NormalParams np;
	//sf::FloatRect gb = sprite->getGlobalBounds();
	//sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);


	np.centerPos = sprite->getPosition() + diff;//sprite->getPosition() + center;//Vector2f(gn.x, gn.y) * (sprite->getLocalBounds().height / 2);
	sprite->setOrigin(oldOrigin);
	//if (auraPoints[action] != NULL)
	//{
	//	testAura->ActivateParticles(auraPoints[action][tileIndex], tr, sprite->getOrigin() + extraParticle0, &np);
	//	testAura1->ActivateParticles(auraPoints[action][tileIndex], tr, sprite->getOrigin() + extraParticle1, &np);
	//	testAura2->ActivateParticles(auraPoints[action][tileIndex], tr, sprite->getOrigin() + extraParticle2, &np);
	//}
}

void Actor::SetupAction(Action a)
{
	//actionLength[WALLATTACK] = 8 * 2;
	//actionLength[DAIR] = 16;
	//actionLength[DASH] = 45;
	//actionLength[DOUBLE] = 28 + 10;
	//actionLength[BACKWARDSDOUBLE] = 40;
	//actionLength[FAIR] = 8 * 2;
	//actionLength[DIAGUPATTACK] = 14 * 2;
	//actionLength[DIAGDOWNATTACK] = 15 * 2;
	//actionLength[JUMP] = 2;
	//actionLength[SEQ_WAIT] = 2;
	//actionLength[SEQ_CRAWLERFIGHT_DODGEBACK] = 2;
	//actionLength[SEQ_CRAWLERFIGHT_STRAIGHTFALL] = 2;
	//actionLength[LAND] = 1;
	//actionLength[SEQ_CRAWLERFIGHT_LAND] = 1;
	//actionLength[LAND2] = 1;
	//actionLength[RUN] = 10 * 4;
	//actionLength[SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY] = 10 * 4;
	//actionLength[SLIDE] = 1;
	//actionLength[SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED] = 1;
	//actionLength[SPRINT] = 8 * 4;
	//actionLength[STAND] = 20 * 8;
	//actionLength[SPRINGSTUN] = 8;
	//actionLength[SEQ_CRAWLERFIGHT_STAND] = 20 * 8;//240;//20 * 8;
	//actionLength[DASHATTACK] = 8 * 2;
	//actionLength[STANDN] = 4 * 4;
	//actionLength[UAIR] = 16;
	//actionLength[GRINDATTACK] = 1;
	//actionLength[STEEPSLIDE] = 1;
	//actionLength[WALLCLING] = 1;
	//actionLength[WALLJUMP] = 9 * 2;
	//actionLength[GRINDBALL] = 1;
	//actionLength[GRINDLUNGE] = 20;
	//actionLength[GRINDSLASH] = 16;
	//actionLength[STEEPCLIMBATTACK] = 4 * 4;
	//actionLength[SKYDIVETOFALL] = 10 * 4;
	//actionLength[WAITFORSHIP] = 60 * 1;
	//actionLength[GRABSHIP] = 4 * 4 + 20;
	//actionLength[GETPOWER_AIRDASH_MEDITATE] = 120;
	//actionLength[RIDESHIP] = 1;
	//actionLength[SKYDIVE] = 9 * 2;
	//actionLength[EXIT] = 27 * 2;
	//actionLength[GRAVREVERSE] = 20;
	//actionLength[JUMPSQUAT] = 3;
	//actionLength[INTRO] = 10 * 4;
	//actionLength[AIRDASH] = 33;//27;
	//actionLength[STEEPSLIDEATTACK] = 6 * 3;
	//actionLength[AIRHITSTUN] = 1;
	//actionLength[STEEPCLIMB] = 8 * 4;
	//actionLength[GROUNDHITSTUN] = 1;
	//actionLength[WIREHOLD] = 1;
	//actionLength[BOUNCEAIR] = 1;
	//actionLength[BOUNCEGROUND] = 15;
	//actionLength[BOUNCEGROUNDEDWALL] = 30;
	//actionLength[DEATH] = 44 * 2;
	//actionLength[GETPOWER_AIRDASH_FLIP] = 20 * 5;
	//actionLength[GOALKILL] = 72 * 2;
	//actionLength[ENTERNEXUS1] = 10 * 4;
	//actionLength[GOALKILLWAIT] = 2;
	//actionLength[SPAWNWAIT] = 120;
	//actionLength[RAILDASH] = 20;
	//switch (a)
	//{
	//	//case 
	//}
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
	if (hasPowerLeftWire &&
		((action != Actor::GRINDBALL && action != Actor::GRINDATTACK)
			|| leftWire->state == Wire::RETRACTING))
	{
		leftWire->Draw(target);
	}
	if (hasPowerRightWire &&
		((action != Actor::GRINDBALL && action != Actor::GRINDATTACK)
			|| rightWire->state == Wire::RETRACTING))
	{
		rightWire->Draw(target);
	}
}

void Actor::UpdateWireQuads()
{
	if (hasPowerLeftWire)
		leftWire->UpdateQuads();

	if (hasPowerRightWire)
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
		CollisionBox &gh = grindHitboxes[0]->GetCollisionBoxes(0)->front();
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

	if (facingRight)
	{
		velocity.x = strengthX;
	}
	else
	{
		velocity.x = -strengthX;
	}

	if (velocity.y < -strengthY)
	{

	}
	else
	{
		velocity.y = -strengthY;
	}


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

Actor::Action Actor::GetDoubleJump()
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
		SetActionExpr( DOUBLE );

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
	if (hasPowerAirDash && !IsSingleWirePulling())
	{
		if ((hasAirDash || inBubble) && ((!prevInput.B && currInput.B) || pauseBufferedDash
			|| stunBufferedDash ))
		{
			hasFairAirDashBoost = (action == FAIR);
			SetActionExpr( AIRDASH );
			return true;
		}
	}

	return false;
}

bool Actor::TryGlide()
{
	bool ad = (hasPowerAirDash && !hasAirDash) || !hasPowerAirDash;
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
	case JUMPSQUAT:
		framesInAir = 0;
		bufferedAttack = JUMP;

		if( currInput.rightShoulder && !prevInput.rightShoulder )
		{
			if( currInput.LUp() )
			{
				bufferedAttack = UAIR; //none
			}
			else if( currInput.LDown() )
			{
				bufferedAttack = DAIR; 
			}
			else
			{
				bufferedAttack = FAIR; 
			}
		}
		break;
	case BACKWARDSDOUBLE:
	case DOUBLE:
		a = GetDoubleJump();
		frame = 0;
		holdDouble = true;
		break;
	case JUMP:
		steepJump = false;
	case WALLJUMP:
	case LAND:
	case LAND2:
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
	case NEXUSKILL:
	case GOALKILL:
		SetExpr( Expr_NEUTRAL );
		velocity = V2d(0, 0);
		groundSpeed = 0;
		grindSpeed = 0;
		break;
	case AIRDASH:
		{
			framesExtendingAirdash = 0;
			BounceFlameOff();
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
	case DASH:
	{
		frame = 0;
		currBBoostCounter = 0;
		break;
	}
	
		
	}

	SetAction( a );
	//action = a;
}

bool Actor::IHitPlayer( int otherPlayerIndex )
{
	Actor *player = owner->GetPlayer( otherPlayerIndex );
	
	if( player->invincibleFrames == 0 )
	{
		if( currHitboxes != NULL )
		{
			bool hit = false;

			list<CollisionBox> *cList = (currHitboxes->GetCollisionBoxes(currHitboxFrame));
			if( cList != NULL )
			for( list<CollisionBox>::iterator it = cList->begin(); it != cList->end(); ++it )
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

bool Actor::IsAttackAction( Action a )
{
	return (a == FAIR || a == DAIR || a == UAIR || a == STANDN || a == DIAGDOWNATTACK
		|| a == DIAGUPATTACK || a == WALLATTACK || a == STEEPCLIMBATTACK || a == STEEPSLIDEATTACK );
}

bool Actor::IsGroundAttackAction(Action a)
{
	return (a == STANDN || a == STEEPCLIMBATTACK || a == STEEPSLIDEATTACK);
}

bool Actor::IsSpringAction(Action a)
{
	return a == SPRINGSTUN || a == SPRINGSTUNGLIDE || a == SPRINGSTUNBOUNCE || a == SPRINGSTUNAIRBOUNCE
		|| a == SPRINGSTUNTELEPORT || a == SPRINGSTUNAIRBOUNCE;
}

bool Actor::IsOnRailAction(Action a)
{
	return a == RAILGRIND || a == RAILSLIDE;
}

bool Actor::IsInHistunAction( Action a )
{
	return a == GROUNDHITSTUN || a == AIRHITSTUN;
}

V2d Actor::GetKnockbackDirFromVel()
{
	if (owner->GetPlayer(0)->ground != NULL)
	{
		return normalize(-owner->GetPlayer(0)->groundSpeed * (owner->GetPlayer(0)->ground->v1 - owner->GetPlayer(0)->ground->v0));
	}
	else
	{
		return normalize(-owner->GetPlayer(0)->velocity);
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
	
	/*Action action = states[ghostFrame].action;
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
	
	}*/
	/*case DIAGUPATTACK:
		{
			if( diagUpHitboxes.count( frame ) > 0 )
			{
				currHitboxes = diagUpHitboxes[frame];
			}
			break;
		}
	case DIAGDOWNATTACK:
		{
			if( diagDownHitboxes.count( frame ) > 0 )
			{
				currHitboxes = diagDownHitboxes[frame];
			}
			break;
		}
	}
*/
	//if( currHitboxes != NULL )
	//{
	//	for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
	//	{
	//		(*it).globalAngle = angle;

	//		(*it).globalPosition = position + V2d( (*it).offset.x * cos( (*it).globalAngle ) + (*it).offset.y * sin( (*it).globalAngle ), 
	//			(*it).offset.x * -sin( (*it).globalAngle ) + (*it).offset.y * cos( (*it).globalAngle ) );

	//		//(*it).globalPosition = position ;//+ (*it).offset;
	//	
	//	}
	//}

	
}

int Actor::CreateAura(std::list<sf::Vector2f> *&outPointList,
	Tileset *ts, int startTile, int numTiles, int layer )
{
	//auraPoints[WALLATTACK] = 8;
	//CreateAura(WALLATTACK, 8);
	if (ts == NULL)
	{
		outPointList = NULL;
		return 0;
	}

	//int endTile = startTile + numTiles;
	if (numTiles == 0)
	{
		numTiles = ts->GetNumTiles() - startTile;
		//endTile = ts->GetNumTiles() - startTile;
	}

	outPointList = new list<Vector2f>[numTiles];

	//Image im = ts->texture->copyToImage();
	if (ts->sourceName == tileset[GOALKILL]->sourceName)
	{
		int bbb = 5;
	}
	for (int i = 0; i < numTiles; ++i)
	{
		Aura::CreateParticlePointList( owner->mainMenu->auraCheckTexture, ts, i + startTile, outPointList[i], layer);
	}

	return numTiles;
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