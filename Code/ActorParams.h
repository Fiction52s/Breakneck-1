#ifndef __ACTORPARAMS_H__
#define __ACTORPARAMS_H__

#include <fstream>
#include "EditSession.h"


struct ActorParams : ISelectable
{
	enum PosType
	{
		GROUND_ONLY,
		AIR_ONLY,
		GROUND_AND_AIR
	};

	virtual ActorParams *Copy() = 0;
	ActorParams(PosType posType);
	virtual void WriteParamFile(std::ofstream &of) = 0;
	void WriteFile(std::ofstream &of);
	void AnchorToGround(TerrainPolygon *poly,
		int eIndex, double quantity);
	void AnchorToGround(GroundInfo &gi);
	void UnAnchor(boost::shared_ptr<ActorParams> &me);
	void UpdateGroundedSprite();
	virtual void SetPath(std::list<sf::Vector2i> &globalPath);
	void DrawBoundary(sf::RenderTarget *target);
	void DrawMonitor(sf::RenderTarget *target);

	virtual void SetParams();
	virtual void SetPanelInfo();


	virtual void SetBoundingQuad();
	virtual void UpdateExtraVisuals()
	{}

	//ISelectable( ISelectableType type );
	virtual bool ContainsPoint(sf::Vector2f test);
	virtual bool Intersects(sf::IntRect rect);
	virtual bool IsPlacementOkay();
	virtual void Move(SelectPtr me, sf::Vector2i delta);
	virtual void BrushDraw(sf::RenderTarget *target,
		bool valid);
	virtual void Draw(sf::RenderTarget *target);
	virtual void DrawPreview(sf::RenderTarget *target);
	virtual void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
	virtual void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);

	virtual void DrawQuad(sf::RenderTarget *target);

	virtual void SetSelected(bool select);

	virtual bool CanApply() = 0;
	bool CanAdd();

	//sf::Sprite icon;
	sf::Sprite image;
	ActorGroup *group;
	ActorType *type;
	sf::Vector2i position;
	PosType posType;

	//if groundInfo is not null
	//then you can handle ground, even 
	//if you arent on it


	GroundInfo *groundInfo;
	bool hasMonitor;
	sf::VertexArray boundingQuad;
};

struct PlayerParams : public ActorParams
{
	PlayerParams(
		sf::Vector2i pos);

	bool CanApply();
	void WriteParamFile(std::ofstream &of);
	void Deactivate(EditSession *edit,
		boost::shared_ptr<ISelectable>  select);
	ActorParams *Copy();
	void Activate(EditSession *edit,
		boost::shared_ptr<ISelectable> select);
};

//extra

struct GroundTriggerParams : public ActorParams
{
	GroundTriggerParams(TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		bool facingRight,
		const std::string &typeStr);
	GroundTriggerParams(TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GroundTriggerParams(EditSession *edit);
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	std::string typeStr;
	bool facingRight;
};

struct FlowerPodParams : public ActorParams
{
	FlowerPodParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		const std::string &typeStr);
	FlowerPodParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	FlowerPodParams();
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	std::string typeStr;
	bool facingRight;
};

struct AirTriggerParams : public ActorParams
{
	AirTriggerParams(sf::Vector2i &pos, const std::string &typeStr,
		int w, int h);
	AirTriggerParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	void SetRect(int width, int height, sf::Vector2i &center);
	ActorParams *Copy();
	void Draw(sf::RenderTarget *target);
	std::string trigType;
	int rectWidth;
	int rectHeight;

	sf::RectangleShape triggerRect;
	sf::Text nameText;
};

struct NexusParams : public ActorParams
{
	NexusParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int nexusIndex);
	NexusParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	NexusParams(EditSession *edit);
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	int nexusIndex;
};

struct ShipPickupParams : public ActorParams
{
	ShipPickupParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		bool facingRight);
	ShipPickupParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	ShipPickupParams();
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool facingRight;
};

struct GoalParams : public ActorParams
{
	GoalParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	GoalParams();
	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	//void Draw( sf::RenderTarget *target );
};

