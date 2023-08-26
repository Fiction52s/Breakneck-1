#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Zone.h"
#include <sstream>
#include "Boss.h"
#include "KeyMarker.h"
#include "Enemy_Comboer.h"
#include "Shield.h"
#include "MainMenu.h"
#include "Enemy_CurveTurret.h"
#include "Wire.h"
#include "Enemy_Specter.h"
#include "Actor.h"
#include "AbsorbParticles.h"
#include "Bullet.h"
#include "SummonGroup.h"

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


int Enemy::bloodLengths[8] = {
	16,
	16,
	16,
	16,
	13,
	17,
	20,
	9 };

const int Enemy::summonDuration = 60;
const int Enemy::minSubstepToCheckHits = 3;
const int Enemy::DEFAULT_DETECT_RADIUS = 800; //default values
const int Enemy::DEFAULT_IGNORE_RADIUS = 2000; //default values

bool Enemy::IsGoalType()
{
	return type == EN_GOAL 
		|| type == EN_SHIPPICKUP
		|| type == EN_NEXUS;
}

void Enemy::SetSequenceIDsAndAddThemToAllSequencesVec()
{
	//empty by default since no sequences are owned
}

void Enemy::CreateSurfaceMover(PositionInfo &pi,
	double rad, SurfaceMoverHandler *handler )
{
	assert(groundMover == NULL && surfaceMover == NULL);
	surfaceMover = new SurfaceMover(pi.GetEdge(), pi.GetQuant(), rad * scale);
	surfaceMover->surfaceHandler = handler;
	highResPhysics = true;
}

void Enemy::CreateGroundMover(PositionInfo &pi,
	double rad, bool steeps, GroundMoverHandler *handler)
{
	assert(groundMover == NULL && surfaceMover == NULL);
	groundMover = new GroundMover(pi.GetEdge(), pi.GetQuant(), rad * scale, steeps, handler );
	highResPhysics = true;
}

void Enemy::SetCurrPosInfo(PositionInfo &pi)
{
	currPosInfo = pi;
	currPosInfo.SetGroundOffset(groundOffset.x);
	currPosInfo.SetGroundHeight(groundOffset.y);
}

void Enemy::SetOffGroundHeight(double h)
{
	groundOffset.y = h;
	currPosInfo.SetGroundHeight(groundOffset.y);
}

void Enemy::SetGroundOffset(double x)
{
	groundOffset.x = x;
	currPosInfo.SetGroundOffset(groundOffset.x);
}

CollisionBody *Enemy::GetCollisionBodyFromID( int id )
{
	if (id < 0)
	{
		return NULL;
	}
	else
	{
		return bodyPtrVec[id];
	}
}

int Enemy::GetCollisionBodyID(CollisionBody *cb)
{
	if (cb == NULL)
	{
		return -1;
	}
	else
	{
		return cb->bodyID;
	}
}

Shield *Enemy::GetShieldFromID(int id)
{
	if (id < 0)
		return NULL;
	else
		return shieldPtrVec[id];
}

int Enemy::GetShieldID(Shield *s)
{
	if (s == NULL)
		return -1;
	else
		return s->shieldID;
}

V2d Enemy::GetPosition()
{
	if (surfaceMover != NULL)
	{
		return surfaceMover->GetPosition();
	}
	else if (groundMover != NULL)
	{
		return groundMover->GetPosition();
	}
	else
	{
		return currPosInfo.GetPosition();
	}
}

Vector2f Enemy::GetPositionF()
{
	return Vector2f(GetPosition());
}

void Enemy::SetEditorActions(int p_editLoopAction, int p_editIdleAction,int p_editIdleFrame)
{
	editLoopAction = p_editLoopAction;
	editIdleAction = p_editIdleAction;
	editIdleFrame = p_editIdleFrame;
}

void Enemy::SetSpawnRect()
{
	spawnRect = sf::Rect<double>(GetAABB());//sf::Rect<double>(gPoint.x - size / 2, gPoint.y - size / 2, size, size);
}

double Enemy::PlayerDist(int index)
{
	return length(GetPosition() - sess->GetPlayerPos(index));
}

double Enemy::PlayerDistSqr(int index)
{
	return lengthSqr(GetPosition() - sess->GetPlayerPos(index));
}

V2d Enemy::PlayerDir( V2d myOffset, V2d playerOffset, int index)
{
	return normalize( 
		( sess->GetPlayerPos(index) + playerOffset) 
		- ( GetPosition() + myOffset ));
}

V2d Enemy::PlayerDiff(int index)
{
	return sess->GetPlayerPos(index) - GetPosition();
}

V2d Enemy::PlayerDir(int index)
{
	return normalize(sess->GetPlayerPos(index)- GetPosition());
}

V2d Enemy::AlongGroundDir()
{
	if (surfaceMover != NULL)
	{
		if (surfaceMover->ground != NULL)
		{
			if (facingRight)
			{
				return surfaceMover->ground->Along();
			}
			else
			{
				return -surfaceMover->ground->Along();
			}
		}
		else if (groundMover->ground != NULL)
		{
			if (facingRight)
			{
				return groundMover->ground->Along();
			}
			else
			{
				return -groundMover->ground->Along();
			}
		}
		else if (currPosInfo.ground != NULL )
		{
			if (facingRight)
			{
				return currPosInfo.GetEdge()->Along();
			}
			else
			{
				return -currPosInfo.GetEdge()->Along();
			}
			
		}
	}
}

bool Enemy::SetHitParams()
{
	switch (type)
	{
	case EnemyType::EN_MULTIPLAYERBASE:
	{
		hitParams.Set(5, .8, (3 * 60) / 1, 3, false);
		break;
	}
	case EnemyType::EN_MULTIPLAYERPROGRESSTARGET:
	{
		hitParams.Set(5, .8, (3 * 60) / 1, 3, false);
		break;
	}
	case EnemyType::EN_CRAWLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_GOAL:
		hitParams.Set(5, 0, 0, 3, false);
		break;
	case EnemyType::EN_PATROLLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_FOOTTRAP:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_COMBOER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	case EnemyType::EN_SPLITCOMBOER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1, false);
		break;
	case EnemyType::EN_AIRDASHER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_BASICTURRET:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SHROOM:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SHROOMJELLY:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_CRAWLERQUEEN:
		hitParams.Set(5, .8, (3 * 60) / 3, 3 * 8);//40);//40);// 120);
		break;
	case EnemyType::EN_QUEENFLOATINGBOMB:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_BLOCKER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	/*case EnemyType::EN_HEALTHFLY:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;*/
	case EnemyType::EN_BOSS_BIRD:
		hitParams.Set(5, .8, (3 * 60) / 40, 400);
		break;
	case EnemyType::EN_BAT:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_AIRDASHJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_GRAVITYJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_ATTRACTJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_PRIMARYJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_BOUNCEJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_GRINDJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_GROUNDEDGRINDJUGGLER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_POISONFROG:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_CURVETURRET:
		hitParams.Set( 5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_LOBTURRET:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_EXPLODINGBARREL:
	{
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	}
	case EnemyType::EN_PREDICTTURRET:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SHOTGUNTURRET:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_PALMTURRET:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_STAGBEETLE:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_GRAVITYFALLER:
		hitParams.Set(4, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SPIDER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_WIDOW:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_PARROT:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_TIGERTARGET:
		hitParams.Set(5, .8, (3 * 60) / 3, 1, false);
		break;
	case EnemyType::EN_PULSER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_FIREFLY:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_TRAILER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_LASERJAYS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_TETHEREDRUSHER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_DRAGON:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_CHESS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_LASERWOLF:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_FALCON:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_BADGER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_COPYCAT:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_BOUNCEFLOATER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	case EnemyType::EN_BOUNCEBOOSTER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	case EnemyType::EN_OWL:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_BALL:
		hitParams.Set(5, 0, 60, 1, false);
		break;
	case EnemyType::EN_ROADRUNNER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SKUNK:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_LIZARD:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_CACTUS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_CACTUSSHOTGUN:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_CHEETAH:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_TIGERSPINTURRET:
		hitParams.Set(5, .8, (3 * 60) / 2, 2);
		break;
	case EnemyType::EN_HUNGRYCOMBOER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1, false);
		break;
	case EnemyType::EN_RELATIVECOMBOER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1, false);
		break;
	case EnemyType::EN_SWARM:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SWARMMEMBER:
		hitParams.Set(5, .8, (3 * 60) / 3, 1,false);
		break;
	case EnemyType::EN_GHOST:
		hitParams.Set(5, .8, (3 * 60) / 3, 1);
		break;
	case EnemyType::EN_GROWINGTREE:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SHARK:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SPECTER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_GORILLA:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_WIRETARGET:
		hitParams.Set(5, 0, 0, 1);
		break;
	case EnemyType::EN_BIRDSHURIKEN:
		hitParams.Set(5, 0, 0, 1, false);
		break;
	case EnemyType::EN_SPECTERBULLET:
		hitParams.Set(5, .8, (3 * 60) / 3, 1, false);
		break;
	case EnemyType::EN_GORILLAWALL:
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	case EnemyType::EN_DIMENSIONEYE:
		hitParams.Set(5, .8, (3 * 60) / 4, 4);
		break;
	case EnemyType::EN_DIMENSIONEXIT:
		hitParams.Set(5, .8, (3 * 60) / 4, 4);
		break;
	case EnemyType::EN_GREYEYE:
		hitParams.Set(5, .8, (3 * 60) / 4, 4);
		break;
	case EnemyType::EN_COYOTEBOSS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_BIRDBOSS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3 * 8);
		break;
	case EnemyType::EN_GATORBOSS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_TIGERBOSS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_SKELETONBOSS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_GREYSKELETONBOSS:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_TURTLE:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_PUFFERFISH:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_FUTURECHECKER:
		hitParams.Set(5, .8, (3 * 60) / 3, 3);
		break;
	case EnemyType::EN_KEYFLY:
		hitParams.Set(5, .8, (3 * 60) / 2, 2, false);
		break;
	case EnemyType::EN_SPECIALTARGET:
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	case EnemyType::EN_PHASESWITCH:
		hitParams.Set(5, .8, (3 * 60) / 3, 3, false);
		break;
	default:
		return false;
	}

	return true;
}

