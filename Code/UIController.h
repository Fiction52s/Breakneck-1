#ifndef __UICONTROLLER_H__
#define __UICONTROLLER_H__

#include <SFML/Graphics.hpp>
#include "Input.h"
#include "ItemSelector.h"

struct UIController
{
	static UIController &GetInstance()
	{
		static UIController instance;
		return instance;
	}

	UIController(UIController const&) = delete;
	void operator=(UIController const&) = delete;
	void Update();
	void Reset();
	void SetRenderWindow(sf::RenderWindow *rw);
	bool IsWindowFocused();
	int ConsumeScroll();
	bool IsConfirmHeld();
	bool IsConfirmPressed();
	bool IsCancelHeld();
	bool IsCancelPressed();
private:
	UIController();
	sf::RenderWindow *currWindow;
	float scrollCounter;
	float maxScroll;

	bool confirmPressed;
	bool cancelPressed;
	bool confirmHeld;
	bool cancelHeld;
};

#define UICONTROLLER UIController::GetInstance()

#endif