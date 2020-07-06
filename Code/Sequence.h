#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__
#include "Tileset.h"
#include "SFML/Graphics.hpp"
#include <list>
#include <vector>
#include "Movement.h"
#include <boost/thread.hpp>
#include <sfeMovie\Movie.hpp>
#include "EffectLayer.h"
#include "MovingGeo.h"
#include "VectorMath.h"

struct Edge;
struct MovementSequence;
struct PoiInfo;
struct MusicInfo;
struct ShapeEmitter;
struct StorySequence;
struct Conversation;
struct Enemy;
struct BasicBossScene;
struct ButtonHolder;
struct ConversationGroup;
struct CameraShot;
struct Barrier;
struct Session;

struct SceneBG;

struct PanInfo
{
	PanInfo(sf::Vector2f &pos,
		sf::Vector2f &diff,
		int start, int len);
	sf::Vector2f GetCurrPos(int f);
	sf::Vector2f totalDelta;
	CubicBezier bez;
	sf::Vector2f origPos;
	int startFrame;
	int frameLength;
};

struct FlashedImage
{
	FlashedImage( Tileset *ts,
		int tileIndex, int appearFrames,
		int holdFrames,
		int disappearFrames,
		sf::Vector2f &pos );
	~FlashedImage();
	void Reset();
	void Flash();
	void Update();
	void Draw(sf::RenderTarget *target);
	int GetNumFrames();
	bool IsDone();
	void StopHolding();
	bool IsHolding();
	bool IsFadingIn();
	int GetFramesUntilDone();

	void UpdateBG();
	void AddPan(sf::Vector2f &pVel,
		int startFrame, int frameLength );
	void AddPanX(float x,
		int startFrame, int frameLength);
	void AddPanY(float y,
		int startFrame, int frameLength);

	void SetSplit(Tileset *ts, Tileset *borderTS,
		int tileIndex, sf::Vector2f &pos);
	void SetBG(SceneBG *p_bg);

	Tileset *ts_image;
	Tileset *ts_split;
	Tileset *ts_splitBorder;
	sf::Shader *splitShader;
	sf::Shader *bgSplitShader;

	Tileset *currBGTileset;
	SceneBG *bg;

	sf::Vertex spr[4];
	sf::Vertex split[4];
	sf::Vertex splitBorder[4];

	std::list<PanInfo*> panList;

	sf::Vector2f origPos;
	sf::Vector2f position;
	sf::Vector2u splitSize;

	PanInfo * currPan;

	//sf::Sprite spr;
	int frame;
	bool flashing;
	int aFrames;
	int dFrames;
	int hFrames;

	bool infiniteHold;
	int infiniteHoldEarlyEnd;
};

struct FlashInfo
{
	FlashInfo(FlashedImage *fi,
		int dStart = 0, bool p_simul = false)
	{
		image = fi;
		delayedStart = dStart;
		startFrame = -1;
		simul = p_simul;
	}
	FlashedImage *image;
	int delayedStart;
	int startFrame;
	bool simul;
};

struct FlashGroup
{
	FlashGroup();
	~FlashGroup();
	void Reset();
	void AddSeqFlash(FlashedImage *fi,
		int earlyEndFrames = 0);
	void AddSimulFlash(FlashedImage *fi,
		int delayedFrames = 0 );
	void SetBG(SceneBG *bg);
	void Init();
	void TryCurrFlashes();
	void Update();
	void UpdateBG();
	std::map<int, std::list<FlashInfo*>> fMap;
	std::map<int, std::list<FlashInfo*>> infEndMap;
	std::list<FlashInfo*> fList;
	void DrawBG(sf::RenderTarget *target);
	//std::list<FlashInfo*>::iterator currFlash;
	bool IsDone();
	SceneBG *bg;
	Tileset *currBGTileset;
	sf::Vertex bgQuad[4];
	int frame;
	int numFrames;
	


	bool done;
};

struct SceneBG
{
	SceneBG(const std::string &p_name,
		std::list<Tileset*> &p_tilesets,
		int p_animFactor);
	std::string name;
	int animFactor;
	std::vector<Tileset*> tilesets;
	//sf::Vertex bgQuad[4];
	//void Draw(sf::RenderTarget *target);
	Tileset * GetCurrTileset(int frame);
};


struct Sequence
{
	int frameCount;
	int frame;
	Sequence *nextSeq;

	std::map <std::string, FlashGroup*> flashGroups;
	FlashGroup *currFlashGroup;
	std::map<std::string, FlashedImage*> flashes;
	std::map<std::string, sfe::Movie*> movies;
	std::map<std::string, Enemy *> enemies;
	std::map<std::string, ConversationGroup*> groups;
	std::map<std::string, CameraShot*> shots;
	std::map<std::string, PoiInfo*> points;
	std::list<FlashedImage*> flashList;
	std::map<std::string, SceneBG*> bgs;
	sfe::Movie *currMovie;
	ConversationGroup *currConvGroup;
	int cIndex;
	int movieFadeFrames;
	sf::Color movieFadeColor;
	int movieStopFrame;
	Session *sess;
	int state;
	int *stateLength;
	int numStates;
	Barrier *barrier;

