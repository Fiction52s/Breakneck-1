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

struct PState;

struct EditSession;

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

struct MotionGhostEffect
{
	MotionGhostEffect( int maxGhosts );
	~MotionGhostEffect();
	void SetParams();
	void SetShader(sf::Shader *pShad);
	void SetRootPos(sf::Vector2f &pos );
	void SetDistInBetween(float dist);
	void SetSpread( int numGhosts, sf::Vector2f &dir, float angle );
	void SetVibrateParams(CubicBezier &vBez, float maxVibrate, int startGhost = 0);
	void SetTileset(Tileset *ts);
	void SetTile(int tileIndex);
	void SetColor(sf::Color &c);
	
	void SetColorGradient(sf::Color &c0,
		sf::Color &c1, CubicBezier &bez);
	void Draw(sf::RenderTarget *target);
	void SetFacing(bool facingRight, bool reversed);
	void SetScaleParams(CubicBezier &sBez, float p_maxScaleUp, float p_maxScaleDown, int startScaleGhost = 0 );
	void ApplyUpdates();

	VertexBuf *motionGhostBuffer;
	sf::Color rootColor;
	sf::Color tailColor;
	CubicBezier colorBez; //starts at root
	bool facingRight;
	float angle;
	bool reversed;

	int tileIndex;
	Tileset *ts;
	sf::Vector2f dir;
	sf::Vector2f rootPos;
	int numGhosts;
	float distInBetween;
	sf::Shader *shader;

	CubicBezier vibrateBez;
	int startVibrateGhost;
	CubicBezier scaleBez;
	int startScaleGhost;
	float maxScaleUp;
	float maxScaleDown;
	float maxVibrate;
	//settileset and settile
	//void SetParams( )
	//setrotation
	//setrootposition, direction, spread dist,offset
	//set scale if i >= 10
	//setnumactivemembers
	//setghostcolor
	//updateverticesg
	//draw

};

//eventually make this an objectpool but don't need to for now

struct AbsorbParticles;
struct GravityModifier;
struct Booster;
struct BounceBooster;
struct Spring;
struct Teleporter;
struct SwingLauncher;
struct EnemyParams;
struct SoundNode;

struct GroundTrigger;

struct KinMask;
struct KinRing;
struct RisingParticleUpdater;
struct EffectPool;
struct KeyExplodeUpdater;
struct ShapeEmitter;

struct BasicEffect;

struct Wire;

using json = nlohmann::json;

