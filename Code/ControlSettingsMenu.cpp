#include "ControlSettingsMenu.h"
#include "MainMenu.h"
#include "Tileset.h"
#include "VectorMath.h"
#include <SFML/Graphics.hpp>
#include "ControlProfile.h"
#include "SaveFile.h"
#include "AdventureManager.h"

using namespace std;
using namespace sf;

ControlSettingsMenu::ControlSettingsMenu( MainMenu *p_mm, TilesetManager *tm)
	:mainMenu( p_mm )
{
	pSel = new ProfileSelector;

	ts_xboxButtons = tm->GetSizedTileset("Menu/xbox_button_icons_128x128.png");
	ts_actionIcons = tm->GetSizedTileset("Menu/power_icon_128x128.png");

	numActions = 6;

	actionQuads = new Vertex[numActions * 4];
	buttonQuads = new Vertex[numActions * 4];
	selectQuads = new Vertex[numActions * 4];
	labelQuads = new Vertex[numActions * 4];
	actionText = new Text[numActions];

	std::string buttonTexts[6] = { "JUMP", "DASH", "ATTACK", "SHIELD/POWER","POWER 6 LEFT", "POWER 6 RIGHT" };// , "MAP", "PAUSE"

	for (int i = 0; i < numActions; ++i)
	{
		actionText[i].setFont(mainMenu->arial);
		actionText[i].setCharacterSize(20);
		actionText[i].setFillColor(Color::Black);
		actionText[i].setString(buttonTexts[i]);
		actionText[i].setOrigin(actionText[i].getLocalBounds().width / 2, actionText[i].getLocalBounds().height);
	}

	ts_currentButtons = NULL;

	SetActionTile(0, 0);
	SetActionTile(1, 2);
	SetActionTile(2, 1);
	SetActionTile(3, 11);
	SetActionTile(4, 12);
	SetActionTile(5, 13);
	//SetActionTile(6, 14);
	//SetActionTile(7, 14);


	//SetActionTile(8, 7);
	//SetActionTile(9, 7);

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 4, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 2, 0);

	Vector2f originPos = Vector2f(100, 500);
	Vector2f spacing(50, 90);
	Vector2f actionSize(ts_actionIcons->tileWidth, ts_actionIcons->tileHeight);
	Vector2f buttonSize(ts_xboxButtons->tileWidth, ts_xboxButtons->tileHeight);

	
	int borderSize = 20;
	int labelHeight = 30;

	int halfNumActions = numActions / 2;
	for (int i = 0; i < numActions; ++i)
	{
		Vector2f index(i % halfNumActions, i / halfNumActions);

		Vector2f quadCenter = originPos + Vector2f(actionSize.x / 2.f, actionSize.y / 2.f)
			+ Vector2f((actionSize.x + buttonSize.x + spacing.x) * index.x, (actionSize.y + spacing.y) * index.y);

		SetRectCenter( actionQuads + i * 4, actionSize.x, actionSize.y, quadCenter);
		SetRectCenter(selectQuads + i * 4, actionSize.x + buttonSize.x + borderSize, actionSize.y + borderSize, quadCenter + Vector2f( actionSize.x / 2, 0 ) );
		
	}

	for (int i = 0; i < numActions; ++i)
	{
		Vector2f index(i % (numActions/2), i / (numActions/2));
		Vector2f quadCenter = originPos + Vector2f(buttonSize.x / 2.f, buttonSize.y / 2.f) + Vector2f( actionSize.x, 0 )
			+ Vector2f((actionSize.x + buttonSize.x + spacing.x) * index.x, (actionSize.y + spacing.y) * index.y);
		SetRectCenter(buttonQuads + i * 4, buttonSize.x, buttonSize.y, quadCenter);
		actionText[i].setPosition(Vector2f(quadCenter.x - buttonSize.x / 2, quadCenter.y - buttonSize.y / 2 - 20));
		SetRectCenter(labelQuads + i * 4, actionSize.x + buttonSize.x + borderSize, labelHeight,
			Vector2f(quadCenter.x - buttonSize.x / 2, quadCenter.y - buttonSize.y / 2 - labelHeight / 2 - borderSize/2));
		SetRectColor(labelQuads + i * 4, Color(Color::White));
	}

	//pSel->currProfile->tempCType = CONTROLLERS.GetWindowsController(0)->GetCType();
	//UpdateXboxButtonIcons();

	UpdateSelectedQuad();

	editMode = false;

	SetGreyActionTiles(!editMode);
}

