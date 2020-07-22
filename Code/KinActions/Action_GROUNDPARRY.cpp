#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDPARRY_Start()
{
}

void Actor::GROUNDPARRY_End()
{
	if (currInput.Y)
	{
		if (currInput.LLeft())
		{
			facingRight = false;
		}
		else if (currInput.LRight())
		{
			facingRight = true;
		}

		if (currInput.LUp())
		{
			SetAction(GROUNDBLOCKHIGH);
		}
		else if (currInput.LDown())
		{
			SetAction(GROUNDBLOCKLOW);
		}
		else
		{
			SetAction(GROUNDBLOCK);
		}

		frame = 0;
	}
	else
	{
		SetAction(STAND);
		frame = 0;
	}
}

void Actor::GROUNDPARRY_Change()
{
	if (frame > 0)
	{
		if (currInput.Y && !prevInput.Y)
		{
			if (currInput.LLeft())
			{
				facingRight = false;
			}
			else if (currInput.LRight())
			{
				facingRight = true;
			}

			if (currInput.LUp())
			{
				SetAction(GROUNDBLOCKHIGH);
			}
			else if (currInput.LDown())
			{
				SetAction(GROUNDBLOCKLOW);
			}
			else
			{
				SetAction(GROUNDBLOCK);
			}
		}
	}
	/*if (!currInput.Y && blockstunFrames == 0)
	{
		SetAction(STAND);
		frame = 0;
	}*/

	//else if( BasicGroundAction)

}

void Actor::GROUNDPARRY_Update()
{
}

void Actor::GROUNDPARRY_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();
}

void Actor::GROUNDPARRY_TransitionToAction(int a)
{
}

void Actor::GROUNDPARRY_TimeIndFrameInc()
{

}

void Actor::GROUNDPARRY_TimeDepFrameInc()
{

}

int Actor::GROUNDPARRY_GetActionLength()
{
	return 15;
}

Tileset * Actor::GROUNDPARRY_GetTileset()
{
	return GetActionTileset("parry_64x64.png");
}