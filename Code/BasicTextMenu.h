#ifndef __BASICTEXTMENU_H__
#define __BASICTEXTMENU_H__

#include <SFML\Graphics.hpp>
#include <vector>
#include <string>

struct MainMenu;

struct BasicTextMenu
{
	sf::Vertex bgQuad[4];
	MainMenu *mainMenu;
	int numOptions;

	std::vector<sf::Text> textVec;

	sf::Color selectedColor;
	sf::Color unselectedColor;

	BasicTextMenu( std::vector<std::string> &options );
	void Reset();
	int GetSelectedIndex();
	int Update();
	void SetSelected(int index);
	void Draw(sf::RenderTarget *target);

private:
	int selectedIndex;
};

#endif