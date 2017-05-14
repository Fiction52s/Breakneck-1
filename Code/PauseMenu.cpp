#include "PauseMenu.h"
#include <sstream>
#include "GameSession.h"
#include "MainMenu.h"
#include <iostream>
#include "SaveFile.h"
#include <fstream>

using namespace sf;
using namespace std;


OptionSelector::OptionSelector( Vector2f &p_pos, int p_optionCount,
		std::string *p_options )
		:optionCount( p_optionCount ), options( NULL ), pos( p_pos ), accelBez( 0, 0, 1, 1 )
{
	selected = false;
	currentText.setFont( MainMenu::arial );
	currentText.setCharacterSize( 80 );
	currentText.setColor( Color::White );
	options = new string[optionCount];
	for( int i = 0; i < optionCount; ++i )
	{
		options[i] = p_options[i];
	}
	currentIndex = 0;

	currentText.setString( options[currentIndex] );
	currentText.setPosition( pos );

	maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	framesWaiting = maxWaitFrames;

	momentum = 0;

	maxMomentum = 4;

	//cout << "end selector init" << endl;
}

void OptionSelector::Right()
{
	if( framesWaiting >= currWaitFrames || momentum <= 0 )
	{
		currentIndex++;
		if( currentIndex == optionCount )
			currentIndex = 0;

		currentText.setString( options[currentIndex] );

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );

		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
		//cout << "switch right" << endl;
	}
}


void OptionSelector::Left()
{
	if( framesWaiting >= currWaitFrames || momentum >= 0 )
	{
		currentIndex--;
		if( currentIndex < 0 )
			currentIndex = optionCount - 1;

		currentText.setString( options[currentIndex] );

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );

		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
}

void OptionSelector::Stop()
{
	framesWaiting = maxWaitFrames;
	momentum = 0;
	//currWaitFrames = maxWaitFrames;
}

void OptionSelector::Update()
{
	framesWaiting++;
}

const std::string & OptionSelector::GetString()
{
	return options[currentIndex];
}

void OptionSelector::Draw( sf::RenderTarget *target )
{
	if( selected )
		currentText.setColor( Color::Red );
	else
		currentText.setColor( Color::White );
	target->draw( currentText );
}

OptionsMenu::OptionsMenu( MainMenu *mainMenu )
{
}

OptionsMenu::OptionsMenu( PauseMenu *pauseMenu )
	:assocSymbols( sf::Quads, ControllerSettings::ButtonType::Count * 4 ),
	buttonVA( sf::Quads, ControllerSettings::ButtonType::Count * 4 ),
	controlIconVA( sf::Quads, ControllerTypes::Count * 4 ),
	schemeKeyboardVA( sf::Quads, 3 * 4 ), 
	schemeControllerVA( sf::Quads, 3 * 4 )
{
	LoadControlOptions();
	mainMenu = pauseMenu->owner->mainMenu;
	mode = LEFTBAR;
	//temporary init here
	autoUseController = true;
	controllerIconsIndex = 0;
	leftBarCurr = L_AUTOCONTROLLER;
	controllerIconsIndex = 0;
	useIconsIndex = 0;
	keyboardSchemeIndex = 0;
	controllerSchemeIndex = 0;
	useKeyboardSchemeIndex = 0;
	useControllerSchemeIndex = 0;
	basePos = Vector2f( 100, 100 );

	UpdateControlIcons();
	UpdateSchemeVA( true );
	UpdateSchemeVA( false );

	ts_xboxButtons = pauseMenu->owner->GetTileset( "Menu/xbox_button_icons_128x128.png", 128, 128 );
	ts_actionIcons = pauseMenu->owner->GetTileset( "Menu/power_icon_128x128.png", 128, 128 );

	ts_currentButtons = ts_xboxButtons;

	string inputTypes[] = { "Xbox", "Keyboard", "PS4", "Gamecube" };
	

	string settings[] = {"Setting 1", "Setting 2", "Setting 3" };

	string thumbSticks[] = { "left analog", "right analog" };

	//inputSelectors = new OptionSelector*[2];

	//inputSelectors[0] = new OptionSelector( basePos + Vector2f( 0, 0 ), 4, inputTypes );
	//inputSelectors[1] = new OptionSelector( basePos + Vector2f( 0, 180 ), 3, settings );


	//inputSelectors[2] = new OptionSelector( Vector2f( 300, 180 ), 2, thumbSticks );

	string bounceSpecial = "bounce";
	string grindSpecial = "grind";
	string timeslowSpecial = "time slow";
	string wireLeftSpecial = "left wire";
	string wireRightSpecial = "right wire";
	string toggleBounce = "toggle bounce";
	string toggleGrind = "toggle grind";
	string toggleTimeSlow = "toggle time slow";


	possibleControllerActions = new string[12];
	possibleControllerActions[0] = "move";
	possibleControllerActions[1] = "jump";
	possibleControllerActions[2] = "dash";
	possibleControllerActions[3] = "attack";
	possibleControllerActions[4] = bounceSpecial;
	possibleControllerActions[5] = grindSpecial;
	possibleControllerActions[6] = timeslowSpecial;
	possibleControllerActions[7] = wireLeftSpecial;
	possibleControllerActions[8] = wireRightSpecial;
	possibleControllerActions[9] = toggleBounce;
	possibleControllerActions[10] = toggleGrind;
	possibleControllerActions[11] = toggleTimeSlow;

	possibleKeyboardActions = new string[15];
	possibleKeyboardActions[0] = "up";
	possibleKeyboardActions[1] = "left";
	possibleKeyboardActions[2] = "down";
	possibleKeyboardActions[3] = "right";
	possibleKeyboardActions[4] = "jump";
	possibleKeyboardActions[5] = "dash";
	possibleKeyboardActions[6] = "attack";
	possibleKeyboardActions[7] = bounceSpecial;
	possibleKeyboardActions[8] = grindSpecial;
	possibleKeyboardActions[9] = timeslowSpecial;
	possibleKeyboardActions[10] = wireLeftSpecial;
	possibleKeyboardActions[11] = wireRightSpecial;
	possibleKeyboardActions[12] = toggleBounce;
	possibleKeyboardActions[13] = toggleGrind;
	possibleKeyboardActions[14] = toggleTimeSlow;
	//SetAssocSymbols();

	optionSelectorIndex = 0;
	maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	momentum = 0;
	maxMomentum = 4;

	currentSelectors = NULL;

	//testing
	currentSelectors = inputSelectors;
	int controlTypeSize = 64;
	Vector2f controlTypeBasePos( 100, 100 );
	for( int i = 0; i < ControllerTypes::Count; ++i )
	{
		controlIconVA[i*4+0].position = controlTypeBasePos + Vector2f( controlTypeSize * i, 0 );
		controlIconVA[i*4+1].position = controlTypeBasePos + Vector2f( controlTypeSize * (i+1), 0 )
			+ Vector2f( 0, 0 );
		controlIconVA[i*4+2].position = controlTypeBasePos + Vector2f( controlTypeSize * (i+1), 0 )
			+ Vector2f( 0, controlTypeSize );
		controlIconVA[i*4+3].position = controlTypeBasePos + Vector2f( controlTypeSize * i, 0 )
			+ Vector2f( 0, controlTypeSize );
	}

	Vector2f schemeBasePos = Vector2f( 100, 200 );
	for( int i = 0; i < 3; ++i )
	{
		schemeKeyboardVA[i*4+0].position = schemeBasePos + Vector2f( controlTypeSize * i, 0 );
		schemeKeyboardVA[i*4+1].position = schemeBasePos + Vector2f( controlTypeSize * (i+1), 0 )
			+ Vector2f( 0, 0 );
		schemeKeyboardVA[i*4+2].position = schemeBasePos + Vector2f( controlTypeSize * (i+1), 0 )
			+ Vector2f( 0, controlTypeSize );
		schemeKeyboardVA[i*4+3].position = schemeBasePos + Vector2f( controlTypeSize * i, 0 )
			+ Vector2f( 0, controlTypeSize );
	}

	Vector2f schemeBasePos1 = Vector2f( 100, 300 );
	for( int i = 0; i < 3; ++i )
	{
		schemeControllerVA[i*4+0].position = schemeBasePos1 + Vector2f( controlTypeSize * i, 0 );
		schemeControllerVA[i*4+1].position = schemeBasePos1 + Vector2f( controlTypeSize * (i+1), 0 )
			+ Vector2f( 0, 0 );
		schemeControllerVA[i*4+2].position = schemeBasePos1 + Vector2f( controlTypeSize * (i+1), 0 )
			+ Vector2f( 0, controlTypeSize );
		schemeControllerVA[i*4+3].position = schemeBasePos1 + Vector2f( controlTypeSize * i, 0 )
			+ Vector2f( 0, controlTypeSize );
	}

	InitAssocSymbols();

	moveDown = false;
	moveUp = false;
	moveLeft = false;
	moveRight = false;
	
	
}

