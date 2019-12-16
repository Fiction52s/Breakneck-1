#ifndef __EDITOR_BG__
#define __EDITOR_BG__

#include <SFML/Graphics.hpp>
#include <list>

struct ScrollingBackground;
struct Background;

struct EditorBG
{
	EditorBG();
	~EditorBG();
	Background *currBackground;
	bool showBG;
	std::list<ScrollingBackground*> scrollingBackgrounds;
	void Draw(sf::RenderTarget *target);
	void Show(bool s = true);
	void FlipShown();

	sf::View bgView;
};

#endif