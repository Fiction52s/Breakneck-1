#ifndef __POSTPRACTICEMATCHMENU_H__
#define __POSTPRACTICEMATCHMENU_H__

#include <SFML\Graphics.hpp>

struct BasicTextMenu;
struct KeepPlayingButton;

struct PostPracticeMatchMenu
{
	BasicTextMenu *textMenu;
	KeepPlayingButton *keepPlayingButton;

	PostPracticeMatchMenu();
	~PostPracticeMatchMenu();
	void Reset();
	int Update();
	bool WantsToKeepPlaying();
	void Draw(sf::RenderTarget *target);
};

#endif