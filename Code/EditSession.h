#ifndef __EDIT_SESSION__
#define __EDIT_SESSION__

#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <list>
#include "VectorMath.h"
#include "GUI.h"
#include "Tileset.h"
#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <map>
#include <boost/filesystem.hpp>
#include "MainMenu.h"
#include "MapHeader.h"
#include "Gate.h"
#include "ISelectable.h"

#include "EditorGateInfo.h"
#include "EditorActors.h"
#include "EditorTerrain.h"
#include "Session.h"
#include "PositionInfo.h"
//#include "ActorParamsBase.h"
//#include "EditorRail.h"
struct GateMarker;
struct MusicSelectorUI;
struct FrameWaiter;
struct TransformTools;
struct EditorGraph;
struct ActorParams;
struct MainMenu;
//struct ActorParams;
struct TerrainPolygon;
//struct GateInfo;
struct EditSession;
struct ActorGroup;
struct ActorType;
struct Brush;
struct Action;
struct CompoundAction;
//struct MovePointsAction;
struct Background;
struct ScrollingBackground;
struct AirTriggerParams;
struct CameraShotParams;
struct ComplexPasteAction;
struct ReplaceBrushAction;

struct Actor;

struct TerrainRail;

struct EnemyChooser;

struct PlayerTracker;

struct BrushManager;

struct DefaultFileSelector;
struct AdventureCreator;
struct FileChooser;
struct ReplayPlayer;

struct WorkshopUploader;

struct GrassDiff
{
	PolyPtr poly;
	int index;
	int newValue;
	int oldValue;
	int oldType;
	int newType;
};

enum TerrainLayers : int
{
	TERRAINLAYER_NORMAL,
	TERRAINLAYER_WATER,
	TERRAINLAYER_FLY,
	TERRAINLAYER_Count,
};

enum EditLayer : int
{
	LAYER_ACTOR,
	LAYER_SEQUENCE,
	LAYER_IMAGE,
	LAYER_TERRAIN,
	LAYER_WATER,
};

struct EditSession : GUIHandler, Session
{
	enum Emode : int
	{
		CREATE_TERRAIN,
		EDIT,
		SELECT_MODE,
		CREATE_PATROL_PATH,
		CREATE_RECT,
		SET_DIRECTION,
		PASTE,
		PAUSED,
		CREATE_ENEMY,
		CREATE_GATES,
		CREATE_IMAGES,
		CREATE_RAILS,
		SET_CAM_ZOOM,
		TEST_PLAYER,
		TRANSFORM,
		MOVE_BORDER,
		MAP_OPTIONS,
		NETPLAY_TEST_GATHER_USERS,
		NETPLAY_TEST_GET_CONNECTIONS,
		EMODE_Count
	};

	enum Tool
	{
		TOOL_DRAW,
		TOOL_BOX,
		TOOL_BRUSH
	};

	enum TerrainTool : int
	{
		TERRAINTOOL_ADD,
		TERRAINTOOL_SUBTRACT,
		TERRAINTOOL_SETINVERSE,
	};

	enum ImageEditTool
	{
		ITOOL_EDIT,
		ITOOL_ROTATE,
		ITOOL_SCALE
	};

	enum GateAdjustOption
	{
		GATEADJUST_A,
		GATEADJUST_B,
		GATEADJUST_MIDDLE,
		GATEADJUST_POINT_A,
		GATEADJUST_POINT_B,
		GATEADJUST_POINT_MIDDLE
	};

	static EditSession *GetSession();
	static EditSession *currSession;