struct Actor : QuadTreeCollider,
	RayCastHandler
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
		UPGRADE_DASH_BOOSTER_1,
		UPGRADE_Count,
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
		STANDN,
		UAIR,
		WALLCLING,
		WALLJUMP,
		GROUNDTECHSIDEWAYS,
		STEEPSLIDE,
		GRAVREVERSE,
		GRINDBALL,
		RAILGRIND,
		RAILSLIDE,
		RAILDASH,
		GRINDLUNGE,
		GRINDSLASH,
		GRINDATTACK,
		AIRDASH,
		STEEPCLIMB,
		AIRHITSTUN,
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
		SPRINGSTUN,
		SPRINGSTUNGLIDE,
		SPRINGSTUNBOUNCE,
		SPRINGSTUNAIRBOUNCE,
		SPRINGSTUNTELEPORT,
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
		Count
	};

	enum SoundType
	{
		S_RUN_STEP1,
		S_RUN_STEP2,
		S_SPRINT_STEP1,
		S_SPRINT_STEP2,
		S_DASH_START,
		S_HIT,
		S_CLIMB_STEP1,
		S_CLIMB_STEP2,
		S_HITCEILING,
		S_HURT,
		S_HIT_AND_KILL,
		S_HIT_AND_KILL_KEY,
		S_FAIR1,
		S_FAIR2,
		S_FAIR3,
		S_DAIR,
		S_DAIR_B,
		S_UAIR,
		S_LAND,
		S_STANDATTACK,
		S_CLIMBATTACK,
		S_WALLJUMP,
		S_WALLATTACK,
		S_GRAVREVERSE,
		S_BOUNCEJUMP,
		S_TIMESLOW,
		S_ENTER,
		S_EXIT,
		S_DIAGUPATTACK,
		S_DIAGDOWNATTACK,
		S_DOUBLE,
		S_DOUBLEBACK,
		S_JUMP,
		S_SLIDE,
		S_STEEPSLIDE,
		S_STEEPSLIDEATTACK,
		S_WALLSLIDE,
		S_GOALKILLSLASH1,
		S_GOALKILLSLASH2,
		S_GOALKILLSLASH3,
		S_GOALKILLSLASH4,
		S_Count
	};

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

	const static int MAX_BUBBLES = 5;

	//definitely do change per frame
	double offsetX;
	int framesSinceClimbBoost;
	bool holdDouble;
	int frame;
	int action;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
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
	int speedLevel; //0,1,2
	double currentSpeedBar;
	double storedGroundSpeed;
	int currBBoostCounter;
	bool steepJump;
	// 0 is not started, 1 is right, 2 is
	//left
	bool airDashStall;
	sf::Vector2<double> startAirDashVel;
	double extraAirDashY;
	int oldAction;
	int drainCounter;
	sf::Vector2<double> oldVelocity;
	bool reversed;
	double storedReverseSpeed;

	double grindActionCurrent;
	int framesGrinding;
	int framesNotGrinding;
	int framesSinceGrindAttempt;
	int maxFramesSinceGrindAttempt;
	Edge *grindEdge;
	double grindQuantity;
	double grindSpeed;

	int slowMultiple;
	int slowCounter;
	bool inBubble;
	bool oldInBubble;

	sf::Vector2<double> bubblePos[MAX_BUBBLES];
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
	bool stunBufferedJump;
	bool stunBufferedDash;
	int stunBufferedAttack;

	int hitlagFrames;
	int hitstunFrames;
	int setHitstunFrames;
	int invincibleFrames;
	HitboxInfo *receivedHit;

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
	sf::Vector2<double> storedBounceVel;
	sf::Vector2<double> bounceNorm;
	sf::Vector2<double> oldBounceNorm;
	Edge *bounceEdge;
	double storedBounceGroundSpeed;
	double bounceQuant;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;
	bool boostBounce;
	bool bounceGrounded;
	bool justToggledBounce;

	int attackLevel;
	int framesSinceAttack;
	int comboCounterResetFrames;


	sf::Vector2f fairSwordOffset[3];
	sf::Vector2f dairSwordOffset[3];
	sf::Vector2f diagUpSwordOffset[3];
	sf::Vector2f diagDownSwordOffset[3];
	sf::Vector2f standSwordOffset[3];
	sf::Vector2f dashAttackSwordOffset[3];
	sf::Vector2f slideAttackOffset[3];
	sf::Vector2f climbAttackOffset[3];

	//values that dont need to be stored
	//at least in this first pass
	sf::SoundBuffer *soundBuffers[SoundType::S_Count];
	Tileset *ts_exitAura;
	Tileset *ts_dirtyAura;
	sf::Shader despFaceShader;
	sf::Shader playerDespShader;
	sf::Shader playerSuperShader;
	sf::Shader auraTestShader;
	bool showExitAura;
	BitField bStartHasUpgradeField;
	bool canStandUp;
	V2d currNormal;
	Tileset *ts_kinFace;
	sf::Sprite dirtyAuraSprite;//for when you're absorbing a power
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
	EffectPool *smallLightningPool[7];
	EffectPool *risingAuraPool;
	MotionGhostEffect *motionGhostsEffects[3];
	EffectPool *keyExplodePool;
	KeyExplodeUpdater *keyExplodeUpdater;
	Tileset *ts_keyExplode;
	Tileset *ts_key;
	MovingGeoGroup *keyExplodeRingGroup;
	EffectPool *dustParticles;
	RisingParticleUpdater *rpu;
	GroundTrigger *storedTrigger;
	AirTrigger *currAirTrigger;
	bool showDirtyAura;
	AirTriggerBehavior airTrigBehavior;
	EffectPool *fairLightningPool[4];
	EffectPool *uairLightningPool[4];
	EffectPool *dairLightningPool[4];
	EffectPool *gateBlackFXPool;
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
	Skin *skin;
	Skin *swordSkin;
	std::string actionFolder;
	bool toggleBounceInput;
	bool toggleTimeSlowInput;
	bool toggleGrindInput;
	Tileset *ts_fx_rightWire;
	Tileset *ts_fx_leftWire;
	Tileset *ts_fx_doubleWire;
	HitboxInfo *wireChargeInfo;
	bool hitGoal;
	bool hitNexus;
	sf::Sprite dodecaSprite;
	Edge *gateTouched;
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
	KinRing *kinRing;
	
	
	bool standNDashBoost;
	int standNDashBoostCurr;
	bool hasFairAirDashBoost;
	int framesStanding;
	int framesSinceRightWireBoost;
	int framesSinceLeftWireBoost;
	int framesSinceDoubleWireBoost;
	int enemiesKilledThisFrame;
	int enemiesKilledLastFrame;
	
	
	

	//unsaved vars
	int possibleEdgeCount;
	GameSession *owner;
	EditSession *editOwner;
	double steepClimbSpeedThresh;
	Contact minContact;
	sf::Shader sh;
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
	sf::Sprite fairSword;
	Tileset *ts_fairSword[3];
	Tileset *ts_fairSwordLightning[3];
	RelEffectInstance *currLockedFairFX;
	RelEffectInstance *currLockedDairFX;
	RelEffectInstance *currLockedUairFX;
	RelEffectInstance *gateBlackFX;
	bool showSword;
	sf::Sprite grindLungeSword;
	Tileset *ts_grindLungeSword[3];
	sf::Sprite dairSword;
	Tileset *ts_dairSword[3];
	Tileset *ts_dairSwordLightning[3];
	sf::Sprite uairSword;
	Tileset *ts_uairSword[3];
	Tileset *ts_uairSwordLightning[3];
	sf::Sprite standingNSword;
	Tileset *ts_standingNSword[3];
	sf::Sprite dashAttackSword;
	Tileset *ts_dashAttackSword[3];
	Tileset *ts_dashAttackSword2[3];
	Tileset *ts_dashAttackSword3[3];
	sf::Sprite wallAttackSword;
	Tileset *ts_wallAttackSword[3];
	sf::Sprite steepSlideAttackSword;
	Tileset *ts_steepSlideAttackSword[3];
	sf::Sprite steepClimbAttackSword;
	Tileset *ts_steepClimbAttackSword[3];
	sf::Sprite diagUpAttackSword;
	Tileset *ts_diagUpSword[3];
	sf::Sprite diagDownAttackSword;
	Tileset *ts_diagDownSword[3];
	
	sf::Sprite bounceFlameSprite;
	double bounceFlameAccel0;
	double bounceFlameAccel1;
	double bounceFlameAccel2;
	Tileset *ts_bounceBoost;
	
	bool scorpSet;
	Spring *currSpring;
	Teleporter *currTeleporter;
	Teleporter *oldTeleporter;
	Booster *currBooster;
	Booster *oldBooster;
	SwingLauncher *currSwingLauncher;
	SwingLauncher *oldSwingLauncher;
	BounceBooster *currBounceBooster;
	BounceBooster *oldBounceBooster;
	Session *sess;
	GravityModifier *currModifier;
	GravityModifier *oldModifier;
	int gravResetFrames;
	sf::Vector2<double> springVel;
	sf::Vector2<double> springExtra;
	int springStunFrames;
	V2d teleportSpringDest;
	V2d teleportSpringVel;
	double glideTurnFactor;
	double glideTurnAccel;
	double maxGlideTurnFactor;
	//these are for your max vel
	double scorpAdditionalCap;
	double scorpAdditionalCapMax;
	double scorpAdditionalAccel;
	double maxVelocity;
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
	sf::Sprite scorpSprite;
	Tileset * ts_fx_airdashUp;
	Tileset * ts_fx_airdashDiagonal;
	Tileset * ts_fx_airdashSmall;
	Tileset * ts_fx_airdashHover;
	Tileset *ts_fx_run;
	Tileset *ts_fx_runStart[3];
	Tileset *ts_fx_sprint;
	Tileset * ts_fx_double;
	Tileset * ts_fx_wallJump[3];
	Tileset * ts_fx_gravReverse;
	Tileset * ts_fx_bigRunRepeat;
	Tileset * ts_fx_chargeBlue0;
	Tileset * ts_fx_chargeBlue1;
	Tileset * ts_fx_chargeBlue2;
	Tileset * ts_fx_chargePurple;
	Tileset *ts_fx_hurtSpack;
	Tileset *ts_fx_dashStart;
	Tileset *ts_fx_dashRepeat;
	Tileset *ts_fx_land[3];
	Tileset *ts_fx_jump[3];
	Tileset *ts_fx_death_1a;
	Tileset *ts_fx_death_1b;
	Tileset *ts_fx_death_1c;
	Tileset *ts_fx_death_1d;
	Tileset *ts_fx_death_1e;
	Tileset *ts_fx_death_1f;
	Tileset *ts_fx_gateEnter;
	int speedParticleCounter;
	int speedParticleRate;
	
	bool hitEnemyDuringPhyiscs;
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
	std::unordered_map<int, HitboxInfo> hitboxInfos;
	HitboxInfo *currVSHitboxInfo;
	CollisionBody *fairHitboxes[3];
	CollisionBody *uairHitboxes[3];
	CollisionBody *dairHitboxes[3];
	CollisionBody *standHitboxes[3];
	CollisionBody *dashHitboxes[3];
	CollisionBody *wallHitboxes[3];
	CollisionBody *steepClimbHitboxes[3];
	CollisionBody *steepSlideHitboxes[3];
	CollisionBody *diagUpHitboxes[3];
	CollisionBody *diagDownHitboxes[3];
	CollisionBody *shockwaveHitboxes;
	CollisionBody *grindHitboxes[3];
	double steepThresh;
	int wallJumpMovementLimit;
	double dashHeight;
	double normalHeight;
	double doubleJumpHeight;
	double sprintHeight;
	double airSlow;
	double slopeLaunchMinSpeed;
	double maxRunInit;
	double maxGroundSpeed;
	double runAccelInit;
	double sprintAccel;
	double maxNormalRun;
	double runAccel;
	double maxFallSpeedSlow;
	double maxFallSpeedFast;
	double gravity;
	double extraGravityModifier;
	double jumpStrength;
	double airAccel;
	double maxAirXSpeed;
	double maxAirXControl;
	double dashSpeed;
	double dashSpeed0;
	double dashSpeed1;
	double dashSpeed2;
	double slideGravFactor;
	Mode kinMode;
	int maxBBoostCount;
	double doubleJumpStrength;
	double backDoubleJumpStrength;
	int timeSlowStrength;
	double lessSlowDownThresh;
	sf::Vector2<double> wallJumpStrength;
	double clingSpeed;
	bool col;
	sf::Vector2<double> tempVel;
	std::string queryMode;
	int maxDespFrames;
	int despCounter;
	int maxSuperFrames;
	int superFrame;
	bool checkValid;
	Edge *rcEdge;
	double rcQuantity;
	std::string rayCastMode;
	bool leftGround;
	ControllerState prevInput;
	ControllerState currInput;
	bool canRailGrind;
	bool canRailSlide;
	double minRailGrindSpeed[3];
	sf::RectangleShape railTest;
	TerrainRail * prevRail;
	bool railGrind;
	int regrindOffCount;
	int regrindOffMax;
	sf::Vector2<double> lungeNormal;
	double grindLungeSpeed0;
	double grindLungeSpeed1;
	double grindLungeSpeed2;
	double slopeTooSteepLaunchLimitX;
	
	int baseSlowMultiple;
	sf::Vector2<double> wallNormal;
	Edge *currWall;
	int gravityGrassCount;
	int bounceGrassCount;
	int boostGrassCount;
	bool grassBoosted;
	double boostGrassAccel;
	int jumpGrassCount;
	bool touchedJumpGrass;
	double jumpGrassExtra;
	bool extraDoubleJump;
	int specialTerrainCount[SPECIAL_TERRAIN_Count];
	
	
	Wire *leftWire;
	Wire *rightWire;
	
	bool wallClimbGravityFactor;
	bool wallClimbGravityOn;
	int maxBubbles;
	int bubbleLifeSpan;
	int bubbleRadius;
	int bubbleRadius0;
	int bubbleRadius1;
	int bubbleRadius2;
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	bool dead;
	sf::Shader swordShaders[3];
	sf::Color flashColor;
	int steepClimbBoostStart;
	
	int drainCounterMax;
	int drainAmount;
	int climbBoostLimit;
	


	Actor( GameSession *owner, 
		EditSession *editOwner, int actorIndex );
	~Actor();
	void Init();
	V2d GetGroundAnchor();

	void LoadHitboxes();
	void SetupHitboxInfo(
		json &j, const std::string &name,
		HitboxInfo &hi);

	void SetFBubbleFrame(int i, float val);
	void SetFBubblePos(int i, sf::Vector2f &pos);
	void SetFBubbleRadiusSize(int i, float rad);

	void PopulateState(PState *ps);
	void PopulateFromState(PState *ps);
	bool TryClimbBoost(V2d &gNorm);
	CollisionBody * GetBubbleHitbox(int index);

	void CreateKeyExplosion();
	void CreateGateExplosion();
	void CollectFly(HealthFly *hf);
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
	void SetupDrain();
	void SetupTimeBubbles();
	void SetGameMode();
	SoundNode * ActivateSound(SoundType st, bool loop = false);
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
	int GetTotalGameFrames();
	Collider &GetCollider();
	sf::SoundBuffer * GetSound(const std::string &name);
	std::map<int, std::list<CollisionBox>> & GetHitboxList(
		const std::string & str);
	bool IsGroundAttack(int a);
	GameController &GetController(int index);
	void HandleGroundTrigger(GroundTrigger *trigger);
	void CheckForAirTrigger();
	void HandleAirTrigger();
	void UpdateCanStandUp();
	void UpdateBounceFlameOn();
	void HitstunBufferedChangeAction();
	void ProcessBooster();
	void UpdateWireStates();
	void ProcessBoostGrass();
	void LimitMaxSpeeds();
	void UpdateBubbles();
	void UpdateRegrindOffCounter();
	void UpdateKnockbackDirectionAndHitboxType();
	void UpdateSmallLightning();
	void UpdateRisingAura();
	void UpdateLockedFX();
	void ProcessSpecialTerrain();
	void SetDirtyAura(bool on);
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
	bool TryGrind();
	bool TryDash();
	bool TryJumpSquat();
	bool TrySlideBrakeOrStand();
	bool TrySprintOrRun(V2d &gNorm);
	void SetSprintStartFrame();
	bool TryAirDash();
	bool TryGlide();
	bool ExitGrind(bool jump);
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
	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void ApplyHit( HitboxInfo *info );
	bool ResolvePhysics( sf::Vector2<double> vel );
	void UpdatePhysics();
	void PhysicsResponse();
	bool TryGroundAttack();
	bool SteepSlideAttack();
	bool SteepClimbAttack();
	void ConfirmEnemyKill( Enemy *e );
	bool IHitPlayer( int otherPlayerIndex );
	std::pair<bool, bool> PlayerHitMe(int otherPlayerIndex);
	void ShipPickupPoint( double eq,
		bool facingRight );
	void GrabShipWire();
	bool physicsOver;
	void UpdatePostPhysics();
	bool CheckWall( bool right );
	bool TryWallJump();
	bool CheckStandUp();
	void CheckBounceFlame();
	bool BasicAirAction();
	bool GlideAction();
	bool BasicGroundAction( V2d &gNorm);
	bool BasicSteepAction(V2d &gNorm);
	bool BasicAirAttackAction();
	sf::Vector2<double> UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );
	void MiniDraw(sf::RenderTarget *target);
	void DeathDraw(sf::RenderTarget *target);
	void DebugDraw( sf::RenderTarget *target );
	void DrawMapWires(sf::RenderTarget *target);
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateHitboxes();
	void AirMovement();
	double GroundedAngle();
	double GroundedAngleAttack( sf::Vector2<double> &trueNormal );
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
	Edge *autoRunStopEdge;
	double autoRunStopQuant;
	double maxAutoRunSpeed;
	void AttackMovement();
	void DodecaLateDraw(sf::RenderTarget *target);
	void SetActionGrind();
	bool CanUnlockGate( Gate *g );
	void HandleWaitingScoreDisplay();
	void EndLevelWithoutGoal();
	void CheckHoldJump();
	void Respawn();
	void BounceFlameOff();
	void BounceFlameOn();
	void SetupAction(int a);
	void SetBoostVelocity();
	void SetBounceBoostVelocity();
	double GetNumSteps();
	double CalcLandingSpeed( sf::Vector2<double> &testVel,
		sf::Vector2<double> &alongVel, 
		sf::Vector2<double> &gNorm, bool rail = false );
	bool CanRailSlide();
	bool CanRailGrind();
	bool IsRailSlideFacingRight();
	bool IsIntangible();
	void SetKinMode(Mode m);
	void KinModeUpdate();
	void ReverseVerticalInputsWhenOnCeiling();
	void ProcessReceivedHit();
	void UpdateDrain();
	void ProcessGravityGrass();
	void UpdateScorpCap();
	void ProcessHitGoal();
	void UpdateWirePhysics();
	void UpdateGrindPhysics(double movement);
	void HandleBounceGrass();
	bool UpdateGrindRailPhysics(double movement);
	bool TryUnlockOnTransfer( Edge *e );
	void LeaveGroundTransfer(bool right, V2d leaveExtra = V2d());
	bool UpdateAutoRunPhysics( double q, double m );
	void UpdateSpeedBar();
	bool CareAboutSpeedAction();
	void UpdateMotionGhosts();
	void UpdateSpeedParticles();
	void UpdateAttackLightning();
	void UpdatePlayerShader();
	void TryEndLevel();
	void UpdateDashBooster();
	void SlowDependentFrameIncrement();
	void UpdateBounceFlameCounters();
	void SetAerialScorpSprite();
	int GetJumpFrame();
	void QueryTouchGrass();
	double GetBounceFlameAccel();
	void GroundExtraAccel();
	double GetDashSpeed();
	double GetAirDashSpeed();
	void AddToFlyCounter(int count);
	void SetCurrHitboxes(CollisionBody *cBody,
		int p_frame);
	void AddActiveComboObj(ComboObject *c);
	void RemoveActiveComboObj(ComboObject *c);
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
	sf::Vector2<double> AddGravity(sf::Vector2<double> vel);
	double GetGravity();
	void ClearSpecialTerrainCounts();
	void HandleSpecialTerrain();
	void HandleSpecialTerrain(int stType);
	V2d GetTrueVel();
	void RestoreDoubleJump();
	void RestoreAirDash();
	int GetBubbleRadius();
	bool IsBeingSlowed();
	void SetAllUpgrades(BitField &b);
	bool HasUpgrade(int index);
	void SetUpgrade(int upgrade, bool on);
	void SetStartUpgrade(int upgrade, bool on);
	bool SpringLaunch();
	bool TeleporterLaunch();
	bool SwingLaunch();
	bool CheckSwing();
	bool CheckNormalSwing();
	bool CheckNormalSwingHeld();
	bool CheckRightStickSwing();
	bool CheckRightStickSwingHeld();
	bool CheckSwingHeld();
	void ClearPauseBufferedActions();
	void UpdateInHitlag();
	bool IsAttackAction( int a );
	bool IsGroundAttackAction(int a);
	bool IsSpringAction(int a);
	bool IsOnRailAction(int a);
	bool IsInHistunAction( int a );

	V2d GetKnockbackDirFromVel();

	//kin action functions
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

	void GROUNDTECHSIDEWAYS_Start();
	void GROUNDTECHSIDEWAYS_End();
	void GROUNDTECHSIDEWAYS_Change();
	void GROUNDTECHSIDEWAYS_Update();
	void GROUNDTECHSIDEWAYS_UpdateSprite();
	void GROUNDTECHSIDEWAYS_TransitionToAction(int a);
	void GROUNDTECHSIDEWAYS_TimeIndFrameInc();
	void GROUNDTECHSIDEWAYS_TimeDepFrameInc();
	int GROUNDTECHSIDEWAYS_GetActionLength();
	Tileset * GROUNDTECHSIDEWAYS_GetTileset();

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

	void STANDN_Start();
	void STANDN_End();
	void STANDN_Change();
	void STANDN_Update();
	void STANDN_UpdateSprite();
	void STANDN_TransitionToAction(int a);
	void STANDN_TimeIndFrameInc();
	void STANDN_TimeDepFrameInc();
	int STANDN_GetActionLength();
	Tileset * STANDN_GetTileset();

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