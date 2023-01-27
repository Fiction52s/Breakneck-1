#include "Input.h"
#include <math.h>
#include <fstream>
#include <assert.h>
#include <iostream>
#include "VectorMath.h"
#include "MainMenu.h"
#include "Config.h"
#include "ControlProfile.h"

using namespace std;
using namespace sf;

#define PI 3.14159265359
const DWORD GameController::LEFT_STICK_DEADZONE = 8500;//7849;
const DWORD GameController::RIGHT_STICK_DEADZONE = 8689;
const DWORD GameController::TRIGGER_THRESHOLD = 30;
const double GameController::GC_LEFT_STICK_DEADZONE = .1;
const double GameController::GC_RIGHT_STICK_DEADZONE = .1;

ControllerState::ControllerState()
	:triggerThresh( 200 )
{
	Clear();
	
}

void ControllerState::Clear()
{
	leftStickMagnitude = 0;
	leftStickRadians = 0;
	rightStickMagnitude = 0;
	rightStickRadians = 0;
	leftTrigger = 0;
	rightTrigger = 0;
	start = false;
	back = false;
	leftShoulder = false;
	rightShoulder = false;
	A = false;
	B = false;
	X = false;
	Y = false;
	pad = 0;
	leftStickPad = 0;
	rightStickPad = 0;
	leftPress = false;
	rightPress = false;
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

int ControllerState::CheckControllerButton(int b)
{
	//b is XBoxButton
	switch (b)
	{
	case XBOX_A:
		return A;
	case XBOX_B:
		return B;
	case XBOX_X:
		return X;
	case XBOX_Y:
		return Y;
	case XBOX_R1:
		return rightShoulder;
	case XBOX_R2:
		return rightTrigger;
	case XBOX_L1:
		return leftShoulder;
	case XBOX_L2:
		return leftTrigger;
	case XBOX_START:
		return start;
	case XBOX_BACK:
		return back;
	}

	return 0;
}

COMPRESSED_INPUT_TYPE ControllerState::GetCompressedState()
{
	COMPRESSED_INPUT_TYPE s = 0;
	int bit = 0;
	s |= LUp() << bit++;
	s |= LDown() << bit++;
	s |= LLeft() << bit++;
	s |= LRight() << bit++;
	s |= RUp() << bit++;
	s |= RDown() << bit++;
	s |= RLeft() << bit++;
	s |= RRight() << bit++;
	s |= A << bit++;
	s |= B << bit++;
	s |= X << bit++;
	s |= Y << bit++;
	s |= leftShoulder << bit++;
	s |= rightShoulder << bit++;
	s |= LeftTriggerPressed() << bit++;
	s |= RightTriggerPressed() << bit++;
	//first 16 bits above^

	int leftStickDir = leftStickDirection;
	s |= (leftStickDir << bit++);

	/*s |= keyboardStickLeft.oldLeft << bit++;
	s |= keyboardStickLeft.oldRight << bit++;
	s |= keyboardStickLeft.oldUp << bit++;
	s |= keyboardStickLeft.oldDown << bit++;
	s |= keyboardStickLeft.oldGoingLeft << bit++;
	s |= keyboardStickLeft.oldGoingRight << bit++;
	s |= keyboardStickLeft.oldGoingUp << bit++;
	s |= keyboardStickLeft.oldGoingDown << bit++;

	s |= keyboardStickRight.oldLeft << bit++;
	s |= keyboardStickRight.oldRight << bit++;
	s |= keyboardStickRight.oldUp << bit++;
	s |= keyboardStickRight.oldDown << bit++;
	s |= keyboardStickRight.oldGoingLeft << bit++;
	s |= keyboardStickRight.oldGoingRight << bit++;
	s |= keyboardStickRight.oldGoingUp << bit++;
	s |= keyboardStickRight.oldGoingDown << bit++;*/

	return s;
}

//notably this doesnt give you the analog stick values
void ControllerState::SetFromCompressedState(COMPRESSED_INPUT_TYPE s)
{
	Clear();

	int bit = 0;
	bool lup = s & (1 << bit++);
	bool ldown = s & (1 << bit++);
	bool lleft = s & (1 << bit++);
	bool lright = s & (1 << bit++);

	bool rup = s & (1 << bit++);
	bool rdown = s & (1 << bit++);
	bool rleft = s & (1 << bit++);
	bool rright = s & (1 << bit++);
	A = s & (1 << bit++);
	B = s & (1 << bit++);
	X = s & (1 << bit++);
	Y = s & (1 << bit++);
	leftShoulder = s & (1 << bit++);
	rightShoulder = s & (1 << bit++);
	if (s & (1 << bit++))
	{
		leftTrigger = 255;
	}
	if (s & (1 << bit++))
	{
		rightTrigger = 255;
	}

	if (lright)
	{
		leftStickPad += 1 << 3;
	}
	else if (lleft)
	{
		leftStickPad += 1 << 2;
	}

	if (lup)
	{
		leftStickPad += 1;
	}
	else if (ldown)
	{
		leftStickPad += 1 << 1;
	}

	if (rright)
	{
		rightStickPad += 1 << 3;
	}
	else if (rleft)
	{
		rightStickPad += 1 << 2;
	}

	if (rup)
	{
		rightStickPad += 1;
	}
	else if (rdown)
	{
		rightStickPad += 1 << 1;
	}

	/*int leftDir = (s >> bit) | ((int)pow( 2, sizeof( leftStickDirection) ) - 1);
	bit += sizeof(leftStickDirection);
	leftStickDirection = leftDir;*/

	leftStickDirection = (s >> bit) | ((int)pow(2, sizeof(leftStickDirection)) - 1);
	bit += sizeof(leftStickDirection);

	/*keyboardStickLeft.oldLeft = s & (1 << bit++);
	keyboardStickLeft.oldRight = s & (1 << bit++);
	keyboardStickLeft.oldUp = s & (1 << bit++);
	keyboardStickLeft.oldDown = s & (1 << bit++);
	keyboardStickLeft.oldGoingLeft = s & (1 << bit++);
	keyboardStickLeft.oldGoingRight = s & (1 << bit++);
	keyboardStickLeft.oldGoingUp = s & (1 << bit++);
	keyboardStickLeft.oldGoingDown = s & (1 << bit++);

	keyboardStickRight.oldLeft = s & (1 << bit++);
	keyboardStickRight.oldRight = s & (1 << bit++);
	keyboardStickRight.oldUp = s & (1 << bit++);
	keyboardStickRight.oldDown = s & (1 << bit++);
	keyboardStickRight.oldGoingLeft = s & (1 << bit++);
	keyboardStickRight.oldGoingRight = s & (1 << bit++);
	keyboardStickRight.oldGoingUp = s & (1 << bit++);
	keyboardStickRight.oldGoingDown = s & (1 << bit++);*/

	//current total is 16 + 8 + 8 + 8
}


bool ControllerState::IsLeftNeutral() const
{
	return leftStickPad == 0;
}

bool ControllerState::IsRightNeutral() const
{
	return rightStickPad == 0;
}

void ControllerState::SetLeftDirection()
{
	int aimingPrimaryAngleRange = 2;
	if (leftStickMagnitude > 0)
	{
		double angle = leftStickRadians;

		double degs = angle / PI * 180.0;
		double sec = 360.0 / 64.0;
		int mult = floor((degs / sec) + .5);

		if (mult < 0)
		{
			mult += 64;
		}

		//dude wtf this is awful LOL fix this when you can
		int test;
		int bigger, smaller;
		for (int i = 0; i < aimingPrimaryAngleRange; ++i)
		{
			test = i + 1;
			for (int j = 0; j < 64; j += 16)
			{
				bigger = mult + test;
				smaller = mult - test;
				if (smaller < 0)
					smaller += 64;
				if (bigger >= 64)
					bigger -= 64;

				if (bigger == j || smaller == j)
				{
					mult = j;
				}
			}
		}

		leftStickDirection = mult;
		//angle = (PI / 32.0) * mult;

		//cout << "mult: " << mult << endl;

		//fireDir.x = cos(angle);
		//fireDir.y = -sin(angle);
	}
	else
	{
		leftStickDirection = 65;
	}
}

bool ControllerState::PowerButtonDown() const
{
	return leftShoulder;
}

bool ControllerState::JumpButtonDown() const
{
	return A;
}

bool ControllerState::DashButtonDown() const
{
	return X;
}

bool ControllerState::ConfirmButtonDown() const
{
	return A;
}

bool ControllerState::BackButtonDown() const
{
	return B;
}

bool ControllerState::AttackButtonDown() const
{
	return rightShoulder;
}

bool ControllerState::PUp() const
{
	return pad & 1;
}

bool ControllerState::PDown() const
{
	return pad & 2;
}

bool ControllerState::PLeft() const
{
	return pad & 4;
}

bool ControllerState::PRight() const
{
	return pad & 8;
}

bool ControllerState::LUp() const
{
	return leftStickPad & 1;
}

bool ControllerState::LDown() const
{
	return leftStickPad & 2;
}

bool ControllerState::LLeft() const
{
	return leftStickPad & 4;
}

bool ControllerState::LRight() const
{
	return leftStickPad & 8;
}

bool ControllerState::LPress() const
{
	return leftPress;
}

bool ControllerState::RUp() const
{
	return rightStickPad & 1;
}

bool ControllerState::RDown() const
{
	return rightStickPad & 2;
}

bool ControllerState::RLeft() const
{
	return rightStickPad & 4;
}

bool ControllerState::RRight() const
{
	return rightStickPad & 8;
}

bool ControllerState::RPress() const
{
	return rightPress;
}

bool ControllerState::RightTriggerPressed() const
{
	return rightTrigger >= triggerThresh;
}

bool ControllerState::LeftTriggerPressed() const
{
	return leftTrigger >= triggerThresh;
}

sf::Vector2<double> ControllerState::GetLeft8Dir() const
{
	sf::Vector2<double> dir(0, 0);
	if (LLeft())
	{
		dir.x = -1;
	}
	else if (LRight())
	{
		dir.x = 1;
	}

	if (LUp())
	{
		dir.y = -1;
	}
	else if (LDown())
	{
		dir.y = 1;
	}

	return normalize(dir);
}

sf::Vector2<double> ControllerState::GetStickVectorLeft() const
{
	float x = cos(leftStickRadians) * leftStickMagnitude;
	float y = -sin(leftStickRadians) * leftStickMagnitude;
	return V2d(x, y);
}

sf::Vector2<double> ControllerState::GetStickVectorRight() const
{
	float x = cos(rightStickRadians) * rightStickMagnitude;
	float y = -sin(rightStickRadians) * rightStickMagnitude;
	return V2d(x, y);
}

void ControllerState::InvertLeftStick()
{
	//int oldLeftStickPad = leftStickPad;
	int newLeftStickPad = 0;

	if (!IsLeftNeutral())
	{
		leftStickDirection += 32;
		if (leftStickDirection > 64)
		{
			leftStickDirection -= 64;
		}

		if (LLeft())
		{
			newLeftStickPad += 1 << 3;
		}
		else if (LRight())
		{
			newLeftStickPad += 1 << 2;
		}
		if (LUp())
		{
			newLeftStickPad += 1 << 1;
		}
		else if (LDown())
		{
			newLeftStickPad += 1;
		}

		leftStickPad = newLeftStickPad;
	}

	/*leftStickPad = 0;
	if (leftStickMagnitude > GameController::stickThresh)
	{
		leftStickRadians += PI;
		if (leftStickRadians >= 2 * PI)
		{
			leftStickRadians -= 2 * PI;
		}

		float x = cos(leftStickRadians);
		float y = sin(leftStickRadians);

		if (x > GameController::stickThresh)
			leftStickPad += 1 << 3;
		if (x < -GameController::stickThresh)
			leftStickPad += 1 << 2;
		if (y > GameController::stickThresh)
			leftStickPad += 1;
		if (y < -GameController::stickThresh)
			leftStickPad += 1 << 1;
	}*/
}

bool GameController::IsKeyPressed(int k)
{
	Keyboard::Key key = (Keyboard::Key)k;

	if (window == NULL)
		return false;
//	assert(window != NULL);

	if (window->hasFocus())
	{
		return Keyboard::isKeyPressed(key);
	}

	return false;
}

ButtonStick::ButtonStick()
{
	Reset();
}
void ButtonStick::Reset()
{
	oldLeft = false;
	oldRight = false;
	oldUp = false;
	oldDown = false;
	//oldStickVec = Vector2<double>(0, 0);
}

sf::Vector2<double> ButtonStick::UpdateStickVec(bool left, bool right, bool up, bool down)
{
	V2d stickVec;

	bool leftPress = left && !oldLeft;
	bool rightPress = right && !oldRight;
	bool upPress = up && !oldUp;
	bool downPress = down && !oldDown;

	if (left && !right)
	{
		stickVec.x = -1;
	}
	else if (!left && right)
	{
		stickVec.x = 1;
	}
	else if (left && right)
	{
		if (leftPress && !rightPress )
		{
			stickVec.x = -1;
		}
		else if (!leftPress && rightPress)
		{
			stickVec.x = 1;
		}
		else if (leftPress && rightPress)
		{
			stickVec.x = 0;
		}
		else
		{
			//stickVec.x = oldGoing.x;
		}
	}
	else
	{
		stickVec.x = 0;
	}

	if (up && !down)
	{
		stickVec.y = -1;
	}
	else if (!up && down)
	{
		stickVec.y = 1;
	}
	else if (up && down)
	{
		if (upPress && !downPress)
		{
			stickVec.y = -1;
		}
		else if (!upPress && downPress)
		{
			stickVec.y = 1;
		}
		else if (upPress && downPress)
		{
			stickVec.y = 0;
		}
		else
		{
			//stickVec.y = oldGoing.y;
		}
	}
	else
	{
		stickVec.y = 0;
	}

	oldLeft = left;
	oldRight = right;
	oldUp = up;
	oldDown = down;
	//oldGoing.x = stickVec.x;
	//oldGoing.y = stickVec.y;

	return stickVec;
}

int GameController::GetGCCLeftTrigger()
{
	int lAxis = gcController.axis.l_axis;
	if (gcDefaultLeftTrigger < 0)
	{
		gcDefaultLeftTrigger = lAxis;
	}

	int trueL = (lAxis - gcDefaultLeftTrigger) * 2;
	if (trueL > 255)
		trueL = 255;
	if (trueL < 0)
		trueL = 0;

	return trueL;
}

int GameController::GetGCCRightTrigger()
{
	int rAxis = gcController.axis.r_axis;
	if (gcDefaultRightTrigger < 0)
	{
		gcDefaultRightTrigger = rAxis;
	}

	int trueR = (rAxis - gcDefaultRightTrigger) * 2;
	if (trueR > 255)
		trueR = 255;
	if (trueR < 0)
		trueR = 0;

	return trueR;
}

void GameController::UpdateLeftStickPad()
{
	m_state.leftStickPad = 0;
	if (m_state.leftStickMagnitude > stickThresh)
	{
		/*m_state.leftStickRadians += PI;
		if (m_state.leftStickRadians >= 2 * PI)
		{
			m_state.leftStickRadians -= 2 * PI;
		}*/

		//cout << "left stick radians: " << currInput.leftStickRadians << endl;
		float x = cos(m_state.leftStickRadians);
		float y = sin(m_state.leftStickRadians);

		if (x > stickThresh)
			m_state.leftStickPad += 1 << 3;
		if (x < -stickThresh)
			m_state.leftStickPad += 1 << 2;
		if (y > stickThresh)
			m_state.leftStickPad += 1;
		if (y < -stickThresh)
			m_state.leftStickPad += 1 << 1;
	}
}

bool GameController::UpdatePS5()
{
	if (!ps5Controller.enabled)
	{
		return false;
	}

	ps5Controller.UpdateState();

	ControllerState tempState;

	//controllerType = ControllerType::CTYPE_PS5;
	
	/*int lTrigger = GetGCCLeftTrigger();
	int rTrigger = GetGCCRightTrigger();

	Vector2i left(gcController.axis.left_x - gcDefaultControl.x,
		gcController.axis.left_y - gcDefaultControl.y);*/

	char leftX = ps5Controller.inState.leftStick.x;
	char leftY = ps5Controller.inState.leftStick.y;

	if (leftX < -127)
	{
		leftX = -127;
	}
	if (leftY < -127)
	{
		leftY = -127;
	}

	double LX = leftX / 127.0;
	double LY = leftY / 127.0;

	double magnitude = sqrt(LX * LX + LY * LY);

	double normalizedMagnitude = 0;

	if (magnitude > PS5Controller::deadZone)//LEFT_STICK_DEADZONE)
	{
		double normalizedLX = LX / magnitude;
		double normalizedLY = LY / magnitude;
		if (magnitude > 1.0)
			magnitude = 1.0;

		normalizedMagnitude = magnitude;

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


	char rightX = ps5Controller.inState.rightStick.x;
	char rightY = ps5Controller.inState.rightStick.y;

	if (rightX < -127)
	{
		rightX = -127;
	}
	if (rightY < -127)
	{
		rightY = -127;
	}

	double RX = rightX / 127.0;
	double RY = rightY / 127.0;

	magnitude = sqrt(RX * RX + RY * RY);

	double normalizedRX = RX / magnitude;
	double normalizedRY = RY / magnitude;

	if (magnitude > PS5Controller::deadZone)
	{
		if (magnitude > 1.0)
			magnitude = 1.0;

		//magnitude -= RIGHT_STICK_DEADZONE;
		normalizedMagnitude = magnitude;// / (32767 - RIGHT_STICK_DEADZONE);
	}
	else
	{
		magnitude = 0.0f;
		normalizedMagnitude = 0.0f;
	}

	m_state.rightStickMagnitude = normalizedMagnitude;
	m_state.rightStickRadians = atan(normalizedRY / normalizedRX);
	if (normalizedRX < 0.0f)
		m_state.rightStickRadians += PI;


	m_state.start = ps5Controller.inState.buttonsA & DS5W_ISTATE_BTN_A_MENU;
	m_state.back = ps5Controller.inState.buttonsA & DS5W_ISTATE_BTN_A_SELECT;
	m_state.leftShoulder = ps5Controller.inState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_BUMPER;
	m_state.rightShoulder = ps5Controller.inState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_BUMPER;
	m_state.A = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_BTX_CROSS;
	m_state.B = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_BTX_CIRCLE;
	m_state.X = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_BTX_SQUARE;
	m_state.Y = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_BTX_TRIANGLE;
	m_state.leftPress = false;//b & XINPUT_GAMEPAD_LEFT_THUMB;
	m_state.rightPress = false;//b & XINPUT_GAMEPAD_RIGHT_THUMB;

	int dpadUp = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_DPAD_UP;
	int dpadLeft = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_DPAD_LEFT;
	int dpadRight = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_DPAD_RIGHT;
	int dpadDown = ps5Controller.inState.buttonsAndDpad & DS5W_ISTATE_DPAD_DOWN;

	m_state.pad = dpadUp + (dpadDown << 1) + (dpadLeft << 2) + (dpadRight << 3);//gcController.buttons.dpad_up //( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 );

	m_state.leftTrigger = ps5Controller.inState.leftTrigger;
	m_state.rightTrigger = ps5Controller.inState.rightTrigger;

	//can use analog triggers for the triggers here now.


	UpdateLeftStickPad();

	m_state.rightStickPad = 0;
	if (m_state.rightStickMagnitude > stickThresh)
	{
		//cout << "left stick radians: " << m_state.leftStickRadians << endl;
		float x = cos(m_state.rightStickRadians);
		float y = sin(m_state.rightStickRadians);

		if (x > stickThresh)
			m_state.rightStickPad += 1 << 3;
		if (x < -stickThresh)
			m_state.rightStickPad += 1 << 2;
		if (y > stickThresh)
			m_state.rightStickPad += 1;
		if (y < -stickThresh)
			m_state.rightStickPad += 1 << 1;
	}

	//m_unfilteredState = m_state;
	//tempState = m_state;

	//tempState.A = Pressed(filter[ControllerSettings::BUTTONTYPE_JUMP]);
	//tempState.B = Pressed(filter[ControllerSettings::BUTTONTYPE_SPECIAL]);
	//tempState.rightShoulder = Pressed(filter[ControllerSettings::BUTTONTYPE_ATTACK]);
	//tempState.X = Pressed(filter[ControllerSettings::BUTTONTYPE_DASH]);
	//tempState.Y = false;//Pressed(filter[ControllerSettings::GRIND]);
	//tempState.leftShoulder = Pressed(filter[ControllerSettings::BUTTONTYPE_SHIELD]);
	//tempState.leftTrigger = Pressed(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]);
	//tempState.rightTrigger = Pressed(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]);
	//tempState.start = Pressed(filter[ControllerSettings::BUTTONTYPE_PAUSE]);

	//m_state = tempState;

	m_state.SetLeftDirection();

	return true;
}

bool GameController::UpdateGCC()
{
	if (!gcController.enabled)
	{
		return false;
	}
	//if (== INPUTFORMAT_GAMECUBE && gcController.enabled)
	ControllerState tempState;

	//controllerType = ControllerType::CTYPE_GAMECUBE;
	if (gcDefaultControl.x < 0)
	{
		gcDefaultControl.x = gcController.axis.left_x;
		gcDefaultControl.y = gcController.axis.left_y;
		gcDefaultC.x = gcController.axis.right_x;
		gcDefaultC.y = gcController.axis.right_y;
	}


	int lTrigger = GetGCCLeftTrigger();
	int rTrigger = GetGCCRightTrigger();

	Vector2i left(gcController.axis.left_x - gcDefaultControl.x,
		gcController.axis.left_y - gcDefaultControl.y);

	double LX = left.x / 127.0;// - 127.0;//gcController.axis.left_x//gcController.axis.left_x//state.Gamepad.sThumbLX;
	double LY = left.y / 127.0;// - 127.0;//state.Gamepad.sThumbLY;
								//i dont think i need a magnitude, magnitude should be calculated for differences
								//between controller states by some other function

	double magnitude = sqrt(LX * LX + LY * LY);
	//cout << "lx: " << LX << ", ly: " << LY << ", mag: " << magnitude << endl;

	double normalizedMagnitude = 0;

	if (magnitude > GC_LEFT_STICK_DEADZONE)//LEFT_STICK_DEADZONE)
	{
		double normalizedLX = LX / magnitude;
		double normalizedLY = LY / magnitude;
		if (magnitude > 1.0)
			magnitude = 1.0;

		//magnitude -= GC_LEFT_STICK_DEADZONE;//.01;LEFT_STICK_DEADZONE;
		normalizedMagnitude = magnitude;//magnitude / (32767 - LEFT_STICK_DEADZONE);


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


	Vector2i right(gcController.axis.right_x - gcDefaultC.x,
		gcController.axis.right_y - gcDefaultC.y);

	double RX = right.x / 127.0;
	double RY = right.y / 127.0;

	magnitude = sqrt(RX * RX + RY * RY);

	double normalizedRX = RX / magnitude;
	double normalizedRY = RY / magnitude;

	if (magnitude > GC_RIGHT_STICK_DEADZONE)
	{
		if (magnitude > 1.0)
			magnitude = 1.0;

		//magnitude -= RIGHT_STICK_DEADZONE;
		normalizedMagnitude = magnitude;// / (32767 - RIGHT_STICK_DEADZONE);
	}
	else
	{
		magnitude = 0.0f;
		normalizedMagnitude = 0.0f;
	}

	m_state.rightStickMagnitude = normalizedMagnitude;
	m_state.rightStickRadians = atan(normalizedRY / normalizedRX);
	if (normalizedRX < 0.0f)
		m_state.rightStickRadians += PI;


	m_state.start = gcController.buttons.start;
	m_state.back = false;
	m_state.leftShoulder = false;
	m_state.rightShoulder = gcController.buttons.z;
	m_state.A = gcController.buttons.a;
	m_state.B = gcController.buttons.b;
	m_state.X = gcController.buttons.x;
	m_state.Y = gcController.buttons.y;
	m_state.leftPress = false;//b & XINPUT_GAMEPAD_LEFT_THUMB;
	m_state.rightPress = false;//b & XINPUT_GAMEPAD_RIGHT_THUMB;

	int dpadUp = gcController.buttons.dpad_up;
	int dpadLeft = gcController.buttons.dpad_left;
	int dpadRight = gcController.buttons.dpad_right;
	int dpadDown = gcController.buttons.dpad_down;

	m_state.pad = dpadUp + (dpadDown << 1) + (dpadLeft << 2) + (dpadRight << 3);//gcController.buttons.dpad_up //( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 );

	m_state.leftTrigger = 255 * (int)(gcController.buttons.l_shoulder);
	m_state.rightTrigger = 255 * (int)(gcController.buttons.r_shoulder);

	//can use analog triggers for the triggers here now.


	UpdateLeftStickPad();

	m_state.rightStickPad = 0;
	if (m_state.rightStickMagnitude > stickThresh)
	{
		//cout << "left stick radians: " << m_state.leftStickRadians << endl;
		float x = cos(m_state.rightStickRadians);
		float y = sin(m_state.rightStickRadians);

		if (x > stickThresh)
			m_state.rightStickPad += 1 << 3;
		if (x < -stickThresh)
			m_state.rightStickPad += 1 << 2;
		if (y > stickThresh)
			m_state.rightStickPad += 1;
		if (y < -stickThresh)
			m_state.rightStickPad += 1 << 1;
	}


	//m_unfilteredState = m_state;
	//tempState = m_state;

	//tempState.A = Pressed(filter[ControllerSettings::BUTTONTYPE_JUMP]);
	//tempState.B = Pressed(filter[ControllerSettings::BUTTONTYPE_SPECIAL]);
	//tempState.rightShoulder = Pressed(filter[ControllerSettings::BUTTONTYPE_ATTACK]);
	//tempState.X = Pressed(filter[ControllerSettings::BUTTONTYPE_DASH]);
	//tempState.Y = false;//Pressed(filter[ControllerSettings::GRIND]);
	//tempState.leftShoulder = Pressed(filter[ControllerSettings::BUTTONTYPE_SHIELD]);
	//tempState.leftTrigger = Pressed(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]);
	//tempState.rightTrigger = Pressed(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]);
	//tempState.start = Pressed(filter[ControllerSettings::BUTTONTYPE_PAUSE]);

	//m_state = tempState;

	m_state.SetLeftDirection();

	return true;
}

bool GameController::UpdateXBOX()
{
	ControllerState tempState;
	DWORD result = -1;

	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	result = XInputGetState(m_index, &state);

	if (result == ERROR_SUCCESS)
	{
		//controllerType = ControllerType::CTYPE_XBOX;
		//cout << "updating controller state " << m_index << endl;
		double LX = state.Gamepad.sThumbLX;
		double LY = state.Gamepad.sThumbLY;
		//i dont think i need a magnitude, magnitude should be calculated for differences
		//between controller states by some other function
		double magnitude = sqrt(LX * LX + LY * LY);

		double normalizedLX = LX / magnitude;
		double normalizedLY = LY / magnitude;

		double normalizedMagnitude = 0;

		if (magnitude > LEFT_STICK_DEADZONE)
		{
			if (magnitude > 32767)
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

		magnitude = sqrt(RX * RX + RY * RY);

		double normalizedRX = RX / magnitude;
		double normalizedRY = RY / magnitude;

		if (magnitude > RIGHT_STICK_DEADZONE)
		{
			if (magnitude > 32767)
				magnitude = 32767;

			magnitude -= RIGHT_STICK_DEADZONE;
			normalizedMagnitude = magnitude / (32767 - RIGHT_STICK_DEADZONE);
		}
		else
		{
			magnitude = 0.0f;
			normalizedMagnitude = 0.0f;
		}

		m_state.rightStickMagnitude = normalizedMagnitude;
		m_state.rightStickRadians = atan(normalizedRY / normalizedRX);
		if (normalizedRX < 0.0f)
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
		m_state.pad = (b & 1) | (b & 2) | (b & 4) | (b & 8);

		m_state.leftStickPad = 0;
		m_state.rightStickPad = 0;
		if (m_state.leftStickMagnitude > stickThresh)
		{
			//cout << "left stick radians: " << currInput.leftStickRadians << endl;
			float x = cos(m_state.leftStickRadians);
			float y = sin(m_state.leftStickRadians);

			if (x > stickThresh)
				m_state.leftStickPad += 1 << 3;
			if (x < -stickThresh)
				m_state.leftStickPad += 1 << 2;
			if (y > stickThresh)
				m_state.leftStickPad += 1;
			if (y < -stickThresh)
				m_state.leftStickPad += 1 << 1;
		}

		if (m_state.rightStickMagnitude > stickThresh)
		{
			//cout << "left stick radians: " << m_state.leftStickRadians << endl;
			float x = cos(m_state.rightStickRadians);
			float y = sin(m_state.rightStickRadians);

			if (x > stickThresh)
				m_state.rightStickPad += 1 << 3;
			if (x < -stickThresh)
				m_state.rightStickPad += 1 << 2;
			if (y > stickThresh)
				m_state.rightStickPad += 1;
			if (y < -stickThresh)
				m_state.rightStickPad += 1 << 1;
		}

		//m_unfilteredState = m_state;
		//tempState = m_state;
		//

		//tempState.A = Pressed(filter[ControllerSettings::BUTTONTYPE_JUMP]);
		//tempState.B = Pressed(filter[ControllerSettings::BUTTONTYPE_SPECIAL]);
		//tempState.rightShoulder = Pressed(filter[ControllerSettings::BUTTONTYPE_ATTACK]);
		//tempState.X = Pressed(filter[ControllerSettings::BUTTONTYPE_DASH]);
		//tempState.Y = false;//Pressed(filter[ControllerSettings::GRIND]);
		//tempState.leftShoulder = Pressed(filter[ControllerSettings::BUTTONTYPE_SHIELD]);
		//tempState.leftTrigger = Pressed(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]);
		//tempState.rightTrigger = Pressed(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]);
		//tempState.start = Pressed(filter[ControllerSettings::BUTTONTYPE_PAUSE]);

		//m_state = tempState;

		m_state.SetLeftDirection();

		return true;
	}

	return false;
}

bool GameController::UpdateKeyboard()
{
	ControllerState tempState;
	if (m_index == 0)
	{
		//controllerType = ControllerType::CTYPE_KEYBOARD; //change to keyboard soon
		//cout << "updating controller state keyboard " << m_index << endl;
		using namespace sf;

		//WORD b = state.Gamepad.wButtons;

		ControlProfile cp;
		cp.SetControllerType(CTYPE_KEYBOARD);
		cp.SetFilterKeyboardMenuDefault();
		
		/*m_state.A = IsKeyPressed(Keyboard::Z);
		m_state.B = IsKeyPressed(Keyboard::A);
		m_state.rightShoulder = IsKeyPressed(Keyboard::C);
		m_state.X = IsKeyPressed(Keyboard::X);
		m_state.Y = IsKeyPressed(Keyboard::V);
		m_state.leftShoulder = 
		m_state.leftTrigger = 
		m_state.rightTrigger = 
		m_state.back = 
		m_state.start = IsKeyPressed(Keyboard::V);*/

		for (int i = 0; i < Keyboard::KeyCount; ++i)
		{
			m_keyboardState.m_state[i] = IsKeyPressed(i);
		}

		m_state.A = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_JUMP)];
		m_state.B = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_SPECIAL)];
		m_state.rightShoulder = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_ATTACK)];
		m_state.X = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_DASH)];
		m_state.Y = false;//IsKeyPressed(keySettings.buttonMap[KeyboardSettings::GRIND]);
		m_state.leftShoulder = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_SHIELD)];
		m_state.leftTrigger = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_LEFTWIRE)];
		m_state.rightTrigger = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_RIGHTWIRE)];
		m_state.start = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_PAUSE)];
		
		m_state.leftPress = false;//b & XINPUT_GAMEPAD_LEFT_THUMB;
		m_state.rightPress = false;//b & XINPUT_GAMEPAD_RIGHT_THUMB;
		m_state.pad = 0;//( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 );

		//CONTROLLERS.UpdateStateKeyboard( )

		bool up = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_LUP)];
		bool down = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_LDOWN)];
		bool left = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_LLEFT)];
		bool right = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_LRIGHT)];

		V2d stickVec = keyboardStickLeft.UpdateStickVec(left, right, up, down);
		if (length(stickVec) > 0)
		{
			m_state.leftStickMagnitude = 1;
		}
		else
		{
			m_state.leftStickMagnitude = 0;
		}
			
		stickVec = normalize(stickVec);

		double angle = -atan2(stickVec.y, stickVec.x);

		m_state.leftStickRadians = angle;


		m_state.leftStickPad = 0;

		if (stickVec.x > 0 )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1 << 3;
			//cout << "RIGHT" << endl;
		}
		else if (stickVec.x < 0 )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1 << 2;
			//cout << "LEFT" << endl;
		}

		if (stickVec.y < 0 )
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1;
			//cout << "UP" << endl;
		}
		else if (stickVec.y > 0)
		{
			m_state.leftStickMagnitude = 1.0;
			m_state.leftStickPad += 1 << 1;
			//cout << "DOWN" << endl;
		}


		bool rup = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_RUP)];
		bool rdown = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_RDOWN)];
		bool rleft = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_RLEFT)];
		bool rright = m_keyboardState.m_state[cp.Filter(ControllerSettings::BUTTONTYPE_RRIGHT)];

		stickVec = keyboardStickRight.UpdateStickVec(rleft, rright, rup, rdown);
		if (length(stickVec) > 0)
		{
			m_state.rightStickMagnitude = 1;
		}
		else
		{
			m_state.rightStickMagnitude = 0;
		}

		stickVec = normalize(stickVec);

		angle = -atan2(stickVec.y, stickVec.x);

		m_state.rightStickRadians = angle;


		m_state.rightStickPad = 0;

		if (stickVec.x > 0)
		{
			m_state.rightStickMagnitude = 1.0;
			m_state.rightStickPad += 1 << 3;
			//cout << "RIGHT" << endl;
		}
		else if (stickVec.x < 0)
		{
			m_state.rightStickMagnitude = 1.0;
			m_state.rightStickPad += 1 << 2;
			//cout << "LEFT" << endl;
		}

		if (stickVec.y < 0)
		{
			m_state.rightStickMagnitude = 1.0;
			m_state.rightStickPad += 1;
			//cout << "UP" << endl;
		}
		else if (stickVec.y > 0)
		{
			m_state.rightStickMagnitude = 1.0;
			m_state.rightStickPad += 1 << 1;
			//cout << "DOWN" << endl;
		}

		return true;
	}

	return false;
}

