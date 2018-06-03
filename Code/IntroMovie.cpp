#include "IntroMovie.h"
#include <assert.h>

IntroMovie::IntroMovie()
{
	assert(movie.openFromFile("Movie/Kin_Film_2c.ogv"));
	movie.fit(sf::FloatRect(0, 0, 1920, 1080));

	/*const sf::Texture &currImage = m.getCurrentImage();*/

	//sh.setUniform("texture", currImage);
}

void IntroMovie::Play()
{
	movie.setPlayingOffset(sf::Time::Zero);
	movie.play();
}

void IntroMovie::Update()
{
	movie.update();
}

void IntroMovie::Stop()
{
	movie.stop();
}

void IntroMovie::Draw(sf::RenderTarget *target)
{
	target->draw(movie);
}