#include "Movie.h"
#include <string>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace sf;

Movie::Movie(const std::string &name, bool p_loop )
{
	if (!mov.openFromFile("Resources/Movie/" + name + ".mp4"))
	{
		cout << "movie not loaded: " << name << "\n";
		assert(false);
	}
	loop = p_loop;
}

Movie::~Movie()
{

}

void Movie::SetSize(sf::Vector2f size, bool preserveRatio )
{
	//mov.fit(sf::FloatRect(0, 0, 750, 505));
	mov.fit(0, 0, size.x, size.y, preserveRatio );
}

void Movie::Update()
{
	mov.update();

	if (IsLooping())
	{
		if (IsStopped())
		{
			Play();
		}
	}
}

void Movie::Stop()
{
	if (mov.getStatus() != sfe::Stopped)
	{
		mov.stop();
	}
}

void Movie::Play()
{
	if (mov.getStatus() == sfe::Playing)
	{
		mov.stop();
	}

	mov.play();
}

void Movie::SetPause(bool pause)
{
	if (pause && mov.getStatus() == sfe::Playing)
	{
		mov.pause();
	}
	else if (!pause && mov.getStatus() == sfe::Paused)
	{
		mov.play();
	}
}

bool Movie::IsLooping()
{
	return loop;
}

bool Movie::IsStopped()
{
	return mov.getStatus() == sfe::Stopped;
}

bool Movie::IsPlaying()
{
	return mov.getStatus() == sfe::Playing;
}

bool Movie::IsPaused()
{
	return mov.getStatus() == sfe::Paused;
}

void Movie::SetPosition(sf::Vector2f pos)
{
	mov.setPosition(pos);
}

void Movie::SetPlayingOffset(const sf::Time &targetSeekTime)
{
	mov.setPlayingOffset(targetSeekTime);
}

void Movie::Draw(sf::RenderTarget *target)
{
	target->draw(mov);
}