struct PoiParams : public ActorParams
{
	enum Barrier
	{
		NONE,
		X,
		Y
	};

	Barrier barrier;
	PoiParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity, Barrier bType,
		const std::string &name);
	PoiParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	PoiParams(
		sf::Vector2i &pos,
		Barrier bType,
		const std::string &name,
		bool hasCameraProperties, float camZoom);
	PoiParams(
		sf::Vector2i &pos);

	void Draw(sf::RenderTarget *target);
	//PoiParams( EditSession *edit );

	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();

	std::string name;
	static sf::Font *font;
	sf::Text nameText;

	bool hasCamProperties;
	float camZoom;

	sf::RectangleShape camRect;
	//void Draw( sf::RenderTarget *target );
};

struct KeyParams : public ActorParams
{
	KeyParams(
		sf::Vector2i &pos, int numKeys,
		int zoneType);
	KeyParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	int numKeys;
	int zoneType;
};

struct ShardParams : public ActorParams
{
	ShardParams(
		sf::Vector2i &pos);
	ShardParams(
		sf::Vector2i &pos, int world,
		int li);
	void WriteParamFile(std::ofstream &of);

	void SetShard(int world, int realX, int realY);
	void SetShard(int world, int li);
	void SetParams();
	void SetPanelInfo();
	void SetShardFromStr();

	int world;
	int sX;
	int sY;
	int localIndex;
	int GetTotalIndex();

	bool CanApply();
	ActorParams *Copy();

	std::string shardStr;

};

struct RaceFightTargetParams : public ActorParams
{
	RaceFightTargetParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
};

struct BlockerParams : public ActorParams
{
	enum BlockerType
	{
		NORMAL,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		BLACK
	};

	BlockerParams(
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		int bType, bool invinc, int spacing);
	BlockerParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<int> &angleList);
	std::list<int> GetAngleList();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalChain();

	void SetParams();
	void SetPanelInfo();

	//std::list<int> angleList;
	bool armored;
	//sf::VertexArray *lines; //local pos

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	int spacing;

	BlockerType bType;
	//will have multiple types
};


//w1
struct ComboerParams : public ActorParams
{
	ComboerParams(
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		float speed,
		bool loop);
	ComboerParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool loop;
	int speed;
	int swoopSpeed;
};

struct BoosterParams : public ActorParams
{
	BoosterParams(
		sf::Vector2i &pos, int strength);
	BoosterParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int strength;
};

struct SpringParams : public ActorParams
{
	SpringParams(
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath,
		int moveFrames);
	SpringParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();
	std::list<sf::Vector2i> GetGlobalPath();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	int moveFrames;
};

struct PatrollerParams : public ActorParams
{
	PatrollerParams(
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		float speed,
		bool loop);
	PatrollerParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();

	void SetParams();
	void SetPanelInfo();

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool loop;
	int speed;
	int swoopSpeed;
};

struct CrawlerParams : public ActorParams
{
	CrawlerParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		bool clockwise, float speed);
	CrawlerParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	CrawlerParams(EditSession *edit);

	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	ActorParams *Copy();
	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	int speed;
	int dist;
};

struct BasicTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	BasicTurretParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		double bulletSpeed,
		int framesWait);
	BasicTurretParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	ActorParams *Copy();
	void SetParams();
	void SetPanelInfo();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	int framesWait;
};

struct FootTrapParams : public ActorParams
{
	FootTrapParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	FootTrapParams(EditSession *edit);

	bool CanApply();
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	void SetParams();
	void SetPanelInfo();
	//void Draw( sf::RenderTarget *target );
};

struct AirdasherParams : public ActorParams
{
	AirdasherParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
};

