#ifndef __SHARDMENU_H__
#define __SHARDMENU_H__

#include <SFML/Graphics.hpp>

struct TutorialMovie
{
	TutorialMovie(
		const std::string &folderPath);
	int frame;
	//sf::Texture *
	//sf::Sprite spr;
	void Draw(sf::RenderTarget *target);
};

struct ShardMenu
{
	//looping movie that plays
	//with button inputs over it?

	//tutorial shards are the first kind,
	//leave room open for other types, but
	//this is the basic one for world 1
	//
	ShardMenu();
	void Update();
	void Draw(sf::RenderTarget *target);

	sf::Vertex *shardQuads;
	int numShardsTotal;

	void SetupShardImages();
};

#endif