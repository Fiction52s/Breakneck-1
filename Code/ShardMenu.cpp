//sfe::Movie m;
//assert(m.openFromFile("Movie/testvid.ogv"));
//m.fit(sf::FloatRect(0, 0, 1920, 1080));
//
//
//m.play();

#include "ShardMenu.h"

using namespace sf;

ShardMenu::ShardMenu()
{
	//currentMovie.scale(.5, .5);

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];

}

void ShardMenu::SetupShardImages()
{

}

void ShardMenu::Update()
{
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	//target->draw(currentMovie);
}