void OptionsMenu::UpdateControlIcons()
{
	Color unselectedColor = Color::Blue;
	Color selectedColor = Color::Magenta;
	
	//Color test = Color::Yellow;
	//only supposed to use the color change for when you're currently

	for( int i = 0; i < ControllerTypes::Count; ++i )
	{
		//if( i == useIconsIndex )
		if( i == controllerIconsIndex )
		{
			controlIconVA[i*4+0].color = selectedColor;
			controlIconVA[i*4+1].color = selectedColor;
			controlIconVA[i*4+2].color = selectedColor;
			controlIconVA[i*4+3].color = selectedColor;
		}
		else
		{
			controlIconVA[i*4+0].color = unselectedColor;
			controlIconVA[i*4+1].color = unselectedColor;
			controlIconVA[i*4+2].color = unselectedColor;
			controlIconVA[i*4+3].color = unselectedColor;
		}
	}	

	/*int i = ControllerTypes::Count - 1;
	controlIconVA[i*4+0].color = test;
	controlIconVA[i*4+1].color = test;
	controlIconVA[i*4+2].color = test;
	controlIconVA[i*4+3].color = test;*/
}

void OptionsMenu::InitAssocSymbols()
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
	string possibleActions[ControllerSettings::Count] = { "jump", "dash", "attack", bounceSpecial,
		grindSpecial, timeslowSpecial, wireLeftSpecial, wireRightSpecial,
		"map", "pause" };
	int count = ControllerSettings::Count;
	for( int i = 0; i < count; ++i )
	{			
		if( i < count / 2 )
		{
			extraX = 0;
			extraY = 0;
		}
		else
		{
			extraX = 500;
			extraY = -(symbolSize + spacing) * count / 2;
		}
			
		assocSymbols[i*4+0].position = basePos + Vector2f( extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY );
		assocSymbols[i*4+1].position = basePos + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY );
		assocSymbols[i*4+2].position = basePos + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );
		assocSymbols[i*4+3].position = basePos + Vector2f( extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );

		IntRect sub = ts_actionIcons->GetSubRect( min( i, 8 ) );

		assocSymbols[i*4+0].texCoords = Vector2f( sub.left, sub.top );
		assocSymbols[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
		assocSymbols[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
		assocSymbols[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );
				
		buttonVA[i*4+0].position = basePos + Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY );
		buttonVA[i*4+1].position = basePos + Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY );
		buttonVA[i*4+2].position = basePos + Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );
		buttonVA[i*4+3].position = basePos + Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );

		actionText[i].setFont( mainMenu->arial );
		actionText[i].setCharacterSize( 24 );
		actionText[i].setPosition( basePos + Vector2f( extraX + textX, (symbolSize + spacing) * i + extraY + textY ) );//textX, 50 * i );
		actionText[i].setColor( Color::White );
		actionText[i].setString( possibleActions[i] );

	}
	/*for( int i = count; i < count + 3; ++i )
	{
		assocSymbols[i*4+0].position = Vector2f( 0, 0 );
		assocSymbols[i*4+1].position = Vector2f( 0, 0 );
		assocSymbols[i*4+2].position = Vector2f( 0, 0 );
		assocSymbols[i*4+3].position = Vector2f( 0, 0 );
	}*/

	/*for( int i = 0; i < numAssocSymbols; ++i )
	{
			
	}*/
}

void OptionsMenu::UpdateSchemeVA( bool kb )
{
	///kb is keyboard
	VertexArray *temp = &schemeKeyboardVA;
	int sel = useKeyboardSchemeIndex;
	sel = keyboardSchemeIndex;

	if( !kb )
	{
		temp = &schemeControllerVA;
		sel = useControllerSchemeIndex;
		sel = controllerSchemeIndex;
	}

	

	VertexArray &va = *temp;
	Color unselectedColor = Color::Blue;
	Color selectedColor = Color::Magenta;
	Color currentColor = Color::Yellow;
	for( int i = 0; i < 3; ++i )
	{
		
		if( i == sel )
		{
			va[i*4+0].color = selectedColor;
			va[i*4+1].color = selectedColor;
			va[i*4+2].color = selectedColor;
			va[i*4+3].color = selectedColor;
		}
		else
		{
			va[i*4+0].color = unselectedColor;
			va[i*4+1].color = unselectedColor;
			va[i*4+2].color = unselectedColor;
			va[i*4+3].color = unselectedColor;
		}
	}
}

