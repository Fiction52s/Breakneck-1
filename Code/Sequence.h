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

struct FlashedImage
{
	FlashedImage( Tileset *ts,
		int tileIndex, int appearFrames,
		int holdFrames,
		int disappearFrames,
		sf::Vector2f &pos );
	void Reset();
	void Flash();
	void Update();
	void Draw(sf::RenderTarget *target);
	int GetNumFrames();
	bool IsDone();
	void StopHolding();
	bool IsHolding();
	bool IsFadingIn();
	sf::Sprite spr;
	int frame;
	bool flashing;
	int aFrames;
	int dFrames;
	int hFrames;
};

struct Cutscene
{
	Cutscene( GameSession *owner,
		sf::Vector2i & origin );
	~Cutscene();
	Tileset **tilesets;
	bool LoadFromFile( const std::string &path );
	sf::View & GetView( int frame );
	void Draw( sf::RenderTarget *target, int frame );
	GameSession *owner;
	//CamInfo *cameras;
	sf::View *cameras;
	std::list<sf::Sprite> **activeSprites;
	int totalFrames;
	bool init;
	sf::Vector2f origin;
};

struct Sequence
{
	//Sequence *next;
	//Sequence *prev;
	Sequence()
		:frameCount(-1),frame(0)
	{

	}
	int frameCount;
	int frame;
	virtual ~Sequence() {}
	virtual void Init() {}
	virtual bool Update() = 0;
	virtual void Reset() = 0;
	virtual bool UsesSequenceMode() { return false; }
	virtual void Draw( sf::RenderTarget *target, 
		EffectLayer layer = EffectLayer::IN_FRONT ) = 0;
};

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

struct BasicMovieSeq : Sequence
{
	BasicMovieSeq(GameSession *owner,
		const std::string &movieName,
		int preMovieLength,
		int postMovieLength );
	~BasicMovieSeq();

	enum State
	{
		PREMOVIE,
		PLAYMOVIE,
		POSTMOVIE,
		END,
		Count
	};

	ButtonHolder *startHolder;
	State state;
	int stateLength[Count];
	
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	virtual void PreMovieUpdate() {}
	virtual void PostMovieUpdate() {}


	GameSession *owner;
	sfe::Movie mov;
};

struct ConversationGroup;
struct CameraShot;
struct Barrier;
struct BasicBossScene : Sequence
{
	BasicBossScene(GameSession *owner);
	virtual ~BasicBossScene();
	virtual void Init();
	virtual bool Update();
	virtual void SetupStates() = 0;
	virtual void Reset();
	virtual void ConvUpdate();
	static BasicBossScene *CreateScene(
		GameSession *owner, const std::string &name);

	virtual void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	virtual void EntranceUpdate();
	virtual bool IsEntering();
	virtual void ReturnToGame();
	virtual	void Wait();
	virtual void SetEntranceRun();
	virtual void SetEntranceShot();
	virtual void UpdateState() = 0;
	void AddGroup(const std::string &fileName,
		const std::string &groupName);
	void AddShot(const std::string &shotName);
	void AddPoint(const std::string &poiName);
	Conversation *GetCurrentConv();
	void StartEntranceRun(bool fr,
		double maxSpeed, const std::string &n0,
		const std::string &n1);
	void SetCameraShot(const std::string &n);

	int fadeFrames;
	int state;
	int *stateLength;//[Count];
	int numStates;


	std::map<std::string, ConversationGroup*> groups;
	std::map<std::string, CameraShot*> shots;
	std::map<std::string, PoiInfo*> points;

	ConversationGroup *currConvGroup;
	int cIndex;

	Barrier *barrier;

	GameSession *owner;
};


#endif 
