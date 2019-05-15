#ifndef __INTRO_MOVIE_H__
#define __INTRO_MOVIE_H__

#include <sfeMovie\Movie.hpp>
struct ButtonHolder;

struct IntroMovie
{
	IntroMovie();
	~IntroMovie();
	sfe::Movie movie;
	void Play();
	void Stop();
	bool Update();
	void Draw(sf::RenderTarget *target);
	ButtonHolder *skipHolder;
};



#endif