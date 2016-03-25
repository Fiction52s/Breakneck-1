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
	numJacobiSteps = 20;
	tileSize = 1;

	for( int i = 0; i < TEXTURE_COUNT; ++i )
	{
		textures[i] = new RenderTexture;
		textures[i]->create( w, h );
		textures[i]->clear(Color::Black); 
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

	textures[TEXTURE_BUFFER]->clear( Color::Black );
	//textures[tex]->clear( Color::Transparent );

	textures[TEXTURE_BUFFER]->draw( rs, renderState );

	textures[TEXTURE_BUFFER]->display();
	//swap
	sf::RenderTexture *temp = textures[tex];
	textures[tex] = textures[TEXTURE_BUFFER];
	textures[TEXTURE_BUFFER] = temp;


}

void GPUFlow::SetImpulse( Textures tex )
{
	
	Vector2f playerCoords = Vector2f( player->position.x, player->position.y ) - Vector2f(position.x, position.y);
	//playerCoords.y = -playerCoords.y;
	
	textures[tex]->display();

	playerCoords.x /= width * 3;// * 2;
	playerCoords.y /= height * 3;// * 2;

	if( tex == TEXTURE_DENSITY )
	{
		playerCoords = Vector2f( .5, .5 );
	}
	//playerCoords.y = 1 - playerCoords.y;

	//if( playerCoords.x < 0 || playerCoords.y < 0 )
	//{
	//	//to take it out of play
	//	playerCoords.x = -1;
	//	playerCoords.y = -1;
	//}
	
	
	Shader &sh = shaders[SHADER_IMPULSE];
	sh.setParameter( "Resolution", Vector2f( width, height) );
	sh.setParameter( "playerCoords", playerCoords );
	sh.setParameter( "tex", textures[tex]->getTexture() );
	currShader = &sh;
}

