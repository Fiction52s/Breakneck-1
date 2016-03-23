#include "flow.h"
#include <iostream>
#include "Actor.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define SWAP( x0, x ) { float * tmp = x0; x0 = x; x = tmp; }

Flow::Flow( const Vector2i &p, int w, int h )
	:width( w ), height( h )
{
	numPoissonSteps = 50;
	gridScale = 1;
	timeStep = 1 / 60.f;
	viscosity = 0.9f;
	inkLongevity = .9995f;
	pressureClearEachStep = true;
	inkColor = Color::Red;
	pos = p;
	dt = 1.f / 60.f;
	diff = .001f;//0.00075f;

	size = (width +2 ) * (height+2);

	m_u = new float[size];
	m_v = new float[size];
	m_u_prev = new float[size];
	m_v_prev = new float[size];
	m_dens = new float[size];
	m_dens_prev = new float[size];
	ink_dens = new float[size];

	for( int i = 0; i < size; ++i )
	{
		m_u[i] = 0;
		m_v[i] = 0;
		m_u_prev[i] = 0;
		m_v_prev[i] = 0;
		m_dens[i] = 0;
		m_dens_prev[i] = 0;
		ink_dens[i] = (rand() % 100) / 100.f;
	}

	//m_dens[IX(10,10)] = 100;

	//m_dens[20 * 20 / 2] = 1;
	va = new VertexArray( sf::Quads, size * 4 );

	velLines = new VertexArray( sf::Lines, size * 2 );

	//m_dens_prev[5] = 1;


	//zeros = new float[ width * height * 4 ];
	//memset( zeros, 0, width * height * 4 * sizeof(float)); //set all 0s

	//box = new VertexArray( sf::Quads, 4 ); //1 quad
	//borders = new VertexArray( sf::Quads, 4 * 4 ); //4 lines
	
	//offscreenBuffer = new RenderTexture;
	//offscreenBuffer->create( w, h, false );
	//offscreenBuffer->clear();


	//textures = new Texture*[TEXTURE_COUNT];
	
	/*for( int i = 0; i < TEXTURE_COUNT; ++i )
	{
		textures[i] = new Texture;
		textures[i]->create( width, height );
		textures[i]->loadFromMemory( zeros, sizeof(float) );
	}*/

	//{
	//	textures[i].
	//	//textures[i].create( w, h, false );
	//	//textures[i].clear(Color::Blue);
	//}

	//AddSource( m_dens_prev, ink_dens );
}

int Flow::IX( int i, int j )
{
	return ( i + (width + 2) * j );
}

Flow::~Flow()
{
	//delete [] zeros;
}

//s = sources, 
//add source field s to field x
void Flow::AddSource( float *x, float *s )
{
	for( int i = 0; i < size; ++i )
	{
		x[i] += dt * s[i];
	}
}

//b == 0: all edges are continuous
//b == 1: left/right reflect, others are continuous
//b == 2: top/bottom reflect, others are continous
void Flow::SetBound( int b, float *x )
{
	int i;
	for( i = 1; i <= height; ++i )
	{
		x[IX(0,i)] =		( b == 1 ? -x[IX(1,i)] : x[IX(1,i)] ); //left side  //if b == 1 then (0,i) has -
		x[IX(width+1,i)] =  ( b == 1 ? -x[IX(width,i)] : x[IX(width,i)] ); //right side
		x[IX(i,0)] =		( b == 2 ? -x[IX(i,1)] : x[IX(i,1)] ); //top
		x[IX(i,height+1)] =	( b == 2 ? -x[IX(i,height)] : x[IX(i,height)] ); //bottom
	}

	//corners average their 2 neighbors into their value
	x[IX(0,0)] = 0.5 * (x[IX(1,0)]+x[IX(0,1)]);
	x[IX(0,height+1)] = 0.5 * (x[IX(1,height+1)] + x[IX(0,height)]);
	x[IX(width+1,0)] = 0.5 * (x[IX(width,0)] + x[IX(width+1,1)]);
	x[IX(width+1,height+1)] = 0.5 * (x[IX(width, height+1)] + x[IX(width+1,height)]);
}

void Flow::Advect( int b, float *d, float *d0, float *u, float *v )
{
	int i,j,i0,j0,i1,j1;
	float x,y,s0,t0,s1,t1,dt0;

	//dt0 = dt * N;
	dt0 = dt * width;
	for( i = 1; i <= width; ++i )
	{
		for( j = 1; j <= height; ++j )
		{
			x = i - dt0 * u[IX(i,j)];
			y = j - dt0 * v[IX(i,j)]; //this is maybe where i dt1?
			if( x < 0.5f ) 
				x = 0.5f; 
			if( x > width + 0.5f ) 
				x = width + 0.5f;

			i0 = (int)x;
			i1 = (i0 + 1);

			if( y < 0.5 ) 
				y = 0.5; 
			if( y > height + 0.5 ) 
				y = height + 0.5;

			j0 = (int)y;
			j1 = j0 + 1;

			s1 = x - i0;
			s0 = 1 - s1;

			t1 = y - j0;
			t0 = 1 - t1;

			d[IX(i,j)] = s0 * ( t0 * d0[IX(i0,j0)] + t1 * d0[IX(i0,j1)] )
				+ s1 * ( t0 * d0[IX(i1,j0)] + t1 * d0[IX(i1,j1)] );
		}
	}

	SetBound( b, d );
}

