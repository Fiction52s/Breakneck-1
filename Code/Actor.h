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
		STANDD,
		STANDN,
		STANDU,
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

	
	enum Expr
	{
		Expr_NEUTRAL,
		Expr_EYES_CLOSED,
		Expr_LOOK_LEFT,
		Expr_RUN,
		Expr_SPRINT,
		Expr_HURT,
		Expr_NONE
	};
	
	void SetActionExpr( Action a );
	void SetExpr( Expr ex );
	Expr expr;
	sf::Vector2<double> movingPlatExtra;
	bool testr;

	Tileset *ts_kinFace;
	sf::Sprite kinFace;

	Actor( GameSession *owner );
	void UpdateSprite();
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
	void RunMovement();
	void AttackMovement();
	void DodecaLateDraw(sf::RenderTarget *target);
	void SetActionGrind();
	bool CanUnlockGate( Gate *g );

	void CheckHoldJump();
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

	int grindActionLength;

	Tileset * tsgsdodeca;
	Tileset * tsgstriblue;
	Tileset * tsgstricym;
	Tileset * tsgstrigreen;
	Tileset * tsgstrioran;
	Tileset * tsgstripurp;
	Tileset * tsgstrirgb;

	sf::Sprite fairSword1;
	Tileset *ts_fairSword1;
	bool showSword1;

	sf::Sprite dairSword1;
	Tileset *ts_dairSword1;

	sf::Sprite uairSword1;
	Tileset *ts_uairSword1;

	sf::Sprite standingNSword1;
	Tileset *ts_standingNSword1;

	sf::Sprite standingDSword1;
	Tileset *ts_standingDSword1;

	sf::Sprite standingUSword1;
	Tileset *ts_standingUSword1;

	Tileset *ts_bounceRun;
	Tileset *ts_bounceSprint;

	Tileset * ts_fx_airdash;
	Tileset * ts_fx_double;
	Tileset * ts_fx_gravReverse;
	Tileset * ts_fx_bigRunRepeat;

	sf::Vector2<double> followerPos;
	sf::Vector2<double> followerVel;
	double followerFac;

	//new variables in here that need to work with clone power later
	int runBounceFlameFrames;
	int runBounceFrame;
	Tileset *ts_runBounceFlame;

	bool steepJump;
	
	int airBounceFlameFrames;
	int airBounceFrame;
	Tileset *ts_airBounceFlame;

	bool bounceFlameOn;
	sf::Sprite bounceFlameSprite;
	double bounceFlameAccel;

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

	double steepClimbGravFactor;
	double steepClimbFastFactor;

	double airDashSpeed;

	CollisionBox b;
	CollisionBox hurtBody;
	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;
	std::map<int, std::list<CollisionBox>*> fairHitboxes;
	std::map<int, std::list<CollisionBox>*> uairHitboxes;
	std::map<int, std::list<CollisionBox>*> dairHitboxes;
	std::map<int, std::list<CollisionBox>*> standNHitboxes;
	std::map<int, std::list<CollisionBox>*> standDHitboxes;
	std::map<int, std::list<CollisionBox>*> standUHitboxes;


	double steepThresh;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action

	int wallJumpMovementLimit;

	sf::SoundBuffer fairBuffer;
	sf::Sound fairSound;

	sf::SoundBuffer runTappingBuffer;
	sf::Sound runTappingSound;

	sf::SoundBuffer dashStartBuffer;
	sf::Sound dashStartSound;

	sf::SoundBuffer jumpBuffer;
	sf::Sound jumpSound;


	sf::SoundBuffer playerHitBuffer;
	sf::Sound playerHitSound;

	double dashHeight;
	double normalHeight;
	double doubleJumpHeight;
	double sprintHeight;

	double airSlow;

	double slopeLaunchMinSpeed;
	double maxRunInit;
	double maxGroundSpeed;
	double runAccelInit;
	double runGain;
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

	int framesGrinding;
	int framesNotGrinding;


	Edge *ground;
	MovingTerrain *movingGround;

	bool hasAirDash;
	bool hasGravReverse;

	Edge *grindEdge;
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
	MovingTerrain *bounceMovingTerrain;
	double bounceQuant;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;
	bool boostBounce;
	

	
	//dont need to store these
	const static int maxBubbles = 6;
	int bubbleLifeSpan;
	int bubbleRadius;
	Tileset * ts_bubble;
	sf::Sprite bubbleSprite;
	//--
	Tileset *ts_fx_hurtSpack;
	Tileset *ts_fx_dashStart;
	Tileset *ts_fx_dashRepeat;
	Tileset *ts_fx_land;
	
	sf::Vector2<double> bubblePos[maxBubbles];
	int bubbleFramesToLive[maxBubbles];
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
	bool hasKey[Gate::GateType::Count];
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
	double storedGroundSpeed;
	
	bool holdDouble;
	int framesSinceDouble;
	
	sf::Shader swordShader;
	//bool hitlagFlash;
	sf::Color flashColor;
	int flashFrames;

	bool bufferedAttack;

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
		STANDD,
		STANDN,
		STANDU,
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
		bool showSword1;
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
	std::map<int, std::list<CollisionBox>*> standNHitboxes;
	std::map<int, std::list<CollisionBox>*> standDHitboxes;
	std::map<int, std::list<CollisionBox>*> standUHitboxes;



};

#endif