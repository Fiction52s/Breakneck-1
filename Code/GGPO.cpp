#include "GGPO.h"
#include <windows.h>
#include "GameSession.h"



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
	int progress;
	switch (info->code) {
	case GGPO_EVENTCODE_CONNECTED_TO_PEER:
		//ngs.SetConnectState(info->u.connected.player, Synchronizing);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
		//progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
		//ngs.UpdateConnectProgress(info->u.synchronizing.player, progress);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
		//ngs.UpdateConnectProgress(info->u.synchronized.player, 100);
		break;
	case GGPO_EVENTCODE_RUNNING:
		//ngs.SetConnectState(Running);
		//renderer->SetStatusText("");
		break;
	case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
		//ngs.SetDisconnectTimeout(info->u.connection_interrupted.player,
		//	timeGetTime(),
		//	info->u.connection_interrupted.disconnect_timeout);
		break;
	case GGPO_EVENTCODE_CONNECTION_RESUMED:
		//ngs.SetConnectState(info->u.connection_resumed.player, Running);
		break;
	case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
		//ngs.SetConnectState(info->u.disconnected.player, Disconnected);
		break;
	case GGPO_EVENTCODE_TIMESYNC:
		Sleep(1000 * info->u.timesync.frames_ahead / 60);
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
	int compressedInputs[4] = { 0 };
	int disconnect_flags;

	GameSession *game = GameSession::GetSession();

	//// Make sure we fetch new inputs from GGPO and use those to update
	//// the game state instead of reading from the keyboard.
	ggpo_synchronize_input(game->ggpo, (void *)compressedInputs, sizeof(int) * 4, 
		&disconnect_flags);
	for (int i = 0; i < 2; ++i)
	{
		game->GetCurrInput(i).SetFromCompressedState(compressedInputs[i]);
	}

	game->AdvanceFrame();

	//VectorWar_AdvanceFrame(inputs, disconnect_flags);
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
	GameSession *game = GameSession::GetSession();
	return game->LoadState(buffer, len);
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
	GameSession *game = GameSession::GetSession();
	return game->SaveState(buffer, len, checksum, frame);
}

bool __cdecl
log_game_state(char *filename, unsigned char *buffer, int)
{
	/*FILE* fp = nullptr;
	fopen_s(&fp, filename, "w");
	if (fp) {
		GameState *gamestate = (GameState *)buffer;
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
		}
		fclose(fp);
	}*/
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

void GGPOInit()
{
	GGPOErrorCode result;


}