void Flow::DensityStep( float *x, float *x0, float *u, float *v )
{
	AddSource( x, x0 );
	SWAP( x0, x ); 
	Diffuse( 0, x, x0 );
	SWAP( x0, x );
	Advect( 0, x, x0, u, v );
}

void Flow::Diffuse( int b, float *x, float *x0 )
{
	float a = dt * diff * width * height;
	
	LinSolve( b, x, x0, a, ( 1 + 4 * a ) );
}

void Flow::VelocityStep( float *u, float *v, float *u0, float *v0 )
{
	AddSource( u, u0 ); AddSource( v, v0 );
	SWAP( u0, u ); Diffuse( 1, u, u0 );
	SWAP( v0, v ); Diffuse( 2, v, v0 );
	Project( u, v, u0, v0 );
	SWAP( u0, u ); SWAP( v0, v );
	Advect( 1, u, u0, u0, v0 ); Advect( 2, v, v0, u0, v0 );
	Project( u, v, u0, v0 );
}

void Flow::Project( float *u, float *v, float *p, float *div )
{
	int i,j,k;
	float h;

	//h = 1.0 / N;
	h = 1.f / width;

	for( i = 1; i <= width; ++i )
	{
		for( j = 1; j <= height; ++j )
		{
			div[IX(i,j)] = -0.5 * h * ( u[IX(i+1,j)] - u[IX(i-1,j)] +
				v[IX(i,j+1)] - v[IX(i,j-1)] );
			p[IX(i,j)] = 0;
		}
	}

	SetBound( 0, div ); SetBound( 0, p );

	LinSolve( 0, p, div, 1, 4 );

	for( i = 1; i <= width; ++i )
	{
		for( j = 1; j <= height; ++j )
		{
			u[IX(i,j)] -= 0.5 * ( p[IX(i+1,j)] - p[IX(i-1,j)] ) / h;
			v[IX(i,j)] -= 0.5 * ( p[IX(i,j+1)] - p[IX(i,j-1)] ) / h;
		}
	}

	SetBound( 1, u ); SetBound( 2, v );
}

void Flow::CreateTextures()
{
}

void Flow::Reset()
{
	for ( int i=0 ; i< size ; i++ ) 
	{
		m_u_prev[i] = m_v_prev[i] = m_dens_prev[i] = m_u[i] = m_v[i] = m_dens[i] = 0.0f;
	}
}

