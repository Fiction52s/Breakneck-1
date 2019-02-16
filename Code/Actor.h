#ifndef __ACTOR_H__
#define __ACTOR_H__

#include <list>
#include <map>
#include <SFML/Audio.hpp>
#include "Tileset.h"
#include "Physics.h"
#include "Input.h"
#include <SFML/Graphics.hpp>
#include "Wire.h"
#include "Light.h"
#include "AirParticles.h"
#include "Movement.h"
#include "Gate.h"
#include "VisualEffects.h"

struct Gate;
struct Critical;
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

struct KinSkin
{
	KinSkin(sf::Color *startCols, sf::Color *endCols,
		int p_numChanges, int p_index)
		:startColors(startCols), endColors( endCols ),
		numChanges( p_numChanges ), index( p_index )
	{

	}
	sf::Color *startColors;
	sf::Color *endColors;
	int numChanges;
	int index;
};

struct MotionGhostEffect
{
	MotionGhostEffect( int maxGhosts );
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
struct AbsorbParticles
{
	enum AbsorbType
	{
		ENERGY,
		DARK,
		SHARD,
	};
	
	struct SingleEnergyParticle
	{
		
		SingleEnergyParticle(AbsorbParticles *parent,
			int tileIndex );
		void UpdateSprite();
		bool Update();
		void Activate(sf::Vector2f &pos, sf::Vector2f &vel);
		void Clear();
		sf::Vector2f pos;
		int frame;
		int tileIndex;
		sf::Vector2f velocity;
		AbsorbParticles *parent;
		int lockFrame;
		float lockDist;

		SingleEnergyParticle *next;
		SingleEnergyParticle *prev;
		
		//Vector2f accel;
	};

