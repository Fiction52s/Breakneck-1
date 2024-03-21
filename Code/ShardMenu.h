#ifndef __SHARDMENU_H__
#define __SHARDMENU_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "Input.h"
#include <SFML\Audio.hpp>
#include <map>
#include <string>
#include <boost/thread.hpp>

struct TilesetManager;
struct TutorialMovie
{
	TutorialMovie(
		const std::string &folderPath);
	
};

struct Tileset;
struct PNGSeq
{
	PNGSeq(const
		std::string &seqName,
		std::list<Tileset*> &tList, bool singleImage = false );// , Tileset *ts);
	~PNGSeq();
	Tileset **tSets;
	Tileset *ts_preview;
	bool singleImage;
	int tileIndex;
	int setIndex;
	std::string seqName;
	int numSets;
	void Reset();
	int numFrames;
	void IncrementFrame();
	void DecrementFrame();
	void Load();
	void Draw(sf::RenderTarget *target);
	void Update();
	sf::Sprite spr;
};

struct SingleAxisSelector;
struct MainMenu;
struct MusicInfo;
struct EffectPool;
struct GameSession;

//struct Sparkle
//{
//	Sparkle(Tileset *ts,int type, sf::Vector2f &pos);
//	void Draw(sf::RenderTarget *target);
//	int frame;
//	Tileset *ts;
//	sf::Vector2f pos;
//	sf::Vertex 
//};


struct Session;

struct ShardDetailedInfo
{
	std::string name;
	std::string desc;
};

struct ShardMenu
{
	enum State
	{
		PLAYING,
		PAUSED,
		WAIT,
		LOADTOPLAY,
	};

	enum SelectMode
	{
		SM_WORLD,
		SM_SHARD,
	};

	sf::Vector2f containerCenter;

	State state;

	EffectPool *sparklePool;
	
	sf::Text worldText;
	sf::Text currShardText;
	sf::Text currShardNameText;

	ShardDetailedInfo **shardInfo;

	TilesetManager *tm;
	
	Session *sess;

	int currSelectMode;
	int selectedIndex;
	int totalFrame;

	sf::Vector2f imagePos;

	TilesetManager tMan;

	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	SingleAxisSelector *worldSelector;

	sf::Vertex selectedBGQuad[4];
	sf::Vertex *shardSelectQuads;
	sf::Vertex shardBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex descriptionBGQuad[4];
	sf::Vertex shardTitleBGQuad[4];
	sf::Vertex largeShardContainer[4];
	sf::Vertex largeShard[4];
	sf::Vertex shardButtons[4 * 4];
	Tileset *ts_shards[7];
	Tileset *ts_shardContainer;
	Tileset *ts_sparkle;

	ShardMenu( TilesetManager *tm );
	~ShardMenu();

	void SetSession(Session *sess);
	void SetTopLeft(sf::Vector2f &pos);
	void Update( ControllerState &currInput,
		ControllerState &prevInput );
	void Draw(sf::RenderTarget *target);
	void SetCurrShard();
	void SetCurrentDescription( bool captured );
	const std::string &GetShardDesc(int w, int li);
	const std::string &GetShardName(int w, int li);
	ShardDetailedInfo &GetCurrShardInfo();
	void LoadShardInfo();
	void UpdateUnlockedShards();
	bool IsCurrShardCaptured();
	void UpdateShardSelectQuads();
	void SetShardTab();
	void UpdateShardQuads();
	void UpdateWorld();
	bool IsShardCaptured(int w, int li);
	void SetWorldMode();
};

#endif