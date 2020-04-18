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
#include "ActorParamsBase.h"
//#include "EditorRail.h"

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
struct Background;
struct ScrollingBackground;
struct AirTriggerParams;
struct CameraShotParams;
struct ComplexPasteAction;
struct ReplaceBrushAction;

struct Actor;

struct TerrainRail;


struct EditSession : GUIHandler, Session
{
	enum Emode
	{
		CREATE_TERRAIN,
		EDIT,
		SELECT_MODE,
		CREATE_PATROL_PATH,
		CREATE_RECT,
		SET_DIRECTION,
		SELECT_POLYGONS,
		PASTE,
		PAUSED,
		CREATE_ENEMY,
		DRAW_PATROL_PATH,
		CREATE_GATES,
		CREATE_IMAGES,
		EDIT_IMAGES,
		SET_LEVEL,
		CREATE_RAILS,
		SET_CAM_ZOOM,
		TEST_PLAYER,
		TRANSFORM,
	};

	Emode mode;

	enum Tool
	{
		TOOL_ADD,
		TOOL_SUBTRACT,
		TOOL_EXTEND,
		TOOL_CUT
	};

	enum ImageEditTool
	{
		ITOOL_EDIT,
		ITOOL_ROTATE,
		ITOOL_SCALE
	};

	V2d oldWorldPosTest;

	TransformTools *transformTools;

	void UpdateDecorSprites();


	void SetInitialView(sf::Vector2f &center,
		sf::Vector2f &size);
	bool initialViewSet;

	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void TestPlayerModeUpdate();
	void TestPlayerMode();

	

	/*QuadTree *terrainBGTree;
	QuadTree *specialTerrainTree;
	QuadTree *barrierTree;
	QuadTree * terrainTree;
	QuadTree * enemyTree;
	QuadTree * grassTree;
	QuadTree * gateTree;
	QuadTree * itemTree;
	QuadTree *envPlantTree;
	QuadTree *specterTree;
	QuadTree *inverseEdgeTree;
	QuadTree *staticItemTree;
	QuadTree *railEdgeTree;
	QuadTree *railDrawTree;
	QuadTree *activeItemTree;
	QuadTree *activeEnemyItemTree;
	QuadTree *airTriggerTree;*/

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
	void ProcessDecorSpr( const std::string &dName,
		sf::Sprite &dSpr, int dLayer, Tileset *d_ts, int dTile);
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

	int tempWriteIndex;

	sf::RenderTexture *mapPreviewTex;

	PolyPtr GetPolygon(int index );
	RailPtr GetRail(int index);


	/*std::list<ParamsInfo> worldEnemyNames[8];
	void AddWorldEnemy(const std::string &name, int w,
		ParamsLoader *pLoader,
		ParamsMaker* pmGround, ParamsMaker *pmAir,
		sf::Vector2i &off, 
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddBasicGroundWorldEnemy(const std::string &name, int w,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddBasicRailWorldEnemy(const std::string &name, int w,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddBasicAerialWorldEnemy(const std::string &name, int w,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	void AddExtraEnemy(const std::string &name,
		ParamsLoader *pLoader,
		ParamsMaker *pmGround, ParamsMaker *pmAir,
		sf::Vector2i &off,
		sf::Vector2i &size,
		bool w_mon,
		bool w_level,
		bool w_path,
		bool w_loop,
		int p_numLevels = 1,
		Tileset *ts = NULL,
		int tileIndex = 0);
	std::list<ParamsInfo> extraEnemyNames;

	void AddGeneralEnemies();
	void AddW1Enemies();
	void AddW2Enemies();
	void AddW3Enemies();
	void AddW4Enemies();
	void AddW5Enemies();
	void AddW6Enemies();*/
	
	std::list<Enemy*> allCurrEnemies;
	void EndTestMode();

	bool IsKeyPressed(int k);
	bool IsMousePressed(int m);

	void SetMode(Emode m);

	std::list<Panel*> allPopups;

	void SetupTerrainTypeSelector();

	const static int TERRAIN_WORLDS = 9;
	const static int MAX_TERRAINTEX_PER_WORLD = 10;
	
	sf::Sprite currTerrainTypeSpr;
	//int currTerrainInd;
	int currTerrainWorld;
	int currTerrainVar;
	void UpdateCurrTerrainType();

