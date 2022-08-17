#ifndef __GPPO_H__
#define __GPPO_H__

#include "VectorMath.h"
#include "ggponet.h"
#include "Physics.h"
#include "Wire.h"
#include "Grass.h"
#include "Actor.h"
#include "Camera.h"
#pragma comment(lib, "wsock32.lib")


//struct Edge;

enum PlayerConnectState {
	Connecting = 0,
	Synchronizing,
	Running,
	Disconnected,
	Disconnecting,
};

struct PlayerConnectionInfo {
	GGPOPlayerType       type;
	GGPOPlayerHandle     handle;
	PlayerConnectState   state;
	int                  connect_progress;
	int                  disconnect_timeout;
	int                  disconnect_start;
};

struct GGPONonGameState
{
	struct ChecksumInfo {
		int framenumber;
		int checksum;
	};

	void SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state) {
		for (int i = 0; i < num_players; i++) {
			if (playerInfo[i].handle == handle) {
				playerInfo[i].connect_progress = 0;
				playerInfo[i].state = state;
				break;
			}
		}
	}

	void SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout) {
		for (int i = 0; i < num_players; i++) {
			if (playerInfo[i].handle == handle) {
				playerInfo[i].disconnect_start = when;
				playerInfo[i].disconnect_timeout = timeout;
				playerInfo[i].state = Disconnecting;
				break;
			}
		}
	}

	void SetConnectState(PlayerConnectState state) {
		for (int i = 0; i < num_players; i++) {
			playerInfo[i].state = state;
		}
	}

	void UpdateConnectProgress(GGPOPlayerHandle handle, int progress) {
		for (int i = 0; i < num_players; i++) {
			if (playerInfo[i].handle == handle) {
				playerInfo[i].connect_progress = progress;
				break;
			}
		}
	}

	GGPOPlayerHandle     local_player_handle;
	PlayerConnectionInfo playerInfo[4];
	int                  num_players;

	ChecksumInfo         now;
	ChecksumInfo         periodic;
};


struct SaveWireInfo
{
	float shaderOffset;
	bool foundPoint;
	WirePoint anchor;
	WirePoint points[Wire::MAX_POINTS];
	Wire::WireState state;
	sf::Vector2i offset;
	V2d fireDir;
	int framesFiring;
	int frame;
	int numPoints;
	V2d realAnchor;
	bool canRetractGround;
	V2d closestPoint;
	double closestDiff;
	int fusePointIndex;
	V2d oldPos;
	V2d storedPlayerPos;
	V2d retractPlayerPos;
	V2d currOffset;
	V2d hitEnemyDelta;
	V2d anchorVel;
	V2d quadOldPosA;
	V2d quadOldWirePosB;
	V2d quadWirePosC;
	V2d quadPlayerPosD;
	double fuseQuantity;
	double minSideOther;
	double minSideAlong;
	double totalLength;
	double segmentLength;
	double minSegmentLength;
	double pullStrength;
	double dragStrength;
	int hitEnemyFrame;
	int hitEnemyFramesTotal;
	int firingTakingUp;
	int numVisibleIndexes;
	int newWirePoints;
	int aimingPrimaryAngleRange;
	int hitStallCounter;
	int antiWireGrassCount;
	CollisionBox movingHitbox;
	bool clockwise;
	Edge *rcEdge;
	double rcQuant;
	double rcCancelDist;
};

struct PState
{
	V2d position;
	V2d velocity;
	int flashFrames;
	int bufferedAttack;
	int doubleJumpBufferedAttack;
	int wallJumpBufferedAttack;
	int pauseBufferedAttack;
	bool pauseBufferedJump;
	bool pauseBufferedDash;

	bool attackingHitlag;
	int hitlagFrames;
	int hitstunFrames;
	int setHitstunFrames;
	int invincibleFrames;
	HitboxInfo receivedHit;
	//sf::Vector2<double> bubblePos[5];
	double xOffset;
	int action;
	int frame;
	double groundSpeed;
	int prevInput;
	int currInput;
	Edge *ground;
	double quant;
	
	double brh;
	double byoffset;
	V2d bpos;

	bool facingRight;
	bool holdDouble;
	int framesSinceClimbBoost;
	bool holdJump;
	int wallJumpFrameCounter;
	bool hasDoubleJump;
	int framesInAir;
	bool hasAirDash;
	int numRemainingExtraAirdashBoosts;
	double storedGroundSpeed;
	int currBBoostCounter;
	int currAirdashBoostCounter;
	bool steepJump;
	int speedLevel; //0,1,2
	double currentSpeedBar;
	bool airDashStall;
	sf::Vector2<double> startAirDashVel;
	double extraAirDashY;
	int oldAction;
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

