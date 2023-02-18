#ifndef __MAP_BROWSER_H__
#define __MAP_BROWSER_H__

#include "GUI.h"
#include "Tileset.h"
#include <boost/filesystem.hpp>
#include "steam/steam_api.h"


struct WorkshopManager;
struct MapBrowser;
struct MapHeader;

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
	bool previewOwner;
	int index;

	//new
	MapHeader *header;
	MapBrowser *myBrowser;
	bool mapDownloaded;
	bool mapUpdating;
	int downloadResult;
	std::string nodeName;
	std::string fileName; //node name can be different from file name
	boost::filesystem::path folderPath;
	bool nameAndDescriptionUpdated;
	std::string description;
	std::string fullMapName;
	std::string previewURL;
	sf::Texture *previewTex;
	bool checkingForPreview;
	HTTPRequestHandle previewRequestHandle;
	PublishedFileId_t publishedFileId;
	
	ImageChooseRect *chooseRect;
	bool isWorkshop;
	bool creatorNameRetrieved;
	bool checkingForCreatorName;
	std::string creatorName;
	uint64 creatorId;

	bool subscribing;
	bool unsubscribing;
	
	CCallResult<MapNode,
		HTTPRequestCompleted_t>
		OnHTTPRequestCompletedCallResult;

	CCallResult<MapNode,
		RemoteStorageSubscribePublishedFileResult_t> onRemoteStorageSubscribePublishedFileResultCallResult;

	CCallResult<MapNode,
		RemoteStorageUnsubscribePublishedFileResult_t> onRRemoteStorageUnsubscribePublishedFileResultCallResult;

	MapNode();
	~MapNode();
	void Clear();
	bool IsSubscribed();
	void Subscribe();
	void Unsubscribe();
	void TryUpdate();
	void CheckDownloadResult();
	void UpdateInstallInfo();
	void Draw(sf::RenderTarget *target);
	void OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback,
		bool bIOFailure);
	void ClearPreview();
	void CreatePreview();
	void UpdateHeaderInfo();
	void Copy(MapNode *mn);

	void OnSubscribe(RemoteStorageSubscribePublishedFileResult_t *callback, bool bIOFailure);
	void OnUnsubscribe(RemoteStorageUnsubscribePublishedFileResult_t *callback, bool bIOFailure);

	void RequestDownloadPreview();
	bool CheckIfFullyInstalled();
	void RequestCreatorInfo();
};

struct MapBrowserHandler : GUIHandler
{
	MapBrowserHandler(int cols, int rows, bool showPreview, int extraImageRects = 0);
	~MapBrowserHandler();

	virtual void Cancel();
	virtual void Confirm();
	virtual void ClickFile(ChooseRect *cr);
	void FocusFile(ChooseRect *cr);
	virtual void LateDraw(sf::RenderTarget *target);

	virtual bool MouseUpdate();

	void BasicUpdate();

	virtual void Draw(sf::RenderTarget *target);
	void ChangePath();
	
	void Clear();
	void ClearSelection();
	void SelectRect(ChooseRect *cr);
	void ClearFocus();
	void Update();
	bool CheckIfSelectedItemInstalled();
	void SubscribeToItem();
	void UnsubscribeFromItem();
	void SetPreviewTopLeft(sf::Vector2f &pos);

	virtual void ChooseRectEvent(ChooseRect *cr, int eventType);

	void ButtonCallback(Button *b, const std::string & e);
	void TabGroupCallback(TabGroup *tg, const std::string &e);
	void ScrollBarCallback(ScrollBar *sb, const std::string &e);
	void ConfirmCallback(Panel *p);
	void CancelCallback(Panel *p);

	sf::Vertex largePreview[4];
	sf::Vertex noPreviewQuad[4];
	Tileset *ts_largePreview;
	bool showPreview;

	sf::Text fullNameText;
	sf::Text descriptionText;

	HyperLink *creatorLink;

	ChooseRect *focusedRect;

	MapBrowser *chooser;

	std::string confirmedMapFilePath;
};

struct MapBrowser : TilesetManager,
	PanelUpdater
{
	enum Mode : int
	{
		WORKSHOP,
		OPEN,
		SAVE,
		EDITOR_OPEN,
		EDITOR_SAVE,
		EDITOR_SAVE_ADVENTURE,
		CREATE_CUSTOM_GAME,
		FREEPLAY,
	};

	//new vars
	enum Action : int
	{
		A_IDLE,
		A_WAITING_FOR_QUERY_RESULTS,
		A_WAITING_FOR_PREVIEW_RESULTS,
		A_WAITING_FOR_MAP_DOWNLOAD,
		A_CANCELLED,
		A_CONFIRMED,

	};


	std::string searchStr;
	std::vector<int> numPlayersAllowed;
	std::vector<int> gameModesAllowed;

	bool isWorkshop;
	Action action;
	ImageChooseRect *selectedRect;
	MapNode *selectedNode;
	
	WorkshopManager *workshop;
	int currWorkshopPage;
	int maxWorkshopPages;
	Button *nextPageButton;
	Button *prevPageButton;
	sf::Text pageLabel;

	TextBox *searchBox;
	Button *searchButton;

	ScrollBar *mapScroller;

	Button *saveButton;
	Button *openButton;
	Button *createLobbyButton;
	Button *okButton;
	Button *cancelButton;
	
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
	Label *fileNameTextBoxLabel;
	Button *upButton;



	Label *folderPathLabel;


	MapBrowser(MapBrowserHandler *handler,
		int p_cols, int p_rows, int extraImageRects = 0);
	~MapBrowser();

	//new functions
	void Update();
	void QueryMaps();
	void ClearFilters();

	bool MouseUpdate();
	
	void ClearPreviews();
	void ClearAllPreviewsButSelected();
	void RequestAllPreviews();
	//panelupdater functions
	
	void LateDraw(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);

	void SelectRect(ChooseRect *cr);
	void ClearSelection();

	void UpdateSearchCriteria(const std::string &s);
	void UpdateNumPlayersCriteria(std::vector<int> &numAllowedPlayers);
	void UpdateGameModeCriteria(std::vector<int> &numAllowedPlayers);
	//void LateDraw(sf::RenderTarget *target);
	
	//---------
	void SetRelativePath(const std::string &p_relPath);
	void SetPath(const std::string &p_path);
	void AddFile(const boost::filesystem::path &filePath);
	void AddFolder(const boost::filesystem::path &folderPath);
	void ClearNodes();
	void PopulateRects();
	void Refresh();
	//bool IsCustomMapSelected();
	
	void Start(const std::string &ext,
		Mode mode, const std::string &path);
	void StartRelative(const std::string &ext,
		Mode mode, const std::string &path);
	void StartWorkshop( Mode mode );
	void Init();
	void TurnOff();
	void SetCurrFileNameText(const std::string &text);

	void ShowFileNameTextBox();
	void HideFileNameTextBox();
	bool CheckFilters(MapNode *mn);
	//void HideConfirmButton();

	
};



#endif