bool OptionsMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	
	switch( mode )
	{
	case LEFTBAR:
		{
			if( currInput.A )
			{
				switch( leftBarCurr )
				{
				case L_AUTOCONTROLLER:
					//check/uncheck
					break;
				case L_CONTROLLER_ICONS:
					UpdateControlIcons();
					//do a selection on the different icons!
					break;
				case L_KB_OPTIONS:
					useKeyboardSchemeIndex = keyboardSchemeIndex;
					break;
				case L_CONTROLLER_OPTIONS:
					useControllerSchemeIndex = controllerSchemeIndex;
					//mainMenu->Getco.SetFilter( xboxInputAssoc[useControllerSchemeIndex] );
					break;
				}
				//mode = SELECT_SCHEME;
			}
			else if( currInput.B )
			{
				return false;
			}
			else if( currInput.X )
			{
				switch( leftBarCurr )
				{
				case L_AUTOCONTROLLER:
					//check/uncheck
					break;
				case L_CONTROLLER_ICONS:
					//UpdateControlIcons();
					//do a selection on the different icons!
					break;
				case L_KB_OPTIONS:
					mode = MODIFY_KEYBOARD_CONTROLS;
					moveDown = false;
					moveUp = false;
					moveLeft = false;
					moveRight = false;
					moveDelayCounter = 0;
					moveDelayFrames = 15;
					moveDelayFramesSmall = 6;
					modifyIndex = 0;
					//useKeyboardSchemeIndex = keyboardSchemeIndex;
					break;
				case L_CONTROLLER_OPTIONS:
					mode = MODIFY_XBOX_CONTROLS;
					moveDown = false;
					moveUp = false;
					moveLeft = false;
					moveRight = false;
					moveDelayCounter = 0;
					moveDelayFrames = 15;
					moveDelayFramesSmall = 6;
					modifyIndex = 0;
					//useControllerSchemeIndex = controllerSchemeIndex;
					break;
				}
			}
			else if( currInput.LRight() && !prevInput.LRight() )
			{
				switch( leftBarCurr )
				{
				case L_AUTOCONTROLLER:
					//nothing
					break;
				case L_CONTROLLER_ICONS:

					controllerIconsIndex++;
					if( controllerIconsIndex == ControllerTypes::Count )
					{
						controllerIconsIndex = 0;
					}

					UpdateControlIcons();
					//do a selection on the different icons!
					break;
				case L_KB_OPTIONS:
					keyboardSchemeIndex++;
					if( keyboardSchemeIndex == 3 )
					{
						keyboardSchemeIndex = 0;
					}

					UpdateSchemeVA( true );
					break;
				case L_CONTROLLER_OPTIONS:
					controllerSchemeIndex++;
					if( controllerSchemeIndex == 3 )
					{
						controllerSchemeIndex = 0;
					}

					UpdateSchemeVA( false );
					UpdateXboxButtonIcons( controllerSchemeIndex );
					break;
				}
			}
			else if( currInput.LLeft() && !prevInput.LLeft() )
			{
				switch( leftBarCurr )
				{
				case L_AUTOCONTROLLER:
					//nothing
					break;
				case L_CONTROLLER_ICONS:

					controllerIconsIndex--;
					if( controllerIconsIndex < 0 )
					{
						controllerIconsIndex = ControllerTypes::Count - 1;
					}

					UpdateControlIcons();
					//do a selection on the different icons!
					break;
				case L_KB_OPTIONS:
					keyboardSchemeIndex--;
					if( keyboardSchemeIndex < 0 )
					{
						keyboardSchemeIndex = 2;
					}

					UpdateSchemeVA( true );
					break;
				case L_CONTROLLER_OPTIONS:
					controllerSchemeIndex--;
					if( controllerSchemeIndex < 0 )
					{
						controllerSchemeIndex = 2;
					}

					UpdateSchemeVA( false );
					UpdateXboxButtonIcons( controllerSchemeIndex );
					break;
				}	
			}
			else if( currInput.LDown() && !prevInput.LDown() )
			{
				leftBarCurr++;
				if( leftBarCurr == L_Count )
				{
					leftBarCurr = L_AUTOCONTROLLER;
				}

				if( leftBarCurr == L_CONTROLLER_OPTIONS )
				{
					//controllerSchemeIndex = 0;
					UpdateXboxButtonIcons( controllerSchemeIndex );	
				}
				else if( leftBarCurr == L_KB_OPTIONS )
				{
					//UpdateKeyoardButtonIcons( controllerSchemeIndex );	
				}
				
			}
			else if( currInput.LUp() && !prevInput.LUp() )
			{
				leftBarCurr--;
				if( leftBarCurr < 0 )
				{
					leftBarCurr = L_CONTROLLER_OPTIONS;
				}
			}
			break;
		}
	case MODIFY_KEYBOARD_CONTROLS:
		{
			if( currInput.A )
			{
				XBoxButton b;
				do
				{
					b = CheckXBoxInput( currInput );
				}
				while( b != XBoxButton::XBOX_BLANK );

				xboxInputAssoc[useControllerSchemeIndex][ControllerSettings::JUMP] = b;

				UpdateXboxButtonIcons( useControllerSchemeIndex );
			}
			else if( currInput.B )
			{
			}
			else if( currInput.LLeft() && !prevInput.LLeft() )
			{
			}
			else if( currInput.LRight() && !prevInput.LRight() )
			{
			}
			else if( currInput.LUp() && !prevInput.LUp() )
			{
			}
			else if( currInput.LDown() && !prevInput.LDown() )
			{
			}
			break;
		}
	//case MODIFY_XBOX_CONTROLS:
	//	{
	//		

	//		if( !currInput.A && prevInput.A )
	//		{
	//			XBoxButton b;
	//			
	//			do
	//			{
	//				mainMenu->controller.UpdateState();
	//				
	//				ControllerState &c = mainMenu->controller.GetState();

	//				b = CheckXBoxInput( c );
	//			}
	//			while( b == XBoxButton::XBOX_BLANK );

	//			XBoxButton bb;
	//			do
	//			{
	//				mainMenu->controller.UpdateState();
	//				
	//				ControllerState &c = mainMenu->controller.GetState();

	//				bb = CheckXBoxInput( c );
	//			}
	//			while( bb != XBoxButton::XBOX_BLANK );
	//			//cout << "b: " << b << endl;
	//			SetButtonAssoc( useControllerSchemeIndex, b );
	//			//xboxInputAssoc[useControllerSchemeIndex][modifyIndex] = b;

	//			UpdateXboxButtonIcons( useControllerSchemeIndex );

	//			SaveControlOptions();

	//			mainMenu->controller.SetFilter( xboxInputAssoc[ useControllerSchemeIndex ] );
	//		}
	//		else if( currInput.B )
	//		{
	//			mode = LEFTBAR;
	//			break;
	//		}
	//		else if( currInput.LLeft() && !prevInput.LLeft() )
	//		{
	//		}
	//		else if( currInput.LRight() && !prevInput.LRight() )
	//		{
	//		}
	//		else if( currInput.LUp() && !prevInput.LUp() )
	//		{
	//		}
	//		else if( currInput.LDown() && !prevInput.LDown() )
	//		{
	//		}


	//		bool canMoveOther = ((moveDelayCounter - moveDelayFramesSmall) <= 0);
	//		bool canMoveSame = (moveDelayCounter == 0);

	//		if( (currInput.LDown() || currInput.PDown()) && ( 
	//				(!moveDown && canMoveOther) || ( moveDown && canMoveSame ) ) )
	//		{
	//			
	//			modifyIndex++;
	//			//currentMenuSelect++;
	//			if( modifyIndex > ControllerSettings::Count - 1 )
	//				modifyIndex -= ControllerSettings::Count;

	//			moveDown = true;
	//			moveDelayCounter = moveDelayFrames;
	//			//soundNodeList->ActivateSound( soundBuffers[S_DOWN] );
	//			//cout << "down modifyIndex: " << modifyIndex << endl;
	//		}
	//		else if( ( currInput.LUp() || currInput.PUp() ) && ( 
	//			(!moveUp && canMoveOther) || ( moveUp && canMoveSame ) ) )
	//		{
	//			//cout << "up modifyIndex: " << modifyIndex << endl;
	//			modifyIndex--;

	//			
	//			
	//			moveUp = true;
	//			moveDelayCounter = moveDelayFrames;
	//			//soundNodeList->ActivateSound( soundBuffers[S_UP] );
	//		}

	//		if( (currInput.LRight() || currInput.PRight()) && ( 
	//			(!moveRight && canMoveOther) || ( moveRight && canMoveSame ) ) )
	//		{
	//			modifyIndex+= ControllerSettings::Count / 2;

	//			if( modifyIndex > ControllerSettings::Count - 1 )
	//				modifyIndex -= ControllerSettings::Count;
	//			//currentMenuSelect++;
	//			/*if( selectedSaveIndex % 2 == 0 )
	//				selectedSaveIndex-= 2;*/
	//			moveRight = true;
	//			moveDelayCounter = moveDelayFrames;
	//		}
	//		else if( ( currInput.LLeft() || currInput.PLeft() ) && ( 
	//			(!moveLeft && canMoveOther) || ( moveLeft && canMoveSame ) ) )
	//		{
	//			modifyIndex -= ControllerSettings::Count / 2;

	//			if( modifyIndex < 0 )
	//				modifyIndex += ControllerSettings::Count;

	//			
	//			moveLeft = true;

	//			moveDelayCounter = moveDelayFrames;
	//		}
	//			
	//		if( moveDelayCounter > 0 )
	//		{
	//			moveDelayCounter--;
	//		}
	//			

	//		if( !(currInput.LDown() || currInput.PDown()) )
	//		{
	//			moveDown = false;
	//		}
	//		if( ! ( currInput.LUp() || currInput.PUp() ) )
	//		{
	//			moveUp = false;
	//		}

	//		if( !(currInput.LRight() || currInput.PRight()) )
	//		{
	//			moveRight = false;
	//		}
	//		if( !(currInput.LLeft() || currInput.PLeft() ) )
	//		{
	//			moveLeft = false;
	//		}

	//		
	//		break;
	//	}
	}
}

