#include "Actor.h"
#include "GameSession.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>
#include "PowerOrbs.h"
#include "Sequence.h"
#include <list>
#include <fstream>
#include "MainMenu.h" //just for glsl color macro
#include "PlayerRecord.h"
#include "VisualEffects.h"
#include "Enemy.h"
#include "Enemy_Booster.h"
#include "HitboxManager.h"
#include "ImageText.h"
#include "KeyMarker.h"
#include "ScoreDisplay.h"
#include "WorldMap.h"
#include "SaveFile.h"
#include "PauseMenu.h"
#include "KinMenu.h"
#include "GroundTrigger.h"
#include "Enemy_Comboer.h"
#include "Enemy_Spring.h"
#include "AirTrigger.h"
#include "Nexus.h"
#include "HUD.h"
#include "VisualEffects.h"
#include "MapHeader.h"
#include "TouchGrass.h"
#include "Enemy_GravityModifier.h"
#include "ParticleEffects.h"
#include "Enemy_CurrencyItem.h"
#include "StorySequence.h"
#include "Enemy_BounceBooster.h"
#include "Enemy_Teleporter.h"
#include "Wire.h"
#include "Enemy_SwingLauncher.h"
#include "Grass.h"
#include "EnvPlant.h"
#include "Barrier.h"
#include "AbsorbParticles.h"
#include "EditSession.h"
#include "EditorRail.h"
#include "MovingGeo.h"
#include "GateMarker.h"
#include "Enemy_SpecialTarget.h"
#include "Enemy_ComboerTarget.h"
#include "Enemy_AimLauncher.h"
#include "Enemy_TimeBooster.h"
#include "Enemy_FreeFlightBooster.h"
#include "Enemy_HomingBooster.h"
#include "Enemy_AntiTimeSlowBooster.h"
#include "Enemy_SwordProjectileBooster.h"
#include "Enemy_SwordProjectile.h"
#include "Enemy_MomentumBooster.h"
#include "Enemy_InspectObject.h"
#include "GameMode.h"
#include "Enemy_RewindBooster.h"
#include "Enemy_TutorialObject.h"
#include "Enemy_ScorpionLauncher.h"

#include "GGPO.h"

#include "Enemy_Gator.h"
#include "Enemy_Bird.h"

#include "BossCommand.h"
#include "PowerSelectorHUD.h"

#include "SoundTypes.h"
#include "MotionGhostEffect.h"
#include "TimerHUD.h"
#include "PaletteShader.h"
#include "AdventureManager.h"
#include "DeathSequence.h"
#include "NameTag.h"
#include "LogMenu.h"
#include "NetplayManager.h"

#include "RushManager.h"
#include "KinUpgrades.h"

using namespace sf;
using namespace std;

#define COLOR_TEAL Color( 0, 0xee, 0xff )

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

//#define SETUP_ACTION_FUNCS(VAR) SetupFuncsForAction(##VAR,&Actor::##VAR_Start,&Actor::X_End,&Actor::X_Change,&Actor::X_Update,&Actor::X_UpdateSprite,&Actor::X_TransitionToAction,&Actor::X_TimeIndFrameInc,&Actor::X_TimeDepFrameInc,&Actor::X_GetActionLength,&Actor::X_GetTileset);

void Actor::Hitter::Clear()
{
	id = -1;
	launcherID = -1;
	framesToStayInArray = -1;
}

void Actor::Hitter::SetEnemy(int enemyID)
{
	id = enemyID;
	launcherID = -1;
	framesToStayInArray = 10;
}

void Actor::Hitter::SetBullet(int bulletID, int p_launcherID)
{
	id = bulletID;
	launcherID = p_launcherID;
	framesToStayInArray = 10;
}

bool Actor::Hitter::CheckEnemy(Enemy *e)
{
	if (e->enemyIndex == -1)
	{
		assert(0); //enemy index not initialized for this enemy
	}

	if (e == NULL)
	{
		assert(0);
		return false;
	}
	
	if (launcherID >= 0)
		return false;

	if (id == e->enemyIndex)
	{
		return true;
	}

	return false;
}

bool Actor::Hitter::CheckBullet(BasicBullet *b)
{
	if (b->bulletID == -1)
	{
		assert(0); //bullet not initialized
	}

	if (b == NULL)
	{
		assert(0);
		return false;
	}

	if (launcherID < 0)
		return false;

	if (id == b->bulletID)
	{
		return true;
	}

	return false;
}

void Actor::Hitter::Update()
{
	if (id >= 0 )
	{
		assert(framesToStayInArray > 0);
		--framesToStayInArray;

		if (framesToStayInArray == 0)
		{
			Clear();
		}
	}
}

void KeyExplodeUpdater::OnDeactivate(EffectInstance *ei)
{
	
	EffectInstance params;
	Transform tr = sf::Transform::Identity;

	params.SetParams(ei->pos, tr, 6, 3, 0);

	actor->ActivateEffect(Actor::PLAYERFX_KEY_EXPLODE, &params);
	//actor->ActivateEffect(DrawLayer::BETWEEN_PLAYER_AND_ENEMIES,
	//	actor->ts_keyExplode, V2d(ei->pos), true, 0, 6, 3, true);
}

void Actor::CheckBirdCommands()
{
	if (currBirdCommandIndex >= 3)
	{
		return;
	}
	if (AttackButtonPressed())
	{
		FightMode *fm = (FightMode*)sess->gameMode;
		birdCommands[currBirdCommandIndex]->action = 1;

		bool fr = facingRight;
		if (currInput.LRight())
		{
			fr = true;
		}
		else if( currInput.LLeft() )
		{
			fr = false;
		}
		birdCommands[currBirdCommandIndex]->facingRight = fr;

		fm->testBird->QueueCommand(*birdCommands[currBirdCommandIndex]);
		currBirdCommandIndex++;
	}
}

//#define cout std::cout

void Actor::PopulateState(PState *ps)
{
	ps->position = position;
	ps->velocity = velocity;
	ps->action = action;
	ps->frame = frame;
	ps->facingRight = facingRight;
	ps->groundSpeed = groundSpeed;
	ps->currInput = currInput.GetCompressedState();
	ps->prevInput = prevInput.GetCompressedState();

	ps->groundInfo.SetFromEdge(ground);
	ps->quant = edgeQuantity;
	ps->xOffset = offsetX;
	ps->holdDouble = holdDouble;
	ps->framesSinceClimbBoost = framesSinceClimbBoost;
	ps->holdJump = holdJump;
	ps->wallJumpFrameCounter = wallJumpFrameCounter;
	ps->hasDoubleJump = hasDoubleJump;
	ps->framesInAir = framesInAir;

	ps->brh = b.rh;
	ps->byoffset = b.offset.y;
	ps->bpos = b.globalPosition;
	ps->hasAirDash = hasAirDash;//true;//
	ps->numRemainingExtraAirdashBoosts = numRemainingExtraAirdashBoosts;
	ps->storedGroundSpeed = storedGroundSpeed;
	ps->currBBoostCounter = currBBoostCounter;
	ps->currAirdashBoostCounter = currAirdashBoostCounter;
	ps->steepJump = steepJump;
	ps->currentSpeedBar = currentSpeedBar;
	ps->speedLevel = speedLevel;

	ps->airDashStall = airDashStall;
	ps->startAirDashVel = startAirDashVel;
	ps->extraAirDashY = extraAirDashY;
	ps->oldAction = oldAction;

	ps->oldVelocity = oldVelocity;

	ps->reversed = reversed;
	ps->storedReverseSpeed = storedReverseSpeed;

	ps->distanceGrinded = distanceGrinded;
	ps->framesGrinding = framesGrinding;
	ps->framesNotGrinding = framesNotGrinding;
	ps->framesSinceGrindAttempt = framesSinceGrindAttempt;
	ps->maxFramesSinceGrindAttempt = maxFramesSinceGrindAttempt;


	ps->grindEdgeInfo.SetFromEdge( grindEdge );

	ps->grindSpeed = grindSpeed;

	ps->slowMultiple = slowMultiple;
	ps->slowCounter = slowCounter;
	ps->inBubble = inBubble;
	ps->oldInBubble = oldInBubble;

	
	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		ps->bubblePos[i] = bubblePos[i];
		ps->bubbleFramesToLive[i] = bubbleFramesToLive[i];
		ps->bubbleRadiusSize[i] = bubbleRadiusSize[i];
	}

	ps->currBubble = currBubble;

	ps->currAttackHit = currAttackHit;
	ps->bounceAttackHit = bounceAttackHit;
	ps->flashFrames = flashFrames;
	ps->bufferedAttack = bufferedAttack;
	ps->doubleJumpBufferedAttack = doubleJumpBufferedAttack;
	ps->wallJumpBufferedAttack = wallJumpBufferedAttack;
	ps->pauseBufferedAttack = pauseBufferedAttack;
	ps->pauseBufferedJump = pauseBufferedJump;
	ps->pauseBufferedDash = pauseBufferedDash;

	ps->hitlagFrames = hitlagFrames;
	ps->hitstunFrames = hitstunFrames;
	ps->setHitstunFrames = setHitstunFrames;
	ps->invincibleFrames = invincibleFrames;
	ps->receivedHit = receivedHit;

	ps->cancelAttack = cancelAttack;

	ps->dairBoostedDouble = dairBoostedDouble;
	ps->aerialHitCancelDouble = aerialHitCancelDouble;

	ps->hurtBody = hurtBody;

	ps->touchEdgeWithLeftWire = touchEdgeWithLeftWire;
	ps->touchEdgeWithRightWire = touchEdgeWithRightWire;
	ps->dWireAirDash = dWireAirDash;
	ps->dWireAirDashOld = dWireAirDashOld;

	leftWire->PopulateWireInfo(&ps->leftWireInfo);
	rightWire->PopulateWireInfo(&ps->rightWireInfo);

	ps->scorpOn = scorpOn;
	ps->framesFlameOn = framesFlameOn;
	ps->bounceFlameOn = bounceFlameOn;
	ps->storedBounceVel = storedBounceVel;
	ps->bounceNorm = bounceNorm;
	ps->oldBounceNorm = oldBounceNorm;

	ps->bounceEdgeInfo.SetFromEdge(bounceEdge);

	ps->storedBounceGroundSpeed = storedBounceGroundSpeed;
	ps->oldBounceEdgeInfo.SetFromEdge(oldBounceEdge);

	ps->framesSinceBounce = framesSinceBounce;
	ps->groundedWallBounce = groundedWallBounce;
	ps->bounceGrounded = bounceGrounded;
	ps->justToggledBounce = justToggledBounce;

	ps->dashAttackLevel = dashAttackLevel;
	ps->standAttackLevel = standAttackLevel;
	ps->upTiltLevel = upTiltLevel;
	ps->downTiltLevel = downTiltLevel;
	ps->framesSinceDashAttack = framesSinceDashAttack;
	ps->framesSinceStandAttack = framesSinceStandAttack;
	ps->framesSinceUpTilt = framesSinceUpTilt;
	ps->framesSinceDownTilt = framesSinceDownTilt;

	ps->framesSinceBlockPress = framesSinceBlockPress;
	ps->framesSinceSuperPress = framesSinceSuperPress;

	ps->standNDashBoost = standNDashBoost;
	ps->standNDashBoostCurr = standNDashBoostCurr;
	ps->hasFairAirDashBoost = hasFairAirDashBoost;
	ps->framesStanding = framesStanding;
	ps->framesSinceRightWireBoost = framesSinceRightWireBoost;
	ps->framesSinceLeftWireBoost = framesSinceLeftWireBoost;
	ps->framesSinceDoubleWireBoost = framesSinceDoubleWireBoost;
	ps->enemiesKilledThisFrame = enemiesKilledThisFrame;
	ps->enemiesKilledLastFrame = enemiesKilledLastFrame;

	ps->hitstunGravMultiplier = hitstunGravMultiplier;

	memcpy(ps->touchedGrass, touchedGrass, sizeof(bool) * Grass::Count);

	
	ps->superLevelCounter = superLevelCounter;
	ps->currActionSuperLevel = currActionSuperLevel;

	ps->attackingHitlag = attackingHitlag;

	ps->blockstunFrames = blockstunFrames;
	memcpy(ps->currAttackHitBlock, currAttackHitBlock, sizeof(int) * 4);

	ps->receivedHitPlayerIndex = sess->GetPlayerIndex(receivedHitPlayer);

	ps->receivedHitReaction = receivedHitReaction;

	ps->hasWallJumpRechargeDoubleJump = hasWallJumpRechargeDoubleJump;
	ps->hasWallJumpRechargeAirDash = hasWallJumpRechargeAirDash;
	ps->hasHitRechargeDoubleJump = hasHitRechargeDoubleJump;
	ps->hasHitRechargeAirDash = hasHitRechargeAirDash;

	ps->framesBlocking = framesBlocking;
	ps->receivedHitPosition = receivedHitPosition;

	ps->superFrame = superFrame;
	ps->kinMode = kinMode;

	//new stuff

	ps->prevRailID = sess->GetRailID(prevRail);
	ps->specialSlow = specialSlow;
	ps->frameAfterAttackingHitlagOver = frameAfterAttackingHitlagOver;
	ps->bouncedFromKill = bouncedFromKill;

	ps->projectileSwordFrames = projectileSwordFrames;
	ps->enemyProjectileSwordFrames = enemyProjectileSwordFrames;
	ps->gravModifyFrames = gravModifyFrames;
	ps->boosterGravModifyFrames = boosterGravModifyFrames;
	ps->extraGravityModifier = extraGravityModifier;
	ps->boosterExtraGravityModifier = boosterExtraGravityModifier;
	ps->waterEntrancePosition = waterEntrancePosition;


	ps->waterEntranceGroundInfo.SetFromEdge(waterEntranceGround);
	ps->waterEntranceGrindEdgeInfo.SetFromEdge(waterEntranceGrindEdge);

	ps->waterEntranceQuantity = waterEntranceQuantity;
	ps->waterEntranceXOffset = waterEntranceXOffset;
	ps->waterEntrancePhysHeight = waterEntrancePhysHeight;
	ps->waterEntranceFacingRight = waterEntranceFacingRight;
	ps->waterEntranceGrindSpeed = waterEntranceGrindSpeed;
	ps->waterEntranceReversed = waterEntranceReversed;
	ps->rewindBoosterPos = rewindBoosterPos;
	ps->rewindOnHitFrames = rewindOnHitFrames;
	ps->currSkinIndex = currSkinIndex;
	ps->waterEntranceVelocity = waterEntranceVelocity;
	ps->invertInputFrames = invertInputFrames;
	ps->currPowerMode = currPowerMode;
	ps->oldSpecialTerrainID = sess->GetSpecialPolyID(oldSpecialTerrain);
	ps->currSpecialTerrainID = sess->GetSpecialPolyID(currSpecialTerrain);
	ps->globalTimeSlowFrames = globalTimeSlowFrames;
	ps->freeFlightFrames = freeFlightFrames;
	ps->homingFrames = homingFrames;
	ps->antiTimeSlowFrames = antiTimeSlowFrames;

	ps->startGlobalTimeSlowFrames = startGlobalTimeSlowFrames;
	ps->startFreeFlightFrames = startFreeFlightFrames;
	ps->startHomingFrames = startHomingFrames;
	ps->startAntiTimeSlowFrames = startAntiTimeSlowFrames;
	ps->startMomentumBoostFrames = startMomentumBoostFrames;
	ps->startBoosterGravModifyFrames = startBoosterGravModifyFrames;

	ps->currTimeBoosterID = sess->GetEnemyID( currTimeBooster);
	ps->currFreeFlightBoosterID = sess->GetEnemyID(currFreeFlightBooster);
	ps->currHomingBoosterID = sess->GetEnemyID(currHomingBooster);
	ps->currAntiTimeSlowBoosterID = sess->GetEnemyID(currAntiTimeSlowBooster);
	ps->currSwordProjectileBoosterID = sess->GetEnemyID(currSwordProjectileBooster);
	ps->currMomentumBoosterID = sess->GetEnemyID(currMomentumBooster);
	ps->currRewindBoosterID = sess->GetEnemyID(currRewindBooster);


	ps->aimLauncherStunFrames = aimLauncherStunFrames;
	ps->airBounceCounter = airBounceCounter;
	ps->airBounceLimit = airBounceLimit;
	ps->momentumBoostFrames = momentumBoostFrames;

	ps->currSpringID = sess->GetEnemyID(currSpring);
	ps->currAimLauncherID = sess->GetEnemyID(currAimLauncher);
	ps->currTeleporterID = sess->GetEnemyID(currTeleporter);
	ps->oldTeleporterID = sess->GetEnemyID(oldTeleporter);
	ps->currBoosterID = sess->GetEnemyID(currBooster);
	ps->oldBoosterID = sess->GetEnemyID(oldBooster);
	ps->currSwingLauncherID = sess->GetEnemyID(currSwingLauncher);
	ps->oldSwingLauncherID = sess->GetEnemyID(oldSwingLauncher);
	ps->currBounceBoosterID = sess->GetEnemyID(currBounceBooster);
	ps->oldBounceBoosterID = sess->GetEnemyID(oldBounceBooster);

	ps->springStunFrames = springStunFrames;
	ps->springStunFramesStart = springStunFramesStart;

	ps->currScorpionLauncherID = sess->GetEnemyID(currScorpionLauncher);
	ps->oldScorpionLauncherID = sess->GetEnemyID(oldScorpionLauncher);

	ps->directionalInputFreezeFrames = directionalInputFreezeFrames;

	ps->numFramesToLive = numFramesToLive;

	ps->shieldPushbackFrames = shieldPushbackFrames;
	ps->shieldPushbackRight = shieldPushbackRight;

	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		ps->recentHitters[i] = recentHitters[i];
	}

	ps->activeComboObjListID = sess->GetComboObjectID(activeComboObjList);

	ps->currTutorialObjectID = sess->GetEnemyID(currTutorialObject);
	ps->currInspectObjectID = sess->GetEnemyID(currInspectObject);
	ps->currGravModifierID = sess->GetEnemyID(currGravModifier);

	ps->springVel = springVel;
	ps->glideTurnFactor = glideTurnFactor;

	ps->currencyCounter = currencyCounter;


	ps->hitGoal = hitGoal;

	ps->currTouchedGateID = sess->GetGateID(gateTouched);

	ps->swordState = swordState;
	//ps->hasWallJumpRecharge = hasWallJumpRecharge;
}

void Actor::PopulateFromState(PState *ps)
{
	position = ps->position;
	velocity = ps->velocity;
	action = ps->action;
	frame = ps->frame;
	facingRight = ps->facingRight;
	groundSpeed = ps->groundSpeed;
	prevInput.SetFromCompressedState(ps->prevInput); //relevant for keyboard
	currInput.SetFromCompressedState(ps->currInput);

	ground = sess->GetEdge(&ps->groundInfo);

	edgeQuantity = ps->quant;
	offsetX = ps->xOffset;

	holdDouble = ps->holdDouble;
	framesSinceClimbBoost = ps->framesSinceClimbBoost;
	holdJump = ps->holdJump;
	wallJumpFrameCounter = ps->wallJumpFrameCounter;
	hasDoubleJump = ps->hasDoubleJump;
	framesInAir = ps->framesInAir;

	b.rh = ps->brh;
	b.offset.y = ps->byoffset;
	b.globalPosition = ps->bpos;
	hasAirDash = ps->hasAirDash;
	numRemainingExtraAirdashBoosts = ps->numRemainingExtraAirdashBoosts;
	storedGroundSpeed = ps->storedGroundSpeed;
	currBBoostCounter = ps->currBBoostCounter;
	currAirdashBoostCounter = ps->currAirdashBoostCounter;
	steepJump = ps->steepJump;
	currentSpeedBar = ps->currentSpeedBar;
	speedLevel = ps->speedLevel;

	airDashStall = ps->airDashStall;
	startAirDashVel = ps->startAirDashVel;
	extraAirDashY = ps->extraAirDashY;

	oldAction = ps->oldAction;

	oldVelocity = ps->oldVelocity;

	reversed = ps->reversed;
	storedReverseSpeed = ps->storedReverseSpeed;

	distanceGrinded = ps->distanceGrinded;
	framesGrinding = ps->framesGrinding;
	framesNotGrinding = ps->framesNotGrinding;
	framesSinceGrindAttempt = ps->framesSinceGrindAttempt;
	maxFramesSinceGrindAttempt = ps->maxFramesSinceGrindAttempt;

	grindEdge = sess->GetEdge(&ps->grindEdgeInfo);

	grindSpeed = ps->grindSpeed;

	slowMultiple = ps->slowMultiple;
	slowCounter = ps->slowCounter;
	inBubble = ps->inBubble;
	oldInBubble = ps->oldInBubble;

	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		bubblePos[i] = ps->bubblePos[i];
		bubbleFramesToLive[i] = ps->bubbleFramesToLive[i];
		bubbleRadiusSize[i] = ps->bubbleRadiusSize[i];
	}

	currBubble = ps->currBubble;

	currAttackHit = ps->currAttackHit;
	bounceAttackHit = ps->bounceAttackHit;
	flashFrames = ps->flashFrames;
	bufferedAttack = ps->bufferedAttack;
	doubleJumpBufferedAttack = ps->doubleJumpBufferedAttack;
	wallJumpBufferedAttack = ps->wallJumpBufferedAttack;
	pauseBufferedAttack = ps->pauseBufferedAttack;
	pauseBufferedJump = ps->pauseBufferedJump;
	pauseBufferedDash = ps->pauseBufferedDash;

	hitlagFrames = ps->hitlagFrames;
	hitstunFrames = ps->hitstunFrames;
	setHitstunFrames = ps->setHitstunFrames;
	invincibleFrames = ps->invincibleFrames;
	receivedHit = ps->receivedHit;

	
	cancelAttack = ps->cancelAttack;

	dairBoostedDouble = ps->dairBoostedDouble;
	aerialHitCancelDouble = ps->aerialHitCancelDouble;

	hurtBody = ps->hurtBody;

	touchEdgeWithLeftWire = ps->touchEdgeWithLeftWire;
	touchEdgeWithRightWire = ps->touchEdgeWithRightWire;
	dWireAirDash = ps->dWireAirDash;
	dWireAirDashOld = ps->dWireAirDashOld;

	leftWire->PopulateFromWireInfo(&ps->leftWireInfo);
	rightWire->PopulateFromWireInfo(&ps->rightWireInfo);

	scorpOn = ps->scorpOn;
	framesFlameOn = ps->framesFlameOn;
	bounceFlameOn = ps->bounceFlameOn;
	storedBounceVel = ps->storedBounceVel;
	bounceNorm = ps->bounceNorm;
	oldBounceNorm = ps->oldBounceNorm;

	bounceEdge = sess->GetEdge(&ps->bounceEdgeInfo);

	storedBounceGroundSpeed = ps->storedBounceGroundSpeed;

	oldBounceEdge = sess->GetEdge(&ps->oldBounceEdgeInfo);

	framesSinceBounce = ps->framesSinceBounce;
	groundedWallBounce = ps->groundedWallBounce;
	bounceGrounded = ps->bounceGrounded;
	justToggledBounce = ps->justToggledBounce;

	dashAttackLevel = ps->dashAttackLevel;
	standAttackLevel = ps->standAttackLevel;
	upTiltLevel = ps->upTiltLevel;
	downTiltLevel = ps->downTiltLevel;

	framesSinceDashAttack = ps->framesSinceDashAttack;
	framesSinceStandAttack = ps->framesSinceStandAttack;
	framesSinceUpTilt = ps->framesSinceUpTilt;
	framesSinceDownTilt = ps->framesSinceDownTilt;

	framesSinceBlockPress = ps->framesSinceBlockPress;
	framesSinceSuperPress = ps->framesSinceSuperPress;

	standNDashBoost = ps->standNDashBoost;
	standNDashBoostCurr = ps->standNDashBoostCurr;
	hasFairAirDashBoost = ps->hasFairAirDashBoost;
	framesStanding = ps->framesStanding;
	framesSinceRightWireBoost = ps->framesSinceRightWireBoost;
	framesSinceLeftWireBoost = ps->framesSinceLeftWireBoost;
	framesSinceDoubleWireBoost = ps->framesSinceDoubleWireBoost;
	enemiesKilledThisFrame = ps->enemiesKilledThisFrame;
	enemiesKilledLastFrame = ps->enemiesKilledLastFrame;

	hitstunGravMultiplier = ps->hitstunGravMultiplier;

	memcpy(touchedGrass, ps->touchedGrass, sizeof(bool) * Grass::Count);

	
	superLevelCounter = ps->superLevelCounter;
	currActionSuperLevel = ps->currActionSuperLevel;

	attackingHitlag = ps->attackingHitlag;

	blockstunFrames = ps->blockstunFrames;
	memcpy(currAttackHitBlock, ps->currAttackHitBlock, sizeof(int) * 4);

	
	receivedHitPlayer = sess->GetPlayer(ps->receivedHitPlayerIndex);

	receivedHitReaction = (HitResult)ps->receivedHitReaction;
	hasWallJumpRechargeDoubleJump = ps->hasWallJumpRechargeDoubleJump;
	hasWallJumpRechargeAirDash = ps->hasWallJumpRechargeAirDash;
	hasHitRechargeDoubleJump = ps->hasHitRechargeDoubleJump;
	hasHitRechargeAirDash = ps->hasHitRechargeAirDash;
	framesBlocking = ps->framesBlocking;
	receivedHitPosition = ps->receivedHitPosition;

	superFrame = ps->superFrame;
	kinMode = (Mode)ps->kinMode;

	prevRail = sess->GetRailFromID(ps->prevRailID);

	specialSlow = ps->specialSlow;
	frameAfterAttackingHitlagOver = ps->frameAfterAttackingHitlagOver;
	bouncedFromKill = ps->bouncedFromKill;

	projectileSwordFrames = ps->projectileSwordFrames;
	enemyProjectileSwordFrames = ps->enemyProjectileSwordFrames;
	gravModifyFrames = ps->gravModifyFrames;
	boosterGravModifyFrames = ps->boosterGravModifyFrames;
	extraGravityModifier = ps->extraGravityModifier;
	boosterExtraGravityModifier = ps->boosterExtraGravityModifier;
	



	waterEntrancePosition = ps->waterEntrancePosition;

	waterEntranceGround = sess->GetEdge(&ps->waterEntranceGroundInfo);

	waterEntranceGrindEdge = sess->GetEdge(&ps->waterEntranceGrindEdgeInfo);

	waterEntranceQuantity = ps->waterEntranceQuantity;
	waterEntranceXOffset = ps->waterEntranceXOffset;
	waterEntrancePhysHeight = ps->waterEntrancePhysHeight;
	waterEntranceFacingRight = ps->waterEntranceFacingRight;
	waterEntranceGrindSpeed = ps->waterEntranceGrindSpeed;
	waterEntranceReversed = ps->waterEntranceReversed;
	rewindBoosterPos = ps->rewindBoosterPos;
	rewindOnHitFrames = ps->rewindOnHitFrames;
	currSkinIndex = ps->currSkinIndex;
	waterEntranceVelocity = ps->waterEntranceVelocity;
	invertInputFrames = ps->invertInputFrames;
	currPowerMode = ps->currPowerMode;
	oldSpecialTerrain = sess->GetSpecialPolyFromID(ps->oldSpecialTerrainID);
	currSpecialTerrain = sess->GetSpecialPolyFromID(ps->currSpecialTerrainID);
	globalTimeSlowFrames = ps->globalTimeSlowFrames;
	freeFlightFrames = ps->freeFlightFrames;
	homingFrames = ps->homingFrames;
	antiTimeSlowFrames = ps->antiTimeSlowFrames;

	startGlobalTimeSlowFrames = ps->startGlobalTimeSlowFrames;
	startFreeFlightFrames = ps->startFreeFlightFrames;
	startHomingFrames = ps->startHomingFrames;
	startAntiTimeSlowFrames = ps->startAntiTimeSlowFrames;
	startMomentumBoostFrames = ps->startMomentumBoostFrames;
	startBoosterGravModifyFrames = ps->startBoosterGravModifyFrames;

	currTimeBooster = (TimeBooster*)sess->GetEnemyFromID( ps->currTimeBoosterID );
	currFreeFlightBooster = (FreeFlightBooster*)sess->GetEnemyFromID( ps->currFreeFlightBoosterID );

	currHomingBooster = (HomingBooster*)sess->GetEnemyFromID( ps->currHomingBoosterID );

	currAntiTimeSlowBooster = (AntiTimeSlowBooster*)sess->GetEnemyFromID( ps->currAntiTimeSlowBoosterID );

	currSwordProjectileBooster = (SwordProjectileBooster*)sess->GetEnemyFromID(ps->currSwordProjectileBoosterID);

	currMomentumBooster = (MomentumBooster*)sess->GetEnemyFromID( ps->currMomentumBoosterID );

	currRewindBooster = (RewindBooster*)sess->GetEnemyFromID(ps->currRewindBoosterID);

	aimLauncherStunFrames = ps->aimLauncherStunFrames;
	airBounceCounter = ps->airBounceCounter;
	airBounceLimit = ps->airBounceLimit;
	momentumBoostFrames = ps->momentumBoostFrames;


	currSpring = (Spring*)sess->GetEnemyFromID(ps->currSpringID);
	currAimLauncher = (AimLauncher*)sess->GetEnemyFromID(ps->currAimLauncherID);
	currTeleporter = (Teleporter*)sess->GetEnemyFromID(ps->currTeleporterID);
	oldTeleporter = (Teleporter*)sess->GetEnemyFromID(ps->oldTeleporterID);
	currBooster = (Booster*)sess->GetEnemyFromID(ps->currBoosterID);
	oldBooster = (Booster*)sess->GetEnemyFromID(ps->oldBoosterID);
	currSwingLauncher = (SwingLauncher*)sess->GetEnemyFromID(ps->currSwingLauncherID);
	oldSwingLauncher = (SwingLauncher*)sess->GetEnemyFromID(ps->oldSwingLauncherID);
	currBounceBooster = (BounceBooster*)sess->GetEnemyFromID(ps->currBounceBoosterID);
	oldBounceBooster = (BounceBooster*)sess->GetEnemyFromID(ps->oldBounceBoosterID);

	currScorpionLauncher = (ScorpionLauncher*)sess->GetEnemyFromID(ps->currScorpionLauncherID);
	oldScorpionLauncher = (ScorpionLauncher*)sess->GetEnemyFromID(ps->oldScorpionLauncherID);

	springStunFrames = ps->springStunFrames;
	springStunFramesStart = ps->springStunFramesStart;

	directionalInputFreezeFrames = ps->directionalInputFreezeFrames;

	numFramesToLive = ps->numFramesToLive;

	//these aren't even used for anything!
	shieldPushbackFrames = ps->shieldPushbackFrames;
	shieldPushbackRight = ps->shieldPushbackRight;

	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		recentHitters[i] = ps->recentHitters[i];
	}

	activeComboObjList = sess->GetComboObjectFromID(ps->activeComboObjListID);

	currTutorialObject = (TutorialObject*)sess->GetEnemyFromID(ps->currTutorialObjectID);
	currInspectObject = (InspectObject*)sess->GetEnemyFromID(ps->currInspectObjectID);
	currGravModifier = (GravityModifier*)sess->GetEnemyFromID(ps->currGravModifierID);

	springVel = ps->springVel;
	glideTurnFactor = ps->glideTurnFactor;

	currencyCounter = ps->currencyCounter;

	hitGoal = ps->hitGoal;

	gateTouched = sess->GetGateFromID(ps->currTouchedGateID);

	swordState = ps->swordState;
}

void Actor::SetSession(Session *p_sess,
	GameSession *game,
	EditSession *edit)
{
	sess = p_sess;
	owner = game;
	editOwner = edit;
}

SoundInfo * Actor::GetSound(const std::string &name)
{
	return MainMenu::GetInstance()->soundManager.GetSound(name);
	/*if (sess != NULL)
	{
		return sess->GetSound(name);
	}
	else
	{
		
	}*/
}

void Actor::CreateCollisionBodies()
{
	fairHitboxes[0] = CreateCollisionBody("fairahitboxes");
	fairHitboxes[1] = CreateCollisionBody("fairbhitboxes");
	fairHitboxes[2] = CreateCollisionBody("fairchitboxes");

	uairHitboxes[0] = CreateCollisionBody("uairahitboxes");
	uairHitboxes[1] = CreateCollisionBody("uairbhitboxes");
	uairHitboxes[2] = CreateCollisionBody("uairchitboxes");

	dairHitboxes[0] = CreateCollisionBody("dairahitboxes");
	dairHitboxes[1] = CreateCollisionBody("dairbhitboxes");
	dairHitboxes[2] = CreateCollisionBody("dairchitboxes");

	standHitboxes1[0] = CreateCollisionBody("stand_att_01_a_hitboxes");
	standHitboxes1[1] = CreateCollisionBody("stand_att_01_b_hitboxes");
	standHitboxes1[2] = CreateCollisionBody("stand_att_01_c_hitboxes");

	standHitboxes2[0] = CreateCollisionBody("stand_att_02_a_hitboxes");
	standHitboxes2[1] = CreateCollisionBody("stand_att_02_b_hitboxes");
	standHitboxes2[2] = CreateCollisionBody("stand_att_02_c_hitboxes");

	standHitboxes3[0] = CreateCollisionBody("stand_att_03_a_hitboxes");
	standHitboxes3[1] = CreateCollisionBody("stand_att_03_b_hitboxes");
	standHitboxes3[2] = CreateCollisionBody("stand_att_03_c_hitboxes");

	standHitboxes4[0] = CreateCollisionBody("stand_att_04_a_hitboxes");
	standHitboxes4[1] = CreateCollisionBody("stand_att_04_b_hitboxes");
	standHitboxes4[2] = CreateCollisionBody("stand_att_04_c_hitboxes");

	dashHitboxes1[0] = CreateCollisionBody("dash_att_01_a_hitboxes");
	dashHitboxes1[1] = CreateCollisionBody("dash_att_01_b_hitboxes");
	dashHitboxes1[2] = CreateCollisionBody("dash_att_01_c_hitboxes");

	dashHitboxes2[0] = CreateCollisionBody("dash_att_02_a_hitboxes");
	dashHitboxes2[1] = CreateCollisionBody("dash_att_02_b_hitboxes");
	dashHitboxes2[2] = CreateCollisionBody("dash_att_02_c_hitboxes");

	dashHitboxes3[0] = CreateCollisionBody("dash_att_03_a_hitboxes");
	dashHitboxes3[1] = CreateCollisionBody("dash_att_03_b_hitboxes");
	dashHitboxes3[2] = CreateCollisionBody("dash_att_03_c_hitboxes");

	wallHitboxes[0] = CreateCollisionBody("wallahitboxes");
	wallHitboxes[1] = CreateCollisionBody("wallbhitboxes");
	wallHitboxes[2] = CreateCollisionBody("wallchitboxes");

	steepClimbHitboxes[0] = CreateCollisionBody("climbahitboxes");
	steepClimbHitboxes[1] = CreateCollisionBody("climbbhitboxes");
	steepClimbHitboxes[2] = CreateCollisionBody("climbchitboxes");

	steepSlideHitboxes[0] = CreateCollisionBody("slideahitboxes");
	steepSlideHitboxes[1] = CreateCollisionBody("slidebhitboxes");
	steepSlideHitboxes[2] = CreateCollisionBody("slidechitboxes");

	diagUpHitboxes[0] = CreateCollisionBody("airdashupahitboxes");
	diagUpHitboxes[1] = CreateCollisionBody("airdashupbhitboxes");
	diagUpHitboxes[2] = CreateCollisionBody("airdashupchitboxes");

	diagDownHitboxes[0] = CreateCollisionBody("airdashdownahitboxes");
	diagDownHitboxes[1] = CreateCollisionBody("airdashdownbhitboxes");
	diagDownHitboxes[2] = CreateCollisionBody("airdashdownchitboxes");
}

CollisionBody *Actor::CreateCollisionBody(const std::string &str)
{
	return MainMenu::GetInstance()->kinHitboxManager->CreateBody(str, currHitboxInfo);
}

void Actor::UpdatePowersMenu()
{
	if (owner != NULL)
	{
		owner->pauseMenu->kinMenu->UpdatePowers(this);
	}
}

QuadTree *Actor::GetTerrainTree()
{
	return sess->terrainTree;
	/*if (owner != NULL)
		return owner->terrainTree;
	else if( editOwner != NULL )
	{
		return editOwner->terrainTree;
	}*/
}

QuadTree *Actor::GetSpecialTerrainTree()
{
	return sess->specialTerrainTree;
}

QuadTree *Actor::GetRailEdgeTree()
{
	return sess->railEdgeTree;
	/*if (owner != NULL)
		return owner->railEdgeTree;
	else if (editOwner != NULL)
	{
		return editOwner->railEdgeTree;
	}*/
}

QuadTree *Actor::GetBarrierTree()
{
	return sess->barrierTree;
	/*if (owner != NULL)
		return owner->barrierTree;
	else if (editOwner != NULL)
	{
		return editOwner->barrierTree;
	}*/
}

QuadTree *Actor::GetBorderTree()
{
	return sess->borderTree;
	/*if (owner != NULL)
		return owner->borderTree;
	else if (editOwner != NULL)
	{
		return editOwner->borderTree;
	}*/
}

void Actor::SetToOriginalPos()
{
	if (owner != NULL)
	{
		position = V2d(owner->playerOrigPos[actorIndex]);
	}
	else if (editOwner != NULL)
	{
		position = editOwner->GetPlayerSpawnPos(actorIndex);
	}
}

SoundNode * Actor::ActivateSound(int st, bool loop )
{
	if (simulationMode)// || sess->simulationMode)
	{
		return NULL;
	}

	if (sess->IsParallelSession())
	{
		return NULL;
	}
		

	SoundInfo *si = soundInfos[st];

	if (si == NULL)
		return NULL;

	if (sess->gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && sess->IsParallelSession())
	{
		return NULL;
	}
	

	//for multiplayer testing
	//if (hitlagFrames == 0)
	//{
		return sess->soundNodeList->ActivateSound(si, loop);
	//}
}

SoundNode * Actor::ActivateRepeatingSound(int st, bool loop )
{
	if (simulationMode)// || sess->simulationMode)
	{
		return NULL;
	}

	if (sess->IsParallelSession())
		return NULL;

	StopRepeatingSound();
	repeatingSound = ActivateSound(st, loop);
	return repeatingSound;
}

void Actor::DeactivateSound(SoundNode *sn)
{
	if (simulationMode)
		return;

	if (sess->IsParallelSession())
		return;
	sess->soundNodeList->DeactivateSound(sn);
}

BasicEffect * Actor::ActivateEffect(
	int layer,
	Tileset *ts,
	sf::Vector2<double> pos,
	bool pauseImmune,
	double angle,
	int frameCount,
	int animationFactor,
	bool right,
	int startFrame,
	float depth)
{
	if (simulationMode)
	{
		return NULL;
	}
	
	BasicEffect *be = sess->ActivateEffect(layer, ts, pos, pauseImmune, angle, frameCount, animationFactor,
		right, startFrame, depth);
	return be;
}

EffectInstance * Actor::ActivateEffect(int pfxType, sf::Vector2f &pos, double angle, int frameCount, int animFactor, bool right, int startFrame)
{
	if (simulationMode)
		return NULL;

	EffectInstance params;
	Transform t;

	t.rotate(angle);

	if (!right)
	{
		t.scale(Vector2f(-1, 1));
	}

	params.SetParams(Vector2f(pos), t, frameCount, animFactor, startFrame);
	EffectInstance *ei = effectPools[pfxType].pool->ActivateEffect(&params);

	return ei;
}

EffectInstance * Actor::ActivateEffect(int pfxType, EffectInstance *params)
{
	if (simulationMode)
		return NULL;

	EffectInstance *ei = effectPools[pfxType].pool->ActivateEffect(params);
	return ei;
}

GameController *Actor::GetController(int index)
{
	if (owner != NULL)
	{
		return owner->GetController(actorIndex);
	}
	else if (editOwner != NULL)
	{
		return editOwner->GetController(actorIndex);
	}
	else
	{
		assert(0);
		return owner->GetController(actorIndex);
	}
}

void Actor::SetCurrHitboxes(CollisionBody *cBody,
	int p_frame)
{
	if (cBody != NULL)
	{
		if (cBody->GetNumFrames() <= p_frame)
		{
			SetCurrHitboxes(NULL, 0);
		}
		else
		{
			currHitboxes = cBody;
			currHitboxFrame = p_frame;
		}
	}
	else
	{
		//recent add. trying to fix rollback issue?
		currHitboxes = NULL;
		currHitboxFrame = 0;
	}
}

Collider &Actor::GetCollider()
{
	return sess->collider;
}

void Actor::SetupFX()
{
	string folder = "Kin/FX/";

	TilesetManager *tm = sess->mainMenu;
	MainMenu *mm = sess->mainMenu;

	effectPools.resize(PLAYERFX_Count);

	effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_FAIR_SWORD_LIGHTNING_0), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_FAIR_SWORD_LIGHTNING_1), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_FAIR_SWORD_LIGHTNING_2), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);

	effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DAIR_SWORD_LIGHTNING_0), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DAIR_SWORD_LIGHTNING_1), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DAIR_SWORD_LIGHTNING_2), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);

	effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_UAIR_SWORD_LIGHTNING_0), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_UAIR_SWORD_LIGHTNING_0), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_UAIR_SWORD_LIGHTNING_0), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);

	effectPools[PLAYERFX_BOUNCE_BOOST].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_BOUNCE_BOOST), EffectType::FX_REGULAR, 20, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false);
	effectPools[PLAYERFX_HURT_SPACK].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_HURT_SPACK), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_DASH_START].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DASH_START), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_DASH_REPEAT].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DASH_REPEAT), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_LAND_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_LAND_0), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_LAND_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_LAND_1), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_LAND_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_LAND_2), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_RUN_START].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_RUN_START), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_SPRINT_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_SPRINT_0), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_SPRINT_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_SPRINT_1), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_SPRINT_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_SPRINT_2), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_RUN].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_RUN), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_JUMP_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_JUMP_0), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_JUMP_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_JUMP_1), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_JUMP_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_JUMP_2), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_WALLJUMP_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_WALLJUMP_0), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_WALLJUMP_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_WALLJUMP_1), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_WALLJUMP_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_WALLJUMP_2), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_DOUBLE].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DOUBLE), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_GRAV_REVERSE].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_GRAV_REVERSE), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_SPEED_LEVEL_CHARGE].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_SPEED_LEVEL_CHARGE), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_RIGHT_WIRE_BOOST].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_RIGHT_WIRE_BOOST), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_LEFT_WIRE_BOOST].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_LEFT_WIRE_BOOST), EffectType::FX_REGULAR, 20);
	effectPools[PLAYERFX_DOUBLE_WIRE_BOOST].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DOUBLE_WIRE_BOOST), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_AIRDASH_DIAGONAL].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_AIRDASH_DIAGONAL), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_AIRDASH_UP].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_AIRDASH_UP), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_AIRDASH_HOVER].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_AIRDASH_HOVER), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_GATE_ENTER].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_GATE_ENTER), EffectType::FX_REGULAR, 20);

	effectPools[PLAYERFX_SMALL_LIGHTNING].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_SMALL_LIGHTNING), EffectType::FX_RELATIVE, 20);

	effectPools[PLAYERFX_GATE_BLACK].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_GATE_BLACK), EffectType::FX_RELATIVE, 2,
		DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false);

	effectPools[PLAYERFX_KEY].Set(sess->ts_key, EffectType::FX_REGULAR, 32,
		DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false);
	keyExplodeUpdater = new KeyExplodeUpdater(this);
	effectPools[PLAYERFX_KEY].pool->SetUpdater(keyExplodeUpdater);

	effectPools[PLAYERFX_KEY_EXPLODE].Set(sess->ts_keyExplode, EffectType::FX_REGULAR, 32, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES,
		true, false);

	effectPools[PLAYERFX_DASH_BOOST].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_DASH_BOOST), EffectType::FX_REGULAR, 8);

	effectPools[PLAYERFX_SPRINT_STAR].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_SPRINT_STAR), EffectType::FX_RELATIVE, 100);

	effectPools[PLAYERFX_LAUNCH_PARTICLE_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_LAUNCH_PARTICLE_0), EffectType::FX_REGULAR, 100,
		DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false);
	effectPools[PLAYERFX_LAUNCH_PARTICLE_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_LAUNCH_PARTICLE_1), EffectType::FX_REGULAR, 100,
		DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false);

	effectPools[PLAYERFX_ENTER].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_ENTER), EffectType::FX_REGULAR, 1, DrawLayer::IN_FRONT);

	//hopefully no problem making only 1 of these
	effectPools[PLAYERFX_EXITENERGY_0].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_EXITENERGY_0), EffectType::FX_IND, 1);
	effectPools[PLAYERFX_EXITENERGY_1].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_EXITENERGY_1), EffectType::FX_IND, 1);
	effectPools[PLAYERFX_EXITENERGY_2].Set(mm->GetPlayerTileset(MainMenu::PTS_FX_EXITENERGY_2), EffectType::FX_IND, 1);


	//effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_0].Set(tm->GetSizedTileset(folder, "fair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	//effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_1].Set(tm->GetSizedTileset(folder, "fair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	//effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_2].Set(tm->GetSizedTileset(folder, "fair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);

	//effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_0].Set(tm->GetSizedTileset(folder, "dair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	//effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_1].Set(tm->GetSizedTileset(folder, "dair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	//effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_2].Set(tm->GetSizedTileset(folder, "dair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);

	//effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_0].Set(tm->GetSizedTileset(folder, "uair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	//effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_1].Set(tm->GetSizedTileset(folder, "uair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);
	//effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_2].Set(tm->GetSizedTileset(folder, "uair_sword_lightning_256x256.png"), EffectType::FX_RELATIVE, 20, DrawLayer::IN_FRONT);

	//effectPools[PLAYERFX_BOUNCE_BOOST].Set(tm->GetSizedTileset(folder, "bounceboost_256x192.png"), EffectType::FX_REGULAR, 20, BETWEEN_PLAYER_AND_ENEMIES, false, false );
	//effectPools[PLAYERFX_HURT_SPACK].Set(tm->GetSizedTileset(folder, "fx_hurt_spack_128x160.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_DASH_START].Set(tm->GetSizedTileset(folder, "fx_dash_start_160x160.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_DASH_REPEAT].Set(tm->GetSizedTileset(folder, "fx_dash_repeat_192x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_LAND_0].Set(tm->GetSizedTileset(folder, "fx_land_a_128x128.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_LAND_1].Set(tm->GetSizedTileset(folder, "fx_land_b_192x208.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_LAND_2].Set(tm->GetSizedTileset(folder, "fx_land_c_224x224.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_RUN_START].Set(tm->GetSizedTileset(folder, "fx_runstart_128x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_SPRINT_0].Set(tm->GetSizedTileset(folder, "fx_sprint_a_192x192.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_SPRINT_1].Set(tm->GetSizedTileset(folder, "fx_sprint_b_320x320.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_SPRINT_2].Set(tm->GetSizedTileset(folder, "fx_sprint_c_320x320.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_RUN].Set(tm->GetSizedTileset(folder, "fx_run_144x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_JUMP_0].Set(tm->GetSizedTileset(folder, "fx_jump_a_128x80.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_JUMP_1].Set(tm->GetSizedTileset(folder, "fx_jump_b_160x192.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_JUMP_2].Set(tm->GetSizedTileset(folder, "fx_jump_c_160x192.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_WALLJUMP_0].Set(tm->GetSizedTileset(folder, "fx_walljump_a_160x160.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_WALLJUMP_1].Set(tm->GetSizedTileset(folder, "fx_walljump_b_224x224.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_WALLJUMP_2].Set(tm->GetSizedTileset(folder, "fx_walljump_c_224x224.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_DOUBLE].Set(tm->GetSizedTileset(folder, "fx_double_256x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_GRAV_REVERSE].Set(tm->GetSizedTileset(folder, "fx_grav_reverse_128x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_SPEED_LEVEL_CHARGE].Set(tm->GetSizedTileset(folder, "fx_elec_128x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_RIGHT_WIRE_BOOST].Set(tm->GetSizedTileset(folder, "wire_boost_r_64x64.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_LEFT_WIRE_BOOST].Set(tm->GetSizedTileset(folder, "wire_boost_b_64x64.png"), EffectType::FX_REGULAR, 20);
	//effectPools[PLAYERFX_DOUBLE_WIRE_BOOST].Set(tm->GetSizedTileset(folder, "wire_boost_m_64x64.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_AIRDASH_DIAGONAL].Set(tm->GetSizedTileset(folder, "fx_airdash_diag_128x160.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_AIRDASH_UP].Set(tm->GetSizedTileset(folder, "fx_airdash_128x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_AIRDASH_HOVER].Set(tm->GetSizedTileset(folder, "fx_airdash_hold_96x80.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_GATE_ENTER].Set(tm->GetSizedTileset(folder, "fx_gate_enter_160x128.png"), EffectType::FX_REGULAR, 20);

	//effectPools[PLAYERFX_SMALL_LIGHTNING].Set(tm->GetSizedTileset(folder, "fx_elec_128x96.png"), EffectType::FX_RELATIVE, 20);

	//effectPools[PLAYERFX_GATE_BLACK].Set(tm->GetSizedTileset(folder, "keydrain_160x160.png"), EffectType::FX_RELATIVE, 2,
	//	DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false);

	//effectPools[PLAYERFX_KEY].Set(sess->ts_key, EffectType::FX_REGULAR, 32,
	//	DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, false, false );
	//keyExplodeUpdater = new KeyExplodeUpdater(this);
	//effectPools[PLAYERFX_KEY].pool->SetUpdater(keyExplodeUpdater);

	//effectPools[PLAYERFX_KEY_EXPLODE].Set(sess->ts_keyExplode, EffectType::FX_REGULAR, 32,DrawLayer::BETWEEN_PLAYER_AND_ENEMIES,
	//	true, false);

	//effectPools[PLAYERFX_DASH_BOOST].Set(tm->GetSizedTileset(folder, "fx_dash_boost_128x256.png"), EffectType::FX_REGULAR, 4);

	//effectPools[PLAYERFX_SPRINT_STAR].Set(tm->GetSizedTileset(folder, "fx_sprint_star_01_64x64.png"), EffectType::FX_RELATIVE, 100);

	//effectPools[PLAYERFX_LAUNCH_PARTICLE_0].Set(tm->GetSizedTileset(folder, "launch_fx_192x128.png"), EffectType::FX_REGULAR, 100,
	//	BETWEEN_PLAYER_AND_ENEMIES, false, false);
	//effectPools[PLAYERFX_LAUNCH_PARTICLE_1].Set(tm->GetSizedTileset(folder, "launch_fx_192x128.png"), EffectType::FX_REGULAR, 100,
	//	BETWEEN_PLAYER_AND_ENEMIES, false, false);

	//effectPools[PLAYERFX_ENTER].Set(tm->GetSizedTileset(folder, "fx_enter_256x256.png"), EffectType::FX_REGULAR, 1, DrawLayer::IN_FRONT);

	////hopefully no problem making only 1 of these
	//effectPools[PLAYERFX_EXITENERGY_0].Set(tm->GetSizedTileset(folder, "exitenergy_0_512x512.png"), EffectType::FX_IND, 1);
	//effectPools[PLAYERFX_EXITENERGY_1].Set(tm->GetSizedTileset(folder, "exitenergy_1_512x512.png"), EffectType::FX_IND, 1);
	//effectPools[PLAYERFX_EXITENERGY_2].Set(tm->GetSizedTileset(folder, "exitenergy_2_512x512.png"), EffectType::FX_IND, 1);

	keyExplodeRingGroup = new MovingGeoGroup;
	keyExplodeRingGroup->AddGeo(new MovingRing(32, 20, 300, 12, 12, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 60));
	keyExplodeRingGroup->Init();

	enemyExplodeRingGroup = new MovingGeoGroup;
	enemyExplodeRingGroup->AddGeo(new MovingRing(32, 20, 300, 12, 12, Vector2f(0, 0), Vector2f(0, 0),
		Color::Red, Color(255, 0, 0, 0), 60));
	enemyExplodeRingGroup->Init();

	enemiesClearedRingGroup = new MovingGeoGroup;
	enemiesClearedRingGroup->AddGeo(new MovingRing(32, 20, 250, 12, 12, Vector2f(0, 0), Vector2f(0, 0),
		Color::Red, Color(255, 0, 0, 0),30));
	enemiesClearedRingGroup->Init();


	enoughKeysToExitRingGroup = new MovingGeoGroup;
	enoughKeysToExitRingGroup->AddGeo(new MovingRing(32, 20, 250, 12, 12, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0),
		/*Color( 200, 200, 200, 255 ), Color(200, 200, 200, 0)*/ 30));
	enoughKeysToExitRingGroup->Init();


	gravityDecreaserOnRingGroup = new MovingGeoGroup;
	gravityDecreaserOnRingGroup->AddGeo(new MovingRing(32, 120, 1, 8, 8, Vector2f(0, 0), Vector2f(0, 0),
		Color::Green, Color(0, 255, 0, 0),
		/*Color( 200, 200, 200, 255 ), Color(200, 200, 200, 0)*/ 30));
	gravityDecreaserOnRingGroup->Init();


	gravityDecreaserOffRingGroup = new MovingGeoGroup;
	gravityDecreaserOffRingGroup->AddGeo(new MovingRing(32, 1, 120, 8, 8, Vector2f(0, 0), Vector2f(0, 0),
		Color::Green, Color(0, 255, 0, 0),
		/*Color( 200, 200, 200, 255 ), Color(200, 200, 200, 0)*/ 30));
	gravityDecreaserOffRingGroup->Init();

	Color incColor = Color(0xdf, 0x71, 0x26);
	Color invIncColor = incColor;
	invIncColor.a = 0;

	gravityIncreaserOnRingGroup = new MovingGeoGroup;
	gravityIncreaserOnRingGroup->AddGeo(new MovingRing(32, 120, 1, 8, 8, Vector2f(0, 0), Vector2f(0, 0),
		incColor, invIncColor,
		/*Color( 200, 200, 200, 255 ), Color(200, 200, 200, 0)*/ 30));
	gravityIncreaserOnRingGroup->Init();


	gravityIncreaserOffRingGroup = new MovingGeoGroup;
	gravityIncreaserOffRingGroup->AddGeo(new MovingRing(32, 1, 120, 8, 8, Vector2f(0, 0), Vector2f(0, 0),
		incColor, invIncColor,
		/*Color( 200, 200, 200, 255 ), Color(200, 200, 200, 0)*/ 30));
	gravityIncreaserOffRingGroup->Init();

	


	for (auto it = effectPools.begin(); it != effectPools.end(); ++it)
	{
		if ((*it).pool != NULL && (*it).usesPlayerSkinShader )
		{
			(*it).pool->effectShader = &(fxPaletteShader->pShader);
		}
		
	}
}

void Actor::SetupSwordTilesets()
{
	string folder = "Kin/Sword/";



	TilesetManager *tm = MainMenu::GetInstance();//sess->mainMenu;

	ts_fairSword[0] = tm->GetSizedTileset(folder, "fair_sword_256x256.png");
	ts_fairSword[1] = tm->GetSizedTileset(folder, "fair_sword_b_288x288.png");
	ts_fairSword[2] = tm->GetSizedTileset(folder, "fair_sword_p_384x384.png");

	ts_dairSword[0] = tm->GetSizedTileset(folder, "dair_sword_256x256.png");
	ts_dairSword[1] = tm->GetSizedTileset(folder, "dair_sword_b_288x288.png");
	ts_dairSword[2] = tm->GetSizedTileset(folder, "dair_sword_p_384x384.png");

	ts_uairSword[0] = tm->GetSizedTileset(folder, "uair_sword_256x256.png");
	ts_uairSword[1] = tm->GetSizedTileset(folder, "uair_sword_b_288x288.png");
	ts_uairSword[2] = tm->GetSizedTileset(folder, "uair_sword_p_320x320.png");

	ts_grindLungeSword[0] = tm->GetSizedTileset(folder, "grind_lunge_sworda_160x160.png");
	ts_grindLungeSword[1] = tm->GetSizedTileset(folder, "grind_lunge_swordb_192x192.png");
	ts_grindLungeSword[2] = tm->GetSizedTileset(folder, "grind_lunge_swordc_224x208.png");

	ts_wallAttackSword[0] = tm->GetSizedTileset(folder, "wall_sword_144x256.png");
	ts_wallAttackSword[1] = tm->GetSizedTileset(folder, "wall_sword_b_240x352.png");
	ts_wallAttackSword[2] = tm->GetSizedTileset(folder, "wall_sword_p_298x400.png");

	ts_steepSlideAttackSword[0] = tm->GetSizedTileset(folder, "steep_att_sword_480x176.png");
	ts_steepSlideAttackSword[1] = tm->GetSizedTileset(folder, "steep_att_sword_b_352x192.png");
	ts_steepSlideAttackSword[2] = tm->GetSizedTileset(folder, "steep_att_sword_p_560x256.png");

	ts_steepClimbAttackSword[0] = tm->GetSizedTileset(folder, "climb_att_sword_352x128.png");
	ts_steepClimbAttackSword[1] = tm->GetSizedTileset(folder, "climb_att_sword_b_416x320.png");
	ts_steepClimbAttackSword[2] = tm->GetSizedTileset(folder, "climb_att_sword_p_496x208.png");

	ts_diagUpSword[0] = tm->GetSizedTileset(folder, "airdash_u_sword_144x208.png");
	ts_diagUpSword[1] = tm->GetSizedTileset(folder, "airdash_u_sword_b_224x240.png");
	ts_diagUpSword[2] = tm->GetSizedTileset(folder, "airdash_u_sword_p_320x384.png");

	ts_diagDownSword[0] = tm->GetSizedTileset(folder, "airdash_sword_128x208.png");
	ts_diagDownSword[1] = tm->GetSizedTileset(folder, "airdash_sword_b_224x240.png");
	ts_diagDownSword[2] = tm->GetSizedTileset(folder, "airdash_sword_p_320x384.png");

	ts_dashAttackSword[0] = tm->GetSizedTileset(folder, "dash_att_01_sword_384x320.png");
	ts_dashAttackSword[1] = tm->GetSizedTileset(folder, "dash_att_01_sword_b_480x480.png");
	ts_dashAttackSword[2] = tm->GetSizedTileset(folder, "dash_att_01_sword_p_480x480.png");

	ts_dashAttackSword2[0] = tm->GetSizedTileset(folder, "dash_att_02_sword_384x384.png");
	ts_dashAttackSword2[1] = tm->GetSizedTileset(folder, "dash_att_02_sword_b_480x480.png");
	ts_dashAttackSword2[2] = tm->GetSizedTileset(folder, "dash_att_02_sword_p_480x480.png");

	ts_dashAttackSword3[0] = tm->GetSizedTileset(folder, "dash_att_03_sword_384x384.png");
	ts_dashAttackSword3[1] = tm->GetSizedTileset(folder, "dash_att_03_sword_b_480x480.png");
	ts_dashAttackSword3[2] = tm->GetSizedTileset(folder, "dash_att_03_sword_p_480x480.png");

	ts_standAttackSword[0] = tm->GetSizedTileset(folder, "stand_att_01_sword_384x384.png");
	ts_standAttackSword[1] = tm->GetSizedTileset(folder, "stand_att_01_sword_b_480x256.png");
	ts_standAttackSword[2] = tm->GetSizedTileset(folder, "stand_att_01_sword_p_480x320.png");

	ts_standAttackSword2[0] = tm->GetSizedTileset(folder, "stand_att_02_sword_384x384.png");
	ts_standAttackSword2[1] = tm->GetSizedTileset(folder, "stand_att_02_sword_b_480x320.png");
	ts_standAttackSword2[2] = tm->GetSizedTileset(folder, "stand_att_02_sword_p_480x320.png");

	ts_standAttackSword3[0] = tm->GetSizedTileset(folder, "stand_att_03_sword_384x384.png");
	ts_standAttackSword3[1] = tm->GetSizedTileset(folder, "stand_att_03_sword_b_480x256.png");
	ts_standAttackSword3[2] = tm->GetSizedTileset(folder, "stand_att_03_sword_p_480x256.png");

	ts_standAttackSword4[0] = tm->GetSizedTileset(folder, "stand_att_04_sword_384x384.png");
	ts_standAttackSword4[1] = tm->GetSizedTileset(folder, "stand_att_04_sword_b_480x400.png");
	ts_standAttackSword4[2] = tm->GetSizedTileset(folder, "stand_att_04_sword_p_480x400.png");
}

void Actor::SetupExtraTilesets()
{
	string folder = "Kin/";

	string powerFolder = folder + "Powers/";
	
	string scorpionFolder = powerFolder + "Scorpion/";
	string grindFolder = powerFolder + "Grind/";
	string timeslowFolder = powerFolder + "TimeSlow/";
	string wireFolder = powerFolder + "Wire/";

	TilesetManager *tm = MainMenu::GetInstance();//sess->mainMenu;

	ts_mapIcon = tm->GetSizedTileset("HUD/Minimap/minimap_kin_64x64.png");
	ts_mapIcon->SetSpriteTexture(mapIconSpr);
	ts_mapIcon->SetSubRect(mapIconSpr, 0);
	//mapIconSpr.setScale(2, 2);
	mapIconSpr.setOrigin(mapIconSpr.getLocalBounds().width / 2, mapIconSpr.getLocalBounds().height / 2);


	ts_scorpRun = tm->GetSizedTileset(scorpionFolder, "scorp_run_192x128.png");
	ts_scorpSlide = tm->GetSizedTileset(scorpionFolder, "scorp_slide_160x128.png");
	ts_scorpSteepSlide = tm->GetSizedTileset(scorpionFolder, "scorp_steep_slide_224x128.png");
	ts_scorpStand = tm->GetSizedTileset(scorpionFolder, "scorp_stand_192x128.png");
	ts_scorpJump = tm->GetSizedTileset(scorpionFolder, "scorp_jump_192x192.png");
	ts_scorpDash = tm->GetSizedTileset(scorpionFolder, "scorp_dash_192x80.png");
	ts_scorpSprint = tm->GetSizedTileset(scorpionFolder, "scorp_sprint_256x128.png");
	ts_scorpClimb = tm->GetSizedTileset(scorpionFolder, "scorp_climb_224x128.png");
	ts_scorpBounce = tm->GetSizedTileset(scorpionFolder, "scorp_bounce_256x256.png");
	ts_scorpBounceWall = tm->GetSizedTileset(scorpionFolder, "scorp_bounce_wall_256x128.png");
	
	ts_bubble = tm->GetSizedTileset(timeslowFolder, "time_bubble_128x128.png");
	ts_dodecaSmall = tm->GetSizedTileset(timeslowFolder, "dodecasmall_180x180.png");
	ts_dodecaBig = tm->GetSizedTileset(timeslowFolder, "dodecabig_360x360.png");

	ts_grind = tm->GetSizedTileset(grindFolder, "grind_tri_128x128.png");
	ts_grindAttackFX = tm->GetSizedTileset(grindFolder, "grind_attack_192x192.png");

	ts_grindAttackFX->SetSpriteTexture(grindAttackSprite);

	ts_blockShield = tm->GetSizedTileset(powerFolder, "block_shield_64x64.png");
	ts_blockShield->SetSpriteTexture(shieldSprite);

	ts_swordProjectile = tm->GetSizedTileset("Enemies/General/comboers_128x128.png");

	ts_homingAttackBall = tm->GetSizedTileset("Kin/FX/homing_att_ball_256x256.png");
	ts_homingAttackBall->SetSpriteTexture(homingAttackBallSprite);

	ts_glideIndicator = tm->GetSizedTileset("Kin/FX/glide_ring_160x160.png");
	ts_glideIndicator->SetSpriteTexture(glideIndicatorSprite);

	ts_antiTimeSlowRing = tm->GetSizedTileset("Kin/FX/anti_timeslow_ring_128x128.png");

	ts_fx_boosterParticles = tm->GetSizedTileset("Kin/FX/booster_particles_32x32.png");

	ts_wire = tm->GetSizedTileset(wireFolder,"wires_16x16.png");
	ts_wireNode = tm->GetSizedTileset(wireFolder, "wire_node_16x16.png");
	ts_wireTip = tm->GetSizedTileset(wireFolder, "wire_tips_16x16.png");

	if (owner != NULL)
	{
		ts_exitAura = owner->mainMenu->GetSizedTileset("Kin/FX/exitaura_256x256.png");
		exitAuraSprite.setTexture(*ts_exitAura->texture);
	}
}

void Actor::SetupActionTilesets()
{
	//memset(tileset, 0, sizeof(tileset));
	for (int i = 0; i < Count; ++i)
	{
		tileset[i] = NULL;
	}

	Tileset *temp;
	const char *name;
	for (int i = 0; i < Count; ++i)
	{
		if (getTilesetNameFuncs[i] != NULL)
		{
			name = (this->*getTilesetNameFuncs[i])();
			if (name != NULL)
			{
				temp = GetActionTileset(name);
				if (temp != NULL)
				{
					tileset[i] = temp;
				}
			}
		}
	}
}

void Actor::UpdateGroundedShieldSprite( int tile)
{
	SetSpriteTexture(action);
	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(tile, r);

	SetGroundedSpriteTransform();

	Vector2f norm(ground->Normal());
	Vector2f test = sprite->getPosition() + norm * 32.f;


	ts_blockShield->SetSubRect(shieldSprite, tile, !facingRight, reversed);
	shieldSprite.setOrigin(shieldSprite.getLocalBounds().width / 2, shieldSprite.getLocalBounds().height / 2);//sprite->getOrigin());
	shieldSprite.setPosition(test);//position.x, position.y - 10);//sprite->getPosition());
	shieldSprite.setRotation(0);//sprite->getRotation());
}


void Actor::UpdateAerialShieldSprite(int tile)
{
	SetSpriteTexture(action);
	SetSpriteTile(tile, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	shieldSprite.setRotation(0);
	shieldSprite.setOrigin(sprite->getOrigin());
	shieldSprite.setPosition(sprite->getPosition());
	ts_blockShield->SetSubRect(shieldSprite, tile, !facingRight);

}

int Actor::GetActionLength(int a)
{
	if (getActionLengthFuncs[a] != NULL)
	{
		return (this->*getActionLengthFuncs[a])();
	}

	return 0;
}

void Actor::ActionTimeIndFrameInc()
{
	if (timeIndFrameIncFuncs[action] != NULL)
	{
		(this->*timeIndFrameIncFuncs[action])();
	}
}

void Actor::ActionTimeDepFrameInc()
{
	if (timeDepFrameIncFuncs[action] != NULL)
	{
		(this->*timeDepFrameIncFuncs[action])();
	}
}


void Actor::SetupTilesets()
{
	
	SetupSwordTilesets();
	SetupExtraTilesets();
	SetupActionTilesets();

	tileset[GOALKILL1] = GetActionTileset("goal_w01_killb_384x256.png");
	tileset[GOALKILL2] = GetActionTileset("goal_w01_killc_384x256.png");
	tileset[GOALKILL3] = GetActionTileset("goal_w01_killd_384x256.png");
	tileset[GOALKILL4] = GetActionTileset("goal_w01_kille_384x256.png");
}

void Actor::Init()
{
	if (owner != NULL && owner->boostEntrance )
	{
		SetAction(SPAWNWAIT);
		//SetAction(INTROBOOST);//INTRO
		frame = 0;
	}
	else
	{
		SetAction(SPAWNWAIT);
		frame = 0;
		//SetAction(INTROBOOST);//INTRO
		//frame = 0;
		/*SetAction(JUMP);
		frame = 1;*/
	}
}

void Actor::SetupFuncsForAction(
	int a,
	void(Actor::* start)(),
	void(Actor::* end) (),
	void(Actor::* change)(),
	void(Actor::* update)(),
	void(Actor::* updateSprite)(),
	void(Actor::* transitionToAction)(int),
	void(Actor::* timeIndInc)(),
	void(Actor::* timeDepInc)(),
	int(Actor::* getActionLength)(),
	const char *(Actor::* getTilesetName)() )
{
	startActionFuncs[a] = start;
	endActionFuncs[a] = end;
	changeActionFuncs[a] = change;
	updateActionFuncs[a] = update;
	updateSpriteFuncs[a] = updateSprite;
	transitionFuncs[a] = transitionToAction;
	timeIndFrameIncFuncs[a] = timeIndInc;
	timeDepFrameIncFuncs[a] = timeDepInc;
	getActionLengthFuncs[a] = getActionLength;
	getTilesetNameFuncs[a] = getTilesetName;
}

Tileset *Actor::GetActionTileset(const std::string &fn)
{
	TilesetManager *tm = MainMenu::GetInstance();//sess->mainMenu;
	return tm->GetSizedTileset(actionFolder, fn);
}

void Actor::SetupActionFunctions()
{
	startActionFuncs.resize(Count);
	endActionFuncs.resize(Count);
	changeActionFuncs.resize(Count);
	updateActionFuncs.resize(Count);
	updateSpriteFuncs.resize(Count);
	transitionFuncs.resize(Count);
	timeIndFrameIncFuncs.resize(Count);
	timeDepFrameIncFuncs.resize(Count);
	getActionLengthFuncs.resize(Count);
	getTilesetNameFuncs.resize(Count);

	SetupFuncsForAction(AIMWAIT,
		&Actor::AIMWAIT_Start,
		&Actor::AIMWAIT_End,
		&Actor::AIMWAIT_Change,
		&Actor::AIMWAIT_Update,
		&Actor::AIMWAIT_UpdateSprite,
		&Actor::AIMWAIT_TransitionToAction,
		&Actor::AIMWAIT_TimeIndFrameInc,
		&Actor::AIMWAIT_TimeDepFrameInc,
		&Actor::AIMWAIT_GetActionLength,
		&Actor::AIMWAIT_GetTilesetName);

	SetupFuncsForAction(AIRBLOCKUP,
		&Actor::AIRBLOCKUP_Start,
		&Actor::AIRBLOCKUP_End,
		&Actor::AIRBLOCKUP_Change,
		&Actor::AIRBLOCKUP_Update,
		&Actor::AIRBLOCKUP_UpdateSprite,
		&Actor::AIRBLOCKUP_TransitionToAction,
		&Actor::AIRBLOCKUP_TimeIndFrameInc,
		&Actor::AIRBLOCKUP_TimeDepFrameInc,
		&Actor::AIRBLOCKUP_GetActionLength,
		&Actor::AIRBLOCKUP_GetTilesetName);

	SetupFuncsForAction(AIRBLOCKUPFORWARD,
		&Actor::AIRBLOCKUPFORWARD_Start,
		&Actor::AIRBLOCKUPFORWARD_End,
		&Actor::AIRBLOCKUPFORWARD_Change,
		&Actor::AIRBLOCKUPFORWARD_Update,
		&Actor::AIRBLOCKUPFORWARD_UpdateSprite,
		&Actor::AIRBLOCKUPFORWARD_TransitionToAction,
		&Actor::AIRBLOCKUPFORWARD_TimeIndFrameInc,
		&Actor::AIRBLOCKUPFORWARD_TimeDepFrameInc,
		&Actor::AIRBLOCKUPFORWARD_GetActionLength,
		&Actor::AIRBLOCKUPFORWARD_GetTilesetName);

	SetupFuncsForAction(AIRBLOCKFORWARD,
		&Actor::AIRBLOCKFORWARD_Start,
		&Actor::AIRBLOCKFORWARD_End,
		&Actor::AIRBLOCKFORWARD_Change,
		&Actor::AIRBLOCKFORWARD_Update,
		&Actor::AIRBLOCKFORWARD_UpdateSprite,
		&Actor::AIRBLOCKFORWARD_TransitionToAction,
		&Actor::AIRBLOCKFORWARD_TimeIndFrameInc,
		&Actor::AIRBLOCKFORWARD_TimeDepFrameInc,
		&Actor::AIRBLOCKFORWARD_GetActionLength,
		&Actor::AIRBLOCKFORWARD_GetTilesetName);

	SetupFuncsForAction(AIRBLOCKDOWNFORWARD,
		&Actor::AIRBLOCKDOWNFORWARD_Start,
		&Actor::AIRBLOCKDOWNFORWARD_End,
		&Actor::AIRBLOCKDOWNFORWARD_Change,
		&Actor::AIRBLOCKDOWNFORWARD_Update,
		&Actor::AIRBLOCKDOWNFORWARD_UpdateSprite,
		&Actor::AIRBLOCKDOWNFORWARD_TransitionToAction,
		&Actor::AIRBLOCKDOWNFORWARD_TimeIndFrameInc,
		&Actor::AIRBLOCKDOWNFORWARD_TimeDepFrameInc,
		&Actor::AIRBLOCKDOWNFORWARD_GetActionLength,
		&Actor::AIRBLOCKDOWNFORWARD_GetTilesetName);

	SetupFuncsForAction(AIRBLOCKDOWN,
		&Actor::AIRBLOCKDOWN_Start,
		&Actor::AIRBLOCKDOWN_End,
		&Actor::AIRBLOCKDOWN_Change,
		&Actor::AIRBLOCKDOWN_Update,
		&Actor::AIRBLOCKDOWN_UpdateSprite,
		&Actor::AIRBLOCKDOWN_TransitionToAction,
		&Actor::AIRBLOCKDOWN_TimeIndFrameInc,
		&Actor::AIRBLOCKDOWN_TimeDepFrameInc,
		&Actor::AIRBLOCKDOWN_GetActionLength,
		&Actor::AIRBLOCKDOWN_GetTilesetName);

	SetupFuncsForAction(AIRDASH,
		&Actor::AIRDASH_Start,
		&Actor::AIRDASH_End,
		&Actor::AIRDASH_Change,
		&Actor::AIRDASH_Update,
		&Actor::AIRDASH_UpdateSprite,
		&Actor::AIRDASH_TransitionToAction,
		&Actor::AIRDASH_TimeIndFrameInc,
		&Actor::AIRDASH_TimeDepFrameInc,
		&Actor::AIRDASH_GetActionLength,
		&Actor::AIRDASH_GetTilesetName);

	SetupFuncsForAction(AIRDASHFORWARDATTACK,
		&Actor::AIRDASHFORWARDATTACK_Start,
		&Actor::AIRDASHFORWARDATTACK_End,
		&Actor::AIRDASHFORWARDATTACK_Change,
		&Actor::AIRDASHFORWARDATTACK_Update,
		&Actor::AIRDASHFORWARDATTACK_UpdateSprite,
		&Actor::AIRDASHFORWARDATTACK_TransitionToAction,
		&Actor::AIRDASHFORWARDATTACK_TimeIndFrameInc,
		&Actor::AIRDASHFORWARDATTACK_TimeDepFrameInc,
		&Actor::AIRDASHFORWARDATTACK_GetActionLength,
		&Actor::AIRDASHFORWARDATTACK_GetTilesetName);

	SetupFuncsForAction(AIRHITSTUN,
		&Actor::AIRHITSTUN_Start,
		&Actor::AIRHITSTUN_End,
		&Actor::AIRHITSTUN_Change,
		&Actor::AIRHITSTUN_Update,
		&Actor::AIRHITSTUN_UpdateSprite,
		&Actor::AIRHITSTUN_TransitionToAction,
		&Actor::AIRHITSTUN_TimeIndFrameInc,
		&Actor::AIRHITSTUN_TimeDepFrameInc,
		&Actor::AIRHITSTUN_GetActionLength,
		&Actor::AIRHITSTUN_GetTilesetName);

	SetupFuncsForAction(AIRPARRY,
		&Actor::AIRPARRY_Start,
		&Actor::AIRPARRY_End,
		&Actor::AIRPARRY_Change,
		&Actor::AIRPARRY_Update,
		&Actor::AIRPARRY_UpdateSprite,
		&Actor::AIRPARRY_TransitionToAction,
		&Actor::AIRPARRY_TimeIndFrameInc,
		&Actor::AIRPARRY_TimeDepFrameInc,
		&Actor::AIRPARRY_GetActionLength,
		&Actor::AIRPARRY_GetTilesetName);

	SetupFuncsForAction(AUTORUN,
		&Actor::AUTORUN_Start,
		&Actor::AUTORUN_End,
		&Actor::AUTORUN_Change,
		&Actor::AUTORUN_Update,
		&Actor::AUTORUN_UpdateSprite,
		&Actor::AUTORUN_TransitionToAction,
		&Actor::AUTORUN_TimeIndFrameInc,
		&Actor::AUTORUN_TimeDepFrameInc,
		&Actor::AUTORUN_GetActionLength,
		&Actor::AUTORUN_GetTilesetName);

	SetupFuncsForAction(BACKWARDSDOUBLE,
		&Actor::BACKWARDSDOUBLE_Start,
		&Actor::BACKWARDSDOUBLE_End,
		&Actor::BACKWARDSDOUBLE_Change,
		&Actor::BACKWARDSDOUBLE_Update,
		&Actor::BACKWARDSDOUBLE_UpdateSprite,
		&Actor::BACKWARDSDOUBLE_TransitionToAction,
		&Actor::BACKWARDSDOUBLE_TimeIndFrameInc,
		&Actor::BACKWARDSDOUBLE_TimeDepFrameInc,
		&Actor::BACKWARDSDOUBLE_GetActionLength,
		&Actor::BACKWARDSDOUBLE_GetTilesetName);

	SetupFuncsForAction(BOUNCEAIR,
		&Actor::BOUNCEAIR_Start,
		&Actor::BOUNCEAIR_End,
		&Actor::BOUNCEAIR_Change,
		&Actor::BOUNCEAIR_Update,
		&Actor::BOUNCEAIR_UpdateSprite,
		&Actor::BOUNCEAIR_TransitionToAction,
		&Actor::BOUNCEAIR_TimeIndFrameInc,
		&Actor::BOUNCEAIR_TimeDepFrameInc,
		&Actor::BOUNCEAIR_GetActionLength,
		&Actor::BOUNCEAIR_GetTilesetName);

	SetupFuncsForAction(BOOSTERBOUNCE,
		&Actor::BOOSTERBOUNCE_Start,
		&Actor::BOOSTERBOUNCE_End,
		&Actor::BOOSTERBOUNCE_Change,
		&Actor::BOOSTERBOUNCE_Update,
		&Actor::BOOSTERBOUNCE_UpdateSprite,
		&Actor::BOOSTERBOUNCE_TransitionToAction,
		&Actor::BOOSTERBOUNCE_TimeIndFrameInc,
		&Actor::BOOSTERBOUNCE_TimeDepFrameInc,
		&Actor::BOOSTERBOUNCE_GetActionLength,
		&Actor::BOOSTERBOUNCE_GetTilesetName);

	SetupFuncsForAction(BOOSTERBOUNCEGROUND,
		&Actor::BOOSTERBOUNCEGROUND_Start,
		&Actor::BOOSTERBOUNCEGROUND_End,
		&Actor::BOOSTERBOUNCEGROUND_Change,
		&Actor::BOOSTERBOUNCEGROUND_Update,
		&Actor::BOOSTERBOUNCEGROUND_UpdateSprite,
		&Actor::BOOSTERBOUNCEGROUND_TransitionToAction,
		&Actor::BOOSTERBOUNCEGROUND_TimeIndFrameInc,
		&Actor::BOOSTERBOUNCEGROUND_TimeDepFrameInc,
		&Actor::BOOSTERBOUNCEGROUND_GetActionLength,
		&Actor::BOOSTERBOUNCEGROUND_GetTilesetName);

	SetupFuncsForAction(BOUNCEGROUND,
		&Actor::BOUNCEGROUND_Start,
		&Actor::BOUNCEGROUND_End,
		&Actor::BOUNCEGROUND_Change,
		&Actor::BOUNCEGROUND_Update,
		&Actor::BOUNCEGROUND_UpdateSprite,
		&Actor::BOUNCEGROUND_TransitionToAction,
		&Actor::BOUNCEGROUND_TimeIndFrameInc,
		&Actor::BOUNCEGROUND_TimeDepFrameInc,
		&Actor::BOUNCEGROUND_GetActionLength,
		&Actor::BOUNCEGROUND_GetTilesetName);

	SetupFuncsForAction(BOUNCEGROUNDEDWALL,
		&Actor::BOUNCEGROUNDEDWALL_Start,
		&Actor::BOUNCEGROUNDEDWALL_End,
		&Actor::BOUNCEGROUNDEDWALL_Change,
		&Actor::BOUNCEGROUNDEDWALL_Update,
		&Actor::BOUNCEGROUNDEDWALL_UpdateSprite,
		&Actor::BOUNCEGROUNDEDWALL_TransitionToAction,
		&Actor::BOUNCEGROUNDEDWALL_TimeIndFrameInc,
		&Actor::BOUNCEGROUNDEDWALL_TimeDepFrameInc,
		&Actor::BOUNCEGROUNDEDWALL_GetActionLength,
		&Actor::BOUNCEGROUNDEDWALL_GetTilesetName);

	SetupFuncsForAction(BRAKE,
		&Actor::BRAKE_Start,
		&Actor::BRAKE_End,
		&Actor::BRAKE_Change,
		&Actor::BRAKE_Update,
		&Actor::BRAKE_UpdateSprite,
		&Actor::BRAKE_TransitionToAction,
		&Actor::BRAKE_TimeIndFrameInc,
		&Actor::BRAKE_TimeDepFrameInc,
		&Actor::BRAKE_GetActionLength,
		&Actor::BRAKE_GetTilesetName);

	SetupFuncsForAction(DAIR,
		&Actor::DAIR_Start,
		&Actor::DAIR_End,
		&Actor::DAIR_Change,
		&Actor::DAIR_Update,
		&Actor::DAIR_UpdateSprite,
		&Actor::DAIR_TransitionToAction,
		&Actor::DAIR_TimeIndFrameInc,
		&Actor::DAIR_TimeDepFrameInc,
		&Actor::DAIR_GetActionLength,
		&Actor::DAIR_GetTilesetName);

	SetupFuncsForAction(DASH,
		&Actor::DASH_Start,
		&Actor::DASH_End,
		&Actor::DASH_Change,
		&Actor::DASH_Update,
		&Actor::DASH_UpdateSprite,
		&Actor::DASH_TransitionToAction,
		&Actor::DASH_TimeIndFrameInc,
		&Actor::DASH_TimeDepFrameInc,
		&Actor::DASH_GetActionLength,
		&Actor::DASH_GetTilesetName);

	SetupFuncsForAction(DASHATTACK,
		&Actor::DASHATTACK_Start,
		&Actor::DASHATTACK_End,
		&Actor::DASHATTACK_Change,
		&Actor::DASHATTACK_Update,
		&Actor::DASHATTACK_UpdateSprite,
		&Actor::DASHATTACK_TransitionToAction,
		&Actor::DASHATTACK_TimeIndFrameInc,
		&Actor::DASHATTACK_TimeDepFrameInc,
		&Actor::DASHATTACK_GetActionLength,
		&Actor::DASHATTACK_GetTilesetName);

	SetupFuncsForAction(DASHATTACK2,
		&Actor::DASHATTACK2_Start,
		&Actor::DASHATTACK2_End,
		&Actor::DASHATTACK2_Change,
		&Actor::DASHATTACK2_Update,
		&Actor::DASHATTACK2_UpdateSprite,
		&Actor::DASHATTACK2_TransitionToAction,
		&Actor::DASHATTACK2_TimeIndFrameInc,
		&Actor::DASHATTACK2_TimeDepFrameInc,
		&Actor::DASHATTACK2_GetActionLength,
		&Actor::DASHATTACK2_GetTilesetName);

	SetupFuncsForAction(DASHATTACK3,
		&Actor::DASHATTACK3_Start,
		&Actor::DASHATTACK3_End,
		&Actor::DASHATTACK3_Change,
		&Actor::DASHATTACK3_Update,
		&Actor::DASHATTACK3_UpdateSprite,
		&Actor::DASHATTACK3_TransitionToAction,
		&Actor::DASHATTACK3_TimeIndFrameInc,
		&Actor::DASHATTACK3_TimeDepFrameInc,
		&Actor::DASHATTACK3_GetActionLength,
		&Actor::DASHATTACK3_GetTilesetName);

	SetupFuncsForAction(DEATH,
		&Actor::DEATH_Start,
		&Actor::DEATH_End,
		&Actor::DEATH_Change,
		&Actor::DEATH_Update,
		&Actor::DEATH_UpdateSprite,
		&Actor::DEATH_TransitionToAction,
		&Actor::DEATH_TimeIndFrameInc,
		&Actor::DEATH_TimeDepFrameInc,
		&Actor::DEATH_GetActionLength,
		&Actor::DEATH_GetTilesetName);

	SetupFuncsForAction(DIAGDOWNATTACK,
		&Actor::DIAGDOWNATTACK_Start,
		&Actor::DIAGDOWNATTACK_End,
		&Actor::DIAGDOWNATTACK_Change,
		&Actor::DIAGDOWNATTACK_Update,
		&Actor::DIAGDOWNATTACK_UpdateSprite,
		&Actor::DIAGDOWNATTACK_TransitionToAction,
		&Actor::DIAGDOWNATTACK_TimeIndFrameInc,
		&Actor::DIAGDOWNATTACK_TimeDepFrameInc,
		&Actor::DIAGDOWNATTACK_GetActionLength,
		&Actor::DIAGDOWNATTACK_GetTilesetName);

	SetupFuncsForAction(DIAGUPATTACK,
		&Actor::DIAGUPATTACK_Start,
		&Actor::DIAGUPATTACK_End,
		&Actor::DIAGUPATTACK_Change,
		&Actor::DIAGUPATTACK_Update,
		&Actor::DIAGUPATTACK_UpdateSprite,
		&Actor::DIAGUPATTACK_TransitionToAction,
		&Actor::DIAGUPATTACK_TimeIndFrameInc,
		&Actor::DIAGUPATTACK_TimeDepFrameInc,
		&Actor::DIAGUPATTACK_GetActionLength,
		&Actor::DIAGUPATTACK_GetTilesetName);

	SetupFuncsForAction(DOUBLE,
		&Actor::DOUBLE_Start,
		&Actor::DOUBLE_End,
		&Actor::DOUBLE_Change,
		&Actor::DOUBLE_Update,
		&Actor::DOUBLE_UpdateSprite,
		&Actor::DOUBLE_TransitionToAction,
		&Actor::DOUBLE_TimeIndFrameInc,
		&Actor::DOUBLE_TimeDepFrameInc,
		&Actor::DOUBLE_GetActionLength,
		&Actor::DOUBLE_GetTilesetName);

	SetupFuncsForAction(ENTERNEXUS1,
		&Actor::ENTERNEXUS1_Start,
		&Actor::ENTERNEXUS1_End,
		&Actor::ENTERNEXUS1_Change,
		&Actor::ENTERNEXUS1_Update,
		&Actor::ENTERNEXUS1_UpdateSprite,
		&Actor::ENTERNEXUS1_TransitionToAction,
		&Actor::ENTERNEXUS1_TimeIndFrameInc,
		&Actor::ENTERNEXUS1_TimeDepFrameInc,
		&Actor::ENTERNEXUS1_GetActionLength,
		&Actor::ENTERNEXUS1_GetTilesetName);

	SetupFuncsForAction(EXIT,
		&Actor::EXIT_Start,
		&Actor::EXIT_End,
		&Actor::EXIT_Change,
		&Actor::EXIT_Update,
		&Actor::EXIT_UpdateSprite,
		&Actor::EXIT_TransitionToAction,
		&Actor::EXIT_TimeIndFrameInc,
		&Actor::EXIT_TimeDepFrameInc,
		&Actor::EXIT_GetActionLength,
		&Actor::EXIT_GetTilesetName);

	SetupFuncsForAction(EXITBOOST,
		&Actor::EXITBOOST_Start,
		&Actor::EXITBOOST_End,
		&Actor::EXITBOOST_Change,
		&Actor::EXITBOOST_Update,
		&Actor::EXITBOOST_UpdateSprite,
		&Actor::EXITBOOST_TransitionToAction,
		&Actor::EXITBOOST_TimeIndFrameInc,
		&Actor::EXITBOOST_TimeDepFrameInc,
		&Actor::EXITBOOST_GetActionLength,
		&Actor::EXITBOOST_GetTilesetName);

	SetupFuncsForAction(EXITWAIT,
		&Actor::EXITWAIT_Start,
		&Actor::EXITWAIT_End,
		&Actor::EXITWAIT_Change,
		&Actor::EXITWAIT_Update,
		&Actor::EXITWAIT_UpdateSprite,
		&Actor::EXITWAIT_TransitionToAction,
		&Actor::EXITWAIT_TimeIndFrameInc,
		&Actor::EXITWAIT_TimeDepFrameInc,
		&Actor::EXITWAIT_GetActionLength,
		&Actor::EXITWAIT_GetTilesetName);

	SetupFuncsForAction(FAIR,
		&Actor::FAIR_Start,
		&Actor::FAIR_End,
		&Actor::FAIR_Change,
		&Actor::FAIR_Update,
		&Actor::FAIR_UpdateSprite,
		&Actor::FAIR_TransitionToAction,
		&Actor::FAIR_TimeIndFrameInc,
		&Actor::FAIR_TimeDepFrameInc,
		&Actor::FAIR_GetActionLength,
		&Actor::FAIR_GetTilesetName);

	SetupFuncsForAction(FREEFLIGHT,
		&Actor::FREEFLIGHT_Start,
		&Actor::FREEFLIGHT_End,
		&Actor::FREEFLIGHT_Change,
		&Actor::FREEFLIGHT_Update,
		&Actor::FREEFLIGHT_UpdateSprite,
		&Actor::FREEFLIGHT_TransitionToAction,
		&Actor::FREEFLIGHT_TimeIndFrameInc,
		&Actor::FREEFLIGHT_TimeDepFrameInc,
		&Actor::FREEFLIGHT_GetActionLength,
		&Actor::FREEFLIGHT_GetTilesetName);

	SetupFuncsForAction(FREEFLIGHTSTUN,
		&Actor::FREEFLIGHTSTUN_Start,
		&Actor::FREEFLIGHTSTUN_End,
		&Actor::FREEFLIGHTSTUN_Change,
		&Actor::FREEFLIGHTSTUN_Update,
		&Actor::FREEFLIGHTSTUN_UpdateSprite,
		&Actor::FREEFLIGHTSTUN_TransitionToAction,
		&Actor::FREEFLIGHTSTUN_TimeIndFrameInc,
		&Actor::FREEFLIGHTSTUN_TimeDepFrameInc,
		&Actor::FREEFLIGHTSTUN_GetActionLength,
		&Actor::FREEFLIGHTSTUN_GetTilesetName);

	SetupFuncsForAction(GETPOWER_AIRDASH_FLIP,
		&Actor::GETPOWER_AIRDASH_FLIP_Start,
		&Actor::GETPOWER_AIRDASH_FLIP_End,
		&Actor::GETPOWER_AIRDASH_FLIP_Change,
		&Actor::GETPOWER_AIRDASH_FLIP_Update,
		&Actor::GETPOWER_AIRDASH_FLIP_UpdateSprite,
		&Actor::GETPOWER_AIRDASH_FLIP_TransitionToAction,
		&Actor::GETPOWER_AIRDASH_FLIP_TimeIndFrameInc,
		&Actor::GETPOWER_AIRDASH_FLIP_TimeDepFrameInc,
		&Actor::GETPOWER_AIRDASH_FLIP_GetActionLength,
		&Actor::GETPOWER_AIRDASH_FLIP_GetTilesetName);

	SetupFuncsForAction(GETPOWER_AIRDASH_MEDITATE,
		&Actor::GETPOWER_AIRDASH_MEDITATE_Start,
		&Actor::GETPOWER_AIRDASH_MEDITATE_End,
		&Actor::GETPOWER_AIRDASH_MEDITATE_Change,
		&Actor::GETPOWER_AIRDASH_MEDITATE_Update,
		&Actor::GETPOWER_AIRDASH_MEDITATE_UpdateSprite,
		&Actor::GETPOWER_AIRDASH_MEDITATE_TransitionToAction,
		&Actor::GETPOWER_AIRDASH_MEDITATE_TimeIndFrameInc,
		&Actor::GETPOWER_AIRDASH_MEDITATE_TimeDepFrameInc,
		&Actor::GETPOWER_AIRDASH_MEDITATE_GetActionLength,
		&Actor::GETPOWER_AIRDASH_MEDITATE_GetTilesetName);

	SetupFuncsForAction(GETSHARD,
		&Actor::GETSHARD_Start,
		&Actor::GETSHARD_End,
		&Actor::GETSHARD_Change,
		&Actor::GETSHARD_Update,
		&Actor::GETSHARD_UpdateSprite,
		&Actor::GETSHARD_TransitionToAction,
		&Actor::GETSHARD_TimeIndFrameInc,
		&Actor::GETSHARD_TimeDepFrameInc,
		&Actor::GETSHARD_GetActionLength,
		&Actor::GETSHARD_GetTilesetName);

	SetupFuncsForAction(GLIDE,
		&Actor::GLIDE_Start,
		&Actor::GLIDE_End,
		&Actor::GLIDE_Change,
		&Actor::GLIDE_Update,
		&Actor::GLIDE_UpdateSprite,
		&Actor::GLIDE_TransitionToAction,
		&Actor::GLIDE_TimeIndFrameInc,
		&Actor::GLIDE_TimeDepFrameInc,
		&Actor::GLIDE_GetActionLength,
		&Actor::GLIDE_GetTilesetName);

	SetupFuncsForAction(GOALKILL,
		&Actor::GOALKILL_Start,
		&Actor::GOALKILL_End,
		&Actor::GOALKILL_Change,
		&Actor::GOALKILL_Update,
		&Actor::GOALKILL_UpdateSprite,
		&Actor::GOALKILL_TransitionToAction,
		&Actor::GOALKILL_TimeIndFrameInc,
		&Actor::GOALKILL_TimeDepFrameInc,
		&Actor::GOALKILL_GetActionLength,
		&Actor::GOALKILL_GetTilesetName);

	SetupFuncsForAction(GOALKILLWAIT,
		&Actor::GOALKILLWAIT_Start,
		&Actor::GOALKILLWAIT_End,
		&Actor::GOALKILLWAIT_Change,
		&Actor::GOALKILLWAIT_Update,
		&Actor::GOALKILLWAIT_UpdateSprite,
		&Actor::GOALKILLWAIT_TransitionToAction,
		&Actor::GOALKILLWAIT_TimeIndFrameInc,
		&Actor::GOALKILLWAIT_TimeDepFrameInc,
		&Actor::GOALKILLWAIT_GetActionLength,
		&Actor::GOALKILLWAIT_GetTilesetName);

	SetupFuncsForAction(GRABSHIP,
		&Actor::GRABSHIP_Start,
		&Actor::GRABSHIP_End,
		&Actor::GRABSHIP_Change,
		&Actor::GRABSHIP_Update,
		&Actor::GRABSHIP_UpdateSprite,
		&Actor::GRABSHIP_TransitionToAction,
		&Actor::GRABSHIP_TimeIndFrameInc,
		&Actor::GRABSHIP_TimeDepFrameInc,
		&Actor::GRABSHIP_GetActionLength,
		&Actor::GRABSHIP_GetTilesetName);

	SetupFuncsForAction(GRAVREVERSE,
		&Actor::GRAVREVERSE_Start,
		&Actor::GRAVREVERSE_End,
		&Actor::GRAVREVERSE_Change,
		&Actor::GRAVREVERSE_Update,
		&Actor::GRAVREVERSE_UpdateSprite,
		&Actor::GRAVREVERSE_TransitionToAction,
		&Actor::GRAVREVERSE_TimeIndFrameInc,
		&Actor::GRAVREVERSE_TimeDepFrameInc,
		&Actor::GRAVREVERSE_GetActionLength,
		&Actor::GRAVREVERSE_GetTilesetName);

	SetupFuncsForAction(GRINDATTACK,
		&Actor::GRINDATTACK_Start,
		&Actor::GRINDATTACK_End,
		&Actor::GRINDATTACK_Change,
		&Actor::GRINDATTACK_Update,
		&Actor::GRINDATTACK_UpdateSprite,
		&Actor::GRINDATTACK_TransitionToAction,
		&Actor::GRINDATTACK_TimeIndFrameInc,
		&Actor::GRINDATTACK_TimeDepFrameInc,
		&Actor::GRINDATTACK_GetActionLength,
		&Actor::GRINDATTACK_GetTilesetName);

	SetupFuncsForAction(GRINDBALL,
		&Actor::GRINDBALL_Start,
		&Actor::GRINDBALL_End,
		&Actor::GRINDBALL_Change,
		&Actor::GRINDBALL_Update,
		&Actor::GRINDBALL_UpdateSprite,
		&Actor::GRINDBALL_TransitionToAction,
		&Actor::GRINDBALL_TimeIndFrameInc,
		&Actor::GRINDBALL_TimeDepFrameInc,
		&Actor::GRINDBALL_GetActionLength,
		&Actor::GRINDBALL_GetTilesetName);

	SetupFuncsForAction(GRINDLUNGE,
		&Actor::GRINDLUNGE_Start,
		&Actor::GRINDLUNGE_End,
		&Actor::GRINDLUNGE_Change,
		&Actor::GRINDLUNGE_Update,
		&Actor::GRINDLUNGE_UpdateSprite,
		&Actor::GRINDLUNGE_TransitionToAction,
		&Actor::GRINDLUNGE_TimeIndFrameInc,
		&Actor::GRINDLUNGE_TimeDepFrameInc,
		&Actor::GRINDLUNGE_GetActionLength,
		&Actor::GRINDLUNGE_GetTilesetName);

	SetupFuncsForAction(GRINDSLASH,
		&Actor::GRINDSLASH_Start,
		&Actor::GRINDSLASH_End,
		&Actor::GRINDSLASH_Change,
		&Actor::GRINDSLASH_Update,
		&Actor::GRINDSLASH_UpdateSprite,
		&Actor::GRINDSLASH_TransitionToAction,
		&Actor::GRINDSLASH_TimeIndFrameInc,
		&Actor::GRINDSLASH_TimeDepFrameInc,
		&Actor::GRINDSLASH_GetActionLength,
		&Actor::GRINDSLASH_GetTilesetName);

	SetupFuncsForAction(GROUNDBLOCKDOWN,
		&Actor::GROUNDBLOCKDOWN_Start,
		&Actor::GROUNDBLOCKDOWN_End,
		&Actor::GROUNDBLOCKDOWN_Change,
		&Actor::GROUNDBLOCKDOWN_Update,
		&Actor::GROUNDBLOCKDOWN_UpdateSprite,
		&Actor::GROUNDBLOCKDOWN_TransitionToAction,
		&Actor::GROUNDBLOCKDOWN_TimeIndFrameInc,
		&Actor::GROUNDBLOCKDOWN_TimeDepFrameInc,
		&Actor::GROUNDBLOCKDOWN_GetActionLength,
		&Actor::GROUNDBLOCKDOWN_GetTilesetName);

	SetupFuncsForAction(GROUNDBLOCKDOWNFORWARD,
		&Actor::GROUNDBLOCKDOWNFORWARD_Start,
		&Actor::GROUNDBLOCKDOWNFORWARD_End,
		&Actor::GROUNDBLOCKDOWNFORWARD_Change,
		&Actor::GROUNDBLOCKDOWNFORWARD_Update,
		&Actor::GROUNDBLOCKDOWNFORWARD_UpdateSprite,
		&Actor::GROUNDBLOCKDOWNFORWARD_TransitionToAction,
		&Actor::GROUNDBLOCKDOWNFORWARD_TimeIndFrameInc,
		&Actor::GROUNDBLOCKDOWNFORWARD_TimeDepFrameInc,
		&Actor::GROUNDBLOCKDOWNFORWARD_GetActionLength,
		&Actor::GROUNDBLOCKDOWNFORWARD_GetTilesetName);

	SetupFuncsForAction(GROUNDBLOCKFORWARD,
		&Actor::GROUNDBLOCKFORWARD_Start,
		&Actor::GROUNDBLOCKFORWARD_End,
		&Actor::GROUNDBLOCKFORWARD_Change,
		&Actor::GROUNDBLOCKFORWARD_Update,
		&Actor::GROUNDBLOCKFORWARD_UpdateSprite,
		&Actor::GROUNDBLOCKFORWARD_TransitionToAction,
		&Actor::GROUNDBLOCKFORWARD_TimeIndFrameInc,
		&Actor::GROUNDBLOCKFORWARD_TimeDepFrameInc,
		&Actor::GROUNDBLOCKFORWARD_GetActionLength,
		&Actor::GROUNDBLOCKFORWARD_GetTilesetName);

	SetupFuncsForAction(GROUNDBLOCKUPFORWARD,
		&Actor::GROUNDBLOCKUPFORWARD_Start,
		&Actor::GROUNDBLOCKUPFORWARD_End,
		&Actor::GROUNDBLOCKUPFORWARD_Change,
		&Actor::GROUNDBLOCKUPFORWARD_Update,
		&Actor::GROUNDBLOCKUPFORWARD_UpdateSprite,
		&Actor::GROUNDBLOCKUPFORWARD_TransitionToAction,
		&Actor::GROUNDBLOCKUPFORWARD_TimeIndFrameInc,
		&Actor::GROUNDBLOCKUPFORWARD_TimeDepFrameInc,
		&Actor::GROUNDBLOCKUPFORWARD_GetActionLength,
		&Actor::GROUNDBLOCKUPFORWARD_GetTilesetName);

	SetupFuncsForAction(GROUNDBLOCKUP,
		&Actor::GROUNDBLOCKUP_Start,
		&Actor::GROUNDBLOCKUP_End,
		&Actor::GROUNDBLOCKUP_Change,
		&Actor::GROUNDBLOCKUP_Update,
		&Actor::GROUNDBLOCKUP_UpdateSprite,
		&Actor::GROUNDBLOCKUP_TransitionToAction,
		&Actor::GROUNDBLOCKUP_TimeIndFrameInc,
		&Actor::GROUNDBLOCKUP_TimeDepFrameInc,
		&Actor::GROUNDBLOCKUP_GetActionLength,
		&Actor::GROUNDBLOCKUP_GetTilesetName);

	SetupFuncsForAction(GROUNDHITSTUN,
		&Actor::GROUNDHITSTUN_Start,
		&Actor::GROUNDHITSTUN_End,
		&Actor::GROUNDHITSTUN_Change,
		&Actor::GROUNDHITSTUN_Update,
		&Actor::GROUNDHITSTUN_UpdateSprite,
		&Actor::GROUNDHITSTUN_TransitionToAction,
		&Actor::GROUNDHITSTUN_TimeIndFrameInc,
		&Actor::GROUNDHITSTUN_TimeDepFrameInc,
		&Actor::GROUNDHITSTUN_GetActionLength,
		&Actor::GROUNDHITSTUN_GetTilesetName);

	SetupFuncsForAction(GROUNDPARRY,
		&Actor::GROUNDPARRY_Start,
		&Actor::GROUNDPARRY_End,
		&Actor::GROUNDPARRY_Change,
		&Actor::GROUNDPARRY_Update,
		&Actor::GROUNDPARRY_UpdateSprite,
		&Actor::GROUNDPARRY_TransitionToAction,
		&Actor::GROUNDPARRY_TimeIndFrameInc,
		&Actor::GROUNDPARRY_TimeDepFrameInc,
		&Actor::GROUNDPARRY_GetActionLength,
		&Actor::GROUNDPARRY_GetTilesetName);

	SetupFuncsForAction(GROUNDPARRYLOW,
		&Actor::GROUNDPARRYLOW_Start,
		&Actor::GROUNDPARRYLOW_End,
		&Actor::GROUNDPARRYLOW_Change,
		&Actor::GROUNDPARRYLOW_Update,
		&Actor::GROUNDPARRYLOW_UpdateSprite,
		&Actor::GROUNDPARRYLOW_TransitionToAction,
		&Actor::GROUNDPARRYLOW_TimeIndFrameInc,
		&Actor::GROUNDPARRYLOW_TimeDepFrameInc,
		&Actor::GROUNDPARRYLOW_GetActionLength,
		&Actor::GROUNDPARRYLOW_GetTilesetName);

	SetupFuncsForAction(GROUNDTECHBACK,
		&Actor::GROUNDTECHBACK_Start,
		&Actor::GROUNDTECHBACK_End,
		&Actor::GROUNDTECHBACK_Change,
		&Actor::GROUNDTECHBACK_Update,
		&Actor::GROUNDTECHBACK_UpdateSprite,
		&Actor::GROUNDTECHBACK_TransitionToAction,
		&Actor::GROUNDTECHBACK_TimeIndFrameInc,
		&Actor::GROUNDTECHBACK_TimeDepFrameInc,
		&Actor::GROUNDTECHBACK_GetActionLength,
		&Actor::GROUNDTECHBACK_GetTilesetName);

	SetupFuncsForAction(GROUNDTECHFORWARD,
		&Actor::GROUNDTECHFORWARD_Start,
		&Actor::GROUNDTECHFORWARD_End,
		&Actor::GROUNDTECHFORWARD_Change,
		&Actor::GROUNDTECHFORWARD_Update,
		&Actor::GROUNDTECHFORWARD_UpdateSprite,
		&Actor::GROUNDTECHFORWARD_TransitionToAction,
		&Actor::GROUNDTECHFORWARD_TimeIndFrameInc,
		&Actor::GROUNDTECHFORWARD_TimeDepFrameInc,
		&Actor::GROUNDTECHFORWARD_GetActionLength,
		&Actor::GROUNDTECHFORWARD_GetTilesetName);

	SetupFuncsForAction(GROUNDTECHINPLACE,
		&Actor::GROUNDTECHINPLACE_Start,
		&Actor::GROUNDTECHINPLACE_End,
		&Actor::GROUNDTECHINPLACE_Change,
		&Actor::GROUNDTECHINPLACE_Update,
		&Actor::GROUNDTECHINPLACE_UpdateSprite,
		&Actor::GROUNDTECHINPLACE_TransitionToAction,
		&Actor::GROUNDTECHINPLACE_TimeIndFrameInc,
		&Actor::GROUNDTECHINPLACE_TimeDepFrameInc,
		&Actor::GROUNDTECHINPLACE_GetActionLength,
		&Actor::GROUNDTECHINPLACE_GetTilesetName);

	SetupFuncsForAction(HIDDEN,
		&Actor::HIDDEN_Start,
		&Actor::HIDDEN_End,
		&Actor::HIDDEN_Change,
		&Actor::HIDDEN_Update,
		&Actor::HIDDEN_UpdateSprite,
		&Actor::HIDDEN_TransitionToAction,
		&Actor::HIDDEN_TimeIndFrameInc,
		&Actor::HIDDEN_TimeDepFrameInc,
		&Actor::HIDDEN_GetActionLength,
		&Actor::HIDDEN_GetTilesetName);

	SetupFuncsForAction(HOMINGATTACK,
		&Actor::HOMINGATTACK_Start,
		&Actor::HOMINGATTACK_End,
		&Actor::HOMINGATTACK_Change,
		&Actor::HOMINGATTACK_Update,
		&Actor::HOMINGATTACK_UpdateSprite,
		&Actor::HOMINGATTACK_TransitionToAction,
		&Actor::HOMINGATTACK_TimeIndFrameInc,
		&Actor::HOMINGATTACK_TimeDepFrameInc,
		&Actor::HOMINGATTACK_GetActionLength,
		&Actor::HOMINGATTACK_GetTilesetName);

	SetupFuncsForAction(INSPECT_END,
		&Actor::INSPECT_END_Start,
		&Actor::INSPECT_END_End,
		&Actor::INSPECT_END_Change,
		&Actor::INSPECT_END_Update,
		&Actor::INSPECT_END_UpdateSprite,
		&Actor::INSPECT_END_TransitionToAction,
		&Actor::INSPECT_END_TimeIndFrameInc,
		&Actor::INSPECT_END_TimeDepFrameInc,
		&Actor::INSPECT_END_GetActionLength,
		&Actor::INSPECT_END_GetTilesetName);

	SetupFuncsForAction(INSPECT_START,
		&Actor::INSPECT_START_Start,
		&Actor::INSPECT_START_End,
		&Actor::INSPECT_START_Change,
		&Actor::INSPECT_START_Update,
		&Actor::INSPECT_START_UpdateSprite,
		&Actor::INSPECT_START_TransitionToAction,
		&Actor::INSPECT_START_TimeIndFrameInc,
		&Actor::INSPECT_START_TimeDepFrameInc,
		&Actor::INSPECT_START_GetActionLength,
		&Actor::INSPECT_START_GetTilesetName);

	SetupFuncsForAction(INTRO,
		&Actor::INTRO_Start,
		&Actor::INTRO_End,
		&Actor::INTRO_Change,
		&Actor::INTRO_Update,
		&Actor::INTRO_UpdateSprite,
		&Actor::INTRO_TransitionToAction,
		&Actor::INTRO_TimeIndFrameInc,
		&Actor::INTRO_TimeDepFrameInc,
		&Actor::INTRO_GetActionLength,
		&Actor::INTRO_GetTilesetName);

	SetupFuncsForAction(INTROBOOST,
		&Actor::INTROBOOST_Start,
		&Actor::INTROBOOST_End,
		&Actor::INTROBOOST_Change,
		&Actor::INTROBOOST_Update,
		&Actor::INTROBOOST_UpdateSprite,
		&Actor::INTROBOOST_TransitionToAction,
		&Actor::INTROBOOST_TimeIndFrameInc,
		&Actor::INTROBOOST_TimeDepFrameInc,
		&Actor::INTROBOOST_GetActionLength,
		&Actor::INTROBOOST_GetTilesetName);

	SetupFuncsForAction(JUMP,
		&Actor::JUMP_Start,
		&Actor::JUMP_End,
		&Actor::JUMP_Change,
		&Actor::JUMP_Update,
		&Actor::JUMP_UpdateSprite,
		&Actor::JUMP_TransitionToAction,
		&Actor::JUMP_TimeIndFrameInc,
		&Actor::JUMP_TimeDepFrameInc,
		&Actor::JUMP_GetActionLength,
		&Actor::JUMP_GetTilesetName);

	SetupFuncsForAction(JUMPSQUAT,
		&Actor::JUMPSQUAT_Start,
		&Actor::JUMPSQUAT_End,
		&Actor::JUMPSQUAT_Change,
		&Actor::JUMPSQUAT_Update,
		&Actor::JUMPSQUAT_UpdateSprite,
		&Actor::JUMPSQUAT_TransitionToAction,
		&Actor::JUMPSQUAT_TimeIndFrameInc,
		&Actor::JUMPSQUAT_TimeDepFrameInc,
		&Actor::JUMPSQUAT_GetActionLength,
		&Actor::JUMPSQUAT_GetTilesetName);

	SetupFuncsForAction(GLIDE,
		&Actor::GLIDE_Start,
		&Actor::GLIDE_End,
		&Actor::GLIDE_Change,
		&Actor::GLIDE_Update,
		&Actor::GLIDE_UpdateSprite,
		&Actor::GLIDE_TransitionToAction,
		&Actor::GLIDE_TimeIndFrameInc,
		&Actor::GLIDE_TimeDepFrameInc,
		&Actor::GLIDE_GetActionLength,
		&Actor::GLIDE_GetTilesetName);

	SetupFuncsForAction(LAND,
		&Actor::LAND_Start,
		&Actor::LAND_End,
		&Actor::LAND_Change,
		&Actor::LAND_Update,
		&Actor::LAND_UpdateSprite,
		&Actor::LAND_TransitionToAction,
		&Actor::LAND_TimeIndFrameInc,
		&Actor::LAND_TimeDepFrameInc,
		&Actor::LAND_GetActionLength,
		&Actor::LAND_GetTilesetName);

	SetupFuncsForAction(LAND2,
		&Actor::LAND2_Start,
		&Actor::LAND2_End,
		&Actor::LAND2_Change,
		&Actor::LAND2_Update,
		&Actor::LAND2_UpdateSprite,
		&Actor::LAND2_TransitionToAction,
		&Actor::LAND2_TimeIndFrameInc,
		&Actor::LAND2_TimeDepFrameInc,
		&Actor::LAND2_GetActionLength,
		&Actor::LAND2_GetTilesetName);

	SetupFuncsForAction(LOCKEDRAILSLIDE,
		&Actor::LOCKEDRAILSLIDE_Start,
		&Actor::LOCKEDRAILSLIDE_End,
		&Actor::LOCKEDRAILSLIDE_Change,
		&Actor::LOCKEDRAILSLIDE_Update,
		&Actor::LOCKEDRAILSLIDE_UpdateSprite,
		&Actor::LOCKEDRAILSLIDE_TransitionToAction,
		&Actor::LOCKEDRAILSLIDE_TimeIndFrameInc,
		&Actor::LOCKEDRAILSLIDE_TimeDepFrameInc,
		&Actor::LOCKEDRAILSLIDE_GetActionLength,
		&Actor::LOCKEDRAILSLIDE_GetTilesetName);

	SetupFuncsForAction(NEXUSKILL,
		&Actor::NEXUSKILL_Start,
		&Actor::NEXUSKILL_End,
		&Actor::NEXUSKILL_Change,
		&Actor::NEXUSKILL_Update,
		&Actor::NEXUSKILL_UpdateSprite,
		&Actor::NEXUSKILL_TransitionToAction,
		&Actor::NEXUSKILL_TimeIndFrameInc,
		&Actor::NEXUSKILL_TimeDepFrameInc,
		&Actor::NEXUSKILL_GetActionLength,
		&Actor::NEXUSKILL_GetTilesetName);

	SetupFuncsForAction(RAILBOUNCE,
		&Actor::RAILBOUNCE_Start,
		&Actor::RAILBOUNCE_End,
		&Actor::RAILBOUNCE_Change,
		&Actor::RAILBOUNCE_Update,
		&Actor::RAILBOUNCE_UpdateSprite,
		&Actor::RAILBOUNCE_TransitionToAction,
		&Actor::RAILBOUNCE_TimeIndFrameInc,
		&Actor::RAILBOUNCE_TimeDepFrameInc,
		&Actor::RAILBOUNCE_GetActionLength,
		&Actor::RAILBOUNCE_GetTilesetName);

	SetupFuncsForAction(RAILBOUNCEGROUND,
		&Actor::RAILBOUNCEGROUND_Start,
		&Actor::RAILBOUNCEGROUND_End,
		&Actor::RAILBOUNCEGROUND_Change,
		&Actor::RAILBOUNCEGROUND_Update,
		&Actor::RAILBOUNCEGROUND_UpdateSprite,
		&Actor::RAILBOUNCEGROUND_TransitionToAction,
		&Actor::RAILBOUNCEGROUND_TimeIndFrameInc,
		&Actor::RAILBOUNCEGROUND_TimeDepFrameInc,
		&Actor::RAILBOUNCEGROUND_GetActionLength,
		&Actor::RAILBOUNCEGROUND_GetTilesetName);

	SetupFuncsForAction(RAILDASH,
		&Actor::RAILDASH_Start,
		&Actor::RAILDASH_End,
		&Actor::RAILDASH_Change,
		&Actor::RAILDASH_Update,
		&Actor::RAILDASH_UpdateSprite,
		&Actor::RAILDASH_TransitionToAction,
		&Actor::RAILDASH_TimeIndFrameInc,
		&Actor::RAILDASH_TimeDepFrameInc,
		&Actor::RAILDASH_GetActionLength,
		&Actor::RAILDASH_GetTilesetName);

	SetupFuncsForAction(RAILGRIND,
		&Actor::RAILGRIND_Start,
		&Actor::RAILGRIND_End,
		&Actor::RAILGRIND_Change,
		&Actor::RAILGRIND_Update,
		&Actor::RAILGRIND_UpdateSprite,
		&Actor::RAILGRIND_TransitionToAction,
		&Actor::RAILGRIND_TimeIndFrameInc,
		&Actor::RAILGRIND_TimeDepFrameInc,
		&Actor::RAILGRIND_GetActionLength,
		&Actor::RAILGRIND_GetTilesetName);

	SetupFuncsForAction(RAILSLIDE,
		&Actor::RAILSLIDE_Start,
		&Actor::RAILSLIDE_End,
		&Actor::RAILSLIDE_Change,
		&Actor::RAILSLIDE_Update,
		&Actor::RAILSLIDE_UpdateSprite,
		&Actor::RAILSLIDE_TransitionToAction,
		&Actor::RAILSLIDE_TimeIndFrameInc,
		&Actor::RAILSLIDE_TimeDepFrameInc,
		&Actor::RAILSLIDE_GetActionLength,
		&Actor::RAILSLIDE_GetTilesetName);

	SetupFuncsForAction(RIDESHIP,
		&Actor::RIDESHIP_Start,
		&Actor::RIDESHIP_End,
		&Actor::RIDESHIP_Change,
		&Actor::RIDESHIP_Update,
		&Actor::RIDESHIP_UpdateSprite,
		&Actor::RIDESHIP_TransitionToAction,
		&Actor::RIDESHIP_TimeIndFrameInc,
		&Actor::RIDESHIP_TimeDepFrameInc,
		&Actor::RIDESHIP_GetActionLength,
		&Actor::RIDESHIP_GetTilesetName);

	SetupFuncsForAction(RUN,
		&Actor::RUN_Start,
		&Actor::RUN_End,
		&Actor::RUN_Change,
		&Actor::RUN_Update,
		&Actor::RUN_UpdateSprite,
		&Actor::RUN_TransitionToAction,
		&Actor::RUN_TimeIndFrameInc,
		&Actor::RUN_TimeDepFrameInc,
		&Actor::RUN_GetActionLength,
		&Actor::RUN_GetTilesetName);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_DODGEBACK,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Start,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_End,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Change,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Update,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_DODGEBACK_GetActionLength,
		&Actor::AIRDASH_GetTilesetName);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_LAND,
		&Actor::SEQ_CRAWLERFIGHT_LAND_Start,
		&Actor::SEQ_CRAWLERFIGHT_LAND_End,
		&Actor::SEQ_CRAWLERFIGHT_LAND_Change,
		&Actor::SEQ_CRAWLERFIGHT_LAND_Update,
		&Actor::SEQ_CRAWLERFIGHT_LAND_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_LAND_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_LAND_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_LAND_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_LAND_GetActionLength,
		&Actor::AIRDASH_GetTilesetName);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_STAND,
		&Actor::SEQ_CRAWLERFIGHT_STAND_Start,
		&Actor::SEQ_CRAWLERFIGHT_STAND_End,
		&Actor::SEQ_CRAWLERFIGHT_STAND_Change,
		&Actor::SEQ_CRAWLERFIGHT_STAND_Update,
		&Actor::SEQ_CRAWLERFIGHT_STAND_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_STAND_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_STAND_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STAND_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STAND_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_STAND_GetTilesetName);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_STRAIGHTFALL,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Start,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_End,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Change,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Update,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetTilesetName);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Start,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_End,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Change,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Update,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetTilesetName);

	SetupFuncsForAction(SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Start,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_End,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Change,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Update,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_UpdateSprite,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TransitionToAction,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeIndFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeDepFrameInc,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetActionLength,
		&Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetTilesetName);

	SetupFuncsForAction(SEQ_ENTERCORE1,
		&Actor::SEQ_ENTERCORE1_Start,
		&Actor::SEQ_ENTERCORE1_End,
		&Actor::SEQ_ENTERCORE1_Change,
		&Actor::SEQ_ENTERCORE1_Update,
		&Actor::SEQ_ENTERCORE1_UpdateSprite,
		&Actor::SEQ_ENTERCORE1_TransitionToAction,
		&Actor::SEQ_ENTERCORE1_TimeIndFrameInc,
		&Actor::SEQ_ENTERCORE1_TimeDepFrameInc,
		&Actor::SEQ_ENTERCORE1_GetActionLength,
		&Actor::SEQ_ENTERCORE1_GetTilesetName);

	SetupFuncsForAction(SEQ_FADE_INTO_NEXUS,
		&Actor::SEQ_FADE_INTO_NEXUS_Start,
		&Actor::SEQ_FADE_INTO_NEXUS_End,
		&Actor::SEQ_FADE_INTO_NEXUS_Change,
		&Actor::SEQ_FADE_INTO_NEXUS_Update,
		&Actor::SEQ_FADE_INTO_NEXUS_UpdateSprite,
		&Actor::SEQ_FADE_INTO_NEXUS_TransitionToAction,
		&Actor::SEQ_FADE_INTO_NEXUS_TimeIndFrameInc,
		&Actor::SEQ_FADE_INTO_NEXUS_TimeDepFrameInc,
		&Actor::SEQ_FADE_INTO_NEXUS_GetActionLength,
		&Actor::SEQ_FADE_INTO_NEXUS_GetTilesetName);

	SetupFuncsForAction(SEQ_FLOAT_TO_NEXUS_OPENING,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Start,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_End,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Change,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Update,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_UpdateSprite,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_TransitionToAction,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_TimeIndFrameInc,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_TimeDepFrameInc,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_GetActionLength,
		&Actor::SEQ_FLOAT_TO_NEXUS_OPENING_GetTilesetName);

	SetupFuncsForAction(SEQ_GATORSTUN,
		&Actor::SEQ_GATORSTUN_Start,
		&Actor::SEQ_GATORSTUN_End,
		&Actor::SEQ_GATORSTUN_Change,
		&Actor::SEQ_GATORSTUN_Update,
		&Actor::SEQ_GATORSTUN_UpdateSprite,
		&Actor::SEQ_GATORSTUN_TransitionToAction,
		&Actor::SEQ_GATORSTUN_TimeIndFrameInc,
		&Actor::SEQ_GATORSTUN_TimeDepFrameInc,
		&Actor::SEQ_GATORSTUN_GetActionLength,
		&Actor::SEQ_GATORSTUN_GetTilesetName);

	SetupFuncsForAction(SEQ_KINFALL,
		&Actor::SEQ_KINFALL_Start,
		&Actor::SEQ_KINFALL_End,
		&Actor::SEQ_KINFALL_Change,
		&Actor::SEQ_KINFALL_Update,
		&Actor::SEQ_KINFALL_UpdateSprite,
		&Actor::SEQ_KINFALL_TransitionToAction,
		&Actor::SEQ_KINFALL_TimeIndFrameInc,
		&Actor::SEQ_KINFALL_TimeDepFrameInc,
		&Actor::SEQ_KINFALL_GetActionLength,
		&Actor::SEQ_KINFALL_GetTilesetName);

	SetupFuncsForAction(SEQ_KINSTAND,
		&Actor::SEQ_KINSTAND_Start,
		&Actor::SEQ_KINSTAND_End,
		&Actor::SEQ_KINSTAND_Change,
		&Actor::SEQ_KINSTAND_Update,
		&Actor::SEQ_KINSTAND_UpdateSprite,
		&Actor::SEQ_KINSTAND_TransitionToAction,
		&Actor::SEQ_KINSTAND_TimeIndFrameInc,
		&Actor::SEQ_KINSTAND_TimeDepFrameInc,
		&Actor::SEQ_KINSTAND_GetActionLength,
		&Actor::SEQ_KINSTAND_GetTilesetName);

	SetupFuncsForAction(SEQ_KINTHROWN,
		&Actor::SEQ_KINTHROWN_Start,
		&Actor::SEQ_KINTHROWN_End,
		&Actor::SEQ_KINTHROWN_Change,
		&Actor::SEQ_KINTHROWN_Update,
		&Actor::SEQ_KINTHROWN_UpdateSprite,
		&Actor::SEQ_KINTHROWN_TransitionToAction,
		&Actor::SEQ_KINTHROWN_TimeIndFrameInc,
		&Actor::SEQ_KINTHROWN_TimeDepFrameInc,
		&Actor::SEQ_KINTHROWN_GetActionLength,
		&Actor::SEQ_KINTHROWN_GetTilesetName);

	SetupFuncsForAction(SEQ_KNEEL,
		&Actor::SEQ_KNEEL_Start,
		&Actor::SEQ_KNEEL_End,
		&Actor::SEQ_KNEEL_Change,
		&Actor::SEQ_KNEEL_Update,
		&Actor::SEQ_KNEEL_UpdateSprite,
		&Actor::SEQ_KNEEL_TransitionToAction,
		&Actor::SEQ_KNEEL_TimeIndFrameInc,
		&Actor::SEQ_KNEEL_TimeDepFrameInc,
		&Actor::SEQ_KNEEL_GetActionLength,
		&Actor::SEQ_KNEEL_GetTilesetName);

	SetupFuncsForAction(SEQ_KNEEL_TO_MEDITATE,
		&Actor::SEQ_KNEEL_TO_MEDITATE_Start,
		&Actor::SEQ_KNEEL_TO_MEDITATE_End,
		&Actor::SEQ_KNEEL_TO_MEDITATE_Change,
		&Actor::SEQ_KNEEL_TO_MEDITATE_Update,
		&Actor::SEQ_KNEEL_TO_MEDITATE_UpdateSprite,
		&Actor::SEQ_KNEEL_TO_MEDITATE_TransitionToAction,
		&Actor::SEQ_KNEEL_TO_MEDITATE_TimeIndFrameInc,
		&Actor::SEQ_KNEEL_TO_MEDITATE_TimeDepFrameInc,
		&Actor::SEQ_KNEEL_TO_MEDITATE_GetActionLength,
		&Actor::SEQ_KNEEL_TO_MEDITATE_GetTilesetName);

	SetupFuncsForAction(SEQ_LOOKUP,
		&Actor::SEQ_LOOKUP_Start,
		&Actor::SEQ_LOOKUP_End,
		&Actor::SEQ_LOOKUP_Change,
		&Actor::SEQ_LOOKUP_Update,
		&Actor::SEQ_LOOKUP_UpdateSprite,
		&Actor::SEQ_LOOKUP_TransitionToAction,
		&Actor::SEQ_LOOKUP_TimeIndFrameInc,
		&Actor::SEQ_LOOKUP_TimeDepFrameInc,
		&Actor::SEQ_LOOKUP_GetActionLength,
		&Actor::SEQ_LOOKUP_GetTilesetName);

	SetupFuncsForAction(SEQ_LOOKUPDISAPPEAR,
		&Actor::SEQ_LOOKUPDISAPPEAR_Start,
		&Actor::SEQ_LOOKUPDISAPPEAR_End,
		&Actor::SEQ_LOOKUPDISAPPEAR_Change,
		&Actor::SEQ_LOOKUPDISAPPEAR_Update,
		&Actor::SEQ_LOOKUPDISAPPEAR_UpdateSprite,
		&Actor::SEQ_LOOKUPDISAPPEAR_TransitionToAction,
		&Actor::SEQ_LOOKUPDISAPPEAR_TimeIndFrameInc,
		&Actor::SEQ_LOOKUPDISAPPEAR_TimeDepFrameInc,
		&Actor::SEQ_LOOKUPDISAPPEAR_GetActionLength,
		&Actor::SEQ_LOOKUPDISAPPEAR_GetTilesetName);

	SetupFuncsForAction(SEQ_MASKOFF,
		&Actor::SEQ_MASKOFF_Start,
		&Actor::SEQ_MASKOFF_End,
		&Actor::SEQ_MASKOFF_Change,
		&Actor::SEQ_MASKOFF_Update,
		&Actor::SEQ_MASKOFF_UpdateSprite,
		&Actor::SEQ_MASKOFF_TransitionToAction,
		&Actor::SEQ_MASKOFF_TimeIndFrameInc,
		&Actor::SEQ_MASKOFF_TimeDepFrameInc,
		&Actor::SEQ_MASKOFF_GetActionLength,
		&Actor::SEQ_MASKOFF_GetTilesetName);

	SetupFuncsForAction(SEQ_MEDITATE,
		&Actor::SEQ_MEDITATE_Start,
		&Actor::SEQ_MEDITATE_End,
		&Actor::SEQ_MEDITATE_Change,
		&Actor::SEQ_MEDITATE_Update,
		&Actor::SEQ_MEDITATE_UpdateSprite,
		&Actor::SEQ_MEDITATE_TransitionToAction,
		&Actor::SEQ_MEDITATE_TimeIndFrameInc,
		&Actor::SEQ_MEDITATE_TimeDepFrameInc,
		&Actor::SEQ_MEDITATE_GetActionLength,
		&Actor::SEQ_MEDITATE_GetTilesetName);

	SetupFuncsForAction(SEQ_MEDITATE_MASKON,
		&Actor::SEQ_MEDITATE_MASKON_Start,
		&Actor::SEQ_MEDITATE_MASKON_End,
		&Actor::SEQ_MEDITATE_MASKON_Change,
		&Actor::SEQ_MEDITATE_MASKON_Update,
		&Actor::SEQ_MEDITATE_MASKON_UpdateSprite,
		&Actor::SEQ_MEDITATE_MASKON_TransitionToAction,
		&Actor::SEQ_MEDITATE_MASKON_TimeIndFrameInc,
		&Actor::SEQ_MEDITATE_MASKON_TimeDepFrameInc,
		&Actor::SEQ_MEDITATE_MASKON_GetActionLength,
		&Actor::SEQ_MEDITATE_MASKON_GetTilesetName);

	SetupFuncsForAction(SEQ_TURNFACE,
		&Actor::SEQ_TURNFACE_Start,
		&Actor::SEQ_TURNFACE_End,
		&Actor::SEQ_TURNFACE_Change,
		&Actor::SEQ_TURNFACE_Update,
		&Actor::SEQ_TURNFACE_UpdateSprite,
		&Actor::SEQ_TURNFACE_TransitionToAction,
		&Actor::SEQ_TURNFACE_TimeIndFrameInc,
		&Actor::SEQ_TURNFACE_TimeDepFrameInc,
		&Actor::SEQ_TURNFACE_GetActionLength,
		&Actor::SEQ_TURNFACE_GetTilesetName);

	SetupFuncsForAction(SEQ_WAIT,
		&Actor::SEQ_WAIT_Start,
		&Actor::SEQ_WAIT_End,
		&Actor::SEQ_WAIT_Change,
		&Actor::SEQ_WAIT_Update,
		&Actor::SEQ_WAIT_UpdateSprite,
		&Actor::SEQ_WAIT_TransitionToAction,
		&Actor::SEQ_WAIT_TimeIndFrameInc,
		&Actor::SEQ_WAIT_TimeDepFrameInc,
		&Actor::SEQ_WAIT_GetActionLength,
		&Actor::SEQ_WAIT_GetTilesetName);

	SetupFuncsForAction(SKYDIVE,
		&Actor::SKYDIVE_Start,
		&Actor::SKYDIVE_End,
		&Actor::SKYDIVE_Change,
		&Actor::SKYDIVE_Update,
		&Actor::SKYDIVE_UpdateSprite,
		&Actor::SKYDIVE_TransitionToAction,
		&Actor::SKYDIVE_TimeIndFrameInc,
		&Actor::SKYDIVE_TimeDepFrameInc,
		&Actor::SKYDIVE_GetActionLength,
		&Actor::SKYDIVE_GetTilesetName);

	SetupFuncsForAction(SKYDIVETOFALL,
		&Actor::SKYDIVETOFALL_Start,
		&Actor::SKYDIVETOFALL_End,
		&Actor::SKYDIVETOFALL_Change,
		&Actor::SKYDIVETOFALL_Update,
		&Actor::SKYDIVETOFALL_UpdateSprite,
		&Actor::SKYDIVETOFALL_TransitionToAction,
		&Actor::SKYDIVETOFALL_TimeIndFrameInc,
		&Actor::SKYDIVETOFALL_TimeDepFrameInc,
		&Actor::SKYDIVETOFALL_GetActionLength,
		&Actor::SKYDIVETOFALL_GetTilesetName);

	SetupFuncsForAction(SLIDE,
		&Actor::SLIDE_Start,
		&Actor::SLIDE_End,
		&Actor::SLIDE_Change,
		&Actor::SLIDE_Update,
		&Actor::SLIDE_UpdateSprite,
		&Actor::SLIDE_TransitionToAction,
		&Actor::SLIDE_TimeIndFrameInc,
		&Actor::SLIDE_TimeDepFrameInc,
		&Actor::SLIDE_GetActionLength,
		&Actor::SLIDE_GetTilesetName);

	SetupFuncsForAction(SPAWNWAIT,
		&Actor::SPAWNWAIT_Start,
		&Actor::SPAWNWAIT_End,
		&Actor::SPAWNWAIT_Change,
		&Actor::SPAWNWAIT_Update,
		&Actor::SPAWNWAIT_UpdateSprite,
		&Actor::SPAWNWAIT_TransitionToAction,
		&Actor::SPAWNWAIT_TimeIndFrameInc,
		&Actor::SPAWNWAIT_TimeDepFrameInc,
		&Actor::SPAWNWAIT_GetActionLength,
		&Actor::SPAWNWAIT_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUN,
		&Actor::SPRINGSTUN_Start,
		&Actor::SPRINGSTUN_End,
		&Actor::SPRINGSTUN_Change,
		&Actor::SPRINGSTUN_Update,
		&Actor::SPRINGSTUN_UpdateSprite,
		&Actor::SPRINGSTUN_TransitionToAction,
		&Actor::SPRINGSTUN_TimeIndFrameInc,
		&Actor::SPRINGSTUN_TimeDepFrameInc,
		&Actor::SPRINGSTUN_GetActionLength,
		&Actor::SPRINGSTUN_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNAIM,
		&Actor::SPRINGSTUNAIM_Start,
		&Actor::SPRINGSTUNAIM_End,
		&Actor::SPRINGSTUNAIM_Change,
		&Actor::SPRINGSTUNAIM_Update,
		&Actor::SPRINGSTUNAIM_UpdateSprite,
		&Actor::SPRINGSTUNAIM_TransitionToAction,
		&Actor::SPRINGSTUNAIM_TimeIndFrameInc,
		&Actor::SPRINGSTUNAIM_TimeDepFrameInc,
		&Actor::SPRINGSTUNAIM_GetActionLength,
		&Actor::SPRINGSTUNAIM_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNAIRBOUNCE,
		&Actor::SPRINGSTUNAIRBOUNCE_Start,
		&Actor::SPRINGSTUNAIRBOUNCE_End,
		&Actor::SPRINGSTUNAIRBOUNCE_Change,
		&Actor::SPRINGSTUNAIRBOUNCE_Update,
		&Actor::SPRINGSTUNAIRBOUNCE_UpdateSprite,
		&Actor::SPRINGSTUNAIRBOUNCE_TransitionToAction,
		&Actor::SPRINGSTUNAIRBOUNCE_TimeIndFrameInc,
		&Actor::SPRINGSTUNAIRBOUNCE_TimeDepFrameInc,
		&Actor::SPRINGSTUNAIRBOUNCE_GetActionLength,
		&Actor::SPRINGSTUNAIRBOUNCE_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNAIRBOUNCEPAUSE,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_Start,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_End,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_Change,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_Update,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_UpdateSprite,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_TransitionToAction,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_TimeIndFrameInc,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_TimeDepFrameInc,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_GetActionLength,
		&Actor::SPRINGSTUNAIRBOUNCEPAUSE_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNANNIHILATION,
		&Actor::SPRINGSTUNANNIHILATION_Start,
		&Actor::SPRINGSTUNANNIHILATION_End,
		&Actor::SPRINGSTUNANNIHILATION_Change,
		&Actor::SPRINGSTUNANNIHILATION_Update,
		&Actor::SPRINGSTUNANNIHILATION_UpdateSprite,
		&Actor::SPRINGSTUNANNIHILATION_TransitionToAction,
		&Actor::SPRINGSTUNANNIHILATION_TimeIndFrameInc,
		&Actor::SPRINGSTUNANNIHILATION_TimeDepFrameInc,
		&Actor::SPRINGSTUNANNIHILATION_GetActionLength,
		&Actor::SPRINGSTUNANNIHILATION_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNANNIHILATIONATTACK,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_Start,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_End,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_Change,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_Update,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_UpdateSprite,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_TransitionToAction,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_TimeIndFrameInc,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_TimeDepFrameInc,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_GetActionLength,
		&Actor::SPRINGSTUNANNIHILATIONATTACK_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNBOUNCE,
		&Actor::SPRINGSTUNBOUNCE_Start,
		&Actor::SPRINGSTUNBOUNCE_End,
		&Actor::SPRINGSTUNBOUNCE_Change,
		&Actor::SPRINGSTUNBOUNCE_Update,
		&Actor::SPRINGSTUNBOUNCE_UpdateSprite,
		&Actor::SPRINGSTUNBOUNCE_TransitionToAction,
		&Actor::SPRINGSTUNBOUNCE_TimeIndFrameInc,
		&Actor::SPRINGSTUNBOUNCE_TimeDepFrameInc,
		&Actor::SPRINGSTUNBOUNCE_GetActionLength,
		&Actor::SPRINGSTUNBOUNCE_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNBOUNCEGROUND,
		&Actor::SPRINGSTUNBOUNCEGROUND_Start,
		&Actor::SPRINGSTUNBOUNCEGROUND_End,
		&Actor::SPRINGSTUNBOUNCEGROUND_Change,
		&Actor::SPRINGSTUNBOUNCEGROUND_Update,
		&Actor::SPRINGSTUNBOUNCEGROUND_UpdateSprite,
		&Actor::SPRINGSTUNBOUNCEGROUND_TransitionToAction,
		&Actor::SPRINGSTUNBOUNCEGROUND_TimeIndFrameInc,
		&Actor::SPRINGSTUNBOUNCEGROUND_TimeDepFrameInc,
		&Actor::SPRINGSTUNBOUNCEGROUND_GetActionLength,
		&Actor::SPRINGSTUNBOUNCEGROUND_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNGLIDE,
		&Actor::SPRINGSTUNGLIDE_Start,
		&Actor::SPRINGSTUNGLIDE_End,
		&Actor::SPRINGSTUNGLIDE_Change,
		&Actor::SPRINGSTUNGLIDE_Update,
		&Actor::SPRINGSTUNGLIDE_UpdateSprite,
		&Actor::SPRINGSTUNGLIDE_TransitionToAction,
		&Actor::SPRINGSTUNGLIDE_TimeIndFrameInc,
		&Actor::SPRINGSTUNGLIDE_TimeDepFrameInc,
		&Actor::SPRINGSTUNGLIDE_GetActionLength,
		&Actor::SPRINGSTUNGLIDE_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNGRIND,
		&Actor::SPRINGSTUNGRIND_Start,
		&Actor::SPRINGSTUNGRIND_End,
		&Actor::SPRINGSTUNGRIND_Change,
		&Actor::SPRINGSTUNGRIND_Update,
		&Actor::SPRINGSTUNGRIND_UpdateSprite,
		&Actor::SPRINGSTUNGRIND_TransitionToAction,
		&Actor::SPRINGSTUNGRIND_TimeIndFrameInc,
		&Actor::SPRINGSTUNGRIND_TimeDepFrameInc,
		&Actor::SPRINGSTUNGRIND_GetActionLength,
		&Actor::SPRINGSTUNGRIND_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNGRINDFLY,
		&Actor::SPRINGSTUNGRINDFLY_Start,
		&Actor::SPRINGSTUNGRINDFLY_End,
		&Actor::SPRINGSTUNGRINDFLY_Change,
		&Actor::SPRINGSTUNGRINDFLY_Update,
		&Actor::SPRINGSTUNGRINDFLY_UpdateSprite,
		&Actor::SPRINGSTUNGRINDFLY_TransitionToAction,
		&Actor::SPRINGSTUNGRINDFLY_TimeIndFrameInc,
		&Actor::SPRINGSTUNGRINDFLY_TimeDepFrameInc,
		&Actor::SPRINGSTUNGRINDFLY_GetActionLength,
		&Actor::SPRINGSTUNGRINDFLY_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNHOMING,
		&Actor::SPRINGSTUNHOMING_Start,
		&Actor::SPRINGSTUNHOMING_End,
		&Actor::SPRINGSTUNHOMING_Change,
		&Actor::SPRINGSTUNHOMING_Update,
		&Actor::SPRINGSTUNHOMING_UpdateSprite,
		&Actor::SPRINGSTUNHOMING_TransitionToAction,
		&Actor::SPRINGSTUNHOMING_TimeIndFrameInc,
		&Actor::SPRINGSTUNHOMING_TimeDepFrameInc,
		&Actor::SPRINGSTUNHOMING_GetActionLength,
		&Actor::SPRINGSTUNHOMING_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNHOMINGATTACK,
		&Actor::SPRINGSTUNHOMINGATTACK_Start,
		&Actor::SPRINGSTUNHOMINGATTACK_End,
		&Actor::SPRINGSTUNHOMINGATTACK_Change,
		&Actor::SPRINGSTUNHOMINGATTACK_Update,
		&Actor::SPRINGSTUNHOMINGATTACK_UpdateSprite,
		&Actor::SPRINGSTUNHOMINGATTACK_TransitionToAction,
		&Actor::SPRINGSTUNHOMINGATTACK_TimeIndFrameInc,
		&Actor::SPRINGSTUNHOMINGATTACK_TimeDepFrameInc,
		&Actor::SPRINGSTUNHOMINGATTACK_GetActionLength,
		&Actor::SPRINGSTUNHOMINGATTACK_GetTilesetName);

	SetupFuncsForAction(SPRINGSTUNTELEPORT,
		&Actor::SPRINGSTUNTELEPORT_Start,
		&Actor::SPRINGSTUNTELEPORT_End,
		&Actor::SPRINGSTUNTELEPORT_Change,
		&Actor::SPRINGSTUNTELEPORT_Update,
		&Actor::SPRINGSTUNTELEPORT_UpdateSprite,
		&Actor::SPRINGSTUNTELEPORT_TransitionToAction,
		&Actor::SPRINGSTUNTELEPORT_TimeIndFrameInc,
		&Actor::SPRINGSTUNTELEPORT_TimeDepFrameInc,
		&Actor::SPRINGSTUNTELEPORT_GetActionLength,
		&Actor::SPRINGSTUNTELEPORT_GetTilesetName);

	SetupFuncsForAction(SPRINT,
		&Actor::SPRINT_Start,
		&Actor::SPRINT_End,
		&Actor::SPRINT_Change,
		&Actor::SPRINT_Update,
		&Actor::SPRINT_UpdateSprite,
		&Actor::SPRINT_TransitionToAction,
		&Actor::SPRINT_TimeIndFrameInc,
		&Actor::SPRINT_TimeDepFrameInc,
		&Actor::SPRINT_GetActionLength,
		&Actor::SPRINT_GetTilesetName);

	SetupFuncsForAction(STAND,
		&Actor::STAND_Start,
		&Actor::STAND_End,
		&Actor::STAND_Change,
		&Actor::STAND_Update,
		&Actor::STAND_UpdateSprite,
		&Actor::STAND_TransitionToAction,
		&Actor::STAND_TimeIndFrameInc,
		&Actor::STAND_TimeDepFrameInc,
		&Actor::STAND_GetActionLength,
		&Actor::STAND_GetTilesetName);

	SetupFuncsForAction(STANDATTACK1,
		&Actor::STANDATTACK1_Start,
		&Actor::STANDATTACK1_End,
		&Actor::STANDATTACK1_Change,
		&Actor::STANDATTACK1_Update,
		&Actor::STANDATTACK1_UpdateSprite,
		&Actor::STANDATTACK1_TransitionToAction,
		&Actor::STANDATTACK1_TimeIndFrameInc,
		&Actor::STANDATTACK1_TimeDepFrameInc,
		&Actor::STANDATTACK1_GetActionLength,
		&Actor::STANDATTACK1_GetTilesetName);

	SetupFuncsForAction(STANDATTACK2,
		&Actor::STANDATTACK2_Start,
		&Actor::STANDATTACK2_End,
		&Actor::STANDATTACK2_Change,
		&Actor::STANDATTACK2_Update,
		&Actor::STANDATTACK2_UpdateSprite,
		&Actor::STANDATTACK2_TransitionToAction,
		&Actor::STANDATTACK2_TimeIndFrameInc,
		&Actor::STANDATTACK2_TimeDepFrameInc,
		&Actor::STANDATTACK2_GetActionLength,
		&Actor::STANDATTACK2_GetTilesetName);

	SetupFuncsForAction(STANDATTACK3,
		&Actor::STANDATTACK3_Start,
		&Actor::STANDATTACK3_End,
		&Actor::STANDATTACK3_Change,
		&Actor::STANDATTACK3_Update,
		&Actor::STANDATTACK3_UpdateSprite,
		&Actor::STANDATTACK3_TransitionToAction,
		&Actor::STANDATTACK3_TimeIndFrameInc,
		&Actor::STANDATTACK3_TimeDepFrameInc,
		&Actor::STANDATTACK3_GetActionLength,
		&Actor::STANDATTACK3_GetTilesetName);

	SetupFuncsForAction(STANDATTACK4,
		&Actor::STANDATTACK4_Start,
		&Actor::STANDATTACK4_End,
		&Actor::STANDATTACK4_Change,
		&Actor::STANDATTACK4_Update,
		&Actor::STANDATTACK4_UpdateSprite,
		&Actor::STANDATTACK4_TransitionToAction,
		&Actor::STANDATTACK4_TimeIndFrameInc,
		&Actor::STANDATTACK4_TimeDepFrameInc,
		&Actor::STANDATTACK4_GetActionLength,
		&Actor::STANDATTACK4_GetTilesetName);

	SetupFuncsForAction(STEEPCLIMB,
		&Actor::STEEPCLIMB_Start,
		&Actor::STEEPCLIMB_End,
		&Actor::STEEPCLIMB_Change,
		&Actor::STEEPCLIMB_Update,
		&Actor::STEEPCLIMB_UpdateSprite,
		&Actor::STEEPCLIMB_TransitionToAction,
		&Actor::STEEPCLIMB_TimeIndFrameInc,
		&Actor::STEEPCLIMB_TimeDepFrameInc,
		&Actor::STEEPCLIMB_GetActionLength,
		&Actor::STEEPCLIMB_GetTilesetName);

	SetupFuncsForAction(STEEPCLIMBATTACK,
		&Actor::STEEPCLIMBATTACK_Start,
		&Actor::STEEPCLIMBATTACK_End,
		&Actor::STEEPCLIMBATTACK_Change,
		&Actor::STEEPCLIMBATTACK_Update,
		&Actor::STEEPCLIMBATTACK_UpdateSprite,
		&Actor::STEEPCLIMBATTACK_TransitionToAction,
		&Actor::STEEPCLIMBATTACK_TimeIndFrameInc,
		&Actor::STEEPCLIMBATTACK_TimeDepFrameInc,
		&Actor::STEEPCLIMBATTACK_GetActionLength,
		&Actor::STEEPCLIMBATTACK_GetTilesetName);

	SetupFuncsForAction(STEEPCLING,
		&Actor::STEEPCLING_Start,
		&Actor::STEEPCLING_End,
		&Actor::STEEPCLING_Change,
		&Actor::STEEPCLING_Update,
		&Actor::STEEPCLING_UpdateSprite,
		&Actor::STEEPCLING_TransitionToAction,
		&Actor::STEEPCLING_TimeIndFrameInc,
		&Actor::STEEPCLING_TimeDepFrameInc,
		&Actor::STEEPCLING_GetActionLength,
		&Actor::STEEPCLING_GetTilesetName);

	SetupFuncsForAction(STEEPSLIDE,
		&Actor::STEEPSLIDE_Start,
		&Actor::STEEPSLIDE_End,
		&Actor::STEEPSLIDE_Change,
		&Actor::STEEPSLIDE_Update,
		&Actor::STEEPSLIDE_UpdateSprite,
		&Actor::STEEPSLIDE_TransitionToAction,
		&Actor::STEEPSLIDE_TimeIndFrameInc,
		&Actor::STEEPSLIDE_TimeDepFrameInc,
		&Actor::STEEPSLIDE_GetActionLength,
		&Actor::STEEPSLIDE_GetTilesetName);

	SetupFuncsForAction(STEEPSLIDEATTACK,
		&Actor::STEEPSLIDEATTACK_Start,
		&Actor::STEEPSLIDEATTACK_End,
		&Actor::STEEPSLIDEATTACK_Change,
		&Actor::STEEPSLIDEATTACK_Update,
		&Actor::STEEPSLIDEATTACK_UpdateSprite,
		&Actor::STEEPSLIDEATTACK_TransitionToAction,
		&Actor::STEEPSLIDEATTACK_TimeIndFrameInc,
		&Actor::STEEPSLIDEATTACK_TimeDepFrameInc,
		&Actor::STEEPSLIDEATTACK_GetActionLength,
		&Actor::STEEPSLIDEATTACK_GetTilesetName);

	SetupFuncsForAction(SWINGSTUN,
		&Actor::SWINGSTUN_Start,
		&Actor::SWINGSTUN_End,
		&Actor::SWINGSTUN_Change,
		&Actor::SWINGSTUN_Update,
		&Actor::SWINGSTUN_UpdateSprite,
		&Actor::SWINGSTUN_TransitionToAction,
		&Actor::SWINGSTUN_TimeIndFrameInc,
		&Actor::SWINGSTUN_TimeDepFrameInc,
		&Actor::SWINGSTUN_GetActionLength,
		&Actor::SWINGSTUN_GetTilesetName);

	SetupFuncsForAction(SUPERBIRD,
		&Actor::SUPERBIRD_Start,
		&Actor::SUPERBIRD_End,
		&Actor::SUPERBIRD_Change,
		&Actor::SUPERBIRD_Update,
		&Actor::SUPERBIRD_UpdateSprite,
		&Actor::SUPERBIRD_TransitionToAction,
		&Actor::SUPERBIRD_TimeIndFrameInc,
		&Actor::SUPERBIRD_TimeDepFrameInc,
		&Actor::SUPERBIRD_GetActionLength,
		&Actor::SUPERBIRD_GetTilesetName);

	SetupFuncsForAction(TELEPORTACROSSTERRAIN,
		&Actor::TELEPORTACROSSTERRAIN_Start,
		&Actor::TELEPORTACROSSTERRAIN_End,
		&Actor::TELEPORTACROSSTERRAIN_Change,
		&Actor::TELEPORTACROSSTERRAIN_Update,
		&Actor::TELEPORTACROSSTERRAIN_UpdateSprite,
		&Actor::TELEPORTACROSSTERRAIN_TransitionToAction,
		&Actor::TELEPORTACROSSTERRAIN_TimeIndFrameInc,
		&Actor::TELEPORTACROSSTERRAIN_TimeDepFrameInc,
		&Actor::TELEPORTACROSSTERRAIN_GetActionLength,
		&Actor::TELEPORTACROSSTERRAIN_GetTilesetName);

	SetupFuncsForAction(TESTSUPER,
		&Actor::TESTSUPER_Start,
		&Actor::TESTSUPER_End,
		&Actor::TESTSUPER_Change,
		&Actor::TESTSUPER_Update,
		&Actor::TESTSUPER_UpdateSprite,
		&Actor::TESTSUPER_TransitionToAction,
		&Actor::TESTSUPER_TimeIndFrameInc,
		&Actor::TESTSUPER_TimeDepFrameInc,
		&Actor::TESTSUPER_GetActionLength,
		&Actor::TESTSUPER_GetTilesetName);

	SetupFuncsForAction(UAIR,
		&Actor::UAIR_Start,
		&Actor::UAIR_End,
		&Actor::UAIR_Change,
		&Actor::UAIR_Update,
		&Actor::UAIR_UpdateSprite,
		&Actor::UAIR_TransitionToAction,
		&Actor::UAIR_TimeIndFrameInc,
		&Actor::UAIR_TimeDepFrameInc,
		&Actor::UAIR_GetActionLength,
		&Actor::UAIR_GetTilesetName);

	SetupFuncsForAction(UPTILT1,
		&Actor::UPTILT1_Start,
		&Actor::UPTILT1_End,
		&Actor::UPTILT1_Change,
		&Actor::UPTILT1_Update,
		&Actor::UPTILT1_UpdateSprite,
		&Actor::UPTILT1_TransitionToAction,
		&Actor::UPTILT1_TimeIndFrameInc,
		&Actor::UPTILT1_TimeDepFrameInc,
		&Actor::UPTILT1_GetActionLength,
		&Actor::UPTILT1_GetTilesetName);

	SetupFuncsForAction(UPTILT2,
		&Actor::UPTILT2_Start,
		&Actor::UPTILT2_End,
		&Actor::UPTILT2_Change,
		&Actor::UPTILT2_Update,
		&Actor::UPTILT2_UpdateSprite,
		&Actor::UPTILT2_TransitionToAction,
		&Actor::UPTILT2_TimeIndFrameInc,
		&Actor::UPTILT2_TimeDepFrameInc,
		&Actor::UPTILT2_GetActionLength,
		&Actor::UPTILT2_GetTilesetName);

	SetupFuncsForAction(UPTILT3,
		&Actor::UPTILT3_Start,
		&Actor::UPTILT3_End,
		&Actor::UPTILT3_Change,
		&Actor::UPTILT3_Update,
		&Actor::UPTILT3_UpdateSprite,
		&Actor::UPTILT3_TransitionToAction,
		&Actor::UPTILT3_TimeIndFrameInc,
		&Actor::UPTILT3_TimeDepFrameInc,
		&Actor::UPTILT3_GetActionLength,
		&Actor::UPTILT3_GetTilesetName);
	

	SetupFuncsForAction(WAITFORSHIP,
		&Actor::WAITFORSHIP_Start,
		&Actor::WAITFORSHIP_End,
		&Actor::WAITFORSHIP_Change,
		&Actor::WAITFORSHIP_Update,
		&Actor::WAITFORSHIP_UpdateSprite,
		&Actor::WAITFORSHIP_TransitionToAction,
		&Actor::WAITFORSHIP_TimeIndFrameInc,
		&Actor::WAITFORSHIP_TimeDepFrameInc,
		&Actor::WAITFORSHIP_GetActionLength,
		&Actor::WAITFORSHIP_GetTilesetName);

	SetupFuncsForAction(WALLATTACK,
		&Actor::WALLATTACK_Start,
		&Actor::WALLATTACK_End,
		&Actor::WALLATTACK_Change,
		&Actor::WALLATTACK_Update,
		&Actor::WALLATTACK_UpdateSprite,
		&Actor::WALLATTACK_TransitionToAction,
		&Actor::WALLATTACK_TimeIndFrameInc,
		&Actor::WALLATTACK_TimeDepFrameInc,
		&Actor::WALLATTACK_GetActionLength,
		&Actor::WALLATTACK_GetTilesetName);

	SetupFuncsForAction(WALLCLING,
		&Actor::WALLCLING_Start,
		&Actor::WALLCLING_End,
		&Actor::WALLCLING_Change,
		&Actor::WALLCLING_Update,
		&Actor::WALLCLING_UpdateSprite,
		&Actor::WALLCLING_TransitionToAction,
		&Actor::WALLCLING_TimeIndFrameInc,
		&Actor::WALLCLING_TimeDepFrameInc,
		&Actor::WALLCLING_GetActionLength,
		&Actor::WALLCLING_GetTilesetName);

	SetupFuncsForAction(WALLJUMP,
		&Actor::WALLJUMP_Start,
		&Actor::WALLJUMP_End,
		&Actor::WALLJUMP_Change,
		&Actor::WALLJUMP_Update,
		&Actor::WALLJUMP_UpdateSprite,
		&Actor::WALLJUMP_TransitionToAction,
		&Actor::WALLJUMP_TimeIndFrameInc,
		&Actor::WALLJUMP_TimeDepFrameInc,
		&Actor::WALLJUMP_GetActionLength,
		&Actor::WALLJUMP_GetTilesetName);

	SetupFuncsForAction(WALLTECH,
		&Actor::WALLTECH_Start,
		&Actor::WALLTECH_End,
		&Actor::WALLTECH_Change,
		&Actor::WALLTECH_Update,
		&Actor::WALLTECH_UpdateSprite,
		&Actor::WALLTECH_TransitionToAction,
		&Actor::WALLTECH_TimeIndFrameInc,
		&Actor::WALLTECH_TimeDepFrameInc,
		&Actor::WALLTECH_GetActionLength,
		&Actor::WALLTECH_GetTilesetName);

	SetupFuncsForAction(WARP_CHARGE,
		&Actor::WARP_CHARGE_Start,
		&Actor::WARP_CHARGE_End,
		&Actor::WARP_CHARGE_Change,
		&Actor::WARP_CHARGE_Update,
		&Actor::WARP_CHARGE_UpdateSprite,
		&Actor::WARP_CHARGE_TransitionToAction,
		&Actor::WARP_CHARGE_TimeIndFrameInc,
		&Actor::WARP_CHARGE_TimeDepFrameInc,
		&Actor::WARP_CHARGE_GetActionLength,
		&Actor::WARP_CHARGE_GetTilesetName);

	SetupFuncsForAction(WARP,
		&Actor::WARP_Start,
		&Actor::WARP_End,
		&Actor::WARP_Change,
		&Actor::WARP_Update,
		&Actor::WARP_UpdateSprite,
		&Actor::WARP_TransitionToAction,
		&Actor::WARP_TimeIndFrameInc,
		&Actor::WARP_TimeDepFrameInc,
		&Actor::WARP_GetActionLength,
		&Actor::WARP_GetTilesetName);

	SetupFuncsForAction(WATERGLIDE,
		&Actor::WATERGLIDE_Start,
		&Actor::WATERGLIDE_End,
		&Actor::WATERGLIDE_Change,
		&Actor::WATERGLIDE_Update,
		&Actor::WATERGLIDE_UpdateSprite,
		&Actor::WATERGLIDE_TransitionToAction,
		&Actor::WATERGLIDE_TimeIndFrameInc,
		&Actor::WATERGLIDE_TimeDepFrameInc,
		&Actor::WATERGLIDE_GetActionLength,
		&Actor::WATERGLIDE_GetTilesetName);

	SetupFuncsForAction(WATERGLIDECHARGE,
		&Actor::WATERGLIDECHARGE_Start,
		&Actor::WATERGLIDECHARGE_End,
		&Actor::WATERGLIDECHARGE_Change,
		&Actor::WATERGLIDECHARGE_Update,
		&Actor::WATERGLIDECHARGE_UpdateSprite,
		&Actor::WATERGLIDECHARGE_TransitionToAction,
		&Actor::WATERGLIDECHARGE_TimeIndFrameInc,
		&Actor::WATERGLIDECHARGE_TimeDepFrameInc,
		&Actor::WATERGLIDECHARGE_GetActionLength,
		&Actor::WATERGLIDECHARGE_GetTilesetName);

	SetupFuncsForAction(WATERGLIDE_HITSTUN,
		&Actor::WATERGLIDE_HITSTUN_Start,
		&Actor::WATERGLIDE_HITSTUN_End,
		&Actor::WATERGLIDE_HITSTUN_Change,
		&Actor::WATERGLIDE_HITSTUN_Update,
		&Actor::WATERGLIDE_HITSTUN_UpdateSprite,
		&Actor::WATERGLIDE_HITSTUN_TransitionToAction,
		&Actor::WATERGLIDE_HITSTUN_TimeIndFrameInc,
		&Actor::WATERGLIDE_HITSTUN_TimeDepFrameInc,
		&Actor::WATERGLIDE_HITSTUN_GetActionLength,
		&Actor::WATERGLIDE_HITSTUN_GetTilesetName);

	SetupFuncsForAction(WIREHOLD,
		&Actor::WIREHOLD_Start,
		&Actor::WIREHOLD_End,
		&Actor::WIREHOLD_Change,
		&Actor::WIREHOLD_Update,
		&Actor::WIREHOLD_UpdateSprite,
		&Actor::WIREHOLD_TransitionToAction,
		&Actor::WIREHOLD_TimeIndFrameInc,
		&Actor::WIREHOLD_TimeDepFrameInc,
		&Actor::WIREHOLD_GetActionLength,
		&Actor::WIREHOLD_GetTilesetName);
}

void Actor::StartAction()
{
	if (startActionFuncs[action] != NULL)
	{
		(this->*startActionFuncs[action])();
	}
}

void Actor::EndAction()
{
	if (endActionFuncs[action] != NULL)
	{
		(this->*endActionFuncs[action])();
	}
}

void Actor::ChangeAction()
{
	if (changeActionFuncs[action] != NULL)
	{
		(this->*changeActionFuncs[action])();
	}
}

void Actor::UpdateAction()
{
	if (updateActionFuncs[action] != NULL)
	{
		(this->*updateActionFuncs[action])();
	}
}

void Actor::UpdateActionSprite()
{
	if (updateSpriteFuncs[action] != NULL)
	{
		(this->*updateSpriteFuncs[action])();
	}
}

Actor::Actor()
	:dead(false), actorIndex(0), optionField(Session::PLAYER_OPTION_BIT_COUNT),
	startOptionField(Session::PLAYER_OPTION_BIT_COUNT),
	exitAuraShader(PlayerSkinShader::ST_BOOST),
	originalProgressionOptionField(Session::PLAYER_OPTION_BIT_COUNT)
	//originalProgressionLogField(LogDetailedInfo::MAX_LOGS)
{
	shallowInit = true;
	sess = NULL;
	editOwner = NULL;
	owner = NULL;
	actionFolder = "Kin/";

	SetupActionFunctions();
	SetupTilesets();

	InitSounds();

	CreateCollisionBodies();
}

Actor::Actor(GameSession *gs, EditSession *es, int p_actorIndex)
	:dead(false), actorIndex(p_actorIndex), optionField(Session::PLAYER_OPTION_BIT_COUNT),
	startOptionField(Session::PLAYER_OPTION_BIT_COUNT),
	exitAuraShader(PlayerSkinShader::ST_BOOST),
	originalProgressionOptionField(Session::PLAYER_OPTION_BIT_COUNT)
	//originalProgressionLogField( LogDetailedInfo::MAX_LOGS )
{
	shallowInit = false;

	adventureManager = MainMenu::GetInstance()->adventureManager;

	SetSession(Session::GetSession(), gs, es);

	kinMask = NULL;

	nameTag = new NameTag;

	pState = new PState;
	memset(pState, 0, sizeof(PState));

	preSimulationState = NULL;
	futurePositions = NULL;
	

	normalWaterMaxFallSpeed = 4.0;

	maxFallSpeedWhileHitting = 4.0;
	hitGrassHitInfo.damage = 60;//3 * 60;
	hitGrassHitInfo.drainX = 1.0;
	hitGrassHitInfo.drainY = 1.0;
	hitGrassHitInfo.hitlagFrames = 0;
	hitGrassHitInfo.hitstunFrames = 20;
	hitGrassHitInfo.knockback = 0;
	hitGrassHitInfo.gravMultiplier = 0.0;
	
	hasWallJumpRechargeDoubleJump = false;
	hasWallJumpRechargeAirDash = false;
	numRemainingExtraAirdashBoosts = 0;

	grindLimitBeforeSlow = 30;
	
	action = -1;

	actionFolder = "Kin/";
	SetupActionFunctions();
	SetupTilesets();

	fxPaletteShader = new PaletteShader("kinfx", "Resources/Kin/FX/kin_fx_palette_256x30.png");
	fxPaletteShader->SetPaletteIndex(0);

	mapIconShader.SetSubRect(ts_mapIcon, ts_mapIcon->GetSubRect(0));

	birdCommands.resize(3);
	for (int i = 0; i < 3; ++i)
	{
		//birdCommands[i] = new BirdCommand;
	}

	for (int i = 0; i < NUM_SWORD_PROJECTILES; ++i)
	{
		swordProjectiles[i] = new SwordProjectile(this);
	}

	

	LoadHitboxes();

	if ( owner != NULL && owner->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && adventureManager != NULL && adventureManager->transferPlayerPowerMode >= 0)
	{
		//transferred from another map, keep the playermode
		currPowerMode = adventureManager->transferPlayerPowerMode;
	}
	else
	{
		currPowerMode = PMODE_SHIELD;
	}
	simulationMode = false;
	receivedHitReaction = HitResult::MISS;
	superActiveLimit = 180;
	attackLevelCounterLimit = 60;

	
	//return;
	

	rpu = new RisingParticleUpdater( this );

	totalHealth = 3600;
	steepClimbBoostStart = 5;
	accelGrassAccel = .25;
	jumpGrassExtra = 15;

	standNDashBoostCooldown = 10;
	standNDashBoostQuant = 3;
	dairBoostVel = 4;
	fairAirDashBoostQuant = 2;
	for (int i = 0; i < 3; ++i)
	{
		//motionGhostsEffects[i] = new MotionGhostEffect(80);
		motionGhostsEffects[i] = new MotionGhostEffect(4, 1);
	}

	gravityIncreaserTrailEmitter = new PlayerBoosterEffectEmitter(this, ShapeEmitter::PARTICLE_BOOSTER_GRAVITY_INCREASER);
	gravityIncreaserTrailEmitter->SetTileset(ts_fx_boosterParticles);
	gravityIncreaserTrailEmitter->CreateParticles();

	gravityDecreaserTrailEmitter = new PlayerBoosterEffectEmitter(this, ShapeEmitter::PARTICLE_BOOSTER_GRAVITY_DECREASER);
	gravityDecreaserTrailEmitter->SetTileset(ts_fx_boosterParticles);
	gravityDecreaserTrailEmitter->CreateParticles();

	momentumBoosterTrailEmitter = new PlayerBoosterEffectEmitter(this, ShapeEmitter::PARTICLE_BOOSTER_MOMENTUM);
	momentumBoosterTrailEmitter->SetTileset(ts_fx_boosterParticles);
	momentumBoosterTrailEmitter->CreateParticles();

	//timeSlowBoosterTrailEmitter = new PlayerBoosterEffectEmitter(this, PlayerBoosterEffectEmitter::BOOSTER_TIMESLOW);
	//timeSlowBoosterTrailEmitter->SetTileset(sess->GetSizedTileset("FX/homingparticle_32x32.png"));//"Env/leaves_128x128.png"));
	//timeSlowBoosterTrailEmitter->CreateParticles();

	homingBoosterTrailEmitter = new PlayerBoosterEffectEmitter(this, ShapeEmitter::PARTICLE_BOOSTER_HOMING);
	homingBoosterTrailEmitter->SetTileset(ts_fx_boosterParticles);
	homingBoosterTrailEmitter->CreateParticles();

	antiTimeSlowBoosterTrailEmitter = new PlayerBoosterEffectEmitter(this, ShapeEmitter::PARTICLE_BOOSTER_ANTITIMESLOW);
	antiTimeSlowBoosterTrailEmitter->SetTileset(ts_fx_boosterParticles);
	antiTimeSlowBoosterTrailEmitter->CreateParticles();

	freeFlightBoosterTrailEmitter = new PlayerBoosterEffectEmitter(this, ShapeEmitter::PARTICLE_BOOSTER_FREEFLIGHT);
	freeFlightBoosterTrailEmitter->SetTileset(ts_fx_boosterParticles);
	freeFlightBoosterTrailEmitter->CreateParticles();

	motionGhostBuffer = new VertexBuf(80, sf::Quads);
	motionGhostBufferBlue = new VertexBuf(80, sf::Quads);
	motionGhostBufferPurple = new VertexBuf(80, sf::Quads);

	//kinMask = new KinMask(this);
		

	//risingAuraPool = new EffectPool(EffectType::FX_RELATIVE, 100, 1.f);
	//risingAuraPool->ts = GetTileset("Kin/FX/rising_8x8.png", 8, 8);

	maxMotionGhosts = 80;
	
	
	
		
	team = (Team)actorIndex; //debug
	
	//SetupTilesets();

	SetupFX();

	/*if (actorIndex == 0)
	{
		SetupTilesets(NULL, NULL);
	}
	else if (actorIndex == 1)
	{
		Skin *swordSkin = new Skin(startChanges, endChanges, 9, 1);
		Skin *skin = new Skin(startChanges, endChanges, 9, 1);
		SetupTilesets(skin, swordSkin);
		delete skin;
		delete swordSkin;
	}*/
		
	//SetupTilesets(NULL,NULL);


	////maybe get rid of this later
	////preload them
	//sess->GetTileset("Kin/exitenergy_0_512x512.png", 512, 512);
	//sess->GetTileset("Kin/exitenergy_2_512x512.png", 512, 512);
	//sess->GetTileset("Kin/exitenergy_1_512x512.png", 512, 512);

	maxFramesSinceGrindAttempt = 30;

	regrindOffMax = 3;

	railTest.setSize(Vector2f(64, 64));
	railTest.setFillColor(Color( COLOR_ORANGE.r, COLOR_ORANGE.g, COLOR_ORANGE.b, 80 ));
	railTest.setOrigin(railTest.getLocalBounds().width / 2, railTest.getLocalBounds().height / 2);	

	singleWireBoostTiming = 4;
	doubleWireBoostTiming = 4;

	glideTurnAccel = .005;//.01;
	maxGlideTurnFactor = .05;//.08;

	spriteAction = FAIR;
	currTileIndex = 0;

	GameController *cont = GetController(actorIndex);
	toggleBounceInput = false;//cont->keySettings.toggleBounce;
	toggleTimeSlowInput = false;//cont->keySettings.toggleTimeSlow;
	toggleGrindInput = false;//cont->keySettings.toggleGrind;
	speedParticleRate = 10; //20
	//re = new RotaryParticleEffect( this );
	//re1 = new RotaryParticleEffect( this );
	//pTrail = new ParticleTrail( this );
	//re1->angle += PI;
	//ae = new AirParticleEffect( position );

	level1SpeedThresh = 25;	//30;//22;//22;//32;
	level2SpeedThresh = 45; 
	speedChangeUp = .5;//03;//.5;
	speedChangeDown = .03;//.005;//.07;


	grindLungeSpeed0 = 15.0;
	grindLungeSpeed1 = 17.0;//20.0;
	grindLungeSpeed2 = 22.0;//28.0;
	//grindLungeExtraMax = 10.0;

	motionGhostSpacing = 1;
	ghostSpacingCounter = 0;

	sess->mainMenu->LoadShader(shieldShader, "shield");
	shieldShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	
	fullBlockShieldColor = Color( 237, 29, 36 );
	halfBlockShieldColor = Color(255, 201, 14);

	
	sess->mainMenu->LoadShader(despFaceShader, "colorswap");
	//uniform vec4 toColor;
	//uniform vec4 fromColor;
	Color c( 0x66, 0xee, 0xff );
	despFaceShader.setUniform( "fromColor", ColorGL( c ) );

	sess->mainMenu->LoadShader(motionGhostShader, "motionghost");
		
	Color mgc = Color::Cyan;
	mgc.a = 25;
	//motionGhostShader.setUniform("energyColor", ColorGL(mgc));
		
	motionGhostShader.setUniform("u_texture", sf::Shader::CurrentTexture);


	sess->mainMenu->LoadShader(playerDespShader, "playerdesperation");
	playerDespShader.setUniform("u_texture", sf::Shader::CurrentTexture);


	sess->mainMenu->LoadShader(playerSuperShader, "playersuper");
	playerSuperShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	//swordShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	
	/*if( !timeSlowShader.loadFromFile( "Shader/timeslow.frag", sf::Shader::Fragment ) )
	{
		cout << "TIME SLOW SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "time slow shader not loaded" );
	}*/
	
	InitSounds();


	currHitboxInfo = new HitboxInfo();
	currHitboxInfo->damage = 20;
	currHitboxInfo->drainX = .5;
	currHitboxInfo->drainY = .5;
	currHitboxInfo->hitlagFrames = 6;
	currHitboxInfo->hitstunFrames = 30;
	currHitboxInfo->knockback = 0;
	currHitboxInfo->freezeDuringStun = true;


	currVSHitboxInfo = new HitboxInfo();
	currVSHitboxInfo->damage = 0;//20;
	currVSHitboxInfo->drainX = 0;//.5;
	currVSHitboxInfo->drainY = 0;//.5;
	currVSHitboxInfo->hitlagFrames = 6;//6;
	currVSHitboxInfo->hitstunFrames = 30;
	currVSHitboxInfo->knockback = 0;
	currVSHitboxInfo->freezeDuringStun = true;
	currVSHitboxInfo->invincibleFrames = 16;
	
	maxDespFrames = 60 * 5;
	maxSuperFrames = 60 * 5;

	bounceBoostSpeed = 4;//6.0;//8.0;//.5;//1;//6.0;//5.0;//4.7;

	sprite = new Sprite;
	if( actorIndex == 1 )
	{
		//sprite->setColor( Color( 255, 0, 0 ) );
	}
		
	CollisionBox cb;
	cb.isCircle = true;
	cb.offset.x = 32;
	cb.offset.y = -8;
	//cb.offsetAngle = 0;
	cb.rw = 64;
	cb.rh = 64;

	//setup hitboxes
	//for( int j = 4; j < 10; ++j )

	/*diagDownSwordOffset[0] = Vector2f(32, 24);
	diagDownSwordOffset[1] = Vector2f(32, 24);
	diagDownSwordOffset[2] = Vector2f(32, 24);*/

	diagDownSwordOffset[0] = Vector2f(32, 24);
	diagDownSwordOffset[1] = Vector2f(16, 32);
	diagDownSwordOffset[2] = Vector2f(16, 64);
	//Vector2f(16, 32)
	//Vector2f(16, 64)

	Vector2i offsets[3];//( 0, 0 );
	offsets[0] = Vector2i(40, -32);
		

	/*diagUpSwordOffset[0] = Vector2f(40, -32);
	diagUpSwordOffset[1] = Vector2f(40, -32);
	diagUpSwordOffset[2] = Vector2f(40, -32);*/

	diagUpSwordOffset[0] = Vector2f(40, -32);
	diagUpSwordOffset[1] = Vector2f(16, -40);
	diagUpSwordOffset[2] = Vector2f(32, -48);
	/*offsets[1] = Vector2i(16, -40);
	offsets[2] = Vector2i(32, -48);*/

	
	climbAttackOffset[0] = Vector2f(0, -32);
	climbAttackOffset[1] = Vector2f(0, -128);
	climbAttackOffset[2] = Vector2f(0, -72);

	slideAttackOffset[0] = Vector2f(0, -56);
	slideAttackOffset[1] = Vector2f(0, -64);
	slideAttackOffset[2] = Vector2f(0, -96);

	if (true)
	{
		CreateCollisionBodies();



		for (int i = 0; i < 3; ++i)
		{
			diagDownHitboxes[i]->OffsetAllFrames(diagDownSwordOffset[i]);
			diagUpHitboxes[i]->OffsetAllFrames(diagUpSwordOffset[i]);
		}

		standSwordOffset[0] = Vector2f(64, -32);//Vector2f(0, -64);
		standSwordOffset[1] = Vector2f(64, -32);//Vector2f(64, 32);//Vector2f(0, -64);
		standSwordOffset[2] = Vector2f(64, -32);//Vector2f(64, 16);//Vector2f(0, -64);

		Vector2f groundAttackOffset(0, -32);
		for (int i = 0; i < 3; ++i)
		{
			Vector2f testOffset = standSwordOffset[i];
			//testOffset.y -= ts_standAttackSword[i]->tileHeight / 2.0;
			//standHitboxes[i]->OffsetAllFrames(standSwordOffset[i]);
			standHitboxes1[i]->OffsetAllFrames(groundAttackOffset);
			standHitboxes2[i]->OffsetAllFrames(groundAttackOffset);
			standHitboxes3[i]->OffsetAllFrames(groundAttackOffset);
			standHitboxes4[i]->OffsetAllFrames(groundAttackOffset);

			dashHitboxes1[i]->OffsetAllFrames(groundAttackOffset);
			dashHitboxes2[i]->OffsetAllFrames(groundAttackOffset);
			dashHitboxes3[i]->OffsetAllFrames(groundAttackOffset);
		}

		for (int i = 0; i < 3; ++i)
		{
			Vector2f testOffset = -slideAttackOffset[i];
			testOffset.y -= ts_steepSlideAttackSword[i]->tileHeight / 2.0;
			steepSlideHitboxes[i]->OffsetAllFrames(testOffset);
		}

		for (int i = 0; i < 3; ++i)
		{
			Vector2f testOffset = -climbAttackOffset[i];
			testOffset.y -= ts_steepClimbAttackSword[i]->tileHeight / 2.0;
			steepClimbHitboxes[i]->OffsetAllFrames(testOffset);
		}

		shockwaveHitboxes = NULL;
		grindHitboxes[0] = NULL;

		cb.rw = 90;
		cb.rh = 90;
		cb.offset.x = 0;
		cb.offset.y = 0;
		grindHitboxes[0] = new CollisionBody( CollisionBox::Hit);
		grindHitboxes[0]->BasicSetup();
		grindHitboxes[0]->AddCollisionBox(0, cb);
		grindHitboxes[0]->hitboxInfo = currHitboxInfo;

		homingHitboxes = new CollisionBody(CollisionBox::Hit);
		homingHitboxes->BasicSetup();
		homingHitboxes->AddCollisionBox(0, cb);
		homingHitboxes->hitboxInfo = currHitboxInfo;


		cb.rw = 500;
		cb.rh = 500;
		annihilationHitboxes = new CollisionBody(CollisionBox::Hit);
		annihilationHitboxes->BasicSetup();
		annihilationHitboxes->AddCollisionBox(0, cb);
		annihilationHitboxes->hitboxInfo = currHitboxInfo;

		
		//up
	}
	else
	{
		fairHitboxes[0] = NULL;
		uairHitboxes[0] = NULL;
		dairHitboxes[0] = NULL;

		fairHitboxes[1] = NULL;
		uairHitboxes[1] = NULL;
		dairHitboxes[1] = NULL;

		fairHitboxes[2] = NULL;
		uairHitboxes[2] = NULL;
		dairHitboxes[2] = NULL;


		standHitboxes1[0] = NULL;
		standHitboxes1[1] = NULL;
		standHitboxes1[2] = NULL;

		standHitboxes2[0] = NULL;
		standHitboxes2[1] = NULL;
		standHitboxes2[2] = NULL;

		standHitboxes3[0] = NULL;
		standHitboxes3[1] = NULL;
		standHitboxes3[2] = NULL;

		standHitboxes4[0] = NULL;
		standHitboxes4[1] = NULL;
		standHitboxes4[2] = NULL;

		dashHitboxes1[0] = NULL;
		dashHitboxes1[1] = NULL;
		dashHitboxes1[2] = NULL;

		dashHitboxes2[0] = NULL;
		dashHitboxes2[1] = NULL;
		dashHitboxes2[2] = NULL;

		dashHitboxes3[0] = NULL;
		dashHitboxes3[1] = NULL;
		dashHitboxes3[2] = NULL;

		wallHitboxes[0] = NULL;

		wallHitboxes[1] = NULL;

		wallHitboxes[2] = NULL;


		steepClimbHitboxes[0] = NULL;
		steepClimbHitboxes[1] = NULL;
		steepClimbHitboxes[2] = NULL;

		steepSlideHitboxes[0] = NULL;
		steepSlideHitboxes[1] = NULL;
		steepSlideHitboxes[2] = NULL;

		diagUpHitboxes[0] = NULL;
		diagDownHitboxes[0] = NULL;

		diagUpHitboxes[1] = NULL;
		diagDownHitboxes[1] = NULL;

		diagUpHitboxes[2] = NULL;
		diagDownHitboxes[2] = NULL;

		shockwaveHitboxes = NULL;
		grindHitboxes[0] = NULL;
	}

		
	//grindHitboxes[0] = new list<CollisionBox>;
	//grindHitboxes[0]->push_back( cb );
	wallThresh = .9999;
	//tileset setup

	maxBBoostCount = GetActionLength(DASH);
	maxAirdashBoostCount = GetActionLength(AIRDASH);
		 
	//sess->mainMenu->LoadShader(swordShaders[0], "sword");
	//swordShaders[0].setUniform("fromColor", ColorGL(Color(0, 238, 255)));//ColorGL(COLOR_TEAL) );
	//swordShaders[0].setUniform("u_texture", sf::Shader::CurrentTexture);

	//sess->mainMenu->LoadShader(swordShaders[1], "sword");
	//swordShaders[1].setUniform( "fromColor", ColorGL(Color( 43, 167, 255 )) );
	//swordShaders[1].setUniform("u_texture", sf::Shader::CurrentTexture);
	//
	//sess->mainMenu->LoadShader(swordShaders[2], "sword");
	//swordShaders[2].setUniform( "fromColor", ColorGL(Color( 140, 146, 255 )) );
	//swordShaders[2].setUniform("u_texture", sf::Shader::CurrentTexture);

	grindActionLength = 32;

	repeatingSound = NULL;
		
	Init();

	baseSlowMultiple = 1;
	baseTimeSlowedMultiple = 5;

	wallJumpMovementLimit = 12; //10 frames

	steepThresh = .4; // go between 0 and 1

	steepSlideGravFactor = .25;//.25;//.4;
	steepSlideFastGravFactor = .3;//.5;

	//ths is now just the dashspeed + 1.0
	wallJumpStrength.x = 0;//10; 

	wallJumpStrength.y = 20;
	clingSpeed = 3;

	slopeTooSteepLaunchLimitX = .1;
		
	steepClimbGravFactor = .4;//.6;//.31;//.6;//.31;
	steepClimbUpFactor = .31;//.17;//.4;//.17;//.31;
	steepClimbDownFactor = steepClimbGravFactor;//.5;
	steepClingSpeedLimit = 2.0;
	//steepClimbFastFactor = .7;//.2;
	
	climbBoostLimit = 30;//25;//22;//15;
	
	airAccel = 1.5;
		
	gravity = 1;//1.9; // 1 
	wallClimbGravityFactor = .7;
	jumpStrength = 21.5;//18;//25;//27.5; // 2 
	doubleJumpStrength = 20;//17;//23;//26.5;
	backDoubleJumpStrength = 22;
	dashSpeed = 9;//12; // 3

	dashSpeed0 = 9;
	dashSpeed1 = 10.5;
	dashSpeed2 = 14;

	airDashSpeed0 = dashSpeed0;
	airDashSpeed1 = dashSpeed1;
	airDashSpeed2 = dashSpeed2;

	airDashSpeed = dashSpeed;

	maxSpeed = 60;
	maxFallSpeedSlow = 30;

	offSlopeByWallThresh = dashSpeed;//18;
	slopeLaunchMinSpeed = 5;//dashSpeed * .7;
	steepClimbSpeedThresh = 4;//dashSpeed - 1;
	slideGravFactor = .25;//.25;//.3;//.45;

		
	maxRunInit = 4;
	maxAirXControl = 6;//maxRunInit;
	airSlow = .3;//.7;//.3;

	//max ground speed should probably start around 60-80 and then get powerups to rise to 100
	//for world 1 lets do the lowest number for the beta
		


	runAccelInit = .5;
		
	runAccel = .03;
	sprintAccel = .08;//.3;//.85;

	holdDashAccel = .05;
	bounceFlameAccel0 = .08;//.15;//.8;
	bounceFlameAccel1 = .1; //this was the original
	bounceFlameAccel2 = .15;//.18;

	dashHeight = 10;
	normalHeight = 20;
	doubleJumpHeight = 10;
	sprintHeight = 16;

	//CollisionBox b;
	b.isCircle = false;
	//b.offsetAngle = 0;
	b.offset.x = 0;
	b.offset.y = 0;
	b.rw = 10;
	b.rh = normalHeight;

	//hurtboxMap[STAND] = new CollisionBody(1);
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.isCircle = false;
	hurtBody.rw = 7;//10;
	hurtBody.rh = 15;//normalHeight - 5;//normalHeight;
	//hurtboxMap[STAND]->AddCollisionBox(0, hurtBody);

	leftWire = new Wire( this, false );
	rightWire = new Wire( this, true );

	minRailGrindSpeed[0] = dashSpeed0 + 10;
	minRailGrindSpeed[1] = dashSpeed1 + 10;
	minRailGrindSpeed[2] = dashSpeed2 + 10;

	bubbleSprite.setTexture( *ts_bubble->texture );
	
	bubbleRadius = 160;

	

	bubbleLifeSpan = 240;

	optionField.Reset();
	startOptionField.Reset();

	

	//for (int i = 0; i < UPGRADE_Count; ++i)
	//{
	//	SetStartUpgrade(i, true);
	//}

	//SetStartUpgrade(POWER_AIRDASH, false);

	SetupTimeBubbles();

	Respawn(false);

	cout << "end player" << endl;
}

Actor::~Actor()
{
	for (int i = 0; i < 3; ++i)
	{
		delete fairHitboxes[i];
		delete uairHitboxes[i];
		delete dairHitboxes[i];

		delete standHitboxes1[i];
		delete standHitboxes2[i];
		delete standHitboxes3[i];
		delete standHitboxes4[i];

		delete wallHitboxes[i];

		delete dashHitboxes1[i];
		delete dashHitboxes2[i];
		delete dashHitboxes3[i];

		//	delete wallHitboxes[i];
		delete steepClimbHitboxes[i];
		delete steepSlideHitboxes[i];
		delete diagUpHitboxes[i];
		delete diagDownHitboxes[i];
		//delete grindHitboxes[i];
	}
	
	if (shallowInit)
		return;

	delete grindHitboxes[0];
	delete homingHitboxes;
	delete annihilationHitboxes;

	

	if( nameTag != NULL )
		delete nameTag;

	/*for (auto it = birdCommands.begin(); it != birdCommands.end(); ++it)
	{
		delete (*it);
	}*/

	if (pState != NULL)
		delete pState;

	if (preSimulationState != NULL)
	{
		delete preSimulationState;
	}

	if (futurePositions != NULL)
	{
		delete[] futurePositions;
		futurePositions = NULL;
	}

	for (int i = 0; i < NUM_SWORD_PROJECTILES; ++i)
	{
		delete swordProjectiles[i];
	}

	//delete skin;
	//delete swordSkin;
	/*if (glideEmitter != NULL)
	{
		delete glideEmitter;
	}*/

	delete rpu;

	/*if( kinMask != NULL)
		delete kinMask;*/


	//delete risingAuraPool;
	for (int i = 0; i < 3; ++i)
	{
		delete motionGhostsEffects[i];
	}

	//need to clear this so that the pools delete. Not sure why the default
	//deletion doesn't do it.
	effectPools.clear();

	delete fxPaletteShader;
	

	delete keyExplodeUpdater;
	delete keyExplodeRingGroup;
	delete enemyExplodeRingGroup;
	delete enemiesClearedRingGroup;
	delete enoughKeysToExitRingGroup;
	delete gravityDecreaserOnRingGroup;
	delete gravityDecreaserOffRingGroup;

	delete gravityIncreaserOnRingGroup;
	delete gravityIncreaserOffRingGroup;

	
	delete gravityIncreaserTrailEmitter;
	delete gravityDecreaserTrailEmitter;
	delete momentumBoosterTrailEmitter;
	delete homingBoosterTrailEmitter;
	delete antiTimeSlowBoosterTrailEmitter;
	delete freeFlightBoosterTrailEmitter;
	
	delete sprite;

	delete motionGhostBuffer;
	delete motionGhostBufferBlue;
	delete motionGhostBufferPurple;

	delete currHitboxInfo;
	delete currVSHitboxInfo;

	/*if (owner != NULL)*/
	

	
	//delete shockwaveHitboxes;

	if (rightWire != NULL)
		delete rightWire;
	if (leftWire != NULL)
		delete leftWire;
}

void Actor::InitSounds()
{
	soundInfos.resize(PlayerSounds::S_Count);

	soundInfos[PlayerSounds::S_HITCEILING] = GetSound("Kin/ceiling");
	soundInfos[PlayerSounds::S_CLIMB_STEP1] = GetSound("Kin/climb_01a");
	//soundInfos[PlayerSounds::S_CLIMB_STEP2] = GetSound("Kin/climb_02a");
	soundInfos[PlayerSounds::S_DAIR] = GetSound("Kin/dair");
	soundInfos[PlayerSounds::S_DOUBLE] = GetSound("Kin/doublejump");
	soundInfos[PlayerSounds::S_DOUBLEBACK] = GetSound("Kin/doublejump_back");
	soundInfos[PlayerSounds::S_FAIR1] = GetSound("Kin/fair");
	soundInfos[PlayerSounds::S_JUMP] = GetSound("Kin/jump");
	soundInfos[PlayerSounds::S_LAND] = GetSound("Kin/land");
	soundInfos[PlayerSounds::S_RUN_STEP1] = GetSound("Kin/run_01a");
	soundInfos[PlayerSounds::S_RUN_STEP2] = GetSound("Kin/run_01b");
	soundInfos[PlayerSounds::S_SLIDE] = GetSound("Kin/slide");
	soundInfos[PlayerSounds::S_SPRINT_STEP1] = GetSound("Kin/sprint_01a");
	soundInfos[PlayerSounds::S_SPRINT_STEP2] = GetSound("Kin/sprint_01b");
	soundInfos[PlayerSounds::S_STANDATTACK] = GetSound("Kin/stand");
	soundInfos[PlayerSounds::S_STEEPSLIDE] = GetSound("Kin/steep");
	soundInfos[PlayerSounds::S_STEEPSLIDEATTACK] = GetSound("Kin/steep_att");
	soundInfos[PlayerSounds::S_UAIR] = GetSound("Kin/uair");
	soundInfos[PlayerSounds::S_WALLATTACK] = GetSound("Kin/wall_att");
	soundInfos[PlayerSounds::S_WALLJUMP] = GetSound("Kin/walljump");
	soundInfos[PlayerSounds::S_WALLSLIDE] = GetSound("Kin/wallslide");

	soundInfos[PlayerSounds::S_GOALKILLSLASH1] = GetSound("Kin/goal_kill_01");
	soundInfos[PlayerSounds::S_GOALKILLSLASH2] = GetSound("Kin/goal_kill_02");
	soundInfos[PlayerSounds::S_GOALKILLSLASH3] = GetSound("Kin/goal_kill_03");
	soundInfos[PlayerSounds::S_GOALKILLSLASH4] = GetSound("Kin/goal_kill_04");

	soundInfos[PlayerSounds::S_ENEMY_GATE_UNLOCKED] = GetSound("Test/Explode");
	soundInfos[PlayerSounds::S_OPEN_ENEMY_GATE] = GetSound("Zone/Gate_Open_03");

	soundInfos[PlayerSounds::S_HIT] = GetSound("Enemies/turret_shoot");

	soundInfos[PlayerSounds::S_DESTROY_GOAL] = GetSound("Test/Explode");
	soundInfos[PlayerSounds::S_LEVEL_COMPLETE] = GetSound("Zone/Level_Complete_06");

	soundInfos[PlayerSounds::S_ENTER_W1] = GetSound("Test/Crawler_Theme_01");
	soundInfos[PlayerSounds::S_ENTER_W2] = GetSound("Test/Bird_Theme_04");
	soundInfos[PlayerSounds::S_ENTER_W3] = GetSound("Test/Coyote_Theme_01");
	soundInfos[PlayerSounds::S_ENTER_W4] = GetSound("Test/Tiger_Theme_01");
	soundInfos[PlayerSounds::S_ENTER_W5] = GetSound("Test/Gator_Theme_01");
	soundInfos[PlayerSounds::S_ENTER_W6] = GetSound("Test/Skele_Theme_01");
	soundInfos[PlayerSounds::S_ENTER_W7] = GetSound("Test/Core_Theme_01");
	soundInfos[PlayerSounds::S_ENTER_W8] = GetSound("Test/Bear_Theme_01");
	soundInfos[PlayerSounds::S_HURT] = GetSound("Kin/Hurt_02");

	soundInfos[PlayerSounds::S_BOUNCEJUMP] = GetSound("Kin/bounce");


	soundInfos[PlayerSounds::S_WATER_NORMAL_ENTER] = GetSound("Water/water_normal_enter");
	soundInfos[PlayerSounds::S_WATER_NORMAL_EXIT] = GetSound("Water/water_normal_exit");
	/*soundInfos[PlayerSounds::S_WATER_NORMAL_BUBBLE_01] = GetSound("Water/water_normal_bubble_01");
	soundInfos[PlayerSounds::S_WATER_NORMAL_BUBBLE_02] = GetSound("Water/water_normal_bubble_02");
	soundInfos[PlayerSounds::S_WATER_NORMAL_BUBBLE_03] = GetSound("Water/water_normal_bubble_03");
	soundInfos[PlayerSounds::S_WATER_NORMAL_BUBBLE_04] = GetSound("Water/water_normal_bubble_04");*/
	soundInfos[PlayerSounds::S_WATER_GLIDE_ENTER] = GetSound("Water/water_glide_enter");
	soundInfos[PlayerSounds::S_WATER_GLIDE_EXIT] = GetSound("Water/water_glide_exit");
	soundInfos[PlayerSounds::S_WATER_LOWGRAV_ENTER] = GetSound("Water/water_lowgrav_enter");
	soundInfos[PlayerSounds::S_WATER_LOWGRAV_EXIT] = GetSound("Water/water_lowgrav_exit");
	soundInfos[PlayerSounds::S_WATER_HEAVYGRAV_ENTER] = GetSound("Water/water_heavygrav_enter");
	soundInfos[PlayerSounds::S_WATER_HEAVYGRAV_EXIT] = GetSound("Water/water_heavygrav_exit");
	soundInfos[PlayerSounds::S_WATER_BUOYANCY_ENTER] = GetSound("Water/water_buoyancy_enter");
	soundInfos[PlayerSounds::S_WATER_BUOYANCY_EXIT] = GetSound("Water/water_buoyancy_exit");
	soundInfos[PlayerSounds::S_WATER_ACCEL_ENTER] = GetSound("Water/water_accel_enter");
	soundInfos[PlayerSounds::S_WATER_ACCEL_EXIT] = GetSound("Water/water_accel_exit");
	soundInfos[PlayerSounds::S_WATER_ZEROGRAV_ENTER] = GetSound("Water/water_zerograv_enter");
	soundInfos[PlayerSounds::S_WATER_ZEROGRAV_EXIT] = GetSound("Water/water_zerograv_exit");
	soundInfos[PlayerSounds::S_WATER_LAUNCHER_ENTER] = GetSound("Water/water_launcher_enter");
	soundInfos[PlayerSounds::S_WATER_LAUNCHER_EXIT] = GetSound("Water/water_launcher_exit");
	soundInfos[PlayerSounds::S_WATER_MOMENTUM_ENTER] = GetSound("Water/water_momentum_enter");
	soundInfos[PlayerSounds::S_WATER_MOMENTUM_EXIT] = GetSound("Water/water_momentum_exit");
	soundInfos[PlayerSounds::S_WATER_TIMESLOW_ENTER] = GetSound("Water/water_timeslow_enter");
	soundInfos[PlayerSounds::S_WATER_TIMESLOW_EXIT] = GetSound("Water/water_timeslow_exit");
	soundInfos[PlayerSounds::S_WATER_POISON_ENTER] = GetSound("Water/water_poison_enter");
	soundInfos[PlayerSounds::S_WATER_POISON_EXIT] = GetSound("Water/water_poison_exit");
	//soundInfos[PlayerSounds::S_WATER_POISON_LOOP] = GetSound("Water/water_poison_loop");
	soundInfos[PlayerSounds::S_WATER_FREEFLIGHT_ENTER] = GetSound("Water/water_freeflight_enter");
	soundInfos[PlayerSounds::S_WATER_FREEFLIGHT_EXIT] = GetSound("Water/water_freeflight_exit");
	soundInfos[PlayerSounds::S_WATER_INVERTEDINPUTS_ENTER] = GetSound("Water/water_invertedinputs_enter");
	soundInfos[PlayerSounds::S_WATER_INVERTEDINPUTS_EXIT] = GetSound("Water/water_invertedinputs_exit");


	//soundInfos[PlayerSounds::S_GRAVREVERSE] = GetSound("Kin/gravreverse");

	/*soundInfos[S_DASH_START] = GetSound( "Kin/dash_02" );
	soundInfos[S_HIT] = GetSound( "kin_hitspack_short" );
	soundInfos[S_HURT] = GetSound( "Kin/hit_1b" );
	soundInfos[S_HIT_AND_KILL] = GetSound( "Kin/kin_hitspack" );
	soundInfos[S_HIT_AND_KILL_KEY] = GetSound( "Kin/key_kill" );

	soundInfos[S_GRAVREVERSE] = GetSound( "Kin/gravreverse" );
	soundInfos[S_BOUNCEJUMP] = GetSound( "Kin/bounce" );

	soundInfos[S_TIMESLOW] = GetSound( "Kin/time_slow_1" );
	soundInfos[S_ENTER] = GetSound( "Kin/enter" );
	soundInfos[S_EXIT] = GetSound( "Kin/exit" );

	soundInfos[S_DIAGUPATTACK] = soundBuffers[S_FAIR1];
	soundInfos[S_DIAGDOWNATTACK] = soundBuffers[S_FAIR1];*/
}

double Actor::GetMaxSpeed()
{
	//starts at 60, goes up to 100
	double upgradeAmount = 5;

	int maxSpeedUpgrades = 0;//NumUpgradeRange(UPGRADE_W6_MAX_SPEED_1, 8);

	return maxSpeed + upgradeAmount * maxSpeedUpgrades;
}


void Actor::SetGameMode()
{
	//int mapType;
	//if (sess->mapHeader == NULL)
	//{
	//	//blank editor files only:
	//	mapType = MapHeader::T_BASIC;
	//}
	//else
	//{
	//	mapType = sess->matchp
	//}
}

void Actor::SetFBubbleFrame(int i, float val)
{
	sess->fBubbleFrame[MAX_BUBBLES * actorIndex + i] = val;
}
void Actor::SetFBubblePos(int i, Vector2f &pos)
{
	sess->fBubblePos[MAX_BUBBLES * actorIndex + i] = pos;
}
void Actor::SetFBubbleRadiusSize(int i, float rad)
{
	sess->fBubbleRadiusSize[MAX_BUBBLES * actorIndex + i] = rad;
}

void Actor::SetupTimeBubbles()
{
	CollisionBox genericBox;
	genericBox.isCircle = true;
	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		bubbleHitboxes[i].boxType = CollisionBox::Hit;
		bubbleHitboxes[i].BasicSetup();
		bubbleHitboxes[i].AddCollisionBox(0, genericBox);
	}

	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		bubbleFramesToLive[i] = 0;
		SetFBubbleFrame(i, 0);
	}
}

bool Actor::CanCancelAttack()
{
	int mostRecentBlocked = MostRecentFrameCurrAttackBlocked();
	HitboxInfo &hi = hitboxInfos[action][currActionSuperLevel];
	if (currAttackHit || 
		(mostRecentBlocked >= 0 && frame - mostRecentBlocked >= hi.hitBlockCancelDelay ) )
	{
		return true;
	}

	return false;
}

double Actor::GetBounceBoostSpeed()
{
	double currBounceBoostSpeed = bounceBoostSpeed;

	double upgradeAmount = 5.0;//2.0;
	int numUpgrades = IsOptionOn(UPGRADE_W4_SCORPION_BOUNCE);
	currBounceBoostSpeed += upgradeAmount * numUpgrades;

	return currBounceBoostSpeed;
}


std::string Actor::GetDisplayName()
{
	return "";
	/*if (owner->IsReplayOn())
	{

	}*/
}

void Actor::LoadHitboxes()
{
	ifstream is;
	is.open("Resources/Kin/Info/hitboxes.json");

	assert(is.is_open());

	json j;
	is >> j;

	DIFactor = j["difactor"];
	blockstunFactor = j["blockstunfactor"];
	DIChangesMagnitude = false;

	SetupHitboxInfo( j, "fair", hitboxInfos[FAIR]);
	SetupHitboxInfo(j, "dair", hitboxInfos[DAIR]);
	SetupHitboxInfo(j, "uair", hitboxInfos[UAIR]);
	SetupHitboxInfo(j, "standattack1", hitboxInfos[STANDATTACK1]);
	SetupHitboxInfo(j, "standattack2", hitboxInfos[STANDATTACK2]);
	SetupHitboxInfo(j, "standattack3", hitboxInfos[STANDATTACK3]);
	SetupHitboxInfo(j, "standattack4", hitboxInfos[STANDATTACK4]);
	SetupHitboxInfo(j, "dashattack", hitboxInfos[DASHATTACK]);
	SetupHitboxInfo(j, "dashattack2", hitboxInfos[DASHATTACK2]);
	SetupHitboxInfo(j, "dashattack3", hitboxInfos[DASHATTACK3]);
	SetupHitboxInfo(j, "wallattack", hitboxInfos[WALLATTACK]);
	SetupHitboxInfo(j, "diagupattack", hitboxInfos[DIAGUPATTACK]);
	SetupHitboxInfo(j, "diagdownattack", hitboxInfos[DIAGDOWNATTACK]);
	SetupHitboxInfo(j, "uptilt1", hitboxInfos[UPTILT1]);
	SetupHitboxInfo(j, "uptilt2", hitboxInfos[UPTILT2]);
	SetupHitboxInfo(j, "uptilt3", hitboxInfos[UPTILT3]);
	SetupHitboxInfo(j, "homing", hitboxInfos[SPRINGSTUNHOMINGATTACK]);


	is.close();
}



void Actor::SetupHitboxInfo( json &j, const std::string &name,
	HitboxInfo *hi)
{
	auto &myj = j[name];

	auto &myNormalHitboxInfo = j[name]["normal"];
	auto &myLevel1HitboxInfo = j[name]["level1"];

	HitboxInfo::SetupHitboxLevelInfo(myNormalHitboxInfo, hi[0]);
	HitboxInfo::SetupHitboxLevelInfo(myLevel1HitboxInfo, hi[1]);
}

void Actor::ActionEnded()
{
	int actionLength = GetActionLength(action);

	if (frame >= actionLength && actionLength >= 0)
	{
		EndAction();
	}
}



void Actor::CheckHoldJump()
{
	if (InWater( TerrainPolygon::WATER_BUOYANCY) || InWater(TerrainPolygon::WATER_ZEROGRAV))//|| InWater( TerrainPolygon::WATER_NORMAL ))
	{
		if (!JumpButtonHeld())
		{
			holdJump = false;
			holdDouble = false;
		}
		return;
	}

	if (freeFlightFrames > 0)
	{
		holdJump = false;
		holdDouble = false;
		return;
	}
	
	/*if (InWater(TerrainPolygon::WATER_NORMAL))
	{

	}*/


	//if( hasDoubleJump && !holdDouble )//&& holdJump)
	if( holdJump )
	{
		if( holdJump && velocity.y >= -8 )
			holdJump = false;

		
		if( holdJump && framesInAir > 2 )
		{
			bool steepCheck = (!steepJump && !JumpButtonHeld()) || (steepJump && !HoldingRelativeUp());
			if (steepCheck)
			{
				if (velocity.y < -8)
				{
					velocity.y = -8;
				}
			}
		}
	}
	else if( holdDouble )
	{
		if (holdDouble && velocity.y >= -8)
			holdDouble = false;

		if (holdDouble && !JumpButtonHeld() && framesInAir > 2)
		{
			if (velocity.y < -8)
			{
				velocity.y = -8;
			}
		}
	}

	/*if (!JumpButtonHeld())
	{
		holdJump = false;
		holdDouble = false;
	}*/
}

bool Actor::SteepSlideAttack()
{
	if (pauseBufferedAttack == Action::STEEPSLIDEATTACK)
	{
		SetAction(pauseBufferedAttack);
		frame = 0;
		return true;
	}

	if (AttackButtonPressed())
	{
		SetAction(STEEPSLIDEATTACK);
		frame = 0;
		return true;
	}

	return false;
}

bool Actor::SteepClimbAttack()
{
	if (pauseBufferedAttack == Action::STEEPCLIMBATTACK)
	{
		SetAction(pauseBufferedAttack);
		frame = 0;
		return true;
	}

	if (AttackButtonPressed())
	{
		SetAction(STEEPCLIMBATTACK);
		frame = 0;
		return true;
	}

	return false;
}

bool Actor::AirAttack()
{
	if ( AttackButtonHeld() && homingFrames > 0 && TryHomingMovement())
	{
		pauseBufferedAttack = Action::Count;
		SetAction(HOMINGATTACK);
		return true;
	}

	if (pauseBufferedAttack != Action::Count)
	{
		SetAction(pauseBufferedAttack);
		frame = 0;
		return true;
	}

	if(AttackButtonPressed())
	{
		if( action == AIRDASH )
		{
			if (currInput.LUp())
			{
				if (currInput.LRight() || currInput.LLeft())
				{
					SetAction(DIAGUPATTACK);
					frame = 0;
					return true;
				}
			}
			else if (currInput.LDown())
			{
				if (currInput.LRight() || currInput.LLeft())
				{
					SetAction(DIAGDOWNATTACK);
					frame = 0;
					return true;
				}
			}
			else
			{
				SetAction(AIRDASHFORWARDATTACK);
				frame = 0;
				return true;
			}
		}
		
		if ((currInput.LUp()))
		{
			if (action == UAIR)
			{
				if (currLockedUairFX != NULL && action != UAIR)
				{
					currLockedUairFX->ClearLockPos();
					currLockedUairFX = NULL;
				}
			}
			SetAction(UAIR);
			frame = 0;
		}
		else if (currInput.LDown())
		{
			if (action == DAIR)
			{
				if (currLockedDairFX != NULL && action != DAIR)
				{
					currLockedDairFX->ClearLockPos();
					currLockedDairFX = NULL;
				}
			}
			SetAction(DAIR);
			frame = 0;
		}
		else
		{
			if (action == FAIR)
			{
				if (currLockedFairFX != NULL && action != FAIR)
				{
					currLockedFairFX->ClearLockPos();
					currLockedFairFX = NULL;
				}
			}
			SetAction(FAIR);
			frame = 0;
		}

		return true;

	}

	return false;
}

void Actor::CreateEnemiesClearedRing()
{
	Vector2f floatPos(position);
	enemiesClearedRingGroup->SetBase(floatPos);
	enemiesClearedRingGroup->Reset();
	enemiesClearedRingGroup->Start();
}

void Actor::CreateEnoughKeysRing()
{
	Vector2f floatPos(position);
	enoughKeysToExitRingGroup->SetBase(floatPos);
	enoughKeysToExitRingGroup->Reset();
	enoughKeysToExitRingGroup->Start();
}

void Actor::CreateGravityDecreaserOnRing()
{
	Vector2f floatPos(position);
	gravityDecreaserOnRingGroup->SetBase(floatPos);
	gravityDecreaserOnRingGroup->Reset();
	gravityDecreaserOnRingGroup->Start();
}


void Actor::CreateGravityDecreaserOffRing()
{
	Vector2f floatPos(position);
	gravityDecreaserOffRingGroup->SetBase(floatPos);
	gravityDecreaserOffRingGroup->Reset();
	gravityDecreaserOffRingGroup->Start();
}

void Actor::CreateGravityIncreaserOnRing()
{
	Vector2f floatPos(position);
	gravityIncreaserOnRingGroup->SetBase(floatPos);
	gravityIncreaserOnRingGroup->Reset();
	gravityIncreaserOnRingGroup->Start();
}


void Actor::CreateGravityIncreaserOffRing()
{
	Vector2f floatPos(position);
	gravityIncreaserOffRingGroup->SetBase(floatPos);
	gravityIncreaserOffRingGroup->Reset();
	gravityIncreaserOffRingGroup->Start();
}

void Actor::CreateGateExplosion( int gateCategory )
{
	Vector2f floatPos(position);

	if (gateCategory == Gate::ALLKEY || gateCategory == Gate::NUMBER_KEY)
	{
		keyExplodeRingGroup->SetBase(floatPos);
		keyExplodeRingGroup->Reset();
		keyExplodeRingGroup->Start();
	}
	else if( gateCategory == Gate::ENEMY)
	{
		enemyExplodeRingGroup->SetBase(floatPos);
		enemyExplodeRingGroup->Reset();
		enemyExplodeRingGroup->Start();
	}

	sess->cam.SetRumble(3, 3, 20);//5
	ActivateSound(PlayerSounds::S_OPEN_ENEMY_GATE);
	//sess->Pause(4);//5
}

void Actor::CreateKeyExplosion( int gateCategory )
{
	//this gets rid of any keys that are still around, instead of
	//letting the player collect them after going through the door
	


	EffectInstance params;
	Transform tr = sf::Transform::Identity;
	//params.SetParams(Vector2f(position), tr, 16, 4, 0);
	//params.SetVelocityParams(Vector2f(0, -1), Vector2f(), 10);

	Transform posTransform;
	Vector2f pos(0, -10);
	Vector2f adjustedPoint;

	Vector2f floatPos(position);
	for (int i = 0; i < numKeysHeld; ++i)
	{
		adjustedPoint = posTransform.transformPoint(pos);
		params.SetParams(floatPos + adjustedPoint, tr, 16, 4, 0);
		params.SetVelocityParams(normalize(adjustedPoint) * 4.f, Vector2f(), 10.f);

		ActivateEffect(PLAYERFX_KEY, &params);
		//keyExplodePool->ActivateEffect(&params);

		posTransform.rotate(360.f / numKeysHeld);
	}

	CreateGateExplosion( gateCategory );
}

void Actor::CreateAttackLightning()
{
	if ( frame != 0 || slowCounter != 1 
		|| ( action != FAIR && action != DAIR && action != UAIR && action != AIRDASHFORWARDATTACK ) )
		return;

	RelEffectInstance params;
	Transform tr = sf::Transform::Identity;
	if (!facingRight)
	{
		tr.scale(Vector2f(-1, 1));
	}

	if (speedLevel == 1)
	{
		tr.scale(1.125, 1.125);
	}
	else if (speedLevel == 2)
	{
		tr.scale(1.5, 1.5);
	}

	params.SetParams(Vector2f(0, 0), tr, 23, 1, 0, &spriteCenter );
	//fair should be 25 but meh

	/*if (!facingRight)
	{
		tr.scale(-1, 1);
	}*/

	

	switch (action)
	{
	case AIRDASHFORWARDATTACK:
	case FAIR:
		currLockedFairFX = (RelEffectInstance*)ActivateEffect(PLAYERFX_FAIR_SWORD_LIGHTNING_0 + speedLevel, &params);
		break;
	case DAIR:
		currLockedDairFX = (RelEffectInstance*)ActivateEffect(PLAYERFX_DAIR_SWORD_LIGHTNING_0 + speedLevel, &params);
		break;
	case UAIR:
		currLockedUairFX = (RelEffectInstance*)ActivateEffect(PLAYERFX_UAIR_SWORD_LIGHTNING_0 + speedLevel, &params);
		break;
	}
}

void Actor::AddActiveComboObj(ComboObject *c)
{
	//cout << "try adding comboer" << endl;
	if (c->data.active)
		return;

	
	//cout << "adding comboer" << endl;

	//int numComboers = 0;

	//ComboObject *testCurr = activeComboObjList;
	//while (testCurr != NULL)
	//{
	//	if (testCurr == c)
	//	{
	//		int xxx = 5;
	//	}

	//	++numComboers;

	//	testCurr = testCurr->nextComboObj;//activeComboObjList->nextComboObj;
	//}

	//cout << "adding comboer. size after: " << numComboers + 1 << endl;

	c->data.active = true;
	if (activeComboObjList == NULL)
	{
		activeComboObjList = c;
	}
	else
	{
		c->data.nextComboObjectID = sess->GetComboObjectID(activeComboObjList);
		activeComboObjList = c;
	}
}

void Actor::RemoveActiveComboObj(ComboObject *c)
{
	//cout << "try removing comboer" << endl;

	if (!c->data.active)
		return;

	if (activeComboObjList == NULL)
		return;
	//assert(activeComboObjList != NULL);

	//cout << "removing comboer" << endl;

	//int numComboers = 0;

	//ComboObject *testCurr = activeComboObjList;
	//while (testCurr != NULL)
	//{
	//	++numComboers;

	//	testCurr = testCurr->nextComboObj;//activeComboObjList->nextComboObj;
	//}



	c->data.active = false;
	if (c == activeComboObjList)
	{
		activeComboObjList = sess->GetComboObjectFromID( activeComboObjList->data.nextComboObjectID );
	}

	ComboObject *curr = activeComboObjList;
	ComboObject *prev = NULL;
	while (curr != NULL)
	{
		if (curr == c)
		{
			prev->data.nextComboObjectID = curr->data.nextComboObjectID;
			break;
		}

		prev = curr;
		curr = sess->GetComboObjectFromID(curr->data.nextComboObjectID);
	}

	c->data.nextComboObjectID = -1;
}

void Actor::ClearActiveComboObjects()
{
	while (activeComboObjList != NULL)
	{
		RemoveActiveComboObj(activeComboObjList);
	}
}

void Actor::DebugDrawComboObj(sf::RenderTarget *target)
{
	ComboObject *curr = activeComboObjList;
	while (curr != NULL)
	{
		curr->Draw(target);
		curr = sess->GetComboObjectFromID(curr->data.nextComboObjectID);
	}
}

void Actor::Respawn( bool setStartPos )
{
	swordShader.SetSkin(0);


	gravityIncreaserTrailEmitter->Reset();
	gravityDecreaserTrailEmitter->Reset();
	momentumBoosterTrailEmitter->Reset();
	//timeSlowBoosterTrailEmitter->Reset();
	homingBoosterTrailEmitter->Reset();
	antiTimeSlowBoosterTrailEmitter->Reset();
	freeFlightBoosterTrailEmitter->Reset();


	effectPools[PLAYERFX_KEY].pool->ts = sess->ts_key;
	effectPools[PLAYERFX_KEY_EXPLODE].pool->ts = sess->ts_keyExplode;

	if (sess->mapHeader != NULL)
	{
		numFramesToLive = min( sess->mapHeader->drainSeconds * 60, 
			MAX_FRAMES_TO_LIVE);
	}
	else
	{
		numFramesToLive = -1;
	}

	hitCeilingSoundPlayedThisFrame = false;
	waterEntranceGround = NULL;
	waterEntranceGrindEdge = NULL;

	receivedHitPlayer = NULL;

	practiceDesyncDetected = false;
	practiceDesyncPosition = V2d();

	bool parallelPracticeAndImParallel = sess->gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && sess->IsParallelSession();

	if (owner != NULL && owner->IsReplayOn())
	{
		//do nothing, powers were already set by the replay
	}
	else
	{
		if (parallelPracticeAndImParallel)
		{
			if (sess->netplayManager != NULL)
			{
				assert(sess->netplayManager != NULL);

				string pName = sess->netplayManager->practicePlayers[sess->parallelSessionIndex].name;
				nameTag->SetName(pName);
				nameTag->SetActive(true);
			}
		}
		else if( sess->gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE && sess->netplayManager != NULL )
		{
			int trueIndex = -1;
			int parIndex = sess->parallelSessionIndex;
			int playerIndex = sess->netplayManager->playerIndex;
		
			if (parIndex == -1)
			{
				trueIndex = playerIndex;
			}
			else if( parIndex < playerIndex)
			{
				trueIndex = parIndex;
			}
			else
			{
				trueIndex = parIndex + 1;
			}

			string pName = sess->netplayManager->netplayPlayers[trueIndex].name;
			nameTag->SetName(pName);
			nameTag->SetActive(true);
		}
		else
		{
			nameTag->SetActive(false);
		}
	}

	if (owner != NULL && owner->IsReplayOn())
	{
		//do nothing, powers were already set by the replay
	}
	else
	{
		SetAllOptions(sess->currPlayerOptionsField);
	}

	/*if (parallelPracticeAndImParallel)
	{
		const BitField &practiceUpgradeField = sess->GetPracticeUpgradeField();

		SetAllUpgrades(practiceUpgradeField);
	}
	else if (owner != NULL && owner->IsReplayOn())
	{
		
	}
	else if (owner != NULL && owner->saveFile != NULL )
	{
		if (owner->originalProgressionModeOn)
		{
			originalProgressionUpgradeField.Set(owner->saveFile->upgradeField);
			originalProgressionUpgradeField.And(owner->originalProgressionPlayerOptionsField);

			SetAllUpgrades(originalProgressionUpgradeField);

			originalProgressionLogField.Set(owner->saveFile->logField);
			originalProgressionLogField.And(owner->originalProgressionLogField);
		}
		else
		{
			SetAllUpgrades(owner->saveFile->upgradeField);
		}
		
	}
	else
	{
		SetAllUpgrades(sess->defaultStartingPlayerOptionsField);
	}*/

	spriteAction = HIDDEN;

	prevInput.Clear();
	currInput.Clear();
	storedGroundSpeed = 0;
	currAirdashBoostCounter = 0;
	edgeQuantity = 0;
	extraAirDashY = 0;
	storedReverseSpeed = 0;
	setHitstunFrames = 0;

	numCalculatedFuturePositions = 0;

	bouncedFromKill = false;

	ClearAllEffects();

	hitEnemyDuringPhysics = false;

	groundedWallBounce = false;
	
	inBubble = false;
	oldInBubble = false;
	specialSlow = false;
	ClearRecentHitters();
	directionalInputFreezeFrames = 0;
	frameAfterAttackingHitlagOver = false;
	invertInputFrames = 0;
	airBounceCounter = 0;
	//fallThroughDuration = 0;
	globalTimeSlowFrames = 0;
	freeFlightFrames = 0;
	antiTimeSlowFrames = 0;
	homingFrames = 0;
	projectileSwordFrames = 0;
	enemyProjectileSwordFrames = 0;
	rewindOnHitFrames = 0;
	momentumBoostFrames = 0;
	currSpecialTerrain = NULL;
	oldSpecialTerrain = NULL;
	//stays between respawns.
	//currPowerMode = PMODE_SHIELD; 
	touchedCoyoteHelper = false;
	coyoteBoostFrames = 0;

	for (int i = 0; i < NUM_SWORD_PROJECTILES; ++i)
	{
		swordProjectiles[i]->Reset();
	}

	ResetGrassCounters();
	ResetAttackHit();

	shieldPushbackFrames = 0;
	currActionSuperLevel = 0;
	blockstunFrames = 0;
	superLevelCounter = 0;
	
	framesSinceBlockPress = -1;
	framesSinceSuperPress = -1;
	
	dashAttackLevel = 0;
	standAttackLevel = 0;
	upTiltLevel = 0;
	downTiltLevel = 0;
	framesSinceDashAttack = 0;
	framesSinceStandAttack = 0;
	framesSinceUpTilt = 0;
	framesSinceDownTilt = 0;

	standNDashBoost = false;
	currencyCounter = 0;

	if (owner != NULL)
	{
		if (owner->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
		{
			//if (!owner->IsReplayOn() && !owner->IsParallelSession())
			if (!owner->IsParallelSession())
			{
				SaveFile *currFile = adventureManager->currSaveFile;
				currencyCounter = currFile->currency;
				AddToCurrencyCounter(0);
			}
		}
	}

	kinMode = K_NORMAL;
	action = -1;
	framesStanding = 0;
	keyExplodeRingGroup->Reset();
	enemyExplodeRingGroup->Reset();
	enemiesClearedRingGroup->Reset();
	enoughKeysToExitRingGroup->Reset();
	gravityDecreaserOnRingGroup->Reset();
	gravityDecreaserOffRingGroup->Reset();

	gravityIncreaserOnRingGroup->Reset();
	gravityIncreaserOffRingGroup->Reset();
	numKeysHeld = 0;
	//glideEmitter->Reset();
	//owner->AddEmitter(glideEmitter, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES);
	autoRunStopEdge = NULL;
	extraDoubleJump = false;

	if( kinMask != NULL )
		kinMask->Reset();

	glideTurnFactor = 0;
	storedTrigger = NULL;
	airTrigBehavior = AT_NONE;
	currAirTrigger = NULL;


	optionField.Set(startOptionField);

	activeComboObjList = NULL;

	gravModifyFrames = 0;
	boosterGravModifyFrames = 0;

	velocity.x = 0;
	velocity.y = 0;
	reversed = false;
	b.offset.y = 0;
	b.rh = normalHeight;
	facingRight = true;
	offsetX = 0;
	prevInput.Clear();// = ControllerState();
	currInput.Clear();// = ControllerState();
	ground = NULL;
	grindEdge = NULL;
	dead = false;

	hitlagFrames = 0;
	hitstunFrames = 0;
	invincibleFrames = 0;
	receivedHit.hType = HitboxInfo::NO_HITBOX;
	speedParticleCounter = 1;
	speedLevel = 0;
	currentSpeedBar = 0;//60;

	//int numStartMomentumUpgrades = 0;
	bool hasStartMomentumUpgrade = false;
	if (sess->mapHeader != NULL)
	{
		switch (sess->mapHeader->envWorldType)
		{
		case 0:
			hasStartMomentumUpgrade = IsOptionOn(UPGRADE_W1_INCREASE_STARTING_MOMENTUM);
			break;
		case 1:
			hasStartMomentumUpgrade = IsOptionOn(UPGRADE_W2_INCREASE_STARTING_MOMENTUM);
			break;
		case 2:
			hasStartMomentumUpgrade = IsOptionOn(UPGRADE_W3_INCREASE_STARTING_MOMENTUM);
			break;
		case 3:
			hasStartMomentumUpgrade = IsOptionOn(UPGRADE_W4_INCREASE_STARTING_MOMENTUM);
			break;
		case 4:
			hasStartMomentumUpgrade = IsOptionOn(UPGRADE_W5_INCREASE_STARTING_MOMENTUM);
			break;
		case 5:
			hasStartMomentumUpgrade = IsOptionOn(UPGRADE_W6_INCREASE_STARTING_MOMENTUM);
			break;
		}
	}

	//float startMomentumUpgradeFactor = 15.0;
	float startMomentumUpgradeFactor = 40;//15.0;
	if (hasStartMomentumUpgrade)
	{
		currentSpeedBar = startMomentumUpgradeFactor;
	}
	//currentSpeedBar = startMomentumUpgradeFactor * numStartMomentumUpgrades;

	currBBoostCounter = 0;
	currTutorialObject = NULL;
	currInspectObject = NULL;
	repeatingSound = NULL;
	currBooster = NULL;
	currFreeFlightBooster = NULL;
	currTimeBooster = NULL;
	currAntiTimeSlowBooster = NULL;
	currSwordProjectileBooster = NULL;
	currSpring = NULL;
	currAimLauncher = NULL;
	currTeleporter = NULL;
	oldTeleporter = NULL;
	currMomentumBooster = NULL;
	currSwingLauncher = NULL;
	oldSwingLauncher = NULL;
	currRewindBooster = NULL;
	currHomingBooster = NULL;
	oldBooster = NULL;
	currScorpionLauncher = NULL;
	oldScorpionLauncher = NULL;

	currBounceBooster = NULL;
	oldBounceBooster = NULL;

	currGravModifier = NULL;
	extraGravityModifier = 1.0;
	boosterExtraGravityModifier = 1.0;
	
	slowMultiple = baseSlowMultiple;
	slowCounter = 1;

	currWall = NULL;
	wallClimbGravityOn = false;
	currHurtboxes = NULL;
	currHitboxes = NULL;

	standNDashBoostCurr = 0;
	ClearPauseBufferedActions();
	currLockedFairFX = NULL;
	currLockedDairFX = NULL;
	currLockedUairFX = NULL;

	ClearAllEffects();

	regrindOffCount = 3;
	prevRail = NULL;
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	canRailSlide = false;
	canRailGrind = false;

	framesSinceRightWireBoost = 0;
	framesSinceLeftWireBoost = 0;
	framesSinceDoubleWireBoost = 0;

	speedParticleCounter = 0;
	framesNotGrinding = 0;
	bufferedAttack = Actor::JUMP;

	//clean up the checkpoint code at some point

	hitGoal = false;
	hitNexus = NULL;

	if( owner != NULL || setStartPos )
		SetToOriginalPos();

	enemiesKilledThisFrame = 0;
	enemiesKilledLastFrame = 0;
	gateTouched = NULL;


	if( owner != NULL && owner->shipEnterScene == NULL )
	{
		//SetAction(INTROBOOST);
		SetAction(SPAWNWAIT);
		frame = 0;
		//ActivateEffect(DrawLayer::IN_FRONT, GetTileset("Kin/FX/enter_fx_320x320.png", 320, 320), position, false, 0, 6, 2, true);
	}
	else if (owner == NULL && editOwner != NULL)
	{
		/*SetAction(JUMP);
		frame = 1;*/
		SetAction(SPAWNWAIT);
		frame = 0;
	}

	


	if(IsOptionOn( POWER_LWIRE ) )
	{
		leftWire->Reset();
	}
	if(IsOptionOn(POWER_RWIRE))
	{
		rightWire->Reset();
	}
	
	//powerBar.Reset();

	flashFrames = 0;
	
	RestoreAirOptions();

	for( int i = 0; i < MAX_BUBBLES; ++i )
	{
		bubbleFramesToLive[i] = 0;
		
	}

	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		SetFBubbleFrame(i, 0);
	}
	
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->Reset();
	}
	//for( int i = 0; i < maxMotionGhosts; ++i )
	//{
	//	motionGhosts[i].setPosition( position.x, position.y );
	//}



	//kinFace.setTextureRect(ts_kinFace->GetSubRect(expr + 6));
	//kinFaceBG.setTextureRect(ts_kinFace->GetSubRect(0));

	
	framesSinceBounce = -1; //haven't bounced yet
	ResetAttackHit();
	bounceAttackHit = false;
	flashFrames = 0;
	bufferedAttack = JUMP;
	doubleJumpBufferedAttack = DOUBLE;
	wallJumpBufferedAttack = WALLJUMP;
	pauseBufferedAttack = Action::Count;
	pauseBufferedJump = false;
	pauseBufferedDash = false;

	oldAction = action;

	framesGrinding = 0;
	holdJump = false;
	steepJump = false;
	airDashStall = false;
	distanceGrinded = 0;
	framesInAir = 0;
	wallJumpFrameCounter = 0;
	framesSinceClimbBoost = 0;
	groundSpeed = 0;
	collision = false;
	grindSpeed = 0;
	bounceGrounded = false;
	touchEdgeWithLeftWire = false;
	touchEdgeWithRightWire = false;
	currBubble = 0;

	BounceFlameOff();

	SetSkin(SKIN_NORMAL);

	//some v
	//doubleJumpBufferedAttack
}

void Actor::ResetAttackHit()
{
	swordState = SWORDSTATE_REGULAR;
	currAttackHit = false;
	for (int i = 0; i < 4; ++i)
	{
		currAttackHitBlock[i] = -1;
	}
}

double Actor::GetBounceFlameAccel()
{
	double bounceFlameAccel = 0;
	switch( speedLevel )
	{
	case 0:
		
		bounceFlameAccel = bounceFlameAccel0;
		//cout << "zero: " << bounceFlameAccel << endl;
		break;
	case 1:
		bounceFlameAccel = bounceFlameAccel1;
		//cout << "one: " << bounceFlameAccel << endl;
		break;
	case 2:
		
		bounceFlameAccel = bounceFlameAccel2;
		//cout << "two: " << bounceFlameAccel << endl;
		break;
	}
	assert( bounceFlameAccel != 0 );

	return bounceFlameAccel;
}

void Actor::CheckForAirTrigger()
{
	if (owner == NULL)
		return;

	currAirTrigger = NULL;
	queryType = Q_AIRTRIGGER;
	Rect<double> r(position.x - b.rw + b.offset.x, position.y - b.rh + b.offset.y, 2 * b.rw, 2 * b.rh);
	owner->airTriggerTree->Query(this, r);
	if (currAirTrigger != NULL)
	{
		switch (currAirTrigger->triggerType)
		{
		case AirTrigger::AUTORUNRIGHT:
		case AirTrigger::AUTORUNRIGHTAIRDASH:
			airTrigBehavior = AT_AUTORUNRIGHT;
			break;
		}
		currAirTrigger = NULL;
	}
}

void Actor::HandleAirTrigger()
{
	if (currAirTrigger != NULL)
	{
		if (airTrigBehavior == AT_AUTORUNRIGHT)
		{
			if (ground != NULL)
			{
				owner->HideHUD(60);
				SetAction(AUTORUN);
				frame = 0;
				maxAutoRunSpeed = 25;
				facingRight = true;
				airTrigBehavior = AT_NONE;
			}
		}
	}
}

void Actor::KinModeUpdate()
{
	if( kinMode == K_DESPERATION && !simulationMode )
	{
		

		int transFrames = 8;
		
		

		Color blah[8];// = { Color( 0x00, 0xff, 0xff ), Color(0x00, 0xbb, 0xff ) };
		blah[0] = Color( 0x00, 0xff, 0xff );
		blah[1] = Color( 0x00, 0xbb, 0xff );
		int cIndex = 2;

		bool allColorsOn = true;

		if(IsOptionOn( POWER_AIRDASH ) || allColorsOn)
		{
			blah[cIndex] = Color( 0x00, 0x55, 0xff );
			cIndex++;
		}
		if(IsOptionOn(POWER_GRAV) || allColorsOn)
		{
			blah[cIndex] = Color( 0x00, 0xff, 0x88 );
			cIndex++;
		}
		if(IsOptionOn(POWER_BOUNCE) || allColorsOn)
		{
			blah[cIndex] = Color( 0xff, 0xff, 0x33 );
			cIndex++;
		}

		if(IsOptionOn(POWER_GRIND) || allColorsOn)
		{
			blah[cIndex] = Color( 0xff, 0x88, 0x00 );
			cIndex++;
		}

		if(IsOptionOn(POWER_TIME ) || allColorsOn)
		{
			blah[cIndex] = Color( 0xff, 0x00, 0x00 );
			cIndex++;
		}

		if(IsOptionOn(POWER_LWIRE) || IsOptionOn(POWER_RWIRE) || allColorsOn)
		{
			blah[cIndex] = Color( 0xff, 0x33, 0xaa );
			cIndex++;
		}
		int numColors = cIndex;

		int tFrame = GetSurvivalFrame() % transFrames;
		int ind = (GetSurvivalFrame() / transFrames) % numColors;

		float fac = (float)tFrame / transFrames;

		Color currCol = blah[ind];
		Color nextCol;
		int nextIndex;
		if( ind == numColors - 1 )
		{
			nextIndex = 0;
		}
		else
		{
			nextIndex = ind + 1;
		}

		int skinBase = 22;

		BlendSkins(ind + skinBase, nextIndex + skinBase, fac);

		nextCol = blah[nextIndex];
		
		currentDespColor.r = floor(currCol.r * ( 1.f - fac ) + nextCol.r * fac + .5);
		currentDespColor.g = floor(currCol.g * ( 1.f - fac ) + nextCol.g * fac + .5);
		currentDespColor.b = floor(currCol.b * ( 1.f - fac ) + nextCol.b * fac + .5);

		float overallFac = (float)GetSurvivalFrame() / 60;
		overallFac = std::min( overallFac, 1.f );
		Color auraColor( 0x66, 0xdd, 0xff );
		auraColor.r = floor( auraColor.r * ( 1.f - overallFac ) + Color::Black.r * fac + .5 );
		auraColor.g = floor( auraColor.g * ( 1.f - overallFac ) + Color::Black.g * fac + .5 );
		auraColor.b = floor( auraColor.b * ( 1.f - overallFac ) + Color::Black.b * fac + .5 );
		//sh.setUniform( "despColor", ColorGL(currentDespColor) );
		despFaceShader.setUniform( "toColor", ColorGL(currentDespColor) );
		playerDespShader.setUniform("toColor", ColorGL(currentDespColor));

		if( numFramesToLive == 0 )
		{
			SetKinMode(K_NORMAL);
			SetAction(DEATH);
			rightWire->Reset();
			leftWire->Reset();
			slowCounter = 1;
			frame = 0;
			sess->cam.SetRumble(15, 15, GetActionLength( DEATH ), 3);
			//springStunFrames = 0;


			sess->deathSeq->Reset();
			sess->SetActiveSequence(sess->deathSeq);

			for (int i = 0; i < 3; ++i)
			{
				effectPools[PLAYERFX_FAIR_SWORD_LIGHTNING_0 + i].pool->Reset();
				effectPools[PLAYERFX_DAIR_SWORD_LIGHTNING_0 + i].pool->Reset();
				effectPools[PLAYERFX_UAIR_SWORD_LIGHTNING_0 + i].pool->Reset();	
			}
			
		}
	}
	else if (kinMode == K_SUPER)
	{
		Color superColor(230, 242, 21, 255);
		playerSuperShader.setUniform("u_auraColor", ColorGL(superColor));
		++superFrame;

		if (superFrame == maxSuperFrames)
		{
			/*if (kinRing != NULL && kinRing->powerRing->IsEmpty())
			{
				SetKinMode(K_DESPERATION);
			}
			else
			{
				SetKinMode(K_NORMAL);
			}*/
		}
	}
}

int Actor::GetSurvivalFrame()
{
	return maxDespFrames - numFramesToLive;
}

void Actor::ReverseVerticalInputs()
{
	//if (reversed)
	{
		bool up = currInput.LUp();
		bool down = currInput.LDown();

		if (up) currInput.leftStickPad -= 1;
		if (down) currInput.leftStickPad -= 2;

		if (up) currInput.leftStickPad += 2;
		if (down) currInput.leftStickPad += 1;
	}
}

//directional influence
V2d Actor::GetAdjustedKnockback(const V2d &kbVec )
{
	double len = length(kbVec);

	V2d modDir(0, 0);
	if (currInput.LUp())
	{
		modDir.y = -1;
	}
	if (currInput.LDown())
	{
		modDir.y = 1;
	}
	if (currInput.LLeft())
	{
		modDir.x = -1;
	}
	if (currInput.LRight())
	{
		modDir.x = 1;
	}

	modDir = normalize(modDir);
	modDir *= len * DIFactor;


	//modDir *=  * DIFactor;

	if (DIChangesMagnitude)
	{
		//cout << "mod: " << modDir.x << ", " << modDir.y << "\n";
		return kbVec + modDir;
	}
	else
	{
		return normalize(kbVec + modDir) * len;
	}
	
}

bool Actor::IsActionGroundBlock(int a)
{
	return a == GROUNDBLOCKDOWN || a == GROUNDBLOCKDOWNFORWARD
		|| a == GROUNDBLOCKFORWARD || a == GROUNDBLOCKUPFORWARD
		|| a == GROUNDBLOCKUP;
}

bool Actor::IsActionAirBlock(int a)
{
	return a == AIRBLOCKDOWN || a == AIRBLOCKDOWNFORWARD
		|| a == AIRBLOCKFORWARD || a == AIRBLOCKUPFORWARD || a == AIRBLOCKUP;
}

void Actor::ResetBoosterEffects()
{
	boosterExtraGravityModifier = 1.0;
	boosterGravModifyFrames = 0;
	startBoosterGravModifyFrames = 0;
	gravityIncreaserTrailEmitter->SetOn(false);
	gravityDecreaserTrailEmitter->SetOn(false);

	homingFrames = 0;
	startHomingFrames = 0;
	homingBoosterTrailEmitter->SetOn(false);

	freeFlightFrames = 0;
	startFreeFlightFrames = 0;
	gravModifyFrames = 0;
	extraGravityModifier = 1.0;
	freeFlightBoosterTrailEmitter->SetOn(false);
	
	momentumBoostFrames = 0;
	startMomentumBoostFrames = 0;
	momentumBoosterTrailEmitter->SetOn(false);

	antiTimeSlowFrames = 0;
	startAntiTimeSlowFrames = 0;
	antiTimeSlowBoosterTrailEmitter->SetOn(false);
	//hopefully this doesn't mess with water stuff. test it.
	SetSkin(SKIN_NORMAL);

	globalTimeSlowFrames = 0;
	startGlobalTimeSlowFrames = 0;
}

void Actor::ProcessReceivedHit()
{
	if (receivedHit.hType != HitboxInfo::NO_HITBOX)
	{
		assert(action != DEATH);

		attackingHitlag = false;
		hitlagFrames = receivedHit.hitlagFrames + receivedHit.extraDefenderHitlag;

		
		/*else
		{
			if( receivedHit->hitPosType == HitboxInfo::HitPosType::GROUND)
		}*/
		//receivedHit->hitPosType = 
		
		ActivateEffect(PLAYERFX_HURT_SPACK, Vector2f(position), 0, 12, 1, facingRight);

		switch (receivedHitReaction)
		{
		case HitResult::FULLBLOCK:
		case HitResult::HALFBLOCK:
		{
			if (!receivedHit.knockbackOnBlock)
			{
				receivedHit.knockback = 0;
				receivedHit.drainX = 0;
				receivedHit.drainY = 0;
			}

			if (IsActionGroundBlock(action))
			{
				blockstunFrames = receivedHit.hitstunFrames * blockstunFactor;
				invincibleFrames = 0;

				V2d otherPos = receivedHitPosition;
				double kbImpulse = 6;

				double shieldKBFactor = 1.0;
				
				if (otherPos.x < position.x)
				{
					groundSpeed += receivedHit.knockback * shieldKBFactor;
				}
				else
				{
					groundSpeed -= receivedHit.knockback * shieldKBFactor;//kbImpulse;
				}
			}
			else if (IsActionAirBlock(action))
			{
				blockstunFrames = receivedHit.hitstunFrames / 2;
				invincibleFrames = 0;

				V2d otherPos = receivedHitPosition;

				double shieldKBFactor = 1.0;//2.0;//.5;

				V2d kb = CalcKnockback(&receivedHit) * shieldKBFactor;

				if (kb.x != 0 || kb.y != 0)
				{
					velocity = kb;
				}
				

				RestoreAirOptions();
			}

			if (receivedHitReaction == FULLBLOCK)
			{
				shieldShader.setUniform("toColor", ColorGL(fullBlockShieldColor));
			}
			else
			{
				shieldShader.setUniform("toColor", ColorGL(halfBlockShieldColor));
			}
			
			break;
		}
		case HitResult::PARRY:
		{
			if (IsActionAirBlock(action))
			{
				SetAction(AIRPARRY);
				V2d(0, 0);
			}
			else
			{
				if (action == GROUNDBLOCKFORWARD)
				{
					SetAction(GROUNDPARRY);
				}
				else if (action == GROUNDBLOCKDOWNFORWARD)
				{
					SetAction(GROUNDPARRYLOW);
				}
				else
				{
					//until implemented everything
					SetAction(GROUNDPARRY);
				}
				frame = 0;
				groundSpeed = 0;
			}
			
			invincibleFrames = 10;

			break;
		}			
		case HitResult::HIT:
		{
			ReactToBeingHit();
			break;
		}
		}

		receivedHit.hType = HitboxInfo::NO_HITBOX;
	}
}

void Actor::ReactToBeingHit()
{
	if (IsHitstunAction( action ))
	{
		hitOutOfHitstunLastFrame = true;
	}

	blockstunFrames = 0;
	hitstunFrames = receivedHit.hitstunFrames;
	setHitstunFrames = hitstunFrames;
	hitstunGravMultiplier = receivedHit.gravMultiplier;
	if (receivedHit.invincibleFrames == -1)
	{
		invincibleFrames = receivedHit.hitstunFrames + 20;//25;//receivedHit->damage;
	}
	else
	{
		invincibleFrames = receivedHit.invincibleFrames;
	}

	ActivateSound(PlayerSounds::S_HURT);

	if (sess->gameModeType == MatchParams::GAME_MODE_FIGHT)
	{
		FightMode *fm = (FightMode*)sess->gameMode;

		int realDamage = receivedHit.damage;
		if (receivedHitPlayer == NULL)
		{
			realDamage = receivedHit.damage / 10; //enemies do too much dmg for this mode
		}

		fm->data.health[actorIndex] -= realDamage;
		if (fm->data.health[actorIndex] <= 0)
		{
			fm->data.health[actorIndex] = 0;

			if (receivedHitPlayer != NULL)
			{
				fm->data.killCounter[receivedHitPlayer->actorIndex]++;
				fm->KillPlayer(actorIndex);
			}

			SetAction(DEATH);
			return;
		}
	}
	else
	{
		int damage = receivedHit.damage;

		bool hasDamageUpgrade = false;

		switch (receivedHit.hType)
		{
		case HitboxInfo::BLUE:
		{
			hasDamageUpgrade = IsOptionOn(UPGRADE_W1_DECREASE_ENEMY_DAMAGE);
			break;
		}
		case HitboxInfo::GREEN:
		{
			hasDamageUpgrade = IsOptionOn(UPGRADE_W2_DECREASE_ENEMY_DAMAGE);
			break;
		}
		case HitboxInfo::YELLOW:
		{
			hasDamageUpgrade = IsOptionOn(UPGRADE_W3_DECREASE_ENEMY_DAMAGE);
			break;
		}
		case HitboxInfo::ORANGE:
		{
			hasDamageUpgrade = IsOptionOn(UPGRADE_W4_DECREASE_ENEMY_DAMAGE);
			break;
		}
		case HitboxInfo::RED:
		{
			hasDamageUpgrade = IsOptionOn(UPGRADE_W5_DECREASE_ENEMY_DAMAGE);
			break;
		}
		case HitboxInfo::MAGENTA:
		{
			hasDamageUpgrade = IsOptionOn(UPGRADE_W6_DECREASE_ENEMY_DAMAGE);
			break;
		}
		/*case HitboxInfo::GREY:
		{
			damageUpgrades = NumUpgradeRange(UPGRADE_W7_DECREASE_DAMAGE_1, 3);
			break;
		}*/

		}

		float dmg = damage;
		float upgradeFactor = .5;//.2;

		if (hasDamageUpgrade)
		{
			dmg -= upgradeFactor;
		}
		damage = dmg;

		if (damage > 0)
		{
			DrainTimer(damage);
		}
	}
		

	//makes being hit always make you aerial
	//its very useful in multiplayer
	//might need to tune/figure out how to use for singleplayer
	if (ground != NULL )
	{
		reversed = false;
		ground = NULL;
		SetAction(JUMP);
		frame = 1;
	}

	if (true)
	{
		bool onRail = IsOnRailAction(action) || (grindEdge != NULL && action == JUMPSQUAT);
		if (grindEdge != NULL && !onRail)
		{
			//do something different for grind ball? you don't wanna be hit out at a sensitive moment
			//owner->powerWheel->Damage( receivedHit->damage ); //double damage for now bleh
			//grindSpeed *= .8;

			V2d op = position;

			V2d grindNorm = grindEdge->Normal();

			if (grindNorm.y < 0)
			{
				double extra = 0;
				if (grindNorm.x > 0)
				{
					offsetX = b.rw;
					extra = .1;
				}
				else if (grindNorm.x < 0)
				{
					offsetX = -b.rw;
					extra = -.1;
				}
				else
				{
					offsetX = 0;
				}

				position.x += offsetX + extra;

				position.y -= normalHeight + .1;

				if (!CheckStandUp())
				{
					position = op;

					DrainTimer(receivedHit.damage);

					//apply extra damage since you cant stand up
				}
				else
				{
					HitOutOfGrind();
				}

			}
			else
			{

				if (grindNorm.x > 0)
				{
					position.x += b.rw + .1;
				}
				else if (grindNorm.x < 0)
				{
					position.x += -b.rw - .1;
				}

				if (grindNorm.y > 0)
					position.y += normalHeight + .1;

				if (!CheckStandUp())
				{
					position = op;

					//owner->powerWheel->Damage( receivedHit->damage );

					DrainTimer(receivedHit.damage);

					//apply extra damage since you cant stand up
				}
				else
				{
					//abs( e0n.x ) < wallThresh )

					if (!IsOptionOn(POWER_GRAV) || (abs(grindNorm.x) >= wallThresh) || grindEdge->IsInvisibleWall())
					{
						HitOutOfCeilingGrindIntoAir();
					}
					else
					{
						HitOutOfCeilingGrindAndReverse();
					}
				}
			}

		}
		else if (ground == NULL || onRail)
		{
			HitWhileAerial();
		}
		else
		{
			HitWhileGrounded();
		}
		bounceEdge = NULL;
	}

	//if (dmgRet > 0 && kinMode != K_DESPERATION)
	//{
	//	SetKinMode(K_NORMAL);
	//	//action = DEATH;
	//	//frame = 0;
	//}
}

void Actor::SetKinMode(Mode m)
{
	if (kinMode == K_DESPERATION )
	{
		sess->pokeTriangleScreenGroup->StopGenerating();
	}

	if (kinMode == m)
	{
		return;
	}
	kinMode = m;
	switch (m)
	{
	case K_NORMAL:
		
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
		break;
	case K_SUPER:
		superFrame = 0;
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
		break;
	case K_DESPERATION:
		sess->pokeTriangleScreenGroup->Start();
		SetExpr(KinMask::Expr::Expr_DESP);
		
		break;
	}
}

void Actor::UpdateDrain()
{
	if (sess->gameModeType != MatchParams::GAME_MODE_BASIC
		&& sess->gameModeType != MatchParams::GAME_MODE_PARALLEL_RACE
		&& sess->gameModeType != MatchParams::GAME_MODE_PARALLEL_PRACTICE)
	{
		return;
	}

	if (simulationMode)
	{
		return; //dont drain while simulating player for now
	}

	if (action != DEATH && sess->timerOn
		&& (sess->currentZone == NULL || sess->currentZone->zType != Zone::MOMENTA))
	{
		//!IsIntroAction(action)
		if (sess->drain
			&& !IsGoalKillAction(action) && !IsExitAction(action) && !IsSequenceAction(action)
			&& sess->activeSequences[0] == NULL)
		{
			if (numFramesToLive > 0)
			{
				numFramesToLive--;

				if (kinMode == K_NORMAL)
				{
					if (numFramesToLive <= maxDespFrames)
					{
						SetKinMode(K_DESPERATION);
						if (numFramesToLive < maxDespFrames)
						{

						}
					}
				}
			}
			
		}
	}
}

void Actor::DrainTimer(int drainFrames)
{
	if (kinMode == K_DESPERATION)
	{
		return;
	}

	if (drainFrames == 0)
		return;


	if (numFramesToLive - drainFrames < maxDespFrames )//if its not the normal timer
	{
		drainFrames = numFramesToLive - maxDespFrames;
	}

	assert(drainFrames > 0);
	numFramesToLive -= drainFrames;
	if (numFramesToLive < 0)
	{
		numFramesToLive = 0;
	}

	AdventureHUD *ah = sess->GetAdventureHUD();
	if (ah != NULL)
	{
		ah->modifierTimer->SetModifiedMinus(drainFrames);
	}
}

void Actor::HealTimer(int healFrames)
{
	if (healFrames == 0)
	{
		return;
	}
	assert(healFrames > 0);
	numFramesToLive += healFrames;

	if (numFramesToLive > MAX_FRAMES_TO_LIVE)
	{
		numFramesToLive = MAX_FRAMES_TO_LIVE;
	}

	if (numFramesToLive > maxDespFrames)
	{
		SetKinMode(K_NORMAL);
		SetSkin(SKIN_NORMAL);
	}

	AdventureHUD *ah = sess->GetAdventureHUD();
	if (ah != NULL)
	{
		ah->modifierTimer->SetModifiedPlus(healFrames);
	}
}

void Actor::ProcessGravityGrass()
{
	if (ground != NULL && ground->rail != NULL && ground->rail->GetRailType() == TerrainRail::CEILING)
	{
		return;
	}

	//drop out of reverse from gravity grass
	if (ground != NULL && reversed
		&& action != GROUNDTECHBACK && action != GROUNDTECHFORWARD
		&& action != GROUNDTECHINPLACE
		&& ((!IsOptionOn(POWER_GRAV)
		&& !touchedGrass[Grass::GRAVREVERSE]) 
		|| touchedGrass[Grass::ANTIGRAVREVERSE]))
	{
		//testgrasscount is from the previous frame. if you're not touching anything in your current spot.
		//need to delay a frame so that the player can see themselves not being in the grass
		//before they fall
		//so you dont jump straight up on a nearly vertical edge
		double blah = .5;

		V2d dir(0, 0);

		dir.y = .2;
		V2d along = normalize(ground->v1 - ground->v0);
		V2d trueNormal = along;
		if (groundSpeed > 0)
			trueNormal = -trueNormal;

		trueNormal = normalize(trueNormal + dir);
		velocity = abs(groundSpeed) * trueNormal;

		//cout << "velocity: " << velocity.x << ", " << velocity.y << ", norm: " << trueNormal.x << ", " << trueNormal.y << "\n";

		holdJump = false;
		ground = NULL;
		frame = 1; //so it doesnt use the jump frame when just dropping
		reversed = false;
		framesInAir = 0;
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::UpdateCanStandUp()
{
	canStandUp = true;
	if (b.rh < normalHeight)
	{
		canStandUp = CheckStandUp();
		if (canStandUp)
		{
			b.rh = normalHeight;
			b.offset.y = 0;

			CheckGates();
		}
	}
}

void Actor::UpdateBounceFlameOn()
{
	justToggledBounce = false;

	if (bounceFlameOn)
	{
		//if (action != BOUNCEAIR && action != BOUNCEGROUND && action != BOUNCEGROUNDEDWALL )
		if( bounceEdge == NULL )
		{
			if (!IsOptionOn(POWER_BOUNCE) || !BounceButtonHeld())//BounceButtonPressed())
			{
				BounceFlameOff();
			}
		}
	}
	else
	{
		if (IsOptionOn(POWER_BOUNCE) && BounceButtonHeld() )//BounceButtonPressed())
		{
			BounceFlameOn();
		}
	}

	/*if (bounceFlameOn)
	{
		if (currPowerMode == PMODE_BOUNCE)
		{
			bool turnOffWhileToggleOn = toggleBounceInput 
				&& currInput.PowerButtonDown() 
				&& !prevInput.PowerButtonDown();
			bool turnOffWhileToggleOff = !toggleBounceInput 
				&& !currInput.PowerButtonDown();

			if (!IsOptionOn(POWER_BOUNCE) || turnOffWhileToggleOff
				|| turnOffWhileToggleOn )
			{
				if (toggleBounceInput)
				{
					justToggledBounce = true;
				}

				BounceFlameOff();

				
			}
		}
		else
		{
			BounceFlameOff();
		}
	}
	else
	{
		bool turnOnWhileToggleOn = toggleBounceInput && currInput.PowerButtonDown()
			&& !prevInput.PowerButtonDown();
		bool turnOnWhileToggleOff = !toggleBounceInput && currInput.PowerButtonDown();

		if (currPowerMode == PMODE_BOUNCE && IsOptionOn(POWER_BOUNCE)
			&& ( turnOnWhileToggleOn || turnOnWhileToggleOff ) )
		{
			if (toggleBounceInput)
			{
				justToggledBounce = true;
			}

			BounceFlameOn();
		}
	}*/
}

void Actor::UpdateWireStates()
{
	if (IsOptionOn(POWER_LWIRE) && ((action != GRINDBALL && action != GRINDATTACK) || leftWire->IsRetracting()))
	{
		leftWire->ClearDebug();
		leftWire->SetStoredPlayerPos(leftWire->GetPlayerPos());
		leftWire->UpdateState(touchEdgeWithLeftWire);
	}

	if (IsOptionOn(POWER_RWIRE) && ((action != GRINDBALL && action != GRINDATTACK) || rightWire->IsRetracting()))
	{
		rightWire->ClearDebug();
		rightWire->SetStoredPlayerPos(rightWire->GetPlayerPos());
		rightWire->UpdateState(touchEdgeWithRightWire);
	}
}

void Actor::ProcessGravModifier()
{
	if (currGravModifier != NULL)
	{
		boosterGravModifyFrames = currGravModifier->duration;
		boosterExtraGravityModifier = currGravModifier->gravFactor;

		startBoosterGravModifyFrames = boosterGravModifyFrames;

		if (boosterExtraGravityModifier > 1.0)
		{
			gravityDecreaserTrailEmitter->SetOn(false);
			gravityIncreaserTrailEmitter->SetOn(true);
			sess->AddEmitter(gravityIncreaserTrailEmitter);
			CreateGravityIncreaserOnRing();
		}
		else if (boosterExtraGravityModifier < 1.0)
		{
			gravityIncreaserTrailEmitter->SetOn(false);
			gravityDecreaserTrailEmitter->SetOn(true);
			
			sess->AddEmitter(gravityDecreaserTrailEmitter);
			CreateGravityDecreaserOnRing();
		}
		else
		{
			assert(0);
		}

		currGravModifier->Modify();

		currGravModifier = NULL;
	}
}

void Actor::ProcessBooster()
{
	if (action == JUMPSQUAT)
	{
		//ignore during jumpsquat
		currBooster = NULL;
		return;
	}

	if (currBooster != NULL && oldBooster == NULL && action != AIRDASH && currBooster->Boost())
	{
		if (ground == NULL && bounceEdge == NULL && grindEdge == NULL)
		{
			SetBoostVelocity();
			//velocity = normalize(velocity) * (length(velocity) + currBooster->strength);
		}
		else if (grindEdge != NULL)
		{
			if (grindSpeed > 0)
			{
				grindSpeed += currBooster->strength;
			}
			else if (grindSpeed < 0)
			{
				grindSpeed -= currBooster->strength;
			}

			if (IsOnRailAction(action))
			{
				velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
			}
		}
		else if (ground != NULL)
		{
			if (action == GRAVREVERSE)
			{
				//haven't tested this but it seems right
				if (storedReverseSpeed > 0)
				{
					storedReverseSpeed += currBooster->strength;
				}
				else if (storedReverseSpeed < 0)
				{
					storedReverseSpeed -= currBooster->strength;
				}
			}
			else
			{
				if (groundSpeed > 0)
				{
					groundSpeed += currBooster->strength;
				}
				else if (groundSpeed < 0)
				{
					groundSpeed -= currBooster->strength;
				}
			}
		}
	}

	if (currBounceBooster != NULL && oldBounceBooster == NULL && currBounceBooster->Boost())
	{
		SetBounceBoostVelocity();
	}

}

void Actor::ProcessTimeBooster()
{
	if (currTimeBooster != NULL && currTimeBooster->IsBoostable() )
	{
		currTimeBooster->Boost();

		cout << "boost" << endl;
		globalTimeSlowFrames = currTimeBooster->strength;
		startGlobalTimeSlowFrames = globalTimeSlowFrames;

		//timeSlowBoosterTrailEmitter->SetOn(true);
		//sess->AddEmitter(timeSlowBoosterTrailEmitter, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES);

		currTimeBooster = NULL;

		RestoreAirOptions();
	}
}

void Actor::ProcessMomentumBooster()
{
	if (currMomentumBooster != NULL && currMomentumBooster->IsBoostable())
	{
		currMomentumBooster->Boost();

		momentumBoostFrames = currMomentumBooster->strength;
		startMomentumBoostFrames = momentumBoostFrames;


		momentumBoosterTrailEmitter->SetOn(true);
		sess->AddEmitter(momentumBoosterTrailEmitter);

		currMomentumBooster = NULL;

		RestoreAirOptions();
	}
}

void Actor::ProcessRewindBooster()
{
	if (currRewindBooster != NULL && currRewindBooster->IsBoostable())
	{
		currRewindBooster->Boost();

		rewindOnHitFrames = currRewindBooster->strength;

		rewindBoosterPos = currRewindBooster->GetPosition();

		currRewindBooster = NULL;

		RestoreAirOptions();
	}
}

void Actor::ProcessAntiTimeSlowBooster()
{
	if (currAntiTimeSlowBooster != NULL && currAntiTimeSlowBooster->IsBoostable())
	{
		currAntiTimeSlowBooster->Boost();

		antiTimeSlowFrames = currAntiTimeSlowBooster->strength;
		startAntiTimeSlowFrames = antiTimeSlowFrames;

		antiTimeSlowBoosterTrailEmitter->SetOn(true);
		sess->AddEmitter(antiTimeSlowBoosterTrailEmitter);

		currAntiTimeSlowBooster = NULL;

		SetSkin(SKIN_NORMAL);

		boosterRingSprite.setTexture(*ts_antiTimeSlowRing->texture);
		boosterRingSprite.setTextureRect(ts_antiTimeSlowRing->GetSubRect(0));

		RestoreAirOptions();
	}
}

void Actor::ProcessSwordProjectileBooster()
{
	if (currSwordProjectileBooster != NULL && currSwordProjectileBooster->IsBoostable())
	{
		currSwordProjectileBooster->Boost();

		if (currSwordProjectileBooster->enemyProjectile)
		{
			enemyProjectileSwordFrames = currSwordProjectileBooster->strength;
		}
		else
		{
			projectileSwordFrames = currSwordProjectileBooster->strength;
		}

		currSwordProjectileBooster = NULL;

		RestoreAirOptions();
	}
}

void Actor::ProcessHomingBooster()
{
	if (currHomingBooster != NULL && currHomingBooster->IsBoostable())
	{
		currHomingBooster->Boost();

		
		homingFrames = currHomingBooster->strength;
		startHomingFrames = homingFrames;

		homingBoosterTrailEmitter->SetOn(true);
		sess->AddEmitter(homingBoosterTrailEmitter );
		RestoreAirOptions();
		//ground = NULL;
		//wallNormal = V2d(0, 0);
		//currWall = NULL;
		//bounceEdge = NULL;
		//grindEdge = NULL;

		currHomingBooster = NULL;
	}
}

void Actor::ProcessFreeFlightBooster()
{
	if (currFreeFlightBooster != NULL && currFreeFlightBooster->IsBoostable() )
	{
		currFreeFlightBooster->Boost();

		SetAction(FREEFLIGHT);
		freeFlightFrames = currFreeFlightBooster->strength;
		startFreeFlightFrames = freeFlightFrames;

		freeFlightBoosterTrailEmitter->SetOn(true);
		sess->AddEmitter(freeFlightBoosterTrailEmitter );
		extraGravityModifier = 0;
		gravModifyFrames = freeFlightFrames;
		RestoreAirOptions();
		ground = NULL;
		wallNormal = V2d(0, 0);
		currWall = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		reversed = false;

		currFreeFlightBooster = NULL;
	}
	
}

void Actor::ProcessAccelGrass()
{
	if (touchedGrass[Grass::ACCELERATE])
	{
		if (ground != NULL)
		{
			if (groundSpeed > 0)
			{
				groundSpeed += accelGrassAccel;
			}
			else if (groundSpeed < 0)
			{
				groundSpeed -= accelGrassAccel;
			}
		}
		else if (grindEdge != NULL)
		{
			if (grindSpeed > 0)
			{
				grindSpeed += accelGrassAccel;
			}
			else if (groundSpeed < 0)
			{
				grindSpeed -= accelGrassAccel;
			}
		}
		else
		{
			double len = length(velocity);
			velocity = normalize(velocity) * (len + accelGrassAccel);
		}
	}
}

void Actor::ProcessDecelGrass()
{
	if (!touchedGrass[Grass::DECELERATE])
		return;

	if (ground != NULL )
	{
		double dSpeed = GetDashSpeed();
		double maxGSpeed = 5;
		double decel = 2.0;
		double maxSlowSpeed = dSpeed;
		if (groundSpeed > maxSlowSpeed)
		{
			groundSpeed -= decel;
		}
		else if (groundSpeed < -maxSlowSpeed)
		{
			groundSpeed += decel;
		}
		else if (groundSpeed > maxGSpeed)
		{
			groundSpeed = maxGSpeed;
		}
		else if (groundSpeed < -maxGSpeed)
		{
			groundSpeed = -maxGSpeed;
		}
	}
	else if( action == WALLCLING )
	{
		double factor = .1;
		if (velocity.y > clingSpeed * factor)
		{
			//cout << "running wallcling" << endl;
			velocity.y = clingSpeed * factor;
		}
	}
}

void Actor::ProcessBoostGrass()
{
	if (!touchedGrass[Grass::BOOST])
		return;

	if (ground != NULL)
	{
		double boostSpeed = 40;
		if (groundSpeed > 0)
		{
			groundSpeed = max(groundSpeed, boostSpeed);
		}
		else if (groundSpeed < 0)
		{
			groundSpeed = min(groundSpeed, -boostSpeed);
		}

		if (action == JUMPSQUAT)
		{
			if (storedGroundSpeed > 0)
			{
				storedGroundSpeed = max(groundSpeed, boostSpeed);
			}
			else if (storedGroundSpeed < 0)
			{
				storedGroundSpeed = min(groundSpeed, -boostSpeed);
			}
		}
	}
	//else if (action == WALLCLING)
	//{
	//	double factor = .1;
	//	if (velocity.y > clingSpeed * factor)
	//	{
	//		//cout << "running wallcling" << endl;
	//		velocity.y = clingSpeed * factor;
	//	}
	//}
}

void Actor::ProcessPoisonGrass()
{
	//if (ground != NULL && touchedGrass[Grass::POISON])
	if (touchedGrass[Grass::POISON])
	{
		RestoreAirOptions();
		DrainTimer(10);

		double poisonAccel = .2;

		if (ground != NULL)
		{
			if (groundSpeed > 0)
			{
				groundSpeed += poisonAccel;
			}
			else if (groundSpeed < 0)
			{
				groundSpeed -= poisonAccel;
			}
		}
		else if (grindEdge != NULL)
		{
			if (grindSpeed > 0)
			{
				grindSpeed += poisonAccel;
			}
			else if (groundSpeed < 0)
			{
				grindSpeed -= poisonAccel;
			}
		}
		else
		{
			double len = length(velocity);
			velocity = normalize(velocity) * (len + poisonAccel);
		}

	}
}

void Actor::ApplyGeneralAcceleration(double accelFactor)
{
	if (ground != NULL)
	{
		if (groundSpeed > 0)
		{
			groundSpeed += accelFactor;
		}
		else if (groundSpeed < 0)
		{
			groundSpeed -= accelFactor;
		}
	}
	else
	{
		double velLen = length(velocity);

		if (velLen > 0)
		{
			velocity = (velLen + accelFactor) * normalize(velocity);
		}

		if (action == AIRDASH)
		{
			if (velocity.x > 0)
			{
				startAirDashVel.x += accelFactor;
			}
			else if (velocity.x < 0)
			{
				startAirDashVel.x -= accelFactor;
			}
		}
	}
}

void Actor::ProcessHitGrass()
{
	if (touchedGrass[Grass::HIT])
	{
		if (invincibleFrames == 0)
		{
			receivedHit = hitGrassHitInfo;
			ReactToBeingHit();
			invincibleFrames += 30;
			receivedHit.hType = HitboxInfo::NO_HITBOX;
		}
	}
}

void Actor::ProcessBounceGrassGrounded()
{
	if ( ground != NULL && touchedGrass[Grass::BOUNCE] && !bounceFlameOn)//ground != NULL && touchedGrass[Grass::BOUNCE] && !bounceFlameOn)
	{
		SetAction(JUMP);
		frame = 1;

		if (action != FAIR && action != DAIR && action != UAIR)
		{
			
		}

		velocity = GetTrueVel();

		

		RestoreAirOptions();
		

		V2d gn = ground->Normal();
		if (gn.y < 0)
		{
			if (velocity.y > 0)
			{
				velocity.y = 0;
			}

			velocity.y -= 25;
			//velocity.y = -25;
		}
		else if (gn.y > 0)
		{
			if (velocity.y < 0)
			{
				velocity.y = 0;
			}


			velocity.y += 25;
			//velocity.y = 25;
		}

		reversed = false;

		ground = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		/*else if (gn.x > 0)
		{
			velocity.x = 18;
		}
		else if (gn.x < 0)
		{
			velocity.x = -18;
		}*/
	}
}

void Actor::LimitMaxSpeeds()
{
	double maxReal = GetMaxSpeed();
	if (ground == NULL && bounceEdge == NULL && grindEdge == NULL
		&& action != ENTERNEXUS1
		&& action != GLIDE
		&& !IsSpringAction( action )
		&& freeFlightFrames == 0
		&& action != HOMINGATTACK
		&& action != WARP_CHARGE
		&& action != WARP )
		//&& action != INVERSEINPUTSTUN )
		//&& action != SPRINGSTUN:
		//&& action != SPRINGSTUNGLIDE
		//&& action != SPRINGSTUNBOUNCE
		//&& action != SPRINGSTUNAIRBOUNCE
		//&& action != SPRINGSTUNTELEPORT)
	{
		if (action != AIRDASH && !(rightWire->IsPulling() && leftWire->IsPulling()) && action != GRINDLUNGE && action != RAILDASH && action != GETSHARD && action != WAITFORSHIP)
		{
			if (!frameAfterAttackingHitlagOver) //hitting enemies was making full hop height lower
			{
				velocity = AddAerialGravity(velocity);
			}
		}

		if (InWater( TerrainPolygon::WATER_NORMAL))//make this into a cleaner function very soon.
		{
			//cout << "running wallcling" << endl;
			if (velocity.y > normalWaterMaxFallSpeed)
			{
				velocity.y = normalWaterMaxFallSpeed;
			}
		}

		if (velocity.x > maxReal)
			velocity.x = maxReal;
		else if (velocity.x < -maxReal)
			velocity.x = -maxReal;

		if (velocity.y > maxReal)
			velocity.y = maxReal;
		else if (velocity.y < -maxReal)
			velocity.y = -maxReal;
	}
	else
	{
		if (groundSpeed > maxReal)
		{
			groundSpeed = maxReal;
		}
		else if (groundSpeed < -maxReal)
		{
			groundSpeed = -maxReal;
		}
	}
}

void Actor::ClearRecentHitters()
{
	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		recentHitters[i].Clear();
	}
}
void Actor::UpdateRecentHitters()
{
	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		recentHitters[i].Update();
	}
}


bool Actor::EnemyRecentlyHitMe(Enemy *e)
{
	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		if ( recentHitters[i].CheckEnemy( e ) )
		{
			return true;
		}
	}
	return false;
}

bool Actor::BulletRecentlyHitMe(BasicBullet *b)
{
	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		if (recentHitters[i].CheckBullet( b ) )
		{
			return true;
		}
	}
	return false;
}

void Actor::AddRecentEnemyHitter(Enemy *e)
{
	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		if (recentHitters[i].id < 0 )
		{
			recentHitters[i].SetEnemy(e->enemyIndex);
			return;
		}
	}
}

void Actor::AddRecentBulletHitter(BasicBullet *b)
{
	for (int i = 0; i < MAX_HITTERS; ++i)
	{
		if (recentHitters[i].id < 0)
		{
			recentHitters[i].SetBullet( b->bulletID, b->launcher->launcherID );
			return;
		}
	}
}

void Actor::ActivateLauncherEffect(int tile)
{
	float framesIn = springStunFramesStart - springStunFrames;
	float doneFactor = framesIn / springStunFramesStart;
	float scaleFactor = 1.f - doneFactor;//.5 + (1.f - doneFactor ) * .5;

	EffectInstance ef;
	Transform ti = Transform::Identity;
	scaleFactor *= .9;
	ti.scale(scaleFactor, scaleFactor);

	V2d velDir = normalize(velocity);
	double ang = GetVectorAngleCW(velDir) / PI * 180.0 + 90;
	//cout << "angle: " << angD << endl;

	ti.rotate(ang);
	ef.SetParams(Vector2f(position), ti, 1, 32, tile);

	//ef.SetVelocityParams( Vector2f( 0, )
	ActivateEffect(PLAYERFX_LAUNCH_PARTICLE_0, &ef);

	ef.SetParams(Vector2f(position), ti, 1, 32, tile + 1);

	float colorFactor;
	float firstThresh = .3f;
	float secondThresh = .8f;
	if (doneFactor < firstThresh)
	{
		colorFactor = 0;
	}
	else if (doneFactor >= firstThresh && doneFactor < secondThresh)
	{
		colorFactor = (doneFactor - firstThresh) / (secondThresh - firstThresh);
	}
	else
	{
		colorFactor = 1;
	}

	ef.color = Color(255, 255, 255, colorFactor * 255);
	ActivateEffect(PLAYERFX_LAUNCH_PARTICLE_1, &ef);
}

bool Actor::CheckExtendedAirdash()
{
	return (( inBubble && IsOptionOn( UPGRADE_W6_BUBBLE_AIRDASH ) )//UPGRADE_W5_INFINITE_AIRDASH_WITHIN_BUBBLES ) 
		|| InWater(TerrainPolygon::WATER_ZEROGRAV));
		//|| InWater( TerrainPolygon::WATER_MOMENTUM ));
}

int Actor::GetNumActiveBubbles()
{
	int numActiveBubbles = 0;
	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		if (bubbleFramesToLive[i] > 0)
		{
			++numActiveBubbles;
		}
	}

	return numActiveBubbles;
}

void Actor::UpdateBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		if (bubbleFramesToLive[i] > 0)
		{
			bubbleFramesToLive[i]--;
			SetFBubbleFrame(i, bubbleFramesToLive[i]);
			bubblePos[i] = position;
		}
	}


	bool bubbleCreated = false;
	oldInBubble = inBubble;
	inBubble = false;

	if (IsOptionOn(POWER_TIME))
	{
		//calculate this all the time so I can give myself infinite airdash
		for (int i = 0; i < MAX_BUBBLES; ++i)
		{
			if (bubbleFramesToLive[i] > 0)
			{
				//if( IsQuadTouchingCircle( hurtB
				if (length(position - bubblePos[i]) < bubbleRadiusSize[i])
				{
					inBubble = true;
					break;
				}
			}
		}
	}

	bool isInOwnBubble = inBubble;
	bool isBeingSlowed = IsBeingSlowed();
	if (isBeingSlowed)
	{
		inBubble = true;
	}

	bool timeSlowTerrain = false;
	if (InWater( TerrainPolygon::WATER_TIMESLOW) )
	{
		timeSlowTerrain = true;
		inBubble = true;
	}

	if (toggleTimeSlowInput && !inBubble && oldInBubble)
	{
		//get rid of toggle inputs probably
		//currinput.Y
		currInput.leftShoulder = false;
	}


	//end extended airdash
	/*if (action == AIRDASH && airDashStall && !CheckExtendedAirdash() )
	{
		SetAction(JUMP);
		frame = 1;
		holdJump = false;
	}*/

	

	if (grindEdge != NULL)
	{
		if (grindEdge->rail != NULL
			&& grindEdge->rail->GetRailType() == TerrainRail::ANTITIMESLOW)
		{
			timeSlowTerrain = false;
		}
	}

	if (timeSlowTerrain )
	{
		specialSlow = true;
	}

	bool powerSlow = CanCreateTimeBubble()
		&& IsOptionOn(POWER_TIME)
		&& PowerButtonHeld()
		&& currPowerMode == PMODE_TIMESLOW;


	if (specialSlow)
	{
		inBubble = true;
	}

	if (IsGoalKillAction(action))
	{
		inBubble = false;
		specialSlow = false;
		powerSlow = false;
	}

	//currInput.leftShoulder before
	int tempSlowCounter = slowCounter;
	if (antiTimeSlowFrames == 0 && ( powerSlow || specialSlow)  )
	{
		if (!prevInput.PowerButtonDown() && !inBubble && !specialSlow )
			//&& GetNumActiveBubbles() < GetMaxBubbles() )
		{
			if (GetNumActiveBubbles() == GetMaxBubbles())
			{
				//if (bubbleFramesToLive[currBubble] > 0)
				{
					int lowestToLiveIndex = -1;
					int lowestAmountToLive = -1;
					for (int i = 0; i < GetMaxBubbles(); ++i)
					{
						if (bubbleFramesToLive[i] > 0)
						{
							if (lowestToLiveIndex < 0 || bubbleFramesToLive[i] < lowestAmountToLive)
							{
								lowestToLiveIndex = i;
								lowestAmountToLive = bubbleFramesToLive[i];
							}
						}
					}
					assert(lowestToLiveIndex >= 0);
					currBubble = lowestToLiveIndex;
					bubbleFramesToLive[currBubble] = 0;
				}
			}

			/*if (currBubble > 0)
			{
				int xxx = 56;
			}*/

			if (bubbleFramesToLive[currBubble] == 0)
			{
				inBubble = true;

				bubbleFramesToLive[currBubble] = bubbleLifeSpan;

				bubbleRadiusSize[currBubble] = GetBubbleRadius();

				SetFBubbleFrame(currBubble, bubbleLifeSpan);
				SetFBubbleRadiusSize(currBubble, bubbleRadiusSize[currBubble]);

				bubblePos[currBubble] = position;
				SetFBubblePos(currBubble, Vector2f(position));

				++currBubble;
				if (currBubble == GetMaxBubbles())
				{
					currBubble = 0;
				}

				bubbleCreated = true;
				ActivateSound(PlayerSounds::S_TIMESLOW);
			}
		}

		if (inBubble)
		{
			if (slowMultiple == baseSlowMultiple)
			{
				//cout << "a" << endl;
				slowCounter = 1;
				slowMultiple = GetBubbleTimeFactor();//5
			}
		}
		else
		{
			//cout << "b" << endl;
			slowCounter = 1;
			slowMultiple = baseSlowMultiple;
		}
	}
	else
	{
		//cout << "C " << endl;
		//slowCounter = 1;
		slowMultiple = baseSlowMultiple;

		//changed when doing editor stuff. before it would always set to one
		if (slowCounter > baseSlowMultiple)
			slowCounter = 1;
	}

	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		if (bubbleFramesToLive[i] > 0)
		{
			CollisionBox &bHitbox = bubbleHitboxes[i].GetCollisionBoxes(0).front();
			bHitbox.globalPosition = bubblePos[i];
			bHitbox.rw = bubbleRadiusSize[i];
			bHitbox.rh = bHitbox.rw;
		}
	}

	if (isBeingSlowed && !isInOwnBubble)
	{
		if (currInput.PowerButtonDown())
		{
			slowCounter = 1;
			slowMultiple = baseSlowMultiple;
		}
		else
		{
			slowCounter = tempSlowCounter;
			slowMultiple = GetBeingSlowedFactor();
		}
	}
}

void Actor::UpdateRegrindOffCounter()
{
	if (action != RAILGRIND)
	{
		if (regrindOffCount < regrindOffMax)
		{
			regrindOffCount++;
		}
	}
}

void Actor::UpdateKnockbackDirectionAndHitboxType()
{
	currHitboxInfo->hType = HitboxInfo::NORMAL;

	V2d trueNorm;
	V2d along;
	if (ground != NULL)
	{
		GroundedAngleAttack(trueNorm);
		along = V2d(-trueNorm.y, trueNorm.x);
	}

	currHitboxInfo->hitPosType = HitboxInfo::AIRFORWARD; //default. just need to distinguish between ground/air
	switch (action)
	{
	case STANDATTACK1:
	case STANDATTACK2:
	case STANDATTACK3:
	case STANDATTACK4:
	case DASHATTACK:
	case DASHATTACK2:
	case DASHATTACK3:
	case UPTILT1:
	case UPTILT2:
	case UPTILT3:
	case STEEPCLIMBATTACK:
	case STEEPSLIDEATTACK:
		currHitboxInfo->hitPosType = HitboxInfo::GROUND;
		break;
	}

	switch (action)
	{
	case AIRDASHFORWARDATTACK:
	case FAIR:
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(1, 0);//HitboxInfo::HitDirection::RIGHT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(-1, 0);
		}
		break;
	case DAIR:
		currHitboxInfo->hDir = V2d(0, 1);
		break;
	case UAIR:
		currHitboxInfo->hDir = V2d(0, -1);
		break;
	case STANDATTACK1:
	case STANDATTACK2:
	case STANDATTACK3:
	case STANDATTACK4:
	case DASHATTACK:
	case DASHATTACK2:
	case DASHATTACK3:
	case UPTILT1:
	case UPTILT2:
	case UPTILT3:
		if ((!reversed && facingRight) || (reversed && !facingRight))
		{
			currHitboxInfo->hDir = along;
		}
		else
		{
			currHitboxInfo->hDir = -along;
		}
		break;
	case STEEPCLIMBATTACK:
		if (reversed)
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = -along;//V2d(-1, 1);//HitboxInfo::HitDirection::DOWNLEFT;
			}
			else
			{
				currHitboxInfo->hDir = along;//V2d(1, 1);//HitboxInfo::HitDirection::DOWNRIGHT;
			}
		}
		else
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = along;//V2d(1, -1);//HitboxInfo::HitDirection::UPRIGHT;
			}
			else
			{
				currHitboxInfo->hDir = -along;//V2d(-1, -1);//HitboxInfo::HitDirection::UPLEFT;
			}
		}
		break;
	case STEEPSLIDEATTACK:
		if (reversed)
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = -along;//V2d(-1, -1);//HitboxInfo::HitDirection::UPLEFT;
			}
			else
			{
				currHitboxInfo->hDir = along;//V2d(1, -1);//HitboxInfo::HitDirection::UPRIGHT;
			}
		}
		else
		{
			if (facingRight)
			{
				currHitboxInfo->hDir = along;//V2d(1, 1);//HitboxInfo::HitDirection::DOWNRIGHT;
			}
			else
			{
				currHitboxInfo->hDir = -along;//V2d(-1, 1);//HitboxInfo::HitDirection::DOWNLEFT;
			}
		}
		break;
	case WALLATTACK:
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(-1, 0);//HitboxInfo::HitDirection::LEFT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(1, 0);//HitboxInfo::HitDirection::RIGHT;
		}
		break;
	case DIAGUPATTACK:
		currHitboxInfo->hType = HitboxInfo::BLUE;
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(1, -1);//HitboxInfo::HitDirection::UPRIGHT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(-1, -1);//HitboxInfo::HitDirection::UPLEFT;
		}
		break;
	case DIAGDOWNATTACK:
		currHitboxInfo->hType = HitboxInfo::BLUE;
		if (facingRight)
		{
			currHitboxInfo->hDir = V2d(1, 1);//HitboxInfo::HitDirection::DOWNRIGHT;
		}
		else
		{
			currHitboxInfo->hDir = V2d(-1, 1);//HitboxInfo::HitDirection::DOWNLEFT;
		}
		break;
	/*case HOMINGATTACK:
	case SPRINGSTUNHOMINGATTACK:
	{
		if (velocity.x == 0 && velocity.y == 0)
		{
			currHitboxInfo->hDir = V2d(0, 0);
		}
		else
		{
			currHitboxInfo->hDir = normalize(velocity);
		}
		
		break;
	}*/
	default:
		currHitboxInfo->hDir = V2d(0, 0);//HitboxInfo::HitDirection::NONE;
		break;
	}
}

bool Actor::CheckTerrainDisappear(Edge *e)
{
	bool disappear = false;
	if (e != NULL )
	{
		//Gate *g;
		/*if (e->edgeType == Edge::OPEN_GATE)
		{
			disappear = true;
		}*/

		if (e->poly != NULL)
		{
			if ( !e->poly->IsActive() )
			{
				disappear = true;
			}
		}
		else if (e->rail != NULL)
		{
			if ( !e->rail->IsActive() )
			{
				disappear = true;
			}
		}
	}

	return disappear;
}

void Actor::UpdatePrePhysics()
{
	//cout << "update pre" << endl;
	if (action == DEATH && simulationMode)
		return;

	if (action == HIDDEN)
		return;

	//cout << "velocity: " << velocity.x << ", " << velocity.y << "\n";
	//cout << "groundspeed: " << groundSpeed << "\n";
	//cout << "parallel index: " << sess->parallelSessionIndex << ", my index: " << actorIndex << ", my action: " << action << "\n";
	/*if (homingFrames > 0)
	{
		if (!boosterTrailEmitter->active)
		{
			sess->AddEmitter(boosterTrailEmitter, DrawLayer::BETWEEN_PLAYER_AND_ENEMIES);
		}
	}*/

	//if (currInput.B && !prevInput.B && sess->totalGameFrames > 10 )//(currInput.respawnTest)
	if( currInput.respawnTest )
	{
		owner->RestartGame();
	}

	//cout << "velocity: " << velocity.x << ", " << velocity.y << "\n";

	//if ( owner != NULL && !simulationMode && sess->gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !sess->IsParallelSession())
	//{
	//	if (currInput.PUp() && !prevInput.PUp())
	//	{
	//		cout << "trying to signal practice players to race" << "\n";

	//		sess->netplayManager->TrySignalPracticePlayersToRace();
	//		//sess->netplayManager->TestNewRaceSystem();
	//		owner->quit = true;
	//		owner->returnVal = GameSession::GR_EXIT_PRACTICE_TO_RACE;
	//	}
	//}

	//cout << "game frame: " << sess->totalGameFrames << ", action: " << action << ", " << "frame: " << frame << ", posiiton: " << position.x << ", " << position.y << "\n";
	/*static int skinTest = 0;
	SetSkin(skinTest / 3);
	++skinTest;
	if (skinTest == 4 * 3)
	{
		skinTest = 0;
	}*/


	//cout << "vely: " << velocity.y << endl;
	//cout << "groundspeed: " << groundSpeed << endl;

	
	if (currInspectObject != NULL && currInspectObject->IsUsed())
	{
		currInspectObject = NULL;
	}

	hitOutOfHitstunLastFrame = false;
	if (actorIndex == 1)
	{
		//currInput.leftStickPad |= 1;
		//currInput.Y = true;
	}

	if (directionalInputFreezeFrames > 0)
	{
		currInput.leftStickPad = prevInput.leftStickPad;
		currInput.leftStickDirection = prevInput.leftStickDirection;
		currInput.leftStickMagnitude = prevInput.leftStickMagnitude;
	}
	else if (invertInputFrames > 0)
	{
		currInput.InvertLeftStick();
	}


	if (CheckTerrainDisappear(ground))
	{
		V2d along = ground->Along();
		if (reversed)
		{
			along = -along;
		}
		velocity = groundSpeed * along;
		SetAirPos(position, facingRight);
	}
	else if (CheckTerrainDisappear(bounceEdge))
	{
		double through = dot(storedBounceVel, -bounceNorm);
		velocity = storedBounceVel + bounceNorm * through;

		SetAirPos(position, facingRight);
	}
	else if (CheckTerrainDisappear(grindEdge))
	{
		TryStandupOnForcedGrindExit();
	}

		
	
	//TryCheckGrass();

	/*for (int i = 0; i < NUM_PAST_INPUTS-1; ++i)
	{
		pastCompressedInputs[i+1] = pastCompressedInputs[i];
	}
	pastCompressedInputs[0] = currInput.GetCompressedState();*/

	if (PowerButtonPressed() && currPowerMode == PMODE_SHIELD)
	{
		framesSinceBlockPress = 0;
	}

	if (currInput.Y && !prevInput.Y)
	{
		if (superLevelCounter < 2 )
		{
			superLevelCounter++;
			framesSinceSuperPress = 0;
		}
	}

	if ( superLevelCounter > 0 && framesSinceSuperPress >= superActiveLimit)
	{
		superLevelCounter = 0;
	}

	if (touchedCoyoteHelper)
	{
		ground = false;
		SetAction(JUMP);
		frame = 1;
		velocity.y = -50;
	}

	if (hitlagFrames > 0)
	{
		UpdateInHitlag();
		return;
	}

	showSword = false;

	SetCurrHitboxes(NULL, 0);

	ProcessGravityGrass();
	CheckForAirTrigger();
	HandleAirTrigger();

	UpdateDrain();
	
	enemiesKilledLastFrame = enemiesKilledThisFrame;
	enemiesKilledThisFrame = 0;

	//DesperationUpdate();

	/*if (reversed)
	{
		ReverseVerticalInputs();
	}*/
	

	ActionEnded();

	

	if (action == DEATH)
	{
		UpdateAction();
		return;
	}

	TryChangePowerMode();


	bool isParallelPracticeAndParallel = sess->gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && sess->IsParallelSession();

	if( IsIntroAction( action ) || (IsGoalKillAction(action) && action != GOALKILLWAIT) || action == EXIT 
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || IsSequenceAction( action ) || action == EXITBOOST || action == EXIT_RUSH )
	{
		if( action == WAITFORSHIP )
		{ 
			if (!isParallelPracticeAndParallel)
			{
				HandleWaitingScoreDisplay();

				if (sess->IsSessTypeEdit() && sess->nextFrameRestartGame)
				{
					editOwner->TestPlayerMode();
				}
				else if ((sess->scoreDisplay == NULL || !sess->scoreDisplay->IsActive()) && sess->activeSequences[0] == NULL)
				{
					sess->SetActiveSequence(sess->shipExitScene);
					sess->shipExitScene->Reset();
					sess->shipExitScene->Update();
					//owner->SetActiveSequence(owner->shipExitSeq);
					//owner->shipExitSeq->Reset();
				}
			}
		}
		if( action == INTRO && frame == 0 )
		{
			int enterIndex = PlayerSounds::S_ENTER_W1 + sess->mapHeader->envWorldType;
			ActivateSound(enterIndex);
		}
		else if( action == EXIT && frame == 30 )
		{
			ActivateSound(PlayerSounds::S_EXIT );
		}
		else if (action == EXIT_RUSH && frame == 30)
		{
			ActivateSound(PlayerSounds::S_EXIT);
		}
		else if (action == EXITWAIT)
		{
			if (frame == 0)
			{
				ActivateEffect(DrawLayer::IN_FRONT, sess->GetTileset("Kin/FX/exitenergy_0_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}
			else if (frame == 6 * 2)
			{
				ActivateEffect(DrawLayer::IN_FRONT, sess->GetTileset("Kin/FX/exitenergy_1_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}
			else if (frame == 6 * 4)
			{
				ActivateEffect(DrawLayer::IN_FRONT, sess->GetTileset("Kin/FX/exitenergy_2_512x512.png", 512, 512), spriteCenter, false, 0, 6, 2, true);
			}	
		}
		else if (action == SEQ_KINFALL)
		{
			velocity = AddAerialGravity(velocity);
		}
		return;
	}
	else if( action == GOALKILLWAIT )
	{
		if (!isParallelPracticeAndParallel)
		{
			HandleWaitingScoreDisplay();

			if (sess->IsSessTypeEdit() && sess->nextFrameRestartGame)
			{
				editOwner->TestPlayerMode();
			}
			else if (!sess->scoreDisplay->IsActive())
			{
				if (sess->IsRushSession())
				{
					//if (owner->mainMenu->rushManager->TryToGoToNextLevel(owner))
					if( owner->mainMenu->rushManager->CanGoToNextLevel())
					{
						SetAction(EXIT);
						sess->Fade(false, 30, Color::Black, true);				
						frame = 0;


						/*V2d pos = position;
						SetAirPos(pos, true);
						velocity = V2d(0, 0);
						hitGoal = false;*/
					}
					else
					{
						if (owner != NULL && owner->resType == GameSession::GameResultType::GR_WINCONTINUE)
						{
							SetAction(EXITBOOST);
							owner->Fade(false, 30, Color::Black, true);
						}
						else
						{
							SetAction(EXIT);
							sess->Fade(false, 30, Color::Black, true);
						}
						frame = 0;
					}
					/*if (owner != NULL && owner->IsRushSession() && )
					{
					V2d pos = position;
					SetAirPos(pos, true);
					velocity = V2d(0, 0);
					hitGoal = false;
					}
					else
					{
					SetAction(GOALKILL);
					}*/
				}
				else
				{
					if (owner != NULL && owner->resType == GameSession::GameResultType::GR_WINCONTINUE)
					{
						SetAction(EXITBOOST);
						owner->Fade(false, 30, Color::Black, true);
					}
					else
					{
						SetAction(EXIT);
						sess->Fade(false, 30, Color::Black, true);
					}
					frame = 0;
				}

				
			}
		}
		return;
	}
	
	ProcessReceivedHit();
	
	UpdateCanStandUp();

	

	UpdateBounceFlameOn();

	TryScorpRailDropThrough();
	
	ChangeAction();

	UpdateAction();

	UpdateWireStates();
	

	ProcessGravModifier();

	if (!simulationMode)
	{
		if (currTutorialObject != NULL)
		{
			if (currTutorialObject->TryDeactivate())
			{
				currTutorialObject = NULL;
			}
		}

		if (currInspectObject != NULL)
		{
			if (currInspectObject->TryDeactivate())
			{
				currInspectObject = NULL;
			}
		}

		ProcessBooster();

		ProcessTimeBooster();

		ProcessHomingBooster();

		ProcessFreeFlightBooster();

		ProcessAntiTimeSlowBooster();

		ProcessMomentumBooster();

		ProcessRewindBooster();

		ProcessSwordProjectileBooster();
	}
	

	ProcessAccelGrass();

	ProcessDecelGrass();

	ProcessBoostGrass();

	ProcessPoisonGrass();

	ProcessHitGrass();

	ProcessBounceGrassGrounded();

	LimitMaxSpeeds();

	WireMovement();

	UpdateBubbles();

	UpdateRegrindOffCounter();

	UpdateKnockbackDirectionAndHitboxType();

	ClearPauseBufferedActions();

	
	oldVelocity.x = velocity.x;
	oldVelocity.y = velocity.y;
	touchEdgeWithLeftWire = false;
	touchEdgeWithRightWire = false;
	oldAction = action;
	collision = false;
	groundedWallBounce = false;
	oldBooster = currBooster;
	oldBounceBooster = currBounceBooster;
	highAccuracyHitboxes = true;
	wallNormal.x = 0;
	wallNormal.y = 0;
	hitEnemyDuringPhysics = false;
	
	specialSlow = false;
	bouncedFromKill = false;
	

	touchedCoyoteHelper = false;

	//if updateprephysics gets canceled early for whatever reason this wont get called. doubt its a problem but maybe.
	hitCeilingSoundPlayedThisFrame = false;
	

	UpdateHitboxes();
}

double Actor::GetNumSteps()
{
	if (highAccuracyHitboxes)
	{
		return NUM_MAX_STEPS;
	}
	else
	{ 
		return NUM_STEPS;
	}
}



void Actor::StartSeqKinThrown( V2d &pos, V2d &vel )
{
	SetAction(SEQ_KINTHROWN);
	frame = 0;
	ground = NULL;
	position = pos;
	velocity = vel;

}

void Actor::SeqKneel()
{
	SetAction(SEQ_KNEEL);
	frame = 0;
}

void Actor::SeqMeditateMaskOn()
{
	SetAction(SEQ_KNEEL_TO_MEDITATE);
	frame = 0;
}

void Actor::SeqMaskOffMeditate()
{
	SetAction(SEQ_MASKOFF);
	frame = 0;
}

void Actor::SeqGetAirdash()
{
	SetAction(GETPOWER_AIRDASH_FLIP);
	frame = 0;
}

void Actor::TransitionAction(int a)
{
	if (action == -1)
		return;
	if (transitionFuncs[action] != NULL)
	{
		(this->*transitionFuncs[action])(a);
	}
}

void Actor::SetAction( int a )
{
	TransitionAction(a);

	action = a;
	frame = 0;
	currHitboxes = NULL;

	StopRepeatingSound();

	StartAction();
		
	if( slowCounter > 1 )
	{
		slowCounter = 1;
	}
}

bool Actor::TryClimbBoost()
{
	if ( pauseBufferedDash || DashButtonPressed()) //the pauseBufferedDash is here for when you cancel an attack with a climb.
	{
		V2d norm = GetGroundedNormal();
		double sp = steepClimbBoostStart + GetDashSpeed();//steepClimbBoostStart;// 8;//13;//10;//5;//20;//5;//jumpStrength + 1;//28.0;
		double fac = min(((double)framesSinceClimbBoost) / climbBoostLimit, 1.0);

		double extra = 10.0;
		double upgradeAmount = 2.0;

		if (reversed)
		{
			int numCeilingClimbUpgrades = IsOptionOn(UPGRADE_W3_CEILING_STEEP_CLIMB_1) + IsOptionOn(UPGRADE_W4_CEILING_STEEP_CLIMB_2) + IsOptionOn(UPGRADE_W5_CEILING_STEEP_CLIMB_3);

			extra += numCeilingClimbUpgrades * upgradeAmount;
		}
		else
		{
			int numClimbUpgrades = IsOptionOn(UPGRADE_W1_STEEP_CLIMB_1) + IsOptionOn(UPGRADE_W2_STEEP_CLIMB_2) + IsOptionOn(UPGRADE_W6_STEEP_CLIMB_3);
			extra += numClimbUpgrades * upgradeAmount;
		}


		if (HoldingRelativeUp())
		{
			double extraUpBoost = 3.0;
			if ((norm.x > 0 && groundSpeed <= 0)
				|| (norm.x < 0 && groundSpeed >= 0))
			{
				extra += extraUpBoost;
			}
			//cout << "boost but better" << endl;
			//extra = 10;//5.5;
		}

		extra = extra * fac;

		//cout << "frames: " << framesSinceClimbBoos
		//cout << "fac: " << fac << " extra: " << extra << endl;
		if (norm.x > 0)//&& currInput.LLeft() )
		{
			groundSpeed = std::min(groundSpeed - extra, -sp);
		}
		else if (norm.x < 0)// && currInput.LRight() )
		{
			//cout << "old groundspeed: " << groundSpeed << " ";
			groundSpeed = std::max(groundSpeed + extra, sp);
			//cout << "groundSpeed: " << groundSpeed << ", extra: " << extra << endl;
		}

		framesSinceClimbBoost = 0;
		return true;
	}

	return false;
}

void Actor::WireMovement()
{
	Wire *wire = rightWire;

	double accel = .15;//.2;//.15;//.15;
	double triggerSpeed = 17;
	double doubleWirePull = 1.0;//2.0

	doubleWireBoost = false; //just for now temp
	rightWireBoost = false;
	leftWireBoost = false;

	if (framesInAir > 1)
	{
		if (rightWire->IsPulling() && leftWire->IsPulling())
		{
			bool canDoubleWireBoostParticle = false;
			if (framesSinceDoubleWireBoost >= doubleWireBoostTiming)
			{
				canDoubleWireBoostParticle = true;
				framesSinceDoubleWireBoost = 0;
			}

			V2d rwPos = rightWire->data.storedPlayerPos;
			V2d lwPos = rightWire->data.storedPlayerPos;
			V2d newVel1, newVel2;
			V2d wirePoint = wire->data.anchor.pos;
			if (wire->data.numPoints > 0)
				wirePoint = wire->data.points[wire->data.numPoints - 1].pos;

			V2d wireDir1 = normalize(wirePoint - rwPos);
			V2d tes = normalize(rwPos - wirePoint);
			double temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			V2d old = velocity;
			//velocity = dot( velocity, tes ) * tes;


			V2d future = rwPos + velocity;

			V2d seg = wirePoint - rwPos;
			double segLength = length(seg);
			V2d diff = wirePoint - future;

			//wire->segmentLength -= 10;
			if (length(diff) > wire->data.segmentLength)
			{
				future += normalize(diff) * (length(diff) - (wire->data.segmentLength));
				newVel1 = future - rwPos;
			}


			wire = leftWire;

			wirePoint = wire->data.anchor.pos;
			if (wire->data.numPoints > 0)
				wirePoint = wire->data.points[wire->data.numPoints - 1].pos;

			V2d wireDir2 = normalize(wirePoint - lwPos);
			tes = normalize(lwPos - wirePoint);
			temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			old = velocity;
			//velocity = dot( velocity, tes ) * tes;


			future = lwPos + velocity;

			seg = wirePoint - lwPos;
			segLength = length(seg);
			diff = wirePoint - future;

			//wire->segmentLength -= 10;
			if (length(diff) > wire->data.segmentLength)
			{
				future += normalize(diff) * (length(diff) - (wire->data.segmentLength));
				newVel2 = future - lwPos;
			}

			V2d totalVelDir = normalize((newVel1 + newVel2));//normalize( wireDir1 + wireDir2 );
															 //velocity = dot( (newVel1 + newVel2)/ 2.0, totalVelDir ) * normalize( totalVelDir );

			totalVelDir = normalize(wireDir1 + wireDir2);

			double dott = dot(-wireDir1, wireDir2);
			bool opposite = (dott > .95);

			V2d otherDir(totalVelDir.y, -totalVelDir.x);
			double dotvel = dot(velocity, otherDir);
			//correction for momentum



			V2d wdirs = (wireDir1 + wireDir2) / 2.0;


			if (opposite)
				wdirs = wireDir1;




			V2d owdirs(wdirs.y, -wdirs.x);

			doubleWireBoostDir = -owdirs;

			V2d inputDir;
			if (currInput.LLeft())
			{
				inputDir.x = -1;
			}
			else if (currInput.LRight())
			{
				inputDir.x = 1;
			}
			if (currInput.LUp())
			{
				inputDir.y = -1;
			}
			else if (currInput.LDown())
			{
				inputDir.y = 1;
			}

			dotvel = -dot(inputDir, owdirs);
			double v = .5;//.73;//.8;//.5;//1.0;
			if (dotvel > 0)
			{
				//cout << "a" << endl;
				if (canDoubleWireBoostParticle)
				{
					doubleWireBoost = true;
				}
				double q = dot(velocity, normalize(-owdirs));

				if (q >= 0 && q < 40)
				{
					velocity += -owdirs * v / (double)slowMultiple;
				}
				else
				{
					velocity += -owdirs * (v * 2) / (double)slowMultiple;
				}

				doubleWireBoostDir = -doubleWireBoostDir;
			}
			else if (dotvel < 0)
			{
				if (canDoubleWireBoostParticle)
				{
					doubleWireBoost = true;
				}
				//cout << "b" << endl;
				double q = dot(velocity, normalize(owdirs));
				if (q >= 0 && q < 40)
				{
					velocity += owdirs * v / (double)slowMultiple;
				}
				else
				{
					velocity += owdirs * (v * 2) / (double)slowMultiple;
				}

			}
			else
			{
			}

			if (!opposite)
			{
				V2d totalAcc;
				totalAcc.x = totalVelDir.x * doubleWirePull / (double)slowMultiple;
				if (totalVelDir.y < 0)
					totalAcc.y = totalVelDir.y * (doubleWirePull) / (double)slowMultiple;
				//totalAcc.y = totalVelDir.y * ( doubleWirePull + 1 )/ (double)slowMultiple;
				else
					totalAcc.y = totalVelDir.y * (doubleWirePull) / (double)slowMultiple;

				double beforeAlongAmount = dot(velocity, totalVelDir);

				if (beforeAlongAmount >= 20)
				{
					totalAcc *= .5;
				}
				else if (beforeAlongAmount >= 40)
				{
					totalAcc = V2d(0, 0);
				}
				//if( length( velocity ) > 20.0 )
				//{
				//	totalAcc *= .5;
				//}
				//totalVel *= dot( totalVelDir, rightWire->
				velocity += totalAcc;
			}

			/*if( opposite && !currInput.LLeft() && !currInput.LDown() && !currInput.LUp() && !currInput.LRight() )
			{
			double wdirsVel = dot( velocity, wdirs );
			double owdirsVel = dot( velocity, owdirs );
			}*/

			if (opposite)
			{
				dotvel = dot(inputDir, wireDir1);
				double towardsExtra = .5;//.7;//1.0;
				if (dotvel > 0)
				{
					//cout << "a" << endl;\



					velocity += wireDir1 * towardsExtra / (double)slowMultiple;
				}
				else if (dotvel < 0)
				{
					//cout << "b" << endl;
					velocity += wireDir2 * towardsExtra / (double)slowMultiple;
				}
				else
				{
				}
			}
			else
			{
				//totalVelDir
				if (dot(wireDir1, wireDir2) > .99)
					velocity = (velocity + AddAerialGravity(velocity)) / 2.0;
			}
			//removing the max velocity cap now that it doesnt pull you in a straight direction
			//double afterAlongAmount = dot( velocity, totalVelDir );
			//double maxAlong = 100;//45.0;

			//if( afterAlongAmount > maxAlong )
			//{
			//	velocity -= ( afterAlongAmount - maxAlong ) * totalVelDir;
			//}

			//if( length( velocity ) > afterAlongAmount )
			//{
			//	velocity = normalize( velocity ) * afterAlongAmount;
			//}

			//velocity = ( dot( velocity, totalVelDir ) + 4.0 ) * totalVelDir; //+ V2d( 0, gravity / slowMultiple ) ;
			///velocity += totalVelDir * doubleWirePull / (double)slowMultiple;
		}
		else if (rightWire->IsPulling())
		{
			V2d wPos = rightWire->data.storedPlayerPos;
			if (position != rightWire->data.storedPlayerPos)
			{
				//cout << "wPos: " << wPos.x << ", " << wPos.y << endl;
				//cout << "pp: " << position.x << ", " << position.y << endl;
				//assert( 0 && "what" );
			}
			V2d wirePoint = wire->data.anchor.pos;
			if (wire->data.numPoints > 0)
				wirePoint = wire->data.points[wire->data.numPoints - 1].pos;

			V2d tes = normalize(wPos - wirePoint);
			double temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			double val = dot(velocity, normalize(wirePoint - wPos));
			V2d otherTes;
			//if( val > 0 )
			{
				otherTes = val * normalize(wirePoint - wPos);
			}


			double speed = dot(velocity, tes);


			if (speed > triggerSpeed)
			{
				//rightWireBoost = true;
				speed += accel / (double)slowMultiple;
			}
			else if (speed < -triggerSpeed)
			{
				//rightWireBoost = true;
				speed -= accel / (double)slowMultiple;
			}
			else
			{


			}

			V2d wireDir = normalize(wirePoint - wPos);
			double otherAccel = .5 / (double)slowMultiple;

			V2d vec45(1, 1);
			vec45 = normalize(vec45);
			double xLimit = vec45.x;

			rightWireBoostDir = -V2d(wireDir.y, -wireDir.x);

			if (abs(wireDir.x) < xLimit)
			{
				if (wireDir.y < 0)
				{
					if (currInput.LLeft())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
					else if (currInput.LRight())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.y > 0)
				{
					if (currInput.LLeft())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LRight())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
				}
			}
			else
			{
				if (wireDir.x > 0)
				{
					if (currInput.LUp())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
					else if (currInput.LDown())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.x < 0)
				{
					if (currInput.LUp())
					{
						rightWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LDown())
					{
						rightWireBoost = true;
						speed -= otherAccel;
						rightWireBoostDir = -rightWireBoostDir;
					}
				}
			}

			if (rightWireBoost && framesSinceRightWireBoost >= singleWireBoostTiming && slowCounter == 1)
			{
				framesSinceRightWireBoost = 0;
			}
			else
			{
				rightWireBoost = false;
			}

			velocity = speed * tes;
			velocity += otherTes;
			

			
			//velocity += otherTes;


			V2d future = wPos + velocity;

			
			V2d seg = wirePoint - wPos;
			double segLength = length(seg);
			V2d diff = wirePoint - future;

			if (length(diff) > wire->data.segmentLength)
			{
				double pullVel = length(diff) - wire->data.segmentLength;
				V2d pullDir = normalize(diff);
				future += pullDir * pullVel;
				velocity = future - wPos;
			}

			//velocity = V2d( 0, 0 );

			//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
		}
		else if (leftWire->IsPulling())
		{
			wire = leftWire;
			V2d wPos = leftWire->data.storedPlayerPos;
			V2d wirePoint = wire->data.anchor.pos;
			if (wire->data.numPoints > 0)
				wirePoint = wire->data.points[wire->data.numPoints - 1].pos;

			V2d tes = normalize(wPos - wirePoint);
			double temp = tes.x;
			tes.x = tes.y;
			tes.y = -temp;

			double val = dot(velocity, normalize(wirePoint - wPos));
			V2d otherTes;
			//if( val > 0 )
			{
				otherTes = val * normalize(wirePoint - wPos);
			}



			V2d old = velocity;

			if (normalize(wirePoint - wPos).y > 0)
			{
				//	velocity -= V2d( 0, gravity );
			}

			//double accel = .3;
			double speed = dot(velocity, tes);

			if (speed > triggerSpeed)
			{

				speed += accel / (double)slowMultiple;
			}
			else if (speed < -triggerSpeed)
			{
				speed -= accel / (double)slowMultiple;
			}
			else
			{


			}

			V2d wireDir = normalize(wirePoint - wPos);
			double otherAccel = .5 / (double)slowMultiple;
			V2d vec45(1, 1);
			vec45 = normalize(vec45);
			double xLimit = vec45.x;
			leftWireBoostDir = -V2d(wireDir.y, -wireDir.x);
			if (abs(wireDir.x) < xLimit)
			{
				if (wireDir.y < 0)
				{
					if (currInput.LLeft())
					{
						leftWireBoost = true;
						leftWireBoostDir = -leftWireBoostDir;
						speed -= otherAccel;
					}
					else if (currInput.LRight())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.y > 0)
				{
					if (currInput.LLeft())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LRight())
					{
						leftWireBoost = true;
						speed -= otherAccel;
						leftWireBoostDir = -leftWireBoostDir;
					}
				}
			}
			else
			{
				if (wireDir.x > 0)
				{
					if (currInput.LUp())
					{
						leftWireBoost = true;
						speed -= otherAccel;
						leftWireBoostDir = -leftWireBoostDir;
					}
					else if (currInput.LDown())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
				}
				else if (wireDir.x < 0)
				{
					if (currInput.LUp())
					{
						leftWireBoost = true;
						speed += otherAccel;
					}
					else if (currInput.LDown())
					{
						leftWireBoost = true;
						speed -= otherAccel;
						leftWireBoostDir = -leftWireBoostDir;
					}
				}


			}


			if (leftWireBoost && framesSinceLeftWireBoost >= singleWireBoostTiming)
			{
				framesSinceLeftWireBoost = 0;
			}
			else
			{
				leftWireBoost = false;
			}

			velocity = speed * tes;
			velocity += otherTes;
			//velocity += otherTes;


			V2d future = wPos + velocity;

			V2d seg = wirePoint - wPos;
			double segLength = length(seg);
			V2d diff = wirePoint - future;

			//wire->segmentLength -= 10;
			if (length(diff) > wire->data.segmentLength)
			{
				future += normalize(diff) * (length(diff) - (wire->data.segmentLength));
				velocity = future - wPos;
			}
		}
	}
}

float Actor::GetSpeedBarPart()
{
	//purple can build up infinitely by hitting enemies. Who cares though, its pretty cool LOL

	float quant = 0;
	if (speedLevel == 0)
	{
		quant = (float)(currentSpeedBar / level1SpeedThresh);
	}
	else if (speedLevel == 1)
	{
		quant = (float)((currentSpeedBar - level1SpeedThresh) / (level2SpeedThresh - level1SpeedThresh));
	}
	else
	{
		quant = (float)((currentSpeedBar - level2SpeedThresh) / (GetMaxSpeed() - level2SpeedThresh));
	}

	if (quant > 1.f)
	{
		quant = 1.f;
	}

	return quant;
	/*if (currentSpeedBar >= level2SpeedThresh)
	{
		speedLevel = 2;
	}
	else if (currentSpeedBar >= level1SpeedThresh)
	{
		speedLevel = 1;
	}
	else
	{
		speedLevel = 0;
	}*/
}

void Actor::RailGrindMovement()
{
	V2d along = normalize(grindEdge->v1 - grindEdge->v0);

	TerrainRail *rail = grindEdge->rail;

	if (rail->GetRailType() == TerrainRail::ACCELERATE)
	{
		double accel = .6;
		if (grindSpeed > 0)
		{
			grindSpeed += accel;
		}
		else if (grindSpeed < 0)
		{
			grindSpeed += -accel;
		}
	}
	else if( rail->GetRailType() == TerrainRail::GRIND )
	{
		double accel = .1;
		if (grindSpeed > 0)
		{
			grindSpeed += accel;
		}
		else if (grindSpeed < 0)
		{
			grindSpeed += -accel;
		}
		//double ffac = 1.0;
		//double startAccel = .3;
		//V2d grn = grindEdge->Normal();

		//bool ceil = false;

		//if (grn.y > 0)
		//{
		//	along = -along;
		//	grn = -grn;
		//	ceil = true;
		//}

		//double extraAccel = .15;
		//double accel = 0;
		//if (GameSession::IsSteepGround(grn) > 0)
		//{
		//	double fac = GetGravity() *steepSlideGravFactor;//gravity * 2.0 / 3.0;
		//	accel = dot(V2d(0, fac), along) / slowMultiple;
		//	//cout << "steep accel: " << accel << endl;
		//}
		//else if (grn.x != 0)
		//{
		//	accel = dot(V2d(0, slideGravFactor * GetGravity()), along) / slowMultiple;
		//	//cout << "normal accel: " << accel << endl;
		//}

		//if (grindSpeed > 0)
		//{
		//	accel += extraAccel;
		//}
		//else if (grindSpeed < 0)
		//{
		//	accel -= extraAccel;
		//}

		//if (ceil)
		//	accel = -accel;

		//if (accel != 0 && abs(grindSpeed + accel) > abs(grindSpeed))
		//	grindSpeed += accel;

	}

	velocity = along * grindSpeed;
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

	if (velocity.x > 0)
	{
		facingRight = true;
	}
	else if (velocity.x < 0)
	{
		facingRight = false;
	}

	//cout << "grindSpeed: " << grindSpeed << endl;
}

bool Actor::CanRailSlide()
{
	bool isAttack = IsAttackAction(action);
	bool aerialRailSlideTest = ground == NULL && grindEdge == NULL && bounceEdge == NULL && action != RAILDASH &&
		action != RAILSLIDE && action != LOCKEDRAILSLIDE && action != AIRDASH && !isAttack;

	bool groundRailSlideTest = ground != NULL && grindEdge == NULL && action != DASH && !isAttack;

	if (aerialRailSlideTest || groundRailSlideTest)
	{
		return true;
	}

	return false;
}

bool Actor::CanRailGrind()
{
	if (IsOptionOn(POWER_GRIND))
	{
		if (!PowerButtonHeld() && prevInput.PowerButtonDown())
		{
			framesSinceGrindAttempt = 0;
		}

		if (ground == NULL && grindEdge == NULL && bounceEdge == NULL && action != RAILDASH)
		{
			if (PowerButtonHeld() || framesSinceGrindAttempt < maxFramesSinceGrindAttempt)
			{
				return true;
			}
		}
		else
		{
			framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
		}
	}

	return false;
}

bool Actor::IsRailSlideFacingRight()
{
	assert(grindEdge != NULL);

	V2d grindNorm = grindEdge->Normal();
	bool r = grindSpeed > 0;
	//(action == RAILGRIND || action == RAILSLIDE) &&
	if ( grindNorm.y > 0)
	{
		grindNorm = -grindNorm;
		r = !r;
	}

	return r;
}

void Actor::SetAllOptions(const BitField &b)
{
	startOptionField.Set(b);
	optionField.Set(b);
}

bool Actor::IsOptionOn(int index)
{
	return optionField.GetBit(index);
}

void Actor::SetOption(int optionIndex, bool on)
{
	optionField.SetBit(optionIndex, on);
	UpdatePowersMenu();
}

void Actor::SetStartOption(int optionIndex, bool on)
{
	startOptionField.SetBit(optionIndex, on);
	SetOption(optionIndex, on);
}

void Actor::ReverseSteepSlideJump()
{
	SetAction(JUMP);
	frame = 1;

	V2d along = ground->Along();
	V2d norm = ground->Normal();
	V2d alongSpeed = along * -groundSpeed;

	if (norm.x > 0)
	{
		alongSpeed.x += 5;
	}
	else if (norm.x < 0)
	{
		alongSpeed.x -= 5;
	}

	alongSpeed.y -= 8;

	reversed = false;
	ground = NULL;

	velocity = alongSpeed;
}

void Actor::CompleteCurrentMap()
{
	bool setRecord = false;
	bool gotGold = false;
	bool gotSilver = false;
	bool gotBronze = false;
	if (owner != NULL)
	{
		if (owner->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
		{
			if (!owner->IsReplayOn() && !owner->IsParallelSession())
			{
				adventureManager->CompleteCurrentMap(owner, setRecord, gotGold, gotSilver, gotBronze);
			}
		}

		owner->scoreDisplay->madeRecord = setRecord;
	}
	else if (editOwner != NULL)
	{
		if (!editOwner->usedWarp)
		{
			int totalFrames = editOwner->totalFramesBeforeGoal;

			int goldFrames = editOwner->mapHeader->goldSeconds * 60;
			int silverFrames = editOwner->mapHeader->silverSeconds * 60;
			int bronzeFrames = editOwner->mapHeader->bronzeSeconds * 60;

			gotGold = totalFrames <= goldFrames;
			gotSilver = totalFrames <= silverFrames;
			gotBronze = totalFrames <= bronzeFrames;
		}
	}

	sess->scoreDisplay->gotGold = gotGold;
	sess->scoreDisplay->gotSilver = gotSilver;
	sess->scoreDisplay->gotBronze = gotBronze;
}

void Actor::HandleWaitingScoreDisplay()
{
	if (sess->IsParallelSession())
	{
		return;
	}

	if (sess->scoreDisplay != NULL && sess->scoreDisplay->IsConfirmable())
	{
		bool aPressed = sess->controllerStates[actorIndex]->ButtonPressed_A();
		if (aPressed)
		{
			sess->scoreDisplay->Confirm();
		}
	}
	else if (sess->scoreDisplay != NULL && sess->scoreDisplay->IsWaiting())
	{
		bool aPressed = sess->controllerStates[actorIndex]->ButtonPressed_A();
		bool xPressed = sess->controllerStates[actorIndex]->ButtonPressed_X();
		bool yPressed = sess->controllerStates[actorIndex]->ButtonPressed_Y();
		bool r1Pressed = sess->controllerStates[actorIndex]->ButtonPressed_RightShoulder();
		bool bPressed = sess->controllerStates[actorIndex]->ButtonPressed_B();
		bool startPressed = sess->controllerStates[actorIndex]->ButtonPressed_Start();

		if (owner != NULL && owner->IsRushSession() )
		{
			if (aPressed)
			{
				owner->resType = GameSession::GameResultType::GR_WINCONTINUE;
				sess->scoreDisplay->Deactivate();
			}
			return;
		}

		if (aPressed || bPressed)
		{
			if (owner != NULL)
			{
				//bool levValid = owner->level != NULL; 
				if (aPressed )// && levValid)
				{
					if (owner->IsAdventureSession())
					{
						assert(owner->level != NULL);
						assert(adventureManager != NULL);

						SaveFile *currFile = adventureManager->currSaveFile;

						if (adventureManager->CanBoostToNextLevel())
						{
							owner->resType = GameSession::GameResultType::GR_WINCONTINUE;
						}
						else
						{
							owner->resType = GameSession::GameResultType::GR_WIN;
						}
						/*if (currFile->IsLevelLastInSector(owner->level))
						{
							
						}
						else
						{
							owner->resType = GameSession::GameResultType::GR_WINCONTINUE;
						}*/
					}
					else
					{
						owner->resType = GameSession::GameResultType::GR_WIN;
					}
				}
				else if (bPressed)
				{
					owner->resType = GameSession::GameResultType::GR_WIN;
				}
			}

			sess->scoreDisplay->Deactivate();
		}
		else if (xPressed)
		{
			if (owner != NULL)
			{
				owner->RestartWithNoReplayOrGhosts();
			}
			else
			{
				//a little hacky but after this function runs, if this flag is set, I know to restart the editor run
				editOwner->nextFrameRestartGame = true;
				//editOwner->TestPlayerMode();
			}
			return;
		}
		else if (yPressed && sess->scoreDisplay->IsIncludingExtraOptions())
		{
			if (owner != NULL )
			{
				owner->TryStartGhosts();
			}
		}
		else if (r1Pressed && sess->scoreDisplay->IsIncludingExtraOptions())
		{
			if (owner != NULL)
			{
				owner->TryStartMyBestReplay();
			}
		}
		else if (startPressed && sess->scoreDisplay->IsIncludingExtraOptions())
		{
			if (owner != NULL && owner->mainMenu->steamOn )
			{
				owner->StartLeaderboard();
			}
		}
	}
}

void Actor::EndLevelWithoutGoal()
{
	sess->ActivateScoreDisplay(60);
	SetAction(Actor::GOALKILLWAIT);
	frame = 0;

	//endtestmode
}

void Actor::SetStandInPlacePos(Edge *g, double q,
	bool fr)
{
	facingRight = fr;
	SetGroundedPos(g, q);
	StandInPlace();
}

void Actor::SetGroundedPos(Edge *g, double q, double xoff)
{
	holdJump = false;
	holdDouble = false;
	ground = g;
	edgeQuantity = q;

	RestoreAirOptions();
	
	offsetX = xoff;

	V2d norm = ground->Normal();

	position = ground->GetPosition(edgeQuantity);

	if (norm.y <= 0)
	{
		reversed = false;
		position.y -= normalHeight;
	}
	else
	{
		reversed = true;
		position.y += normalHeight;
	}

	position.x += offsetX;
}

void Actor::SetAirPos(V2d &pos, bool fr)
{
	ground = NULL;
	bounceEdge = NULL;
	grindEdge = NULL;
	SetAction(JUMP);
	frame = 1;
	position = pos;
	facingRight = fr;
	offsetX = 0;
	reversed = false;
}

void Actor::SetGroundedPos(Edge *g, double q)
{
	holdJump = false;
	holdDouble = false;
	ground = g;
	edgeQuantity = q;

	RestoreAirOptions();

	V2d norm = ground->Normal();
	if (norm.x > 0)
	{
		offsetX = b.rw;
	}
	else if (norm.x < 0)
	{
		offsetX = -b.rw;

	}
	else
	{
		offsetX = 0;
	}

	position = ground->GetPosition(edgeQuantity);

	if (norm.y <= 0)
	{
		reversed = false;
		position.y -= normalHeight;
	}
	else
	{
		reversed = true;
		position.y += normalHeight;
	}

	position.x += offsetX;
}

void Actor::SetStoryRun(bool fr, double maxAutoRun, Edge * g,
	double q)
{
	SetGroundedPos(g, q);
	SetAutoRun(fr, maxAutoRun);
}

void Actor::SetStoryRun(bool fr, double maxAutoRun, Edge * g,
	double q, Edge *end, double eq )
{
	SetGroundedPos(g, q);
	SetAutoRun(fr, maxAutoRun);
	SetAutoRunStopper(end, eq);
}

void Actor::SetAutoRunStopper(Edge *g, double q)
{
	autoRunStopEdge = g;
	autoRunStopQuant = q;
}

bool Actor::IsAutoRunning()
{
	return action == AUTORUN;
}

void Actor::TurnFace()
{
	assert(ground != NULL);
	SetAction(SEQ_TURNFACE);
	frame = 0;
}

void Actor::GatorStun()
{
	SetAirPos(position, facingRight );
	SetAction(SEQ_GATORSTUN);
	frame = 0;
	velocity = V2d(0, 0);
}

void Actor::StandInPlace()
{
	assert(ground != NULL);
	SetAction(STAND);
	frame = 0;
	groundSpeed = 0;
}

void Actor::WaitInPlace()
{
	sess->SetPlayerInputOn(false);
	SetAction(STAND);
	//SetAction(SEQ_WAIT);
	frame = 0;
	physicsOver = true;
	groundSpeed = 0;
}

void Actor::Wait()
{
	SetAction(SEQ_WAIT);
	frame = 0;
}

bool Actor::CheckWall( bool right )
{
	double wThresh = 8;
	V2d vel;
	if( right )
	{
		vel.x = wThresh;
	}
	else
	{
		vel.x = -wThresh;
	}
	V2d newPos = (position) + vel;
	//test.collisionPriority = 10000;
	//test.edge = NULL;

	//(minContact.normal.y > 0 && abs(minContact.normal.x) > .9) )

	minContact.collisionPriority = 10000;
	minContact.edge = NULL;
	minContact.resolution = V2d( 0, 0 );
	col = false;
	queryType = Q_CHECKWALL;
	tempVel = vel;

	//sf::Rect<double> r( 
	Rect<double> r( position.x + tempVel.x + b.offset.x - b.rw, position.y + tempVel.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );


	GetTerrainTree()->Query( this, r );
	
	
	sess->barrierTree->Query(this, r);

	GetRailEdgeTree()->Query(this, r);
	


	if( !col )
	{
		return false;
	}

	bool wally = false;
	if( minContact.edge != NULL && !minContact.edge->IsInvisibleWall() )
	{
		//double quant = minContact.edge->GetQuantity( test.position );
		double quant = minContact.edge->GetQuantity(minContact.position);

		bool zero = false;
		bool one = false;
		if( quant <= 0 )
		{
			zero = true;
			quant = 0;
		}
		else if( quant >= length( minContact.edge->v1 - minContact.edge->v0 ) )
		{
			one = true;
			quant = length( minContact.edge->v1 - minContact.edge->v0 );
		}

		//if( !zero && !one )
		//	return false;

		//cout << "zero: " << zero << ", one: " << one << endl;
		//cout << "haha: "  << quant << ", " << length( test.edge->v1 - test.edge->v0 ) << endl;
		Edge *e = minContact.edge;
		V2d en = e->Normal();
		Edge *e0 = e->edge0;
		Edge *e1 = e->edge1;


		if( approxEquals(en.x,1) || approxEquals(en.x,-1) )
		{
			return true;
		}

		
		if( en.y > 0 && abs( en.x ) > .8 )
		{
			//cout << "here" << endl;
			return true;
		}

		if( (one && en.x < 0 && en.y > 0 ) )
		{
			if (e1 != NULL)
			{
				V2d te = e1->v1 - e1->v0;
				if (te.x > 0)
				{
					return true;
				}
			}
		}

		if( (zero && en.x > 0 && en.y > 0 ) )
		{
			if (e0 != NULL)
			{
				V2d te = e0->v0 - e0->v1;
				if (te.x < 0)
				{
					return true;
				}
			}
		}
	}
	return false;

}

bool Actor::TryStandupOnForcedGrindExit()
{
	V2d op = position;

	V2d grindNorm = grindEdge->Normal();

	bool oldReversed = reversed;

	if (grindNorm.y < 0)
	{
		double extra = 0;
		if (grindNorm.x > 0)
		{
			offsetX = b.rw;
			extra = .1;
		}
		else if (grindNorm.x < 0)
		{
			offsetX = -b.rw;
			extra = -.1;
		}
		else
		{
			offsetX = 0;
		}

		position.x += offsetX + extra;

		position.y -= normalHeight + .1;
	}
	else if (grindNorm.y > 0)
	{
		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}

		position.y += normalHeight + .1;

		reversed = true;
	}
	else
	{
		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}
	}

	double ex = .05;//.1 //needs to be smaller than the extra amount given to grind for 
					//standup to work for grind.
	Rect<double> r;
	r = Rect<double>(position.x + b.offset.x - b.rw - ex, position.y - ex /*+ b.offset.y*/ - normalHeight, 2 * b.rw + 2 * ex, 2 * normalHeight + 2 * ex);

	queryType = Q_CHECK_GRIND_TRANSFER;
	grindTransferCheckEdge = NULL;

	GetTerrainTree()->Query(this, r);
	sess->railEdgeTree->Query(this, r);

	reversed = oldReversed;

	//sess->barrierTree->Query(this, r);

	if (grindTransferCheckEdge != NULL)
	{
		position = op;
		grindEdge = grindTransferCheckEdge;
		edgeQuantity = grindTransferCheckEdge->GetQuantity(position);

		CheckGrindEdgeForTerrainFade();

		CheckGates();
		return true;
	}
	else
	{
		framesInAir = 0;
		SetAction(JUMP);
		frame = 1;
		ground = NULL;
		grindEdge = NULL;
		reversed = false;
		offsetX = 0;
		PhysicsResponse();
		if (velocity.x > 0)
		{
			facingRight = true;
		}
		else if (velocity.x < 0)
		{
			facingRight = false;
		}

		CheckGates();

		return false;
	}
	
	
}

bool Actor::CheckStandUp()
{
	double ex = .05;//.1 //needs to be smaller than the extra amount given to grind for 
					//standup to work for grind.
	Rect<double> r;

	r = Rect<double>(position.x + b.offset.x - b.rw - ex, position.y - ex - normalHeight, 2 * b.rw + 2 * ex, 2 * normalHeight + 2 * ex);

	queryType = Q_CHECK;
	checkValid = true;
	GetTerrainTree()->Query(this, r);
	sess->railEdgeTree->Query(this, r);

	sess->barrierTree->Query(this, r);

	possibleEdgeCount = 0;

	/*if( checkValid )
	cout << "check valid" << endl;
	else
	{
	cout << "cant stand up" << endl;
	}*/

	/*if( checkValid )
	{
	cout << "canstand" << endl;
	}
	else
	{
	cout << "cannot stand" << endl;
	}*/
	return checkValid;
}

bool Actor::CheckStandUpToDash()
{
	double ex = .08;//.1 //needs to be smaller than the extra amount given to grind for 
					//standup to work for grind.
	Rect<double> r;

	double yOff = normalHeight - dashHeight;
	if (reversed)
		yOff = -yOff;

	r = Rect<double>(position.x + b.offset.x - b.rw - ex, position.y + yOff - ex - dashHeight, 2 * b.rw + 2 * ex, 2 * dashHeight + 2 * ex);

	queryType = Q_CHECK;
	checkValid = true;
	GetTerrainTree()->Query(this, r);
	sess->railEdgeTree->Query(this, r);

	sess->barrierTree->Query(this, r);

	possibleEdgeCount = 0;

	return checkValid;
}

bool Actor::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	Rect<double> oldR( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	
	position += vel;

	if (action == SPRINGSTUNTELEPORT || action == WARP )
		return false;
	
	Rect<double> newR( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	minContact.collisionPriority = 1000000;
	
	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = right;
	if (oldRight > maxRight)
	{
		maxRight = oldRight;
	}

	double maxBottom = bottom;
	if (oldBottom > maxBottom)
	{
		maxBottom = oldBottom;
	}

	double minLeft = newR.left;
	if (oldR.left < minLeft)
	{
		minLeft = oldR.left;
	}

	double minTop = newR.top;
	if (oldR.top < minTop)
	{
		minTop = oldR.top;
	}
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	

	double ex = 1;
	Rect<double> r( minLeft - ex, minTop - ex, (maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2 );

	//collision = false;
	col = false;

	tempVel = vel;
	minContact.edge = NULL;

	//cout << "---STARTING QUERY--- vel: " << vel.x << ", " << vel.y << endl;
	queryType = Q_RESOLVE;

	Edge *oldGround = ground;
	//double oldQuant = edgeQuantity;
	//double oldOffsetX = offsetX;
	double oldGs = groundSpeed;
	
//	Query( this, owner->testTree, r );

	//cout << "Start resolve" << endl;

	if (action != LOCKEDRAILSLIDE)//change this to be a more general bool later
	{
		GetTerrainTree()->Query(this, r);
	}
	
	sess->railEdgeTree->Query(this, r);

	
	sess->barrierTree->Query(this, r);

	if( col )
	{
		/*if( minContact.movingPlat != NULL )
		{
			velocity += minContact.movingPlat->vel;
		}*/
		if( minContact.normal.x == 0 && minContact.normal.y == 0 )
		{
			
			minContact.normal = minContact.edge->Normal();
			//cout << "setting the normal to: " << minContact.normal.x << ", " <<minContact.normal.y << endl;
		}
		else
		{
			if( oldGround != NULL )
			{
				if( oldGs > 0 && minContact.edge == oldGround->edge0 

					|| ( oldGs > 0 && minContact.edge == oldGround->edge1 ) 
					|| minContact.edge == oldGround )
				{
					//col = false;
					//cout << "setting false" << endl;
				}
			}
			
		}
	}

	TryCheckGrass();

	
	

	//queryMode = "item";
	//owner->itemTree->Query( this, r );
	
	if (owner != NULL)
	{
		queryType = Q_ENVPLANT;
		owner->envPlantTree->Query(this, r);

		Rect<double> staticItemRect(position.x - 400, position.y - 400, 800, 800);//arbitrary decent sized area around kin
		owner->staticItemTree->Query(NULL, staticItemRect);
	}

	canRailGrind = CanRailGrind();
	canRailSlide = true;//CanRailSlide();

	//moved canrailslide calculation to after querying, for locked rails

	//if (ground == NULL && bounceEdge == NULL && grindEdge == NULL && (canRailGrind || canRailSlide ) )
	//if( owner != NULL && owner->totalRails > 0 && (canRailGrind || canRailSlide) )
	//if( canRailGrind || canRailSlide )
	{
		queryType = Q_RAIL;
		sess->railEdgeTree->Query(this, r);
	}

	
	currBooster = NULL;
	currTimeBooster = NULL;
	currFreeFlightBooster = NULL;
	currBounceBooster = NULL;
	currGravModifier = NULL;
	currHomingBooster = NULL;

	double activeExtra = 500;
	sf::Rect<double> activeR = r;
	activeR.left -= activeExtra;
	activeR.top -= activeExtra;
	activeR.width += activeExtra * 2;
	activeR.height += activeExtra * 2;

	if (!simulationMode)
	{
		queryType = Q_ACTIVEITEM;
		sess->activeItemTree->Query(this, r);//activeR);
	}
	
	
	return col;
}

V2d Actor::UpdateReversePhysics()
{
	V2d leftGroundExtra( 0, 0 );
	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );
	wallNormal.x = 0;
	wallNormal.y = 0;
	currWall = NULL;
	if( grindEdge != NULL )
	{
		movement = grindSpeed / (double)slowMultiple / GetNumSteps();
	}
	else if( ground != NULL )
	{
		movement = groundSpeed / (double)slowMultiple / GetNumSteps();
	}
	else
	{
		movementVec = velocity / (double)slowMultiple / GetNumSteps();
	}

	movement = -movement;

		
	while( (ground != NULL && !approxEquals( movement, 0 )))// || ( ground == NULL && length( movementVec ) > 0 ) )
	{
		if( ground != NULL )
		{
			double steal = 0;
			if( movement > 0 )
			{
				if( movement > maxMovement )
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else 
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}


			//double extra = 0;
			extra = 0;
			bool leaveGround = false;
			double q = edgeQuantity;

			V2d gNormal = ground->Normal();
			
			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;

			V2d e0n, e1n;
			if (e0 != NULL)
			{
				e0n = e0->Normal();
			}

			if (e1 != NULL)
			{
				e1n = e1->Normal();
			}

			gNormal = -gNormal;
			e0n = -e0n;
			e1n = -e1n;
			offsetX = -offsetX;

			double m = movement;
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			

			bool transferLeft =  e0 != NULL && q == 0 && movement < 0 //&& (groundSpeed < -steepClimbSpeedThresh || e0n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
				|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ));

			//bool a = q == 0 && movement < 0;
			//bool c = ((gNormal.x == 0 && e0n.x == 0 )
			//	|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
			//	|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ));

			
			bool transferRight = e1 != NULL && q == groundLength && movement > 0 //&& (groundSpeed > steepClimbSpeedThresh || e1n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
			
			
			//cout << "a: " << a << " b: " << c << endl;
			if( transferLeft )
			{
				//cout << "transfer left" << endl;
				bool unlockedGateJumpOff = false;
				if( e0->IsGateEdge() )
				{
					Gate * g = (Gate*)e0->info;
					if( CanUnlockGate( g ) )
					{
						UnlockGate( g );

						SetTouchedGate(g);
					}

					if (!g->IsLocked())
					{
						unlockedGateJumpOff = true;
					}
				}

				//gNormal.x = -gNormal.x;
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				V2d nextNorm = e0n;
				double yDist = abs( gNormal.x ) * -groundSpeed;

				//bool jumpOff = (currInput.LUp() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x <= 0 );
				//jumpOff = jumpOff || abs( e0n.x ) >= wallThresh || nextNorm.y >= 0;
				
				bool nextSteep = e0n.y > -steepThresh;
				bool currSteep = gNormal.y > -steepThresh;
				bool movingDown = gNormal.x < 0;
				bool movingUp = gNormal.x > 0;
				bool nextMovingDown = e0n.x < 0;
				bool nextMovingUp = e0n.x > 0;

				//if( !jumpOff )
				//{

				bool jumpOff = false;
				bool tip = false;
				
				if( nextNorm.y >= 0 || abs( e0n.x ) >= wallThresh || e0->IsInvisibleWall())
				{
					jumpOff = true;
				}
				else if( currSteep )
				{
					if( movingUp )
					{
						if(nextMovingDown && nextSteep)
						{
							jumpOff = true;
							tip = true;
						}
						else
						{
							jumpOff = false;
						}
					}
					else
					{
						jumpOff = false;
					}
				}
				else
				{
					if( movingUp )
					{
						if( nextSteep && nextMovingDown )
						{
							jumpOff = true;
							//tip = true;
						}
						else
						{
							jumpOff = false;
						}
					}
					else
					{
						if (gNormal.x == 0 && nextMovingDown && nextSteep)
						{
							jumpOff = true;
						}
						else
						{
							jumpOff = false;
						}
					}
				}


				//}
				
				if( jumpOff || unlockedGateJumpOff)
				{	
					if (tip)
					{
						LeaveTipTransfer(true, V2d(0, 1));
						//honestly not sure what causes this to hit here! LOL I removed one of the negative cases
						//by commenting it out and idk when this happens now
					}
					else
					{
						LeaveGroundTransfer(true, V2d(0, 1));
					}


					/*reversed = false;

					V2d along = ground->Along();

					if (tip)
					{
						along = V2d(-1, 0);
					}

					velocity = along * -groundSpeed;
					movementVec = along * extra;

					movementVec.y += .1;
					if( movementVec.x <= .1 )
					{
						movementVec.x = .1;
					}

					leftGround = true;
					SetAction( JUMP );
					frame = 1;
					
					ground = NULL;*/

					leftGroundExtra = movementVec;
					return leftGroundExtra;
				}
				else if( nextSteep && nextMovingUp )
				{
					//steeptesting 1 reverse
					if( groundSpeed <= steepClimbSpeedThresh && ( !ground->IsSteepGround() || movingDown) )
						//&& action != STEEPCLIMB && action != STEEPCLIMBATTACK)
					{
						offsetX = -offsetX;
						groundSpeed = 0;
						break;
					}
					else
					{
						ground = next;
						q = length( ground->v1 - ground->v0 );	
					}
				}
				else
				{
					ground = next;
					q = length( ground->v1 - ground->v0 );	
				}

				//rightWire->UpdateAnchors( V2d( 0, 0 ) );
				//leftWire->UpdateAnchors( V2d( 0, 0 ) );
			}
			else if( transferRight )
			{
				//cout << "transferright" << endl;

				bool jumpOff = false;
				bool tip = false;
				bool unlockedGateJumpOff = false;
				if( e1->IsGateEdge() )
				{
					Gate * g = (Gate*)e1->info;
					if( CanUnlockGate( g ) )
					{
						UnlockGate( g );

						SetTouchedGate(g);
					}

					if (!g->IsLocked())
					{
						unlockedGateJumpOff = true;
					}
				}


				Edge *next = ground->edge1;
				V2d nextNorm = e1n;
				double yDist = abs( gNormal.x ) * -groundSpeed;

				bool nextSteep = e1n.y > -steepThresh;
				bool currSteep = gNormal.y > -steepThresh;
				bool movingDown = gNormal.x > 0;
				bool movingUp = gNormal.x < 0;
				bool nextMovingDown = e1n.x > 0;
				bool nextMovingUp = e1n.x < 0;

				
				
				if( nextNorm.y >= 0 || abs( e1n.x ) >= wallThresh || e1->IsInvisibleWall())
				{
					jumpOff = true;
				}
				else if( currSteep )
				{
					if( movingUp )
					{
						if( nextMovingDown && nextSteep )
						{
							jumpOff = true;
							tip = true;
						}
						else
						{
							jumpOff = false;
						}
					}
					else
					{
						jumpOff = false;
					}
				}
				else
				{
					if( movingUp )
					{
						if( nextSteep && nextMovingDown )
						{
							jumpOff = true;
							//tip = true;
						}
						else
						{
							jumpOff = false;
						}
					}
					else
					{
						if( gNormal.x == 0 && nextMovingDown && nextSteep )
						{
							jumpOff = true;
						}
						else
						{
							jumpOff = false;
						}
						//jumpOff = false;
					}
				}

				if( jumpOff || unlockedGateJumpOff)
				{
					/*reversed = false;
					
					V2d along = ground->Along();

					if (tip)
					{
						along = V2d(-1, 0);
					}

					velocity = along * -groundSpeed;
					movementVec = along * extra;

					movementVec.y += .1;
					if( movementVec.x >= -.1 )
					{
						movementVec.x = -.1;
					}

					leftGround = true;
					SetAction( JUMP );
					frame = 1;
					ground = NULL;
					holdJump = false;*/

					if (tip)
					{
						LeaveTipTransfer(false, V2d(0, 1));
						//honestly not sure what causes this to hit here! LOL I removed one of the negative cases
						//by commenting it out and idk when this happens now
					}
					else
					{
						LeaveGroundTransfer(false, V2d(0, 1));
					}


					//cout << "jumpoff b: " << along.x << ", " << along.y << endl;

					leftGroundExtra = movementVec;
					return leftGroundExtra;
				}
				else if( nextSteep && nextMovingUp )
				{
					//steeptesting 2 reverse
					if( groundSpeed >= -steepClimbSpeedThresh && (!ground->IsSteepGround() || movingDown) )
						//&& action != STEEPCLIMB && action != STEEPCLIMBATTACK)
					{
						groundSpeed = 0;
						offsetX = -offsetX;
						break;
					}
					else
					{
						ground = next;
						q = 0;
					}
				}
				else
				{
					ground = next;
					q = 0;
				}

				//rightWire->UpdateAnchors( V2d( 0, 0 ) );
				//leftWire->UpdateAnchors( V2d( 0, 0 ) );

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//cout << "slide: " << q << ", " << offsetX << endl;
				if( movement > 0 )
					extra = (offsetX + movement) - b.rw;
				else 
				{
					extra = (offsetX + movement) + b.rw;
				}
				double m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offsetX = b.rw;
					}
					else
					{
						offsetX = -b.rw;
					}
				}
				else
				{
					movement = 0;
					offsetX += m;
				}


				//wire problem could arise later because i dont update anchors when i hit an edge.
				//if (approxEquals(m, 0))
				//{
				//	offsetX = -offsetX;
				//	break;
				//}
				//else
				{
					//new approxtesting reverse changeoffset
					if (approxEquals(m, 0))
					{
						if (movement > 0)
						{
							/*Edge *next = ground->edge1;
							if (next == NULL)
							{
								LeaveGroundTransfer(true, V2d(0, -gravity * 2));
								break;
							}
							else*/
							{
								m = .01;
							}
						}
						else if (movement < 0)
						{
							/*Edge *next = ground->edge0;

							if (next == NULL)
							{
								LeaveGroundTransfer(false, V2d(0, -gravity * 2));
								break;
							}
							else*/
							{
								m = -.01;
							}
						}
					}


				
					V2d oldPos = position;
					bool hit = ResolvePhysics( V2d( -m, 0 ));
					
					//cout << "check offsetX: " << xOffset << "\n";

					if (IsOnRailAction(action))
					{
						offsetX = -offsetX;
						break;
					}
					
					//cout << "hit: " << hit << endl;
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						V2d eNorm = minContact.normal;
						m = -m;
						//cout << "eNorm: " << eNorm.x << ", " << eNorm.y << ", m: " << m << endl;
						if( eNorm.y > 0 )
						{
							//this could be a problem later hopefully i solved it!
							bool speedTransfer = (eNorm.x < 0 && eNorm.y < steepThresh 
								&& groundSpeed > 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y < steepThresh 
									&& groundSpeed < 0 && groundSpeed <= steepClimbSpeedThresh);

							if( minContact.position.y <= position.y - b.rh + 5 && !speedTransfer )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									//cout << "a" << endl;
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

									edgeQuantity = q;
									offsetX = -b.rw;

									ProcessGroundedCollision();
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									//cout << "b" << endl;
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

									edgeQuantity = q;
									offsetX = b.rw;

									ProcessGroundedCollision();
									continue;
								}
								else
								{
									ProcessGroundedCollision();
								}
								

							}
							else
							{
								//cout << "c2:" << speedTransfer << endl;
								offsetX -= minContact.resolution.x;
								offsetX = -offsetX;
								groundSpeed = 0;
								//offsetX = -offsetX;

								ProcessGroundedCollision();
								break;
							}
						}
						else
						{

							if( bounceFlameOn && abs( groundSpeed ) > 1 )
							{
								storedBounceGroundSpeed = groundSpeed * slowMultiple;
								groundedWallBounce = true;
							}

							offsetX -= minContact.resolution.x;
							offsetX = -offsetX;

							//cout << "m: " << m << "\n";
							//cout << "setting offsetX: " << offsetX << "\n";
							groundSpeed = 0;
							//offsetX = -offsetX; 

							ProcessGroundedCollision();
							break;
						}
					}
					else
					{
						if (movement > 0 && offsetX == b.rw && gNormal.x > 0 && gNormal.y > -steepThresh)// && groundSpeed > 5)
						{
							if (e0n.x < 0 && e0n.y > -steepThresh)
							{
								LeaveTipTransfer(true, V2d(0, 1));
							}
							else
							{
								LeaveGroundTipTransfer(true, e0, V2d(0, gravity * 2));
							}
							break;
						}
						else if (movement < 0 && offsetX == -b.rw && gNormal.x < 0 && gNormal.y > -steepThresh)// && groundSpeed < -5)
						{
							if (e1n.x > 0 && e1n.y > -steepThresh)
							{
								LeaveTipTransfer(false, V2d(0, 1));
							}
							else
							{
								LeaveGroundTipTransfer(false, e1, V2d(0, gravity * 2));
							}
							break;
						}
						else
						{
							V2d wVel = position - oldPos;
							edgeQuantity = q;
						}

						//reversed
						if (movement > 0 && offsetX == b.rw && gNormal.x > 0 && gNormal.y > -steepThresh )
						{
							LeaveTipTransfer(true, V2d(0, 1));
							break;
						}
						else if (movement < 0 && offsetX == -b.rw && gNormal.x < 0 && gNormal.y > -steepThresh )
						{
							LeaveTipTransfer(false, V2d(0, 1));
							break;
						}
						else
						{
							V2d wVel = position - oldPos;
							edgeQuantity = q;
						}

						/*V2d wVel = position - oldPos;
						edgeQuantity = q;*/
						
					}

					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				
				

				//if( approxEquals(m, 0))
				if( false )
				{

					//cout << "reverse secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					if (groundSpeed > 0)
					{
						Edge *next = ground->edge0;

						if (next == NULL)
						{
							//using 1 instead of double grav since grav * 2 is 2, and I feel like that might be too much
							LeaveGroundTransfer(true, V2d(0, 1));
							break;
						}
						
						/*double yDist = abs(gNormal.x) * -groundSpeed;
						Edge *next = ground->edge0;
						V2d nextNorm = e0n;
						if (next != NULL && !next->IsInvisibleWall() && nextNorm.y < 0 && abs(e0n.x) < wallThresh && !(HoldingRelativeUp() && !currInput.LLeft() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && nextNorm.x < gNormal.x))
						{
							if (e0n.x > 0 && e0n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh)
							{
								if (!TryUnlockOnTransfer(e0))
								{
									groundSpeed = 0;
								}
							}
						}
						else if (next != NULL && abs(e0n.x) >= wallThresh && !e0->IsInvisibleWall())
						{
							if (!TryUnlockOnTransfer(e0))
							{
								if (bounceFlameOn && abs(groundSpeed) > 1)
								{
									storedBounceGroundSpeed = groundSpeed * slowMultiple;
									groundedWallBounce = true;
								}
								groundSpeed = 0;
							}
						}
						else
						{
							reversed = false;
							velocity = normalize(ground->v1 - ground->v0) * -groundSpeed;
							movementVec = normalize(ground->v1 - ground->v0) * extra;
							leftGround = true;

							ground = NULL;
						}*/
					}
					else if (groundSpeed < 0)
					{

						Edge *next = ground->edge1;


						if (next == NULL)
						{
							LeaveGroundTransfer(false, V2d(0, 1));
							break;
						}
						//cout << "right"<< endl;
						/*Edge *next = ground->edge1;
						V2d nextNorm = e1n;
						double yDist = abs(gNormal.x) * -groundSpeed;
						if (next != NULL && !next->IsInvisibleWall() && nextNorm.y < 0 && abs(e1n.x) < wallThresh && !(HoldingRelativeUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && nextNorm.x > 0))
						{
							if (e1n.x < 0 && e1n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh)
							{
								if (!TryUnlockOnTransfer(e1))
								{
									groundSpeed = 0;
								}
							}
						}
						else if (next != NULL && !e1->IsInvisibleWall() && abs(e1n.x) >= wallThresh)
						{
							if (!TryUnlockOnTransfer(e1))
							{
								if (bounceFlameOn && abs(groundSpeed) > 1)
								{
									storedBounceGroundSpeed = groundSpeed * slowMultiple;
									groundedWallBounce = true;
								}
								groundSpeed = 0;
							}
						}
						else
						{
							velocity = normalize(ground->v1 - ground->v0) * -groundSpeed;

							movementVec = normalize(ground->v1 - ground->v0) * extra;

							leftGround = true;
							reversed = false;
							ground = NULL;
						}*/

					}

					if (ground != NULL)
					{
						offsetX = -offsetX;
						break;
					}
				}
				else
				{	
					//new approxtesting reverse
					if (approxEquals(m, 0))
					{
						if (movement > 0)
						{
							Edge *next = ground->edge1;
							if (next == NULL)
							{
								LeaveGroundTransfer(true, V2d(0, 1));
								break;
							}
							else
							{
								m = .01;
							}
						}
						else if (movement < 0)
						{
							Edge *next = ground->edge0;

							if (next == NULL)
							{
								LeaveGroundTransfer(false, V2d(0, 1));
								break;
							}
							else
							{
								m = -.01;
							}
						}
					}



					//wire problem could arise later because i dont update anchors when i hit an edge.
					V2d oldPos = position;
					//cout << "moving: " << (normalize( ground->v1 - ground->v0 ) * m).x << ", " << 
					//	( normalize( ground->v1 - ground->v0 ) * m).y << endl;

					V2d resMove = normalize( ground->v1 - ground->v0 ) * m;
					bool hit = ResolvePhysics( resMove );
					

					if (IsOnRailAction(action))
					{
						offsetX = -offsetX;
						break;
					}

					//cout << "hit: " << hit << endl;
					if( hit && (( m > 0 && ( minContact.edge != ground->edge0) ) || ( m < 0 && ( minContact.edge != ground->edge1 ) ) ) )
					{
						//honestly no idea why I had this in the first place?
						if ( minContact.edge->IsInvisibleWall() &&
							minContact.edge->Normal().y == 1.0 )
						{
							velocity = normalize(ground->v1 - ground->v0) * -groundSpeed;
							velocity.y = 0;
							movementVec = normalize(ground->v1 - ground->v0) * extra;

							leftGround = true;
							reversed = false;
							ground = NULL;

							ProcessGroundedCollision();
							break;
							//return V2d( 0, 0 );
						}
						else
						{

							V2d eNorm = minContact.normal;//minContact.edge->Normal();
							eNorm = -eNorm;


							//cout<< "blah" << endl;
							if (eNorm.y < 0)
							{
								bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x > 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh);


								if (minContact.position.y <= position.y + minContact.resolution.y - b.rh + b.offset.y + 5 && !speedTransfer)
								{
									double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									double sum = position.x + minContact.resolution.x;
									double diff = sum - minContact.position.x;

									if ((test < -b.rw && !approxEquals(test, -b.rw)) || (test > b.rw && !approxEquals(test, b.rw)))
									{
										//this is for corner border cases
										cout << "REVERSED CORNER BORDER CASE: " << test << endl;

										q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);

										groundSpeed = 0;
										edgeQuantity = q;
										offsetX = -offsetX;

										ProcessGroundedCollision();
										break;
									}
									else
									{
										//cout << "c" << endl;   
										//cout << "eNorm: " << eNorm.x << ", " << eNorm.y << endl;

										position += minContact.resolution; //probably does nothing

										ground = minContact.edge;

										q = ground->GetQuantity(minContact.position);


										if (minContact.normal.x > 0)
										{
											offsetX = b.rw;
										}
										else if (minContact.normal.x < 0)
										{
											offsetX = -b.rw;
										}
										else
										{
											offsetX = (position.x + b.offset.x) - minContact.position.x;
										}
										offsetX = -offsetX;

										ProcessGroundedCollision();
									}
								}
								else
								{
									q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);
									groundSpeed = 0;
									edgeQuantity = q;
									offsetX = -offsetX;

									ProcessGroundedCollision();
									break;
								}
							}
							else
							{
								V2d testVel = normalize(ground->v1 - ground->v0) * -groundSpeed;
								//maybe want to do this on some ceilings but its hard for now. do i need it?
								//if( currInput.LUp() && testVel.y < -offSlopeByWallThresh && eNorm.y == 0 )

								//might cause some weird stuff w/ bounce but i can figure it out later
								if (testVel.y > offSlopeByWallThresh && eNorm.y == 0 && !bounceFlameOn && !minContact.edge->IsInvisibleWall())
								{
									assert(abs(eNorm.x) > wallThresh);
									//		cout << "testVel: " << testVel.x << ", " << testVel.y << endl;
									velocity = testVel;

									position += minContact.resolution;

									movementVec = normalize(ground->v1 - ground->v0) * steal;//-extra;

									leftGround = true;
									ground = NULL;
									SetAction(JUMP);
									holdJump = false;
									frame = 1;
									reversed = false;

									ProcessGroundedCollision();
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
								}
								else
								{
									if (bounceFlameOn && abs(groundSpeed) > 1)
									{
										//steeptesting 3 reverse

										//if (action != STEEPCLIMB
										//	&& action != STEEPCLIMBATTACK)
										//if( gNormal.x < 0 && movement )
										{

											storedBounceGroundSpeed = groundSpeed * slowMultiple;
											groundSpeed = 0;
											groundedWallBounce = true;
											offsetX = -offsetX;
										}
									}
									else if (minContact.normal.y < 0 && minContact.normal.y > -steepThresh
										//&& (action == STEEPCLIMB || action == STEEPCLIMBATTACK) //dont think this is necessary, because
										//sometimes dash attack etc can make it to this position
										&& action != GRAVREVERSE //never transition to ground from gravreverse state
										&& ( ground->IsSteepGround() || HoldingRelativeUp() )
										&& ((ground->Normal().x > 0 && groundSpeed < 0) || (ground->Normal().x < 0 && groundSpeed > 0))
										/*&& (IsOptionOn(POWER_GRAV) || touchedGrass[Grass::GRAVREVERSE])
										&& !touchedGrass[Grass::ANTIGRAVREVERSE]
										&& (((DashButtonHeld() && currInput.LUp()) || touchedGrass[Grass::GRAVREVERSE]) || (IsOptionOn(POWER_GRIND) && GrindButtonHeld()))*/
										&& !minContact.edge->IsInvisibleWall())
									{
										collision = true;
										velocity = groundSpeed * ground->Along();

										prevRail = NULL;

										if (b.rh < normalHeight)
										{
											b.offset.y = (normalHeight - b.rh);
										}

										/*if (minContact.edge->Normal().y <= 0)
										{
											if (minContact.position == minContact.edge->v0)
											{
												if (minContact.edge->edge0->Normal().y >= 0)
												{
													minContact.edge = minContact.edge->edge0;
												}
											}
										}*/

										RestoreAirOptions();
										reversed = false;

										ground = minContact.edge;

										q = edgeQuantity;
										edgeQuantity = minContact.edge->GetQuantity(minContact.position);

										double groundLength = length(ground->v1 - ground->v0);

										groundSpeed = 0;

										V2d gno = ground->Normal();


										double angle = atan2(gno.x, -gno.y);

										if (-gno.y > -steepThresh)
										{
											groundSpeed = -dot(velocity, normalize(ground->v1 - ground->v0));
										}
										else
										{
											groundSpeed = -dot(velocity, normalize(ground->v1 - ground->v0));
										}


										if (gno.x > 0)
										{
											offsetX = b.rw;
										}
										else if (gno.x < 0)
										{
											offsetX = -b.rw;
										}
										else
										{
											offsetX = 0;
										}

										movement = 0;
										//break;

										//offsetX = (position.x + b.offset.x) - minContact.position.x;

										/*if (gno.x < 0)
										{
										facingRight = true;
										SetAction(STEEPSLIDE);
										}
										else if (gno.x > 0)
										{
										facingRight = false;
										SetAction(STEEPSLIDE);
										}*/

									}
									else
									{
										q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);

										groundSpeed = 0;
										edgeQuantity = q;
										offsetX = -offsetX;

										ProcessGroundedCollision();
										//break;
									}
									break;

									
								}

								/*if (bounceFlameOn && abs(groundSpeed) > 1)
								{
									storedBounceGroundSpeed = groundSpeed * slowMultiple;
									groundedWallBounce = true;
								}
								q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);
								groundSpeed = 0;
								offsetX = -offsetX;
								edgeQuantity = q;*/
								break;
							}
						}
					}
					else
					{
						if (movement > 0 && extra > 0 && gNormal.x < 0 && gNormal.y > -steepThresh && e1n.x >= 0 )
								//|| ( gNormal.y <= -steepThresh && e1n.x >= 0 && e1n.y > -steepThresh ) ) )
						{
							reversed = false;

							V2d along = ground->Along();

							velocity = along * -groundSpeed;
							movementVec = along * extra;

							movementVec.y += .1;
							if (movementVec.x <= .1)
							{
								movementVec.x = .1;
							}

							//cout << "airborne 2" << endl;
							leftGround = true;
							SetAction(JUMP);
							frame = 1;

							ground = NULL;

							//cout << "jumpoff c: " << along.x << ", " << along.y << endl;

							leftGroundExtra = movementVec;
							return leftGroundExtra;
						}
						else if (movement < 0 && extra < 0 && gNormal.x > 0 && gNormal.y > -steepThresh && e0n.x <= 0 )
							//	|| ( gNormal.y <= -steepThresh && e0n.x <= 0 && e0n.y > -steepThresh ) ) )
						{
							reversed = false;

							V2d along = ground->Along();

							velocity = along * -groundSpeed;
							movementVec = along * extra;

							movementVec.y += .1;
							if (movementVec.x >= -.1)
							{
								movementVec.x = -.1;
							}

							//cout << "airborne 2" << endl;
							leftGround = true;
							SetAction(JUMP);
							frame = 1;

							ground = NULL;

							//cout << "jumpoff d: " << along.x << ", " << along.y << endl;

							leftGroundExtra = movementVec;
							return leftGroundExtra;
						}
						else
						{
							V2d wVel = position - oldPos;
							edgeQuantity = q;
						}
						//V2d wVel = position - oldPos;
						//edgeQuantity = q;
						
						
					}
					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
					
				}
			
			}

			offsetX = -offsetX;

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;	
		}
	}

	/*if (ground == NULL)
	{
		cout << "check if this is a bug" << endl;
	}*/

	return leftGroundExtra;
}

void Actor::CheckGates()
{
	minContact.collisionPriority = 10000;
	minContact.edge = NULL;
	minContact.resolution = V2d(0, 0);
	col = false;
	queryType = Q_CHECK_GATE;
	Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);

	GetTerrainTree()->Query(this, r);
}

bool Actor::ExitGrind(bool jump)
{
	V2d op = position;

	V2d grindNorm = grindEdge->Normal();

	if (grindNorm.y < 0)
	{
		//removed extra because we actually do wanna check for you being on the ground. only the one where it checks
		//you being in the air needs the extra offset to move it away from any edges. When ground is set it works.
		double extra = 0;
		if (grindNorm.x > 0)
		{
			offsetX = b.rw;
			//extra = .1;
		}
		else if (grindNorm.x < 0)
		{
			offsetX = -b.rw;
			//extra = -.1;
		}
		else
		{
			offsetX = 0;
		}

		position.x += offsetX;// +extra;

		position.y -= normalHeight;// +.1;

		if (!CheckStandUp())
		{
			if (!CheckStandUpToDash())
			{
				position = op;
				return false;
			}
			else
			{
				b.rh = dashHeight;
				b.offset.y = (normalHeight - dashHeight);
				if (reversed)
					b.offset.y = -b.offset.y;

				if (!jump)
				{
					if (grindSpeed > 0)
					{
						facingRight = true;
					}
					else
					{
						facingRight = false;
					}

					framesNotGrinding = 0;
					RestoreAirOptions();

					ground = grindEdge;
					SetAction(DASH);

					frame = 0;
					groundSpeed = grindSpeed;

					if (currInput.LRight())
					{
						facingRight = true;
						if (groundSpeed < 0)
						{
							groundSpeed = 0;
						}
					}
					else if (currInput.LLeft())
					{
						facingRight = false;
						if (groundSpeed > 0)
						{
							groundSpeed = 0;
						}
					}
				}
				else
				{
					ground = grindEdge;
					groundSpeed = grindSpeed;
					SetAction(JUMPSQUAT);
					frame = 0;
				}

				grindEdge = NULL;
				reversed = false;

				CheckGates();
			}
		}
		else
		{
			if (grindSpeed > 0)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}

			framesNotGrinding = 0;
			RestoreAirOptions();

			if (!jump)
			{
				ground = grindEdge;
				//SetAction(LAND);
				SetAction(LAND2);
				//in the grindball file this is LAND2
				frame = 0;
				groundSpeed = grindSpeed;

				if (currInput.LRight())
				{
					facingRight = true;
					if (groundSpeed < 0)
					{
						groundSpeed = 0;
					}
				}
				else if (currInput.LLeft())
				{
					facingRight = false;
					if (groundSpeed > 0)
					{
						groundSpeed = 0;
					}
				}
			}
			else
			{
				ground = grindEdge;
				groundSpeed = grindSpeed;
				SetAction(JUMPSQUAT);
				frame = 0;
			}


			grindEdge = NULL;
			reversed = false;

			CheckGates();
		}

	}
	else if (grindNorm.y > 0)
	{
		if (grindNorm.x > 0)
		{
			position.x += b.rw;// +.1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw;// -.1;
		}

		//if (grindNorm.y > 0)
		position.y += normalHeight;// +.1;

		bool oldReversed = reversed;
		reversed = true;

		if (!CheckStandUp())
		{
			if (!CheckStandUpToDash())
			{
				reversed = oldReversed;
				position = op;
				return false;
			}
			else
			{
				reversed = oldReversed;

				framesNotGrinding = 0;

				if (!IsOptionOn(POWER_GRAV) || jump)
				{
					if (grindNorm.x > 0)
					{
						offsetX = b.rw;
					}
					else if (grindNorm.x < 0)
					{
						offsetX = -b.rw;
					}
					else
					{
						offsetX = 0;
					}

					if (grindSpeed < 0)
					{
						facingRight = true;
					}
					else
					{
						facingRight = false;
					}

					RestoreAirOptions();

					ground = grindEdge;
					groundSpeed = -grindSpeed;
					reversed = true;

					b.rh = dashHeight;
					b.offset.y = (normalHeight - dashHeight);
					if (reversed)
						b.offset.y = -b.offset.y;

					grindEdge = NULL;

					SetAction(JUMPSQUAT);
					framesNotGrinding = 0;
					frame = 0;
				}
				else if (abs(grindNorm.x) >= wallThresh || grindEdge->IsInvisibleWall())
				{
					reversed = oldReversed;
					position = op;
					return false;
				}
				else
				{
					if (grindNorm.x > 0)
					{
						offsetX = b.rw;
					}
					else if (grindNorm.x < 0)
					{
						offsetX = -b.rw;
					}
					else
					{
						offsetX = 0;
					}

					if (grindSpeed < 0)
					{
						facingRight = true;
					}
					else
					{
						facingRight = false;
					}

					RestoreAirOptions();

					ground = grindEdge;
					groundSpeed = -grindSpeed;
					reversed = true;

					b.rh = dashHeight;
					b.offset.y = (normalHeight - dashHeight);
					if (reversed)
						b.offset.y = -b.offset.y;

					grindEdge = NULL;

					SetAction(DASH);
					framesNotGrinding = 0;
					frame = 0;


					double angle = GroundedAngle() / PI * 180.0;

					ActivateEffect(PLAYERFX_GRAV_REVERSE, Vector2f(position), angle, 25, 1, facingRight);
					ActivateSound(PlayerSounds::S_GRAVREVERSE);
				}

				CheckGates();
			}
		}
		else
		{
			reversed = oldReversed;
			if (!IsOptionOn( POWER_GRAV ) || (abs(grindNorm.x) >= wallThresh) || jump || grindEdge->IsInvisibleWall())
			{
				if (grindSpeed < 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}


				framesNotGrinding = 0;

				velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;

				SetAction(JUMP);
				frame = 1;
				ground = NULL;
				grindEdge = NULL;
				reversed = false;
			}
			else
			{
				if (grindNorm.x > 0)
				{
					offsetX = b.rw;
				}
				else if (grindNorm.x < 0)
				{
					offsetX = -b.rw;
				}
				else
				{
					offsetX = 0;
				}

				if (grindSpeed < 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}

				RestoreAirOptions();

				ground = grindEdge;
				groundSpeed = -grindSpeed;
				reversed = true;
				grindEdge = NULL;

				SetAction(LAND2);
				framesNotGrinding = 0;
				frame = 0;


				double angle = GroundedAngle() / PI * 180.0;

				ActivateEffect(PLAYERFX_GRAV_REVERSE, Vector2f(position), angle, 25, 1, facingRight);
				ActivateSound(PlayerSounds::S_GRAVREVERSE);
			}

			CheckGates();
		}
	}
	else
	{
		//this was the old code
		//SetAction(DOUBLE);
		//frame = 0;

		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}

		if (CheckStandUp())
		{
			framesInAir = 0;
			SetAction(JUMP);
			frame = 1;
			grindEdge = NULL;
			ground = NULL;

			CheckGates();
			//TODO: this might glitch grind areas? test it with the range of your get out of grind query

		}
		else
		{
			position = op;
			return false;
		}
	}

	return true;
}

bool Actor::TrySlideBrakeOrStand()
{
	if (!(currInput.LLeft() || currInput.LRight()))//&& t )
	{
		if (currInput.LDown() || currInput.LUp())
		{
			SetAction(SLIDE);
		}
		else
		{
			double absGroundSpeed = abs(groundSpeed);
			double dSpeed = 22;//GetDashSpeed();
			if (( action == BRAKE && absGroundSpeed < 2) ||
				( action != STAND && action != BRAKE && absGroundSpeed < dSpeed ) )
			{
				if (action == LAND)
				{
					return false;
				}
				else
				{
					SetAction(STAND);
					frame = 0;
				}
			}
			else if( action != BRAKE && action != STAND )
			{
				SetAction(BRAKE);
				frame = 0;
			}
		}
		return true;
	}
	return false;
}

bool Actor::BasicSteepAction()
{
	V2d norm = GetGroundedNormal();
	if (reversed)
	{
		if (-norm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{
			if ((groundSpeed > 0 && norm.x < 0) || (groundSpeed < 0 && norm.x > 0))
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				return true;
			}
			else
			{
				if (norm.x < 0)
					facingRight = false;
				else
					facingRight = true;
				SetAction(STEEPSLIDE);
				frame = 0;
				return true;
			}
		}
	}
	else
	{
		if (norm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{
			if ((groundSpeed > 0 && norm.x < 0) || (groundSpeed < 0 && norm.x > 0))
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				return true;
			}
			else
			{
				if (norm.x > 0)
					facingRight = true;
				else
					facingRight = false;

				SetAction(STEEPSLIDE);
				frame = 0;
				return true;
			}
		}
	}

	return false;
}

void Actor::SetSprintStartFrame()
{
	if (action == RUN)
	{
		frame = frame / 4;

		if (frame < 3)
		{
			frame = frame + 1;
		}
		else if (frame == 8)
		{
			frame = 7;
		}

		else if (frame == 9)
		{
			frame = 0;
		}
		frame = frame * 4;
	}	
	else
	{
		frame = 0;
	}
}

bool Actor::TrySprintOrRun()
{
	bool isLandingAction = action == LAND || action == LAND2;
	V2d norm = GetGroundedNormal();
	if (currInput.LLeft())
	{
		facingRight = false;
		if ((HoldingRelativeDown() && norm.x < 0) || (HoldingRelativeUp() && norm.x > 0))
		{
			if (action != SPRINT)
			{
				SetAction(SPRINT);
				SetSprintStartFrame();
			}
		}
		else
		{
			if (action != RUN)
			{
				SetAction(RUN);
				frame = 0;
			}
		}

		if (groundSpeed > 0)
		{
			groundSpeed = 0;
		}
		return true;
	}
	else if (currInput.LRight())
	{
		facingRight = true;
		if ((HoldingRelativeDown() && norm.x > 0) || (HoldingRelativeUp() && norm.x < 0))
		{
			if (action != SPRINT)
			{
				SetAction(SPRINT);
				SetSprintStartFrame();
			}
		}
		else
		{
			if (action != RUN)
			{
				SetAction(RUN);
				frame = 0;
			}
		}

		if (groundSpeed < 0)
		{
			groundSpeed = 0;
		}

		return true;
	}

	return false;
}

double Actor::GetOriginalDashSpeed()
{
	double dSpeed;
	switch (speedLevel)
	{
	case 0:
		dSpeed = dashSpeed0;
		break;
	case 1:
		dSpeed = dashSpeed1;
		break;
	case 2:
	{
		double sbp = GetSpeedBarPart();

		if (sbp > .8)
			sbp = 1.0;
		dSpeed = dashSpeed2 + 4.0 * sbp;
		break;
	}

	}

	return dSpeed;
}

double Actor::GetDashSpeed()
{
	double dSpeed = GetOriginalDashSpeed();

	int numBaseDashUpgrades = IsOptionOn(UPGRADE_W1_BASE_DASH_1) + IsOptionOn(UPGRADE_W3_BASE_DASH_2) + IsOptionOn(UPGRADE_W6_BASE_DASH_3);
	double upgradeAmount = 3;
	dSpeed += upgradeAmount * numBaseDashUpgrades;

	return dSpeed;
}

double Actor::GetAirDashSpeed()
{
	//return 45;
	/*switch (speedLevel)
	{
	case 0:
		return airDashSpeed0;
		break;
	case 1:
		return airDashSpeed1;
		break;
	case 2:
		return airDashSpeed2;
		break;
	}*/

	double dSpeed = GetOriginalDashSpeed();

	int numBaseAirdashUpgrades = IsOptionOn(UPGRADE_W2_BASE_AIRDASH_1) + IsOptionOn(UPGRADE_W5_BASE_AIRDASH_2) + IsOptionOn(UPGRADE_W6_BASE_AIRDASH_3);
	double upgradeAmount = 3;
	dSpeed += upgradeAmount * numBaseAirdashUpgrades;

	return dSpeed;

}

bool Actor::TryJumpSquat()
{
	if (JumpButtonPressed())
	{
		SetAction(JUMPSQUAT);
		frame = 0;
		return true;
	}

	return false;
}

bool Actor::TryDash()
{
	bool landAction = action == LAND || action == LAND2;
	if (DashButtonHeld() && (!prevInput.DashButtonDown() || landAction ))
	{
		SetAction(DASH);
		frame = 0;
		return true;
	}
	
	return false;
}

bool Actor::TryFloorRailDropThrough()
{
	if (currInput.LDown() && !currInput.LLeft() && !currInput.LRight())
	{
		if (ground != NULL && ground->rail != NULL
			&& ground->rail->GetRailType() == TerrainRail::FLOOR)
		{
			SetAction(JUMP);
			frame = 1;
			velocity = ground->Along() * groundSpeed;
			ground = NULL;
			framesInAir = 0;
			return true;
		}
	}

	return false;
}

bool Actor::TryScorpRailDropThrough()
{
	//this almost works. try to make a fallthrough algorithm for normal rails
	//and see if it helps this.
	//if (bounceEdge == NULL || !scorpOn)
	//{
	//	if (ground != NULL && ground->rail != NULL
	//		&& ground->rail->GetRailType() == TerrainRail::SCORPIONONLY)
	//	{
	//		SetAction(JUMP);
	//		frame = 1;
	//		velocity = ground->Along() * groundSpeed;
	//		ground = NULL;
	//		framesInAir = 0;
	//		scorpOn = false; //turns off scorp for one frame to allow dropthrough.
	//						 //this is a bit hacky, and i probably need to fix it more later
	//		return true;
	//	}
	//}

	return false;
}

bool Actor::TryGroundBlock()
{
	if (PowerButtonHeld() && currPowerMode == PMODE_SHIELD)
	{
		SetGroundBlockAction();
		return true;
	}

	return false;
}

bool Actor::TryAirBlock()
{
	if (PowerButtonHeld() && currPowerMode == PMODE_SHIELD)
	{
		SetAirBlockAction();
		return true;
	}

	return false;
}

void Actor::TryChangePowerMode()
{
	bool noHoriz = !currInput.RLeft() && !currInput.RRight();
	bool noVert = !currInput.RUp() && !currInput.RDown();

	bool hasTimeSlow = IsOptionOn(POWER_TIME);
	bool hasGrind = IsOptionOn(POWER_GRIND);
	bool hasBounce = IsOptionOn(POWER_BOUNCE);

	int oldPowerMode = currPowerMode;

	if ((currPowerMode == PMODE_TIMESLOW && !hasTimeSlow )
		|| (currPowerMode == PMODE_GRIND && !hasGrind )
		|| (currPowerMode == PMODE_BOUNCE && !hasBounce))
	{
		currPowerMode = PMODE_SHIELD;
	}

	if (hasBounce && currInput.RLeft())
	{
		currPowerMode = PMODE_BOUNCE;
	}
	else if (hasGrind && currInput.RRight())
	{
		currPowerMode = PMODE_GRIND;
	}
	else if ( hasTimeSlow && currInput.RDown() )
	{
		currPowerMode = PMODE_TIMESLOW;
	}
	else if (currInput.RUp())
	{
		currPowerMode = PMODE_SHIELD;
	}
	
	


	/*if (currInput.RUp() && noHoriz)
	{
		currPowerMode = PMODE_SHIELD;
	}
	else if (currInput.RDown() && noHoriz && hasTimeSlow)
	{
		currPowerMode = PMODE_TIMESLOW;
	}
	else if (currInput.RRight() && noVert && hasGrind)
	{
		currPowerMode = PMODE_GRIND;
	}
	else if (currInput.RLeft() && noVert && hasBounce )
	{
		currPowerMode = PMODE_BOUNCE;
	}*/

	if (oldPowerMode == PMODE_BOUNCE && currPowerMode != PMODE_BOUNCE)
	{
		BounceFlameOff();
	}
}

bool Actor::CheckSetToAerialFromNormalWater()
{
	if (InWater(TerrainPolygon::WATER_NORMAL))// && velocity.y > normalWaterMaxFallSpeed)//make this into a cleaner function very soon.
	{
		bool overVelLimit = false;
		if (reversed)
		{
			if (velocity.y < -normalWaterMaxFallSpeed)
			{
				velocity.y = -normalWaterMaxFallSpeed;
				overVelLimit = true;
			}
		}
		else
		{
			if (velocity.y > normalWaterMaxFallSpeed)
			{
				velocity.y = normalWaterMaxFallSpeed;
				overVelLimit = true;
			}
		}

		if (overVelLimit)
		{
			SetAction(JUMP);
			frame = 1;
			RestoreAirOptions();
			ground = NULL;
			wallNormal = V2d(0, 0);
			currWall = NULL;
			bounceEdge = NULL;
			grindEdge = NULL;
			reversed = false;
			//velocity.y = normalWaterMaxFallSpeed;//min(normalWaterMaxFallSpeed, velocity.y);
			return true;
		}
		
	}

	return false;
}

bool Actor::BasicGroundAction()
{
	CheckBounceFlame();

	if (CheckSetToAerialFromNormalWater())
	{
		return true;
	}

	
	if (currInspectObject != NULL && currInspectObject->IsShowingIcon())
	{
		if (JumpButtonPressed())
		{
			SetAction(INSPECT_START);
			frame = 0;
			return true;
		}
	}
	
	//button-based inputs

	if (TryGroundBlock()) return true;

	if (TryFloorRailDropThrough()) return true;

	if (TryPressGrind()) return true;

	if (TryJumpSquat()) return true;

	if (TryGroundAttack()) return true;
	
	if (TryDash()) return true;

	//control only

	if (BasicSteepAction()) return true;

	if (TrySprintOrRun()) return true;

	if (TrySlideBrakeOrStand()) return true;


	assert(0);
	return false;
}

bool Actor::BasicAirAction()
{
	if( TryAirBlock() ) return true;

	CheckBounceFlame();

	if (TryDoubleJump()) return true;

	if (TryAirDash()) return true;

	TryExtraAirdashBoost();
	//if (TryGlide()) return true;

	if (TryWallJump()) return true;

	if( AirAttack()) return true;

	if (SpecialButtonPressed() && sess->HasLevelFinisher() )
	{
		SetAction(WARP_CHARGE); 
		return true;
	}

	return false;
}

bool Actor::GlideAction()
{
	CheckBounceFlame();

	if (TryDoubleJump()) return true;

	if (TryAirDash()) return true;

	//if (TryGlide()) return true;

	if (TryWallJump()) return true;

	if (AirAttack())
	{
		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}
		return true;
	}

	return false;
}

bool Actor::HomingAction()
{
	//CheckBounceFlame();

	if (TryDoubleJump()) return true;

	if (TryAirDash()) return true;

	//if (TryGlide()) return true;

	if (TryWallJump()) return true;

	/*if (AirAttack())
	{
		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}
		return true;
	}*/

	return false;
}

bool Actor::BasicAirAttackAction()
{
	CheckBounceFlame();

	if (CanCancelAttack())
	{
		if (TryAirDash()) return true;

		if (TryDoubleJump()) return true;

		if (AirAttack()) return true;

		if (TryAirBlock()) return true;
	}
	
	return false;
}

bool Actor::IsHomingAttackAction(int a)
{
	return a == HOMINGATTACK || a == SPRINGSTUNHOMINGATTACK;
}

void Actor::CheckBounceFlame()
{
	//currInput.X
	/*if ( HasUpgrade( POWER_BOUNCE ) )
	{
		if (currPowerMode == PMODE_BOUNCE 
			&& PowerButtonHeld() && !bounceFlameOn && !justToggledBounce)
		{
			BounceFlameOn();
			oldBounceEdge = NULL;
		}
		else if (!PowerButtonHeld() && bounceFlameOn)
		{
			BounceFlameOff();
		}
	}*/
}

bool Actor::TryWallJump()
{
	if (touchedGrass[Grass::BOUNCE])
	{
		return false;
	}
	if (bounceFlameOn)
	{
		return false;
	}

	bool wj = false;
	if (CheckWall(false))
	{
		if (!currInput.LDown() && currInput.LRight() && !prevInput.LRight())
		{
			SetAction(WALLJUMP);
			frame = 0;
			facingRight = true;
			wj = true;
		}
	}

	if (CheckWall(true))
	{
		if (!currInput.LDown() && currInput.LLeft() && !prevInput.LLeft())
		{
			SetAction(WALLJUMP);
			frame = 0;
			facingRight = false;
			wj = true;
		}
	}

	if (wj)
	{
		
	}
	

	return false;
}

void Actor::TryDashBoost()
{
	if (!IsOptionOn(UPGRADE_W1_DASH_BOOST))
	{
		return;
	}

	if (currBBoostCounter >= 20)
	{
		bool fr = facingRight;
		if (reversed)
		{
			fr = !facingRight;
		}

		EffectInstance params;
		Transform t;
		if (!fr)
		{
		//	t.scale(Vector2f(-1, 1));
		}

		//must scale after rotating
		t.rotate(GroundedAngle() / PI * 180.0);

		if (!fr)
		{
			t.scale(-1, 4);
		}
		else
		{
			t.scale(1, 4);
		}

		params.SetParams(Vector2f(position), t, 5, 4, 1);

		ActivateEffect(PLAYERFX_DASH_BOOST, &params);
		//ActivateEts_fx_dashBoostFX

		double dashFactor = 1.85;//1.5;
		double bboostSpeed = GetDashSpeed() * dashFactor;

		if (bboostSpeed > abs(groundSpeed))
		{
			if (groundSpeed > 0)
			{
				groundSpeed = bboostSpeed;
			}
			else
			{
				groundSpeed = -bboostSpeed;
			}
		}
		else
		{
			//passively has 2 high speed boost upgrades on. test to make sure its not crazy
			//double highSpeedBoost = 3;//5

			//int highSpeedBoostUpgrades = 2;
			//double upgradeAmount = 4;//2;

			double highSpeedBoost = 5;//5

			//highSpeedBoost += upgradeAmount;//highSpeedBoostUpgrades * upgradeAmount;

			if (groundSpeed > 0)
			{
				groundSpeed += highSpeedBoost;
			}
			else
			{
				groundSpeed -= highSpeedBoost;
			}
		}

		V2d along = ground->Along();

		V2d trueVel = along * groundSpeed;

		V2d rumbleDir = normalize(trueVel);
		//sess->cam.SetRumble(round(6 * rumbleDir.x), round(6 * rumbleDir.y), 6);

		if (!simulationMode)
		{
			sess->cam.SetRumble(round(4 * rumbleDir.x), round(4 * rumbleDir.y), 6, 0, true);
		}
		
		//sess->cam.SetRumble(6, 6, 6);

		/*double dashFactor = 3.0;
		double ag = abs(groundSpeed);

		if (ag > 30)
		{
		dashFactor = 3.0;
		}
		else
		{
		dashFactor = 2.0;
		}*/

		//double bboost = GetDashSpeed() / dashFactor;

		/*if (groundSpeed > 0)
		{
		groundSpeed += bboost;
		}
		else
		{
		groundSpeed -= bboost;
		}*/
		//currBBoostCounter = 0;
	}
}

void Actor::TryAirdashBoost()
{
	if (!IsOptionOn(UPGRADE_W2_AIRDASH_BOOST))
	{
		return;
	}

	if (DashButtonHeld() &&
		(currInput.LLeft() || currInput.LUp()
			|| currInput.LRight() || currInput.LDown()))
	{
		ActivateAirdashBoost();
	}
}

void Actor::TryExtraAirdashBoost()
{
	//if (!IsOptionOn(UPGRADE_W7_DOUBLE_AIRDASH_BOOST))
	{
		return;
	}

	V2d dir = currInput.GetLeft8Dir();

	if (dir.x == 0 && dir.y == 0)
	{
		return;
	}

	if (DashButtonPressed() && action != AIRDASH && !hasAirDash && numRemainingExtraAirdashBoosts > 0)
	{
		numRemainingExtraAirdashBoosts--;

		double aSpeed = GetAirDashSpeed();
		

		if (dir.x > 0)
		{
			velocity.x = max(velocity.x, aSpeed);
		}
		else if (dir.x < 0)
		{
			velocity.x = min(velocity.x, -aSpeed);
		}
		else
		{
			velocity.x = 0;
		}
		if (dir.y > 0)
		{
			velocity.y = max(velocity.y, aSpeed);
		}
		else if (dir.y < 0)
		{
			velocity.y = min(velocity.y, -aSpeed);
		}
		else
		{
			velocity.y = 0;
		}

		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}
		ActivateAirdashBoost();
	}
}

void Actor::ActivateAirdashBoost()
{
	double dashFactor = 1.85;//1.5;
	double bboostSpeed = GetAirDashSpeed() * dashFactor;
	double velLen = length(velocity);
	double highSpeedBoost = 5;
	V2d velDir = normalize(velocity);
	if (velLen < bboostSpeed)
	{
		if (velDir.y < 0)
		{
			velocity = V2d(velDir.x * bboostSpeed, velDir.y * bboostSpeed * 1.5);
		}
		else if (velDir.y > 0)
		{
			velocity = V2d(velDir.x * bboostSpeed, velDir.y * bboostSpeed);
		}
		else
		{
			velocity = velDir * bboostSpeed;
		}

	}
	else
	{
		velocity = velDir * (velLen + highSpeedBoost);
	}

	double ang = GetVectorAngleCW(velDir);// / PI * 180.0;


	EffectInstance params;
	Transform t;

	//apparently you must rotate before scaling to get accurate results!
	t.rotate(ang / PI * 180.0);
	t.scale(1, 4);
	params.SetParams(Vector2f(position), t, 5, 4, 1);

	ActivateEffect(PLAYERFX_DASH_BOOST, &params);

	V2d rumbleDir = normalize(velocity);
	//sess->cam.SetRumble(round(6 * rumbleDir.x), round(6 * rumbleDir.y), 6, 0, true );
	if (!simulationMode)
	{
		sess->cam.SetRumble(round(4 * rumbleDir.x), round(4 * rumbleDir.y), 6, 0, true);
	}
	
	//sess->cam.SetRumble(6, 6, 6 );
}

void Actor::TryCheckGrass()
{
	//this was previously in ResolvePhysics and used the r rect from
	//there. now im only checking once per frame so
	//lets hope nothing feels weird.
	if ((owner != NULL && owner->hasAnyGrass) || editOwner != NULL)
	{
		memcpy(oldTouchedGrass, touchedGrass, sizeof(bool) * Grass::Count);
		memset(touchedGrass, 0, sizeof(bool) * Grass::Count);

		Rect<double> grassR(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
		//grassR is the same as the rect used in HandleEntrant for "grass"
		//might need to make a function at some point
		queryType = Q_GRASS;
		sess->grassTree->Query(this, grassR);
	}
}

bool Actor::EnemyIsFar(V2d &enemyPos)
{
	double len = length(position - enemyPos);
	bool isFar;
	isFar = (len > MAX_VELOCITY * 2);

	if (!isFar)
		return false;
	else
	{
		ComboObject *curr = activeComboObjList;
		while (curr != NULL)
		{
			len = length(curr->GetComboPos() - enemyPos); //doesnt account for origin of shape
			isFar = (len > MAX_VELOCITY * 2);
			if (!isFar)
				return false;
			curr = sess->GetComboObjectFromID(curr->data.nextComboObjectID);
		}
	}

	return true;
}

ComboObject * Actor::IntersectMyComboHitboxes(Enemy *e, CollisionBody *cb,
	int cbFrame)
{
	if (cb == NULL || activeComboObjList == NULL)
		return NULL;

	ComboObject *curr = activeComboObjList;
	while (curr != NULL)
	{
		if (e != curr->enemy)
		{
			if ( curr->enemy->CanComboHit( e ) && curr->enemyHitBody.Intersects(curr->data.enemyHitboxFrame, cb, cbFrame))
			{
				return curr;
			}
		}
		curr = sess->GetComboObjectFromID(curr->data.nextComboObjectID);
	}

	return NULL;
}

Wire * Actor::IntersectMyWireHitboxes(Enemy *e, CollisionBody *cb,
	int cbFrame)
{
	if (cb == NULL)
		return NULL;

	bool canHitRight = e->CanBeHitByWireTip(true);
	bool canHitLeft = e->CanBeHitByWireTip(false);

	CollisionBox *rightBox = NULL;
	CollisionBox *leftBox = NULL;
	if (rightWire != NULL && canHitRight )
	{
		rightBox = rightWire->GetTipHitbox();
	}
	if (leftWire != NULL && canHitLeft)
	{
		leftBox = leftWire->GetTipHitbox();
	}

	if (rightBox != NULL)
	{
		if (cb->Intersects(cbFrame, rightBox))
		{
			return rightWire;
		}
	}
	if (leftBox != NULL)
	{
		if (cb->Intersects(cbFrame, leftBox))
		{
			return leftWire;
		}
	}

	return false;
}

bool Actor::IntersectMyHurtboxes(CollisionBox &cb)
{
	/*if (IsInvincible()) //handled by invincible hit, not just missing
		return false;*/

	return cb.Intersects(hurtBody);
}

bool Actor::IntersectMyHurtboxes(CollisionBody *cb, int cbFrame )
{
	if (cb == NULL)
		return false;

	return cb->Intersects( cbFrame, &hurtBody); 
	//if ( cb == NULL || currHurtboxes == NULL)
	//	return false;

	//return currHurtboxes->Intersects(currHurtboxFrame, cb, cbFrame);
}

bool Actor::IntersectMyHitboxes(CollisionBody *cb,
	int cbFrame)
{
	if (cb == NULL || currHitboxes == NULL )
		return false;

	return currHitboxes->Intersects(currHitboxFrame, cb, cbFrame);
}

bool Actor::IntersectMySlowboxes(CollisionBody *cb, int cbFrame )
{
	if (globalTimeSlowFrames > 0)
	{
		return true;
	}

	if (cb == NULL )
		return false;

	CollisionBody *bubbleBody;
	for (int i = 0; i < MAX_BUBBLES; ++i)
	{
		if (bubbleFramesToLive[i] > 0)
		{
			bubbleBody = GetBubbleHitbox(i);
			if (bubbleBody != NULL)
			{
				if (bubbleBody->Intersects(0, cb, cbFrame))
				{
					return true;
				}
			}
		}
	}
	return false;
}

int Actor::GetMaxBubbles()
{
	int numBubbles = 1 + IsOptionOn(UPGRADE_W6_EXTRA_BUBBLES_1) * 2 + IsOptionOn(UPGRADE_W6_EXTRA_BUBBLES_2) * 2;
	return numBubbles;
}



int Actor::GetBubbleRadius()
{
	bool hasBubbleSizeUpgrade = IsOptionOn(UPGRADE_W6_BUBBLE_SIZE);
	int upgradeFactor = 30;//15;

	int currRad = bubbleRadius;
	if (hasBubbleSizeUpgrade)
	{
		currRad += upgradeFactor;
	}
	
	return currRad;
}

int Actor::GetBubbleTimeFactor()
{
	return baseTimeSlowedMultiple;
}

int Actor::GetBeingSlowedFactor()
{
	bool hasBeingSlowedUpgrade = IsOptionOn(UPGRADE_W5_SLOW_RESISTANCE);
	int mult = baseTimeSlowedMultiple;

	if (hasBeingSlowedUpgrade)
	{
		mult -= 2;
	}
	return mult;
}

double Actor::GetFullSprintAccel( bool downSlope, sf::Vector2<double> &gNorm )
{
	double extraSprintAccel;

	extraSprintAccel = abs(dot( gNorm, V2d( 1, 0 ) ));


	if( downSlope )
	{
	}
	else
	{
		extraSprintAccel = min( .3, extraSprintAccel );
	}
	extraSprintAccel *= .09;
	
	double upgradeSprintAmount = .03;

	if (reversed)
	{
		int numCeilingSprintUpgrades = IsOptionOn(UPGRADE_W3_CEILING_SPRINT_1) + IsOptionOn(UPGRADE_W4_CEILING_SPRINT_2) + IsOptionOn(UPGRADE_W5_CEILING_SPRINT_3);
		extraSprintAccel += extraSprintAccel * numCeilingSprintUpgrades;
	}
	else
	{
		int numSprintUpgrades = IsOptionOn(UPGRADE_W1_SPRINT_1) + IsOptionOn(UPGRADE_W2_SPRINT_2) + IsOptionOn(UPGRADE_W6_SPRINT_3);
		extraSprintAccel += upgradeSprintAmount * numSprintUpgrades;
	}

	return sprintAccel + extraSprintAccel;
}

double Actor::GetMinRailGrindSpeed()
{
	return minRailGrindSpeed[speedLevel];
}

//eventually need to change resolve physics so that the player can't miss going by enemies. i understand the need now
//for universal substeps. guess box2d makes more sense now doesn't it XD

void Actor::SprintAccel()
{
	V2d trueNormal = GetGroundedNormal();
	if (HoldingRelativeDown() && ((facingRight && trueNormal.x > 0) || (!facingRight && trueNormal.x < 0)))
	{
		//cout << "sprint down" << endl;
		double sprAccel = GetFullSprintAccel(true, trueNormal);// / sprFactor;
		if (facingRight)
		{
			groundSpeed += sprAccel / slowMultiple;
		}
		else
		{
			groundSpeed -= sprAccel / slowMultiple;
		}
	}
	else if (HoldingRelativeUp() && ((facingRight && trueNormal.x < 0) || (!facingRight && trueNormal.x > 0)))
	{
		//cout << "sprint up" << endl;
		double sprAccel = GetFullSprintAccel(false, trueNormal);// / sprFactor;

		if (facingRight)
		{
			groundSpeed += sprAccel / slowMultiple;
		}
		else
		{
			groundSpeed -= sprAccel / slowMultiple;
		}
	}
}

void Actor::GroundExtraAccel()
{
	double extraAccel = 0;
	double upgradeFactor = .03;

	if (reversed)
	{
		int numPassiveCeilingAccelUpgrades = IsOptionOn(UPGRADE_W3_CEILING_PASSIVE_GROUND_1)
			+ IsOptionOn(UPGRADE_W4_CEILING_PASSIVE_GROUND_2)
			+ IsOptionOn(UPGRADE_W5_CEILING_PASSIVE_GROUND_3);
		extraAccel = numPassiveCeilingAccelUpgrades * upgradeFactor;
	}
	else
	{
		int numPassiveAccelUpgrades = IsOptionOn(UPGRADE_W1_PASSIVE_GROUND_1) + IsOptionOn(UPGRADE_W2_PASSIVE_GROUND_2) + IsOptionOn(UPGRADE_W6_PASSIVE_GROUND_3);
		extraAccel = numPassiveAccelUpgrades * upgradeFactor;
	}

	if (groundSpeed > 0)
	{
		groundSpeed += extraAccel;
	}
	else if (groundSpeed < 0)
	{
		groundSpeed -= extraAccel;
	}
}


void Actor::UpdateWirePhysics()
{
	/*if (rightWire != NULL)
		rightWire->UpdateChargesPhysics();
	if (leftWire != NULL)
		leftWire->UpdateChargesPhysics();*/

	if (rightWire != NULL)
	{
		rightWire->UpdateEnemyAnchor();
	}

	if (leftWire != NULL)
	{
		leftWire->UpdateEnemyAnchor();
	}
}

void Actor::StopGrind()
{
	V2d op = position;

	V2d grindNorm = grindEdge->Normal();

	if (grindNorm.y < 0)
	{
		double extra = 0;
		if (grindNorm.x > 0)
		{
			offsetX = b.rw;
			extra = .1;
		}
		else if (grindNorm.x < 0)
		{
			offsetX = -b.rw;
			extra = -.1;
		}
		else
		{
			offsetX = 0;
		}

		position.x += offsetX + extra;

		position.y -= normalHeight + .1;

		if (!CheckStandUp())
		{
			position = op;
		}
		else
		{
			if (grindSpeed > 0)
			{
				facingRight = true;
			}
			else
			{
				facingRight = false;
			}

			framesNotGrinding = 0;
			RestoreAirOptions();
			SetAction(JUMP);
			frame = 1;
			grindEdge = NULL;
			reversed = false;
			offsetX = 0;
		}

	}
	else if (grindNorm.y > 0)
	{
		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}

		if (grindNorm.y > 0)
			position.y += normalHeight + .1;

		if (!CheckStandUp())
		{
			position = op;
		}
		else
		{
			if (!IsOptionOn(POWER_GRAV) || (abs(grindNorm.x) >= wallThresh) || grindEdge->IsInvisibleWall())
			{
				if (grindSpeed < 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}


				framesNotGrinding = 0;
				if (reversed)
				{
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * -grindSpeed;
				}
				else
				{
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
				}

				SetAction(JUMP);
				frame = 1;
				ground = NULL;
				grindEdge = NULL;
				reversed = false;
			}
			else
			{
				if (grindNorm.x > 0)
				{
					offsetX = b.rw;
				}
				else if (grindNorm.x < 0)
				{
					offsetX = -b.rw;
				}
				else
				{
					offsetX = 0;
				}

				if (grindSpeed < 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}

				RestoreAirOptions();

				ground = grindEdge;
				groundSpeed = -grindSpeed;
				reversed = true;
				grindEdge = NULL;

				SetAction(LAND2);
				framesNotGrinding = 0;
				frame = 0;


				double angle = GroundedAngle();

				ActivateEffect(PLAYERFX_GRAV_REVERSE, Vector2f(position), RadiansToDegrees(angle), 25, 1, facingRight);
				ActivateSound(PlayerSounds::S_GRAVREVERSE);
			}
		}
	}
	else
	{

		if (grindNorm.x > 0)
		{
			position.x += b.rw + .1;
		}
		else if (grindNorm.x < 0)
		{
			position.x += -b.rw - .1;
		}

		if (CheckStandUp())
		{
			framesInAir = 0;
			SetAction(JUMP);
			frame = 1;
			grindEdge = NULL;
			ground = NULL;

			//TODO: this might glitch grind areas? test it with the range of your get out of grind query

		}
		else
		{
			position = op;
		}
	}
}

void Actor::UpdateGrindPhysics(double movement, bool checkRailAndTerrainTransfers)
{
	Edge *e0 = grindEdge->edge0;
	Edge *e1 = grindEdge->edge1;

	V2d e0n;
	V2d e1n;

	if (e0 != NULL)
	{
		e0n = e0->Normal();
	}
	
	if (e1 != NULL)
	{
		e1n = e1->Normal();
	}

	double q = edgeQuantity;
	double hitBorderSpeed = GetDashSpeed() / 2;


	V2d oldGrindPos = grindEdge->GetPosition(edgeQuantity);
	Rect<double> oldR(oldGrindPos.x + b.offset.x - b.rw, oldGrindPos.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);


	while (!approxEquals(movement, 0))
	{
		//cout << "movement: " << movement << endl;
		double gLen = length(grindEdge->v1 - grindEdge->v0);
		if (movement > 0)
		{
			double extra = q + movement - gLen;
			V2d gPoint = grindEdge->GetPosition(q + movement);
			if (!sess->IsWithinCurrentBounds(gPoint))
			{
				grindSpeed = max(-grindSpeed, -hitBorderSpeed);
				//grindSpeed = -grindSpeed;
				return;
			}

			if (extra > 0)
			{
				movement -= gLen - q;


				V2d v0 = grindEdge->v0;
				V2d v1 = grindEdge->v1;

				//this is possibly inefficient to just loop through but very little math is done so its probably fine
				//number of gates should be somewhat low anyway
				for (auto it = sess->gates.begin(); it != sess->gates.end(); ++it)
				{
					//if( (*it)->temp0next == e1 || (*it)
					bool origUnlocked = false;
					if (!(*it)->IsLocked())
					{
						origUnlocked = true;
						(*it)->SetLocked(true);
					}

					if (grindEdge->edge1 != NULL && grindEdge->edge1->GetGate() == (*it))
					{
						if ((*it)->IsSoft())
						{
							if (CanUnlockGate((*it)))
							{
								//cout << "unlock gate" << endl;
								//UnlockGate((*it));

								SetTouchedGate((*it));

								//e1 = grindEdge->edge1;
							}
						}

						if (origUnlocked)
						{
							(*it)->SetLocked(false);
						}

						break;
					}
					else
					{
						if (origUnlocked)
						{
							(*it)->SetLocked(false);
						}
					}
					
				}
				
				if (e1 != NULL)
				{
					grindEdge = e1;
					if (GameSession::IsWall(grindEdge->Normal()) == -1)
					{
						if (IsOptionOn(POWER_GRAV) || grindEdge->Normal().y < 0)
						{
							RestoreAirOptions();
						}
					}
					q = 0;
				}
				else
				{
					TryStandupOnForcedGrindExit();
					return;
				}
			}
			else
			{
				q += movement;
				movement = 0;
			}
		}
		else if (movement < 0)
		{
			double extra = q + movement;

			V2d gPoint = grindEdge->GetPosition(q + movement);
			if (!sess->IsWithinCurrentBounds(gPoint))
			{
				grindSpeed = min(-grindSpeed, hitBorderSpeed);

				return;
			}

			if (extra < 0)
			{
				movement -= movement - extra;

				V2d v0 = grindEdge->v0;
				sf::Rect<double> r(v0.x - 1, v0.y - 1, 2, 2);

				//this is possibly inefficient to just loop through but very little math is done so its probably fine
				//number of gates should be somewhat low anyway
				for (auto it = sess->gates.begin(); it != sess->gates.end(); ++it)
				{
					//if( (*it)->temp0next == e1 || (*it)
					bool origUnlocked = false;
					if (!(*it)->IsLocked())
					{
						origUnlocked = true;
						(*it)->SetLocked(true);
					}

					if ( grindEdge->edge0 != NULL && grindEdge->edge0->GetGate() == (*it))
					{
						if ((*it)->IsSoft())
						{
							if (CanUnlockGate((*it)))
							{
								//cout << "unlock gate" << endl;
								//UnlockGate((*it));

								SetTouchedGate((*it));
							}
						}

						if (origUnlocked)
						{
							(*it)->SetLocked(false);
						}

						break;
					}
					else
					{
						if (origUnlocked)
						{
							(*it)->SetLocked(false);
						}
					}
				}

				if (e0 != NULL)
				{
					grindEdge = e0;
					q = length(grindEdge->v1 - grindEdge->v0);

					if (GameSession::IsWall(grindEdge->Normal()) == -1)
					{
						if (IsOptionOn(POWER_GRAV) || grindEdge->Normal().y < 0)
						{
							RestoreAirOptions();
						}
					}
				}
				else
				{
					/*bool exitGrind = false;

					position = grindEdge->GetPosition(edgeQuantity);

					if (CheckStandUp())
					{
						exitGrind = true;
					}
					else
					{
						if (!TryStandupOnForcedGrindExit())
						{
							exitGrind = true;
						}
					}

					if (exitGrind)
					{
						framesInAir = 0;
						SetAction(JUMP);
						frame = 1;
						ground = NULL;
						grindEdge = NULL;
						offsetX = 0;
						PhysicsResponse();
						if (velocity.x > 0)
						{
							facingRight = true;
						}
						else if (velocity.x < 0)
						{
							facingRight = false;
						}
					}*/

					TryStandupOnForcedGrindExit();
					return;
				}
			}
			else
			{
				q += movement;
				movement = 0;
			}
		}
	}
	edgeQuantity = q;


	if (!simulationMode)
	{
		V2d newGrindPos = grindEdge->GetPosition(edgeQuantity);
		Rect<double> newR(newGrindPos.x + b.offset.x - b.rw, newGrindPos.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);

		double oldRight = oldR.left + oldR.width;
		double right = newR.left + newR.width;

		double oldBottom = oldR.top + oldR.height;
		double bottom = newR.top + newR.height;

		double maxRight = right;
		if (oldRight > maxRight)
		{
			maxRight = oldRight;
		}

		double maxBottom = bottom;
		if (oldBottom > maxBottom)
		{
			maxBottom = oldBottom;
		}

		double minLeft = newR.left;
		if (oldR.left < minLeft)
		{
			minLeft = oldR.left;
		}

		double minTop = newR.top;
		if (oldR.top < minTop)
		{
			minTop = oldR.top;
		}

		currBooster = NULL;
		currTimeBooster = NULL;
		currFreeFlightBooster = NULL;
		currBounceBooster = NULL;
		currGravModifier = NULL;
		currHomingBooster = NULL;

		double ex = 1;
		Rect<double> r(minLeft - ex, minTop - ex, (maxRight - minLeft) + ex * 2, (maxBottom - minTop) + ex * 2);

		queryType = Q_ACTIVEITEM;
		sess->activeItemTree->Query(this, r);//activeR);

		queryType = Q_RAIL;
		sess->railEdgeTree->Query(this, r);

		queryType = Q_GRASS;
		sess->grassTree->Query(this, r);

		double extraMovement = .9;//.001;

		if (checkRailAndTerrainTransfers)
		{
			tempVel = newGrindPos - oldGrindPos;
			if (grindEdge->rail != NULL)
			{
				queryType = Q_RAIL_GRIND_TERRAIN_CHECK;
				sess->terrainTree->Query(this, r);

				if (grindEdge->rail == NULL)
				{
					physicsOver = true;

					if (grindSpeed > 0)
					{
						UpdateGrindPhysics(extraMovement, false);
					}
					else if (grindSpeed < 0)
					{
						UpdateGrindPhysics(-extraMovement, false);
					}
				}
			}
			else
			{
				queryType = Q_TERRAIN_GRIND_RAIL_CHECK;
				sess->railEdgeTree->Query(this, r);

				if (grindEdge->rail != NULL)
				{
					physicsOver = true;

					if (grindSpeed > 0)
					{
						UpdateGrindPhysics(extraMovement, false);
					}
					else if (grindSpeed < 0)
					{
						UpdateGrindPhysics(-extraMovement, false);
					}
				}
			}

		}
		
	}


	PhysicsResponse();
}

void Actor::HandleBounceGrass()
{
	//might not work for flat /\ tops of hills like that.

	V2d bn = bounceNorm;//bounceEdge->Normal();//minContact.normal;


	if (ground != NULL)
	{
		position = ground->GetPosition(edgeQuantity);
		position.x += offsetX + b.offset.x;

		if (ground->Normal().y > 0)
		{
			{
				position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
			}
		}
		else
		{
			if (ground->Normal().y < 0)
			{
				position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
			}
		}
	}

	physicsOver = true;

	V2d oldVel = velocity;

	double extraBUp = .2;
	double extraBDown = .2;
	double currBoostBounceSpeed = GetBounceBoostSpeed();
	double dSpeed = GetDashSpeed();

	if (ground != NULL && ground != bounceEdge)
	{
		velocity = ground->Along() * storedBounceGroundSpeed;//groundSpeed;
	}

	ground = NULL;

	velocity = sess->CalcBounceReflectionVel(bounceNorm, velocity);//minContact.edge, velocity);

	bounceEdge = NULL;

	V2d vDir = normalize(velocity);

	if (bn.y != 0)
	{
		if ((bn.x > 0 && velocity.x > 0) || (bn.x < 0 && velocity.x < 0))
		{
			velocity = bn * length(velocity);
		}
	}
	else
	{
		if (currInput.LUp())
		{
			vDir = normalize(vDir + V2d(0, -extraBUp));
			velocity = vDir * length(velocity);
		}
		else if (currInput.LDown())
		{
			vDir = normalize(vDir + V2d(0, extraBDown));
			velocity = vDir * length(velocity);
		}
	}


	velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;

	if (bounceNorm.y != 0)
	{
		/*if (oldVel.x > 0 && bounceNorm.x > 0 && velocity.x < oldVel.x)
		{
			velocity.x = oldVel.x;
		}
		else if (oldVel.x < 0 && bounceNorm.x < 0 && velocity.x < oldVel.x)
		{
			velocity.x = oldVel.x;
		}*/

		if (oldVel.x > 0 && velocity.x > 0 && velocity.x < oldVel.x)
		{
			velocity.x = oldVel.x;
		}
		else if (oldVel.x < 0 && velocity.x < 0 && velocity.x > oldVel.x)
		{
			velocity.x = oldVel.x;
		}
	}

	//minimum bounce along normal. wall minimum bounce is smaller
	double alongNormal = dot(velocity, bn);
	double limit = 10.0;
	if (bn.x == 1.0 || bn.x == -1.0)
	{
		limit = 2.0;
	}
	if (alongNormal < limit)
	{
		//cout << "heres the test" << endl;
		double diff = limit - alongNormal;
		velocity += diff * bn;
	}

	if (bn.y == 1 || bn.y == -1)
	{
		if (velocity.x == 0)
		{
			if (currInput.LLeft())
			{
				velocity.x = -dSpeed;
			}
			else if (currInput.LRight())
			{
				velocity.x = dSpeed;
			}
		}
		else if (velocity.x > 0)
		{
			if (currInput.LLeft())
			{
				velocity.x = -dSpeed;
			}
		}
		else if (velocity.x < 0)
		{
			if (currInput.LRight())
			{
				velocity.x = dSpeed;
			}
		}
	}

	RestoreAirOptions();

	SetAction(RAILBOUNCE);

	frame = 0;
}

void Actor::HandleBounceRail()
{
	V2d bn = bounceEdge->Normal();//minContact.normal;


	if (ground != NULL)
	{
		position = ground->GetPosition(edgeQuantity);
		position.x += offsetX + b.offset.x;

		if (ground->Normal().y > 0)
		{
			{
				position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
			}
		}
		else
		{
			if (ground->Normal().y < 0)
			{
				position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
			}
		}
	}


	V2d oldVel = velocity;

	physicsOver = true;


	double maxBounceBoost = 40;




	double extraBUp = .2;
	double extraBDown = .2;
	double currBoostBounceSpeed = GetBounceBoostSpeed();
	double dSpeed = GetDashSpeed();

	if (ground != NULL && ground != bounceEdge )
	{
		//cout << "groundSpeed: " << groundSpeed << "\n";
		if (groundSpeed == 0)
		{
			int xxx = 5;
		}
		velocity = ground->Along() * storedBounceGroundSpeed;//groundSpeed;
	}

	ground = NULL;
	reversed = false;

	velocity = sess->CalcBounceReflectionVel(bounceNorm, velocity);//minContact.edge, velocity);

	bounceEdge = NULL;

	V2d vDir = normalize(velocity);

	double testBarelyAngledThresh = .2;
	if (bn.y != 0)
	{
		if ((bn.x > testBarelyAngledThresh && velocity.x > 0) || (bn.x < -testBarelyAngledThresh && velocity.x < 0))
		{
			velocity = bn * length(velocity);
		}
	}
	else
	{
		if (currInput.LUp())
		{
			vDir = normalize(vDir + V2d(0, -extraBUp));
			velocity = vDir * length(velocity);
		}
		else if (currInput.LDown())
		{
			vDir = normalize(vDir + V2d(0, extraBDown));
			velocity = vDir * length(velocity);
		}
	}

	velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;
	
	if (bounceNorm.y != 0)
	{
		if (oldVel.x > 0 && bounceNorm.x > 0 && velocity.x < oldVel.x)
		{
			velocity.x = oldVel.x;
		}
		else if (oldVel.x < 0 && bounceNorm.x < 0 && velocity.x < oldVel.x)
		{
			velocity.x = oldVel.x;
		}
	}

	if (bn.y == 1 || bn.y == -1)
	{
		if (velocity.x == 0)
		{
			if (currInput.LLeft())
			{
				velocity.x = -dSpeed;
			}
			else if (currInput.LRight())
			{
				velocity.x = dSpeed;
			}
		}
		else if (velocity.x > 0)
		{
			if (currInput.LLeft())
			{
				velocity.x = -dSpeed;
			}
		}
		else if (velocity.x < 0)
		{
			if (currInput.LRight())
			{
				velocity.x = dSpeed;
			}
		}
	}

	/*double minVel = 20.0;

	if (minContact.edge->IsFlatGround()
		|| minContact.edge->IsSlopedGround())
	{
		if (minContact.normal.y < 0 )
		{
			if (velocity.y > -minVel)
			{
				velocity.y = -minVel;
			}
		}
		else if (minContact.normal.y > 0)
		{
			if (velocity.y < minVel)
			{
				velocity.y = minVel;
			}
		}
	}
	else if (minContact.edge->IsSteepGround())
	{
		if (length(velocity) < minVel)
		{
			velocity = normalize(velocity) * minVel;
		}
	}
	else if (minContact.edge->IsWall())
	{
		if (minContact.normal.x > 0)
		{
			velocity.x = minVel;
		}
		else if (minContact.normal.x < 0)
		{
			velocity.x = -minVel;
		}
	}*/


	


	RestoreAirOptions();

	/*if (action == AIRDASH || IsSpringAction(action))
	{
		SetAction(JUMP);
		frame = 1;
	}*/

	
	
	//position.x += offsetX;
	//offsetX = 0;
	

	SetAction(RAILBOUNCE);
	
	frame = 0;
}

bool Actor::UpdateGrindRailPhysics(double movement)
{
	Edge *e0 = grindEdge->edge0;
	Edge *e1 = grindEdge->edge1;
	//V2d e0n = e0->Normal();
	//V2d e1n = e1->Normal();

	double q = edgeQuantity;
	while (!approxEquals(movement, 0))
	{
		//cout << "movement: " << movement << endl;
		double gLen = length(grindEdge->v1 - grindEdge->v0);
		if (movement > 0)
		{
			double extra = q + movement - gLen;
			if (extra > 0)
			{
				movement -= gLen - q;



				V2d v0 = grindEdge->v0;
				V2d v1 = grindEdge->v1;

				if (e1 != NULL)
				{
					grindEdge = e1;
					q = 0;
				}
				else
				{
					SetAction(JUMP);
					frame = 1;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesNotGrinding = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

					PhysicsResponse();
					return true;
				}
			}
			else
			{
				q += movement;
				V2d posOld = position;
				bool col = ResolvePhysics(normalize(grindEdge->v1 - grindEdge->v0) * movement);

				/*if (IsOnRailAction(action))
				{
					break;
				}*/

				if (col)
				{
					CheckCollisionForTerrainFade();
				}

				if (col)
				{
					position = posOld;
					SetAction(JUMP);
					frame = 1;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesNotGrinding = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
				}
				movement = 0;
			}
		}
		else if (movement < 0)
		{
			double extra = q + movement;
			if (extra < 0)
			{
				movement -= movement - extra;

				V2d v0 = grindEdge->v0;
				sf::Rect<double> r(v0.x - 1, v0.y - 1, 2, 2);

				if (e0 != NULL)
				{
					grindEdge = e0;
					q = length(grindEdge->v1 - grindEdge->v0);
				}
				else
				{
					SetAction(JUMP);
					frame = 1;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesSinceGrindAttempt = 0;
					framesNotGrinding = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

					PhysicsResponse();
					return true;

				}
			}
			else
			{
				q += movement;
				V2d posOld = position;
				bool col = ResolvePhysics(normalize(grindEdge->v1 - grindEdge->v0) * movement);

				/*if (IsOnRailAction(action))
				{
					break;
				}*/

				if (col)
				{
					CheckCollisionForTerrainFade();
				}

				if (col)
				{
					position = posOld;
					SetAction(JUMP);
					frame = 1;
					framesNotGrinding = 0;
					velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					grindEdge = NULL;
					regrindOffCount = 0;
					framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
				}
				movement = 0;
			}
		}
	}

	if (IsOnRailAction(action))
	{
		edgeQuantity = q;

		PhysicsResponse();
		return true;
	}

	return false;
}

bool Actor::TryUnlockOnTransfer( Edge * e)
{
	if (e->IsGateEdge() )
	{
		Gate * g = (Gate*)e->info;

		if (CanUnlockGate(g))
		{
			UnlockGate(g);

			SetTouchedGate(g);
			return true;
		}
	}

	return false;
}

void Actor::LeaveTipTransfer(bool right, V2d leaveExtra)
{
	V2d along(1, 0);

	/*if (reversed)
	{
		along = -along;
	}*/
	/*if (!right)
	{
		along = -along;
	}*/
	velocity = along * groundSpeed + leaveExtra;//normalize(ground->v1 - ground->v0) * groundSpeed + leaveExtra;

	movementVec = along * extra;

	//recently added this because it makes sense with LeaveGroundTransfer but haven't tested.
	//originally was just movementVec.y -= .01; for all
	if (!reversed)
	{
		movementVec.y -= .01;
	}
	else
	{
		movementVec.y += .01;
	}

	if (right)
	{
		if (movementVec.x <= .01)
		{
			movementVec.x = .01;
		}
	}
	else
	{
		if (movementVec.x >= -.01)
		{
			movementVec.x = -.01;
		}
	}

	//cout << "leave tip transfer: " << along.x << ", " << along.y << endl;

	reversed = false;
	leftGround = true;
	ground = NULL;
	SetAction(JUMP);
	frame = 1;
	holdJump = false;
}

void Actor::LeaveGroundTransfer(bool right, V2d leaveExtra )
{
	double speed = groundSpeed;
	if (reversed)
	{
		speed = -groundSpeed;
	}

	V2d along = ground->Along();
	velocity = along * speed + leaveExtra;//normalize(ground->v1 - ground->v0) * groundSpeed + leaveExtra;

	movementVec = along * extra;

	if (!reversed)
	{
		movementVec.y -= .01;
	}
	else
	{
		movementVec.y += .01;
	}
	if (right)
	{
		if (movementVec.x <= .01)
		{
			movementVec.x = .01;
		}
	}
	else
	{
		if (movementVec.x >= -.01)
		{
			movementVec.x = -.01;
		}
	}
	
	//cout << "leave ground transfer: " << along.x << ", " << along.y << endl;

	reversed = false;
	leftGround = true;
	ground = NULL;
	SetAction(JUMP);
	frame = 1;
	holdJump = false;
}

void Actor::LeaveGroundTipTransfer(bool rightTransfer, Edge *altEdge, V2d leaveExtra )
{
	//V2d leaveExtra(0, -gravity * 2);
	V2d along = altEdge->Along();//ground->Along();

	double speed = groundSpeed;
	if (reversed)
	{
		speed = -speed;
	}

	velocity = along * speed + leaveExtra;//normalize(ground->v1 - ground->v0) * groundSpeed + leaveExtra;

	//cout << "transfer vel: " << velocity.x << ", " << velocity.y << endl;

	movementVec = along * extra;

	if (reversed)
	{
		movementVec.y += .01;
	}
	else
	{
		movementVec.y -= .01;
	}
	
	bool r = ( rightTransfer && !reversed ) || ( !rightTransfer && reversed );

	if (r)
	{
		if (movementVec.x <= .01)
		{
			movementVec.x = .01;
		}
	}
	else
	{
		if (movementVec.x >= -.01)
		{
			movementVec.x = -.01;
		}
	}

	reversed = false;
	leftGround = true;
	ground = NULL;
	SetAction(JUMP);
	frame = 1;
	holdJump = false;
}

bool Actor::UpdateAutoRunPhysics( double q, double m )
{
	if (IsAutoRunning())
	{
		if (autoRunStopEdge != NULL && autoRunStopEdge == ground)
		{
			if (facingRight)
			{
				if (autoRunStopQuant <= q && autoRunStopQuant >= q - m)
				{
					WaitInPlace();
					return true;
				}
			}
			else
			{
				if (autoRunStopQuant >= q && autoRunStopQuant <= q - m)
				{
					WaitInPlace();
					return true;
				}
			}
		}
	}

	return false;
}

void Actor::RestoreAirOptions()
{
	hasDoubleJump = true;
	hasAirDash = true;

	//if (HasUpgrade(UPGRADE_W1_WALLJUMP_RESTORES_DOUBLEJUMP))
	{
		hasWallJumpRechargeDoubleJump = true;
	}
	
	//if (HasUpgrade(UPGRADE_W1_WALLJUMP_RESTORES_AIRDASH))
	{
		hasWallJumpRechargeAirDash = true;
	}

	/*if (HasUpgrade(UPGRADE_W7_DOUBLE_AIRDASH_BOOST))
	{
		numRemainingExtraAirdashBoosts = 1;
	}*/
	//else
	{
		numRemainingExtraAirdashBoosts = 0;
	}
	
	
	hasHitRechargeDoubleJump = true;
	hasHitRechargeAirDash = true;
}

void Actor::UpdatePhysics()
{
	if (hitlagFrames > 0)
	{
		//cout << "cancel update physics because of hitlag" << endl;
		return;
	}

	//there are some potential bugs with minContact.edge->Normal(), in some cases just use minContact.normal
	//haven't found other situations where the issue occurs but the remaining instances of the call to Normal
	//should be an easy fix if it ever comes up


	//cout << "update physics " << endl;

	if (IsIntroAction(action) || IsGoalKillAction(action) || action == EXIT
		|| action == RIDESHIP || action == WAITFORSHIP || action == SEQ_WAIT
		|| action == GRABSHIP || action == EXITWAIT || action == EXITBOOST || action == EXIT_RUSH
		|| action == DEATH || action == HIDDEN || hitEnemyDuringPhysics)
		return;

	UpdateWirePhysics();

	double temp_groundSpeed = groundSpeed / slowMultiple;
	V2d temp_velocity = velocity / (double)slowMultiple;
	double temp_grindSpeed = grindSpeed / slowMultiple;

	leftGround = false;
	double movement = 0;
	double maxMovement = min(b.rw, b.rh);
	movementVec = V2d(0, 0);
	V2d lastExtra(100000, 100000);

	if (grindEdge != NULL)
	{
		if (reversed)
		{
			reversed = false;
			grindSpeed = -grindSpeed;
		}
		movement = temp_grindSpeed / GetNumSteps();
	}
	else if (ground != NULL)
	{

		movement = temp_groundSpeed / GetNumSteps();
		if (movement != 0 && abs(movement) < .00001)
		{
			//maybe here I should reduce the groundspeed to 0? 
			//i seemed to solve the weird teleportation/super fast movement
			//glitch from before but I'm still not quite sure how it works
			//you just get a huge movement value somehow from having a really
			//low groundspeed in a weird circumstance. hopefully it 
			//doesn't cause any more problems
			//happens mashing jump into a steep slope w/ an acute in-cut ceiling

			//cout << "what movement: " << movement << ", " << temp_groundSpeed << endl;
			return;
		}

	}
	else
	{
		//to prevent falling too fast while hitting an enemy and falling
		//maybe only trigger when using dair?
		if( frameAfterAttackingHitlagOver )
		{
			if (temp_velocity.y > maxFallSpeedWhileHitting)
			{
				temp_velocity.y = maxFallSpeedWhileHitting;
			}
		}

		movementVec = temp_velocity / GetNumSteps();
	}

	if (physicsOver)
	{
		//still need to do hitbox/hurtbox responses if hes not moving
		return;
	}

	if (grindEdge == NULL && movement == 0 && movementVec.x == 0 && movementVec.y == 0)
	{
		ResolvePhysics(V2d(0, 0));

		PhysicsResponse();
		return;
	}

	if (grindEdge != NULL && IsGrindAction( action ))
	{
		UpdateGrindPhysics(movement, true);
		return;
	}
	else if (grindEdge != NULL && IsOnRailAction(action))
	{
		if (UpdateGrindRailPhysics(movement))
		{
			return;
		}
	}
	else if( reversed )
	{
		//if you slide off a reversed edge you need a little push so you dont slide through the point.
		V2d reverseExtra = UpdateReversePhysics();
		if( reverseExtra.x == 0 && reverseExtra.y == 0 )
		{
			PhysicsResponse();
			return;
		}
		movementVec = reverseExtra;
	}

	do
	{
		if( ground != NULL )
		{
			double steal = 0;
			if( movement > 0 )
			{
				if( movement > maxMovement )
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else if( movement < 0 )
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}
			else
			{
				
			}


			extra = 0;
			bool leaveGround = false;
			double q = edgeQuantity;

			V2d gNormal = ground->Normal();



			double m = movement;
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals(q, groundLength) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;

			V2d e0n, e1n;
			if (e0 != NULL)
			{
				e0n = e0->Normal();
			}

			if (e1 != NULL)
			{
				e1n = e1->Normal();
			}

			//cout << "offsetX: " << offsetX << "\n";

			bool transferLeft = e0 != NULL && q == 0 && movement < 0 //&& (groundSpeed < -steepClimbSpeedThresh || e0n.y <= -steepThresh || e0n.x <= 0 )
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0)  ) 
				|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ) );
			bool transferRight = e1 != NULL && q == groundLength && movement > 0 //(groundSpeed < -steepClimbSpeedThresh || e1n.y <= -steepThresh || e1n.x >= 0 )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ));

			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;

			//on reverse doesnt need to fly up off of edges
			if( transferLeft )
			{
				TryUnlockOnTransfer(e0);

				Edge *next = ground->edge0;
				if( next->Normal().y < 0 && abs( e0n.x ) < wallThresh )
				{
					if( e0n.x > 0 && e0n.y > -steepThresh )
					{
						//steeptesting 4

						bool movingDown = gNormal.x < 0;
						if( groundSpeed >= -steepClimbSpeedThresh && (!ground->IsSteepGround() || movingDown ) )
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
					else if( gNormal.x > 0 && gNormal.y > -steepThresh )
					{
						//this is for walking left/right from the top of a steep tip
						if (e0n.x <= 0 && e0n.y > -steepThresh )
						{
							LeaveTipTransfer(false, V2d(0, -gravity * 2));//V2d( 0, -1 ));
							break;
							//LeaveGroundTransfer(false);//, V2d(0, -gravity * 2));
						}
						else
						{
							ground = next;
							q = length(ground->v1 - ground->v0);
						}
						
					}
					else
					{
						if( e0n.y > -steepThresh )
						{
							if( e0n.x < 0 )
							{
								if( gNormal.x >= 0 )
								{
									bool isRail = false;
									if (ground->rail != NULL)
									{
										isRail = true;
									}
									LeaveGroundTransfer(false);

									//fixes the bug where you land on the rail again 
									//in the same frame not sure why the bug happens, 
									//but this fixes it.
									if (isRail)
										break; 
									

								}
								else
								{
									if (!IsGroundAttackAction(action))
									{
										if (action != STEEPCLING)
										{
											SetAction(STEEPSLIDE);
											frame = 0;

											facingRight = false;
										}
									}
									ground = next;
									q = length( ground->v1 - ground->v0 );	
								}
							}
							else if( e0n.x > 0 )
							{
								SetAction(STEEPCLIMB);
								frame = 0;
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
							else
							{
								ground = next;
								q = length( ground->v1 - ground->v0 );	
							}
						}
						else
						{
							ground = next;
							q = length( ground->v1 - ground->v0 );	
						}
					}
				}
				else
				{
					bool isRail = false;
					if (ground->rail != NULL)
					{
						isRail = true;
					}
					LeaveGroundTransfer(false);

					//fixes the bug where you land on the rail again 
					//in the same frame not sure why the bug happens, 
					//but this fixes it.
					if (isRail)
						break;
				}
			}
			else if( transferRight )
			{
				TryUnlockOnTransfer(e1);
				

				Edge *next = ground->edge1;
				V2d nextNorm = next->Normal();
				if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh )
				{

					if( e1n.x < 0 && e1n.y > -steepThresh )
					{
						//steeptesting 5
						bool movingDown = gNormal.x > 0;
						if( groundSpeed <= steepClimbSpeedThresh && (!ground->IsSteepGround() || movingDown ) )
							//&& action != STEEPCLIMB && action != STEEPCLIMBATTACK)
						{
							groundSpeed = 0;
							break;
						}
						else
						{
							ground = next;
							q = 0;
						}
					}
					else if( gNormal.x < 0 && gNormal.y > -steepThresh )
					{
						if (e1n.x >= 0 && e1n.y > -steepThresh)//&& groundSpeed > 5)
						{
							LeaveTipTransfer(true, V2d(0, -gravity * 2));//V2d( 0, -1));
							break;
						}
						else
						{
							ground = next;
							q = 0;
						}
					}
					else
					{

						if( e1n.y > -steepThresh)
						{
							if( e1n.x > 0 )
							{
								if( gNormal.x <= 0 )
								{
									bool isRail = false;
									if (ground->rail != NULL)
									{
										isRail = true;
									}
									LeaveGroundTransfer(true);

									//fixes the bug where you land on the rail again 
									//in the same frame not sure why the bug happens, 
									//but this fixes it.
									if (isRail)
										break;
								}
								else
								{
									
									if (!IsGroundAttackAction(action))
									{
										if (action != STEEPCLING)
										{
											SetAction(STEEPSLIDE);
											frame = 0;
											facingRight = true;
										}
									}
									ground = next;
									q = 0;
								}
							}
							else if( e1n.x < 0 )
							{
								SetAction(STEEPCLIMB);
								frame = 0;
								ground = next;
								q = 0;
							}
							else
							{
								ground = next;
								q = 0;
							}
							
						}
						else
						{
							ground = next;
							q = 0;
						}
						
						
					}
					
				}
				else
				{
					bool isRail = false;
					if (ground->rail != NULL)
					{
						isRail = true;
					}
					LeaveGroundTransfer(true);

					//fixes the bug where you land on the rail again 
					//in the same frame not sure why the bug happens, 
					//but this fixes it.
					if (isRail)
						break;
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//some kind of bug in here for sure! deltashine got stuck on a rail while changing offset.

				if( movement > 0 )
					extra = (offsetX + movement) - b.rw;
				else 
				{
					extra = (offsetX + movement) + b.rw;
				}
				double m = movement;

				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offsetX = b.rw;
					}
					else
					{
						offsetX = -b.rw;
					}
				}
				else
				{
					movement = 0;

					offsetX += m;
				}


				

				
				//if(approxEquals(m, 0))
				//{
				//	break;
				//}
				//else
				{
					//new approxtesting changeoffset
					if (approxEquals(m, 0))
					{
						if (groundSpeed > 0)
						{
							/*Edge *next = ground->edge1;
							if (next == NULL)
							{
								LeaveGroundTransfer(true, V2d(0, -gravity * 2));
								break;
							}
							else*/
							{
								m = .01;
							}
						}
						else if (groundSpeed < 0)
						{
							/*Edge *next = ground->edge0;

							if (next == NULL)
							{
								LeaveGroundTransfer(false, V2d(0, -gravity * 2));
								break;
							}
							else*/
							{
								m = -.01;
							}
						}
					}



					V2d oldPos = position;
					bool hit = ResolvePhysics( V2d( m, 0 ));

					if (IsOnRailAction(action))
					{
						break;
					}

					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) 
						|| ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.normal;//.edge->Normal();

						if( eNorm.y < 0 )
						{
							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);
							if( minContact.position.y >= position.y + b.rh - 5 && !speedTransfer)
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

									edgeQuantity = q;
									offsetX = -b.rw;
									ProcessGroundedCollision();
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;

									q = ground->GetQuantity( minContact.position );

									edgeQuantity = q;
									offsetX = b.rw;
									ProcessGroundedCollision();

									continue;
								}
								else
								{
									ProcessGroundedCollision();
								}
								

							}
							else
							{
								offsetX += minContact.resolution.x;

								ProcessGroundedCollision();

								groundSpeed = 0;
								
								break;
							}
						}
						else
						{
								offsetX += minContact.resolution.x;

								ProcessGroundedCollision();

								groundSpeed = 0;
								
								break;
						}
					}
					else
					{
						if (movement > 0 && offsetX == b.rw && gNormal.x > 0 && gNormal.y > -steepThresh  )// && groundSpeed > 5)
						{
							if (e0n.x < 0 && e0n.y > -steepThresh)
							{
								LeaveTipTransfer(true, V2d(0, -gravity * 2));
							}
							else
							{
								LeaveGroundTipTransfer(true, e0, V2d(0, -gravity * 2)); //uses the previous edge's along
							}
							
							break;
						}
						else if (movement < 0 && offsetX == -b.rw && gNormal.x < 0 && gNormal.y > -steepThresh )// && groundSpeed < -5)
						{
							if (e1n.x > 0 && e1n.y > -steepThresh)
							{
								LeaveTipTransfer(false, V2d(0, -gravity * 2));
							}
							else
							{
								LeaveGroundTipTransfer(false, e1, V2d(0, -gravity * 2));
							}
							
							break;
						}
						else
						{
							V2d wVel = position - oldPos;
							edgeQuantity = q;
						}						
					}

					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{

						//cout << "checking for airborne" << endl;
						if (gNormal.x < 0 && gNormal.y > -steepThresh && e1n.x >= 0
							&& abs(e1n.x) < wallThresh && groundSpeed > 5)
						{
							LeaveGroundTransfer(true, V2d(0, -gravity * 2));
							break;
						}
						else
						{
							q = groundLength;
						}
						//here is where i really lift off
					}
					else
					{
						if (gNormal.x > 0 && gNormal.y > -steepThresh && e0n.x <= 0
							&& abs(e0n.x) < wallThresh && groundSpeed < -5)
						{
							LeaveGroundTransfer(false, V2d(0, -gravity * 2));
							break;
						}
						else
						{
							q = 0;
						}
						
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				
				if (UpdateAutoRunPhysics(q, m))
					return;

				
				//else
				{	
					//if (approxEquals(m, 0))
					if( false )
					{
						if (groundSpeed > 0)
						{
							Edge *next = ground->edge1;


							if (next == NULL)
							{
								LeaveGroundTransfer(true, V2d(0, -gravity * 2));
							}

							/*double yDist = abs(gNormal.x) * groundSpeed;
							if (next != NULL && next->Normal().y < 0 && abs(e1n.x) < wallThresh && !(HoldingRelativeUp() && !currInput.LRight() && gNormal.x < 0 && yDist > slopeLaunchMinSpeed && next->Normal().x >= 0))
							{
							if (e1n.x < 0 && e1n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh)
							{
							if (!TryUnlockOnTransfer(e1))
							{
							groundSpeed = 0;
							}
							}
							}
							else if (next != NULL && abs(e1n.x) >= wallThresh)
							{
							if (!TryUnlockOnTransfer(e1))
							{
							if (bounceFlameOn && abs(groundSpeed) > 1)
							{
							storedBounceGroundSpeed = groundSpeed * slowMultiple;
							groundedWallBounce = true;
							}

							groundSpeed = 0;
							}
							}
							else
							{
							velocity = normalize(ground->v1 - ground->v0) * groundSpeed;

							movementVec = normalize(ground->v1 - ground->v0) * extra;

							leftGround = true;
							ground = NULL;
							}*/
						}
						else if (groundSpeed < 0)
						{
							Edge *next = ground->edge0;

							if (next == NULL)
							{
								LeaveGroundTransfer(false, V2d(0, -gravity * 2));
							}

							/*double yDist = abs(gNormal.x) * groundSpeed;
							Edge *next = ground->edge0;
							if (next != NULL && next->Normal().y < 0 && abs(e0n.x) < wallThresh && !(HoldingRelativeUp() && !currInput.LLeft() && gNormal.x > 0 && yDist < -slopeLaunchMinSpeed && next->Normal().x < gNormal.x))
							{
							if (e0n.x > 0 && e0n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh)
							{
							if (!TryUnlockOnTransfer(e0))
							{
							groundSpeed = 0;
							}
							}
							}
							else if (next != NULL && abs(e0n.x) >= wallThresh)
							{
							if (!TryUnlockOnTransfer(e0))
							{
							if (bounceFlameOn && abs(groundSpeed) > 1)
							{
							storedBounceGroundSpeed = groundSpeed * slowMultiple;
							groundedWallBounce = true;
							}

							groundSpeed = 0;
							}
							}
							else
							{
							velocity = normalize(ground->v1 - ground->v0) * groundSpeed;
							movementVec = normalize(ground->v1 - ground->v0) * extra;
							leftGround = true;

							ground = NULL;
							}*/
						}

						/*if (ground != NULL)
						{
							break;
						}*/
					}

					//new approxtesting
					if (approxEquals(m,0))
					{
						if (groundSpeed > 0)
						{
							Edge *next = ground->edge1;
							if (next == NULL)
							{
								LeaveGroundTransfer(true, V2d(0, -gravity * 2));
								break;
							}
							else
							{
								m = .01;
							}
						}
						else if (groundSpeed < 0)
						{
							Edge *next = ground->edge0;

							if (next == NULL)
							{
								LeaveGroundTransfer(false, V2d(0, -gravity * 2));
								break;
							}
							else
							{
								m = -.01;
							}
						}
					}

					bool down = true;
					V2d oldPos = position;
					
					V2d resMove = normalize( ground->v1 - ground->v0 ) * m;
					bool hit = ResolvePhysics( resMove );

					if (IsOnRailAction(action))
					{
						break;
					}

					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) 
						|| ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						if( down)
						{
							V2d eNorm = minContact.normal;
							
							if( minContact.position == minContact.edge->v0 )
							{

							}
							if( minContact.position.y > position.y + b.offset.y + b.rh - 5 
								&& minContact.edge->Normal().y >= 0 )
							{
								if( minContact.position == minContact.edge->v0 ) 
								{
									if( minContact.edge->edge0->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge0;
									}
								}
							}



							if( eNorm.y < 0 )
							{
								bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh 
									&& groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh 
										&& groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);

								if( minContact.position.y >= position.y + minContact.resolution.y + b.rh + b.offset.y - 5  
									&& !speedTransfer)
								{
									double test = position.x + minContact.resolution.x - minContact.position.x;
									if( (test < -b.rw && !approxEquals(test,-b.rw))
										|| (test > b.rw && !approxEquals(test,b.rw)) )
									{
										//corner border case. hope it doesn't cause problems
										cout << "CORNER BORDER CASE: " << test << endl;
										q = ground->GetQuantity( ground->GetPosition( q ) + minContact.resolution);
										
										edgeQuantity = q;

										ProcessGroundedCollision();
										groundSpeed = 0;
										
										break;
									}
									else
									{	
										position += minContact.resolution;

										if (minContact.normal.x > 0)
										{
											offsetX = b.rw;
										}
										else if (minContact.normal.x < 0)
										{
											offsetX = -b.rw;
										}
										else
										{
											offsetX = (position.x + b.offset.x) - minContact.position.x;
										}

										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );

										V2d eNorm = minContact.normal;

										

										//hopefully this doesn't cause any bugs. 
										//if it does i know exactly where to find it

										//CHANGED OFFSET
										//offsetX = position.x + minContact.resolution.x - minContact.position.x;

										//changed from previous^ because you could get stuck

										
										

										
										

										ProcessGroundedCollision();
									}

									/*if( offsetX < -b.rw || offsetX > b.rw )
									{
										cout << "BROKEN OFFSET: " << offsetX << endl;
										assert( false && "T_T" );
									}*/
								}
								else
								{
									q = ground->GetQuantity( ground->GetPosition( q ) + minContact.resolution);
									
									edgeQuantity = q;

									ProcessGroundedCollision();

									groundSpeed = 0;
									break;
								}
							}
							else
							{
								V2d testVel = normalize(ground->v1 - ground->v0 ) * groundSpeed;
								//maybe want to do this on some ceilings but its hard for now. do i need it?
								//if( currInput.LUp() && testVel.y < -offSlopeByWallThresh && eNorm.y == 0 )

								//might cause some weird stuff w/ bounce but i can figure it out later
								if( testVel.y < -offSlopeByWallThresh && eNorm.y == 0 && !bounceFlameOn && !minContact.edge->IsInvisibleWall() )
								{
									assert( abs(eNorm.x ) > wallThresh );
							//		cout << "testVel: " << testVel.x << ", " << testVel.y << endl;
									velocity = testVel;
						
									position += minContact.resolution;

									//resMove -= minContact.resolution;

									//cout << "extra: " << extra << "\n";

									movementVec = normalize(ground->v1 - ground->v0) * steal;//V2d(0, 0);//normalize(ground->v1 - ground->v0) * steal;//resMove;

									//this basically is always V2d(0,0) afaik. The issue before
									//was that I was using * extra, and that fails if you hit the wall but its not at the end
									//of your edge, such as when using a floor rail.

									//if I have other skipping bugs from physics it will most likely be because of doing *extra but I haven't found any others yet
									


									leftGround = true;
									ground = NULL;
									SetAction( JUMP );
									holdJump = false;
									frame = 1;

									ProcessGroundedCollision();
									//rightWire->UpdateAnchors( V2d( 0, 0 ) );
									//leftWire->UpdateAnchors( V2d( 0, 0 ) );
								}
								else
								{
									if( bounceFlameOn && abs( groundSpeed ) > 1)
									{
										//steeptesting 6
										//if( action != STEEPCLIMB && action != STEEPCLIMBATTACK)
										{
											storedBounceGroundSpeed = groundSpeed * slowMultiple;
											groundedWallBounce = true;
										}

										break;
									}
									else if (minContact.normal.y > 0 )//&& DefaultGravReverseCheck() )
									{
										//this is so that DefaultGravReverseCheck() and DefaultCeilingLanding() are accurate
										position += minContact.resolution;







										/*q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);
										edgeQuantity = q;

										Vector2<double> groundPoint = ground->GetPosition(edgeQuantity);

										position = groundPoint;

										position.x += offsetX + b.offset.x;
										position.y += normalHeight;*/





										//offsetX = (position.x + b.offset.x) - minContact.position.x;

										

										//if (reversed)
										//{
										//	if (gn.y > 0 || abs(offsetX) != b.rw)
										//	{
										//		position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
										//	}
										//}
										//else
										//{
										//	if (gn.y < 0 || abs(offsetX) != b.rw)
										//	{
										//		position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
										//	}
										//}

										if (DefaultGravReverseCheck())
										{
											//position -= minContact.resolution;
											collision = true;
											velocity = groundSpeed * ground->Along();

											DefaultCeilingLanding(movement);

											break;
										}
										else
										{
											//position -= minContact.resolution; //doubt I need this
											//don't need to move the position back because it gets instantly corrected
											//within PhysicsResponse anyway
											q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);


											edgeQuantity = q;

											ProcessGroundedCollision();

											groundSpeed = 0;

											break;
										}
									}
									else
									{
										q = ground->GetQuantity(ground->GetPosition(q) + minContact.resolution);


										edgeQuantity = q;

										ProcessGroundedCollision();

										groundSpeed = 0;

										break;
									}

									
									
								}
							}
						}
						else
						{
							q = ground->GetQuantity( ground->GetPosition( q ) + minContact.resolution);
							
							edgeQuantity = q;

							ProcessGroundedCollision();

							groundSpeed = 0;
							break;
						}
						
					}
					else
					{
						V2d wVel = position - oldPos;
						edgeQuantity = q;
					}

					leftWire->UpdateAnchors( V2d( 0, 0 ) );
					rightWire->UpdateAnchors( V2d( 0, 0 ) );
				}	
			}

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
		else
		{
			V2d stealVec(0,0);
			double moveLength = length( movementVec );
			
			V2d velDir = normalize( movementVec );
			if( moveLength > maxMovement )
			{
				stealVec = velDir * ( moveLength - maxMovement);
				movementVec = velDir * maxMovement;
			}

			V2d newVel( 0, 0 );
			V2d oldPos = position;

			/*UpdateHitboxes();
			V2d B = b.GetQuadVertex(1);
			Zone *currZone = sess->currentZone;
			bool inZone = currZone->ContainsPoint(B);
			if (!inZone)
			{
				int x = 5;
			}*/

			bool tempCollision = ResolvePhysics( movementVec );


			if (IsOnRailAction(action))
			{
				break;
			}
			
			if (tempCollision)
			{
				CheckCollisionForTerrainFade();
			}

			V2d extraVel(0, 0);
			if( tempCollision  )
			{
				collision = true;
				//cout << "movementvec: " << movementVec.x << ", " << movementVec.y << "\n";
				position += minContact.resolution;
				//cout << "resolution: " << minContact.resolution.x << ", " << minContact.resolution.y << "\n";
				
				UpdateHitboxes();
				//V2d B = b.GetQuadVertex(1);
				//Zone *currZone = sess->currentZone;
				//bool inZone = currZone->ContainsPoint(B);
				//if (!inZone)
				//{
				//	int x = 5;
				//}

				Edge *e = minContact.edge;
				V2d en = e->Normal();
				Edge *e0 = e->edge0;
				Edge *e1 = e->edge1;
				
				V2d e0n;// = e0->Normal();
				V2d e1n;// = e1->Normal();

				
				if( e0 != NULL )
					e0n = e0->Normal();
				if( e1 != NULL )
					e1n = e1->Normal();
				

				if( minContact.position.y > position.y + b.offset.y + b.rh - 5 && minContact.edge->Normal().y >= 0 )
				{
					if( minContact.position == minContact.edge->v0 ) 
					{
						if( minContact.edge->edge0 != NULL && minContact.edge->edge0->Normal().y <= 0 )
						{
							minContact.edge = minContact.edge->edge0;
						}
					}
				}

				if( abs(minContact.edge->Normal().x) > wallThresh )
				{
					wallNormal = minContact.edge->Normal();
					currWall = minContact.edge;
				}


				V2d extraDir =  normalize( minContact.edge->v1 - minContact.edge->v0 );

				if( e0 != NULL && minContact.position == e->v0 && en.x < 0 && en.y < 0 )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}
				else if(e1 != NULL && minContact.position == e->v1 && en.x < 0 && en.y > 0  )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}

				else if(e1 != NULL && minContact.position == e->v1 && en.x < 0 && en.y < 0  )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = extraDir;
					}
				}
				else if(e0 != NULL && minContact.position == e->v0 && en.x > 0 && en.y < 0 )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{	
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}
				else if( (e1 != NULL && minContact.position == e->v1 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = V2d( 1, 0 );//extraDir;
					}
				}
				else if( (e0 != NULL && minContact.position == e->v0 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = V2d( 1, 0 );
					}
				}

				


				if( (e1 != NULL && minContact.position == e->v1 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
					}
				}
				else if( ( e0 != NULL && minContact.position == e->v0 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
					}
				}
				
				if( minContact.normal.x != 0 || minContact.normal.y != 0 )
				{
					if( abs( minContact.normal.x ) > wallThresh || ( minContact.normal.y > 0 && abs( minContact.normal.x ) > .9 ) )
					{
						wallNormal = minContact.normal;
					}
					extraDir = V2d( minContact.normal.y, -minContact.normal.x );
				}				
				
				double blah = length( velocity ) - length( minContact.resolution );

				//might still need some more work
				extraVel = dot( normalize( velocity ), extraDir ) * length( minContact.resolution ) * extraDir;
				
				if( length(extraVel) < .01 )
					extraVel = V2d( 0, 0 );

				//extraVel = V2d( 0, 0 );
				newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );
				
				if( length( stealVec ) > 0 )
				{
					stealVec = length( stealVec ) * normalize( extraVel );
				}
				if( approxEquals( extraVel.x, lastExtra.x ) && approxEquals( extraVel.y, lastExtra.y ) )
				{
					break;		
				}
				if( length( extraVel ) > 0 )
				{
					lastExtra.x = extraVel.x;
					lastExtra.y = extraVel.y;
				}
			}
			else if( length( stealVec ) == 0 )
			{

				movementVec.x = 0;
				movementVec.y = 0;
			}

			V2d wVel = position - oldPos;

			leftWire->UpdateAnchors( wVel );
			rightWire->UpdateAnchors( wVel );
			
			int maxJumpHeightFrame = 10;

			if( leftWire->IsPulling() || leftWire->IsHit() )
			{
				touchEdgeWithLeftWire = tempCollision;
				if( action == WALLCLING )
				{
					touchEdgeWithLeftWire = true;
				}
			}
			

			if( rightWire->IsPulling() || rightWire->IsPulling() )
			{
				touchEdgeWithRightWire = tempCollision;
				if( action == WALLCLING )
				{
					touchEdgeWithRightWire = true;
				}
			}

			//note: when reversed you won't cancel on a jump onto a small ceiling. i hope this mechanic is okay
			//also theres a jump && false condition that would need to be changed back
			
			if( tempCollision && minContact.normal.y >= 0 )
			{
				framesInAir = maxJumpHeightFrame + 1;
			}

			if( tempCollision )
			{
				//dont remember why this was here. When doing rail bounces,
				//there is a case where you land and hit the ground and the rail almost at the same time
				//you set a bounce edge and then unset it here, never landing or grounding yourself.
				/*if( bounceEdge != NULL )
				{
					bounceOkay = false;
					bounceEdge = NULL;
					oldBounceEdge = NULL;
					SetAction( JUMP );
					holdJump = false;
					frame = 1;
					break;
				}*/
				V2d en = minContact.normal;
			}

			if (tempCollision && touchedGrass[Grass::BOUNCE] && !bounceFlameOn)
			{
				if (bounceEdge == NULL)
				{
					bounceEdge = minContact.edge;
					bounceNorm = minContact.normal;
				}
				//HandleBounceGrass();
			}
			else if (tempCollision && minContact.edge->rail != NULL
				&& minContact.edge->rail->GetRailType() == TerrainRail::BOUNCE)
			{
				if (bounceEdge == NULL)
				{
					bounceEdge = minContact.edge;
					bounceNorm = minContact.normal;
				}
				//HandleBounceRail();
			}
			else if (tempCollision && action == FREEFLIGHTSTUN )
			{
				velocity = newVel;
			}
			else if (tempCollision && ( action == WATERGLIDE || action == WATERGLIDE_HITSTUN || action == WATERGLIDECHARGE ))
			{
				velocity = newVel;
			}
			else if (tempCollision && InWater( TerrainPolygon::WATER_BUOYANCY ) && minContact.normal.y <= 0)
			{
				//reverse grav water
				velocity = newVel;
			}

			/*else if (tempCollision && grassCount[Grass::UNTECHABLE] > 0) 
			{
				//just a test.
				//needs an animation/lag frames after bouncing if you want to use this.
				//would be interesting in some contexts though. 
				//might use the bounce grass for this.

				if (velocity.x > 0)
				{
					velocity.x += 10;
				}
				else if (velocity.x < 0)
				{
					velocity.x -= 10;
				}

				velocity.x = -velocity.x;
			}*/
			else if (tempCollision && action == SPRINGSTUNBOUNCE)
			{
				//not sure if this ever even gets called

				//needs work later probably
				V2d newDir = minContact.edge->GetReflectionDir(normalize(springVel));
				
				V2d inputDir = currInput.GetLeft8Dir();

				if (inputDir.x != 0 || inputDir.y != 0)
				{
					double inputFactor = .3;
					newDir = normalize(newDir + inputDir * inputFactor);
				}
				springVel = newDir * length(springVel);

				if (springVel.x > 0)
				{
					facingRight = true;
				}
				else if( springVel.x < 0 )
				{
					facingRight = false;
				}

				velocity = springVel;

				break;
			}
			else if (tempCollision && action == SWINGSTUN)
			{
				if (action == BOUNCEAIR || action == BOUNCEGROUND || bounceFlameOn)
				{
					BounceCollision(movement);
					break;
				}
				else
				{
					velocity = newVel;

					if (minContact.normal.y >= 0)
					{
						if (minContact.normal.y > 0 && DefaultGravReverseCheck())
						{
							DefaultCeilingLanding(movement);
						}
						else
						{
							SetAction(JUMP);
							frame = 1;
						}
					}
					else
					{
						DefaultGroundLanding(movement);
					}
				}
			}
			else if (tempCollision && action == SPRINGSTUNAIM)
			{
				SetAction(SPRINGSTUNBOUNCEGROUND);

				bounceEdge = minContact.edge;
				bounceNorm = minContact.normal;
				edgeQuantity = bounceEdge->GetQuantity(minContact.position);

				framesSinceGrindAttempt = maxFramesSinceGrindAttempt;

				if (minContact.normal.x > 0)
				{
					offsetX = b.rw;
				}
				else if (minContact.normal.x < 0)
				{
					offsetX = -b.rw;
				}
				else
				{
					offsetX = (position.x + b.offset.x) - minContact.position.x;
				}

				//offsetX = (position.x + b.offset.x) - minContact.position.x;

				if (b.rh < normalHeight)
				{
					if (minContact.normal.y > 0)
						b.offset.y = -(normalHeight - b.rh);
					else if (minContact.normal.y < 0)
						b.offset.y = (normalHeight - b.rh);
				}
				else
				{
					b.offset.y = 0;
				}
				//needs work later probably
				V2d newDir = minContact.edge->GetReflectionDir(normalize(springVel));

				springVel = newDir * length(springVel);

				if (springVel.x > 0)
				{
					facingRight = true;
				}
				else if (springVel.x < 0)
				{
					facingRight = false;
				}

				velocity = springVel;

				V2d alongVel = V2d(-minContact.normal.y, minContact.normal.x);

				V2d bn = bounceEdge->Normal();

				V2d testVel = velocity;

				groundSpeed = CalcLandingSpeed(testVel, alongVel, bn);

				aimLauncherStunFrames = springStunFrames;

				break;
			}
			else if (tempCollision && action == SPRINGSTUNGRINDFLY)
			{
				prevRail = NULL;
				Edge *e = minContact.edge;
				V2d mp = minContact.position;
				double q = e->GetQuantity(mp);
				ground = e;
				edgeQuantity = q;

				groundSpeed = dot(velocity, e->Along());

				if (approxEquals(groundSpeed, 0.0))
				{
					if (facingRight)
					{
						groundSpeed = 1.0;
					}
					else
					{
						groundSpeed = -1.0;
					}
				}

				/*if (e->Normal().x > 0)
				{
					groundSpeed = velocity.y;
				}
				else
				{
					groundSpeed = -velocity.y;
				}*/

				SetActionGrind();
			}
			else if( ( action == BOUNCEAIR || action == BOUNCEGROUND || bounceFlameOn ) && tempCollision )
			{
				BounceCollision(movement);
				break;
			}
			else if( ((action == JUMP && /*!holdJump*/false) || ( framesInAir > maxJumpHeightFrame || velocity.y > -8 || !holdJump ) || action == WALLCLING || action == WALLATTACK ) && tempCollision && minContact.normal.y < 0 && abs( minContact.normal.x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			{
				DefaultGroundLanding(movement);
			}
			else if( tempCollision && DefaultGravReverseCheck() )
			{
				//position += minContact.resolution;

				DefaultCeilingLanding(movement);
				break;
			}
			else if(!touchedGrass[Grass::ANTIGRIND] 
				&& tempCollision //&& currPowerMode == PMODE_GRIND 
				&& !InWater(TerrainPolygon::WATER_INVERTEDINPUTS)
				&& IsOptionOn(POWER_GRIND)
				&& CanBufferGrind()//PowerButtonHeld()
				&& velocity.y != 0 //remove this soon
				&& abs( minContact.normal.x ) >= wallThresh 
				&& !minContact.edge->IsInvisibleWall()  )
			{
				prevRail = NULL;
				Edge *e = minContact.edge;
				V2d mp = minContact.position;
				double q = e->GetQuantity( mp );
				ground = e;
				edgeQuantity = q;

				if( e->Normal().x > 0 )
				{
					groundSpeed = velocity.y;
				}
				else
				{
					groundSpeed = -velocity.y;
				}
				
				SetActionGrind();
			}
			else if( tempCollision )
			{
				velocity = newVel;
			}
			else
			{
				//cout << "no temp collision" << endl;
			}

			if( length( extraVel ) > 0 )
			{
				movementVec = stealVec + extraVel;
			}

			else
			{
				movementVec = stealVec;
			}


		}
	}
	while( (ground != NULL && !approxEquals( movement, 0 ) ) || ( ground == NULL && length( movementVec ) > 0 ) );


	PhysicsResponse();
}

bool Actor::CheckSwing()
{
	return CheckNormalSwing();
}

bool Actor::CheckNormalSwingHeld()
{
	return currInput.rightShoulder;
}

bool Actor::CheckSwingHeld()
{
	return CheckNormalSwingHeld();
}

bool Actor::CheckNormalSwing()
{
	bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
	return normalSwing;
}

void Actor::ResetSuperLevel()
{
	superLevelCounter = 0;
}

void Actor::SetActionSuperLevel()
{
	currActionSuperLevel = superLevelCounter;
}

bool Actor::IsGroundAttack(int a)
{
	return a == Action::STANDATTACK1
		|| a == Action::STANDATTACK2
		|| a == Action::STANDATTACK3
		|| a == Action::STANDATTACK4
		|| a == Action::DASHATTACK
		|| a == Action::DASHATTACK2 
		|| a == Action::DASHATTACK3
		|| a == Action::UPTILT1
		|| a == Action::UPTILT2
		|| a == Action::UPTILT3;
}

bool Actor::TryGroundAttack()
{
	bool normalSwing = CheckNormalSwing();

	if ( normalSwing || IsGroundAttackAction(pauseBufferedAttack) )
	{
		V2d groundNorm = ground->Normal();
		if (IsOnSteepGround())
		{
			if (groundNorm.x > 0)
			{
				if (facingRight)
				{
					SetAction(STEEPSLIDEATTACK);
				}
				else
				{
					SetAction(STEEPCLIMBATTACK);
				}
			}
			else
			{
				if (facingRight)
				{
					SetAction(STEEPCLIMBATTACK);
				}
				else
				{
					SetAction(STEEPSLIDEATTACK);
				}
			}
		}
		else
		{
			if (currInput.LLeft())
			{
				facingRight = false;
			}
			else if (currInput.LRight())
			{
				facingRight = true;
			}

			if (action == DASH || ((action == DASHATTACK
				|| action == DASHATTACK2 || action == DASHATTACK3) && DashButtonHeld()
				&& (currInput.LLeft() || currInput.LRight())))
			{
				SetAction(GetCurrDashAttack());
			}
			//else if (currInput.LUp())
			//{
			//	SetAction(GetCurrUpTilt());
			//}
			else
			{
				SetAction(GetCurrStandAttack());
			}
		}

		frame = 0;
		

		return true;
	}

	return false;
	
	//if( currInput.B )//action == DASH )
	//{
	//	
	//}
	//else
	//{
	//	action = DASHATTACK;
	//	frame = 0;
	//}
}

bool Actor::TryGroundAttackCancelDash()
{
	if (pauseBufferedDash || DashButtonPressed())
	{
		if (IsOnSteepGround())
		{
			V2d norm = GetGroundedNormal();

			//up and away from a steep slope shouldn't make you climb out of a slide!
			if (groundSpeed < 0 && norm.x < 0 && (currInput.LRight() || HoldingRelativeUp()) && !currInput.LLeft() && !HoldingRelativeDown())
			{
				SetAction(STEEPCLIMB);
				if (groundSpeed < steepClimbBoostStart)
				{
					groundSpeed = steepClimbBoostStart;
				}
				
				//groundSpeed = 0;//steepClimbBoostStart;
				//groundSpeed = steepClimbBoostStart;
				frame = 0;

				return true;
			}
			else if (groundSpeed > 0 && norm.x > 0 && (currInput.LLeft() || HoldingRelativeUp()) && !currInput.LRight() && !HoldingRelativeDown())
			{
				SetAction(STEEPCLIMB);

				if (groundSpeed > -steepClimbBoostStart)
				{
					groundSpeed = -steepClimbBoostStart;
				}
				
				//groundSpeed = 0;//-steepClimbBoostStart;
				//groundSpeed = -steepClimbBoostStart;
				frame = 0;

				return true;
			}
		}
		else
		{
			if ( currAttackHit && standNDashBoostCurr == 0)
			{
				standNDashBoost = true;
				standNDashBoostCurr = standNDashBoostCooldown;
			}
			SetAction(DASH);
			frame = 0;
			return true;
		}
	}

	return false;
}

void Actor::HandleTouchedGate()
{ 
	if (simulationMode)
	{
		return;
	}

	if (gateTouched == NULL)
		return;

	Gate *g = gateTouched;

	bool activate = false;

	V2d A = b.GetQuadVertex(0);//(b.globalPosition.x - b.rw, b.globalPosition.y - b.rh);
	V2d B = b.GetQuadVertex(1);//(b.globalPosition.x + b.rw, b.globalPosition.y - b.rh);
	V2d C = b.GetQuadVertex(2);// (b.globalPosition.x + b.rw, b.globalPosition.y + b.rh);
	V2d D = b.GetQuadVertex(3);// (b.globalPosition.x - b.rw, b.globalPosition.y + b.rh);

	
	Edge *myZoneEdge = NULL;
	Zone *currZone = sess->currentZone;
	Zone *newZone = NULL;
	
	if (currZone == g->zoneA)
	{
		myZoneEdge = g->edgeA;
	}
	else if (currZone == g->zoneB)
	{
		myZoneEdge = g->edgeB;
	}
	else
	{
		assert(0);
	}

	V2d nEdge = myZoneEdge->Normal();//normalize( edge->v1 - edge->v0 );
	double ang = atan2(nEdge.x, -nEdge.y);

	double alongAmount = dot(b.globalPosition - myZoneEdge->v0, normalize(myZoneEdge->v1 - myZoneEdge->v0));
	alongAmount /= length(myZoneEdge->v1 - myZoneEdge->v0);

	

	if (currZone == g->zoneA)
	{
		newZone = g->zoneB;
	}
	else
	{
		newZone = g->zoneA;
	}

	
	bool pointsInNewZone = false;

	bool aNewZone = newZone->ContainsPoint(A);
	bool bNewZone = newZone->ContainsPoint(B);
	bool cNewZone = newZone->ContainsPoint(C);
	bool dNewZone = newZone->ContainsPoint(D);

	

	if (aNewZone && bNewZone && cNewZone && dNewZone )
	{
		pointsInNewZone = true;
	}

	if (pointsInNewZone )//&& !pointsInCurrZone )
	{
		/*Zone *currZone = sess->currentZone;
		Zone *newZone = NULL;

		if (currZone == g->zoneA)
		{
			newZone = g->zoneB;
		}
		else
		{
			newZone = g->zoneA;
		}*/
		bool currIsSubOfNew = false;
		for (auto it = newZone->subZones.begin(); it != newZone->subZones.end(); ++it)
		{
			if ((*it) == currZone)
			{
				currIsSubOfNew = true;
				break;
			}
		}

		if (currIsSubOfNew)
		{
			bool anyPointsInCurrZone = false;
			bool aCurrZone = currZone->ContainsPoint(A);
			bool bCurrZone = currZone->ContainsPoint(B);
			bool cCurrZone = currZone->ContainsPoint(C);
			bool dCurrZone = currZone->ContainsPoint(D);

			if (aCurrZone || bCurrZone || cCurrZone || dCurrZone)
			{
				anyPointsInCurrZone = true;
			}

			if (!anyPointsInCurrZone)
			{
				activate = true;
			}
		}
		else
		{
			activate = true;
		}
		

		/*bool isInNewZone = newZone->ContainsPoint(position);
		bool isInCurrZone = currZone->ContainsPoint(position);

		bool newIsSubOfOld = false;
		for (auto it = currZone->subZones.begin(); it != currZone->subZones.end(); ++it)
		{
			if ((*it) == newZone)
			{
				newIsSubOfOld = true;
				break;
			}
		}

		if (newIsSubOfOld)
		{
			if (isInNewZone)
			{
				activate = true;
			}
			else
			{
				activate = false;
			}
		}
		else
		{
			if (isInNewZone && !isInCurrZone)
			{
				activate = true;
			}
			else
			{
				activate = false;
			}
		}*/
	}


	//g->SetLocked(true);

	if (grindEdge != NULL)
	{
		

		Edge *otherEdge;
		if (myZoneEdge == g->edgeA)
			otherEdge = g->edgeB;
		else
			otherEdge = g->edgeA;

			//this could be buggy, as it seems like myZoneEdge used to be the opposite edge for some reason?
		if (grindEdge == otherEdge->edge0 || grindEdge == otherEdge->edge1)
		{
			//this could be flawed. needs more testing

			activate = true;
		}
	}

	//g->SetLocked(false);

	if (activate)
	{

		RelEffectInstance params;
		Transform tr = sf::Transform::Identity;
		int s = 1;
		if (!facingRight)
		{
			tr.scale(Vector2f(-s, s));
		}
		else
		{
			tr.scale(Vector2f(s, s));
		}
		params.SetParams(Vector2f(0, 0), tr, 8, 2, 0, &spriteCenter);
		//fair should be 25 but meh

		//gateBlackFX = (RelEffectInstance*)gateBlackFXPool->ActivateEffect(&params);

		ActivateEffect(PLAYERFX_GATE_BLACK, &params);

		//lock all the gates from this zone now that I chose one


		g->PassThrough(alongAmount);

		if (g->IsZoneType())
		{

			//owner->SuppressEnemyKeys(g);

			Zone *oldZone = NULL;
			Zone *newZone = NULL;
			if (myZoneEdge == g->edgeA)
			{
				oldZone = g->zoneA;
				newZone = g->zoneB;
			}
			else
			{
				oldZone = g->zoneB;
				newZone = g->zoneA;
			}

			if (oldZone != newZone)
			{

				bool twoWay = g->IsTwoWay();
				if (!twoWay) //for secret gates
				{
					if (oldZone != NULL && oldZone->IsActive())
					{
						oldZone->ReformAllGates(g);
					}
				}
				sess->ActivateZone(newZone);

				if (!twoWay) //for secret gates
				{
					if (sess->hud != NULL && sess->hud->hType == HUD::ADVENTURE && !sess->IsParallelSession() )
					{
						sess->SetKeyMarkerToCurrentZone();
						//AdventureHUD *ah = (AdventureHUD*)sess->hud;
						//ah->keyMarker->Reset();
					}
				}
				
				if (!twoWay)
				{
					CreateKeyExplosion(g->category);
					sess->absorbDarkParticles->KillAllActive();
					numKeysHeld = 0;
				}
				else
				{
					CreateGateExplosion(g->category);
				}

				RestoreAirOptions();

				ClearActiveComboObjects();
			}
		}

		//V2d gEnterPos = alongPos;// +nEdge;// *32.0;

		V2d enterPos = myZoneEdge->v1 + normalize(myZoneEdge->v0 - myZoneEdge->v1) * (1.0-alongAmount) * myZoneEdge->GetLength()
			+ nEdge * 0.0; //could change this for offset along the normal

		ActivateEffect(PLAYERFX_GATE_ENTER, Vector2f(enterPos), RadiansToDegrees(ang), 8, 3, true);

		//set gate action to disperse
		//maybe have another gate action when you're on the gate and its not sure whether to blow up or not
		//it only enters this state if you already unlock it though

		//cout << "gatetouched is null from passing through a gate. was " << (int)gateTouched << "\n";

		gateTouched = NULL;
		
	}
	else
	{
		//if( g->edgeA->)
		if (!aNewZone && !bNewZone && !cNewZone && !dNewZone)
		{
			//if( IsEdgeTouchingBox( g->edgeA, )

			/*bool aCurrZone = currZone->ContainsPoint(A);
			bool bCurrZone = currZone->ContainsPoint(B);
			bool cCurrZone = currZone->ContainsPoint(C);
			bool dCurrZone = currZone->ContainsPoint(D);*/

			/*if ( aCurrZone && bCurrZone && 
			{
				gateTouched = NULL;
				sess->LockGate(g);
			}*/

			bool allPointsInCurrZone = false;
			bool aCurrZone = currZone->ContainsPoint(A);
			bool bCurrZone = currZone->ContainsPoint(B);
			bool cCurrZone = currZone->ContainsPoint(C);
			bool dCurrZone = currZone->ContainsPoint(D);

			if (aCurrZone && bCurrZone && cCurrZone && dCurrZone)
			{
				allPointsInCurrZone = true;
			}

			if (allPointsInCurrZone)
			{
				//cout << "gatetouched is null from all points in curr zone. was " << (int)gateTouched << "\n";
				gateTouched = NULL;
				//sess->LockGate(g);
			}
			//might need extra checks at some point
		}
		//cout << "went back" << endl;
		

	}
}

bool Actor::CanTech()
{
	return (framesSinceBlockPress >= 0 && framesSinceBlockPress < 20
		&& !touchedGrass[Grass::UNTECHABLE]);
}

bool Actor::InWater(int wType)
{
	return currSpecialTerrain != NULL && wType == currSpecialTerrain->waterType;
}

int Actor::MostRecentFrameCurrAttackBlocked()
{
	int maxFrame = -1;
	for (int i = 0; i < 4; ++i)
	{
		if (currAttackHitBlock[i] >= maxFrame)
			maxFrame = currAttackHitBlock[i];
	}

	return maxFrame;
}

void Actor::HitOutOfCeilingGrindAndReverse()
{
	V2d grindNorm = grindEdge->Normal();
	//	velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
	if (grindNorm.x > 0)
	{
		offsetX = b.rw;
	}
	else if (grindNorm.x < 0)
	{
		offsetX = -b.rw;
	}
	else
	{
		offsetX = 0;
	}

	RestoreAirOptions();


	ground = grindEdge;
	groundSpeed = -grindSpeed;
	grindEdge = NULL;
	reversed = true;

	hurtBody.isCircle = false;
	hurtBody.rw = 7;
	hurtBody.rh = normalHeight;

	SetAction(GROUNDHITSTUN);
	frame = 0;

	if (toggleGrindInput)
	{
		currInput.Y = false; //need to go over toggling again and make sure it works.
	}

	if (receivedHit.knockback > 0)
	{
		groundSpeed = receivedHit.GetKnockbackVector().x;
	}
	else
	{
		groundSpeed *= (1 - receivedHit.drainX) * abs(grindNorm.y) + (1 - receivedHit.drainY) * abs(grindNorm.x);
	}

	frame = 0;
	framesNotGrinding = 0;

	double angle = GroundedAngle();

	ActivateEffect(PLAYERFX_GRAV_REVERSE, Vector2f(position), RadiansToDegrees(angle), 25, 1, facingRight);
	ActivateSound(PlayerSounds::S_GRAVREVERSE);
}

void Actor::HitOutOfCeilingGrindIntoAir()
{
	framesNotGrinding = 0;
	if (reversed)
	{
		velocity = normalize(grindEdge->v1 - grindEdge->v0) * -grindSpeed;
	}
	else
	{
		velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
	}


	//SetAction( JUMP );
	SetAction(AIRHITSTUN);
	frame = 0;
	if (receivedHit.knockback > 0)
	{
		velocity = receivedHit.GetKnockbackVector();
	}
	else
	{
		velocity.x *= (1 - receivedHit.drainX);
		velocity.y *= (1 - receivedHit.drainY);
	}

	if (toggleGrindInput)
	{
		currInput.Y = false;
	}

	hurtBody.isCircle = false;
	hurtBody.rw = 7;
	hurtBody.rh = normalHeight;

	//	frame = 0;
	ground = NULL;
	grindEdge = NULL;
	reversed = false;
}

bool Actor::TryHandleHitWhileRewindBoosted()
{
	if (rewindOnHitFrames > 0)
	{
		SetAirPos(rewindBoosterPos, facingRight);
		receivedHit.hType = HitboxInfo::NO_HITBOX;
		invincibleFrames = 0;
		rightWire->Reset();
		leftWire->Reset();
		return true;
	}

	return false;
}


bool Actor::TryHandleHitInRewindWater()
{
	if (InWater(TerrainPolygon::WATER_REWIND))
	{
		reversed = false;
		ground = NULL;
		grindEdge = NULL;
		bounceEdge = NULL;
		groundSpeed = 0;
		velocity = V2d(0, 0);
		position = waterEntrancePosition;
		b.rh = waterEntrancePhysHeight;
		invincibleFrames = 0;
		receivedHit.hType = HitboxInfo::NO_HITBOX;
		rightWire->Reset();
		leftWire->Reset();

		if (waterEntranceGrindEdge != NULL)
		{
			//a little buggy but not in any major ways
			facingRight = waterEntranceFacingRight;

			V2d grNormal = waterEntranceGrindEdge->Normal();
			if (grNormal.y == 0 && grNormal.x < 0)
			{
				facingRight = !facingRight;
			}

			ground = waterEntranceGrindEdge;
			edgeQuantity = waterEntranceQuantity;
			position = waterEntrancePosition;
			SetActionGrind();
		}
		else if (waterEntranceGround != NULL)
		{
			reversed = waterEntranceReversed;
			SetAction(STAND);
			frame = 0;

			facingRight = waterEntranceFacingRight;
			SetGroundedPos(waterEntranceGround, waterEntranceQuantity, waterEntranceXOffset);
		}
		else
		{
			SetAction(JUMP);
			frame = 1;
			RestoreAirOptions();

			velocity = V2d(0, 0);
		}
		return true;
	}

	return false;
}

void Actor::HitOutOfGrind()
{
	if (TryHandleHitInRewindWater())
		return;

	if (TryHandleHitWhileRewindBoosted())
		return;

	V2d grindNorm = grindEdge->Normal();

	framesNotGrinding = 0;
	RestoreAirOptions();
	ground = grindEdge;
	groundSpeed = grindSpeed;

	hurtBody.isCircle = false;
	hurtBody.rw = 7;
	hurtBody.rh = normalHeight;

	SetAction(GROUNDHITSTUN);
	frame = 0;

	if (receivedHit.knockback > 0)
	{
		groundSpeed = receivedHit.GetKnockbackVector().x;
	}
	else
	{
		groundSpeed *= (1 - receivedHit.drainX) * abs(grindNorm.y) + (1 - receivedHit.drainY) * abs(grindNorm.x);
	}

	if (toggleGrindInput)
	{
		currInput.Y = false;
	}

	grindEdge = NULL;
	reversed = false;
}




void Actor::HitWhileGrounded()
{
	if (TryHandleHitInRewindWater())
		return;

	if (TryHandleHitWhileRewindBoosted())
		return;

	SetAction(GROUNDHITSTUN);
	frame = 0;

	if (receivedHit.knockback > 0)
	{
		groundSpeed = receivedHit.GetKnockbackVector().x;
	}
	else
	{
		V2d norm = GetGroundedNormal();
		groundSpeed *= (1 - receivedHit.drainX) * abs(norm.y) + (1 - receivedHit.drainY) * abs(norm.x);
	}
}

V2d Actor::CalcKnockback(HitboxInfo *hit)
{
	double upwardsMult = 2.0;

	V2d knockbackVec;
	if (hit->knockback > 0)
	{
		V2d diff = position - receivedHitPosition;
		if (hit->hitPosType == HitboxInfo::OMNI)
		{
			V2d hitDir = normalize(diff);
			knockbackVec = hitDir * hit->knockback;
			if (knockbackVec.y < 0)
			{
				knockbackVec.y *= upwardsMult;
			}
		}
		else
		{
			knockbackVec = hit->GetKnockbackVector();

			if (receivedHitPlayer == NULL) //enemies only
			{
				if (knockbackVec.y < 0)
				{
					knockbackVec.y *= upwardsMult;
				}

				if (hit->reversableKnockback && ((diff.x < 0 && !hit->flipHorizontalKB)
					|| (diff.x > 0 && hit->flipHorizontalKB)))
				{
					knockbackVec.x = -knockbackVec.x;
				}
			}
		}
	}
	else
	{
		knockbackVec.x *= (1 - hit->drainX);
		knockbackVec.y *= (1 - hit->drainY);
	}

	return knockbackVec;
}

void Actor::HitWhileAerial()
{
	if (TryHandleHitInRewindWater())
		return;
	
	if (TryHandleHitWhileRewindBoosted())
		return;

	if (action == WATERGLIDE || action == WATERGLIDECHARGE)
	{
		SetAction(WATERGLIDE_HITSTUN);
	}
	else
	{
		SetAction(AIRHITSTUN);
	}
	
	frame = 0;

	ground = NULL;
	grindEdge = NULL;
	bounceEdge = NULL;


	velocity = CalcKnockback(&receivedHit);
}

void Actor::ApplyBlockFriction()
{
	double friction = .5;
	if (groundSpeed > 0)
	{
		groundSpeed -= friction;
		if (groundSpeed < 0)
			groundSpeed = 0;
	}
	else if( groundSpeed < 0 )
	{
		groundSpeed += friction;
		if (groundSpeed > 0)
			groundSpeed = 0;
	}
}

void Actor::SlideOffWhileInGroundHitstun()
{
	SetAction(AIRHITSTUN);
	frame = 0;
}

void Actor::HitWallWhileInAirHitstun()
{
	if (CanTech())
	{
		SetAction(WALLTECH);
		frame = 0;
		physicsOver = true;
	}
	else
	{
		if ((wallNormal.x > 0 && oldVelocity.x < 0)
			|| (wallNormal.x < 0 && oldVelocity.x > 0))
		{
			velocity.x = -oldVelocity.x;
			physicsOver = true;
		}
	}
}

void Actor::HitGroundWhileInAirHitstun()
{
	if (CanTech())
	{
		bool forward = (facingRight && currInput.LRight())
			|| (!facingRight && currInput.LLeft());
		bool back = (facingRight && currInput.LLeft()) ||
			(!facingRight && currInput.LRight());

		if (forward)
		{
			SetAction(GROUNDTECHFORWARD);
		}
		else if (back)
		{
			SetAction(GROUNDTECHBACK);
		}
		else
		{
			SetAction(GROUNDTECHINPLACE);
		}

		frame = 0;
		physicsOver = true;
	}
	else
	{
		V2d gNormal = ground->Normal();


		double angle = GroundedAngle();

		if (angle == 0)
		{
			gNormal = V2d(0, -1);
		}

		//guessing this will apply to reversed but havent tested.
		if ( (!reversed && velocity.y < 10) || (reversed && velocity.y > -10 ))
		{
			SetAction(GROUNDHITSTUN);
			frame = 0;
			physicsOver = true;
		}
		else
		{
			//cout << "bouncing off ground \n";
			double d = dot(velocity, gNormal);
			V2d oldVel = velocity;
			velocity = velocity - (2.0 * d * gNormal);

			velocity.x *= .7; //dont want player bouncing far

			double bounceCap = 20;

			if (velocity.y < -bounceCap)
			{
				velocity.y = -bounceCap;
			}
			//velocity.y = -velocity.y;
			ground = NULL;
			physicsOver = true;
		}
	}
}

void Actor::SetTouchedGate(Gate *g)
{
	if (gateTouched == g)
	{
		return;
	}

	//cout << "setting gate touched to: " << (int)g << "\n";
	gateTouched = g;
}

void Actor::TryHitPlayer(int targetIndex)
{
	if (targetIndex == actorIndex)
		return; //its me!

	if (currHitboxes != NULL && !simulationMode)
	{
		Actor *pTarget = NULL;
		int target = 0;
		pTarget = sess->GetPlayer(targetIndex);

		if (pTarget == NULL)
			return;

		HitResult checkHit = HitResult::MISS;

		HitboxInfo &hi = hitboxInfos[action][currActionSuperLevel];


		hi.damage = 10; //just testing for now

		if (currAttackHitBlock[target] < 0)
		{
			checkHit = pTarget->CheckIfImHit(currHitboxes, currHitboxFrame,
				hi.hitPosType, position, facingRight,
				hi.canBeParried, hi.canBeBlocked);
		}

		if (checkHit != HitResult::MISS && checkHit != HitResult::INVINCIBLEHIT)
		{
			*currVSHitboxInfo = hi;

			currVSHitboxInfo->flipHorizontalKB = !facingRight;
			//if (!facingRight)
			//	currVSHitboxInfo->kbDir.x = -currVSHitboxInfo->kbDir.x;

			attackingHitlag = true;
			hitlagFrames = currVSHitboxInfo->hitlagFrames;
			pTarget->ApplyHit(currVSHitboxInfo, this, checkHit, position);

			if (checkHit == HitResult::HIT)
			{
				//need to work these in later for hitlag, they are only here for testing for now.
				if (hasHitRechargeDoubleJump && !hasDoubleJump)
				{
					hasDoubleJump = true;
					hasHitRechargeDoubleJump = false;
				}
				if (hasHitRechargeAirDash && !hasAirDash)
				{
					hasAirDash = true;
					hasHitRechargeAirDash = false;
				}

				currAttackHit = true;
			}
			else if (checkHit == HitResult::FULLBLOCK
				|| checkHit == HitResult::HALFBLOCK)
			{
				//doesnt currently affect anything!
				shieldPushbackFrames = 30;
				if (pTarget->position.x >= position.x)
				{
					shieldPushbackRight = false;
				}
				else
				{
					shieldPushbackRight = true;
				}

				currAttackHitBlock[target] = frame;
			}
		}
		else
		{

		}

	}
}

void Actor::PhysicsResponse()
{
	V2d gn;
	//Edge *e;

	

	if (IsOnRailAction(action) && action != LOCKEDRAILSLIDE && collision )
	{
		grindEdge = NULL;
		SetAction(JUMP);
		frame = 1;
		framesNotGrinding = 0;
		
		regrindOffCount = 0;
		framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	}
	else if (action == SPRINGSTUNAIRBOUNCE && collision)
	{
		SetAction(JUMP);
		frame = 1;
	}

	wallClimbGravityOn = false;
	if( grindEdge != NULL )
	{
		//e = grindEdge;
		framesInAir = 0;

		V2d oldv0 = grindEdge->v0;
		V2d oldv1 = grindEdge->v1;


		V2d grindPoint = grindEdge->GetPosition( edgeQuantity );

		position = grindPoint;

		if (SpringLaunch()) return;

		if (ScorpionLaunch()) return;

		if (TeleporterLaunch())return;

		if (SwingLaunch())return;

		if (AimLauncherAim()) return;


		bool leaveGround = false;
		if (grindEdge->IsGateEdge())
		{
			Gate *g = (Gate*)grindEdge->info;

			if (CanUnlockGate(g))
			{
				SetTouchedGate(g);

				UnlockGate(g);

				SetAction(JUMP);
				frame = 1;

				framesInAir = 0;
				holdJump = false;
				
				velocity = grindSpeed * normalize(grindEdge->v1 - grindEdge->v0);

				//velocity = V2d( 0, 0 );
				leaveGround = true;
				ground = NULL;
				grindEdge = NULL;
				reversed = false;
				//return;

			}
		}
	}
	else if( bounceEdge != NULL )
	{
		//e = bounceEdge;
		V2d bn = bounceNorm;

		if( action != BOUNCEGROUND ) //added this line for testing
		if( action == BOUNCEAIR || bounceFlameOn )
		{
			physicsOver = true;
			//cout << "BOUNCING HERE" << endl;

			storedBounceVel = velocity;
			//scorpOn = false;

			SetAction(BOUNCEGROUND);
			frame = 0;
			groundSpeed = 0;

			if (bn.y < 0)
			{
				RestoreAirOptions();
			}

			if( bn.y <= 0 && bn.y > -steepThresh )
			{
				//RestoreAirOptions();
				if( storedBounceVel.x > 0 && bn.x < 0 && facingRight 
					|| storedBounceVel.x < 0 && bn.x > 0 && !facingRight )
				{
					facingRight = !facingRight;
				}
			}
			else if( bn.y >= 0 && -bn.y > -steepThresh )
			{
				if( storedBounceVel.x > 0 && bn.x < 0 && facingRight 
					|| storedBounceVel.x < 0 && bn.x > 0 && !facingRight )
				{
					facingRight = !facingRight;
				}
			}
			else if( bn.y == 0  )
			{
				facingRight = !facingRight;
			}
			//else if( bn.y < 0 )
			//{
			//	RestoreAirOptions();


			//	//if( abs( storedBounceVel.y ) < 10 )
			//	//{
			//	//	//cout << "land: " << abs(storedBounceVel.y) << endl;
			//	//	BounceFlameOn();
			//	//	
			//	//	SetAction(LAND);
			//	//	frame = 0;
			//	//	//bounceEdge = NULL;
			//	//	ground = bounceEdge;
			//	//	bounceEdge = NULL;
			//	//	//oldBounceEdge = NULL;
			//	//}
			//}

			if( bn.y != 0 )
			{
				if( bounceEdge != NULL )
				{
					V2d oldv0 = bounceEdge->v0;
					V2d oldv1 = bounceEdge->v1;

					
					position = bounceEdge->GetPosition( edgeQuantity );

				}
				else
				{
					V2d oldv0 = ground->v0;
					V2d oldv1 = ground->v1;


					position = ground->GetPosition(edgeQuantity);
				}
		
				position.x += offsetX + b.offset.x;

				if( bn.y > 0 )
				{
					{
						position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}
				else
				{
					if( bn.y < 0 )
					{
						position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}
			}
		}
		else if (action == SPRINGSTUNBOUNCEGROUND)
		{

		}
		else
		{
			if (bounceEdge->rail != NULL)
			{
				HandleBounceRail();
			}
			else
			{
				HandleBounceGrass();
			}
		}
	}
	else if( ground != NULL )
	{
		if (SpringLaunch()) return;

		if (ScorpionLaunch()) return;

		if (TeleporterLaunch())return;

		if (SwingLaunch())return;

		if (AimLauncherAim()) return;

		//e = ground;
		bool leaveGround = false;
		if( ground->IsGateEdge() )
		{
			Gate *g = (Gate*)ground->info;
				
			if( CanUnlockGate( g ) )
			{
				SetTouchedGate(g);

				UnlockGate( g );

				//this is the same code as down below in !leaveGround section,
				//not a huge deal to have it duplicated rn imo. Just gonna leave it.
				//could potentially cl ean up a little bit later
				gn = ground->Normal();

				Vector2<double> groundPoint = ground->GetPosition(edgeQuantity);

				position = groundPoint;

				position.x += offsetX + b.offset.x;

				if (reversed)
				{
					if (gn.y > 0 || abs(offsetX) != b.rw)
					{
						position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}
				else
				{
					if (gn.y < 0 || abs(offsetX) != b.rw)
					{
						position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}

				if (reversed)
				{
					if ((action == STEEPCLIMB || action == STEEPSLIDE) && (-gn.y <= -steepThresh || !approxEquals(abs(offsetX), b.rw)))
					{
						SetAction(LAND2);
						frame = 0;
					}
				}
				else
				{

					if ((action == STEEPCLIMB || action == STEEPSLIDE) && (gn.y <= -steepThresh || !approxEquals(abs(offsetX), b.rw)))
					{
						//cout << "here no: " << action << ", " << offsetX << endl;
						SetAction(LAND2);
						frame = 0;
					}
					else
					{

					}
				}

				SetAction( JUMP );
				frame = 1;
				
				framesInAir = 0;
				holdJump = false;
				if (reversed)
				{
					velocity = -groundSpeed * normalize(ground->v1 - ground->v0);
				}
				else
				{
					velocity = groundSpeed * normalize(ground->v1 - ground->v0);
				}
				
				//velocity = V2d( 0, 0 );
				leaveGround = true;
				ground = NULL;
				reversed = false;
				//return;

			}
		}

		if (!leaveGround)
		{

			gn = ground->Normal();
			if (collision)
			{
				if (action == AIRHITSTUN)
				{
					HitGroundWhileInAirHitstun();
				}
				else if (action == BOOSTERBOUNCE)
				{
					int currFrame = frame;
					SetAction(BOOSTERBOUNCEGROUND);
					physicsOver = true;
					//current frame of boosterbounce stays the same, 
					//just transition the action
					frame = currFrame;
					
					//frame = 0;
				}
				else if (action == RAILBOUNCE)
				{
					int currFrame = frame;
					SetAction(RAILBOUNCEGROUND);
					//physicsOver = true;

					//current frame of boosterbounce stays the same, 
					//just transition the action
					frame = currFrame;
				}
				else if (!IsHitstunAction(action) && action != LAND2 && action != LAND
					&& action != SEQ_CRAWLERFIGHT_STRAIGHTFALL
					&& action != SEQ_CRAWLERFIGHT_LAND
					&& action != SEQ_CRAWLERFIGHT_DODGEBACK && action != GRAVREVERSE
					&& action != JUMPSQUAT && action != WATERGLIDE && action != WATERGLIDE_HITSTUN && action != WATERGLIDECHARGE 
					&& action != RAILBOUNCEGROUND  && action != BOOSTERBOUNCEGROUND )
				{
					storedBounceVel = velocity;

					if (TryBufferGrind())
					{
						holdJump = false;
						holdDouble = false;
						currBBoostCounter = 0;
					}
					else
					{
						//only testing railbounceground but could probably apply to boosterbounceground too, so added that
						if (currInput.LLeft() || currInput.LRight())
						{
							SetAction(LAND2);
							ActivateSound(PlayerSounds::S_LAND);
							frame = 0;
						}
						else
						{
							if (reversed)
							{
								SetAction(GRAVREVERSE);

								if (currInput.LLeft() || currInput.LRight())
								{
									storedReverseSpeed = 0;
								}
								else
								{
									storedReverseSpeed = -groundSpeed;
								}
							}
							else
							{

								SetAction(LAND);
								ActivateSound(PlayerSounds::S_LAND);
							}
							frame = 0;
						}
					}
				}
				else if (action == SEQ_CRAWLERFIGHT_STRAIGHTFALL || action == SEQ_CRAWLERFIGHT_DODGEBACK)
				{
					//cout << "action = 41" << endl;
					SetAction(SEQ_CRAWLERFIGHT_LAND);
					frame = 0;
					groundSpeed = 0;
				}
			}
			framesInAir = 0;

			//need to check this again because it might bounce off the ground 
			//while in hitstun, etc
			if (ground != NULL)
			{


				Vector2<double> groundPoint = ground->GetPosition(edgeQuantity);

				position = groundPoint;

				position.x += offsetX + b.offset.x;

				if (reversed)
				{
					if (gn.y > 0 || abs(offsetX) != b.rw)
					{
						position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}
				else
				{
					if (gn.y < 0 || abs(offsetX) != b.rw)
					{
						position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					}
				}

				if (reversed)
				{
					if ((action == STEEPCLIMB || action == STEEPSLIDE) && (-gn.y <= -steepThresh || !approxEquals(abs(offsetX), b.rw)))
					{
						SetAction(LAND2);
						frame = 0;
					}
				}
				else
				{

					if ((action == STEEPCLIMB || action == STEEPSLIDE) && (gn.y <= -steepThresh || !approxEquals(abs(offsetX), b.rw)))
					{
						//cout << "here no: " << action << ", " << offsetX << endl;
						SetAction(LAND2);
						frame = 0;
					}
					else
					{

					}
				}

			}
		}
		
	}
	else
	{
		if (SpringLaunch()) return;

		if (ScorpionLaunch()) return;

		if (TeleporterLaunch())return;

		if (SwingLaunch())return;

		if (AimLauncherAim()) return;

		if( action == GROUNDHITSTUN )
		{
			SlideOffWhileInGroundHitstun();
		}

		if (action == AIRHITSTUN)
		{
			if (collision)
			{
				if (length(wallNormal) > 0)
				{
					HitWallWhileInAirHitstun();
				}
				else if (gn.y > 0 && ground != NULL)
				{
					
				}
			}
		}
		else if( !IsAirHitstunAction(action) && action != AIRDASH && !IsActionAirBlock( action ) && action != WATERGLIDE
			&& action != WATERGLIDE_HITSTUN && action != WATERGLIDECHARGE
			&& action != FREEFLIGHTSTUN && action != DIAGUPATTACK && action != AIRDASHFORWARDATTACK && action != LOCKEDRAILSLIDE )
		{
			if (collision && touchedGrass[Grass::DECELERATE] && length( wallNormal ) > 0 && currWall != NULL )
			{
				//if (length(wallNormal) > 0
					//&& (currWall == NULL || !currWall->IsInvisibleWall())
					//&& oldVelocity.y >= 0 && !(currWall != NULL && currWall->rail != NULL && currWall->rail->GetRailType() == TerrainRail::BOUNCE))
				//double velLen = length(velocity);
				double velY = velocity.y;

				double dSpeed = GetDashSpeed();
				double maxGSpeed = 5;
				double decel = 2.0;
				double maxSlowSpeed = dSpeed + 5;
				if (velY > maxSlowSpeed)
				{
					velY -= decel;
				}
				else if (velY < -maxSlowSpeed)
				{
					velY += decel * .2;
				}
				else if (velY > maxGSpeed)
				{
					velY = maxGSpeed;
				}
				/*else if (velY < -maxGSpeed)
				{
					velY = -maxGSpeed;
				}*/

				//velocity = normalize(velocity) * velLen;
				velocity.y = velY;
			}


			if( collision && action != WALLATTACK && action != WALLCLING )
			{
				if( length( wallNormal ) > 0 
					&& (currWall == NULL || !currWall->IsInvisibleWall())
					&& oldVelocity.y >= 0 && !( currWall != NULL && currWall->rail != NULL && currWall->rail->GetRailType() == TerrainRail::BOUNCE ) )
				{
					if( wallNormal.x > 0)
					{
						if( currInput.LLeft() && !currInput.LDown() )
						{
							facingRight = true;
							SetAction(WALLCLING);
							frame = 0;
						}
					}
					else
					{
						if( currInput.LRight() && !currInput.LDown() )
						{
							facingRight = false;
							SetAction(WALLCLING);
							
							frame = 0;
						}
					
					}
				}
			}
			
			else if( oldAction == WALLCLING || oldAction == WALLATTACK )
			{
				bool stopWallClinging = false;
				if( collision && length( wallNormal ) > 0 )
				{
					if( wallNormal.x > 0 )
					{
						if( !currInput.LLeft() || currInput.LDown() )
						{
							//stopWallClinging = true;
						}
					}
					else
					{
						if( !currInput.LRight() || currInput.LDown() )
						{
							//stopWallClinging = true;
						}
					}
				}
				else
				{
					//cout << "zzz: " << (int)collision << endl;//wallNormal.x << ", " << wallNormal.y << endl;
					stopWallClinging = true;
					
				}

				if( stopWallClinging )
				{
					//cout << "stop wall clinging" << endl;
					SetAction( JUMP );
					frame = 1;
					holdJump = false;
				}
			}

			if( leftGround )
			{
				SetAction( JUMP );
				frame = 1;
				holdJump = false;
			}
		}
		else if (!IsAirHitstunAction(action) && action != WALLATTACK && action != AIRDASH)
		{
			//not aware of this getting hit at any time but I'll look into it more later
			if (collision)
			{
				if (length(wallNormal) > 0 && oldVelocity.y <= 0)
				{
					wallClimbGravityOn = true;
				}
			}
		}

		if (collision && minContact.normal.y > 0 && !reversed && action != WALLCLING 
			&& !rightWire->IsPulling() && !leftWire->IsPulling()
			&& action != SEQ_KINFALL && action != WATERGLIDE && action != WATERGLIDE_HITSTUN && action != WATERGLIDECHARGE && action != SPRINGSTUNGLIDE
			&& action != FREEFLIGHT && action != FREEFLIGHTSTUN
			&& !InWater( TerrainPolygon::WATER_BUOYANCY))
			//&& hitCeilingCounter == 0 )
		{
			//hitCeilingCounter = hitCeilingLockoutFrames;
			if (!hitCeilingSoundPlayedThisFrame)
			{
				ActivateSound(PlayerSounds::S_HITCEILING);
				hitCeilingSoundPlayedThisFrame = true;
			}
			

			if (action == SEQ_KINTHROWN)
			{
				SetAction(SEQ_KINFALL);
				frame = 1;
				velocity = V2d(0, 0);
				physicsOver = true;
			}
		}
	}

	

	if( groundedWallBounce && action != BOUNCEGROUNDEDWALL )
	{
		SetAction(BOUNCEGROUNDEDWALL);
		frame = 0;

		//already bounced by here i think
		if( storedBounceGroundSpeed < 0 )
		{
			//cout << "facing right: " << groundSpeed << endl;
			facingRight = true;
		}
		else
		{
			//cout << "facing left: " << groundSpeed << endl;
			facingRight = false;
		}
	}

	UpdateHitboxes();

	HandleTouchedGate();
	
	//multiplayer
	for (int i = 0; i < 4; ++i)
	{
		TryHitPlayer(i);
	}
}

void Actor::SetGroundedSpriteTransform()
{
	double angle = GroundedAngle();
	SetGroundedSpriteTransform(ground, angle );
}

void Actor::SetGroundedSpriteTransform( Edge * e, double angle )
{
	V2d groundP = e->GetPosition(edgeQuantity);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	double factor;
	if (reversed)
	{
		if (angle < 0)
			angle += 2 * PI;
		factor = 1.0 - abs(((angle-PI) / (PI / 6)));
	}
	else
	{
		factor = 1.0 - abs((angle / (PI / 6)));
	}
	
	//cout << "factor: " << factor << "angle: " << angle << endl;
		//if (factor < .9)
		//	factor = 0;
	V2d along = e->Along();
	if (reversed)
	{
		along = -along;
	}
	Vector2f testOffset(along * offsetX * factor);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
	{
		sprite->setPosition(groundP.x + offsetX, groundP.y);
	}
	else
		sprite->setPosition(Vector2f(groundP) + testOffset);
}

V2d Actor::GetGroundAnchor()
{
	double angle = GroundedAngle();
	Edge *e = ground;
	V2d groundP = e->GetPosition(edgeQuantity);

	double factor;
	if (reversed)
	{
		if (angle < 0)
			angle += 2 * PI;
		factor = 1.0 - abs(((angle - PI) / (PI / 6)));
	}
	else
	{
		factor = 1.0 - abs((angle / (PI / 6)));
	}

	//aligns based on slopes
	V2d along = e->Along();
	if (reversed)
	{
		along = -along;
	}
	V2d testOffset(along * offsetX * factor);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
	{
		return V2d(groundP.x + offsetX, groundP.y);
	}
	else
	{
		return groundP + testOffset;
	}
}

void Actor::SetGroundedSpritePos( Edge * e, double angle )
{
	V2d groundP = e->GetPosition(edgeQuantity);

	double factor;
	if (reversed)
	{
		if (angle < 0)
			angle += 2 * PI;
		factor = 1.0 - abs(((angle - PI) / (PI / 6)));
	}
	else
	{
		factor = 1.0 - abs((angle / (PI / 6)));
	}

	//aligns based on slopes
	V2d along = e->Along();
	if (reversed)
	{
		along = -along;
	}
	Vector2f testOffset(along * offsetX * factor);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
	{
		sprite->setPosition(groundP.x + offsetX, groundP.y);
	}
	else
		sprite->setPosition(Vector2f(groundP) + testOffset);
}

void Actor::UpdateHitboxes()
{
	double angle = 0;
	V2d gn;
	V2d gd;

	if( grindEdge != NULL )
	{
		gn = grindEdge->Normal();
		gd = normalize( grindEdge->v1 - grindEdge->v0 );
		angle = atan2( gn.x, -gn.y );
	}
	else if( ground != NULL )
	{	
		if( !approxEquals( abs(offsetX), b.rw ) )
		{
			gn = V2d( 0, -1 );
			gd = V2d( 1, 0 );
			if( reversed )
			{
				//cout << "BLAH BLAH" << endl;
				angle = PI;
				gn = V2d( 0, 1 );
				gd = V2d( -1, 0 );
			}
			//this should never happen
		}
		else
		{
			gn = ground->Normal();
			angle = atan2( gn.x, -gn.y );
			gd = normalize( ground->v1 - ground->v0 );
		}
	}



	if( currHitboxes != NULL )
	{
		vector<CollisionBox> *cList = &(currHitboxes->GetCollisionBoxes(currHitboxFrame));
		if( cList != NULL )
		for( auto it = cList->begin(); it != cList->end(); ++it )
		{
			if( ground != NULL )
			{
				(*it).globalAngle = angle;
			}
			else
			{
				(*it).globalAngle = 0;
			}

			(*it).flipHorizontal = ((!facingRight && !reversed) 
				|| (facingRight && reversed));

			V2d pos = position;
			if( grindEdge != NULL )
			{
				pos = grindEdge->GetPosition( edgeQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
			}
			else if( ground != NULL )
			{
				pos = GetGroundAnchor();//position;//V2d( sprite->getPosition().x, sprite->getPosition().y );
			}

			(*it).globalPosition = pos;
		}
	}
	
	
	if( action == AIRDASH )
	{
		//hurtBody.isCircle = true;
		hurtBody.rw = 10;
		hurtBody.rh = 10;
		//hurtBody.offset = 
	}
	else if( action != GRINDBALL && action != GRINDATTACK )
	{
		if( action == DASH )
		{
			hurtBody.rh = dashHeight;
		}
		else if( action == SPRINT )
		{
			hurtBody.rh = sprintHeight;
		}
		else if( action == STEEPCLIMB )
		{
			hurtBody.rh = 12;
		}
		else if( action == DOUBLE )
		{
			hurtBody.rh = doubleJumpHeight;
		}
		else
		{
			hurtBody.rh = normalHeight;//15;
		}

		hurtBody.isCircle = false;
		hurtBody.rw = 7;
		
		
		//hurtBody.offset = b.offset;
	}

	
	if( grindEdge != NULL )
	{
		hurtBody.globalPosition = grindEdge->GetPosition( edgeQuantity );// + gn * (double)(b.rh);// + hurtBody.rh );
		hurtBody.globalAngle = angle;
	}
	else if( ground != NULL )
	{
		if( gn.x == 0 )
		{
			if (action == DASH)
			{
				int xxxx = 6;
			}
			hurtBody.globalPosition = ground->GetPosition(edgeQuantity) + V2d( offsetX, 0 ) + hurtBody.offset + gn * hurtBody.rh ;//+ V2d( 0, -hurtBody.rh );
			hurtBody.globalAngle = angle;
		}
		else if( gn.y > -steepThresh && !reversed )
		{
			double xoff = 0;
			if( gn.x > 0 )
			{
				xoff = 10;
			}
			else if( gn.x < 0 )
			{
				xoff = -10;
			}

			
			//might not work reversed
			hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + V2d( 0, -1 ) * (double)(b.rh) + V2d( xoff, 0 );// + hurtBody.rh );

			if( action == STEEPCLIMB )
			{
			}
			else if( action == STEEPSLIDE )
			{
				hurtBody.globalPosition += V2d( 0, 15 );
			}
			hurtBody.globalAngle = 0;
			//hurtBody.offset = V2d( 0, 0 );
		}
		else if( -gn.y > -steepThresh && reversed )
		{
			double xoff = 0;
			if( gn.x > 0 )
			{
				xoff = 10;
			}
			else if( gn.x < 0 )
			{
				xoff = -10;
			}

			
			//might not work reversed
			hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + V2d( 0, 1 ) * (double)(b.rh) + V2d( xoff, 0 );// + hurtBody.rh );

			if( action == STEEPCLIMB )
			{
			}
			else if( action == STEEPSLIDE )
			{
				hurtBody.globalPosition -= V2d( 0, 15 );
			}
			hurtBody.globalAngle = PI;
		}
		else
		{
			hurtBody.globalPosition = ground->GetPosition( edgeQuantity ) + gn * (double)(b.rh);// + hurtBody.rh );
			hurtBody.globalAngle = angle;
		}
	}
	else
	{
		hurtBody.globalPosition = position + hurtBody.offset;
		hurtBody.globalAngle = angle;
	}
	
	b.globalPosition = position;// +b.offset;
	b.globalAngle = 0;
}

V2d Actor::GetTrueCenter()
{
	return position + b.offset;
}

void Actor::TryStartWaterGlide()
{
	if (action == WATERGLIDE || action == WATERGLIDE_HITSTUN || action == WATERGLIDECHARGE)
	{
		return;
	}

	if (grindEdge != NULL)
	{
		if (!ExitGrind(false))
		{
			return;
		}
	}

	if (reversed && ground != NULL)
	{
		groundSpeed = -groundSpeed;
	}

	V2d vel = GetTrueVel();

	if (ground != NULL)
	{
		ground = NULL;
		framesInAir = 0;

		if (reversed)
		{
			vel.y += 5;
		}
		else
		{
			vel.y -= 5;
		}

	}

	if (length(vel) < 15.0)
	{
		vel = normalize(vel) * 15.0;
	}

	springVel = vel;

	springExtra = V2d(0, 0);

	if (vel.x < 0)
		facingRight = false;
	else if (vel.x > 0)
		facingRight = true;

	SetAction(WATERGLIDE);

	holdJump = false;
	holdDouble = false;
	RestoreAirOptions();
	rightWire->Reset();
	leftWire->Reset();
	frame = 0;
	UpdateHitboxes();
	ground = NULL;
	wallNormal = V2d(0, 0);
	velocity = V2d(0, 0);
	currWall = NULL;
	bounceEdge = NULL;
	grindEdge = NULL;
	reversed = false;

	if (IsNormalSkin())
		SetSkin(SKIN_GLIDE);
}

void Actor::HandleWaterSituation(int wType,
	SpecialTerrainSituation sit)
{
	HandleWaterSounds(wType, sit);


	if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
	{
		int skinChoice = SKIN_NORMAL;

		switch (wType)
		{
		case TerrainPolygon::WATER_NORMAL:
		{
			skinChoice = SKIN_ORANGE;
			break;
		}
		case TerrainPolygon::WATER_GLIDE:
		{
			skinChoice = SKIN_GLIDE;
			break;
		}
		case TerrainPolygon::WATER_LOWGRAV:
		{
			skinChoice = SKIN_PINK;
			break;
		}
		case TerrainPolygon::WATER_HEAVYGRAV:
		{
			skinChoice = SKIN_DAWN;
			break;
		}
		case TerrainPolygon::WATER_BUOYANCY:
		{
			skinChoice = SKIN_GDUBS;
			break;
		}
		case TerrainPolygon::WATER_ACCEL:
		{
			skinChoice = SKIN_PURPLE;
			break;
		}
		case TerrainPolygon::WATER_ZEROGRAV:
		{
			skinChoice = SKIN_GOLD;
			break;
		}
		case TerrainPolygon::WATER_LAUNCHER:
		{
			skinChoice = SKIN_DUSK;
			break;
		}
		case TerrainPolygon::WATER_MOMENTUM:
		{
			skinChoice = SKIN_VILLAIN;
			break;
		}
		case TerrainPolygon::WATER_TIMESLOW:
		{
			skinChoice = SKIN_BONFIRE;
			break;
		}
		case TerrainPolygon::WATER_POISON:
		{
			skinChoice = SKIN_TOXIC;
			break;
		}
		case TerrainPolygon::WATER_FREEFLIGHT:
		{
			skinChoice = SKIN_MAGI;
			break;
		}
		case TerrainPolygon::WATER_INVERTEDINPUTS:
		{
			skinChoice = SKIN_DAWN;
			break;
		}
		case TerrainPolygon::WATER_REWIND:
		{
			skinChoice = SKIN_AMERICA;
			break;
		}
		case TerrainPolygon::WATER_SWORDPROJECTILE:
		{
			skinChoice = SKIN_RED;
			break;
		}
		case TerrainPolygon::WATER_SUPER:
		{
			skinChoice = SKIN_GHOST;
			break;
		}
		}

		if (skinChoice != SKIN_NORMAL)
		{
			if (IsNormalSkin())
				SetSkin(skinChoice);
		}
	}
	else if( sit == SPECIALT_EXIT )
	{
		SetSkin(SKIN_NORMAL);
	}


	switch (wType)
	{
	case TerrainPolygon::WATER_NORMAL:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			extraGravityModifier = .8;
			gravModifyFrames = 1;
			RestoreAirOptions();
			ApplyGeneralAcceleration(.1);

			
		}
		else if (sit == SPECIALT_EXIT)
		{
			SetSkin(SKIN_NORMAL);
			RestoreAirOptions();
		}
		break;
	}
	case TerrainPolygon::WATER_GLIDE:
	{
		if (sit == SPECIALT_ENTER)
			//if (action != WATERGLIDE)
		{
			
		}

		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			TryStartWaterGlide();

			//if i add glide attack, adjust this
			//this makes sure that you don't
			//get hit and then hitstun cancels the glide.
			/*if (action != WATERGLIDE && action != WATERGLIDE_HITSTUN && action != WATERGLIDECHARGE)
			{
				SetAction(WATERGLIDE);
			}*/
			if (action == WATERGLIDE || action == WATERGLIDECHARGE )
			{
				springStunFrames = 2;
			}
		}

		if (sit == SPECIALT_EXIT)
		{
			RestoreAirOptions();
		}
		break;
	}
	case TerrainPolygon::WATER_LOWGRAV:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			extraGravityModifier = .5;
			gravModifyFrames = 2;
		}
		break;
	}
	case TerrainPolygon::WATER_HEAVYGRAV:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			extraGravityModifier = 2.0;
			gravModifyFrames = 2;
		}
		break;
	}
	case TerrainPolygon::WATER_BUOYANCY:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			if (sit == SPECIALT_ENTER)
			{
				RestoreAirOptions();
			}

			double revGrav = .3;
			V2d gNorm;
			if (ground != NULL)
			{
				gNorm = ground->Normal();
				if (gNorm.y < 0)
				{
					ground = NULL;
					SetAction(JUMP);
					frame = 1;
				}
			}

			if (ground != NULL && gNorm.y > 0)
			{
				extraGravityModifier = revGrav;
			}
			else
			{
				extraGravityModifier = -revGrav;
			}

			gravModifyFrames = 2;

			if (velocity.y < -40)
			{
				velocity.y = -40;
			}

			if (velocity.y > 0)
			{
				velocity.y -= .5;
			}
		}
		else if (sit == SPECIALT_EXIT)
		{
			Edge *exitEdge = RayCastSpecialTerrainExit();
			if (exitEdge != NULL)
			{
				if (exitEdge->Normal().y < 0)
				{
					//cout << "boost" << endl;
					velocity.y -= 10.0;
				}
			}

			if (sit == SPECIALT_EXIT)
			{
				RestoreAirOptions();
			}
		}
		break;
	}
	case TerrainPolygon::WATER_ACCEL:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			ApplyGeneralAcceleration(.5);
		}
		break;
	}
	case TerrainPolygon::WATER_ZEROGRAV:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			extraGravityModifier = 0;//2.0;
			gravModifyFrames = 2;
			RestoreAirOptions();
			//springStunFrames = 2;
		}
		break;
	}
	case TerrainPolygon::WATER_LAUNCHER:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			RestoreAirOptions();
		}
		else if (sit == SPECIALT_EXIT)
		{
			if (grindEdge == NULL)
			{
				Edge *exitEdge = RayCastSpecialTerrainExit();
				if (exitEdge != NULL)
				{
					V2d norm = exitEdge->Normal();
					SetAction(SPRINGSTUN);
					springStunFrames = 60;
					springStunFramesStart = springStunFrames;

					//make into function soon
					reversed = false;
					holdJump = false;
					holdDouble = false;
					RestoreAirOptions();
					rightWire->Reset();
					leftWire->Reset();
					frame = 0;
					UpdateHitboxes();
					ground = NULL;
					wallNormal = V2d(0, 0);
					velocity = V2d(0, 0);
					currWall = NULL;
					double speed = 40;

					springVel = norm * speed;

					if (springVel.x > 0)
					{
						facingRight = true;
					}
					else if (springVel.x < 0)
					{
						facingRight = false;
					}
				}
			}
		}
		break;
	}
	case TerrainPolygon::WATER_MOMENTUM:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			momentumBoostFrames = 2;
			RestoreAirOptions();
			extraGravityModifier = .65;
			gravModifyFrames = 2;
		}
		
		break;
	}
	case TerrainPolygon::WATER_TIMESLOW:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			RestoreAirOptions();

			if (IsNormalSkin())
			{
				SetSkin(SKIN_TOXIC);
			}
		}
		else if (sit == SPECIALT_EXIT)
		{
			SetSkin(SKIN_NORMAL);
		}
		break;
	}
	case TerrainPolygon::WATER_POISON:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			RestoreAirOptions();
			ApplyGeneralAcceleration(.2);
			DrainTimer(10);
		}
		break;
	}
	case TerrainPolygon::WATER_FREEFLIGHT:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			if (!IsGrindAction(action))
			{
				SetAction(FREEFLIGHTSTUN);
				springStunFrames = 2;
				extraGravityModifier = 0;
				gravModifyFrames = 2;
				RestoreAirOptions();
				ground = NULL;
				wallNormal = V2d(0, 0);
				currWall = NULL;
				bounceEdge = NULL;
				grindEdge = NULL;
				reversed = false;
			}
		}
		break;
	}
	case TerrainPolygon::WATER_INVERTEDINPUTS:
	{
		if (sit == SPECIALT_ENTER || sit == SPECIALT_REMAIN)
		{
			invertInputFrames = 2;
		}

		if (sit == SPECIALT_ENTER || sit == SPECIALT_EXIT)
		{
			if (currInput.LLeft() || currInput.LRight()
				|| currInput.LDown() || currInput.LUp())
			{
				directionalInputFreezeFrames = 20;
			}
			//SetAction(INVERSEINPUTSTUN);
		}
		break;
	}
	case TerrainPolygon::WATER_REWIND:
	{
		if (sit == SPECIALT_ENTER )
		{
			

			waterEntranceGround = ground;
			waterEntranceXOffset = offsetX;
			waterEntrancePhysHeight = b.rh;
			waterEntranceFacingRight = facingRight;
			waterEntranceGrindEdge = grindEdge;

			if (grindEdge != NULL )
			{
				waterEntranceQuantity = edgeQuantity;
			}
			if (ground != NULL)
			{
				waterEntranceQuantity = edgeQuantity;
				waterEntranceReversed = reversed;
			}

			waterEntrancePosition = position;
		}
		break;
	}
	case TerrainPolygon::WATER_SWORDPROJECTILE:
	{
		projectileSwordFrames = 2;
		break;
	}
	case TerrainPolygon::WATER_SUPER:
	{
		break;
	}
	}
}

void Actor::HandleWaterSounds(int wType,
	SpecialTerrainSituation sit)
{
	if (sit == SPECIALT_ENTER)
	{
		switch (wType)
		{
		case TerrainPolygon::WATER_NORMAL:
		{
			ActivateSound(PlayerSounds::S_WATER_NORMAL_ENTER);
			break;
		}
		case TerrainPolygon::WATER_GLIDE:
		{
			ActivateSound(PlayerSounds::S_WATER_GLIDE_ENTER);
			break;
		}
		case TerrainPolygon::WATER_LOWGRAV:
		{
			ActivateSound(PlayerSounds::S_WATER_LOWGRAV_ENTER);
			break;
		}
		case TerrainPolygon::WATER_HEAVYGRAV:
		{
			ActivateSound(PlayerSounds::S_WATER_HEAVYGRAV_ENTER);
			break;
		}
		case TerrainPolygon::WATER_BUOYANCY:
		{
			ActivateSound(PlayerSounds::S_WATER_BUOYANCY_ENTER);
			break;
		}
		case TerrainPolygon::WATER_ACCEL:
		{
			ActivateSound(PlayerSounds::S_WATER_ACCEL_ENTER);
			break;
		}
		case TerrainPolygon::WATER_ZEROGRAV:
		{
			ActivateSound(PlayerSounds::S_WATER_ZEROGRAV_ENTER);
			break;
		}
		case TerrainPolygon::WATER_LAUNCHER:
		{
			ActivateSound(PlayerSounds::S_WATER_LAUNCHER_ENTER);
			break;
		}
		case TerrainPolygon::WATER_MOMENTUM:
		{
			ActivateSound(PlayerSounds::S_WATER_MOMENTUM_ENTER);
			break;
		}
		case TerrainPolygon::WATER_TIMESLOW:
		{
			ActivateSound(PlayerSounds::S_WATER_TIMESLOW_ENTER);
			break;
		}
		case TerrainPolygon::WATER_POISON:
		{
			ActivateSound(PlayerSounds::S_WATER_POISON_ENTER);
			break;
		}
		case TerrainPolygon::WATER_FREEFLIGHT:
		{
			ActivateSound(PlayerSounds::S_WATER_FREEFLIGHT_ENTER);
			break;
		}
		case TerrainPolygon::WATER_INVERTEDINPUTS:
		{
			ActivateSound(PlayerSounds::S_WATER_INVERTEDINPUTS_ENTER);
			break;
		}
		case TerrainPolygon::WATER_REWIND:
		{
			break;
		}
		case TerrainPolygon::WATER_SWORDPROJECTILE:
		{
			break;
		}
		case TerrainPolygon::WATER_SUPER:
		{
			break;
		}
		}
	}
	else if (sit == SPECIALT_EXIT)
	{
		switch (wType)
		{
		case TerrainPolygon::WATER_NORMAL:
		{
			ActivateSound(PlayerSounds::S_WATER_NORMAL_EXIT);
			break;
		}
		case TerrainPolygon::WATER_GLIDE:
		{
			ActivateSound(PlayerSounds::S_WATER_GLIDE_EXIT);
			break;
		}
		case TerrainPolygon::WATER_LOWGRAV:
		{
			ActivateSound(PlayerSounds::S_WATER_LOWGRAV_EXIT);
			break;
		}
		case TerrainPolygon::WATER_HEAVYGRAV:
		{
			ActivateSound(PlayerSounds::S_WATER_HEAVYGRAV_EXIT);
			break;
		}
		case TerrainPolygon::WATER_BUOYANCY:
		{
			ActivateSound(PlayerSounds::S_WATER_BUOYANCY_EXIT);
			break;
		}
		case TerrainPolygon::WATER_ACCEL:
		{
			ActivateSound(PlayerSounds::S_WATER_ACCEL_EXIT);
			break;
		}
		case TerrainPolygon::WATER_ZEROGRAV:
		{
			ActivateSound(PlayerSounds::S_WATER_ZEROGRAV_EXIT);
			break;
		}
		case TerrainPolygon::WATER_LAUNCHER:
		{
			ActivateSound(PlayerSounds::S_WATER_LAUNCHER_EXIT);
			break;
		}
		case TerrainPolygon::WATER_MOMENTUM:
		{
			ActivateSound(PlayerSounds::S_WATER_MOMENTUM_EXIT);
			break;
		}
		case TerrainPolygon::WATER_TIMESLOW:
		{
			ActivateSound(PlayerSounds::S_WATER_TIMESLOW_EXIT);
			break;
		}
		case TerrainPolygon::WATER_POISON:
		{
			ActivateSound(PlayerSounds::S_WATER_POISON_EXIT);
			break;
		}
		case TerrainPolygon::WATER_FREEFLIGHT:
		{
			ActivateSound(PlayerSounds::S_WATER_FREEFLIGHT_EXIT);
			break;
		}
		case TerrainPolygon::WATER_INVERTEDINPUTS:
		{
			ActivateSound(PlayerSounds::S_WATER_INVERTEDINPUTS_EXIT);
			break;
		}
		case TerrainPolygon::WATER_REWIND:
		{
			break;
		}
		case TerrainPolygon::WATER_SWORDPROJECTILE:
		{
			break;
		}
		case TerrainPolygon::WATER_SUPER:
		{
			break;
		}
		}
	}
}

void Actor::HandleSpecialTerrain()
{
	if (action == DEATH)
	{
		return;
	}

	//cout << "velocity: " << velocity.x << ", " << velocity.y << "\n";

	int wType;
	int oldWType; 

	if (currSpecialTerrain != NULL )
	{
		wType = currSpecialTerrain->waterType;

		if (oldSpecialTerrain == NULL )
		{
			//enter terrain from no terrain
			HandleWaterSituation( wType, SPECIALT_ENTER);
		}
		else
		{
			oldWType = oldSpecialTerrain->waterType;

			if (wType == oldWType)
			{
				//remain in current terrain
				HandleWaterSituation(wType, SPECIALT_REMAIN);
			}
			else
			{
				//exit old terrain, enter current one
				HandleWaterSituation(oldWType, SPECIALT_EXIT);
				HandleWaterSituation(wType, SPECIALT_ENTER);
			}
		}
	}
	else
	{
		if (oldSpecialTerrain != NULL)
		{
			oldWType = oldSpecialTerrain->waterType;

			HandleWaterSituation(oldWType, SPECIALT_EXIT);
		}
	}


	/*if (!InWater( TerrainPolygon::WATER_TEL)
	{
		inTeleportAcrossWater = false;
	}*/

}

Edge * Actor::RayCastSpecialTerrainExit()
{
	V2d targetPos = position - normalize(velocity) * 200.0;
	rayMode = RAYMODE_WATER;
	rayCastInfo.Reset();
	rayCastInfo.rayStart = position;
	rayCastInfo.rayEnd = targetPos;
	RayCast(this, oldSpecialTerrain->myTerrainTree->startNode, position, targetPos);
	return rayCastInfo.rcEdge;
}

Edge * Actor::RayCastSpecialTerrainEnter()
{
	V2d targetPos = position - normalize(velocity) * 200.0;
	rayMode = RAYMODE_WATER;
	rayCastInfo.Reset();
	rayCastInfo.rayStart = position;
	rayCastInfo.rayEnd = targetPos;
	RayCast(this, currSpecialTerrain->myTerrainTree->startNode, position, targetPos);
	return rayCastInfo.rcEdge;
}

void Actor::UpdateSmallLightning()
{
	int smallLightningCounter = -1;
	switch (speedLevel)
	{
	case 0:
		smallLightningCounter = 30;
		break;
	case 1:
		smallLightningCounter = 20;
		break;
	case 2:
		smallLightningCounter = 10;
		break;
	}

	bool dontActivateLightningAction = action == SEQ_MEDITATE_MASKON ||
		action == SEQ_MASKOFF || action == SEQ_MEDITATE;

	if (!IsIntroAction(action) && sess->totalGameFrames % smallLightningCounter == 0
		&& !dontActivateLightningAction)
	{
		RelEffectInstance params;
		//EffectInstance params;
		Transform tr = sf::Transform::Identity;
		//params.SetParams(Vector2f(position.x, position.y - 100) , tr, 7, 1, 0);
		Vector2f randPos(rand() % 100 - 50, rand() % 100 - 50);

		int r = rand() % 7;

		params.SetParams(randPos, tr, 9, 1, 9 * r, &spriteCenter);

		

		ActivateEffect(PLAYERFX_SMALL_LIGHTNING, &params);
	}
}

void Actor::UpdateRisingAura()
{
	//this is turned off atm
	if (!IsIntroAction(action) && sess->totalGameFrames % 30 == 0)
	{
		RelEffectInstance params;
		//EffectInstance params;
		Transform tr = sf::Transform::Identity;
		Vector2f randPos(rand() % 20 - 10, rand() % 20 - 10);

		params.SetParams(randPos, tr, 1, 60, 0, &spriteCenter, 40);
		//EffectInstance *ei = risingAuraPool->ActivateEffect(&params);
		//ei->SetVelocityParams(Vector2f(0, 0), Vector2f(0, -.02), 5 );
	}
}

void Actor::UpdateLockedFX()
{
	if (currLockedFairFX != NULL && action != FAIR && action != AIRDASHFORWARDATTACK)
	{
		currLockedFairFX->ClearLockPos();
		currLockedFairFX = NULL;
	}
	if (currLockedDairFX != NULL && action != DAIR)
	{
		currLockedDairFX->ClearLockPos();
		currLockedDairFX = NULL;
	}
	if (currLockedUairFX != NULL && action != UAIR)
	{
		currLockedUairFX->ClearLockPos();
		currLockedUairFX = NULL;
	}
}

void Actor::ProcessSpecialTerrain()
{
	oldSpecialTerrain = currSpecialTerrain;
	currSpecialTerrain = NULL;
	queryType = Q_SPECIALTERRAIN;
	V2d trueCenter = GetTrueCenter();
	Rect<double> r(trueCenter.x - b.rw, trueCenter.y - b.rh, 2 * b.rw, 2 * b.rh);
	GetSpecialTerrainTree()->Query(this, r);
	HandleSpecialTerrain();
}

void Actor::HitGoal()
{
	hitGoal = true;
	ActivateSound(PlayerSounds::S_DESTROY_GOAL);
}

void Actor::ProcessHitGoal()
{
	if (sess->gameModeType != MatchParams::GAME_MODE_BASIC
		&& sess->gameModeType != MatchParams::GAME_MODE_PARALLEL_PRACTICE)
	{
		return;
	}

	if (hitGoal)// && action != GOALKILL && action != EXIT && action != GOALKILLWAIT && action != EXITWAIT)
	{
		SetAction(GOALKILL);

		/*if (owner != NULL && owner->IsRushSession() && owner->mainMenu->rushManager->TryToGoToNextLevel(owner))
		{
			V2d pos = position;
			SetAirPos(pos, true);
			velocity = V2d(0, 0);
			hitGoal = false;
		}
		else
		{
			SetAction(GOALKILL);
		}*/
	}
	else if (hitNexus != NULL && action != NEXUSKILL && action != SEQ_FLOAT_TO_NEXUS_OPENING
		&& action != SEQ_FADE_INTO_NEXUS)
	{
		SetAction(NEXUSKILL);
	}
}

bool Actor::CareAboutSpeedAction()
{
	return action != DEATH && action != EXIT && !IsGoalKillAction(action) && action != RIDESHIP && action != GRINDBALL
		&& action != GRINDATTACK;
}

bool Actor::TryThrowSwordProjectile(V2d &offset, V2d &dir)
{
	for (int i = 0; i < NUM_SWORD_PROJECTILES; ++i)
	{
		if (!swordProjectiles[i]->IsActive())
		{
			swordProjectiles[i]->Throw(actorIndex, position + offset, dir);
			return true;
		}
	}

	return false;
}

bool Actor::TryThrowEnemySwordProjectileBasic()
{
	if (enemyProjectileSwordFrames > 0)
	{
		UpdateKnockbackDirectionAndHitboxType();
		return TryThrowSwordProjectile(V2d(0, 0), currHitboxInfo->hDir);
	}

	return false;
}

bool Actor::TryThrowSwordProjectileBasic()
{
	if (projectileSwordFrames > 0)
	{
		UpdateKnockbackDirectionAndHitboxType();
		return TryThrowSwordProjectile(V2d(0, 0), currHitboxInfo->hDir);
	}
	
	return false;
}

void Actor::UpdateSpeedBar()
{
	V2d trueVel;
	double speed;
	if (ground != NULL) //ground
	{
		trueVel = normalize(ground->v1 - ground->v0) * groundSpeed;
		//speed = abs(groundSpeed);
	}
	else //air
	{
		trueVel = velocity;
		//speed = length( velocity );
	}

	trueVel.y = min(40.0, trueVel.y); //falling can only help your speed so much

	speed = length(trueVel);

	if (momentumBoostFrames > 0)
	{
		speed = GetMaxSpeed();
	}

	if (CareAboutSpeedAction())
	{
		if (speed > currentSpeedBar)
		{
			currentSpeedBar += speedChangeUp;
			if (currentSpeedBar > speed)
				currentSpeedBar = speed;//currentSpeedBar * (1.0 -fUp) + speed * fUp;
		}
		else if (speed < currentSpeedBar)
		{
			currentSpeedBar -= speedChangeDown;
			if (currentSpeedBar < speed)
			{
				currentSpeedBar = speed;
			}
			//currentSpeedBar = currentSpeedBar * (1.0 -fDown) + speed * fDown;
		}

		if (currentSpeedBar >= level2SpeedThresh)
		{
			speedLevel = 2;
		}
		else if (currentSpeedBar >= level1SpeedThresh)
		{
			speedLevel = 1;
		}
		else
		{
			speedLevel = 0;
		}
	}
}
//static int testBlah = 0;

void Actor::UpdateMotionGhosts()
{
	if (simulationMode)
		return;
	//shouldn't be max motion ghosts cuz thats not efficient
	for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhostBuffer->SetRotation(i, sprite->getRotation() / 180.f * PI);
		motionGhostBufferBlue->SetRotation(i, sprite->getRotation() / 180.f * PI);
		motionGhostBufferPurple->SetRotation(i, sprite->getRotation() / 180.f * PI);
	}

	V2d motionGhostDir;
	double motionMagnitude = 0;
	if (ground != NULL)
	{
		motionGhostDir = -normalize(normalize(ground->v1 - ground->v0) * groundSpeed);
		if (reversed)
			motionGhostDir = -motionGhostDir;
		motionMagnitude = abs(groundSpeed);
	}
	else
	{
		motionGhostDir = -normalize(velocity);
		motionMagnitude = length(velocity);
	}

	float dist = motionGhostSpacing;

	int showMotionGhosts = min(motionMagnitude / 1.0, 80.0);

	if (speedLevel == 0)
	{
		showMotionGhosts = min(showMotionGhosts * 2.0, 80.0);
	}
	/*for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetSpread(showMotionGhosts, Vector2f(motionGhostDir.x, motionGhostDir.y), sprite->getRotation() / 180.f * PI);
		motionGhostsEffects[i]->SetRootPos(Vector2f(spriteCenter.x, spriteCenter.y));
	}*/

	if (!IsIntroAction(action))
	{
		if (sess->totalGameFrames % 4 == 0)
		{
			for (int i = 0; i < 3; ++i)
			{
				motionGhostsEffects[i]->AddPosition(Vector2f(spriteCenter.x, spriteCenter.y));
				motionGhostsEffects[i]->angle = sprite->getRotation() / 180.f * PI;
			}
		}
	}
	
	
	//testBlah++;
	
}

void Actor::UpdateSpeedParticles()
{
	if (speedParticleCounter == speedParticleRate && CareAboutSpeedAction())
	{
		if (speedLevel == 1)
		{
			int effectIndex = 0;
			int randTex = rand() % 3;

			int rx = 30;
			int ry = 30;

			if (ground != NULL)
			{

				double angle = GroundedAngle();
				V2d groundPos = ground->GetPosition(edgeQuantity);

				V2d norm = GetGroundedNormal();

				V2d truePos = groundPos + norm * normalHeight;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);

				//blue charge
				ActivateEffect(PLAYERFX_SPEED_LEVEL_CHARGE, Vector2f(truePos), RadiansToDegrees(angle), 12, 3, facingRight, randTex * 12);
			}
			else
			{
				V2d truePos = position;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);
				double angle = GroundedAngle();

				ActivateEffect(PLAYERFX_SPEED_LEVEL_CHARGE, Vector2f(truePos), RadiansToDegrees(angle), 12, 3, facingRight, randTex * 12);
			}

		}
		else if (speedLevel == 2)
		{
			int rx = 30;
			int ry = 30;
			if (ground != NULL)
			{
				V2d groundPos = ground->GetPosition(edgeQuantity);
				V2d norm = GetGroundedNormal();
				V2d truePos = groundPos + norm * normalHeight;//.0;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);
				double angle = GroundedAngle();

				//purple charge
				ActivateEffect(PLAYERFX_SPEED_LEVEL_CHARGE, Vector2f(truePos), RadiansToDegrees(angle), 12, 3, facingRight, 12 * 5);
			}
			else
			{
				V2d truePos = position;
				int randx = rand() % rx;
				randx -= rx / 2;
				int randy = rand() % ry;
				randy -= ry / 2;
				truePos += V2d(randx, randy);
				double angle = GroundedAngle();
				//purple charge
				ActivateEffect(PLAYERFX_SPEED_LEVEL_CHARGE, Vector2f(truePos), RadiansToDegrees(angle), 12, 3, facingRight, 12 * 5);
			}
		}

		

		speedParticleCounter = 0;
	}
}
void Actor::UpdateAttackLightning()
{
	CreateAttackLightning();
}

void Actor::UpdateAllEffects()
{
	if (simulationMode)
	{
		return;
	}
	for (auto it = effectPools.begin(); it != effectPools.end(); ++it)
	{
		if ((*it).pool != NULL)
		{
			(*it).pool->Update();
		}
	}
}

void Actor::UpdatePlayerShader()
{
	if (simulationMode)
		return;

	if (kinMode == K_DESPERATION )
	{
		skinShader.pShader.setUniform("despFrame", (float)GetSurvivalFrame());
	}
	else
	{
		float invinc = min(invincibleFrames, 1);
		skinShader.pShader.setUniform("u_invincible", invinc);

		float super = superLevelCounter;
		skinShader.pShader.setUniform("u_super", super);
		
		skinShader.pShader.setUniform("despFrame", -1.f);
	}
}

void Actor::UpdateDashBooster()
{
	//happens even when in time slow
	if (action == DASH)
	{
		if (currBBoostCounter < maxBBoostCount)
		{
			currBBoostCounter++;
		}
	}
	else if (action == AIRDASH)
	{
		if (currAirdashBoostCounter < maxAirdashBoostCount)
		{
			currAirdashBoostCounter++;
		}
	}
}

void Actor::SlowDependentFrameIncrement()
{
	if (slowCounter == slowMultiple)
	{
		ActionTimeDepFrameInc();

		if (wallJumpFrameCounter < wallJumpMovementLimit)
		{
			
			wallJumpFrameCounter++;
		}
			
		//cout << "++frames in air: "<< framesInAir << " to " << (framesInAir+1) << endl;
		framesInAir++;

		UpdateModifiedGravity();

		if (invertInputFrames > 0)
		{
			--invertInputFrames;
		}

		if (action == GRINDBALL || action == GRINDATTACK || action == RAILGRIND)
		{
			framesGrinding++;
			framesNotGrinding = 0;
		}
		else
		{
			framesGrinding = 0;
			framesNotGrinding++;
		}

		framesSinceBounce++;

		//seems better to just update this all the time
		/*if (action == BOUNCEAIR && oldBounceEdge != NULL)
		{
			framesSinceBounce++;
		}*/

		++framesSinceRightWireBoost;
		++framesSinceLeftWireBoost;
		++framesSinceDoubleWireBoost;

		++frame;

		if (framesSinceDashAttack < attackLevelCounterLimit)
			framesSinceDashAttack++;
		else
		{
			dashAttackLevel = 0;
		}

		if (framesSinceStandAttack < attackLevelCounterLimit)
			framesSinceStandAttack++;
		else
		{
			standAttackLevel = 0;
		}

		if (framesSinceUpTilt < attackLevelCounterLimit)
			framesSinceUpTilt++;
		else
		{
			upTiltLevel = 0;
		}

		if (framesSinceDownTilt < attackLevelCounterLimit)
			framesSinceDownTilt++;
		else
		{
			downTiltLevel = 0;
		}

		if (springStunFrames > 0 && action != SPRINGSTUNANNIHILATIONATTACK )
			--springStunFrames;

		if (directionalInputFreezeFrames > 0)
		{
			--directionalInputFreezeFrames;
		}
		
		//cout << "frame: " << frame << endl;

		++framesSinceClimbBoost;
		++speedParticleCounter;


		if( globalTimeSlowFrames > 0 )
		{
			--globalTimeSlowFrames;

			if (globalTimeSlowFrames == 0)
			{
				//timeSlowBoosterTrailEmitter->SetOn(false);
			}
		}

		if (freeFlightFrames > 0)
		{
			--freeFlightFrames;

			if (freeFlightFrames == 0)
			{
				gravModifyFrames = 0; //this might be unnecessary
				//grav is already turned off for freeflightframes > 0

				freeFlightBoosterTrailEmitter->SetOn(false);
			}
		}

		if (homingFrames > 0)
		{
			--homingFrames;

			if (homingFrames == 0)
			{
				homingBoosterTrailEmitter->SetOn(false);
			}
		}

		if (antiTimeSlowFrames > 0)
		{
			--antiTimeSlowFrames;
			if (antiTimeSlowFrames == 0)
			{
				SetSkin(SKIN_NORMAL);
				antiTimeSlowBoosterTrailEmitter->SetOn(false);
			}
		}

		if (rewindOnHitFrames > 0)
		{
			--rewindOnHitFrames;
		}

		if (momentumBoostFrames > 0)
		{
			--momentumBoostFrames;

			if (momentumBoostFrames == 0)
			{
				momentumBoosterTrailEmitter->SetOn(false);
			}
		}

		if (projectileSwordFrames > 0)
		{
			--projectileSwordFrames;
		}

		if (enemyProjectileSwordFrames > 0)
		{
			--enemyProjectileSwordFrames;
		}

		slowCounter = 1;

		if (!IsIntroAction(action))
			if (invincibleFrames > 0)
				--invincibleFrames;

		if (flashFrames > 0)
		{
			--flashFrames;
			if (flashFrames <= 2)
			{
				swordState = SWORDSTATE_REGULAR;
			}
		}
			


	}
	else
		slowCounter++;
}

void Actor::SlowIndependentFrameIncrement()
{
	if (framesSinceSuperPress >= 0)
	{
		framesSinceSuperPress++;
	}
	if (framesSinceBlockPress >= 0)
	{
		framesSinceBlockPress++;
	}
}

void Actor::UpdateBounceFlameCounters()
{
	if (bounceFlameOn)
	{
		++framesFlameOn;
	}
}

void Actor::TryEndLevel()
{
	if (sess->IsRushSession() && action == EXITWAIT && frame == GetActionLength(EXITWAIT))
	{
		if (owner->mainMenu->rushManager->TryToGoToNextLevel(owner))
		{

		}
		else
		{
			
		}
	}
	else if (action == EXITWAIT && frame == GetActionLength(EXITWAIT))
	{
		sess->EndLevel();
		//owner->goalDestroyed = true;	
	}
	else if (action == EXITBOOST && frame == GetActionLength(EXITBOOST))
	{
		owner->EndLevel();
		//owner->goalDestroyed = true;
	}
}

void Actor::UpdatePostPhysics()
{
	gravityDecreaserOnRingGroup->SetBase(Vector2f(position));
	gravityDecreaserOffRingGroup->SetBase(Vector2f(position));

	gravityIncreaserOnRingGroup->SetBase(Vector2f(position));
	gravityIncreaserOffRingGroup->SetBase(Vector2f(position));

	gravityIncreaserTrailEmitter->SetPos(Vector2f(position));
	gravityDecreaserTrailEmitter->SetPos(Vector2f(position));
	if (startBoosterGravModifyFrames > 0)
	{
		gravityIncreaserTrailEmitter->data.boostPortion = 1.f - (boosterGravModifyFrames / (float)startBoosterGravModifyFrames);
		gravityDecreaserTrailEmitter->data.boostPortion = 1.f - (boosterGravModifyFrames / (float)startBoosterGravModifyFrames);
	}
	
	momentumBoosterTrailEmitter->SetPos(Vector2f(position));
	if (startMomentumBoostFrames > 0)
	{
		momentumBoosterTrailEmitter->data.boostPortion = 1.f - (momentumBoostFrames / (float)startMomentumBoostFrames);
	}
	
	//timeSlowBoosterTrailEmitter->SetPos(Vector2f(position));
	//if (startGlobalTimeSlowFrames > 0)
	//{
	//	timeSlowBoosterTrailEmitter->boostPortion = 1.f - (globalTimeSlowFrames / (float)startGlobalTimeSlowFrames);
	//}
	
	homingBoosterTrailEmitter->SetPos(Vector2f(position));
	if (startHomingFrames > 0)
	{
		homingBoosterTrailEmitter->data.boostPortion = 1.f - (homingFrames / (float)startHomingFrames);
	}

	antiTimeSlowBoosterTrailEmitter->SetPos(Vector2f(position));
	if (startAntiTimeSlowFrames > 0)
	{
		antiTimeSlowBoosterTrailEmitter->data.boostPortion = 1.f - (antiTimeSlowFrames / (float)startAntiTimeSlowFrames);
	}

	freeFlightBoosterTrailEmitter->SetPos(Vector2f(position));
	if (startFreeFlightFrames > 0)
	{
		freeFlightBoosterTrailEmitter->data.boostPortion = 1.f - (freeFlightFrames / (float)startFreeFlightFrames);
	}


	if (action == DEATH && simulationMode)
	{
		return;
	}

	if (action == HIDDEN)
	{
		UpdateAllEffects();
		return;
	}

	if (kinMode == K_DESPERATION && !simulationMode)
	{
		float despFactor = GetSurvivalFrame() / (float)maxDespFrames;
		float lengthFactor = min(1.f, despFactor + .3f);//min(1.f, despFactor + .3f);
		sess->pokeTriangleScreenGroup->SetLengthFactor(lengthFactor);

		double maxRumble = 7;
		sess->cam.SetRumble(max(1.0, maxRumble * despFactor), max(1.0, maxRumble * despFactor), 60 );// 60, 2 * despFactor );
	}
	

	if (hitlagFrames > 0)
	{
		--hitlagFrames;

		//cout << "update hitlag: " << hitlagFrames << endl;

		if (hitlagFrames > 0)
		{
			UpdateSprite();
			return;
		}
		else
		{
			if (!attackingHitlag && !IsBlockAction( action ) )
			{
				velocity = GetAdjustedKnockback(velocity);
			}
			else
			{
				frameAfterAttackingHitlagOver = true;
			}
		}
	}
	else
	{
		frameAfterAttackingHitlagOver = false;
	}

	if (ground != NULL) //doesn't work when grinding or bouncing yet
	{
		double g = groundSpeed;
		if (reversed)
			g = -g;
		velocity = normalize(ground->v1 - ground->v0) * g;
	}

	KinModeUpdate();
	
	QueryTouchGrass();

	ProcessSpecialTerrain();

	if( action == DEATH )
	{
		sprite->setTexture( *(tileset[DEATH]->texture));
		if( facingRight )
		{
			sprite->setTextureRect( tileset[DEATH]->GetSubRect( frame / 2 ) );
		}
		else
		{
			sf::IntRect ir = tileset[DEATH]->GetSubRect( frame / 2 );
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );
		sprite->setRotation( 0 );

		UpdateAllEffects();
		//if( frame % 1 == 0 )
		//{
		//	double startRadius = 64;
		//	double endRadius = 500;

		//	double part = frame / 88.f;

		//	double currRadius = startRadius * ( 1.f - part ) + endRadius * ( part );

		//	int randAng = (rand() % 360);
		//	double randAngle = randAng / 180.0 * PI;
		//	//randAngle += PI / 2.0;
		//	V2d pos( sin( randAngle ) * currRadius, -cos( randAngle ) * currRadius );

		//	pos += position;
		//	double fxAngle = randAngle - PI / 3.5;
		//	//cout << "randAngle: " << randAngle << endl;

		//	//cout << "randang: " << randAng << endl;
		//	ActivateEffect( DrawLayer::IN_FRONT, ts_fx_death_1c, pos, false, fxAngle, 12, 2, true );
		//}

		if( kinMask != NULL)
			kinMask->Update( speedLevel, kinMode == K_DESPERATION );


		++frame;
		return;
	}

	if (action == SWINGSTUN)
	{
		V2d anchor = oldSwingLauncher->anchor;
		V2d dirToAnchor = normalize(position - anchor);
		oldSwingLauncher->data.currAngle = GetVectorAngleCW(dirToAnchor);
	}
	


	ProcessHitGoal();

	
	UpdateSprite();

	UpdateAttackLightning();

	UpdateAllEffects();

	UpdateSpeedBar();

	UpdateMotionGhosts();

	UpdateSpeedParticles();

	UpdatePlayerShader();

	UpdateDashBooster();

	SlowDependentFrameIncrement();

	SlowIndependentFrameIncrement(); //empty atm

	ActionTimeIndFrameInc();

	if (framesSinceGrindAttempt < maxFramesSinceGrindAttempt)
	{
		++framesSinceGrindAttempt;
	}

	if( standNDashBoostCurr > 0 )
		standNDashBoostCurr--;

	UpdateRecentHitters();

	UpdateBounceFlameCounters();
	
	UpdateHitboxes(); //just for debug draw
	

	//pTrail->Update( position );

	if( kinMask != NULL)
		kinMask->Update(speedLevel, kinMode == K_DESPERATION );


	//needed before and after water does stuff
	if (ground != NULL) //doesn't work when grinding or bouncing yet
	{
		double g = groundSpeed;
		if (reversed)
			g = -g;
		velocity = normalize(ground->v1 - ground->v0) * g;
	}

	//moving this before the water so it'll be accurate. also fixing for reversed
	//if( ground != NULL ) //doesn't work when grinding or bouncing yet
	//{
	//	velocity = normalize( ground->v1 - ground->v0) * groundSpeed;
	//}

	nameTag->SetPos(Vector2f( position ) );

	TryEndLevel();
}

void Actor::BounceFlameOn()
{
	framesFlameOn = 0;
	bounceFlameOn = true;
	scorpOn = true;
	oldBounceEdge = NULL; //recently added this line
	//was previously only in the code for LAND action
}

void Actor::BounceFlameOff()
{
	if (action == BOUNCEGROUND && bounceEdge != NULL && bounceEdge->rail != NULL && bounceEdge->rail->GetRailType() == TerrainRail::SCORPIONONLY)
	{
		SetAction(JUMP);
		ground = NULL;
		frame = 1;
		holdJump = false;

		double through = dot(storedBounceVel, -bounceNorm);
		velocity = storedBounceVel + bounceNorm * through;

		//if (TryLandFromBounceGround())
		//{
		//	SetAction(JUMP);
		//	velocity = ground->Along() * groundSpeed;
		//	ground = NULL;
		//	frame = 1;
		//	holdJump = false;
	}

	scorpOn = false;
	framesFlameOn = 0;
	bounceFlameOn = false;
	oldBounceEdge = NULL;
	bounceEdge = NULL;
	bounceGrounded = false;
}

bool Actor::IsBeingSlowed()
{
	//multiplayer

	Actor *other;
	if( actorIndex == 0 )
	{
		other = sess->GetPlayer( 1 );
	}
	else
	{
		other = sess->GetPlayer( 0 );
	}
		
	if (other == NULL)
		return false;


	bool found = false;
	for( int i = 0; i < MAX_BUBBLES; ++i )
	{
		if( other->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - other->bubblePos[i] ) <= other->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

bool Actor::CanShootWire()
{
	return !IsSpringAction(action);
}

bool Actor::CanCreateTimeBubble()
{
	return action != SPRINGSTUNTELEPORT;
}

void Actor::HandleGroundTrigger(GroundTrigger *trigger)
{
	storedTrigger = trigger;
	switch (trigger->trigType)
	{
	case TRIGGER_SHIPPICKUP:
	{
		//ShipPickupPoint(trigger->currPosInfo.GetQuant(), trigger->facingRight);
		break;
	}
	case TRIGGER_HOUSEFAMILY:
	{
		SetKinMode(K_NORMAL);
		SetExpr(KinMask::Expr_NEUTRAL);
		assert(ground != NULL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;
		//facingRight = trigger->facingRight;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}
		TurnFace();
		owner->SetStorySeq(trigger->storySeq);
		//owner->state = GameSession::STORY;
		//owner->state = GameSession::SEQUENCE;
		//physicsOver = true;
		//owner->activeSequence = trigger->gameSequence;

		break;
	}
	case TRIGGER_GETAIRDASH:
	{
		SetKinMode(K_NORMAL);
		SetExpr(KinMask::Expr_NEUTRAL);
		assert(ground != NULL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;
		//facingRight = trigger->facingRight;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		//SetAction(GETPOWER_AIRDASH_MEDITATE);
		//frame = 0;
		physicsOver = true;
		sess->activeSequences[0] = trigger->gameSequence;
		//owner->currStorySequence = trigger->storySeq;
		//owner->state = GameSession::STORY;
		break;
	}
	case TRIGGER_DESTROYNEXUS1:
	{
		SetKinMode(K_NORMAL);
		//SetExpr(Expr_NEUTRAL);
		//assert(ground != NULL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		//owner->Fade(false, 30, Color::Black);

		sess->activeSequences[0] = trigger->gameSequence;

		SetAction(SEQ_ENTERCORE1);
		frame = 0;
		physicsOver = true;

		//owner->currStorySequence = trigger->storySeq;
		//owner->state = GameSession::STORY;
		break;
	}
	case TRIGGER_CRAWLERATTACK:
	{
		SetKinMode(K_NORMAL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		SetAction(SEQ_LOOKUP);
		frame = 0;
		physicsOver = true;

		sess->activeSequences[0] = trigger->gameSequence;
		break;
	}
	case TRIGGER_TEXTTEST:
	{
		SetKinMode(K_NORMAL);
		edgeQuantity = trigger->currPosInfo.GetQuant();
		groundSpeed = 0;

		if (ground->Normal().y == -1)
		{
			offsetX = 0;
		}

		WaitInPlace();
		physicsOver = true;

		sess->activeSequences[0] = trigger->gameSequence;
		//owner->SetStorySeq(trigger->storySeq);
		break;
	}
	}
}

void Actor::UpdateModifiedGravity()
{
	if (gravModifyFrames > 0)
	{
		--gravModifyFrames;
		if (gravModifyFrames == 0)
		{
			extraGravityModifier = 1.0;
		}
	}

	if (boosterGravModifyFrames > 0)
	{
		--boosterGravModifyFrames;
		if (boosterGravModifyFrames == 0)
		{
			if (boosterExtraGravityModifier > 1.0)
			{
				gravityIncreaserTrailEmitter->SetOn(false);
				CreateGravityIncreaserOffRing();
			}
			else if (boosterExtraGravityModifier < 1.0)
			{
				gravityDecreaserTrailEmitter->SetOn(false);
				CreateGravityDecreaserOffRing();
			}
			else
			{
				assert(0);
			}


			boosterExtraGravityModifier = 1.0;


		}
	}
}

double Actor::GetGravity()
{
	if ( ground != NULL && freeFlightFrames > 0 )
	{
		//when you use a freeflight booster, gravity should be normal
		//while moving on the ground
		return gravity;	
	}

	return gravity * extraGravityModifier;
}

sf::Vector2<double> Actor::AddAerialGravity( sf::Vector2<double> vel )
{
	double normalGravity;
	if( vel.y >= maxFallSpeedSlow )
	{
		normalGravity = gravity * .4 / slowMultiple;
	}
	//else if (abs( vel.y ) < 8 )
	/*else if (abs(vel.y) < 8)
	{
		normalGravity = gravity * .9 / slowMultiple;
	}*/
	else if( vel.y < 0 )
	{
		normalGravity = gravity * 1.2 / slowMultiple;
	}
	else
	{
		normalGravity = gravity / slowMultiple;
	}

	if (wallClimbGravityOn)
	{
		//is this still getting used?
		normalGravity *= wallClimbGravityFactor;
	}

	if (IsAirHitstunAction(action))
	{
		normalGravity = gravity * hitstunGravMultiplier / slowMultiple;
	}

	normalGravity *= extraGravityModifier;

	normalGravity *= boosterExtraGravityModifier;

	vel += V2d(0, normalGravity );

	return vel;
}

//void Actor::StartSeq( SeqType s )
//{
//	switch( s )
//	{
//	case SEQ_CRAWLER_FIGHT:
//		{
//			PoiInfo *pi = owner->poiMap["crawlerfighttrigger"];
//			V2d startFall = pi->pos;
//
//			action = JUMP;
//			frame = 1;
//			velocity = V2d( 0, 0 );
//			cutInput = true;
//			owner->cam.manual = true;
//		}
//		break;
//	}
//}
//
//void Actor::UpdateSeq()
//{
//	switch( seq )
//	{
//	case SEQ_CRAWLER_FIGHT:
//		{
//
//		}
//		break;
//	}
//}

bool Actor::SwingLaunch()
{
	if (currSwingLauncher != NULL)
	{
		/*if (oldSwingLauncher != NULL)
		{
			oldSwingLauncher->Recover();
		}*/
		SetAction(SWINGSTUN);

		currSwingLauncher->Launch();

		oldSwingLauncher = currSwingLauncher;

		position = currSwingLauncher->GetPosition();
		V2d dir = currSwingLauncher->dir;

		double s = currSwingLauncher->speed;

		springVel = currSwingLauncher->dir * s;

		springExtra = V2d(0, 0);

		springStunFrames = 0;//currSwingLauncher->stunFrames;

		ground = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		

		currSwingLauncher = NULL;

		holdJump = false;
		holdDouble = false;
		RestoreAirOptions();
		rightWire->Reset();
		leftWire->Reset();
		frame = 0;
		UpdateHitboxes();
		ground = NULL;
		wallNormal = V2d(0, 0);
		velocity = V2d(0, 0);
		currWall = NULL;
		return true;
	}

	return false;
}

bool Actor::TeleporterLaunch()
{
	if (currTeleporter != NULL)
	{
		if (!currTeleporter->TryTeleport())
		{
			currTeleporter = NULL;
			return false;
		}

		oldTeleporter = currTeleporter;

		//position = currTeleporter->dest;
		////holdJump = false;
		////holdDouble = false;
		//RestoreAirOptions();
		//rightWire->Reset();
		//leftWire->Reset();
		//frame = 0;
		//UpdateHitboxes();
		//ground = NULL;
		//bounceEdge = NULL;
		//grindEdge = NULL;
		//wallNormal = V2d(0, 0);
		////velocity = V2d(0, 0);
		//currWall = NULL;
		//currTeleporter = NULL;

		//return true;
		
		position = currTeleporter->GetPosition();
		V2d dir = currTeleporter->dir;

		double s = currTeleporter->speed;

		springVel = currTeleporter->dir * s;
		springExtra = V2d(0, 0);
		springStunFrames = currTeleporter->stunFrames;

		

		SetAction(SPRINGSTUNTELEPORT);
		teleportSpringDest = currTeleporter->dest;
		teleportSpringVel = velocity;

		holdJump = false;
		holdDouble = false;
		RestoreAirOptions();
		rightWire->Reset();
		leftWire->Reset();
		frame = 0;
		UpdateHitboxes();
		ground = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		wallNormal = V2d(0, 0);
		velocity = V2d(0, 0);
		currWall = NULL;
		currTeleporter = NULL;

		return true;
	}

	return false;
}

bool Actor::AimLauncherAim()
{
	if (currAimLauncher != NULL && action != AIMWAIT )
	{
		currAimLauncher->StartAiming();

		SetAction(AIMWAIT);
		frame = 0;

		wallNormal = V2d(0, 0);
		bounceEdge = NULL;
		grindEdge = NULL;
		currWall = NULL;
		ground = NULL;
		holdJump = false;
		holdDouble = false;
		position = currAimLauncher->GetPosition();
		RestoreAirOptions();
		rightWire->Reset();
		leftWire->Reset();
		reversed = false;
		springStunFrames = 0;
		
		velocity = V2d(0, 0);

		scorpOn = false;

		//currAimLauncher = NULL;

		return true;
	}

	return false;
}

bool Actor::ScorpionLaunch()
{
	if (currScorpionLauncher != NULL)
	{
		oldScorpionLauncher = currScorpionLauncher;
		
		currScorpionLauncher->Launch();
		position = currScorpionLauncher->GetPosition();
		V2d sprDir = currScorpionLauncher->dir;

		double s = currScorpionLauncher->speed;

		velocity = sprDir * s;

		SetAction(BOOSTERBOUNCE);
		frame = 0;

		currScorpionLauncher = NULL;

		holdJump = false;
		holdDouble = false;
		RestoreAirOptions();
		ground = NULL;
		currWall = NULL;
		wallNormal = V2d(0, 0);
		

		if (velocity.x > 0)
		{
			facingRight = true;
		}
		else if (velocity.x < 0)
		{
			facingRight = false;
		}

		return true;
	}

	return false;
}

bool Actor::SpringLaunch()
{
	if (currSpring != NULL)
	{
		currSpring->Launch();
		position = currSpring->GetPosition();
		V2d sprDir = currSpring->dir;
		
		double s = currSpring->speed;

		/*if (currSpring->springType == Spring::BOUNCE)
		{
			V2d testVel(velocity.x, velocity.y);
			double along = dot(testVel, sprDir);
			if (along > s)
			{
				s = along;
			}
		}*/

		springVel = currSpring->dir * s;

		springExtra = V2d( 0, 0 );

		if (springVel.x > .01)
		{
			facingRight = true;
		}
		else if (springVel.x < -.01)
		{
			facingRight = false;
		}
		
		/*if (aimLauncherType == AimLauncher::TYPE_BOUNCE)
		{
			SetAction(SPRINGSTUNAIM);
			aimLauncherStunFrames = currAimLauncher->stunFrames;
		}
		else if (aimLauncherType == AimLauncher::TYPE_AIRBOUNCE)
		{
			SetAction(SPRINGSTUNAIRBOUNCE);
			airBounceCounter = 0;
			aimLauncherStunFrames = currAimLauncher->stunFrames;
		}
		else if (aimLauncherType == AimLauncher::TYPE_GRIND)
		{
			SetAction(SPRINGSTUNGRINDFLY);
			aimLauncherStunFrames = currAimLauncher->stunFrames;
		}
		else if (aimLauncherType == AimLauncher::TYPE_HOMING)
		{
			SetAction(SPRINGSTUNHOMING);
			aimLauncherStunFrames = currAimLauncher->stunFrames;
		}*/ 
		springStunFrames = currSpring->stunFrames;
		springStunFramesStart = springStunFrames;

		if (currSpring->springType == Spring::TYPE_GLIDE)
		{
			SetAction(SPRINGSTUNGLIDE);
		}
		else if (currSpring->springType == Spring::TYPE_BOUNCE)
		{
			SetAction(SPRINGSTUNAIM);
		}
		else if (currSpring->springType == Spring::TYPE_AIRBOUNCE)
		{
			SetAction(SPRINGSTUNAIRBOUNCE);
			airBounceCounter = 0;
			aimLauncherStunFrames = currSpring->stunFrames;
		}
		else if (currSpring->springType == Spring::TYPE_ANNIHILATION_GLIDE)
		{
			//action = SPRINGSTUNGLIDE;
			SetAction( SPRINGSTUNANNIHILATION);
		}
		else if (currSpring->springType == Spring::TYPE_GRIND)
		{
			SetAction(SPRINGSTUNGRINDFLY);
		}
		/*else if (currSpring->springType == Spring::TYPE_HOMING)
		{
			SetAction(SPRINGSTUNHOMING);
		}*/
		else
		{
			SetAction(SPRINGSTUN);
		}

		currSpring = NULL;
		
		holdJump = false;
		holdDouble = false;
		reversed = false;
		RestoreAirOptions();
		rightWire->Reset();
		leftWire->Reset();
		frame = 0;
		UpdateHitboxes();
		ground = NULL;
		grindEdge = NULL;
		bounceEdge = NULL;
		wallNormal = V2d(0, 0);
		velocity = V2d(0, 0);
		currWall = NULL;
		return true;
	}

	return false;
}

void Actor::SetBoostVelocity()
{
	velocity = normalize(velocity) * (length(velocity) + currBooster->strength);
}

void Actor::CoyoteBulletBounce()
{

}

void Actor::SetBounceBoostVelocity()
{
	if (grindEdge != NULL)
	{
		if (!ExitGrind(false))
		{
			return;
		}
	}

	assert(grindEdge == NULL);

	double s = currBounceBooster->strength;

	if (currBounceBooster->upOnly)
	{
		if (ground == NULL && bounceEdge == NULL)
		{
			//SetAction(BOOSTERBOUNCE);
			//frame = 0;
		}
		else
		{
			SetAction(JUMP);
			frame = 1;

			velocity = GetTrueVel();

			ground = NULL;
			bounceEdge = NULL;
			grindEdge = NULL;

			//SetAction(BOOSTERBOUNCE);
		}

		ground = NULL;
		bounceEdge = NULL;
		grindEdge = NULL;
		reversed = false;

		velocity.y = -s;

		if (action == AIRDASH)
		{
			SetAction(JUMP);
			frame = 1;
		}
		holdJump = false;
	}
	else
	{
		V2d dir = normalize(position - currBounceBooster->GetPosition());

		if (ground != NULL)
		{
			V2d norm = ground->Normal();

			double d = dot(dir, norm);
			double dAlong = dot(dir, ground->Along());

			if ( d < 0 )
			{
				//if (dAlong > 0)
				//{
				//	groundSpeed = s * dAlong;
				//}
				//else if (dAlong < 0)
				//{
				//	groundSpeed = s * dAlong;
				//}
				//else
				//{
				//	bool fr = (facingRight && !reversed) || (!facingRight && reversed);
				//	if (fr)
				//	{
				//		groundSpeed = s;
				//	}
				//	else
				//	{
				//		groundSpeed = -s;
				//	}
				//	//assert(0);
				//}
				if (reversed)
				{
					groundSpeed = -s * dAlong;
				}
				else
				{
					groundSpeed = s * dAlong;
				}
				

				SetAction(BOOSTERBOUNCEGROUND);
				frame = 0;
			}
			else
			{
				SetAction(JUMP);
				frame = 1;

				velocity = GetTrueVel();

				ground = NULL;
				bounceEdge = NULL;
				grindEdge = NULL;
				
				reversed = false;

				velocity = dir * s;
				velocity.x *= .6;


				if (velocity.y > 0)
					velocity.y *= .5;

				SetAction(BOOSTERBOUNCE);
				frame = 0;
			}
		}
		else
		{
			if (ground == NULL && bounceEdge == NULL)
			{
				//SetAction(BOOSTERBOUNCE);
				//frame = 0;
			}
			else
			{
				SetAction(JUMP);
				frame = 1;

				velocity = GetTrueVel();

				ground = NULL;
				bounceEdge = NULL;
				grindEdge = NULL;

				//SetAction(BOOSTERBOUNCE);
			}

			ground = NULL;
			bounceEdge = NULL;
			grindEdge = NULL;
			reversed = false;

			velocity = dir * s;
			velocity.x *= .6;



			if (velocity.y > 0)
				velocity.y *= .5;

			SetAction(BOOSTERBOUNCE);
			frame = 0;
		}	
	}


	RestoreAirOptions();
}

bool Actor::IsGoalKillAction(int a)
{
	return (a == GOALKILL || a == GOALKILL1 || a == GOALKILL2 || a == GOALKILL3 || a == GOALKILL4 || a == GOALKILLWAIT
		|| a == NEXUSKILL || a == SEQ_FLOAT_TO_NEXUS_OPENING || a == SEQ_FADE_INTO_NEXUS);
}

void Actor::QueryTouchGrass()
{
	if (simulationMode)
		return;

	Rect<double> queryR = hurtBody.GetAABB();//(position.x - b.rw + b.offset.x, position.y - b.rh + b.offset.y, 2 * b.rw, 2 * b.rh);
	Rect<double> queryRExtended;
	if (currHitboxes != NULL)
	{
		queryR = currHitboxes->GetAABB(currHitboxFrame);
	}

	queryRExtended = queryR;
	double extra = 300;
	queryRExtended.left -= extra;
	queryRExtended.top -= extra;
	queryRExtended.width += extra * 2;
	queryRExtended.height += extra * 2;

	possibleEdgeCount = 0;

	polyQueryList = NULL;
	queryType = Q_TOUCHGRASSPOLY;
	GetBorderTree()->Query(this, queryRExtended);

	queryType = Q_TOUCHGRASS;
	PolyPtr tempT = polyQueryList;
	while (tempT != NULL)
	{
		tempT->QueryTouchGrass(this, queryR);
		tempT = tempT->queryNext;
	}
}

bool Actor::IsIntroAction(int a)
{
	return a == INTRO || a == SPAWNWAIT || a == INTROBOOST;
}

bool Actor::IsSequenceAction(int a)
{
	return a == SEQ_ENTERCORE1 || action == SEQ_LOOKUP || action == SEQ_KINTHROWN
		|| action == SEQ_KINFALL || action == SEQ_GATORSTUN;
}

bool Actor::IsExitAction(int a)
{
	return a == EXIT || a == EXITWAIT || a == WAITFORSHIP || a == GRABSHIP || a == EXITBOOST;
}

void Actor::SeqAfterCrawlerFight()
{
	PoiInfo *kp = owner->poiMap["kinaftercrawlerfight"];
	assert(kp != NULL);
	ground = kp->edge;
	edgeQuantity = kp->edgeQuantity;
	facingRight = true;
	offsetX = 0;
	SetAction(Actor::STAND);
	frame = 0;
}

void Actor::AddToCurrencyCounter(int count)
{
	currencyCounter += count;
	
	AdventureHUD *ah = sess->GetAdventureHUD();
	if( ah != NULL ) ah->currencyCountText.setString("x" + to_string(currencyCounter));
}

void Actor::HandleEntrant(QuadTreeEntrant *qte)
{
	if (queryType == Q_RESOLVE)
	{
		Edge *e = (Edge*)qte;

		//if (e->edgeType == Edge::OPEN_GATE)
		/*if( e->IsUnlockedGateEdge() )
		{
			return;
		}*/
		if (e->IsOpenGateEdge())
		{
			return;
		}
		if (e->edgeType == Edge::BARRIER)
		{
			Barrier *b = (Barrier*)(e->info);
			if (!b->edgeActive)
			{
				return;
			}
		}
		else if (e->poly != NULL && !e->poly->IsActive())
		{
			return;
		}
		else if (e->rail != NULL )
		{
			if (!e->rail->IsActive())
			{
				return;
			}
			else if (e->rail->IsTerrainType())
			{
				if (!e->rail->IsEdgeActive(e))
				{
					return;
				}
				else if (ground != NULL && ((ground->v0 == e->v0) || (ground->v1 == e->v1)))
				{
					//cout << "ignored" << endl;
					return;
				}
				else if (e->rail->GetRailType() == TerrainRail::SCORPIONONLY)
				{
					if (!scorpOn)
					{
						return;
					}
				}
				else if (e->rail->GetRailType() == TerrainRail::BOUNCE)
				{
					if (scorpOn)
					{
						return;
					}
				}
				else if ( e->rail->GetRailType() == TerrainRail::FLOOR)
				{
					if (ground == NULL)
					{
						if (currInput.LDown() && !IsAttackAction(action) && !currInput.LLeft() && !currInput.LRight())
						{
							return;
						}
					}
					else
					{
						if (ground->GetNextEdge() == e || ground->GetPrevEdge() == e)
						{

						}
						else if (currInput.LDown())
						{
							return;
						}
					}
					
				}
			}
			else
			{
				return;
			}
			
		}

		bool bb = false;

		if (ground != NULL && groundSpeed != 0)
		{
			V2d gn = ground->Normal();
			//bb fixes the fact that its easier to hit corners now, so it doesnt happen while you're running
			V2d nextn, prevn;

			if (ground->edge1 != NULL)
			{
				nextn = ground->edge1->Normal();
			}
			if (ground->edge0 != NULL)
			{
				prevn = ground->edge0->Normal();
			}


			bool a = false;
			bool b = false;
			if (!reversed)
			{
				if (groundSpeed > 0 && ground->edge1 != NULL )
				{
					if ((ground->edge1 == e
						&& ((gn.x > 0 && nextn.x > 0 && nextn.y < 0) || (gn.x < 0 && nextn.x < 0 && nextn.y < 0)))
						|| ground->edge0 == e)
					{
						a = true;
					}
				}
				else if (groundSpeed < 0 && ground->edge0 != NULL )
				{
					if ((ground->edge0 == e
						&& ((gn.x < 0 && prevn.x < 0 && prevn.y < 0) || (gn.x > 0 && prevn.x > 0 && prevn.y < 0)))
						|| ground->edge1 == e)
					{
						a = true;
					}
				}
			}
			else
			{
				if (ground->edge0 != NULL && ground->edge1 != NULL)
				{
					if (groundSpeed > 0 )
					{
						if ((ground->edge0 == e
							&& ((gn.x < 0 && prevn.x < 0 && prevn.y > 0) || (gn.x > 0 && prevn.x > 0 && prevn.y > 0)))
							|| ground->edge1 == e)
						{
							//cout << "one" << endl;
							b = true;
						}
					}
					else if (groundSpeed < 0)
					{
						bool c = ground->edge1 == e;
						bool h = (gn.x > 0 && nextn.x > 0 && nextn.y > 0);
						bool g = (gn.x < 0 && nextn.x < 0 && nextn.y > 0);
						bool d = h || g;
						bool f = ground->edge0 == e;
						if ((c && d) || f)
						{
							b = true;
						}
					}
				}
			}
			//	a = false;
			//	b = false;


				//a = !reversed && ((groundSpeed > 0 && gn.x < 0 && nextn.x < 0 && nextn.y < 0) || ( groundSpeed < 0 && gn.x > 0 && prevn.x > 0 && prevn.y < 0 )
				//	|| ( groundSpeed > 0 && gn.x > 0 && nextn.x > 0 && prevn.y < 0 ) || ( groundSpeed < 0 && gn.x < 0 && prevn.x < 0 && prevn.y < 0 ));
				//bool b = reversed && (( gn.x < 0 && nextn.x < 0 || ( gn.x > 0 && prevn.x > 0 )));
			bb = (a || b);
		}




		if (e == ground || bb)
		{
			return;
		}

		//so you can run on gates without transfer issues hopefully
		if (ground != NULL && ground->IsLockedGateEdge() )
		{
			Gate *g = (Gate*)ground->info;
			Edge *edgeA = g->edgeA;
			Edge *edgeB = g->edgeB;
			if (ground == g->edgeA)
			{
				if (e == edgeB->edge0
					|| e == edgeB->edge1
					|| e == edgeB)
				{
					return;
				}


			}
			else if (ground == g->edgeB)
			{
				if (e == edgeA->edge0
					|| e == edgeA->edge1
					|| e == edgeA)
				{
					return;
				}
			}
		}
		else if (ground != NULL)
		{
			if (groundSpeed > 0)
			{
				if (ground->edge0 != NULL && ground->edge0->IsLockedGateEdge() )
				{
					Gate *g = (Gate*)ground->edge0->info;
					Edge *e0 = ground->edge0;
					if (e0 == g->edgeA)
					{
						Edge *edgeB = g->edgeB;
						if (e == edgeB->edge0
							|| e == edgeB->edge1
							|| e == edgeB)
						{
							return;
						}
					}
					else if (e0 == g->edgeB)
					{
						Edge *edgeA = g->edgeA;
						if (e == edgeA->edge0
							|| e == edgeA->edge1
							|| e == edgeA)
						{
							return;
						}
					}
				}
			}
			else if (groundSpeed < 0)
			{
				if (ground->edge1 != NULL && ground->edge1->IsLockedGateEdge())
				{
					Gate *g = (Gate*)ground->edge1->info;
					Edge *e1 = ground->edge1;
					if (e1 == g->edgeA)
					{
						Edge *edgeB = g->edgeB;
						if (e == edgeB->edge0
							|| e == edgeB->edge1
							|| e == edgeB)
						{
							return;
						}
					}
					else if (e1 == g->edgeB)
					{
						Edge *edgeA = g->edgeA;
						if (e == edgeA->edge0
							|| e == edgeA->edge1
							|| e == edgeA)
						{
							return;
						}
					}
				}
			}
		}

		Contact *c = GetCollider().collideEdge(position + b.offset, b, e, tempVel, V2d(0, 0));

		if (c != NULL)
		{
			//cout << "c isnt null: " << e->Normal().x << ", " << e->Normal().y << endl;
			bool surface = (c->normal.x == 0 && c->normal.y == 0);

			//these make sure its a point of conention and not the other edge end point
			double len0 = length(c->position - e->v0);
			double len1 = length(c->position - e->v1);

			//recently added IsSoft because sometimes you would pass through a soft gate and hit this condition from a wall 
			//into a steep slope and it would ignore the edge
			if ( e->edge0 != NULL && e->edge0->IsGateEdge() && len0 < 1 && !e->edge0->GetGate()->IsSoft())
			{

				//cout << "len0: " << len0 << endl;
				V2d pVec = normalize(position - e->v0);
				double pAngle = atan2(-pVec.y, pVec.x);

				if (pAngle < 0)
				{
					pAngle += 2 * PI;
				}

				Edge *e0 = e->edge0;
				Gate *g = (Gate*)e0->info;

				V2d startVec = normalize(e0->v0 - e->v0);
				V2d endVec = normalize(e->v1 - e->v0);

				double diff = GetVectorAngleDiffCW(startVec, endVec);
				double pDiff = GetVectorAngleDiffCW(startVec, pVec);

				if (pDiff > diff)
				{
					return;
				}


				//double startAngle = atan2(-startVec.y, startVec.x);
				//if (startAngle < 0)
				//{
				//	startAngle += 2 * PI;
				//}
				//double endAngle = atan2(-endVec.y, endVec.x);
				//if (endAngle < 0)
				//{
				//	endAngle += 2 * PI;
				//}

				//double temp = startAngle;
				//startAngle = endAngle;
				//endAngle = temp;

				//if (endAngle < startAngle)
				//{
				//	if (pAngle >= endAngle || pAngle <= startAngle)
				//	{

				//	}
				//	else
				//	{
				//		//return;
				//	}
				//}
				//else
				//{
				//	if (pAngle >= startAngle && pAngle <= endAngle)
				//	{
				//	}
				//	else
				//	{
				//	//	return;
				//	}
				//}


			}
			else if ( e->edge1 != NULL && e->edge1->IsGateEdge() && len1 < 1 && !e->edge1->GetGate()->IsSoft())
			{
				//cout << "len1: " << len1 << endl;
				V2d pVec = normalize(position - e->v1);
				double pAngle = atan2(-pVec.y, pVec.x);

				if (pAngle < 0)
				{
					pAngle += 2 * PI;
				}

				Edge *e1 = e->edge1;
				Gate *g = (Gate*)e1->info;

				V2d startVec = normalize(e->v0 - e->v1);
				V2d endVec = normalize(e1->v1 - e->v1);

				double diff = GetVectorAngleDiffCW(startVec, endVec);
				double pDiff = GetVectorAngleDiffCW(startVec, pVec);

				if (pDiff > diff)
				{
					return;
				}


				//double startAngle = atan2(-startVec.y, startVec.x);
				//if (startAngle < 0)
				//{
				//	startAngle += 2 * PI;
				//}
				//double endAngle = atan2(-endVec.y, endVec.x);
				//if (endAngle < 0)
				//{
				//	endAngle += 2 * PI;
				//}

				//double temp = startAngle;
				//startAngle = endAngle;
				//endAngle = temp;

				//if (endAngle < startAngle)
				//{
				//	if (pAngle >= endAngle || pAngle <= startAngle)
				//	{

				//	}
				//	else
				//	{
				//	//	return;
				//	}
				//}
				//else
				//{
				//	if (pAngle >= startAngle && pAngle <= endAngle)
				//	{
				//	}
				//	else
				//	{
				//		//return;
				//	
				//	}
				//}
			}
			else if (c->edge->IsGateEdge() )
			{
				Gate *g = c->edge->GetGate();

				if (CanUnlockGate(g))
				{

					SetTouchedGate(g);

					UnlockGate(g);

					return;

				}
			}
			else if (e->rail != NULL)
			{
				if (e->edge0 != NULL  && e->v0 == c->position)
				{
					double c = cross(position - e->v0, e->edge0->Along());
					if (c <= 0)
					{
						return;
					}
				}
				else if (e->edge1 != NULL && e->v1 == c->position)
				{
					double c = cross(position - e->v1, e->edge1->Along());
					if (c <= 0)
					{
						return;
					}
				}
			}

			if ((c->normal.x == 0 && c->normal.y == 0)) //non point
			{
				//cout << "SURFACE. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << ", pri: " << c->collisionPriority << endl;
			}
			else //point
			{
				//cout << "POINT. n: " << c->edge->Normal().x << ", " << c->edge->Normal().y << endl;
			}

			//if (c->weirdPoint)
			//{
			//	cout << "weird point\n";

			//	//okay, I fixed the bugs involving this. There might still
			//	//be cases where weirdpoint solves need to be implemented.
			//	//ill put some prints here and then if someone encounters a bug
			//	//ill know where to look.

			//	Edge *edge = e;
			//	Edge *prev = edge->edge0;
			//	Edge *next = edge->edge1;

			//	V2d v0 = edge->v0;
			//	V2d v1 = edge->v1;

			//	//note: approxequals could be broken slightly
			//	if (approxEquals(c->position.x, e->v0.x) && approxEquals(c->position.y, e->v0.y))
			//	{
			//		V2d pv0 = prev->v0;
			//		V2d pv1 = prev->v1;

			//		V2d pn = prev->Normal();
			//		V2d en = e->Normal();

			//		//second weirdpoint glitch. remove for now. similar to the one
			//		//below it where it can make you fall through terrain.
			//		//i dont know which case i added this in for in the
			//		//first place.

			//		//this causes a fallthrough on the launcher/water map
			//		//if you land right on the corner.
			//		if (ground == NULL && pn.y >= 0 && en.y < 0)
			//		{
			//			//falling off and you dont want to keep hitting the ground
			//			//assert(!reversed);
			//			//return;
			//		}

			//		//--end second weirdpoint glitch

			//		//first weirdpointglitch is right here. 
			//		//ground != NULL
			//		if (pn.y < en.y)
			//		{
			//			//this could cause some glitches. patch them up as they come. prioritizes ground/higher up edges i think? kinda weird
			//			//cout << "sfdfdsfsdfdsfds" << endl;


			//			//commented the 2 lines below out because of a bug outlined below in the next case. 
			//			//i dont remember what case this code is solving. if you find a weird bug, 
			//			//come back and fix this code for both cases, as well as writing down
			//			//what this code is here for.

			//			//c->edge = prev;
			//			//return;

			//			//c->normal = V2d( 0, -1 );
			//		}
			//	}
			//	else if (approxEquals(c->position.x, e->v1.x) && approxEquals(c->position.y, e->v1.y))
			//	{
			//		V2d nn = next->Normal();
			//		V2d en = e->Normal();
			//		if (ground == NULL && en.y < 0 && nn.y >= 0)
			//		{
			//			//falling off and you dont want to keep hitting the ground
			//			//assert(!reversed);
			//			//return;
			//		}

			//		//ground != NULL
			//		if (nn.y < en.y)
			//		{
			//			//this could cause some glitches. patch them up as they come. prioritizes ground/higher up edges i think? kinda weird
			//			//cout << "herererere" << endl;
			//		//	return;



			//			//previously these next 2 lines were uncommented. It caused a bug where you'd fall through the terrain
			//			//when you were wallsliding on a steep reverse slope with a very low ceiling above it.
			//			//I don't know what case this is supposed to cover, but removing these 2 lines
			//			//fixes the bug. 


			//			//c->edge = next;
			//			//return;






			//			//c->normal = V2d( 0, -1 );
			//		}
			//	}
			//}

			if (!col || (minContact.collisionPriority < 0)
				|| (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0))//|| ( closedGate && !minGate ) )
			{
				if (c->collisionPriority == minContact.collisionPriority)//&& !closedGate )
				{
					if ((c->normal.x == 0 && c->normal.y == 0))//|| minContact.normal.y  0 )
					//if( length(c->resolution) > length(minContact.resolution) )
					{
						//	cout << "now the min" << endl;
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.normal = c->normal;
						col = true;
					}
					else
					{
						//cout << "happens here!!!!" << endl;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					//cout << "pri: " << c->collisionPriority << endl;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.normal = c->normal;
					col = true;

				}
			}
		}

	}
	else if (queryType == Q_CHECK)
	{
		Edge *e = (Edge*)qte;

		if (e->IsUnlockedGateEdge())
		{
			return;
		}
		else if (e->poly != NULL && !e->poly->IsActive())
		{
			return;
		}
		if (e->rail != NULL 
			&& (!e->rail->IsTerrainType() || !e->rail->IsEdgeActive( e )))
		{
			return;
		}
		//cout << "checking: " << e << endl;
		if ((grindEdge == NULL && ground == e) || grindEdge == e)
			return;

		//Edge *testEdge = ground;


		if (ground != NULL && ground->rail != NULL && ground->rail == e->rail)
		{
			if (ground->v0 == e->v1 && ground->v1 == e->v0)
			{
				return; //this might not cover everything
			}
		}


		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight );
		//Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		if (action != GRINDBALL && action != GRINDATTACK)
		{
			//cout << "here" << endl;
			if (ground != NULL)
			{



				V2d en = e->Normal();
				if (reversed)
				{
					if (en.y >= 0)
					{
						return;
					}
				}
				else
				{
					if (e == ground->edge1)
					{
						double c = cross(normalize(e->v1 - ground->v0),
							normalize(ground->v1 - ground->v0));
						if (en.y >= 0 && c < 0)
						{
							return;
						}
					}
					else if (e == ground->edge0)
					{
						double c = cross(normalize(e->v0 - ground->v1),
							normalize(ground->v0 - ground->v1));
						if (en.y >= 0 && c > 0)
						{
							return;
						}
					}

					if (en.y <= 0)
					{



						return;
					}
				}


				//for travelling so you don't hit the other side of the gate on accident
				if (ground->IsGateEdge() )
				{
					Gate *g = (Gate*)ground->info;
					if (ground == g->edgeA)
					{
						if (e == g->edgeB || e == g->edgeB->edge0 || e == g->edgeB->edge1)
						{
							return;
						}
					}
					else
					{
						if (e == g->edgeA || e == g->edgeA->edge0 || e == g->edgeA->edge1)
						{
							return;
						}
					}

					if (e == g->edgeA && ground == g->edgeB
						|| e == g->edgeB && ground == g->edgeA)//|| e == g->edgeB )
					{

						//cout << "returnning early" << endl;
						return;
					}
				}
				else if ( ground->edge1 != NULL && groundSpeed > 0 && ground->edge1->IsGateEdge() )
				{
					Edge *e1 = ground->edge1;
					Gate *g = (Gate*)e1->info;
					if (e == g->edgeA && e1 == g->edgeB
						|| e == g->edgeB && e1 == g->edgeA)
					{
						return;
					}
				}
				else if (ground->edge0 != NULL &&groundSpeed < 0 && ground->edge0->IsGateEdge() )
				{
					Edge *e0 = ground->edge0;
					Gate *g = (Gate*)e0->info;
					if (e == g->edgeA && e0 == g->edgeB
						|| e == g->edgeB && e0 == g->edgeA)
					{
						return;
					}
				}

			}
		}
		else
		{
			if (grindEdge->IsGateEdge() )
			{
				Gate *g = (Gate*)grindEdge->info;
				if (grindEdge == g->edgeA)
				{
					if (e == g->edgeB || e == g->edgeB->edge0 || e == g->edgeB->edge1)
					{
						return;
					}
				}
				else
				{
					if (e == g->edgeA || e == g->edgeA->edge0 || e == g->edgeA->edge1)
					{
						return;
					}
				}

				if (e == g->edgeA && grindEdge == g->edgeB
					|| e == g->edgeB && grindEdge == g->edgeA)//|| e == g->edgeB )
				{

					//cout << "returnning early" << endl;
					return;
				}
			}
			else if (grindSpeed > 0 && grindEdge->edge1 != NULL && grindEdge->edge1->IsGateEdge() )
			{
				Edge *e1 = grindEdge->edge1;
				Gate *g = (Gate*)e1->info;
				if (e == g->edgeA && e1 == g->edgeB
					|| e == g->edgeB && e1 == g->edgeA)
				{
					return;
				}
			}
			else if (grindSpeed < 0 && grindEdge->edge0 != NULL && grindEdge->edge0->IsGateEdge() )
			{
				Edge *e0 = grindEdge->edge0;
				Gate *g = (Gate*)e0->info;
				if (e == g->edgeA && e0 == g->edgeB
					|| e == g->edgeB && e0 == g->edgeA)
				{
					return;
				}
			}
		}

		//cout << "hit edge: " << e->Normal().x << ", " << e->Normal().y << endl;
		/*if( grindEdge != NULL && (e == grindEdge->edge0 || e== grindEdge->edge1 ) )
		{

		}*/
		//cout << "valid is false" << endl;
		checkValid = false;

		//}
	}
	else if (queryType == Q_CHECKWALL)
	{
		Edge *e = (Edge*)qte;
		if (ground == e)
			return;

		if (e->rail != NULL)
		{
			if (!e->rail->IsActive())
			{
				return;
			}
			else if (!e->rail->IsTerrainType())// && e->rail->IsActive() && e->rail->IsEdgeActive( e ) )
			{
				return;
			}
			else if (e->rail->GetRailType() == TerrainRail::BOUNCE)
			{
				return;
			}
		}
		else if (e->poly != NULL && !e->poly->IsActive())
		{
			return;
		}

		if (e->IsUnlockedGateEdge())
		{
			return;
		}

		if (action == SPRINGSTUNBOUNCE || action == SPRINGSTUNGRINDFLY )
		{
			return;
		}

		Contact *c = GetCollider().collideEdge(position + tempVel, b, e, tempVel, V2d(0, 0));

		if (c != NULL)
			if (!col || (c->collisionPriority >= -.00001 && (c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001)))
			{
				if (c->collisionPriority == minContact.collisionPriority)
				{
					if (length(c->resolution) > length(minContact.resolution))
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						col = true;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					col = true;

				}
			}
	}
	else if (queryType == Q_CHECK_GATE)
	{
		Edge *e = (Edge*)qte;
		if (!e->IsUnlockedGateEdge())
		{
			return;
		}

		Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
		if (e->IsTouchingBox(r))
		{
			Gate *g = e->GetGate();

			if (CanUnlockGate(g))
			{
				//UnlockGate(g); //not sure why this is on so many of the other ones. figure out what is right

				if (gateTouched == NULL)
				{
					//cout << "set touched gate from check gate\n";
					SetTouchedGate(g);

					UnlockGate(g); //recently added these to line them up with other stuff, could be bugs?
				}
				else if (gateTouched != NULL && gateTouched != g)
				{
					//this means gates are both getting touched by you at once which is a problem in the engine anyway
					//assuming its not going to happen in any normal levels
					//assert(0);
					SetTouchedGate(g);

					UnlockGate(g);

				}
			}
		}
	}
	else if (queryType == Q_CHECK_GRIND_TRANSFER)
	{
		Edge *e = (Edge*)qte;

		if (e->IsUnlockedGateEdge())
		{
			return;
		}
		else if (e->poly != NULL && !e->poly->IsActive())
		{
			return;
		}
		if (e->rail != NULL
			&& (!e->rail->IsTerrainType() || !e->rail->IsEdgeActive(e)))
		{
			return;
		}

		if (e == grindEdge || e == grindEdge->edge0 || e == grindEdge->edge1)
		{
			//not my edge or those adjacent to me
			return;
		}
		
		if (e->edgeType == Edge::BORDER)
		{
			return;
		}

		//not sure if this applies correctly
		if (grindEdge->IsGateEdge())
		{
			Gate *g = (Gate*)grindEdge->info;
			if (grindEdge == g->edgeA)
			{
				if (e == g->edgeB || e == g->edgeB->edge0 || e == g->edgeB->edge1)
				{
					return;
				}
			}
			else
			{
				if (e == g->edgeA || e == g->edgeA->edge0 || e == g->edgeA->edge1)
				{
					return;
				}
			}

			if (e == g->edgeA && grindEdge == g->edgeB
				|| e == g->edgeB && grindEdge == g->edgeA)//|| e == g->edgeB )
			{

				//cout << "returnning early" << endl;
				return;
			}
		}
		else if (grindSpeed > 0 && grindEdge->edge1 != NULL && grindEdge->edge1->IsGateEdge())
		{
			Edge *e1 = grindEdge->edge1;
			Gate *g = (Gate*)e1->info;
			if (e == g->edgeA && e1 == g->edgeB
				|| e == g->edgeB && e1 == g->edgeA)
			{
				return;
			}
		}
		else if (grindSpeed < 0 && grindEdge->edge0 != NULL && grindEdge->edge0->IsGateEdge())
		{
			Edge *e0 = grindEdge->edge0;
			Gate *g = (Gate*)e0->info;
			if (e == g->edgeA && e0 == g->edgeB
				|| e == g->edgeB && e0 == g->edgeA)
			{
				return;
			}
		}

		
		
		if (grindTransferCheckEdge == NULL)
		{
			grindTransferCheckEdge = e;
		}
		else
		{
			double eQuant = e->GetQuantity(position);
			V2d ePos = e->GetPosition(eQuant);

			double currQuant = grindTransferCheckEdge->GetQuantity( position );
			V2d currPos = grindTransferCheckEdge->GetPosition(currQuant);

			if (length(ePos - position) < length(currPos - position))
			{
				grindTransferCheckEdge = e;
			}

		}		
	}
	else if (queryType == Q_RAIL_GRIND_TERRAIN_CHECK)
	{
		Edge *e = (Edge*)qte;

		if (e->IsUnlockedGateEdge())
		{
			return;
		}
		else if (e->poly != NULL && !e->poly->IsActive())
		{
			return;
		}

		assert(e->rail == NULL);
		//assert(grindEdge->rail != NULL);

		if (e->rail != NULL)
			return;

		if (grindEdge->rail == NULL)
			return;

		//if momentum is parallel to edge or away from it ignore it
		if (dot(tempVel, e->Normal()) >= 0)
		{
			return;
		}

		V2d grindPos = grindEdge->GetPosition(edgeQuantity);

		LineIntersection li = SegmentIntersect(grindPos, grindPos - tempVel, e->v0, e->v1);
	
		if (!li.parallel )
		{
			V2d p = li.position;
			double quant = e->GetQuantity(p);

			//if (quant >= 0 && quant <= e->GetLength())
			{
				grindEdge = e;
				edgeQuantity = quant;
				CheckGrindEdgeForTerrainFade();
				//SetAction(GRINDBALL);
			}
		}
		else
		{
			//if no intersection, just ignore
		}
	}
	else if (queryType == Q_TERRAIN_GRIND_RAIL_CHECK)
	{
		Edge *e = (Edge*)qte;

		if (e->IsUnlockedGateEdge())
		{
			return;
		}
		else if (e->rail != NULL)
		{
			if (!e->rail->IsActive())
			{
				return;
			}
			else if (e->rail->IsTerrainType())
			{
				if (!e->rail->IsEdgeActive(e))
				{
					return;
				}
				else if (e->rail->GetRailType() == TerrainRail::SCORPIONONLY)
				{
					if (!scorpOn)
					{
						return;
					}
				}
				else if (e->rail->GetRailType() == TerrainRail::BOUNCE)
				{
					if (scorpOn)
					{
						return;
					}
				}
				else if (e->rail->GetRailType() == TerrainRail::FLOOR)
				{
					if (ground == NULL)
					{
						if (currInput.LDown() && !IsAttackAction(action) && !currInput.LLeft() && !currInput.LRight())
						{
							return;
						}
					}
					else
					{
						if (ground->GetNextEdge() == e || ground->GetPrevEdge() == e)
						{

						}
						else if (currInput.LDown())
						{
							return;
						}
					}

				}
			}
			else
			{
				return;
			}

		}

		assert(e->rail != NULL);
		//assert(grindEdge->rail == NULL);

		if (e->rail == NULL)
			return;

		if (grindEdge->rail != NULL)
			return;

		//if momentum is parallel to edge or away from it ignore it
		if (dot(tempVel, e->Normal()) >= 0)
		{
			return;
		}

		V2d grindPos = grindEdge->GetPosition(edgeQuantity);

		LineIntersection li = SegmentIntersect(grindPos, grindPos - tempVel, e->v0, e->v1);

		if (!li.parallel)
		{
			V2d p = li.position;
			double quant = e->GetQuantity(p);

			//if (quant >= 0 && quant <= e->GetLength())
			{
				grindEdge = e;
				edgeQuantity = quant;
				CheckGrindEdgeForTerrainFade();
				//SetAction(RAILGRIND);
			}
		}
		else
		{
			//if no intersection, just ignore
		}
	}
	else if (queryType == Q_GRASS)
	{
		//cout << "got some grass in here" << endl;
		Grass *g = (Grass*)qte;
		Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
		if (g->IsTouchingBox(r))
		{
			touchedGrass[g->grassType] = true;
		}
	}
	else if (queryType == Q_ENVPLANT)
	{
		EnvPlant *ev = (EnvPlant*)qte;

		if (!ev->activated)
		{
			ev->activated = true;
			if (ground != NULL)
			{
				ev->particle->dir = ground->Normal();//normalize( normalize( ground->v1 - ground->v0 ) * groundSpeed );
			}
			else
			{
				ev->particle->dir = normalize(velocity);
			}

			if (owner->activeEnvPlants == NULL)
			{
				ev->next = NULL;
				owner->activeEnvPlants = ev;
			}
			else
			{
				ev->next = owner->activeEnvPlants;
				owner->activeEnvPlants = ev;
			}
			ev->frame = 0;
		}
	}
	else if (queryType == Q_RAIL)
	{
		Edge *e = (Edge*)qte;
		RailPtr rail = e->rail;

		if (action == LOCKEDRAILSLIDE)
			return;

		if ( grindEdge != NULL && grindEdge->rail == rail)
		{
			return;
		}

		if (e->rail->GetRailType() != TerrainRail::LOCKED && !CanRailSlide() )
		{
			return;
		}

		if ((rail->RequiresPowerToGrind() && !canRailGrind) 
			|| IsInHistunAction(action) || rail->IsTerrainType()
			|| rail->GetRailType() == TerrainRail::WIREBLOCKING 
			|| rail->GetRailType() == TerrainRail::WIREONLY )
		{
			return;
		}

		if (e->rail->GetRailType() == TerrainRail::LOCKED )
		{
			Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
			if (IsEdgeTouchingBox(e, r))
			{
				RestoreAirOptions();
				frame = 0;
				framesGrinding = 0;
				grindEdge = e;
				prevRail = grindEdge->rail;//->info;
				ground = NULL;
				bounceEdge = NULL;
				grindSpeed = 0;
				currWall = NULL;

				//CheckGrindEdgeForTerrainFade(); //not needed because its a locked rail already

				//leftGround = true;
				reversed = false;




				edgeQuantity = e->GetQuantity(position);

				
				SetAction(LOCKEDRAILSLIDE);
				return;
			}
		}
		


		V2d eNorm = e->Normal();
		if (eNorm.y > 0)
		{
			eNorm = -eNorm;
		}

		if (e->rail->GetRailType() != TerrainRail::ANTITIMESLOW)
		{
			//test this if you only wanna be able to grab from one side

			double velMatchesNormDot = dot(velocity, eNorm);
			if (velMatchesNormDot >= 0)
			{
				return;
			}
		}
		

		bool canGrabRail = (rail->RequiresPowerToGrind() && canRailGrind) 
			|| (!rail->RequiresPowerToGrind() && canRailSlide);

		if ((rail != prevRail || regrindOffCount == regrindOffMax) && canGrabRail)
		{
			V2d r;
			V2d eFocus;
			bool ceiling = false;
			V2d en = e->Normal();
			if (en.y > 0)
			{
				r = e->v0 - e->v1;
				eFocus = e->v1;
				ceiling = true;
			}
			else
			{
				r = e->v1 - e->v0;
				eFocus = e->v0;
			}
		
			V2d along = normalize(r);
			double lenR = length(r);
			double q = dot(position - eFocus, along);

			if (ground != NULL)
			{
				V2d gAlong = normalize(ground->v1 - ground->v0);
				V2d rAlong = along;
				double diff = GetVectorAngleDiffCW(rAlong, gAlong);
				diff -= PI;

				bool groundTransferOkay = (diff < 0 && groundSpeed > 0) || (diff > 0 && groundSpeed < 0);
				if (!groundTransferOkay)
					return;
			}


			double c = cross(position - e->v0, along);
			double preC = cross((position - tempVel) - e->v0, along);

			double alongQuantVel = dot(velocity, along);

			bool cStuff = (c >= 0 && preC <= 0) || (c <= 0 && preC >= 0);

			if (cStuff && q >= 0 && q <= lenR)//&& alongQuantVel != 0)
			{
				V2d rn(along.y, -along.x);


				double railSpeed;
				double minRailCurr = GetMinRailGrindSpeed();

				if (ground == NULL)
				{
					if (e->IsWall())
					{
						railSpeed = velocity.y;

						if (railSpeed >= 0 && railSpeed < minRailCurr)
							railSpeed = minRailCurr;
						else if (railSpeed <= 0 && railSpeed > -minRailCurr)
							railSpeed = -minRailCurr;
						
						if (e->Along().y < 0) //right wall, moving down
						{
							railSpeed = -railSpeed;
						}
					}
					else
					{
						double landingSpeed = CalcLandingSpeed(velocity, along, rn, true);
						if (landingSpeed == 0)
						{
							if (facingRight)
							{
								railSpeed = minRailCurr;
							}
							else
							{
								railSpeed = -minRailCurr;
							}
						}
						else if (landingSpeed > 0)
						{
							railSpeed = max(landingSpeed, minRailCurr);
						}
						else if (landingSpeed < 0)
						{
							railSpeed = min(landingSpeed, -minRailCurr);
						}

						if (!e->IsSteepGround())
						{
							if (velocity.x > 0 && railSpeed < velocity.x)//groundSpeed > 0 && groundSpeed < velocity.x)
							{
								//cout << "railspeed a from " << railSpeed << " to : " << velocity.x << "\n";
								railSpeed = max(minRailCurr, velocity.x);

							}
							else if (velocity.x < 0 && railSpeed > velocity.x)//groundSpeed < 0 && groundSpeed > velocity.x)
							{
								//cout << "railspeed b from " << railSpeed << " to : " << velocity.x << "\n";
								railSpeed = min(-minRailCurr, velocity.x);
							}
						}
					}


					
				}
				else
				{
					if (groundSpeed > 0)
					{
						railSpeed = max(groundSpeed, minRailCurr);
					}
					else if (groundSpeed < 0)
					{
						railSpeed = min(groundSpeed, -minRailCurr);
					}
					else
					{
						if (facingRight)
						{
							railSpeed = minRailCurr;
						}
						else
						{
							railSpeed = -minRailCurr;
						}
					}
				}


				

				double oldDist = e->GetDistAlongNormal(position - tempVel);
				double newDist = e->GetDistAlongNormal(position);


				//this makes it so that jumping off of the rail doesn't accidentally catch you and put you back on
				if (abs(oldDist) < .00000001)
				{
					oldDist = 0;
					
				}

				if ((oldDist >= 0 && newDist > 0)//used to be >=
					|| (oldDist <= 0 && newDist < 0))//used to be <=
				{
					return;
				}
				
				LineIntersection li;
				lineIntersection( li, position, position - tempVel, e->v0, e->v1);
				if (!li.parallel)
				{
					V2d p = li.position;
					edgeQuantity = e->GetQuantity(p);
				}
				else
				{
					cout << "parallel with rail issue. Already checked to avoid this" << endl;
					assert(0);
				}

				RestoreAirOptions();
				frame = 0;
				framesGrinding = 0;
				grindEdge = e;
				prevRail = grindEdge->rail;//->info;
				ground = NULL;
				bounceEdge = NULL;

				//CheckGrindEdgeForTerrainFade(); //not needed because these are only for rails that aren't terrain

				

				

				//SegmentIntersect()
				//LineIntersection li;
				////lineIntersection( li, position, testPos, grindEdge->v0, grindEdge->v1);
				//li = SegmentIntersect(position, testPos, grindEdge->v0, grindEdge->v1);
				//if (!li.parallel)
				//{
				//	V2d p = li.position;
				//	edgeQuantity = grindEdge->GetQuantity(p);
				//}
				//else
				//{
				//	cout << "parallel with rail issue" << endl;
				//	assert(0);
				//}

				if (ceiling) //ceiling rail
				{
					grindSpeed = -railSpeed;
				}
				else
				{
					grindSpeed = railSpeed;
				}

				SetAction(RAILSLIDE);

				/*if (canRailGrind && CanBufferGrind())
				{
					SetAction(RAILGRIND);
				}
				else
				{
					SetAction(RAILSLIDE);
				}*/
			}
		}
	}
	else if (queryType == Q_ACTIVEITEM)
	{
		Enemy *en = (Enemy*)qte;
		if (en->type == EnemyType::EN_TUTORIALOBJECT)
		{
			TutorialObject *tut = (TutorialObject*)qte;

			if (tut->spawned)
			{
				if (currTutorialObject == NULL)
				{
					if (tut->TryActivate())
					{
						currTutorialObject = tut;
					}
				}
			}
		}
		else if (en->type == EnemyType::EN_INSPECTOBJECT)
		{
			InspectObject *iobj = (InspectObject*)qte;

			if (iobj->spawned)
			{
				if (currInspectObject == NULL)
				{
					if (iobj->TryActivate())
					{
						currInspectObject = iobj;
					}
				}
			}
		}
		else if (en->type == EnemyType::EN_BOOSTER)
		{
			Booster *boost = (Booster*)qte;

			if (currBooster == NULL)
			{
				if (boost->hitBody.Intersects(boost->currHitboxFrame, &hurtBody) && boost->IsBoostable())
				{
					currBooster = boost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_BOUNCEBOOSTER)
		{
			BounceBooster *boost = (BounceBooster*)qte;

			if (currBounceBooster == NULL)
			{
				if (boost->hitBody.Intersects(boost->currHitboxFrame, &hurtBody) && boost->IsBoostable())
				{
					currBounceBooster = boost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_SCORPIONLAUNCHER)
		{
			ScorpionLauncher *sLaunch = (ScorpionLauncher*)qte;

			if (currScorpionLauncher == NULL)
			{
				if (sLaunch->hitBody.Intersects(sLaunch->currHitboxFrame, &hurtBody) 
					&& sLaunch->action == Spring::IDLE)
				{
					currScorpionLauncher = sLaunch;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_TIMEBOOSTER)
		{
			TimeBooster *tboost = (TimeBooster*)qte;

			if (currTimeBooster == NULL)
			{
				if (tboost->hitBody.Intersects(tboost->currHitboxFrame, &hurtBody) && tboost->IsBoostable())
				{
					currTimeBooster = tboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_HOMINGBOOSTER)
		{
			HomingBooster *hboost = (HomingBooster*)qte;

			if (currHomingBooster == NULL)
			{
				if (hboost->hitBody.Intersects(hboost->currHitboxFrame, 
					&hurtBody) && hboost->IsBoostable())
				{
					currHomingBooster = hboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_FREEFLIGHTBOOSTER)
		{
			FreeFlightBooster *ffboost = (FreeFlightBooster*)qte;

			if (currFreeFlightBooster == NULL)
			{
				if (ffboost->hitBody.Intersects(ffboost->currHitboxFrame, &hurtBody) && ffboost->IsBoostable())
				{
					currFreeFlightBooster = ffboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_ANTITIMESLOWBOOSTER)
		{
			AntiTimeSlowBooster *atsboost = (AntiTimeSlowBooster*)qte;

			if (currAntiTimeSlowBooster == NULL)
			{
				if (atsboost->hitBody.Intersects(atsboost->currHitboxFrame,
					&hurtBody) && atsboost->IsBoostable())
				{
					currAntiTimeSlowBooster = atsboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_SWORDPROJECTILEBOOSTER)
		{
			SwordProjectileBooster *spboost = (SwordProjectileBooster*)qte;

			if (currSwordProjectileBooster == NULL)
			{
				if (spboost->hitBody.Intersects(spboost->currHitboxFrame,
					&hurtBody) && spboost->IsBoostable())
				{
					currSwordProjectileBooster = spboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_MOMENTUMBOOSTER)
		{
			MomentumBooster *mboost = (MomentumBooster*)qte;

			if (currMomentumBooster == NULL)
			{
				if (mboost->hitBody.Intersects(mboost->currHitboxFrame,
					&hurtBody) && mboost->IsBoostable())
				{
					currMomentumBooster = mboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_REWINDBOOSTER)
		{
			RewindBooster *rboost = (RewindBooster*)qte;

			if (currRewindBooster == NULL)
			{
				if (rboost->hitBody.Intersects(rboost->currHitboxFrame,
					&hurtBody) && rboost->IsBoostable())
				{
					currRewindBooster = rboost;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_GRAVITYMODIFIER)
		{
			GravityModifier *mod = (GravityModifier*)qte;

			if (currGravModifier == NULL)
			{
				if (mod->hitBody.Intersects(mod->currHitboxFrame, &hurtBody) && mod->IsModifiable())
				{
					currGravModifier = mod;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_SPECIALTARGET)
		{
			SpecialTarget *spTarget = (SpecialTarget*)qte;

			if (spTarget->IsInteractible())
			{
				switch (spTarget->targetType)
				{
				case SpecialTarget::TARGET_GLIDE:
				{
					bool isGlideAction = action == SPRINGSTUNGLIDE || action == WATERGLIDE || action == WATERGLIDECHARGE;

					if (isGlideAction && spTarget->hitBody.Intersects(spTarget->currHitboxFrame, &hurtBody))
					{
						spTarget->Collect();
					}
					break;
				}
				case SpecialTarget::TARGET_BOUNCE:
				{
					bool isValidAction = action == SPRINGSTUNAIM || action == SPRINGSTUNBOUNCEGROUND || action == SPRINGSTUNAIRBOUNCE
						|| action == SPRINGSTUNAIRBOUNCEPAUSE;

					if (isValidAction && spTarget->hitBody.Intersects(spTarget->currHitboxFrame, &hurtBody))
					{
						spTarget->Collect();
					}
					break;
				}
				case SpecialTarget::TARGET_SCORPION:
				{
					if (scorpOn && spTarget->hitBody.Intersects(spTarget->currHitboxFrame, &hurtBody))
					{
						spTarget->Collect();
					}
					break;
				}
				case SpecialTarget::TARGET_GRIND:
				{
					bool isValidAction = action == SPRINGSTUNGRINDFLY || action == SPRINGSTUNGRIND || action == RAILGRIND || action == GRINDBALL
						|| action == RAILSLIDE;

					if (isValidAction && spTarget->hitBody.Intersects(spTarget->currHitboxFrame, &hurtBody))
					{
						spTarget->Collect();
					}
					break;
				}
				case SpecialTarget::TARGET_SWING:
				{
					bool isValidAction = action == SWINGSTUN || rightWire->IsPulling() || leftWire->IsPulling();

					if (isValidAction && spTarget->hitBody.Intersects(spTarget->currHitboxFrame, &hurtBody))
					{
						spTarget->Collect();
					}
					break;
				}
				case SpecialTarget::TARGET_FREEFLIGHT:
				{
					bool isFreeFlightAction = action == FREEFLIGHT || action == FREEFLIGHTSTUN;
					
					if( isFreeFlightAction && spTarget->hitBody.Intersects(spTarget->currHitboxFrame, &hurtBody))
					{
						spTarget->Collect();
					}
					break;
				}
				}
			}
		}
		else if (en->type == EnemyType::EN_SPRING)
		{
			Spring *spr = (Spring*)qte;
			if (currSpring == NULL)
			{
				if (spr->hitBody.Intersects(spr->currHitboxFrame, &hurtBody) && spr->action == Spring::IDLE)
				{
					currSpring = spr;
				}
			}
			else
			{
				//some replacement formula later
			}
		}

		else if (en->type == EnemyType::EN_TELEPORTER)
		{
			Teleporter *tele = (Teleporter*)qte;
			if (currTeleporter == NULL)
			{
				if (tele->hitBody.Intersects(tele->currHitboxFrame, &hurtBody) && tele->action == Teleporter::IDLE)
				{
					currTeleporter = tele;
				}
			}
			else
			{
				//some replacement formula later
			}
		}
		else if (en->type == EnemyType::EN_SWINGLAUNCHER)
		{
			SwingLauncher *sw = (SwingLauncher*)qte;
			if (currSwingLauncher == NULL)
			{
				if (sw->hitBody.Intersects(sw->currHitboxFrame, &hurtBody) && sw->action == SwingLauncher::IDLE)
				{
					currSwingLauncher = sw;
				}
			}
		}
		else if (en->type == EnemyType::EN_AIMLAUNCHER)
		{
			AimLauncher *al = (AimLauncher*)qte;

			if (currAimLauncher == NULL)
			{
				if (al->hitBody.Intersects(al->currHitboxFrame, &hurtBody) && al->action == AimLauncher::IDLE)
				{
					currAimLauncher = al;
				}
			}
		}
		else if (en->type == EnemyType::EN_CURRENCYITEM)
		{
			CurrencyItem *ci = (CurrencyItem*)qte;

			if (ci->hurtBody.Intersects(ci->currHurtboxFrame, &hurtBody) && ci->IsCollectable())
			{
				CollectCurrency(ci);
			}
		}
	}
	else if (queryType == Q_AIRTRIGGER)
	{
		AirTrigger *at = (AirTrigger*)qte;
		currAirTrigger = at;
	}
	else if (queryType == Q_TOUCHGRASS)
	{
		TouchGrass *tGrass = (TouchGrass*)qte;
		if (currHitboxes != NULL)
		{
			if (tGrass->Intersects(currHitboxes, currHitboxFrame))
			{
				tGrass->Destroy( this );
			}
		}
		else
		{
			if (tGrass->hurtBody.Intersects(0, &hurtBody))
			{
				tGrass->Touch(this);
			}
		}
	}
	else if (queryType == Q_TOUCHGRASSPOLY)
	{
		PolyPtr poly = (PolyPtr)qte;
		poly->queryNext = NULL;
		if (polyQueryList == NULL)
			polyQueryList = poly;
		else
		{
			poly->queryNext = polyQueryList;
			polyQueryList = poly;
		}
			

		/*TerrainPiece *tPiece = (TerrainPiece*)qte;
		tPiece->next = NULL;
		if (polyQueryList == NULL)
		{
			polyQueryList = tPiece;
		}
		else
		{
			tPiece->next = polyQueryList;
			polyQueryList = tPiece;
		}*/
	}
	else if (queryType == Q_SPECIALTERRAIN)
	{
		if (currSpecialTerrain == NULL)
		{
			PolyPtr poly = (PolyPtr)qte;
			Rect<double> r(position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh);
			{
				if (poly->IsInsideArea(position))
				{
					currSpecialTerrain = poly;
				}
			}
		}

	}
	++possibleEdgeCount; //not needed
}

bool Actor::TryLandFromBounceGround()
{
	assert(bounceEdge != NULL);
	V2d bn = bounceNorm;
	if (bn.y < 0)
	{
		V2d alongVel = V2d(-bn.y, bn.x);
		ground = bounceEdge;
		bounceEdge = NULL;

		if (bn.y > -steepThresh)
		{

		}
		else
		{
		}
		//SetAction(LAND);
		//frame = 0;

		bounceFlameOn = true;
		scorpOn = true;

		V2d testVel = storedBounceVel;


		if (testVel.y > 20)
		{
			testVel.y *= .7;
		}
		else if (testVel.y < -30)
		{

			testVel.y *= .5;
		}

		groundSpeed = CalcLandingSpeed(testVel, alongVel, bn);

		V2d norm = GetGroundedNormal();

		//if( gNorm.y <= -steepThresh )
		{
			RestoreAirOptions();
		}

		if (testVel.x < 0 && norm.y <= -steepThresh)
		{
			groundSpeed = min(testVel.x, dot(testVel, normalize(ground->v1 - ground->v0)) * .7);
			//cout << "left boost: " << groundSpeed << endl;
		}
		else if (testVel.x > 0 && norm.y <= -steepThresh)
		{
			groundSpeed = max(testVel.x, dot(testVel, normalize(ground->v1 - ground->v0)) * .7);
			//cout << "right boost: " << groundSpeed << endl;
		}

		return true;
	}

	return false;
}

double Actor::CalcLandingSpeed( V2d &testVel,
		V2d &alongVel, V2d &gNorm, bool rail )
{
	//cout << "vel: " << velocity.x << ", " << velocity.y << " test: " << testVel.x << ", " << testVel.y;
	double gSpeed = 0; //groundSpeed;
	bool noLeftRight = !(currInput.LLeft() || currInput.LRight());

	double alongSpeed = dot(testVel, alongVel);

	if (action == BOOSTERBOUNCE) //special case, to avoid having to add a bunch of code to just do this same thing
	{
		gSpeed = alongSpeed;
	}
	else if ((HoldingRelativeDown() || rail ) && noLeftRight)
	{
		gSpeed = alongSpeed;
	}
	else if ((HoldingRelativeUp() || rail ) && noLeftRight)
	{
		gSpeed = alongSpeed;
	}
	else if( ( currInput.LLeft() && testVel.x <= 0 ) || ( currInput.LRight() && testVel.x >= 0 ))
	{
		double res = dot( testVel, alongVel );

		if( gNorm.y <= -steepThresh )
		{
			if( testVel.x > 0 && gNorm.x < 0 )
			{
				V2d straight( 1, 0 );
				double temp = dot(testVel, straight);
				res = max( res, temp );
			}
			else if( testVel.x < 0 && gNorm.x > 0 )
			{
				V2d straight( 1, 0 );
				double temp = dot(testVel, straight);
				res = min( res, temp );
			}
		}
		gSpeed = res;
					
		//groundSpeed = 
	}
	else
	{
		if( gNorm.y > -steepThresh )
		{
			gSpeed = dot( testVel, alongVel );
		}
		else
		{
			gSpeed = 0;
		}
	}

	//cout << " gspeed: " << gSpeed << endl;
	
	return gSpeed;
}

bool Actor::IsInvincible()
{
	return kinMode == K_SUPER || invincibleFrames > 0;
}


void Actor::ApplyHit( HitboxInfo *info,
	Actor *attackingPlayer, HitResult res, V2d &pos )
{
	if (info == NULL)
		return;

	//removed the below line because we already
	//check for invinc when getting hit.
	//this is redundant and ruins the boss moves 
	//hitting through invinc..might change later

	//use the first hit you got. no stacking hits for now
	//if (invincibleFrames == 0)
	
	{
		if (receivedHit.hType == HitboxInfo::NO_HITBOX || info->damage > receivedHit.damage)
		{
			//cout << "apply hit" << endl;
			receivedHit = *info;
			receivedHitPlayer = attackingPlayer;
			//receivedHitEnemy = attackingEnemy;
			receivedHitReaction = res;
			receivedHitPosition = pos;
		}
	}
}

CollisionBody * Actor::GetBubbleHitbox(int index)
{
	if (bubbleFramesToLive[index] > 0)
	{
		return &bubbleHitboxes[index];
	}
	return NULL;
}

void Actor::MiniDraw(sf::RenderTarget *target)
{
	keyExplodeRingGroup->Draw(target);
	enemyExplodeRingGroup->Draw(target);
}

void Actor::MapDraw(sf::RenderTarget *target, bool drawPlayer, bool drawNameTag, float scale )
{
	//vv.setSize(mapTex->getSize().x * mapZoomFactor, mapTex->getSize().y * mapZoomFactor);
	sf::View oldView = target->getView();

	//float zoom = target->getView().getSize().x / target->getSize().x;

	Vector2i iconPixel = target->mapCoordsToPixel(Vector2f(position));
	sf::View iconView;
	iconView.setCenter(0, 0);
	iconView.setSize(target->getSize().x, target->getSize().y);
	target->setView(iconView);

	Vector2f trueIconPos = target->mapPixelToCoords(iconPixel);

	if (!drawPlayer)
	{
		mapIconSpr.setPosition(trueIconPos);
		mapIconSpr.setScale(scale, scale);

		target->draw(mapIconSpr, &mapIconShader.pShader);
	}

	if (drawNameTag && nameTag->IsActive())
	{
		//nameTag->SetName("test test test");
		nameTag->MapDraw(target, Vector2f(0, -30), Vector2f(trueIconPos));
	}

	target->setView(oldView);

	if (drawPlayer)
	{
		DrawPlayerSprite(target);
	}
}



void Actor::DrawPlayerSprite( sf::RenderTarget *target )
{
	if (kinMode == K_DESPERATION)
	{
		target->draw(*sprite, &skinShader.pShader);
		//target->draw(*sprite, &playerDespShader);
	}
	else if (kinMode == K_SUPER)
	{
		target->draw(*sprite, &playerSuperShader);
	}
	else
	{
		target->draw(*sprite, &skinShader.pShader);
	}
}

void Actor::DefaultGroundLanding( double &movement )
{
	
	prevRail = NULL;

	//offsetX = (position.x + b.offset.x) - minContact.position.x;


	if (minContact.normal.x > 0)
	{
		offsetX = b.rw;
	}
	else if (minContact.normal.x < 0)
	{
		offsetX = -b.rw;
	}
	else
	{
		offsetX = (position.x + b.offset.x) - minContact.position.x;
	}

	//if (offsetX > b.rw + .00001 || offsetX < -b.rw - .00001) //stops glitchyness with _\ weird offsets
	//{
	//	//removing the print here now, but it could always cause something weird later...
	//	//this triggered last time when climbing a steep slope and jumping onto it while going up
	//	//honestly I feel like this probably doesn't cause any problems.

	//	/*cout << "normal that offset is glitchy on: " << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << ", offset: " << offsetX
	//		<< ", truenormal: " << minContact.normal.x << ", " << minContact.normal.y << endl;
	//	cout << "position.x: " << position.x << ", minx " << minContact.position.x << endl;*/
	//	if (offsetX > 0)
	//	{
	//		offsetX = b.rw;
	//		minContact.position.x = position.x - b.rw;
	//	}
	//	else
	//	{
	//		offsetX = -b.rw;
	//		minContact.position.x = position.x + b.rw;
	//	}
	//}

	if (b.rh < normalHeight)
	{
		if (minContact.normal.y > 0)
			b.offset.y = -(normalHeight - b.rh);
		else if (minContact.normal.y < 0)
			b.offset.y = (normalHeight - b.rh);
	}
	else
	{
		b.offset.y = 0;
	}

	assert(!(minContact.normal.x == 0 && minContact.normal.y == 0));
	ground = minContact.edge;
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt; //turn off grind attempter

	edgeQuantity = minContact.edge->GetQuantity(minContact.position);

	V2d alongVel = V2d(-minContact.normal.y, minContact.normal.x);

	double groundLength = length(ground->v1 - ground->v0);

	V2d gNorm = ground->Normal();

	V2d testVel = velocity;

	if (testVel.y > 20)
	{
		testVel.y *= .7;
	}
	else if (testVel.y < -30)
	{
		testVel.y *= .5;
	}

	gNorm = ground->Normal();

	groundSpeed = CalcLandingSpeed(testVel, alongVel, gNorm);

	RestoreAirOptions();

	if (action != BOOSTERBOUNCE)
	{
		if (velocity.x < 0 && gNorm.y <= -steepThresh)
		{
			groundSpeed = min(velocity.x, dot(velocity, normalize(ground->v1 - ground->v0)) * .7);
		}
		else if (velocity.x > 0 && gNorm.y <= -steepThresh)
		{
			groundSpeed = max(velocity.x, dot(velocity, normalize(ground->v1 - ground->v0)) * .7);
		}
	}
	movement = 0;
}

bool Actor::DefaultGravReverseCheck()
{
	bool steepTransferCheck = ground != NULL && ground->IsSteepGround() && minContact.edge->IsSteepGround();

	return ((IsOptionOn(POWER_GRAV) || touchedGrass[Grass::GRAVREVERSE] || ( minContact.edge->rail != NULL && minContact.edge->rail->GetRailType() == TerrainRail::CEILING ))
		//&& tempCollision
		&& !IsHitstunAction(action)
		&& !touchedGrass[Grass::ANTIGRAVREVERSE]
		&& ((((DashButtonHeld() || steepTransferCheck ) && currInput.LUp()) /*|| touchedGrass[Grass::GRAVREVERSE]*/) || (IsOptionOn(POWER_GRIND) && GrindButtonHeld()))
		&& minContact.normal.y > 0
		&& abs(minContact.normal.x) < wallThresh
		&& minContact.position.y <= position.y - b.rh + b.offset.y + 1
		&& !minContact.edge->IsInvisibleWall());
}

void Actor::DefaultCeilingLanding(double &movement)
{
	prevRail = NULL;

	if (b.rh < normalHeight)
	{
		b.offset.y = -(normalHeight - b.rh);
	}

	if (minContact.edge->Normal().y <= 0)
	{
		if (minContact.position == minContact.edge->v0)
		{
			if (minContact.edge->edge0->Normal().y >= 0)
			{
				minContact.edge = minContact.edge->edge0;
			}
		}
	}

	RestoreAirOptions();
	reversed = true;

	ground = minContact.edge;

	edgeQuantity = minContact.edge->GetQuantity(minContact.position);

	double groundLength = length(ground->v1 - ground->v0);
	groundSpeed = 0;


	if (minContact.normal.x > 0)
	{
		offsetX = b.rw;
	}
	else if (minContact.normal.x < 0)
	{
		offsetX = -b.rw;
	}
	else
	{
		offsetX = (position.x + b.offset.x) - minContact.position.x;
	}
	//offsetX = (position.x + b.offset.x) - minContact.position.x;

	V2d gno = GetGroundedNormal();//ground->Normal();


	double angle = atan2(gno.x, -gno.y);

	V2d along(-gno.y, gno.x);//ground->Along();
	if (-gno.y > -steepThresh) //is steep
	{
		groundSpeed = -dot(velocity, along);
	}
	else
	{
		groundSpeed = -dot(velocity, along);

		if (velocity.x > 0 && groundSpeed < velocity.x)//groundSpeed > 0 && groundSpeed < velocity.x)
		{
			groundSpeed = velocity.x;
		}
		else if (velocity.x < 0 && groundSpeed > velocity.x)//groundSpeed < 0 && groundSpeed > velocity.x)
		{
			groundSpeed = velocity.x;
		}
		
	}

	movement = 0;


	
	//offsetX = -10;//(position.x + b.offset.x) - minContact.position.x;
	
	//offsetX = -offsetX;

	//cout << "offsetX: " << offsetX << "\n";
	//offsetX is always correct if the math is right for the case I'm in.
	

	//cout << "actual landing offsetX: " << offsetX << endl;


	ActivateEffect(PLAYERFX_GRAV_REVERSE, Vector2f(position), RadiansToDegrees(angle), 25, 1, facingRight);
	ActivateSound(PlayerSounds::S_GRAVREVERSE);
}

void Actor::BounceCollision( double &movement )
{
	//if (action == BOUNCEAIR || action == BOUNCEGROUND || bounceFlameOn)

	assert(action == BOUNCEAIR || action == BOUNCEGROUND || bounceFlameOn);

	prevRail = NULL;
	//this condition might only work when not reversed? does it matter?
	if (bounceEdge == NULL)
	{
		bounceEdge = minContact.edge;
		bounceNorm = minContact.normal;
		framesSinceGrindAttempt = maxFramesSinceGrindAttempt; //turn off grind attempter


		V2d oldv0 = bounceEdge->v0;
		V2d oldv1 = bounceEdge->v1;

		edgeQuantity = bounceEdge->GetQuantity(minContact.position);

		if (minContact.normal.x > 0)
		{
			offsetX = b.rw;
		}
		else if (minContact.normal.x < 0)
		{
			offsetX = -b.rw;
		}
		else
		{
			offsetX = (position.x + b.offset.x) - minContact.position.x;
		}

		//offsetX = (position.x + b.offset.x) - minContact.position.x;

		if (b.rh < normalHeight)
		{
			if (minContact.normal.y > 0)
				b.offset.y = -(normalHeight - b.rh);
			else if (minContact.normal.y < 0)
				b.offset.y = (normalHeight - b.rh);
		}
		else
		{
			b.offset.y = 0;
		}

		movement = 0;

		V2d alongVel = V2d(-minContact.normal.y, minContact.normal.x);

		//double groundLength = length(ground->v1 - ground->v0);

		V2d bn = bounceNorm;//bounceEdge->Normal();

		V2d testVel = velocity;

		groundSpeed = CalcLandingSpeed(testVel, alongVel, bn);
	}
	else
	{
		//if( oldBounceEdge != NULL && minContact.edge != oldBounceEdge && action == BOUNCEAIR && framesInAir < 11 )
		//if( bounceEdge != NULL && minContact.edge != bounceEdge )
		{
			if (action == BOUNCEAIR)
			{
				cout << "bounce air" << endl;
			}
			else
			{
				cout << "bounce ground" << endl;
			}
			bounceEdge = NULL;
			oldBounceEdge = NULL;
			SetAction(JUMP);
			holdJump = false;
			frame = 1;
		}
	}
}

bool Actor::IsVisibleAction(int a)
{
	if (a == DEATH || a == EXITWAIT || a == SPAWNWAIT 
		|| a == SEQ_LOOKUPDISAPPEAR || a == SPRINGSTUNTELEPORT
		|| a == HIDDEN )
	{
		return false;
	}

	return true;
}

void Actor::SetAuraColor(Color c)
{
	skinShader.SetAuraColor(c);
}

void Actor::Draw( sf::RenderTarget *target )
{
	if (!IsVisibleAction(action))
	{
		return;
	}
	//dustParticles->Draw(target);
	
	//risingAuraPool->Draw(target);
	
	/*double c = cos( -currInput.leftStickRadians);
	double s = sin( -currInput.leftStickRadians);
	V2d left( c, s );
	CircleShape ccs;
	ccs.setRadius( 10 );
	ccs.setOrigin( ccs.getLocalBounds().width/2, ccs.getLocalBounds().height / 2 );
	ccs.setPosition( position.x + left.x * 100, position.y + left.y * 100 );
	target->draw( ccs );*/
	//target->draw( speedCircle );
	/*if( action == DASH )
	{
		target->draw( *re->particles );
		target->draw( *re1->particles );
	}*/
	//target->draw( *pTrail->particles );
	
	//auras were here before
	
	
	//testAura3->Draw(target);

	//if (action == SEQ_KNEEL || action == SEQ_KNEEL_TO_MEDITATE
	//	|| action == SEQ_MEDITATE_MASKON)

	if (antiTimeSlowFrames > 0)
	{
		target->draw(boosterRingSprite);
	}

	if (action == SPRINGSTUNGLIDE)
	{
		target->draw(glideIndicatorSprite);
	}

	if( bounceFlameOn && !IsExitAction( action ) && !IsGoalKillAction(action) && action != GRINDBALL 
		&& action != RAILGRIND )
	{
		target->draw( scorpSprite );
	}

	if (canRailGrind)
	{
		railTest.setPosition(position.x, position.y);
		//target->draw(railTest);
	}

	if (action == GRINDATTACK)
	{
		target->draw(grindAttackSprite);
	}

	if (action == HOMINGATTACK || action == SPRINGSTUNHOMINGATTACK)
	{
		sf::CircleShape homingTestCircle;
		homingTestCircle.setFillColor(Color( 255, 0, 0, 100 ));
		homingTestCircle.setRadius(homingHitboxes->GetCollisionBoxes(0)[0].rw);
		homingTestCircle.setOrigin(homingTestCircle.getLocalBounds().width / 2,
			homingTestCircle.getLocalBounds().height / 2);
		homingTestCircle.setPosition(Vector2f(position));
		target->draw(homingTestCircle);
	}

	V2d motionGhostDir;
	double motionMagnitude = 0;
	if (ground != NULL)
	{
		motionGhostDir = -normalize(normalize( ground->v1 - ground->v0 ) * groundSpeed );
		if (reversed)
			motionGhostDir = -motionGhostDir;
		motionMagnitude = abs(groundSpeed);
	}
	else
	{
		motionGhostDir = -normalize(velocity);
		motionMagnitude = length(velocity);
	}

	//int testf = 20;

	V2d motionNormal(motionGhostDir.y, -motionGhostDir.x);

	float dist = motionGhostSpacing;

	//RotateCW( diff,);
	int showMotionGhosts = min( motionMagnitude / 1.0, 79.0 );

	//if (showMotionGhosts == 0)
	//	showMotionGhosts = 1;

	Vector2f sprPos(spriteCenter.x, spriteCenter.y );//sprite->getPosition() + diff;
	Vector2f tempPos;
	int testq = 100;
	int ttest;// = 20;

	bool fr = facingRight;
	if (ground != NULL && reversed )
	{
		fr = !fr;
	}

	for (int i = 0; i < showMotionGhosts; ++i)
	{
		if (i < 20)
		{
			ttest = 0;
		}
		else if (i < 40)
		{
			ttest = 10;
		}
		else if (i < 80)
		{
			ttest = 20;
		}
		tempPos = Vector2f(sprPos.x + motionGhostDir.x * (i * dist), sprPos.y + motionGhostDir.y * (i * dist));
		if( ttest != 0 )
			tempPos += Vector2f(motionNormal * (double)(rand() % ttest - ttest / 2) );
		//motionGhosts[i].setPosition( tempPos );

		int tf = 10;
		Vector2f ff(rand() % tf - tf / 2, rand() % tf - tf / 2);
		Vector2f ff1(rand() % tf - tf / 2, rand() % tf - tf / 2);

		motionGhostBuffer->SetPosition(i, tempPos);
		motionGhostBufferBlue->SetPosition(i, tempPos + ff);
		motionGhostBufferPurple->SetPosition(i, tempPos + ff1);

		float x = 1.f;
		if (!fr)
			x = -x;
		float y = 1.f;
		//if (reversed)
		//	y = -y;

		if (i >= 10)
		{
			//float blah = ((rand() % testq) - testq / 2) / ((float)testq / 2);
			float blah = ((rand() % testq)) / ((float)testq);
			blah /= 4.f;//2.f;
			//blah = -blah;
			//motionGhosts[i].setScale( Vector2f(1.f + blah, 1.f + blah));
			motionGhostBuffer->SetScale( i, Vector2f(x + blah, y + blah) );
			motionGhostBufferBlue->SetScale(i, Vector2f(x + blah, y + blah));
			motionGhostBufferPurple->SetScale(i, Vector2f(x + blah, y + blah));
		}
		else
		{
			motionGhostBuffer->SetScale(i, Vector2f( x, y ));
			motionGhostBufferBlue->SetScale(i, Vector2f(x, y));
			motionGhostBufferPurple->SetScale(i, Vector2f(x, y));
		}
	}

	//motionGhostShader.setUniform("textureSize", Vector2f( sprite->getTextureRect().width,
	//	sprite->getTextureRect().height ) );

	motionGhostBuffer->SetNumActiveMembers(showMotionGhosts);
	motionGhostBufferBlue->SetNumActiveMembers(showMotionGhosts);
	motionGhostBufferPurple->SetNumActiveMembers(showMotionGhosts);
	
	//if( showMotionGhosts > 0 )
	{

		CubicBezier cb(.11, 1.01, .4, .96);//(.1, .82, .49, .86);//(.29, .71, .49, .86);//(.11, 1.01, .4, .96);
		float start = 128.f;
		for (int i = 0; i < showMotionGhosts; ++i)
		{
			//motionGhosts[i] = *sprite;
			float factor = (float)i / showMotionGhosts;
			float a = cb.GetValue(factor);
			int alpha = max(start - a * start, 0.f);
			//motionGhosts[i].setColor();
			Color tColor = Color::Cyan;

			float startF = 64;

			tColor.a = max(startF - a * startF, 0.f);

			Color bColor = Color::Blue;
			bColor.a = max(startF - a * startF, 0.f);

			//b500f7
			Color pColor = Color(0xb5, 0x00, 0xf7);//Color(255, 0, 255);
			//6100bd
			pColor.a = max(startF - a * startF, 0.f);
			//motionGhostBuffer->SetColor(i, sf::Color(255, 255, 255, alpha));
			//motionGhostBufferBlue->SetColor(i, sf::Color(255, 255, 255, alpha));
			//motionGhostBufferPurple->SetColor(i, sf::Color(255, 255, 255, alpha));
			motionGhostBuffer->SetColor(i, tColor);
			motionGhostBufferBlue->SetColor(i, bColor);
			motionGhostBufferPurple->SetColor(i, pColor);
		}

		//swordShader.setUniform( "isTealAlready", 0 );
		//
		//for( int i = 0; i < showMotionGhosts; ++i )
	//	{
			//float opac = .5 * ((MAX_MOTION_GHOSTS - i ) / (float)MAX_MOTION_GHOSTS);
			//swordShader.setUniform( "opacity", opac );
			//cout << "setting : " << i << endl;
			//motionGhosts[i].setColor( Color( 50, 50, 255, 50 ) );
			//motionGhosts[i].setColor( Color( 50, 50, 255, 100 * ( 1 - (double)i / showMotionGhosts ) ) );
			//target->draw( fairSword1, &swordShader );
			//target->draw(motionGhosts[i], &motionGhostShader);//&motionGhostShader );// , &swordShader );
		//}
		
		//motionGhostBuffer->Draw(target, &motionGhostShader );
		//motionGhostBuffer->Draw(target, motionGhostShader);
		//target->draw()
		

		

		
		/*{
			motionGhostBuffer->UpdateVertices();
			motionGhostBuffer->Draw(target, &motionGhostShader);
		}

		if (speedLevel > 0)
		{
			motionGhostBufferBlue->UpdateVertices();
			motionGhostBufferBlue->Draw(target, &motionGhostShader);
		}

		if (speedLevel > 1)
		{
			motionGhostBufferPurple->UpdateVertices();
			motionGhostBufferPurple->Draw(target, &motionGhostShader);
		}*/


		motionGhostsEffects[speedLevel]->SetShader(&motionGhostShader);
		motionGhostsEffects[speedLevel]->ApplyUpdates();
		motionGhostsEffects[speedLevel]->Draw(target);

		//for (int i = speedLevel; i >= 0; --i)
		//for (int i = 0; i >= 0; --i)
		/*{
			motionGhostsEffects[i]->SetShader(&motionGhostShader);
			motionGhostsEffects[i]->ApplyUpdates();
			motionGhostsEffects[i]->Draw(target);
		}*/
		
	}
	
	if (showExitAura)
	{
		target->draw(exitAuraSprite, &exitAuraShader.pShader);
		//if we add another "extra aura" should just make a single sprite for them, combining
		//this and dirtyaura
	}

	{

		
	}
	if (action == GRINDBALL || action == GRINDATTACK || action == RAILGRIND)
	{
		target->draw(grindQuads, 4 * NUM_GRIND_QUADS, sf::Quads, ts_grind->texture);

		DrawPlayerSprite(target);
	}
	else
	{
		/*if (owner != NULL)
			flashFrames = owner->pauseFrames;
		else
			flashFrames = 0;*/
		
		DrawPlayerSprite( target );
		
		if (showSword)
		{
			//sf::Shader &swordSh = swordShaders[speedLevel];

			swordShader.SetOffset(swordState);
			target->draw(swordSprite, &swordShader.pShader);

			//if (flashFrames > 2)//0)
			//{
			//	target->draw(swordSprite, &swordShader.pShader);//&swordSh);
			//}
			//else
			//{
			//	target->draw(swordSprite, &swordShader.pShader);
			//	//target->draw(swordSprite);// , &swordShader.pShader);
			//}

		}
	}
	
	for( int i = 0; i < MAX_BUBBLES; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			bubbleSprite.setTextureRect(ts_bubble->GetSubRect(0));//bubbleFramesToLive[i] % 11 ) );
			//this isnt going to be final anyway
			//cout << "drawing " << i << ": " << bubbleRadiusSize[i] << endl;
			double fac = ((double)bubbleRadiusSize[i]) / 128.0;
			//cout << "fac: " << fac << endl;
			bubbleSprite.setScale( fac * 2, fac * 2 );
			//bubbleSprite.setScale( 2, 2 );
			bubbleSprite.setOrigin( bubbleSprite.getLocalBounds().width / 2, bubbleSprite.getLocalBounds().height / 2 );
			bubbleSprite.setPosition( bubblePos[i].x, bubblePos[i].y );
			bubbleSprite.setColor( Color( 255, 255, 255, 100 ) );
			//CircleShape cs;
		//	cs.setFillColor( sf::Color::Transparent );
			//cs.setRadius( 

			target->draw( bubbleSprite );// &timeSlowShader );
		}
	}

	keyExplodeRingGroup->Draw(target);
	enemyExplodeRingGroup->Draw(target);
	enemiesClearedRingGroup->Draw(target);
	enoughKeysToExitRingGroup->Draw(target);
	
	gravityDecreaserOnRingGroup->Draw(target);
	gravityDecreaserOffRingGroup->Draw(target);

	gravityIncreaserOnRingGroup->Draw(target);
	gravityIncreaserOffRingGroup->Draw(target);
	//keyExplodePool->Draw(target);
}

void Actor::DrawShield(sf::RenderTarget *target)
{
	if (IsBlockAction(action))
	{
		if (blockstunFrames > 0)
		{
			target->draw(shieldSprite, &shieldShader);
		}
		else
		{
			target->draw(shieldSprite);
		}
	}
}

void Actor::DrawHomingBall(sf::RenderTarget *target)
{
	if (action == HOMINGATTACK)
	{
		target->draw(homingAttackBallSprite);
	}
}

void Actor::DrawNameTag(sf::RenderTarget *target)
{
	if (nameTag->IsActive() )
	{
		nameTag->Draw(target);
	}
}

void Actor::ResetGrassCounters()
{
	memset(oldTouchedGrass, 0, sizeof(bool) * Grass::Count);
	memset(touchedGrass, 0, sizeof(bool) * Grass::Count);
}

void Actor::DeathDraw(sf::RenderTarget *target)
{
	if (action == DEATH)
	{
		target->draw(*sprite);// , &sh);
	}
}

void Actor::DrawMapWires(sf::RenderTarget *target)
{
	if ((action != Actor::GRINDBALL && action != Actor::GRINDATTACK))
	{
		rightWire->DrawMinimap(target);
		leftWire->DrawMinimap(target);
	}
}

void Actor::GrabShipWire()
{
	SetAction(GRABSHIP);
//	ground = NULL;
	frame = 0;
}

void Actor::ShipPickupPoint( V2d pos, bool fr )
{
	if( action != WAITFORSHIP && action != GRABSHIP )
	{
		sess->totalFramesBeforeGoal = sess->totalGameFrames;
		SetKinMode(K_NORMAL);
		SetExpr(KinMask::Expr_NEUTRAL);
		
		
		//WriteBestTimeRecordings();

		
		frame = 0;
		assert( ground != NULL );

		SetAirPos(pos, fr);
		velocity = V2d(0, 0);

		SetAction(WAITFORSHIP);

		CompleteCurrentMap();

		if (sess->scoreDisplay != NULL)
		{
			sess->ActivateScoreDisplay(60);
		}
	}
}

void Actor::EnterNexus( int nexusIndex, sf::Vector2<double> &pos )
{
	if( nexusIndex == 0 )
	{
		SetAction(ENTERNEXUS1);
		frame = 0;
		position = pos;
		velocity = V2d( 0, 0 );
	}
	
}

void Actor::DodecaLateDraw(sf::RenderTarget *target)
{
	int dodecaFactor = 1;
	for( int i = 0; i < MAX_BUBBLES; ++i )
	{
		if( bubbleFramesToLive[i] > 0 )
		{
			int trueFrame = bubbleLifeSpan - ( bubbleFramesToLive[i] - 1);
			if( trueFrame / dodecaFactor - 9 < 12 )
			{
				if( trueFrame / dodecaFactor < 9 )
				{
					dodecaSprite.setTexture( *ts_dodecaSmall->texture );
					dodecaSprite.setTextureRect( ts_dodecaSmall->GetSubRect( trueFrame / dodecaFactor ) );
					dodecaSprite.setOrigin( dodecaSprite.getLocalBounds().width / 2, dodecaSprite.getLocalBounds().height / 2 );
				}
				else
				{
					dodecaSprite.setTexture( *ts_dodecaBig->texture );
					dodecaSprite.setTextureRect( ts_dodecaBig->GetSubRect( trueFrame / dodecaFactor - 9 ) );
					dodecaSprite.setOrigin( dodecaSprite.getLocalBounds().width / 2, dodecaSprite.getLocalBounds().height / 2 );
				}
				dodecaSprite.setPosition( bubblePos[i].x, bubblePos[i].y );

				target->draw( dodecaSprite );
			}
		}
	}
}

void Actor::DebugDraw( RenderTarget *target )
{
	if( currHitboxes != NULL )
	{
		currHitboxes->DebugDraw( currHitboxFrame, target);
	}

	CircleShape cs;
	cs.setFillColor(Color::Green);
	cs.setRadius(2);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	
	cs.setPosition(Vector2f( b.globalPosition ));
	//hurtBody.DebugDraw(target)

	b.DebugDraw( CollisionBox::Physics, target);

	target->draw(cs);

	leftWire->DebugDraw( target );
	rightWire->DebugDraw( target );

	DebugDrawComboObj(target);

}

void Actor::SetFakeCurrInput( ControllerState &state )
{
	currInput = state;
}

int Actor::GetJumpFrame()
{
	sf::IntRect ir;
	int tFrame = -1;
	

	if (velocity.y < -15)
	{
		tFrame = 1;
	}
	else if (velocity.y < -6)
	{
		tFrame = 2;
	}
	else if (velocity.y < 2)
	{
		tFrame = 3;
	}
	else if (velocity.y < 7)
	{
		tFrame = 4;
	}
	else if (velocity.y < 9)
	{
		tFrame = 5;
	}
	else if (velocity.y < 12)
	{
		tFrame = 6;
	}
	else if (velocity.y < 16)
	{
		tFrame = 7;
	}
	else if (velocity.y < 20)
	{
		tFrame = 8;
	}
	else if (velocity.y < 26)
	{
		tFrame = 9;
	}
	else if (velocity.y < 30)
	{
		tFrame = 10;
	}
	else if (velocity.y < 35)
	{
		tFrame = 11;
	}
	else if (velocity.y < 37)
	{
		tFrame = 12;
	}
	else if (velocity.y < 40)
	{
		tFrame = 13;
	}
	else
	{
		tFrame = 14;
	}

	//if (tFrame = -1)
	//	tFrame = 14;

	//assert(tFrame != -1);
	return tFrame;
}

bool Actor::IsMovingRight()
{
	if (ground == NULL && grindEdge == NULL && bounceEdge == NULL)
	{
		if (velocity.x > 0)
		{
			return true;
		}
	}
	else if (ground != NULL)
	{
		if ((!reversed && groundSpeed > 0) || (reversed && groundSpeed < 0))
		{
			return true;
		}
	}
	
	return false;
	//fill out grind and bounce later
}

bool Actor::IsMovingLeft()
{
	if (ground == NULL && grindEdge == NULL && bounceEdge == NULL)
	{
		if (velocity.x < 0)
		{
			return true;
		}
	}
	else if (ground != NULL)
	{
		if ((!reversed && groundSpeed < 0) || (reversed && groundSpeed > 0))
		{
			return true;
		}
	}

	return false;
	//fill out grind and bounce later
}

void Actor::UpdateSprite()
{
	if (simulationMode)
	{
		return;
	}

	UpdateActionSprite();


	Vector2f oldOrigin = sprite->getOrigin();
	Vector2f center(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	Vector2f diff = center - oldOrigin;
	RotateCW(diff, sprite->getRotation() / 180.f * PI);
	spriteCenter = V2d(sprite->getPosition() + diff);

	if (sess->hud != NULL && sess->hud->hType == HUD::ADVENTURE && !sess->IsParallelSession() )
	{
		AdventureHUD *ah = (AdventureHUD*)sess->hud;
		ah->powerSelector->Update(currPowerMode);
	}

	/*keyExplodePool->Update();*/
	if (!keyExplodeRingGroup->Update())
	{
		//keyExplodeRingGroup->Reset();
	}

	enemyExplodeRingGroup->Update();

	enemiesClearedRingGroup->Update();
	enoughKeysToExitRingGroup->Update();
	gravityDecreaserOnRingGroup->Update();
	gravityDecreaserOffRingGroup->Update();

	gravityIncreaserOnRingGroup->Update();
	gravityIncreaserOffRingGroup->Update();

	UpdateSmallLightning();

	boosterRingSprite.setPosition(Vector2f(spriteCenter));
	boosterRingSprite.setOrigin(boosterRingSprite.getLocalBounds().width / 2,
		boosterRingSprite.getLocalBounds().height / 2);

	glideIndicatorSprite.setPosition(Vector2f(spriteCenter));
	glideIndicatorSprite.setOrigin(glideIndicatorSprite.getLocalBounds().width / 2,
		glideIndicatorSprite.getLocalBounds().height / 2);

	UpdateRisingAura();

	UpdateLockedFX();

	scorpSet = false;
	showExitAura = false;

	if (ground != NULL)
	{
		EffectInstance params;
		params.SetParams(Vector2f(ground->GetPosition(edgeQuantity)),
			Transform(Transform::Identity), 1, 20, 0);

		Vector2f v(0, -.2);
		Transform tr;

		int r = (rand() % 20) - 10;
		tr.rotate(r);
		
		
		params.SetVelocityParams(tr.transformPoint( v ), Vector2f(0, 0), 1 );
		//dustParticles->ActivateEffect(&params);
		float zz = .2;
		v = Vector2f(normalize(ground->v1 - ground->v0)) * zz;//Vector2f(zz, 0);
		
		params.SetVelocityParams(tr.transformPoint(v), Vector2f(0, 0), 1);
		//dustParticles->ActivateEffect(&params);
		v = -Vector2f(normalize(ground->v1 - ground->v0)) * zz;
		params.SetVelocityParams(tr.transformPoint(v), Vector2f(0, 0), 1);
		//dustParticles->ActivateEffect(&params);
		
	}

	//dustParticles->Update(&rpu);

	if( ground == NULL )
	{
		bool r = rightWire->IsPulling();
		bool l = leftWire->IsPulling();


		if( r && l && doubleWireBoost )
		{
			
			//create double wire boost
		}
		else if( r && rightWireBoost )
		{
			double angle = atan2(rightWireBoostDir.y, rightWireBoostDir.x);
			ActivateEffect(PLAYERFX_RIGHT_WIRE_BOOST, Vector2f(position), RadiansToDegrees(angle), 8, 2, true);
			//create right wire boost
		}
		else if( l && leftWireBoost )
		{
			double angle = atan2(leftWireBoostDir.y, leftWireBoostDir.x);
			ActivateEffect(PLAYERFX_LEFT_WIRE_BOOST, Vector2f(position), RadiansToDegrees(angle), 8, 2, true);
			//create left wire boost
		}
	}

	

	

	
	//Vector2f extraParticle0(rand() % t - t / 2, rand() % t - t / 2);
	Vector2f extraParticle0;
	int t = 2;
	Vector2f extraParticle1(rand() % t - t / 2, rand() % t - t / 2);
	t = 10;
	Vector2f extraParticle2(rand() % t - t / 2, rand() % t - t / 2);

	bool fr = facingRight;
	if (reversed)
		fr = !fr;

	Transform tr;
	tr.rotate(sprite->getRotation());
	if (!fr)
	{
		tr.scale(Vector2f(-1, 1));
	}
	
	if( scorpOn && !scorpSet )
	{
		if( ground != NULL )
		{
			
		}
		else
		{
		}
	}
}

void Actor::ConfirmEnemyKill( Enemy *e )
{
	/*if( e->hasMonitor && !e->suppressMonitor )
	{
		ActivateSound( soundBuffers[S_HIT_AND_KILL_KEY] );
	}
	else
	{
		ActivateSound( soundBuffers[S_HIT_AND_KILL] );
	}*/

	enemiesKilledThisFrame++;

	TryThrowEnemySwordProjectileBasic();

	if ( e->CountsForEnemyGate() && sess->hud != NULL && sess->hud->hType == HUD::ADVENTURE && !sess->IsParallelSession())
	{
		AdventureHUD *ah = (AdventureHUD*)sess->hud;
		ah->UpdateEnemyNumbers();
		//ah->keyMarker->Reset();
	}

	//new bounce physics make killing enemies and bouncing off them a little awkward since you might wanna do that normally without bouncing. I'll find a way to include this.
	//if (HasUpgrade(UPGRADE_W3_SCORPION_ENEMY_KILL_BOUNCE))
	/*{
		if (!bouncedFromKill)
		{
			if (bounceFlameOn && (action == FAIR || action == UAIR || action == DAIR
				|| action == DIAGDOWNATTACK || action == DIAGUPATTACK ))
			{
				bouncedFromKill = true;
			}
		}
	}*/

	//for the growing tree
	//wrong
}

void Actor::ConfirmEnemyNoKill( Enemy *e )
{
	//cout << "hit sound" << endl;
	ActivateSound(PlayerSounds::S_HIT );
}

void Actor::ConfirmHit( Enemy *e )
{
	HitParams &hitParams = e->hitParams;
	//owner->cam.SetRumble(3, 3, 5);

	if (!hitParams.canBeHit)
		return;

	//cout << "hit enemy" << endl;

	//slows the player down so its easier to kill enemies while falling
	//might remove and test it
	/*if (ground == NULL && velocity.y > 0 && action == DAIR )
	{
		velocity.y = 4;
	}*/

	bool hasMomentumUpgrade = false;
	bool hasRegenUpgrade = false;

	swordState = SWORDSTATE_HITTING;

	Color c;
	switch(e->world )
	{
	case 1:
		c = COLOR_BLUE;
		hasMomentumUpgrade = IsOptionOn(UPGRADE_W1_INCREASE_ENEMY_MOMENTUM);
		hasRegenUpgrade = IsOptionOn(UPGRADE_W1_INCREASE_ENEMY_REGEN);
		break;
	case 2:
		c = COLOR_GREEN;
		hasMomentumUpgrade = IsOptionOn(UPGRADE_W2_INCREASE_ENEMY_MOMENTUM);
		hasRegenUpgrade = IsOptionOn(UPGRADE_W2_INCREASE_ENEMY_REGEN);
		break;
	case 3:
		c = COLOR_YELLOW;
		hasMomentumUpgrade = IsOptionOn(UPGRADE_W3_INCREASE_ENEMY_MOMENTUM);
		hasRegenUpgrade = IsOptionOn(UPGRADE_W3_INCREASE_ENEMY_REGEN);
		break;
	case 4:
		c = COLOR_ORANGE;
		hasMomentumUpgrade = IsOptionOn(UPGRADE_W4_INCREASE_ENEMY_MOMENTUM);
		hasRegenUpgrade = IsOptionOn(UPGRADE_W4_INCREASE_ENEMY_REGEN);
		break;
	case 5:
		c = COLOR_RED;
		hasMomentumUpgrade = IsOptionOn(UPGRADE_W5_INCREASE_ENEMY_MOMENTUM);
		hasRegenUpgrade = IsOptionOn(UPGRADE_W5_INCREASE_ENEMY_REGEN);
		break;
	case 6:
		c = COLOR_MAGENTA;
		hasMomentumUpgrade = IsOptionOn(UPGRADE_W6_INCREASE_ENEMY_MOMENTUM);
		hasRegenUpgrade = IsOptionOn(UPGRADE_W6_INCREASE_ENEMY_REGEN);
		break;
	case 7:
		c = Color::White;
		break;
	}

	if (e->numHealth == 1)
	{
		c = Color::Red;
		swordState = SWORDSTATE_KILLING;
	}

	float speedBarAddition = hitParams.speedBar;

	
	float momentumUpgradeAmount = .5;//.2;
	if (hasMomentumUpgrade)
	{
		speedBarAddition += speedBarAddition * momentumUpgradeAmount;
	}

	
	

	currentSpeedBar += hitParams.speedBar;
	hitEnemyDuringPhysics = true;
	currAttackHit = true;
	if( bounceFlameOn )
	{
		bounceAttackHit = true;
	}
	else
	{
		bounceAttackHit = false;
	}

	//replacement for pausing in hitlag
	hitlagFrames = currHitboxInfo->hitlagFrames;
	attackingHitlag = true;

	flashColor = c;	
	flashFrames = currHitboxInfo->hitlagFrames;// +1;

	swordShader.SetSkin(0);
	swordShader.SetOffset(swordState);
	/*for( int i = 0; i < 3; ++i )
	{
		swordShaders[i].setUniform( "toColor", ColorGL( flashColor ) );
	}*/

	//owner->powerWheel->Charge( charge );

	float ch = hitParams.charge;

	float regenUpgradeAmount = .5;//.2 * numChargeUpgrades;
	ch += regenUpgradeAmount * ch;

	int charge = ch;

	HealTimer(charge);
	
	if (kinMode == K_DESPERATION)
	{
		SetKinMode(K_NORMAL);
	}
	
	
	RestoreAirOptions();
	/*switch (action)
	{
	case UAIR:
		if( velocity.y <= 0 )
			hasDoubleJump = true;
		break;
	case DAIR:
		dairBoostedDouble = true;
		break;
	}*/

	/*if (ground != NULL && bounceEdge != NULL && grindEdge != NULL)
	{
		if (velocity.y > 14)
		{
			slowFallFromHit = true;
		}
	}

	if( velocity.y > */

	/*double slowDownFall = 14;
	if (velocity.y > slowDownFall)
	{
		velocity.y = slowDownFall;
	}*/
}

bool Actor::IsHitstunAction(int a)
{
	return IsAirHitstunAction(a) || a == GROUNDHITSTUN;
}

bool Actor::IsAirHitstunAction(int a)
{
	return a == AIRHITSTUN || a == WATERGLIDE_HITSTUN;
}

//void Actor::ConfirmWireHit( bool right )
//{
//
//}


void Actor::UnlockGate(Gate *g)
{
	owner->UnlockGate(g);
	
	//if (rightWire != NULL)
	//	rightWire->Retract();

	//if (leftWire != NULL)
	//	leftWire->Retract();
}

void Actor::HandleRayCollision( Edge *edge, double p_edgeQuantity, double rayPortion )
{
	if (rayMode == RAYMODE_WATER)
	{
		RayCastHandler::HandleRayCollision(edge, p_edgeQuantity, rayPortion);
		/*if (rcEdge == NULL)
		{
			rcEdge = edge;
			rcQuantity = rayPortion;
		}
		else
		{
			if (rayPortion < rcQuantity)
			{
				rcEdge = edge;
				rcQuantity = rayPortion;
			}
		}*/
	}
	/*if( rayPortion > 1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - position ) < length( rcEdge->GetPoint( rcQuantity ) - position ) ) )
	{
		rcEdge = edge;
		rcQuantity = edgeQuantity;
	}*/
}

double Actor::GroundedAngle()
{
	V2d gn;
	if( ground != NULL )
	{
		if (!approxEquals(abs(offsetX), b.rw))
		{
			if (reversed)
			{
				return PI;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			gn = ground->Normal();
			return atan2(gn.x, -gn.y);
		}
	}
	else if( grindEdge != NULL )
	{
		gn = grindEdge->Normal();
		if (gn.y > 0)
		{
			gn = -gn;
		}

		return atan2(gn.x, -gn.y);
	}
	else if (bounceEdge != NULL)
	{
		if (!approxEquals(abs(offsetX), b.rw))
		{
			if (reversed)
			{
				return PI;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			gn = bounceEdge->Normal();
			return atan2(gn.x, -gn.y);
		}
	}
	else
	{
		return 0;
	}
}

double Actor::GroundedAngleAttack( sf::Vector2<double> &trueNormal )
{
	assert( ground != NULL );
	V2d gn = ground->Normal();

	double angle = 0;

	bool extraCase;
	//wtf is the extra case??? its bugging
	if( !reversed )
	{
		extraCase = ( offsetX < 0 && approxEquals( edgeQuantity, 0 ) )
		|| ( offsetX > 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) );
	}
	else
	{
		extraCase = ( offsetX > 0 && approxEquals( edgeQuantity, 0 ) )
		|| ( offsetX < 0 && approxEquals( edgeQuantity, length( ground->v1 - ground->v0 ) ) );
	}
	extraCase = false;
	if( !approxEquals( abs(offsetX), b.rw ) || extraCase )
	{
		trueNormal = V2d( 0, -1 );
		if( reversed )
		{
			angle = PI;
			trueNormal = V2d( 0, 1 );
		}
	}
	else
	{
		angle = atan2( gn.x, -gn.y );
		trueNormal = gn;
	}

	return angle;
}

V2d Actor::GetGroundedNormal()
{
	//if (ground == NULL)
	//	return V2d(0, -1);


	assert(ground != NULL);
	V2d gn = ground->Normal();

	if (!approxEquals(abs(offsetX), b.rw))
	{
		gn = V2d(0, -1);
		if (reversed)
		{
			gn = V2d(0, 1);
		}
	}

	return gn;
}

bool Actor::IsOnSteepGround()
{
	if (ground != NULL && GetGroundedNormal().x != 0 && ground->IsSteepGround())
	{
		return true;
	}
	
	return false;
}

bool Actor::HoldingRelativeUp()
{
	return (!reversed && currInput.LUp()) || (reversed && currInput.LDown());
}

bool Actor::HoldingRelativeDown()
{
	return (!reversed && currInput.LDown()) || (reversed && currInput.LUp());
}

void Actor::SetSpriteTexture( int a )
{
	spriteAction = a;
	Tileset *ts = tileset[a];
	sprite->setTexture( *ts->texture );
	//orbSprite.setTexture( )
	motionGhostBuffer->SetTileset(ts);
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetTileset(ts);
	}
	
	motionGhostBufferBlue->SetTileset(ts);
	motionGhostBufferPurple->SetTileset(ts);

	//const auto &size = ts->texture->getSize();

}

void Actor::SetAerialScorpSprite()
{
	assert(scorpOn);
	int tFrame = GetJumpFrame();
	scorpSprite.setTexture(*ts_scorpJump->texture);

	SetSpriteTile(&scorpSprite, ts_scorpJump, tFrame, facingRight);

	scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
		scorpSprite.getLocalBounds().height / 2);
	scorpSprite.setPosition(position.x, position.y);
	scorpSprite.setRotation(sprite->getRotation());
	scorpSet = true;
}

void Actor::SetSpriteTile(int tileIndex, bool noFlipX, bool noFlipY)
{
	currTileIndex = tileIndex;

	Tileset *ts = tileset[spriteAction];
	IntRect ir = ts->GetSubRect(currTileIndex);

	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	if (kinMode == K_NORMAL || kinMode == K_DESPERATION)
	{		
		skinShader.SetQuad(ts, currTileIndex);
			
		//static float testCounter = 0;
		//skinShader.pShader.setUniform("u_slide", testCounter);
		//testCounter += .01f;
	}
	else if( kinMode == K_SUPER )
	{
		playerSuperShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
			(ir.left + ir.width) / width, (ir.top + ir.height) / height));
	}

	
	
	/*if (testCounter > 1.f)
	{
		testCounter -= 1.f;
	}*/


	if( !noFlipX )
	{
		flipTileX = true;
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	else
	{
		flipTileX = false;
	}

	if( !noFlipY )
	{
		flipTileY = true;
		ir.top += ir.height;
		ir.height = -ir.height;
	}
	else
	{
		flipTileY = false;
	}

	


	//CubicBezier cb(.11, 1.01, .4, .96);
	CubicBezier cb;
	Color cols[3] = { Color::Cyan, Color( 91, 136, 253 ), Color::Magenta};//Color(100, 0, 255) };
	for (int i = 0; i < 3; ++i)
	{
		motionGhostsEffects[i]->SetTileset(tileset[spriteAction]);
		motionGhostsEffects[i]->SetTile(currTileIndex);
		motionGhostsEffects[i]->SetFacing(facingRight, reversed);
		//motionGhostsEffects[i]->SetDistInBetween(1.f);
		//motionGhostsEffects[i]->SetScaleParams(CubicBezier(), .25 + i * .15, 0, 10);
		//motionGhostsEffects[i]->SetVibrateParams(CubicBezier(), i * 10);

		Color t(cols[i]);
		Color b(cols[i]);//(Color::Blue);


		if (kinMode == K_DESPERATION)
		{
			t = currentDespColor;
		}
		else
		{
			t = cols[i];//GetBlendColor(Color::White, cols[i], .8f);//i / 3.f);
		}
		
		b = t;

		
		t.a = 200;
		
		b.a = 80;

		motionGhostsEffects[i]->SetColorGradient(t, b, cb);
	}
	
	//t.r = rand() % 255;
	//t.g = rand() % 255;
	//t.b = rand() % 255;

	//b.r = rand() % 255;
	//b.g = rand() % 255;
	//b.b = rand() % 255;

	//t.a = rand() % 255;//80;
	//b.a = rand() % 255;//20;
	
	
	
	//	CubicBezier());
	

	for (int i = 0; i < maxMotionGhosts; ++i)
	{
		motionGhostBuffer->SetTile(i, currTileIndex);
		motionGhostBufferBlue->SetTile(i, currTileIndex);
		motionGhostBufferPurple->SetTile(i, currTileIndex);
	}
	
	sprite->setTextureRect( ir );
}

void Actor::SetSpriteTile( sf::Sprite *spr, 
		Tileset *tss, int tileIndex, bool noFlipX, bool noFlipY )
{
	//if (spr == sprite)
	//{
	//	//inefficient
	//	for (int i = 0; i < maxMotionGhosts; ++i)
	//	{
	//		motionGhostBuffer->SetTile( i, tileIndex);
	//	}
	//	
	//}
	IntRect ir = tss->GetSubRect( tileIndex );
	if( !noFlipX )
	{
		flipTileX = true;
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	else
	{
		flipTileX = false;
	}

	if( !noFlipY )
	{
		flipTileY = true;
		ir.top += ir.height;
		ir.height = -ir.height;
	}
	else
	{
		flipTileY = false;
	}


	spr->setTextureRect( ir );

	int t = 4;
	//Vector2f extraParticle0(rand() % t - t / 2, rand() % t - t / 2);
	Vector2f extraParticle0(0, 0);
	Vector2f extraParticle1(rand() % t - t / 2, rand() % t - t / 2);
	Vector2f extraParticle2(rand() % t - t / 2, rand() % t - t / 2);

	bool fr = facingRight;
	if (reversed)
		fr = !fr;

	Transform tr;
	tr.rotate(sprite->getRotation());
	if (!fr)
	{
		tr.scale(Vector2f(-1, 1));
	}
}

void Actor::SetupAction(int a)
{
}

void Actor::FreeFlightMovement()
{
	double accelFactor = 1.0;
	double accelFactorFast = .3;//.1;//.3;
	double maxAccelSpeed = 40;// 15;
	double decelFactor = 2.0;//1.0;//1.5;
	double highSpeedThresh = 15;
	 
	double neutralDecelFactor = .02;//2.0;

	if (currInput.LUp())
	{
		if (velocity.y > 0)
		{
			velocity.y += -decelFactor;
		}
		else if (velocity.y > -highSpeedThresh)
		{
			velocity.y += -accelFactor;
		}
		else if( velocity.y > -maxAccelSpeed )
		{
			velocity.y += -accelFactorFast;

			if (velocity.y < -maxAccelSpeed)
				velocity.y = -maxAccelSpeed;
		}
	}
	if (currInput.LDown())
	{
		if (velocity.y < 0)
		{
			velocity.y += decelFactor;
		}
		else if (velocity.y < highSpeedThresh)
		{
			velocity.y += accelFactor;
		}
		else if (velocity.y < maxAccelSpeed)
		{
			velocity.y += accelFactorFast;

			if (velocity.y > maxAccelSpeed)
				velocity.y = maxAccelSpeed;
		}
	}
	if (currInput.LLeft())
	{
		if (velocity.x > 0)
		{
			velocity.x += -decelFactor;
		}
		else if (velocity.x > -highSpeedThresh)
		{
			velocity.x += -accelFactor;
		}
		else if (velocity.x > -maxAccelSpeed)
		{
			velocity.x += -accelFactorFast;

			if (velocity.x < -maxAccelSpeed)
				velocity.x = -maxAccelSpeed;
		}

	}
	if (currInput.LRight())
	{
		if (velocity.x < 0)
		{
			velocity.x += decelFactor;
		}
		else if (velocity.x < highSpeedThresh)
		{
			velocity.x += accelFactor;
		}
		else if (velocity.x < maxAccelSpeed)
		{
			velocity.x += accelFactorFast;

			if (velocity.x > maxAccelSpeed)
				velocity.x = maxAccelSpeed;
		}
	}

	if (!currInput.LLeft() && !currInput.LRight())
	{
		if (velocity.x > 0)
		{
			velocity.x -= neutralDecelFactor;
			if (velocity.x < 0)
			{
				velocity.x = 0;
			}
		}
		else if (velocity.x < 0)
		{
			velocity.x += neutralDecelFactor;
			if (velocity.x > 0)
			{
				velocity.x = 0;
			}
		}
	}

	if (!currInput.LUp() && !currInput.LDown())
	{
		if (velocity.y > 0)
		{
			velocity.y -= neutralDecelFactor;
			if (velocity.y < 0)
			{
				velocity.y = 0;
			}
		}
		else if (velocity.y < 0)
		{
			velocity.y += neutralDecelFactor;
			if (velocity.y > 0)
			{
				velocity.y = 0;
			}
		}
	}
}

bool Actor::TryHomingMovement()
{
	Enemy *foundEnemy = NULL;
	int foundIndex;

	if (GetClosestEnemyPos(position, 2000, foundEnemy, foundIndex))
	{
		V2d eDir = normalize(foundEnemy->GetPosition() - position);

		velocity += eDir * 5.0;;
		double limit = 28;
		if (length(velocity) > limit)
		{
			velocity = normalize(velocity) * limit;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void Actor::AirMovement()
{
	if( leftWire->IsPulling() || rightWire->IsPulling())
	{
	}
	else if (freeFlightFrames > 0)
	{
		FreeFlightMovement();
	}
	else
	{
		double dSpeed = GetOriginalDashSpeed();//GetDashSpeed();
		if( currInput.LLeft() )
		{
			if( velocity.x > dSpeed )
			{
				velocity.x -= airAccel;
			}
			//else if( speedyJump && velocity.x > -dashSpeed )
			//{
			//	velocity.x = -dashSpeed;
			//}
			else if( velocity.x > -maxAirXControl )
			{
				velocity.x = -maxAirXControl;
			}
		}
		else if( currInput.LRight() )
		{
			if( velocity.x < -dSpeed )
			{
				velocity.x += airAccel;
			}
			//else if( speedyJump && velocity.x < dashSpeed )
			//{
			//	velocity.x = dashSpeed;
			//}
			else if( velocity.x < maxAirXControl )
			{
				velocity.x = maxAirXControl;
			}
		}
		else if( !currInput.LUp() && !currInput.LDown() )
		{
			if( velocity.x > dSpeed )
			{
				velocity.x -= airSlow;
				if( velocity.x < dSpeed ) 
				{
					velocity.x = dSpeed;
				}
			}
			else if( velocity.x > 0 )
			{
				velocity.x += -airSlow;
				if( velocity.x < 0 ) velocity.x = 0;
				else if( velocity.x >= -dSpeed )
				{
					velocity.x = 0;
				}
				//velocity.x = 0;
			}
			else if( velocity.x < -dSpeed )
			{
				velocity.x += airSlow;
				if( velocity.x > -dSpeed ) 
				{
					velocity.x = -dSpeed;
				}
			}
			else if( velocity.x < 0 )
			{
				velocity.x += airSlow;
				if( velocity.x > 0 ) velocity.x = 0;
				else if( velocity.x >= -dSpeed )
				{
					velocity.x = 0;
				}
			}
		}
	}
}

void Actor::DrawWires(sf::RenderTarget *target)
{
	if (IsOptionOn(POWER_LWIRE) &&
		((action != Actor::GRINDBALL && action != Actor::GRINDATTACK)
			|| leftWire->IsRetracting()))
	{
		leftWire->Draw(target);
	}
	if (IsOptionOn(POWER_RWIRE) &&
		((action != Actor::GRINDBALL && action != Actor::GRINDATTACK)
			|| rightWire->IsRetracting()))
	{
		rightWire->Draw(target);
	}
}

void Actor::UpdateWireQuads()
{
	if (IsOptionOn(POWER_LWIRE))
		leftWire->UpdateQuads();

	if (IsOptionOn(POWER_RWIRE))
		rightWire->UpdateQuads();
}

Vector2i Actor::GetWireOffset()
{
	Vector2i offset;
	switch( action )
	{
	case DASH:
	case DOUBLE:
		offset = Vector2i( 0, 0 );
		break;
	case STEEPSLIDE:
		//cout << "steep slide" << endl;
		offset = Vector2i( 0, 0 );
		break;
	case SPRINT:
		offset = Vector2i( 0, 0 );
		break;
	case LAND:
	case LAND2:
		offset = Vector2i( 0, 0 );
		break;
	default:
		offset = Vector2i( 0, 4.9 );
	}

	offset = Vector2i( 0, 4.9 );

	if( reversed )
	{
		//offset.y = -offset.y;
	}

	//if( b.rh < normalHeight )
	/*{
		if( ground != NULL )
		{
			offset = Vector2i( 0, 5 );
		}
		else
		{
			offset = Vector2i( 0, 0 );
		}
		
	}*/

	return offset;
}

void Actor::SetAutoRun(bool fr, double maxAutoRun)
{
	SetAction(AUTORUN);
	frame = 0;
	autoRunStopEdge = NULL;

	maxAutoRunSpeed = maxAutoRun;
	facingRight = fr;
}

void Actor::BrakeMovement()
{
	double brakeAmount = 1.0;
	double dSpeed = dashSpeed;
	if (groundSpeed > 0)
	{
		groundSpeed -= brakeAmount;
	}
	else if (groundSpeed < 0)
	{
		groundSpeed += brakeAmount;
	}
}

void Actor::RunMovement()
{
	if (action != AUTORUN)
	{
		if (currInput.LLeft())
			facingRight = false;
		else if (currInput.LRight())
			facingRight = true;
	}

	if( !facingRight )
	{
		if( groundSpeed > 0 )
		{
			groundSpeed = 0;
		}
		else
		{
			if( groundSpeed > -maxRunInit )
			{
				groundSpeed -= runAccelInit / slowMultiple;
				if( groundSpeed < -maxRunInit )
					groundSpeed = -maxRunInit;
			}
			else
			{
				groundSpeed -= runAccel / slowMultiple;
			}
				
		}
		
		GroundExtraAccel();

		if (action == AUTORUN)
		{
			if (-groundSpeed > maxAutoRunSpeed)
			{
				groundSpeed = -maxAutoRunSpeed;
			}
		}
	}
	else
	{
		if (groundSpeed < 0 )
			groundSpeed = 0;
		else
		{
			if( groundSpeed < maxRunInit )
			{
				groundSpeed += runAccelInit / slowMultiple;
				if( groundSpeed > maxRunInit )
					groundSpeed = maxRunInit;
			}
			else
			{
				groundSpeed += runAccel / slowMultiple;
			}
		}

		GroundExtraAccel();

		if (action == AUTORUN)
		{
			if (groundSpeed > maxAutoRunSpeed)
			{
				groundSpeed = maxAutoRunSpeed;
			}
		}
	}
}

void Actor::AttackMovement()
{
	assert(ground != NULL);
	if (IsOnSteepGround())
	{
		V2d norm = GetGroundedNormal();

		if ((groundSpeed > 0 && norm.x < 0) || ( groundSpeed < 0 && norm.x > 0 ))
		{
			SteepClimbMovement();
		}
		else
		{
			SteepSlideMovement();
		}
	}
	else
	{
		double dSpeed = GetDashSpeed();
		if (currInput.LLeft())
		{
			if (groundSpeed > 0)
			{
				if (DashButtonHeld())
				{
					groundSpeed = -dSpeed;
				}
				else
				{
					groundSpeed = 0;
				}
			}
			else
			{
				if (groundSpeed > -dSpeed && DashButtonHeld())
				{
					groundSpeed = -dSpeed;
				}
				else if (groundSpeed > -maxRunInit)
				{
					groundSpeed -= runAccelInit / slowMultiple;
					if (groundSpeed < -maxRunInit)
						groundSpeed = -maxRunInit;
				}
				else
				{
					groundSpeed -= runAccel / slowMultiple;
				}

			}
		}
		else if (currInput.LRight())
		{
			if (groundSpeed < 0)
			{
				if (DashButtonHeld())
				{
					groundSpeed = dSpeed;
				}
				else
				{
					groundSpeed = 0;
				}
			}
			else
			{
				if (groundSpeed < dSpeed && DashButtonHeld())
				{
					groundSpeed = dSpeed;
				}
				else if (groundSpeed < maxRunInit)
				{
					groundSpeed += runAccelInit / slowMultiple;
					if (groundSpeed > maxRunInit)
						groundSpeed = maxRunInit;
				}
				else
				{
					groundSpeed += runAccel / slowMultiple;
				}
			}
		}
		else if (currInput.LDown() || currInput.LUp())
		{
			//groundspeed stays the same

		}
		else
		{
			groundSpeed = 0;
		}
	}
	

	if (shieldPushbackFrames > 0)
	{
		/*if (shieldPushbackRight)
		{
			if (groundSpeed < 0)
			{
				groundSpeed *= .2;
			}
		}
		else
		{
			if (groundSpeed > 0)
			{
				groundSpeed *= .2;
			}
		}*/


		shieldPushbackFrames--;
	}

	SprintAccel();

	GroundExtraAccel();
}

bool Actor::CanBufferGrind()
{
	return !touchedGrass[Grass::ANTIGRIND]
		&& !InWater(TerrainPolygon::WATER_INVERTEDINPUTS)
		&& currPowerMode == PMODE_GRIND 
		&& IsOptionOn(POWER_GRIND) && currInput.PowerButtonDown();//currInput.RDown();//currInput.Y;
}

bool Actor::CanPressGrind()
{
	return CanBufferGrind() && !prevInput.PowerButtonDown();//!prevInput.RDown();//!prevInput.PowerButtonDown();
}

bool Actor::TryBufferGrind()
{
	if (CanBufferGrind())
	{
		SetActionGrind();
		return true;
	}

	return false;
}

bool Actor::TryPressGrind()
{
	if (CanPressGrind())
	{
		SetActionGrind();
		return true;
	}

	return false;
}

bool Actor::GrindButtonPressed()
{
	return currPowerMode == PMODE_GRIND && PowerButtonPressed();//(GrindButtonHeld() && !prevInput.RDown());
}

bool Actor::GrindButtonHeld()
{
	return currPowerMode == PMODE_GRIND && PowerButtonHeld();//currInput.RDown();
}

bool Actor::BounceButtonPressed()
{
	return currPowerMode == PMODE_BOUNCE && PowerButtonPressed();//BounceButtonHeld() && !prevInput.RLeft();
}

bool Actor::BounceButtonHeld()
{
	return currPowerMode == PMODE_BOUNCE && PowerButtonHeld();//currInput.RLeft();
}

bool Actor::JumpButtonPressed()
{
	return currInput.JumpButtonDown() && !prevInput.JumpButtonDown();
}

bool Actor::DashButtonPressed()
{
	return currInput.DashButtonDown() && !prevInput.DashButtonDown();
}

bool Actor::JumpButtonHeld()
{
	return currInput.JumpButtonDown();
}

bool Actor::DashButtonHeld()
{
	return currInput.DashButtonDown();
}

bool Actor::AttackButtonPressed()
{
	return currInput.AttackButtonDown() && !prevInput.AttackButtonDown();
}

bool Actor::AttackButtonHeld()
{
	return currInput.AttackButtonDown();
}

bool Actor::PowerButtonHeld()
{
	return currInput.PowerButtonDown();
}

bool Actor::PowerButtonPressed()
{
	return currInput.PowerButtonDown() && !prevInput.PowerButtonDown();
}

bool Actor::SpecialButtonPressed()
{
	return currInput.BackButtonDown() && !prevInput.BackButtonDown();
}

void Actor::BounceFloaterBoost( V2d &hitDir )
{
	Actor *player = sess->GetPlayer(0);

	double minX = 20;

	holdJump = false;
	holdDouble = false;

	if (player->ground != NULL)
	{
		if (facingRight)
		{
			player->groundSpeed = -minX;
		}
		else
		{
			player->groundSpeed = minX;
		}

		/*if (player->groundSpeed > 0)
		{
			player->groundSpeed = min(-player->groundSpeed, );
		}
		else if (player->groundSpeed < 0)
		{
			player->groundSpeed = max(-player->groundSpeed, minX);
		}*/

		/*if (player->groundSpeed > 0)
		{
			player->groundSpeed = min(-player->groundSpeed, );
		}
		else if( player->groundSpeed < 0 )
		{
			player->groundSpeed = max(-player->groundSpeed, minX);
		}*/
	}
	else
	{
		
		bool fr = player->facingRight;
		if (hitDir.x != 0)//&& dir.y == 0)
		{
			if (hitDir.x > 0 )
			{
				player->velocity.x = -minX;
			}
			else
			{
				player->velocity.x = minX;
			}


			/*double velx = player->velocity.x;
			if (fr && velx < minX)
			{
				velx = minX;
			}
			else if (!fr && velx > -minX)
			{
				velx = -minX;
			}

			player->velocity.x = -velx;*/
		}
		if (hitDir.y != 0)//&& dir.x == 0)
		{
			double minUp = -20;
			double minDown = 40;

			if (hitDir.y > 0)
			{
				player->velocity.y = -minDown;
			}
			else
			{
				player->velocity.y = -minUp;
			}
			/*double vely = player->velocity.y;
			if (hitDir.y > 0 && vely < minDown)
			{
				vely = -minDown;
			}
			else if (hitDir.y < 0 && vely > minUp)
			{
				vely = -minUp;
			}

			player->velocity.y = vely;*/


		}
	}
}


void Actor::SetActionGrind()
{

	BounceFlameOff();

	double minGrindSpeed = GetDashSpeed();

	if (action == SPRINGSTUNGRINDFLY)
	{
		minGrindSpeed = length(springVel);
	}
	//doesnt account for reversed?
	if( groundSpeed == 0 )
	{
		if( facingRight )
		{
			grindSpeed = minGrindSpeed;
		}
		else
		{
			grindSpeed = -minGrindSpeed;
		}
	}
	else if( groundSpeed > 0 )
	{
		grindSpeed = std::min(GetMaxSpeed(), std::max( groundSpeed, minGrindSpeed) );
	}
	else
	{
		grindSpeed = std::max( -GetMaxSpeed(), std::min( groundSpeed, -minGrindSpeed) );
	}
	


	framesGrinding = 0;
	if( rightWire != NULL )
		rightWire->Retract();

	if( leftWire != NULL )
		leftWire->Retract();
	//rightWire->Reset();
	//leftWire->Reset();

	if (action == SPRINGSTUNGRINDFLY)
	{
		SetAction(SPRINGSTUNGRIND);
	}
	else
	{
		SetAction(GRINDBALL);
	}
	
	grindEdge = ground;

	CheckGrindEdgeForTerrainFade();
	frame = 0;


	if (grindHitboxes[0] != NULL)
	{
		double grindHitRadius[] = { 90, 100, 110 };
		CollisionBox &gh = grindHitboxes[0]->GetCollisionBoxes(0).front();
		gh.rw = gh.rh = grindHitRadius[speedLevel];
	}
	
	hurtBody.isCircle = true;
	double grindHurtRadius = 40;
	hurtBody.rw = grindHurtRadius;
	hurtBody.rh = grindHurtRadius;

		
	ground = NULL;
	bounceEdge = NULL;

	if( reversed )
	{
	//	grindSpeed = -grindSpeed;
	}
}

bool Actor::CanUnlockGate( Gate *g )
{
	return g->CanUnlock();
}

void Actor::SetExpr( int ex )
{
	if (kinMask != NULL && !simulationMode)
	{
		kinMask->SetExpr((KinMask::Expr)ex);
	}
}

void Actor::ExecuteDoubleJump()
{
	//add direction later
	ActivateEffect(PLAYERFX_DOUBLE, Vector2f(position.x, position.y - 20), 0, 14, 2, facingRight);

	//velocity = groundSpeed * normalize(ground->v1 - ground->v0 );
	if (velocity.y > 0)
		velocity.y = 0;

	

	double currStrength;
	if (action == BACKWARDSDOUBLE)
	{
		currStrength = backDoubleJumpStrength;
	}
	else
	{
		currStrength = doubleJumpStrength;
	}

	if (touchedGrass[Grass::JUMP])
	{
		currStrength += jumpGrassExtra;
	}

	double scorpionExtra = 10;
	if (bounceFlameOn && IsOptionOn( UPGRADE_W4_SCORPION_DOUBLE_JUMP))
	{
		currStrength += scorpionExtra;
	}
	
	if (freeFlightFrames > 0)
	{
		velocity.y += -currStrength;
	}
	else
	{
		velocity.y = -currStrength;
	}
	

	extraDoubleJump = false;
	hasDoubleJump = false;

	if (currInput.LLeft())
	{
		if (velocity.x > -maxRunInit)
		{
			velocity.x = -maxRunInit;
		}
	}
	else if (currInput.LRight())
	{
		if (velocity.x < maxRunInit)
		{
			velocity.x = maxRunInit;
		}
	}
	else
	{
		velocity.x = 0;
	}

	if (aerialHitCancelDouble)
	{
		if (cancelAttack == FAIR || cancelAttack == AIRDASHFORWARDATTACK)
		{
			if (facingRight && currInput.LLeft()
				|| (!facingRight && currInput.LRight()))
			{
				if (facingRight)
				{
					velocity.x = min( velocity.x, -GetDashSpeed() );
				}
				else
				{
					velocity.x = max( velocity.x, GetDashSpeed() );
				}
				facingRight = !facingRight;
			}
		}
		else if (cancelAttack == DAIR || cancelAttack == DIAGDOWNATTACK)
		{
			velocity.y -= dairBoostVel;
			if (cancelAttack == DIAGDOWNATTACK)
			{
				if (facingRight && currInput.LLeft()
					|| (!facingRight && currInput.LRight()))
				{
					if (facingRight)
					{
						velocity.x = -GetDashSpeed();
					}
					else
					{
						velocity.x = GetDashSpeed();
					}
				}
			}
		}
	}

	if( action == DOUBLE)
	{
		ActivateSound(PlayerSounds::S_DOUBLE);
	}
	else if( action == BACKWARDSDOUBLE)
	{
		ActivateSound(PlayerSounds::S_DOUBLEBACK);
	}
}

void Actor::ExecuteWallJump()
{
	wallJumpFrameCounter = 0;

	double strengthX = dashSpeed0 + 1.0;
	if (currInput.DashButtonDown())
	{
		strengthX = GetDashSpeed() + 1.0;
	}

	
	double strengthY = wallJumpStrength.y;

	if (touchedGrass[Grass::JUMP])
	{
		strengthY += strengthX;//10;
	}

	
	if (touchedGrass[Grass::BOOST])
	{
		strengthX = max(strengthX, 40.0);
	}

	if (facingRight)
	{
		velocity.x = strengthX;
	}
	else
	{
		velocity.x = -strengthX;
	}

	double movingVertStrength = strengthY * .5; //for when you're moving up already
	velocity.y = min(velocity.y - movingVertStrength, -strengthY);


	ActivateSound(PlayerSounds::S_WALLJUMP);

	V2d fxPos = position;
	if (facingRight)
	{
		fxPos += V2d(0, 0);
	}
	else
	{
		fxPos += V2d(0, 0);
	}

	ActivateEffect(PLAYERFX_WALLJUMP_0 + speedLevel, Vector2f(fxPos), 0, 7, 3, facingRight);
}

int Actor::GetDoubleJump()
{
	if( (facingRight && currInput.LLeft()) || ( !facingRight && currInput.LRight() ) )
	{
		if (action == FAIR || action == AIRDASHFORWARDATTACK )
		{
			return DOUBLE;
		}
		else
		{
			return BACKWARDSDOUBLE;
		}
	}
	else
	{
		return DOUBLE;
	}
}

bool Actor::CanDoubleJump()
{
	return ( (hasDoubleJump || extraDoubleJump || (IsOptionOn( UPGRADE_W6_BUBBLE_AIRDASH) && inBubble ) ) &&
		(JumpButtonPressed() || pauseBufferedJump ) && !IsSingleWirePulling() );
}

bool Actor::IsDoubleWirePulling()
{
	return ( rightWire->IsPulling() && leftWire->IsPulling() );
}

bool Actor::TryDoubleJump()
{
	if( CanDoubleJump() )
	{
		aerialHitCancelDouble = IsAttackAction(action);
		if (aerialHitCancelDouble)
		{
			cancelAttack = action;
		}
		dairBoostedDouble = (action == DAIR || action == UAIR || action == DIAGDOWNATTACK || action == DIAGUPATTACK );
		SetAction( GetDoubleJump() );

		if (AttackButtonPressed() && !pauseBufferedJump)
		{
			if (currInput.LUp())
			{
				doubleJumpBufferedAttack = UAIR; //none
			}
			else if (currInput.LDown())
			{
				doubleJumpBufferedAttack = DAIR;
			}
			else
			{
				doubleJumpBufferedAttack = FAIR;
			}
		}
		else
		{
			doubleJumpBufferedAttack = DOUBLE;
		}

		return true;
	}
	
	return false;
}

bool Actor::TryAirDash()
{
	if (IsOptionOn(POWER_AIRDASH) && !IsSingleWirePulling())
	{
		//removed infinite airdash within bubbles, going to replace it as a shard
		if (hasAirDash && (DashButtonPressed() || pauseBufferedDash))
		//if ((hasAirDash || inBubble) && (DashButtonPressed() || pauseBufferedDash))
		{
			hasFairAirDashBoost = (action == FAIR || action == AIRDASHFORWARDATTACK );
			SetAction( AIRDASH );
			return true;
		}
	}

	return false;
}

bool Actor::TryGlide()
{
	bool ad = (IsOptionOn(POWER_AIRDASH) && !hasAirDash)
		|| !IsOptionOn(POWER_AIRDASH);
	if (DashButtonPressed())
	{
		SetAction(GLIDE);
		frame = 0;
		return true;
	}
	return false;
}
//you can pull with both or neither to return false. pulling with a single wire will return true;
bool Actor::IsSingleWirePulling()
{
	bool pulling = (rightWire->IsPulling() || leftWire->IsPulling());
	bool hitAndTryingToPull = (rightWire->IsHit() && currInput.RightTriggerPressed())
		|| (leftWire->IsHit() && currInput.LeftTriggerPressed());
	return ( pulling || hitAndTryingToPull ) 
		&& !IsDoubleWirePulling();
}

bool Actor::CanBlockEnemy(HitboxInfo::HitPosType hpt, V2d &hitPos )
{
	bool facingHitbox = (facingRight && position.x - hitPos.x <= 0)
		|| (!facingRight && position.x - hitPos.x >= 0);

	if (HitboxInfo::IsAirType(hpt) || hpt == HitboxInfo::OMNI)
	{
		V2d diff = position - hitPos;
		if (reversed)
		{
			diff.y = -diff.y;
		}

		hpt = HitboxInfo::GetAirType(normalize(diff));
	}


	//basically changed the block to include 180+ degrees instead of <180
	if (action == GROUNDBLOCKDOWN )
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDLOW)
			return true;
	}
	else if (action == GROUNDBLOCKDOWNFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::GROUND
				|| hpt == HitboxInfo::HitPosType::GROUNDLOW)))
			return true;
	}
	else if (action == GROUNDBLOCKFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRDOWN
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::GROUND))
			return true;
	}
	else if (action == GROUNDBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
				|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)))
			return true;
	}
	else if (action == GROUNDBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)
			return true;
	}
	else if (action == AIRBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD )
			return true;
	}
	else if (action == AIRBLOCKDOWNFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| (facingHitbox && (hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| (facingHitbox &&
				  (hpt == HitboxInfo::HitPosType::AIRUPFORWARD
					|| hpt == HitboxInfo::HitPosType::AIRFORWARD
					|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD)
			return true;
	}

	/*if (action == GROUNDBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDLOW)
			return true;
	}
	else if (action == GROUNDBLOCKDOWNFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::GROUND
				|| hpt == HitboxInfo::HitPosType::GROUNDLOW)))
			return true;
	}
	else if (action == GROUNDBLOCKFORWARD)
	{
		if ((facingHitbox && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::GROUND))
			return true;
	}
	else if (action == GROUNDBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)))
			return true;
	}
	else if (action == GROUNDBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)
			return true;
	}
	else if (action == AIRBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			return true;
	}
	else if (action == AIRBLOCKDOWNFORWARD)
	{
		if ( hpt == HitboxInfo::HitPosType::AIRUP
			|| (facingHitbox && (hpt == HitboxInfo::HitPosType::AIRUPFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKFORWARD)
	{
		if (facingHitbox && 
			( hpt == HitboxInfo::HitPosType::AIRUPFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD))
			return true;
	}
	else if (action == AIRBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| ( facingHitbox && 
				( hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
					|| hpt == HitboxInfo::HitPosType::AIRFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD )
			return true;
	}*/

	return false;
}

bool Actor::CanBlockBullet(HitboxInfo::HitPosType hpt, V2d &hitPos, bool attackFacingRight )
{
	//lenient. if you are facing its position OR if its momentum is opposite to your shield you 
	//will be able to block it.
	bool facingHitbox = ((facingRight && position.x - hitPos.x <= 0)
		|| (!facingRight && position.x - hitPos.x >= 0)) || facingRight != attackFacingRight;

	//basically changed the block to include 180+ degrees instead of <180
	if (action == GROUNDBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDLOW)
			return true;
	}
	else if (action == GROUNDBLOCKDOWNFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::GROUND
				|| hpt == HitboxInfo::HitPosType::GROUNDLOW)))
			return true;
	}
	else if (action == GROUNDBLOCKFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRDOWN
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			|| (facingHitbox && hpt == HitboxInfo::HitPosType::GROUND))
			return true;
	}
	else if (action == GROUNDBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
				|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)))
			return true;
	}
	else if (action == GROUNDBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)
			return true;
	}
	else if (action == AIRBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD)
			return true;
	}
	else if (action == AIRBLOCKDOWNFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| (facingHitbox && (hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::AIRUPFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| (facingHitbox &&
			(hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRFORWARD)))
			return true;
	}
	else if (action == AIRBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| hpt == HitboxInfo::HitPosType::AIRFORWARD)
			return true;
	}

	return false;
}

bool Actor::CanFullBlock(HitboxInfo::HitPosType hpt, V2d &hitPos, bool attackFacingRight)
{
	bool facingHitbox = (facingRight && position.x - hitPos.x <= 0)
		|| (!facingRight && position.x - hitPos.x >= 0);

	bool opposingFacing = facingRight != attackFacingRight;

	bool frontalAttack = opposingFacing || facingHitbox;

	if (hpt == HitboxInfo::OMNI)
	{
		hpt = HitboxInfo::GetAirType(normalize(position - hitPos));
	}

	if (action == GROUNDBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| hpt == HitboxInfo::HitPosType::AIRUPFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDLOW)
			return true;
	}
	else if (action == GROUNDBLOCKDOWNFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP
			|| ( opposingFacing && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			|| (frontalAttack &&
				(hpt == HitboxInfo::HitPosType::GROUND
				|| hpt == HitboxInfo::HitPosType::GROUNDLOW ) ) )
			return true;
	}
	else if (action == GROUNDBLOCKFORWARD)
	{
		if ( (opposingFacing && hpt == HitboxInfo::HitPosType::AIRFORWARD)
			|| (frontalAttack && hpt == HitboxInfo::HitPosType::GROUND ) )
			return true;
	}
	else if (action == GROUNDBLOCKUPFORWARD)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| (frontalAttack &&
			(hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)))
			return true;
	}
	else if (action == GROUNDBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN
			|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)
			return true;
	}
	else if (action == AIRBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUP)
			return true;
	}
	else if (action == AIRBLOCKDOWNFORWARD)
	{
		if (opposingFacing && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			return true;
	}
	else if (action == AIRBLOCKFORWARD)
	{
		if (opposingFacing && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			return true;
	}
	else if (action == AIRBLOCKUPFORWARD)
	{
		if (opposingFacing && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			return true;
	}
	else if (action == AIRBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWN)
			return true;
	}

	return false;
}

bool Actor::CanHalfBlock(HitboxInfo::HitPosType hpt, V2d &hitPos, bool attackFacingRight)
{
	bool facingHitbox = (facingRight && position.x - hitPos.x <= 0)
		|| (!facingRight && position.x - hitPos.x >= 0);

	bool opposingFacing = facingRight != attackFacingRight;

	bool frontalAttack = opposingFacing || facingHitbox;

	double posAngle = GetVectorAngleCCW(normalize( position - hitPos));
	if (posAngle < 0)
	{
		posAngle += 2.0 * PI;
	}
	int angleMult = posAngle / (PI / 4.0);

	if (hpt == HitboxInfo::OMNI)
	{
		hpt = HitboxInfo::GetAirType(normalize(position - hitPos));
	}

	if (action == GROUNDBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::GROUND)
			return true;
	}
	else if (action == GROUNDBLOCKDOWNFORWARD)
	{
		if( (!opposingFacing && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			|| (!frontalAttack && hpt == HitboxInfo::HitPosType::GROUNDLOW) )
			return true;
	}
	else if (action == GROUNDBLOCKFORWARD)
	{
		if (frontalAttack)
		{
			if (hpt == HitboxInfo::HitPosType::GROUNDHIGH
				|| hpt == HitboxInfo::HitPosType::GROUNDLOW)
				return true;
		}
		if (opposingFacing)
		{
			if (hpt == HitboxInfo::HitPosType::AIRUPFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
				return true;
		}
	}
	else if (action == GROUNDBLOCKUPFORWARD)
	{
		if (frontalAttack)
		{
			if (hpt == HitboxInfo::HitPosType::GROUND)
				return true;
		}
		else
		{
			if (hpt == HitboxInfo::HitPosType::AIRFORWARD
				|| hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD
				|| hpt == HitboxInfo::HitPosType::GROUNDHIGH)
				return true;
		}
	}
	else if (action == GROUNDBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRFORWARD
			|| hpt == HitboxInfo::HitPosType::GROUND)
			return true;
	}
	else if (action == AIRBLOCKDOWN)
	{
		if (hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			return true;
	}
	else if (action == AIRBLOCKDOWNFORWARD)
	{
		if (opposingFacing && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			return true;
	}
	else if (action == AIRBLOCKFORWARD)
	{
		if (opposingFacing && hpt == HitboxInfo::HitPosType::AIRUPFORWARD)
			return true;
	}
	else if (action == AIRBLOCKUPFORWARD)
	{
		if (opposingFacing && hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD)
			return true;
	}
	else if (action == AIRBLOCKUP)
	{
		if (hpt == HitboxInfo::HitPosType::AIRDOWNFORWARD )
			return true;
	}

	return false;
}

bool Actor::CanParry(HitboxInfo::HitPosType hpt, V2d &hitPos, bool attackFacingRight)
{
	if ((CanFullBlock(hpt, hitPos, attackFacingRight)
		|| CanHalfBlock(hpt, hitPos, attackFacingRight))
		&& framesBlocking < 10)
	{
		return true;
	}

	return false;
}



Actor::HitResult Actor::CheckIfImHitByEnemy( Enemy *hitter, CollisionBody *hitBody, int hitFrame,
	HitboxInfo::HitPosType hpt, V2d &hitPos, bool attackFacingRight,
	bool canBeParried, bool canBeBlocked)
{
	if (IntersectMyHurtboxes(hitBody, hitFrame))
	{
		if (hitter != NULL && EnemyRecentlyHitMe(hitter))
		{
			return HitResult::MISS;
		}

		if( hitBody->hitboxInfo != NULL && !hitBody->hitboxInfo->sensor )
			AddRecentEnemyHitter(hitter);


		if (IsInvincible()
			&& (hitBody->hitboxInfo == NULL
				|| !hitBody->hitboxInfo->hitsThroughInvincibility))
		{
			return HitResult::INVINCIBLEHIT;
		}
		/*else if (canBeParried && CanParry(hpt, hitPos, attackFacingRight))
		{
			return HitResult::PARRY;
		}*/
		else if (canBeBlocked && CanBlockEnemy(hpt, hitPos))
		{
			return HitResult::FULLBLOCK;
		}
		/*else if (canBeBlocked && CanFullBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::FULLBLOCK;
		}
		else if (canBeBlocked && CanHalfBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::HALFBLOCK;
		}*/
		else
		{
			return HitResult::HIT;
		}
	}

	return HitResult::MISS;
}

Actor::HitResult Actor::CheckIfImHitByBullet( BasicBullet *hitter, CollisionBox &cb, HitboxInfo::HitPosType hpt,
	V2d &hitPos, bool attackFacingRight,
	bool canBeParried, bool canBeBlocked)
{
	if (IntersectMyHurtboxes(cb))
	{
		if (hitter != NULL && BulletRecentlyHitMe(hitter))
		{
			return HitResult::MISS;
		}

		AddRecentBulletHitter(hitter);

		if (IsInvincible())
		{
			return HitResult::INVINCIBLEHIT;
		}
		else if (canBeBlocked && CanBlockBullet(hpt, hitPos, attackFacingRight))//, hitPos))
		{
			return HitResult::FULLBLOCK;
		}
		/*else if (canBeParried && CanParry(hpt, hitPos, attackFacingRight))
		{
			return HitResult::PARRY;
		}
		else if (canBeBlocked && CanFullBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::FULLBLOCK;
		}
		else if (canBeBlocked && CanHalfBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::HALFBLOCK;
		}*/
		else
		{
			return HitResult::HIT;
		}
	}

	return HitResult::MISS;
}

Actor::HitResult Actor::CheckIfImHit(CollisionBody *hitBody, int hitFrame,
	 HitboxInfo::HitPosType hpt, V2d &hitPos, bool attackFacingRight,
	bool canBeParried, bool canBeBlocked )
{
	if (IntersectMyHurtboxes(hitBody, hitFrame))
	{
		if (IsInvincible() 
			&& (hitBody->hitboxInfo == NULL 
			|| !hitBody->hitboxInfo->hitsThroughInvincibility))
		{
			return HitResult::INVINCIBLEHIT;
		}
		else if (canBeParried && CanParry(hpt, hitPos, attackFacingRight))
		{
			return HitResult::PARRY;
		}
		else if (canBeBlocked && CanFullBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::FULLBLOCK;
		}
		else if (canBeBlocked && CanHalfBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::HALFBLOCK;
		}
		else
		{
			return HitResult::HIT;
		}
	}

	return HitResult::MISS;
}

Actor::HitResult Actor::CheckIfImHit(CollisionBox &cb, HitboxInfo::HitPosType hpt,
	V2d &hitPos, bool attackFacingRight,
	bool canBeParried, bool canBeBlocked)
{
	if (IntersectMyHurtboxes(cb))
	{
		if (IsInvincible() )
		{
			return HitResult::INVINCIBLEHIT;
		}
		else if ( canBeParried && CanParry(hpt, hitPos, attackFacingRight))
		{
			return HitResult::PARRY;
		}
		else if ( canBeBlocked && CanFullBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::FULLBLOCK;
		}
		else if ( canBeBlocked && CanHalfBlock(hpt, hitPos, attackFacingRight))
		{
			return HitResult::HALFBLOCK;
		}
		else
		{
			return HitResult::HIT;
		}
	}

	return HitResult::MISS;
}

void Actor::ClearPauseBufferedActions()
{
	pauseBufferedJump = false;
	pauseBufferedAttack = Action::Count;
	pauseBufferedDash = false;
}

bool Actor::IsBlockAction(int a)
{
	return IsActionAirBlock(a) || IsActionGroundBlock(a);
}

bool Actor::IsGrindAction(int a)
{
	return action == GRINDBALL || action == GRINDATTACK || action == SPRINGSTUNGRIND;
}

bool Actor::IsAttackAction( int a )
{
	return (a == FAIR || a == DAIR || a == UAIR || a == AIRDASHFORWARDATTACK || a == DIAGDOWNATTACK
		|| a == DIAGUPATTACK || a == WALLATTACK || IsGroundAttackAction(a));
}

bool Actor::IsGroundAttackAction(int a)
{
	return (a == STANDATTACK1
		|| a == STANDATTACK2
		|| a == STANDATTACK3
		|| a == STANDATTACK4
		|| a == STEEPCLIMBATTACK || a == STEEPSLIDEATTACK
		|| a == DASHATTACK || a == DASHATTACK2 || a == DASHATTACK3 );
}

bool Actor::IsSpringAction(int a)
{
	return a == SPRINGSTUN || a == SPRINGSTUNGLIDE || a == SPRINGSTUNBOUNCE || a == SPRINGSTUNAIRBOUNCE
		|| a == SPRINGSTUNTELEPORT
		|| a == SPRINGSTUNAIM || a == AIMWAIT || a == SPRINGSTUNHOMING
		|| a == SPRINGSTUNHOMINGATTACK || a == SPRINGSTUNGRIND 
		|| a == SPRINGSTUNGRINDFLY || a == SPRINGSTUNANNIHILATION
		|| a == SPRINGSTUNANNIHILATIONATTACK;
}

bool Actor::IsOnRailAction(int a)
{
	return a == RAILGRIND || a == RAILSLIDE || a == LOCKEDRAILSLIDE;
}

bool Actor::IsInHistunAction( int a )
{
	return a == GROUNDHITSTUN || a == AIRHITSTUN;
}

V2d Actor::GetKnockbackDirFromVel()
{
	if (ground != NULL)
	{
		return normalize(-groundSpeed * (ground->v1 - ground->v0));
	}
	else
	{
		if (velocity.x > 0)
		{
			return V2d(-1, 0);
		}
		else
		{
			return V2d(1, 0);
		}
		//return normalize(-velocity);
	}
}

V2d Actor::GetTrueVel()
{
	if (grindEdge != NULL)
	{
		return grindEdge->Along() * grindSpeed;
	}
	else if (bounceEdge != NULL)
	{
		return storedBounceVel;
	}
	else if (ground != NULL)
	{
		return ground->Along() * groundSpeed;
	}
	else
	{
		return velocity;
	}
}

void Actor::RestoreDoubleJump()
{
	if( !hasDoubleJump )
		extraDoubleJump = true;
	//hasDoubleJump = true;
}

void Actor::RestoreAirDash()
{
	hasAirDash = true;
}

int Actor::GetCurrStandAttack()
{
	int a = -1;
	switch (standAttackLevel)
	{
	case 0:
		a = STANDATTACK1;
		break;
	case 1:
		a = STANDATTACK2;
		break;
	case 2:
		a = STANDATTACK3;
		break;
	case 3:
		a = STANDATTACK4;
		break;
	}

	return a;
}

int Actor::GetCurrDashAttack()
{
	int a = -1;
	switch (dashAttackLevel)
	{
	case 0:
		a = DASHATTACK;
		break;
	case 1:
		a = DASHATTACK2;
		break;
	case 2:
		a = DASHATTACK3;
		break;
	}

	return a;
}

int Actor::GetCurrUpTilt()
{
	int a = -1;
	switch (upTiltLevel)
	{
	case 0:
		a = UPTILT1;
		break;
	case 1:
		a = UPTILT2;
		break;
	case 2:
		a = UPTILT3;
		break;
	}

	return a;
}

int Actor::GetCurrDownTilt()
{
	return -1;
}

void Actor::SteepSlideMovement()
{
	double fac = GetGravity() * steepSlideGravFactor;//gravity * 2.0 / 3.0;

	if (HoldingRelativeDown())
	{
		double currFactor = 0;
		double upgradeAmount = steepSlideFastGravFactor * .2;


		if (reversed)
		{
			int numCeilingSlideUpgrades = IsOptionOn(UPGRADE_W3_CEILING_STEEP_SLIDE_1) + IsOptionOn(UPGRADE_W4_CEILING_STEEP_SLIDE_2) + IsOptionOn(UPGRADE_W5_CEILING_STEEP_SLIDE_3);
			currFactor = steepSlideFastGravFactor + upgradeAmount * numCeilingSlideUpgrades;
		}
		else
		{
			int numSlideUpgrades = IsOptionOn(UPGRADE_W1_STEEP_SLIDE_1) + IsOptionOn(UPGRADE_W2_STEEP_SLIDE_2) + IsOptionOn(UPGRADE_W6_STEEP_SLIDE_3);
			currFactor = steepSlideFastGravFactor + upgradeAmount * numSlideUpgrades;
		}

		fac = GetGravity() * currFactor;
	}


	groundSpeed += dot(V2d(0, fac), normalize(ground->v1 - ground->v0)) / slowMultiple;

	if (InWater(TerrainPolygon::WATER_NORMAL))
	{
		V2d vel;
		if (reversed)
		{
			vel = ground->Along() * -groundSpeed;

			if (vel.y < -normalWaterMaxFallSpeed)
			{
				vel.y = -normalWaterMaxFallSpeed;
			}
		}
		else
		{
			vel = ground->Along() * groundSpeed;

			if (vel.y > normalWaterMaxFallSpeed)
			{
				vel.y = normalWaterMaxFallSpeed;
			}
		}

		double len = length(vel);
		if (groundSpeed > 0)
		{
			groundSpeed = len;
		}
		else
		{
			groundSpeed = -len;
		}
	}
}

void Actor::SteepClimbMovement()
{
	bool boost = TryClimbBoost();

	float factor = steepClimbGravFactor;
	if (HoldingRelativeUp())
	{
		//the factor is just to make you climb a little farther
		factor = steepClimbUpFactor;
	}
	else if (HoldingRelativeDown())
	{
		factor = steepClimbDownFactor;
	}

	if (reversed)
	{
		groundSpeed += dot(V2d(0, GetGravity() * factor), normalize(ground->v1 - ground->v0)) / slowMultiple;
	}
	else
	{
		groundSpeed += dot(V2d(0, GetGravity() * factor), normalize(ground->v1 - ground->v0)) / slowMultiple;
		//cout << "groundspeed: " << groundSpeed << endl;
	}
}

void Actor::StartStandAttack()
{
	standAttackLevel++;
	if (standAttackLevel == 4)
		standAttackLevel = 0;

	framesSinceStandAttack = 0;
}

void Actor::StartDashAttack()
{
	dashAttackLevel++;
	if (dashAttackLevel == 3)
		dashAttackLevel = 0;

	framesSinceDashAttack = 0;
}

void Actor::StartUpTilt()
{
	upTiltLevel++;
	if (upTiltLevel == 3)
		upTiltLevel = 0;

	framesSinceUpTilt = 0;
}

void Actor::StartDownTilt()
{
	downTiltLevel++;
	if (downTiltLevel == 3)
		downTiltLevel = 0;

	framesSinceDownTilt = 0;
}

void Actor::UpdateInHitlag()
{
	if (IsAttackAction(action) && currAttackHit )
	{
		if (pauseBufferedAttack == Action::Count)
		{
			if (AttackButtonPressed())
			{
				if (ground == NULL)
				{
					if (currInput.LUp())
					{
						pauseBufferedAttack = UAIR;
					}
					else if (currInput.LDown())
					{
						pauseBufferedAttack = DAIR;
					}
					else
					{
						pauseBufferedAttack = FAIR;
					}
				}
				else
				{
					if (action == STEEPCLIMBATTACK)
						pauseBufferedAttack = STEEPCLIMBATTACK;
					else if (action == STEEPSLIDEATTACK)
					{
						pauseBufferedAttack = STEEPSLIDEATTACK;
					}
					/*else if (currInput.LUp())
					{
						pauseBufferedAttack = GetCurrUpTilt();
					}*/
					else
					{
						pauseBufferedAttack = GetCurrStandAttack();
					}
				}
			}
		}

		if (!pauseBufferedJump && JumpButtonPressed())
		{
			pauseBufferedJump = true;
		}

		if (!pauseBufferedDash && DashButtonPressed())
		{
			if( ground != NULL || ( ground == NULL && hasAirDash ) )
				pauseBufferedDash = true;
		}
	}

	UpdateDrain();

	UpdateBubbles();

	TryChangePowerMode();

	if (flashFrames > 0)
	{
		--flashFrames;

		if (flashFrames <= 2)
		{
			swordState = SWORDSTATE_REGULAR;
		}
	}
		

	//might be more stuff missing here?

	//if (flashFrames > 0)
	//{
	//	//not having the >0 thing caused a desync here at some point and I don't know why
	//	//but at least its fixed for now...so weird.
	//	--flashFrames;
	//}
}

 pair<bool, bool> Actor::PlayerHitMe( int otherPlayerIndex )
{
	Actor *player = NULL;
	switch( otherPlayerIndex )
	{
	case 0:
		player = sess->GetPlayer( 0 );
		break;
	case 1:
		player = sess->GetPlayer( 1 );
		break;
	}

	if( player->currHitboxes != NULL )
	{
		return pair<bool, bool>(true,false);
	}
	return pair<bool, bool>(false,false);
}

 void Actor::CollectCurrency(CurrencyItem *ci)
 {
	 HealTimer(ci->GetHealAmount());
	 ci->Collect();
	 AddToCurrencyCounter(ci->GetCounterAmount());
 }

 void Actor::SetAirBlockAction()
 {
	 bool forwardHeld = currInput.LLeft() || currInput.LRight();

	 if (currInput.LUp())
	 {
		 if (forwardHeld)
		 {
			 if (action != AIRBLOCKUPFORWARD)
				SetAction(AIRBLOCKUPFORWARD);
		 }
		 else
		 {
			 if (action != AIRBLOCKUP)
			 SetAction(AIRBLOCKUP);
		 }
	 }
	 else if (currInput.LDown())
	 {
		 if (forwardHeld)
		 {
			 if (action != AIRBLOCKDOWNFORWARD)
				SetAction(AIRBLOCKDOWNFORWARD);
		 }
		 else
		 {
			 if (action != AIRBLOCKDOWN)
				SetAction(AIRBLOCKDOWN);
		 }
	 }
	 else
	 {
		 if (action != AIRBLOCKFORWARD)
			 SetAction(AIRBLOCKFORWARD);
	 }
 }

 void Actor::SetGroundBlockAction()
 {
	 bool forwardHeld = currInput.LLeft() || currInput.LRight();

	 bool up = HoldingRelativeUp();
	 bool down = HoldingRelativeDown();

	/* if (reversed)
	 {
		 up = !up;
		 down = !down;
	 }*/

	 if (up)
	 {
		 if (forwardHeld)
		 {
			 if (action != GROUNDBLOCKUPFORWARD)
				 SetAction(GROUNDBLOCKUPFORWARD);
		 }
		 else
		 {
			 if (action != GROUNDBLOCKUP)
				 SetAction(GROUNDBLOCKUP);
		 }
	 }
	 else if (down)
	 {
		 if (forwardHeld)
		 {
			 if (action != GROUNDBLOCKDOWNFORWARD)
				 SetAction(GROUNDBLOCKDOWNFORWARD);
		 }
		 else
		 {
			 if (action != GROUNDBLOCKDOWN)
				 SetAction(GROUNDBLOCKDOWN);
		 }
	 }
	 else
	 {
		 if (action != GROUNDBLOCKFORWARD)
			 SetAction(GROUNDBLOCKFORWARD);
	 }
 }

 void Actor::TryResetBlockCounter()
 {
	 //framesSinceBlockPress might break parrying slightly?
	 //its to prevent buffered parries. we'll have to test it out
	 if (!IsBlockAction(oldAction) && framesSinceBlockPress >= 0 && framesSinceBlockPress < 10 )
	 {
		 framesBlocking = 0;
	 }
 }

 void Actor::AirBlockChange()
 {
	 if (currInput.LLeft())
	 {
		 facingRight = false;
	 }
	 else if (currInput.LRight())
	 {
		 facingRight = true;
	 }

	 if (!PowerButtonHeld() && blockstunFrames == 0)
	 {
		 SetAction(JUMP);
		 frame = 1;
	 }
	 else
	 {
		 if (TryDoubleJump()) return;

		 if (AirAttack()) return;

		 SetAirBlockAction();
	 }
 }

 void Actor::GroundBlockChange()
 {
	 if (currInput.LLeft())
	 {
		 facingRight = false;
	 }
	 else if (currInput.LRight())
	 {
		 facingRight = true;
	 }

	 if (blockstunFrames == 0)
	 {
		 if (!PowerButtonHeld())
		 {
			 SetAction(STAND);
			 frame = 0;
			 return;
		 }
		 else
		 {
			 if (TryJumpSquat()) return;

			 if (TryGroundAttack()) return;
		 }
	 }

	 SetGroundBlockAction();
 }

 void Actor::UpdateGroundedSwordSprite( Tileset *ts, int startFrame, int endFrame, int animMult,
	 Vector2f &offset )
 {
	 showSword = true;

	 if ( frame < startFrame * animMult || ( endFrame >= 0 && frame >= endFrame * animMult) )
		 showSword = false;

	 if (showSword)
	 {
		 swordSprite.setTexture(*ts->texture);
	 }

	 bool r = (facingRight && !reversed) || (!facingRight && reversed);

	 if (showSword)
	 {
		 int tile = frame / 2 - startFrame;
		 if (r)
		 {
			 
			 swordSprite.setTextureRect(ts->GetSubRect(tile));

			 //swordShader.SetQuad(ts, tile);
			 //swordShader.SetAuraColor(Color::White);
			 
		 }
		 else
		 {
			 sf::IntRect irSword = ts->GetSubRect(tile);
			 sf::IntRect ir(irSword.left + irSword.width, irSword.top, -irSword.width, irSword.height);
			 //swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
			//	 irSword.top, -irSword.width, irSword.height));
			 swordSprite.setTextureRect(ir);

			 //swordShader.SetQuad(ts, tile);
			 //swordShader.SetAuraColor(Color::White);

			 offset.x = -offset.x;
		 }
	 }

	 V2d trueNormal;
	 double angle = GroundedAngleAttack(trueNormal);

	 if (showSword)
	 {
		 swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2,
			 swordSprite.getLocalBounds().height / 2);
		 swordSprite.setRotation(angle / PI * 180);
	 }

	 V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	 V2d truDir(-trueNormal.y, trueNormal.x);

	 pos += truDir * (double)offset.x;
	 pos += -trueNormal * (double)(offset.y - 32);//sprite->getLocalBounds().height / 2);

	 swordSprite.setPosition(pos.x, pos.y);
 }

 void Actor::UpdateGroundedAttackSprite(
	 int a, Tileset *ts_sword, int startSword, int endSword, int animMult, Vector2f &swordOffset)
 {
	 int f = frame / animMult;

	 SetSpriteTexture(a);
	 bool r = (facingRight && !reversed) || (!facingRight && reversed);
	 SetSpriteTile(f, r);
	 SetGroundedSpriteTransform();
	 UpdateGroundedSwordSprite(ts_sword, startSword, endSword, animMult, swordOffset );

	 //use placeholder tile for now, eventually include parameters to use custom scorp stuff per attack
	 if (scorpOn)
	 {
		 scorpSprite.setTexture(*ts_scorpRun->texture);

		 bool r = (facingRight && !reversed) || (!facingRight && reversed);

		 SetSpriteTile(&scorpSprite, ts_scorpDash, 0, r, !reversed);

		 scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			 scorpSprite.getLocalBounds().height / 2 + 20);
		 scorpSprite.setPosition(position.x, position.y);
		 scorpSprite.setRotation(sprite->getRotation());
		 scorpSet = true;
	 }
 }

 bool Actor::CheckIfEnemyIsTrackable(Enemy *e)
 {
	 return e->IsHomingTarget() && EnemyTracker::CheckIfEnemyIsTrackable(e);
 }

 void Actor::SetSkin(int skinIndex)
 {
	 if (skinIndex == SKIN_NORMAL)
	 {
		 currSkinIndex = sess->GetPlayerNormalSkin(actorIndex);
	 }
	 else
	 {
		 currSkinIndex = skinIndex;
	 }

	 if (skinIndex == SKIN_NORMAL)
	 {
		if( antiTimeSlowFrames > 0)
		{
			currSkinIndex = SKIN_PINK;
		}
	 }

	 skinShader.SetSkin(currSkinIndex);
	 fxPaletteShader->SetPaletteIndex(currSkinIndex);
	 mapIconShader.SetSkin(currSkinIndex);
 }

 

 void Actor::BlendSkins(int first, int second, float progress)
 {
	skinShader.BlendSkins( first, second, progress );
 }

 bool Actor::IsNormalSkin()
 {
	 return (currSkinIndex == sess->GetPlayerNormalSkin(actorIndex));
 }

 int Actor::GetSkinIndexFromString(const std::string &s)
 {
	if (s == "SKIN_NORMAL"){return SKIN_NORMAL; }
	 else if (s == "SKIN_RED"){return SKIN_RED;}
	 else if (s == "SKIN_BLUE"){return SKIN_BLUE; }
	 else if (s == "SKIN_ORANGE"){return SKIN_ORANGE; }
	 else if (s == "SKIN_PINK"){return SKIN_PINK; }
	 else if (s == "SKIN_LIGHT"){return SKIN_LIGHT; }
	 else if (s == "SKIN_GHOST"){return SKIN_GHOST; }
	 else if (s == "SKIN_DARK"){return SKIN_DARK; }
	 else if (s == "SKIN_VILLAIN"){return SKIN_VILLAIN; }
	 else if (s == "SKIN_TOXIC"){return SKIN_TOXIC; }
	 else if (s == "SKIN_AMERICA"){return SKIN_AMERICA; }
	 else if (s == "SKIN_METAL"){return SKIN_METAL; }
	 else if (s == "SKIN_GOLD"){return SKIN_GOLD; }
	 else if (s == "SKIN_PURPLE"){return SKIN_PURPLE; }
	 else if (s == "SKIN_MAGI"){return SKIN_MAGI; }
	 else if (s == "SKIN_GLIDE"){return SKIN_GLIDE; }
	 else if (s == "SKIN_BONFIRE"){return SKIN_BONFIRE; }
	 else if (s == "SKIN_GDUBS"){return SKIN_GDUBS; }
	 else if (s == "SKIN_SHADOW"){return SKIN_SHADOW; }
	 else if (s == "SKIN_DUSK"){return SKIN_DUSK; }
	 else if (s == "SKIN_DAWN"){return SKIN_DAWN; }
	 else if (s == "SKIN_TRIX"){return SKIN_TRIX; }
	 else
	 {
		 cout << "wrong string for skin: " << s << endl;
		 assert(0);
		 return SKIN_NORMAL;
	 }
 }

 void Actor::StopRepeatingSound()
 {
	 if (simulationMode)
	 {
		 return;
	 }

	 if (repeatingSound != NULL)
	 {
		 DeactivateSound(repeatingSound);
		 repeatingSound = NULL;
	 }
 }

 Actor::FXInfo::FXInfo()
 {
	 pool = NULL;
	 layer = DrawLayer::BETWEEN_PLAYER_AND_ENEMIES;
	 pauseImmune = false;
	 
	 //startFrame = 0;
	 //duration = 0;
	 //animFactor = 0;
 }

 Actor::FXInfo::~FXInfo()
 {
	 if (pool != NULL)
	 {
		 delete pool;
	 }
 }

 void Actor::FXInfo::Set(Tileset *p_ts, int p_fxType, int p_maxEffects, int p_drawLayer, bool p_pauseImmune, bool p_usesPlayerSkinShader)
 {
	 if (p_ts == NULL)
	 {
		 cout << "missing tileset" << endl;
		 assert(0);
	 }

	 usesPlayerSkinShader = p_usesPlayerSkinShader;

	 pool = new EffectPool((EffectType)p_fxType, p_maxEffects);
	 pool->ts = p_ts;

	 layer = p_drawLayer;

	 pauseImmune = p_pauseImmune;
 }

 void Actor::LayeredDrawEffects(int p_drawLayer, sf::RenderTarget *target)
 {
	 for (auto it = effectPools.begin(); it != effectPools.end(); ++it)
	 {
		 if( (*it).pool != NULL && (*it).layer == p_drawLayer)
		 {
			if ((*it).usesPlayerSkinShader)
			{
				fxPaletteShader->SetTileset((*it).pool->ts);
			}
			(*it).pool->Draw(target);
			 
		 }
	 }
 }

 void Actor::ClearAllEffects()
 {
	 for (auto it = effectPools.begin(); it != effectPools.end(); ++it)
	 {
		 if ((*it).pool != NULL)
		 {
			 (*it).pool->Reset();
		 }
	 }
 }

 void Actor::RevertAfterSimulating()
 {
	 assert(preSimulationState != NULL);
	 PopulateFromState(preSimulationState);
 }

 void Actor::ForwardSimulate(int frames, bool storePositions)
 {
	 assert(preSimulationState != NULL);

	 assert(frames <= Session::MAX_SIMULATED_FUTURE_PLAYER_FRAMES);

	 PopulateState(preSimulationState);
	 simulationMode = true;

	 numCalculatedFuturePositions = frames;

	 int numSteps;
	 for (int i = 0; i < frames; ++i)
	 {
		 UpdatePrePhysics();
		 physicsOver = false;
		 highAccuracyHitboxes = false; //maybe get this a few more substeps in the future. not sure why its not as accurate.
		 int numSteps = GetNumSteps();
		 for (int substep = 0; substep < numSteps; ++substep)
		 {
			 UpdatePhysics();
		 }
		 UpdatePostPhysics();
		 sess->UpdatePlayerInput(actorIndex);

		 if (storePositions)
		 {
			 futurePositions[i] = position;
		 }
	 }
	 simulationMode = false;
 }

 void Actor::InitPreFrameCalculations()
 {
	 numCalculatedFuturePositions = 0;
	 currFrameSimulationFrames = 0;
 }

 void Actor::UpdatePreFrameCalculations()
 {
	 if (currFrameSimulationFrames > 0)
	 {
		 ForwardSimulate(currFrameSimulationFrames, true);
		 RevertAfterSimulating();
	 }
 }

 void Actor::UpdateNumFuturePositions()
 {
	 int numFrames = sess->numSimulatedFramesRequired;
	 if (numFrames == 0)
	 {
		 if (preSimulationState != NULL)
		 {
			 delete preSimulationState;
			 preSimulationState = NULL;
		 }
	 }
	 else
	 {
		 if (preSimulationState == NULL)
		 {
			 preSimulationState = new PState;
			 memset(preSimulationState, 0, sizeof(PState));
		 }
	 }

	 if (futurePositions != NULL)
	 {
		 delete [] futurePositions;
		 futurePositions = NULL;
	 }

	 futurePositions = new V2d[numFrames];
 }

 void Actor::ProcessGroundedCollision()
 {
	 CheckCollisionForTerrainFade();

	 if (minContact.edge->rail != NULL
		 && minContact.edge->rail->GetRailType() == TerrainRail::BOUNCE)
	 {
		 if (bounceEdge == NULL)
		 {
			 bounceEdge = minContact.edge;
			 storedBounceGroundSpeed = groundSpeed;
		 }
		 //HandleBounceRail();
	 }
 }

 void Actor::CheckCollisionForTerrainFade()
 {
	 if (!simulationMode)
	 {
		 assert(minContact.edge != NULL);
		 if (minContact.edge->poly != NULL
			 && minContact.edge->poly->IsSometimesActiveType())
		 {
			 minContact.edge->poly->FadeOut();
		 }
		 else if (minContact.edge->rail != NULL
			 && minContact.edge->rail->rType == TerrainRail::FADE)
		 {
			 minContact.edge->rail->FadeOut();
		 }
	 }
 }

 void Actor::CheckGrindEdgeForTerrainFade()
 {
	 if (!simulationMode)
	 {
		 assert(grindEdge != NULL);
		 if (grindEdge->poly != NULL
			 && grindEdge->poly->IsSometimesActiveType())
		 {
			 grindEdge->poly->FadeOut();
		 }
		 else if (grindEdge->rail != NULL
			 && grindEdge->rail->rType == TerrainRail::FADE)
		 {
			 grindEdge->rail->FadeOut();
		 }
	 }
 }

 void Actor::InitEmitters()
 {
	 gravityIncreaserTrailEmitter->SetIDAndAddToAllEmittersVec();
	 gravityDecreaserTrailEmitter->SetIDAndAddToAllEmittersVec();
	 momentumBoosterTrailEmitter->SetIDAndAddToAllEmittersVec();
	 homingBoosterTrailEmitter->SetIDAndAddToAllEmittersVec();
	 antiTimeSlowBoosterTrailEmitter->SetIDAndAddToAllEmittersVec();
	 freeFlightBoosterTrailEmitter->SetIDAndAddToAllEmittersVec();
 }

 int Actor::GetNumStoredBytes()
 {
	 int totalSize = 0;

	 totalSize += sizeof(PState);

	 totalSize += sess->numSimulatedFramesRequired * sizeof(V2d);

	 return totalSize;
 }

 void Actor::StoreBytes(unsigned char *bytes)
 {
	 PopulateState(pState);
	 memcpy(bytes, pState, sizeof(PState));

	 bytes += sizeof(PState);

	 int numFrames = sess->numSimulatedFramesRequired;

	 int positionsSize = numFrames * sizeof(V2d);

	 memcpy(bytes, futurePositions, positionsSize);

	 bytes += positionsSize;
 }

 void Actor::SetFromBytes(unsigned char *bytes)
 {
	 memcpy(pState, bytes, sizeof(PState));
	 PopulateFromState(pState);

	 bytes += sizeof(PState);

	 int numFrames = sess->numSimulatedFramesRequired;

	 int positionsSize = numFrames * sizeof(V2d);

	 memcpy(futurePositions, bytes, positionsSize);

	 bytes += positionsSize;
 }