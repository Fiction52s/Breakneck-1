#ifndef __SHARDMENU_H__
#define __SHARDMENU_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "Input.h"
#include "ShardTypes.h"
#include <SFML\Audio.hpp>
#include <map>

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

	bool singleImage;
	int tileIndex;
	int setIndex;
	std::string seqName;
	int numSets;
	void Reset();
	int numFrames;
	void Load();
	void Draw(sf::RenderTarget *target);
	void Update();
	sf::Sprite spr;
};

struct SingleAxisSelector;
struct MainMenu;
struct MusicInfo;
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

	ShardMenu( MainMenu *mm );
	~ShardMenu();
	void Update( ControllerState &currInput );
	void Draw(sf::RenderTarget *target);

	ShardButtonState currButtonState;
	bool LoadPNGSequences();
	sf::Vertex *shardQuads;
	MainMenu *mainMenu;
	int numShardsTotal;
	PNGSeq *testSeq;	
	sf::Vector2f imagePos;
	void SetCurrSequence();
	void SetupShardImages();
	TilesetManager tMan;
	PNGSeq * GetSequence(const std::string &str);
	MusicInfo *GetShardMusic(const std::string &str);
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	std::map<std::string, PNGSeq*> seqMap;
	std::string **shardNames;
	MusicInfo *currShardMusic;
	void SetCurrMusic();
	void StopMusic();
	

	


	sf::Vertex *shardSelectQuads;
	void UpdateShardSelectQuads();
};

#endif