ControlSettingsMenu::~ControlSettingsMenu()
{
	delete pSel;

	delete[] actionQuads;
	delete[] buttonQuads;
	delete[] selectQuads;
	delete[] labelQuads;
	delete[] actionText;

	delete xSelector;
	delete ySelector;
}

void ControlSettingsMenu::UpdateSelectedQuad()
{
	int selIndex = xSelector->currIndex + ySelector->currIndex * (numActions/2);
	for (int i = 0; i < numActions; ++i)
	{
		if (i == selIndex)
		{
			if (currButtonState == S_SELECTED)
			{
				SetRectColor(selectQuads + i * 4, Color(Color::Red));
			}
			else
			{
				SetRectColor(selectQuads + i * 4, Color(Color::Yellow));
			}
			
		}
		else
		{
			SetRectColor(selectQuads + i * 4, Color(Color::Black) );
		}
	}
}

void ControlSettingsMenu::SetGreyActionTiles(bool greyOn)
{
	if (greyOn)
	{

		Color grey = Color(100, 100, 100);
		for (int i = 0; i < numActions; ++i)
		{
			SetRectColor(actionQuads + i * 4, grey);
			SetRectColor(buttonQuads + i * 4, grey);
		}
	}
	else
	{
		Color white = Color::White;
		for (int i = 0; i < numActions; ++i)
		{
			SetRectColor(actionQuads + i * 4, white);
			SetRectColor(buttonQuads + i * 4, white);
		}
	}
}

void ControlSettingsMenu::SetActionTile(int actionIndex, int actionType)
{
	SetRectSubRect(actionQuads + actionIndex * 4, ts_actionIcons->GetSubRect(actionType));
}



void ControlSettingsMenu::SetButtonAssoc()
{
	GameController *con = CONTROLLERS.GetWindowsController(0);//NULL;//mainMenu->GetController(0);
	//pSel->currProfile->tempCType = con->GetCType();

	//con->SetFilter(pSel->currProfile->GetCurrFilter());

	/*SaveFile *currFile = mainMenu->adventureManager->currSaveFile;
	if (currFile != NULL)
	{
		currFile->controlProfileName = pSel->currProfile->name;
		currFile->Save();
	}*/
}