void OptionsMenu::Draw( sf::RenderTarget *target )
{
	/*for( int i = 0; i < 2; ++i )
	{
		inputSelectors[i]->Draw( target );
	}*/
	switch( mode )
	{
	case LEFTBAR:
		target->draw( controlIconVA );
		target->draw( schemeKeyboardVA );
		target->draw( schemeControllerVA );

		if( leftBarCurr == L_CONTROLLER_OPTIONS ||
			leftBarCurr == L_KB_OPTIONS )
		{
			target->draw( assocSymbols, ts_actionIcons->texture );
			
			
			for( int i = 0; i < ControllerSettings::Count; ++i )
			{
				target->draw( actionText[i] );
			}

			target->draw( buttonVA, ts_currentButtons->texture );
		}

		break;
	case MODIFY_XBOX_CONTROLS:
		{
			target->draw( controlIconVA );
			target->draw( schemeKeyboardVA );
			target->draw( schemeControllerVA );

			target->draw( controlIconVA );
			target->draw( schemeKeyboardVA );
			target->draw( schemeControllerVA );

			target->draw( assocSymbols, ts_actionIcons->texture );
			
			
			for( int i = 0; i < ControllerSettings::Count; ++i )
			{
				target->draw( actionText[i] );
			}

			target->draw( buttonVA, ts_currentButtons->texture );
		
			break;
		}
	
	
	/*case SELECT_SCHEME:
		break;*/
	}
}


void OptionsMenu::LoadControlOptions()
{
	string filename = "controlsettings";
	ifstream is( filename );
	if( !is.is_open() )
	{
		cout << "error loading: " << filename << endl;
		assert( 0 );
	} 

	//int inputType;

	//controllerType = (ControllerTypes::Type)inputType;

	for( int schemeIndex = 0; schemeIndex < 3; ++schemeIndex )
	{
		for( int i = 0; i < ControllerSettings::ButtonType::Count; ++i )
		{
			int temp;
			is >> temp;
			xboxInputAssoc[schemeIndex][i] = (XBoxButton)temp;
		}
	}

	is.close();

	
}

void OptionsMenu::SaveControlOptions()
{
	string filename = "controlsettings";
	ofstream of( filename );
	if( !of.is_open() )
	{
		cout << "error save to: " << filename << endl;
	}

	for( int schemeIndex = 0; schemeIndex < 3; ++schemeIndex )
	{
		for( int i = 0; i < ControllerSettings::ButtonType::Count; ++i )
		{
			of << (int)xboxInputAssoc[schemeIndex][i] << endl;
		}
	}
	of.close();
}

void OptionsMenu::UpdateButtonIcons()
{
	switch( controllerType )
	{
	case ControllerTypes::XBOX:
		UpdateXboxButtonIcons( selectedIndex );
		break;
	case ControllerTypes::GAMECUBE:
		break;
	case ControllerTypes::PS4:
		break;
	}
}

