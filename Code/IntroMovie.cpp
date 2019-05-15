#include "IntroMovie.h"
#include <assert.h>
#include "ButtonHolder.h"

IntroMovie::IntroMovie()
{
	assert(movie.openFromFile("Resources/Movie/Kin_Intro.ogv"));
	movie.fit(sf::FloatRect(0, 0, 1920, 1080));

	/*const sf::Texture &currImage = m.getCurrentImage();*/
	skipHolder = new ButtonHolder(60);
	//sh.setUniform("texture", currImage);
}

IntroMovie::~IntroMovie()
{
	delete skipHolder;
}

void IntroMovie::Play()
{
	movie.setPlayingOffset(sf::Time::Zero);
	movie.play();
	skipHolder->Reset();
}

bool IntroMovie::Update()
{
	movie.update();

	if (movie.getStatus() == sfe::Status::End
		|| movie.getStatus() == sfe::Status::Stopped || skipHolder->IsHoldComplete())
	{
		return false;
	}

	return true;
}

void IntroMovie::Stop()
{
	movie.stop();
}

void IntroMovie::Draw(sf::RenderTarget *target)
{
	target->draw(movie);
}