	WorkshopUploader *workshopUploader;
	MusicInfo *previewMusic;
	MusicSelectorUI *musicSelectorUI;
	ReplayPlayer *debugReplayPlayer;
	bool debugReplayPlayerOn;
	int currGrassType;
	FrameWaiter *removeProgressPointWaiter;
	std::list<Panel*> activePanels;
	Panel *focusedPanel;
	std::map<int, void(EditSession::*)()> handleEventFunctions;
	std::map<int, void(EditSession::*)()> updateModeFunctions;
	std::map<int, void(EditSession::*)()> loseFocusFunctions;
	PlayerTracker *playerTracker;
	const static int MAX_RECENT_ENEMIES = 12;
	std::list<std::pair<ActorType*, int>> recentEnemies;
	Emode mode;
	sf::Vector2f startBoxPos;
	sf::Vertex boxToolQuad[4];
	sf::Color boxToolColor;
	bool boxDrawStarted;
	CreateEnemyModeUI *createEnemyModeUI;
	CreateDecorModeUI *createDecorModeUI;
	CreateTerrainModeUI *createTerrainModeUI;
	CreateRailModeUI *createRailModeUI;
	CreateGatesModeUI *createGatesModeUI;
	MapOptionsUI *mapOptionsUI;
	EditModeUI *editModeUI;
	sf::Text choiceNameText;
	bool showChoiceNameText;
	GeneralUI *generalUI;
	V2d oldWorldPosTest;
	int spriteUpdateFrames;
	TransformTools *transformTools;
	bool initialViewSet;
	sf::Sprite playerZoomIcon;
	ConfirmPopup *confirmPopup;
	MessagePopup *messagePopup;
	bool saveUpdated;
	int tempWriteIndex;
	sf::RenderTexture *mapPreviewTex;
	sf::RenderTexture *brushPreviewTex;
	std::list<Enemy*> allCurrEnemies;
	std::list<Panel*> allPopups;
	Tileset *ts_mapBGThumbnails;
	ImageEditTool currImageTool;
	Tileset *ts_shards[7];
	Tileset *ts_logs;
	int grassSize;
	int grassSpacing;
	GrassDiff *grassChanges;
	TerrainPoint *potentialRailAttachPoint;
	RailPtr potentialRailAttach;
	TerrainPoint *railAttachStartPoint;
	RailPtr railAttachStart;
	TerrainPoint *railAttachEndPoint;
	RailPtr railAttachEnd;
	bool runToResave;
	sf::Sprite scaleSprite;
	sf::Vertex fullBounds[4 * 3];
	std::list<GateInfoPtr> gateInfoList;
	MainMenu *mainMenu;
	PositionInfo worldPosRail;
	PositionInfo worldPosGround;
	V2d worldPos;
	ActorParams* tempActor;
	//MapHeader newMapHeader;
	const static int POINT_SIZE = 4;
	const static double PRIMARY_LIMIT;
	const static double SLIVER_LIMIT;
	int validityRadius;
	sf::Vector2i pointGrabPos;
	sf::Vector2i pointGrabDelta;
	sf::Vector2i oldPointGrabPos;
	bool justCompletedPolyWithClick;
	bool justCompletedRailWithClick;
	sf::View v;
	static double zoomMultiple;
	sf::Vector2f testPoint;
	double minimumEdgeLength;
	std::list<RailPtr> rails;
	int currTerrainWorld[TERRAINLAYER_Count];
	int currTerrainVar[TERRAINLAYER_Count];
	sf::Sprite currTerrainTypeSpr;
	Panel *newMapPanel;
	int terrainGridSize;
	Panel *matTypePanel;
	std::vector<ImageChooseRect*> matTypeRects[TERRAINLAYER_Count];
	int matTypeRectsCurrLayer;
	Panel *shardTypePanel;
	int shardGridSize;
	int shardNumX;
	int shardNumY;
	int logNumX;
	int logNumY;
	int logGridSize;
	Panel *logTypePanel;
	std::vector<ImageChooseRect*> logTypeRects;
	std::vector<ImageChooseRect*> shardTypeRects;
	Panel *nameBrushPanel;
	//allpolygons should hold all the lists but
	//is currently unusued
	std::vector<std::list<PolyPtr>> allPolygons;
	std::list<PolyPtr> polygons;
	std::list<PolyPtr> waterPolygons;
	std::list<PolyPtr> flyPolygons;
	BrushManager *brushManager;
	DefaultFileSelector *fileChooser;
	AdventureCreator *adventureCreator;
	PolyPtr polygonInProgress;
	RailPtr railInProgress;
	sf::Font arialFont;
	sf::Text cursorLocationText;
	sf::Text scaleText;
	sf::RectangleShape scaleSpriteBGRect;
	int enemyEdgeIndex;
	PolyPtr enemyEdgePolygon;
	TerrainRail *enemyEdgeRail;
	double enemyEdgeQuantity;
	int patrolPathLengthSize;
	ActorParams *trackingEnemyParams;
	DecorPtr trackingDecor;
	int enemySelectLevel;
	GridSelector *enemyGrid[4];
	std::vector<sf::Vector2i> patrolPath;
	double minimumPathEdgeLength;
	std::string tempGridResult;
	int tempGridX;
	int tempGridY;
	bool reload;
	bool reloadNew;
	double minZoom;
	double maxZoom;
	bool gameCam;
	int borderMove;
	bool panning;
	Brush *progressBrush;
	std::list<Action*> doneActionStack;
	std::list<Action*> undoneActionStack;
	V2d panAnchor;
	bool showGraph;
	EditorGraph *graph;
	sf::Clock editClock;
	double editAccumulator;
	double editCurrentTime;
	sf::Event ev;
	ComplexPasteAction *complexPaste;
	double brushRepeatDist;
	V2d lastBrushPastePos;
	int pasteAxis;
	PolyPtr gateInProgressTestPoly;
	TerrainPoint *gateInProgressTestPoint;
	sf::Vector2i pointMouseDown;
	sf::Vector2f uiMousePos;
	sf::Vector2i pixelPos;
	Brush *mapStartBrush;
	Brush *selectedBrush;
	Brush *copiedBrush;
	Brush *freeActorCopiedBrush;
	PointMap selectedPoints;
	RailPointMap selectedRailPoints;
	EnemyVariationSelector *variationSelector;
	sf::Vertex border[8];
	CompoundAction *moveAction;
	bool mainWindowLostFocus;
	bool moveActive;
	bool editMouseDownBox;
	bool editMouseDownMove;
	bool editStartMove;
	int editMoveThresh;
	SelectPtr grabbedObject;
	ActorPtr grabbedActor;
	DecorPtr grabbedImage;
	TerrainPoint *grabbedPoint;
	int grabbedBorderIndex;
	sf::Vector2i editMouseGrabPos;
	sf::Vector2i editMouseOrigPos;
	V2d circleTopPos;
	V2d circleUpperRightPos;
	V2d circleLowerRightPos;
	V2d circleUpperLeftPos;
	V2d circleLowerLeftPos;
	V2d circleBottomPos;
	double menuCircleRadius;
	double menuCircleDist;
	std::string menuSelection;
	sf::Sprite guiMenuSprite;
	sf::Font arial;
	V2d menuDownPos;
	Emode menuDownStored;
	Emode stored;
	ActorPtr playerMarkers[MAX_PLAYERS];
	ActorType *playerType;
	int gatePoints;
	bool quit;
	int returnVal;
	ErrorBar errorBar;
	ErrorType mostRecentError;
	GateInfo testGateInfo;
	sf::Vector2i gatePoint0;
	sf::Vector2i gatePoint1;
	GateInfoPtr modifyGate;
	std::list<DecorPtr> decorImages[EffectLayer::EFFECTLAYER_Count];
	Panel *decorPanel;
	Panel *editDecorPanel;
	std::string currDecorName;
	Tileset *ts_currDecor;
	sf::Sprite tempDecorSprite;
	int currDecorLayer;
	int currDecorTile;
	std::map<std::string, std::list<int>> decorTileIndexMap;
	bool drawingCreateRect;
	sf::Vector2i createRectStartPoint;
	sf::Vector2i createRectCurrPoint;
	AirTriggerParams *rectCreatingTrigger;
	CameraShotParams *currentCameraShot;
	int setLevelCurrent;
	//these hold the real bounds while in test mode.
	int realLeftBounds;
	int realTopBounds;
	int realBoundsWidth;
	int realBoundsHeight;