void OptionsMenu::UpdateXboxButtonIcons( int controlSetIndex )
{
	ts_currentButtons = ts_xboxButtons;
	for( int i = 0; i < ControllerSettings::ButtonType::Count; ++i )
	{
		int ind = (int)xboxInputAssoc[controlSetIndex][i] - 1;
		IntRect sub = ts_xboxButtons->GetSubRect( ind );
		buttonVA[i*4+0].texCoords = Vector2f( sub.left, sub.top );
		buttonVA[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
		buttonVA[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
		buttonVA[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );
	}
}

void OptionsMenu::SetButtonAssoc( int controlIndex, XBoxButton b )
{
	XBoxButton old = xboxInputAssoc[controlIndex][modifyIndex];
	xboxInputAssoc[controlIndex][modifyIndex] = b;

	for( int i = 0; i < ControllerSettings::Count; ++i )
	{
		if( i == modifyIndex )
			continue;

		if( xboxInputAssoc[controlIndex][i] == b )
		{
			xboxInputAssoc[controlIndex][i] = old;
			break;
		}
	}
}

void OptionsMenu::SetAssocSymbols( bool kb )
{
	/*for( int i = 0; i < 15; ++i )
	{
		buttonVA[i*4+0].position = Vector2f( 0, 0 );
		buttonVA[i*4+1].position = Vector2f( 0, 0 );
		buttonVA[i*4+2].position = Vector2f( 0, 0 );
		buttonVA[i*4+3].position = Vector2f( 0, 0 );
	}*/

	int symbolX = 288;
	int symbolY = 100;
	int textX = 288;
	int textY = 70;
	int extraX;
	int extraY;
	int symbolSize = 128;//140;//128;
	int spacing = 32;//16;
	int buttonIconSpacing = 150;
	//string inputType = inputSelectors[0]->GetString();
	if( !kb )
	{
		int numAssocSymbols = ControllerSettings::ButtonType::Count; //+1
		

		//move comes first
		for( int i = 0; i < numAssocSymbols; ++i )
		{
			/*assocSymbols[i*4+0].color = Color::Red;
			assocSymbols[i*4+1].color = Color::Green;
			assocSymbols[i*4+2].color = Color::Blue;
			assocSymbols[i*4+3].color = Color::Magenta;*/
			
			if( i < (numAssocSymbols) / 2 )
			{
				extraX = 0;
				extraY = 0;
			}
			else
			{
				extraX = 500;
				extraY = -(symbolSize + spacing) * ((numAssocSymbols) / 2);
			}
			
			assocSymbols[i*4+0].position = Vector2f( extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY );
			assocSymbols[i*4+1].position = Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY );
			assocSymbols[i*4+2].position = Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );
			assocSymbols[i*4+3].position = Vector2f( extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );

			IntRect sub = ts_actionIcons->GetSubRect( min( i, 9 ) );

			assocSymbols[i*4+0].texCoords = Vector2f( sub.left, sub.top );
			assocSymbols[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
			assocSymbols[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
			assocSymbols[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );

			/*if( i > 0 )
			{

				buttonVA[(i-1)*4+0].color = Color::Blue;
				buttonVA[(i-1)*4+1].color = Color::Blue;
				buttonVA[(i-1)*4+2].color = Color::Blue;
				buttonVA[(i-1)*4+3].color = Color::Blue;
				
				buttonVA[(i-1)*4+0].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX, (symbolSize + spacing) * (i-1) + extraY );
				buttonVA[(i-1)*4+1].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * (i-1) + extraY );
				buttonVA[(i-1)*4+2].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * (i-1) + symbolSize + extraY );
				buttonVA[(i-1)*4+3].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX, (symbolSize + spacing) * (i-1) + symbolSize + extraY );
			}*/

		

			/*buttonVA[i*4+0].color = Color::Blue;
			buttonVA[i*4+1].color = Color::Blue;
			buttonVA[i*4+2].color = Color::Blue;
			buttonVA[i*4+3].color = Color::Blue;*/
				
			buttonVA[i*4+0].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY );
			buttonVA[i*4+1].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY );
			buttonVA[i*4+2].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );
			buttonVA[i*4+3].position = Vector2f( buttonIconSpacing, 0 ) + Vector2f( extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY );

			actionText[i].setFont( mainMenu->arial );
			actionText[i].setCharacterSize( 24 );
			actionText[i].setPosition( Vector2f( extraX + textX, (symbolSize + spacing) * i + extraY + textY ) );//textX, 50 * i );
			actionText[i].setColor( Color::White );
			actionText[i].setString( possibleControllerActions[i] );

		}
		for( int i = numAssocSymbols; i < numAssocSymbols + 3; ++i )
		{
			assocSymbols[i*4+0].position = Vector2f( 0, 0 );
			assocSymbols[i*4+1].position = Vector2f( 0, 0 );
			assocSymbols[i*4+2].position = Vector2f( 0, 0 );
			assocSymbols[i*4+3].position = Vector2f( 0, 0 );
		}

		for( int i = 0; i < numAssocSymbols; ++i )
		{
			
		}
	}
	else
	{
		int numAssocSymbols = ControllerSettings::ButtonType::Count + 4;
		
		//move comes first
		for( int i = 0; i < numAssocSymbols; ++i )
		{
			assocSymbols[i*4+0].color = Color::Red;
			assocSymbols[i*4+1].color = Color::Green;
			assocSymbols[i*4+2].color = Color::Blue;
			assocSymbols[i*4+3].color = Color::Magenta;

			assocSymbols[i*4+0].position = Vector2f( symbolX, (symbolSize + spacing) * i );
			assocSymbols[i*4+1].position = Vector2f( symbolX + symbolSize, (symbolSize + spacing) * i );
			assocSymbols[i*4+2].position = Vector2f( symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize );
			assocSymbols[i*4+3].position = Vector2f( symbolX, (symbolSize + spacing) * i + symbolSize );
		}

		for( int i = 0; i < numAssocSymbols; ++i )
		{
			actionText[i].setFont( mainMenu->arial );
			actionText[i].setCharacterSize( 24 );
			actionText[i].setPosition( 100, 50 * i );
			actionText[i].setColor( Color::White );
			actionText[i].setString( possibleKeyboardActions[i] );
		}
	}
}

