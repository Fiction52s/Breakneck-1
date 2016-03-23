#ifndef __FLOW_H__
#define __FLOW_H__

#include <SFML/Graphics.hpp>

struct Actor;
struct Flow
{
	Flow( const sf::Vector2i &p, int w, int h );
	~Flow();
	enum Textures
	{
		TEXTURE_VELOCITY,
		TEXTURE_DENSITY,
		TEXTURE_DIVERGENCE,
		TEXTURE_PRESSURE,
		//TEXTURE_VORTICITY,
		//TEXTURE_VELOCITY_OFFSETS,
		//TEXTURE_PRESSURE_OFFETTS,
		TEXTURE_COUNT
	};

	sf::VertexArray *va;
	sf::VertexArray *velLines;
	void Update();
	void Draw( sf::RenderTarget *target );
	void CreateTextures();
	int IX( int x, int y );
	void AddSource( float *x, float *s );
	void Diffuse( int b, float *x, float *x0 );
	void SetBound( int b, float *x );
	void Advect( int b, float *d, float *d0, float *u, float *v );
	void DensityStep( float *x, float *x0, float *u, float *v );
	void VelocityStep( float *u, float *v, float *u0, float *v0 );
	void Project( float *u, float *v, float *p, float *div );
	void LinSolve( int b, float *x, float *x0, float a, float c );

	Actor *player;
	float diff;

	float *m_u;
	float *m_v;
	float *m_u_prev;
	float *m_v_prev;
	float *m_dens;
	float *m_dens_prev;
	float *ink_dens;
	int size;
	float dt;

	int width;
	int height;
	float *zeros;
	int numPoissonSteps;
	float gridScale;
	float timeStep;
	float viscosity;
	float inkLongevity;
	bool pressureClearEachStep;
	sf::Color inkColor;
	sf::VertexArray *box;
	sf::VertexArray *borders;
	sf::Vector2i pos;
	sf::RenderTexture *offscreenBuffer;
	sf::Texture **textures;
	sf::Shader *shaders;
};

#endif