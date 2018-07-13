#include "Input.h"
#include <math.h>
#include <fstream>
#include <assert.h>
#include <iostream>

using namespace std;

#define PI 3.14159265359
const DWORD GameController::LEFT_STICK_DEADZONE = 7849;
const DWORD GameController::RIGHT_STICK_DEADZONE = 8689;
const DWORD GameController::TRIGGER_THRESHOLD = 30;

ControllerState::ControllerState()
	:leftStickMagnitude( 0 ), leftStickRadians( 0 ), rightStickMagnitude( 0 ), 
	rightStickRadians( 0 ), leftTrigger( 0 ), rightTrigger( 0 ), start( false ), 
	back( false ), leftShoulder( false ), rightShoulder( false ), A( false ), B( false ), 
	X( false ), Y( false ), pad( 0 ), leftStickPad( 0 ), rightStickPad( 0 ), 
	leftPress( false ), rightPress( false ), triggerThresh( 200 )
{

	
}

void ControllerState::Set( const ControllerState &state )
{
	leftStickMagnitude = state.leftStickMagnitude;
	leftStickRadians = state.leftStickRadians;
	rightStickMagnitude = state.rightStickMagnitude;
	rightStickRadians = state.rightStickRadians;
	leftTrigger = state.leftTrigger;
	rightTrigger = state.rightTrigger;
	start = state.start;
	back = state.back;
	leftShoulder = state.leftShoulder;
	rightShoulder = state.rightShoulder;
	A = state.A;
	B = state.B;
	X = state.X;
	Y = state.Y;
	pad = state.pad;
	leftStickPad = state.leftStickPad;
	rightStickPad = state.rightStickPad;
	leftPress = state.leftPress;
	rightPress = state.rightPress;
//	altPad = state.altPad;
}

bool ControllerState::PUp()
{
	return pad & 1;
}

bool ControllerState::PDown()
{
	return pad & 2;
}

bool ControllerState::PLeft()
{
	return pad & 4;
}

bool ControllerState::PRight()
{
	return pad & 8;
}

bool ControllerState::LUp()
{
	return leftStickPad & 1;
}

bool ControllerState::LDown()
{
	return leftStickPad & 2;
}

bool ControllerState::LLeft()
{
	return leftStickPad & 4;
}

bool ControllerState::LRight()
{
	return leftStickPad & 8;
}

bool ControllerState::LPress()
{
	return leftPress;
}

bool ControllerState::RUp()
{
	return rightStickPad & 1;
}

bool ControllerState::RDown()
{
	return rightStickPad & 2;
}

bool ControllerState::RLeft()
{
	return rightStickPad & 4;
}

bool ControllerState::RRight()
{
	return rightStickPad & 8;
}

bool ControllerState::RPress()
{
	return rightPress;
}

bool ControllerState::RightTriggerPressed()
{
	return rightTrigger >= triggerThresh;
}

bool ControllerState::LeftTriggerPressed()
{
	return leftTrigger >= triggerThresh;
}

