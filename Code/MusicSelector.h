#ifndef __MUSIC_SELECTOR_H__
#define __MUSIC_SELECTOR_H__
#include <string>
#include <list>
#include <SFML\Graphics.hpp>
#include "UIWindow.h"
#include "ItemSelector.h"

struct MusicInfo;

struct MusicSelector
{
	//TODO scrollbar to show how far in to the names you are
	static const int NUM_BOXES = 7;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	MusicSelector ( MainMenu *mainMenu, sf::Vector2f &p_topMid,
		std::list<MusicInfo*> &p_songs );

	MainMenu *mainMenu;

	void SetupBoxes();
	void Update();
	void MoveUp();
	void MoveDown();
	void UpdateNames();
	void UpdateBoxesDebug();
	void Draw(sf::RenderTarget *target);

	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text musicNames[NUM_BOXES];
	sf::Vector2f topMid;

	//int currIndex;
	int oldCurrIndex;
	int topIndex;

	std::list<MusicInfo*> &rawSongs;
	std::list<MusicInfo*> songs;
	sf::Font font;

	MusicInfo *currSong;
	SingleAxisSelector *saSelector;
};

struct MusicManager
{

	MusicManager();
	~MusicManager();
	bool LoadFolderPaths();
	bool LoadMusic();

	std::list<MusicInfo*> songs;
};

#endif