Enemy::Enemy(EnemyType t, ActorParams *ap)
	:hurtBody( CollisionBox::BoxType::Hurt ), hitBody(CollisionBox::BoxType::Hit )
{
	type = t;
	if (ap == NULL)
	{
		OnCreate(NULL, 0);
	}
	else
	{
		OnCreate(ap, ap->GetWorld());
	}
}

Enemy::Enemy(EnemyType t, int w)
	:hurtBody(CollisionBox::BoxType::Hurt), hitBody(CollisionBox::BoxType::Hit)
{
	type = t;
	OnCreate(NULL, w);
}

void Enemy::OnCreate(ActorParams *ap,
	int w)
{
	if (ap != NULL)
	{
		name = ap->GetTypeName();
	}

	enemyDrawLayer = ENEMYDRAWLAYER_DEFAULT;

	RegisterCollisionBody(hitBody);
	RegisterCollisionBody(hurtBody);

	pauseFramesFromAttacking = false;
	active = false;
	summonGroup = NULL;
	prev = NULL;
	next = NULL;
	zone = NULL;
	spawned = false;
	suppressMonitor = false;
	ts_hitSpack = NULL;
	dead = false;

	summonFrame = 0;
	playerIndex = -1; //can be affected by all players
	keyShaderLoaded = false;
	origFacingRight = true;
	facingRight = true;
	editLoopAction = 0;
	editIdleFrame = 0;

	enemyIndex = -1;

	groundMover = NULL;
	surfaceMover = NULL;
	currShield = NULL;

	hasMonitor = false;

	if (ap != NULL)
	{
		world = ap->GetWorld();
		Enemy::UpdateOnPlacement(ap);
	}
	else
	{
		hasMonitor = false;
		world = w;
	}

	sess = Session::GetSession();

	//need this for updateparamssettings calls
	editParams = ap;
	/*if (sess->IsSessTypeEdit())
	{
		
	}
	else
	{
		editParams = NULL;
	}*/


	if (CanTouchSpecter())
	{
		//specterTester = NULL;
		specterTester = new SpecterTester(this);
	}
	else
	{
		specterTester = NULL;
	}

	scale = 1.f;

	comboObj = NULL;
	hitboxInfo = NULL;

	if (hasMonitor)
	{
		sess->numTotalKeys++;
	}

	pauseFrames = 0;
	ts_zoned = GetSizedTileset("Enemies/enemy_zone_icon_128x128.png");
	zonedSprite.setTexture(*ts_zoned->texture);

	genericDeathSound = GetSound("Enemies/kill");
	finalDeathSound = GetSound("Test/Heal_01");
	keyDeathSound = GetSound("Enemies/Key_Kill_02");
	keyUnlockDeathSound = GetSound("Test2/Key_Complete_01");

	highResPhysics = false;
	numLaunchers = 0;
	currHitboxes = NULL;
	currHurtboxes = NULL;

	ResetSlow();

	if (SetHitParams())
	{
		maxHealth = hitParams.maxHealth;
	}
	else
	{
		//assert(false);
		//cout << "hitparams not found" << endl;
		maxHealth = 1;//maxHealth;
	}

	numHealth = maxHealth;

	if (hitParams.cuttable)
	{
		cutObject = new CuttableObject;
	}
	else
	{
		cutObject = NULL;
	}

	if (!hurtShader.loadFromFile("Resources/Shader/enemyhurt_shader.frag", sf::Shader::Fragment))
	{
		cout << "couldnt load enemy enemyhurt shader" << endl;
		assert(false);
	}
	hurtShader.setUniform("toColor", Glsl::Vec4(Color::White.r, Color::White.g, Color::White.b, Color::White.a));
	//hurtShader.setUniform("auraColor", Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a));

	if (world == 0)
	{
		ts_hitSpack = NULL;
		ts_blood = NULL;
		return;
	}



	SetKey();


	int fxWorld = world;

	//stringstream ss;
	//ss << "FX/hit_spack_" << fxWorld << "_128x128.png";

	ts_hitSpack = NULL;//sess->GetTileset( ss.str(), 128, 128 );

					   //ss.clear();
					   //ss.str("");

					   //ss << "FX/w" << fxWorld << "_kill_fx_512x512.png";

					   //ts_killSpack = sess->GetTileset(ss.str(), 512, 512);
					   //assert(ts_killSpack != NULL);
					   //ss.clear();
					   //ss.str("");
	if (fxWorld > 0)
	{
		stringstream ss;
		ss << "FX/Blood/blood_w" << fxWorld << "_256x256.png";

		ts_blood = GetSizedTileset(ss.str());
	}
	else
	{
		ts_blood = NULL;
	}
}


Tileset * Enemy::GetTileset(const std::string &s,
	int tileWidth,
	int tileHeight)
{
	if( sess->specialTempTilesetManager != NULL )
	{
		return sess->specialTempTilesetManager->GetTileset(s, tileWidth, tileHeight);
	}
	else
	{
		return sess->GetTileset(s, tileWidth, tileHeight);
	}
}

Tileset *Enemy::GetSizedTileset(const std::string &s)
{
	if (sess->specialTempTilesetManager != NULL)
	{
		return sess->specialTempTilesetManager->GetSizedTileset(s);
	}
	else
	{
		return sess->GetSizedTileset(s);
	}
}

SoundInfo * Enemy::GetSound(const std::string &s)
{
	if (sess->specialTempSoundManager != NULL)
	{
		return sess->specialTempSoundManager->GetSound(s);
	}
	else
	{
		return sess->GetSound(s);
	}
}

