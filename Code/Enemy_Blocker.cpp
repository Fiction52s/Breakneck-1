#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Blocker.h"
#include "CircleGroup.h"
#include "EnemyChain.h"

#include "Actor.h"
//testing knockback stuff

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


BlockerChain::BlockerChain(ActorParams *ap)
	:EnemyChain(ap, EN_BLOCKERCHAIN)
{
	if (!blockerShader.loadFromFile("Resources/Shader/blocker_shader.frag", sf::Shader::Fragment))
	{
		cout << "couldnt load blocker shader" << endl;
		assert(false);
	}

	sf::Image paletteImage;
	paletteImage.loadFromFile("Resources/Enemies/blocker_palette_16x9.png");
	
	int skinIndex = 0;
	
	const string &typeName = ap->GetTypeName();
	if (typeName == "blocker")
	{
		blockerType = Blocker::GREY;
		skinIndex = 0;
	}
	if (typeName == "blueblocker")
	{
		blockerType = Blocker::BLUE;
		skinIndex = 1;
	}
	else if (typeName == "greenblocker")
	{
		blockerType = Blocker::GREEN;
		skinIndex = 2;
	}
	else if (typeName == "yellowblocker")
	{
		blockerType = Blocker::YELLOW;
		skinIndex = 3;
	}
	else if (typeName == "orangeblocker")
	{
		blockerType = Blocker::ORANGE;
		skinIndex = 4;
	}
	else if (typeName == "redblocker")
	{
		blockerType = Blocker::RED;
		skinIndex = 5;
	}
	else if (typeName == "magentablocker")
	{
		blockerType = Blocker::MAGENTA;
		skinIndex = 6;
	}
	else if (typeName == "blackblocker")
	{
		blockerType = Blocker::BLACK;
		skinIndex = 8;
	}

	for (int i = 0; i < 16; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(paletteImage.getPixel(i, skinIndex));
	}

	/*GREY,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		BLACK,*/

	/*for (int i = 0; i < 9; ++i)
	{
		paletteArray[i] = sf::Glsl::Vec4(Color::Magenta);
	}*/

	blockerShader.setUniformArray("u_palette", paletteArray, 16);

	
	
	//blockerShader.setUniform("toColor", Glsl::Vec4(Color::White.r, Color::White.g, Color::White.b, Color::White.a));


	BlockerParams *bParams = (BlockerParams*)ap;

	SetLevel(ap->GetLevel());

	SetSpawnRect();

	UpdateParams(ap);
}

void BlockerChain::UpdateStartPosition(int ind, V2d &pos)
{
	((Blocker*)enemies[ind])->SetStartPosition(pos);
}

void BlockerChain::EnemyDraw(sf::RenderTarget *target)
{
	blockerShader.setUniform("u_texture", *ts->texture);
	target->draw(va, numEnemies * 4, sf::Quads, &blockerShader);
}

Tileset *BlockerChain::GetTileset(int variation)
{
	switch (variation)
	{
	case Blocker::GREY:
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;														 
	case Blocker::BLUE:											 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;														 
	case Blocker::GREEN:											 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;														 
	case Blocker::YELLOW:											 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;														 
	case Blocker::ORANGE:											 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;						 
	case Blocker::RED:												 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;														 
	case Blocker::MAGENTA:											 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;														 
	case Blocker::BLACK:											 
		return GetSizedTileset("Enemies/blocker_w1_192x192.png");
		break;
	}
}

Enemy *BlockerChain::CreateEnemy(V2d &pos, int ind)
{
	return new Blocker( this, pos, ind );
}

void BlockerChain::ReadParams(ActorParams *params)
{
	BlockerParams *bParams = (BlockerParams*)params;
	paramsVariation = bParams->bType;
	paramsSpacing = bParams->spacing;
	fill = bParams->fill;
}

