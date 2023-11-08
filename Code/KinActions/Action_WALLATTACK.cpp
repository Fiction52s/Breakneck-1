#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::WALLATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(PlayerSounds::S_WALLATTACK);
	ResetAttackHit();
}

void Actor::WALLATTACK_End()
{
	SetAction(WALLCLING);
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

	if (frame == 0 && slowCounter == 1)
	{

		TryThrowSwordProjectileBasic();

	}

	if (frame == 0)
	{
		
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
		swordSprite.setTexture(*curr_ts->texture);
	}

	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	//Vector2i offset( 32, -16 );
	Vector2i offset(-8, -8);

	if (showSword)
	{
		if (facingRight)
		{
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}

		swordSprite.setRotation(0);
		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height / 2);
		swordSprite.setPosition(position.x + offset.x, position.y + offset.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::WALLATTACK_TransitionToAction(int a)
{
	ResetSuperLevel();
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

const char * Actor::WALLATTACK_GetTilesetName()
{
	return "wall_att_64x128.png";
}