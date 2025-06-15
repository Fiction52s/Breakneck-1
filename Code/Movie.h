#ifndef __MOVIE_H__
#define __MOVIE_H__

#include <sfeMovie\Movie.hpp>

struct Movie
{
	Movie( const std::string &name, bool p_loop = false );
	Movie();
	~Movie();
	void Load(const std::string &name, bool p_loop = false);
	void SetSize(sf::Vector2f size, bool preserveRatio = true );
	void Update();
	void Stop();
	void Play();
	void SetPause(bool p);
	bool IsLooping();
	bool IsStopped();
	bool IsPlaying();
	bool IsPaused();
	void SetPosition(sf::Vector2f pos);
	void SetPlayingOffset( const sf::Time &targetSeekTime );
	void Draw(sf::RenderTarget *target);

private:
	sfe::Movie mov;
	bool loop;
};

#endif