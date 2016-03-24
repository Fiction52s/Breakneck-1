#include "flow.h"
#include <iostream>
#include "Actor.h"
#include <assert.h>

using namespace std;
using namespace sf;

GPUFlow::GPUFlow( const sf::Vector2i &p, int w, int h )
	:width( w ), height( h ), position( p )
{
	currShader = NULL;
	visc = .0001f;
	timestep = 1.f / 60.f;
	shaders = new Shader[SHADER_COUNT];
	textures = new RenderTexture*[TEXTURE_COUNT];
	numJacobiSteps = 50;

	for( int i = 0; i < TEXTURE_COUNT; ++i )
	{
		textures[i] = new RenderTexture;
		textures[i]->create( w, h );
		textures[i]->clear(Color::Transparent); 
	}

	if (!shaders[SHADER_ADVECT].loadFromFile("advect.frag", sf::Shader::Fragment ) )
	{
		cout << "advect SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_BOUNDARY].loadFromFile("boundary.frag", sf::Shader::Fragment ) )
	{
		cout << "boundary SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_DIVERGENCE].loadFromFile("divergence.frag", sf::Shader::Fragment ) )
	{
		cout << "divergence SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_GRADIENT].loadFromFile("gradient.frag", sf::Shader::Fragment ) )
	{
		cout << "gradient SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_JACOBI].loadFromFile("jacobi.frag", sf::Shader::Fragment ) )
	{
		cout << "jacobi SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_IMPULSE].loadFromFile("impulse.frag", sf::Shader::Fragment ) )
	{
		cout << "impulse SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

}

//executes the shader in currShader over the rectangle of the fluid area
void GPUFlow::ExecuteShaderRect( Textures tex )
{
	//textures[tex]->display();
	//sf::Texture t = textures[tex]->getTexture();

	sf::RectangleShape rs( Vector2f( width, height ) );
	RenderStates renderState;
	renderState.shader = currShader;

	textures[TEXTURE_BUFFER]->clear( Color::Transparent );
	//textures[tex]->clear( Color::Transparent );

	textures[TEXTURE_BUFFER]->draw( rs, renderState );

	//swap
	sf::RenderTexture *temp = textures[tex];
	textures[tex] = textures[TEXTURE_BUFFER];
	textures[TEXTURE_BUFFER] = temp;
}

void GPUFlow::SetImpulse()
{
	Shader &sh = shaders[SHADER_IMPULSE];
	sh.setParameter( "Resolution", Vector2f( width, height) );
	currShader = &sh;
}

void GPUFlow::SetAdvect( Textures velTex, Textures quantityTex )
{
	Shader &sh = shaders[SHADER_ADVECT];

	Vector2u blah = textures[quantityTex]->getSize();
	Vector2f si( blah.x, blah.y );

	textures[velTex]->display();
	textures[quantityTex]->display();
	//need to make sure the texture is displayed before doing this
	sh.setParameter( "u", textures[velTex]->getTexture() );
	sh.setParameter( "x", textures[quantityTex]->getTexture() );
	sh.setParameter( "rdx", 1.f / tileSize );
	sh.setParameter( "timestep", timestep );
	sh.setParameter( "texSize", si );
	currShader = &sh;
}

void GPUFlow::SetJacobi( float alpha, float rBeta, Textures xTex, Textures bTex )
{
	textures[xTex]->display();
	textures[bTex]->display();
	Shader &sh = shaders[SHADER_JACOBI];
	sh.setParameter( "alpha", alpha );
	sh.setParameter( "rBeta", rBeta );
	sh.setParameter( "x", textures[xTex]->getTexture() );
	sh.setParameter( "b", textures[bTex]->getTexture() );
	currShader = &sh;
}

void GPUFlow::SetDivergence( Textures wTex )
{
	textures[wTex]->display();
	Shader &sh = shaders[SHADER_DIVERGENCE];
	sh.setParameter( "halfrdx", .5f / tileSize );
	sh.setParameter( "w", textures[wTex]->getTexture() );
	currShader = &sh;
}

void GPUFlow::SetGradient( Textures wTex, Textures pTex )
{
	textures[wTex]->display();
	textures[pTex]->display();
	Shader &sh = shaders[SHADER_GRADIENT];
	sh.setParameter( "halfrdx", .5f / tileSize );
	sh.setParameter( "w", textures[wTex]->getTexture() );
	sh.setParameter( "p", textures[pTex]->getTexture() );
	currShader = &sh;
}

void GPUFlow::SetBoundary( Vector2f &offset, Textures xTex )
{
	Shader &sh = shaders[SHADER_BOUNDARY];
	sh.setParameter( "offset", offset );
	sh.setParameter( "scale", tileSize );
	sh.setParameter( "x", textures[xTex]->getTexture() );
	currShader = &sh;
}

void GPUFlow::SetDiffuse( Textures xTex, Textures bTex )
{	
	textures[bTex]->display();
	if( visc > 0 )
	{
		float a =   (width * height) / (timestep * visc);
		float stencilFactor = 1.0f / (4.0f + a );
		
		SetJacobi( a, stencilFactor, xTex, bTex );
		Shader &sh = shaders[SHADER_JACOBI];
		for( int i = 0; i < 1; ++i )
		//for( int i = 0; i < numJacobiSteps; ++i )
		{	
			ExecuteShaderRect( xTex );
			textures[xTex]->display();
			sh.setParameter( "x", textures[xTex]->getTexture());
		}
		
	}
	//LinSolve( b, x, x0, a, ( 1 + 4 * a ) );
}

GPUFlow::~GPUFlow()
{
	for( int i = 0; i < TEXTURE_COUNT; ++i )
	{
		delete textures[i];
	}
	delete [] textures;
	delete [] shaders;
}

void GPUFlow::Draw( sf::RenderTarget *target )
{	
	Textures t = TEXTURE_VELOCITY;
	textures[t]->display();
	Sprite sp;
	sp.setTexture( textures[t]->getTexture() );
	sp.setPosition( position.x, position.y );
	sp.setScale( 3, 3 );

	target->draw( sp );
	/*RenderStates rs;
	rs.shader = &shaders[SHADER_IMPULSE];
	target->draw( sp, rs );*/
}

void GPUFlow::Reset()
{

}

void GPUFlow::AddSource( Textures receiver, Textures source )
{

}

void GPUFlow::VelocityStep()
{
	/*AddSource( u, u0 ); AddSource( v, v0 );
	SWAP( u0, u ); Diffuse( 1, u, u0 );
	SWAP( v0, v ); Diffuse( 2, v, v0 );
	Project( u, v, u0, v0 );
	SWAP( u0, u ); SWAP( v0, v );
	Advect( 1, u, u0, u0, v0 ); Advect( 2, v, v0, u0, v0 );
	Project( u, v, u0, v0 );*/
}

void GPUFlow::Update()
{
	SetImpulse();
	//ExecuteShaderRect( TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	//advect velocity
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );


	////advect ink
	//SetAdvect( TEXTURE_DENSITY, TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	//SetDivergence( TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_DIVERGENCE );


	////Diffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY ); //test with visc at 0 first
	////Divergence( TEXTURE_DIVERGENCE );
	////Jacobi( -timestep * timestep, .25f, TEXTURE_PRESSURE,
	////	TEXTURE_DIVERGENCE );

	////clear pressure each step
	////textures[TEXTURE_PRESSURE]->clear( Color::Transparent );

	//SetJacobi( -width * height, 0.25f, TEXTURE_PRESSURE,
	//	TEXTURE_DIVERGENCE );
	//for( int i = 0; i < numJacobiSteps; ++i )
	//{
	//	ExecuteShaderRect( TEXTURE_PRESSURE );
	//}

	//SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );
	//// Apply the first 3 operators in Equation 12.
	//u = advect(u);
	//u = diffuse(u);
	//u = addForces(u);
	//// Now apply the projection operator to the result.
	//p = computePressure(u);
	//u = subtractPressureGradient(u, p);


}