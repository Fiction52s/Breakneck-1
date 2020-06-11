#ifndef __ADVENTURE_CREATOR__
#define __ADVENTURE_CREATOR__

#include "FileChooser.h"

struct AdventureCreator : FileChooserHandler
{
	enum State
	{
		BROWSE,
		DRAG,
	};

	AdventureCreator();
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
	

	State state;

	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;

	void ChooseRectEvent(ChooseRect *cr, int eventType);

	sf::Vertex grabbedFileQuad[4];
	FileNode *grabbedFile;

	ImageChooseRect *worldRects[8];
	ImageChooseRect *currWorldRect;
	void ExpandWorlds();
	void CollapseWorlds();
	void ChooseWorld(int w);
	int currWorld;
	bool worldsExpanded;

	//ImageChooseRect *worldRects[8];
	//ImageChooseRect *currWorldRect;

	ImageChooseRect *mapRects[8];

	const static int EXTRA_RECTS = 9 + 8;
};

#endif