ControlSettingsMenu::UpdateState ControlSettingsMenu::Update(ControllerDualStateQueue *controllerInput)
{
	UpdateState uState = NORMAL;
	int oldSel = pSel->saSelector->currIndex;
	if( !editMode )
		pSel->Update( controllerInput);

	
	//pSel->currProfile->tempCType = cType;
	if (oldSel != pSel->saSelector->currIndex)
	{
		//UpdateXboxButtonIcons();
	}

	if (!editMode && pSel->action == ProfileSelector::A_SELECTED && controllerInput->ButtonPressed_X() && pSel->saSelector->currIndex > 0)
	{
		editMode = true;
		SetGreyActionTiles(!editMode);
	}
	else if ( editMode && currButtonState != S_SELECTED && (controllerInput->ButtonPressed_X() || controllerInput->ButtonPressed_B()))
	{
		editMode = false;
		SetGreyActionTiles(!editMode);
		currButtonState = S_NEUTRAL;
		uState = CONFIRM;
	}
	


	if (currButtonState == S_SELECTED && editMode)
	{
		XBoxButton b = XBOX_A;//CheckXBoxInput(currInput);
		if (b != XBoxButton::XBOX_BLANK)
		{
			int ind = xSelector->currIndex + ySelector->currIndex * 4;
			XBoxButton old = pSel->tempFilter[ind];
			for (int i = 0; i < numActions; ++i)
			{
				if (pSel->tempFilter[i] == b)
				{
					pSel->tempFilter[i] = old;
					break;
				}
			}
			pSel->tempFilter[ind] = b;

			pSel->SaveCurrConfig();
			currButtonState = S_NEUTRAL;
			UpdateSelectedQuad();
		}
	}
	else if (editMode && currButtonState != S_SELECTED)
	{
		int xchanged;
		int ychanged;


		xchanged = xSelector->UpdateIndex(controllerInput->GetCurrState().LLeft(), controllerInput->GetCurrState().LRight());
		ychanged = ySelector->UpdateIndex(controllerInput->GetCurrState().LUp(), controllerInput->GetCurrState().LDown());


		if (xchanged != 0 || ychanged != 0)
		{
			currButtonState = S_NEUTRAL;
			UpdateSelectedQuad();
		}

		bool A = controllerInput->GetCurrState().A;
		switch (currButtonState)
		{
		/*case S_WAITING:
			if (CheckXBoxInput(currInput) == XBOX_BLANK)
			{
				currButtonState = S_NEUTRAL;
				break;
			}
			break;*/
		case S_NEUTRAL:
			if (controllerInput->ButtonPressed_A())
			{
				currButtonState = S_PRESSED;
			}
			break;
		case S_PRESSED:
			if (!A)
			{
				currButtonState = S_SELECTED;
				UpdateSelectedQuad();
				//pSel->currProfile->tempCType = cType;

				//XBoxButton *fil = pSel->currProfile->GetCurrFilter();

				/*for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
				{
					pSel->oldFilter[i] = fil[i];
					pSel->tempFilter[i] = fil[i];
				}

				if (cType == CTYPE_GAMECUBE)
				{
					SetFilterDefaultGCC(fil);	
				}
				else
				{
					SetFilterDefault(fil);
				}*/

				//mainMenu->GetController(0)->SetFilter(fil);
				
				
				return CONFIRM;
			}
			break;
		case S_SELECTED:
			if (A)
			{
				currButtonState = S_UNPRESSED;
			}
			break;
		case S_UNPRESSED:
			if (!A)
			{
				currButtonState = S_NEUTRAL;
			}
			break;
		}

		
	}

	
		//state = S_EDIT_CONFIG;
		
		//xboxInputAssoc[useControllerSchemeIndex][ControllerSettings::JUMP] = b;

		//UpdateXboxButtonIcons(useControllerSchemeIndex);
	return uState;

}

void ControlSettingsMenu::Draw(sf::RenderTarget *target )
{
	if( editMode )
		target->draw(selectQuads, numActions * 4, sf::Quads);

	target->draw(labelQuads, numActions * 4, sf::Quads);
	target->draw(actionQuads, numActions * 4, sf::Quads, ts_actionIcons->texture);
	target->draw(buttonQuads, numActions * 4, sf::Quads, ts_currentButtons->texture);
	for (int i = 0; i < numActions; ++i)
	{
		target->draw(actionText[i]);
	}

	pSel->Draw(target);
}

XBoxButton ControlSettingsMenu::GetFilteredButton( ControllerType cType,
	ControllerSettings::ButtonType b )
{	
	/*switch (cType)
	{
	case CTYPE_XBOX:
		return pSel->currProfile->filter[b];
		break;
	case CTYPE_GAMECUBE:
		return pSel->currProfile->gccFilter[b];
		break;
	case CTYPE_PS4:
		return pSel->currProfile->filter[b];
		break;
	case CTYPE_NONE:
		assert(0);
		break;
	}*/

	return pSel->currProfile->filter[b];
}

