#ifndef __PRIMITIVE3D_H__
#define __PRIMITIVE3D_H__

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

struct Primitive
{
	Primitive();
	int sides;
	void Draw( sf::RenderTarget *target );
	void Draw2( sf::RenderTarget *target );
	void DrawTetrahedron( sf::RenderTarget *target );
	GLfloat rtri;
	GLfloat rquad;
	GLfloat scale;
	sf::Color color;
};

#endif 