struct BossCrawlerParams : public ActorParams
{
	BossCrawlerParams(

		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
};


//w2
struct BatParams : public ActorParams
{
	BatParams(
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		int framesBetween,
		//int nodeDistance,
		int bulletSpeed,
		bool loop);
	BatParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	int bulletSpeed;
	//int nodeDistance;
	int framesBetweenNodes;
	bool loop;
	//int speed;
};

struct GravityFallerParams : public ActorParams
{
	GravityFallerParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int variation);
	GravityFallerParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	void WriteParamFile(std::ofstream &of);
	void Draw(sf::RenderTarget *target);
	ActorParams *Copy();
	bool CanApply();
	void SetParams();
	void SetPanelInfo();

	int variation;
};

struct StagBeetleParams : public ActorParams
{
	StagBeetleParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		bool clockwise, float speed);
	//StagBeetleParams( 
	//	sf::Vector2i &pos, bool facingRight,
	//	float speed );
	StagBeetleParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	//void Draw( sf::RenderTarget *target );
	bool clockwise;
	float speed;
};

struct PoisonFrogParams : public ActorParams
{
	PoisonFrogParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpWaitFrames);
	PoisonFrogParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity);
	PoisonFrogParams(EditSession *edit);
	void WriteParamFile(std::ofstream &of);
	void UpdatePath();
	void Draw(sf::RenderTarget *target);
	void UpdateExtraVisuals();
	//void SetParams();
	//void SetPanelInfo();
	ActorParams *Copy();
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	int gravFactor;
	sf::Vector2i jumpStrength;
	//int jumpStrengthX;
	//int jumpStrengthY;
	int jumpWaitFrames;
	sf::VertexArray pathQuads;
};

struct CurveTurretParams : public ActorParams
{
	//std::string SetAsBasicTurret( ActorType *t, ); 
	CurveTurretParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex,
		double edgeQuantity,
		double bulletSpeed,
		int framesWait,
		sf::Vector2i gravFactor,
		bool relativeGrav);
	ActorParams *Copy();
	CurveTurretParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);
	void SetParams();
	void SetPanelInfo();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void UpdateBulletCurve();
	void Draw(sf::RenderTarget *target);
	void UpdateExtraVisuals();
	//void Draw( sf::RenderTarget *target );
	float bulletSpeed;
	bool relativeGrav;
	int framesWait;
	sf::Vector2i gravFactor;
	sf::VertexArray bulletPathQuads;

	int curveFactor;
};

struct BossBirdParams : public ActorParams
{
	BossBirdParams(

		sf::Vector2i &pos);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void Draw(sf::RenderTarget *target);
	sf::VertexArray debugLines;
	void CreateFormation();
	int width;
	int height;
};


//w3
struct PulserParams : public ActorParams
{
	PulserParams(
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &globalPath,
		int framesBetween,
		bool loop);
	PulserParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalPath();
	void Draw(sf::RenderTarget *target);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	int framesBetweenNodes;
	bool loop;
	//int speed;
};

struct OwlParams : public ActorParams
{
	OwlParams(
		sf::Vector2i &pos,
		int moveSpeed,
		int bulletSpeed,
		int rhythmFrames);
	OwlParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int moveSpeed;
	int bulletSpeed;
	int rhythmFrames;
};

struct BadgerParams : public ActorParams
{
	BadgerParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		int speed, int jumpStrength);

	BadgerParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int speed;
	int jumpStrength;
};

struct CactusParams : public ActorParams
{
	CactusParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		int bulletSpeed, int rhythm,
		int amplitude);

	CactusParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int bulletSpeed;
	int rhythm;
	int amplitude;
};

struct BossCoyoteParams : public ActorParams
{
	BossCoyoteParams(
		sf::Vector2i &pos);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void CreateFormation();
	void Draw(sf::RenderTarget *target);
	int radius;
	sf::VertexArray debugLines;
};


//w4
struct TurtleParams : public ActorParams
{
	TurtleParams(
		sf::Vector2i &pos);
	/*int moveSpeed,
	int bulletSpeed,
	int rhythmFrames );
	TurtleParams(
	sf::Vector2i &pos );*/
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
};

struct CoralParams : public ActorParams
{
	CoralParams(
		sf::Vector2i &pos,
		int moveFrames);
	CoralParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int moveFrames;
};