bool GameController::UpdateState()
{
	bool res = false;
	ControllerState tempState;

	switch (controllerType)
	{
	case ControllerType::CTYPE_GAMECUBE:
		res = UpdateGCC();
		break;
	case ControllerType::CTYPE_XBOX:
		res = UpdateXBOX();
		break;
	case ControllerType::CTYPE_PS5:
		res = UpdatePS5();
		break;
	case ControllerType::CTYPE_KEYBOARD:
		res = UpdateKeyboard();
		break;
	}

	isConnected = res;

	return res;
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

	return 0;
}

float GameController::stickThresh = .45;//.4;
GameController::GameController( DWORD index, ControllerType ct )
	:m_index( index ), window( NULL )
{
	controllerType = ct;
	gcDefaultControl.x = -1;
	gcDefaultControl.y = -1;
	gcDefaultC.x = -1;
	gcDefaultC.y = -1;

	gcDefaultLeftTrigger = -1;
	gcDefaultRightTrigger = -1;

	isConnected = false;

	//UpdateState();
	/*for( int i = 0; i < ControllerSettings::Count; ++i )
	{
		filter[i] = (XBoxButton)(i+1);
	}*/
	//SetFilterDefault( filter );
	//SetFilterDefaultGCC();
}

ControllerState & GameController::GetState()
{
	return m_state;
}

