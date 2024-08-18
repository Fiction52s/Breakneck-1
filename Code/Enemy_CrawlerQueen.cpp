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
#include "BossHealth.h"

using namespace std;
using namespace sf;


CrawlerQueen::CrawlerQueen(ActorParams *ap)
	:Boss(EnemyType::EN_CRAWLERQUEEN, ap),
	crawlerSummonGroup(this,
		new BasicGroundEnemyParams(sess->types["crawler"], 1),
		5, 5, 1),
	bombSummonGroup(this, new ActorParams(sess->types["queenfloatingbomb"]),
		20, 20, 1, true)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(8, 2);

	level = ap->GetLevel();

	scale = 1.5;
	sprite.setScale(scale, scale);

	actionLength[DIG_IN] = 21;
	actionLength[DIG_OUT] = 12;
	actionLength[SLASH] = 26;
	actionLength[UNDERGROUND] = 60;
	actionLength[SUMMON] = 60;
	actionLength[BOOST] = 120;//180;
	actionLength[BOOSTCHARGE] = 11;
	actionLength[CHASE] = 120;//180;
	actionLength[LUNGESTART] = 10;
	actionLength[LUNGE] = 1000;
	actionLength[LUNGELAND] = 14;

	animFactor[COMBOMOVE] = 1;
	animFactor[SLASH] = 4;
	animFactor[DIG_IN] = 4;
	animFactor[DIG_OUT] = 4;
	animFactor[LUNGESTART] = 4;
	animFactor[LUNGE] = 4;
	animFactor[LUNGELAND] = 4;
	animFactor[BOOSTCHARGE] = 3;

	myBonus = NULL;

	//stageMgr.AddActiveOptionToStages(0, DIG_IN, 2);
	stageMgr.AddActiveOption(0, TEST_POST, 2);
	
	/*stageMgr.AddActiveOptionToStages(0, CHASE, 2);

	stageMgr.AddActiveOptionToStages(1, BOOSTCHARGE, 2);

	stageMgr.AddActiveOptionToStages(2, DIG_IN, 2);

	stageMgr.AddActiveOptionToStages(3, SUMMON, 2);*/






	//digDecidePicker.AddActiveOption(DIG_OUT, 2);
	//digDecidePicker.AddActiveOption(LUNGESTART, 2);
	//digDecidePicker.AddActiveOption(SLASH, 2);
	//digDecidePicker.AddActiveOption(GROUND_SHAKE, 2);

	clockwisePicker.AddActiveOption(0, 2);
	clockwisePicker.AddActiveOption(1, 2);

	ts_move = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_dash_320x320.png");
	ts_slash = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_slash_320x320.png");
	ts_dig_in = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_dig_in_320x320.png");
	ts_dig_out = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_dig_out_320x320.png");
	ts_jump = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_jump_320x320.png");
	ts_lunge = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_lunge_320x320.png");
	ts_boostCharge = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_charge_320x320.png");

	postFightScene = NULL;
	postFightScene2 = NULL;

	CreateSurfaceMover(startPosInfo, 30, this);

	SetOffGroundHeight(128);

	lungeSpeed = 20;

	LoadParams();

	//CreateHitboxManager("Enemies/Bosses/Crawler");
	//SetupHitboxes(MOVE, "normal");

	BasicCircleHurtBodySetup(70, 0, V2d( 30, -40 ), V2d());
	BasicCircleHitBodySetup(70, 0, V2d( 30, -40 ), V2d());

	hitBody.hitboxInfo = &hitboxInfos[MOVE];

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

	if (myBonus != NULL)
		delete myBonus;
}