Blocker::Blocker( BlockerChain *p_bc, V2d &pos, int index)
	:Enemy( EnemyType::EN_BLOCKER, NULL ),//false, 1, false), 
	bc(p_bc), vaIndex(index)
{
	SetNumActions(Count);
	SetEditorActions(0, WAIT, 0);

	level = bc->level;

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

	minimapCirclePoints = 20;
	
	receivedHit = NULL;

	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	
	hitboxInfo = new HitboxInfo;
	/*hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 10;*/

	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 10;
	hitboxInfo->hitPosType = HitboxInfo::OMNI;
	hitboxInfo->reversableKnockback = false;;

	BasicCircleHurtBodySetup(40);
	BasicCircleHitBodySetup(40);

	hitBody.hitboxInfo = hitboxInfo;

	actionLength[WAIT] = 15;
	actionLength[MALFUNCTION] = 13;
	actionLength[HITTING] = 2;
	actionLength[EXPLODE] = 10;

	animFactor[WAIT] = 5;
	animFactor[HITTING] = 20;
	animFactor[MALFUNCTION] = 4;
	animFactor[EXPLODE] = 2;

	ResetEnemy();

	SetSpawnRect();
}

sf::FloatRect Blocker::GetAABB()
{
	return GetQuadAABB(bc->va + vaIndex * 4);
}

void Blocker::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case WAIT:
			frame = 0;
			break;
		case HITTING:
			frame = 0;
			action = WAIT;
			break;
		case MALFUNCTION:
			action = EXPLODE;
			frame = 0;
			SetHitboxes(NULL, 0);
			break;
		case EXPLODE:
			numHealth = 0;
			dead = true;
			ClearSprite();
			bc->circleGroup->SetVisible(vaIndex, false);
			break;
		}
	}

	switch (action)
	{
	case WAIT:

		break;
	case MALFUNCTION:
		//cout << "mal: " << frame << endl;
		break;
	case EXPLODE:
		break;
	}

	switch (action)
	{
	case WAIT:
		//SetHitboxes(hitBody, 0);
		//SetHurtboxes(hurtBody, 0);
		break;
	case MALFUNCTION:
		//SetHitboxes(hitBody, 0);
		//SetHurtboxes(NULL, 0);
		break;
	case EXPLODE:
		//SetHitboxes(NULL, 0);
		//SetHurtboxes(NULL, 0);
		//SetHitboxes(NULL, 0);
		break;
	}
}

void Blocker::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();

		if (receivedHit->hType == HitboxInfo::COMBO)
		{
			comboHitEnemy->ComboKill(this);
		}

		if (IsFastDying())
		{
			action = EXPLODE;
			frame = 0;
			SetHitboxes(NULL, 0);
			SetHurtboxes(NULL, 0);
		}
		else
		{
			action = MALFUNCTION;
			frame = 0;
			SetHitboxes(&hitBody, 0);
			SetHurtboxes(NULL, 0);
		}
	}
}

void Blocker::SetStartPosition(V2d &pos)
{
	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	Vector2f spriteSize(bc->ts->tileWidth * scale, bc->ts->tileHeight * scale);
	SetRectCenter(bc->va + vaIndex * 4, spriteSize.x, spriteSize.y, Vector2f( pos ) );
}

void Blocker::HandleQuery(QuadTreeCollider * qtc)
{
	if (!dead)
	{
		if (qtc != NULL)
		{
			qtc->HandleEntrant(this);
		}
		
		//bc->checkColArr[vaIndex] = true;
	}
}

bool Blocker::IsTouchingBox(const sf::Rect<double> &r)
{
	return r.intersects(spawnRect);
}

void Blocker::ClearSprite()
{
	Vertex *va = bc->va;
	va[vaIndex * 4 + 0].position = Vector2f(0, 0);
	va[vaIndex * 4 + 1].position = Vector2f(0, 0);
	va[vaIndex * 4 + 2].position = Vector2f(0, 0);
	va[vaIndex * 4 + 3].position = Vector2f(0, 0);
}