bool GameController::UpdateState()
{
	XINPUT_STATE state;
	ControllerState tempState;
	ZeroMemory( &state, sizeof( XINPUT_STATE ) );
	DWORD result = XInputGetState( m_index, &state );

	if( result == ERROR_SUCCESS )
	{
		//cout << "updating controller state " << m_index << endl;
		double LX = state.Gamepad.sThumbLX;
		double LY = state.Gamepad.sThumbLY;
		//i dont think i need a magnitude, magnitude should be calculated for differences
		//between controller states by some other function
		double magnitude = sqrt( LX * LX + LY * LY );

		double normalizedLX = LX / magnitude;
		double normalizedLY = LY / magnitude;

		double normalizedMagnitude = 0;

		if( magnitude > LEFT_STICK_DEADZONE )
		{
			if( magnitude > 32767 )
				magnitude = 32767;

			magnitude -= LEFT_STICK_DEADZONE;
			normalizedMagnitude = magnitude / (32767 - LEFT_STICK_DEADZONE);

			
			m_state.leftStickRadians = atan(normalizedLY / normalizedLX);
			if (normalizedLX < 0.0f)
				m_state.leftStickRadians += PI;
		}
		else 
		{
			magnitude = 0.0f;
			normalizedMagnitude = 0.0;

			m_state.leftStickRadians = 0.0;
		}

		m_state.leftStickMagnitude = normalizedMagnitude;

		//cout << "radians: " << m_state.leftStickRadians << endl;

		double RX = state.Gamepad.sThumbRX;
		double RY = state.Gamepad.sThumbRY;

		magnitude = sqrt( RX * RX + RY * RY );

		double normalizedRX = RX / magnitude;
		double normalizedRY = RY / magnitude;

		if( magnitude > RIGHT_STICK_DEADZONE )
		{
			if( magnitude > 32767 )
				magnitude = 32767;

			magnitude -= RIGHT_STICK_DEADZONE;
			normalizedMagnitude = magnitude / ( 32767 - RIGHT_STICK_DEADZONE );
		}
		else 
		{
			magnitude = 0.0f;
			normalizedMagnitude = 0.0f;
		}

		m_state.rightStickMagnitude = normalizedMagnitude;
		m_state.rightStickRadians = atan( normalizedRY / normalizedRX );
		if( normalizedRX < 0.0f )
			m_state.rightStickRadians += PI;

		m_state.leftTrigger = state.Gamepad.bLeftTrigger;
		m_state.rightTrigger = state.Gamepad.bRightTrigger;

		WORD b = state.Gamepad.wButtons;
		m_state.start = (b & 0x10) > 0;
		m_state.back = (b & 0x20) > 0;
		m_state.leftShoulder = (b & 0x100) > 0;
		m_state.rightShoulder = (b & 0x200) > 0;
		m_state.A = (b & 0x1000) > 0;
		m_state.B = (b & 0x2000) > 0;
		m_state.X = (b & 0x4000) > 0;
		m_state.Y = (b & 0x8000) > 0;
		m_state.leftPress = b & XINPUT_GAMEPAD_LEFT_THUMB;
		m_state.rightPress = b & XINPUT_GAMEPAD_RIGHT_THUMB;
		m_state.pad = ( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 ); 

		m_state.leftStickPad = 0;
		m_state.rightStickPad = 0;
		if( m_state.leftStickMagnitude > stickThresh )
		{
			//cout << "left stick radians: " << currInput.leftStickRadians << endl;
			float x = cos( m_state.leftStickRadians );
			float y = sin( m_state.leftStickRadians );

			if( x > stickThresh )
				m_state.leftStickPad += 1 << 3;
			if( x < -stickThresh )
				m_state.leftStickPad += 1 << 2;
			if( y > stickThresh )
				m_state.leftStickPad += 1;
			if( y < -stickThresh )
				m_state.leftStickPad += 1 << 1;
		}

		if( m_state.rightStickMagnitude > stickThresh )
		{
			//cout << "left stick radians: " << m_state.leftStickRadians << endl;
			float x = cos( m_state.rightStickRadians );
			float y = sin( m_state.rightStickRadians );

			if( x > stickThresh )
				m_state.rightStickPad += 1 << 3;
			if( x < -stickThresh )
				m_state.rightStickPad += 1 << 2;
			if( y > stickThresh )
				m_state.rightStickPad += 1;
			if( y < -stickThresh )
				m_state.rightStickPad += 1 << 1;
		}
		tempState = m_state;
		m_unfilteredState = m_state;

		tempState.A = Pressed( filter[ControllerSettings::JUMP] );
		tempState.B = Pressed( filter[ControllerSettings::DASH] );
		tempState.rightShoulder = Pressed( filter[ControllerSettings::ATTACK] );
		tempState.X = Pressed( filter[ControllerSettings::BOUNCE] );
		tempState.Y = Pressed( filter[ControllerSettings::GRIND] );
		tempState.leftShoulder = Pressed( filter[ControllerSettings::TIMESLOW] );
		tempState.leftTrigger = Pressed( filter[ControllerSettings::LEFTWIRE] );
		tempState.rightTrigger = Pressed( filter[ControllerSettings::RIGHTWIRE] );
		tempState.back = Pressed( filter[ControllerSettings::MAP] );
		tempState.start = Pressed( filter[ControllerSettings::PAUSE] );

		m_state = tempState;
	}
	else
	{
		//cout << "updating controller state keyboard " << m_index << endl;
		using namespace sf;
		WORD b = state.Gamepad.wButtons;
		m_state.start = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::PAUSE] );
		m_state.back = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::MAP] );
		m_state.leftShoulder = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::TIMESLOW] );
		m_state.rightShoulder = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::SLASH] );
		m_state.A = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::JUMP] );
		m_state.B = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::DASH] );
		m_state.X = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::BOUNCE] );
		m_state.Y = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::GRIND] );
		m_state.leftPress = false;//b & XINPUT_GAMEPAD_LEFT_THUMB;
		m_state.rightPress = false;//b & XINPUT_GAMEPAD_RIGHT_THUMB;
		m_state.pad = 0;//( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 );

		m_state.leftTrigger = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::LEFTWIRE] );
		m_state.rightTrigger = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::RIGHTWIRE] );

		/*bool up = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::UP] );
		bool down = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::DOWN] );
		bool left = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::LEFT] );
		bool right = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::RIGHT] );*/

		bool up = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::UP] );
		bool down = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::DOWN] );
		bool left = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::LEFT] );
		bool right = Keyboard::isKeyPressed( keySettings.buttonMap[KeyboardSettings::RIGHT] );

		sf::Vector2<double> thing( 0, 0 );
		if( left )
		{
			//m_state.leftStickMagnitude = 1;
			thing.x -= 1.0;
		}
		if( right )
		{
			//m_state.leftStickMagnitude = 1;
			thing.x += 1.0;
		}

		if( up )
		{
			//m_state.leftStickMagnitude = 1;
			thing.y -= 1.0;
		}
		if( down )
		{
			//m_state.leftStickMagnitude = 1;
			thing.y += 1.0;
		}

		if( thing.x != 0 || thing.y != 0 )
			m_state.leftStickMagnitude = 1;

		double mag = sqrt( thing.x * thing.x + thing.y * thing.y );
		thing = sf::Vector2<double>( thing.x / mag, thing.y / mag );

		double angle = atan2( thing.y, thing.x );
		
		m_state.leftStickRadians = angle;

		
		m_state.leftStickPad = 0;

		if( right )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1 << 3;
			//cout << "RIGHT" << endl;
		}
		else if( left )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1 << 2;
			//cout << "LEFT" << endl;
		}

		if( up )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1 << 1;
			//cout << "UP" << endl;
		}
		else if( down )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1;
			//cout << "DOWN" << endl;
		}

		m_state.rightStickMagnitude = 0;
		m_state.rightStickPad = 0;


		//if( m_state.leftStickMagnitude > stickThresh )
		//{
		//	//cout << "left stick radians: " << currInput.leftStickRadians << endl;
		//	float x = cos( m_state.leftStickRadians );
		//	float y = sin( m_state.leftStickRadians );

		//	if( x > stickThresh )
		//		m_state.leftStickPad += 1 << 3;
		//	if( x < -stickThresh )
		//		m_state.leftStickPad += 1 << 2;
		//	if( y > stickThresh )
		//		m_state.leftStickPad += 1;
		//	if( y < -stickThresh )
		//		m_state.leftStickPad += 1 << 1;
		//}

		//if( m_state.rightStickMagnitude > stickThresh )
		//{
		//	//cout << "left stick radians: " << m_state.leftStickRadians << endl;
		//	float x = cos( m_state.rightStickRadians );
		//	float y = sin( m_state.rightStickRadians );

		//	if( x > stickThresh )
		//		m_state.rightStickPad += 1 << 3;
		//	if( x < -stickThresh )
		//		m_state.rightStickPad += 1 << 2;
		//	if( y > stickThresh )
		//		m_state.rightStickPad += 1;
		//	if( y < -stickThresh )
		//		m_state.rightStickPad += 1 << 1;
		//}

		

		
	}

	

	return ( result == ERROR_SUCCESS );
}

