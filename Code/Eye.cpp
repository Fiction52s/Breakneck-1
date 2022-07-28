#include "Eye.h"
#include "GameSession.h"
#include "Enemy_Patroller.h"

PatrollerEye::PatrollerEye(Patroller *p)
	:parent(p)
{
	highlight = (eye + 4);
	ts_eye = p->sess->GetTileset("Enemies/W1/patroller_eye_48x48.png", 48, 48);
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
	activateRange = 900;
	trackRange = 700;
	deactivateRange = 1200;
	deactivateTrackRange = 1000;
}

void PatrollerEye::Reset()
{
	data.frame = 0;
	data.state = S_WAITING;
}

void PatrollerEye::Draw(sf::RenderTarget *target, sf::Shader *sh)
{
	sf::RenderStates rs;
	rs.shader = sh;
	rs.texture = ts_eye->texture;
	target->draw(eye, 8, sf::Quads, rs);
}

bool PatrollerEye::IsEyeActivated()
{
	return data.state == S_TARGET || data.state == S_ACTIVE;
}

void PatrollerEye::ProcessState( sf::Vector2f &targetPos )
{
	sf::Vector2f dir = normalize(targetPos - data.pos);
	float dist = length(targetPos - data.pos);
	data.angle = atan2(dir.y, dir.x);
	float angleDeg = data.angle / PI * 180.f;

	switch (data.state)
	{
	case S_WAITING:
	{
		if (dist < activateRange )
		{
			if (actionLength[S_STARTUP] > 0)
				data.state = S_STARTUP;
			else if (actionLength[S_TARGET] > 0)
				data.state = S_TARGET;
			else
				data.state = S_ACTIVE;
			data.frame = 0;
		}
		break;
	}
	case S_STARTUP:
	{
		if (data.frame == actionLength[S_STARTUP])
		{
			data.state = S_WAITTOTARGET;
			data.frame = 0;
		}
		break;
	}
	case S_WAITTOTARGET:
	{
		if (dist < trackRange)
		{
			if (actionLength[S_TARGET] > 0)
				data.state = S_TARGET;
			else
				data.state = S_ACTIVE;
			data.frame = 0;
		}
		else if (dist < deactivateRange)
		{
		}
		else
		{
			data.state = S_SHUTDOWN;
			data.frame = 0;
		}
		break;
	}
	case S_TARGET:
	{
		if (data.frame == actionLength[S_TARGET])
		{
			data.state = S_ACTIVE;
			data.frame = 0;
		}
		break;
	}
	case S_ACTIVE:
	{
		if (dist > deactivateTrackRange)
		{
			data.state = S_UNTARGET;
			data.frame = 0;
		}
		break;
	}
	case S_UNTARGET:
	{
		if (data.frame == actionLength[S_UNTARGET])
		{
			data.state = S_WAITTOTARGET;
			data.frame = 0;
		}
		break;
	}
	case S_SHUTDOWN:
	{
		if (data.frame == actionLength[S_SHUTDOWN])
		{
			data.state = S_WAITING;
			data.frame = 0;
		}
		break;
	}
	}

	switch (data.state)
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

	if (data.state == S_ACTIVE)
	{
		
	}
	else
	{
		data.angle = 0;
	}

	//std::cout << "state: " << state << " frame: " << frame << std::endl;
}

void PatrollerEye::UpdateSprite()
{
	//std::cout << "state: " << state << "frame: " << frame << std::endl;
	int tile = 0;
	switch (data.state)
	{
	case S_WAITING:
	{
		tile = 0;
		break;
	}
	case S_STARTUP:
	{
		tile = data.frame / animFactor[S_STARTUP] + 1;
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
		tile = (1 - data.frame ) / animFactor[S_STARTUP] + 1;
		break;
	}
	}
	SetRectSubRect(eye, ts_eye->GetSubRect(tile));

	++data.frame;
}

void PatrollerEye::SetPosition(sf::Vector2f &p_pos)
{
	data.pos = p_pos;
	SetRectRotation(eye, data.angle, ts_eye->tileWidth * parent->scale, ts_eye->tileHeight * parent->scale, data.pos);
	SetRectCenter(highlight, ts_eye->tileWidth * parent->scale, ts_eye->tileHeight * parent->scale, p_pos);
	//SetRectRotation(highlight, 0, ts_eye->tileWidth, ts_eye->tileHeight, pos);
	//SetRectCenter(eye, ts_eye->tileWidth, ts_eye->tileHeight, p_pos);
}

int PatrollerEye::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void PatrollerEye::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void PatrollerEye::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	bytes += sizeof(MyData);
}