void Blocker::ResetEnemy()
{
	Vector2f p = GetPositionF();

	Vector2f spriteSize(bc->ts->tileWidth * scale, bc->ts->tileHeight * scale);
	SetRectCenter(bc->va + vaIndex * 4, spriteSize.x, spriteSize.y, p);

	action = WAIT;
	dead = false;

	//random start frame so they are all desynced
	int r = rand() % (animFactor[WAIT] * actionLength[WAIT]);
	frame = r;

	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);

	//UpdateHitboxes();

	UpdateSprite();
}

void Blocker::IHitPlayer(int index)
{
	V2d playerPos = sess->GetPlayerPos(index);
	if (dot(normalize(playerPos - GetPosition()), hitboxInfo->kbDir) < 0)
	{
		hitboxInfo->kbDir = -hitboxInfo->kbDir;
	}
	if (action == WAIT)
	{
		action = HITTING;
		frame = 0;
	}
}

bool Blocker::IsFastDying()
{
	//if( receivedHit->hType == HitboxInfo::)

	switch (bc->chainEnemyVariation)
	{
	case GREY:
		return true;
	case BLUE:
		if (receivedHit->hType == HitboxInfo::BLUE)
			return true;
		break;
	case GREEN:
		if (receivedHit->hType == HitboxInfo::GREEN)
			return true;
		break;
	case YELLOW:
		if (receivedHit->hType == HitboxInfo::YELLOW)
			return true;
		break;
	case ORANGE:
		if (receivedHit->hType == HitboxInfo::ORANGE)
			return true;
		break;
	case RED:
		if (receivedHit->hType == HitboxInfo::RED)
			return true;
		break;
	case MAGENTA:
		if (receivedHit->hType == HitboxInfo::MAGENTA)
			return true;
		break;
	}

	return false;
}

void Blocker::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	int numEnemies = bc->numEnemies;
	if ( numEnemies > 1)
	{
		

		if (vaIndex == 0 || vaIndex == numEnemies - 1)
		{
			hitboxInfo->hitPosType = HitboxInfo::HitPosType::OMNI;
		}
		else
		{
			int prev = vaIndex - 1;
			int next = vaIndex + 1;


			if (bc->enemies[prev]->dead || bc->enemies[next]->dead )
			{
				hitboxInfo->hitPosType = HitboxInfo::HitPosType::OMNI;
			}
			else
			{
				hitboxInfo->hitPosType = HitboxInfo::HitPosType::AIRFORWARD;

				V2d prevPos = bc->enemies[vaIndex - 1]->GetPosition();
				V2d nextPos = bc->enemies[vaIndex + 1]->GetPosition();

				V2d along = normalize(nextPos - prevPos);
				V2d norm(along.y, -along.x);

				V2d playerPos = sess->GetPlayerPos();
				V2d playerDir = normalize(playerPos - GetPosition());
				if (dot(playerDir, norm) >= 0)
				{
					hitboxInfo->kbDir = norm;
				}
				else
				{
					hitboxInfo->kbDir = -norm;
				}
			}
			//hitboxInfo->kbDir = 
		}
	}
	else
	{
		hitboxInfo->hitPosType = HitboxInfo::HitPosType::OMNI;
	}
	

}

void Blocker::UpdateSprite()
{
	if (dead)
	{
		ClearSprite();
	}
	else
	{
		int f = 0;
		switch (action)
		{
		case WAIT:
			f = frame / animFactor[WAIT];
			break;
		case MALFUNCTION:
			f = frame / animFactor[MALFUNCTION] + 17;
			break;
		case HITTING:
			f = frame / animFactor[HITTING] + 15;
			break;
		case EXPLODE:
			f = frame / animFactor[EXPLODE] + 31;
			break;
		}
		if (f == 44)
		{
			int xx = 5;
		}
		IntRect subRect = bc->ts->GetSubRect(f);
		SetRectSubRectGL(bc->va + vaIndex * 4, subRect, Vector2f(bc->ts->texture->getSize() ));
		//SetRectSubRect(bc->va + vaIndex * 4, subRect);
	}
}

//void Blocker::DrawMinimap(sf::RenderTarget *target)
//{
//}