XBoxButton ControlSettingsMenu::CheckXBoxInput(ControllerState &currInput)
{
	bool leftMovement = true;
	//bool leftMovement = (inputSelectors[2]->GetString() == "left analog");
	if (currInput.A)
	{
		return XBOX_A;
	}
	else if (currInput.B)
	{
		return XBOX_B;
	}
	else if (currInput.X)
	{
		return XBOX_X;
	}
	else if (currInput.Y)
	{
		return XBOX_Y;
	}
	else if (currInput.leftShoulder)
	{
		return XBOX_L1;
	}
	else if (currInput.rightShoulder)
	{
		return XBOX_R1;
	}
	else if (currInput.LeftTriggerPressed())
	{
		return XBOX_L2;
	}
	else if (currInput.RightTriggerPressed())
	{
		return XBOX_R2;
	}
	else if (currInput.PLeft())
	{
		return XBOX_PLEFT;
	}
	else if (currInput.PUp())
	{
		return XBOX_PUP;
	}
	else if (currInput.PRight())
	{
		return XBOX_PRIGHT;
	}
	else if (currInput.PDown())
	{
		return XBOX_PDOWN;
	}
	else if (leftMovement && currInput.RLeft())
	{
		return XBOX_RLEFT;
	}
	else if (leftMovement && currInput.RUp())
	{
		return XBOX_RUP;
	}
	else if (leftMovement && currInput.RRight())
	{
		return XBOX_RRIGHT;
	}
	else if (leftMovement && currInput.RDown())
	{
		return XBOX_RDOWN;
	}
	else if (!leftMovement && currInput.LLeft())
	{
		return XBOX_LLEFT;
	}
	else if (!leftMovement && currInput.LUp())
	{
		return XBOX_LUP;
	}
	else if (!leftMovement && currInput.LRight())
	{
		return XBOX_LRIGHT;
	}
	else if (!leftMovement && currInput.LDown())
	{
		return XBOX_LDOWN;
	}
	else if (currInput.back)
	{
		return XBOX_BLANK;//XBOX_BACK;
	}
	else if (currInput.start)
	{
		return XBOX_BLANK;//XBOX_START;
	}
	else
	{
		return XBOX_BLANK;
	}
}

void ControlSettingsMenu::UpdateControlIcons()
{
	Color unselectedColor = Color::Blue;
	Color selectedColor = Color::Magenta;
}

bool ControlSettingsMenu::IsEditingButtons()
{
	return editMode;
}

void ControlSettingsMenu::InitAssocSymbols()
{
	int symbolX = 288;
	int symbolY = 100;
	int textX = 288;
	int textY = 70;
	int extraX;
	int extraY;
	int symbolSize = 128;//140;//128;
	int spacing = 32;//16;
	int buttonIconSpacing = 150;

	string bounceSpecial = "bounce";
	string grindSpecial = "grind";
	string timeslowSpecial = "time slow";
	string wireLeftSpecial = "left wire";
	string wireRightSpecial = "right wire";
	string toggleBounce = "toggle bounce";
	string toggleGrind = "toggle grind";
	string toggleTimeSlow = "toggle time slow";
	//these will be turned off when you dont have the powers
	//string possibleActions[ControllerSettings::Count] = { "jump", "dash", "attack", bounceSpecial,
	//	grindSpecial, timeslowSpecial, wireLeftSpecial, wireRightSpecial,
	//	"map", "pause" };
	int count = ControllerSettings::BUTTONTYPE_Count;
	for (int i = 0; i < count; ++i)
	{
		if (i < count / 2)
		{
			extraX = 0;
			extraY = 0;
		}
		else
		{
			extraX = 500;
			extraY = -(symbolSize + spacing) * count / 2;
		}

		//assocSymbols[i * 4 + 0].position = basePos + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY);
		//assocSymbols[i * 4 + 1].position = basePos + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY);
		//assocSymbols[i * 4 + 2].position = basePos + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);
		//assocSymbols[i * 4 + 3].position = basePos + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);

		//IntRect sub = ts_actionIcons->GetSubRect(min(i, 8));

		//assocSymbols[i * 4 + 0].texCoords = Vector2f(sub.left, sub.top);
		//assocSymbols[i * 4 + 1].texCoords = Vector2f(sub.left + sub.width, sub.top);
		//assocSymbols[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
		//assocSymbols[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);

		//buttonVA[i * 4 + 0].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY);
		//buttonVA[i * 4 + 1].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY);
		//buttonVA[i * 4 + 2].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);
		//buttonVA[i * 4 + 3].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);

		//actionText[i].setFont(mainMenu->arial);
		//actionText[i].setCharacterSize(24);
		//actionText[i].setPosition(basePos + Vector2f(extraX + textX, (symbolSize + spacing) * i + extraY + textY));//textX, 50 * i );
		//actionText[i].setFillColor(Color::White);
		//actionText[i].setString(possibleActions[i]);

	}
}