int GameController::Pressed( XBoxButton b )
{
	switch( b )
	{
	case XBOX_A:
		return m_state.A;
		break;
	case XBOX_B:
		return m_state.B;
		break;
	case XBOX_X:
		return m_state.X;
		break;
	case XBOX_Y:
		return m_state.Y;
		break;
	case XBOX_R1:
		return m_state.rightShoulder;
		break;
	case XBOX_R2:
		return m_state.rightTrigger;
		break;
	case XBOX_L1:
		return m_state.leftShoulder;
		break;
	case XBOX_L2:
		return m_state.leftTrigger;
		break;
	case XBOX_START:
		return m_state.start;
		break;
	case XBOX_BACK:
		return m_state.back;
		break;
	}
}

float GameController::stickThresh = .4;
GameController::GameController( DWORD index )
	:m_index( index )
{
	

	/*for( int i = 0; i < ControllerSettings::Count; ++i )
	{
		filter[i] = (XBoxButton)(i+1);
	}*/

	SetFilterDefault( filter );
}

ControllerState & GameController::GetState()
{
	return m_state;
}

ControllerState & GameController::GetUnfilteredState()
{
	return m_unfilteredState;
}


bool GameController::IsConnected()
{
	XINPUT_STATE state;
	ZeroMemory( &state, sizeof( XINPUT_STATE ) );
	DWORD result = XInputGetState( m_index, &state );

	return ( result == ERROR_SUCCESS );
}

