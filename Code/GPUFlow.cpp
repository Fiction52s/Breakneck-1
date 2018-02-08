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

	/*boundLeft.setSize( Vector2f( 1, height + 2 ) );
	boundLeft.setPosition( position.x - 1, position.y - 1 );

	boundRight.setSize( Vector2f( 1, height + 2 ) );
	boundRight.setPosition( position.x + 1, position.y - 1 );

	boundTop.setSize( Vector2f( width, 1 ) );
	boundTop.setPosition( position.x, position.y - 1 );

	boundBot.setSize( Vector2f( width, 1 ) );
	boundBot.setPosition( position.x, position.y + height + 1 );*/


	if (!shaders[SHADER_ADVECT].loadFromFile("Shader/advect.frag", sf::Shader::Fragment ) )
	{
		cout << "advect SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_BOUNDARY].loadFromFile("Shader/boundary.frag", sf::Shader::Fragment ) )
	{
		cout << "boundary SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_DIVERGENCE].loadFromFile("Shader/divergence.frag", sf::Shader::Fragment ) )
	{
		cout << "divergence SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_GRADIENT].loadFromFile("Shader/gradient.frag", sf::Shader::Fragment ) )
	{
		cout << "gradient SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_JACOBI].loadFromFile("Shader/jacobi.frag", sf::Shader::Fragment ) )
	{
		cout << "jacobi SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!shaders[SHADER_IMPULSE].loadFromFile("Shader/impulse.frag", sf::Shader::Fragment ) )
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

//void GPUFlow::ExecuteBounds()
//{
//
//}

void GPUFlow::SetImpulse( Textures tex )
{
	Vector2f playerCoords = Vector2f( player->position.x, player->position.y ) - Vector2f(position.x, position.y);
	//playerCoords.y = -playerCoords.y;
	
	textures[tex]->display();

	playerCoords.x /= width * 3;// * 2;
	playerCoords.y /= height * 3;// * 2;
	Vector2f vel = Vector2f( player->velocity.x, player->velocity.y );
	if( tex == TEXTURE_DENSITY )
	{
		vel = Vector2f( 128, 0 );
	//	playerCoords = Vector2f( .5, .5 );
	}
	//playerCoords.y = 1 - playerCoords.y;

	//if( playerCoords.x < 0 || playerCoords.y < 0 )
	//{
	//	//to take it out of play
	//	playerCoords.x = -1;
	//	playerCoords.y = -1;
	//}
	
	
	Shader &sh = shaders[SHADER_IMPULSE];
	sh.setUniform( "Resolution", Vector2f( width, height) );
	sh.setUniform( "playerCoords", playerCoords );
	sh.setUniform( "vel", vel );
	sh.setUniform( "tex", textures[tex]->getTexture() );
	currShader = &sh;
}

void GPUFlow::SetAdvect( Textures velTex, Textures quantityTex )
{
	Shader &sh = shaders[SHADER_ADVECT];

	
	

	//textures[velTex]->display();
	//textures[quantityTex]->display();
	//need to make sure the texture is displayed before doing this
	sh.setUniform( "u", textures[velTex]->getTexture() );
	sh.setUniform( "x", textures[quantityTex]->getTexture() );
	sh.setUniform( "rdx", 1.f / tileSize );
	sh.setUniform( "timestep", timestep );
	Vector2u blah = textures[quantityTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setUniform( "texSize", si );
	currShader = &sh;
}

void GPUFlow::SetJacobi( float alpha, float rBeta, Textures xTex, Textures bTex )
{
	//textures[xTex]->display();
	//textures[bTex]->display();
	Shader &sh = shaders[SHADER_JACOBI];
	sh.setUniform( "alpha", alpha );
	sh.setUniform( "rBeta", rBeta );
	sh.setUniform( "x", textures[xTex]->getTexture() );
	sh.setUniform( "b", textures[bTex]->getTexture() );

	Vector2u blah = textures[xTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setUniform( "texSize", si );

	currShader = &sh;
}

void GPUFlow::SetDivergence( Textures wTex )
{
	

	//textures[wTex]->display();
	Shader &sh = shaders[SHADER_DIVERGENCE];
	sh.setUniform( "halfrdx", .5f / tileSize );
	sh.setUniform( "w", textures[wTex]->getTexture() );

	Vector2u blah = textures[wTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setUniform( "texSize", si );

	currShader = &sh;
}

void GPUFlow::SetGradient( Textures wTex, Textures pTex )
{
	//textures[wTex]->display();
	//textures[pTex]->display();
	Shader &sh = shaders[SHADER_GRADIENT];
	sh.setUniform( "halfrdx", .5f / tileSize );
	sh.setUniform( "w", textures[wTex]->getTexture() );
	sh.setUniform( "p", textures[pTex]->getTexture() );

	Vector2u blah = textures[pTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setUniform( "texSize", si );

	currShader = &sh;
}

void GPUFlow::SetBoundary( Vector2f &offset, Textures xTex )
{
	Shader &sh = shaders[SHADER_BOUNDARY];
	sh.setUniform( "offset", offset );
	sh.setUniform( "scale", tileSize );
	sh.setUniform( "x", textures[xTex]->getTexture() );

	Vector2u blah = textures[xTex]->getSize();
	Vector2f si( blah.x, blah.y );
	sh.setUniform( "texSize", si );

	currShader = &sh;
}

void GPUFlow::SetDiffuse( Textures xTex, Textures bTex )
{	
	//textures[bTex]->display();
	//return;
	//if( visc > 0 )
	{
		
		float a = timestep * visc;//1;//(width * height * timestep * visc);//.001; 
		float stencilFactor = 1.0f / (4.0 + a);
		//float a = (width*height) * timestep * visc;
		//float stencilFactor = 1.0f / (4.0f * a );
		
		SetJacobi( a, stencilFactor, xTex, bTex );
		Shader &sh = shaders[SHADER_JACOBI];
		for( int i = 0; i < numJacobiSteps; ++i )
		//for( int i = 0; i < numJacobiSteps; ++i )
		{	
			ExecuteShaderRect( xTex );
			//textures[xTex]->display();
			sh.setUniform( "x", textures[xTex]->getTexture());
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
	sp.setTexture( textures[TEXTURE_DENSITY]->getTexture() );
	sp.setPosition( position.x, position.y );
	sp.setScale( 3, 3 );
	
	target->draw( sp );

	sp.setPosition( position.x + width * 3 + 5, position.y );
	sp.setTexture( textures[TEXTURE_VELOCITY]->getTexture() );

	target->draw( sp );

	sp.setPosition( position.x + (width * 3 + 5) * 2, position.y );
	sp.setTexture( textures[TEXTURE_DIVERGENCE]->getTexture() );

	target->draw( sp );

	sp.setPosition( position.x + (width * 3 + 5) * 3, position.y );
	sp.setTexture( textures[TEXTURE_PRESSURE]->getTexture() );

	target->draw( sp );
	//this is why i have to scale by three in the shader coords


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
		sh.setUniform( "x", textures[TEXTURE_PRESSURE]->getTexture());
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
		sh.setUniform( "x", textures[pTex]->getTexture());
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

Vector2f Normalize( Vector2f vec )
{
	float len = sqrt( vec.x * vec.x + vec.y * vec.y );
	vec.x /= len;
	vec.y /= len;
	return vec;
}

Vector2f Test( Color c )
{
	float r = c.r / 256.0;
	float g = c.g / 256.0;
	float b = c.b;// / 256.0;

	Vector2f test( r, g );
	cout << "teststart: " << test.x << ", " << test.y << ", mag: " << b << endl;
	Vector2f no = Normalize( test );
	no *= b;
	
	//cout << "test: " << test.x << ", " << test.y << endl;
	return no;
}

Color Test2( Vector2f vec )
{
	Color c;

	float len = sqrt( vec.x * vec.x + vec.y * vec.y );
	c.b = len;
	c.r = vec.x / len * 256;
	c.g = vec.y / len * 256;

	return c;
}

void GPUFlow::Update()
{
	//again, try to do the same thing as the cpu solver just for now

	//adding impulse
	SetImpulse( TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	SetImpulse( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );
	
	SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	

	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	SetDivergence( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_DIVERGENCE );

	textures[TEXTURE_PRESSURE]->clear( Color::Transparent );
	textures[TEXTURE_PRESSURE]->display();

	SetJacobi( -1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );
	Shader &sh = shaders[SHADER_JACOBI];

	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( TEXTURE_PRESSURE );
		sh.setUniform( "x", textures[TEXTURE_PRESSURE]->getTexture());
	}

	SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	ExecuteShaderRect( TEXTURE_VELOCITY );
	/*

	textures[TEXTURE_PRESSURE]->clear( Color::Transparent );
	textures[TEXTURE_PRESSURE]->display();

	SetJacobi( -1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );
	Shader &sh = shaders[SHADER_JACOBI];

	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( TEXTURE_PRESSURE );
		sh.setUniform( "x", textures[TEXTURE_PRESSURE]->getTexture());
	}

	SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	ExecuteShaderRect( TEXTURE_VELOCITY );*/

	//SetDiffuse( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	
	/*SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	SetAdvect( TEXTURE_VELOCITY, TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );



	SetDivergence( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_DIVERGENCE );


	textures[TEXTURE_PRESSURE]->clear( Color::Transparent );
	textures[TEXTURE_PRESSURE]->display();

	SetJacobi( -1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );
	Shader &sh = shaders[SHADER_JACOBI];

	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( TEXTURE_PRESSURE );
		sh.setUniform( "x", textures[TEXTURE_PRESSURE]->getTexture());
	}

	SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );
	ExecuteShaderRect( TEXTURE_VELOCITY );*/

	//Color c( 1, 1, 10 );
	//Vector2f t = Test( c );
	//cout << "test: " << t.x << ", " << t.y << endl;

	//Color back = Test2( t );
	//cout << "col: " << (int)back.r << ", " << (int)back.g << ", " << (int)back.b << endl;

	
	//advect ink
	/*SetAdvect( TEXTURE_VELOCITY, TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	SetImpulse( TEXTURE_DENSITY );
	ExecuteShaderRect( TEXTURE_DENSITY );

	SetImpulse( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_VELOCITY );

	SetDiffuse( TEXTURE_VELOCITY, TEXTURE_VELOCITY );

	SetDivergence( TEXTURE_VELOCITY );
	ExecuteShaderRect( TEXTURE_DIVERGENCE );

	textures[TEXTURE_PRESSURE]->clear( Color::Transparent );
	textures[TEXTURE_PRESSURE]->display();

	SetJacobi( -1, 0.25f, TEXTURE_PRESSURE,
		TEXTURE_DIVERGENCE );
	Shader &sh = shaders[SHADER_JACOBI];

	for( int i = 0; i < 20; ++i )
	{
		ExecuteShaderRect( TEXTURE_PRESSURE );
		sh.setUniform( "x", textures[TEXTURE_PRESSURE]->getTexture());
	}

	SetGradient( TEXTURE_VELOCITY, TEXTURE_PRESSURE );*/
	//ExecuteShaderRect( TEXTURE_VELOCITY );


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
	//	sh.setUniform( "x", textures[TEXTURE_PRESSURE]->getTexture());
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