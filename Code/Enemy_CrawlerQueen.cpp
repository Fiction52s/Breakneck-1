#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CrawlerQueen.h"
#include "Actor.h"
#include "SequenceW1.h"
#include "SequenceW4.h"
#include "Enemy_Crawler.h"
#include "Enemy_QueenFloatingBomb.h"
#include <algorithm>

using namespace std;
using namespace sf;


CrawlerQueen::CrawlerQueen(ActorParams *ap)
	:Boss(EnemyType::EN_CRAWLERQUEEN, ap),
	crawlerSummonGroup(this, 
		new BasicGroundEnemyParams(sess->types["crawler"], 1),
		5, 5, 1),
	bombSummonGroup( this, new ActorParams( sess->types["queenfloatingbomb"]),
		10, 10, 1, true ),
	nodeGroupA(Color::Magenta)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	level = ap->GetLevel();

	actionLength[DIG_IN] = 21;
	actionLength[DIG_OUT] = 12;
	actionLength[SLASH] = 26;
	actionLength[UNDERGROUND] = 60;
	actionLength[SUMMON] = 60;

	animFactor[COMBOMOVE] = 1;
	animFactor[SLASH] =4;
	animFactor[DIG_IN] = 4;
	animFactor[DIG_OUT] = 4;

	stageMgr.AddActiveOption(0, MOVE, 2);
	stageMgr.AddActiveOption(0, SUMMON, 2);
	stageMgr.AddActiveOption(0, DIG_IN, 2);

	stageMgr.AddActiveOption(1, MOVE, 2);
	stageMgr.AddActiveOption(1, SUMMON, 2);
	stageMgr.AddActiveOption(1, DIG_IN, 2);

	stageMgr.AddActiveOption(2, MOVE, 2);
	stageMgr.AddActiveOption(2, SUMMON, 2);
	stageMgr.AddActiveOption(2, DIG_IN, 2);

	stageMgr.AddActiveOption(3, MOVE, 2);
	stageMgr.AddActiveOption(3, SUMMON, 2);
	stageMgr.AddActiveOption(3, DIG_IN, 2);

	digDecidePicker.AddActiveOption(DIG_OUT, 2);
	digDecidePicker.AddActiveOption(LUNGE, 2);
	digDecidePicker.AddActiveOption(SLASH, 2);

	clockwisePicker.AddActiveOption(0, 2);
	clockwisePicker.AddActiveOption(1, 2);

	ts_move = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_dash_320x320.png");
	ts_slash = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_slash_320x320.png");
	ts_dig_in = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_dig_in_320x320.png");
	ts_dig_out = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_dig_out_320x320.png");
	ts_jump = sess->GetSizedTileset("Bosses/Crawler/crawler_queen_jump_320x320.png");
	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	postFightScene = NULL;
	postFightScene2 = NULL;

	CreateSurfaceMover(startPosInfo, 30, this);

	SetOffGroundHeight(128);

	lungeSpeed = 20;

	LoadParams();

	BasicCircleHurtBodySetup(70);
	BasicCircleHitBodySetup(70);

	ResetEnemy();
}

CrawlerQueen::~CrawlerQueen()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}

	if (postFightScene2 != NULL)
	{
		delete postFightScene2;
	}
}

void CrawlerQueen::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void CrawlerQueen::ResetEnemy()
{
	currPosInfo = startPosInfo;

	wasAerial = false;

	invincibleFrames = 0;
	
	BossReset();

	crawlerSummonGroup.Reset();
	bombSummonGroup.Reset();

	facingRight = true;

	currDashSpeed = 20;
	currDashAccel = 1.0;

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 1);
		}
	}
	

	StartFight();

	hitPlayer = false;
	comboMoveFrames = 0;

	UpdateSprite();
}

void CrawlerQueen::HandleFinishTargetedMovement()
{
	surfaceMover->Set(enemyMover.currPosInfo);
}

void CrawlerQueen::DebugDraw(sf::RenderTarget *target)
{
	surfaceMover->DebugDraw(target);
}