	static Sequence *CreateScene(const std::string &name);
	Sequence();
	virtual ~Sequence();
	virtual void StartRunning() {}
	virtual bool Update();
	void UpdateFlashes();
	void Init();
	virtual void UpdateState() {}
	virtual void SetupStates() {}
	virtual void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	FlashGroup * AddFlashGroup(const std::string &n);
	void SetFlashGroup(const std::string & n);
	void UpdateFlashGroup();
	void AddGroup(const std::string &fileName,
		const std::string &groupName);
	void AddShot(const std::string &shotName);
	void AddPoint(const std::string &poiName);
	void AddMovie(const std::string &movieName);
	void AddEnemy(const std::string &enName,
		Enemy *e);
	FlashedImage * AddFlashedImage(const std::string &imageName,
		Tileset *ts, int tileIndex,
		int appearFrames,
		int holdFrames,
		int disappearFrames,
		sf::Vector2f &pos);
	void SetNumStates(int count);
	void UpdateMovie();
	void SetConvGroup(const std::string &n);
	Conversation *GetCurrentConv();
	void SetCurrMovie(const std::string &name,
		int movFadeFrames = 0,
		sf::Color movFadeColor = sf::Color::Black);
	void SetCameraShot(const std::string &n);
	void AddSeqFlashToGroup(FlashGroup *,
		const std::string &n, int delayedStart = 0);
	void AddSimulFlashToGroup(FlashGroup *,
		const std::string &n, int delayedStart = 0);
	void EndCurrState();
	void BasicFlashUpdateState(
		const std::string &flashName);
	void RumbleDuringState(int x, int y);
	void Rumble(int x, int y, int duration);
	virtual void ConvUpdate();
	SceneBG * AddBG(const std::string &name, std::list<Tileset*> &anim,
		int animFactor);
	SceneBG *GetBG(const std::string &name);
	bool IsCamMoving();
	bool IsLastFrame();
	void EaseShot(const std::string &shotName,
		int frames, CubicBezier bez = CubicBezier());
	void EasePoint(const std::string &pointName,
		float targetZoom, int frames,
		CubicBezier bez = CubicBezier());
	void Flash(const std::string &flashName);
	virtual void ReturnToGame() {}
	virtual void DrawFlashes(sf::RenderTarget *target);
	virtual void SpecialInit() {}
	virtual void AddShots() {}
	virtual void AddPoints() {}
	virtual void AddFlashes() {}
	virtual void AddEnemies() {}
	virtual void AddGroups() {}
	virtual void AddMovies() {}
	virtual void Reset();
	bool StateIncrement();
};

struct BasicBossScene : Sequence
{
	enum EntranceType
	{
		RUN,
		APPEAR,
	};

	
	EntranceType entranceType;
	int fadeFrames;
	int entranceIndex;
	
	virtual bool Update();
	static BasicBossScene *CreateScene(const std::string &name);
	BasicBossScene(EntranceType et );
	virtual ~BasicBossScene();
	virtual void StartRunning();
	virtual void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	virtual void EntranceUpdate();
	virtual bool IsAutoRunState();
	virtual void ReturnToGame();
	virtual	void Wait();
	virtual void SetEntranceRun();
	virtual void SetEntranceStand();
	virtual void SetEntranceShot();
	void SetPlayerStandPoint(const std::string &n,
		bool fr);
	void SetEntranceIndex(int ind);
	void AddStartPoint();
	void AddStopPoint();
	void AddStandPoint();
	void AddStartAndStopPoints();
	void SetPlayerStandDefaultPoint(bool fr);
	void StartEntranceRun(bool fr,
		double maxSpeed, const std::string &n0,
		const std::string &n1);
	void StartEntranceStand(bool fr,
		const std::string &n);
};

struct ShipEnterScene : Sequence
{
	enum State
	{
		SHIP_FLYING,
		KIN_JUMP,
		Count
	};

	//ship sequence. should be in its own structure
	Tileset *ts_w1ShipClouds0;
	Tileset *ts_w1ShipClouds1;
	Tileset *ts_ship;
	sf::Vertex cloud0[3 * 4];
	sf::Vertex cloud1[3 * 4];
	sf::Vertex cloudBot0[3 * 4];
	sf::Vertex cloudBot1[3 * 4];
	/*sf::VertexArray cloud0;
	sf::VertexArray cloud1;
	sf::VertexArray cloudBot0;
	sf::VertexArray cloudBot1;*/
	sf::Vector2f relShipVel;
	sf::Sprite shipSprite;
	sf::RectangleShape middleClouds;
	bool shipSequence;
	//bool hasShipEntrance;
	V2d shipEntrancePos;
	sf::Vector2f cloudVel;
	//int shipSeqFrame;
	sf::Vector2f shipStartPos;

	ShipEnterScene();
	void Reset();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void AddFlashes();
	//void Draw(sf::RenderTarget *target);
};

struct ShipExitScene : Sequence
{
	enum State
	{
		SHIP_SWOOP,
		Count
	};

	int enterTime;
	int exitTime;
	sf::Vector2<double> abovePlayer;
	sf::Vector2<double> origPlayer;
	sf::Vector2<double> attachPoint;
	MovementSequence shipMovement;
	MovementSequence center;
	Tileset *ts_ship;
	sf::Sprite shipSprite;

	ShipExitScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddFlashes();
	void AddEnemies();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
};

struct MomentaScene : Sequence
{
	enum State
	{
		BROADCAST,
		Count
	};

	MomentaScene();
	void SetupStates();
	void ReturnToGame();
	void AddFlashes();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
};

struct StorySequence;
struct StoryScene : Sequence
{
	enum State
	{
		SHOWSTORY,
		Count
	};

	StorySequence *story;

	StoryScene( const std::string &name );
	void Reset();
	~StoryScene();
	void SetupStates();
	void ReturnToGame();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
};

#endif 
