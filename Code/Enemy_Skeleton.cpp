#include "Session.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Skeleton.h"
#include "Actor.h"
#include "SequenceW6.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


Skeleton::Skeleton(ActorParams *ap)
	:Boss(EnemyType::EN_SKELETONBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	StageSetup(4, 4);

	level = ap->GetLevel();

	//ts_move = GetSizedTileset("Bosses/Gator/dominance_384x384.png");

	ts_charge = GetSizedTileset("Bosses/Skeleton/skele_charge_128x128.png");
	ts_stand = GetSizedTileset("Bosses/Skeleton/skele_128x128.png");
	ts_hop = GetSizedTileset("Bosses/Skeleton/skele_hop_128x128.png");
	ts_laser = GetSizedTileset("Bosses/Skeleton/skele_laser_160x128.png");

	//stageMgr.AddActiveOption(0, SHOOT_LASER, 2);
	//stageMgr.AddActiveOption(0, GATHER_ENERGY_START, 2);
	
	//stageMgr.AddActiveOption(0, MOVE_WIRE_DASH, 2);
	stageMgr.AddActiveOption(0, PLAN_PATTERN, 2);
	//stageMgr.AddActiveOption(0, CHASE_MOVE, 2);

	//stageMgr.AddActiveOption(0, MOVE_OTHER, 2);

	maxChargeLoopFrames = 60;

	extraHeight = 32;
	eyeExtraHeight = 32;

	stageMgr.AddActiveOption(1, MOVE_WIRE_DASH, 2);

	stageMgr.AddActiveOption(2, MOVE_WIRE_DASH, 2);

	stageMgr.AddActiveOption(3, MOVE_WIRE_DASH, 2);

	actionLength[JUMPSQUAT] = 2;
	//actionLength[HOP] = 2;
	actionLength[LAND] = 2;

	actionLength[SHOOT_LASER_HOMING] = 21;
	animFactor[SHOOT_LASER_HOMING] = 3;

	actionLength[SHOOT_LASER] = 21;
	animFactor[SHOOT_LASER] = 3;

	actionLength[DASH_RECOVER] = 30;
	animFactor[DASH_RECOVER] = 1;

	actionLength[REDIRECT_TEST] = 61;
	animFactor[REDIRECT_TEST] = 1;

	actionLength[LASER_SPAM] = 60;
	animFactor[LASER_SPAM] = 1;

	animFactor[JUMPSQUAT] = 3;//1
	//animFactor[HOP] = 1;
	animFactor[LAND] = 3;

	actionLength[GATHER_ENERGY_START] = 10;
	animFactor[GATHER_ENERGY_START] = 1;

	actionLength[GATHER_ENERGY_LOOP] = 5;
	animFactor[GATHER_ENERGY_LOOP] = 4;

	actionLength[GATHER_ENERGY_END] = 10;
	animFactor[GATHER_ENERGY_END] = 1;


	SetRectColor(wireQuad, Color::Red);
	

	postFightScene = NULL;

	//sprite.setColor(Color::Black);

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	patternFlickerFrames = 30;
	
	

	patternTypePicker.Reset();
	patternTypePicker.AddActiveOption(PATTERN_MOVE);
	patternTypePicker.AddActiveOption(LASER_SPAM);
	patternTypePicker.AddActiveOption(SHOOT_LASER);
	patternTypePicker.AddActiveOption(SHOOT_LASER_HOMING);
	patternTypePicker.AddActiveOption(REDIRECT_TEST);

	pattern.reserve(9);
	patternType.reserve(9);
	patternOrder.reserve(9);

	SetPatternLength(3);

	patternPreview.setFillColor(Color::Magenta);
	patternPreview.setRadius(60);
	patternPreview.setOrigin(patternPreview.getLocalBounds().width / 2,
		patternPreview.getLocalBounds().height / 2);

	patternNumberText.setFont(sess->mainMenu->arial);
	patternNumberText.setCharacterSize(90);
	patternNumberText.setFillColor(Color::Black);

	ResetEnemy();
}

