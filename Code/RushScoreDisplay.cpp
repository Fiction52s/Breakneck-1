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
#include "BackpackCounter.h"
#include "GoalMedal.h"
#include "AbsorbParticles.h"
#include <string>

using namespace sf;
using namespace std;


RushScorePanel::RushScorePanel(TilesetManager *tm)
	:showBez( 0, 0, 1, 1 ), hideBez( 0, 0, 1, 1 )
{
	//SetRectColor(panelQuad, Color::Red);
	Reset();

	/*A_HIDE,
		A_ENTER,
		A_SHOW,
		A_LEAVE,
		A_Count*/

	ts_panel = tm->GetSizedTileset("HUD/SidePanel_619x882.png");

	actionLength[A_HIDE] = 1;
	actionLength[A_ENTER] = 30;
	actionLength[A_SHOW] = 1;
	actionLength[A_LEAVE] = 30;

	showPos = Vector2f(1301, 115);
	hidePos = Vector2f(1920 + 50, 115);

	MainMenu *mm = MainMenu::GetInstance();
	levelNumberText.setFont(mm->oxanium);
	levelNumberText.setCharacterSize(50);
	levelNumberOffset = Vector2f(117, 118);

	levelNumberText.setString("1-2");

	levelNameText.setFont(mm->oxanium);
	levelNameText.setCharacterSize(45);
	levelNameOffset = Vector2f(196, 118);

	levelNameText.setString("testing");

	bestTimeText.setFont(mm->oxanium);
	bestTimeText.setFillColor(Color::Yellow);
	bestTimeText.setCharacterSize(90);
	bestTimeOffset = Vector2f(204, 292);

	bestTimeText.setString("10 : 45");

	currentTimeText.setFont(mm->oxanium);
	currentTimeText.setCharacterSize(90);
	currentTimeOffset = Vector2f(204, 440);

	currentTimeText.setString("11 : 45");

	skillPointsText.setFont(mm->oxanium);
	skillPointsText.setCharacterSize(75);
	skillPointsOffset = Vector2f(138, 585);

	skillPointsText.setString("x4");

	currencyText.setFont(mm->oxanium);
	currencyText.setCharacterSize(75);
	currencyOffset = Vector2f(406, 585);

	currencyText.setString("x50");

	nextText.setFont(mm->oxanium);
	nextText.setCharacterSize(40);
	nextOffset = Vector2f(98, 720);
	nextText.setString("NEXT");
	nextText.setOrigin(0, nextText.getLocalBounds().top + nextText.getLocalBounds().height / 2);

	retryText.setFont(mm->oxanium);
	retryText.setCharacterSize(40);
	retryOffset = Vector2f(356, 720);
	retryText.setString("RETRY");
	retryText.setOrigin(0, retryText.getLocalBounds().top + retryText.getLocalBounds().height / 2);
	
	buttonSize = 64;
	
	continueButtonOffset = Vector2f(242, 686);
	retryButtonOffset = Vector2f(498, 686);

	SetRectSubRect(panelQuad, ts_panel->GetSubRect(0));
}

void RushScorePanel::Reset()
{
	action = A_HIDE;
	frame = 0;
	
}

void RushScorePanel::Update()
{
	float width = ts_panel->tileWidth;
	float height = ts_panel->tileHeight;

	if (frame == actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case A_ENTER:
			action = A_SHOW;
			break;
		case A_LEAVE:
			action = A_HIDE;
			break;
		}
	}

	switch (action)
	{
	case A_ENTER:
	{
		double df = frame;
		df = df / actionLength[action];
		float f = showBez.GetValue(df);
		Vector2f newPos = showPos * f + hidePos * (1.f - f);
		SetTopLeft(newPos);
		break;
	}
	case A_SHOW:
		SetTopLeft(showPos);
		break;
	case A_LEAVE:
	{
		double df = frame;
		df = df / actionLength[action];
		float f = hideBez.GetValue(df);
		Vector2f newPos = hidePos * f + showPos * (1.f - f);
		SetTopLeft(newPos);
		break;
	}
	}

	++frame;
}

