#ifndef __MAP_BROWSER_H__
#define __MAP_BROWSER_H__

#include "GUI.h"
#include "Tileset.h"
#include <boost/filesystem.hpp>
#include "steam/steam_api.h"


struct WorkshopManager;
struct MapBrowser;
struct MapHeader;
struct LobbyParams;

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

	Type type;

	Action action;
	boost::filesystem::path filePath;
	Tileset *ts_preview;
	int index;

	//new
	
	bool mapDownloaded;
	std::string nodeName;
	boost::filesystem::path folderPath;
	std::string description;
	std::string previewURL;
	sf::Texture *previewTex;
	bool checkingForPreview;
	HTTPRequestHandle previewRequestHandle;
	PublishedFileId_t publishedFileId;
	ImageChooseRect *chooseRect;
	
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
	~MapBrowserHandler();
	void Cancel();
	void Confirm();
	void ClickFile(ChooseRect *cr);
	void FocusFile(ChooseRect *cr);
	void UnfocusFile(ChooseRect *cr);
	void Draw(sf::RenderTarget *target);
	void ChangePath();
	void ClearSelection();
	void SelectRect(ChooseRect *cr);

	void Update();
	bool CheckIfSelectedItemInstalled();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	//void SliderCallback(Slider *slider);

	sf::Vertex largePreview[4];
	Tileset *ts_largePreview;

	sf::Text descriptionText;

	MapBrowser *chooser;

	
};

struct MapBrowser : TilesetManager,
	PanelUpdater
{
	enum Mode : int
	{
		WORKSHOP,
		OPEN,
		SAVE,
		CREATE_CUSTOM_GAME,
	};

	//new vars
	enum Action : int
	{
		A_IDLE,
		A_WAITING_FOR_QUERY_RESULTS,
		A_WAITING_FOR_PREVIEW_RESULTS,
		A_WAITING_FOR_MAP_DOWNLOAD,
		A_CANCELLED,
	};

	Action action;
	ImageChooseRect *selectedRect;
	WorkshopManager *workshop;
	int currWorkshopPage;
	int maxWorkshopPages;
	Button *playButton;
	Button *nextPageButton;
	Button *prevPageButton;
	sf::Text pageLabel;

	MapBrowser(MapBrowserHandler *handler,
		int p_cols, int p_rows, int extraImageRects = 0);
	~MapBrowser();

	//new functions
	void Update();
	void QueryMaps();
	

	//panelupdater functions
	//bool MouseUpdate();
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);
	//void LateDraw(sf::RenderTarget *target);
	
	//---------
	void SetRelativePath(const std::string &p_relPath);
	void SetPath(const std::string &p_path);
	void AddFile(const boost::filesystem::path &filePath);
	void AddFolder(const boost::filesystem::path &folderPath);
	void ClearNodes();
	void PopulateRects();
	//bool IsCustomMapSelected();
	
	void Start(const std::string &ext,
		Mode mode, const std::string &path);
	void StartRelative(const std::string &ext,
		Mode mode, const std::string &path);
	void StartWorkshop();
	void Init();
	void TurnOff();
	//void HideConfirmButton();

	Mode mode;
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

	//EditSession *edit; //removed var
	
	Panel *panel;
	ImageChooseRect **imageRects;
	TextBox *fileNameTextBox;
	Button *upButton;

	
	
	sf::Text *folderPathText;
};

struct MapOptionsPopup : GUIHandler
{
	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
		A_CONFIRMED,
		A_CANCELLED,
	};

	Panel *panel;
	bool active;
	Action action;

	MapHeader *currMapHeader;
	LobbyParams *currLobbyParams;
	int chosenGameModeType;
	std::string mapPath;

	MapOptionsPopup();
	~MapOptionsPopup();
	void Activate( const std::string &p_mapPath );
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);

	void ButtonCallback(Button *b, const std::string & e);
};

#endif