void Enemy::SetKey()
{
	if (hasMonitor)
	{
		switch (world)
		{
		case 1:
			keyColor = COLOR_BLUE;
			break;
		case 2:
			keyColor = COLOR_GREEN;
			break;
		case 3:
			keyColor = COLOR_YELLOW;
			break;
		case 4:
			keyColor = COLOR_ORANGE;
			break;
		case 5:
			keyColor = COLOR_RED;
			break;
		case 6:
			keyColor = COLOR_MAGENTA;
			break;
		}
		keyColor = Color::White;

		if (!keyShaderLoaded)
		{
			if (!keyShader.loadFromFile("Resources/Shader/key_shader.frag", sf::Shader::Fragment))
			{
				cout << "couldnt load enemy key shader" << endl;
				assert(false);
			}
			
			keyShaderLoaded = true;
		}

		keyShader.setUniform("toColor", ColorGL(keyColor));//Glsl::Vec4( keyColor.r, keyColor.g, keyColor.b, keyColor.a ) );

		keyShader.setUniform("auraColor", ColorGL(Color::Transparent));
		//keyShader.setUniform("auraColor", ColorGL(auraColor));//Glsl::Vec4(auraColor.r, auraColor.g, auraColor.b, auraColor.a) );

		int w = world;
		if (world > 2)
		{
			w = 2; //we dont have all the sprites yet
		}

		//stringstream ss;
		//ss << "FX/key_w0" << w << "_1_128x128.png";
		//ts_key = sess->GetTileset(ss.str(), 128, 128);


		ts_key = sess->GetSizedTileset("FX/key_w1_256x256.png");

		keySprite.setTexture(*ts_key->texture);
		UpdateKeySprite();
	}
	else
	{
		ts_key = NULL;
	}
}

void Enemy::UpdateParamsSettings()
{
	if (sess->IsSessTypeEdit())
	{
		bool oldHasMonitor = hasMonitor;
		hasMonitor = editParams->hasMonitor;

		if (hasMonitor != oldHasMonitor)
		{
			SetKey();
		}
	}
}

void Enemy::SetActionEditLoop()
{
	action = editLoopAction;
	frame = 0;
}

void Enemy::DefaultHitboxesOn( int hFrame )
{
	SetHitboxes(&hitBody, hFrame );
}

void Enemy::DefaultHurtboxesOn( int hFrame )
{
	SetHurtboxes(&hurtBody, hFrame);
}

void Enemy::HurtboxesOff()
{
	SetHurtboxes(NULL, 0);
}

void Enemy::HitboxesOff()
{
	SetHitboxes(NULL, 0);
}

void Enemy::SetNumLaunchers(int num)
{
	numLaunchers = num;
	launchers.resize(num);
}

void Enemy::UpdateOnPlacement( ActorParams *ap )
{
	startPosInfo = ap->posInfo;
	hasMonitor = ap->hasMonitor;

	if (surfaceMover != NULL)
	{
		surfaceMover->Set(ap->posInfo);
	}
	else if (groundMover != NULL)
	{
		groundMover->Set(ap->posInfo);
	}
	else
	{
		SetCurrPosInfo(ap->posInfo);
	}

	if (currShield != NULL)
	{
		currShield->SetPosition(GetPosition());
	}

}

void Enemy::UpdateFromParams( ActorParams *ap, int numFrames )
{
	//assert(editParams != NULL);

	frame += numFrames;
	int editLoopLength = GetEditIdleLength();
	if (editLoopLength == 0)
		frame = 0;
	else if (frame >= editLoopLength)
	{
		frame = frame % editLoopLength;
	}

	//UpdateFromParams(editParams);
	UpdateKeySprite();
	UpdateSprite();
	UpdateSpriteFromParams(ap);

	if (currShield != NULL)
	{
		currShield->UpdateSprite();
	}

	SetSpawnRect();
}

void Enemy::SetNumActions( int num )
{
	actionLength.resize(num);

	//defaults to animating at 100 percent speed
	animFactor.resize(num);
	for (int i = 0; i < num; ++i)
	{
		animFactor[i] = 1;
	}
}

int Enemy::GetEditIdleLength()
{
	return actionLength[editLoopAction] * animFactor[editLoopAction];
}

Enemy::~Enemy()
{
	if (specterTester != NULL)
		delete specterTester;

	if (numLaunchers > 0)
	{
		for (int i = 0; i < numLaunchers; ++i)
		{
			delete launchers[i];
		}

		//launchers.clear();
	}

	if (cutObject != NULL)
	{
		delete cutObject;
	}

	/*if (currShield != NULL)
		delete currShield;*/

	if (hitboxInfo != NULL)
		delete hitboxInfo;

	if (comboObj != NULL)
	{
		delete comboObj;
	}

	if (surfaceMover != NULL)
		delete surfaceMover;

	if (groundMover != NULL)
		delete groundMover;
}

bool Enemy::ReadPath(std::ifstream &is,
	int &pLen, std::vector<Vector2i> &localPath)
{
	is >> pLen;

	localPath.reserve(pLen);
	for (int i = 0; i < pLen; ++i)
	{
		int localX, localY;
		is >> localX;
		is >> localY;
		localPath.push_back(Vector2i(localX, localY));
	}

	return true;
}

bool Enemy::ReadBool(std::ifstream &is,
	bool &b)
{
	int x;
	is >> x;
	b = x;
	return true;
}

void Enemy::PlayKeyDeathSound()
{
	if (sess->currentZone != NULL
		&& sess->currentZone->HasKeyGateOfNumber(sess->GetPlayer(receivedHitPlayerIndex)->numKeysHeld))
	{
		sess->ActivateSound(keyUnlockDeathSound);
	}
	else
	{
		sess->ActivateSound(keyDeathSound);
	}
}

void Enemy::PlayDeathSound()
{
	if( hasMonitor )
	{
		PlayKeyDeathSound();
		return;
	}

	if ( sess->currentZone != NULL && sess->currentZone->HasEnemyGate())
	{
		int numEnemiesRemaining = sess->currentZone->GetNumRemainingKillableEnemies();
		if (numEnemiesRemaining == 0)
		{
			//if( sess->currentZone->)

			sess->ActivateSound(finalDeathSound);
		}
		else
		{
			sess->ActivateSound(genericDeathSound);
		}
	}
	else
	{
		sess->ActivateSound(genericDeathSound);
	}
	
	
}

void Enemy::SetZoneSpritePosition()
{
	zonedSprite.setPosition(GetPositionF() );
}

std::vector<CollisionBox> * Enemy::GetComboHitboxes()
{
	if (comboObj != NULL)
	{
		CollisionBody &body = comboObj->enemyHitBody;
		if (!body.IsEmpty())
		{
			return &body.GetCollisionBoxes(comboObj->data.enemyHitboxFrame);
		}
	}

	return NULL;
}

void Enemy::DrawSprite( sf::RenderTarget *target, sf::Sprite &spr )
{
	bool drawHurtShader = ( pauseFrames >= 2 && !pauseFramesFromAttacking ) && currShield == NULL;
		//(pauseFrames < 2 || currShield != NULL);
		//(sess->GetPauseFrames() < 2 && pauseFrames < 2) 
		//|| ( receivedHit == NULL && pauseFrames < 2 );

	if (hasMonitor && !suppressMonitor)
	{
		if (drawHurtShader)
		{
			target->draw(spr, &hurtShader);
		}
		else
		{
			target->draw(spr, &keyShader);
		}
		target->draw(keySprite);
	}
	else
	{
		if(drawHurtShader)
		{
			target->draw(spr, &hurtShader);
		}
		else
		{
			target->draw(spr);
		}
	}
}

int Enemy::NumTotalBullets()
{
	return 0;
}

void Enemy::Reset()
{
	numHealth = maxHealth;
	
	if (cutObject != NULL)
		cutObject->Reset();
	ResetSlow();
	suppressMonitor = false;
	facingRight = origFacingRight;
	spawned = false;
	prev = NULL;
	next = NULL;
	currHitboxes = NULL;
	currHurtboxes = NULL;
	dead = false;
	currShield = NULL;
	receivedHit.SetEmpty();
	receivedHitPlayerIndex = -1;
	comboHitEnemyID = -1;
	action = 0;
	pauseFrames = 0;
	frame = 0;
	summonFrame = 0;
	active = false;
	currHitboxFrame = 0;
	currHurtboxFrame = 0;
	SetCurrPosInfo(startPosInfo);
	

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->Reset();
	}

	ResetEnemy();

	UpdateKeySprite();

	UpdateHitboxes();
}

void Enemy::SetEnemyIDAndAddToAllEnemiesVec()
{
	//assert(enemyIndex == -1);

	enemyIndex = sess->allEnemiesVec.size();
	sess->allEnemiesVec.push_back(this);

	if (comboObj != NULL)
	{
		comboObj->SetIDAndAddToAllComboObjectsVec();
	}
}

void Enemy::SetSummonGroup(SummonGroup *p_summonGroup )
{
	summonGroup = p_summonGroup;
}

bool Enemy::IsSummoning()
{
	return (summonGroup != NULL && !summonGroup->instantSummon && summonFrame < summonDuration);
}

