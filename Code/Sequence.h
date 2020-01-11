#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__
#include "Tileset.h"
#include "SFML/Graphics.hpp"
#include <list>
#include "Movement.h"
#include <boost/thread.hpp>
#include <sfeMovie\Movie.hpp>
#include "EffectLayer.h"
#include "MovingGeo.h"

struct Edge;
struct GameSession;
struct MovementSequence;
struct PoiInfo;
struct MusicInfo;
struct ShapeEmitter;
struct StorySequence;
struct Conversation;
struct Enemy;

struct PanInfo
{
	PanInfo(sf::Vector2f &v,
		int start, int len)
	{
		velocity = v;
		startFrame = start;
		frameLength = len;
	}
	sf::Vector2f velocity;
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
	void AddPan(sf::Vector2f &pVel,
		int startFrame, int frameLength );
	void AddPanX(float x,
		int startFrame, int frameLength);
	void AddPanY(float y,
		int startFrame, int frameLength);

	std::list<PanInfo*> panList;

	sf::Vector2f origPos;
	sf::Vector2f position;

	PanInfo * currPan;

	sf::Sprite spr;
	int frame;
	bool flashing;
	int aFrames;
	int dFrames;
	int hFrames;
};

struct Sequence
{
	//Sequence *next;
	//Sequence *prev;
	Sequence()
		:frameCount(-1),frame(0),
		nextSeq( NULL )
	{

	}
	int frameCount;
	int frame;
	Sequence *nextSeq;
	virtual ~Sequence();
	virtual void StartRunning(){}
	virtual bool Update() = 0;
	virtual void Reset() = 0;
	virtual void Draw( sf::RenderTarget *target, 
		EffectLayer layer = EffectLayer::IN_FRONT ) = 0;
};

struct BasicBossScene;

struct ShipExitSeq : Sequence
{
	enum State
	{
		SHIP_SWOOP,
		STORYSEQ,
		END
	};

	State state;
	int stateLength[END];
	ShipExitSeq( GameSession *owner );
	~ShipExitSeq();
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	StorySequence *storySeq;
	BasicBossScene *scene;
	//sfe::Movie mov;
	Tileset *ts_ship;
	sf::Sprite shipSprite;
	MovementSequence shipMovement;
	MovementSequence center;
	GameSession *owner;
	int enterTime;
	int exitTime;
	sf::Vector2<double> abovePlayer;
	sf::Vector2<double> origPlayer;
	sf::Vector2<double> attachPoint;
};

struct ButtonHolder;

struct ConversationGroup;
struct CameraShot;
struct Barrier;

//typedef std::list<FlashedImage*> FlashGroup;

struct FlashInfo
{
	FlashInfo(FlashedImage *fi,
		int eEnd = 0)
	{
		image = fi;
		earlyEnd = eEnd;
	}
	FlashedImage *image;
	int earlyEnd;
};

struct FlashGroup
{
	~FlashGroup();
	void Reset();
	void AddFlash(FlashedImage *fi,
		int earlyEndFrames = 0);
	std::list<FlashInfo*> fList;
	std::list<FlashInfo*>::iterator currFlash;
	bool IsDone();



	bool done;
};

struct BasicBossScene : Sequence
{
	enum EntranceType
	{
		RUN,
		APPEAR,
	};

	BasicBossScene(GameSession *owner, 
		EntranceType et );
	EntranceType entranceType;
	virtual ~BasicBossScene();
	void Init();
	virtual void StartRunning();
	virtual bool Update();
	virtual void SetupStates() = 0;
	void SetNumStates(int count);
	virtual void Reset();
	virtual void AddShots(){}
	virtual void AddPoints(){}
	virtual void AddFlashes(){}
	virtual void AddEnemies(){}
	virtual void AddGroups(){}
	virtual void AddMovies() {}
	virtual void SpecialInit(){}
	virtual void ConvUpdate();
	void EndCurrState();
	bool IsCamMoving();
	bool IsLastFrame();
	static BasicBossScene *CreateScene(
		GameSession *owner, const std::string &name);
	virtual void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	virtual void DrawFlashes(sf::RenderTarget *target);
	virtual void EntranceUpdate();
	virtual bool IsEntering();
	virtual void ReturnToGame();
	virtual	void Wait();
	virtual void SetEntranceRun();
	virtual void SetEntranceStand();
	virtual void SetEntranceShot();
	void SetPlayerStandPoint(const std::string &n,
		bool fr);
	virtual void UpdateState() = 0;
	void EaseShot(const std::string &shotName,
		int frames, CubicBezier bez = CubicBezier());
	void EasePoint(const std::string &pointName,
		float targetZoom, int frames,
		CubicBezier bez = CubicBezier());
	void Flash(const std::string &flashName);
	void Rumble(int x, int y, int duration);
	void RumbleDuringState(int x, int y);
	void BasicFlashUpdateState(
	const std::string &flashName );
	void AddGroup(const std::string &fileName,
		const std::string &groupName);
	void AddShot(const std::string &shotName);
	void AddPoint(const std::string &poiName);
	void SetEntranceIndex(int ind);
	void AddStartPoint();
	void AddStopPoint();
	void AddStandPoint();
	void AddStartAndStopPoints();
	void SetPlayerStandDefaultPoint(bool fr);
	FlashedImage * AddFlashedImage(const std::string &imageName,
		Tileset *ts, int tileIndex,
		int appearFrames,
		int holdFrames,
		int disappearFrames,
		sf::Vector2f &pos);
	void AddMovie(const std::string &movieName);
	
	void AddEnemy( const std::string &enName,
		Enemy *e);

	void UpdateFlashes();
	void UpdateMovie();
	void SetConvGroup(const std::string &n);
	Conversation *GetCurrentConv();

	void SetCurrMovie(const std::string &name,
		int movFadeFrames = 0,
		sf::Color movFadeColor = sf::Color::Black );
	int movieFadeFrames;
	sf::Color movieFadeColor;
	int movieStopFrame;

	void StartEntranceRun(bool fr,
		double maxSpeed, const std::string &n0,
		const std::string &n1);
	void StartEntranceStand(bool fr,
		const std::string &n);
	void SetCameraShot(const std::string &n);

	int fadeFrames;
	int state;
	int *stateLength;//[Count];
	int numStates;
	int entranceIndex;


	std::map<std::string, ConversationGroup*> groups;
	std::map<std::string, CameraShot*> shots;
	std::map<std::string, PoiInfo*> points;
	std::map<std::string, FlashedImage*> flashes;
	std::map <std::string, FlashGroup*> flashGroups;
	FlashGroup * AddFlashGroup(const std::string &n);
	void AddFlashToGroup(FlashGroup *,
		const std::string &n, int earlyEnd = 0);
	void UpdateFlashGroup();
	void SetFlashGroup( const std::string &n);
	std::list<FlashedImage*> flashList;
	std::map<std::string, Enemy *> enemies;
	std::map<std::string, sfe::Movie*> movies;

	FlashGroup *currFlashGroup;

	sfe::Movie *currMovie;
	ConversationGroup *currConvGroup;
	int cIndex;

	Barrier *barrier;

	GameSession *owner;
};

#endif 