struct CheetahParams : public ActorParams
{
	/*CactusParams(
	TerrainPolygon *edgePolygon,
	int edgeIndex, double edgeQuantity,
	int bulletSpeed, int rhythm,
	int amplitude );*/

	CheetahParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();
};

struct SpiderParams : public ActorParams
{
	SpiderParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity,
		int speed);

	SpiderParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();

	int speed;
};

struct RailParams : public ActorParams
{
	RailParams(
		sf::Vector2i pos,
		std::list<sf::Vector2i> &globalPath,
		bool energized);
	RailParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);
	void SetPath(
		std::list<int> &angleList);
	std::list<int> GetAngleList();
	void Draw(sf::RenderTarget *target);

	bool CanApply();
	ActorParams *Copy();
	void SetPath(
		std::list<sf::Vector2i> &globalPath);
	std::list<sf::Vector2i> GetGlobalChain();

	void SetParams();
	void SetPanelInfo();

	//std::list<int> angleList;
	bool armored;
	//sf::VertexArray *lines; //local pos

	std::list<sf::Vector2i> localPath;
	sf::VertexArray *lines; //local pos

	bool energized;
};

struct BossTigerParams : public ActorParams
{
	BossTigerParams(

		sf::Vector2i &pos);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void CreateFormation();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void Draw(sf::RenderTarget *target);
	sf::VertexArray debugLines;
	int radius1;
	int radius2;
};


//w5
struct SharkParams : public ActorParams
{
	SharkParams(
		sf::Vector2i &pos,
		int circleFrames);
	SharkParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int circleFrames;
};

struct SwarmParams : public ActorParams
{
	SwarmParams(
		sf::Vector2i &pos,
		int liveFrames);
	SwarmParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int liveFrames;
};

struct GhostParams : public ActorParams
{
	GhostParams(
		sf::Vector2i &pos,
		int speed);
	GhostParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int speed;
};

struct OvergrowthParams : public ActorParams
{
	OvergrowthParams(
		TerrainPolygon *edgePolygon,
		int edgeIndex, double edgeQuantity);

	/*OvergrowthParams(
	TerrainPolygon *edgePolygon,
	int edgeIndex, double edgeQuantity );*/


	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void SetParams();
	void SetPanelInfo();
	ActorParams *Copy();
};

struct BossGatorParams : public ActorParams
{
	BossGatorParams(

		sf::Vector2i &pos);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
	void Draw(sf::RenderTarget *target);
	sf::CircleShape circles[5];
	int orbRadius;
	int radius;
};


//w6
struct SpecterParams : public ActorParams
{
	SpecterParams(
		sf::Vector2i &pos);
	/*int moveSpeed,
	int bulletSpeed,
	int rhythmFrames );
	TurtleParams(
	sf::Vector2i &pos );*/
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
};

struct CopycatParams : public ActorParams
{
	CopycatParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
};

struct NarwhalParams : public ActorParams
{
	NarwhalParams(
		sf::Vector2i &pos,
		sf::Vector2i &dest,
		int moveFrames
	);
	NarwhalParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();
	void SetPath(
		std::list<sf::Vector2i> &globalPath);

	int moveFrames;
	sf::Vector2i dest;
	void Draw(sf::RenderTarget *target);
};

struct GorillaParams : public ActorParams
{
	GorillaParams(
		sf::Vector2i &pos, int wallWidth,
		int followFrames);
	GorillaParams(
		sf::Vector2i &pos);
	void WriteParamFile(std::ofstream &of);

	void SetParams();
	void SetPanelInfo();

	bool CanApply();
	ActorParams *Copy();

	int wallWidth;
	int followFrames;
};

struct BossSkeletonParams : public ActorParams
{
	BossSkeletonParams(

		sf::Vector2i &pos);
	//CrawlerParams( EditSession *edit );
	ActorParams *Copy();
	void WriteParamFile(std::ofstream &of);
	bool CanApply();
};


#endif