DWORD GameController::GetIndex()
{
	return m_index;
}

void GameController::SetFilter( XBoxButton *buttons )
{
	for( int i = 0; i < ControllerSettings::Count; ++i )
	{
		filter[i] = buttons[i];
	}
}

//ControllerState &GameController::GetKeyboardState()
//{
//	using namespace sf;
//
//	/*if( Keyboard::isKeyPressed( buttonMap[UP] ) )
//	{
//
//	}*/
//	//need to address these
//	//m_state.leftStickMagnitude = normalizedMagnitude;
//	//m_state.leftStickRadians = atan( normalizedLY / normalizedLX );
//
//	//if( normalizedLX < 0.0f )
//	//	m_state.leftStickRadians += PI;
//
//	//m_state.rightStickMagnitude = normalizedMagnitude;
//	//m_state.rightStickRadians = atan( normalizedRY / normalizedRX );
//	//if( normalizedRX < 0.0f )
//	//	m_state.rightStickRadians += PI;
//
//	//m_state.leftTrigger = state.Gamepad.bLeftTrigger; //0 or 255
//	//m_state.rightTrigger = state.Gamepad.bRightTrigger;
//
//	//WORD b = state.Gamepad.wButtons;
//	//m_state.start = (b & 0x10) > 0;
//	//m_state.back = (b & 0x20) > 0;
//	//m_state.leftShoulder = (b & 0x100) > 0;
//	//m_state.rightShoulder = (b & 0x200) > 0;
//	//m_state.A = (b & 0x1000) > 0;
//	//m_state.B = (b & 0x2000) > 0;
//	//m_state.X = (b & 0x4000) > 0;
//	//m_state.Y = (b & 0x8000) > 0;
//	//m_state.leftPress = b & XINPUT_GAMEPAD_LEFT_THUMB;
//	//m_state.rightPress = b & XINPUT_GAMEPAD_RIGHT_THUMB;
//	//m_state.pad = ( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 ); 
//
//	//m_state.leftStickPad = 0;
//	//m_state.rightStickPad = 0;
//	//if( m_state.leftStickMagnitude > stickThresh )
//	//{
//	//	//cout << "left stick radians: " << currInput.leftStickRadians << endl;
//	//	float x = cos( m_state.leftStickRadians );
//	//	float y = sin( m_state.leftStickRadians );
//
//	//	if( x > stickThresh )
//	//		m_state.leftStickPad += 1 << 3;
//	//	if( x < -stickThresh )
//	//		m_state.leftStickPad += 1 << 2;
//	//	if( y > stickThresh )
//	//		m_state.leftStickPad += 1;
//	//	if( y < -stickThresh )
//	//		m_state.leftStickPad += 1 << 1;
//	//}
//
//	//if( m_state.rightStickMagnitude > stickThresh )
//	//{
//	//	//cout << "left stick radians: " << m_state.leftStickRadians << endl;
//	//	float x = cos( m_state.rightStickRadians );
//	//	float y = sin( m_state.rightStickRadians );
//
//	//	if( x > stickThresh )
//	//		m_state.rightStickPad += 1 << 3;
//	//	if( x < -stickThresh )
//	//		m_state.rightStickPad += 1 << 2;
//	//	if( y > stickThresh )
//	//		m_state.rightStickPad += 1;
//	//	if( y < -stickThresh )
//	//		m_state.rightStickPad += 1 << 1;
//	//}
//}