	int origLeftBounds;
	int origTopBounds;
	int origBoundsRight;
	int origBoundsBot;

	EditSession(MainMenu *p_mainMenu,
		const boost::filesystem::path &p_filePath);
	~EditSession();

	void SetBackground(const std::string &bgName);
	int GetMouseOnBorderIndex();
	void ClearActivePanels();
	void AddActivePanel(Panel *p);
	void RemoveActivePanel(Panel *p);
	void HandleEventFunc(int m);
	void UpdateModeFunc(int m);
	void LoseFocusFunc(int m);
	void GeneralEventHandler();
	void GeneralMouseUpdate();
	void DrawPlayerTracker(sf::RenderTarget *target);
	void AddRecentEnemy(ActorPtr a);
	bool IsDrawMode(Emode em);
	void UpdateInputNonGame();
	void FinishEnemyCreation();
	void CancelEnemyCreation();
	void UpdateDecorSprites();
	void SetInitialView(sf::Vector2f &center,
		sf::Vector2f &size);
	bool TestPlayerModeUpdate();
	void TestPlayerMode();
	void CleanupTestPlayerMode();
	void RepPlayerUpdateInput();

	void TestNetplay();




	void PublishMap();

	void UpdateEnvShaders();
	void Load();
	void Init();
	void DefaultInit();
	void TryReloadNew();
	void TrySaveMap();
	void TryExitEditor();
	void SaveMapDialog();
	void OpenMapDialog();
	void ProcessDecorFromFile(const std::string &name,
		int tile);
	void ProcessHeader();
	void ProcessDecorSpr(const std::string &name,
		Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
		float rotation, sf::Vector2f &scale);
	void ProcessTerrain( PolyPtr poly );
	void ProcessSpecialTerrain(PolyPtr poly);
	void ProcessBGTerrain(PolyPtr poly);
	void ProcessRail(RailPtr rail);
	void ProcessActor(ActorPtr a);
	void ProcessGate(int gCat, int gVar,
		int numToOpen,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex, int seconds);
	bool ReadPlayer(std::ifstream &is);
	bool ReadDecor(std::ifstream &is);
	bool ReadBGTerrain(std::ifstream &is);
	bool ReadSpecialTerrain(std::ifstream &is);
	bool ReadGates(std::ifstream &is);
	bool ReadRails(std::ifstream &is);
	bool WriteFile();
	void WritePolygons(std::ofstream &of,
		int bgPlatCount0 );
	void WriteSpecialPolygons(std::ofstream &of);
	void WriteRails(std::ofstream &of);
	void WriteActors(std::ofstream &of);
	void WriteGates(std::ofstream &of);
	void WriteDecor(std::ofstream &of);
	void WriteMapHeader(std::ofstream &of);
	void CreatePreview(sf::Vector2i imageSize);
	void WriteInversePoly(std::ofstream &of);
	void WritePlayerOptions(std::ofstream &of);
	void ClearSelectedPolys();
	void ClearSelectedBrush();
	void SelectObject(SelectPtr sel);
	void DeselectObject(SelectPtr sel);
	void DeselectObjectType(ISelectable::ISelectableType sType);
	void DeselectActorType(const std::string &typeName);
	void ChooseFileOpen(FileChooser *fc,
		const std::string &fileName);
	void ChooseFileSave(FileChooser *fc,
		const std::string &fileName);
	PolyPtr GetPolygon(int index );
	RailPtr GetRail(int index);
	void EndTestMode();
	void SetMode(Emode m);
	void SetBoxTool();
	ActorParams * AttachActorToPolygon( ActorPtr actor, PolyPtr poly );
	ActorParams * AttachActorToRail(ActorPtr actor, RailPtr rail);
	void AttachActorsToPolygon( std::list<ActorPtr> &actors, PolyPtr poly );
	int Run();
	int EditRun();
	void CleanupForReload();
	void LoadAndResave();
	void SetOriginalMusic(const std::string &name);
	void SetPreviewMusic(const std::string &name);
	void PlayMusic(MusicInfo *mi);
	void StopMusic(MusicInfo *mi);
	void StopCurrentMusic();
	void CleanupMusic(MusicInfo *&mi);
	void SnapPointToGraph(sf::Vector2f &p, int gridSize);
	void SnapPointToGraph(V2d &p, int gridSize);
	TerrainPoint * TrySnapPosToPoint(sf::Vector2f &p, SelectPtr &obj, double radius);
	TerrainPoint * TrySnapPosToPoint( V2d &p, 
		SelectPtr &obj, double radius);
	static bool PointOnLine(V2d &pos, V2d &p0, V2d &p1, double width = 0);
	void TryPlaceGatePoint(V2d &pos);
	void RegularOKButton(Panel *p );
	void CreatePathButton(ActorParams *ap = NULL);
	void CreateChainButton(ActorParams *ap = NULL);
	void SetDirectionButton( ActorParams *ap = NULL );
	void SetZoomButton(ActorParams *ap = NULL);
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );
	void SliderCallback(Slider *slider);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	bool IsPolygonExternallyValid( PolyPtr poly,
		 PolyPtr ignore );
	bool IsPolygonValid( PolyPtr poly,
		PolyPtr ignore );
	bool IsEnemyValid(ActorPtr actor);
	void SetEnemyEditPanel();
	void SetDecorEditPanel();
	void SetDecorParams();
	bool CanCreateGate( GateInfo &testGate );
	void SetPanelDefault( ActorType *type );
	void TryKeepGrass(PolyPtr origPoly,
		std::list<PolyPtr> & newPolys);
	void TryKeepGrass(std::list<PolyPtr> & origPolys,
		std::list<PolyPtr> & newPolys);
	void TryKeepGrass(std::set<PolyPtr> & origPolys,
		std::list<PolyPtr> & newPolys);
	void TryAttachActorsToRails(
		std::list<RailPtr> & origRails,
		std::list<RailPtr> & newRails,
		Brush *b);
	void TryAttachActorsToRail(
		RailPtr orig,
		std::list<RailPtr> & newRails,
		Brush *b);
	void TryAttachActorsToPolys(
		std::list<PolyPtr> & origPolys,
		std::list<PolyPtr> & newPolys,
		Brush *b);
	void TryAttachActorsToPolys(
		std::set<PolyPtr> & origPolys,
		std::list<PolyPtr> & newPolys,
		Brush *b);
	void TryAttachActorsToPoly(
		PolyPtr orig,
		std::list<PolyPtr> & newPolys,
		Brush *b);
	bool TryAttachPlayerToPolys(V2d &groundPosition,
		double xoff );
	void TryKeepGates( 
		std::list<GateInfoPtr> &gateInfoList,
		std::list<PolyPtr> &newPolys,
		Brush *b );
	void AddFullPolysToBrush(
		std::list<PolyPtr> & polyList,
		std::list<GateInfoPtr> &gateInfoList,
		Brush *b);
	void AddFullRailsToBrush(
		std::list<RailPtr> &railList,
		Brush *b);
	void AddFullRailToBrush(
		RailPtr rail,
		Brush *b);
	void AddFullPolysToBrush(
		std::set<PolyPtr> & polyList,
		std::list<GateInfoPtr> &gateInfoList,
		Brush *b);
	void AddFullPolyToBrush(
		PolyPtr p, 
		std::list<GateInfoPtr> &gateInfoList,
		Brush *b);
	void ClearSelectedPoints();
	void SelectPoint(PolyPtr poly,
		TerrainPoint *point);
	void SelectPoint(RailPtr rail,
		TerrainPoint *point);
	void DeselectPoint(PolyPtr poly,
		TerrainPoint *point);
	void DeselectPoint(RailPtr rail,
		TerrainPoint *point);
	void SetInversePoly();
	void SetInversePoly(Brush &orig, Brush &result );
	void UpdateFullBounds();
	void AddActorMove(Action *a);
	PositionInfo ConvertPointToGround( sf::Vector2i point,
		ActorPtr actor, Brush *brush );
	PositionInfo ConvertPointToRail(sf::Vector2i point);
	void CreateActor( ActorPtr actor );
	void CreateDecorImage(DecorPtr dec);
	void FusePathClusters(ClipperLib::Path &p,
		ClipperLib::Path &clipperIntersections,
		ClipperIntPointSet &fusedPoints );
	void StartMoveSelectedPoints();
	void MoveSelectedPoints();
	void MoveSelectedRailPoints(V2d worldPos);
	bool PerformMovePointsAction();
	bool IsGateAttachedToAffectedPoints(
		GateInfoPtr gi, PointMover *pm,
		bool &a);
	bool IsGateAttachedToAffectedPoly(
		GateInfoPtr gi, Brush *b,
		bool &a);
	double GetZoomedPointSize();
	void DestroyCopiedBrushes();
	void TryRemoveSelectedPoints();
	bool IsOnlyPlayerSelected();
	bool IsSingleActorSelected();
	void RemoveSelectedObjects();
	void TryRemoveSelectedObjects();
	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	static LineIntersection LimitSegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d, bool firstLimitOnly = false );
	double GetZoomedMinEdgeLength();
	int GetCurrTerrainWorld();
	int GetCurrTerrainVariation();
	void UpdateCurrTerrainType();
	void SetupNewMapPanel();
	
	void ActivateNewMapPanel();
	void SetMatTypePanelLayer(int layer);
	void ModifySelectedTerrainMat(
		int world, int var);
	void SetupTerrainSelectPanel();
	void SetCurrSelectedShardType(int w, int li);
	void SetCurrSelectedLogType(int w, int li);
	void SetupShardSelectPanel();
	void SetupLogSelectPanel();
	void SetupBrushPanels();
	void EditModeDelete();
	void EditModeTransform();
	void EditModeCopy();
	void EditModePaste();
	std::list<PolyPtr> &GetCorrectPolygonList(int ind);
	std::list<PolyPtr> &GetCorrectPolygonList(PolyPtr t);
	std::list<PolyPtr> &GetCorrectPolygonList();
	int GetSpecialTerrainMode();
	void SetTrackingEnemy(ActorType *type, int level);
	void SetTrackingDecor(DecorPtr dec);
	void ChooseRectEvent(ChooseRect *cr, int eventType );
	Panel * CreatePopupPanel( const std::string &p );
	void GetShardWorldAndIndex(int selX, int selY,
		int &w, int &li );
	void SetEnemyGridIndex( GridSelector *gs,
		int x, int y,
		const std::string &eName );
	int IsRemovePointsOkay();
	int CountSelectedPoints();
	void Reload(const boost::filesystem::path &p_filePath);
	void ReloadNew();
	void StartSelectedMove();
	void ContinueSelectedMove();
	void TrySelectedMove();
	void PreventNearPrimaryAnglesOnPolygonInProgress();
	void PreventNearPrimaryAnglesOnRailInProgress();
	void TryAddPointToPolygonInProgress();
	void TryAddPointToRailInProgress();
	void SetSelectedTerrainLayer(int layer);
	void MoveTopBorder(int amount);
	void MoveLeftBorder(int amount);
	void MoveRightBorder(int amount);
	bool IsShowingPoints();
	void ShowGrass(bool s);
	void BackupGrass();
	void ChangeGrassAction();
	void ModifyZoom(double factor);
	void SetZoom(double z);
	void TryAddToPatrolPath();
	void UpdatePanning();
	void UpdatePolyShaders();
	void AddDoneAction(Action *a);
	void ClearUndoneActions();
	void TempMoveSelectedBrush();
	void DrawGraph();
	bool IsGridOn();
	bool IsSnapPointsOn();
	void DrawSpecialTerrain(sf::RenderTarget *target);
	void DrawTerrain(sf::RenderTarget *target);
	void DrawFlyTerrain(sf::RenderTarget *target);
	void DrawRails(sf::RenderTarget *target);
	void DrawPolygonInProgress();
	void DrawRailInProgress();
	void DrawActors();
	void DrawGateInfos();
	void DrawDecor( EffectLayer ef, sf::RenderTarget *target);
	void Draw();
	void DrawUI();
	void Display();
	void DrawBoxSelection();
	void DrawTrackingEnemy();
	void DrawPatrolPathInProgress();
	void DrawGateInProgress();
	void DrawMode();
	void DrawModeUI();
	void UpdateMode();
	void HandleEvents();
	void CreateTerrainModeHandleEvent();
	void CreateRailsModeHandleEvent();
	void EditModeHandleEvent();
	void PasteModeHandleEvent();
	void CreateEnemyModeHandleEvent();
	void PausedModeHandleEvent();
	void SelectModeHandleEvent();
	void CreatePatrolPathModeHandleEvent();
	void CreateRectModeHandleEvent();
	void SetCamZoomModeHandleEvent();
	void SetDirectionModeHandleEvent();
	void CreateGatesModeHandleEvent();
	void CreateImagesModeHandleEvent();
	void MoveBorderModeHandleEvent();
	void TransformModeHandleEvent();
	void TestPlayerModeHandleEvent();
	void NetplayTestGatherUsersModeHandleEvent();
	void NetplayTestGetConnectionsModeHandleEvent();

	void CreateTerrainModeUpdate();
	void CreateRailsModeUpdate();
	void EditModeUpdate();
	void SelectModeUpdate();
	void PasteModeUpdate();
	void CreateEnemyModeUpdate();
	void CreatePatrolPathModeUpdate();
	void CreateRectModeUpdate();
	void SetCamZoomModeUpdate();
	void SetDirectionModeUpdate();
	void CreateGatesModeUpdate();
	void CreateImagesModeUpdate();
	void TransformModeUpdate();
	void MoveBorderModeUpdate();
	void NetplayTestGatherUsersModeUpdate();
	void NetplayTestGetConnectionsModeUpdate();
	


	void UndoMostRecentAction();
	void RedoMostRecentUndoneAction();
	void RemovePointFromPolygonInProgress();
	void ClearPolygonInProgress();
	void RemovePointFromRailInProgress();
	void ClearRailInProgress();
	bool AnchorSelectedEnemies();
	void MoveActors(sf::Vector2i &delta, 
		V2d &grabCenter,
		Brush *brush );
	void FixPathSlivers(ClipperLib::Path &p);
	bool FixPathSlivers(ClipperLib::Path &p,
		ClipperIntPointSet &fusedPoints );
	sf::Vector2i GetPixelPos();
	void TryCompleteSelectedMove();
	void TryCompleteEnemyCreation();
	sf::Vector2i GetCopiedCenter();
	void UpdateCurrEnemyParamsFromPanel();
	void RevertMovedPoints(PointMap::iterator it);
	void RevertMovedPoints(RailPointMap::iterator it);
	V2d GetPlayerSpawnPos( int i );
	void ClearMostRecentError();
	void ShowMostRecentError();
	void CreateError(ErrorType);
	void HideErrorBar();
	bool PointSelectDecor(V2d &pos);
	bool PointSelectActor(  V2d &pos);
	bool PointSelectTerrain(V2d &pos, int terrainLayer );
	bool PointSelectPolyPoint(V2d &pos, int terrainLayer );
	bool PointSelectPoly(V2d &pos, int terrainLayer );
	bool PointSelectRailPoint(V2d &pos);
	bool PointSelectGeneralRail(V2d &pos);
	bool PointSelectRail(V2d &pos);
	void UpdateGrass();
	void ModifyGrass();
	bool BoxSelectPoints(sf::IntRect &rect,
		double radius, int terrainLayer );
	bool BoxSelectActors(sf::IntRect &rect);
	bool BoxSelectDecor(sf::IntRect &rect);
	bool BoxSelectPolys(sf::IntRect &rect, int terrainLayer );
	bool BoxSelectRails(sf::IntRect &rect);
	void TryBoxSelect();
	bool ExecuteTerrainCompletion();
	void ExecuteRailCompletion();
	void GetIntersectingPolys(
		PolyPtr p,
		std::list<PolyPtr> & intersections);
	bool PolyContainsPolys(PolyPtr p,
		PolyPtr ignore );
	bool PolyIsContainedByPolys(PolyPtr p,
		PolyPtr ignore);
	bool PolyIsTouchingEnemiesOrBeingTouched(
		PolyPtr p,
		PolyPtr ignore);
	bool GateIsTouchingEnemies(GateInfo *gi);
	bool PolyIntersectsGates(PolyPtr poly);
	bool GateIntersectsPolys(GateInfo *gi);
	bool GateIntersectsGates(GateInfo *gi);
	bool PolyGatesIntersectOthers(PolyPtr poly);
	bool PolyGatesMakeSliverAngles(PolyPtr poly);
	bool IsGateValid(GateInfo *gi );
	bool IsGateInProgressValid(PolyPtr startPoly,
		TerrainPoint *startPoint);
	bool GateMakesSliverAngles(GateInfo *gi);
	bool IsSliver( TerrainPoint *prev,
		TerrainPoint *curr, 
		TerrainPoint *next);
	bool IsCloseToPrimary(sf::Vector2i &p0,
		sf::Vector2i &p1, sf::Vector2i &prim);
	bool GetPrimaryAdjustment(sf::Vector2i &p0,
		sf::Vector2i &p1, sf::Vector2i &adjust);
	bool PasteInverseTerrain(Brush *cBrush);
	void PasteTerrain(Brush *cBrush, Brush *freeActorBrush );
	bool ExecuteTerrainMultiAdd(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result, int terrainLayer );
	bool ExecuteTerrainMultiSubtract(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result, int terrainLayer );
	bool ExecuteTerrainInverseMultiAdd(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result, int terrainLayer
	);
	bool TryGateAdjustAction( GateAdjustOption option,
		GateInfo *gi, sf::Vector2i &adjust,
		CompoundAction *compound );
	bool TryGateAdjustActionPoly(
		GateInfo *gi,
		sf::Vector2i &adjust, bool a,
		PolyPtr p, CompoundAction *compound);
	bool TryGateAdjustActionPoint(
		GateInfo *gi, sf::Vector2i &adjust,
		bool a, CompoundAction *compound);
	void GetNearPrimaryGateList(
		PointMap &pmap, 
		std::list<GateInfoPtr> &gList);
	bool HoldingShift();
	bool HoldingControl();
	bool HoldingAlt();
	void InitDecorPanel();
	void SetupGates();
	bool IsShardCaptured(int sType);
	bool UpdateRunModeBackAndStartButtons();
	void UpdateCamera();
	bool RunPostUpdate();
	void SequenceGameModeRespondToGoalDestroyed();

	bool GGPOTestPlayerModeUpdate();

	std::string preLevelSceneName;
	std::string postLevelSceneName;

	void SetGameMode(int gm);
	void SetNumPlayers( int num );
	void UpdateTerrainStates();
	void UpdateRailStates();

	Actor *allPlayers[MAX_PLAYERS];
	Panel *ggpoStatsPanel;
	void SetupGGPOStatsPanel();
	void UpdateNetworkStats();

	void RestartGame();
};



#endif