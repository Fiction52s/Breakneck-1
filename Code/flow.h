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
	void Reset();

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

struct GPUFlow
{
	enum Textures
	{
		TEXTURE_BUFFER,
		TEXTURE_VELOCITY,
		TEXTURE_DENSITY,
		TEXTURE_DIVERGENCE,
		TEXTURE_PRESSURE,
		//TEXTURE_VORTICITY,
		//TEXTURE_VELOCITY_OFFSETS,
		//TEXTURE_PRESSURE_OFFETTS,
		TEXTURE_COUNT
	};

	enum Shaders
	{
		SHADER_ADVECT,
		SHADER_BOUNDARY,
		SHADER_DIVERGENCE,
		SHADER_GRADIENT,
		SHADER_JACOBI,
		SHADER_IMPULSE,
		SHADER_COUNT
	};

	sf::Vector2i position;
	int numJacobiSteps;
	float visc;
	sf::Shader *currShader;
	void ExecuteShaderRect(
		Textures tex );
	sf::RenderTexture **textures;
	GPUFlow( const sf::Vector2i &p, int w, int h );
	void Update();
	void Draw( sf::RenderTarget *target );
	void SetDiffuse( Textures xTex, Textures bTex );
	void AddSource( Textures receiver, Textures source );
	void VelocityStep();
	void SetImpulse( Textures tex );
	void SetAdvect(Textures velTex, Textures quantityTex);
	void SetJacobi( float alpha, float rBeta, Textures xTex, Textures bTex );
	void SetDivergence( Textures wTex );
	void SetGradient( Textures wTex, Textures pTex );
	void SetBoundary( sf::Vector2f &offset, Textures xTex );
	~GPUFlow();
	void Reset();
	
	sf::Shader *shaders;

	Actor *player;
	float timestep;
	float tileSize;
	int width;
	int height;
};

#endif