Skeleton::~Skeleton()
{
	if (postFightScene != NULL)
		delete postFightScene;
}

void Skeleton::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Skeleton::ResetEnemy()
{
	currNode = NULL;
	patternIndex = -1;

	laserPool.Reset();

	facingRight = true;

	rayCastInfo.tree = sess->terrainTree;

	currPosInfo.SetAerial();
	currPosInfo.position += V2d(0, -extraHeight);
	enemyMover.currPosInfo = currPosInfo;

	if (sess->preLevelScene == NULL) //fight testing
	{
		CameraShot *cs = sess->cameraShotMap["fightcam"];
		if (cs != NULL)
		{
			sess->cam.Set(Vector2f(cs->centerPos), cs->zoom, 0);
		}
	}

	BossReset();

	StartFight();

	UpdateSprite();
}

bool Skeleton::TryComboMove(V2d &comboPos, int comboMoveDuration,
	int moveDurationBeforeStartNextAction,
	V2d &comboOffset)
{
	return false;
}

int Skeleton::ChooseActionAfterStageChange()
{
	return Boss::ChooseActionAfterStageChange();
}

void Skeleton::ActivatePostFightScene()
{
	postFightScene->Reset();
	sess->SetActiveSequence(postFightScene);
}

void Skeleton::ActionEnded()
{
	switch (action)
	{
	case WAIT:
	case DASH_RECOVER:
	case MOVE_OTHER:
		Decide();
		break;
	case MOVE_WIRE_DASH:
	{
		SetAction(DASH_RECOVER);
		break;
	}
	case CHASE_MOVE:
	{
		SetAction(CHASE_MOVE);
		break;
	}
	case PLAN_PATTERN:
	{
		patternIndex = 0;

		SetAction(PATTERN_MOVE);
		break;
	}
	case PATTERN_MOVE:
	{
		cout << "finish move" << endl;
		FinishPatternMove();

		break;
	}
	case JUMPSQUAT:
	{
		SetAction(HOP);
		int hopFrames = enemyMover.SetModeNodeProjectile(hopTarget, V2d(0, 2), 100);
			//enemyMover.SetModeNodeJump(hopTarget, hopExtraHeight, hopSpeed);
		animFactor[HOP] = hopFrames / 2;
		//used a different algorithm in sequenceskeleton, maybe change this
		//out at some point
		break;
	}
	case HOP:
	{
		SetAction(LAND);
		break;
	}
	case LAND:
	{
		FinishPatternMove();
		//Decide();
		break;
	}
	case REDIRECT_TEST:
	{
		if (patternIndex == numPatternMoves)
		{
			Wait(30);
		}
		else
		{
			SetAction(PATTERN_MOVE);
		}

		break;
	}
	case SHOOT_LASER_HOMING:
	case SHOOT_LASER:
	{
		if (patternIndex == numPatternMoves)
		{
			Wait(30);
		}
		else
		{
			SetAction(PATTERN_MOVE);
		}
		//FinishPatternMove();
		//Decide(); //for now
		break;
	}
	case LASER_SPAM:
	{
		if (patternIndex == numPatternMoves)
		{
			Wait(30);
		}
		else
		{
			SetAction(PATTERN_MOVE);
		}
		break;
	}
	case GATHER_ENERGY_START:
	{
		SetAction(GATHER_ENERGY_LOOP);
		currChargeLoopFrame = 0;
		break;
	}
	case GATHER_ENERGY_LOOP:
	{
		frame = 0;
		break;
	}
	case GATHER_ENERGY_END:
	{
		Decide();

		break;
	}
		
	}
}

