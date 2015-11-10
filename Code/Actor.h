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


struct GameSession;
struct PlayerGhost;

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
		DEATH,
		Count
	};

	Actor( GameSession *owner );
	
	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void ApplyHit( HitboxInfo *info );
	bool ResolvePhysics( sf::Vector2f vel );
	void UpdatePhysics();
	void UpdatePostPhysics();
	bool CheckWall( bool right );
	bool CheckStandUp();
	sf::Vector2f UpdateReversePhysics();
	void Draw( sf::RenderTarget *target );
	void DebugDraw( sf::RenderTarget *target );
	void HandleRayCollision( Edge *edge, float edgeQuantity, float rayPortion );
	void UpdateHitboxes();
	void AirMovement();
	float GroundedAngle();
	float GroundedAngleAttack( sf::Vector2f &trueNormal );
	sf::Vector2i GetWireOffset();
	void RunMovement();
	void AttackMovement();
	
	Light *testLight;

	//unsaved vars
	int possibleEdgeCount;
	GameSession *owner;
	float steepClimbSpeedThresh;
	Contact minContact;
	sf::Shader sh;
	bool collision;
	sf::Sprite *sprite;
	Tileset *tileset[Count];
	Tileset *normal[Count];

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
	Tileset * ts_fx_float;
	Tileset * ts_fx_gravReverse;
	Tileset * ts_fx_bigRunRepeat;

	
	float offSlopeByWallThresh;
	const static int MAX_MOTION_GHOSTS = 2;
	sf::Sprite motionGhosts[MAX_MOTION_GHOSTS];
	int numMotionGhosts;
	float percentCloneChanged;
	float percentCloneRate;
	bool changingClone;

	float holdDashAccel;
	float wallThresh;

	float airDashSpeed;

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


	float steepThresh;

	int actionLength[Action::Count]; //actionLength-1 is the max frame counter for each action

	int wallJumpMovementLimit;

	sf::SoundBuffer fairBuffer;
	sf::Sound fairSound;

	sf::SoundBuffer runTappingBuffer;
	sf::Sound runTappingSound;

	sf::SoundBuffer dashStartBuffer;
	sf::Sound dashStartSound;


	sf::SoundBuffer playerHitBuffer;
	sf::Sound playerHitSound;

	float dashHeight;
	float normalHeight;
	float floatJumpHeight;
	float sprintHeight;

	float airSlow;

	float slopeLaunchMinSpeed;
	float maxRunInit;
	float maxGroundSpeed;
	float runAccelInit;
	float runGain;
	float sprintAccel;

	float maxNormalRun;
	float runAccel;
	float maxFallSpeed;
	float gravity;

	float jumpStrength;
	float airAccel;
	float maxAirXSpeed;
	float maxAirXControl;
	float dashSpeed;

	float floatJumpStrength;

	int timeSlowStrength;

	sf::Vector2f wallJumpStrength;
	float clingSpeed;

	bool col;
	sf::Vector2f tempVel;
	std::string queryMode;
	MovingTerrain *currMovingTerrain;

	int maxDespFrames;
	int despCounter;
	bool desperationMode;
	//MovingTerrain *minContactTerrain;

	bool checkValid;

	Edge *rcEdge;
	float rcQuantity;
	std::string rayCastMode;

	bool leftGround;
	float grindActionCurrent;
	ControllerState prevInput;
	ControllerState currInput;
	sf::Vector2f oldVelocity;
	int framesInAir;
	sf::Vector2f startAirDashVel;

	int framesGrinding;
	int framesNotGrinding;

	Edge *ground;
	MovingTerrain *movingGround;

	bool hasAirDash;
	bool hasGravReverse;

	Edge *grindEdge;
	MovingTerrain *grindMovingTerrain;
	float grindQuantity;
	float grindSpeed;

	bool reversed;

	float edgeQuantity;
	
	float groundOffsetX;

	float offsetX;

	bool bounceGrounded;

	bool holdJump;

	int wallJumpFrameCounter;

	float groundSpeed;

	bool facingRight;
	
	bool hasfloatJump;

	int slowMultiple;
	int slowCounter;

	sf::Vector2f wallNormal;
	int testGrassCount;
	Action action;
	int frame;
	sf::Vector2f position;
	sf::Vector2f velocity;
	//CollisionBox *physBox;

	int hitlagFrames;
	int hitstunFrames;
	int invincibleFrames;
	HitboxInfo *receivedHit;

	sf::Vector2f storedBounceVel;
	sf::Vector2f bounceNorm;
	sf::Vector2f oldBounceNorm;
	float storedBounceGroundSpeed;
	
	Wire *leftWire;
	Wire *rightWire;
	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;

	Edge *bounceEdge;
	MovingTerrain *bounceMovingTerrain;
	float bounceQuant;
	Edge *oldBounceEdge;
	int framesSinceBounce;
	bool groundedWallBounce;

	

	
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
	
	sf::Vector2f bubblePos[maxBubbles];
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


	//unstored while working on
	bool dead;	

	//end unstored

	void SaveState();
	void LoadState();

	struct Stored
	{
		bool leftGround;
		float grindActionCurrent;
		ControllerState prevInput;
		ControllerState currInput;
		sf::Vector2f oldVelocity;
		int framesInAir;
		sf::Vector2f startAirDashVel;
		Edge *ground;
		Edge *lastGround;
		bool hasAirDash;
		bool hasGravReverse;

		Edge *grindEdge;
		bool bounceGrounded;
		float grindQuantity;
		float grindSpeed;

		bool reversed;

		float edgeQuantity;
	
		float groundOffsetX;

		float offsetX;

		bool holdJump;

		int wallJumpFrameCounter;

		float groundSpeed;

		bool facingRight;
	
		bool hasfloatJump;

		int slowMultiple;
		int slowCounter;

		sf::Vector2f wallNormal;

		Action action;
		int frame;
		sf::Vector2f position;
		sf::Vector2f velocity;
		//CollisionBox *physBox;

		int hitlagFrames;
		int hitstunFrames;
		int invincibleFrames;
		HitboxInfo *receivedHit;

		sf::Vector2f storedBounceVel;
		Wire *wire;
		Edge *bounceEdge;
		float bounceQuant;

		Edge *oldBounceEdge;
		int framesSinceBounce;

		bool touchEdgeWithLeftWire;
		bool touchEdgeWithRightWire;

		sf::Vector2f bubblePos[maxBubbles];
		int bubbleFramesToLive[maxBubbles];
		int currBubble;

		sf::Vector2f bounceNorm;
		sf::Vector2f oldBounceNorm;
		float storedBounceGroundSpeed;

		bool groundedWallBounce;

		int framesGrinding;
		int framesNotGrinding;
	};
	Stored stored;


	//float rotation;
	



	PlayerGhost *testGhost;
	const static int MAX_GHOSTS = 4;
	PlayerGhost *ghosts[MAX_GHOSTS];
	int recordedGhosts;
	int record;
	//bool record;
	bool blah;
	int ghostFrame;

	

	

	//sf::Vector2f bounceOffset;

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
		DEATH,
		Count
	};
	PlayerGhost();

	struct P
	{
		sf::Vector2f position;
		Action action;
		int frame;
		sf::Sprite s;
		bool showSword1;
		sf::Sprite swordSprite1;
		float angle;
		bool createBubble;
		sf::Rect<float> screenRect;
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