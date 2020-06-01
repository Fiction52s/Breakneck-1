#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WALLATTACK_Start()
{
}

void Actor::WALLATTACK_End()
{
	SetActionExpr(WALLCLING);
	frame = 0;
}

void Actor::WALLATTACK_Change()
{
	if (!BasicAirAttackAction() && frame > 6)
	{
		if (currInput.LDown())
		{
			SetAction(JUMP);
			frame = 1;
			return;
		}
		else
		{
			if (!facingRight)
			{
				if (currInput.LLeft())
				{
					SetAction(WALLJUMP);
					frame = 0;
					return;
				}
			}
			else
			{
				if (currInput.LRight())
				{
					SetAction(WALLJUMP);
					frame = 0;
					return;
				}
			}
		}
	}
}

void Actor::WALLATTACK_Update()
{
	int f = frame / 2;
	if (f < 8)
	{
		SetCurrHitboxes(wallHitboxes[speedLevel], frame / 2);
	}


	if (frame == 0)
	{
		currAttackHit = false;
		ActivateSound(S_WALLATTACK);
		//	fairSound.play();
	}

	if (velocity.y > clingSpeed)
	{
		//cout << "running wallcling" << endl;
		velocity.y = clingSpeed;
	}

	double constantWallCling = 5;
	if (facingRight)
	{
		velocity.x = -constantWallCling;
	}
	else
	{
		velocity.x = constantWallCling;
	}
}

void Actor::WALLATTACK_UpdateSprite()
{
	Tileset *curr_ts = ts_wallAttackSword[speedLevel];
	//cout << "fair frame : " << frame / 2 << endl;
	int startFrame = 0;
	showSword = true;//frame >= startFrame && frame / 2 <= 9;

	if (showSword)
	{
		wallAttackSword.setTexture(*curr_ts->texture);
	}

	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	//Vector2i offset( 32, -16 );
	Vector2i offset(-8, -8);

	if (showSword)
	{
		if (facingRight)
		{
			wallAttackSword.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
			wallAttackSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}

		wallAttackSword.setOrigin(wallAttackSword.getLocalBounds().width / 2, wallAttackSword.getLocalBounds().height / 2);
		wallAttackSword.setPosition(position.x + offset.x, position.y + offset.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::WALLATTACK_TransitionToAction(int a)
{

}

void Actor::WALLATTACK_TimeIndFrameInc()
{

}

void Actor::WALLATTACK_TimeDepFrameInc()
{

}

int Actor::WALLATTACK_GetActionLength()
{
	return 8 * 2;
}

Tileset * Actor::WALLATTACK_GetTileset()
{
	return GetActionTileset("wall_att_64x128.png");
}