ControllerType GameController::GetCType()
{
	return controllerType;
}


bool GameController::IsConnected()
{
	return isConnected;
	/*if (controllerType != CTYPE_NONE)
	{
		return true;
	}
	else
	{
		return false;
	}*/
}


DWORD GameController::GetIndex()
{
	return m_index;
}


std::string GetXBoxButtonString( int button )
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


std::string GetKeyboardButtonString(int key)
{
	char c = 0;
	string str;

	if (key >= Keyboard::A && key <= Keyboard::Z)
	{
		c = ( key - Keyboard::A ) + 'A';
		str.push_back(c);
		return str;
	}
	else if (key >= Keyboard::Num0 && key <= Keyboard::Num9)
	{
		c = ( key - Keyboard::Num0 ) + '0';
		str.push_back(c);
		return str;
	}
	/*else if (key >= Keyboard::Numpad0 && key <= Keyboard::Numpad9)
	{
		return "NUMPAD" + to_string(key - Keyboard::Numpad0);
	}
	else if (key >= Keyboard::F1 && key <= Keyboard::F12)
	{
		return "F" + to_string(key - Keyboard::F1);
	}*/
	else
	{
		switch (key)
		{
		case Keyboard::LBracket:
			return "[";
		case Keyboard::RBracket:
			return "]";
		case Keyboard::Semicolon:
			return ";";
		case Keyboard::Comma:
			return ",";
		case Keyboard::Period:
			return ".";
		case Keyboard::Quote:
			return "\'";
		case Keyboard::Slash:
			return "/";
		case Keyboard::Backslash:
			return "\\";
		case Keyboard::Tilde:
			return "`";
		case Keyboard::Equal:
			return "=";
		case Keyboard::Hyphen:
			return "-";
		case Keyboard::LControl:
			return "LCONTROL";
		case Keyboard::LShift:
			return "LSHIFT";
		case Keyboard::LAlt:
			return "LALT";
		case Keyboard::RControl:
			return "RCONTROL";
		case Keyboard::RShift:
			return "RSHIFT";
		case Keyboard::RAlt:
			return "RALT";
		case Keyboard::Space:
			return "SPACE";
		case Keyboard::Enter:
			return "ENTER";
		case Keyboard::BackSpace:
			return "BACKSPACE";
		case Keyboard::Tab:
			return "TAB";
		case Keyboard::PageUp:
			return "PAGEUP";
		case Keyboard::PageDown:
			return "PAGEDOWN";
		case Keyboard::End:
			return "END";
		case Keyboard::Home:
			return "HOME";
		case Keyboard::Insert:
			return "INSERT";
		case Keyboard::Delete:
			return "DELETE";
		case Keyboard::Left:
			return "LEFT";
		case Keyboard::Right:
			return "RIGHT";
		case Keyboard::Up:
			return "UP";
		case Keyboard::Down:
			return "DOWN";
		}
	}

	
	//case Keyboard::BackSpace:
	//{
	//	//text.setString( text.getString().substring( 0, cursorIndex ) + text.getString().substring( cursorIndex + 1 ) );

	//	if (cursorIndex > 0)
	//	{
	//		sf::String s = text.getString();
	//		if (s.getSize() > 0)
	//		{
	//			s.erase(cursorIndex - 1);
	//			SetString(s);
	//			SetCursorIndex(cursorIndex - 1);
	//		}
	//	}

	//	break;
	//}
	//case Keyboard::Enter:
	//{
	//	int numLines = lineStartIndexes.size();
	//	if (numLines < maxRows)
	//	{
	//		c = '\n';
	//	}
	//	break;
	//}
	//case Keyboard::Left:
	//	SetCursorIndex(cursorIndex - 1);
	//	break;
	//case Keyboard::Right:
	//	SetCursorIndex(cursorIndex + 1);
	//	break;
	//case Keyboard::Up:
	//{


	//testText.setString(text.getString().substring(0, cursorIndex));
	//cursor.setPosition(pos.x + testText.getLocalBounds().width, pos.y);

	return "error";
}