void CrawlerQueen::LoadParams()
{
	ifstream is;
	is.open("Resources/Enemies/Bosses/Crawler/crawlerparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["normal"], hitboxInfos[MOVE]);
	//hitboxInfos[MOVE].hitsThroughInvincibility = false;
	//HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);
}

void CrawlerQueen::AddToGame()
{
	Enemy::AddToGame();

	crawlerSummonGroup.SetEnemyIDAndAddToAllEnemiesVec();
	bombSummonGroup.SetEnemyIDAndAddToAllEnemiesVec();
}

void CrawlerQueen::ResetEnemy()
{
	//currPosInfo = startPosInfo;

	if (myBonus != NULL)
	{
		myBonus->RestartLevel();
	}

	//digDecidePicker.Reset();
	//digDecidePicker.AddActiveOption(SLASH, 2);

	wasAerial = false;
	
	BossReset();

	crawlerSummonGroup.Reset();
	bombSummonGroup.Reset();

	facingRight = true;

	currDashSpeed = 30;//20;
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

	UpdateSprite();
}

void CrawlerQueen::HandleFinishTargetedMovement()
{
	surfaceMover->Set(enemyMover.currPosInfo);
}

void CrawlerQueen::DebugDraw(sf::RenderTarget *target)
{
	surfaceMover->DebugDraw(target);

	if (currHitboxes != NULL)
		currHitboxes->DebugDraw(currHitboxFrame, target);
	if (currHurtboxes != NULL)
		currHurtboxes->DebugDraw(currHurtboxFrame, target);
}

void CrawlerQueen::GoUnderground(int numFrames)
{
	action = UNDERGROUND;
	frame = 0;
	actionLength[UNDERGROUND] = numFrames;
	HitboxesOff();
	HurtboxesOff();
}

void CrawlerQueen::ActionEnded()
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
	case CHASE:
		Decide();
		break;
	case BOOST:
	{
		surfaceMover->SetSpeed(0);
		Decide();
		break;
	}
	case BOOSTCHARGE:
	{
		SetAction(BOOST);
		break;
	}
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
	case LUNGESTART:
	{
		SetAction(LUNGE);
		break;
	}
	case LUNGELAND:
	{
		Decide();
		break;
	}
	}
}

bool CrawlerQueen::GetPlayerClockwise()
{
	Edge *e = surfaceMover->ground;
	V2d along = e->Along();

	double d = dot(targetPlayer->position - GetPosition(), along);

	bool cw = true;
	if (d > 0)
	{
		cw = true;
	}
	else
	{
		cw = false;
	}

	return cw;
}

double CrawlerQueen::GetCurrDashSpeed()
{
	return 30;
	/*switch (stageMgr.GetCurrStage())
	{
	case 0:
		return 30;
	case 1:
		return 35;
	case 2:
		return 40;
	case 3:
		return 45;
	}*/
}

double CrawlerQueen::GetCurrBoostSpeed()
{
	return 60;
	/*switch (stageMgr.GetCurrStage())
	{
	case 0:
		return 60;
	case 1:
		return 70;
	case 2:
		return 80;
	case 3:
		return 90;
	}*/
}

double CrawlerQueen::GetCurrThrowSpeed()
{
	return 3;
	/*switch (stageMgr.GetCurrStage())
	{
	case 0:
		return 6;
	case 1:
		return 10;
	case 2:
		return 18;
	case 3:
		return 25;
	}*/
}

int CrawlerQueen::GetNumSimulationFramesRequired()
{
	return 0;// sess->MAX_SIMULATED_FUTURE_PLAYER_FRAMES;
}