void Skeleton::HandleAction()
{
	switch (action)
	{
	case PLAN_PATTERN:
	{
		if (frame % patternFlickerFrames == 0)
		{
			int mult = frame / patternFlickerFrames;
			if (mult < numPatternMoves)
			{
				Vector2f pos(pattern[mult]->pos);
				//currBabyScorpPos = pattern[mult]->pos;
				patternPreview.setPosition( pos );
				patternNumberText.setPosition(pos);
				patternNumberText.setString(to_string(patternOrder[mult]+1));
				patternNumberText.setOrigin(patternNumberText.getLocalBounds().width / 2, 
					patternNumberText.getLocalBounds().height / 2);
				//patternNumber.setString( )
				//patternNumber.setPosition(patternPreview.getPosition());

				if (patternType[patternOrder[mult]] == PATTERN_MOVE)
				{
					patternPreview.setFillColor(Color::Magenta);
				}
				else if (patternType[patternOrder[mult]] == SHOOT_LASER)
				{
					patternPreview.setFillColor(Color::Yellow);
				}
				else if (patternType[patternOrder[mult]] == REDIRECT_TEST)
				{
					patternPreview.setFillColor(Color::Cyan);
				}
				else if (patternType[patternOrder[mult]] == LASER_SPAM)
				{
					patternPreview.setFillColor(Color::Red);
				}
				else if (patternType[patternOrder[mult]] == SHOOT_LASER_HOMING)
				{
					patternPreview.setFillColor(Color::Green);
				}
			}
		}
		break;
	}
	case SHOOT_LASER_HOMING:
	case SHOOT_LASER:
	{
		if (frame == 9 * animFactor[action] && slowCounter == 1)
		{
			V2d offset = V2d(0, -extraHeight);
			if (facingRight)
			{
				offset += V2d(20, 16);
			}
			else
			{
				offset += V2d(-20, 16);
			}
			V2d shootDir = PlayerDir(offset, V2d());
			double angChange = .1 * PI;

			
			int shootType = 0;

			if (action == SHOOT_LASER_HOMING)
			{
				shootType = 2;
			}
			laserPool.Throw(shootType, GetPosition() + offset,
				shootDir);
			/*laserPool.Throw(shootType, GetPosition() + offset,
				shootDir );
			RotateCCW(shootDir, angChange);
			laserPool.Throw(shootType, GetPosition() + offset,
				shootDir);
			RotateCW(shootDir, angChange * 2);
			laserPool.Throw(shootType, GetPosition() + offset,
				shootDir);*/
		}
		
		break;
	}
	case REDIRECT_TEST:
	{
		if (frame == 60 && slowCounter == 1)
		{
			laserPool.SetAllSpeed(50);
			laserPool.RedirectAllTowards(sess->GetPlayerPos(0));
		}
		/*else if (frame == 150 && slowCounter == 1)
		{
			laserPool.SetAllSpeed(40);
			laserPool.RedirectAllTowards(sess->GetPlayerPos(0));
		}*/
		break;
	}
	case LASER_SPAM:
	{
		if (slowCounter == 1)
		{
			int division = 10;
			if (frame % 5 == 0 )//frame % division == 0 && frame / division < 3)
			{
				//cout << "active: " << laserPool.getnu
				V2d offset = V2d(0, -extraHeight);
				if (facingRight)
				{
					offset += V2d(20, 16);
				}
				else
				{
					offset += V2d(-20, 16);
				}
				V2d shootDir = PlayerDir(offset, V2d());//V2d(0, -1);//
				int numLasers = 20;
				double angChange = (1.0 * PI) / numLasers;//.1 * PI;

				int spamCounter = frame / division;

				//RotateCCW(shootDir, (angChange / 2) * spamCounter );
				int shootType = 3;
				laserPool.Throw(shootType, GetPosition() + offset,
					shootDir);

				/*
				for (int i = 0; i < numLasers; ++i)
				{
					
					RotateCCW(shootDir, angChange);
				}*/
			}
		}
		break;
	}
	case PATTERN_MOVE:
	{
		if (frame % 10 == 0)//frame % division == 0 && frame / division < 3)
		{
			//cout << "active: " << laserPool.getnu
			V2d offset = V2d(0, -extraHeight);
			if (facingRight)
			{
				offset += V2d(20, 16);
			}
			else
			{
				offset += V2d(-20, 16);
			}
			offset = V2d(0, 0);
			V2d shootDir = PlayerDir(offset, V2d());//V2d(0, -1);//
			int numLasers = 20;
			double angChange = (1.0 * PI) / numLasers;//.1 * PI;

			//int spamCounter = frame / division;

			//RotateCCW(shootDir, (angChange / 2) * spamCounter );
			int shootType = 3;
			//laserPool.Throw(shootType, GetPosition() + offset,
			//	shootDir);

			/*
			for (int i = 0; i < numLasers; ++i)
			{

			RotateCCW(shootDir, angChange);
			}*/
		}
		break;
	}
	}
}