bool IsKeyValidForInput(int key)
{
	if (key >= Keyboard::A && key <= Keyboard::Z)
	{
		return true;
	}
	else if (key >= Keyboard::Num0 && key <= Keyboard::Num9)
	{
		return true;
	}
	else if (key >= Keyboard::Numpad0 && key <= Keyboard::Numpad9)
	{
		return true;
	}
	else if (key >= Keyboard::F1 && key <= Keyboard::F12)
	{
		return true;
	}
	else
	{
		switch (key)
		{
		case Keyboard::LBracket:
		case Keyboard::RBracket:
		case Keyboard::Semicolon:
		case Keyboard::Comma:
		case Keyboard::Period:
		case Keyboard::Quote:
		case Keyboard::Slash:
		case Keyboard::Backslash:
		case Keyboard::Tilde:
		case Keyboard::Equal:
		case Keyboard::Hyphen:
		case Keyboard::LControl:
		case Keyboard::LShift:
		case Keyboard::LAlt:
		case Keyboard::RControl:
		case Keyboard::RShift:
		case Keyboard::RAlt:
		case Keyboard::Space:
		case Keyboard::Enter:
		case Keyboard::BackSpace:
		case Keyboard::Tab:
		case Keyboard::PageUp:
		case Keyboard::PageDown:
		case Keyboard::End:
		case Keyboard::Home:
		case Keyboard::Insert:
		case Keyboard::Delete:
		case Keyboard::Left:
		case Keyboard::Right:
		case Keyboard::Up:
		case Keyboard::Down:
			return true;
		}

		return false;

	}
}

