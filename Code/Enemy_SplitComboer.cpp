#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SplitComboer.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )



SplitPiece::SplitPiece(SplitComboer *splitComb )
	:Enemy( EnemyType::EN_SPLITCOMBOER, false, 1, false)
{
	level = splitComb->level;;
	sc = splitComb;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}

	action = S_FLY;
	position.x = splitComb->position.x;
	position.y = splitComb->position.y;

	spawnRect = sf::Rect<double>(position.x - 16, position.y - 16, 16 * 2, 16 * 2);

	frame = 0;

	Tileset *ts = sc->ts;
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(position.x, position.y);

	comboObj = new ComboObject(this);

	double rad = 48;

	comboObj->enemyHitBody.BasicCircleSetup(rad * scale);

	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;
	comboObj->enemyHitboxFrame = 0;

	dead = false;

	facingRight = true;

	actionLength[S_FLY] = 120;
	actionLength[S_EXPLODE] = 20;

	animFactor[S_FLY] = 1;
	animFactor[S_EXPLODE] = 1;

	hitLimit = 6;


	ResetEnemy();
}

SplitPiece::~SplitPiece()
{
	//delete comboObj;
}

void SplitPiece::ResetEnemy()
{
	sprite.setTextureRect(sc->ts->GetSubRect(0));
	sprite.setRotation(0);
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	velocity = V2d(0, 0);
	SetHitboxes(&hitBody, 0);
	//SetHurtboxes(&hurtBody, 0);
	dead = false;
	action = S_FLY;
	frame = 0;
	position = sc->position;
	receivedHit = NULL;
	currHits = 0;
	UpdateHitboxes();

	UpdateSprite();
}

void SplitPiece::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_FLY:
			action = S_EXPLODE;
			frame = 0;
			break;
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			sess->PlayerRemoveActiveComboer(comboObj);
			break;
		}
	}
}

//void SplitPiece::HandleNoHealth()
//{
//
//}

void SplitPiece::UpdateEnemyPhysics()
{
	if( action == S_FLY )
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		position += movementVec;
	}
}

void SplitPiece::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}
}

void SplitPiece::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);
	/*int tIndex = 0;
	switch (action)
	{
	case S_FLOAT:
	tIndex = 0;
	break;
	case S_SHOT:

	break;
	case S_EXPLODE:
	break;
	}
	sprite.setTextureRect(ts->GetSubRect(frame));*/
}

void SplitPiece::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

void SplitPiece::Shoot(V2d dir)
{
	velocity = dir * sc->shootSpeed;
	sess->PlayerAddActiveComboObj(comboObj);
}

SplitComboer::SplitComboer(Vector2i pos, list<Vector2i> &pathParam,
	bool p_loop,
	int p_level)
	:Enemy(EnemyType::EN_SPLITCOMBOER, false, 1, false)
{
	level = p_level;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}

	action = S_FLOAT;

	loop = p_loop;

	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	pathLength = pathParam.size() + 1;

	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for (list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it)
	{
		path[index] = (*it) + pos;
		++index;
		//path.push_back( (*it) );

	}

	//eventually maybe put this on a multiplier for more variation?
	//doubt ill need it though
	speed = 15;//pspeed;
	frame = 0;

	//ts = owner->GetTileset( "SplitComboer.png", 80, 80 );
	ts = sess->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48, position);
	BasicCircleHitBodySetup(48, position);
	hitBody.hitboxInfo = hitboxInfo;

	
	targetNode = 1;
	forward = true;

	dead = false;

	facingRight = true;

	actionLength[S_FLOAT] = 18;
	actionLength[S_SPLIT] = 3;

	animFactor[S_FLOAT] = 2;
	animFactor[S_SPLIT] = 1;

	shootSpeed = 10;

	pieces[0] = new SplitPiece(this);
	pieces[1] = new SplitPiece(this);

	ResetEnemy();

	
}

SplitComboer::~SplitComboer()
{
	delete pieces[0];
	delete pieces[1];

	delete[] path;
}


void SplitComboer::ResetEnemy()
{
	pieces[0]->Reset();
	pieces[1]->Reset();
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);
	velocity = V2d(0, 0);
	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);
	targetNode = 1;
	forward = true;
	dead = false;
	action = S_FLOAT;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
}

void SplitComboer::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		action = S_SPLIT;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);
		//comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		V2d dir = normalize(receivedHit->hDir);
		if (dir.x != 0 && dir.y != 0)
		{
			double absX = abs(dir.x);
			double absY = abs(dir.y);
			if (absX - absY > -.01)
			{
				dir.y = 0;
			}
			else
			{
				dir.x = 0;
			}
		}

		dir = normalize(dir);

		V2d dir0 = dir;
		V2d dir1 = dir;

		RotateCW(dir0, PI / 5);
		RotateCCW(dir1, PI / 5);

		sess->AddEnemy(pieces[0]);
		sess->AddEnemy(pieces[1]);

		pieces[0]->Shoot(dir0);
		pieces[1]->Shoot(dir1);
	}
}

void SplitComboer::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_SPLIT:
			numHealth = 0;
			dead = true;
			break;
		}
	}
}

void SplitComboer::UpdateEnemyPhysics()
{
	switch (action)
	{
	case S_FLOAT:
	{
		double movement = speed / numPhysSteps;

		if (pathLength > 1)
		{
			movement /= (double)slowMultiple;

			while (movement != 0)
			{
				//cout << "movement loop? "<< endl;
				V2d targetPoint = V2d(path[targetNode].x, path[targetNode].y);
				V2d diff = targetPoint - position;
				double len = length(diff);
				if (len >= abs(movement))
				{
					position += normalize(diff) * movement;
					movement = 0;
				}
				else
				{
					position += diff;
					movement -= length(diff);
					AdvanceTargetNode();
				}
			}
		}
		break;
	}
	}


}

void SplitComboer::AdvanceTargetNode()
{
	if (loop)
	{
		++targetNode;
		if (targetNode == pathLength)
			targetNode = 0;
	}
	else
	{
		if (forward)
		{
			++targetNode;
			if (targetNode == pathLength)
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if (targetNode < 0)
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void SplitComboer::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);
	/*int tIndex = 0;
	switch (action)
	{
	case S_FLOAT:
	tIndex = 0;
	break;
	case S_SHOT:

	break;
	case S_EXPLODE:
	break;
	}
	sprite.setTextureRect(ts->GetSubRect(frame));*/
}

void SplitComboer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

void SplitComboer::CheckedMiniDraw(sf::RenderTarget *target,
	sf::FloatRect &rect)
{
	pieces[0]->CheckedMiniDraw(target, rect);
	pieces[1]->CheckedMiniDraw(target, rect);
	Enemy::CheckedMiniDraw(target, rect);
}

void SplitComboer::SetZoneSpritePosition()
{
	pieces[0]->SetZoneSpritePosition();
	pieces[1]->SetZoneSpritePosition();
	Enemy::SetZoneSpritePosition();
}