void Skeleton::StartAction()
{
	switch (action)
	{
	case MOVE_WIRE_DASH:
	{
		V2d pPos = targetPlayer->position;
		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = GetPosition();
		rayCastInfo.rayEnd = pPos + PlayerDir(targetPlayerIndex) * 5000.0;
		ignorePointsCloserThanPlayer = true;
		playerDist = PlayerDist(targetPlayerIndex);
		RayCast(this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

		if (rayCastInfo.rcEdge != NULL)
		{
			assert(rayCastInfo.rcEdge != NULL);

			V2d basePos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);

			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();
			//enemyMover.SetModeNodeLinear(basePos, CubicBezier(), 60);
			enemyMover.SetModeNodeLinearConstantSpeed(basePos, CubicBezier(), 40);
			//enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
		}
		else
		{
			Wait(1);
			assert(0);
		}
		break;
	}
	case MOVE_OTHER:
	{
		V2d nodePos = nodeGroupB.AlwaysGetNextNode()->pos;
		//StartMovement(nodePos + V2d( 0, -extraHeight));
		StartMovement(nodePos + V2d(0, -extraHeight));
		break;
	}
	case PATTERN_MOVE:
	{
		cout << "start pattern move" << endl;
		int trueIndex = -1;
		for (int i = 0; i < numPatternMoves; ++i)
		{
			if (patternOrder[i] == patternIndex)
			{
				trueIndex = i;
				break;
			}
		}

		assert(trueIndex != -1);
		currNode = pattern[trueIndex];

		StartMovement(currNode->pos + V2d( 0, -extraHeight ));

		//laserPool.Throw(0, GetPosition() + V2d( 0, -eyeExtraHeight),
		//	PlayerDir( V2d( 0, -eyeExtraHeight), V2d() ));
		break;
	}
	case CHASE_MOVE:
	{
		StartMovement(sess->GetPlayerPos( 0 ) );
		break;
	}

	case PLAN_PATTERN:
	{
		//babyScorpionGroup.Reset();
		PoiInfo *testNode;
		nodeGroupB.pickers[0].ShuffleActiveOptions();
		for (int i = 0; i < numPatternMoves; ++i)
		{
			do
			{
				testNode = nodeGroupB.AlwaysGetNextNode();
			} while (testNode == currNode);

			pattern[i] = testNode;
		}

		for (int i = 0; i < numPatternMoves; ++i)
		{
			patternType[i] = patternTypePicker.AlwaysGetNextOption();
		}

		for (int i = 0; i < numPatternMoves; ++i)
		{
			patternOrder[i] = patternOrderPicker.AlwaysGetNextOption();
		}

		actionLength[PLAN_PATTERN] = patternFlickerFrames * numPatternMoves;
		break;
	}
	case SHOOT_LASER_HOMING:
	case SHOOT_LASER:
	{
		if (PlayerDir().x > 0)
		{
			facingRight = true;
		}
		else if (PlayerDir().x < 0)
		{
			facingRight = false;
		}
		break;
	}
	case REDIRECT_TEST:
	{
		laserPool.SetAllSpeed(5);
		break;
	}
	case LASER_SPAM:
	{
		cout << "start laser spam" << endl;
		break;
	}
	
	}
}

