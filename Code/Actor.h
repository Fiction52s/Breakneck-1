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
struct Rail;
struct ComboObject;
struct Actor;
struct ObjectPool;
struct RelEffectInstance;
struct VertexBuf;
struct Kin;

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
	//updatevertices
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
struct EffectPool;
struct EnemyParams;
struct SoundNode;

struct GroundTrigger;

struct KinMask;
struct KinRing;
struct RisingParticleUpdater;
struct EffectPool;
struct ShapeEmitter;

struct BasicEffect;

struct Wire;

struct Actor : QuadTreeCollider,
	RayCastHandler
{
	enum PowerType
	{
		POWER_AIRDASH,
		POWER_GRAV,
		POWER_BOUNCE,
		POWER_GRIND,
		POWER_TIME,
		POWER_RWIRE,
		POWER_LWIRE
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
		SPRINT,
		STAND,
		DASHATTACK,
		WALLATTACK,
		STEEPCLIMBATTACK,
		STEEPSLIDEATTACK,
		STANDN,
		UAIR,
		WALLCLING,
		WALLJUMP,
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

	void SetupDrain();
	void SetupTimeBubbles();
	void SetGameMode();

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
	sf::SoundBuffer *soundBuffers[SoundType::S_Count];
	//ShapeEmitter *glideEmitter;
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
	void UpdatePowers();

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


	Tileset * GetTileset(const std::string & s, int tileWidth, int tileHeight, int altColorIndex = 0);
	Tileset * GetTileset(const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numColorChanges,
		sf::Color *startColorBuf, sf::Color *endColorBuf);
	Tileset * GetTileset(const std::string & s, int tileWidth, int tileHeight, Skin *skin);

	GameController &GetController(int index);

	//EffectPool *testPool;
	KinMask *kinMask;

	EffectPool *smallLightningPool[7];
	EffectPool *risingAuraPool;
	MotionGhostEffect *motionGhostsEffects[3];

	KinRing *kinRing;

	EffectPool *dustParticles;
	RisingParticleUpdater *rpu;
	void HandleGroundTrigger(GroundTrigger *trigger);
	GroundTrigger *storedTrigger;

	AirTrigger *currAirTrigger;
	void HandleAirTrigger();

	sf::Sprite exitAuraSprite;
	Tileset *ts_exitAura;
	bool showExitAura;

	//for when you're absorbing a power
	sf::Sprite dirtyAuraSprite;
	Tileset *ts_dirtyAura;
	//void UpdateDirtyAura();
	void SetDirtyAura(bool on);
	bool showDirtyAura;

	enum AirTriggerBehavior
	{
		AT_NONE,
		AT_AUTORUNRIGHT,
	};
	AirTriggerBehavior airTrigBehavior;
	
	

	enum Expr
	{
		Expr_NEUTRAL,
		Expr_HURT,
		Expr_SPEED1,
		Expr_SPEED2,
		Expr_DESP,
		Expr_DEATH,
		Expr_NONE
	};

	enum Team
	{
		T_BLUE,
		T_RED,
		T_GREEN,
		T_PURPLE
	};
	
	
	void TurnFace();
	void StandInPlace();
	void WaitInPlace();
	void Wait();
	bool IsGoalKillAction(Action a);
	bool IsIntroAction(Action a);
	bool IsExitAction(Action a);
	bool IsSequenceAction(Action a);

	void StartSeqKinThrown( V2d &pos, V2d &vel );
	void SeqKneel();
	void SeqMeditateMaskOn();
	void SeqMaskOffMeditate();
	void SeqGetAirdash();

	void CreateAttackLightning();
	EffectPool *fairLightningPool[4];
	EffectPool *uairLightningPool[4];
	EffectPool *dairLightningPool[4];
	EffectPool *gateBlackFXPool;


	bool CanShootWire();
	bool CanCreateTimeBubble();

	SoundNode *repeatingSound;
	//int hitCeilingCounter;
	//int hitCeilingLockoutFrames;
	
	void UnlockGate(Gate *g);

	Team team;

	Action spriteAction;
	int currTileIndex;
	bool flipTileX;
	bool flipTileY;

	

	sf::Vector2<double> spriteCenter;
	bool dairBoostedDouble;
	bool aerialHitCancelDouble;
	Action cancelAttack;
	double dairBoostVel;
	bool standNDashBoost;
	double standNDashBoostQuant;
	int standNDashBoostCooldown;
	int standNDashBoostCurr;

	bool hasFairAirDashBoost;
	double fairAirDashBoostQuant;

	//these are for the wire boost particles 
	sf::Vector2<double> leftWireBoostDir;
	sf::Vector2<double> rightWireBoostDir;
	sf::Vector2<double> doubleWireBoostDir;

	void WireMovement();

	Action GetDoubleJump();
	bool CanDoubleJump();
	void ExecuteDoubleJump();
	void ExecuteWallJump();
	bool IsSingleWirePulling();
	bool IsDoubleWirePulling();
	bool TryDoubleJump();
	bool TryGrind();
	bool TrySlide();
	bool TrySprint(V2d &gNorm);
	
	bool TryAirDash();
	bool TryGlide();

	int framesSinceRightWireBoost;
	int singleWireBoostTiming;
	int framesSinceLeftWireBoost;
	int framesSinceDoubleWireBoost;
	int doubleWireBoostTiming;

	bool leftWireBoost;
	bool rightWireBoost;
	bool doubleWireBoost;

	bool updateAura;

	bool ExitGrind(bool jump);
	
	void SetSpriteTexture( Action a );
	void SetSpriteTile( int tileIndex, bool noFlipX = true, bool noFlipY = true );
	void SetSpriteTile( sf::Sprite *spr, 
		Tileset *t, int tileIndex, bool noFlipX = true, bool noFlipY = true );
	void SetActionExpr( Action a );
	void SetExpr( Expr ex );
	void SetAction( Action a );
	void StartAction(Action a);

	void SetupTilesets(Skin *kSkin, Skin*swordSkin);

	void RailGrindMovement();

	bool AirAttack();
	Expr expr;
	sf::Vector2<double> movingPlatExtra;
	bool testr;
	
	bool toggleBounceInput;
	bool toggleTimeSlowInput;
	bool toggleGrindInput;

	void EnterNexus( int nexusIndex, sf::Vector2<double> &pos );

	void SetFakeCurrInput( 
		ControllerState &state );

	Tileset *ts_kinFace;
	sf::Shader despFaceShader;
	sf::Shader playerDespShader;
	sf::Sprite kinFace;
	sf::Sprite kinFaceBG;
	sf::Sprite kinUnderOutline;
	sf::Sprite kinTealOutline;
	sf::Sprite kinBlueOutline;
	sf::Sprite kinPurpleOutline;
	sf::Color currentDespColor;
	float GetSpeedBarPart();



	Tileset *ts_fx_rightWire;
	Tileset *ts_fx_leftWire;
	Tileset *ts_fx_doubleWire;


	HitboxInfo *wireChargeInfo;
	//sf::Shader speedBarShader;
	bool hitGoal;
	bool hitNexus;
	int enemiesKilledThisFrame;
	int enemiesKilledLastFrame;

	Actor( GameSession *owner, 
		EditSession *editOwner, int actorIndex );
	~Actor();
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
	bool CaptureMonitor( Monitor *m );
	void UpdatePhysics();
	void PhysicsResponse();
	bool GroundAttack();
	bool SteepSlideAttack();
	bool SteepClimbAttack();
	void ConfirmEnemyKill( Enemy *e );
	bool IHitPlayer( int otherPlayerIndex );
	std::pair<bool,bool> PlayerHitMe( int otherPlayerIndex );

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

	bool justToggledBounce;
	sf::Vector2<double> UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void DrawMapWires(sf::RenderTarget *target);
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateHitboxes();
	void AirMovement();
	double GroundedAngle();
	double GroundedAngleAttack( sf::Vector2<double> &trueNormal );
	sf::Vector2i GetWireOffset();
	void RunMovement();
	void AutoRunMovement();
	void SetAutoRun( bool fr, double maxAutoRun );
	void SetGroundedPos(Edge *g, double q);
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

	sf::Sprite dodecaSprite;

	void SetActivePowers(
		bool canAirDash,
		bool canGravReverse,
		bool canBounce,
		bool canGrind,
		bool canTimeSlow,
		bool canWire );
	// 0 is not started, 1 is right, 2 is
	//left
	bool airDashStall;
	//Light *testLight;
	//Gate *gateTouched;
	Edge *gateTouched;
	
	double level1SpeedThresh;
	double level2SpeedThresh;
	double speedChangeUp;
	double speedChangeDown;
	int speedLevel; //0,1,2
	double currentSpeedBar;
	sf::CircleShape speedCircle;

	//RotaryParticleEffect *re;
	//RotaryParticleEffect *re1;
	//ParticleTrail *pTrail;
	double rotaryAngle;
	//AirParticleEffect *ae;
	//double speedBarVel;


	
	//unsaved vars
	int possibleEdgeCount;
	GameSession *owner;
	EditSession *editOwner;

	double steepClimbSpeedThresh;
	Contact minContact;
	Contact storedContact;
	sf::Shader sh;
	sf::Shader timeSlowShader;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tileset[Count];

		//std::map<int, 
	void SetupAction(Action a);
	void LoadAllAuras();
	Tileset *ts_dodecaSmall;
	Tileset *ts_dodecaBig;

	sf::Sprite gsdodeca;
	sf::Sprite gstriblue;
	sf::Sprite gstricym;
	sf::Sprite gstrigreen;
	sf::Sprite gstrioran;
	sf::Sprite gstripurp;
	sf::Sprite gstrirgb;

	//int 
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
	//sf::Vector2i dairOffset[3];

	sf::Sprite uairSword;
	Tileset *ts_uairSword[3];
	Tileset *ts_uairSwordLightning[3];

	sf::Sprite standingNSword;
	Tileset *ts_standingNSword[3];

	sf::Sprite dashAttackSword;
	Tileset *ts_dashAttackSword[3];

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

	int runBounceFlameFrames;
	int runBounceFrame;
	Tileset *ts_runBounceFlame;

	Aura *testAura;
	Aura *testAura1;
	Aura *testAura2;
	Aura* testAura3;

	int airBounceFlameFrames;
	int airBounceFrame;
	Tileset *ts_airBounceFlame;

	int framesFlameOn;
	bool bounceFlameOn;
	sf::Sprite bounceFlameSprite;
	double bounceFlameAccel0;
	double bounceFlameAccel1;
	double bounceFlameAccel2;

	Tileset *ts_bounceBoost;
	bool scorpOn;
	bool scorpSet;

	Spring *currSpring;
	Teleporter *currTeleporter;
	Teleporter *oldTeleporter;
	Booster *currBooster;
	Booster *oldBooster;

	SwingLauncher *currSwingLauncher;
	SwingLauncher *oldSwingLauncher;



	void SetBoostVelocity();
	void SetBounceBoostVelocity();

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
	double GetNumSteps();

	double CalcLandingSpeed( sf::Vector2<double> &testVel,
		sf::Vector2<double> &alongVel, 
		sf::Vector2<double> &gNorm, bool rail = false );
	bool CanRailSlide();
	bool CanRailGrind();
	bool IsRailSlideFacingRight();

	void SetAerialScorpSprite();
	int GetJumpFrame();
	void QueryTouchGrass();

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


	sf::Vector2<double> followerPos;
	sf::Vector2<double> followerVel;
	double followerFac;

	int speedParticleCounter;
	int speedParticleRate;
	bool steepJump;

	//new variables in here that need to work with clone power later


	bool test;
	
	double offSlopeByWallThresh;
	//const static int MAX_MOTION_GHOSTS = 10;
	VertexBuf *motionGhostBuffer;
	VertexBuf *motionGhostBufferBlue;
	VertexBuf *motionGhostBufferPurple;
	

	int maxMotionGhosts;
	sf::Shader motionGhostShader;

	int numMotionGhosts;
	int motionGhostSpacing;
	int ghostSpacingCounter;
	float percentCloneChanged;
	float percentCloneRate;
	bool changingClone;

	void GroundExtraAccel();
	double GetBounceFlameAccel();

	double holdDashAccel;
	double wallThresh;

	double bounceBoostSpeed;
	double steepSlideGravFactor;
	double steepSlideFastGravFactor;
	double storedReverseSpeed;

	double steepClimbGravFactor;
	double steepClimbFastFactor;

	double GetDashSpeed();
	double GetAirDashSpeed();
	double airDashSpeed;

	double airDashSpeed0;
	double airDashSpeed1;
	double airDashSpeed2;

	CollisionBox b;
	CollisionBox hurtBody;
	CollisionBody *currHitboxes;
	int currHitboxFrame;
	void SetCurrHitboxes(CollisionBody *cBody,
		int p_frame);
	ComboObject *activeComboObjList;
	void AddActiveComboObj(ComboObject *c);
	void RemoveActiveComboObj(ComboObject *c);
	void DebugDrawComboObj(sf::RenderTarget *target);
	bool IsMovingRight();
	bool IsMovingLeft();
	//int numCurrHitboxes;

	CollisionBody *currHurtboxes;
	int currHurtboxFrame;
	//std::map<Action, CollisionBody*> hurtboxMap;
	bool IntersectMyHurtboxes(CollisionBody *cb,
		int cbFrame );
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

	sf::Vector2f fairSwordOffset[3];
	sf::Vector2f dairSwordOffset[3];
	sf::Vector2f diagUpSwordOffset[3];
	sf::Vector2f diagDownSwordOffset[3];
	sf::Vector2f standSwordOffset[3];
	sf::Vector2f slideAttackOffset[3];
	sf::Vector2f climbAttackOffset[3];

	HitboxInfo *currHitboxInfo;
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
	//CollisionBox grindHitbox;



	double steepThresh;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action
	std::list<sf::Vector2f> *auraPoints[3][Action::Count];
	bool usingAura;
	int CreateAura( std::list<sf::Vector2f> *&outPointList,
		Tileset *ts, int startTile = 0, int numTiles= 0, int layer = 0);
	//std::list<sf::Vector2f> *runPoints;
	//std::list<sf::Vector2f> *standPoints;

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
	//double fastFallGravity;

	double jumpStrength;
	double airAccel;
	double maxAirXSpeed;
	double maxAirXControl;
	double dashSpeed;
	
	double dashSpeed0;
	double dashSpeed1;
	double dashSpeed2;
	double slideGravFactor;
	double GetFullSprintAccel( bool downSlope,
		sf::Vector2<double> &gNorm );

	int maxBBoostCount;
	int currBBoostCounter;

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
	bool desperationMode;

	bool checkValid;

	Edge *rcEdge;
	double rcQuantity;
	std::string rayCastMode;

	bool leftGround;
	double grindActionCurrent;
	ControllerState prevInput;
	ControllerState currInput;
	sf::Vector2<double> oldVelocity;
	int framesInAir;
	int trueFramesInAir; 
	//need this^ because i manipulate
	//the framesInAir for the jump
	//but I need to make sure 
	sf::Vector2<double> startAirDashVel;
	double extraAirDashY;
	sf::Vector2<double> AddGravity( sf::Vector2<double> vel );
	double GetGravity();

	int framesGrinding;
	int framesNotGrinding;

	int framesSinceGrindAttempt;
	int maxFramesSinceGrindAttempt;
	//bool canGrabRail;
	bool canRailGrind;
	bool canRailSlide;
	double minRailGrindSpeed[3];
	double GetMinRailGrindSpeed();

	sf::RectangleShape railTest;

	Edge *ground;

	Rail *prevRail;

	bool hasAirDash;
	bool hasGravReverse;

	Edge *grindEdge;
	bool railGrind;
	int regrindOffCount;
	int regrindOffMax;
	
	sf::Vector2<double> lungeNormal;
	double grindLungeSpeed0;
	double grindLungeSpeed1;
	double grindLungeSpeed2;
	double grindQuantity;
	double grindSpeed;

	double slopeTooSteepLaunchLimitX;

	bool reversed;

	double edgeQuantity;
	
	double groundOffsetX;

	double offsetX;

	bool bounceGrounded;

	bool holdJump;

	int wallJumpFrameCounter;

	double groundSpeed;

	bool facingRight;
	
	bool hasDoubleJump;

	int slowMultiple;
	int slowCounter;

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

	void ClearSpecialTerrainCounts();
	int specialTerrainCount[SPECIAL_TERRAIN_Count];
	//int specialTerrainCount[2];
	void HandleSpecialTerrain();
	void HandleSpecialTerrain(int stType);

	Action action;
	int steepClimbBoostStart;
	bool TryClimbBoost( V2d &gNorm);
	int frame;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	V2d GetTrueVel();
	void RestoreDoubleJump();
	void RestoreAirDash();
	//CollisionBox *physBox;

	int hitlagFrames;
	int hitstunFrames;
	int setHitstunFrames;
	int invincibleFrames;
	HitboxInfo *receivedHit;

	bool inBubble;
	bool oldInBubble;

	sf::Vector2<double> storedBounceVel;
	sf::Vector2<double> bounceNorm;
	sf::Vector2<double> oldBounceNorm;
	double storedBounceGroundSpeed;
	
	Wire *leftWire;
	Wire *rightWire;
	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;

	Edge *bounceEdge;
	double bounceQuant;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;
	bool boostBounce;
	
	bool wallClimbGravityFactor;
	bool wallClimbGravityOn;
	
	//dont need to store these
	
	//const static int MAX_BUBBLES_RACEFIGHT = 1;
	int maxBubbles;
	int bubbleLifeSpan;
	int bubbleRadius;
	int bubbleRadius0;
	int bubbleRadius1;
	int bubbleRadius2;
	
	int GetBubbleRadius();
	bool IsBeingSlowed();
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	//--
	
	const static int MAX_BUBBLES = 5;
	CollisionBody * GetBubbleHitbox(int index);
	sf::Vector2<double> *bubblePos;//[maxBubbles];
	int *bubbleFramesToLive;//[maxBubbles];
	int *bubbleRadiusSize;//[maxBubbles];
	float *fBubbleRadiusSize;
	sf::Vector2f *fBubblePos;
	CollisionBody **bubbleHitboxes;

	float *fBubbleFrame;

	int currBubble;

	bool hasPowerAirDash;
	bool hasPowerGravReverse;
	bool hasPowerBounce;
	bool hasPowerGrindBall;
	bool hasPowerTimeSlow;
	bool hasPowerLeftWire;
	bool hasPowerRightWire;

	bool HasPower(int index);

	bool startHasPowerAirDash;
	bool startHasPowerGravReverse;
	bool startHasPowerBounce;
	bool startHasPowerGrindBall;
	bool startHasPowerTimeSlow;
	bool startHasPowerLeftWire;
	bool startHasPowerRightWire;

	int hasPowerClones;


	int lastWire;

	//unstored while working on
	bool dead;	

	//dummy key at 0 for GREY
	//int hasKey[Gate::GateType::Count];
	int numKeys;
	

	int framesSinceClimbBoost;
	int climbBoostLimit;

	bool longWallJump;
	Action oldAction;

	bool currAttackHit;
	bool bounceAttackHit;
	double storedGroundSpeed;
	
	bool holdDouble;
	int framesSinceDouble;
	
	sf::Shader swordShaders[3];

	//bool hitlagFlash;
	sf::Color flashColor;
	int flashFrames;

	//bool bufferedAttack;
	Action bufferedAttack;
	Action doubleJumpBufferedAttack;
	Action wallJumpBufferedAttack;

	bool SpringLaunch();
	bool TeleporterLaunch();
	bool SwingLaunch();
	bool CheckSwing();
	bool CheckNormalSwing();
	bool CheckNormalSwingHeld();
	bool CheckRightStickSwing();
	bool CheckRightStickSwingHeld();
	bool CheckSwingHeld();

	Action pauseBufferedAttack;
	bool pauseBufferedJump;
	bool pauseBufferedDash;

	bool stunBufferedJump;
	bool stunBufferedDash;
	Action stunBufferedAttack;
	
	void ClearPauseBufferedActions();
	void UpdateInHitlag();
	bool IsAttackAction( Action a );
	bool IsGroundAttackAction(Action a);
	bool IsSpringAction(Action a);
	bool IsOnRailAction(Action a);
	bool IsInHistunAction( Action a );

	V2d GetKnockbackDirFromVel();



	int drainCounter;
	int drainCounterMax;
	int drainAmount;


	sf::Vector2<double> dWireAirDash;
	sf::Vector2<double> dWireAirDashOld;
	//end unstored


	//PlayerGhost *testGhost;
	const static int MAX_GHOSTS = 4;
	PlayerGhost *ghosts[MAX_GHOSTS];
	int recordedGhosts;
	int record;
	//bool record;
	bool blah;
	int ghostFrame;

	
	

	

	//sf::Vector2<double> bounceOffset;

};

struct PlayerGhost
{
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
		SPRINT,
		STAND,
		DASHATTACK,
		WALLATTACK,
		STEEPCLIMBATTACK,
		STEEPSLIDEATTACK,
		STANDN,		
		UAIR,
		WALLCLING,
		WALLJUMP,
		STEEPSLIDE,
		GRINDBALL,
		RAILGRIND,
		RAILDASH,
		AIRDASH,
		STEEPCLIMB,
		AIRHITSTUN,
		GROUNDHITSTUN,
		WIREHOLD,
		BOUNCEAIR,
		BOUNCEGROUND,
		BOUNCEGROUNDEDWALL,
		JUMPSQUAT,
		DEATH,
		Count
	};
	PlayerGhost();

	struct P
	{
		sf::Vector2<double> position;
		Action action;
		int frame;
		sf::Sprite s;
		bool showSword;
		sf::Sprite swordSprite1;
		double angle;
		bool createBubble;
		sf::Rect<double> screenRect;
	};
	
	void Draw( sf::RenderTarget *target );
	void UpdatePrePhysics( int ghostFrame );
	void DebugDraw( sf::RenderTarget *target );

	const static int MAX_FRAMES = 300;
	P states[MAX_FRAMES];//[240];
	int totalRecorded;
	int currFrame;

	std::list<CollisionBox> *currHitboxes;

	std::map<int, std::list<CollisionBox>*> fairHitboxes;
	std::map<int, std::list<CollisionBox>*> uairHitboxes;
	std::map<int, std::list<CollisionBox>*> dairHitboxes;
	std::map<int, std::list<CollisionBox>*> standHitboxes;
	std::map<int, std::list<CollisionBox>*> dashHitboxes;
	std::map<int, std::list<CollisionBox>*> wallHitboxes;
	std::map<int, std::list<CollisionBox>*> steepClimbHitboxes;
	std::map<int, std::list<CollisionBox>*> steepSlideHitboxes;
	std::map<int, std::list<CollisionBox>*> diagUpHitboxes;
	std::map<int, std::list<CollisionBox>*> diagDownHitboxes;



};





#endif