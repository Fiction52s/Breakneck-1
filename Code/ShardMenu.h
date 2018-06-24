#ifndef __SHARDMENU_H__
#define __SHARDMENU_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "Input.h"
#include "ShardTypes.h"
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

//struct Sparkle
//{
//	Sparkle(Tileset *ts,int type, sf::Vector2f &pos);
//	void Draw(sf::RenderTarget *target);
//	int frame;
//	Tileset *ts;
//	sf::Vector2f pos;
//	sf::Vertex 
//};

struct ShardMenu
{
	//looping movie that plays
	//with button inputs over it?

	//tutorial shards are the first kind,
	//leave room open for other types, but
	//this is the basic one for world 1
	//
	enum ShardButtonState
	{
		S_NEUTRAL,
		S_PRESSED,
		S_UNPRESSED,
		S_SELECTED,
	};

	enum State
	{
		PLAYING,
		PAUSED,
		WAIT,
		LOADTOPLAY,
	};
	State state;
	EffectPool *sparklePool;
	ShardMenu( MainMenu *mm );
	~ShardMenu();
	void Update( ControllerState &currInput,
		ControllerState &prevInput );
	void Draw(sf::RenderTarget *target);

	void SetCurrShard();

	int totalFrame;

	sf::Text currShardText;
	sf::Text currShardNameText;
	std::string **shardDesc;// [ShardType::SHARD_W1_TEACH_JUMP];
	Tileset *ts_shards[7];
	sf::Sprite selectedShardHighlight;
	ShardButtonState currButtonState;
	bool SetDescription(std::string &nameStr, std::string &destStr, const std::string &shardTypeStr );
	void SetCurrentDescription( bool captured );
	bool LoadPNGSequences();
	sf::Vertex *shardQuads;
	MainMenu *mainMenu;
	int numShardsTotal;
	
	sf::Vector2f imagePos;
	void SetCurrSequence();
	void SetupShardImages();
	TilesetManager tMan;
	PNGSeq * GetSequence(const std::string &str);
	MusicInfo *GetShardMusic(const std::string &str);
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	Tileset **ts_preview;
	sf::Sprite previewSpr;
	std::map<std::string, PNGSeq*> seqMap;
	std::string **shardNames;
	std::string **shardDescriptionNames;
	MusicInfo *currShardMusic;
	void SetCurrMusic();
	void StopMusic();
	boost::thread ***seqLoadThread;
	PNGSeq ***shardSeq;
	void AsyncGetSequence(const std::string &str);
	static void sGetSequence( const std::string &str, 
		ShardMenu *sMenu );
	PNGSeq *&GetCurrSeq();
	boost::thread *&GetCurrLoadThread();
	
	void UpdateUnlockedShards();
	bool IsShardCaptured( int x, int y );
	bool IsCurrShardCaptured();

	sf::Vertex *shardSelectQuads;
	void UpdateShardSelectQuads();

	sf::Vertex shardBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex descriptionBGQuad[4];
	sf::Vertex controlsQuadBGQuad[4];
	sf::Vertex shardTitleBGQuad[4];
	sf::Vertex largeShardContainer[4];
	sf::Vertex largeShard[4];
	sf::Vertex shardButtons[4 * 4];

	void SetShardTab();
	Tileset *ts_shardButtons;
	Tileset *ts_shardContainer;
	Tileset *ts_sparkle;
	Tileset *ts_bigShards;

	Tileset *ts_notCapturedPreview;
	Tileset *ts_noPreview;
	
};

#endif