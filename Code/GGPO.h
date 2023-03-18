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
	double xOffset;
	int action;
	int frame;
	double groundSpeed;
	COMPRESSED_INPUT_TYPE prevInput;
	COMPRESSED_INPUT_TYPE currInput;
	EdgeInfo groundInfo;
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
	EdgeInfo grindEdgeInfo;
	double grindSpeed;

	int slowMultiple;
	int slowCounter;
	bool inBubble;
	bool oldInBubble;

	V2d bubblePos[Actor::MAX_BUBBLES];
	int bubbleRadiusSize[Actor::MAX_BUBBLES];
	int bubbleFramesToLive[Actor::MAX_BUBBLES];
	int currBubble;
	bool currAttackHit;
	bool bounceAttackHit;
	//flashframes

	int cancelAttack;

	bool dairBoostedDouble;
	bool aerialHitCancelDouble;

	CollisionBox hurtBody;

	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;
	V2d dWireAirDash;
	V2d dWireAirDashOld;

	Wire::MyData leftWireInfo;
	Wire::MyData rightWireInfo;

	bool scorpOn;
	int framesFlameOn;
	bool bounceFlameOn;
	V2d storedBounceVel;
	V2d bounceNorm;
	V2d oldBounceNorm;
	EdgeInfo bounceEdgeInfo;
	double storedBounceGroundSpeed;
	EdgeInfo oldBounceEdgeInfo;
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
	int receivedHitPlayerIndex;
	int receivedHitReaction;

	bool hasWallJumpRechargeDoubleJump;
	bool hasWallJumpRechargeAirDash;
	bool hasHitRechargeDoubleJump;
	bool hasHitRechargeAirDash;

	int framesBlocking;
	V2d receivedHitPosition;

	int superFrame;

	int kinMode;

	int prevRailID;
	bool specialSlow;
	bool frameAfterAttackingHitlagOver;

	//new stuff I added recently
	int projectileSwordFrames;
	int enemyProjectileSwordFrames;
	int gravModifyFrames;
	double extraGravityModifier;
	V2d waterEntrancePosition;
	EdgeInfo waterEntranceGroundInfo;
	EdgeInfo waterEntranceGrindEdgeInfo;
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

	int currTimeBoosterID;
	int currFreeFlightBoosterID;
	int currHomingBoosterID;
	int currAntiTimeSlowBoosterID;
	int currSwordProjectileBoosterID;
	int currPhaseBoosterID;
	int currMomentumBoosterID;
	int currRewindBoosterID;

	int aimLauncherStunFrames;
	int airBounceCounter;
	int airBounceLimit;
	int phaseFrames;
	int momentumBoostFrames;

	int currSpringID;
	int currAimLauncherID;
	int currTeleporterID;
	int oldTeleporterID;
	int currBoosterID;
	int oldBoosterID;
	int currSwingLauncherID;
	int oldSwingLauncherID;
	int currBounceBoosterID;
	int oldBounceBoosterID;
	int currScorpionLauncherID;
	int oldScorpionLauncherID;

	int springStunFrames;
	int springStunFramesStart;
	int directionalInputFreezeFrames;

	bool bouncedFromKill;

	int numFramesToLive;

	int shieldPushbackFrames;
	bool shieldPushbackRight;

	Actor::Hitter recentHitters[Actor::MAX_HITTERS];

	ComboObject *activeComboObjList;

	int currTutorialObjectID;
	int currGravModifierID;

	V2d springVel;
	double glideTurnFactor;

	bool hitGoal;

	void Print();
};

struct Sequence;
struct SaveGameState
{
	int totalGameFrames;

	int activeEnemyListID;
	int activeEnemyListTailID;
	int inactiveEnemyListID;

	int currentZoneID;

	int pauseFrames;
	int currSuperPlayerIndex;
	int gameState; //game mode such as RUN or FROZEN
	bool nextFrameRestartGame;
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