void RushScorePanel::SetInfo(Session *sess)
{
	RushManager *rm = MainMenu::GetInstance()->rushManager;

	if (rm != NULL)
	{
		levelNumberText.setString(to_string(rm->currWorld + 1) + "-" + to_string(rm->currRushMapIndex + 1));
		levelNumberText.setOrigin(levelNumberText.getLocalBounds().left + levelNumberText.getLocalBounds().width / 2,
			levelNumberText.getLocalBounds().top + levelNumberText.getLocalBounds().height / 2);

		levelNameText.setString(sess->mapHeader->fullName);
		levelNameText.setOrigin(0, levelNameText.getLocalBounds().top + levelNameText.getLocalBounds().height / 2);

		bestTimeText.setString(GetTimeStr(sess->totalFramesBeforeGoal));
		bestTimeText.setOrigin(0, bestTimeText.getLocalBounds().top + bestTimeText.getLocalBounds().height / 2);

		currentTimeText.setString(GetTimeStr(sess->totalFramesBeforeGoal));
		currentTimeText.setOrigin(0, currentTimeText.getLocalBounds().top + currentTimeText.getLocalBounds().height / 2);

		skillPointsText.setString(to_string(rm->storePoints));
		skillPointsText.setOrigin(skillPointsText.getLocalBounds().left + skillPointsText.getLocalBounds().width / 2,
			skillPointsText.getLocalBounds().top + skillPointsText.getLocalBounds().height / 2);

		currencyText.setString("50");
		currencyText.setOrigin(currencyText.getLocalBounds().left + currencyText.getLocalBounds().width / 2,
			currencyText.getLocalBounds().top + currencyText.getLocalBounds().height / 2);
	}
}

void RushScorePanel::Enter()
{
	assert(action == A_HIDE);
	action = A_ENTER;
	frame = 0;
	UpdateButtonIconsWhenControllerIsChanged();
}

void RushScorePanel::Leave()
{
	assert(action == A_SHOW);
	action = A_LEAVE;
	frame = 0;
}

bool RushScorePanel::IsHidden()
{
	return action == A_HIDE;
}

bool RushScorePanel::IsShowing()
{
	return action == A_SHOW;
}

void RushScorePanel::UpdateButtonIconsWhenControllerIsChanged()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	int cType = Session::GetSession()->controllerStates[0]->GetControllerType();

	auto button = XBoxButton::XBOX_A;
	SetRectSubRect(buttonQuads, mainMenu->GetButtonIconTileForMenu(cType, button));

	button = XBoxButton::XBOX_X;
	SetRectSubRect(buttonQuads + 4, mainMenu->GetButtonIconTileForMenu(cType, button));
}

void RushScorePanel::SetTopLeft(Vector2f topLeft)
{
	float width = ts_panel->tileWidth;
	float height = ts_panel->tileHeight;

	SetRectTopLeft(panelQuad, width, height, topLeft);

	levelNumberText.setPosition(topLeft + levelNumberOffset);
	levelNameText.setPosition(topLeft + levelNameOffset);
	bestTimeText.setPosition(topLeft + bestTimeOffset);
	//bestTimeText.setOrigin(0, bestTimeText.getLocalBounds().top);
	currentTimeText.setPosition(topLeft + currentTimeOffset);
	skillPointsText.setPosition(topLeft + skillPointsOffset);
	currencyText.setPosition(topLeft + currencyOffset);
	nextText.setPosition(topLeft + nextOffset);
	retryText.setPosition(topLeft + retryOffset);
	SetRectTopLeft(buttonQuads, buttonSize, buttonSize, topLeft + continueButtonOffset);
	SetRectTopLeft(buttonQuads + 4, buttonSize, buttonSize, topLeft + retryButtonOffset);
}

void RushScorePanel::Draw(sf::RenderTarget *target)
{
	if (action != A_HIDE)
	{
		target->draw(panelQuad, 4, sf::Quads, ts_panel->texture);

		target->draw(levelNumberText);
		target->draw(levelNameText);
		target->draw(bestTimeText);
		target->draw(currentTimeText);
		target->draw(skillPointsText);
		target->draw(currencyText);
		target->draw(nextText);
		target->draw(retryText);
		
		target->draw(buttonQuads, 4 * 2, sf::Quads, Session::GetSession()->GetButtonIconTileset(0)->texture);
	}
}