AllControllers::AllControllers()
{
	windowsControllers.resize(4);
	gcControllers.resize(4);
	rawGCControllers.reserve(4);


	gccStatesVec.resize(4);
	windowsStatesVec.resize(4);

	for (int i = 0; i < 4; ++i)
	{
		windowsControllers[i] = new GameController(i, CTYPE_XBOX);
		gcControllers[i] = new GameController(i, CTYPE_GAMECUBE);

		gccStatesVec[i] = new ControllerDualStateQueue( gcControllers[i] );

		windowsStatesVec[i] = new ControllerDualStateQueue( windowsControllers[i] );
	}

	keyboardController = new GameController(0, CTYPE_KEYBOARD);
	keyboardStates = new ControllerDualStateQueue( keyboardController );

	
	window = NULL;
}

AllControllers::~AllControllers()
{
	for (int i = 0; i < 4; ++i)
	{
		delete windowsControllers[i];
		delete gcControllers[i];
		delete gccStatesVec[i];
		delete windowsStatesVec[i];
	}

	delete keyboardStates;
	delete keyboardController;

	if (joys != NULL)
		delete joys;

	if (gccDriver != NULL)
		delete gccDriver;
}

void AllControllers::Update()
{
	GCCUpdate();

	for (int i = 0; i < 4; ++i)
	{
		windowsControllers[i]->UpdateState();
		windowsStatesVec[i]->AddInput(windowsControllers[i]->GetState());
	}

	prevKeyboard = currKeyboard;
	currKeyboard.Update();

	keyboardController->UpdateState();
	keyboardStates->AddInput(keyboardController->GetState());
}