void Flow::Update()
{
	for ( int i=0 ; i< size ; i++ ) 
	{
		m_dens_prev[i] = 0.0f;
		//m_u_prev[i] = 0.0f;
		//m_v_prev[i] = 0.0f;
		//m_v_prev[i] = .01f;
		//m_u_prev[i] = m_v_prev[i] = m_dens_prev[i] = 0.0f;
	//	m_dens_prev[i] = 0.0f;
	}

	//m_dens[IX(10,10)] = 100;

	float h = 1.0f / width;
	int tileSize = 16;
	float force = 1;
	int gx = (((int)player->position.x) - pos.x ) / tileSize;
	int gy = (((int)player->position.y) - pos.y ) / tileSize;

	m_dens_prev[IX((width+2)/2,(width+2)/2)] = 30;
	if( gx >= 0 && gx <= width + 1 && gy >= 0 && gy <= height + 1 )
	{
		sf::Vector2<double> vel = player->velocity;
		if( player->ground != NULL )
		{
			vel = normalize(player->ground->v1 - player->ground->v0 ) * player->groundSpeed;
		}
		double mult = 20;
		//cout << "influencing: " << gx << ", " << gy << endl;
		m_u_prev[IX(gx,gy)] = vel.x * mult;//vel.x * mult;//100;// * gx;
		m_v_prev[IX(gx,gy)] = vel.y * mult;//100;//-vel.y;//100;//vel.y * mult;// * gy;
		//m_dens_prev[IX(gx,gy)] = 30;
		//m_dens[IX(gx,gy)] = 1;
		//cout << "before: " << m_dens[IX(gx,gy)] << endl;
		//m_dens[IX(gx,gy)] = .8;
		//m_u_prev[IX(gx,gy)] += 1;
		//m_v_prev[IX(gx,gy)] += 1;
		
	}


	//m_dens_prev[10] = 1;
	//AddSource
	
	VelocityStep( m_u, m_v, m_u_prev, m_v_prev );
	DensityStep( m_dens, m_dens_prev, m_u, m_v );

	VertexArray &v = *va;
	VertexArray &vl = *velLines;
	int row,col;
	
	int index = 0;
	for( int i = 0; i <= width; ++i )
	{
		for( int j = 0; j <= height; ++j )
		{
			Vector2f indexPos( i * tileSize, j * tileSize );

			v[index*4+0].position = indexPos + Vector2f( pos.x, pos.y );
			v[index*4+1].position = indexPos + Vector2f( pos.x + tileSize, pos.y );
			v[index*4+2].position = indexPos + Vector2f( pos.x + tileSize, pos.y + tileSize );
			v[index*4+3].position = indexPos + Vector2f( pos.x, pos.y + tileSize );

			float d0 = m_dens[IX(i,j)];
			float d1 = m_dens[IX(i,j+1)];
			float d2 = m_dens[IX(i+1,j)];
			float d3 = m_dens[IX(i+1,j+1)];

			/*assert( d0 <= 1 );
			assert( d1 <= 1 );
			assert( d2 <= 1 );
			assert( d3 <= 1 );*/

			d0 *= 255;
			d1 *= 255;
			d2 *= 255;
			d3 *= 255;

			d0 = std::min( 255.f, d0 );
			d1 = std::min( 255.f, d1 );
			d2 = std::min( 255.f, d2 );
			d3 = std::min( 255.f, d3 );

			float fac = 2;
			float a0 = std::min( 255.f, d0 * fac );
			float a1 = std::min( 255.f, d1 * fac );
			float a2 = std::min( 255.f, d2 * fac );
			float a3 = std::min( 255.f, d3 * fac );

			v[index*4+0].color = Color( d0, 0, 0, a0 ); //Color( d0, d0, d0);//Color( 255, 0,0, 255 * m_dens[i]);
			v[index*4+1].color = Color( d2, 0, 0, a2 );//Color( d2, d2, d2);//Color( 255, 0,0, 255 * m_dens[i]);
			v[index*4+2].color = Color( d3, 0, 0, a3 );//Color( d3, d3, d3);//Color( 255, 0,0, 255 * m_dens[i]);
			v[index*4+3].color = Color( d1, 0, 0, a1 );//Color( d1, d1, d1);//Color( 255, 0,0, 255 * m_dens[i]);

			++index;
		}
	}
	//for( int i = 0; i < size; ++i )
	//{
	//	col = i / (width+2);
	//	row = i % (width+2);

	//	Vector2f indexPos( row * tileSize, col * tileSize );

	//	v[i*4+0].position = indexPos + Vector2f( pos.x, pos.y );
	//	v[i*4+1].position = indexPos + Vector2f( pos.x + tileSize, pos.y );
	//	v[i*4+2].position = indexPos + Vector2f( pos.x + tileSize, pos.y + tileSize );
	//	v[i*4+3].position = indexPos + Vector2f( pos.x, pos.y + tileSize );

	//	float d0 = m_dens[IX(row,col)];
	//	float d1 = m_dens[IX(row,col+1)];
	//	float d2 = m_dens[IX(row+1,col)];
	//	float d3 = m_dens[IX(row+1,col+1)];

	//	v[i*4+0].color = Color( d0 * 255, d0 * 255, d0 * 255 );//Color( 255, 0,0, 255 * m_dens[i]);
	//	v[i*4+1].color = Color( d2 * 255, d2 * 255, d2 * 255 );//Color( 255, 0,0, 255 * m_dens[i]);
	//	v[i*4+2].color = Color( d3 * 255, d3 * 255, d3 * 255 );//Color( 255, 0,0, 255 * m_dens[i]);
	//	v[i*4+3].color = Color( d1 * 255, d1 * 255, d1 * 255 );//Color( 255, 0,0, 255 * m_dens[i]);




	//	vl[i*2].position = indexPos;
	//	vl[i*2+1].position = Vector2f( indexPos.x + m_u[i] * width,indexPos.y + m_v[i] * width);
	//		
	//	vl[i*2].color = Color::Green;
	//	vl[i*2+1].color = Color::Green;
	//}

	//prepared to render to the offscreen buffer

	//boundaries.compute()

	//advect.compute()

	//boundaries.compute()

	//advect.compute()

	//addimpulse.compute()

	//addimpulse.compute()

	//for each poisson step
		//poissonsolver.compute()

	//divergence.compute()

	//for each poisson step
		//boundaries.compute()
		//poissonsolver.compute()

	//boundaries.compute()

	//subtractGradient.compute()

	//drawing is done!!
}

//gauss seidel linear solver because it uses the values of x as it computes them instead of waiting 
//to update between iterations like a jacobi solver
void Flow::LinSolve( int b, float *x, float *x0, float a, float c )
{
	int i,j,k;
	for( k = 0; k < 20; ++k )
	{
		for( i = 1; i <= width; ++i )
		{
			for( j = 1; j <= height; ++j )
			{
				x[IX(i,j)] = (x0[IX(i,j)] 
				+ a * ( 
					x[IX(i-1,j)] 
				  + x[IX(i+1,j)]
				  + x[IX(i,j-1)]
				  + x[IX(i,j+1)]
				  ) ) / c;
			}
		}
		SetBound( b, x );
	}
}

void Flow::Draw( sf::RenderTarget *target )
{
	target->draw( *va );
	//target->draw( *velLines );
}