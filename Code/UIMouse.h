#ifndef __UIMOUSE_H__
#define __UIMOUSE_H__

#include <SFML/Graphics.hpp>
#include "Input.h"

struct CustomCursor;

struct UIMouse
{
	static UIMouse &GetInstance()
	{
		static UIMouse instance;
		return instance;
	}


	UIMouse(UIMouse const&) = delete;
	void operator=(UIMouse const&) = delete;

	bool IsMouseDownLeft();
	bool IsMouseDownRight();
	bool IsMouseLeftClicked();
	bool IsMouseLeftReleased();
	bool IsMouseRightClicked();
	bool IsMouseRightReleased();
	void Update(sf::Vector2i &mousePos);
	//void Update(sf::Vector2i &mousePos, std::vector<ControllerState> & states);
	const sf::Vector2i &GetPos();
	sf::Vector2f GetFloatPos();
	void ResetMouse();
	bool IsConsumed() { return consumed; }
	void Consume() { consumed = true; }
	void SetRenderWindow(sf::RenderWindow *rw);
	void SetCustomCursor(CustomCursor *cc);
	void SetControllersOn(bool on);
	bool IsWindowFocused();
	void SetPosition(sf::Vector2i &pos);
	sf::Vector2i GetRealPixelPos();
	void Hide();
	void Show();
	void Grab();
	void Release();
private:
	UIMouse();

	sf::Vector2i mousePos;
	sf::Vector2i myPos;
	bool isMouseDownLeft;
	bool lastMouseDownLeft;

	bool isMouseDownRight;
	bool lastMouseDownRight;
	bool consumed;
	bool controllersOn;

	const static float stickDeadZone;

	sf::RenderWindow *currWindow;
	CustomCursor *customCursor;

};

#define MOUSE UIMouse::GetInstance()

#endif