void Enemy::SetHitboxes(CollisionBody *cb, int frame)
{
	currHitboxes = cb;
	currHitboxFrame = frame;
}

void Enemy::SetHurtboxes(CollisionBody *cb, int frame)
{
	currHurtboxes = cb;
	currHurtboxFrame = frame;
}

void Enemy::BasicCircleHurtBodySetup(double radius, double angle, V2d &offset, V2d &pos)
{
	if (radius > 0)
	{
		hurtBody.ResetFrames(); //in case it happens more than once
		hurtBody.BasicCircleSetup(radius * scale, angle, offset * scale);
		hurtBody.SetBasicPos(pos, angle);
	}
}

void Enemy::BasicCircleHitBodySetup(double radius, double angle, V2d &offset, V2d &pos)
{
	if (radius > 0)
	{
		hitBody.ResetFrames(); //in case it happens more than once
		hitBody.BasicCircleSetup(radius * scale, angle, offset * scale);
		hitBody.SetBasicPos(pos, angle);
	}
}

void Enemy::Setup()
{
	SetSpawnRect();
}

void Enemy::BasicCircleHurtBodySetup(double radius)
{
	BasicCircleHurtBodySetup(radius, 0, V2d(), GetPosition());
}

void Enemy::BasicCircleHitBodySetup(double radius)
{
	BasicCircleHitBodySetup(radius, 0, V2d(), GetPosition());
}

void Enemy::BasicRectHurtBodySetup(
	double w, double h, double angle, V2d &offset,
	V2d &pos)
{
	if (w > 0 && h > 0)
	{
		hurtBody.ResetFrames();
		hurtBody.BasicRectSetup(w * scale, h * scale, angle, offset * scale);
		hurtBody.SetBasicPos(pos, angle);
	}
}

void Enemy::BasicRectHurtBodySetup(
	double w, double h, double angle, V2d &offset)
{
	BasicRectHurtBodySetup(w, h, angle, offset, GetPosition());
}

void Enemy::BasicRectHitBodySetup(
	double w, double h, double angle, V2d &offset,
	V2d &pos)
{
	if (w > 0 && h > 0)
	{
		hitBody.ResetFrames();
		hitBody.BasicRectSetup(w * scale, h * scale, angle, offset * scale);
		hitBody.SetBasicPos(pos, angle);
	}
}

void Enemy::BasicRectHitBodySetup(
	double w, double h, double angle, V2d &offset)
{
	BasicRectHitBodySetup(w, h, angle, offset, GetPosition());
}

bool Enemy::IsTouchingSpecterField( SpecterArea *sa )
{
	return WithinDistance(sa->position, GetPosition(), sa->radius);
}

void Enemy::CheckTouchingSpecterField(SpecterArea *sa)
{
	specterProtected = IsTouchingSpecterField(sa);

	if (currShield != NULL)
		currShield->specterProtected = specterProtected;
}

bool Enemy::CanTouchSpecter()
{
	return true;
}

void Enemy::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Enemy::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( spawnRect, r );//r.intersects( spawnRect );// 
}

void Enemy::AddToGame()
{
	SetEnemyIDAndAddToAllEnemiesVec();

	AddToWorldTrees();
	sess->enemyTree->Insert(this);
}

void Enemy::UpdateSpriteFromParams(ActorParams *ap)
{
	//editparams always exists here
	if (ap->posInfo.IsAerial() && ap->type->CanBeGrounded())
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setPosition(editParams->GetFloatPos());
		sprite.setRotation(0);
	}
}


void Enemy::DirectKill()
{
	if (!dead)
	{

		//sess->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_killSpack, GetPosition(), true, 0, 10, 4, true);

		dead = true;

		numHealth = 0;
		HandleNoHealth();
		receivedHit.SetEmpty();

		if (cutObject != NULL)
		{
			SyncCutObject();
			cutObject->SetCutRootPos(GetPositionF());
		}
	}
}

bool Enemy::RightWireHitMe( CollisionBox p_hurtBox )
{
	/*Actor *player = sess->GetPlayer(0);
	if( player->HasUpgrade(Actor::UPGRADE_POWER_RWIRE))
	{
		WireCharge *charge = player->rightWire->activeChargeList;
		while( charge != NULL )
		{
			if( charge->hitbox.Intersects( p_hurtBox ) )
			{
				receivedHit = player->wireChargeInfo;
				receivedHitPlayer = player;
				charge->HitEnemy();
				return true;
			}
			charge = charge->next;
		}
	}*/

	return false;
}

bool Enemy::LeftWireHitMe( CollisionBox p_hurtBox )
{
	/*Actor *player = sess->GetPlayer(0);
	if(player->HasUpgrade(Actor::UPGRADE_POWER_LWIRE))
	{
		WireCharge *charge = player->leftWire->activeChargeList;
		while( charge != NULL )
		{
			if( charge->hitbox.Intersects( p_hurtBox ) )
			{
				receivedHit = player->wireChargeInfo;
				receivedHitPlayer = player;
				charge->HitEnemy();
				return true;
			}
			charge = charge->next;
		}
	}*/

	return false;
}

void Enemy::SyncSpriteInfo(sf::Sprite &dest, sf::Sprite &source)
{
	dest.setTextureRect(source.getTextureRect());
	dest.setOrigin(source.getOrigin());
	dest.setRotation(source.getRotation());
	dest.setScale(source.getScale());
	dest.setPosition(source.getPosition());
}

void Enemy::CheckSpecters()
{
	specterProtected = false;
	if (currShield != NULL)
		currShield->specterProtected = specterProtected;

	if (CanTouchSpecter())
	{
		assert(specterTester != NULL);

		double extra = 5;
		sf::Rect<double> r;
		V2d pos = GetPosition();
		r.left = pos.x - extra;
		r.top = pos.y - extra;
		r.width = extra * 2;
		r.height = extra * 2;

		specterTester->Query(r);
	}
}

void Enemy::UpdatePrePhysics()
{
	if (IsSummoning())
	{
		return;
	}

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePrePhysics();
	}

	receivedHit.SetEmpty();
	pauseBeganThisFrame = false;

	if (pauseFrames > 0)
	{
		--pauseFrames;
		if( pauseFrames > 0 )
			return;
	}

	if ( dead )
		return;

	CheckSpecters();

	ProcessState();
	if (currShield != NULL)
	{
		if (!currShield->ProcessState())
		{
			ShieldDestroyed(currShield);
			currShield = NULL;
		}
	}


	/*bool isCloseEnoughForAddedPrecision = false;

	Actor *player = NULL;
	for (int i = 0; i < 4; ++i)
	{
		player = sess->GetPlayer(i);
		if (player != NULL)
		{
			if (!player->EnemyIsFar(GetPosition()))
			{
				isCloseEnoughForAddedPrecision = true;
				break;
			}
		}
	}

	if (isCloseEnoughForAddedPrecision)
	{
		numPhysSteps = NUM_MAX_STEPS;
	}
	else
	{
		numPhysSteps = NUM_STEPS;
	}

	if (highResPhysics)
	{
		numPhysSteps = NUM_MAX_STEPS;
	}*/

	//this optimization causes some issues with surfacemovers (which is why I now made it that any enemy with a surfacemover/groundmover has
	//highresphysics always turned on). I'm thinking that with comboers and stuff now it might be better to just take the performance hit
	//in exchange for consistency

	numPhysSteps = NUM_MAX_STEPS;
	
}

bool Enemy::LaunchersAreDone()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		if (launchers[i]->activeBullets != NULL)
			return false;
	}

	return true;
}