RushScoreDisplay::RushScoreDisplay(RushManager *p_rushManager, sf::Font &f)
	:ScoreDisplay(p_rushManager, f)
{
	rushManager = p_rushManager;

	CreateDescriptionTable();

	upgradePop = new RushUpgradePopup(rushManager);

	kinStore = rushManager->kinStore;

	scorePanel = new RushScorePanel(rushManager);

	backpackCounter = new BackpackCounter(rushManager);

	medal = NULL;

	Reset();

	//ts_test = rushManager->GetSizedTileset("Menu/AdventureScoreDisplay/adventurescoretest_1920x1080.png");
	//ts_test->SetSpriteTexture(testSpr);

	testSpr.setPosition(0, 0);

	backpackCounter->SetCenter(Vector2f(1600, 540));
}

RushScoreDisplay::~RushScoreDisplay()
{
	delete upgradePop;
	delete scorePanel;
	delete backpackCounter;
}

void RushScoreDisplay::Reset()
{
	ScoreDisplay::Reset();
	action = A_IDLE;
	frame = 0;

	scorePanel->Reset();

	upgradePop->SetCenter(Vector2f(960, 800));
	//upgradePop->SetUpgrade(2);
	upgradePop->Reset();

	backpackCounter->Reset();
}

void RushScoreDisplay::OpenStore()
{
	//action = A_SHOW;
	kinStore->sess = Session::GetSession();
	kinStore->Open();
	//action = A_STORE;//A_WAIT; //SHOW is for effects and transitions and stuff
	//frame = 0;
	//upgradePop->SetToMostRecentUpgrade();
}

void RushScoreDisplay::ParticleDestroyed()
{
	backpackCounter->AddParticle();
}

void RushScoreDisplay::Activate()
{
	action = A_PANEL_ENTER;
	frame = 0;

	Session *sess = Session::GetSession();
	if (sess->goal != NULL)
	{
		medal = sess->goal->medal;
		medal->particles->reactor = this;
	}
	else
	{
		medal = NULL;
	}

	scorePanel->SetInfo(sess);

	scorePanel->Enter();


	//action = A_EXP;
	//expBar->AddMedal(medalRank);
	//frame = 0;
}

void RushScoreDisplay::Confirm()
{
	action = A_PANEL_HIDING;
	frame = 0;
	scorePanel->Leave();
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

	scorePanel->Update();

	switch (action)
	{
	case A_PANEL_ENTER:
		if (scorePanel->IsShowing())
		{
			action = A_PANEL_SHOW;
			frame = 0;
		}
		break;
	case A_PANEL_HIDING:
		if (scorePanel->IsHidden())
		{
			action = A_ENERGY_TRANSFER;
			frame = 0;
			medal->Disperse();
		}
		break;
	case A_ENERGY_TRANSFER:
		if (medal->IsDone())
		{
			action = A_WAIT_POST_ENERGY;
			frame = 0;
		}
		break;
	case A_WAIT_POST_ENERGY:
		if (frame == 30)
		{
			Deactivate();
		}
		break;

		/*A_PANEL_ENTER,
			A_PANEL_SHOW,
			A_PANEL_HIDING,
			A_ENERGY_TRANSFER,*/
	//case A_EXP:
	//{
	//	if (expBar->action == KinExperienceBar::A_IDLE)
	//	{
	//		rushManager->storePoints += expBar->gainedLevels;
	//		OpenStore();
	//		return;
	//	}
	//	/*else if (expBar->action == KinExperienceBar::A_LEVEL_UP)
	//	{
	//		rushManager->storePoints += 1;
	//		OpenStore();
	//		return;
	//	}*/

	//	expBar->Update();
	//	break;
	//}
	//case A_STORE:
	//{
	//	if (kinStore->IsReadyToClose())
	//	{
	//		action = A_WAIT;
	//		frame = 0;
	//		return;
	//	}

	//	kinStore->Update();
	//	break;
	//}
	}
	
	backpackCounter->Update();
	
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
	return action == A_PANEL_SHOW;
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
	backpackCounter->SetSession(sess);
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
		/*if (action == A_EXP)
		{
			rushManager->expBar->Draw(target);
		}
		else if (action == A_WAIT || action == A_STORE)
		{
			kinStore->Draw(target);
		}*/

		if (action == A_PANEL_HIDING || action == A_ENERGY_TRANSFER || action == A_WAIT_POST_ENERGY)
		{
			backpackCounter->Draw(target);
		}

		//medal->Draw(target);

		scorePanel->Draw(target);
	}
}