#ifndef __SHADERTESTER_H__
#define __SHADERTESTER_H__

#include <SFML\Graphics.hpp>
struct GameSession;

struct ShaderTester
{
	enum ShaderType
	{
		FIRE
	};
	ShaderTester(ShaderType t, GameSession *owner);
	ShaderType type;
	void Update();
	sf::Shader sh;
	int frame;
	sf::Vertex quad[4];
	void Draw(sf::RenderTarget *target);

};


#endif