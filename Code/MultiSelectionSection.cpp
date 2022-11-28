#include "MainMenu.h"
#include "ControlProfile.h"
#include "PowerOrbs.h"

using namespace sf;
using namespace std;

MultiSelectionSection::MultiSelectionSection(MainMenu *p_mainMenu, MapSelectionMenu *p_parent,
	int p_playerIndex, Vector2f &p_topMid)
	:mainMenu(p_mainMenu), parent(p_parent), playerIndex(p_playerIndex), team(T_NOT_CHOSEN),
	skinIndex(S_STANDARD), active(false), topMid(p_topMid)
{
	profileSelect = new ControlProfileMenu(this, playerIndex,
		mainMenu->cpm->profiles, p_topMid);
	bHoldThresh = 30;


	if (parent == NULL)
	{
		active = true;
	}
	holdingB = false;
	FillRingSection *blah[] = { new FillRingSection(mainMenu->tilesetManager, Color::Red, sf::Color::Black,
		sf::Color::Blue,
		3, 40, 0) };

	backLoaderOffset = Vector2f(0, 50);
	backLoader = new FillRing(topMid + backLoaderOffset, 1, blah);
	backLoader->ResetEmpty();

	Tileset *ts_bg = mainMenu->tilesetManager.GetTileset("Menu/multiprofile.png", 480, 400);
	bgSprite.setTexture(*ts_bg->texture);
	bgSprite.setTextureRect(ts_bg->GetSubRect(p_playerIndex));
	bgSprite.setOrigin(bgSprite.getLocalBounds().width / 2, 0);
	bgSprite.setPosition(topMid);

	offRect.setSize(Vector2f(ts_bg->tileWidth, ts_bg->tileHeight));
	offRect.setFillColor(Color(0, 0, 0, 180));
	offRect.setOrigin(offRect.getLocalBounds().width / 2, 0);
	offRect.setPosition(topMid);
	//profileSelect->UpdateNames();	
}

MultiSelectionSection::~MultiSelectionSection()
{
	delete profileSelect;
	delete backLoader;
}

bool MultiSelectionSection::IsReady()
{
	if (profileSelect->state == ControlProfileMenu::S_SELECTED)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MultiSelectionSection::SetTopMid(sf::Vector2f &tm)
{
	topMid = tm;
	backLoader->SetPosition(topMid + backLoaderOffset);
	profileSelect->SetTopMid(topMid);
	bgSprite.setPosition(topMid);
	offRect.setPosition(topMid);
}

bool MultiSelectionSection::ButtonEvent(UIEvent eType,
	ButtonEventParams *param)
{
	return true;
}

void MultiSelectionSection::Update()
{
	MainMenu *mm = mainMenu;
	//ControllerState &currInput = mm->GetCurrInput(playerIndex);
	//ControllerState &prevInput = mm->GetPrevInput(playerIndex);

	//if (!active)
	//{
	//	bool a = currInput.A && !prevInput.A;
	//	if (a)
	//	{
	//		active = true;
	//		bHoldFrames = 0;
	//	}
	//	return;
	//}
	//else
	//{
	//	if (profileSelect->state == ControlProfileMenu::S_SELECTED)
	//	{
	//		if (currInput.B)
	//		{
	//			holdingB = true;
	//		}
	//		else
	//		{
	//			holdingB = false;
	//			bHoldFrames = 0;
	//		}

	//		if (holdingB)
	//		{
	//			if (backLoader->Fill(1) > 0)
	//			{

	//			}
	//			backLoader->Update();
	//		}
	//		else
	//		{
	//			backLoader->Drain(1);
	//			backLoader->Update();
	//		}
	//	}
	//	else
	//	{
	//		holdingB = false;
	//	}
	//}

	////if (!IsReady())
	//{
	//	bool rT = (currInput.RightTriggerPressed() && !prevInput.RightTriggerPressed());
	//	bool lT = (currInput.LeftTriggerPressed() && !prevInput.LeftTriggerPressed());

	//	bool rS = (currInput.rightShoulder && !prevInput.rightShoulder);
	//	bool lS = (currInput.leftShoulder && !prevInput.leftShoulder);
	//	if (rT)
	//	{
	//		switch (team)
	//		{
	//		case T_NOT_CHOSEN:
	//		{
	//			team = T_RED;
	//			break;
	//		}
	//		case T_RED:
	//		{
	//			team = T_BLUE;
	//			break;
	//		}
	//		case T_BLUE:
	//		{
	//			team = T_RED;
	//			break;
	//		}
	//		}
	//	}
	//	else if (lT)
	//	{
	//		switch (team)
	//		{
	//		case T_NOT_CHOSEN:
	//		{
	//			team = T_BLUE;
	//			break;
	//		}
	//		case T_RED:
	//		{
	//			team = T_BLUE;
	//			break;
	//		}
	//		case T_BLUE:
	//		{
	//			team = T_RED;
	//			break;
	//		}
	//		}
	//	}

	//	if (rS)
	//	{
	//		if (skinIndex < S_Count)
	//		{
	//			++skinIndex;
	//		}
	//		else
	//		{
	//			skinIndex = 0;
	//		}
	//	}
	//	else if (lS)
	//	{
	//		if (skinIndex > 0)
	//		{
	//			--skinIndex;
	//		}
	//		else
	//		{
	//			skinIndex = S_Count - 1;
	//		}
	//	}

	//	ControlProfile *oldProf = profileSelect->currProfile;
	//	//profileSelect->tempCType = mm->GetController(playerIndex)->GetCType();
	//	profileSelect->Update(currInput, prevInput);

	//	/*if (profileSelect->currProfile != oldProf)
	//	{
	//	selectedProfileText.setString(profileSelect->currProfile->name);
	//	selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().width / 2, 0);
	//	}*/
	//}

	int numPlayersActive = 0;
	if (parent != NULL)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (parent->multiPlayerSection[i]->active)
			{
				++numPlayersActive;
			}
		}
		//numPlayersActive  = parent->GetNumActivePlayers();
	}
	else
	{
		numPlayersActive = 1;
	}
}

void MultiSelectionSection::Draw(sf::RenderTarget *target)
{

	//if (parent != NULL)
	/*{
	Color c;
	switch (playerIndex)
	{
	case 0:
	c = Color::Red;
	break;
	case 1:
	c = Color::Yellow;
	break;
	case 2:
	c = Color::White;
	break;
	case 3:
	c = Color::Magenta;
	break;
	}

	sf::RectangleShape rs;
	rs.setFillColor(c);
	rs.setSize(Vector2f(1920 / 4, 400));
	rs.setOrigin(rs.getLocalBounds().width / 2, 0);
	rs.setPosition(profileSelect->topMid);
	target->draw(rs);
	}*/


	target->draw(bgSprite);
	target->draw(playerSprite);
	profileSelect->Draw(target);

	if (holdingB)
	{
		backLoader->Draw(target);
	}

	if (!active)
		target->draw(offRect);
}

bool MultiSelectionSection::ShouldGoBack()
{
	if (backLoader->IsFull())
	{
		backLoader->ResetEmpty();
		return true;
	}

	return false;
}