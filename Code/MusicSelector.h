#ifndef __MUSIC_SELECTOR_H__
#define __MUSIC_SELECTOR_H__
#include <string>
#include <list>
#include <SFML\Graphics.hpp>
#include "UIWindow.h"
#include "ItemSelector.h"
#include <boost\filesystem.hpp>
#include <SFML\Audio.hpp>
struct MusicInfo;
struct MainMenu;



struct MusicManager;
struct MapSelectionMenu;
struct MusicSelector
{
	static const int NUM_SLIDER_OPTIONS;
	//TODO scrollbar to show how far in to the names you are
	static const int NUM_BOXES = 7;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	MusicSelector ( MainMenu *mainMenu, MapSelectionMenu *mapMenu,
		sf::Vector2f &p_topMid,
		MusicManager *mm );

	VertSlider vSlider;
	MainMenu *mainMenu;

	void SetupBoxes();
	void Update();
	void MoveUp();
	void MoveDown();
	void UpdateNames();
	void LoadNames();
	void UpdateBoxesDebug();
	void Draw(sf::RenderTarget *target);

	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text musicNames[NUM_BOXES];
	sf::Vector2f topMid;
	SingleAxisSlider *oftenSlider[NUM_BOXES];
	bool modifiedValues;

	//int currIndex;
	int oldCurrIndex;
	int topIndex;

	//std::list<MusicInfo*> &rawSongs;
	MusicManager *musicMan;
	//std::list<MusicInfo*> songs;
	MusicInfo **songs;
	MusicInfo *previewSong;
	int numSongs;
	sf::Font font;

	//int currSongIndex;
	SingleAxisSelector *saSelector;
	MapSelectionMenu *mapMenu;
};

struct MusicManager;
struct MusicInfo
{
	MusicInfo();
	~MusicInfo();
	sf::Music *music;
	boost::filesystem::path songPath;
	bool Load();
};

struct MusicManager
{

	MusicManager( MainMenu *p_mainMenu );
	~MusicManager();
	bool LoadFolderPaths();
	bool LoadMusicNames();
	bool rLoadMusicNames( const boost::filesystem::path &p );
	bool LoadSong(const std::string &name );
	bool DebugLoadMusic();
	std::list<std::string> folderPaths;
	//std::list<boost::filesystem::path> songPaths;
	std::map<std::string, MusicInfo*> songMap;
	std::list<MusicInfo*> songs;
	MainMenu *mainMenu;
};

#endif
