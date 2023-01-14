#ifndef __MUSIC_SELECTOR_H__
#define __MUSIC_SELECTOR_H__
#include <string>
#include <list>
#include <SFML\Graphics.hpp>
#include "UIWindow.h"
#include "ItemSelector.h"
#include <boost\filesystem.hpp>
#include <SFML\Audio.hpp>
#include "GUI.h"

struct MusicInfo;
struct MainMenu;
struct MusicManager;
struct MusicPlayer;

struct MusicManager;
struct MusicInfo
{
	MusicInfo();
	~MusicInfo();
	void Cleanup();
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

	MusicInfo *GetMusicInfo( const std::string &name );
	std::list<std::string> folderPaths;
	//std::list<boost::filesystem::path> songPaths;
	std::map<std::string, MusicInfo*> songMap;
	MainMenu *mainMenu;
};




struct ListChooser;
struct MapHeader;
struct ListChooserHandler : GUIHandler
{
	ListChooserHandler(int rows);
	void SetHeader(MapHeader *mh);
	virtual ~ListChooserHandler();
	virtual void ClickText(ChooseRect *cr) = 0;
	//virtual void Cancel() = 0;
	//virtual void Confirm() = 0;
	//virtual void ClickFile(ChooseRect *cr) = 0;
	//virtual void FocusFile(ChooseRect *cr) = 0;
	//virtual void UnfocusFile(ChooseRect *cr) = 0;
	virtual	bool MouseUpdate() { return true; }
	virtual void Draw(sf::RenderTarget *target) {}
	//virtual void ChangePath() {}
	virtual void LateDraw(sf::RenderTarget *target) {}

	////guihandler functions
	//virtual void ChooseRectEvent(ChooseRect *cr, int eventType);
	//virtual void ButtonCallback(Button *b, const std::string & e);
	//virtual void SliderCallback(Slider *slider) {}
	//---------
	//virtual void PanelCallback(Panel *p, const std::string & e);

	ListChooser *chooser;
};

struct MusicChooserHandler : ListChooserHandler
{
	enum State
	{
		BROWSE,
		DRAG,
	};

	sf::Text grabbedText;
	std::string grabbedString;
	State state;

	MusicChooserHandler(int rows);
	~MusicChooserHandler();
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);

	void ClickText(ChooseRect *cr);
	//virtual void ChangePath() {}
	void LateDraw(sf::RenderTarget *target);

	////guihandler functions
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	//virtual void SliderCallback(Slider *slider) {}
	//---------
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
	void CancelCallback(Panel *p);
};

struct ListChooser : PanelUpdater
{
	int topRow;
	int maxTopRow;
	int numEntries;
	int totalRects;
	EditSession *edit;
	TextChooseRect **textRects;
	TextChooseRect **myMusicRects;
	int numMyMusicRects;
	Panel *panel;
	Slider *sliders[3];
	ListChooserHandler *handler;
	std::map<std::string, std::vector<std::string>> songNames;
	std::string playingSongName;
	CheckBox *playOriginalCheckbox;
	Dropdown *worldDropdown;
	MapHeader *mh;

	TextChooseRect *currPlayingRect;
	TextChooseRect *currPlayingMyRect;
	//Button *okButton;
	//Button *cancelButton;


	ListChooser( ListChooserHandler *handler, int rows);
	~ListChooser();
	void ResetSlider(const std::string &str);
	void HideSlider(const std::string &str);
	void SetPlayingColor(const std::string &str);
	void SetPlayingColorMyRects(const std::string &str);
	void SetStoppedColor();
	void SetStoppedColorMyRects();
	void OpenPopup();
	void ClosePopup();
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);
	void LateDraw(sf::RenderTarget *target);
	void PopulateRects();
};

struct MusicSelectorUI : GUIHandler
{
	MusicChooserHandler *listHandler;
	EditSession *edit;
	

	

	MusicSelectorUI();
	~MusicSelectorUI();
	bool ShouldPlayOriginal();
	void OpenPopup();
	void ClosePopup();
	void Draw(sf::RenderTarget *target);
	void Update();
};

//struct AdventureMusicListChooser : PanelUpdater
//{
//	int topRow;
//	int maxTopRow;
//	int numEntries;
//	int totalRects;
//	EditSession *edit;
//	TextChooseRect **textRects;
//	TextChooseRect **myMusicRects;
//	int numMyMusicRects;
//	Panel *panel;
//	Slider *sliders[3];
//	ListChooserHandler *handler;
//	std::map<std::string, std::vector<std::string>> songNames;
//	std::string playingSongName;
//	CheckBox *playOriginalCheckbox;
//	Dropdown *worldDropdown;
//	MapHeader *mh;
//
//	TextChooseRect *currPlayingRect;
//	TextChooseRect *currPlayingMyRect;
//	//Button *okButton;
//	//Button *cancelButton;
//
//
//	AdventureMusicListChooser(ListChooserHandler *handler, int rows,
//		int numMyMusicRects, TextChooseRect **musicRects );
//	~AdventureMusicListChooser();
//	void ResetSlider(const std::string &str);
//	void HideSlider(const std::string &str);
//	void SetPlayingColor(const std::string &str);
//	void SetPlayingColorMyRects(const std::string &str);
//	void SetStoppedColor();
//	void SetStoppedColorMyRects();
//	void OpenPopup();
//	void ClosePopup();
//	bool MouseUpdate();
//	void Draw(sf::RenderTarget *target);
//	void Deactivate();
//	void MouseScroll(int delta);
//	void LateDraw(sf::RenderTarget *target);
//	void PopulateRects();
//};

#endif
