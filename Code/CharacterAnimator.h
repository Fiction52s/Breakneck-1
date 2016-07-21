#ifndef __CHARACTERANIMATOR_H__
#define __CHARACTERANIMATOR_H__
#include "EditSession.h"
#include <list>

struct Entity
{
	float angle;
	sf::Vector2f scale;
	bool facingRight;
	sf::Sprite sprite;
};

struct Layer
{
	std::list<Entity*> entities;

};

struct Frame
{
	std::list<Layer*> layers;
};

struct Animator
{
	Animator();
	std::list<Frame*> frames;
};
//list<Layer> layers;

#endif