GameController * AllControllers::GetGCController(int index)
{
	return gcControllers[index];
}

GameController * AllControllers::GetWindowsController(int index)
{
	return windowsControllers[index];
}

GameController * AllControllers::GetController(int cType, int index)
{
	switch (cType)
	{
	case CTYPE_XBOX:
		return windowsControllers[index];
	case CTYPE_GAMECUBE:
		return gcControllers[index];
	case CTYPE_PS4:
		return NULL;
	case CTYPE_PS5:
		return NULL;
	case CTYPE_KEYBOARD:
	{
		if (index == 0)
			return keyboardController;
		else
			return NULL;
	}
	}

	assert(0);
	return NULL;
}

void AllControllers::GCCUpdate()
{
	if (!gccDriverEnabled)
	{
		return;
	}
	else
	{
		rawGCControllers = gccDriver->getState();
	}

	int numRawGCC = rawGCControllers.size();

	for (int i = 0; i < numRawGCC; ++i)
	{
		gcControllers[i]->gcController = rawGCControllers[i];
		gcControllers[i]->UpdateState();
		gccStatesVec[i]->AddInput(gcControllers[i]->GetState());
	}
}

void AllControllers::CheckForControllers()
{
	//ps5ControllerManager.CheckForControllers();

	//ps5ControllerManager.InitControllers(this);
	gccDriver = new GCC::USBDriver;
	if (gccDriver->getStatus() == GCC::USBDriver::Status::READY)
	{
		//cout << "ready" << endl;
		gccDriverEnabled = true;
		joys = new GCC::VJoyGCControllers(*gccDriver);
		{
			auto controllers = gccDriver->getState();
			//for (int i = 0; i < 4; ++i)
			//{
			//	//GameController &c = GetController(i);

			//	//c.gcDefaultControl.x = controllers[i].enabled
			//}
		}
	}
	else
	{
		//cout << "failing" << endl;
		joys = NULL;
		gccDriverEnabled = false;
		delete gccDriver;
		gccDriver = NULL;
	}
}