XBoxButton OptionsMenu::CheckXBoxInput( ControllerState &currInput )
{
	bool leftMovement = true;
	//bool leftMovement = (inputSelectors[2]->GetString() == "left analog");
	if( currInput.A )
	{
		return XBOX_A;
	}
	else if( currInput.B )
	{
		return XBOX_B;
	}
	else if( currInput.X )
	{
		return XBOX_X;
	}
	else if( currInput.Y )
	{
		return XBOX_Y;
	}
	else if( currInput.leftShoulder )
	{
		return XBOX_L1;
	}
	else if( currInput.rightShoulder )
	{
		return XBOX_R1;
	}
	else if( currInput.LeftTriggerPressed() )
	{
		return XBOX_L2;
	}
	else if( currInput.RightTriggerPressed() )
	{
		return XBOX_R2;
	}
	else if( currInput.PLeft() )
	{
		return XBOX_PLEFT;
	}
	else if( currInput.PUp() )
	{
		return XBOX_PUP;
	}
	else if( currInput.PRight() )
	{
		return XBOX_PRIGHT;
	}
	else if( currInput.PDown() )
	{
		return XBOX_PDOWN;
	}
	else if( leftMovement && currInput.RLeft() )
	{
		return XBOX_RLEFT;
	}
	else if( leftMovement && currInput.RUp() )
	{
		return XBOX_RUP;
	}
	else if( leftMovement && currInput.RRight() )
	{
		return XBOX_RRIGHT;
	}
	else if( leftMovement && currInput.RDown() )
	{
		return XBOX_RDOWN;
	}
	else if( !leftMovement && currInput.LLeft() )
	{
		return XBOX_LLEFT;
	}
	else if( !leftMovement && currInput.LUp() )
	{
		return XBOX_LUP;
	}
	else if( !leftMovement && currInput.LRight() )
	{
		return XBOX_LRIGHT;
	}
	else if( !leftMovement && currInput.LDown() )
	{
		return XBOX_LDOWN;
	}
	else if( currInput.back )
	{
		return XBOX_BACK;
	}
	else if( currInput.start )
	{
		return XBOX_START;
	}
	else
	{
		return XBOX_BLANK;
	}
}

PauseMenu::PauseMenu( GameSession *p_owner )
	:owner( p_owner ), currentTab( Tab::MAP ),  accelBez( 0, 0, 1, 1 )
	
{
	optionType = OptionType::O_INPUT;
	cOptions = new OptionsMenu( this );
	ts_background[0] = owner->GetTileset( "Menu/pause_1_map_1820x980.png", 1820, 980 );
	ts_background[1] = owner->GetTileset( "Menu/pause_2_kin_1820x980.png", 1820, 980 );
	ts_background[2] = owner->GetTileset( "Menu/pause_3_shards_1820x980.png", 1820, 980 );
	ts_background[3] = owner->GetTileset( "Menu/pause_4_options_1820x980.png", 1820, 980 );
	ts_background[4] = owner->GetTileset( "Menu/pause_5_pause_1820x980.png", 1820, 980 );

	ts_select = owner->GetTileset( "Menu/menu_select_800x140.png", 800, 140 );
	

	
	selectSprite.setTexture( *ts_select->texture );

	bgSprite.setPosition( 0, 0 );

	pauseSelectIndex = 0;

	numVideoOptions = 3;
	videoSelectors = new OptionSelector*[numVideoOptions];

	

	//cout << "MIDDLE this initialization" << endl;

	if( owner->saveFile != NULL )
	{
		SaveFile *sf = owner->saveFile;

		//if they dont have the power yet, rename them to specials 1-5 

		//if 
		//sf->
		
	}
	
	/*{ 
		"move", "jump", "dash", "attack", bounceSpecial, grindSpecial,
		timeslowSpecial, wireLeftSpecial, wireRightSpecial, 
		toggleBounce, toggleGrind, toggleTimeSlow };*/

	

	/*string possibleKeyboardActions[15] = { 
	"up", "left", "down", "right", "jump", "dash", "attack", bounceSpecial, grindSpecial,
	timeslowSpecial, wireLeftSpecial, wireRightSpecial, 
	toggleBounce, toggleGrind, toggleTimeSlow };*/


	

	//resolution
	//fullscreen
	//vsync

	//video
	string resolutions[] = { "1920 x 1080", "1600 x 900" };
	videoSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 2, resolutions );

	string windowType[] = { "Fullscreen", "Window", "Borderless Window" };
	videoSelectors[1] = new OptionSelector( Vector2f( 100, 100 ), 3, windowType );
	//
	string vsync[] = { "on", "off" };
	videoSelectors[2] = new OptionSelector( Vector2f( 100, 180 ), 2, vsync );

	////cout << "YYYYY this initialization" << endl;
	string masterVolume[101];
	string musicVolume[101];
	string soundVolume[101];

	////cout << "WATTTT" << endl;

	//
	for( int i = 0; i < 101; ++i )
	{
		//cout << "starting loop: " << i << endl;

		stringstream ss;	
		ss << i;

		
		masterVolume[i] = ss.str();
		musicVolume[i] = ss.str();
		soundVolume[i] = ss.str();
		//cout << "end loop: " << i << endl;
	}
	//cout << "WATTTT" << endl;
	numSoundOptions = 5;
	soundSelectors = new OptionSelector*[numSoundOptions];

	string enable[] = {"on", "off"};

	//cout << "awfhaweioph ihewhtf a" << endl;
	soundSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 101, masterVolume );
	soundSelectors[1] = new OptionSelector( Vector2f( 100, 180 ), 2, enable );
	soundSelectors[2] = new OptionSelector( Vector2f( 100, 260 ), 101, musicVolume );
	soundSelectors[3] = new OptionSelector( Vector2f( 100, 340 ), 2, enable );
	soundSelectors[4] = new OptionSelector( Vector2f( 100, 420 ), 101, soundVolume );

	//cout << "ZZZZ this initialization" << endl;

	currentSelectors = soundSelectors;
	numCurrentSelectors = numSoundOptions;
	optionSelectorIndex = 0;
	maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	momentum = 0;
	maxMomentum = 4;
	/*maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	framesWaiting = maxWaitFrames;
	momentum = 0;
	maxMomentum = 4;*/

	//bgSprite.setPosition( (1920 - 1820) / 2, (1080 - 980) / 2);

	//SetTab( MAP );
	//SetAssocSymbols();
	//cout << "end this initialization" << endl;
}

PauseMenu::~PauseMenu()
{
	/*for( int i = 0; i < numVideoOptions; ++i )
	{
		delete videoSelectors[i];
	}
	delete [] videoSelectors;*/
}










void PauseMenu::TabLeft()
{
	int index = (int)currentTab;
	index--;
	if( index < 0 )
		index = 4;

	currentTab = (Tab)index;
	SetTab( currentTab );
}

void PauseMenu::TabRight()
{
	int index = (int)currentTab;
	index++;
	if( index > 4 )
		index = 0;

	currentTab = (Tab)index;
	SetTab( currentTab );
}

