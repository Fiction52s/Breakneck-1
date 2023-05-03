#ifndef __INPUT_H__
#define __INPUT_H__
#include <Windows.h>
#include <Xinput.h>
#include <map>
#include <SFML/Graphics.hpp>
#include "GCC/GCController.h"
#include "PS5Controller.h"
#include "GCC/USBDriver.h"
#include "GCC/VJoyGCController.h"
//true if down, false if up
//^^make this more efficient if I'm using it for networking
//  later

//

#pragma comment(lib, "Resources/XInput.lib")

#define COMPRESSED_INPUT_TYPE sf::Uint32
#define COMPRESSED_INPUT_SIZE sizeof(sf::Uint32)

//just the spot on the spritesheet thats blank for now. contrasts with XBOX_BLANK
//#define KEYBOARD_BLANK 75

/** Remarks:
For each button, its bool its true if the it is down 
(pressed) and false if it is up (not pressed).
*/

struct ControlProfile;

enum XBoxButton
{
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
	XBOX_PLEFT,
	XBOX_PRIGHT,
	XBOX_PUP,
	XBOX_PDOWN,
	XBOX_LLEFT,
	XBOX_LRIGHT,
	XBOX_LUP,
	XBOX_LDOWN,
	XBOX_RLEFT,
	XBOX_RRIGHT,
	XBOX_RUP,
	XBOX_RDOWN,
	XBOX_BLANK
};
std::string GetXBoxButtonString(int button);
std::string GetKeyboardButtonString(int key);
bool IsKeyValidForInput(int key);

struct KeyboardState
{
	bool m_state[sf::Keyboard::KeyCount];
	void Update(sf::RenderWindow *win);
	KeyboardState();
	void Clear();
};

struct ControllerState
{
	double leftStickMagnitude; // 0 - 1.0
	double leftStickRadians;
	double rightStickMagnitude; // 0 - 1.0
	double rightStickRadians;

	BYTE leftTrigger;
	BYTE rightTrigger;
	BYTE triggerThresh;
	unsigned char leftStickDirection;
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

	bool respawnTest;

	ControllerState();
	void Set(const ControllerState &state);
	void SetLeftDirection();
	int GetLeftStickDirection();
	
	sf::Vector2<double> GetLeft8Dir() const;
	sf::Vector2<double> GetStickVectorLeft() const;
	sf::Vector2<double> GetStickVectorRight() const;
	COMPRESSED_INPUT_TYPE GetCompressedState();
	void SetFromCompressedState(COMPRESSED_INPUT_TYPE s);
	void InvertLeftStick();
	void Clear();
	int CheckControllerButton(int b);

	bool IsLeftNeutral() const;
	bool IsRightNeutral() const;

	bool LeftTriggerPressed()  const;
	bool RightTriggerPressed()  const;

	bool LUp() const;
	bool LDown() const;
	bool LLeft() const;
	bool LRight() const;
	bool LPress() const;

	bool RUp() const;
	bool RDown() const;
	bool RLeft() const;
	bool RRight() const;
	bool RPress() const;

	bool PUp()  const;
	bool PDown() const;
	bool PLeft() const;
	bool PRight() const;

	bool PowerButtonDown() const;
	bool JumpButtonDown() const;
	bool DashButtonDown() const;
	bool AttackButtonDown() const;

	bool ConfirmButtonDown() const;
	bool BackButtonDown() const;


	//0x1 = up, 0x2 = down, 0x4 = left, 
				 //0x8 = right
	
};

struct ControllerSettings
{
	enum ButtonType
	{
		BUTTONTYPE_JUMP,
		BUTTONTYPE_DASH,
		BUTTONTYPE_ATTACK,
		BUTTONTYPE_SHIELD,
		BUTTONTYPE_SPECIAL,
		BUTTONTYPE_LEFTWIRE,
		BUTTONTYPE_RIGHTWIRE,
		BUTTONTYPE_PAUSE,
		BUTTONTYPE_LLEFT,
		BUTTONTYPE_LRIGHT,
		BUTTONTYPE_LUP,
		BUTTONTYPE_LDOWN,
		BUTTONTYPE_RLEFT,
		BUTTONTYPE_RRIGHT,
		BUTTONTYPE_RUP,
		BUTTONTYPE_RDOWN,
		BUTTONTYPE_Count
	};

	static void InitStrings();
	static const std::string &GetButtonTypeStr(ControllerSettings::ButtonType bType);
private:
	static std::string buttonTypeStrings[BUTTONTYPE_Count];
};



enum ControllerType
{
	CTYPE_XBOX,
	CTYPE_GAMECUBE,
	CTYPE_PS4,
	CTYPE_PS5,
	CTYPE_KEYBOARD,
	CTYPE_NONE,
	CTYPE_Count
};

/** Remarks:
Wrapper for XINPUT controller. Used to access the actual
controllers and generate state information for use in the 
game.
*/
struct MainMenu;
class GameController
{
public:
	sf::RenderWindow *window;
	//ButtonStick keyboardStickLeft;
	//ButtonStick keyboardStickRight;
	ControllerState m_state;
	static float stickThresh;
	GCC::GCController gcController;
	PS5Controller ps5Controller;
	sf::Vector2i gcDefaultControl;
	sf::Vector2i gcDefaultC;
	int gcDefaultLeftTrigger;
	int gcDefaultRightTrigger;