void Enemy::UpdatePostPhysics()
{
	if (IsSummoning())
	{
		++summonFrame;
		UpdateSprite();
		return;
	}
	//cout << "suppress: " << (int)suppressMonitor << endl;
	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePostPhysics();
	}

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdateSprites();
	}

	if (currShield != NULL)
	{
		if (currShield->data.pauseFrames > 0)
		{
			--currShield->data.pauseFrames;
			//if (UpdateAccountingForSlow()) //shield might have its own at some point
			//{
			//	
			//}
		}
	}

	if (pauseFrames > 0 && !pauseBeganThisFrame )
	{
		return;
	}
	//if (pauseFrames > 0)
	//{
	//	--pauseFrames;
	//	if (pauseFrames > 0)
	//	{
	//		if (UpdateAccountingForSlow())
	//		{

	//		}
	//		return;
	//	}

	//	//cout << "update enemy pause frames: " << pauseFrames << endl;
	//	//return;
	//}

	for (int i = 0; i < 4; ++i)
	{
		if (sess->GetPlayer(i) != NULL)
		{
			SlowCheck(i);
		}
	}
	 //moved here from physics for speed?
	
	if (currShield != NULL)
	{
		if( currShield->data.pauseFrames == 0)
			currShield->ProcessHit();
	}
	else
	{
		ProcessHit();
	}
	

	if (numHealth == 0 && LaunchersAreDone()
		&& ( ( cutObject != NULL && !cutObject->active ) 
			|| cutObject == NULL && dead ) )
	{
		dead = true;
		sess->RemoveEnemy(this);
		
		return;
	}

	if (!dead)
	{
		UpdateKeySprite();
		UpdateSprite();
		if (currShield != NULL)
		{
			currShield->UpdateSprite();
		}

	}
	else if( cutObject != NULL )
		cutObject->UpdateCutObject( slowCounter );

	if (UpdateAccountingForSlow())
	{
		++frame;
		if (cutObject != NULL)
			cutObject->IncrementFrame();
		
		if (!dead)
		{
			FrameIncrement();
			if (currShield != NULL && currShield->data.pauseFrames == 0 )
			{
				currShield->FrameIncrement();
			}
		}
	}
}

void Enemy::UpdateKeySprite()
{
	if (hasMonitor && !suppressMonitor)
	{
		int fac = 5;
		int kFrame = sess->totalGameFrames % (16 * fac);
		keySprite.setTextureRect(ts_key->GetSubRect(kFrame / fac));
		keySprite.setOrigin(keySprite.getLocalBounds().width / 2,
			keySprite.getLocalBounds().height / 2);
		keySprite.setPosition(GetPositionF());
		keySprite.setColor(Color(255, 255, 255, 255));
	}
}

void Enemy::CheckedMiniDraw(sf::RenderTarget *target, sf::FloatRect &rect)
{
	if ( zone == NULL || ( zone != NULL && zone->IsActive() ) )
	{
		if (rect.intersects(GetAABB()))
		{
			DrawMinimap(target);
		}
	}
}

void Enemy::CheckedZoneDraw(sf::RenderTarget *target, sf::FloatRect &rect)
{
	if (zone != NULL && zone->IsShowingEnemyZoneSprites() )
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			UpdateZoneSprite();
			ZoneDraw(target);
		}
	}
}

void Enemy::CheckedZoneUpdate(sf::FloatRect &rect)
{
	if (zone != NULL && !zone->IsActive())
	{
		if (rect.intersects(zonedSprite.getGlobalBounds()))
		{
			UpdateZoneSprite();
		}
	}
}


void Enemy::ProcessShieldHit()
{

}

void Enemy::ProcessHit()
{
	if (!dead && HasReceivedHit())// && numHealth > 0 )
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor )
			{
				//sess->CollectKey();
			}

			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
		}

		receivedHit.SetEmpty();
	}
}
void Enemy::MovePos(V2d &vel,
	int slowMult,
	int numPhysSteps)
{
	V2d movementVec = vel;
	movementVec /= slowMultiple * (double)numPhysSteps;
	currPosInfo.position += movementVec;
}

void Enemy::HandleRemove()
{
	if (summonGroup != NULL)
	{
		summonGroup->HandleSummonedEnemyRemoval( this );
	}
}

void Enemy::ConfirmHitNoKill()
{
	assert(!receivedHit.IsEmpty());

	HitboxInfo::HitboxType hType = receivedHit.hType;
	if (receivedHit.comboer)
	{
		pauseFrames = 5;
	}
	else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
	{
		pauseFrames = 5;
	}
	else
	{
		//sess->Pause(5);
		//pauseFrames = 0;
		//actually gets out of lag a frame after the player
		//if value is set to receivedHit->hitlagFrames;
		pauseFrames = receivedHit.hitlagFrames;// -1;//4;//receivedHit->hitlagFrames;
	}

	pauseBeganThisFrame = true;

	pauseFramesFromAttacking = false;
	
	HandleHitAndSurvive();

	if (!receivedHit.comboer)
	{
		sess->cam.SetRumble(.5, .5, pauseFrames);
	}
}

void Enemy::HandleNoHealth()
{
}

void Enemy::SyncCutObject()
{
	cutObject->SetCutRootPos(GetPositionF());
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->SetScale(scale);
	cutObject->SetRotation(sprite.getRotation());
}

void Enemy::ConfirmKill()
{
	//assert(receivedHit != NULL);


	HitboxInfo::HitboxType hType;
	
	
	if (!receivedHit.IsEmpty())
	{
		hType = receivedHit.hType;
	}
	else
	{
		hType = HitboxInfo::HitboxType::NORMAL;
	}
	if (receivedHit.comboer)
	{
		pauseFrames = 7;
		Enemy *ce = sess->GetEnemyFromID(comboHitEnemyID);
		ce->ComboKill(this);

	}
	else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
	{
		pauseFrames = 7;
	}
	else
	{
		pauseFrames = 7;
		//sess->Pause(7);
		//pauseFrames = 0;
	}
	pauseBeganThisFrame = true;

	pauseFramesFromAttacking = false;

	if (world >= 1)
	{
		if (type != EN_COMBOERTARGET) //exception list to having blood
		{
			sess->ActivateEffect(EffectLayer::BEHIND_ENEMIES, ts_blood, GetPosition(), true, 0, bloodLengths[world - 1], 5, true);
		}
		
	}

	if (!receivedHit.comboer)
	{
		sess->cam.SetRumble(1.5, 1.5, 7);
	}
	
	
	
	if (hasMonitor && !suppressMonitor)
	{
		sess->ActivateAbsorbParticles( AbsorbParticles::AbsorbType::DARK,
			sess->GetPlayer(receivedHitPlayerIndex), GetNumDarkAbsorbParticles(), GetPosition());
	}
	else
	{
		sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::ENERGY,
			sess->GetPlayer(receivedHitPlayerIndex), GetNumEnergyAbsorbParticles(), GetPosition());
	}

	dead = true;

	if (cutObject != NULL)
	{
		SyncCutObject();
	}

	HandleNoHealth();
	PlayDeathSound();
}

void Enemy::ComboHit()
{
	//empty default
}

bool Enemy::CanComboHit(Enemy *e)
{
	return true;
}

void Enemy::ComboKill(Enemy *e)
{
	//empty default
}

void Enemy::Draw(int p_enemyDrawLayer, sf::RenderTarget *target)
{
	if (enemyDrawLayer != p_enemyDrawLayer)
	{
		return;
	}

	if (cutObject != NULL)
	{
		if (dead && cutObject->active )
		{
			cutObject->Draw(target);
		}
		else if( !dead )
		{
			EnemyDraw( target );
			if (currShield != NULL)
			{
				currShield->Draw(target);
			}
		}
	}
	else
	{
		EnemyDraw( target );
		if (currShield != NULL)
		{
			currShield->Draw(target);
		}
	}

	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->Draw(target);
	}
}

void Enemy::UpdateZoneSprite()
{
	//before this didn't include the !data.reexplored line. hopefully no bugs.

	if (zone != NULL && zone->IsStartingToOpen() )//&& zone->data.action == Zone::OPENING && zone->data.frame <= 20 )
	{
		float f = 1.f - zone->GetFrame() / 20.f;
		zonedSprite.setColor(Color(255, 255, 255, f * 255));
	}
	else
	{
		zonedSprite.setColor(Color::White);
	}

	int fr = (sess->totalGameFrames % (12 * 4 )) / 4;

	zonedSprite.setTextureRect(ts_zoned->GetSubRect(fr));
	zonedSprite.setOrigin(zonedSprite.getLocalBounds().width / 2,
		zonedSprite.getLocalBounds().height / 2);
	
}

void Enemy::ZoneDraw(sf::RenderTarget *target)
{
	target->draw(zonedSprite);
}



void Enemy::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		if (hasMonitor && !suppressMonitor)
		{
			CircleShape cs;
			cs.setRadius(50);
			cs.setFillColor(Color::White);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(GetPositionF());
			target->draw(cs);
		}
		else
		{
			CircleShape cs;
			cs.setRadius(40);
			cs.setFillColor(Color::Red);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(GetPositionF());
			target->draw(cs);
		}
	}
}