	sf::Vector2<double> bubblePos[Actor::MAX_BUBBLES];
	int bubbleRadiusSize[Actor::MAX_BUBBLES];
	int bubbleFramesToLive[Actor::MAX_BUBBLES];
	int currBubble;
	bool currAttackHit;
	bool bounceAttackHit;
	//flashframes

	
	

	//CollisionBody *currHitboxes;
	//int currHitboxFrame;
	int cancelAttack;

	bool dairBoostedDouble;
	bool aerialHitCancelDouble;

	CollisionBox hurtBody;

	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;
	V2d dWireAirDash;
	V2d dWireAirDashOld;

	SaveWireInfo leftWireInfo;
	SaveWireInfo rightWireInfo;

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

	int dashAttackLevel;
	int standAttackLevel;
	int upTiltLevel;
	int downTiltLevel;
	int framesSinceDashAttack;
	int framesSinceStandAttack;
	int framesSinceUpTilt;
	int framesSinceDownTilt;

	int framesSinceBlockPress;
	int framesSinceSuperPress;

	bool standNDashBoost;
	int standNDashBoostCurr;
	bool hasFairAirDashBoost;
	int framesStanding;
	int framesSinceRightWireBoost;
	int framesSinceLeftWireBoost;
	int framesSinceDoubleWireBoost;
	int enemiesKilledThisFrame;
	int enemiesKilledLastFrame;

	bool touchedGrass[Grass::GrassType::Count];

	double hitstunGravMultiplier;

	
	int superLevelCounter;
	int currActionSuperLevel;

	int blockstunFrames;
	int currAttackHitBlock[4];
	Actor *receivedHitPlayer;
	int receivedHitReaction;

	bool hasWallJumpRechargeDoubleJump;
	bool hasWallJumpRechargeAirDash;
	bool hasHitRechargeDoubleJump;
	bool hasHitRechargeAirDash;

	int framesBlocking;
	V2d receivedHitPosition;

	int superFrame;

	int kinMode;

	TerrainRail * prevRail;
	bool specialSlow;
	bool frameAfterAttackingHitlagOver;

	//new stuff I added recently
	int projectileSwordFrames;
	int enemyProjectileSwordFrames;
	int gravModifyFrames;
	double extraGravityModifier;
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
	int modifiedDrainFrames;
	int modifiedDrain;
	int invertInputFrames;
	int currPowerMode;
	PolyPtr oldSpecialTerrain;
	PolyPtr currSpecialTerrain;
	int globalTimeSlowFrames;
	int freeFlightFrames;
	int homingFrames;
	int antiTimeSlowFrames;
	TimeBooster *currTimeBooster;
	FreeFlightBooster *currFreeFlightBooster;
	HomingBooster *currHomingBooster;
	AntiTimeSlowBooster *currAntiTimeSlowBooster;
	SwordProjectileBooster *currSwordProjectileBooster;
	PhaseBooster *currPhaseBooster;
	MomentumBooster *currMomentumBooster;
	RewindBooster *currRewindBooster;
	int aimLauncherStunFrames;
	int airBounceCounter;
	int airBounceLimit;
	int phaseFrames;
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
	ScorpionLauncher *currScorpionLauncher;
	ScorpionLauncher *oldScorpionLauncher;

	

	int springStunFrames;
	int springStunFramesStart;
	int directionalInputFreezeFrames;

	bool bouncedFromKill;

	int numFramesToLive;

	int shieldPushbackFrames;
	bool shieldPushbackRight;

	Actor::Hitter recentHitters[Actor::MAX_HITTERS];

	ComboObject *activeComboObjList;

	TutorialObject *currTutorialObject;
	GravityModifier *currGravModifier;

	V2d springVel;
	double glideTurnFactor;

	//HitboxInfo currVSHitboxInfo;

	bool hitGoal;

	void Print();
};

struct Sequence;
struct SaveGameState
{
	int totalGameFrames;
	Enemy *activeEnemyList;
	Enemy *activeEnemyListTail;
	Enemy *inactiveEnemyList;
	int pauseFrames;
	Actor *currSuperPlayer;
	int gameState; //game mode such as RUN or FROZEN
	Sequence  *activeSequence;
	uint32 randomState;
	Camera cam;
	void Print();
};

int fletcher32_checksum(short *data, 
	size_t len);

bool __cdecl
begin_game_callback(const char *);

bool __cdecl
on_event_callback(GGPOEvent *info);

bool __cdecl
advance_frame_callback(int flags);

bool __cdecl
load_game_state_callback(unsigned char *buffer, int len);

bool __cdecl
save_game_state_callback(unsigned char **buffer, int *len, int *checksum, int frame);

bool __cdecl
log_game_state(char *filename, unsigned char *buffer, int);

void __cdecl
free_buffer(void *buffer);

void __cdecl 
confirm_frame(int frameCount);

#endif