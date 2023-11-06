#ifndef __ACTOR_H__
#define __ACTOR_H__

#include <list>
#include <map>
#include <SFML/Audio.hpp>
#include "Tileset.h"
#include "Physics.h"
#include "Input.h"
#include <SFML/Graphics.hpp>
//#include "Wire.h"
#include "Light.h"
#include "AirParticles.h"
#include "Movement.h"
#include "Gate.h"
#include "SpecialTerrainTypes.h"
#include "EffectLayer.h"
#include "BitField.h"
#include "VectorMath.h"
#include <unordered_map>
#include "nlohmann\json.hpp"
#include "Grass.h"
#include <iostream>
#include "EnemyTracker.h"
#include "PlayerSkinShader.h"

struct AdventureManager;

struct SoundInfo;
struct PaletteShader;

struct SwordProjectile;

struct PState;

struct EditSession;
struct EffectInstance;

struct Session;
struct TerrainPolygon;

struct AirTrigger;
struct Gate;
struct GameSession;
struct PlayerGhost;
struct Monitor;
struct Enemy;
struct Aura;
//struct Rail;
struct ComboObject;
struct Actor;
struct ObjectPool;
struct RelEffectInstance;
struct VertexBuf;
struct Kin;
struct MovingGeoGroup;
struct HealthFly;

struct TerrainRail;
struct Nexus;

struct TutorialObject;
struct MotionGhostEffect;


//eventually make this an objectpool but don't need to for now

struct AbsorbParticles;
struct GravityModifier;
struct MomentumBooster;
struct Booster;
struct ScorpionLauncher;
struct BounceBooster;
struct TimeBooster;
struct FreeFlightBooster;
struct HomingBooster;
struct AntiTimeSlowBooster;
struct RewindBooster;
struct SwordProjectileBooster;
struct Spring;
struct Teleporter;
struct SwingLauncher;
struct EnemyParams;
struct SoundNode;
struct AimLauncher;
struct BasicBullet;
struct GroundTrigger;

struct KinMask;
struct RisingParticleUpdater;
struct EffectPool;
struct KeyExplodeUpdater;
struct ShapeEmitter;
struct PlayerBoosterEffectEmitter;

struct BasicEffect;
struct PlayerEffect;

struct Wire;

struct BossCommand;
struct NameTag;

using json = nlohmann::json;