KeyboardFilter::KeyboardFilter()
{
	for( int i = 0; i < sf::Keyboard::KeyCount; ++i )
	{
		keyFilter[i] = (sf::Keyboard::Key)i;
	}
}

sf::Keyboard::Key KeyboardFilter::Filter( sf::Keyboard::Key key )
{
	int i = (int)key;

	return keyFilter[i];
}

void SetKey( sf::Keyboard::Key old, 
		sf::Keyboard::Key newKey )
{
	//keyFilter[
}

void LoadInputMapKeyboard( ControllerState &cs, const std::string &fileName, KeyboardFilter &filter )
{
	ifstream is;
	is.open( fileName );

	if( is.is_open() )
	{
		/*sf::Keyboard::Key up;
		sf::Keyboard::Key left;
		sf::Keyboard::Key down;
		sf::Keyboard::Key right;

		is >> up;
		is >> left;
		is >> down;
		is >> right;

		if( x > stickThresh )
			m_state.leftStickPad += 1 << 3;
		if( x < -stickThresh )
			m_state.leftStickPad += 1 << 2;
		if( y > stickThresh )
			m_state.leftStickPad += 1;
		if( y < -stickThresh )
			m_state.leftStickPad += 1 << 1;*/
	}
	else
	{
		cout << "file: " << fileName << endl;
		assert( "failed to load keyboard inputs" );
	}
}

KeyboardSettings::KeyboardSettings()
{
	using namespace sf;
	buttonMap[UP] = Keyboard::Up;
	buttonMap[LEFT] = Keyboard::Left;
	buttonMap[DOWN] = Keyboard::Down;
	buttonMap[RIGHT] = Keyboard::Right;

	buttonMap[JUMP] = Keyboard::Z;
	buttonMap[SLASH] = Keyboard::C;
	buttonMap[DASH] = Keyboard::X;

	buttonMap[BOUNCE] = Keyboard::F;
	buttonMap[GRIND] = Keyboard::D;
	buttonMap[TIMESLOW] = Keyboard::LShift;
	buttonMap[LEFTWIRE] = Keyboard::LControl;
	buttonMap[RIGHTWIRE] = Keyboard::RControl;
	buttonMap[MAP] = Keyboard::Tilde;
	buttonMap[PAUSE] = Keyboard::Delete;

	toggleBounce = false;
	toggleGrind = false;
	toggleTimeSlow = false;
	//SaveToFile( "defaultkeys" );
}

void KeyboardSettings::LoadFromFile( const std::string &fileName )
{
	ifstream is;
	is.open( fileName );

	if( is.is_open() )
	{
		for( int i = 0; i < ButtonType::Count; ++i )
		{
			int temp;
			is >> temp;

			buttonMap[i] = (sf::Keyboard::Key)temp;
		}
	}
	else
	{
		cout << "file: " << fileName << endl;
		assert( "failed to load keyboard inputs" );
	}

	int tBounce, tGrind, tSlow;
	is >> tBounce;
	is >> tGrind;
	is >> tSlow;

	toggleBounce = (bool)tBounce;
	toggleGrind = (bool)tGrind;
	toggleTimeSlow = (bool)tSlow;

	is.close();
}

void KeyboardSettings::SaveToFile( const std::string &fileName )
{
	ofstream of;
	of.open( fileName );

	if( of.is_open() )
	{
		for( int i = 0; i < ButtonType::Count; ++i )
		{
			of << buttonMap[i] << endl;
		}
	}
	else
	{
		cout << "FAILED TO SAVE KEYBOARD FILTER" << endl;
		assert( "failed to save keyboard inputs" );
	}

	
	if( toggleBounce )
	{
		of << 1 << endl;
	}
	else
	{
		of << 0 << endl;
	}

	if( toggleGrind )
	{
		of << 1 << endl;
	}
	else
	{
		of << 0 << endl;
	}

	if( toggleTimeSlow )
	{
		of << 1 << endl;
	}
	else
	{
		of << 0 << endl;
	}

	of.close();
}

