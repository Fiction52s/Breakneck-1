#ifndef __EDITORRAIL_H__
#define __EDITORRAIL_H__

#include <list>
#include <SFML\Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "ISelectable.h"
#include "VectorMath.h"
#include "EditorTerrain.h"

struct Panel;
struct TerrainPoint;
struct ActorParams;
struct TerrainPolygon;
struct TransformTools;
struct CircleGroup;
struct BlockerChain;

struct EnemyChain;

struct TerrainRail : ISelectable, QuadTreeEntrant
{
	enum RenderMode
	{
		RENDERMODE_NORMAL,
	};
	RenderMode renderMode;

	

	enum RailType
	{
		FLOORANDCEILING,
		FLOOR,
		CEILING,
		BOUNCE,
		SCORPIONONLY,
		GRIND,
		PHASE,
		INVERSEPHASE,
		ACCELERATE,
		FADE,
		LOCKED,
		ANTITIMESLOW,
		WIREONLY,
		WIREBLOCKING,
		HIT,
		BLOCKER,
		FLY,
		/*NORMAL,
		LOCKED,
		TIMESLOW,
		WIREONLY,
		WIREBLOCKING,
		FLOORANDCEILING,
		FLOOR,
		CEILING,
		BOUNCE,
		SCORPIONONLY,
		BLOCKER,
		FLY,*/
	};


	enum State : int
	{
		IDLE,
		FADINGOUT,
		INACTIVE,
		TEMPORARILYINACTIVE,
		FADINGIN,
	};

	int frame;
	State state;

	Session *sess;
	int rType;
	Tileset *ts_rail;
	RailPtr queryNext;
	sf::Rect<double> aabb;
	ActorParams *enemyParams;
	EnemyChain *enemyChain;
	RailPtr mostRecentCopy;

	ActorParams *blockerParams;
	BlockerChain *blockerChain;
	std::vector<Edge> edges;

	std::vector<TerrainPoint> pointVector;
	double railRadius;
	CircleGroup *coloredNodeCircles;
	sf::Vertex *coloredQuads;
	sf::Vertex *texturedQuads;
	int numTexturedQuads;
	sf::Vertex *lines;
	int numLineVerts;
	int numColoredQuads;
	float quadHalfWidth;

	sf::Vertex *quads;

	int left;
	int right;
	int top;
	int bottom;

	std::map<TerrainPoint*, std::list<ActorPtr>> enemies;
	std::vector<std::pair<ActorPtr,
		PositionInfo>> enemyPosBackups;

	int writeIndex;
	bool finalized;

	const static int MAX_RAIL_LEVEL = 12;

	bool IsEdgeActive(Edge *e);
	bool IsTerrainType();
	void SetRailType(int r);
	
	TerrainRail();
	TerrainRail(TerrainRail &r);
	~TerrainRail();
	int GetRailType();
	void ResetState();
	void UpdateState();
	void FadeOut();
	bool IsActive();
	void TryCreateEnemyChain();
	sf::Color GetRailColor();
	void SetRailToActorType(ActorParams *ap);
	bool PointsTooCloseToEachOther(double radius);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	void Draw(sf::RenderTarget *target);
	void AddEnemyChainToWorldTrees();
	bool RequiresPowerToGrind();
	void UpdateEnemyChain();
	void SetChainPath();
	void AddEnemiesToBrush(Brush *b);
	int GetNumSelectedPoints();
	void CreateNewRailsWithSelectedPointsRemoved(
		std::list<RailPtr> &rails);
	void FillSelectedIndexBuffer(
		std::vector<int> &selectedIndexes);
	void SetupEdges();
	void AddEdgesToQuadTree(QuadTree *tree);
	void BackupEnemyPositions();
	void Init();
	bool ContainsPoint(sf::Vector2f test);
	bool ContainsPoint(sf::Vector2f test,double rad);
	bool Intersects(sf::IntRect rect);
	void SetPointPos(int index, sf::Vector2i &p);
	void MovePoint(int index, sf::Vector2i &delta);
	ActorPtr GetFurthestEnemy(int index, double &maxQuant);
	ActorPtr GetClosestEnemy(int index, double &minQuant);
	void Move(sf::Vector2i delta);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	bool IsInternallyValid();
	void Draw(double zoomMultiple, bool showPoints, 
		sf::RenderTarget *target);
	void Deactivate();
	void Activate();
	bool CanApply();
	bool CanAdd();
	void SetSelected(bool select);
	void UpdateLineColor(sf::Vertex *li, 
		int i, int index);
	void UpdateLines();
	void UpdateColoredQuads();
	void SwitchDirection();
	void SetParams(Panel *p);
	void UpdatePanel(Panel *p);
	void CopyPointsFromPoly(PolyPtr tp);
	void CopyPointsFromRail(TerrainRail *rail);
	TerrainRail *Copy();
	TerrainPoint * AddPoint(sf::Vector2i &p, bool sel);	
	void RemoveLastPoint();
	void ClearPoints();
	TerrainPoint *GetPoint(int i);
	TerrainPoint *GetEndPoint();
	Edge *GetEdge(int index);
	void Reserve(int numP);
	void RemoveSelectedPoints();
	int IsRemovePointsOkayEnemies(EditSession *edit);
	void Finalize();
	void Reset();
	void SoftReset();
	bool AlignExtremes(std::vector<PointMoveInfo> &lockPoints);
	bool AlignExtremes();
	void UpdateBounds();
	void WriteFile(std::ofstream &of);
	void Load(std::ifstream &is);
	void BrushSave(std::ofstream &of);
	TerrainPoint *GetClosePoint(double radius, V2d &pos);
	int GetNumPoints();
	void CancelTransformation();
	RailPtr CompleteTransformation(TransformTools *tr);
	void UpdateTransformation(TransformTools *tr);
	void StoreEnemyPositions(std::vector<std::pair<ActorPtr, 
		PositionInfo>>&b);

};

#endif