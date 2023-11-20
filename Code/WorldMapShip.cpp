#include "WorldMapShip.h"
#include "WorldMap.h"
#include "Tileset.h"
#include "VectorMath.h"

using namespace std;
using namespace sf;

WorldMapShip::WorldMapShip(WorldMap *wm)
{
	ts = wm->GetSizedTileset("Menu/WorldMap/ship_128x128.png");
	SetAlpha(1.0);
	facingIndex = 0;
	frame = 0;

	scale = 1.f;
}

void WorldMapShip::Update(ControllerDualStateQueue *controllerInput)
{
	ControllerState currState = controllerInput->GetCurrState();
	float maxSpeed = 10;

	if (currState.leftStickMagnitude > GameController::stickThresh )
	{
		float x = cos(currState.leftStickRadians) * currState.leftStickMagnitude;
		float y = -sin(currState.leftStickRadians) * currState.leftStickMagnitude;
		
		sf::Vector2f movement(round(x * maxSpeed), round(y * maxSpeed));

		if (currState.LUp())
		{
			if (currState.LLeft())
			{
				facingIndex = 5;
			}
			else if (currState.LRight())
			{
				facingIndex = 7;
			}
			else
			{
				facingIndex = 6;
			}
		}
		else if (currState.LDown())
		{
			if (currState.LLeft())
			{
				facingIndex = 3;
			}
			else if (currState.LRight())
			{
				facingIndex = 1;
			}
			else
			{
				facingIndex = 2;
			}
		}
		else
		{
			if (currState.LLeft())
			{
				facingIndex = 4;
			}
			else if (currState.LRight())
			{
				facingIndex = 0;
			}
		}


		//facingIndex = (currState.leftStickDirection / 8);
		//currState.leftStickRadians

		SetPosition(position + movement);

		++frame;
	}
	else if (controllerInput->ButtonHeld_PadAny())
	{
		if (controllerInput->ButtonHeld_PadUp())
		{
			if (controllerInput->ButtonHeld_PadLeft())
			{
				facingIndex = 5;
			}
			else if (controllerInput->ButtonHeld_PadRight())
			{
				facingIndex = 7;
			}
			else
			{
				facingIndex = 6;
			}
		}
		else if (controllerInput->ButtonHeld_PadDown())
		{
			if (controllerInput->ButtonHeld_PadLeft())
			{
				facingIndex = 3;
			}
			else if (controllerInput->ButtonHeld_PadRight())
			{
				facingIndex = 1;
			}
			else
			{
				facingIndex = 2;
			}
		}
		else
		{
			if (controllerInput->ButtonHeld_PadLeft())
			{
				facingIndex = 4;
			}
			else if (controllerInput->ButtonHeld_PadRight())
			{
				facingIndex = 0;
			}
		}

		sf::Vector2f movement(0, 0);
		if (controllerInput->ButtonHeld_PadLeft())
		{
			movement.x = -1;
		}
		else if (controllerInput->ButtonHeld_PadRight())
		{
			movement.x = 1;
		}

		if (controllerInput->ButtonHeld_PadUp())
		{
			movement.y = -1;
		}
		else if (controllerInput->ButtonHeld_PadDown())
		{
			movement.y = 1;
		}

		movement = normalize(movement) * maxSpeed;

		SetPosition(position + movement);

		++frame;
	}
	else
	{
		frame = 0;
	}

	UpdateQuads();
	/*else
	{
		if (currState.LLeft())
		{
			myPos.x -= 10;
		}
		else if (currState.LRight())
		{
			myPos.x += 10;
		}

		if (currState.LUp())
		{
			myPos.y -= 10;
		}
		else if (currState.LDown())
		{
			myPos.y += 10;
		}
	}*/

	
}

void WorldMapShip::SetPosition(sf::Vector2f &pos)
{
	position = pos;

	Vector2f limit(1920, 1080);

	if (position.x < 0)
	{
		position.x = 0;
	}
	if (position.x > limit.x)
	{
		position.x = limit.x;
	}
	if (position.y < 0)
	{
		position.y = 0;
	}
	if (position.y > limit.y)
	{
		position.y = limit.y;
	}

	
}

sf::Vector2f WorldMapShip::GetPosition()
{
	return position;
}

void WorldMapShip::UpdateQuads()
{
	ts->SetQuadSubRect(shipQuad, facingIndex * 5 + (( frame / 2 ) % 5));
	SetRectCenter(shipQuad, ts->tileWidth * scale, ts->tileHeight * scale, position);
}

void WorldMapShip::SetAlpha(float alpha)
{
	if (alpha < 0)
		alpha = 0;
	for (int i = 0; i < 4; ++i)
	{
		shipQuad[i].color.a = min(alpha * 255.f, 255.f);
	}
}

void WorldMapShip::Draw(sf::RenderTarget *target)
{
	target->draw(shipQuad, 4, sf::Quads, ts->texture);
}