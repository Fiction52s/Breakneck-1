#ifndef __ADVENTURE_CREATOR__
#define __ADVENTURE_CREATOR__

#include "SaveFile.h"
#include "MusicSelector.h"
#include "MapBrowser.h"
struct AdventureFile;

struct AdventureCreator : MapBrowserHandler
{
	enum Action
	{
		BROWSE,
		DRAG,
	};

	AdventureCreator();
	~AdventureCreator();

	//inherited
	void Confirm();
	void ClickFile(ChooseRect *cr);
	//void FocusFile(ChooseRect *cr);
	bool MouseUpdate();
	//void Draw(sf::RenderTarget *target);
	void LateDraw(sf::RenderTarget *target);

	void Open();
	void ExpandWorlds();
	void CollapseWorlds();
	void ChooseWorld(int w);
	void ChooseSector(int s);
	
	MusicChooserHandler *musicListHandler;
	int action;

	void ChooseRectEvent(ChooseRect *cr, int eventType);

	sf::Vertex grabbedFileQuad[4];
	MapNode *grabbedFile;
	MapNode tempGrabbedFile;

	Label *sectorLabel;

	void SetRectNode(ChooseRect *cr, MapNode *mn);
	int GetNodeStart();
	void LoadAdventure(const std::string &path, const std::string &adventureName);
	void SaveAdventure(const std::string &path, const std::string &adventureName,
		AdventureFile::CopyMode copyMode );

	
	Tileset *ts_sectorIcons;

	//512 = 8 * 8 * 8
	MapNode adventureNodes[512];
	MapNode * GetCurrNode(int m);


	
	int currWorld;
	int currSector;
	bool worldsExpanded;

	//ImageChooseRect *worldRects[8];
	//ImageChooseRect *currWorldRect;

	ImageChooseRect *worldRects[8];
	ImageChooseRect *currWorldRect;

	ImageChooseRect *sectorRects[8];

	ImageChooseRect *mapRects[8];

	TextChooseRect *musicRects[8];

	AdventureFile *adventure;

	const static int EXTRA_RECTS = 9 + 8 + 8;
};

#endif