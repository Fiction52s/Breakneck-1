#include "Eye.h"
#include "GameSession.h"

PatrollerEye::PatrollerEye( GameSession *owner )
{
	highlight = (eye + 4);
	ts_eye = owner->GetTileset("Enemies/patroller_eye_48x48.png", 48, 48);
	SetRectSubRect(eye, ts_eye->GetSubRect(0));
	SetRectSubRect(highlight, ts_eye->GetSubRect(8));
	Reset();


	animFactor[S_WAITING] = 1;
	animFactor[S_STARTUP] = 1;
	animFactor[S_TARGET] = 1;
	animFactor[S_ACTIVE] = 1;
	animFactor[S_UNTARGET] = 1;
	animFactor[S_SHUTDOWN] = 1;

	actionLength[S_WAITING] = 1 * animFactor[S_WAITING];
	actionLength[S_STARTUP] = 2 * animFactor[S_STARTUP];
	actionLength[S_TARGET] = 1 * animFactor[S_TARGET];
	actionLength[S_ACTIVE] = 1 * animFactor[S_ACTIVE];
	actionLength[S_UNTARGET] = 1 * animFactor[S_UNTARGET];
	actionLength[S_SHUTDOWN] = 2 * animFactor[S_SHUTDOWN];
	activateRange = 700;
	trackRange = 600;
	deactivateRange = 1200;
	deactivateTrackRange = 1000;
}

void PatrollerEye::Reset()
{
	frame = 0;
	state = S_WAITING;
}

void PatrollerEye::Draw(sf::RenderTarget *target)
{
	target->draw(eye, 8, sf::Quads, ts_eye->texture);
}

bool PatrollerEye::IsEyeActivated()
{
	return state == S_TARGET || state == S_ACTIVE;
}

void PatrollerEye::ProcessState( sf::Vector2f &targetPos )
{
	sf::Vector2f dir = normalize(targetPos - pos);
	float dist = length(targetPos - pos);
	angle = atan2(dir.y, dir.x);
	float angleDeg = angle / PI * 180.f;

	switch (state)
	{
	case S_WAITING:
	{
		if (dist < activateRange )
		{
			if (actionLength[S_STARTUP] > 0)
				state = S_STARTUP;
			else if (actionLength[S_TARGET] > 0)
				state = S_TARGET;
			else
				state = S_ACTIVE;
			frame = 0;
		}
		break;
	}
	case S_STARTUP:
	{
		if (frame == actionLength[S_STARTUP])
		{
			state = S_WAITTOTARGET;
			frame = 0;
		}
		break;
	}
	case S_WAITTOTARGET:
	{
		if (dist < trackRange)
		{
			if (actionLength[S_TARGET] > 0)
				state = S_TARGET;
			else
				state = S_ACTIVE;
			frame = 0;
		}
		else if (dist < deactivateRange)
		{
		}
		else
		{
			state = S_SHUTDOWN;
			frame = 0;
		}
		break;
	}
	case S_TARGET:
	{
		if (frame == actionLength[S_TARGET])
		{
			state = S_ACTIVE;
			frame = 0;
		}
		break;
	}
	case S_ACTIVE:
	{
		if (dist > deactivateTrackRange)
		{
			state = S_UNTARGET;
			frame = 0;
		}
		break;
	}
	case S_UNTARGET:
	{
		if (frame == actionLength[S_UNTARGET])
		{
			state = S_WAITTOTARGET;
			frame = 0;
		}
		break;
	}
	case S_SHUTDOWN:
	{
		if (frame == actionLength[S_SHUTDOWN])
		{
			state = S_WAITING;
			frame = 0;
		}
		break;
	}
	}

	switch (state)
	{
	case S_WAITING:
	{
		
		break;
	}
	case S_STARTUP:
	{
		break;
	}
	case S_WAITTOTARGET:
	{
		break;
	}
	case S_TARGET:
	{
		break;
	}
	case S_ACTIVE:
	{
		
		break;
	}
	case S_UNTARGET:
	{
		break;
	}
	case S_SHUTDOWN:
	{
		break;
	}
	}

	if (state == S_ACTIVE)
	{
		
	}
	else
	{
		angle = 0;
	}

	//std::cout << "state: " << state << " frame: " << frame << std::endl;
}

void PatrollerEye::UpdateSprite()
{
	//std::cout << "state: " << state << "frame: " << frame << std::endl;
	int tile = 0;
	switch (state)
	{
	case S_WAITING:
	{
		tile = 0;
		break;
	}
	case S_STARTUP:
	{
		tile = frame / animFactor[S_STARTUP] + 1;
		break;
	}
	case S_WAITTOTARGET:
	{
		tile = 3;
		break;
	}
	case S_TARGET:
	{
		tile = 4;
		break;
	}
	case S_ACTIVE:
	{
		tile = 5;
		break;
	}
	case S_UNTARGET:
	{
		tile = 5;
		break;
	}
	case S_SHUTDOWN:
	{
		tile = (1 - frame ) / animFactor[S_STARTUP] + 1;
		break;
	}
	}
	SetRectSubRect(eye, ts_eye->GetSubRect(tile));

	++frame;
}

void PatrollerEye::SetPosition(sf::Vector2f &p_pos)
{
	pos = p_pos;
	SetRectRotation(eye, angle, ts_eye->tileWidth, ts_eye->tileHeight, pos);
	SetRectCenter(highlight, ts_eye->tileWidth, ts_eye->tileHeight, p_pos);
	//SetRectRotation(highlight, 0, ts_eye->tileWidth, ts_eye->tileHeight, pos);
	//SetRectCenter(eye, ts_eye->tileWidth, ts_eye->tileHeight, p_pos);
}