	Tool currTool;
	ImageEditTool currImageTool;

	Tileset *ts_shards[7];
	
	//need to clean this up
	int grassSize;
	int grassSpacing;


	
	ActorParams * AttachActorToPolygon( ActorPtr actor, PolyPtr poly );
	void AttachActorsToPolygon( std::list<ActorPtr> &actors, PolyPtr poly );
	int Run();
	
	void SnapPointToGraph(sf::Vector2f &p, int gridSize);
	void SnapPointToGraph(V2d &p, int gridSize);
	bool TrySnapPosToPoint(sf::Vector2f &p, double radius,
		PolyPtr &poly, TerrainPoint *&point );
	bool TrySnapPosToPoint( V2d &p, double radius,
		PolyPtr &poly, TerrainPoint *&point);
	static bool PointOnLine(V2d &pos, V2d &p0, V2d &p1, double width = 0);
	void TryPlaceGatePoint(V2d &pos);
	
	void RegularOKButton();
	void RegularCreatePathButton();
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );

	bool IsPolygonExternallyValid( PolyPtr poly,
		 PolyPtr ignore );
	bool IsPolygonValid( PolyPtr poly,
		PolyPtr ignore );
	void SetEnemyEditPanel();
	void SetDecorEditPanel();
	void SetDecorParams();
	static bool QuadPolygonIntersect(PolyPtr poly,
		sf::Vector2i a, sf::Vector2i b, 
		sf::Vector2i c, sf::Vector2i d );
	bool CanCreateGate( GateInfo &testGate );
	void SetPanelDefault( ActorType *type );

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
	void TryKeepGates( 
		std::list<GateInfoPtr> &gateInfoList,
		std::list<PolyPtr> &newPolys,
		Brush *b );
	void AddFullPolysToBrush(
		std::list<PolyPtr> & polyList,
		std::list<GateInfoPtr> &gateInfoList,
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

	

	GroundInfo ConvertPointToGround( sf::Vector2i point );
	GroundInfo ConvertPointToRail(sf::Vector2i point);
	void CreateActor( ActorPtr actor );
	void CreateDecorImage(DecorPtr dec);

	std::list<GateInfoPtr> gates;
	MainMenu *mainMenu;
	
	void FusePathClusters(ClipperLib::Path &p,
		ClipperLib::Path &clipperIntersections,
		ClipperIntPointSet &fusedPoints );


	void MoveSelectedPoints( V2d worldPos );
	void MoveSelectedRailPoints(V2d worldPos);
	void PerformMovePointsAction();

	bool IsGateAttachedToAffectedPoints(
		GateInfoPtr gi, PointVectorMap &pm,
		bool &a);
	bool IsGateAttachedToAffectedPoints(
		GateInfoPtr gi, PointMap &pm,
		bool &a);
	bool IsGateAttachedToAffectedPoly(
		GateInfoPtr gi, Brush *b,
		bool &a);

	GroundInfo worldPosRail;
	GroundInfo worldPosGround;
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

	bool showGrass;
	bool showPoints;

	bool justCompletedPolyWithClick;

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

	std::list<PolyPtr> &GetCorrectPolygonList(int ind);
	std::list<PolyPtr> &GetCorrectPolygonList(PolyPtr t);
	std::list<PolyPtr> &GetCorrectPolygonList();
	std::list<PolyPtr> polygons;
	std::list<PolyPtr> waterPolygons;

	bool IsSpecialTerrainMode();

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

	sf::Sprite enemySprite;
	sf::RectangleShape enemyQuad;
	ActorType *trackingEnemy;//bool trackingEnemy;
	Panel *showPanel;	

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
	void SetActiveEnemyGrid(int i);

	int IsRemovePointsOkay();

	Panel *CreateOptionsPanel( const std::string &name );
	void WriteGrass( PolyPtr p, std::ofstream &of );
	int CountSelectedPoints();

	std::list<sf::Vector2i> patrolPath;
	double minimumPathEdgeLength;
	
	bool ConfirmationPop( const std::string &question );
	void GridSelectPop( const std::string &type );
	std::string tempGridResult;
	int tempGridX;
	int tempGridY;
	void MessagePop( const std::string &message );
	void ErrorPop( const std::string &error );

	void StartTerrainMove();
	void ContinueTerrainMove();
	void TryTerrainMove();

	void PreventNearPrimaryAnglesOnPolygonInProgress();
	void PreventNearPrimaryAnglesOnRailInProgress();
	void TryAddPointToPolygonInProgress();
	void TryAddPointToRailInProgress();

	void SetSelectedTerrainLayer(int layer);

	void MoveTopBorder(int amount);
	void MoveLeftBorder(int amount);
	void MoveRightBorder(int amount);

	

	void ShowGrass(bool s);

	void ModifyZoom(double factor);
	double minZoom;
	double maxZoom;

	float oldShaderZoom;
	sf::Vector2f oldShaderBotLeft;

	void TryPlaceTrackingEnemy();
	void AnchorTrackingEnemyOnTerrain();
	void MoveTrackingEnemy();

	void TryAddToPatrolPath();

	void UpdatePanning();

	void UpdatePolyShaders();

	void SetEnemyLevel();

	int borderMove;
	bool panning;

	Brush *progressBrush;
	std::list<Action*> doneActionStack;
	std::list<Action*> undoneActionStack;
	void ClearUndoneActions();

	void TempMoveSelectedBrush();
	
	void DrawGraph();
	void SetupGraph();

	bool IsPathWithinPath(
		ClipperLib::Path &pInner,
		ClipperLib::Path &pOuter);
	V2d panAnchor;
	bool showGraph;

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
	void CreateImagesHandleEvent();
	//void EditImagesModeHandleEvent();
	void SetLevelModeHandleEvent();
	void TransformModeHandleEvent();

	void CreateTerrainModeUpdate();
	void CreateRailsModeUpdate();
	void EditModeUpdate();
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
	void SetLevelModeUpdate();
	void TransformModeUpdate();
	
	
	sf::Event ev;
	void UndoMostRecentAction();
	void RedoMostRecentUndoneAction();

	void RemovePointFromPolygonInProgress();
	void RemovePointFromRailInProgress();

	bool AnchorSelectedAerialEnemy();

	void MoveSelectedActor( sf::Vector2i &delta );

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

	void TryMoveSelectedBrush();

	Brush *mapStartBrush;
	Brush *selectedBrush;
	Brush *copiedBrush;
	PointMap selectedPoints;
	RailPointMap selectedRailPoints;
	

	sf::Vertex border[8];

	//----------------------

	CompoundAction *moveAction;

	bool moveActive;
	bool editMouseDownBox;
	bool editMouseDownMove;
	bool editStartMove;
	int editMoveThresh;
	SelectPtr grabbedObject;
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
	

	GateInfo testGateInfo;
	sf::Vector2i gatePoint0;
	sf::Vector2i gatePoint1;
	GateInfoPtr modifyGate;
	
	bool PointSelectDecor(V2d &pos);
	bool PointSelectActor(  V2d &pos);
	bool PointSelectTerrain(V2d &pos);
	bool PointSelectPolyPoint( V2d &pos );
	bool PointSelectPoly(V2d &pos);
	bool PointSelectRailPoint(V2d &pos);
	bool PointSelectGeneralRail(V2d &pos);
	bool PointSelectRail(V2d &pos);

	void UpdateGrass();
	void ModifyGrass();

	bool BoxSelectPoints(sf::IntRect &rect,
		double radius);
	bool BoxSelectActors(sf::IntRect &rect);
	bool BoxSelectDecor(sf::IntRect &rect);
	bool BoxSelectPolys(sf::IntRect &rect);
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
	void PasteTerrain(Brush *b);
	bool ExecuteTerrainMultiAdd(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result );
	bool ExecuteTerrainMultiSubtract(
		std::list<PolyPtr> &brushPolys,
		Brush &orig,
		Brush &result);
	
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

	bool decorTracking;
	int *decorTileIndexes;
	std::map<std::string, std::list<int>> decorTileIndexMap;


	bool drawingCreateRect;
	sf::Vector2i createRectStartPoint;
	sf::Vector2i createRectCurrPoint;
	AirTriggerParams *rectCreatingTrigger;
	CameraShotParams *currentCameraShot;

	int setLevelCurrent;

	
	
};



#endif