void CrawlerQueen::HandleAction()
{
	/*if (!actionHitPlayer)
	{
		SetHitboxes(hitBodies[action], frame / animFactor[action]);
	}*/

	double cDashSpeed = GetCurrDashSpeed();

	switch (action)
	{
	case MOVE:
	case CHASE:


		if (chaseOver)
		{
			if (frame < actionLength[CHASE] - 30)
			{
				frame = actionLength[CHASE] - 30;
			}
		}

		if (frame % 15 == 0 && frame != 0 )
		{
			bool cw = GetPlayerClockwise();
			facingRight = cw;

			
		}

		

		if (facingRight && surfaceMover->GetGroundSpeed() < cDashSpeed)
		{
			surfaceMover->SetSpeed(surfaceMover->GetGroundSpeed() + currDashAccel);
			if (surfaceMover->GetGroundSpeed() > cDashSpeed)
			{
				surfaceMover->SetSpeed(cDashSpeed);
			}
		}
		else if (!facingRight && surfaceMover->GetGroundSpeed() > -cDashSpeed)
		{
			surfaceMover->SetSpeed(surfaceMover->GetGroundSpeed() - currDashAccel);
			if (surfaceMover->GetGroundSpeed() < -cDashSpeed)
			{
				surfaceMover->SetSpeed(-cDashSpeed);
			}
		}
		break;
	case BOOST:
	{
		break;
	}
	case SUMMON:
		if (frame == 20 && slowCounter == 1)
		{
			crawlerSummonGroup.Summon();
		}
		break;
	case SLASH:
		if (frame == 11 * animFactor[SLASH] && slowCounter == 1)
		{
			V2d norm = surfaceMover->ground->Normal();;
			bombThrowDir = norm;
			bombThrowSpeed = GetCurrThrowSpeed();

			double diff = PI / 6;

			RotateCCW(bombThrowDir, diff * 2 );

			for (int i = 0; i < 5; ++i)
			{
				bombSummonGroup.Summon();
				RotateCW(bombThrowDir, diff);
			}
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
	double cDashSpeed = GetCurrDashSpeed();
	switch (action)
	{
	case MOVE:
	{
		actionLength[MOVE] = 120;
		int cw = clockwisePicker.AlwaysGetNextOption();

		if (cw == 0)
		{
			if (surfaceMover->GetGroundSpeed() == 0)
			{
				surfaceMover->SetSpeed(cDashSpeed);
			}

			facingRight = true;
		}
		else if (cw == 1)
		{
			if (surfaceMover->GetGroundSpeed() == 0)
			{
				surfaceMover->SetSpeed(-cDashSpeed);
			}

			facingRight = false;
		}

		break;
	}
	case CHASE:
	{
		bool cw = GetPlayerClockwise();
		chaseOver = false;
		if (cw)
		{
			if (surfaceMover->GetGroundSpeed() == 0)
			{
				surfaceMover->SetSpeed(cDashSpeed);
			}

			facingRight = true;
		}
		else
		{
			if (surfaceMover->GetGroundSpeed() == 0)
			{
				surfaceMover->SetSpeed(-cDashSpeed);
			}

			facingRight = false;
		}
		break;
	}
	case BOOST:
	{
		double boostSpeed = GetCurrBoostSpeed();

		if (facingRight)
		{
			surfaceMover->SetSpeed(boostSpeed);
		}
		else
		{
			surfaceMover->SetSpeed(-boostSpeed);
		}
		break;
	}
	case BOOSTCHARGE:
	{
		surfaceMover->SetSpeed(0);

		facingRight = GetPlayerClockwise();
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
		DefaultHurtboxesOn();
		DefaultHitboxesOn();
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
		/*if (lungeVel.x > 0)
		{
			facingRight = true;
		}
		else if (lungeVel.x < 0)
		{
			facingRight = false;
		}*/
		break;
	}
	case LUNGESTART:
	{
		DefaultHurtboxesOn();
		DefaultHitboxesOn();

		V2d gn = surfaceMover->ground->Normal();
		if (gn.x > 0)
		{
			facingRight = true;
		}
		else if (gn.x < 0)
		{
			facingRight = false;
		}
		break;
	}
	case TEST_POST:
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			sess->RemoveBoss(this);
			game->SetBonus(myBonus, GetPosition());
		}
		break;
	}
	case DIG_OUT:
	{
		DefaultHurtboxesOn();
		DefaultHitboxesOn();
		break;
	}

	}
}

void CrawlerQueen::SetupPostFightScenes()
{
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		assert(myBonus == NULL);
		myBonus = game->CreateBonus("FinishedScenes/W1/postcrawlerfight1");
	}
	else
	{
		myBonus = NULL;
	}

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
			//postFightScene->queen = this;
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
			//postFightScene2->queen = this;
			postFightScene2->Init();
		}
		
	}
}

void CrawlerQueen::SetupNodeVectors()
{
	nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_CRAWLER, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_CRAWLER, "B"));
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
	//HitboxesOff();
}

void CrawlerQueen::StartFight()
{
	Wait(30);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	//HitboxesOff();
	
}

void CrawlerQueen::RespondToTakingFullHit()
{
	if (action == CHASE)
	{
		chaseOver = true;
	}
	//HitboxesOff();
}