void Skeleton::StartMovement(V2d &pos)
{
	V2d nodePos = pos;
	V2d nodeDiff = nodePos - GetPosition();
	double absNodeDiffX = abs(nodeDiff.x);

	enemyMover.currPosInfo.SetAerial();
	currPosInfo.SetAerial();

	if (nodeDiff.y < -600)
	{
		cout << "zipping" << endl;
		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = nodePos + V2d(0, -10);
		rayCastInfo.rayEnd = nodePos + V2d(0, -1) * 5000.0;//other * 5000.0;
		ignorePointsCloserThanPlayer = false;
		RayCast(this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

		currentPatternMoveType = PM_RISING_ZIP;

		if (rayCastInfo.rcEdge != NULL)
		{
			assert(rayCastInfo.rcEdge != NULL);

			V2d basePos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);
			enemyMover.SetModeZipAndFall(basePos, V2d(0, 2), nodePos);
			//enemyMover.SetModeRadial(basePos, speed, dest);
			//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
		}
		else
		{
			//what happens here??
		}
		//enemyMover.SetModeZipAndFall(, speed, centerPoint, nodePos);

	}
	else if (absNodeDiffX > 600)
	{
		cout << "swinging" << endl;
		//enemyMover.currPosInfo.position += V2d(0, -extraHeight);
		//currPosInfo = enemyMover.currPosInfo;

		currentPatternMoveType = PM_SWING;

		
		//nodePos += V2d(0, -extraHeight);
		V2d myPos = GetPosition();// +V2d(0, -extraHeight);

		//stuff
		V2d along = nodePos - myPos;
		V2d midPoint = myPos + along / 4.0;
		V2d centerPoint = (nodePos + myPos) / 2.0;
		along = normalize(along);

		V2d other(along.y, -along.x);

		double speed = -40;//dot(startVel, dir);

		if (other.y >= 0)
		{
			speed = -speed;
			other = -other;
		}


		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = midPoint;
		rayCastInfo.rayEnd = midPoint + V2d(0, -1) * 5000.0;//other * 5000.0;
		RayCast(this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd);

		if (rayCastInfo.rcEdge != NULL)
		{
			assert(rayCastInfo.rcEdge != NULL);

			V2d basePos = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);

			//basePos += V2d(0, -extraHeight);
			//V2d dir = normalize(basePos - GetPosition());
			//V2d along(-dir.y, dir.x);



			enemyMover.SetModeRadialDoubleJump(basePos, speed, centerPoint, nodePos);
			//enemyMover.SetModeRadial(basePos, speed, dest);
			//enemyMover.SetModeSwing(basePos, length(basePos - GetPosition()), 60);
		}
	}
	else
	{
		cout << "hopping" << endl;
		Hop(nodePos, 5, 100);
		//hopTarget = nodePos;
		//SetAction(JUMPSQUAT);
		//currentPatternMoveType = PM_HOP;
		//enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 2), 100);
	}
}

void Skeleton::FrameIncrement()
{
	Boss::FrameIncrement();
	if (action == GATHER_ENERGY_LOOP)
	{
		++currChargeLoopFrame;
		if (currChargeLoopFrame == maxChargeLoopFrames)
		{
			SetAction(GATHER_ENERGY_END);
		}
	}
}

void Skeleton::SetupPostFightScenes()
{
	if (postFightScene == NULL)
	{
		postFightScene = new SkeletonPostFightScene;
		//postFightScene->skeleton = this;
		//postFightScene->coyHelper = coyHelper;
		postFightScene->Init();
	}
}

void Skeleton::SetupNodeVectors()
{
	//nodeGroupA.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_SKELETON, "A"));
	nodeGroupB.SetNodeVec(sess->GetBossNodeVector(BossFightType::FT_SKELETON, "B"));
}

bool Skeleton::IsDecisionValid(int d)
{
	return true;
}

bool Skeleton::IsEnemyMoverAction(int a)
{
	return a == MOVE_WIRE_DASH || a == MOVE_OTHER || a == PATTERN_MOVE || a == HOP
		|| a == CHASE_MOVE;
}

