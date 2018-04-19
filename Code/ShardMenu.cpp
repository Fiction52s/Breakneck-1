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
	currentMovie.scale(.5, .5);
	currentMovie.openFromFile("Movie/Untitled.ogv");
	currentMovie.setPosition(500, 500);
	//currentMovie.play();

	numShardsTotal = 1;
	shardQuads = new Vertex[numShardsTotal * 4];

}

void ShardMenu::SetupShardImages()
{

}

void ShardMenu::Update()
{
	//if( currentMovie.getStatus() == sfe::Stopped )
	if (currentMovie.getDuration().asMilliseconds() - currentMovie.getPlayingOffset().asMilliseconds() < 400 )
	{
		//currentMovie.stop();
		//currentMovie.play();
		currentMovie.setPlayingOffset(sf::Time::Zero);
		//currentMovie.play();
	}

	currentMovie.update();
}

void ShardMenu::Draw(sf::RenderTarget *target)
{
	target->draw(currentMovie);
}