int CrawlerQueen::ChooseActionAfterStageChange()
{
	
	if (stageMgr.currStage == 2)
	{
		digDecidePicker.Reset();
		digDecidePicker.AddActiveOption(DIG_OUT, 2);
		digDecidePicker.AddActiveOption(LUNGESTART, 2);
		digDecidePicker.AddActiveOption(SLASH, 2);
		//digDecidePicker.AddActiveOption(LUNGESTART, 2);
	}
	else if (stageMgr.currStage == 3)
	{
		//digDecidePicker.Reset();
		//digDecidePicker.AddActiveOption(DIG_OUT, 2);
		
		//digDecidePicker.AddActiveOption(SLASH, 2);
	}
	/*else if (stageMgr.currStage == 4)
	{
		digDecidePicker.Reset();
		digDecidePicker.AddActiveOption(DIG_OUT, 2);
		digDecidePicker.AddActiveOption(LUNGESTART, 2);
		
	}*/
	/*else if (stageMgr.currStage == 5)
	{
		digDecidePicker.AddActiveOption(DIG_OUT, 2);
		digDecidePicker.AddActiveOption(LUNGESTART, 2);
	}*/


	return ChooseNextAction();//will add stages to crawler later
}

void CrawlerQueen::ActivatePostFightScene()
{
	sess->RemoveBoss(this);
	if (level == 1)
	{
		postFightScene->Reset();
		sess->SetActiveSequence(postFightScene);
	}
	else if (level == 2)
	{
		postFightScene2->Reset();
		sess->SetActiveSequence(postFightScene2);
	}
}

void CrawlerQueen::SetExtraIDsAndAddToVectors()
{
	if (postFightScene != NULL)
	{
		postFightScene->SetIDs();
	}
	else if (postFightScene2 != NULL)
	{
		postFightScene2->SetIDs();
	}
}

void CrawlerQueen::UpdateSprite()
{
	bool isAerial = enemyMover.currPosInfo.IsAerial();
	if (!isAerial)
	{
		

	}

	int extraHeight = -90;

	/*if (action == BOOSTCHARGE)
	{
		sprite.setColor(Color::Blue);
	}
	else
	{
		sprite.setColor(Color::White);
	}*/

	switch (action)
	{
	case MOVE:
	case CHASE:
	case BOOST:
	{
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case BOOSTCHARGE:
	{
		sprite.setTexture(*ts_boostCharge->texture);
		ts_boostCharge->SetSubRect(sprite, frame/animFactor[BOOSTCHARGE], !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight + 20);
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
		if (frame / animFactor[LUNGE] < 6)
		{
			sprite.setTexture(*ts_lunge->texture);
			ts_slash->SetSubRect(sprite, frame / animFactor[LUNGE] + 4, !facingRight);
		}
		/*V2d vel = surfaceMover->velocity;
		double ang = RadiansToDegrees(GetVectorAngleCCW(normalize(vel)));
		sprite.setTexture(*ts_jump->texture);*/
		//ts_lunge->SetSubRect(sprite, 1, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
		//sprite.setRotation(ang);
		break;
	}
	case LUNGESTART:
	{
		if (frame / animFactor[LUNGESTART] < 6)
		{
			sprite.setTexture(*ts_slash->texture);
			ts_slash->SetSubRect(sprite, frame / animFactor[LUNGESTART], !facingRight);
		}
		else
		{
			sprite.setTexture(*ts_lunge->texture);
			ts_lunge->SetSubRect(sprite, (frame / animFactor[LUNGESTART]) - 6, !facingRight);
		}
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees());
		break;
	}
	case LUNGELAND:
	{
		sprite.setTexture(*ts_lunge->texture);
		ts_lunge->SetSubRect(sprite, frame / animFactor[LUNGELAND] + 10, !facingRight);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, 60);//sprite.getLocalBounds().height + extraHeight);
		sprite.setRotation(surfaceMover->GetAngleDegrees() + 180);
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
	SetAction(LUNGELAND);
	//Decide();
}

void CrawlerQueen::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &crawlerSummonGroup)
	{
		PoiInfo *summonNode;

		//nodeGroupB
		summonNode = nodeGroupA.AlwaysGetNextNode();
		e->startPosInfo.SetGround(summonNode->poly,
			summonNode->edgeIndex, summonNode->edgeQuantity);
	}
	else if (group == &bombSummonGroup)
	{
		QueenFloatingBomb *bomb = (QueenFloatingBomb*)e;

		V2d gn = surfaceMover->ground->Normal();
		bomb->Init(GetPosition() + gn * 80.0, bombThrowDir * bombThrowSpeed);
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