void GPUFlow::SetAdvect( Textures velTex, Textures quantityTex )
{
	Shader &sh = shaders[SHADER_ADVECT];

	
	

	//textures[velTex]->display();
	//textures[quantityTex]->display();
	//need to make sure the texture is displayed before doing this
	sh.setParameter( "u", textures[velTex]->getTexture() );
	sh.setParameter( "x", textures[quantityTex]->getTexture() );
	sh.setParameter( "rdx", 1.f / tileSize );
	sh.setParameter( "timestep", timestep );
	Vector2u blah = textures[quantityTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setParameter( "texSize", si );
	currShader = &sh;
}

void GPUFlow::SetJacobi( float alpha, float rBeta, Textures xTex, Textures bTex )
{
	//textures[xTex]->display();
	//textures[bTex]->display();
	Shader &sh = shaders[SHADER_JACOBI];
	sh.setParameter( "alpha", alpha );
	sh.setParameter( "rBeta", rBeta );
	sh.setParameter( "x", textures[xTex]->getTexture() );
	sh.setParameter( "b", textures[bTex]->getTexture() );

	Vector2u blah = textures[xTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setParameter( "texSize", si );

	currShader = &sh;
}

void GPUFlow::SetDivergence( Textures wTex )
{
	

	//textures[wTex]->display();
	Shader &sh = shaders[SHADER_DIVERGENCE];
	sh.setParameter( "halfrdx", .5f / tileSize );
	sh.setParameter( "w", textures[wTex]->getTexture() );

	Vector2u blah = textures[wTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setParameter( "texSize", si );

	currShader = &sh;
}

void GPUFlow::SetGradient( Textures wTex, Textures pTex )
{
	//textures[wTex]->display();
	//textures[pTex]->display();
	Shader &sh = shaders[SHADER_GRADIENT];
	sh.setParameter( "halfrdx", .5f / tileSize );
	sh.setParameter( "w", textures[wTex]->getTexture() );
	sh.setParameter( "p", textures[pTex]->getTexture() );

	Vector2u blah = textures[pTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setParameter( "texSize", si );

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
	//textures[bTex]->display();
	if( visc > 0 )
	{
		float a = timestep * visc;//1;//(width * height * timestep * visc);//.001; 
		float stencilFactor = 1.0f / (4.0f + a );
		//float a = (width*height) * timestep * visc;
		//float stencilFactor = 1.0f / (4.0f * a );
		
		SetJacobi( a, stencilFactor, xTex, bTex );
		Shader &sh = shaders[SHADER_JACOBI];
		for( int i = 0; i < numJacobiSteps; ++i )
		//for( int i = 0; i < numJacobiSteps; ++i )
		{	
			ExecuteShaderRect( xTex );
			//textures[xTex]->display();
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
	//textures[t]->display();
	Sprite sp;
	sp.setTexture( textures[t]->getTexture() );
	sp.setPosition( position.x, position.y );
	sp.setScale( 3, 3 );
	//this is why i have to scale by three in the shader coords

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



	
	
	//adding impulse
	SetImpulse( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );


	SetDivergence( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_DIVERGENCE );

	//textures[TEXTURE_PRESSURE]->clear( Color::Transparent );

	SetJacobi( 1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );
	Shader &sh = shaders[SHADER_JACOBI];
	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( TEXTURE_PRESSURE );
		sh.setParameter( "x", textures[TEXTURE_PRESSURE]->getTexture());
	}

	SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	ExecuteShaderRect( TEXTURE_VELOCITY );







	////textures[TEXTURE_PRESSURE]->clear( Color::Transparent );

	//Project( TEXTURE_VELOCITY, TEXTURE_PRESSURE, TEXTURE_DIVERGENCE );

	//SetGradient( TEXTURE_VELOCITY,TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );


	
	//Project( TEXTURE_VELOCITY, TEXTURE_PRESSURE, TEXTURE_DIVERGENCE );
	//adding impulse
	//SetImpulse( TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	//advect velocity
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	////advect density
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	//

	////diffuse velocity
	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	////project
	//SetDivergence( TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_DIVERGENCE );

	////solve for poisson pressure using jacobi
	//// u = gradient( p );

	////clear pressure each update for now
	//textures[TEXTURE_PRESSURE]->clear( Color::Transparent );

	//SetJacobi( -width * height, 0.25f, TEXTURE_PRESSURE,
	//	TEXTURE_DIVERGENCE );
	//for( int i = 0; i < 20; ++i )
	//{
	//	ExecuteShaderRect( TEXTURE_PRESSURE );
	//}

	//SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );


	//SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );
}

void GPUFlow::Project(Textures velTex, Textures pTex, Textures divTex )
{
	//project

	//get divergence of the velocity
	SetDivergence( velTex );
	ExecuteShaderRect( divTex );

	// u = gradient( p );

	//clear pressure each update for now
	

	//solve for poisson pressure using jacobi
	/*SetJacobi( -1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );*/
	SetJacobi( 1, 0.25f, pTex,
		divTex );

	Shader &sh = shaders[SHADER_JACOBI];
	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( pTex );
		sh.setParameter( "x", textures[pTex]->getTexture());
	}
}

void GPUFlow::DensityStep()
{
	//adding impulse
	SetImpulse( TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	
	
	//SetDiffuse( TEXTURE_DENSITY, TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	//advect ink
	SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	

	//advect velocity
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	////advect density/ink
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	////adding impulse
	//SetImpulse( TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	

	////diffuse velocity
	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	//Project( TEXTURE_VELOCITY, TEXTURE_PRESSURE, TEXTURE_DIVERGENCE );

	//SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );


	//SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );
}

void GPUFlow::Update()
{
	//again, try to do the same thing as the cpu solver just for now

	//adding impulse
	SetImpulse( TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	SetImpulse( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	//advect ink
	SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	SetDivergence( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_DIVERGENCE );

	textures[TEXTURE_PRESSURE]->clear( Color::Transparent );

	SetJacobi( 1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );
	Shader &sh = shaders[SHADER_JACOBI];

	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( TEXTURE_PRESSURE );
		sh.setParameter( "x", textures[TEXTURE_PRESSURE]->getTexture());
	}

	SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	ExecuteShaderRect( TEXTURE_VELOCITY );
	//VelocityStep();
	//DensityStep();
	//adding impulse density
	/*SetImpulse( TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	SetDiffuse( TEXTURE_DENSITY, TEXTURE_PRESSURE );
	SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );*/

	//set the impulses

	//velocitystep


	//densitystep

	//ready to draw

}

void GPUFlow::Update2()
{
	DensityStep();

	//1. Add Impulse ---------------------------------

	//adding impulse velocity
	//SetImpulse( TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	////adding impulse density
	//SetImpulse( TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	////2. Advect --------------------------------
	//	//-creates a divergent field which must be corrected in step 4

	////advect velocity
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	////advect ink
	//SetAdvect( TEXTURE_DENSITY, TEXTURE_DENSITY );
	//ExecuteShaderRect( TEXTURE_DENSITY );

	////3. Apply VorticityConfinement --------------------------------
	//	//-not doing this yet

	////4. Diffuse( if viscosity > 0 ) ---------------------------------------
	//	//-diffuses velocity, solving a poisson problem

	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_PRESSURE );

	//5. Project( computes divergence-free velocity from divergent field ) ---------------------------
		//-a. Compute the divergence of the velocity field
		//-b. Solve the poisson equation, Laplacian(p) = div(u),
			//for p using Jacobi iteration
		//-c. now that we have p, compute the divergence free velocity: u = gradient(p)

	//a: compute the divergence
	//SetDivergence( TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_DIVERGENCE );

	////b: solve the poisson equation for p
	//textures[TEXTURE_PRESSURE]->clear( Color::Transparent );
	//SetJacobi( 1, 0.25f, TEXTURE_PRESSURE, TEXTURE_DIVERGENCE );

	//Shader &sh = shaders[SHADER_JACOBI];
	//for( int i = 0; i < 20; ++i )
	//{
	//	ExecuteShaderRect( TEXTURE_PRESSURE );
	//	sh.setParameter( "x", textures[TEXTURE_PRESSURE]->getTexture());
	//}

	////c: compute the divergence free velocity
	//SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	////input
	//Velo

	//VelocityStep();
	//DensityStep();
	

	//advect velocity
	//SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_VELOCITY );

	//


	//////advect ink
	////SetAdvect( TEXTURE_DENSITY, TEXTURE_DENSITY );
	////ExecuteShaderRect( TEXTURE_DENSITY );

	//SetDivergence( TEXTURE_VELOCITY );
	//ExecuteShaderRect( TEXTURE_DIVERGENCE );


	////Diffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY ); //test with visc at 0 first
	//Divergence( TEXTURE_DIVERGENCE );
	////Jacobi( -timestep * timestep, .25f, TEXTURE_PRESSURE,
	////	TEXTURE_DIVERGENCE );

	////clear pressure each step
	
	//// Apply the first 3 operators in Equation 12.
	//u = advect(u);
	//u = diffuse(u);
	//u = addForces(u);
	//// Now apply the projection operator to the result.
	//p = computePressure(u);
	//u = subtractPressureGradient(u, p);


}