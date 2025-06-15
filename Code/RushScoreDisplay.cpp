#include "RushScoreDisplay.h"
#include "Medal.h"
#include "MedalSequence.h"
#include "Session.h"
#include "MainMenu.h"
#include "RushManager.h"
#include "KinUpgrades.h"
#include "RushUpgradePopup.h"
#include "TutorialBox.h"
#include "KinStore.h"
#include "KinExperienceBar.h"

using namespace sf;

RushScoreDisplay::RushScoreDisplay(RushManager *p_rushManager, sf::Font &f)
	:ScoreDisplay(p_rushManager, f)
{
	rushManager = p_rushManager;

	expBar = rushManager->expBar;

	CreateDescriptionTable();

	upgradePop = new RushUpgradePopup(rushManager);

	kinStore = rushManager->kinStore;

	Reset();

	//ts_test = rushManager->GetSizedTileset("Menu/AdventureScoreDisplay/adventurescoretest_1920x1080.png");
	//ts_test->SetSpriteTexture(testSpr);

	testSpr.setPosition(0, 0);

	
}

RushScoreDisplay::~RushScoreDisplay()
{
	delete upgradePop;
}

void RushScoreDisplay::Reset()
{
	ScoreDisplay::Reset();
	action = A_IDLE;
	frame = 0;

	upgradePop->SetCenter(Vector2f(960, 800));
	//upgradePop->SetUpgrade(2);
	upgradePop->Reset();
}

void RushScoreDisplay::OpenStore()
{
	//action = A_SHOW;
	kinStore->sess = Session::GetSession();
	kinStore->Open();
	action = A_STORE;//A_WAIT; //SHOW is for effects and transitions and stuff
	frame = 0;
	//upgradePop->SetToMostRecentUpgrade();
}

void RushScoreDisplay::Activate()
{
	action = A_EXP;
	expBar->AddMedal(medalRank);
	frame = 0;
}

void RushScoreDisplay::Confirm()
{
	/*if (action == A_EXPBAR)
	{
		action = A_SHOW;
		frame = 0;
	}*/
}

void RushScoreDisplay::Deactivate()
{
	action = A_IDLE;
	frame = 0;
}

void RushScoreDisplay::Update()
{
	if (!IsActive())
		return;


	switch (action)
	{
	case A_EXP:
	{
		if (expBar->action == KinExperienceBar::A_IDLE)
		{
			rushManager->storePoints += expBar->gainedLevels;
			OpenStore();
			return;
		}
		/*else if (expBar->action == KinExperienceBar::A_LEVEL_UP)
		{
			rushManager->storePoints += 1;
			OpenStore();
			return;
		}*/

		expBar->Update();
		break;
	}
	case A_STORE:
	{
		if (kinStore->IsReadyToClose())
		{
			action = A_WAIT;
			frame = 0;
			return;
		}

		kinStore->Update();
		break;
	}
	}
	

	
	/*bool aPressed = sess->controllerStates[actorIndex]->ButtonPressed_A();
	bool xPressed = sess->controllerStates[actorIndex]->ButtonPressed_X();
	bool yPressed = sess->controllerStates[actorIndex]->ButtonPressed_Y();
	bool r1Pressed = sess->controllerStates[actorIndex]->ButtonPressed_RightShoulder();
	bool bPressed = sess->controllerStates[actorIndex]->ButtonPressed_B();
	bool startPressed = sess->controllerStates[actorIndex]->ButtonPressed_Start();*/
	/*if (action == A_SHOW && frame == 100)
	{
		action = A_WAIT;
		frame = 0;
	}*/

	++frame;
}

bool RushScoreDisplay::IsActive()
{
	return action != A_IDLE;
}

bool RushScoreDisplay::IsConfirmable()
{
	return false;
}

bool RushScoreDisplay::IsWaiting()
{
	return action == A_WAIT;
}



bool RushScoreDisplay::IsIncludingExtraOptions()
{
	return false;
}