	GameController( DWORD index, ControllerType ct );
	bool IsConnected();
	ControllerType GetCType();
	DWORD GetIndex();
	bool UpdateState();
	ControllerState & GetState();
	int Pressed( XBoxButton b );
	int GetGCCLeftTrigger();
	int GetGCCRightTrigger();
	void UpdateLeftStickPad();
private:
	bool isConnected;
	DWORD m_index;
	ControllerType controllerType;

	const static DWORD LEFT_STICK_DEADZONE;
	const static DWORD RIGHT_STICK_DEADZONE;
	const static DWORD TRIGGER_THRESHOLD;
	const static double GC_LEFT_STICK_DEADZONE;
	const static double GC_RIGHT_STICK_DEADZONE;

	bool UpdateGCC();
	bool UpdateXBOX();
	bool UpdatePS5();
	bool UpdateKeyboard();
};

struct ControllerStateQueue
{
	std::vector<ControllerState> states;
	GameController *con;

	ControllerStateQueue(int size, GameController *con );
	void AddInput(ControllerState &s);
	int GetNumStates();
	bool ButtonHeld_A();
	bool ButtonPressed_A();
	bool ButtonHeld_B();
	bool ButtonPressed_B();
	bool ButtonHeld_X();
	bool ButtonPressed_X();
	bool ButtonHeld_Y();
	bool ButtonPressed_Y();
	bool ButtonHeld_LeftShoulder();
	bool ButtonPressed_LeftShoulder();
	bool ButtonHeld_RightShoulder();
	bool ButtonPressed_RightShoulder();
	bool ButtonHeld_LeftTrigger();
	bool ButtonPressed_LeftTrigger();
	bool ButtonHeld_RightTrigger();
	bool ButtonPressed_RightTrigger();
	bool ButtonHeld_Start();
	bool ButtonPressed_Start();
	bool ButtonHeld_Any();
	bool ButtonPressed_Any();
	bool DirPressed_Left();
	bool DirPressed_Right();
	bool DirPressed_Up();
	bool DirPressed_Down();
	bool IsDoingAnything();
	


	int GetControllerType();
	int GetIndex();
	const ControllerState &GetCurrState();
	const ControllerState &GetPrevState();
};



struct ControllerDualStateQueue : ControllerStateQueue
{
	ControllerDualStateQueue(GameController *con );
};

struct AllControllers
{
	static AllControllers &GetInstance()
	{
		static AllControllers instance;
		return instance;
	}
	AllControllers(AllControllers const&) = delete;
	void operator=(AllControllers const&) = delete;

	GameController * GetGCController(int index);
	GameController * GetWindowsController(int index);
	GameController * GetController(int cType, int index);

	ControllerDualStateQueue * GetStateQueue( int cType, int index);
	ControllerDualStateQueue * GetStateQueue(GameController *con);
	
	void Update();
	void CheckForControllers();
	void SetRenderWindow(sf::RenderWindow *rw);
	void SetKeyboardActiveAsController(bool keyActive);
	bool AltF4();



	bool ButtonHeld_A();
	bool ButtonPressed_A();
	bool ButtonHeld_B();
	bool ButtonPressed_B();
	bool ButtonHeld_X();
	bool ButtonPressed_X();
	bool ButtonHeld_Y();
	bool ButtonPressed_Y();
	bool ButtonHeld_LeftShoulder();
	bool ButtonPressed_LeftShoulder();
	bool ButtonHeld_RightShoulder();
	bool ButtonPressed_RightShoulder();
	bool ButtonHeld_Start();
	bool ButtonPressed_Start();
	bool ButtonHeld_Any();
	bool ButtonPressed_Any();
	bool DirPressed_Left();
	bool DirPressed_Right();
	bool DirPressed_Up();
	bool DirPressed_Down();
	bool KeyboardButtonPressed(int key);
	bool KeyboardButtonHeld(int key);
	bool KeyboardButtonHeldPrev(int key);
	void UpdateFilteredKeyboardState(ControlProfile *cp, ControllerState &state, const ControllerState &prevState);
	void UpdateFilteredKeyboardStick(ControlProfile *cp, bool rightStick, ControllerState &state, const ControllerState &prevState);
	void UpdateUnfilteredKeyboardState(ControllerState &state);
	int GetMenuKeyFromControllerButton(XBoxButton button);
private:
	void UpdateKeyboardStick( bool rightStick, ControllerState &state, const ControllerState &prevState, 
		bool left, bool right, bool up, bool down, 
		bool oldLeft, bool oldRight, bool oldUp, bool oldDown);

	bool isKeyboardActive;
	int menuKeyboardMap[XBOX_BLANK];

	KeyboardState prevKeyboard;
	KeyboardState currKeyboard;

	std::vector<GameController*> gcControllers;
	std::vector<GameController*> windowsControllers;
	GameController *keyboardController;

	std::vector<ControllerDualStateQueue*> gccStatesVec;
	std::vector<ControllerDualStateQueue*> windowsStatesVec;

	ControllerDualStateQueue *keyboardStates;

	GCC::USBDriver *gccDriver;
	GCC::VJoyGCControllers *joys;
	bool gccDriverEnabled;
	std::vector<GCC::GCController> rawGCControllers;
	sf::RenderWindow *window;

	PS5ControllerManager ps5ControllerManager;

	AllControllers();
	~AllControllers();

	void GCCUpdate();
};

#define CONTROLLERS AllControllers::GetInstance()

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