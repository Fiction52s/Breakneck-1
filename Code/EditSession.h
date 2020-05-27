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
#include "ShardTypes.h"
#include "Gate.h"
#include "ISelectable.h"

#include "EditorGateInfo.h"
#include "EditorActors.h"
#include "EditorTerrain.h"
#include "Session.h"
#include "PositionInfo.h"
//#include "ActorParamsBase.h"
//#include "EditorRail.h"


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

struct GrassDiff
{
	PolyPtr poly;
	int index;
	int newValue;
	int oldValue;
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
		EMODE_Count
	};

	FrameWaiter *removeProgressPointWaiter;

	void ClearActivePanels();
	void AddActivePanel(Panel *p);
	void RemoveActivePanel(Panel *p);
	std::list<Panel*> activePanels;
	Panel *focusedPanel;
	std::map<int, void(EditSession::*)()> handleEventFunctions;
	std::map<int, void(EditSession::*)()> updateModeFunctions;
	void HandleEventFunc(int m);
	void UpdateModeFunc(int m);
	void GeneralEventHandler();
	void GeneralMouseUpdate();
	void DrawPlayerTracker(sf::RenderTarget *target);
	PlayerTracker *playerTracker;

	const static int MAX_RECENT_ENEMIES = 12;
	std::list<std::pair<ActorType*, int>> recentEnemies;
	void AddRecentEnemy(ActorPtr a);

	bool IsDrawMode(Emode em);
	Emode mode;
	void UpdateInputNonGame();
	

	enum Tool
	{
		TOOL_DRAW,
		TOOL_BOX
	};

	enum TerrainTool : int
	{
		TERRAINTOOL_ADD,
		TERRAINTOOL_SUBTRACT,
		TERRAINTOOL_SETINVERSE,
	};

	sf::Vector2f startBoxPos;
	sf::Vertex boxToolQuad[4];
	sf::Color boxToolColor;
	bool boxDrawStarted;

	enum ImageEditTool
	{
		ITOOL_EDIT,
		ITOOL_ROTATE,
		ITOOL_SCALE
	};

	//ActorPtr 


	CreateEnemyModeUI *createEnemyModeUI;
	CreateDecorModeUI *createDecorModeUI;
	CreateTerrainModeUI *createTerrainModeUI;
	CreateGatesModeUI *createGatesModeUI;
	EditModeUI *editModeUI;

	//EnemyChooser *enemyChooser;

	V2d oldWorldPosTest;

	int spriteUpdateFrames;

	TransformTools *transformTools;

	void FinishEnemyCreation();
	void CancelEnemyCreation();

	void UpdateDecorSprites();

	

	void SetInitialView(sf::Vector2f &center,
		sf::Vector2f &size);
	bool initialViewSet;

	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void TestPlayerModeUpdate();
	void TestPlayerMode();

	EditSession(MainMenu *p_mainMenu,
		const boost::filesystem::path &p_filePath );
	
	~EditSession();

	//singleton
	static EditSession *GetSession();
	static EditSession *currSession;

	//process stuff read from file
	void ProcessDecorFromFile(const std::string &name,
		int tile);
	void ProcessHeader();
	void ProcessDecorSpr(const std::string &name,
		Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
		float rotation, sf::Vector2f &scale);
	void ProcessPlayerStartPos();
	void ProcessTerrain( PolyPtr poly );
	void ProcessSpecialTerrain(PolyPtr poly);
	void ProcessBGTerrain(PolyPtr poly);
	void ProcessRail(RailPtr rail);

	void ProcessActor(ActorPtr a);

	void ProcessGate(int gType,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex);

	//void ProcessActorGroup(ActorGroup *group);
	//void ProcessActor(const std::string &groupName, int numActors,
	//	const std::string &actorType);
	//file stuff
	//bool OpenFile();
	//bool ReadHeader(std::ifstream &is);
	bool ReadPlayer(std::ifstream &is);
	bool ReadDecor(std::ifstream &is);
	//bool ReadTerrain(std::ifstream &is);
	bool ReadBGTerrain(std::ifstream &is);
	bool ReadSpecialTerrain(std::ifstream &is);
	
	bool ReadGates(std::ifstream &is);
	bool ReadRails(std::ifstream &is);
	void WriteFile(std::string fileName);
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

	void ClearSelectedPolys();
	void ClearSelectedBrush();
	void SelectObject(SelectPtr sel);
	void DeselectObject(SelectPtr sel);
	void DeselectObjectType(ISelectable::ISelectableType sType);

	int tempWriteIndex;

	sf::RenderTexture *mapPreviewTex;

	PolyPtr GetPolygon(int index );
	RailPtr GetRail(int index);

	std::list<Enemy*> allCurrEnemies;
	void EndTestMode();

	bool IsKeyPressed(int k);
	bool IsMousePressed(int m);

	void SetMode(Emode m);

	std::list<Panel*> allPopups;

	void SetupTerrainTypeSelector();

	const static int TERRAIN_WORLDS = 10;
	const static int MAX_TERRAINTEX_PER_WORLD = 10;
	const static int TOTAL_TERRAIN_TEXTURES =
		TERRAIN_WORLDS * MAX_TERRAINTEX_PER_WORLD;
	
	

	Tool currTool;
	ImageEditTool currImageTool;

	Tileset *ts_shards[7];
	
	//need to clean this up
	int grassSize;
	int grassSpacing;
	GrassDiff *grassChanges;

	TerrainPoint *potentialRailAttachPoint;
	RailPtr potentialRailAttach;

	TerrainPoint *railAttachStartPoint;
	RailPtr railAttachStart;

	TerrainPoint *railAttachEndPoint;
	RailPtr railAttachEnd;

	
	ActorParams * AttachActorToPolygon( ActorPtr actor, PolyPtr poly );
	ActorParams * AttachActorToRail(ActorPtr actor, RailPtr rail);
	void AttachActorsToPolygon( std::list<ActorPtr> &actors, PolyPtr poly );
	int Run();
	void LoadAndResave();
	bool runToResave;

	
	void SnapPointToGraph(sf::Vector2f &p, int gridSize);
	void SnapPointToGraph(V2d &p, int gridSize);
	TerrainPoint * TrySnapPosToPoint(sf::Vector2f &p, SelectPtr &obj, double radius);
	TerrainPoint * TrySnapPosToPoint( V2d &p, 
		SelectPtr &obj, double radius);
	static bool PointOnLine(V2d &pos, V2d &p0, V2d &p1, double width = 0);
	void TryPlaceGatePoint(V2d &pos);
	
	void RegularOKButton(Panel *p );
	void RegularCreatePathButton( Panel *p );
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );
	void SliderCallback(Slider *slider, const std::string & e);
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
	sf::Sprite scaleSprite;

	//mapheader stuff. make another mapheader to hold this?
	std::string envName;
	int envWorldType;
	int leftBound;
	int topBound;
	int boundWidth;
	int boundHeight;
	int drainSeconds;
	int bossType;
	
	sf::VertexArray fullBounds;
	void UpdateFullBounds();

	void AddActorMove(Action *a);

	PositionInfo ConvertPointToGround( sf::Vector2i point,
		ActorPtr actor, Brush *brush );
	PositionInfo ConvertPointToRail(sf::Vector2i point);
	void CreateActor( ActorPtr actor );
	void CreateDecorImage(DecorPtr dec);

	std::list<GateInfoPtr> gates;
	MainMenu *mainMenu;
	
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
	//bool IsGateAttachedToAffectedPoints(
	//	GateInfoPtr gi, PointMover *pm,
	//	bool &a);
	bool IsGateAttachedToAffectedPoly(
		GateInfoPtr gi, Brush *b,
		bool &a);

	PositionInfo worldPosRail;
	PositionInfo worldPosGround;
	V2d worldPos;
	ActorParams* tempActor;
	MapHeader newMapHeader;

	const static int POINT_SIZE = 4;
	double GetZoomedPointSize();

	const static double PRIMARY_LIMIT;
	const static double SLIVER_LIMIT;
	sf::RenderTexture *preScreenTex;
	int validityRadius;
	
	sf::Vector2i pointGrabPos;
	sf::Vector2i pointGrabDelta;
	sf::Vector2i oldPointGrabPos;


	bool justCompletedPolyWithClick;
	bool justCompletedRailWithClick;
	bool lastLeftMouseDown;

	void TryRemoveSelectedPoints();
	bool IsOnlyPlayerSelected();
	bool IsSingleActorSelected();

	void RemoveSelectedObjects();
	void TryRemoveSelectedObjects();

	sf::View v;
	
	//sf::Vector2i goalPosition;
	std::string currentFile;
	static double zoomMultiple;
	sf::Vector2f testPoint;
	

	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	static LineIntersection LimitSegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d, bool firstLimitOnly = false );


	double minimumEdgeLength;

	double GetZoomedMinEdgeLength();
	//double minAngle;
	
	std::list<RailPtr> rails;

	

	int currTerrainWorld[TERRAINLAYER_Count];
	int currTerrainVar[TERRAINLAYER_Count];
	int GetCurrTerrainWorld();
	int GetCurrTerrainVariation();
	Tileset *GetMatTileset(int tWorld, int tVar);

	sf::Sprite currTerrainTypeSpr;

	void UpdateCurrTerrainType();

	////search for terrainlayer_ get layer_
	//std::map<int, int> terrainEditLayerMap;
	//
	////search for layer_ get terrainlayer_
	//std::map<int, int> terrainEditLayerReverseMap;

	std::list<PolyPtr> &GetCorrectPolygonList(int ind);
	std::list<PolyPtr> &GetCorrectPolygonList(PolyPtr t);
	std::list<PolyPtr> &GetCorrectPolygonList();
	//allpolygons should hold all the lists
	std::vector<std::list<PolyPtr>> allPolygons;
	std::list<PolyPtr> polygons;
	std::list<PolyPtr> waterPolygons;
	std::list<PolyPtr> flyPolygons;

	int GetSpecialTerrainMode();
	

	PolyPtr polygonInProgress;
	RailPtr railInProgress;
	PolyPtr inversePolygon;
	
	sf::Font arialFont;
	sf::Text cursorLocationText;
	sf::Text scaleText;
	sf::RectangleShape scaleSpriteBGRect;

	int enemyEdgeIndex;
	PolyPtr enemyEdgePolygon;
	TerrainRail *enemyEdgeRail;
	double enemyEdgeQuantity;
	
	int patrolPathLengthSize;

	void SetTrackingEnemy(ActorType *type, int level);
	ActorParams *trackingEnemyParams;

	void SetTrackingDecor(DecorPtr dec);
	DecorPtr trackingDecor;

	void ChooseRectEvent(ChooseRect *cr, int eventType );
	

	Panel * CreatePopupPanel( const std::string &p );
	Panel *messagePopup;
	Panel *errorPopup;
	Panel *bgPopup;
	Panel *shardSelectPopup;
	Panel *gateSelectorPopup;
	Panel *terrainSelectorPopup;
	Panel *enemySelectPanel;

	Panel *mapOptionsPanel;
	Panel *terrainOptionsPanel;
	Panel *railOptionsPanel;

	void CreateShardGridSelector( Panel *p,
		sf::Vector2i &pos );
	void GetShardWorldAndIndex(int selX, int selY,
		int &w, int &li );

	int enemySelectLevel;
	GridSelector *enemyGrid[4];
	void SetEnemyGridIndex( GridSelector *gs,
		int x, int y,
		const std::string &eName );

	int IsRemovePointsOkay();

	Panel *CreateOptionsPanel( const std::string &name );
	int CountSelectedPoints();

	std::vector<sf::Vector2i> patrolPath;
	double minimumPathEdgeLength;
	
	bool ConfirmationPop( const std::string &question );
	void GridSelectPop( const std::string &type );
	std::string tempGridResult;
	int tempGridX;
	int tempGridY;
	void MessagePop( const std::string &message );
	void ErrorPop( const std::string &error );

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

	//void UpdateInput()
	bool IsShowingPoints();

	void ShowGrass(bool s);

	void ModifyZoom(double factor);
	void SetZoom(double z);
	double minZoom;
	double maxZoom;

	bool gameCam;

	float oldShaderZoom;
	sf::Vector2f oldShaderBotLeft;

	void TryAddToPatrolPath();

	void UpdatePanning();

	void UpdatePolyShaders();

	int borderMove;
	bool panning;

	Brush *progressBrush;
	std::list<Action*> doneActionStack;
	std::list<Action*> undoneActionStack;
	void AddDoneAction(Action *a);
	void ClearUndoneActions();

	void TempMoveSelectedBrush();
	
	void DrawGraph();
	void SetupGraph();

	bool IsPathWithinPath(
		ClipperLib::Path &pInner,
		ClipperLib::Path &pOuter);
	V2d panAnchor;
	bool showGraph;

	bool IsGridOn();
	bool IsSnapPointsOn();

	EditorGraph *graph;



	void DrawPolygons();
	void DrawRails();
	void DrawPolygonInProgress();
	void DrawRailInProgress();

	void DrawActors();
	void DrawGates();
	void DrawDecorBehind();
	void DrawDecorBetween();
	void DrawDecorFront();

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
	//void EditImagesModeHandleEvent();
	void TransformModeHandleEvent();
	void TestPlayerModeHandleEvent();

	void CreateTerrainModeUpdate();
	void CreateRailsModeUpdate();
	void EditModeUpdate();
	void SelectModeUpdate();
	void PasteModeUpdate();
	void CreateEnemyModeUpdate();
	//void PausedModeUpdate();
	//void SelectModeUpdate();
	void CreatePatrolPathModeUpdate();
	void CreateRectModeUpdate();
	void SetCamZoomModeUpdate();
	void SetDirectionModeUpdate();
	void CreateGatesModeUpdate();
	void CreateImagesModeUpdate();
	//void EditImagesModeUpdate();
	void TransformModeUpdate();
	
	
	sf::Clock editClock;
	double editAccumulator;
	double editCurrentTime;

	sf::Event ev;
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

	ComplexPasteAction *complexPaste;
	double brushRepeatDist;
	V2d lastBrushPastePos;
	int pasteAxis;

	PolyPtr gateInProgressTestPoly;
	TerrainPoint *gateInProgressTestPoint;


	sf::Vector2i pointMouseDown;
	sf::Vector2f uiMousePos;

	sf::Vector2i pixelPos;
	sf::Vector2i GetPixelPos();

	void TryCompleteSelectedMove();

	Brush *mapStartBrush;
	Brush *selectedBrush;
	Brush *copiedBrush;
	Brush *freeActorCopiedBrush;
	sf::Vector2i GetCopiedCenter();
	PointMap selectedPoints;
	RailPointMap selectedRailPoints;
	
	EnemyVariationSelector *variationSelector;

	sf::Vertex border[8];

	//----------------------
	
	CompoundAction *moveAction;

	void RevertMovedPoints(PointMap::iterator it);
	void RevertMovedPoints(RailPointMap::iterator it);
	//MovePointsAction *movePointsAction;

	bool moveActive;
	bool editMouseDownBox;
	bool editMouseDownMove;
	bool editStartMove;
	int editMoveThresh;
	SelectPtr grabbedObject;
	ActorPtr grabbedActor;
	DecorPtr grabbedImage;
	TerrainPoint *grabbedPoint;
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

	ActorPtr player;
	V2d GetPlayerSpawnPos();
	ActorType *playerType;
	//std::list<Action*>::iterator currAction;

	enum ConfirmChoices
	{
		NONE,
		CONFIRM,
		CANCEL
	};
	ConfirmChoices confirmChoice; 

	//0 for no choice
	//1 for confirm
	//2 for cancel

	Panel *confirm;
	sf::View view;
	
	
	

	int gatePoints;

	bool quit;
	int returnVal;
	
	ErrorBar errorBar;
	void ClearMostRecentError();
	void ShowMostRecentError();
	ErrorType mostRecentError;
	void CreateError(ErrorType);
	void HideErrorBar();

	GateInfo testGateInfo;
	sf::Vector2i gatePoint0;
	sf::Vector2i gatePoint1;
	GateInfoPtr modifyGate;
	
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
	void PasteTerrain(Brush *cBrush, Brush *freeActorBrush );
	bool ExecuteTerrainMultiAdd(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result, int terrainLayer );
	bool ExecuteTerrainMultiSubtract(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result, int terrainLayer );
	
	enum GateAdjustOption
	{
		GATEADJUST_A,
		GATEADJUST_B,
		GATEADJUST_MIDDLE,
		GATEADJUST_POINT_A,
		GATEADJUST_POINT_B,
		GATEADJUST_POINT_MIDDLE
	};
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

	std::list<DecorPtr> decorImagesBehindTerrain;
	std::list<DecorPtr> decorImagesBetween;
	std::list<DecorPtr> decorImagesFrontTerrain;
	Panel *decorPanel;
	Panel *editDecorPanel;
	void InitDecorPanel();
	std::string currDecorName;
	Tileset *ts_currDecor;
	sf::Sprite tempDecorSprite;
	int currDecorLayer;
	int currDecorTile;

	//int *decorTileIndexes;
	std::map<std::string, std::list<int>> decorTileIndexMap;


	bool drawingCreateRect;
	sf::Vector2i createRectStartPoint;
	sf::Vector2i createRectCurrPoint;
	AirTriggerParams *rectCreatingTrigger;
	CameraShotParams *currentCameraShot;

	int setLevelCurrent;

	//create enmy mode vars

	
};



#endif