void Skeleton::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void Skeleton::SeqWait()
{
	action = SEQ_WAIT;
	frame = 0;
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Skeleton::StartFight()
{
	Wait(10);
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	HitboxesOff();
}

void Skeleton::UpdateSprite()
{
	switch (action)
	{
	case WAIT:
	case MOVE:
	case COMBOMOVE:
		sprite.setTexture(*ts_stand->texture);
		ts_stand->SetSubRect(sprite, 0, !facingRight);
		break;
	case PATTERN_MOVE:
	{
		switch (currentPatternMoveType)
		{
		case PM_RISING_ZIP:
		{
			sprite.setTexture(*ts_stand->texture);
			ts_stand->SetSubRect(sprite, 0, !facingRight);
			break;
		}
		case PM_SWING:
		{
			sprite.setTexture(*ts_stand->texture);
			ts_stand->SetSubRect(sprite, 0, !facingRight);
		}
		case PM_HOP:
		{

			break;
		}
		}
		break;
	}
	case JUMPSQUAT: 
	case HOP: 
	case LAND:
	{
		int extra = 0;
		switch (action)
		{
		case JUMPSQUAT:
			extra = 0;
			break;
		case HOP:
			extra = 2;
			break;
		case LAND:
			extra = 4;
			break;
		}
		sprite.setTexture(*ts_hop->texture);
		ts_hop->SetSubRect(sprite, frame / animFactor[action] + extra, !facingRight);
	}
	case SHOOT_LASER_HOMING:
	case SHOOT_LASER:
	{
		ts_laser->SetSpriteTexture(sprite);
		ts_laser->SetSubRect(sprite, frame / animFactor[action], !facingRight );
		break;
	}
	case GATHER_ENERGY_START:
	{
		ts_charge->SetSpriteTexture(sprite);
		ts_charge->SetSubRect(sprite, 0, !facingRight);
		break;
	}
	case GATHER_ENERGY_LOOP:
	{
		ts_charge->SetSpriteTexture(sprite);
		ts_charge->SetSubRect(sprite, frame/ animFactor[action], !facingRight);
		break;
	}
	case GATHER_ENERGY_END:
	{
		ts_charge->SetSpriteTexture(sprite);
		ts_charge->SetSubRect(sprite, 4, !facingRight);
		break;
	}
	}

	//sprite.setRotation(currPosInfo.GetGroundAngleDegrees());

	sprite.setPosition(GetPositionF() + Vector2f(0, -extraHeight)); //skeleton sprite is spaced badly
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	UpdateWireQuad();
}

V2d Skeleton::GetCenter()
{
	return GetPosition() + V2d(0, -extraHeight);
}

void Skeleton::UpdateWireQuad()
{
	if (enemyMover.moveType == EnemyMover::RADIAL_DOUBLE_JUMP
		|| enemyMover.moveType == EnemyMover::ZIP_AND_FALL
		|| enemyMover.moveType == EnemyMover::SWING
		|| enemyMover.moveType == EnemyMover::SWINGJUMP)
	{

		V2d currWirePos = enemyMover.swingAnchor;
		V2d currPos = currPosInfo.GetPosition();//GetCenter();//currPosInfo.GetPosition();
		V2d other = normalize(currWirePos - currPos);
		other = V2d(other.y, -other.x);
		double width = 5;

		wireQuad[0] = Vector2f(currPos + other * width);
		wireQuad[1] = Vector2f(currPos - other * width);
		wireQuad[2] = Vector2f(currWirePos - other * width);
		wireQuad[3] = Vector2f(currWirePos + other * width);
		SetRectColor(wireQuad, Color::Red);
	}
	else
	{
		ClearRect(wireQuad);
	}
	
	/*V2d A = currPos + other * width;
	V2d B = swingAnchor + other * width;
	V2d C = swingAnchor - other * width;
	V2d D = currPos - other * width;*/
}

void Skeleton::EnemyDraw(sf::RenderTarget *target)
{
	if (action == PLAN_PATTERN)
	{
		target->draw(patternPreview);
		target->draw( patternNumberText);
	}
	
	/*sf::CircleShape cs;
	cs.setFillColor(Color::Green);
	cs.setRadius(20);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	cs.setPosition(GetPositionF());*/

	target->draw(wireQuad, 4, sf::Quads);

	DrawSprite(target, sprite);

	//target->draw(cs);
	//laserPool.Draw(target);
}

void Skeleton::SetPatternLength(int len)
{
	patternOrderPicker.Reset();
	for (int i = 0; i < len; ++i)
	{
		patternOrderPicker.AddActiveOption(i);
	}
	numPatternMoves = len;

	
	pattern.resize(numPatternMoves);

	patternType.resize(numPatternMoves);

	
	patternOrder.resize(numPatternMoves);
}

int Skeleton::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Skeleton::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);
}

