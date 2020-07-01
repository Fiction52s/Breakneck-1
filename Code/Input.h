#ifndef __INPUT_H__
#define __INPUT_H__
#include <Windows.h>
#include <Xinput.h>
#include <map>
#include <SFML/Graphics.hpp>
#include "GCC/GCController.h"

//true if down, false if up
//^^make this more efficient if I'm using it for networking
//  later

//

#pragma comment(lib, "Resources/XInput.lib")

/** Remarks:
For each button, its bool its true if the it is down 
(pressed) and false if it is up (not pressed).
*/

struct KeyboardState;
struct ControllerState
{
	ControllerState();
	void Set( const ControllerState &state );
	void SetLeftDirection();
	int GetCompressedState();
	void SetFromCompressedState(int s);
	double leftStickMagnitude; // 0 - 1.0
	double leftStickRadians;
	double rightStickMagnitude; // 0 - 1.0
	double rightStickRadians;
	BYTE leftTrigger;
	BYTE rightTrigger;
	bool LeftTriggerPressed();
	bool RightTriggerPressed();
	BYTE triggerThresh;
	void Clear();
	//bool leftTriggerPress;
	//bool rightTriggerPress;
	bool start;
	bool back;
	bool leftShoulder;
	bool rightShoulder;
	bool A;
	bool B;
	bool X;
	bool Y;
	bool leftPress;
	bool rightPress;
	unsigned char pad;
	unsigned char leftStickPad;
	unsigned char rightStickPad;
	bool LUp();
	bool LDown();
	bool LLeft();
	bool LRight();
	bool LPress();

	bool RUp();
	bool RDown();
	bool RLeft();
	bool RRight();
	bool RPress();

	bool PUp();
	bool PDown();
	bool PLeft();
	bool PRight();

	unsigned char leftStickDirection;
	//0x1 = up, 0x2 = down, 0x4 = left, 
				 //0x8 = right
	
};



struct KeyboardFilter
{
	KeyboardFilter();
	void SetKey( sf::Keyboard::Key old, 
		sf::Keyboard::Key newKey );
	bool LoadReplacements( const std::string &file );
	sf::Keyboard::Key Filter( sf::Keyboard::Key key );
	//std::map<sf::Keyboard::Key, sf::Keyboard::Key> filter;
	sf::Keyboard::Key keyFilter[sf::Keyboard::KeyCount];
};

struct KeyboardSettings
{
	enum ButtonType
	{
		UP,
		LEFT,
		DOWN,
		RIGHT,
		JUMP,
		SLASH,
		DASH,
		BOUNCE,
		GRIND,
		TIMESLOW,
		LEFTWIRE,
		RIGHTWIRE,
		MAP,
		PAUSE,
		Count
	};

	KeyboardSettings();
	sf::Keyboard::Key buttonMap[ButtonType::Count];
	void LoadFromFile( const std::string &fileName );
	void SaveToFile( const std::string &fileName );

	bool toggleBounce;
	bool toggleGrind;
	bool toggleTimeSlow;
};

enum XBoxButton
{
	XBOX_BLANK,
	XBOX_A,
	XBOX_B,
	XBOX_X,
	XBOX_Y,
	XBOX_R1,
	XBOX_R2,
	XBOX_L1,
	XBOX_L2,
	XBOX_START,
	XBOX_BACK,
	XBOX_PUP,
	XBOX_PDOWN,
	XBOX_PLEFT,
	XBOX_PRIGHT,
	XBOX_LUP,
	XBOX_LLEFT,
	XBOX_LRIGHT,
	XBOX_LDOWN,
	XBOX_RLEFT,
	XBOX_RUP,
	XBOX_RRIGHT,
	XBOX_RDOWN,
	XBOX_Count
};	
void SetFilterDefault( XBoxButton *filter);
void SetFilterDefaultGCC(XBoxButton *filter);
std::string GetXBoxButtonString( XBoxButton button );

struct ControllerSettings
{
	enum ButtonType	
	{
		JUMP,
		DASH,
		ATTACK,
		BOUNCE,
		GRIND,
		TIMESLOW,
		LEFTWIRE,
		RIGHTWIRE,
		MAP,
		PAUSE,
		Count
	};

	
};

enum ControllerType
{
	CTYPE_XBOX,
	CTYPE_GAMECUBE,
	CTYPE_PS4,
	CTYPE_KEYBOARD,
	CTYPE_NONE,
	CTYPE_Count
};

void LoadInputMapKeyboard( ControllerState &cs, 
	const std::string &fileName,
	KeyboardFilter &filter );

/** Remarks:
Wrapper for XINPUT controller. Used to access the actual
controllers and generate state information for use in the 
game.
*/
struct MainMenu;
class GameController
{
public:
	///index 0-3, corresponding to the different physical
	///controllers. 0 = top left light, 1 = top right light,
	///2 = bottom left light, 3 = bottom right light
	GameController( DWORD index);
	sf::RenderWindow *window;
	///Gets if the controller is plugged in or functioning
	bool IsKeyPressed(int k);
	bool IsConnected();
	ControllerType GetCType();
	//Gets the that was passed to the constructor
	DWORD GetIndex();
	//Queries and stores the current state of the physical 
	///controller
	bool UpdateState();
	///Gets the state most recently queried from the
	///controller. Used to update external ControllerStates
	ControllerState & GetState();
	ControllerState & GetUnfilteredState();
	void SetFilter( XBoxButton *buttons );
	int Pressed( XBoxButton b );
	int GetGCCLeftTrigger();
	int GetGCCRightTrigger();

	XBoxButton filter[ControllerSettings::Count];
	//ControllerState & GetKeyboardState(); //also updates
	ControllerState m_state;
	ControllerState m_unfilteredState;
	KeyboardSettings keySettings;
	static float stickThresh;
	GCC::GCController gcController;
	sf::Vector2i gcDefaultControl;
	sf::Vector2i gcDefaultC;
	int gcDefaultLeftTrigger;
	int gcDefaultRightTrigger;
private:
	DWORD m_index;
	
	ControllerType controllerType;

	const static DWORD LEFT_STICK_DEADZONE;
	const static DWORD RIGHT_STICK_DEADZONE;
	const static DWORD TRIGGER_THRESHOLD;
	const static double GC_LEFT_STICK_DEADZONE;
	const static double GC_RIGHT_STICK_DEADZONE;
};

//wButtons
/*XINPUT_GAMEPAD_DPAD_UP         0x00000001
XINPUT_GAMEPAD_DPAD_DOWN        0x00000002
XINPUT_GAMEPAD_DPAD_LEFT        0x00000004
XINPUT_GAMEPAD_DPAD_RIGHT       0x00000008
XINPUT_GAMEPAD_START            0x00000010
XINPUT_GAMEPAD_BACK             0x00000020
XINPUT_GAMEPAD_LEFT_THUMB       0x00000040
XINPUT_GAMEPAD_RIGHT_THUMB      0x00000080
XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
XINPUT_GAMEPAD_A                0x1000
XINPUT_GAMEPAD_B                0x2000
XINPUT_GAMEPAD_X                0x4000
XINPUT_GAMEPAD_Y                0x8000*/


#endif