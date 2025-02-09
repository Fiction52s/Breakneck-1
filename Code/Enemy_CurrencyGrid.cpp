#include "Enemy_CurrencyGrid.h"
#include "Enemy.h"
#include "Session.h"
#include "VectorMath.h"
#include <iostream>
#include "Actor.h"
#include "Enemy_CurrencyItem.h"

using namespace std;
using namespace sf;

CurrencyGrid::GridCurrencyInfo::GridCurrencyInfo()
{
	exists = false;
	state = 0;
	frame = 0;
	index = 0;
	playerChasingIndex = -1;
	myGrid = NULL;

	
}
void CurrencyGrid::GridCurrencyInfo::HandleQuery(QuadTreeCollider * qtc)
{
	if (exists && state == CurrencyGrid::NEUTRAL )
	{
		return qtc->HandleEntrant(this);
	}
}
bool CurrencyGrid::GridCurrencyInfo::IsTouchingBox(const sf::Rect<double> &r)
{
	return IsBoxTouchingBox(r, myRect);
}

void CurrencyGrid::GridCurrencyInfo::Reset()
{
	state = CurrencyGrid::NEUTRAL;
	frame = 0;
	playerChasingIndex = -1;
}

void CurrencyGrid::GridCurrencyInfo::ProcessState()
{
	if (frame == myGrid->actionLength[state] * myGrid->animFactor[state])
	{
		switch (state)
		{
		case NEUTRAL:
		{
			frame = 0;
			break;
		}
		case PLAYER_COLLECT:
		{
			state = INACTIVE;
			frame = 0;
			//numHealth = 0;
			//ClearSprite();
			//dead = true;

			//action = DEATH;
			//frame = 0;
			break;
		}
		}
	}

	if (state == PLAYER_COLLECT)
	{
		double df = frame + 1;
		double f = df / myGrid->actionLength[PLAYER_COLLECT];
		center = V2d(myGrid->GetCurrencyStartPosition(index) * (1.0 - f) + (myGrid->sess->GetPlayer(playerChasingIndex)->position - myGrid->GetPosition()) * f);
	}
}

void CurrencyGrid::GridCurrencyInfo::UpdateSprite()
{
	if (state == CurrencyGrid::NEUTRAL || state == CurrencyGrid::PLAYER_COLLECT )
	{
		int tile = 0;
		if (myGrid->currencyType == CurrencyItem::CURRENCY_NORMAL)
		{
			tile = 0;
		}
		else if (myGrid->currencyType == CurrencyItem::CURRENCY_BIG)
		{
			tile = 1;
		}

		SetRectSubRect(myGrid->va + index * 4, myGrid->ts->GetSubRect(tile));
		Vector2f fOrigin = myGrid->GetPositionF();
		SetRectCenter(myGrid->va + index * 4, myGrid->ts->tileWidth, myGrid->ts->tileHeight, Vector2f(center.x + fOrigin.x, center.y + fOrigin.y));
	}
	else
	{
		ClearRect(myGrid->va + index * 4);
	}
}

void CurrencyGrid::GridCurrencyInfo::Collect( int pIndex )
{
	state = CurrencyGrid::PLAYER_COLLECT;
	frame = 0;
	playerChasingIndex = pIndex;
}


void CurrencyGrid::SetActionEditLoop()
{
	/*for (int i = 0; i < numEnemies; ++i)
	{
		enemies[i]->SetActionEditLoop();
	}*/
}

void CurrencyGrid::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);
	UpdateStartPositions(ap->GetPosition());
}

void CurrencyGrid::UpdateFromParams(ActorParams *ap, int numFrames)
{
	assert(editParams != NULL);

	/*for (int i = 0; i < numEnemies; ++i)
	{
		enemies[i]->UpdateFromParams(ap, numFrames);
	}*/

	//Enemy::UpdateFromParams(ap, numFrames);
	UpdateSpriteFromParams(ap);
}

void CurrencyGrid::SetOriginPosition(V2d pos)
{
	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	UpdateSprite();
}

void CurrencyGrid::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
	//for (int i = 0; i < numEnemies; ++i)
	//{
	//	enemies[i]->AddToWorldTrees(); //currency needed to get added here, blocker doesn't use a custom world tree add so its fine
	//								   //sess->enemyTree->Insert(enemies[i]);
	//								   
	//}
}