void Enemy::DebugDraw(sf::RenderTarget *target)
{
	if (!dead)
	{
		for (int i = 0; i < numLaunchers; ++i)
		{
			launchers[i]->DebugDraw(target);
		}


		if( currHitboxes != NULL )
			currHitboxes->DebugDraw( currHitboxFrame, target);
		if( currHurtboxes != NULL )
			currHurtboxes->DebugDraw( currHurtboxFrame, target);
	}
}

void Enemy::UpdateHitboxes()
{
	BasicUpdateHitboxes();
	BasicUpdateHitboxInfo();
}

double Enemy::GetGroundedAngleRadians()
{
	if (surfaceMover != NULL && surfaceMover->ground != NULL )
	{
		return surfaceMover->GetAngleRadians();
	}
	else if (groundMover != NULL && groundMover->ground != NULL )
	{
		return groundMover->GetAngleRadians();
	}
	else
	{
		return currPosInfo.GetGroundAngleRadians();
	}
}

void Enemy::BasicUpdateHitboxes()
{
	V2d position = GetPosition();
	
	double ang = GetGroundedAngleRadians();
	//can update this with a universal angle at some point
	if (!hurtBody.IsEmpty())
	{
		hurtBody.SetBasicPos(position, ang);
		hurtBody.GetCollisionBoxes(0).at(0).flipHorizontal = !facingRight;
	}

	if (!hitBody.IsEmpty())
	{
		hitBody.SetBasicPos(position, ang);
		hitBody.GetCollisionBoxes(0).at(0).flipHorizontal = !facingRight;
	}

	auto comboBoxes = GetComboHitboxes();
	if (comboBoxes != NULL)
	{
		for (auto it = comboBoxes->begin(); it != comboBoxes->end(); ++it)
		{
			(*it).globalPosition = position;
		}
	}
}

void Enemy::StoreBasicEnemyData(StoredEnemyData &ed)
{
	ed.slowableObjectData.slowMultiple = slowMultiple;
	ed.slowableObjectData.slowCounter = slowCounter;
	ed.slowableObjectData.isSlowable = isSlowable;
	ed.hittableObjectData.receivedHit = receivedHit;
	ed.hittableObjectData.receivedHitPlayerIndex = receivedHitPlayerIndex;
	ed.hittableObjectData.comboHitEnemyID = comboHitEnemyID;
	ed.hittableObjectData.numHealth = numHealth;

	currPosInfo.PopulateData(ed.posInfoData);

	ed.facingRight = facingRight;
	ed.action = action;
	ed.frame = frame;
	ed.active = active;

	ed.prevEnemyID = sess->GetEnemyID(prev);
	ed.nextEnemyID = sess->GetEnemyID(next);
	ed.pauseFrames = pauseFrames;
	ed.pauseFramesFromAttacking = pauseFramesFromAttacking;
	ed.dead = dead;
	ed.spawned = spawned;

	ed.scale = scale;

	//cout << "store receivedHit: " << receivedHit << endl;


	if (surfaceMover != NULL)
	{
		surfaceMover->PopulateData(ed.surfaceMoverData);
	}
	else if (groundMover != NULL)
	{
		groundMover->PopulateData(ed.surfaceMoverData, ed.groundMoverData);
	}

	ed.currHitboxesBodyID = GetCollisionBodyID(currHitboxes);
	ed.currHurtboxesBodyID = GetCollisionBodyID(currHurtboxes);

	
	ed.currHitboxFrame = currHitboxFrame;
	ed.currHurtboxFrame = currHurtboxFrame;

	ed.currShieldID = GetShieldID(currShield);
}
void Enemy::SetBasicEnemyData(StoredEnemyData &ed)
{
	slowMultiple = ed.slowableObjectData.slowMultiple;
	slowCounter = ed.slowableObjectData.slowCounter;
	isSlowable = ed.slowableObjectData.isSlowable;
	receivedHit = ed.hittableObjectData.receivedHit;
	receivedHitPlayerIndex = ed.hittableObjectData.receivedHitPlayerIndex;
	comboHitEnemyID = ed.hittableObjectData.comboHitEnemyID;
	numHealth = ed.hittableObjectData.numHealth;

	currPosInfo.PopulateFromData(ed.posInfoData);

	if (surfaceMover != NULL)
	{
		surfaceMover->PopulateFromData(ed.surfaceMoverData);
	}
	else if (groundMover != NULL)
	{
		groundMover->PopulateFromData(ed.surfaceMoverData, ed.groundMoverData);
	}


	currHitboxes = GetCollisionBodyFromID(ed.currHitboxesBodyID);
	currHitboxFrame = ed.currHitboxFrame;
	currHurtboxes = GetCollisionBodyFromID(ed.currHurtboxesBodyID);
	currHurtboxFrame = ed.currHurtboxFrame;

	scale = ed.scale;

	currShield = GetShieldFromID(ed.currShieldID);

	//cout << "set receivedHit: " << receivedHit << endl;

	/*if (currShield != NULL)
	{
		currShield->SetPosition(GetPosition());
	}*/

	//currPosInfo = ed.currPosInfo;
	facingRight = ed.facingRight;
	action = ed.action;
	frame = ed.frame;
	active = ed.active;

	prev = sess->GetEnemyFromID( ed.prevEnemyID );
	next = sess->GetEnemyFromID(ed.nextEnemyID);
	pauseFrames = ed.pauseFrames;
	pauseFramesFromAttacking = ed.pauseFramesFromAttacking;
	dead = ed.dead;
	spawned = ed.spawned;
}

void Enemy::BasicUpdateHitboxInfo()
{
	/*if (hitboxInfo != NULL)
	{
		hitboxInfo->kbDir = sess->GetPlayerKnockbackDirFromVel();
	}*/
}

void Enemy::UpdateEnemyPhysics()
{
	if (numHealth > 0)
	{
		if (surfaceMover != NULL)
		{
			surfaceMover->Move(slowMultiple, numPhysSteps);
		}
		else if (groundMover != NULL)
		{
			groundMover->Move(slowMultiple, numPhysSteps);
		}
	}
}

void Enemy::CheckPlayerInteractions(int substep, int i)
{
	//SlowCheck(i);

	//this is for the player using a
	//sword vs enemies. enough movement must happen
	//from both the player and enemy end to feel natural.
	if (numPhysSteps < NUM_MAX_STEPS || substep >= minSubstepToCheckHits)
	{
		/*if (substep == NUM_MAX_STEPS - 1)
		{
			int xxx = 5;
		}*/
		if (currShield != NULL)
		{
			if (currShield->data.pauseFrames == 0)
				currShield->CheckHit(sess->GetPlayer(i), this);
		}
		else
		{
			CheckHit(sess->GetPlayer(i), this);
		}

		CheckHitPlayer(i);
	}
}

void Enemy::UpdatePhysics( int substep )
{
	if (IsSummoning())
	{
		return;
	}

	bool validSubstep = (substep < numPhysSteps);

	if (validSubstep)
	{
		UpdatePreLauncherPhysics();
	}
	
	for (int i = 0; i < numLaunchers; ++i)
	{
		launchers[i]->UpdatePhysics( substep );
	}

	if (dead || !validSubstep )
	{
		return;
	}

	if (pauseFrames > 0)
	{
		//recently added to fix the issue where a comboer that hits both enemies and player (air mine) would not hit the player
		//until it was done hitting enemies in the area

		//might change the mine to have no pause frames on combo hit anyway, but this change doesn't seem
		//to cause any bugs and seems to fix the issue entirely, so I'm leaving it in for now.
		UpdateHitboxes();

		if (playerIndex == -1)
		{
			Actor *a;
			for (int i = 0; i < Session::MAX_PLAYERS; ++i)
			{
				a = sess->GetPlayer(i);
				if (a != NULL)
				{
					if (numPhysSteps < NUM_MAX_STEPS || substep >= minSubstepToCheckHits)
					{
						CheckHitPlayer(i);
					}
					//CheckPlayerInteractions(substep, i);
				}
			}
		}
		else
		{
			//changed this because you only want to hit the player during pause frames, not be hit
			if (numPhysSteps < NUM_MAX_STEPS || substep >= minSubstepToCheckHits)
			{
				CheckHitPlayer(playerIndex);
			}
			//CheckPlayerInteractions(substep, playerIndex);
		}
		return;
	}
		

	UpdateEnemyPhysics();

	UpdateHitboxes();

	if (playerIndex == -1)
	{
		Actor *a;
		for (int i = 0; i < Session::MAX_PLAYERS; ++i)
		{
			a = sess->GetPlayer(i);
			if (a != NULL)
			{
				CheckPlayerInteractions( substep, i);
			}
		}
	}
	else
	{
		CheckPlayerInteractions( substep, playerIndex);
	}
}

