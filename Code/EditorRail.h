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

struct TerrainRail : ISelectable
{
	enum RenderMode
	{
		RENDERMODE_NORMAL,
	};
	RenderMode renderMode;
	/*enum RenderMode
	{
		RENDERMODE_NORMAL,
		RENDERMODE_MOVING_POINTS,
		RENDERMODE_TRANSFORM,
		RENDERMODE_FLIES,
	};*/

	TerrainRail();
	TerrainRail(TerrainRail &r);
	~TerrainRail();

	void SetupEdges();
	std::vector<Edge> edges;
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

	//bool IsPlacementOkay();

	void Move(sf::Vector2i delta);
	void BrushDraw(sf::RenderTarget *target,
		bool valid);
	bool IsInternallyValid();

	void Draw(double zoomMultiple, bool showPoints, 
		sf::RenderTarget *target);

	//void Draw(sf::RenderTarget *target);
	void Deactivate();
	void Activate();
	bool CanApply();
	bool CanAdd();


	void SetSelected(bool select);


	void UpdateLineColor(sf::Vertex *li, 
		int i, int index);
	void UpdateLines();
	void SwitchDirection();
	void SetParams(Panel *p);
	void UpdatePanel(Panel *p);
	void CopyPointsFromPoly(PolyPtr tp);
	void CopyPointsFromRail(TerrainRail *rail);
	TerrainRail *Copy();

	//bool IsPoint(sf::Vector2i &p);

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

	TerrainPoint *GetClosePoint(double radius, V2d &pos);

	std::vector<TerrainPoint> pointVector;

	int GetNumPoints();

	double railRadius;

	sf::Vertex *lines;
	int numLineVerts;

	sf::Vertex *quads;

	int left;
	int right;
	int top;
	int bottom;

	void CancelTransformation();
	RailPtr CompleteTransformation(TransformTools *tr);
	void UpdateTransformation(TransformTools *tr);

	void StoreEnemyPositions(std::vector<std::pair<ActorPtr, 
		PositionInfo>>&b);

	std::map<TerrainPoint*, std::list<ActorPtr>> enemies;
	std::vector<std::pair<ActorPtr, 
		PositionInfo>> enemyPosBackups;

	int writeIndex;
	bool finalized;

	bool requirePower;
	bool accelerate;
	int level;

	const static int MAX_RAIL_LEVEL = 12;
};

#endif