void CrawlerQueen::GoUnderground(int numFrames)
{
	action = UNDERGROUND;
	frame = 0;
	actionLength[UNDERGROUND] = numFrames;
}

void CrawlerQueen::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case COMBOMOVE:
			frame = 0;
			break;
		case WAIT:
			Decide();
			break;
		case SUMMON:
			Decide();
			break;
		case SLASH:
			GoUnderground(15);
			break;
		case MOVE:
			Decide();
			break;
		case DIG_IN:
			GoUnderground(60);
			break;
		case DIG_OUT:
			Decide();
			break;
		case UNDERGROUND:
		{
			PoiInfo *node = nodeGroupA.AlwaysGetNextNode();
			PositionInfo nodePosInfo;
			nodePosInfo.SetGround(node->poly, node->edgeIndex, node->edgeQuantity);
			surfaceMover->Set(nodePosInfo);

			int digChoice = digDecidePicker.AlwaysGetNextOption();
			SetAction(digChoice);
			break;
		}
		}
	}
}

void CrawlerQueen::HandleAction()
{
	switch (action)
	{
	case MOVE:
		if (facingRight && surfaceMover->groundSpeed < currDashSpeed)
		{
			surfaceMover->groundSpeed += currDashAccel;
			if (surfaceMover->groundSpeed > currDashSpeed)
			{
				surfaceMover->groundSpeed = currDashSpeed;
			}
		}
		else if (!facingRight && surfaceMover->groundSpeed > -currDashSpeed)
		{
			surfaceMover->groundSpeed += -currDashAccel;
			if (surfaceMover->groundSpeed < -currDashSpeed)
			{
				surfaceMover->groundSpeed = -currDashSpeed;
			}
		}
		break;
	case SUMMON:
		if (frame == 20 && slowCounter == 1)
		{
			crawlerSummonGroup.Summon();
		}
		break;
	case SLASH:
		if (frame == 11 * animFactor[SLASH] && slowCounter == 1)
		{
			bombSummonGroup.Summon();
		}
		break;
	case LUNGE:
	{
		if (frame == 0 && slowCounter == 1)
		{
			


		}
	}
	}
}

void CrawlerQueen::StartAction()
{
	switch (action)
	{
	case MOVE:
	{
		actionLength[MOVE] = 120;
		int cw = clockwisePicker.AlwaysGetNextOption();

		if (cw == 0)
		{
			if (surfaceMover->groundSpeed == 0)
			{
				surfaceMover->SetSpeed(currDashSpeed);
			}

			facingRight = true;
		}
		else if (cw == 1)
		{
			if (surfaceMover->groundSpeed == 0)
			{
				surfaceMover->SetSpeed(-currDashSpeed);
			}

			facingRight = false;
		}

		break;
	}
	case WAIT:
	{
		surfaceMover->SetSpeed(0);
		break;
	}
	case SUMMON:
	{
		surfaceMover->SetSpeed(0);
		break;
	}
	case SLASH:
	{
		surfaceMover->SetSpeed(0);
		break;
	}
	case DIG_IN:
	{
		surfaceMover->SetSpeed(0);
		break;
	}
	case LUNGE:
	{
		V2d gn = surfaceMover->ground->Normal();
		V2d lungeVel = gn * lungeSpeed;
		surfaceMover->Jump(lungeVel);
		if (lungeVel.x > 0)
		{
			facingRight = true;
		}
		else if (lungeVel.x < 0)
		{
			facingRight = false;
		}
		break;
	}

	}
}

void CrawlerQueen::SetupPostFightScenes()
{
	if (level == 1)
	{
		if (postFightScene2 != NULL)
		{
			delete postFightScene2;
			postFightScene2 = NULL;
		}

		if (postFightScene == NULL)
		{
			postFightScene = new CrawlerPostFightScene;
			postFightScene->queen = this;
			postFightScene->Init();
		}
	}
	else if (level == 2)
	{

		if (postFightScene != NULL)
		{
			delete postFightScene;
			postFightScene = NULL;
		}

		if (postFightScene2 == NULL)
		{
			postFightScene2 = new CrawlerPostFight2Scene;
			postFightScene2->queen = this;
			postFightScene2->Init();
		}
		
	}
}

