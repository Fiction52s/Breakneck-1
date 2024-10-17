#include "Enemy_CurrencyGrid.h"
#include "Enemy.h"
#include "Session.h"
#include "VectorMath.h"
#include <iostream>

using namespace std;
using namespace sf;

//void CurrencyGrid::SetActionEditLoop()
//{
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->SetActionEditLoop();
//	}
//}
//
//void CurrencyGrid::UpdateOnPlacement(ActorParams *ap)
//{
//	Enemy::UpdateOnPlacement(ap);
//	UpdateStartPositions(ap->GetPosition());
//}
//
//void CurrencyGrid::UpdateFromParams(ActorParams *ap, int numFrames)
//{
//	assert(editParams != NULL);
//
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->UpdateFromParams(ap, numFrames);
//	}
//
//	//Enemy::UpdateFromParams(ap, numFrames);
//	UpdateSpriteFromParams(ap);
//}
//
//void CurrencyGrid::AddToWorldTrees()
//{
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->AddToWorldTrees(); //currency needed to get added here, blocker doesn't use a custom world tree add so its fine
//									   //sess->enemyTree->Insert(enemies[i]);
//									   //sess->activeItemTree->Insert(enemies[i]);
//	}
//}
//
//void CurrencyGrid::UpdateStartPositions(V2d &pos)
//{
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		UpdateStartPosition(i, pos + enemyOffsets[i]);
//	}
//}
//
//void CurrencyGrid::UpdateSpriteFromParams(ActorParams *ap)
//{
//	if (ap->posInfo.IsAerial())
//	{
//		UpdateStartPositions(ap->GetPosition());
//	}
//}
//
//void CurrencyGrid::UpdateParams(ActorParams *params)
//{
//	startPosInfo.position = params->GetPosition();
//	SetCurrPosInfo(startPosInfo);
//
//	paramsSpacing = -1;
//	paramsVariation = -1;
//
//	bool oldFill = fill;
//
//	ReadParams(params);
//
//
//
//	assert(paramsSpacing >= 0 && paramsVariation >= 0);
//
//	bool pathChanged = false;
//
//	if (oldFill != fill)
//	{
//		pathChanged = true;
//	}
//
//	if (spacing != paramsSpacing)
//	{
//		pathChanged = true;
//		spacing = paramsSpacing;
//	}
//
//	if (paramsVariation != chainEnemyVariation || ts == NULL)
//	{
//		chainEnemyVariation = paramsVariation;
//
//		ts = GetTileset(chainEnemyVariation);
//	}
//
//	auto &paramsLocalPath = params->GetLocalPath();
//	if (!pathChanged)
//	{
//		int paramsPathSize = paramsLocalPath.size();
//		if (paramsPathSize == localPath.size())
//		{
//			bool diff = false;
//			for (int i = 0; i < paramsPathSize; ++i)
//			{
//				if (paramsLocalPath[i] != localPath[i])
//				{
//					diff = true;
//					break;
//				}
//			}
//
//			if (diff)
//			{
//				pathChanged = true;
//			}
//		}
//		else
//		{
//			pathChanged = true;
//		}
//	}
//
//	if (pathChanged || (enemies == NULL && localPath.size() == 0))
//	{
//		localPath = paramsLocalPath;
//
//		params->MakeGlobalPath(globalPath);
//
//		int oldNumEnemies = numEnemies;
//
//		CreateEnemies();
//
//
//		if (oldNumEnemies != numEnemies)
//		{
//			if (circleGroup != NULL)
//			{
//				delete circleGroup;
//				circleGroup = NULL;
//			}
//			circleGroup = new CircleGroup(numEnemies, 40, Color::Red, 20);
//		}
//
//		for (int i = 0; i < numEnemies; ++i)
//		{
//			circleGroup->SetPosition(i, Vector2f(enemies[i]->GetPosition()));
//		}
//
//		circleGroup->ShowAll();
//	}
//}
//
//void CurrencyGrid::CreateEnemies()
//{
//	double rad = 32;
//	double minDistance = 0;
//	double totalLength = 0;
//	int pathSize = localPath.size();
//	double dist = minDistance + spacing;
//	V2d position = GetPosition();
//
//	int oldNumEnemies = numEnemies;
//
//	if (pathSize == 0)
//	{
//		numEnemies = 1;
//	}
//	else
//	{
//		if (fill)
//		{
//			V2d prev = position;
//			V2d temp;
//			int i = 0;
//			for (auto it = localPath.begin(); it != localPath.end(); ++it)
//			{
//				temp = V2d((*it).x + position.x, (*it).y + position.y);
//				totalLength += length(temp - prev);
//				prev = temp;
//			}
//
//
//			numEnemies = totalLength / dist + 1;
//		}
//		else
//		{
//			numEnemies = localPath.size() + 1;
//		}
//	}
//
//	bool newEnemiesCreated = false;
//	if (numEnemies != oldNumEnemies)
//	{
//		if (enemies != NULL)
//		{
//			for (int i = 0; i < oldNumEnemies; ++i)
//			{
//				delete enemies[i];
//			}
//
//			//delete[] checkColArr;
//
//			delete[] enemies;
//			enemies = NULL;
//
//			delete[] va;
//			va = NULL;
//		}
//
//		//numBlockers = pathParam.size();
//
//		enemies = new Enemy*[numEnemies];
//		//checkColArr = new bool[numEnemies];
//		va = new Vertex[numEnemies * 4];
//
//		enemyOffsets.resize(numEnemies);
//
//		newEnemiesCreated = true;
//	}
//
//	if (numEnemies > 1)
//	{
//		if (fill)
//		{
//			auto currPoint = globalPath.begin();
//			++currPoint;
//			auto nextPoint = currPoint;
//			--currPoint;
//
//			V2d currWalk = position;
//			V2d nextD((*nextPoint).x, (*nextPoint).y);
//			double pathWalk = 0;
//			double travel = 0;
//			double tempLen = 0;
//			bool end = false;
//			//while (pathWalk < totalLength)
//			int ind = 0;
//
//			while (true)
//			{
//				if (ind == numEnemies)
//					break;
//				assert(ind < numEnemies);
//
//				enemyOffsets[ind] = currWalk - position;
//
//
//
//
//				if (newEnemiesCreated)
//				{
//					enemies[ind] = CreateEnemy(currWalk, ind);
//				}
//				else
//				{
//					UpdateStartPosition(ind, currWalk);
//				}
//
//				//cout << enemies[ind]->GetPosition().x << ", " << enemies[ind]->GetPosition().y << endl;
//				travel = dist;
//
//				tempLen = length(nextD - currWalk);
//
//				while (tempLen < travel)
//				{
//					travel -= tempLen;
//					currPoint = nextPoint;
//					currWalk = nextD;
//					nextPoint++;
//					if (nextPoint == globalPath.end())
//					{
//						end = true;
//						break;
//					}
//
//					nextD = V2d((*nextPoint).x, (*nextPoint).y);
//
//					tempLen = length(nextD - currWalk);
//				}
//
//				if (end)
//					break;
//
//				if (travel > 0)
//				{
//					currWalk += travel * normalize(nextD - currWalk);
//				}
//
//				++ind;
//			}
//
//
//			SetKnockbackDirs();
//			//for blocker chains
//			/*V2d prevPos;
//			V2d nextPos;
//			V2d currPos;
//			V2d bisector;
//			V2d dir0;
//			V2d dir1;
//
//			for (int i = 0; i < numEnemies; ++i)
//			{
//			currPos = enemies[i]->GetPosition();
//			if (i > 0)
//			{
//			prevPos = enemies[i - 1]->GetPosition();
//			}
//			if (i < numEnemies - 1)
//			{
//			nextPos = enemies[i + 1]->GetPosition();
//			}
//
//			if (i > 0 && i < numEnemies - 1)
//			{
//			dir0 = normalize(currPos - prevPos);
//			dir0 = V2d(dir0.y, -dir0.x);
//			dir1 = normalize(nextPos - currPos);
//			dir1 = V2d(dir1.y, -dir1.x);
//			bisector = normalize(dir0 + dir1);
//			}
//			else if (i == 0)
//			{
//			dir1 = normalize(nextPos - currPos);
//			dir1 = V2d(dir1.y, -dir1.x);
//			bisector = dir1;
//			}
//			else if (i == numEnemies - 1)
//			{
//			dir0 = normalize(currPos - prevPos);
//			dir0 = V2d(dir0.y, -dir0.x);
//			bisector = dir0;
//			}
//
//			enemies[i]->hitboxInfo->kbDir = bisector;
//			}*/
//		}
//		else
//		{
//			V2d currPos;
//			for (int i = 0; i < numEnemies; ++i)
//			{
//				currPos = V2d(globalPath[i]);
//
//				if (newEnemiesCreated)
//				{
//					enemies[i] = CreateEnemy(currPos, i);
//				}
//				else
//				{
//					UpdateStartPosition(i, currPos);
//				}
//
//				enemyOffsets[i] = currPos - position;
//			}
//
//
//		}
//	}
//	else
//	{
//		enemyOffsets[0] = V2d(0, 0);
//		enemies[0] = CreateEnemy(position, 0);
//
//	}
//	int minX = enemies[0]->spawnRect.left;
//	int maxX = enemies[0]->spawnRect.left + enemies[0]->spawnRect.width;
//	int minY = enemies[0]->spawnRect.top;
//	int maxY = enemies[0]->spawnRect.top + enemies[0]->spawnRect.height;
//
//	for (int i = 1; i < numEnemies; ++i)
//	{
//		minX = min((int)enemies[i]->spawnRect.left, minX);
//		maxX = max((int)enemies[i]->spawnRect.left + (int)enemies[i]->spawnRect.width, maxX);
//		minY = min((int)enemies[i]->spawnRect.top, minY);
//		maxY = max((int)enemies[i]->spawnRect.top + (int)enemies[i]->spawnRect.height, maxY);
//	}
//
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->frame = 0; //for editor
//		enemies[i]->UpdateSprite();
//	}
//}
//
//void CurrencyGrid::UpdateFromPath(ActorParams *ap)
//{
//
//}
//
//FloatRect CurrencyGrid::GetAABB()
//{
//	if (enemies == NULL)
//	{
//		V2d pos = currPosInfo.GetPosition();
//		return FloatRect(pos.x - 100, pos.y - 100, 200, 200);
//	}
//	else
//	{
//		float left, right, top, bottom;
//		left = enemies[0]->GetPositionF().x;
//		right = left;
//		top = enemies[0]->GetPositionF().y;
//		bottom = top;
//
//		for (int i = 1; i < numEnemies; ++i)
//		{
//			Vector2f currPos = enemies[i]->GetPositionF();
//			if (currPos.x < left)
//			{
//				left = currPos.x;
//			}
//			if (currPos.x > right)
//			{
//				right = currPos.x;
//			}
//			if (currPos.y < top)
//			{
//				top = currPos.y;
//			}
//			if (currPos.y > bottom)
//			{
//				bottom = currPos.y;
//			}
//		}
//
//		float extra = 100;
//
//		return FloatRect(left - extra, top - extra,
//			(right - left) + extra * 2,
//			(bottom - top) + extra * 2);
//	}
//}
//
//CurrencyGrid::CurrencyGrid(ActorParams *ap, EnemyType et)
//	:Enemy(et, ap)
//{
//	fill = true;
//	enemies = NULL;
//	va = NULL;
//	circleGroup = NULL;
//	ts = NULL;
//
//	spacing = -1;
//	chainEnemyVariation = -1;
//	SetNumActions(Count);
//	SetEditorActions(0, EXIST, 0);
//	numEnemies = -1;
//
//	usesCustomPhysics = true;
//
//
//	//for minimap
//
//}
//
//CurrencyGrid::~CurrencyGrid()
//{
//	delete circleGroup;
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		delete enemies[i];
//	}
//	//delete[] checkColArr;
//	delete[] enemies;
//	delete[] va;
//}
//
//void CurrencyGrid::AddToGame()
//{
//	Enemy::AddToGame();
//
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->SetEnemyIDAndAddToAllEnemiesVec();
//	}
//}
//
//void CurrencyGrid::ResetEnemy()
//{
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->Reset();
//	}
//	circleGroup->ShowAll();
//}
//
//void CurrencyGrid::SetZone(Zone *p_zone)
//{
//	zone = p_zone;
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->SetZone(p_zone);
//	}
//}
//
//void CurrencyGrid::ResetCheckCollisions()
//{
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		//checkColArr[i] = false;
//	}
//}
//
//void CurrencyGrid::UpdateCustomPhysics(int substep)
//{
//	//int updateCount = 0;
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		//if (checkColArr[i])
//		{
//			enemies[i]->UpdatePhysics(substep);
//			//++updateCount;
//		}
//
//	}
//
//	//cout << "num updating: " << updateCount << endl;
//}
//
//void CurrencyGrid::UpdatePrePhysics()
//{
//	//ResetCheckCollisions();
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->UpdatePrePhysics();
//		if (enemies[i]->dead)
//		{
//			circleGroup->SetVisible(i, false);
//		}
//		else
//		{
//			circleGroup->SetVisible(i, true);
//		}
//	}
//
//
//}
//
//void CurrencyGrid::ProcessState()
//{
//	checkCol = false;
//}
//
//void CurrencyGrid::UpdateEnemyPhysics()
//{
//	if (checkCol)
//	{
//		for (int i = 0; i < numEnemies; ++i)
//		{
//			enemies[i]->UpdateEnemyPhysics();
//		}
//	}
//}
//
//void CurrencyGrid::UpdatePostPhysics()
//{
//	int activeNum = 0;
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		if (!enemies[i]->dead)
//		{
//			enemies[i]->UpdatePostPhysics();
//			activeNum++;
//		}
//	}
//
//	if (activeNum == 0)
//	{
//		sess->RemoveEnemy(this);
//	}
//}
//
//void CurrencyGrid::EnemyDraw(sf::RenderTarget *target)
//{
//	target->draw(va, numEnemies * 4, sf::Quads, ts->texture);
//}
//
//void CurrencyGrid::DrawMinimap(sf::RenderTarget *target)
//{
//	circleGroup->Draw(target);
//}
//
//void CurrencyGrid::DebugDraw(RenderTarget *target)
//{
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->DebugDraw(target);
//	}
//}
//
//int CurrencyGrid::GetNumCamPoints()
//{
//	return numEnemies;
//}
//
//V2d CurrencyGrid::GetCamPoint(int index)
//{
//	return enemies[index]->GetPosition();
//}
//
//int CurrencyGrid::GetNumStoredBytes()
//{
//	return sizeof(MyData) + enemies[0]->GetNumStoredBytes() * numEnemies;
//}
//
//void CurrencyGrid::StoreBytes(unsigned char *bytes)
//{
//	StoreBasicEnemyData(data);
//	memcpy(bytes, &data, sizeof(MyData));
//	bytes += sizeof(MyData);
//
//	cout << "storing enemy chain bytes" << endl;
//	//data.Print();
//
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->StoreBytes(bytes);
//		bytes += enemies[i]->GetNumStoredBytes();
//	}
//}
//
//void CurrencyGrid::SetFromBytes(unsigned char *bytes)
//{
//	memcpy(&data, bytes, sizeof(MyData));
//	SetBasicEnemyData(data);
//	bytes += sizeof(MyData);
//
//	for (int i = 0; i < numEnemies; ++i)
//	{
//		enemies[i]->SetFromBytes(bytes);
//		bytes += enemies[i]->GetNumStoredBytes();
//	}
//}