void PauseMenu::SetTab( Tab t )
{
	currentTab = t;
	bgSprite.setTexture( *ts_background[currentTab]->texture );

	switch( t )
	{
	case MAP:
		mapCenter.x = owner->GetPlayer( 0 )->position.x;
		mapCenter.y = owner->GetPlayer( 0 )->position.y;
		mapZoomFactor = 16;	
		break;
	case KIN:
		break;
	case SHARDS:
		break;
	case OPTIONS:
		//LoadControlOptions();
		//UpdateButtonIcons();
		break;
	case PAUSE:
		pauseSelectIndex = 0;
		break;
	}
}

void PauseMenu::Draw( sf::RenderTarget *target )
{
	target->draw( bgSprite );

	if( currentTab == PAUSE )
	{
		target->draw( selectSprite );
	}
	else if( currentTab == OPTIONS )
	{
		/*string inputType = inputSelectors[0]->GetString();
		int num = numCurrentSelectors;
		if( inputType == "Keyboard" )
		{
			num = num - 1;
		}
		for( int i = 0; i < num; ++i )
		{
			currentSelectors[i]->Draw( target );
		}*/
		

		switch( optionType )
		{
		case O_VIDEO:
			{
				//return UpdateVideoOptions( currInput, prevInput );
				break;
			}
		case O_AUDIO:
			{
				//return UpdateAudioOptions( currInput, prevInput );
				break;
			}
		case O_INPUT:
			{
				
				cOptions->Draw( target );
				break;
			}
		}
		//if( currentSelectors == inputSelectors )
		//{
		//	target->draw( assocSymbols, ts_actionIcons->texture );

		//	
		//	int cap;
		//	if( inputType == "Xbox" )
		//	{
		//		cap = 12;
		//	}
		//	else if( inputType == "Keyboard" )
		//	{
		//		cap = 15;
		//	}
		//	else
		//	{
		//		//test
		//		cap = 8;
		//	}
		//	for( int i = 0; i < cap; ++i )
		//	{
		//		target->draw( actionText[i] );
		//	}

		//	target->draw( buttonVA, ts_currentButtons->texture );
		//}

		
	}
}

void PauseMenu::ApplyVideoSettings()
{
	string resolution = videoSelectors[0]->GetString();
	string windowType = videoSelectors[1]->GetString();
	string vsync = videoSelectors[2]->GetString();

	int width, height;
	int style;
	if( resolution == "1920 x 1080" )
	{
		width = 1920;
		height = 1080;
		//owner->mainMenu->ResizeWindow( 1920, 1080 );
		/*sf::VideoMode( windowWidth, windowHeight ), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ) );*/
		//owner->window->create( VideoMode( 1920, 1080 ), "Breakneck",sf::Style::None, sf::ContextSettings( 0, 0, 0, 0, 0 )  );

		//owner->updatewin
		//owner->sets
	}
	else if( resolution == "1600 x 900" )
	{
		width = 1600;
		height = 900;
		//owner->mainMenu->ResizeWindow( 1600, 900 );
	}
	else
	{
		assert( 0 );
	}
	

	if( windowType == "Fullscreen" )
	{
		style = sf::Style::Fullscreen;
	}
	else if( windowType == "Window" )
	{
		style = sf::Style::Default;
	}
	else if( windowType == "Borderless Window" )
	{
		style = sf::Style::None;
	}
	else
	{
		assert( 0 );
	}
	owner->mainMenu->ResizeWindow( width, height, style );


	if( vsync == "on" )
	{
		owner->window->setVerticalSyncEnabled( true );
	}
	else if( vsync == "off" )
	{
		owner->window->setVerticalSyncEnabled( false );
	}
	else
	{
		assert( 0 );
	}
	
}

void PauseMenu::ApplySoundSettings()
{
	/* masterVolume 
	enable );
	, musicVolume 
	enable );
	, soundVolume */


	string masterVolume = soundSelectors[0]->GetString();
	string enableMusic = soundSelectors[1]->GetString();
	string musicVolume = soundSelectors[2]->GetString();
	string enableSounds= soundSelectors[3]->GetString();
	string soundVolume = soundSelectors[4]->GetString();

	int width, height;
	int style;
	int master,music,sound;
	stringstream ss;

	master = stoi( masterVolume );
	music = stoi( musicVolume );
	sound = stoi( soundVolume );

	cout << "apply master: " << master << ", music: " << music << ", sound: " << sound << endl;

	bool enSounds;
	if( enableSounds == "on" )
		enSounds = true;
	else if( enableSounds == "off" )
		enSounds = false;

	bool enMusic;
	if( enableMusic == "on" )
		enMusic = true;
	else if( enableMusic == "off" )
		enMusic = false;


	owner->soundNodeList->SetGlobalVolume( master );
	owner->mainMenu->soundNodeList->SetGlobalVolume( master );

	owner->soundNodeList->SetSoundsEnable( enSounds );
	owner->mainMenu->soundNodeList->SetSoundsEnable( enSounds );

	//owner->soundNodeList->SetMusicEnable( enMusic );
	//owner->mainMenu->soundNodeList->SetMusicEnable( enMusic );

	owner->soundNodeList->SetRelativeSoundVolume( sound );
	owner->mainMenu->soundNodeList->SetRelativeSoundVolume( sound );

	//owner->soundNodeList->SetRelativeMusicVolume( music );
	//owner->mainMenu->soundNodeList->SetRelativeMusicVolume( music );
}

//run this in a loop