void RushScoreDisplay::SetSession(Session *sess)
{
	upgradePop->tutBox->sess = sess;
}

void RushScoreDisplay::CreateDescriptionTable()
{
	upgradeDescriptionStringTable.resize(200); //just placeholder big number

	SetTableEntry( POWER_AIR_DASH, "Airdash",
		"-Hold DASH in the air to hover!\n"
		"-Hold DASH and a direction to airdash in any of the 8 directions!\n"
		"-Press ATTACK while Airdashing diagonally for a special attack!");

	SetTableEntry( POWER_GRAVITY_CLING, "Gravity Reverse", 
		"-Hold DASH and up while touching a ceiling to reverse your gravity!\n"
		"-Gravity will remain reversed until you leave the ceiling you are on.");

	SetTableEntry(POWER_BOUNCE_SCORPION, "Bounce Scorpion",
		"-Use RLEFT to enter scorpion mode, and hold SHIELD to activate.\n"
		"-While on, you will bounce off of any surface you collide with!");

	SetTableEntry(POWER_GRIND_BALL, "Grind Wheel",
		"-Use RRIGHT to enter grind mode, and hold SHIELD to activate!\n"
		"-While on, you will move along your current surface regardless of slope.\n"
		"-Press ATTACK while grinding for a grind attack!");

	SetTableEntry(POWER_HOMING_RUSH, "Time Slow Bubble",
		"-Use RDOWN to enter time slow mode, and press SHIELD to create a bubble!\n"
		"-Enemies and bullets are slowed down while in a bubble.\n"
		"-Hold SHIELD while in a bubble to slow yourself down too!");

	/*SetTableEntry(POWER_RWIRE, "Double Wires",
		"-Use the double wires to swing and move around with total freedom!\n"
		"-Use LEFTWIRE to use the blue wire, and RIGHTWIRE to use the red wire!\n"
		"-Press a direction when launching the wire to aim it!\n"
		"-Keep holding the wire button after it is attached to swing from it!");

	SetTableEntry(POWER_RWIRE, "Double Wires",
		"-Use the double wires to swing and move around with total freedom!\n"
		"-Use LEFTWIRE to use the blue wire, and RIGHTWIRE to use the red wire!\n"
		"-Press a direction when launching the wire to aim it!\n"
		"-Keep holding the wire button after it is attached to swing from it!");*/

	/*SetTableEntry(UPGRADE_W1_DASH_BOOST, "Unlock Dash Boost",
		"Let go of dash near the end to get a boost of speed!");

	SetTableEntry(UPGRADE_W1_STEEP_CLIMB_1, "Upgrade Steep Climb 1/3",
		"Climb steep slopes faster!");

	SetTableEntry(UPGRADE_W1_STEEP_SLIDE_1, "Upgrade Steep Slide 1/3",
		"Slide down steep slopes faster!");

	SetTableEntry(UPGRADE_W1_PASSIVE_GROUND_1, "Upgrade Ground Acceleration 1/3",
		"Increased passive acceleration during grounded movement!");

	SetTableEntry(UPGRADE_W1_SPRINT_1, "Upgrade Sprint 1/3",
		"Increased acceleration from sprinting on slopes!");

	SetTableEntry(UPGRADE_W1_BASE_DASH_1, "Upgrade Base Dash Speed 1/3",
		"Dash speed increased!");*/
		

	//leftwire entry left blank for now, since right wire is double
}

void RushScoreDisplay::SetTableEntry( int index, const std::string & s1, const std::string &s2)
{
	upgradeDescriptionStringTable[index] = std::make_pair(s1, s2);
}

void RushScoreDisplay::Draw(sf::RenderTarget *target)
{
	if (IsActive())
	{
		if (action == A_EXP)
		{
			rushManager->expBar->Draw(target);
		}
		else if (action == A_WAIT || action == A_STORE)
		{
			kinStore->Draw(target);
		}
	}
}