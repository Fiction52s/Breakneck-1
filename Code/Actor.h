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

#include "Gate.h"

struct Gate;
struct Critical;
struct GameSession;
struct PlayerGhost;
struct Monitor;
struct Enemy;

struct Actor : QuadTreeCollider,
	RayCastHandler
{
	enum Action
	{
		DAIR,
		DASH,
		DOUBLE,
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
		GOALKILL,
		GOALKILLWAIT,
		SPAWNWAIT,
		DEATH,
		RIDESHIP,
		SKYDIVE,
		SKYDIVETOFALL,
		WAITFORSHIP,
		GRABSHIP,
		DIAGUPATTACK,
		DIAGDOWNATTACK,
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
		S_HURT,
		S_HIT_AND_KILL,
		S_HIT_AND_KILL_KEY,
		S_FAIR,
		S_DAIR,
		S_UAIR,
		S_STANDATTACK,
		S_WALLJUMP,
		S_WALLATTACK,
		S_GRAVREVERSE,
		S_BOUNCEJUMP,
		S_TIMESLOW,
		S_ENTER,
		S_EXIT,
		S_DIAGUPATTACK,
		S_DIAGDOWNATTACK,
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

	Action spriteAction;
	int currTileIndex;
	bool flipTileX;
	bool flipTileY;

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
	void SetActionExpr( Action a );
	void SetExpr( Expr ex );
	void SetAction( Action a );

	bool AirAttack();
	Expr expr;
	sf::Vector2<double> movingPlatExtra;
	bool testr;

	Tileset **ts_goalKillArray;
	
	bool toggleBounceInput;
	bool toggleTimeSlowInput;
	bool toggleGrindInput;

	void EnterNexus( int nexusIndex, sf::Vector2<double> &pos );

	void SetFakeCurrInput( 
		ControllerState &state );

	Tileset *ts_kinFace;
	sf::Shader despFaceShader;
	sf::Sprite kinFace;
	sf::Sprite kinUnderOutline;
	sf::Sprite kinTealOutline;
	sf::Sprite kinBlueOutline;
	sf::Sprite kinPurpleOutline;
	sf::Color currentDespColor;

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
	void ConfirmHit( int worldIndex, 
		int flashFrames, double speedBar, int charge );
	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void ApplyHit( HitboxInfo *info );
	bool ResolvePhysics( sf::Vector2<double> vel );
	bool CaptureMonitor( Monitor *m );
	void UpdatePhysics();
	void UpdateFullPhysics();
	void PhysicsResponse();
	bool physicsOver;
	void GroundAttack();
	void ConfirmEnemyKill( Enemy *e );
	bool IHitPlayer( int otherPlayerIndex );
	std::pair<bool,bool> PlayerHitMe( int otherPlayerIndex );
	

	void ShipPickupPoint( double eq,
		bool facingRight );
	void GrabShipWire();


	void UpdatePostPhysics();
	bool CheckWall( bool right );
	bool CheckStandUp();
	sf::Vector2<double> UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	void UpdateHitboxes();
	void AirMovement();
	double GroundedAngle();
	double GroundedAngleAttack( sf::Vector2<double> &trueNormal );
	sf::Vector2i GetWireOffset();
	void RunMovement( double bounceFlameAccel );
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
	double speedBarTarget;
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
	Tileset *normal[Count];

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
	bool showSword;

	sf::Sprite dairSword;
	Tileset *ts_dairSword[3];
	//sf::Vector2i dairOffset[3];

	sf::Sprite uairSword;
	Tileset *ts_uairSword[3];

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

	Tileset *ts_bounceRun;
	Tileset *ts_bounceSprint;

	Tileset * ts_fx_airdashUp;
	Tileset * ts_fx_airdashDiagonal;
	Tileset * ts_fx_airdashSmall;
	Tileset * ts_fx_airdashHover;

	Tileset *ts_fx_run;
	Tileset *ts_fx_runStart;
	Tileset *ts_fx_sprint;

	Tileset * ts_fx_double;
	Tileset * ts_fx_wallJump;
	
	Tileset * ts_fx_gravReverse;
	Tileset * ts_fx_bigRunRepeat;
	Tileset * ts_fx_chargeBlue0;
	Tileset * ts_fx_chargeBlue1;
	Tileset * ts_fx_chargeBlue2;
	Tileset * ts_fx_chargePurple;
	Tileset *ts_fx_hurtSpack;
	Tileset *ts_fx_dashStart;
	Tileset *ts_fx_dashRepeat;
	Tileset *ts_fx_land;
	Tileset *ts_fx_jump;

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

	//new variables in here that need to work with clone power later
	int runBounceFlameFrames;
	int runBounceFrame;
	Tileset *ts_runBounceFlame;

	bool steepJump;
	
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

	bool test;
	
	double offSlopeByWallThresh;
	const static int MAX_MOTION_GHOSTS = 4;
	sf::Sprite motionGhosts[MAX_MOTION_GHOSTS];
	int numMotionGhosts;
	int motionGhostSpacing;
	int ghostSpacingCounter;
	float percentCloneChanged;
	float percentCloneRate;
	bool changingClone;

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
	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;
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
	std::map<int, std::list<CollisionBox>*> shockwaveHitboxes;
	std::map<int, std::list<CollisionBox>*> grindHitboxes;
	//CollisionBox grindHitbox;



	double steepThresh;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action

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

	double doubleJumpStrength;

	int timeSlowStrength;

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
	sf::Vector2<double> startAirDashVel;
	double extraAirDashY;
	sf::Vector2<double> AddGravity( sf::Vector2<double> vel );

	int framesGrinding;
	int framesNotGrinding;


	Edge *ground;
	MovingTerrain *movingGround;

	bool hasAirDash;
	bool hasGravReverse;

	Edge *grindEdge;
	sf::Vector2<double> lungeNormal;
	double grindLungeSpeed0;
	double grindLungeSpeed1;
	double grindLungeSpeed2;
	//double grindLungeExtraMax;
	MovingTerrain *grindMovingTerrain;
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
	int testGrassCount;
	Action action;
	int frame;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	//CollisionBox *physBox;

	int hitlagFrames;
	int hitstunFrames;
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
	

	
	//dont need to store these
	const static int maxBubbles = 5;
	int bubbleLifeSpan;
	int bubbleRadius;
	int bubbleRadius0;
	int bubbleRadius1;
	int bubbleRadius2;
	
	int GetBubbleRadius();
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	//--
	
	
	sf::Vector2<double> bubblePos[maxBubbles];
	int bubbleFramesToLive[maxBubbles];
	int bubbleRadiusSize[maxBubbles];
	int currBubble;

	
	bool hasPowerAirDash;
	bool hasPowerGravReverse;
	bool hasPowerBounce;
	bool hasPowerGrindBall;
	bool hasPowerTimeSlow;
	bool hasPowerLeftWire;
	bool hasPowerRightWire;
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

	int drainCounter;
	int drainCounterMax;

	Critical *currentCheckPoint;

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

		sf::Vector2<double> bubblePos[maxBubbles];
		int bubbleFramesToLive[maxBubbles];
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


struct SprInfo
{
	sf::Vector2f position;
	sf::Vector2f origin;
	float rotation;
	bool flipX;
	bool flipY;

	int action;
	int tileIndex;

	int speedLevel;
};

struct RecordGhost
{
	RecordGhost( Actor *p );
	int frame;
	void StartRecording();
	void StopRecording();
	int numTotalFrames;
	Actor *player;
	void WriteToFile( const std::string &fileName );
	void RecordFrame();
	const static int MAX_RECORD = 3600 * 60;
	SprInfo sprBuffer[MAX_RECORD]; //1 hour
};

struct ReplayGhost
{
	bool init;
	ReplayGhost( Actor *p );
	bool OpenGhost( const std::string &fileName );
	sf::Sprite replaySprite;
	
	void UpdateReplaySprite();
	
	void Draw( sf::RenderTarget *target );

	SprInfo *sprBuffer;
	Actor *player;
	int frame;
	int numTotalFrames;
};

#endif