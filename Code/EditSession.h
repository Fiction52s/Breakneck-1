#ifndef __EDIT_SESSION__
#define __EDIT_SESSION__

#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <list>
#include "VectorMath.h"
#include "GUI.h"
#include "Tileset.h"


struct ActorParams;

struct GrassSeg
{
	GrassSeg( int edgeI, int grassIndex, int rep )
		:edgeIndex( edgeI ), index( grassIndex ), 
		reps (rep)
	{}
	int edgeIndex;
	int index;
	int reps;
};

struct TerrainPolygon;
struct GateInfo;

struct GatePoint
{
	//TerrainPolygon *poly;
	int vertexIndex;
	GateInfo *info;
};

struct TerrainPoint
{
	TerrainPoint( sf::Vector2i &pos, bool selected );
	~TerrainPoint()
	{
		//delete gate;
	}
	sf::Vector2i pos;
	bool selected;
	std::list<int> grass;
	GateInfo *gate;
	bool firstPoint; 
	TerrainPoint *next;
	TerrainPoint *prev;
	int GetEdgeIndex();
	//int special;
};

typedef std::pair<sf::Vector2i,sf::Vector2i> PointPair;

struct EditSession;

struct TerrainBrush
{
	TerrainBrush( TerrainPolygon *poly );
	TerrainBrush( TerrainBrush &brush );
	~TerrainBrush();
	void AddPoint( TerrainPoint* tp);
	void UpdateLines();
	void Move( sf::Vector2i delta );
	void Draw( sf::RenderTarget *target );
	int numPoints;
	sf::VertexArray lines;
	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	int left;
	int right;
	int top;
	int bot;
	
};

struct GateInfo
{
	GateInfo();
	TerrainPoint *point0;
	TerrainPoint *point1;
	TerrainPolygon *poly0;
	int vertexIndex0;
	TerrainPolygon *poly1;
	int vertexIndex1;
	sf::VertexArray thickLine;
	EditSession *edit;
	void UpdateLine();
	void WriteFile( std::ofstream &of );
	void Draw( sf::RenderTarget *target );
};


typedef std::map<TerrainPoint*,std::list<ActorParams*>> EnemyMap;

struct TerrainPolygon
{
	TerrainPolygon( sf::Texture *grassTex );
	~TerrainPolygon();
	
	TerrainPoint *pointStart;
	TerrainPoint *pointEnd;
	int numPoints;
	void AddPoint( TerrainPoint* tp);
	void RemovePoint( TerrainPoint *tp );
	void DestroyEnemies();
	void ClearPoints();
	std::string material;
	void RemoveSelectedPoints();
	bool IsRemovePointsOkayTerrain(EditSession *edit);
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	void Reset();
	void SoftReset();
	void Draw( bool showPath, double zoomMultiple, sf::RenderTarget * rt, bool showPoints, TerrainPoint *dontShow );
	void FixWinding();
	bool IsClockwise();
	void AlignExtremes( double primLimit );
	void UpdateGrass();
	void ShowGrass( bool show );
	void Extend( TerrainPoint* startPoint, TerrainPoint*endPoint, TerrainPolygon *inProgress );
	void SwitchGrass( sf::Vector2<double> mousePos );
	bool ContainsPoint( sf::Vector2f p );
	void SetSelected( bool select );
	bool IsTouching( TerrainPolygon *p );
	bool IsMovePointsOkay( EditSession *edit,
		sf::Vector2i delta );
	bool IsMovePointsOkay( EditSession *edit,
		sf::Vector2i pointGrabDelta,
		sf::Vector2i *deltas );
	bool IsMovePolygonOkay( EditSession *edit, 
		sf::Vector2i delta );
	void MoveSelectedPoints(sf::Vector2i move);
	bool movingPointMode;
	
	sf::Rect<int> TempAABB();

	void Move( sf::Vector2i move );

	sf::Vertex *lines;
	sf::VertexArray *va;
	sf::VertexArray *grassVA;
	int numGrassTotal;
	sf::Texture *grassTex;
	int vaSize;
	bool selected;
	int left;
	int right;
	int top;
	int bottom;
	std::list<sf::Vector2i> path;
	
	EnemyMap enemies;
	int writeIndex;
	bool isGrassShowing;
	bool finalized;

};

