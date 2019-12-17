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
struct KinSkin;
struct Background;
struct ScrollingBackground;
struct AirTriggerParams;


struct EditorBG;

struct EditSession : GUIHandler, TilesetManager
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

	
	
	EditSession(MainMenu *p_mainMenu);
	~EditSession();

	//singleton
	static EditSession *GetSession();
	static EditSession *currSession;

	//file stuff
	bool OpenFile();
	void WriteFile(std::string fileName);
	void CreatePreview(sf::Vector2i imageSize);
	sf::RenderTexture *mapPreviewTex;




	EditorBG *background;
	

	TerrainPolygon *GetPolygon(int index, int &edgeIndex);

	std::list<ParamsInfo> worldEnemyNames[8];
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
	void AddW6Enemies();
	

	void SetupEnemyTypes();
	void SetupEnemyType(ParamsInfo &pi);

	bool IsKeyPressed(int k);
	bool IsMousePressed(int m);

	void SetMode(Emode m);

	std::list<Panel*> allPopups;

	const static int MAX_TERRAINTEX_PER_WORLD = 10;
	sf::Texture *terrainTextures[9 * MAX_TERRAINTEX_PER_WORLD];
	sf::Shader polyShaders[9 * MAX_TERRAINTEX_PER_WORLD];


	Tool currTool;
	ImageEditTool currImageTool;

	Tileset *ts_shards[7];
	
	//need to clean this up
	int grassSize;
	int grassSpacing;


	
	ActorParams * AttachActorToPolygon( ActorPtr actor, TerrainPolygon *poly );
	void AttachActorsToPolygon( std::list<ActorPtr> &actors, TerrainPolygon *poly );
	int Run(const boost::filesystem::path &p_filePath,
		sf::Vector2f cameraPos, 
		sf::Vector2f cameraSize );
	
	sf::Vector2f SnapPointToGraph(sf::Vector2f &p, int gridSize);
	V2d SnapPointToGraph(V2d &p, int gridSize);
	sf::Vector2f SnapPosToPoint(sf::Vector2f &p, double radius);
	static bool PointOnLine(V2d &pos, V2d &p0, V2d &p1, double width = 0);
	void TryPlaceGatePoint(V2d &pos);
	
	void RegularOKButton();
	void RegularCreatePathButton();
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );

	bool IsPolygonExternallyValid( TerrainPolygon &poly,
		 TerrainPolygon* ignore );
	bool IsPolygonInternallyValid( TerrainPolygon &poly );
	bool IsPolygonValid( TerrainPolygon &poly,
		TerrainPolygon* ignore );
	void SetEnemyEditPanel();
	void SetDecorEditPanel();
	void SetDecorParams();
	bool QuadPolygonIntersect( TerrainPolygon* poly, 
		sf::Vector2i a, sf::Vector2i b, 
		sf::Vector2i c, sf::Vector2i d );
	bool CanCreateGate( GateInfo &testGate );
	void SetPanelDefault( ActorType *type );
	void ClearSelectedPoints();
	void SelectPoint(TerrainPolygon *poly,
		TerrainPoint *point);
	void DeselectPoint(TerrainPolygon *poly,
		TerrainPoint *point);
	bool PolyIntersectGate( TerrainPolygon &poly );

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
	void CreateActor( ActorPtr actor );
	void CreateDecorImage(
		EditorDecorPtr dec);


	std::list<GateInfoPtr> gates;
	MainMenu *mainMenu;
	
	void MoveSelectedPoints( V2d worldPos );
	void PerformMovePointsAction();

	GroundInfo worldPosGround;
	V2d worldPos;
	ActorParams* tempActor;
	MapHeader mapHeader;

	const static double PRIMARY_LIMIT;
	sf::RenderTexture *preScreenTex;
	int validityRadius;
	
	sf::Texture grassTex;
	sf::Vector2i pointGrabPos;
	sf::Vector2i pointGrabDelta;

	bool showGrass;
	bool showPoints;

	TerrainPolygon *cutPoly0;
	TerrainPolygon *cutPoly1;
	void ChooseCutPoly( TerrainPolygon *choice );
	bool cutChoose;
	bool cutChooseUp;
	void CutPoly();

	void TryRemoveSelectedPoints();
	bool IsOnlyPlayerSelected();
	bool IsSingleActorSelected();

	void RemoveSelectedObjects();
	void TryRemoveSelectedObjects();

	sf::View v;
	
	sf::RenderWindow *w;
	//sf::Vector2i goalPosition;
	std::string currentFile;
	boost::filesystem::path currentPath;
	static double zoomMultiple;
	sf::Vector2f testPoint;
	std::map<std::string, ActorGroup*> groups;
	std::map<std::string, ActorType*> types;

	//CREATE_TERRAIN mode
	enum AddResult
	{
		ADD_SUCCESS,
		ADD_INVALID_RESULT,
		ADD_NO_CHANGE,
		ADD_
	};
	AddResult Add( boost::shared_ptr<TerrainPolygon> brush,
		boost::shared_ptr<TerrainPolygon> poly, TerrainPolygon *&outPoly,
		std::list<boost::shared_ptr<GateInfo>> &gateInfoList );
	AddResult InverseAdd(boost::shared_ptr<TerrainPolygon> brush,
		boost::shared_ptr<TerrainPolygon> poly, std::list<TerrainPolygon*> &outPolyList);
	TerrainPoint * GetNextAddPoint(
		TerrainPoint *previousPoint,
		sf::Vector2i &stargSegPos, TerrainPoint *&curr,
		PolyPtr &currPoly, PolyPtr &otherPoly, bool &skipBorderCase,
		bool &replaceLastPoint );
	void Sub(PolyPtr brushPtr,
		std::list<PolyPtr> &polys,
		std::list<PolyPtr> &results );
	void Extend(boost::shared_ptr<TerrainPolygon> extension,
		boost::shared_ptr<TerrainPolygon> poly );
	
	bool PointValid( sf::Vector2i prev, sf::Vector2i point );

	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	static LineIntersection LimitSegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d, bool firstLimitOnly = false );

	double minimumEdgeLength;
	//double minAngle;
	
	std::list<boost::shared_ptr<TerrainPolygon>> polygons;
	std::list<boost::shared_ptr<TerrainPolygon>> selectedPolygons;
	
	boost::shared_ptr<TerrainPolygon> polygonInProgress;
	boost::shared_ptr<TerrainPolygon> inversePolygon;
	
	sf::Font arialFont;
	sf::Text cursorLocationText;
	sf::Text scaleText;
	sf::RectangleShape scaleSpriteBGRect;

	int enemyEdgeIndex;
	TerrainPolygon *enemyEdgePolygon;
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
	void TryAddPointToPolygonInProgress();

	void SetSelectedTerrainLayer(int layer);

	void MoveTopBorder(int amount);
	void MoveLeftBorder(int amount);
	void MoveRightBorder(int amount);

	void ShowGrass(bool s);

	void ModifyZoom(double factor);
	double minZoom;
	double maxZoom;

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
	void MovePasteBrushes();
	void DrawPasteBrushes();

	void TempMoveSelectedBrush();
	
	void DrawGraph();
	void SetupGraph();
	V2d panAnchor;
	bool showGraph;
	sf::Color graphColor;

	sf::VertexArray *graphLinesVA;
	int numGraphLines;

	void DrawPolygons();
	void DrawPolygonInProgress();

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
	
	sf::Event ev;
	void UndoMostRecentAction();
	void RedoMostRecentUndoneAction();

	void RemovePointFromPolygonInProgress();

	bool AnchorSelectedAerialEnemy();

	void MoveSelectedActor( sf::Vector2i &delta );

	sf::Vector2i pointMouseDown;
	sf::Vector2f uiMousePos;

	sf::Vector2i pixelPos;
	sf::Vector2i GetPixelPos();

	void TryMoveSelectedBrush();


	Brush *selectedBrush;
	Brush *copiedBrush;
	PointMap selectedPoints;

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
	sf::View uiView;
	sf::View view;
	
	std::list<TerrainBrush*> copyBrushes;
	std::list<TerrainBrush*> pasteBrushes;
	void ClearCopyBrushes();
	void ClearPasteBrushes();
	void CopyToPasteBrushes();
	sf::Vector2i pastePos;
	

	int gatePoints;

	bool quit;
	int returnVal;
	

	GateInfo testGateInfo;
	//TerrainPolygon *gatePoly0;
	//TerrainPolygon *gatePoly1;
	//TerrainPoint *gatePoint0;
	//TerrainPoint *gatePoint1;
	sf::Vector2i gatePoint0;
	sf::Vector2i gatePoint1;
	GateInfoPtr modifyGate;
	
	bool PointSelectDecor(V2d &pos);
	bool PointSelectActor(  V2d &pos);
	bool PointSelectTerrain(V2d &pos);
	bool PointSelectPoint( V2d &pos );
	bool PointSelectPoly(V2d &pos);

	void UpdateGrass();
	void ModifyGrass();

	bool BoxSelectPoints(sf::IntRect &rect,
		double radius);
	bool BoxSelectActors(sf::IntRect &rect);
	bool BoxSelectDecor(sf::IntRect &rect);
	bool BoxSelectPolys(sf::IntRect &rect);
	void TryBoxSelect();
	void TryPaste();
	bool CheckValidPaste();
	void Paste();
	void ExecuteTerrainCompletion();
	Action* ExecuteTerrainAdd(
		std::list<PolyPtr> &intersectingPolys);
	void PasteTerrain( Brush *b );
	Action* ExecuteTerrainSubtract( std::list<PolyPtr> &intersectingPolys);
	Action *ChooseAddOrSub(std::list<PolyPtr> &intersectingPolys);
	
	bool HoldingShift();
	bool HoldingControl();

	std::list<boost::shared_ptr<EditorDecorInfo>> decorImagesBehindTerrain;
	std::list<boost::shared_ptr<EditorDecorInfo>> decorImagesBetween;
	std::list<boost::shared_ptr<EditorDecorInfo>> decorImagesFrontTerrain;
	Panel *decorPanel;
	Panel *editDecorPanel;
	void InitDecorPanel();
	std::string currDecorName;
	Tileset *ts_currDecor;
	sf::Sprite tempDecorSprite;
	int currDecorLayer;
	int currDecorTile;

	bool decorTracking;
	void LoadDecorImages();
	std::map<std::string, Tileset*> decorTSMap;
	int *decorTileIndexes;
	std::map<std::string, std::list<int>> decorTileIndexMap;


	bool drawingCreateRect;
	sf::Vector2i createRectStartPoint;
	sf::Vector2i createRectCurrPoint;
	AirTriggerParams *rectCreatingTrigger;

	int setLevelCurrent;

	
	
};



#endif