struct Actor : QuadTreeCollider,
	RayCastHandler, EnemyTracker
{
	enum Mode
	{
		K_NORMAL,
		K_SUPER,
		K_DESPERATION,
	};

	enum UpgradeType
	{
		UPGRADE_POWER_AIRDASH,
		UPGRADE_POWER_GRAV,
		UPGRADE_POWER_BOUNCE,
		UPGRADE_POWER_GRIND,
		UPGRADE_POWER_TIME,
		UPGRADE_POWER_RWIRE,
		UPGRADE_POWER_LWIRE,

		//W1
		UPGRADE_W1_DECREASE_ENEMY_DAMAGE,
		UPGRADE_W1_INCREASE_ENEMY_REGEN,
		UPGRADE_W1_INCREASE_ENEMY_MOMENTUM,
		UPGRADE_W1_INCREASE_STARTING_MOMENTUM,

		UPGRADE_W1_DASH_BOOST,
		UPGRADE_W1_STEEP_CLIMB_1,
		UPGRADE_W1_STEEP_SLIDE_1,
		UPGRADE_W1_PASSIVE_GROUND_1,
		UPGRADE_W1_SPRINT_1,
		UPGRADE_W1_BASE_DASH_1,

		UPGRADE_W1_EMPTY_1,
		UPGRADE_W1_EMPTY_2,
		UPGRADE_W1_EMPTY_3,
		UPGRADE_W1_EMPTY_4,
		UPGRADE_W1_EMPTY_5,
		UPGRADE_W1_EMPTY_6,

		//W2
		UPGRADE_W2_DECREASE_ENEMY_DAMAGE,
		UPGRADE_W2_INCREASE_ENEMY_REGEN,
		UPGRADE_W2_INCREASE_ENEMY_MOMENTUM,
		UPGRADE_W2_INCREASE_STARTING_MOMENTUM,

		UPGRADE_W2_AIRDASH_BOOST,
		UPGRADE_W2_STEEP_CLIMB_2,
		UPGRADE_W2_STEEP_SLIDE_2,
		UPGRADE_W2_PASSIVE_GROUND_2,
		UPGRADE_W2_SPRINT_2,
		UPGRADE_W2_BASE_AIRDASH_1,

		UPGRADE_W2_EMPTY_1,
		UPGRADE_W2_EMPTY_2,
		UPGRADE_W2_EMPTY_3,
		UPGRADE_W2_EMPTY_4,
		UPGRADE_W2_EMPTY_5,
		UPGRADE_W2_EMPTY_6,

		//W3
		UPGRADE_W3_DECREASE_ENEMY_DAMAGE,
		UPGRADE_W3_INCREASE_ENEMY_REGEN,
		UPGRADE_W3_INCREASE_ENEMY_MOMENTUM,
		UPGRADE_W3_INCREASE_STARTING_MOMENTUM,

		UPGRADE_W3_CEILING_DROP_AERIAL,
		UPGRADE_W3_CEILING_STEEP_CLIMB_1,
		UPGRADE_W3_CEILING_STEEP_SLIDE_1,
		UPGRADE_W3_CEILING_PASSIVE_GROUND_1,
		UPGRADE_W3_CEILING_SPRINT_1,
		UPGRADE_W3_BASE_DASH_2,

		UPGRADE_W3_EMPTY_1,
		UPGRADE_W3_EMPTY_2,
		UPGRADE_W3_EMPTY_3,
		UPGRADE_W3_EMPTY_4,
		UPGRADE_W3_EMPTY_5,
		UPGRADE_W3_EMPTY_6,

		//W4
		UPGRADE_W4_DECREASE_ENEMY_DAMAGE,
		UPGRADE_W4_INCREASE_ENEMY_REGEN,
		UPGRADE_W4_INCREASE_ENEMY_MOMENTUM,
		UPGRADE_W4_INCREASE_STARTING_MOMENTUM,

		UPGRADE_W4_SCORPION_ATTACK,
		UPGRADE_W4_SCORPION_JUMP,
		UPGRADE_W4_SCORPION_DOUBLE_JUMP,
		UPGRADE_W4_SCORPION_BOUNCE,
		UPGRADE_W4_CEILING_STEEP_CLIMB_2,
		UPGRADE_W4_CEILING_STEEP_SLIDE_2,
		UPGRADE_W4_CEILING_PASSIVE_GROUND_2,
		UPGRADE_W4_CEILING_SPRINT_2,

		UPGRADE_W4_EMPTY_1,
		UPGRADE_W4_EMPTY_2,
		UPGRADE_W4_EMPTY_3,
		UPGRADE_W4_EMPTY_4,

		//W5
		UPGRADE_W5_DECREASE_ENEMY_DAMAGE,
		UPGRADE_W5_INCREASE_ENEMY_REGEN,
		UPGRADE_W5_INCREASE_ENEMY_MOMENTUM,
		UPGRADE_W5_INCREASE_STARTING_MOMENTUM,

		UPGRADE_W5_GRIND_LUNGE,
		UPGRADE_W5_SLOW_RESISTANCE,
		UPGRADE_W5_CEILING_STEEP_CLIMB_3,
		UPGRADE_W5_CEILING_STEEP_SLIDE_3,
		UPGRADE_W5_CEILING_PASSIVE_GROUND_3,
		UPGRADE_W5_CEILING_SPRINT_3,
		UPGRADE_W5_BASE_AIRDASH_2,

		UPGRADE_W5_EMPTY_1,
		UPGRADE_W5_EMPTY_2,
		UPGRADE_W5_EMPTY_3,
		UPGRADE_W5_EMPTY_4,
		UPGRADE_W5_EMPTY_5,

		//W6
		UPGRADE_W6_DECREASE_ENEMY_DAMAGE,
		UPGRADE_W6_INCREASE_ENEMY_REGEN,
		UPGRADE_W6_INCREASE_ENEMY_MOMENTUM,
		UPGRADE_W6_INCREASE_STARTING_MOMENTUM,

		UPGRADE_W6_BUBBLE_AIRDASH,
		UPGRADE_W6_BUBBLE_SIZE,
		UPGRADE_W6_EXTRA_BUBBLES_1,
		UPGRADE_W6_EXTRA_BUBBLES_2,
		UPGRADE_W6_BASE_DASH_3,
		UPGRADE_W6_BASE_AIRDASH_3,
		UPGRADE_W6_STEEP_CLIMB_3,
		UPGRADE_W6_STEEP_SLIDE_3,
		UPGRADE_W6_PASSIVE_GROUND_3,
		UPGRADE_W6_SPRINT_3,

		UPGRADE_W6_EMPTY_1,
		UPGRADE_W6_EMPTY_2,


		UPGRADE_Count,
	};

	enum QueryType
	{
		Q_RESOLVE,
		Q_CHECK,
		Q_CHECK_GRIND_TRANSFER,
		Q_CHECKWALL,
		Q_CHECK_GATE,
		Q_GRASS,
		Q_RAIL_GRIND_TERRAIN_CHECK,
		Q_TERRAIN_GRIND_RAIL_CHECK,
		Q_ENVPLANT,
		Q_RAIL,
		Q_ACTIVEITEM,
		Q_AIRTRIGGER,
		Q_TOUCHGRASS,
		Q_TOUCHGRASSPOLY,
		Q_SPECIALTERRAIN,
	};

	enum Action
	{
		DAIR,
		DASH,
		DOUBLE,
		BACKWARDSDOUBLE,
		FAIR,
		JUMP,
		LAND,
		LAND2,
		RUN,
		SLIDE,
		BRAKE,
		SPRINT,
		STAND,
		DASHATTACK,
		DASHATTACK2,
		DASHATTACK3,
		WALLATTACK,
		STEEPCLIMBATTACK,
		STEEPSLIDEATTACK,
		STEEPCLING,
		STANDATTACK1,
		STANDATTACK2,
		STANDATTACK3,
		STANDATTACK4,
		UAIR,
		WALLCLING,
		WALLJUMP,
		GROUNDTECHBACK,
		GROUNDTECHFORWARD,
		GROUNDTECHINPLACE,
		WALLTECH,
		WATERGLIDE,
		WATERGLIDE_HITSTUN,
		WATERGLIDECHARGE,
		STEEPSLIDE,
		GRAVREVERSE,
		GRINDBALL,
		RAILGRIND,
		RAILSLIDE,
		LOCKEDRAILSLIDE,
		RAILDASH,
		GRINDLUNGE,
		GRINDSLASH,
		GRINDATTACK,
		AIRDASH,
		STEEPCLIMB,
		BOOSTERBOUNCE,
		BOOSTERBOUNCEGROUND,
		RAILBOUNCE,
		RAILBOUNCEGROUND,
		AIRHITSTUN,
		AIRPARRY,
		GROUNDHITSTUN,
		WIREHOLD,
		BOUNCEAIR,
		BOUNCEGROUND,
		BOUNCEGROUNDEDWALL,
		JUMPSQUAT,
		INTRO,
		INTROBOOST,
		EXIT,
		EXITWAIT,
		EXITBOOST,
		GOALKILL,
		GOALKILL1,
		GOALKILL2,
		GOALKILL3,
		GOALKILL4,
		GOALKILLWAIT,
		NEXUSKILL,
		SPAWNWAIT,
		DEATH,
		RIDESHIP,
		SKYDIVE,
		SKYDIVETOFALL,
		WAITFORSHIP,
		GRABSHIP,
		DIAGUPATTACK,
		DIAGDOWNATTACK,
		AIRDASHFORWARDATTACK,
		SPRINGSTUN,
		SPRINGSTUNGLIDE,
		SPRINGSTUNBOUNCEGROUND,
		SPRINGSTUNBOUNCEWALL,
		SPRINGSTUNBOUNCE,
		SPRINGSTUNGRIND,
		SPRINGSTUNGRINDFLY,
		SPRINGSTUNANNIHILATION,
		SPRINGSTUNANNIHILATIONATTACK,
		AIMWAIT,
		SPRINGSTUNAIM,
		SPRINGSTUNAIRBOUNCE,
		SPRINGSTUNAIRBOUNCEPAUSE,
		SPRINGSTUNTELEPORT,
		SPRINGSTUNHOMING,
		SPRINGSTUNHOMINGATTACK,
		HOMINGATTACK,
		FREEFLIGHT,
		FREEFLIGHTSTUN,
		SWINGSTUN,
		GLIDE,
		SEQ_ENTERCORE1,
		SEQ_CRAWLERFIGHT_STRAIGHTFALL,
		SEQ_CRAWLERFIGHT_LAND,
		SEQ_CRAWLERFIGHT_STAND,
		SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY,
		SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED,
		SEQ_CRAWLERFIGHT_DODGEBACK,
		SEQ_WAIT,
		GETPOWER_AIRDASH_MEDITATE,
		GETPOWER_AIRDASH_FLIP,
		ENTERNEXUS1,
		AUTORUN,
		SEQ_LOOKUP,
		SEQ_LOOKUPDISAPPEAR,
		SEQ_KINTHROWN,
		SEQ_KINFALL,
		SEQ_KINSTAND,
		SEQ_KNEEL,
		SEQ_KNEEL_TO_MEDITATE,
		SEQ_MEDITATE_MASKON,
		SEQ_MASKOFF,
		SEQ_MEDITATE,
		SEQ_FLOAT_TO_NEXUS_OPENING,
		SEQ_FADE_INTO_NEXUS,
		SEQ_TURNFACE,
		GETSHARD,
		SUPERBIRD,
		TESTSUPER,
		TELEPORTACROSSTERRAIN,
		GROUNDBLOCKDOWN,
		GROUNDBLOCKDOWNFORWARD,
		GROUNDBLOCKFORWARD,
		GROUNDBLOCKUPFORWARD,
		GROUNDBLOCKUP,
		AIRBLOCKUP,
		AIRBLOCKUPFORWARD,
		AIRBLOCKFORWARD,
		AIRBLOCKDOWNFORWARD,
		AIRBLOCKDOWN,
		GROUNDPARRY,
		GROUNDPARRYLOW,
		UPTILT1,
		UPTILT2,
		UPTILT3,
		SEQ_GATORSTUN,
		HIDDEN,
		Count
	};

	enum PlayerEffectType
	{
		PLAYERFX_FAIR_SWORD_LIGHTNING_0,
		PLAYERFX_FAIR_SWORD_LIGHTNING_1,
		PLAYERFX_FAIR_SWORD_LIGHTNING_2,
		PLAYERFX_DAIR_SWORD_LIGHTNING_0,
		PLAYERFX_DAIR_SWORD_LIGHTNING_1,
		PLAYERFX_DAIR_SWORD_LIGHTNING_2,
		PLAYERFX_UAIR_SWORD_LIGHTNING_0,
		PLAYERFX_UAIR_SWORD_LIGHTNING_1,
		PLAYERFX_UAIR_SWORD_LIGHTNING_2,
		PLAYERFX_BOUNCE_BOOST,
		PLAYERFX_HURT_SPACK,
		PLAYERFX_DASH_START,
		PLAYERFX_DASH_REPEAT,
		PLAYERFX_LAND_0,
		PLAYERFX_LAND_1,
		PLAYERFX_LAND_2,
		PLAYERFX_RUN_START,
		PLAYERFX_SPRINT_0,
		PLAYERFX_SPRINT_1,
		PLAYERFX_SPRINT_2,
		PLAYERFX_RUN,
		PLAYERFX_JUMP_0,
		PLAYERFX_JUMP_1,
		PLAYERFX_JUMP_2,
		PLAYERFX_WALLJUMP_0,
		PLAYERFX_WALLJUMP_1,
		PLAYERFX_WALLJUMP_2,
		PLAYERFX_DOUBLE,
		PLAYERFX_GRAV_REVERSE,
		PLAYERFX_SPEED_LEVEL_CHARGE,
		PLAYERFX_RIGHT_WIRE_BOOST,
		PLAYERFX_LEFT_WIRE_BOOST,
		PLAYERFX_DOUBLE_WIRE_BOOST,
		PLAYERFX_AIRDASH_DIAGONAL,
		PLAYERFX_AIRDASH_UP,
		PLAYERFX_AIRDASH_HOVER,
		/*PLAYERFX_DEATH_1A,
		PLAYERFX_DEATH_1B,
		PLAYERFX_DEATH_1C,
		PLAYERFX_DEATH_1D,
		PLAYERFX_DEATH_1E,
		PLAYERFX_DEATH_1F,*/
		PLAYERFX_GATE_ENTER,
		PLAYERFX_SMALL_LIGHTNING,
		PLAYERFX_GATE_BLACK,
		PLAYERFX_KEY,
		PLAYERFX_KEY_EXPLODE,
		PLAYERFX_DASH_BOOST,
		PLAYERFX_SPRINT_STAR,
		PLAYERFX_LAUNCH_PARTICLE_0,
		PLAYERFX_LAUNCH_PARTICLE_1,
		PLAYERFX_ENTER,
		PLAYERFX_EXITENERGY_0,
		PLAYERFX_EXITENERGY_1,
		PLAYERFX_EXITENERGY_2,
		PLAYERFX_Count,
	};

	struct FXInfo
	{
		EffectLayer layer;
		EffectPool *pool;
		bool pauseImmune;
		bool usesPlayerSkinShader;
		
		//int startFrame;
		//int duration;
		//int animFactor;


		FXInfo();
		~FXInfo();
		void Set(Tileset *ts, int fxType, int maxEffects, EffectLayer effectLayer = EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
			bool p_pauseImmune = false, bool p_usesPlayerSkinShader = true );

		/*void Set(Tileset *p_ts, int p_startFrame, int p_duration, int p_animFactor, int p_maxReps)
		{
			ts = p_ts;
			startFrame = p_startFrame;
			duration = p_duration;
			animFactor = p_animFactor;
		}*/

	};

	
	std::vector<FXInfo> effectPools;
	PaletteShader *fxPaletteShader;


	enum AirTriggerBehavior
	{
		AT_NONE,
		AT_AUTORUNRIGHT,
	};

	enum Team
	{
		T_BLUE,
		T_RED,
		T_GREEN,
		T_PURPLE
	};

	enum HitResult : int
	{
		MISS,
		HIT,
		FULLBLOCK,
		HALFBLOCK,
		PARRY,
		INVINCIBLEHIT,
	};

	enum PowerMode
	{
		PMODE_SHIELD,
		PMODE_BOUNCE,
		PMODE_GRIND,
		PMODE_TIMESLOW,
		PMODE_Count
	};

	enum RayMode
	{
		RAYMODE_WATER,
	};

	enum SkinTypes
	{
		SKIN_NORMAL,
		SKIN_RED,
		SKIN_BLUE,
		SKIN_ORANGE,
		SKIN_PINK,
		SKIN_LIGHT,
		SKIN_GHOST,
		SKIN_DARK,
		SKIN_VILLAIN,
		SKIN_TOXIC,
		SKIN_AMERICA,
		SKIN_METAL,
		SKIN_GOLD,
		SKIN_PURPLE,
		SKIN_MAGI,
		SKIN_GLIDE,
		SKIN_BONFIRE,
		SKIN_GDUBS,
		SKIN_SHADOW,
		SKIN_DUSK,
		SKIN_DAWN,
		SKIN_TRIX,
		SKIN_Count
	};

	
	PlayerBoosterEffectEmitter *gravityIncreaserTrailEmitter;
	PlayerBoosterEffectEmitter *gravityDecreaserTrailEmitter;
	PlayerBoosterEffectEmitter *momentumBoosterTrailEmitter;
	PlayerBoosterEffectEmitter *homingBoosterTrailEmitter;
	PlayerBoosterEffectEmitter *antiTimeSlowBoosterTrailEmitter;
	PlayerBoosterEffectEmitter *freeFlightBoosterTrailEmitter;

	//PlayerBoosterEffectEmitter *timeSlowBoosterTrailEmitter;

	V2d practiceDesyncPosition;
	bool practiceDesyncDetected;

	double normalWaterMaxFallSpeed;

	NameTag *nameTag;

	const static int SHARD_START_INDEX = UPGRADE_POWER_LWIRE + 1;

	AdventureManager *adventureManager;

	const static int MAX_BUBBLES = 5;

	const static int MAX_FRAMES_TO_LIVE = (100 * 60 * 60) - 1;

	PState *pState;

	//havent put into rollback yet
	int numCalculatedFuturePositions;
	V2d* futurePositions;
	PState *preSimulationState;

	int currFrameSimulationFrames;

	//--empty for now

	//Enemy *receivedHitEnemy;
	
	struct Hitter
	{
		/*enum HitterType
		{
			HT_ENEMY,
			HT_BULLET,
		};*/

		int id;
		int launcherID; //launcherID

		//void *info;
		int framesToStayInArray;

		void SetEnemy( int enemyID );
		void SetBullet(int bulletID, int launcherID);
		bool CheckEnemy(Enemy *e);
		bool CheckBullet(BasicBullet *b);

		void Clear();
		void Update();
	};

	const static int MAX_HITTERS = 16;
	Hitter recentHitters[MAX_HITTERS];

	//---

	bool hitCeilingSoundPlayedThisFrame;

	int numFramesToLive;

	TerrainRail * prevRail;
	
	bool specialSlow;
	bool frameAfterAttackingHitlagOver;
	bool bouncedFromKill;
	const static int NUM_SWORD_PROJECTILES = 6;
	SwordProjectile *swordProjectiles[NUM_SWORD_PROJECTILES];
	int projectileSwordFrames;
	int enemyProjectileSwordFrames;
	int gravModifyFrames;
	int boosterGravModifyFrames;
	V2d waterEntrancePosition;
	Edge *waterEntranceGround;
	Edge *waterEntranceGrindEdge;
	double waterEntranceQuantity;
	double waterEntranceXOffset;
	double waterEntrancePhysHeight;
	bool waterEntranceFacingRight;
	double waterEntranceGrindSpeed;
	bool waterEntranceReversed;
	V2d rewindBoosterPos;
	int rewindOnHitFrames;
	int currSkinIndex;

	V2d waterEntranceVelocity;
	int invertInputFrames;
	int currPowerMode;
	PolyPtr oldSpecialTerrain;
	PolyPtr currSpecialTerrain;
	int startGlobalTimeSlowFrames;
	int startFreeFlightFrames;
	int startHomingFrames;
	int startAntiTimeSlowFrames;
	int startMomentumBoostFrames;
	int startBoosterGravModifyFrames;

	int globalTimeSlowFrames;
	int freeFlightFrames;
	int homingFrames;
	int antiTimeSlowFrames;
	TimeBooster *currTimeBooster;
	FreeFlightBooster *currFreeFlightBooster;
	HomingBooster *currHomingBooster;
	AntiTimeSlowBooster *currAntiTimeSlowBooster;
	SwordProjectileBooster *currSwordProjectileBooster;
	MomentumBooster *currMomentumBooster;
	RewindBooster *currRewindBooster;
	int aimLauncherStunFrames;
	int airBounceCounter;
	int airBounceLimit;
	int momentumBoostFrames;

	Spring *currSpring;
	AimLauncher *currAimLauncher;
	Teleporter *currTeleporter;
	Teleporter *oldTeleporter;
	Booster *currBooster;
	Booster *oldBooster;
	SwingLauncher *currSwingLauncher;
	SwingLauncher *oldSwingLauncher;
	BounceBooster *currBounceBooster;
	BounceBooster *oldBounceBooster;
	int directionalInputFreezeFrames;
	TutorialObject *currTutorialObject;
	ScorpionLauncher *currScorpionLauncher;
	ScorpionLauncher *oldScorpionLauncher;


	
	
	
	
	//bool oldTouchedGrass[Grass::GrassType::Count];
	//^this needs to sync too

	//new but synced
	Mode kinMode;
	int superFrame;

	//dont sync
	int rayMode;
	



	//stuff ive put in rollback already
	//definitely do change per frame
	
	double offsetX;
	int framesSinceClimbBoost;
	bool holdDouble;
	int frame;
	int action;
	V2d position;
	V2d velocity;
	bool holdJump;
	int wallJumpFrameCounter;
	double groundSpeed;
	bool facingRight;
	bool hasDoubleJump;
	Edge *ground;
	double edgeQuantity;
	int framesInAir;
	CollisionBox b;
	bool hasAirDash;
	int numRemainingExtraAirdashBoosts;
	int speedLevel; //0,1,2
	double currentSpeedBar;
	double storedGroundSpeed;
	
	int currBBoostCounter;
	int currAirdashBoostCounter;
	bool steepJump;
	// 0 is not started, 1 is right, 2 is
	//left
	bool airDashStall;
	V2d startAirDashVel;
	double extraAirDashY;
	int oldAction;
	V2d oldVelocity;
	bool reversed;
	double storedReverseSpeed;

	double grindActionCurrent;
	int framesGrinding;
	int framesNotGrinding;
	int framesSinceGrindAttempt;
	int maxFramesSinceGrindAttempt;
	Edge *grindEdge;
	double grindSpeed;

	int slowMultiple;
	int slowCounter;
	bool inBubble;
	bool oldInBubble;


	V2d bubblePos[MAX_BUBBLES];
	int bubbleFramesToLive[MAX_BUBBLES];
	int bubbleRadiusSize[MAX_BUBBLES];
	CollisionBody bubbleHitboxes[MAX_BUBBLES];
	int currBubble;

	bool currAttackHit;
	
	bool bounceAttackHit;
	int flashFrames;
	int bufferedAttack;
	int doubleJumpBufferedAttack;
	int wallJumpBufferedAttack;
	int pauseBufferedAttack;
	bool pauseBufferedJump;
	bool pauseBufferedDash;

	//bool moretest;
	bool attackingHitlag; //sometimes causes memory problems
	//some kind of earlier memory problem.
	int hitlagFrames;
	int hitstunFrames;
	int setHitstunFrames;
	int invincibleFrames;
	HitboxInfo receivedHit;
	HitboxInfo hitGrassHitInfo;
	
	

	CollisionBody *currHitboxes;
	int currHitboxFrame;
	int cancelAttack;

	bool dairBoostedDouble;
	bool aerialHitCancelDouble;

	CollisionBox hurtBody;

	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;
	V2d dWireAirDash;
	V2d dWireAirDashOld;

	bool scorpOn;
	int framesFlameOn;
	bool bounceFlameOn;
	V2d storedBounceVel;
	V2d bounceNorm;
	V2d oldBounceNorm;
	Edge *bounceEdge;
	double storedBounceGroundSpeed;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;
	bool bounceGrounded;
	bool justToggledBounce;

	int dashAttackLevel;
	int standAttackLevel;
	int upTiltLevel;
	int downTiltLevel;
	int framesSinceDashAttack;
	int framesSinceStandAttack;
	int framesSinceUpTilt;
	int framesSinceDownTilt;
	
	

	bool standNDashBoost;
	int standNDashBoostCurr;
	bool hasFairAirDashBoost;
	int framesStanding;
	int framesSinceRightWireBoost;
	int framesSinceLeftWireBoost;
	int framesSinceDoubleWireBoost;
	int enemiesKilledThisFrame;
	int enemiesKilledLastFrame;

	double hitstunGravMultiplier;

	int framesSinceBlockPress;
	int framesSinceSuperPress;
	int superLevelCounter;
	int currActionSuperLevel;

	int blockstunFrames;
	int currAttackHitBlock[4];
	Actor *receivedHitPlayer;
	HitResult receivedHitReaction;
	
	
	bool hasWallJumpRechargeDoubleJump;
	bool hasWallJumpRechargeAirDash;
	bool hasHitRechargeDoubleJump;
	bool hasHitRechargeAirDash;

	int framesBlocking;
	V2d receivedHitPosition;

	std::vector<BossCommand*> birdCommands;
	int currBirdCommandIndex;

	bool hitOutOfHitstunLastFrame;

	int shieldPushbackFrames;
	bool shieldPushbackRight;

	bool touchedCoyoteHelper;
	int coyoteBoostFrames;

	//int pastCompressedInputs[60];

	//---end of saved vars
	//const static int NUM_PAST_INPUTS = 60;

	PlayerSkinShader skinShader;
	PlayerSkinShader exitAuraShader;

	PlayerSkinShader swordShader;

	int superActiveLimit;
	int attackLevelCounterLimit;
	sf::Vector2f fairSwordOffset[3];
	sf::Vector2f dairSwordOffset[3];
	sf::Vector2f diagUpSwordOffset[3];
	sf::Vector2f diagDownSwordOffset[3];
	sf::Vector2f standSwordOffset[3];
	sf::Vector2f dashAttackSwordOffset[3];
	sf::Vector2f slideAttackOffset[3];
	sf::Vector2f climbAttackOffset[3];

	sf::Color fullBlockShieldColor;
	sf::Color halfBlockShieldColor;
	sf::Shader shieldShader;
	sf::Sprite shieldSprite;
	sf::Sprite grindAttackSprite;
	sf::Sprite homingAttackBallSprite;
	Tileset *ts_blockShield;
	Tileset *ts_homingAttackBall;
	
	std::vector<SoundInfo*> soundInfos;
	Tileset *ts_exitAura;
	Tileset *ts_antiTimeSlowRing;
	sf::Sprite boosterRingSprite;
	
	sf::Shader despFaceShader;
	sf::Shader playerDespShader;
	sf::Shader playerSuperShader;
	sf::Shader auraTestShader;
	bool showExitAura;
	BitField bStartHasUpgradeField;
	BitField originalProgressionUpgradeField;
	BitField originalProgressionLogField;
	bool canStandUp;
	Tileset *ts_kinFace;
	sf::Sprite exitAuraSprite;
	std::vector<void(Actor::*)()> startActionFuncs;
	std::vector<void(Actor::*)()> endActionFuncs;
	std::vector<void(Actor::*)()> changeActionFuncs;
	std::vector<void(Actor::*)()> updateActionFuncs;
	std::vector<void(Actor::*)()> updateSpriteFuncs;
	std::vector<void(Actor::*)(int)> transitionFuncs;
	std::vector<void(Actor::*)()> timeIndFrameIncFuncs;
	std::vector<void(Actor::*)()> timeDepFrameIncFuncs;
	std::vector<int(Actor::*)()> getActionLengthFuncs;
	std::vector<Tileset*(Actor::*)()> getTilesetFuncs;
	EffectPool *risingAuraPool;
	MotionGhostEffect *motionGhostsEffects[3];
	EffectPool *keyExplodePool;
	KeyExplodeUpdater *keyExplodeUpdater;
	MovingGeoGroup *keyExplodeRingGroup;
	MovingGeoGroup *enemyExplodeRingGroup;
	MovingGeoGroup *enemiesClearedRingGroup;
	MovingGeoGroup *enoughKeysToExitRingGroup;
	MovingGeoGroup *gravityDecreaserOnRingGroup;
	MovingGeoGroup *gravityDecreaserOffRingGroup;

	EffectPool *dustParticles;
	RisingParticleUpdater *rpu;
	GroundTrigger *storedTrigger;
	AirTrigger *currAirTrigger;
	AirTriggerBehavior airTrigBehavior;
	//might need more repeating sounds
	//in future, but for now this works.
	SoundNode *repeatingSound;
	Team team;
	int spriteAction;
	int currTileIndex;
	bool flipTileX;
	bool flipTileY;
	sf::Vector2<double> spriteCenter;
	double dairBoostVel;
	double standNDashBoostQuant;
	int standNDashBoostCooldown;
	double fairAirDashBoostQuant;
	//these are for the wire boost particles 
	sf::Vector2<double> leftWireBoostDir;
	sf::Vector2<double> rightWireBoostDir;
	sf::Vector2<double> doubleWireBoostDir;
	int singleWireBoostTiming;
	int doubleWireBoostTiming;
	bool leftWireBoost;
	bool rightWireBoost;
	bool doubleWireBoost;
	std::string actionFolder;
	bool toggleBounceInput;
	bool toggleTimeSlowInput;
	bool toggleGrindInput;
	bool hitGoal;
	Nexus *hitNexus;

	sf::Sprite dodecaSprite;
	Gate *gateTouched;
	double level1SpeedThresh;
	double level2SpeedThresh;
	double speedChangeUp;
	double speedChangeDown;
	//EffectPool *testPool;
	//ShapeEmitter *glideEmitter;

	BitField bHasUpgradeField;
	int numKeysHeld;	
	KinMask *kinMask;
	sf::Color currentDespColor;
	//KinRing *kinRing;
	
	
	
	
	
	

	//unsaved vars
	double maxFallSpeedWhileHitting;
	bool simulationMode;
	int possibleEdgeCount;
	GameSession *owner;
	EditSession *editOwner;
	double steepClimbSpeedThresh;
	Contact minContact;
	sf::Shader timeSlowShader;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tileset[Count];
	Tileset *ts_dodecaSmall;
	Tileset *ts_dodecaBig;
	sf::Sprite gsdodeca;
	sf::Sprite gstriblue;
	sf::Sprite gstricym;
	sf::Sprite gstrigreen;
	sf::Sprite gstrioran;
	sf::Sprite gstripurp;
	sf::Sprite gstrirgb;
	int grindActionLength;
	Tileset * tsgsdodeca;
	Tileset * tsgstriblue;
	Tileset * tsgstricym;
	Tileset * tsgstrigreen;
	Tileset * tsgstrioran;
	Tileset * tsgstripurp;
	Tileset * tsgstrirgb;
	Tileset *ts_grindAttackFX;
	
	RelEffectInstance *currLockedFairFX;
	RelEffectInstance *currLockedDairFX;
	RelEffectInstance *currLockedUairFX;
	bool showSword;
	
	sf::Sprite swordSprite;

	Tileset *ts_grindLungeSword[3];
	
	Tileset *ts_dairSword[3];
	Tileset *ts_uairSword[3];
	Tileset *ts_fairSword[3];

	sf::Sprite fairSword;
	

	Tileset *ts_standAttackSword[3];
	Tileset *ts_standAttackSword2[3];
	Tileset *ts_standAttackSword3[3];
	Tileset *ts_standAttackSword4[3];

	
	Tileset *ts_dashAttackSword[3];
	Tileset *ts_dashAttackSword2[3];
	Tileset *ts_dashAttackSword3[3];
	
	Tileset *ts_wallAttackSword[3];
	
	Tileset *ts_steepSlideAttackSword[3];
	
	Tileset *ts_steepClimbAttackSword[3];
	
	Tileset *ts_diagUpSword[3];
	
	Tileset *ts_diagDownSword[3];
	
	sf::Sprite bounceFlameSprite;
	double bounceFlameAccel0;
	double bounceFlameAccel1;
	double bounceFlameAccel2;
	
	bool scorpSet;


	Session *sess;
	GravityModifier *currGravModifier;
	V2d springVel;
	V2d springExtra;
	int springStunFramesStart;
	int springStunFrames;
	V2d teleportSpringDest;
	V2d teleportSpringVel;
	double glideTurnFactor;
	double glideTurnAccel;
	double maxGlideTurnFactor;
	//these are for your max vel
	double maxSpeed;
	bool highAccuracyHitboxes;
	V2d movementVec;
	double extra;
	int framesExtendingAirdash;
	TerrainPolygon *polyQueryList;
	Tileset *ts_scorpRun;
	Tileset *ts_scorpSlide;
	Tileset *ts_scorpSteepSlide;
	Tileset *ts_scorpStart;
	Tileset *ts_scorpStand;
	Tileset *ts_scorpJump;
	Tileset *ts_scorpDash;
	Tileset *ts_scorpSprint;
	Tileset *ts_scorpClimb;
	Tileset *ts_scorpBounce;
	Tileset *ts_scorpBounceWall;
	sf::Sprite scorpSprite;
	
	int speedParticleCounter;
	int speedParticleRate;
	
	bool hitEnemyDuringPhysics;
	double offSlopeByWallThresh;
	VertexBuf *motionGhostBuffer;
	VertexBuf *motionGhostBufferBlue;
	VertexBuf *motionGhostBufferPurple;
	int maxMotionGhosts;
	sf::Shader motionGhostShader;
	int numMotionGhosts;
	int motionGhostSpacing;
	int ghostSpacingCounter;
	double holdDashAccel;
	double wallThresh;
	double bounceBoostSpeed;
	double steepSlideGravFactor;
	double steepSlideFastGravFactor;
	double steepClimbGravFactor;
	double steepClimbUpFactor;
	double steepClimbDownFactor;
	double steepClingSpeedLimit;
	double airDashSpeed;
	double airDashSpeed0;
	double airDashSpeed1;
	double airDashSpeed2;
	int flyCounter;
	
	ComboObject *activeComboObjList;
	CollisionBody *currHurtboxes;
	int currHurtboxFrame;

	HitboxInfo *currHitboxInfo;
	std::unordered_map<int, HitboxInfo[2]> hitboxInfos;
	HitboxInfo *currVSHitboxInfo;
	CollisionBody *fairHitboxes[3];
	CollisionBody *uairHitboxes[3];
	CollisionBody *dairHitboxes[3];
	CollisionBody *standHitboxes1[3];
	CollisionBody *standHitboxes2[3];
	CollisionBody *standHitboxes3[3];
	CollisionBody *standHitboxes4[3];
	CollisionBody *dashHitboxes1[3];
	CollisionBody *dashHitboxes2[3];
	CollisionBody *dashHitboxes3[3];
	CollisionBody *wallHitboxes[3];
	CollisionBody *steepClimbHitboxes[3];
	CollisionBody *steepSlideHitboxes[3];
	CollisionBody *diagUpHitboxes[3];
	CollisionBody *diagDownHitboxes[3];
	CollisionBody *shockwaveHitboxes;
	CollisionBody *grindHitboxes[3];
	CollisionBody *homingHitboxes;
	CollisionBody *annihilationHitboxes;
	double steepThresh;
	int wallJumpMovementLimit;
	double dashHeight;
	double normalHeight;
	double doubleJumpHeight;
	double sprintHeight;
	double airSlow;
	double slopeLaunchMinSpeed;
	double maxRunInit;
	double runAccelInit;
	double sprintAccel;
	double runAccel;
	double maxFallSpeedSlow;
	double gravity;
	double extraGravityModifier;
	double boosterExtraGravityModifier;
	double jumpStrength;
	double airAccel;
	double maxAirXControl;
	double dashSpeed;
	double dashSpeed0;
	double dashSpeed1;
	double dashSpeed2;
	double slideGravFactor;
	
	int maxBBoostCount;
	int maxAirdashBoostCount;
	double doubleJumpStrength;
	double backDoubleJumpStrength;
	double lessSlowDownThresh;
	sf::Vector2<double> wallJumpStrength;
	double clingSpeed;
	bool col;
	sf::Vector2<double> tempVel;
	QueryType queryType;
	int maxDespFrames;
	
	int maxSuperFrames;
	
	bool checkValid;
	Edge *grindTransferCheckEdge;
	
	bool leftGround;
	ControllerState prevInput;
	ControllerState currInput;
	bool canRailGrind;
	bool canRailSlide;
	double minRailGrindSpeed[3];
	sf::RectangleShape railTest;
	
	bool railGrind;
	int regrindOffCount;
	int regrindOffMax;
	sf::Vector2<double> lungeNormal;
	double grindLungeSpeed0;
	double grindLungeSpeed1;
	double grindLungeSpeed2;
	double slopeTooSteepLaunchLimitX;
	
	int baseSlowMultiple;
	int baseTimeSlowedMultiple;
	sf::Vector2<double> wallNormal;
	Edge *currWall;
	bool touchedGrass[Grass::GrassType::Count];
	bool oldTouchedGrass[Grass::GrassType::Count];
	void ResetGrassCounters();
	double accelGrassAccel;
	double jumpGrassExtra;
	bool extraDoubleJump;
	
	
	
	Wire *leftWire;
	Wire *rightWire;
	
	bool wallClimbGravityFactor;
	bool wallClimbGravityOn;
	int bubbleLifeSpan;
	int bubbleRadius;
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	bool dead;
	sf::Shader swordShaders[3];
	sf::Color flashColor;
	int steepClimbBoostStart;
	
	int climbBoostLimit;
	
	double DIFactor;
	double blockstunFactor;
	bool DIChangesMagnitude;


	Edge *autoRunStopEdge;
	double autoRunStopQuant;
	double maxAutoRunSpeed;

	int grindLimitBeforeSlow;

	

	Actor( GameSession *owner, 
		EditSession *editOwner, int actorIndex );
	~Actor();
	void Init();
	void GatorStun();
	std::string GetDisplayName();
	void DrawShield(sf::RenderTarget *target);
	void DrawHomingBall(sf::RenderTarget *target);
	void DrawNameTag(sf::RenderTarget *target);
	bool CanParry(HitboxInfo::HitPosType hpt,
		V2d &hitPos,
		bool attackFacingRight );
	bool CanFullBlock(HitboxInfo::HitPosType hpt,
		V2d &hitPos,
		bool attackFacingRight );
	bool CanHalfBlock(HitboxInfo::HitPosType hpt,
		V2d &hitPos,
		bool attackFacingRight);
	bool IsNormalSkin();
	static int GetSkinIndexFromString(const std::string &s);
	bool CanBlockEnemy(HitboxInfo::HitPosType hpt, V2d &hitPos );
	void RestoreAirOptions();
	bool CanCancelAttack();
	int MostRecentFrameCurrAttackBlocked();
	V2d GetAdjustedKnockback(const V2d &kbDir);
	V2d GetGroundAnchor();
	double GetBounceBoostSpeed();
	int GetSurvivalFrame();
	void TryStartWaterGlide();

	bool TryThrowSwordProjectile(V2d &offset,V2d &dir);
	bool TryThrowSwordProjectileBasic();
	bool TryThrowEnemySwordProjectileBasic();
	void ResetAttackHit();
	void LoadHitboxes();
	void SetupHitboxInfo(
		json &j, const std::string &name,
		HitboxInfo *hi);

	void DefaultGroundLanding( double &movement );
	bool DefaultGravReverseCheck();
	void DefaultCeilingLanding( double &movement );
	void BounceCollision(double &movement);

	bool IsVisibleAction(int a);
	void SetAuraColor(sf::Color c);

	bool TryHandleHitInRewindWater();
	bool TryHandleHitWhileRewindBoosted();
	void SetFBubbleFrame(int i, float val);
	void SetFBubblePos(int i, sf::Vector2f &pos);
	void SetFBubbleRadiusSize(int i, float rad);
	int GetNumActiveBubbles();

	void PopulateState(PState *ps);
	void PopulateFromState(PState *ps);
	bool TryClimbBoost();
	CollisionBody * GetBubbleHitbox(int index);

	int GetMaxBubbles();
	double GetMaxSpeed();

	void ActivateLauncherEffect(int tile);

	bool CheckExtendedAirdash();

	void ClearRecentHitters();
	void UpdateRecentHitters();
	bool EnemyRecentlyHitMe( Enemy *e );
	bool BulletRecentlyHitMe(BasicBullet *b);
	void AddRecentEnemyHitter(Enemy *e);
	void AddRecentBulletHitter(BasicBullet *b);

	V2d CalcKnockback(HitboxInfo *receivedHit);

	bool CheckTerrainDisappear(Edge *e);
	void CheckBirdCommands();
	void SetActionSuperLevel();
	void ResetSuperLevel();
	bool CanTech();
	void CreateKeyExplosion( int gateCategory );
	void CreateGateExplosion( int gateCategory );
	void CreateEnemiesClearedRing();
	void CreateEnoughKeysRing();
	void CreateGravityDecreaserOnRing();
	void CreateGravityDecreaserOffRing();
	void CollectFly(HealthFly *hf);
	bool CheckSetToAerialFromNormalWater();
	void SetupActionFunctions();
	void StartAction();
	void EndAction();
	void ChangeAction();
	void UpdateAction();
	void UpdateActionSprite();
	void HandleTouchedGate();
	void SetGroundedSpritePos(Edge * e, double angle);
	void SetGroundedSpriteTransform();
	void SetGroundedSpriteTransform(Edge * e, double angle);
	void TransitionAction(int a);
	void ActionTimeIndFrameInc();
	void ActionTimeDepFrameInc();
	int GetActionLength(int a);
	void SetupFuncsForAction(int a,
		void(Actor::*)(),
		void(Actor::*)(),
		void(Actor::*)(),
		void(Actor::*)(),
		void(Actor::*)(),
		void(Actor::*)(int),
		void(Actor::*)(),
		void(Actor::*)(),
		int(Actor::*)(),
		Tileset*(Actor::*)());
	void SetupTimeBubbles();
	void SetGameMode();
	void UpdateModifiedGravity();
	bool CanBufferGrind();
	bool CanPressGrind();
	bool TryBufferGrind();
	bool TryPressGrind();
	bool JumpButtonPressed();
	bool DashButtonPressed();
	bool GrindButtonPressed();
	bool GrindButtonHeld();
	bool BounceButtonPressed();
	bool BounceButtonHeld();
	bool JumpButtonHeld();
	bool DashButtonHeld();
	bool AttackButtonPressed();
	bool AttackButtonHeld();
	bool PowerButtonHeld();
	bool PowerButtonPressed();
	bool SpecialButtonPressed();
	SoundNode * ActivateSound(int st, bool loop = false);
	SoundNode *ActivateRepeatingSound(int st, bool loop = false);
	BasicEffect * ActivateEffect(
		EffectLayer layer,
		Tileset *ts,
		sf::Vector2<double> pos,
		bool pauseImmune,
		double angle,
		int frameCount,
		int animationFactor,
		bool right,
		int startFrame = 0,
		float depth = 1.f);
	EffectInstance * ActivateEffect(int pfxType, sf::Vector2f &pos, double angle, int frameCount, int animFactor, bool right, int startFrame = 0);
	EffectInstance * ActivateEffect(int pfxType, EffectInstance *params );
	void DeactivateSound(SoundNode *sn);
	void SetToOriginalPos();
	void UpdatePowersMenu();
	void SetSession(Session *sess,
		GameSession *game,
		EditSession *edit);
	void ReverseSteepSlideJump();
	void UpdateWireQuads();
	void DrawWires(sf::RenderTarget *target);
	QuadTree *GetTerrainTree();
	QuadTree *GetSpecialTerrainTree();
	QuadTree *GetRailEdgeTree();
	QuadTree *GetBarrierTree();
	QuadTree *GetBorderTree();
	Collider &GetCollider();
	SoundInfo * GetSound(const std::string &name);
	CollisionBody *CreateCollisionBody(const std::string &str);
	bool IsGroundAttack(int a);
	GameController *GetController(int index);
	void HandleGroundTrigger(GroundTrigger *trigger);
	void CheckForAirTrigger();
	void HandleAirTrigger();
	void BounceFloaterBoost( V2d &hitDir );
	void UpdateCanStandUp();
	void UpdateBounceFlameOn();
	void ProcessBooster();
	void ProcessTimeBooster();
	void ProcessMomentumBooster();
	void ProcessRewindBooster();
	void ProcessAntiTimeSlowBooster();
	void ProcessHomingBooster();
	void ProcessFreeFlightBooster();
	void ProcessSwordProjectileBooster();
	void ProcessGravModifier();
	void UpdateWireStates();
	void ProcessAccelGrass();
	void ProcessDecelGrass();
	void ProcessBoostGrass();
	void ProcessHitGrass();
	void ProcessBounceGrassGrounded();
	void ProcessPoisonGrass();
	void ApplyGeneralAcceleration(double amt);
	void TryCheckGrass();
	void LimitMaxSpeeds();
	void UpdateBubbles();
	void UpdateRegrindOffCounter();
	void UpdateKnockbackDirectionAndHitboxType();
	void UpdateSmallLightning();
	void UpdateRisingAura();
	void UpdateLockedFX();
	void ProcessSpecialTerrain();
	void TurnFace();
	void StandInPlace();
	void WaitInPlace();
	void Wait();
	bool IsGoalKillAction(int a);
	bool IsIntroAction(int a);
	bool IsExitAction(int a);
	bool IsSequenceAction(int a);
	void StartSeqKinThrown(V2d &pos, V2d &vel);
	void SeqKneel();
	void SeqMeditateMaskOn();
	void SeqMaskOffMeditate();
	void SeqGetAirdash();
	void CreateAttackLightning();
	bool CanShootWire();
	bool CanCreateTimeBubble();
	void UnlockGate(Gate *g);
	void WireMovement();
	int GetDoubleJump();
	bool CanDoubleJump();
	void ExecuteDoubleJump();
	void ExecuteWallJump();
	bool IsSingleWirePulling();
	bool IsDoubleWirePulling();
	bool TryDoubleJump();
	bool TryGroundBlock();
	bool TryFloorRailDropThrough();
	bool TryScorpRailDropThrough();
	bool TryAirBlock();
	void TryChangePowerMode();
	bool TryDash();
	bool TryJumpSquat();
	bool TrySlideBrakeOrStand();
	bool TrySprintOrRun();
	void SetSprintStartFrame();
	bool TryAirDash();
	bool TryGlide();
	bool ExitGrind(bool jump);
	void CheckGates();
	void SetSpriteTexture(int a);
	void SetSpriteTile(int tileIndex, bool noFlipX = true, bool noFlipY = true);
	void SetSpriteTile(sf::Sprite *spr,
		Tileset *t, int tileIndex, bool noFlipX = true, bool noFlipY = true);
	void SetExpr(int ex);
	void SetAction(int a);
	void SetupTilesets();
	void SetupFXTilesets();
	void SetupSwordTilesets();
	void SetupExtraTilesets();
	void SetupActionTilesets();
	void RailGrindMovement();
	bool AirAttack();
	void EnterNexus(int nexusIndex, sf::Vector2<double> &pos);
	void SetFakeCurrInput(
		ControllerState &state);
	float GetSpeedBarPart();
	Tileset *GetActionTileset(const std::string &fn);
	float totalHealth;
	int actorIndex;
	void UpdateSprite();
	void ConfirmEnemyNoKill( Enemy *e );
	void ConfirmHit(Enemy * e);
	bool IsHitstunAction(int a);
	bool IsAirHitstunAction(int a);
	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void ApplyHit( HitboxInfo *info,
		Actor *receivedHitPlayer,
		//Enemy *recievedHitEnemy,
		HitResult res, V2d &pos );
	bool ResolvePhysics( V2d vel );
	void UpdatePhysics();
	void PhysicsResponse();
	bool TryGroundAttack();
	bool SteepSlideAttack();
	bool SteepClimbAttack();
	void ConfirmEnemyKill( Enemy *e );
	HitResult CheckIfImHitByEnemy( Enemy *hitter, CollisionBody *hitBody,
		int hitFrame,
		HitboxInfo::HitPosType hpt,
		V2d &hitPos,
		bool attackFacingRight,
		bool canBeParried,
		bool canBeBlocked);
	HitResult CheckIfImHitByBullet(BasicBullet *hitter, CollisionBox &cb,
		HitboxInfo::HitPosType hpt,
		V2d &hitPos, bool attackFacingRight,
		bool canBeParried, bool canBeBlocked);
	HitResult CheckIfImHit(CollisionBody *hitBody,
		int hitFrame,
		HitboxInfo::HitPosType hpt,
		V2d &hitPos, 
		bool attackFacingRight,
		bool canBeParried,
		bool canBeBlocked);
	HitResult CheckIfImHit(CollisionBox &cb,
		HitboxInfo::HitPosType hpt,
		V2d &hitPos,
		bool attackFacingRight,
		bool canBeParried,
		bool canBeBlocked);
	HitResult CheckHitByEnemy(Enemy *e);
	void TryHitPlayer(int targetIndex);
	void SetTouchedGate(Gate *g);
	std::pair<bool, bool> PlayerHitMe(int otherPlayerIndex);
	void ShipPickupPoint( double eq,
		bool facingRight );
	void GrabShipWire();
	bool physicsOver;
	void UpdatePostPhysics();
	bool CheckWall( bool right );
	bool TryWallJump();
	bool CheckStandUp();
	bool TryStandupOnForcedGrindExit();
	void TryDashBoost();
	void TryAirdashBoost();
	void TryExtraAirdashBoost();
	void ActivateAirdashBoost();
	void CheckBounceFlame();
	bool BasicAirAction();
	bool GlideAction();
	bool HomingAction();
	bool IsHomingAttackAction(int a);
	bool BasicGroundAction();
	bool BasicSteepAction();
	bool BasicAirAttackAction();
	sf::Vector2<double> UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );
	void DrawEffects(int effectLayer, sf::RenderTarget *target);
	void DrawPlayerSprite(sf::RenderTarget *target);
	void MiniDraw(sf::RenderTarget *target);
	void DeathDraw(sf::RenderTarget *target);
	void DebugDraw( sf::RenderTarget *target );
	void DrawMapWires(sf::RenderTarget *target);
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateHitboxes();
	bool InWater(int wType );
	void AirMovement();
	void FreeFlightMovement();
	bool TryHomingMovement();
	double GroundedAngle();
	double GroundedAngleAttack( sf::Vector2<double> &trueNormal );
	V2d GetGroundedNormal();
	bool IsOnSteepGround();
	bool HoldingRelativeUp();
	bool HoldingRelativeDown();
	void TryFlipRightSideUp();
	sf::Vector2i GetWireOffset();
	void RunMovement();
	void BrakeMovement();
	void SetAutoRun( bool fr, double maxAutoRun );
	void SetAirPos(V2d &pos, bool facingRight);
	void SetGroundedPos(Edge *g, double q);
	void SetGroundedPos(Edge *g, double q, double xoff);
	void SetStandInPlacePos(Edge *g, double q,
		bool fr);
	void SetStoryRun(bool fr, double maxAutoRun, Edge * g,
		double q, Edge *end, double endQ );
	void SetStoryRun(bool fr, double maxAutoRun, Edge * g,
		double q);
	void SetAutoRunStopper(Edge *g, double q);
	bool IsAutoRunning();
	void AttackMovement();
	void DodecaLateDraw(sf::RenderTarget *target);
	void SetActionGrind();
	bool CanUnlockGate( Gate *g );
	void HandleWaitingScoreDisplay();
	void EndLevelWithoutGoal();
	void CheckHoldJump();
	void Respawn( bool setStartPos = true);
	void BounceFlameOff();
	void BounceFlameOn();
	void SetupAction(int a);
	void SetBoostVelocity();
	void SetBounceBoostVelocity();
	void CoyoteBulletBounce();
	double GetNumSteps();
	double CalcLandingSpeed( sf::Vector2<double> &testVel,
		sf::Vector2<double> &alongVel, 
		sf::Vector2<double> &gNorm, bool rail = false );
	bool CanRailSlide();
	bool CanRailGrind();
	bool IsRailSlideFacingRight();
	bool IsInvincible();
	void SetKinMode(Mode m);
	void KinModeUpdate();
	void ReverseVerticalInputs();
	void ProcessReceivedHit();
	void HitGoal();
	void UpdateDrain();
	void DrainTimer(int drainFrames);
	void HealTimer(int healFrames);
	void ProcessGravityGrass();
	void ProcessHitGoal();
	void UpdateWirePhysics();
	void UpdateGrindPhysics(double movement, bool checkRailAndTerrainTransfers);
	void StopGrind();
	void HandleBounceGrass();
	void HandleBounceRail();
	bool UpdateGrindRailPhysics(double movement);
	bool TryUnlockOnTransfer( Edge *e );
	void LeaveGroundTransfer(bool right, V2d leaveExtra = V2d());
	void LeaveGroundTipTransfer(bool right, Edge *altEdge, V2d leaveExtra = V2d());
	void LeaveTipTransfer(bool right, V2d leaveExtra = V2d());
	bool UpdateAutoRunPhysics( double q, double m );
	void UpdateSpeedBar();
	bool CareAboutSpeedAction();
	void UpdateMotionGhosts();
	void UpdateSpeedParticles();
	void UpdateAttackLightning();
	void UpdateAllEffects();
	void UpdatePlayerShader();
	void TryEndLevel();
	void UpdateDashBooster();
	void SlowDependentFrameIncrement();
	void SlowIndependentFrameIncrement();
	void UpdateBounceFlameCounters();
	void SetAerialScorpSprite();
	int GetJumpFrame();
	void QueryTouchGrass();
	double GetBounceFlameAccel();
	void SprintAccel();
	void GroundExtraAccel();
	double GetDashSpeed();
	void StopRepeatingSound();
	double GetOriginalDashSpeed();
	double GetAirDashSpeed();
	void AddToFlyCounter(int count);
	void SetCurrHitboxes(CollisionBody *cBody,
		int p_frame);
	void AddActiveComboObj(ComboObject *c);
	void RemoveActiveComboObj(ComboObject *c);
	void ClearActiveComboObjects();
	void DebugDrawComboObj(sf::RenderTarget *target);
	bool IsMovingRight();
	bool IsMovingLeft();
	bool IntersectMyHurtboxes(CollisionBody *cb,
		int cbFrame );
	bool IntersectMyHurtboxes(CollisionBox &cb);
	bool IntersectMyHitboxes(CollisionBody *cb,
		int cbFrame);
	bool IntersectMySlowboxes(CollisionBody *cb,
		int cbFrame );
	ComboObject * IntersectMyComboHitboxes(
		Enemy *e, CollisionBody *cb,
		int cbFrame);
	Wire * IntersectMyWireHitboxes( Enemy *e,
		CollisionBody *cb,
		int cbFrame);
	bool EnemyIsFar(V2d &enemyPos);
	void SeqAfterCrawlerFight();
	double GetFullSprintAccel(bool downSlope,
		sf::Vector2<double> &gNorm);
	double GetMinRailGrindSpeed();
	sf::Vector2<double> AddAerialGravity(sf::Vector2<double> vel);
	double GetGravity();
	void HandleSpecialTerrain();
	V2d GetTrueCenter();

	enum SpecialTerrainSituation
	{
		SPECIALT_ENTER,
		SPECIALT_REMAIN,
		SPECIALT_EXIT
	};

	void HandleWaterSituation(
		int wType,
		SpecialTerrainSituation sit);
	V2d GetTrueVel();
	void RestoreDoubleJump();
	void RestoreAirDash();
	int GetBubbleRadius();
	int GetBubbleTimeFactor();
	int GetBeingSlowedFactor();
	bool IsBeingSlowed();
	void SetAllUpgrades(const BitField &b);
	bool HasUpgrade(int index);
	int NumUpgradeRange(int index, int numUpgrades);
	void SetUpgrade(int upgrade, bool on);
	void SetStartUpgrade(int upgrade, bool on);
	bool SpringLaunch();
	bool ScorpionLaunch();
	bool TeleporterLaunch();
	bool AimLauncherAim();
	bool SwingLaunch();
	bool CheckSwing();
	bool CheckNormalSwing();
	bool CheckNormalSwingHeld();
	bool CheckSwingHeld();
	void ClearPauseBufferedActions();
	void UpdateInHitlag();
	bool IsBlockAction(int a);
	bool IsGrindAction(int a);
	bool IsAttackAction( int a );
	bool IsGroundAttackAction(int a);
	bool IsSpringAction(int a);
	bool IsOnRailAction(int a);
	bool IsInHistunAction( int a );
	bool IsActionGroundBlock(int a);
	bool IsActionAirBlock(int a);

	void ResetBoosterEffects();

	void SetSkin(int skinIndex);
	void BlendSkins(int first, int second, float progress);

	void InitEmitters();

	V2d GetKnockbackDirFromVel();

	void HitOutOfCeilingGrindAndReverse();
	void HitOutOfCeilingGrindIntoAir();
	void HitOutOfGrind();
	void HitWhileGrounded();
	void HitWhileAerial();
	void ReactToBeingHit();
	void SlideOffWhileInGroundHitstun();
	void HitGroundWhileInAirHitstun();
	void HitWallWhileInAirHitstun();
	Edge * RayCastSpecialTerrainExit();
	Edge * RayCastSpecialTerrainEnter();

	void SetGroundBlockAction();
	void SetAirBlockAction();
	void TryResetBlockCounter();
	void AirBlockChange();
	void GroundBlockChange();
	void UpdateGroundedShieldSprite(int tile);
	void UpdateAerialShieldSprite(int tile);
	void StartStandAttack();
	void StartDashAttack();
	void StartUpTilt();
	void StartDownTilt();

	int GetCurrStandAttack();
	int GetCurrDashAttack();
	int GetCurrUpTilt();
	int GetCurrDownTilt();
	void SteepSlideMovement();
	void SteepClimbMovement();

	bool CheckIfEnemyIsTrackable(Enemy *e);

	void RevertAfterSimulating();
	void ForwardSimulate(int frames, bool storePositions);

	void UpdateGroundedSwordSprite(
		Tileset *ts,
		int startFrame,
		int endFrame,
		int animMult,
		sf::Vector2f &offset );
	void UpdateGroundedAttackSprite(
		int a, Tileset *ts_sword,
		int startSword,
		int endSword,
		int animMult,
		sf::Vector2f &swordOffset );
	void ApplyBlockFriction();
	void ClearAllEffects();
	void UpdatePreFrameCalculations();
	void InitPreFrameCalculations();
	void UpdateNumFuturePositions();

	void CheckCollisionForTerrainFade();
	void CheckGrindEdgeForTerrainFade();

	void ProcessGroundedCollision();
	bool TryLandFromBounceGround();

	
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);


	//void PopulateFromState(PState *ps);
	//kin action functions

	void AIMWAIT_Start();
	void AIMWAIT_End();
	void AIMWAIT_Change();
	void AIMWAIT_Update();
	void AIMWAIT_UpdateSprite();
	void AIMWAIT_TransitionToAction(int a);
	void AIMWAIT_TimeIndFrameInc();
	void AIMWAIT_TimeDepFrameInc();
	int AIMWAIT_GetActionLength();
	Tileset * AIMWAIT_GetTileset();

	void AIRBLOCKUP_Start();
	void AIRBLOCKUP_End();
	void AIRBLOCKUP_Change();
	void AIRBLOCKUP_Update();
	void AIRBLOCKUP_UpdateSprite();
	void AIRBLOCKUP_TransitionToAction(int a);
	void AIRBLOCKUP_TimeIndFrameInc();
	void AIRBLOCKUP_TimeDepFrameInc();
	int AIRBLOCKUP_GetActionLength();
	Tileset * AIRBLOCKUP_GetTileset();

	void AIRBLOCKUPFORWARD_Start();
	void AIRBLOCKUPFORWARD_End();
	void AIRBLOCKUPFORWARD_Change();
	void AIRBLOCKUPFORWARD_Update();
	void AIRBLOCKUPFORWARD_UpdateSprite();
	void AIRBLOCKUPFORWARD_TransitionToAction(int a);
	void AIRBLOCKUPFORWARD_TimeIndFrameInc();
	void AIRBLOCKUPFORWARD_TimeDepFrameInc();
	int AIRBLOCKUPFORWARD_GetActionLength();
	Tileset * AIRBLOCKUPFORWARD_GetTileset();

	void AIRBLOCKFORWARD_Start();
	void AIRBLOCKFORWARD_End();
	void AIRBLOCKFORWARD_Change();
	void AIRBLOCKFORWARD_Update();
	void AIRBLOCKFORWARD_UpdateSprite();
	void AIRBLOCKFORWARD_TransitionToAction(int a);
	void AIRBLOCKFORWARD_TimeIndFrameInc();
	void AIRBLOCKFORWARD_TimeDepFrameInc();
	int AIRBLOCKFORWARD_GetActionLength();
	Tileset * AIRBLOCKFORWARD_GetTileset();

	void AIRBLOCKDOWNFORWARD_Start();
	void AIRBLOCKDOWNFORWARD_End();
	void AIRBLOCKDOWNFORWARD_Change();
	void AIRBLOCKDOWNFORWARD_Update();
	void AIRBLOCKDOWNFORWARD_UpdateSprite();
	void AIRBLOCKDOWNFORWARD_TransitionToAction(int a);
	void AIRBLOCKDOWNFORWARD_TimeIndFrameInc();
	void AIRBLOCKDOWNFORWARD_TimeDepFrameInc();
	int AIRBLOCKDOWNFORWARD_GetActionLength();
	Tileset * AIRBLOCKDOWNFORWARD_GetTileset();

	void AIRBLOCKDOWN_Start();
	void AIRBLOCKDOWN_End();
	void AIRBLOCKDOWN_Change();
	void AIRBLOCKDOWN_Update();
	void AIRBLOCKDOWN_UpdateSprite();
	void AIRBLOCKDOWN_TransitionToAction(int a);
	void AIRBLOCKDOWN_TimeIndFrameInc();
	void AIRBLOCKDOWN_TimeDepFrameInc();
	int AIRBLOCKDOWN_GetActionLength();
	Tileset * AIRBLOCKDOWN_GetTileset();

	void AIRDASH_Start();
	void AIRDASH_End();
	void AIRDASH_Change();
	void AIRDASH_Update();
	void AIRDASH_UpdateSprite();
	void AIRDASH_TransitionToAction(int a);
	void AIRDASH_TimeIndFrameInc();
	void AIRDASH_TimeDepFrameInc();
	int AIRDASH_GetActionLength();
	Tileset * AIRDASH_GetTileset();

	void AIRDASHFORWARDATTACK_Start();
	void AIRDASHFORWARDATTACK_End();
	void AIRDASHFORWARDATTACK_Change();
	void AIRDASHFORWARDATTACK_Update();
	void AIRDASHFORWARDATTACK_UpdateSprite();
	void AIRDASHFORWARDATTACK_TransitionToAction(int a);
	void AIRDASHFORWARDATTACK_TimeIndFrameInc();
	void AIRDASHFORWARDATTACK_TimeDepFrameInc();
	int AIRDASHFORWARDATTACK_GetActionLength();
	Tileset * AIRDASHFORWARDATTACK_GetTileset();


	void AIRHITSTUN_Start();
	void AIRHITSTUN_End();
	void AIRHITSTUN_Change();
	void AIRHITSTUN_Update();
	void AIRHITSTUN_UpdateSprite();
	void AIRHITSTUN_TransitionToAction(int a);
	void AIRHITSTUN_TimeIndFrameInc();
	void AIRHITSTUN_TimeDepFrameInc();
	int AIRHITSTUN_GetActionLength();
	Tileset * AIRHITSTUN_GetTileset();

	void AIRPARRY_Start();
	void AIRPARRY_End();
	void AIRPARRY_Change();
	void AIRPARRY_Update();
	void AIRPARRY_UpdateSprite();
	void AIRPARRY_TransitionToAction(int a);
	void AIRPARRY_TimeIndFrameInc();
	void AIRPARRY_TimeDepFrameInc();
	int AIRPARRY_GetActionLength();
	Tileset * AIRPARRY_GetTileset();

	void AUTORUN_Start();
	void AUTORUN_End();
	void AUTORUN_Change();
	void AUTORUN_Update();
	void AUTORUN_UpdateSprite();
	void AUTORUN_TransitionToAction(int a);
	void AUTORUN_TimeIndFrameInc();
	void AUTORUN_TimeDepFrameInc();
	int AUTORUN_GetActionLength();
	Tileset * AUTORUN_GetTileset();

	void BACKWARDSDOUBLE_Start();
	void BACKWARDSDOUBLE_End();
	void BACKWARDSDOUBLE_Change();
	void BACKWARDSDOUBLE_Update();
	void BACKWARDSDOUBLE_UpdateSprite();
	void BACKWARDSDOUBLE_TransitionToAction(int a);
	void BACKWARDSDOUBLE_TimeIndFrameInc();
	void BACKWARDSDOUBLE_TimeDepFrameInc();
	int BACKWARDSDOUBLE_GetActionLength();
	Tileset * BACKWARDSDOUBLE_GetTileset();

	void BOUNCEAIR_Start();
	void BOUNCEAIR_End();
	void BOUNCEAIR_Change();
	void BOUNCEAIR_Update();
	void BOUNCEAIR_UpdateSprite();
	void BOUNCEAIR_TransitionToAction(int a);
	void BOUNCEAIR_TimeIndFrameInc();
	void BOUNCEAIR_TimeDepFrameInc();
	int BOUNCEAIR_GetActionLength();
	Tileset * BOUNCEAIR_GetTileset();

	void BOOSTERBOUNCE_Start();
	void BOOSTERBOUNCE_End();
	void BOOSTERBOUNCE_Change();
	void BOOSTERBOUNCE_Update();
	void BOOSTERBOUNCE_UpdateSprite();
	void BOOSTERBOUNCE_TransitionToAction(int a);
	void BOOSTERBOUNCE_TimeIndFrameInc();
	void BOOSTERBOUNCE_TimeDepFrameInc();
	int BOOSTERBOUNCE_GetActionLength();
	Tileset * BOOSTERBOUNCE_GetTileset();

	void BOOSTERBOUNCEGROUND_Start();
	void BOOSTERBOUNCEGROUND_End();
	void BOOSTERBOUNCEGROUND_Change();
	void BOOSTERBOUNCEGROUND_Update();
	void BOOSTERBOUNCEGROUND_UpdateSprite();
	void BOOSTERBOUNCEGROUND_TransitionToAction(int a);
	void BOOSTERBOUNCEGROUND_TimeIndFrameInc();
	void BOOSTERBOUNCEGROUND_TimeDepFrameInc();
	int BOOSTERBOUNCEGROUND_GetActionLength();
	Tileset * BOOSTERBOUNCEGROUND_GetTileset();

	void BOUNCEGROUND_Start();
	void BOUNCEGROUND_End();
	void BOUNCEGROUND_Change();
	void BOUNCEGROUND_Update();
	void BOUNCEGROUND_UpdateSprite();
	void BOUNCEGROUND_TransitionToAction(int a);
	void BOUNCEGROUND_TimeIndFrameInc();
	void BOUNCEGROUND_TimeDepFrameInc();
	int BOUNCEGROUND_GetActionLength();
	Tileset * BOUNCEGROUND_GetTileset();

	void BOUNCEGROUNDEDWALL_Start();
	void BOUNCEGROUNDEDWALL_End();
	void BOUNCEGROUNDEDWALL_Change();
	void BOUNCEGROUNDEDWALL_Update();
	void BOUNCEGROUNDEDWALL_UpdateSprite();
	void BOUNCEGROUNDEDWALL_TransitionToAction(int a);
	void BOUNCEGROUNDEDWALL_TimeIndFrameInc();
	void BOUNCEGROUNDEDWALL_TimeDepFrameInc();
	int BOUNCEGROUNDEDWALL_GetActionLength();
	Tileset * BOUNCEGROUNDEDWALL_GetTileset();

	void BRAKE_Start();
	void BRAKE_End();
	void BRAKE_Change();
	void BRAKE_Update();
	void BRAKE_UpdateSprite();
	void BRAKE_TransitionToAction(int a);
	void BRAKE_TimeIndFrameInc();
	void BRAKE_TimeDepFrameInc();
	int BRAKE_GetActionLength();
	Tileset * BRAKE_GetTileset();

	void DAIR_Start();
	void DAIR_End();
	void DAIR_Change();
	void DAIR_Update();
	void DAIR_UpdateSprite();
	void DAIR_TransitionToAction(int a);
	void DAIR_TimeIndFrameInc();
	void DAIR_TimeDepFrameInc();
	int DAIR_GetActionLength();
	Tileset * DAIR_GetTileset();

	void DASH_Start();
	void DASH_End();
	void DASH_Change();
	void DASH_Update();
	void DASH_UpdateSprite();
	void DASH_TransitionToAction(int a);
	void DASH_TimeIndFrameInc();
	void DASH_TimeDepFrameInc();
	int DASH_GetActionLength();
	Tileset * DASH_GetTileset();

	void DASHATTACK_Start();
	void DASHATTACK_End();
	void DASHATTACK_Change();
	void DASHATTACK_Update();
	void DASHATTACK_UpdateSprite();
	void DASHATTACK_TransitionToAction(int a);
	void DASHATTACK_TimeIndFrameInc();
	void DASHATTACK_TimeDepFrameInc();
	int DASHATTACK_GetActionLength();
	Tileset * DASHATTACK_GetTileset();

	void DASHATTACK2_Start();
	void DASHATTACK2_End();
	void DASHATTACK2_Change();
	void DASHATTACK2_Update();
	void DASHATTACK2_UpdateSprite();
	void DASHATTACK2_TransitionToAction(int a);
	void DASHATTACK2_TimeIndFrameInc();
	void DASHATTACK2_TimeDepFrameInc();
	int DASHATTACK2_GetActionLength();
	Tileset * DASHATTACK2_GetTileset();

	void DASHATTACK3_Start();
	void DASHATTACK3_End();
	void DASHATTACK3_Change();
	void DASHATTACK3_Update();
	void DASHATTACK3_UpdateSprite();
	void DASHATTACK3_TransitionToAction(int a);
	void DASHATTACK3_TimeIndFrameInc();
	void DASHATTACK3_TimeDepFrameInc();
	int DASHATTACK3_GetActionLength();
	Tileset * DASHATTACK3_GetTileset();

	void DEATH_Start();
	void DEATH_End();
	void DEATH_Change();
	void DEATH_Update();
	void DEATH_UpdateSprite();
	void DEATH_TransitionToAction(int a);
	void DEATH_TimeIndFrameInc();
	void DEATH_TimeDepFrameInc();
	int DEATH_GetActionLength();
	Tileset * DEATH_GetTileset();

	void DIAGDOWNATTACK_Start();
	void DIAGDOWNATTACK_End();
	void DIAGDOWNATTACK_Change();
	void DIAGDOWNATTACK_Update();
	void DIAGDOWNATTACK_UpdateSprite();
	void DIAGDOWNATTACK_TransitionToAction(int a);
	void DIAGDOWNATTACK_TimeIndFrameInc();
	void DIAGDOWNATTACK_TimeDepFrameInc();
	int DIAGDOWNATTACK_GetActionLength();
	Tileset * DIAGDOWNATTACK_GetTileset();

	void DIAGUPATTACK_Start();
	void DIAGUPATTACK_End();
	void DIAGUPATTACK_Change();
	void DIAGUPATTACK_Update();
	void DIAGUPATTACK_UpdateSprite();
	void DIAGUPATTACK_TransitionToAction(int a);
	void DIAGUPATTACK_TimeIndFrameInc();
	void DIAGUPATTACK_TimeDepFrameInc();
	int DIAGUPATTACK_GetActionLength();
	Tileset * DIAGUPATTACK_GetTileset();

	void DOUBLE_Start();
	void DOUBLE_End();
	void DOUBLE_Change();
	void DOUBLE_Update();
	void DOUBLE_UpdateSprite();
	void DOUBLE_TransitionToAction(int a);
	void DOUBLE_TimeIndFrameInc();
	void DOUBLE_TimeDepFrameInc();
	int DOUBLE_GetActionLength();
	Tileset * DOUBLE_GetTileset();

	void ENTERNEXUS1_Start();
	void ENTERNEXUS1_End();
	void ENTERNEXUS1_Change();
	void ENTERNEXUS1_Update();
	void ENTERNEXUS1_UpdateSprite();
	void ENTERNEXUS1_TransitionToAction(int a);
	void ENTERNEXUS1_TimeIndFrameInc();
	void ENTERNEXUS1_TimeDepFrameInc();
	int ENTERNEXUS1_GetActionLength();
	Tileset * ENTERNEXUS1_GetTileset();

	void EXIT_Start();
	void EXIT_End();
	void EXIT_Change();
	void EXIT_Update();
	void EXIT_UpdateSprite();
	void EXIT_TransitionToAction(int a);
	void EXIT_TimeIndFrameInc();
	void EXIT_TimeDepFrameInc();
	int EXIT_GetActionLength();
	Tileset * EXIT_GetTileset();

	void EXITBOOST_Start();
	void EXITBOOST_End();
	void EXITBOOST_Change();
	void EXITBOOST_Update();
	void EXITBOOST_UpdateSprite();
	void EXITBOOST_TransitionToAction(int a);
	void EXITBOOST_TimeIndFrameInc();
	void EXITBOOST_TimeDepFrameInc();
	int EXITBOOST_GetActionLength();
	Tileset * EXITBOOST_GetTileset();

	void EXITWAIT_Start();
	void EXITWAIT_End();
	void EXITWAIT_Change();
	void EXITWAIT_Update();
	void EXITWAIT_UpdateSprite();
	void EXITWAIT_TransitionToAction(int a);
	void EXITWAIT_TimeIndFrameInc();
	void EXITWAIT_TimeDepFrameInc();
	int EXITWAIT_GetActionLength();
	Tileset * EXITWAIT_GetTileset();

	void FAIR_Start();
	void FAIR_End();
	void FAIR_Change();
	void FAIR_Update();
	void FAIR_UpdateSprite();
	void FAIR_TransitionToAction(int a);
	void FAIR_TimeIndFrameInc();
	void FAIR_TimeDepFrameInc();
	int FAIR_GetActionLength();
	Tileset * FAIR_GetTileset();

	void FREEFLIGHT_Start();
	void FREEFLIGHT_End();
	void FREEFLIGHT_Change();
	void FREEFLIGHT_Update();
	void FREEFLIGHT_UpdateSprite();
	void FREEFLIGHT_TransitionToAction(int a);
	void FREEFLIGHT_TimeIndFrameInc();
	void FREEFLIGHT_TimeDepFrameInc();
	int FREEFLIGHT_GetActionLength();
	Tileset * FREEFLIGHT_GetTileset();

	void FREEFLIGHTSTUN_Start();
	void FREEFLIGHTSTUN_End();
	void FREEFLIGHTSTUN_Change();
	void FREEFLIGHTSTUN_Update();
	void FREEFLIGHTSTUN_UpdateSprite();
	void FREEFLIGHTSTUN_TransitionToAction(int a);
	void FREEFLIGHTSTUN_TimeIndFrameInc();
	void FREEFLIGHTSTUN_TimeDepFrameInc();
	int FREEFLIGHTSTUN_GetActionLength();
	Tileset * FREEFLIGHTSTUN_GetTileset();

	void GETPOWER_AIRDASH_FLIP_Start();
	void GETPOWER_AIRDASH_FLIP_End();
	void GETPOWER_AIRDASH_FLIP_Change();
	void GETPOWER_AIRDASH_FLIP_Update();
	void GETPOWER_AIRDASH_FLIP_UpdateSprite();
	void GETPOWER_AIRDASH_FLIP_TransitionToAction(int a);
	void GETPOWER_AIRDASH_FLIP_TimeIndFrameInc();
	void GETPOWER_AIRDASH_FLIP_TimeDepFrameInc();
	int GETPOWER_AIRDASH_FLIP_GetActionLength();
	Tileset * GETPOWER_AIRDASH_FLIP_GetTileset();

	void GETPOWER_AIRDASH_MEDITATE_Start();
	void GETPOWER_AIRDASH_MEDITATE_End();
	void GETPOWER_AIRDASH_MEDITATE_Change();
	void GETPOWER_AIRDASH_MEDITATE_Update();
	void GETPOWER_AIRDASH_MEDITATE_UpdateSprite();
	void GETPOWER_AIRDASH_MEDITATE_TransitionToAction(int a);
	void GETPOWER_AIRDASH_MEDITATE_TimeIndFrameInc();
	void GETPOWER_AIRDASH_MEDITATE_TimeDepFrameInc();
	int GETPOWER_AIRDASH_MEDITATE_GetActionLength();
	Tileset * GETPOWER_AIRDASH_MEDITATE_GetTileset();

	void GETSHARD_Start();
	void GETSHARD_End();
	void GETSHARD_Change();
	void GETSHARD_Update();
	void GETSHARD_UpdateSprite();
	void GETSHARD_TransitionToAction(int a);
	void GETSHARD_TimeIndFrameInc();
	void GETSHARD_TimeDepFrameInc();
	int GETSHARD_GetActionLength();
	Tileset * GETSHARD_GetTileset();

	void GLIDE_Start();
	void GLIDE_End();
	void GLIDE_Change();
	void GLIDE_Update();
	void GLIDE_UpdateSprite();
	void GLIDE_TransitionToAction(int a);
	void GLIDE_TimeIndFrameInc();
	void GLIDE_TimeDepFrameInc();
	int GLIDE_GetActionLength();
	Tileset * GLIDE_GetTileset();

	void GOALKILL_Start();
	void GOALKILL_End();
	void GOALKILL_Change();
	void GOALKILL_Update();
	void GOALKILL_UpdateSprite();
	void GOALKILL_TransitionToAction(int a);
	void GOALKILL_TimeIndFrameInc();
	void GOALKILL_TimeDepFrameInc();
	int GOALKILL_GetActionLength();
	Tileset * GOALKILL_GetTileset();

	void GOALKILL1_Start();
	void GOALKILL1_End();
	void GOALKILL1_Change();
	void GOALKILL1_Update();
	void GOALKILL1_UpdateSprite();

	void GOALKILL2_Start();
	void GOALKILL2_End();
	void GOALKILL2_Change();
	void GOALKILL2_Update();
	void GOALKILL2_UpdateSprite();

	void GOALKILL3_Start();
	void GOALKILL3_End();
	void GOALKILL3_Change();
	void GOALKILL3_Update();
	void GOALKILL3_UpdateSprite();

	void GOALKILL4_Start();
	void GOALKILL4_End();
	void GOALKILL4_Change();
	void GOALKILL4_Update();
	void GOALKILL4_UpdateSprite();

	void GOALKILLWAIT_Start();
	void GOALKILLWAIT_End();
	void GOALKILLWAIT_Change();
	void GOALKILLWAIT_Update();
	void GOALKILLWAIT_UpdateSprite();
	void GOALKILLWAIT_TransitionToAction(int a);
	void GOALKILLWAIT_TimeIndFrameInc();
	void GOALKILLWAIT_TimeDepFrameInc();
	int GOALKILLWAIT_GetActionLength();
	Tileset * GOALKILLWAIT_GetTileset();

	void GRABSHIP_Start();
	void GRABSHIP_End();
	void GRABSHIP_Change();
	void GRABSHIP_Update();
	void GRABSHIP_UpdateSprite();
	void GRABSHIP_TransitionToAction(int a);
	void GRABSHIP_TimeIndFrameInc();
	void GRABSHIP_TimeDepFrameInc();
	int GRABSHIP_GetActionLength();
	Tileset * GRABSHIP_GetTileset();

	void GRAVREVERSE_Start();
	void GRAVREVERSE_End();
	void GRAVREVERSE_Change();
	void GRAVREVERSE_Update();
	void GRAVREVERSE_UpdateSprite();
	void GRAVREVERSE_TransitionToAction(int a);
	void GRAVREVERSE_TimeIndFrameInc();
	void GRAVREVERSE_TimeDepFrameInc();
	int GRAVREVERSE_GetActionLength();
	Tileset * GRAVREVERSE_GetTileset();

	void GRINDATTACK_Start();
	void GRINDATTACK_End();
	void GRINDATTACK_Change();
	void GRINDATTACK_Update();
	void GRINDATTACK_UpdateSprite();
	void GRINDATTACK_TransitionToAction(int a);
	void GRINDATTACK_TimeIndFrameInc();
	void GRINDATTACK_TimeDepFrameInc();
	int GRINDATTACK_GetActionLength();
	Tileset * GRINDATTACK_GetTileset();

	void GRINDBALL_Start();
	void GRINDBALL_End();
	void GRINDBALL_Change();
	void GRINDBALL_Update();
	void GRINDBALL_UpdateSprite();
	void GRINDBALL_TransitionToAction(int a);
	void GRINDBALL_TimeIndFrameInc();
	void GRINDBALL_TimeDepFrameInc();
	int GRINDBALL_GetActionLength();
	Tileset * GRINDBALL_GetTileset();

	void GRINDLUNGE_Start();
	void GRINDLUNGE_End();
	void GRINDLUNGE_Change();
	void GRINDLUNGE_Update();
	void GRINDLUNGE_UpdateSprite();
	void GRINDLUNGE_TransitionToAction(int a);
	void GRINDLUNGE_TimeIndFrameInc();
	void GRINDLUNGE_TimeDepFrameInc();
	int GRINDLUNGE_GetActionLength();
	Tileset * GRINDLUNGE_GetTileset();

	void GRINDSLASH_Start();
	void GRINDSLASH_End();
	void GRINDSLASH_Change();
	void GRINDSLASH_Update();
	void GRINDSLASH_UpdateSprite();
	void GRINDSLASH_TransitionToAction(int a);
	void GRINDSLASH_TimeIndFrameInc();
	void GRINDSLASH_TimeDepFrameInc();
	int GRINDSLASH_GetActionLength();
	Tileset * GRINDSLASH_GetTileset();

	void GROUNDBLOCKDOWN_Start();
	void GROUNDBLOCKDOWN_End();
	void GROUNDBLOCKDOWN_Change();
	void GROUNDBLOCKDOWN_Update();
	void GROUNDBLOCKDOWN_UpdateSprite();
	void GROUNDBLOCKDOWN_TransitionToAction(int a);
	void GROUNDBLOCKDOWN_TimeIndFrameInc();
	void GROUNDBLOCKDOWN_TimeDepFrameInc();
	int GROUNDBLOCKDOWN_GetActionLength();
	Tileset * GROUNDBLOCKDOWN_GetTileset();

	void GROUNDBLOCKDOWNFORWARD_Start();
	void GROUNDBLOCKDOWNFORWARD_End();
	void GROUNDBLOCKDOWNFORWARD_Change();
	void GROUNDBLOCKDOWNFORWARD_Update();
	void GROUNDBLOCKDOWNFORWARD_UpdateSprite();
	void GROUNDBLOCKDOWNFORWARD_TransitionToAction(int a);
	void GROUNDBLOCKDOWNFORWARD_TimeIndFrameInc();
	void GROUNDBLOCKDOWNFORWARD_TimeDepFrameInc();
	int GROUNDBLOCKDOWNFORWARD_GetActionLength();
	Tileset * GROUNDBLOCKDOWNFORWARD_GetTileset();

	void GROUNDBLOCKFORWARD_Start();
	void GROUNDBLOCKFORWARD_End();
	void GROUNDBLOCKFORWARD_Change();
	void GROUNDBLOCKFORWARD_Update();
	void GROUNDBLOCKFORWARD_UpdateSprite();
	void GROUNDBLOCKFORWARD_TransitionToAction(int a);
	void GROUNDBLOCKFORWARD_TimeIndFrameInc();
	void GROUNDBLOCKFORWARD_TimeDepFrameInc();
	int GROUNDBLOCKFORWARD_GetActionLength();
	Tileset * GROUNDBLOCKFORWARD_GetTileset();

	void GROUNDBLOCKUPFORWARD_Start();
	void GROUNDBLOCKUPFORWARD_End();
	void GROUNDBLOCKUPFORWARD_Change();
	void GROUNDBLOCKUPFORWARD_Update();
	void GROUNDBLOCKUPFORWARD_UpdateSprite();
	void GROUNDBLOCKUPFORWARD_TransitionToAction(int a);
	void GROUNDBLOCKUPFORWARD_TimeIndFrameInc();
	void GROUNDBLOCKUPFORWARD_TimeDepFrameInc();
	int GROUNDBLOCKUPFORWARD_GetActionLength();
	Tileset * GROUNDBLOCKUPFORWARD_GetTileset();

	void GROUNDBLOCKUP_Start();
	void GROUNDBLOCKUP_End();
	void GROUNDBLOCKUP_Change();
	void GROUNDBLOCKUP_Update();
	void GROUNDBLOCKUP_UpdateSprite();
	void GROUNDBLOCKUP_TransitionToAction(int a);
	void GROUNDBLOCKUP_TimeIndFrameInc();
	void GROUNDBLOCKUP_TimeDepFrameInc();
	int GROUNDBLOCKUP_GetActionLength();
	Tileset * GROUNDBLOCKUP_GetTileset();

	void GROUNDHITSTUN_Start();
	void GROUNDHITSTUN_End();
	void GROUNDHITSTUN_Change();
	void GROUNDHITSTUN_Update();
	void GROUNDHITSTUN_UpdateSprite();
	void GROUNDHITSTUN_TransitionToAction(int a);
	void GROUNDHITSTUN_TimeIndFrameInc();
	void GROUNDHITSTUN_TimeDepFrameInc();
	int GROUNDHITSTUN_GetActionLength();
	Tileset * GROUNDHITSTUN_GetTileset();

	void GROUNDPARRY_Start();
	void GROUNDPARRY_End();
	void GROUNDPARRY_Change();
	void GROUNDPARRY_Update();
	void GROUNDPARRY_UpdateSprite();
	void GROUNDPARRY_TransitionToAction(int a);
	void GROUNDPARRY_TimeIndFrameInc();
	void GROUNDPARRY_TimeDepFrameInc();
	int GROUNDPARRY_GetActionLength();
	Tileset * GROUNDPARRY_GetTileset();

	void GROUNDPARRYLOW_Start();
	void GROUNDPARRYLOW_End();
	void GROUNDPARRYLOW_Change();
	void GROUNDPARRYLOW_Update();
	void GROUNDPARRYLOW_UpdateSprite();
	void GROUNDPARRYLOW_TransitionToAction(int a);
	void GROUNDPARRYLOW_TimeIndFrameInc();
	void GROUNDPARRYLOW_TimeDepFrameInc();
	int GROUNDPARRYLOW_GetActionLength();
	Tileset * GROUNDPARRYLOW_GetTileset();

	void GROUNDTECHBACK_Start();
	void GROUNDTECHBACK_End();
	void GROUNDTECHBACK_Change();
	void GROUNDTECHBACK_Update();
	void GROUNDTECHBACK_UpdateSprite();
	void GROUNDTECHBACK_TransitionToAction(int a);
	void GROUNDTECHBACK_TimeIndFrameInc();
	void GROUNDTECHBACK_TimeDepFrameInc();
	int GROUNDTECHBACK_GetActionLength();
	Tileset * GROUNDTECHBACK_GetTileset();

	void GROUNDTECHFORWARD_Start();
	void GROUNDTECHFORWARD_End();
	void GROUNDTECHFORWARD_Change();
	void GROUNDTECHFORWARD_Update();
	void GROUNDTECHFORWARD_UpdateSprite();
	void GROUNDTECHFORWARD_TransitionToAction(int a);
	void GROUNDTECHFORWARD_TimeIndFrameInc();
	void GROUNDTECHFORWARD_TimeDepFrameInc();
	int GROUNDTECHFORWARD_GetActionLength();
	Tileset * GROUNDTECHFORWARD_GetTileset();

	void GROUNDTECHINPLACE_Start();
	void GROUNDTECHINPLACE_End();
	void GROUNDTECHINPLACE_Change();
	void GROUNDTECHINPLACE_Update();
	void GROUNDTECHINPLACE_UpdateSprite();
	void GROUNDTECHINPLACE_TransitionToAction(int a);
	void GROUNDTECHINPLACE_TimeIndFrameInc();
	void GROUNDTECHINPLACE_TimeDepFrameInc();
	int GROUNDTECHINPLACE_GetActionLength();
	Tileset * GROUNDTECHINPLACE_GetTileset();

	void HIDDEN_Start();
	void HIDDEN_End();
	void HIDDEN_Change();
	void HIDDEN_Update();
	void HIDDEN_UpdateSprite();
	void HIDDEN_TransitionToAction(int a);
	void HIDDEN_TimeIndFrameInc();
	void HIDDEN_TimeDepFrameInc();
	int HIDDEN_GetActionLength();
	Tileset * HIDDEN_GetTileset();

	void HOMINGATTACK_Start();
	void HOMINGATTACK_End();
	void HOMINGATTACK_Change();
	void HOMINGATTACK_Update();
	void HOMINGATTACK_UpdateSprite();
	void HOMINGATTACK_TransitionToAction(int a);
	void HOMINGATTACK_TimeIndFrameInc();
	void HOMINGATTACK_TimeDepFrameInc();
	int HOMINGATTACK_GetActionLength();
	Tileset * HOMINGATTACK_GetTileset();

	void INTRO_Start();
	void INTRO_End();
	void INTRO_Change();
	void INTRO_Update();
	void INTRO_UpdateSprite();
	void INTRO_TransitionToAction(int a);
	void INTRO_TimeIndFrameInc();
	void INTRO_TimeDepFrameInc();
	int INTRO_GetActionLength();
	Tileset * INTRO_GetTileset();

	void INTROBOOST_Start();
	void INTROBOOST_End();
	void INTROBOOST_Change();
	void INTROBOOST_Update();
	void INTROBOOST_UpdateSprite();
	void INTROBOOST_TransitionToAction(int a);
	void INTROBOOST_TimeIndFrameInc();
	void INTROBOOST_TimeDepFrameInc();
	int INTROBOOST_GetActionLength();
	Tileset * INTROBOOST_GetTileset();

	void JUMP_Start();
	void JUMP_End();
	void JUMP_Change();
	void JUMP_Update();
	void JUMP_UpdateSprite();
	void JUMP_TransitionToAction(int a);
	void JUMP_TimeIndFrameInc();
	void JUMP_TimeDepFrameInc();
	int JUMP_GetActionLength();
	Tileset * JUMP_GetTileset();

	void JUMPSQUAT_Start();
	void JUMPSQUAT_End();
	void JUMPSQUAT_Change();
	void JUMPSQUAT_Update();
	void JUMPSQUAT_UpdateSprite();
	void JUMPSQUAT_TransitionToAction(int a);
	void JUMPSQUAT_TimeIndFrameInc();
	void JUMPSQUAT_TimeDepFrameInc();
	int JUMPSQUAT_GetActionLength();
	Tileset * JUMPSQUAT_GetTileset();

	void LAND_Start();
	void LAND_End();
	void LAND_Change();
	void LAND_Update();
	void LAND_UpdateSprite();
	void LAND_TransitionToAction(int a);
	void LAND_TimeIndFrameInc();
	void LAND_TimeDepFrameInc();
	int LAND_GetActionLength();
	Tileset * LAND_GetTileset();

	void LAND2_Start();
	void LAND2_End();
	void LAND2_Change();
	void LAND2_Update();
	void LAND2_UpdateSprite();
	void LAND2_TransitionToAction(int a);
	void LAND2_TimeIndFrameInc();
	void LAND2_TimeDepFrameInc();
	int LAND2_GetActionLength();
	Tileset * LAND2_GetTileset();

	void LOCKEDRAILSLIDE_Start();
	void LOCKEDRAILSLIDE_End();
	void LOCKEDRAILSLIDE_Change();
	void LOCKEDRAILSLIDE_Update();
	void LOCKEDRAILSLIDE_UpdateSprite();
	void LOCKEDRAILSLIDE_TransitionToAction(int a);
	void LOCKEDRAILSLIDE_TimeIndFrameInc();
	void LOCKEDRAILSLIDE_TimeDepFrameInc();
	int LOCKEDRAILSLIDE_GetActionLength();
	Tileset * LOCKEDRAILSLIDE_GetTileset();

	void NEXUSKILL_Start();
	void NEXUSKILL_End();
	void NEXUSKILL_Change();
	void NEXUSKILL_Update();
	void NEXUSKILL_UpdateSprite();
	void NEXUSKILL_TransitionToAction(int a);
	void NEXUSKILL_TimeIndFrameInc();
	void NEXUSKILL_TimeDepFrameInc();
	int NEXUSKILL_GetActionLength();
	Tileset * NEXUSKILL_GetTileset();

	void RAILBOUNCE_Start();
	void RAILBOUNCE_End();
	void RAILBOUNCE_Change();
	void RAILBOUNCE_Update();
	void RAILBOUNCE_UpdateSprite();
	void RAILBOUNCE_TransitionToAction(int a);
	void RAILBOUNCE_TimeIndFrameInc();
	void RAILBOUNCE_TimeDepFrameInc();
	int RAILBOUNCE_GetActionLength();
	Tileset * RAILBOUNCE_GetTileset();

	void RAILBOUNCEGROUND_Start();
	void RAILBOUNCEGROUND_End();
	void RAILBOUNCEGROUND_Change();
	void RAILBOUNCEGROUND_Update();
	void RAILBOUNCEGROUND_UpdateSprite();
	void RAILBOUNCEGROUND_TransitionToAction(int a);
	void RAILBOUNCEGROUND_TimeIndFrameInc();
	void RAILBOUNCEGROUND_TimeDepFrameInc();
	int RAILBOUNCEGROUND_GetActionLength();
	Tileset * RAILBOUNCEGROUND_GetTileset();

	void RAILDASH_Start();
	void RAILDASH_End();
	void RAILDASH_Change();
	void RAILDASH_Update();
	void RAILDASH_UpdateSprite();
	void RAILDASH_TransitionToAction(int a);
	void RAILDASH_TimeIndFrameInc();
	void RAILDASH_TimeDepFrameInc();
	int RAILDASH_GetActionLength();
	Tileset * RAILDASH_GetTileset();

	void RAILGRIND_Start();
	void RAILGRIND_End();
	void RAILGRIND_Change();
	void RAILGRIND_Update();
	void RAILGRIND_UpdateSprite();
	void RAILGRIND_TransitionToAction(int a);
	void RAILGRIND_TimeIndFrameInc();
	void RAILGRIND_TimeDepFrameInc();
	int RAILGRIND_GetActionLength();
	Tileset * RAILGRIND_GetTileset();

	void RAILSLIDE_Start();
	void RAILSLIDE_End();
	void RAILSLIDE_Change();
	void RAILSLIDE_Update();
	void RAILSLIDE_UpdateSprite();
	void RAILSLIDE_TransitionToAction(int a);
	void RAILSLIDE_TimeIndFrameInc();
	void RAILSLIDE_TimeDepFrameInc();
	int RAILSLIDE_GetActionLength();
	Tileset * RAILSLIDE_GetTileset();

	void RIDESHIP_Start();
	void RIDESHIP_End();
	void RIDESHIP_Change();
	void RIDESHIP_Update();
	void RIDESHIP_UpdateSprite();
	void RIDESHIP_TransitionToAction(int a);
	void RIDESHIP_TimeIndFrameInc();
	void RIDESHIP_TimeDepFrameInc();
	int RIDESHIP_GetActionLength();
	Tileset * RIDESHIP_GetTileset();

	void RUN_Start();
	void RUN_End();
	void RUN_Change();
	void RUN_Update();
	void RUN_UpdateSprite();
	void RUN_TransitionToAction(int a);
	void RUN_TimeIndFrameInc();
	void RUN_TimeDepFrameInc();
	int RUN_GetActionLength();
	Tileset * RUN_GetTileset();

	void SEQ_CRAWLERFIGHT_DODGEBACK_Start();
	void SEQ_CRAWLERFIGHT_DODGEBACK_End();
	void SEQ_CRAWLERFIGHT_DODGEBACK_Change();
	void SEQ_CRAWLERFIGHT_DODGEBACK_Update();
	void SEQ_CRAWLERFIGHT_DODGEBACK_UpdateSprite();
	void SEQ_CRAWLERFIGHT_DODGEBACK_TransitionToAction(int a);
	void SEQ_CRAWLERFIGHT_DODGEBACK_TimeIndFrameInc();
	void SEQ_CRAWLERFIGHT_DODGEBACK_TimeDepFrameInc();
	int SEQ_CRAWLERFIGHT_DODGEBACK_GetActionLength();
	Tileset * SEQ_CRAWLERFIGHT_DODGEBACK_GetTileset();

	void SEQ_CRAWLERFIGHT_LAND_Start();
	void SEQ_CRAWLERFIGHT_LAND_End();
	void SEQ_CRAWLERFIGHT_LAND_Change();
	void SEQ_CRAWLERFIGHT_LAND_Update();
	void SEQ_CRAWLERFIGHT_LAND_UpdateSprite();
	void SEQ_CRAWLERFIGHT_LAND_TransitionToAction(int a);
	void SEQ_CRAWLERFIGHT_LAND_TimeIndFrameInc();
	void SEQ_CRAWLERFIGHT_LAND_TimeDepFrameInc();
	int SEQ_CRAWLERFIGHT_LAND_GetActionLength();
	Tileset * SEQ_CRAWLERFIGHT_LAND_GetTileset();

	void SEQ_CRAWLERFIGHT_STAND_Start();
	void SEQ_CRAWLERFIGHT_STAND_End();
	void SEQ_CRAWLERFIGHT_STAND_Change();
	void SEQ_CRAWLERFIGHT_STAND_Update();
	void SEQ_CRAWLERFIGHT_STAND_UpdateSprite();
	void SEQ_CRAWLERFIGHT_STAND_TransitionToAction(int a);
	void SEQ_CRAWLERFIGHT_STAND_TimeIndFrameInc();
	void SEQ_CRAWLERFIGHT_STAND_TimeDepFrameInc();
	int SEQ_CRAWLERFIGHT_STAND_GetActionLength();
	Tileset * SEQ_CRAWLERFIGHT_STAND_GetTileset();

	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_Start();
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_End();
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_Change();
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_Update();
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_UpdateSprite();
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_TransitionToAction(int a);
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeIndFrameInc();
	void SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeDepFrameInc();
	int SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetActionLength();
	Tileset * SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetTileset();

	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Start();
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_End();
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Change();
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Update();
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_UpdateSprite();
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TransitionToAction(int a);
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeIndFrameInc();
	void SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeDepFrameInc();
	int SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetActionLength();
	Tileset * SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetTileset();

	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Start();
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_End();
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Change();
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Update();
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_UpdateSprite();
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TransitionToAction(int a);
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeIndFrameInc();
	void SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeDepFrameInc();
	int SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetActionLength();
	Tileset * SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetTileset();

	void SEQ_ENTERCORE1_Start();
	void SEQ_ENTERCORE1_End();
	void SEQ_ENTERCORE1_Change();
	void SEQ_ENTERCORE1_Update();
	void SEQ_ENTERCORE1_UpdateSprite();
	void SEQ_ENTERCORE1_TransitionToAction(int a);
	void SEQ_ENTERCORE1_TimeIndFrameInc();
	void SEQ_ENTERCORE1_TimeDepFrameInc();
	int SEQ_ENTERCORE1_GetActionLength();
	Tileset * SEQ_ENTERCORE1_GetTileset();

	void SEQ_FADE_INTO_NEXUS_Start();
	void SEQ_FADE_INTO_NEXUS_End();
	void SEQ_FADE_INTO_NEXUS_Change();
	void SEQ_FADE_INTO_NEXUS_Update();
	void SEQ_FADE_INTO_NEXUS_UpdateSprite();
	void SEQ_FADE_INTO_NEXUS_TransitionToAction(int a);
	void SEQ_FADE_INTO_NEXUS_TimeIndFrameInc();
	void SEQ_FADE_INTO_NEXUS_TimeDepFrameInc();
	int SEQ_FADE_INTO_NEXUS_GetActionLength();
	Tileset * SEQ_FADE_INTO_NEXUS_GetTileset();

	void SEQ_FLOAT_TO_NEXUS_OPENING_Start();
	void SEQ_FLOAT_TO_NEXUS_OPENING_End();
	void SEQ_FLOAT_TO_NEXUS_OPENING_Change();
	void SEQ_FLOAT_TO_NEXUS_OPENING_Update();
	void SEQ_FLOAT_TO_NEXUS_OPENING_UpdateSprite();
	void SEQ_FLOAT_TO_NEXUS_OPENING_TransitionToAction(int a);
	void SEQ_FLOAT_TO_NEXUS_OPENING_TimeIndFrameInc();
	void SEQ_FLOAT_TO_NEXUS_OPENING_TimeDepFrameInc();
	int SEQ_FLOAT_TO_NEXUS_OPENING_GetActionLength();
	Tileset * SEQ_FLOAT_TO_NEXUS_OPENING_GetTileset();

	void SEQ_GATORSTUN_Start();
	void SEQ_GATORSTUN_End();
	void SEQ_GATORSTUN_Change();
	void SEQ_GATORSTUN_Update();
	void SEQ_GATORSTUN_UpdateSprite();
	void SEQ_GATORSTUN_TransitionToAction(int a);
	void SEQ_GATORSTUN_TimeIndFrameInc();
	void SEQ_GATORSTUN_TimeDepFrameInc();
	int SEQ_GATORSTUN_GetActionLength();
	Tileset * SEQ_GATORSTUN_GetTileset();

	void SEQ_KINFALL_Start();
	void SEQ_KINFALL_End();
	void SEQ_KINFALL_Change();
	void SEQ_KINFALL_Update();
	void SEQ_KINFALL_UpdateSprite();
	void SEQ_KINFALL_TransitionToAction(int a);
	void SEQ_KINFALL_TimeIndFrameInc();
	void SEQ_KINFALL_TimeDepFrameInc();
	int SEQ_KINFALL_GetActionLength();
	Tileset * SEQ_KINFALL_GetTileset();

	void SEQ_KINSTAND_Start();
	void SEQ_KINSTAND_End();
	void SEQ_KINSTAND_Change();
	void SEQ_KINSTAND_Update();
	void SEQ_KINSTAND_UpdateSprite();
	void SEQ_KINSTAND_TransitionToAction(int a);
	void SEQ_KINSTAND_TimeIndFrameInc();
	void SEQ_KINSTAND_TimeDepFrameInc();
	int SEQ_KINSTAND_GetActionLength();
	Tileset * SEQ_KINSTAND_GetTileset();

	void SEQ_KINTHROWN_Start();
	void SEQ_KINTHROWN_End();
	void SEQ_KINTHROWN_Change();
	void SEQ_KINTHROWN_Update();
	void SEQ_KINTHROWN_UpdateSprite();
	void SEQ_KINTHROWN_TransitionToAction(int a);
	void SEQ_KINTHROWN_TimeIndFrameInc();
	void SEQ_KINTHROWN_TimeDepFrameInc();
	int SEQ_KINTHROWN_GetActionLength();
	Tileset * SEQ_KINTHROWN_GetTileset();

	void SEQ_KNEEL_Start();
	void SEQ_KNEEL_End();
	void SEQ_KNEEL_Change();
	void SEQ_KNEEL_Update();
	void SEQ_KNEEL_UpdateSprite();
	void SEQ_KNEEL_TransitionToAction(int a);
	void SEQ_KNEEL_TimeIndFrameInc();
	void SEQ_KNEEL_TimeDepFrameInc();
	int SEQ_KNEEL_GetActionLength();
	Tileset * SEQ_KNEEL_GetTileset();

	void SEQ_KNEEL_TO_MEDITATE_Start();
	void SEQ_KNEEL_TO_MEDITATE_End();
	void SEQ_KNEEL_TO_MEDITATE_Change();
	void SEQ_KNEEL_TO_MEDITATE_Update();
	void SEQ_KNEEL_TO_MEDITATE_UpdateSprite();
	void SEQ_KNEEL_TO_MEDITATE_TransitionToAction(int a);
	void SEQ_KNEEL_TO_MEDITATE_TimeIndFrameInc();
	void SEQ_KNEEL_TO_MEDITATE_TimeDepFrameInc();
	int SEQ_KNEEL_TO_MEDITATE_GetActionLength();
	Tileset * SEQ_KNEEL_TO_MEDITATE_GetTileset();

	void SEQ_LOOKUP_Start();
	void SEQ_LOOKUP_End();
	void SEQ_LOOKUP_Change();
	void SEQ_LOOKUP_Update();
	void SEQ_LOOKUP_UpdateSprite();
	void SEQ_LOOKUP_TransitionToAction(int a);
	void SEQ_LOOKUP_TimeIndFrameInc();
	void SEQ_LOOKUP_TimeDepFrameInc();
	int SEQ_LOOKUP_GetActionLength();
	Tileset * SEQ_LOOKUP_GetTileset();

	void SEQ_LOOKUPDISAPPEAR_Start();
	void SEQ_LOOKUPDISAPPEAR_End();
	void SEQ_LOOKUPDISAPPEAR_Change();
	void SEQ_LOOKUPDISAPPEAR_Update();
	void SEQ_LOOKUPDISAPPEAR_UpdateSprite();
	void SEQ_LOOKUPDISAPPEAR_TransitionToAction(int a);
	void SEQ_LOOKUPDISAPPEAR_TimeIndFrameInc();
	void SEQ_LOOKUPDISAPPEAR_TimeDepFrameInc();
	int SEQ_LOOKUPDISAPPEAR_GetActionLength();
	Tileset * SEQ_LOOKUPDISAPPEAR_GetTileset();

	void SEQ_MASKOFF_Start();
	void SEQ_MASKOFF_End();
	void SEQ_MASKOFF_Change();
	void SEQ_MASKOFF_Update();
	void SEQ_MASKOFF_UpdateSprite();
	void SEQ_MASKOFF_TransitionToAction(int a);
	void SEQ_MASKOFF_TimeIndFrameInc();
	void SEQ_MASKOFF_TimeDepFrameInc();
	int SEQ_MASKOFF_GetActionLength();
	Tileset * SEQ_MASKOFF_GetTileset();

	void SEQ_MEDITATE_Start();
	void SEQ_MEDITATE_End();
	void SEQ_MEDITATE_Change();
	void SEQ_MEDITATE_Update();
	void SEQ_MEDITATE_UpdateSprite();
	void SEQ_MEDITATE_TransitionToAction(int a);
	void SEQ_MEDITATE_TimeIndFrameInc();
	void SEQ_MEDITATE_TimeDepFrameInc();
	int SEQ_MEDITATE_GetActionLength();
	Tileset * SEQ_MEDITATE_GetTileset();

	void SEQ_MEDITATE_MASKON_Start();
	void SEQ_MEDITATE_MASKON_End();
	void SEQ_MEDITATE_MASKON_Change();
	void SEQ_MEDITATE_MASKON_Update();
	void SEQ_MEDITATE_MASKON_UpdateSprite();
	void SEQ_MEDITATE_MASKON_TransitionToAction(int a);
	void SEQ_MEDITATE_MASKON_TimeIndFrameInc();
	void SEQ_MEDITATE_MASKON_TimeDepFrameInc();
	int SEQ_MEDITATE_MASKON_GetActionLength();
	Tileset * SEQ_MEDITATE_MASKON_GetTileset();

	void SEQ_TURNFACE_Start();
	void SEQ_TURNFACE_End();
	void SEQ_TURNFACE_Change();
	void SEQ_TURNFACE_Update();
	void SEQ_TURNFACE_UpdateSprite();
	void SEQ_TURNFACE_TransitionToAction(int a);
	void SEQ_TURNFACE_TimeIndFrameInc();
	void SEQ_TURNFACE_TimeDepFrameInc();
	int SEQ_TURNFACE_GetActionLength();
	Tileset * SEQ_TURNFACE_GetTileset();

	void SEQ_WAIT_Start();
	void SEQ_WAIT_End();
	void SEQ_WAIT_Change();
	void SEQ_WAIT_Update();
	void SEQ_WAIT_UpdateSprite();
	void SEQ_WAIT_TransitionToAction(int a);
	void SEQ_WAIT_TimeIndFrameInc();
	void SEQ_WAIT_TimeDepFrameInc();
	int SEQ_WAIT_GetActionLength();
	Tileset * SEQ_WAIT_GetTileset();

	void SKYDIVE_Start();
	void SKYDIVE_End();
	void SKYDIVE_Change();
	void SKYDIVE_Update();
	void SKYDIVE_UpdateSprite();
	void SKYDIVE_TransitionToAction(int a);
	void SKYDIVE_TimeIndFrameInc();
	void SKYDIVE_TimeDepFrameInc();
	int SKYDIVE_GetActionLength();
	Tileset * SKYDIVE_GetTileset();

	void SKYDIVETOFALL_Start();
	void SKYDIVETOFALL_End();
	void SKYDIVETOFALL_Change();
	void SKYDIVETOFALL_Update();
	void SKYDIVETOFALL_UpdateSprite();
	void SKYDIVETOFALL_TransitionToAction(int a);
	void SKYDIVETOFALL_TimeIndFrameInc();
	void SKYDIVETOFALL_TimeDepFrameInc();
	int SKYDIVETOFALL_GetActionLength();
	Tileset * SKYDIVETOFALL_GetTileset();

	void SLIDE_Start();
	void SLIDE_End();
	void SLIDE_Change();
	void SLIDE_Update();
	void SLIDE_UpdateSprite();
	void SLIDE_TransitionToAction(int a);
	void SLIDE_TimeIndFrameInc();
	void SLIDE_TimeDepFrameInc();
	int SLIDE_GetActionLength();
	Tileset * SLIDE_GetTileset();

	void SPAWNWAIT_Start();
	void SPAWNWAIT_End();
	void SPAWNWAIT_Change();
	void SPAWNWAIT_Update();
	void SPAWNWAIT_UpdateSprite();
	void SPAWNWAIT_TransitionToAction(int a);
	void SPAWNWAIT_TimeIndFrameInc();
	void SPAWNWAIT_TimeDepFrameInc();
	int SPAWNWAIT_GetActionLength();
	Tileset * SPAWNWAIT_GetTileset();

	void SPRINGSTUN_Start();
	void SPRINGSTUN_End();
	void SPRINGSTUN_Change();
	void SPRINGSTUN_Update();
	void SPRINGSTUN_UpdateSprite();
	void SPRINGSTUN_TransitionToAction(int a);
	void SPRINGSTUN_TimeIndFrameInc();
	void SPRINGSTUN_TimeDepFrameInc();
	int SPRINGSTUN_GetActionLength();
	Tileset * SPRINGSTUN_GetTileset();

	void SPRINGSTUNAIM_Start();
	void SPRINGSTUNAIM_End();
	void SPRINGSTUNAIM_Change();
	void SPRINGSTUNAIM_Update();
	void SPRINGSTUNAIM_UpdateSprite();
	void SPRINGSTUNAIM_TransitionToAction(int a);
	void SPRINGSTUNAIM_TimeIndFrameInc();
	void SPRINGSTUNAIM_TimeDepFrameInc();
	int SPRINGSTUNAIM_GetActionLength();
	Tileset * SPRINGSTUNAIM_GetTileset();

	void SPRINGSTUNAIRBOUNCE_Start();
	void SPRINGSTUNAIRBOUNCE_End();
	void SPRINGSTUNAIRBOUNCE_Change();
	void SPRINGSTUNAIRBOUNCE_Update();
	void SPRINGSTUNAIRBOUNCE_UpdateSprite();
	void SPRINGSTUNAIRBOUNCE_TransitionToAction(int a);
	void SPRINGSTUNAIRBOUNCE_TimeIndFrameInc();
	void SPRINGSTUNAIRBOUNCE_TimeDepFrameInc();
	int SPRINGSTUNAIRBOUNCE_GetActionLength();
	Tileset * SPRINGSTUNAIRBOUNCE_GetTileset();

	void SPRINGSTUNAIRBOUNCEPAUSE_Start();
	void SPRINGSTUNAIRBOUNCEPAUSE_End();
	void SPRINGSTUNAIRBOUNCEPAUSE_Change();
	void SPRINGSTUNAIRBOUNCEPAUSE_Update();
	void SPRINGSTUNAIRBOUNCEPAUSE_UpdateSprite();
	void SPRINGSTUNAIRBOUNCEPAUSE_TransitionToAction(int a);
	void SPRINGSTUNAIRBOUNCEPAUSE_TimeIndFrameInc();
	void SPRINGSTUNAIRBOUNCEPAUSE_TimeDepFrameInc();
	int SPRINGSTUNAIRBOUNCEPAUSE_GetActionLength();
	Tileset * SPRINGSTUNAIRBOUNCEPAUSE_GetTileset();

	void SPRINGSTUNANNIHILATION_Start();
	void SPRINGSTUNANNIHILATION_End();
	void SPRINGSTUNANNIHILATION_Change();
	void SPRINGSTUNANNIHILATION_Update();
	void SPRINGSTUNANNIHILATION_UpdateSprite();
	void SPRINGSTUNANNIHILATION_TransitionToAction(int a);
	void SPRINGSTUNANNIHILATION_TimeIndFrameInc();
	void SPRINGSTUNANNIHILATION_TimeDepFrameInc();
	int SPRINGSTUNANNIHILATION_GetActionLength();
	Tileset * SPRINGSTUNANNIHILATION_GetTileset();

	void SPRINGSTUNANNIHILATIONATTACK_Start();
	void SPRINGSTUNANNIHILATIONATTACK_End();
	void SPRINGSTUNANNIHILATIONATTACK_Change();
	void SPRINGSTUNANNIHILATIONATTACK_Update();
	void SPRINGSTUNANNIHILATIONATTACK_UpdateSprite();
	void SPRINGSTUNANNIHILATIONATTACK_TransitionToAction(int a);
	void SPRINGSTUNANNIHILATIONATTACK_TimeIndFrameInc();
	void SPRINGSTUNANNIHILATIONATTACK_TimeDepFrameInc();
	int SPRINGSTUNANNIHILATIONATTACK_GetActionLength();
	Tileset * SPRINGSTUNANNIHILATIONATTACK_GetTileset();

	

	void SPRINGSTUNBOUNCE_Start();
	void SPRINGSTUNBOUNCE_End();
	void SPRINGSTUNBOUNCE_Change();
	void SPRINGSTUNBOUNCE_Update();
	void SPRINGSTUNBOUNCE_UpdateSprite();
	void SPRINGSTUNBOUNCE_TransitionToAction(int a);
	void SPRINGSTUNBOUNCE_TimeIndFrameInc();
	void SPRINGSTUNBOUNCE_TimeDepFrameInc();
	int SPRINGSTUNBOUNCE_GetActionLength();
	Tileset * SPRINGSTUNBOUNCE_GetTileset();

	void SPRINGSTUNBOUNCEGROUND_Start();
	void SPRINGSTUNBOUNCEGROUND_End();
	void SPRINGSTUNBOUNCEGROUND_Change();
	void SPRINGSTUNBOUNCEGROUND_Update();
	void SPRINGSTUNBOUNCEGROUND_UpdateSprite();
	void SPRINGSTUNBOUNCEGROUND_TransitionToAction(int a);
	void SPRINGSTUNBOUNCEGROUND_TimeIndFrameInc();
	void SPRINGSTUNBOUNCEGROUND_TimeDepFrameInc();
	int SPRINGSTUNBOUNCEGROUND_GetActionLength();
	Tileset * SPRINGSTUNBOUNCEGROUND_GetTileset();

	void SPRINGSTUNGLIDE_Start();
	void SPRINGSTUNGLIDE_End();
	void SPRINGSTUNGLIDE_Change();
	void SPRINGSTUNGLIDE_Update();
	void SPRINGSTUNGLIDE_UpdateSprite();
	void SPRINGSTUNGLIDE_TransitionToAction(int a);
	void SPRINGSTUNGLIDE_TimeIndFrameInc();
	void SPRINGSTUNGLIDE_TimeDepFrameInc();
	int SPRINGSTUNGLIDE_GetActionLength();
	Tileset * SPRINGSTUNGLIDE_GetTileset();

	void SPRINGSTUNGRIND_Start();
	void SPRINGSTUNGRIND_End();
	void SPRINGSTUNGRIND_Change();
	void SPRINGSTUNGRIND_Update();
	void SPRINGSTUNGRIND_UpdateSprite();
	void SPRINGSTUNGRIND_TransitionToAction(int a);
	void SPRINGSTUNGRIND_TimeIndFrameInc();
	void SPRINGSTUNGRIND_TimeDepFrameInc();
	int SPRINGSTUNGRIND_GetActionLength();
	Tileset * SPRINGSTUNGRIND_GetTileset();

	void SPRINGSTUNGRINDFLY_Start();
	void SPRINGSTUNGRINDFLY_End();
	void SPRINGSTUNGRINDFLY_Change();
	void SPRINGSTUNGRINDFLY_Update();
	void SPRINGSTUNGRINDFLY_UpdateSprite();
	void SPRINGSTUNGRINDFLY_TransitionToAction(int a);
	void SPRINGSTUNGRINDFLY_TimeIndFrameInc();
	void SPRINGSTUNGRINDFLY_TimeDepFrameInc();
	int SPRINGSTUNGRINDFLY_GetActionLength();
	Tileset * SPRINGSTUNGRINDFLY_GetTileset();

	void SPRINGSTUNHOMING_Start();
	void SPRINGSTUNHOMING_End();
	void SPRINGSTUNHOMING_Change();
	void SPRINGSTUNHOMING_Update();
	void SPRINGSTUNHOMING_UpdateSprite();
	void SPRINGSTUNHOMING_TransitionToAction(int a);
	void SPRINGSTUNHOMING_TimeIndFrameInc();
	void SPRINGSTUNHOMING_TimeDepFrameInc();
	int SPRINGSTUNHOMING_GetActionLength();
	Tileset * SPRINGSTUNHOMING_GetTileset();


	void SPRINGSTUNHOMINGATTACK_Start();
	void SPRINGSTUNHOMINGATTACK_End();
	void SPRINGSTUNHOMINGATTACK_Change();
	void SPRINGSTUNHOMINGATTACK_Update();
	void SPRINGSTUNHOMINGATTACK_UpdateSprite();
	void SPRINGSTUNHOMINGATTACK_TransitionToAction(int a);
	void SPRINGSTUNHOMINGATTACK_TimeIndFrameInc();
	void SPRINGSTUNHOMINGATTACK_TimeDepFrameInc();
	int SPRINGSTUNHOMINGATTACK_GetActionLength();
	Tileset * SPRINGSTUNHOMINGATTACK_GetTileset();

	void SPRINGSTUNTELEPORT_Start();
	void SPRINGSTUNTELEPORT_End();
	void SPRINGSTUNTELEPORT_Change();
	void SPRINGSTUNTELEPORT_Update();
	void SPRINGSTUNTELEPORT_UpdateSprite();
	void SPRINGSTUNTELEPORT_TransitionToAction(int a);
	void SPRINGSTUNTELEPORT_TimeIndFrameInc();
	void SPRINGSTUNTELEPORT_TimeDepFrameInc();
	int SPRINGSTUNTELEPORT_GetActionLength();
	Tileset * SPRINGSTUNTELEPORT_GetTileset();

	void SPRINT_Start();
	void SPRINT_End();
	void SPRINT_Change();
	void SPRINT_Update();
	void SPRINT_UpdateSprite();
	void SPRINT_TransitionToAction(int a);
	void SPRINT_TimeIndFrameInc();
	void SPRINT_TimeDepFrameInc();
	int SPRINT_GetActionLength();
	Tileset * SPRINT_GetTileset();

	void STAND_Start();
	void STAND_End();
	void STAND_Change();
	void STAND_Update();
	void STAND_UpdateSprite();
	void STAND_TransitionToAction(int a);
	void STAND_TimeIndFrameInc();
	void STAND_TimeDepFrameInc();
	int STAND_GetActionLength();
	Tileset * STAND_GetTileset();

	void STANDATTACK1_Start();
	void STANDATTACK1_End();
	void STANDATTACK1_Change();
	void STANDATTACK1_Update();
	void STANDATTACK1_UpdateSprite();
	void STANDATTACK1_TransitionToAction(int a);
	void STANDATTACK1_TimeIndFrameInc();
	void STANDATTACK1_TimeDepFrameInc();
	int STANDATTACK1_GetActionLength();
	Tileset * STANDATTACK1_GetTileset();

	void STANDATTACK2_Start();
	void STANDATTACK2_End();
	void STANDATTACK2_Change();
	void STANDATTACK2_Update();
	void STANDATTACK2_UpdateSprite();
	void STANDATTACK2_TransitionToAction(int a);
	void STANDATTACK2_TimeIndFrameInc();
	void STANDATTACK2_TimeDepFrameInc();
	int STANDATTACK2_GetActionLength();
	Tileset * STANDATTACK2_GetTileset();

	void STANDATTACK3_Start();
	void STANDATTACK3_End();
	void STANDATTACK3_Change();
	void STANDATTACK3_Update();
	void STANDATTACK3_UpdateSprite();
	void STANDATTACK3_TransitionToAction(int a);
	void STANDATTACK3_TimeIndFrameInc();
	void STANDATTACK3_TimeDepFrameInc();
	int STANDATTACK3_GetActionLength();
	Tileset * STANDATTACK3_GetTileset();

	void STANDATTACK4_Start();
	void STANDATTACK4_End();
	void STANDATTACK4_Change();
	void STANDATTACK4_Update();
	void STANDATTACK4_UpdateSprite();
	void STANDATTACK4_TransitionToAction(int a);
	void STANDATTACK4_TimeIndFrameInc();
	void STANDATTACK4_TimeDepFrameInc();
	int STANDATTACK4_GetActionLength();
	Tileset * STANDATTACK4_GetTileset();

	void STEEPCLIMB_Start();
	void STEEPCLIMB_End();
	void STEEPCLIMB_Change();
	void STEEPCLIMB_Update();
	void STEEPCLIMB_UpdateSprite();
	void STEEPCLIMB_TransitionToAction(int a);
	void STEEPCLIMB_TimeIndFrameInc();
	void STEEPCLIMB_TimeDepFrameInc();
	int STEEPCLIMB_GetActionLength();
	Tileset * STEEPCLIMB_GetTileset();

	void STEEPCLIMBATTACK_Start();
	void STEEPCLIMBATTACK_End();
	void STEEPCLIMBATTACK_Change();
	void STEEPCLIMBATTACK_Update();
	void STEEPCLIMBATTACK_UpdateSprite();
	void STEEPCLIMBATTACK_TransitionToAction(int a);
	void STEEPCLIMBATTACK_TimeIndFrameInc();
	void STEEPCLIMBATTACK_TimeDepFrameInc();
	int STEEPCLIMBATTACK_GetActionLength();
	Tileset * STEEPCLIMBATTACK_GetTileset();

	void STEEPCLING_Start();
	void STEEPCLING_End();
	void STEEPCLING_Change();
	void STEEPCLING_Update();
	void STEEPCLING_UpdateSprite();
	void STEEPCLING_TransitionToAction(int a);
	void STEEPCLING_TimeIndFrameInc();
	void STEEPCLING_TimeDepFrameInc();
	int STEEPCLING_GetActionLength();
	Tileset * STEEPCLING_GetTileset();

	void STEEPSLIDE_Start();
	void STEEPSLIDE_End();
	void STEEPSLIDE_Change();
	void STEEPSLIDE_Update();
	void STEEPSLIDE_UpdateSprite();
	void STEEPSLIDE_TransitionToAction(int a);
	void STEEPSLIDE_TimeIndFrameInc();
	void STEEPSLIDE_TimeDepFrameInc();
	int STEEPSLIDE_GetActionLength();
	Tileset * STEEPSLIDE_GetTileset();

	void STEEPSLIDEATTACK_Start();
	void STEEPSLIDEATTACK_End();
	void STEEPSLIDEATTACK_Change();
	void STEEPSLIDEATTACK_Update();
	void STEEPSLIDEATTACK_UpdateSprite();
	void STEEPSLIDEATTACK_TransitionToAction(int a);
	void STEEPSLIDEATTACK_TimeIndFrameInc();
	void STEEPSLIDEATTACK_TimeDepFrameInc();
	int STEEPSLIDEATTACK_GetActionLength();
	Tileset * STEEPSLIDEATTACK_GetTileset();

	void SWINGSTUN_Start();
	void SWINGSTUN_End();
	void SWINGSTUN_Change();
	void SWINGSTUN_Update();
	void SWINGSTUN_UpdateSprite();
	void SWINGSTUN_TransitionToAction(int a);
	void SWINGSTUN_TimeIndFrameInc();
	void SWINGSTUN_TimeDepFrameInc();
	int SWINGSTUN_GetActionLength();
	Tileset * SWINGSTUN_GetTileset();

	void TESTSUPER_Start();
	void TESTSUPER_End();
	void TESTSUPER_Change();
	void TESTSUPER_Update();
	void TESTSUPER_UpdateSprite();
	void TESTSUPER_TransitionToAction(int a);
	void TESTSUPER_TimeIndFrameInc();
	void TESTSUPER_TimeDepFrameInc();
	int TESTSUPER_GetActionLength();
	Tileset * TESTSUPER_GetTileset();

	void SUPERBIRD_Start();
	void SUPERBIRD_End();
	void SUPERBIRD_Change();
	void SUPERBIRD_Update();
	void SUPERBIRD_UpdateSprite();
	void SUPERBIRD_TransitionToAction(int a);
	void SUPERBIRD_TimeIndFrameInc();
	void SUPERBIRD_TimeDepFrameInc();
	int SUPERBIRD_GetActionLength();
	Tileset * SUPERBIRD_GetTileset();

	void TELEPORTACROSSTERRAIN_Start();
	void TELEPORTACROSSTERRAIN_End();
	void TELEPORTACROSSTERRAIN_Change();
	void TELEPORTACROSSTERRAIN_Update();
	void TELEPORTACROSSTERRAIN_UpdateSprite();
	void TELEPORTACROSSTERRAIN_TransitionToAction(int a);
	void TELEPORTACROSSTERRAIN_TimeIndFrameInc();
	void TELEPORTACROSSTERRAIN_TimeDepFrameInc();
	int TELEPORTACROSSTERRAIN_GetActionLength();
	Tileset * TELEPORTACROSSTERRAIN_GetTileset();

	void UAIR_Start();
	void UAIR_End();
	void UAIR_Change();
	void UAIR_Update();
	void UAIR_UpdateSprite();
	void UAIR_TransitionToAction(int a);
	void UAIR_TimeIndFrameInc();
	void UAIR_TimeDepFrameInc();
	int UAIR_GetActionLength();
	Tileset * UAIR_GetTileset();

	void UPTILT1_Start();
	void UPTILT1_End();
	void UPTILT1_Change();
	void UPTILT1_Update();
	void UPTILT1_UpdateSprite();
	void UPTILT1_TransitionToAction(int a);
	void UPTILT1_TimeIndFrameInc();
	void UPTILT1_TimeDepFrameInc();
	int UPTILT1_GetActionLength();
	Tileset * UPTILT1_GetTileset();

	void UPTILT2_Start();
	void UPTILT2_End();
	void UPTILT2_Change();
	void UPTILT2_Update();
	void UPTILT2_UpdateSprite();
	void UPTILT2_TransitionToAction(int a);
	void UPTILT2_TimeIndFrameInc();
	void UPTILT2_TimeDepFrameInc();
	int UPTILT2_GetActionLength();
	Tileset * UPTILT2_GetTileset();

	void UPTILT3_Start();
	void UPTILT3_End();
	void UPTILT3_Change();
	void UPTILT3_Update();
	void UPTILT3_UpdateSprite();
	void UPTILT3_TransitionToAction(int a);
	void UPTILT3_TimeIndFrameInc();
	void UPTILT3_TimeDepFrameInc();
	int UPTILT3_GetActionLength();
	Tileset * UPTILT3_GetTileset();

	void WAITFORSHIP_Start();
	void WAITFORSHIP_End();
	void WAITFORSHIP_Change();
	void WAITFORSHIP_Update();
	void WAITFORSHIP_UpdateSprite();
	void WAITFORSHIP_TransitionToAction(int a);
	void WAITFORSHIP_TimeIndFrameInc();
	void WAITFORSHIP_TimeDepFrameInc();
	int WAITFORSHIP_GetActionLength();
	Tileset * WAITFORSHIP_GetTileset();

	void WALLATTACK_Start();
	void WALLATTACK_End();
	void WALLATTACK_Change();
	void WALLATTACK_Update();
	void WALLATTACK_UpdateSprite();
	void WALLATTACK_TransitionToAction(int a);
	void WALLATTACK_TimeIndFrameInc();
	void WALLATTACK_TimeDepFrameInc();
	int WALLATTACK_GetActionLength();
	Tileset * WALLATTACK_GetTileset();

	void WALLCLING_Start();
	void WALLCLING_End();
	void WALLCLING_Change();
	void WALLCLING_Update();
	void WALLCLING_UpdateSprite();
	void WALLCLING_TransitionToAction(int a);
	void WALLCLING_TimeIndFrameInc();
	void WALLCLING_TimeDepFrameInc();
	int WALLCLING_GetActionLength();
	Tileset * WALLCLING_GetTileset();

	void WALLJUMP_Start();
	void WALLJUMP_End();
	void WALLJUMP_Change();
	void WALLJUMP_Update();
	void WALLJUMP_UpdateSprite();
	void WALLJUMP_TransitionToAction(int a);
	void WALLJUMP_TimeIndFrameInc();
	void WALLJUMP_TimeDepFrameInc();
	int WALLJUMP_GetActionLength();
	Tileset * WALLJUMP_GetTileset();

	void WALLTECH_Start();
	void WALLTECH_End();
	void WALLTECH_Change();
	void WALLTECH_Update();
	void WALLTECH_UpdateSprite();
	void WALLTECH_TransitionToAction(int a);
	void WALLTECH_TimeIndFrameInc();
	void WALLTECH_TimeDepFrameInc();
	int WALLTECH_GetActionLength();
	Tileset * WALLTECH_GetTileset();

	void WATERGLIDE_Start();
	void WATERGLIDE_End();
	void WATERGLIDE_Change();
	void WATERGLIDE_Update();
	void WATERGLIDE_UpdateSprite();
	void WATERGLIDE_TransitionToAction(int a);
	void WATERGLIDE_TimeIndFrameInc();
	void WATERGLIDE_TimeDepFrameInc();
	int WATERGLIDE_GetActionLength();
	Tileset * WATERGLIDE_GetTileset();

	void WATERGLIDECHARGE_Start();
	void WATERGLIDECHARGE_End();
	void WATERGLIDECHARGE_Change();
	void WATERGLIDECHARGE_Update();
	void WATERGLIDECHARGE_UpdateSprite();
	void WATERGLIDECHARGE_TransitionToAction(int a);
	void WATERGLIDECHARGE_TimeIndFrameInc();
	void WATERGLIDECHARGE_TimeDepFrameInc();
	int WATERGLIDECHARGE_GetActionLength();
	Tileset * WATERGLIDECHARGE_GetTileset();

	void WATERGLIDE_HITSTUN_Start();
	void WATERGLIDE_HITSTUN_End();
	void WATERGLIDE_HITSTUN_Change();
	void WATERGLIDE_HITSTUN_Update();
	void WATERGLIDE_HITSTUN_UpdateSprite();
	void WATERGLIDE_HITSTUN_TransitionToAction(int a);
	void WATERGLIDE_HITSTUN_TimeIndFrameInc();
	void WATERGLIDE_HITSTUN_TimeDepFrameInc();
	int WATERGLIDE_HITSTUN_GetActionLength();
	Tileset * WATERGLIDE_HITSTUN_GetTileset();

	void WIREHOLD_Start();
	void WIREHOLD_End();
	void WIREHOLD_Change();
	void WIREHOLD_Update();
	void WIREHOLD_UpdateSprite();
	void WIREHOLD_TransitionToAction(int a);
	void WIREHOLD_TimeIndFrameInc();
	void WIREHOLD_TimeDepFrameInc();
	int WIREHOLD_GetActionLength();
	Tileset * WIREHOLD_GetTileset();

};


#endif