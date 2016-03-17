#ifndef __AIRPARTICLES_H__
#define __AIRPARTICLES_H__
#include <SFML/Graphics.hpp>


struct Emitter
{
	int numParticles;
	
	//const sf::Vector2<double> & GetVel( int index );


	//sf::Vector2<double> velocity[
};

struct AirParticleEffect
{
	AirParticleEffect( sf::Vector2<double> &position );
	void Update( sf::Vector2<double> &playerPos );
	void SetDirection( sf::Vector2<double> &direction );
	~AirParticleEffect();
	void UpdateParticle( int index );
	void KillParticle( int index );
	void ResetParticle( int index );
	void Reset();
	int emitFrame;
	int emitDuration;
	double angleRange;
	sf::Vector2<double> dir;
	sf::IntRect particleSize;
	
	double maxDurationToLive;
	int pastParts;
	double particleAcc;
	double particleRate; //release particles as
	//it passes integer values

	int numParticles;
	double *durationToLive;
	sf::Vector2<double> position;
	sf::Vector2<double> *velocities;
	sf::Vector2<double> *positions;
	sf::VertexArray *particles;
	//bool activated;
};

struct Actor;
struct RotaryParticleEffect
{
	RotaryParticleEffect( Actor *pl );
	void Update( sf::Vector2<double> &playerPos );
	void SetDirection( sf::Vector2<double> &direction );
	~RotaryParticleEffect();
	void UpdateParticle( int index );
	void KillParticle( int index );
	void ResetParticle( int index );
	void SetRadius( int radius );
	void Reset();
	int emitFrame;
	int emitDuration;
	double angleRange;
	sf::Vector2<double> dir;
	sf::IntRect particleSize;
	Actor *player;

	double angularVel; //degrees
	double angle; //degrees
	double radius;
	
	double maxDurationToLive;
	int pastParts;
	double particleAcc;
	double particleRate; //release particles as
	//it passes integer values

	int numParticles;
	double *durationToLive;
	sf::Vector2<double> position;
	sf::Vector2<double> *velocities;
	sf::Vector2<double> *positions;
	sf::VertexArray *particles;
	//bool activated;
};


#endif