void KeyboardSettings::Update( ControllerState &cs )
{
	/*if( x > stickThresh )
			m_state.leftStickPad += 1 << 3;
		if( x < -stickThresh )
			m_state.leftStickPad += 1 << 2;
		if( y > stickThresh )
			m_state.leftStickPad += 1;
		if( y < -stickThresh )
			m_state.leftStickPad += 1 << 1;*/

	bool up = sf::Keyboard::isKeyPressed( buttonMap[UP] );
	bool left = sf::Keyboard::isKeyPressed( buttonMap[LEFT] );
	bool down = sf::Keyboard::isKeyPressed( buttonMap[DOWN] );
	bool right = sf::Keyboard::isKeyPressed( buttonMap[RIGHT] );

	bool jump = sf::Keyboard::isKeyPressed( buttonMap[JUMP] );
	bool slash = sf::Keyboard::isKeyPressed( buttonMap[SLASH] );
	bool dash = sf::Keyboard::isKeyPressed( buttonMap[DASH] );

	bool bounce = sf::Keyboard::isKeyPressed( buttonMap[BOUNCE] );
	bool grind = sf::Keyboard::isKeyPressed( buttonMap[GRIND] );
	bool timeSlow = sf::Keyboard::isKeyPressed( buttonMap[TIMESLOW] );
	bool leftWire = sf::Keyboard::isKeyPressed( buttonMap[LEFTWIRE] );
	bool rightWire = sf::Keyboard::isKeyPressed( buttonMap[RIGHTWIRE] );

	bool map = sf::Keyboard::isKeyPressed( buttonMap[MAP] );
	bool pause = sf::Keyboard::isKeyPressed( buttonMap[PAUSE] );

	
	//if( up || left || down || right )
	//{
	//	cs.leftStickMagnitude = 1.0;
	//}
	//else
	//{
	//	cs.leftStickMagnitude = 0.0;
	//}
	//
	//if( up 
	//cs.leftStickRadians = atan( normalizedLY / normalizedLX );

	//if( normalizedLX < 0.0f )
	//	cs.leftStickRadians += PI;

	//cs.rightStickMagnitude = normalizedMagnitude;
	//cs.rightStickRadians = atan( normalizedRY / normalizedRX );
	//if( normalizedRX < 0.0f )
	//	cs.rightStickRadians += PI;

	//cs.leftTrigger = state.Gamepad.bLeftTrigger; //0 or 255
	//cs.rightTrigger = state.Gamepad.bRightTrigger;
	
}

void SetFilterDefault( XBoxButton *filter)
{
	filter[ControllerSettings::JUMP] = XBOX_A;
	filter[ControllerSettings::DASH] = XBOX_B;
	filter[ControllerSettings::ATTACK] = XBOX_R1;
	filter[ControllerSettings::BOUNCE] = XBOX_X;
	filter[ControllerSettings::GRIND] = XBOX_Y;
	filter[ControllerSettings::TIMESLOW] = XBOX_L1;
	filter[ControllerSettings::LEFTWIRE] = XBOX_L2;
	filter[ControllerSettings::RIGHTWIRE] = XBOX_R2;
	filter[ControllerSettings::MAP] = XBOX_BACK;
	filter[ControllerSettings::PAUSE] = XBOX_START;
}

std::string GetXBoxButtonString( XBoxButton button )
{
	switch( button )
	{
	case XBOX_A:
		return "A";
	case XBOX_B:
		return "B";
	case XBOX_X:
		return "X";
	case XBOX_Y:
		return "Y";
	case XBOX_R1:
		return "R1";
	case XBOX_L1:
		return "L1";
	case XBOX_R2:
		return "R2";
	case XBOX_L2:
		return "L2";
	default:
		assert( 0 );
		return "";
	}
}