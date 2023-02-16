#include "GGPO.h"
#include <windows.h>
#include "Session.h"
#include <iostream>
#include "NetplayManager.h"

using namespace std;

void SaveGameState::Print()
{
	cout << "totalGameFrames: " << totalGameFrames << "\n";
	cout << "activeEnemyList: " << activeEnemyList << "\n";
	cout << "activeEnemyListTail: " << activeEnemyListTail << "\n";
	cout << "inactiveEnemyList: " << inactiveEnemyList << "\n";
	cout << "pauseFrames: " << pauseFrames << "\n";
	cout << "currSuperPlayer: " << currSuperPlayer << "\n";
	cout << "gameState: " << gameState << "\n";
	cout << "activeSequence: " << activeSequence << "\n";
	cout << "next frame restart: " << (int)nextFrameRestartGame << "\n";
}

void PState::Print()
{
	cout << "position: " << position.x << ", " << position.y << "\n";
	cout << "velocity: " << velocity.x << ", " << velocity.y << "\n";
	cout << "flashFrames: " << flashFrames << "\n";
	cout << "bufferedAttack: " << bufferedAttack << "\n";
	cout << "doubleJumpBufferedAttack: " << doubleJumpBufferedAttack << "\n";
	cout << "wallJumpBufferedAttack: " << wallJumpBufferedAttack << "\n";
	cout << "pauseBufferedAttack: " << pauseBufferedAttack << "\n";
	cout << "pauseBufferedJump: " << pauseBufferedJump << "\n";
	cout << "pauseBufferedDash: " << pauseBufferedDash << "\n";
	cout << "attackingHitlag: " << attackingHitlag << "\n";
	cout << "hitlagFrames: " << hitlagFrames << "\n";
	cout << "hitstunFrames: " << hitstunFrames << "\n";
	cout << "setHitstunFrames: " << setHitstunFrames << "\n";
	cout << "invincibleFrames: " << invincibleFrames << "\n";
	cout << "receivedHit: " << "not implemented" << "\n";
	cout << "xOffset: " << xOffset << "\n";
	cout << "action: " << action << "\n";
	cout << "frame: " << frame << "\n";
	cout << "groundSpeed: " << groundSpeed << "\n";
	cout << "prevInput: " << prevInput << "\n";
	cout << "currInput: " << currInput << "\n";
	cout << "ground: " << ground << "\n";
	cout << "quant: " << quant << "\n";
	cout << "brh: " << brh << "\n";
	cout << "byoffset: " << byoffset << "\n";
	cout << "bpos: " << bpos.x << ", " << bpos.y << "\n";
	cout << "facingRight: " << facingRight << "\n";
	cout << "holdDouble: " << holdDouble << "\n";
	cout << "framesSinceClimbBoost: " << framesSinceClimbBoost << "\n";
	cout << "holdJump: " << holdJump << "\n";
	cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << "\n";
	cout << "hasDoubleJump: " << hasDoubleJump << "\n";
	cout << "framesInAir: " << framesInAir << "\n";
	cout << "hasAirDash: " << hasAirDash << "\n";
	cout << "numRemainingExtraAirdashBoosts: " << numRemainingExtraAirdashBoosts << "\n";
	cout << "storedGroundSpeed: " << storedGroundSpeed << "\n";
	cout << "currBBoostCounter: " << currBBoostCounter << "\n";
	cout << "currAirdashBoostCounter: " << currAirdashBoostCounter << "\n";
	cout << "steepJump: " << steepJump << "\n";
	cout << "speedLevel: " << speedLevel << "\n";
	cout << "currentSpeedBar: " << currentSpeedBar << "\n";
	cout << "airDashStall: " << airDashStall << "\n";
	cout << "startAirDashVel: " << startAirDashVel.x << ", " << startAirDashVel.y << "\n";
	cout << "extraAirDashY: " << extraAirDashY << "\n";
	cout << "oldAction: " << oldAction << "\n";
	cout << "oldVelocity: " << oldVelocity.x << ", " << oldVelocity.y << "\n";
	cout << "reversed: " << reversed << "\n";
	cout << "storedReverseSpeed: " << storedReverseSpeed << "\n";
	cout << "grindActionCurrent: " << grindActionCurrent << "\n";
	cout << "framesGrinding: " << framesGrinding << "\n";
	cout << "framesNotGrinding: " << framesNotGrinding << "\n";
	cout << "framesSinceGrindAttempt: " << framesSinceGrindAttempt << "\n";
	cout << "maxFramesSinceGrindAttempt: " << maxFramesSinceGrindAttempt << "\n";
	cout << "grindEdge: " << grindEdge << "\n";
	cout << "grindSpeed: " << grindSpeed << "\n";
	cout << "slowMultiple: " << slowMultiple << "\n";
	cout << "slowCounter: " << slowCounter << "\n";
	cout << "inBubble: " << inBubble << "\n";
	cout << "oldInBubble: " << oldInBubble << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
	//cout << "hasAirDash: " << hasAirDash << "\n";
}

