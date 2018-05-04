#ifndef __SHARDMENU_H__
#define __SHARDMENU_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "Input.h"

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
		std::list<Tileset*> &tList);// , Tileset *ts);
	~PNGSeq();
	Tileset **tSets;

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
struct ShardMenu
{
	//looping movie that plays
	//with button inputs over it?

	//tutorial shards are the first kind,
	//leave room open for other types, but
	//this is the basic one for world 1
	//
	ShardMenu();
	~ShardMenu();
	void Update( ControllerState &currInput );
	void Draw(sf::RenderTarget *target);

	bool LoadPNGSequences();
	sf::Vertex *shardQuads;
	int numShardsTotal;
	PNGSeq *testSeq;

	void SetupShardImages();
	TilesetManager tMan;
	PNGSeq * GetSequence(const std::string &str);
	 
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;

	sf::Vertex *shardSelectQuads;
	void UpdateShardSelectQuads();
};

#endif