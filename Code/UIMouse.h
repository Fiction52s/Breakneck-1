#ifndef __UIMOUSE_H__
#define __UIMOUSE_H__

#include <SFML/Graphics.hpp>
#include "Input.h"

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
	bool IsWindowFocused();
	void SetPosition(sf::Vector2i &pos);
private:

	bool controllerMode;

	UIMouse();

	sf::Vector2i mousePos;
	sf::Vector2i myPos;
	bool isMouseDownLeft;
	bool lastMouseDownLeft;

	bool isMouseDownRight;
	bool lastMouseDownRight;
	bool consumed;

	sf::RenderWindow *currWindow;

};

#define MOUSE UIMouse::GetInstance()

#endif