PauseMenu::UpdateResponse PauseMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	if( (currInput.start && !prevInput.start) || (currInput.back && !prevInput.back ) )
	{
		owner->state = GameSession::State::RUN;
		owner->soundNodeList->Pause( false );
	}

	if( currInput.leftShoulder && !prevInput.leftShoulder )
	{
		TabLeft();
		return R_NONE;
	}
	else if( currInput.rightShoulder && !prevInput.rightShoulder )
	{
		TabRight();
		return R_NONE;
	}


	switch( currentTab )
	{
	case MAP:
		{
			float fac = .05;
			if( currInput.A )
			{
				mapZoomFactor -= fac * mapZoomFactor;
			}
			else if( currInput.B )
			{
				mapZoomFactor += fac * mapZoomFactor;
			}

			if( mapZoomFactor < 1.f )
			{
				mapZoomFactor = 1.f;
			}
			else if( mapZoomFactor > 128.f )
			{
				mapZoomFactor = 128.f;
			}

			float move = 20.0 * mapZoomFactor / 2.0;
			if( currInput.LLeft() )
			{
				mapCenter.x -= move;
			}
			else if( currInput.LRight() )
			{
				mapCenter.x += move;
			}

			if( currInput.LUp() )
			{
				mapCenter.y -= move;
			}
			else if( currInput.LDown() )
			{
				mapCenter.y += move;
			}

			if( mapCenter.x < owner->leftBounds )
			{
				mapCenter.x = owner->leftBounds;
			}
			else if( mapCenter.x > owner->leftBounds + owner->boundsWidth )
			{
				mapCenter.x = owner->leftBounds + owner->boundsWidth;
			}

			if( mapCenter.y < owner->topBounds )
			{
				mapCenter.y = owner->topBounds;
			}
			else if( mapCenter.y > owner->topBounds + owner->boundsHeight )
			{
				mapCenter.y = owner->topBounds + owner->boundsHeight;
			}
			break;
		}
	case KIN:
		{
			break;
		}
	case SHARDS:
		{
			break;
		}
	case OPTIONS:
		{	
			return UpdateOptions( currInput, prevInput );

		
			
			break;
		}
	case PAUSE:
		{
			if( currInput.A && !prevInput.A )
			{
				UpdateResponse ur = (UpdateResponse)(pauseSelectIndex+1);
				return ur;
				//switch( pauseSelectIndex )
				//{
				//case 0: //resume
				//	{
				//		//owner->state = GameSession::RUN;
				//		//owner->soundNodeList->Pause( false );
				//		break;
				//	}
				//case 1: //restart
				//	{

				//		break;
				//	}
				//case 2: //exit level
				//	break;
				//case 3: //exit to title
				//	break;
				//case 4: //exit game
				//	break;
				//}
			}
			else if( currInput.LDown() && !prevInput.LDown() )
			{
				pauseSelectIndex++;
				if( pauseSelectIndex == 5 )
				{
					pauseSelectIndex = 0;
				}

				
			}
			else if( currInput.LUp() && !prevInput.LUp() )
			{
				pauseSelectIndex--;
				if( pauseSelectIndex == -1 )
				{
					pauseSelectIndex = 4;
				}
			}

			int h = 200;
			selectSprite.setPosition( 100, 100 + h * pauseSelectIndex );
			break;
		}
	}

	return R_NONE;
}


PauseMenu::UpdateResponse PauseMenu::UpdateOptions( ControllerState &currInput,
	ControllerState &prevInput )
{
	switch( optionType )
	{
	case O_VIDEO:
		return UpdateVideoOptions( currInput, prevInput );
		break;
	case O_AUDIO:
		return UpdateAudioOptions( currInput, prevInput );
		break;
	case O_INPUT:
		
		cOptions->Update( currInput, prevInput );
		//return UpdateResponse::R_NONE;
		break;
	}
	/*if( currentSelectors == inputSelectors )
	{
		return UpdateInputOptions( currInput, prevInput );
	}
	else if( currentSelectors == videoSelectors )
	{
		
	}
	else if( currentSelectors == soundSelectors )
	{
		
	}
	else
	{
		assert( 0 );
		return R_NONE;
	}*/
	

	return R_NONE;
}

PauseMenu::UpdateResponse PauseMenu::UpdateVideoOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	if( currInput.A && !prevInput.A )
	{
		ApplyVideoSettings();
				
		return R_NONE;
	}
	else if( currInput.B && !prevInput.B )
	{
		//if( currentSelectors == inputSelectors )
		//{
		//	if( selectingProfile )
		//	{
		//		//go back to selecting option type
		//	}
		//	else
		//	{
		//		selectingProfile = true;
		//	}
		//}
	}
	if( currInput.LDown() && ( framesWaiting >= currWaitFrames || momentum <= 0 ))
	{
		//cout << "DOWN" << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex++;
		if( optionSelectorIndex == numCurrentSelectors )
		{
			optionSelectorIndex = 0;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
	else if( currInput.LUp() && ( framesWaiting >= currWaitFrames || momentum >= 0 ))
	{
		//cout << "up! " << framesWaiting << ", " << momentum << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex--;
		if( optionSelectorIndex == -1 )
		{
			optionSelectorIndex = numCurrentSelectors - 1;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );

				
	}
	else if( currInput.LRight() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted right" << endl;
		currentSelectors[optionSelectorIndex]->Right();
	}
	else if( currInput.LLeft() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted left" << endl;
		currentSelectors[optionSelectorIndex]->Left();
	}
			
	if( !currInput.LUp() && !currInput.LDown() )
	{
		momentum = 0;
		framesWaiting = maxWaitFrames;
		//currWaitFrames = maxWaitFrames;
	}

	if( !currInput.LLeft() && !currInput.LRight() )
	{
		currentSelectors[optionSelectorIndex]->Stop();
	}
	currentSelectors[optionSelectorIndex]->Update();
	++framesWaiting;


	/*if( selectingProfile )
	{

	}*/
}

PauseMenu::UpdateResponse PauseMenu::UpdateAudioOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	//cout <<  framesWaiting << ", " << momentum << ", curr: " << currWaitFrames << endl;
	if( currInput.A && !prevInput.A )
	{
		ApplySoundSettings();
					
		return R_NONE;
	}
	else if( currInput.B && !prevInput.B )
	{
		
	}
	if( currInput.LDown() && ( framesWaiting >= currWaitFrames || momentum <= 0 ))
	{
		//cout << "DOWN" << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex++;
		if( optionSelectorIndex == numCurrentSelectors )
		{
			optionSelectorIndex = 0;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
	else if( currInput.LUp() && ( framesWaiting >= currWaitFrames || momentum >= 0 ))
	{
		//cout << "up! " << framesWaiting << ", " << momentum << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex--;
		if( optionSelectorIndex == -1 )
		{
			optionSelectorIndex = numCurrentSelectors - 1;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );

				
	}
	else if( currInput.LRight() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted right" << endl;
		currentSelectors[optionSelectorIndex]->Right();

		
	}
	else if( currInput.LLeft() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted left" << endl;
		currentSelectors[optionSelectorIndex]->Left();

		
	}
			
	if( !currInput.LUp() && !currInput.LDown() )
	{
		momentum = 0;
		framesWaiting = maxWaitFrames;
		//currWaitFrames = maxWaitFrames;
	}

	if( !currInput.LLeft() && !currInput.LRight() )
	{
		currentSelectors[optionSelectorIndex]->Stop();
	}
	currentSelectors[optionSelectorIndex]->Update();
	++framesWaiting;


	//if( selectingProfile )
	//{

	//}
}

PauseMenu::UpdateResponse PauseMenu::UpdateInputOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	return UpdateResponse::R_NONE;
}
//using namespace std;
//using namespace sf;