void CurrencyGrid::UpdateStartPositions(V2d &pos)
{
	/*for (int i = 0; i < numEnemies; ++i)
	{
		UpdateStartPosition(i, pos + enemyOffsets[i]);
	}*/
}

void CurrencyGrid::UpdateSpriteFromParams(ActorParams *ap)
{
	if (ap->posInfo.IsAerial())
	{
		UpdateStartPositions(ap->GetPosition());
	}
}

void CurrencyGrid::UpdateParams(ActorParams *params)
{
}

void CurrencyGrid::CreateEnemies()
{
}

void CurrencyGrid::UpdateFromPath(ActorParams *ap)
{

}

FloatRect CurrencyGrid::GetAABB()
{
	Vector2f posf = GetPositionF();
	FloatRect fr(posf.x - currencyRad, posf.y - currencyRad, gridDistance * gridSize.x + currencyRad * 2, gridDistance * gridSize.y + currencyRad * 2);
	return fr;
}

V2d CurrencyGrid::GetCurrencyStartPosition(int index)
{
	int x, y;

	x = index % gridSize.x;
	y = index / gridSize.x;
	return V2d(x * gridDistance + gridDistance / 2, y * gridDistance + gridDistance / 2);
}

CurrencyGrid::CurrencyGrid(ActorParams *ap, EnemyType et, V2d pos, sf::Vector2i p_gridSize, std::vector<bool> & p_hasCurrencyVec, int p_currencyType )
	:Enemy(et, ap)
{
	SetNumActions(Count);

	actionLength[NEUTRAL] = 5;
	actionLength[INACTIVE] = 1;
	actionLength[PLAYER_COLLECT] = 20;

	animFactor[NEUTRAL] = 5;
	animFactor[INACTIVE] = 1;
	animFactor[PLAYER_COLLECT] = 1;

	va = NULL;
	ts = NULL;

	currencyType = p_currencyType;

	switch (currencyType)
	{
	case CurrencyItem::CURRENCY_NORMAL:
	{
		currencyRad = 128;
		givenHealAmount = 20;
		givenCurrencyAmount = 1;
		
		break;
	}
	case CurrencyItem::CURRENCY_BIG:
	{
		currencyRad = 128;
		givenHealAmount = 20;
		givenCurrencyAmount = 5;
		break;
	}
	}

	gridDistance = CurrencyItem::GetSpacing(currencyType);

	currencyTree = new QuadTree(1000000, 1000000);

	

	gridSize = p_gridSize;

	numCurrencyTotal = gridSize.x * gridSize.y;

	//currency vec is the same size as numCurrencyTotal
	assert(p_hasCurrencyVec.size() == numCurrencyTotal);

	currencyInfoVec.resize(numCurrencyTotal);

	int x, y;
	V2d center;
	for (int i = 0; i < p_hasCurrencyVec.size(); ++i)
	{
		currencyInfoVec[i].exists = p_hasCurrencyVec[i];
		currencyInfoVec[i].myGrid = this;

		x = i % gridSize.x;
		y = i / gridSize.x;

		center = GetCurrencyStartPosition( i );
		currencyInfoVec[i].myRect = sf::Rect<double>(center.x - currencyRad, center.y - currencyRad, currencyRad * 2, currencyRad * 2);
		currencyInfoVec[i].center = center;
		currencyInfoVec[i].index = i;
		currencyTree->Insert(&currencyInfoVec[i]);
	}

	

	//ts = GetSizedTileset("Enemies/General/currency_64x64.png");
	ts = GetSizedTileset("Enemies/General/currency_test_128x128.png");

	va = new Vertex[numCurrencyTotal * 4];

	testBody.ResetFrames(); //in case it happens more than once
	testBody.BasicCircleSetup(currencyRad * scale, 0, V2d(0,0));

	SetNumActions(Count);
	SetEditorActions(0, NEUTRAL, 0);

	SetOriginPosition(pos);

	SetSpawnRect();

	ResetEnemy();

}

CurrencyGrid::~CurrencyGrid()
{
	delete[] va;
	delete currencyTree;
}

void CurrencyGrid::CheckCollection(Actor *a)
{
	currCheckingCollectionActor = a;
	sf::Rect<double> queryRect = a->currQueryRect;
	queryRect.top -= GetPosition().y;
	queryRect.left -= GetPosition().x;
	currencyTree->Query(this, queryRect );
	//cout << "check collection" << endl;
	//Vector2i aGridPos = 
	//for (int i = 0; i < numCurrencyTotal; ++i)
	//{
	//	if( currencyInfoVec[i].exists)geta
	//}
	//testBody.SetBasicPos(pos, 0);
}