struct StaticLight
{
	StaticLight( sf::Color c, sf::Vector2i &pos, int radius, int brightness );
	void Draw( sf::RenderTarget *target );
	sf::Rect<double> GetAABB();
	int radius;
	int brightness;
	sf::Color color;
	sf::Vector2i position;
	void WriteFile( std::ofstream &of );
};



struct ActorType
{
	ActorType( const std::string & name, Panel *panel );
	void Init();
	std::string name;
	sf::Texture iconTexture;
	sf::Texture imageTexture;
	int width;
	int height;
	bool canBeGrounded;
	bool canBeAerial;
	Panel *panel;
};

struct ActorGroup;
struct ActorParams
{
	ActorParams();
	virtual void WriteParamFile( std::ofstream &of ) = 0;
	void WriteFile( std::ofstream &of );



	void AnchorToGround( TerrainPolygon *poly, 
		int eIndex, double quantity );
	void UpdateGroundedSprite();
	virtual void SetBoundingQuad();
	//sf::Sprite icon;
	sf::Sprite image;
	ActorGroup *group;
	ActorType *type;
	sf::Vector2i position;

	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it
	struct GroundInfo
	{
		TerrainPoint *edgeStart;
		//TerrainPoint *edgeEnd;
		double groundQuantity;
		TerrainPolygon *ground;
		int GetEdgeIndex();
		//int edgeIndex;
	};
	
	GroundInfo *groundInfo;

	sf::VertexArray boundingQuad;
	
	virtual void Draw( sf::RenderTarget *target );
	virtual void DrawQuad( sf::RenderTarget *target );

};

struct PatrollerParams : public ActorParams
{
	PatrollerParams( EditSession *edit,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath, 
		float speed,
		bool loop );
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );
	std::list<sf::Vector2i> localPath;
	bool loop;
	float speed;
};

struct KeyParams : public ActorParams
{
	KeyParams( EditSession *edit,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath, 
		float speed,
		bool loop,
		int stayFrames,
		bool teleport );
	void WriteParamFile( std::ofstream &of );
	void SetPath( 
		std::list<sf::Vector2i> &globalPath );
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw( sf::RenderTarget *target );
	std::list<sf::Vector2i> localPath;
	bool loop;
	float speed;
	int stayFrames;
	bool teleport;
};

struct CrawlerParams : public ActorParams
{ 
	CrawlerParams( EditSession *edit, 
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity, 
		bool clockwise, float speed );
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	float speed;
};

struct BasicTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	BasicTurretParams( EditSession *edit,  
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity, 
		double bulletSpeed, 
		int framesWait );
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	int framesWait;
};

struct FootTrapParams : public ActorParams
{
	FootTrapParams( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity );
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
};

struct GoalParams : public ActorParams
{
	GoalParams ( EditSession *edit,
		TerrainPolygon *edgePolygon,
		int edgeIndex, 
		double edgeQuantity );
	void WriteParamFile( std::ofstream &of );
	//void Draw( sf::RenderTarget *target );
};

//no params for goal and foottrap atm
struct ActorGroup
{
	ActorGroup( const std::string &name );
	std::string name;
	std::list<ActorParams*> actors;
	void Draw( sf::RenderTarget *target );
	void WriteFile( std::ofstream &of );
};

struct EditSession : GUIHandler
{
	EditSession( sf::RenderWindow *w,
		sf::RenderTexture *preScreenTex );
	~EditSession();
		
