#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK2_Start()
{
	DASHATTACK_Start();
}

void Actor::DASHATTACK2_End()
{
	DASHATTACK_End();
}

void Actor::DASHATTACK2_Change()
{
	DASHATTACK_Change();
}

void Actor::DASHATTACK2_Update()
{
	//change this to dash attack 2 hitboxes soon
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		
	}

	AttackMovement();
}

void Actor::DASHATTACK2_UpdateSprite()
{
	int startFrame = 0;
	showSword = true;

	Tileset *curr_ts = ts_dashAttackSword2[speedLevel];

	if (frame >= 13 * 2)
		showSword = false;

	if (showSword)
	{
		swordSprite.setTexture(*curr_ts->texture);
	}
	Vector2f offset = dashAttackSwordOffset[speedLevel];

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame/2, r);

	if (showSword)
	{
		if (r)
		{
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame/2 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame/2 - startFrame);
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}
	}


	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height / 2);
		swordSprite.setRotation(angle / PI * 180);
		//standingNSword1.setPosition( position.x + offset.x, position.y + offset.y );
	}

	SetGroundedSpriteTransform();

	//V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y ) + V2d( offset.x * cos( angle ) + offset.y * sin( angle ), 
	//offset.x * -sin( angle ) +  offset.y * cos( angle ) );
	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);//normalize( ground->v1 - ground->v0 );

	pos += truDir * (double)offset.x;
	pos += -trueNormal * (double)( offset.y - sprite->getLocalBounds().height / 2 );


	swordSprite.setPosition(pos.x, pos.y);
}

void Actor::DASHATTACK2_TransitionToAction(int a)
{

}

void Actor::DASHATTACK2_TimeIndFrameInc()
{

}

void Actor::DASHATTACK2_TimeDepFrameInc()
{

}

int Actor::DASHATTACK2_GetActionLength()
{
	return 14 * 2;
}

Tileset * Actor::DASHATTACK2_GetTileset()
{
	return GetActionTileset("dash_att_02_128x64.png");
}