void CurrencyGrid::HandleEntrant(QuadTreeEntrant *qte)
{
	GridCurrencyInfo *gci = (GridCurrencyInfo*)qte;
	
	testBody.SetBasicPos(gci->center + GetPosition());

	

	if (testBody.Intersects(0, &currCheckingCollectionActor->hurtBody) )
	{
		gci->Collect(currCheckingCollectionActor->actorIndex );
		currCheckingCollectionActor->CollectCurrency(givenCurrencyAmount, givenHealAmount);
		//player still needs to collect, but we're getting there
	}
}

void CurrencyGrid::ResetEnemy()
{
	int x, y;
	for (int i = 0; i < numCurrencyTotal; ++i)
	{
		currencyInfoVec[i].Reset();
		x = i % gridSize.x;
		y = i / gridSize.x;
		currencyInfoVec[i].center = V2d(x * gridDistance + gridDistance / 2, y * gridDistance + gridDistance / 2);
	}

	UpdateSprite();
}



void CurrencyGrid::ResetCheckCollisions()
{
}

void CurrencyGrid::UpdatePrePhysics()
{
	if (IsSummoning())
	{
		return;
	}

	/*receivedHit.SetEmpty();
	pauseBeganThisFrame = false;

	if (pauseFrames > 0)
	{
		--pauseFrames;
		if (pauseFrames > 0 && !ignorePauseFrames)
			return;
		else
		{
			if (numHealth <= 0)
			{
				Die();
			}
		}
	}

	if (dead)
		return;

	CheckSpecters();*/

	ProcessState();

	numPhysSteps = NUM_MAX_STEPS;
}

void CurrencyGrid::ProcessState()
{
	checkCol = false;

	for (int i = 0; i < numCurrencyTotal; ++i)
	{
		currencyInfoVec[i].ProcessState();
	}
}

void CurrencyGrid::UpdateEnemyPhysics()
{
	if (checkCol)
	{
		
	}
}

void CurrencyGrid::UpdatePostPhysics()
{
	int activeNum = 0;
	for (int i = 0; i < numCurrencyTotal; ++i)
	{
		if (!currencyInfoVec[i].exists)
		{
			continue;
		}

		if (currencyInfoVec[i].state != INACTIVE)
		{
			++activeNum;
		}

		currencyInfoVec[i].frame++;
		/*if ()
		{
			enemies[i]->UpdatePostPhysics();
			activeNum++;
		}*/
	}

	UpdateSprite();

	if (activeNum == 0)
	{
		sess->RemoveEnemy(this);
	}
}

void CurrencyGrid::UpdateSprite()
{
	Vector2f p;
	int x, y;
	for (int i = 0; i < numCurrencyTotal; ++i)
	{
		if (currencyInfoVec[i].exists)
		{
			currencyInfoVec[i].UpdateSprite();
		}
		else
		{
			ClearRect(va + i * 4);
		}	
	}
}

void CurrencyGrid::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(va, numCurrencyTotal * 4, sf::Quads, ts->texture);
}

void CurrencyGrid::DrawMinimap(sf::RenderTarget *target)
{
	//circleGroup->Draw(target);
}

void CurrencyGrid::DebugDraw(RenderTarget *target)
{
	/*for (int i = 0; i < numEnemies; ++i)
	{
		enemies[i]->DebugDraw(target);
	}*/
}

int CurrencyGrid::GetNumCamPoints()
{
	return numCurrencyTotal;
}

V2d CurrencyGrid::GetCamPoint(int index)
{
	int x = index % gridSize.x;
	int y = index / gridSize.x;
	return GetPosition() + currencyInfoVec[index].center;
}

int CurrencyGrid::GetNumStoredBytes()
{
	return sizeof(MyData);// +enemies[0]->GetNumStoredBytes() * numEnemies;
}

void CurrencyGrid::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	cout << "storing enemy grid bytes" << endl;
	//data.Print();

	/*for (int i = 0; i < numEnemies; ++i)
	{
		enemies[i]->StoreBytes(bytes);
		bytes += enemies[i]->GetNumStoredBytes();
	}*/
}

void CurrencyGrid::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	/*for (int i = 0; i < numEnemies; ++i)
	{
		enemies[i]->SetFromBytes(bytes);
		bytes += enemies[i]->GetNumStoredBytes();
	}*/
}