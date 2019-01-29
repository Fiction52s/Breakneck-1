#ifndef __STORYSEQUENCE_H__
#define __STORYSEQUENCE_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "Input.h"

struct MusicInfo;
struct StoryMusic
{
	std::string musicName;
	MusicInfo *musicInfo;
	sf::Time startTime;
	float transitionSeconds;
};

struct StoryText
{
	StoryText( sf::Font &font, const std::string &p_str, sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	bool Update( ControllerState &prev, ControllerState &curr);
	void Reset();
	std::string textStr;
	sf::Text text;
	bool done;
};

struct StoryPart
{
	StoryPart();
	void Reset();
	Tileset *ts;
	sf::Sprite spr;
	sf::Vector2f pos;
	bool hasIntro;
	
	int layer;
	float time;
	int frame;
	int totalFrames;
	bool Update(ControllerState &prev, ControllerState &curr);
	void Draw(sf::RenderTarget *target);
	std::string imageName;
	StoryText *text;
	StoryPart *sub;

	StoryMusic *music;
};

struct GameSession;
struct StorySequence
{
	StorySequence( sf::Font &font, TilesetManager *tm);
	StorySequence(GameSession *owner);
	bool Load( const std::string &sequenceName );
	void Reset();
	bool Update(ControllerState &prev, ControllerState &curr);
	void Draw( sf::RenderTarget *target );
	std::list<StoryPart*> parts;
	std::list<StoryPart*>::iterator currPartIt;
	TilesetManager *tm;
	sf::Font &myFont;
	GameSession *owner;

	MusicInfo *oldMusic;
};

#endif