#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__
#include "Tileset.h"
#include "SFML/Graphics.hpp"
#include <list>
#include "Movement.h"

struct GameSession;
struct MovementSequence;
struct Cutscene
{
	Cutscene( GameSession *owner,
		sf::Vector2i & origin );
	~Cutscene();
	Tileset **tilesets;
	bool LoadFromFile( const std::string &path );
	sf::View & GetView( int frame );
	void Draw( sf::RenderTarget *target, int frame );
	GameSession *owner;
	//CamInfo *cameras;
	sf::View *cameras;
	std::list<sf::Sprite> **activeSprites;
	int totalFrames;
	bool init;
	sf::Vector2f origin;
};

struct Sequence
{
	//Sequence *next;
	//Sequence *prev;
	int frameCount;
	int frame;
	virtual bool Update() = 0;
	virtual void Reset() = 0;
	virtual void Draw( sf::RenderTarget *target ) = 0;
};

struct CrawlerFightSeq : Sequence
{
	CrawlerFightSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target );
	void Reset();

	MovementSequence camMove0;
	//Tileset *ts_fall;
	//Tileset *ts_land;

	sf::Vector2<double> startFallPos;
	sf::Vector2<double> nexus1Pos;
	sf::Vector2<double> cfightCenter;
	Edge *landGround;
	double landQuant;
	int playerStartFrame;


	

	Tileset *ts_crawlerBoss;


	//Tileset *

	GameSession *owner;
};

#endif 
