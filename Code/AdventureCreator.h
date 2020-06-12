#ifndef __ADVENTURE_CREATOR__
#define __ADVENTURE_CREATOR__

#include "FileChooser.h"
#include "SaveFile.h"
struct AdventureFile;

struct AdventureCreator : FileChooserHandler
{
	enum State
	{
		BROWSE,
		DRAG,
	};

	AdventureCreator();
	~AdventureCreator();
	void Cancel();
	void Confirm();
	void ClickFile(ChooseRect *cr);
	void FocusFile(ChooseRect *cr);
	void UnfocusFile(ChooseRect *cr);
	void Draw(sf::RenderTarget *target);
	void LateDraw(sf::RenderTarget *target);
	void ChangePath();
	void Open();
	bool MouseUpdate();
	void ExpandWorlds();
	void CollapseWorlds();
	void ChooseWorld(int w);
	void ChooseSector(int s);
	

	State state;

	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void SliderCallback(Slider *slider);

	sf::Vertex grabbedFileQuad[4];
	FileNode *grabbedFile;
	FileNode tempGrabbedFile;

	Slider *sectorRequirementsSlider;
	sf::Text *sectorLabel;

	void SetRectNode(ChooseRect *cr, FileNode *fn);
	int GetNodeStart();
	void LoadAdventure(const std::string &path, const std::string &adventureName);
	void SaveAdventure(const std::string &path, const std::string &adventureName,
		AdventureFile::CopyMode copyMode );

	
	Tileset *ts_sectorIcons;

	//512 = 8 * 8 * 8
	FileNode adventureNodes[512];
	FileNode * GetCurrNode(int m);


	
	int currWorld;
	int currSector;
	bool worldsExpanded;

	//ImageChooseRect *worldRects[8];
	//ImageChooseRect *currWorldRect;

	ImageChooseRect *worldRects[8];
	ImageChooseRect *currWorldRect;

	ImageChooseRect *sectorRects[8];

	ImageChooseRect *mapRects[8];

	AdventureFile *adventure;

	const static int EXTRA_RECTS = 9 + 8 + 8;
};

#endif