int fletcher32_checksum(short *data, size_t len)
{
	int sum1 = 0xffff, sum2 = 0xffff;

	while (len) {
		size_t tlen = len > 360 ? 360 : len;
		len -= tlen;
		do {
			sum1 += *data++;
			sum2 += sum1;
		} while (--tlen);
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	return sum2 << 16 | sum1;
}

/*
* vw_begin_game_callback --
*
* The begin game callback.  We don't need to do anything special here,
* so just return true.
*/
bool __cdecl
begin_game_callback(const char *)
{
	return true;
}


/*
* vw_on_event_callback --
*
* Notification from GGPO that something has happened.  Update the status
* text at the bottom of the screen to notify the user.
*/
bool __cdecl
on_event_callback(GGPOEvent *info)
{
	Session *sess = Session::GetSession();
	int progress;
	switch (info->code) {
	case GGPO_EVENTCODE_CONNECTED_TO_PEER:
		
		/*if (sess->netplayManager != NULL)
		{
			cout << "leaving lobby because ggpo is now connected" << endl;
			sess->netplayManager->LeaveLobby();
		}*/

		cout << "connected to peer" << endl;
		sess->ngs->SetConnectState(info->u.connected.player, Synchronizing);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
		cout << "synchronizing to peer" << endl;
		progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
		sess->ngs->UpdateConnectProgress(info->u.synchronizing.player, progress);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
		cout << "synchronized with peer" << endl;
		sess->ngs->UpdateConnectProgress(info->u.synchronized.player, 100);
		break;
	case GGPO_EVENTCODE_RUNNING:
		sess->ggpoReady = true;
		cout << "running" << endl;
		sess->ngs->SetConnectState(Running);
		//renderer->SetStatusText("");
		break;
	case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
		cout << "connection interrupted" << endl;
		sess->ngs->SetDisconnectTimeout(info->u.connection_interrupted.player,
			timeGetTime(),
			info->u.connection_interrupted.disconnect_timeout);
		break;
	case GGPO_EVENTCODE_CONNECTION_RESUMED:
		cout << "connection resumed" << endl;
		sess->ngs->SetConnectState(info->u.connection_resumed.player, Running);
		break;
	case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
		cout << "disconnected from peer" << endl;
		sess->ngs->SetConnectState(info->u.disconnected.player, Disconnected);
		break;
	case GGPO_EVENTCODE_TIMESYNC:
		cout << "timesync: " << info->u.timesync.frames_ahead << endl;
		//Sleep(1000 * info->u.timesync.frames_ahead / 60 ); //buggy
		sess->timeSyncFrames = info->u.timesync.frames_ahead;
		break;
	}
	return true;
}


/*
* vw_advance_frame_callback --
*
* Notification from GGPO we should step foward exactly 1 frame
* during a rollback.
*/
bool __cdecl
advance_frame_callback(int flags)
{
	int disconnect_flags;

	Session *sess = Session::GetSession();

	sess->frameConfirmed = false;
	ggpo_synchronize_input(sess->ggpo, (void *)sess->ggpoCompressedInputs, COMPRESSED_INPUT_SIZE * GGPO_MAX_PLAYERS,
		&disconnect_flags);

	sess->CopyGGPOInputsToParallelSessions();

	sess->UpdateAllPlayersInput();

	sess->SimulateGGPOGameFrame();
	return true;
}

/*
* vw_load_game_state_callback --
*
* Makes our current state match the state passed in by GGPO.
*/
bool __cdecl
load_game_state_callback(unsigned char *buffer, int len)
{
	//cout << "rollback" << "\n";
	Session *sess = Session::GetSession();
	return sess->LoadState(buffer, len);
}

/*
* vw_save_game_state_callback --
*
* Save the current state to a buffer and return it to GGPO via the
* buffer and len parameters.
*/
bool __cdecl
save_game_state_callback(unsigned char **buffer, int *len, int *checksum, int frame)
{
	//cout << "save state" << "\n";
	Session *sess = Session::GetSession();
	return sess->SaveState(buffer, len, checksum, frame);
}

bool __cdecl
log_game_state(char *filename, unsigned char *buffer, int)
{
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "w");
	if (fp) {
		fprintf(fp, "test\n");
		/*GameState *gamestate = (GameState *)buffer;
		fprintf(fp, "GameState object.\n");
		fprintf(fp, "  bounds: %d,%d x %d,%d.\n", gamestate->_bounds.left, gamestate->_bounds.top,
			gamestate->_bounds.right, gamestate->_bounds.bottom);
		fprintf(fp, "  num_ships: %d.\n", gamestate->_num_ships);
		for (int i = 0; i < gamestate->_num_ships; i++) {
			Ship *ship = gamestate->_ships + i;
			fprintf(fp, "  ship %d position:  %.4f, %.4f\n", i, ship->position.x, ship->position.y);
			fprintf(fp, "  ship %d velocity:  %.4f, %.4f\n", i, ship->velocity.dx, ship->velocity.dy);
			fprintf(fp, "  ship %d radius:    %d.\n", i, ship->radius);
			fprintf(fp, "  ship %d heading:   %d.\n", i, ship->heading);
			fprintf(fp, "  ship %d health:    %d.\n", i, ship->health);
			fprintf(fp, "  ship %d speed:     %d.\n", i, ship->speed);
			fprintf(fp, "  ship %d cooldown:  %d.\n", i, ship->cooldown);
			fprintf(fp, "  ship %d score:     %d.\n", i, ship->score);
			for (int j = 0; j < MAX_BULLETS; j++) {
				Bullet *bullet = ship->bullets + j;
				fprintf(fp, "  ship %d bullet %d: %.2f %.2f -> %.2f %.2f.\n", i, j,
					bullet->position.x, bullet->position.y,
					bullet->velocity.dx, bullet->velocity.dy);
			}
		}*/
		fclose(fp);
	}
	return true;
}

/*
* vw_free_buffer --
*
* Free a save state buffer previously returned in vw_save_game_state_callback.
*/
void __cdecl
free_buffer(void *buffer)
{
	free(buffer);
}

void __cdecl 
confirm_frame(int frameCount)
{
	Session *sess = Session::GetSession();
	sess->ConfirmFrame(frameCount);
}