void AllControllers::SetRenderWindow(sf::RenderWindow *rw)
{
	window = rw;
	keyboardController->window = window;

}

bool AllControllers::ButtonHeld_A()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_A())
				return true;
		}
	}
	
	return false;
}

bool AllControllers::ButtonPressed_A()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_A())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_B()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_B())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_B()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_B())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_X()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_X())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_X()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_X())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_Y()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_Y())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_Y()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_Y())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_LeftShoulder()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_LeftShoulder())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_LeftShoulder()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_LeftShoulder())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_RightShoulder()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_RightShoulder())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_RightShoulder()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_RightShoulder())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_Start()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_Start())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_Start()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_Start())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonHeld_Any()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonHeld_Any())
				return true;
		}
	}

	return false;
}

bool AllControllers::ButtonPressed_Any()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_Any())
				return true;
		}
	}

	return false;
}

bool AllControllers::KeyboardButtonPressed(int key)
{
	return (!prevKeyboard.m_state[key] && currKeyboard.m_state[key]);
}

bool AllControllers::KeyboardButtonHeld(int key)
{
	return currKeyboard.m_state[key];
}

void AllControllers::UpdateKeyboardStick(ControlProfile *cp, bool rightStick, ControllerState &state, const ControllerState &prevState)
{
	int leftIndex = ControllerSettings::BUTTONTYPE_LLEFT;
	if (rightStick)
	{
		leftIndex = ControllerSettings::ButtonType::BUTTONTYPE_RLEFT;
	}

	bool left = KeyboardButtonHeld(cp->Filter((ControllerSettings::ButtonType)leftIndex));
	bool right = KeyboardButtonHeld(cp->Filter((ControllerSettings::ButtonType)(leftIndex+1)));
	bool up = KeyboardButtonHeld(cp->Filter((ControllerSettings::ButtonType)(leftIndex + 2)));
	bool down = KeyboardButtonHeld(cp->Filter((ControllerSettings::ButtonType)(leftIndex + 3)));	

	bool oldLeft = prevKeyboard.m_state[cp->Filter((ControllerSettings::ButtonType)(leftIndex))];
	bool oldRight = prevKeyboard.m_state[cp->Filter((ControllerSettings::ButtonType)(leftIndex+1))];
	bool oldUp = prevKeyboard.m_state[cp->Filter((ControllerSettings::ButtonType)(leftIndex+2))];
	bool oldDown = prevKeyboard.m_state[cp->Filter((ControllerSettings::ButtonType)(leftIndex+3))];

	V2d stickVec;

	bool leftPress = left && !oldLeft;
	bool rightPress = right && !oldRight;
	bool upPress = up && !oldUp;
	bool downPress = down && !oldDown;

	if (left && !right)
	{
		stickVec.x = -1;
	}
	else if (!left && right)
	{
		stickVec.x = 1;
	}
	else if (left && right)
	{
		if (leftPress && !rightPress)
		{
			stickVec.x = -1;
		}
		else if (!leftPress && rightPress)
		{
			stickVec.x = 1;
		}
		else if (leftPress && rightPress)
		{
			stickVec.x = 0;
		}
		else
		{
			if (rightStick)
			{
				if (prevState.RLeft())
				{
					stickVec.x = -1;
				}
				else if (prevState.RRight())
				{
					stickVec.x = 1;
				}
				else
				{
					stickVec.x = 0;
				}
			}
			else
			{
				if (prevState.LLeft())
				{
					stickVec.x = -1;
				}
				else if (prevState.LRight())
				{
					stickVec.x = 1;
				}
				else
				{
					stickVec.x = 0;
				}
			}
		}
	}
	else
	{
		stickVec.x = 0;
	}

	if (up && !down)
	{
		stickVec.y = -1;
	}
	else if (!up && down)
	{
		stickVec.y = 1;
	}
	else if (up && down)
	{
		if (upPress && !downPress)
		{
			stickVec.y = -1;
		}
		else if (!upPress && downPress)
		{
			stickVec.y = 1;
		}
		else if (upPress && downPress)
		{
			stickVec.y = 0;
		}
		else
		{
			if (rightStick)
			{
				if (prevState.RUp())
				{
					stickVec.y = -1;
				}
				else if (prevState.RDown())
				{
					stickVec.y = 1;
				}
				else
				{
					stickVec.y = 0;
				}
			}
			else
			{
				if (prevState.LUp())
				{
					stickVec.y = -1;
				}
				else if (prevState.LDown())
				{
					stickVec.y = 1;
				}
				else
				{
					stickVec.y = 0;
				}
			}
		}
	}
	else
	{
		stickVec.y = 0;
	}

	double *magPtr = &state.leftStickMagnitude;
	double *radPtr = &state.leftStickRadians;
	unsigned char *padPtr = &state.leftStickPad;
	if (rightStick)
	{
		magPtr = &state.rightStickMagnitude;
		radPtr = &state.rightStickRadians;
		padPtr = &state.rightStickPad;
	}

	if (length(stickVec) > 0)
	{
		*magPtr = 1;
	}
	else
	{
		*magPtr = 0;
	}

	stickVec = normalize(stickVec);

	double angle = -atan2(stickVec.y, stickVec.x);

	*radPtr = angle;

	*padPtr = 0;

	if (stickVec.x > 0)//right
	{
		*magPtr = 1.0;
		*padPtr += 1 << 3;
	}
	else if (stickVec.x < 0)//left
	{
		*magPtr = 1.0;
		*padPtr += 1 << 2;
	}

	if (stickVec.y < 0)//up
	{
		*magPtr = 1.0;
		*padPtr += 1;
	}
	else if (stickVec.y > 0)//down
	{
		*magPtr = 1.0;
		*padPtr += 1 << 1;
	}
}