void CrawlerQueen::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_CRAWLER, "A"));
}

void CrawlerQueen::StartAngryYelling()
{

}

void CrawlerQueen::StartInitialUnburrow()
{

}

void CrawlerQueen::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void CrawlerQueen::StartFight()
{
	Wait(30);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	HitboxesOff();
}

void CrawlerQueen::Wait(int numFrames)
{
	SetAction(WAIT);
	assert(numFrames > 0);
	actionLength[WAIT] = numFrames;
}

int CrawlerQueen::ChooseActionAfterStageChange()
{
	return ChooseNextAction();//will add stages to crawler later
}


void CrawlerQueen::UpdateSprite()
{
	bool isAerial = enemyMover.currPosInfo.IsAerial();
	if (!isAerial)
	{
		

	}

	int extraHeight = -80;

	switch (action)
	{
	case MOVE:
	{
		sprite.setTexture(*ts_move->texture);
		
		

		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight );
		break;
	}
	case WAIT:
	{
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case SLASH:
	{
		sprite.setTexture(*ts_slash->texture);
		ts_slash->SetSubRect(sprite, frame / animFactor[SLASH], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - 80);
		sprite.setRotation(surfaceMover->GetAngleDegrees());

		V2d gn = surfaceMover->ground->Normal();

		if (gn.y <= 0)
		{
			if (gn.x < 0)
			{
				facingRight = false;
			}
			else if (gn.x > 0)
			{
				facingRight = true;
			}
			else
			{
				//fine either way
			}
		}
		else if (gn.y > 0)
		{
			if (gn.x < 0)
			{
				facingRight = false;
			}
			else if (gn.x > 0)
			{
				facingRight = true;
			}
			else
			{
				//fine either way
			}
		}
		/*else
		{
			if (gn.x < 0)
			{
				facingRight = false;
			}
			else if (gn.x > 0)
			{
				facingRight = true;
			}
		}*/

		break;
	}
	case SUMMON:
	{
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		
		break;
	}
	case DIG_IN:
	{
		sprite.setTexture(*ts_dig_in->texture);
		ts_dig_in->SetSubRect(sprite, frame / animFactor[DIG_IN], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - 80);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case DIG_OUT:
	{
		sprite.setTexture(*ts_dig_out->texture);
		ts_dig_out->SetSubRect(sprite, frame / animFactor[DIG_OUT], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case LUNGE:
	{
		V2d vel = surfaceMover->velocity;
		double ang = RadiansToDegrees(GetVectorAngleCCW(normalize(vel)));
		sprite.setTexture(*ts_jump->texture);
		ts_jump->SetSubRect(sprite, 1, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(ang);
		break;
	}
		
		
	}
	
	

	sprite.setPosition(GetPositionF());

	
	if (invincibleFrames > 0)
	{
		sprite.setColor(Color::Red);
	}
	else
	{
		sprite.setColor(Color::White);
	}

	

	{
		//float angleD = enemyMover.currPosInfo.GetGroundAngleDegrees();
		//if ( !isAerial )
		//{
		//	double distForward = 70;
		//	if (enemyMover.grindSpeed > 0)
		//	{
		//		angleD = enemyMover.CheckGround(distForward).GetGroundAngleDegrees();
		//	}
		//	else if (enemyMover.grindSpeed < 0)
		//	{
		//		angleD = enemyMover.CheckGround(-distForward).GetGroundAngleDegrees();
		//	}
		//	
		//}

		//if ( isAerial )
		//{
		//	sprite.setRotation(0);
		//	wasAerial = true;
		//}
		//else if (wasAerial)
		//{
		//	sprite.setRotation(enemyMover.currPosInfo.GetGroundAngleDegrees());
		//}
		//else
		//{
		//	Edge *e = currPosInfo.GetEdge();
		//	Edge *prevEdge;
		//	Edge *nextEdge;
		//	V2d currAlong = e->Along();
		//	V2d nextAlong;// = currPosInfo.GetEdge()->GetNextEdge()->Along();
		//	V2d prevAlong;
		//	if (enemyMover.grindSpeed > 0)
		//	{
		//		prevEdge = e->GetPrevEdge();
		//		prevAlong = prevEdge->Along();

		//		nextEdge = e->GetNextEdge();
		//		nextAlong = nextEdge->Along();
		//		
		//		double distToNext = e->GetLength() - currPosInfo.GetQuant();
		//		double nextFactor = 0.0;
		//		double test = 64.0;

		//		V2d realAlong;

		//		if (distToNext <= test)
		//		{
		//			nextFactor = 1.0 - distToNext / test;
		//			//realAlong = currAlong * (1.0 - factor) + nextAlong * (1.0 - factor);
		//		}

		//		double distToPrev = currPosInfo.GetQuant();
		//		double prevFactor = 0.0;
		//		if (distToPrev < test)
		//		{
		//			prevFactor = distToPrev / test;
		//		}

		//		if (nextFactor > 0)
		//		{
		//			realAlong = currAlong * (1.0 - nextFactor) + nextAlong * nextFactor;
		//		}
		//		else if (prevFactor > 0)
		//		{
		//			realAlong = currAlong * (1.0 - prevFactor) + prevAlong * prevFactor;
		//		}
		//		else
		//		{
		//			realAlong = currAlong;
		//		}
		//		
		//		
		//		/*if (nextFactor > 0)
		//		{
		//			realAlong = currAlong * (1.0 - nextFactor) + nextAlong * nextFactor;
		//		}
		//		

		//		V2d realAlong = currAlong * factor + nextAlong * ( 1.0 - factor);*/
		//		realAlong = normalize(realAlong);
		//		double realAng = GetVectorAngleCW(realAlong);
		//		double realAngD = realAng / PI * 180.0;

		//		sprite.setRotation(realAngD);

		//	}
		//	/*float rotateVel = 2;
		//	float ang = sprite.getRotation();
		//	if (angleD > ang)
		//	{
		//		ang += rotateVel;
		//		if (ang > angleD)
		//			ang = angleD;
		//	}
		//	else if (angleD < ang)
		//	{
		//		ang += -rotateVel;
		//		if (ang < angleD)
		//			ang = angleD;
		//	}
		//	sprite.setRotation(ang);*/
		//}

		////sprite.setRotation(0);
		///*if (enemyMover.currPosInfo.IsAerial())
		//{
		//	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
		//}
		//else
		//{
		//	
		//}*/
	}
	
}

void CrawlerQueen::EnemyDraw(sf::RenderTarget *target)
{
	if (action != UNDERGROUND)
	{
		DrawSprite(target, sprite);
	}
}

bool CrawlerQueen::IsDecisionValid(int d)
{
	if (d == SUMMON && !crawlerSummonGroup.CanSummon())
	{
		return false;
	}

	return true;
}

void CrawlerQueen::HitTerrainAerial(Edge *e, double quant)
{
	Decide();
}

void CrawlerQueen::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &crawlerSummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeGroupA.AlwaysGetNextNode();
		e->startPosInfo.SetGround(summonNode->poly,
			summonNode->edgeIndex, summonNode->edgeQuantity);
	}
	else if (group == &bombSummonGroup)
	{
		QueenFloatingBomb *bomb = (QueenFloatingBomb*)e;

		V2d gn = surfaceMover->ground->Normal();
		bomb->Init(GetPosition() + gn * 80.0, gn * 2.0);
	}
}

//Rollback
int CrawlerQueen::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void CrawlerQueen::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void CrawlerQueen::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	bytes += sizeof(MyData);
}