	AbsorbType abType;
	sf::Vector2f GetTargetPos(AbsorbType ab);
	AbsorbParticles( GameSession *owner,
		AbsorbType p_abType );
	~AbsorbParticles();	
	void Reset();
	sf::Vertex *va;
	int maxNumParticles;
	void Activate( Actor *playerTarget, int storedHits, V2d &pos,
		float startAngle = 0 );
	void Update();
	void Draw(sf::RenderTarget *rt);
	float startAngle;
	GameSession *owner;
	Tileset *ts;
	int animFactor;
	sf::Vector2f pos;
	sf::Vector2f *particlePos;
	int numActivatedParticles;
	Actor *playerTarget;
	double maxSpeed;
	SingleEnergyParticle *GetInactiveParticle();
	void DeactivateParticle(SingleEnergyParticle *sp);
	SingleEnergyParticle *activeList;
	SingleEnergyParticle *inactiveList;
	void AllocateParticle(int tileIndex );
};

struct Booster;
struct Spring;
struct EffectPool;
struct EnemyParams;
struct SoundNode;

struct GroundTrigger;

struct Actor : QuadTreeCollider,
	RayCastHandler
{
	//EffectPool *testPool;
	EffectPool *smallLightningPool[7];
	EffectPool *risingAuraPool;
	MotionGhostEffect *motionGhostsEffects[3];

	EffectPool *dustParticles;
	RisingParticleUpdater rpu;
	void HandleGroundTrigger(GroundTrigger *trigger);
	
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
		EXIT,
		EXITWAIT,
		GOALKILL,
		GOALKILL1,
		GOALKILL2,
		GOALKILL3,
		GOALKILL4,
		GOALKILLWAIT,
		NEXUSKILLWAIT,
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
	sf::SoundBuffer *soundBuffers[SoundType::S_Count];

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
	


	bool IsGoalKillAction(Action a);
	bool IsIntroAction(Action a);
	bool IsExitAction(Action a);
	bool IsSequenceAction(Action a);

	void CreateAttackLightning();
	EffectPool *fairLightningPool[4];
	EffectPool *uairLightningPool[4];
	EffectPool *dairLightningPool[4];
	EffectPool *gateBlackFXPool;

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

	/*enum SeqType
	{
		SEQ_NOTHING,
		SEQ_CRAWLER_FIGHT,
		SEQ_Count
	};
	void StartSeq( SeqType s );
	void UpdateSeq();
	ControllerState seqInput;
	bool cutInput;

	
	SeqType seq;
	int seqFrame;*/
	
	void SetSpriteTexture( Action a );
	void SetSpriteTile( int tileIndex, bool noFlipX = true, bool noFlipY = true );
	void SetSpriteTile( sf::Sprite *spr, 
		Tileset *t, int tileIndex, bool noFlipX = true, bool noFlipY = true );
	void SetActionExpr( Action a );
	void SetExpr( Expr ex );
	void SetAction( Action a );

	void SetupTilesets(KinSkin *kSkin, KinSkin *swordSkin);

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

	Tileset *ts_fx_rune0;
	Tileset *ts_fx_rune1;
	Tileset *ts_fx_rune2;




	sf::Sprite runeSprite;
	int runeStep;
	int runeLength;
	bool showRune;

	HitboxInfo *wireChargeInfo;
	//sf::Shader speedBarShader;
	bool hitGoal;
	int enemiesKilledThisFrame;
	int enemiesKilledLastFrame;

	Actor( GameSession *owner, int actorIndex );
	int actorIndex;
	void UpdateSprite();
	void ConfirmEnemyNoKill( Enemy *e );
	void ConfirmHit(EnemyParams * hitParams);
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
	bool BasicGroundAction( V2d &gNorm);
	bool BasicSteepAction(V2d &gNorm);
	bool BasicAirAttackAction();

	bool justToggledBounce;
	sf::Vector2<double> UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateHitboxes();
	void AirMovement();
	double GroundedAngle();
	double GroundedAngleAttack( sf::Vector2<double> &trueNormal );
	sf::Vector2i GetWireOffset();
	void RunMovement();
	void AutoRunMovement();
	void SetAutoRun( bool fr, double maxAutoRun );
	double maxAutoRunSpeed;
	void AttackMovement();
	void DodecaLateDraw(sf::RenderTarget *target);
	void SetActionGrind();
	bool CanUnlockGate( Gate *g );

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
	Light *testLight;
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
	ParticleTrail *pTrail;
	double rotaryAngle;
	//AirParticleEffect *ae;
	//double speedBarVel;


	
	//unsaved vars
	int possibleEdgeCount;
	GameSession *owner;
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
	Booster *currBooster;
	Booster *oldBooster;
	sf::Vector2<double> springVel;
	int springStunFrames;

	//these are for your max vel
	double scorpAdditionalCap;
	double scorpAdditionalCapMax;
	double scorpAdditionalAccel;
	double maxVelocity;
	bool highAccuracyHitboxes;
	double GetNumSteps();

	double CalcLandingSpeed( sf::Vector2<double> &testVel,
		sf::Vector2<double> &alongVel, 
		sf::Vector2<double> &gNorm );
	double CalcRailLandingSpeed(sf::Vector2<double> &testVel,
		sf::Vector2<double> &alongDir,
		sf::Vector2<double> &railNorm); 

	void SetAerialScorpSprite();
	int GetJumpFrame();

	int framesExtendingAirdash;

	
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
	sf::Sprite *motionGhosts;// [MAX_MOTION_GHOSTS];
	VertexBuf *motionGhostBuffer;
	VertexBuf *motionGhostBufferBlue;
	VertexBuf *motionGhostBufferPurple;
	VertexBuf *testBuffer;
	

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
	//CollisionBox grindHurtBody;
	CollisionBody *currHitboxes;
	int currHitboxFrame;
	void SetCurrHitboxes(CollisionBody *cBody,
		int p_frame)
	{
		currHitboxes = cBody;
		currHitboxFrame = p_frame;
	}
	ComboObject *activeComboObjList;
	void AddActiveComboObj(ComboObject *c);
	void RemoveActiveComboObj(ComboObject *c);
	void DebugDrawComboObj(sf::RenderTarget *target);
	bool IsMovingRight();
	bool IsMovingLeft();
	//int numCurrHitboxes;

	CollisionBody *currHurtboxes;
	int currHurtboxFrame;
	std::map<Action, CollisionBody*> hurtboxMap;
	bool IntersectMyHurtboxes(CollisionBody *cb,
		int cbFrame );
	bool IntersectMyHitboxes(CollisionBody *cb,
		int cbFrame);
	bool IntersectMySlowboxes(CollisionBody *cb,
		int cbFrame );
	ComboObject * IntersectMyComboHitboxes(CollisionBody *cb,
		int cbFrame);
	bool EnemyIsFar(V2d &enemyPos);

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
	MovingTerrain *currMovingTerrain;

	int maxDespFrames;
	int despCounter;
	bool desperationMode;
	//MovingTerrain *minContactTerrain;

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

	int framesGrinding;
	int framesNotGrinding;

	int framesSinceGrindAttempt;
	int maxFramesSinceGrindAttempt;
	bool canGrabRail;
	double minRailGrindSpeed[3];
	double GetMinRailGrindSpeed();

	sf::RectangleShape railTest;

	Edge *ground;
	MovingTerrain *movingGround;

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
	//double grindLungeExtraMax;
	MovingTerrain *grindMovingTerrain;
	double grindQuantity;
	double grindSpeed;

	double slopeTooSteepLaunchLimitX;

	void HitEdge(
		sf::Vector2<double> &newVel );

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
	int testGrassCount;
	int gravityGrassCount;
	Action action;
	int frame;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
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
	MovingTerrain *bounceMovingTerrain; //not used
	double bounceQuant;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;
	bool boostBounce;
	
	void InitAfterEnemies();
	bool wallClimbGravityFactor;
	bool wallClimbGravityOn;
	
	//dont need to store these
	//const static int MAX_BUBBLES = 5;
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
	
	CollisionBody * GetBubbleHitbox(int index);
	sf::Vector2<double> *bubblePos;//[maxBubbles];
	int *bubbleFramesToLive;//[maxBubbles];
	int *bubbleRadiusSize;//[maxBubbles];
	float *fBubbleRadiusSize;
	sf::Vector2f *fBubblePos;
	CollisionBody **bubbleHitboxes;
	HitboxInfo *hInfo;

	float *fBubbleFrame;

	int currBubble;

	
	bool hasPowerAirDash;
	bool hasPowerGravReverse;
	bool hasPowerBounce;
	bool hasPowerGrindBall;
	bool hasPowerTimeSlow;
	bool hasPowerLeftWire;
	bool hasPowerRightWire;

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
	/*bool hasBlueKey;
	bool hasGreenKey;
	bool hasYellowKey;
	bool hasOrangeKey;
	bool hasRedKey;
	bool hasMagentaKey;*/

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

	Action pauseBufferedAttack;
	bool pauseBufferedJump;
	bool pauseBufferedDash;
	
	void ClearPauseBufferedActions();
	void UpdateInHitlag();
	bool IsAttackAction( Action a );
	bool IsGroundAttackAction(Action a);



	int drainCounter;
	int drainCounterMax;
	int drainAmount;

	Critical *currentCheckPoint;

	sf::Vector2<double> dWireAirDash;
	sf::Vector2<double> dWireAirDashOld;
	//end unstored

	void SaveState();
	void LoadState();

	struct Stored
	{
		bool leftGround;
		double grindActionCurrent;
		ControllerState prevInput;
		ControllerState currInput;
		sf::Vector2<double> oldVelocity;
		int framesInAir;
		sf::Vector2<double> startAirDashVel;
		Edge *ground;
		Edge *lastGround;
		bool hasAirDash;
		bool hasGravReverse;

		Edge *grindEdge;
		bool bounceGrounded;
		double grindQuantity;
		double grindSpeed;

		bool reversed;

		double edgeQuantity;
	
		double groundOffsetX;

		double offsetX;

		bool holdJump;

		int wallJumpFrameCounter;

		double groundSpeed;

		bool facingRight;
	
		bool hasDoubleJump;

		int slowMultiple;
		int slowCounter;

		sf::Vector2<double> wallNormal;
		

		Action action;
		int frame;
		sf::Vector2<double> position;
		sf::Vector2<double> velocity;
		//CollisionBox *physBox;

		int hitlagFrames;
		int hitstunFrames;
		int invincibleFrames;
		HitboxInfo *receivedHit;

		sf::Vector2<double> storedBounceVel;
		Wire *wire;
		Edge *bounceEdge;
		double bounceQuant;

		Edge *oldBounceEdge;
		int framesSinceBounce;

		bool touchEdgeWithLeftWire;
		bool touchEdgeWithRightWire;

		//sf::Vector2<double> bubblePos[maxBubbles];
		//int bubbleFramesToLive[maxBubbles];
		int currBubble;

		

		sf::Vector2<double> bounceNorm;
		sf::Vector2<double> oldBounceNorm;
		double storedBounceGroundSpeed;

		bool groundedWallBounce;

		int framesGrinding;
		int framesNotGrinding;







		//not added yet to the functions
		bool boostBounce;
		int lastWire;
	};
	Stored stored;


	//double rotation;
	



	PlayerGhost *testGhost;
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