bool Enemy::IsSlowed( int index )
{
	Actor *player = sess->GetPlayer(index);
	return (player->IntersectMySlowboxes(currHurtboxes, currHurtboxFrame));
}

int Enemy::GetSlowFactor(int playerIndex)
{
	Actor *player = sess->GetPlayer(playerIndex);

	return player->GetBubbleTimeFactor();
}

HitboxInfo * Enemy::IsHit(int pIndex )
{
	if (currHurtboxes == NULL)
		return NULL;

	Actor *player = sess->GetPlayer(pIndex);

	if (CanBeHitByPlayer())
	{
		if (player->IntersectMyHitboxes(currHurtboxes, currHurtboxFrame))
		{
			return player->currHitboxes->hitboxInfo;
		}
	}

	if (CanBeHitByComboer())
	{
		ComboObject *co = player->IntersectMyComboHitboxes(this, currHurtboxes, currHurtboxFrame);
		if (co != NULL)
		{
			HitboxInfo *hi = co->enemyHitboxInfo;

			co->enemy->ComboHit();
			comboHitEnemyID = co->enemy->enemyIndex;
			return hi;
		}

	}

	
	Wire *wire = player->IntersectMyWireHitboxes(this, currHurtboxes, currHurtboxFrame);
	if( wire != NULL )
	{
		HandleWireHit(wire);
		return wire->tipHitboxInfo;
	}
	
	return NULL;
}

bool Enemy::BasicCheckHitPlayer(CollisionBody *body, int index)
{
	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;

	if (body != NULL && body->hitboxInfo != NULL)
	{
		V2d playerPos = player->position;
		
		HitboxInfo::HitPosType hpt = body->hitboxInfo->hitPosType;

		
		Actor::HitResult hitResult = player->CheckIfImHitByEnemy( this, body, currHitboxFrame, hpt,
			GetPosition(), IsHitFacingRight(),
			body->hitboxInfo->canBeParried,
			body->hitboxInfo->canBeBlocked);
		//Actor::HitResult hitResult = Actor::HitResult::MISS;


		if (hitResult != Actor::HitResult::MISS)
		{
			//IHitPlayer(index); used to be here
			if (body != NULL && hitResult != Actor::HitResult::INVINCIBLEHIT) //needs a second check in case ihitplayer changes the hitboxes
			{
				if (hitResult == Actor::HitResult::FULLBLOCK)
				{
					IHitPlayerShield(index);
				}
				else
				{
					IHitPlayer(index);
				}

				if (body->hitboxInfo != NULL)
				{
					pauseFrames = body->hitboxInfo->hitlagFrames;
					pauseBeganThisFrame = true;
					pauseFramesFromAttacking = true;
				}
				player->ApplyHit(body->hitboxInfo,
					NULL, hitResult, GetPosition());
			}
		}
	}
}

bool Enemy::CheckHitPlayer(int index)
{
	return BasicCheckHitPlayer(currHitboxes, index );
}

int Enemy::SetLaunchersStartIndex(int ind)
{
	int currIndex = ind;
	for (int i = 0; i < numLaunchers; ++i)
	{
		if (!launchers[i]->drawOwnBullets)
		{
			launchers[i]->SetStartIndex(currIndex);
			currIndex += launchers[i]->totalBullets;
		}
	}

	return currIndex;
}

int Enemy::GetNumEnergyAbsorbParticles()
{ 
	return 6; 
}

int Enemy::GetNumDarkAbsorbParticles()
{
	if (hasMonitor) 
		return 1;
	else 
		return 0;
}

int Enemy::GetNumShardAbsorbParticles()
{ 
	return 0; 
}

Actor *Enemy::GetFocusedPlayer()
{
	double closestDistance = -1;
	int closestIndex = -1;
	Actor *p = NULL;
	if (playerIndex == -1)
	{
		for( int i = 0; i < 4; ++i )
		{
			p = sess->GetPlayer(i);
			if (p != NULL)
			{
				if (closestIndex == -1)
				{
					closestIndex = i;
					closestDistance = PlayerDist(i);
				}
				else
				{
					double currDist = PlayerDist(i);
					if (currDist < closestDistance)
					{
						closestDistance = currDist;
						closestIndex = i;
					}
				}
			}
		}
	}

	assert(closestDistance != -1);

	return sess->GetPlayer(closestIndex);
}

V2d Enemy::GetFocusedPlayerPos()
{
	return GetFocusedPlayer()->position;
}

double Enemy::GetFocusedPlayerDist()
{
	return PlayerDist(GetFocusedPlayer()->actorIndex);
}

V2d Enemy::GetFocusedPlayerDir()
{
	return PlayerDir(GetFocusedPlayer()->actorIndex);
}

void Enemy::RegisterCollisionBody(CollisionBody &cb)
{
	cb.bodyID = bodyPtrVec.size();
	bodyPtrVec.push_back(&cb);
}

void Enemy::RegisterShield(Shield *s)
{
	s->shieldID = shieldPtrVec.size();
	shieldPtrVec.push_back(s);
}

HittableObject::HittableObject()
{
	receivedHit.SetEmpty();
}

int HittableObject::GetReceivedHitPlayerIndex()
{
	return receivedHitPlayerIndex;
}

bool HittableObject::CheckHit( Actor *player, Enemy *e )
{
	if (receivedHit.IsEmpty() && !specterProtected &&
		( e->playerIndex < 0 || e->playerIndex == player->actorIndex) )
	{
		comboHitEnemyID = e->enemyIndex;

		HitboxInfo *hit = IsHit(player->actorIndex);
		if (hit != NULL)
		{
			receivedHit = *hit;
			if (receivedHit.hDir.x == 0 && receivedHit.hDir.y == 0)
			{
				receivedHit.hDir = normalize(e->GetPosition() - player->position);;
			}
		}
		else
		{
			receivedHit.SetEmpty();
		}

		receivedHitPlayerIndex = player->actorIndex;
		if (receivedHit.IsEmpty())
			return false;

		if (receivedHit.hType < HitboxInfo::HitboxType::WIREHITRED && !receivedHit.comboer)
		{
			player->ConfirmHit(e);
		}
		else
		{
			int ff = 0;
			//assert(0);
		}

		return true;
	}
	return false;
}

HitboxInfo * HittableObject::IsHit(int pIndex)
{
	return NULL;
}

const bool HittableObject::HasReceivedHit()
{
	return !receivedHit.IsEmpty();
}

void HittableObject::ProcessHit()
{
}

CuttableObject::CuttableObject()
{
	Reset();
	separateSpeed = .4;
	ts = NULL;
	rectWidth = 32;
	rectHeight = 32;
	scale = 1.f;

	for (int i = 0; i < 2; ++i)
	{
		quads[i * 4 + 0].color = Color::Blue;
		quads[i * 4 + 1].color = Color::Blue;
		quads[i * 4 + 2].color = Color::Blue;
		quads[i * 4 + 3].color = Color::Blue;
	}
}

void CuttableObject::SetSubRectFront( int fIndex )
{
	if (ts != NULL)
	{
		tIndexFront = fIndex;
		IntRect fr = ts->GetSubRect(tIndexFront);
		quads[4].texCoords = Vector2f(fr.left, fr.top);
		quads[5].texCoords = Vector2f(fr.left + fr.width, fr.top);
		quads[6].texCoords = Vector2f(fr.left + fr.width, fr.top + fr.height);
		quads[7].texCoords = Vector2f(fr.left, fr.top + fr.height);

		quads[4].color = Color::White;
		quads[5].color = Color::White;
		quads[6].color = Color::White;
		quads[7].color = Color::White;
	}
}

