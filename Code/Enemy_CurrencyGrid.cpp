#include "Enemy_CurrencyGrid.h"
#include "Enemy.h"
#include "Session.h"
#include "VectorMath.h"
#include <iostream>

using namespace std;
using namespace sf;

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
	//for (int i = 0; i < numEnemies; ++i)
	//{
	//	enemies[i]->AddToWorldTrees(); //currency needed to get added here, blocker doesn't use a custom world tree add so its fine
	//								   //sess->enemyTree->Insert(enemies[i]);
	//								   //sess->activeItemTree->Insert(enemies[i]);
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
	return FloatRect(GetPositionF().x, GetPositionF().y, gridDist.x * gridSize.x, gridDist.y * gridSize.y);
}

CurrencyGrid::CurrencyGrid(ActorParams *ap, EnemyType et, V2d pos, sf::Vector2i p_gridSize, sf::Vector2i p_gridDist, std::vector<bool> & p_hasCurrencyVec )
	:Enemy(et, ap)
{
	va = NULL;
	ts = NULL;

	hasCurrencyVec = p_hasCurrencyVec;

	gridSize = p_gridSize;
	gridDist = p_gridDist;

	ts = GetSizedTileset("Enemies/General/healthfly_64x64.png");

	numCurrencyTotal = gridSize.x * gridSize.y;

	currStates = new int[numCurrencyTotal];
	currFrames = new int[numCurrencyTotal];

	va = new Vertex[numCurrencyTotal * 4];

	SetNumActions(Count);
	SetEditorActions(0, EXIST, 0);

	SetOriginPosition(pos);

	ResetEnemy();

}

CurrencyGrid::~CurrencyGrid()
{
	delete[] currStates;
	delete[] currFrames;
	delete[] va;
}

void CurrencyGrid::ResetEnemy()
{
	for (int i = 0; i < numCurrencyTotal; ++i)
	{
		currStates[i] = EXIST;
		currFrames[i] = 0;
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
		/*if ()
		{
			enemies[i]->UpdatePostPhysics();
			activeNum++;
		}*/
	}

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
		if (hasCurrencyVec[i])
		{
			x = i % gridSize.x;
			y = i / gridSize.x;

			SetRectTopLeft(va + i * 4, 64, 64, GetPositionF() + Vector2f(x * gridDist.x, y * gridDist.y));
			SetRectSubRect(va + i * 4, ts->GetSubRect(0));
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
	return GetPosition() + V2d(x * gridDist.x, y * gridDist.y);
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