	int Run(std::string fileName, 
		sf::Vector2f cameraPos, 
		sf::Vector2f cameraSize );
	void Draw();
	bool OpenFile( std::string fileName );
	void WriteFile(std::string fileName);
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );

	bool IsPointValid( sf::Vector2i oldPoint, sf::Vector2i point, TerrainPolygon *poly );
	void ExtendAdd();
	bool IsPolygonExternallyValid( TerrainPolygon &poly,
		 TerrainPolygon *ignore );
	bool IsPolygonInternallyValid( TerrainPolygon &poly );
	bool IsPolygonValid( TerrainPolygon &poly,
		TerrainPolygon *ignore );
	sf::Vector2<double> GraphPos( sf::Vector2<double> realPos );
	void SetEnemyEditPanel();
	bool QuadPolygonIntersect( TerrainPolygon *poly, 
		sf::Vector2i a, sf::Vector2i b, 
		sf::Vector2i c, sf::Vector2i d );
	bool CanCreateGate( GateInfo &testGate );
	std::list<GateInfo*> gates;
	GateInfo *selectedGate;


	const static double PRIMARY_LIMIT;
	sf::RenderTexture *preScreenTex;
	int validityRadius;
	bool showGrass;
	sf::Texture grassTex;
	bool pointGrab;
	sf::Vector2i pointGrabPos;
	sf::Vector2i pointGrabDelta;
	bool polyGrab;
	//sf::Vector2i polyGrabPos;
	//sf::Vector2f polyGrabPos;
	sf::Vector2i polyGrabPos;
	sf::Vector2i polyGrabDelta;
	sf::Vector2f polyMove;

	bool makingRect;
	sf::Vector2i rectStart;



	void ExtendPolygon();
	bool showPoints;
	TerrainPolygon *extendingPolygon;
	TerrainPoint *extendingPoint;

	sf::View v;


	bool showTerrainPath;
	
	sf::RenderWindow *w;
	sf::Vector2i playerPosition;
	//sf::Vector2i goalPosition;
	std::string currentFile;
	double zoomMultiple;
	sf::Vector2f testPoint;
	std::map<std::string, ActorGroup*> groups;
	std::map<std::string, ActorType*> types;
	ActorParams *selectedActor;
	ActorParams *editActor;

	StaticLight *selectedLight;
	bool selectedLightGrabbed;
	sf::Vector2i lightGrabPos;

	bool selectedPlayer;
	bool grabPlayer;
	sf::Vector2i grabPos;
	bool selectedActorGrabbed;

	//CREATE_TERRAIN mode
	void Add( TerrainPolygon *brush, TerrainPolygon *poly);	
	bool PointValid( sf::Vector2i prev, sf::Vector2i point );
	static LineIntersection SegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );
	static LineIntersection LimitSegmentIntersect( sf::Vector2i a, 
		sf::Vector2i b, sf::Vector2i c, 
		sf::Vector2i d );

	double minimumEdgeLength;
	double minAngle;
	
	std::list<TerrainPolygon*> polygons;
	TerrainPolygon *polygonInProgress;
	std::list<sf::VertexArray*> progressDrawList;
	
	//sf::Text polygonTimeoutText;
	//int polygonTimeoutTextTimer;
	//int polygonTimeoutTextLength;

	//static void TestButton();
	std::list<StaticLight*> lights;
	sf::Vector2i lightPos;
	bool lightActive;

	int enemyEdgeIndex;
	TerrainPolygon *enemyEdgePolygon;
	double enemyEdgeQuantity;

	bool radiusOption;
	bool lightPosDown;
	double lightRadius;
	int lightBrightness;
	std::list<TerrainPolygon*> selectedPolygons;

	sf::Sprite enemySprite;
	sf::RectangleShape enemyQuad;
	ActorType *trackingEnemy;//bool trackingEnemy;
	Panel *showPanel;	
	bool trackingEnemyDown;

	Panel * CreatePopupPanel( const std::string &p );
	Panel *messagePopup;
	Panel *errorPopup;

	int IsRemovePointsOkay();

	Panel *CreateOptionsPanel( const std::string &name );
	void WriteGrass( TerrainPolygon * p, std::ofstream &of );
	int CountSelectedPoints();

	std::list<sf::Vector2i> patrolPath;
	double minimumPathEdgeLength;

	sf::IntRect fullRect;
	
	bool ConfirmationPop( const std::string &question );
	void MessagePop( const std::string &message );
	void ErrorPop( const std::string &error );

	//bool closePopup; //for messsage/error only
	
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
	//void GoPopup();

	std::list<TerrainBrush*> copyBrushes;
	std::list<TerrainBrush*> pasteBrushes;
	void ClearCopyBrushes();
	void ClearPasteBrushes();
	void CopyToPasteBrushes();
	sf::Vector2i pastePos;

	enum Emode
	{
		CREATE_TERRAIN,
		EDIT,
		SELECT_MODE,
		CREATE_PATROL_PATH,
		//PLACE_PLAYER,
		//PLACE_GOAL,
		SELECT_POLYGONS,
		PAUSED,
		CREATE_ENEMY,
		DRAW_PATROL_PATH,
		CREATE_TERRAIN_PATH,
		CREATE_LIGHTS
	};

	Emode mode;
	
};



#endif