#ifndef __GPPO_H__
#define __GPPO_H__

#include "VectorMath.h"
#include "ggponet.h"
#include "Physics.h"
#include "Wire.h"
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
	bool foundPoint;
	WirePoint anchor;
	WirePoint points[Wire::MAX_POINTS];
	Wire::WireState state;
	sf::Vector2i offset;
	V2d fireDir;
	int framesFiring;
	int frame;
	int numPoints;
};

struct PState
{
	const static int MAX_BUBBLES = 5;

	HitboxInfo *receivedHit;
	//sf::Vector2<double> bubblePos[5];
	double xOffset;
	V2d position;
	V2d velocity;
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
	double storedGroundSpeed;
	int currBBoostCounter;
	bool steepJump;
	int speedLevel; //0,1,2
	double currentSpeedBar;
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
	int bubbleRadiusSize[MAX_BUBBLES];
	int bubbleFramesToLive[MAX_BUBBLES];
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
	

	//CollisionBody *currHitboxes;
	//int currHitboxFrame;
	int cancelAttack;

	int currRing;
	int prevRingValue[3]; //3 fillringsections as part of KinRing
	int currRingValue[3];

	bool dairBoostedDouble;
	bool aerialHitCancelDouble;

	CollisionBox hurtBody;

	bool touchEdgeWithLeftWire;
	bool touchEdgeWithRightWire;
	V2d dWireAirDash;
	V2d dWireAirDashOld;

	SaveWireInfo leftWireInfo;
	SaveWireInfo rightWireInfo;


};

struct SaveGameState
{
	PState states[2];
	int totalGameFrames;
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

#endif