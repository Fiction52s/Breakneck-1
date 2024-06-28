#include "RushScoreDisplay.h"
#include "Medal.h"
#include "MedalSequence.h"
#include "Session.h"
#include "MainMenu.h"
#include "RushManager.h"
#include "KinUpgrades.h"
#include "RushUpgradePopup.h"
#include "TutorialBox.h"

using namespace sf;

RushScoreDisplay::RushScoreDisplay(RushManager *p_rushManager, sf::Font &f)
	:ScoreDisplay(p_rushManager, f)
{
	rushManager = p_rushManager;

	CreateDescriptionTable();

	upgradePop = new RushUpgradePopup(rushManager);

	Reset();

	ts_test = rushManager->GetSizedTileset("Menu/AdventureScoreDisplay/adventurescoretest_1920x1080.png");
	ts_test->SetSpriteTexture(testSpr);

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

void RushScoreDisplay::Activate()
{
	//action = A_SHOW;
	action = A_WAIT; //SHOW is for effects and transitions and stuff
	frame = 0;
	upgradePop->SetToMostRecentUpgrade();
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

	SetTableEntry( POWER_AIRDASH, "Airdash",
		"-Hold DASH in the air to hover!\n"
		"-Hold DASH and a direction to airdash in any of the 8 directions!\n"
		"-Press ATTACK while Airdashing diagonally for a special attack!");

	SetTableEntry( POWER_GRAV, "Gravity Reverse", 
		"-Hold DASH and up while touching a ceiling to reverse your gravity!\n"
		"-Gravity will remain reversed until you leave the ceiling you are on.");

	SetTableEntry(POWER_BOUNCE, "Bounce Scorpion",
		"-Use RLEFT to enter scorpion mode, and hold SHIELD to activate.\n"
		"-While on, you will bounce off of any surface you collide with!");

	SetTableEntry(POWER_GRIND, "Grind Wheel",
		"-Use RRIGHT to enter grind mode, and hold SHIELD to activate!\n"
		"-While on, you will move along your current surface regardless of slope.\n"
		"-Press ATTACK while grinding for a grind attack!");

	SetTableEntry(POWER_TIME, "Time Slow Bubble",
		"-Use RDOWN to enter time slow mode, and press SHIELD to create a bubble!\n"
		"-Enemies and bullets are slowed down while in a bubble.\n"
		"-Hold SHIELD while in a bubble to slow yourself down too!");

	SetTableEntry(POWER_RWIRE, "Double Wires",
		"-Use the double wires to swing and move around with total freedom!\n"
		"-Use LEFTWIRE to use the blue wire, and RIGHTWIRE to use the red wire!\n"
		"-Press a direction when launching the wire to aim it!\n"
		"-Keep holding the wire button after it is attached to swing from it!");

	SetTableEntry(POWER_RWIRE, "Double Wires",
		"-Use the double wires to swing and move around with total freedom!\n"
		"-Use LEFTWIRE to use the blue wire, and RIGHTWIRE to use the red wire!\n"
		"-Press a direction when launching the wire to aim it!\n"
		"-Keep holding the wire button after it is attached to swing from it!");

	SetTableEntry(UPGRADE_W1_DASH_BOOST, "Unlock Dash Boost",
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
		"Dash speed increased!");
		

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
		if (action == A_SHOW || action == A_WAIT)
		{
			target->draw(testSpr);
			upgradePop->Draw(target);
		}
	}
}