void AllControllers::UpdateStateKeyboard(ControlProfile *cp, ControllerState &state, const ControllerState &prevState)
{
	state.Clear();

	state.A = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_JUMP));
	state.B = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_SPECIAL));
	state.X = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_DASH));
	state.Y = false;

	state.leftShoulder = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_SHIELD));
	state.rightShoulder = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_ATTACK));

	state.leftTrigger = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_LEFTWIRE));
	state.rightTrigger = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_RIGHTWIRE));

	state.start = KeyboardButtonHeld(cp->Filter(ControllerSettings::BUTTONTYPE_PAUSE));

	UpdateKeyboardStick(cp, false, state, prevState);
	UpdateKeyboardStick(cp, true, state, prevState);
}



ControllerDualStateQueue * AllControllers::GetStateQueue(GameController *con)
{
	return GetStateQueue(con->GetCType(), con->GetIndex());
}

ControllerDualStateQueue * AllControllers::GetStateQueue(int cType, int index)
{
	switch (cType)
	{
	case CTYPE_XBOX:
		return windowsStatesVec[index];
	case CTYPE_GAMECUBE:
		return gccStatesVec[index];
	case CTYPE_PS4:
		return NULL;
	case CTYPE_PS5:
		return NULL;
	case CTYPE_KEYBOARD:
	{
		if (index == 0)
			return keyboardStates;
		else
			return NULL;
	}
	}

	assert(0);
	return NULL;
}

ControllerStateQueue::ControllerStateQueue(int size, GameController *p_con )
{
	assert(size > 1);
	states.resize(size);
	con = p_con;
}

void ControllerStateQueue::AddInput(ControllerState &s)
{
	int numStates = states.size();
	for (int i = numStates - 1; i > 0; --i)
	{
		states[i] = states[i - 1];
	}

	states[0] = s;
}

int ControllerStateQueue::GetNumStates()
{
	return states.size();
}

bool ControllerStateQueue::ButtonHeld_A()
{
	return states[0].A;
}

bool ControllerStateQueue::ButtonPressed_A()
{
	return states[0].A && !states[1].A;
}

bool ControllerStateQueue::ButtonHeld_B()
{
	return states[0].B;
}

bool ControllerStateQueue::ButtonPressed_B()
{
	return states[0].B && !states[1].B;
}

bool ControllerStateQueue::ButtonHeld_X()
{
	return states[0].X;
}

bool ControllerStateQueue::ButtonPressed_X()
{
	return states[0].X && !states[1].X;
}

bool ControllerStateQueue::ButtonHeld_Y()
{
	return states[0].Y;
}

bool ControllerStateQueue::ButtonPressed_Y()
{
	return states[0].Y && !states[1].Y;
}

bool ControllerStateQueue::ButtonHeld_LeftShoulder()
{
	return states[0].leftShoulder;
}

bool ControllerStateQueue::ButtonPressed_LeftShoulder()
{
	return states[0].leftShoulder && !states[1].leftShoulder;
}

bool ControllerStateQueue::ButtonHeld_RightShoulder()
{
	return states[0].rightShoulder;
}

bool ControllerStateQueue::ButtonPressed_RightShoulder()
{
	return states[0].rightShoulder && !states[1].rightShoulder;
}

bool ControllerStateQueue::ButtonHeld_LeftTrigger()
{
	return states[0].LeftTriggerPressed();
}

bool ControllerStateQueue::ButtonPressed_LeftTrigger()
{
	return states[0].LeftTriggerPressed() && !states[1].LeftTriggerPressed();
}

bool ControllerStateQueue::ButtonHeld_RightTrigger()
{
	return states[0].RightTriggerPressed();
}

bool ControllerStateQueue::ButtonPressed_RightTrigger()
{
	return states[0].RightTriggerPressed() && !states[1].RightTriggerPressed();
}

bool ControllerStateQueue::ButtonHeld_Start()
{
	return states[0].start;
}

bool ControllerStateQueue::ButtonPressed_Start()
{
	return states[0].start && !states[1].start;
}

bool ControllerStateQueue::ButtonHeld_Any()
{
	return ButtonHeld_A() || ButtonHeld_B() || ButtonHeld_X() || ButtonHeld_Y() || ButtonHeld_LeftShoulder() || ButtonHeld_RightShoulder();
}

bool ControllerStateQueue::ButtonPressed_Any()
{
	return ButtonPressed_A() || ButtonPressed_B() || ButtonPressed_X() || ButtonPressed_Y() || ButtonPressed_LeftShoulder() || ButtonPressed_RightShoulder();
}

bool ControllerStateQueue::DirPressed_Left()
{
	return states[0].LLeft() && !states[1].LLeft();
}

bool ControllerStateQueue::DirPressed_Right()
{
	return states[0].LRight() && !states[1].LRight();
}

bool ControllerStateQueue::DirPressed_Up()
{
	return states[0].LUp() && !states[1].LUp();
}

bool ControllerStateQueue::DirPressed_Down()
{
	return states[0].LDown() && !states[1].LDown();
}

int ControllerStateQueue::GetControllerType()
{
	if (con != NULL)
	{
		return con->GetCType();
	}
	else
	{
		return -1;
	}
}

int ControllerStateQueue::GetIndex()
{
	if (con != NULL)
	{
		return con->GetIndex();
	}
	else
	{
		return -1;
	}
}

const ControllerState &ControllerStateQueue::GetCurrState()
{
	return states[0];
}

const ControllerState &ControllerStateQueue::GetPrevState()
{
	return states[1];
}

ControllerDualStateQueue::ControllerDualStateQueue( GameController *p_con )
	:ControllerStateQueue(2, p_con)
{

}

void KeyboardState::Update()
{
	for (int i = 0; i < Keyboard::KeyCount; ++i)
	{
		m_state[i] = Keyboard::isKeyPressed( (sf::Keyboard::Key)i);
	}
}