void CuttableObject::Setup(
	Tileset *ts, int frontIndex,
	int backIndex, double scale,
	double angle, bool flipX, bool flipY)
{
	SetTileset(ts);
	SetSubRectFront(frontIndex);
	SetSubRectBack(backIndex);
	SetScale(scale);
	rotateAngle = scale;
	flipHoriz = flipX;
	flipVert = flipY;
}

void CuttableObject::SetRotation(float rotate)
{
	rotateAngle = rotate;
}


void CuttableObject::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
	rectWidth = ts->tileWidth;
	rectHeight = ts->tileHeight;
}

void CuttableObject::SetSubRectBack( int bIndex )
{
	if (ts != NULL)
	{
		tIndexBack = bIndex;

		IntRect fr = ts->GetSubRect(tIndexBack);
		quads[0].texCoords = Vector2f(fr.left, fr.top);
		quads[1].texCoords = Vector2f(fr.left + fr.width, fr.top);
		quads[2].texCoords = Vector2f(fr.left + fr.width, fr.top + fr.height);
		quads[3].texCoords = Vector2f(fr.left, fr.top + fr.height);

		quads[0].color = Color::White;
		quads[1].color = Color::White;
		quads[2].color = Color::White;
		quads[3].color = Color::White;
	}
}

void CuttableObject::SetCutRootPos(sf::Vector2f &p_rPos )
{
	separateFrame = 0;
	rootPos = p_rPos;
	active = true;
}
bool CuttableObject::DoneSeparatingCut()
{
	return active
		&& separateFrame == totalSeparateFrames;
}

void CuttableObject::Reset()
{
	splitDir = Vector2f(-1, 0);
	separateFrame = 0;
	totalSeparateFrames = 60;
	active = false;
	rotateAngle = 0;
	flipHoriz = false;
	flipVert = false;
}

void CuttableObject::UpdateCutObject( int slowCounter )
{
	Vector2f currSplitDir = splitDir;

	if (active)
	{
		Transform sprT;
		sprT.rotate(rotateAngle);
		
		if (flipHoriz)
		{
			sprT.scale(-1, 1);
		}
		if (flipVert)
		{
			sprT.scale(1, -1);
		}

		currSplitDir = sprT.transformPoint(currSplitDir);

		int halfWidth = rectWidth / 2 * scale;
		int halfHeight = rectHeight / 2 * scale;

		Vector2f root[2];
		root[0] = rootPos + currSplitDir
			* (separateSpeed * (float)separateFrame + (separateSpeed / slowCounter));
		root[1] = rootPos - currSplitDir
			* (separateSpeed * (float)separateFrame + (separateSpeed / slowCounter));

		Vector2f temp;

		float f;
		if (separateFrame <= 30)
		{
			f = 1.f;
		}
		else
		{
			f = 1.f - ( separateFrame - (totalSeparateFrames / 2) ) / (float)(totalSeparateFrames / 2);
		}
		for (int i = 0; i < 2; ++i)
		{
			quads[i * 4 + 0].position = root[i] + sprT.transformPoint(Vector2f(-halfWidth, -halfHeight));
			quads[i * 4 + 1].position = root[i] + sprT.transformPoint(Vector2f(halfWidth, -halfHeight));
			quads[i * 4 + 2].position = root[i] + sprT.transformPoint(Vector2f(halfWidth, halfHeight));
			quads[i * 4 + 3].position = root[i] + sprT.transformPoint( Vector2f(-halfWidth, halfHeight) );

			SetRectColor(quads + i * 4, Color(255, 255, 255, 255 * f));
			
			/*if (flipHoriz)
			{
				temp = quads[i * 4 + 0].position;
				quads[i * 4 + 0].position = quads[i * 4 + 1].position;
				quads[i * 4 + 1].position = temp;

				temp = quads[i * 4 + 3].position;
				quads[i * 4 + 3].position = quads[i * 4 + 2].position;
				quads[i * 4 + 2].position = temp;
			}*/
			

			
		}
	}
}

void CuttableObject::SetScale(float p_scale)
{
	scale = p_scale;
}

void CuttableObject::Draw(sf::RenderTarget *target)
{
	if( ts != NULL )
		target->draw(quads, 8, sf::Quads, ts->texture);
	else
		target->draw(quads, 8, sf::Quads);
}

void CuttableObject::IncrementFrame()
{
	if (active)
	{
		if (separateFrame < totalSeparateFrames)
		{
			++separateFrame;
		}
		else
		{
			active = false;
		}
	}
}


ComboObject::ComboObject(Enemy *en)
	:enemy(en), enemyHitBody(CollisionBox::Hit)
{
	data.enemyHitboxFrame = -1;
	data.nextComboObj = NULL;
	enemyHitboxInfo = NULL;

	comboObjectID = -1;
}

ComboObject::~ComboObject()
{
	if (enemyHitboxInfo != NULL)
		delete enemyHitboxInfo;
}

void ComboObject::Reset()
{
	data.enemyHitboxFrame = 0;
	data.nextComboObj = NULL;
	data.active = false;
}

void ComboObject::SetIDAndAddToAllComboObjectsVec()
{
	comboObjectID = enemy->sess->allComboObjectsVec.size();
	enemy->sess->allComboObjectsVec.push_back(this);
}

V2d ComboObject::GetComboPos()
{
	sf::Rect<double> r = enemyHitBody.GetAABB(data.enemyHitboxFrame);
	
	return V2d(r.left + r.width / 2.0, r.top + r.height / 2.0);
}

void ComboObject::ComboHit()
{

}

void ComboObject::Draw(RenderTarget *target)
{
	enemyHitBody.DebugDraw(data.enemyHitboxFrame, target);
}

int ComboObject::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ComboObject::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void ComboObject::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	bytes += sizeof(MyData);
}

void BasicPathFollower::SetParams(ActorParams *ap)
{
	ap->MakeGlobalPath(path);
	loop = ap->loop;
}

void BasicPathFollower::Reset()
{
	data.targetNode = 1;
	data.forward = true;
}

void BasicPathFollower::AdvanceTargetNode()
{
	int pathLength = path.size();
	if (loop)
	{
		++data.targetNode;
		if (data.targetNode == pathLength)
			data.targetNode = 0;
	}
	else
	{
		if (data.forward)
		{
			++data.targetNode;
			if (data.targetNode == pathLength)
			{
				data.targetNode -= 2;
				data.forward = false;
			}
		}
		else
		{
			--data.targetNode;
			if (data.targetNode < 0)
			{
				data.targetNode = 1;
				data.forward = true;
			}
		}
	}
}

void BasicPathFollower::Move(double movement, V2d &pos )
{
	if (path.size() > 1)
	{
		while (movement != 0)
		{
			V2d targetPoint = V2d(path[data.targetNode].x, path[data.targetNode].y);
			V2d diff = targetPoint - pos;
			double len = length(diff);
			if (len >= abs(movement))
			{
				pos += normalize(diff) * movement;
				movement = 0;
			}
			else
			{
				pos += diff;
				movement -= length(diff);
				AdvanceTargetNode();
			}
		}
	}
}

int BasicPathFollower::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void BasicPathFollower::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void BasicPathFollower::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	bytes += sizeof(MyData);
}

void StoredEnemyData::Print()
{
	/*cout << "slowMultiple: " << storedEneslowMultiple << endl;
	cout << "slowCounter: " << slowCounter << endl;
	cout << "receivedHit: " << receivedHit << endl;
	cout << "receivedHitPlayer: " << receivedHitPlayer << endl;
	cout << "comboHitEnemy: " << comboHitEnemy << endl;
	cout << "numHealth: " << numHealth << endl;
	cout << "facingRight: " << facingRight << endl;
	cout << "action: " << action << endl;
	cout << "frame: " << frame << endl;
	cout << "active: " << active << endl;
	cout << "prev: " << prev << endl;
	cout << "next: " << next << endl;
	cout << "pauseFrames: " << pauseFrames << endl;
	cout << "pauseFramesFromAttacking: " << pauseFramesFromAttacking << endl;
	cout << "dead: " << dead << endl;
	cout << "spawned: " << spawned << endl;
	cout << "currHitboxFrame: " << currHitboxFrame << endl;
	cout << "currHurtboxFrame: " << currHurtboxFrame << endl;
	cout << "currHitboxes: " << currHitboxes << endl;
	cout << "currHurtboxes: " << currHurtboxes << endl;*/
}