void Skeleton::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);
}

void Skeleton::HandleRayCollision(Edge *edge, double edgeQuantity,
	double rayPortion)
{
	if (edge->edgeType == Edge::BORDER)
	{
		return;
	}

	V2d dir = normalize(rayCastInfo.rayEnd - rayCastInfo.rayStart);
	V2d pos = edge->GetPosition(edgeQuantity);

	//if (!sess->IsWithinBounds(pos))
	//{
	//	return; //prevents it from hitting the birdtransform area currently
	//}

	double along = dot(dir, edge->Normal());

	double posDist = length(pos - GetPosition());

	if (along < 0 && (rayCastInfo.rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - rayCastInfo.rayStart) <
		length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcPortion) - rayCastInfo.rayStart)))
	{
		if (!ignorePointsCloserThanPlayer || (ignorePointsCloserThanPlayer && posDist > playerDist))
		{
			rayCastInfo.rcEdge = edge;
			rayCastInfo.rcQuant = edgeQuantity;
		}
	}
}

void Skeleton::Hop(V2d &pos, double p_hopSpeed, double p_hopExtraHeight)
{
	SetAction(JUMPSQUAT);

	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else if (pos.x > GetPosition().x)
	{
		facingRight = true;
	}

	hopTarget = pos;// +V2d(0, -extraHeight);
	hopSpeed = p_hopSpeed;
	hopExtraHeight = p_hopExtraHeight;

}

void Skeleton::FinishPatternMove()
{
	if (patternIndex == numPatternMoves - 1)
	{
		/*if (stageMgr.GetCurrStage() > 0)
		{
		launchers[0]->position = GetPosition();
		launchers[0]->facingDir = PlayerDir();
		launchers[0]->Fire();
		}*/

		if (patternType[patternIndex] == PATTERN_MOVE)
		{
			Wait(30);
		}
		else
		{
			SetAction(patternType[patternIndex]);
			++patternIndex; //to finish after lasers etc
		}
	}
	else
	{
		++patternIndex;
		SetAction(patternType[patternIndex - 1]);

	}
}

int Skeleton::GetNumSimulationFramesRequired()
{
	return 0;
}

//void Skeleton::ShootStandingLaser(int shootType,V2d &pos,V2d &dir)
//{
//	V2d offset = V2d(0, -extraHeight);
//	if (facingRight)
//	{
//		offset += V2d(20, 16);
//	}
//	else
//	{
//		offset += V2d(-20, 16);
//	}
//	V2d shootDir = PlayerDir(offset, V2d());
//	double angChange = .1 * PI;
//
//	int shootType = 2;
//	laserPool.Throw(shootType, GetPosition() + offset,
//		shootDir);
//	/*laserPool.Throw(shootType, GetPosition() + offset,
//	shootDir );
//	RotateCCW(shootDir, angChange);
//	laserPool.Throw(shootType, GetPosition() + offset,
//	shootDir);
//	RotateCW(shootDir, angChange * 2);
//	laserPool.Throw(shootType, GetPosition() + offset,
//	shootDir);*/
//}