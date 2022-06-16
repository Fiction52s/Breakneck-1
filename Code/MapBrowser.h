#ifndef __MAP_BROWSER_H__
#define __MAP_BROWSER_H__

#include "GUI.h"
#include "Tileset.h"
#include <boost/filesystem.hpp>
#include "steam/steam_api.h"


struct WorkshopManager;
struct MapBrowser;

struct MapNode
{
	enum Type
	{
		FILE,
		FOLDER,
	};

	enum Action
	{
		IDLE,
		LOADING_PREVIEW,
		DOWNLOADING,
	};

	Action action;
	Type type;
	bool mapDownloaded;
	boost::filesystem::path folderPath;
	std::string mapName;
	boost::filesystem::path filePath;
	std::string description;
	std::string previewURL;
	Tileset *ts_preview;
	sf::Texture *previewTex;
	bool checkingForPreview;
	//sf::Texture previewTexture;
	HTTPRequestHandle previewRequestHandle;
	PublishedFileId_t publishedFileId;
	ImageChooseRect *chooseRect;
	int index;
	CCallResult<MapNode,
		HTTPRequestCompleted_t>
		OnHTTPRequestCompletedCallResult;

	MapNode();
	~MapNode();
	void Draw(sf::RenderTarget *target);
	void OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback,
		bool bIOFailure);
	void RequestDownloadPreview();
	
};

struct MapBrowserHandler : GUIHandler
{
	MapBrowserHandler(int cols, int rows, int extraImageRects = 0);
	virtual ~MapBrowserHandler();
	virtual void Cancel() = 0;
	virtual void Confirm() = 0;
	virtual void ClickFile(ChooseRect *cr) = 0;
	virtual void FocusFile(ChooseRect *cr) = 0;
	virtual void UnfocusFile(ChooseRect *cr) = 0;
	virtual bool MouseUpdate() { return true; }
	virtual void Draw(sf::RenderTarget *target) {}
	virtual void ChangePath() {}
	virtual void LateDraw(sf::RenderTarget *target) {}

	//guihandler functions
	virtual void ChooseRectEvent(ChooseRect *cr, int eventType);
	virtual void ButtonCallback(Button *b, const std::string & e);
	virtual void SliderCallback(Slider *slider) {}
	//---------

	bool CheckIfSelectedItemInstalled();

	MapBrowser *chooser;
};

struct DefaultMapBrowserHandler : MapBrowserHandler
{
	DefaultMapBrowserHandler();
	void Cancel();
	void Confirm();
	void ClickFile(ChooseRect *cr);
	void FocusFile(ChooseRect *cr);
	void UnfocusFile(ChooseRect *cr);
	void Draw(sf::RenderTarget *target);
	void ChangePath();

	void Update();

	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;

	sf::Text descriptionText;

	
};

struct MapBrowser : TilesetManager,
	PanelUpdater
{
	enum Mode : int
	{
		WORKSHOP,
		OPEN,
		SAVE
	};

	enum Action : int
	{
		A_IDLE,
		A_WAITING_FOR_QUERY_RESULTS,
		A_WAITING_FOR_PREVIEW_RESULTS,
		A_WAITING_FOR_MAP_DOWNLOAD,
	};

	Action action;

	int currWorkshopPage;
	int maxWorkshopPages;

	MapBrowser(MapBrowserHandler *handler,
		int p_cols, int p_rows, int extraImageRects = 0);
	~MapBrowser();

	void Update();
	//panelupdater functions
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);
	void LateDraw(sf::RenderTarget *target);

	void QueryMaps();
	//---------


	void SetRelativePath(const std::string &p_relPath);
	void SetPath(const std::string &p_path);
	void SetToWorkshop();
	void AddFile(const boost::filesystem::path &filePath);
	void AddFolder(const boost::filesystem::path &folderPath);
	void ClearNodes();
	void PopulateRects();
	void UpdatePreviews();
	void Start(const std::string &ext,
		Mode fMode, const std::string &path);
	void StartRelative(const std::string &ext,
		Mode fMode, const std::string &path);
	void Init();
	void TurnOff();
	//void HideConfirmButton();

	ImageChooseRect *selectedRect;
	WorkshopManager *workshop;
	Mode fMode;
	MapBrowserHandler *handler;

	std::string ext;

	int topRow;
	int maxTopRow;
	int numEntries;
	int cols;
	int rows;
	int totalRects;

	boost::filesystem::path currPath;

	std::vector<MapNode*> nodes;

	Panel *panel;
	ImageChooseRect **imageRects;
	TextBox *fileNameTextBox;
	Button *playButton;
	Button *nextPageButton;
	Button *prevPageButton;
	sf::Text pageLabel;
	sf::Text *folderPathText;
};

#endif