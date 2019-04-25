#include "Zone.h"
#include "poly2tri/poly2tri.h"
#include <iostream>
#include "Enemy.h"
#include "GameSession.h"

#define TIMESTEP 1.0 / 60.0

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

using namespace sf;
using namespace std;

Zone::Zone( GameSession *p_owner, TerrainPolygon &tp )
	:active( false ), owner( p_owner )
{
	parentZone = NULL;
	requiredKeys = 0;
	showShadow = true;
	tp.FixWinding();

	TerrainPoint * curr = tp.pointStart;

	while( curr != NULL )
	{
		points.push_back( curr->pos );
		curr = curr->next;
	}

	activeNext = NULL;

	zShader = NULL;

	zType = NORMAL;

	ts_z = NULL;

	Reset();
}

Zone::~Zone()
{
	delete definedArea;
	if (zShader != NULL)
	{
		delete zShader;
		zShader = NULL;
	}
}

void Zone::SetZoneType( ZoneType zt )
{
	zType = zt;
}

void Zone::ReformAllGates( Gate *ignoreGate)
{
	for (list<Edge*>::iterator it = gates.begin(); it !=
		gates.end(); ++it)
	{
		Gate *og = (Gate*)(*it)->info;
		if (ignoreGate == og)
			continue;
		if ((og->gState == Gate::HARD
			|| og->gState == Gate::SOFT
			|| og->gState == Gate::SOFTEN))
		{
			og->gState = Gate::REFORM;
			og->frame = 0;
			float aa = .5;
			og->centerShader.setUniform("breakPosQuant", aa);
		}
	}
}

void Zone::Init()
{
	p2t::Point *testP = new p2t::Point(0, 0);
	delete testP;

	vector<p2t::Point*> polyline;

	//int numPoints = 0;

	for (list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it)
	{
		polyline.push_back(new p2t::Point((*it).x, (*it).y));
	}

	p2t::CDT cdt(polyline);

	//cout << "adding holes for my: " << subZones.size() << " subzones" << endl;
	list<Zone*> possibleSubs = subZones;

	list<list<Zone*>> groupedZones;

	bool visibleZone = false;
	if (gates.empty() || action == OPEN )
	{
		visibleZone = true;
	}
	else
	{
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			Gate *g = (Gate*)(*it)->info;
			if (g->visible)
			{
				visibleZone = true;
			}
		}
	}

	if (!visibleZone)
	{
		action = OPEN;
	}
	

	list<Edge*> relGates;
	
	//cout << "subZones: " << subZones.size() << endl;
	for( list<Zone*>::iterator it = subZones.begin(); it != subZones.end(); ++it )
	{
		for( list<Edge*>::iterator eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit )
		{
			Gate *g = (Gate*)(*eit)->info;

			/*if( g->zoneA == g->zoneB )//g->zoneB )
			{
				cout << "--------------------------------------------------------------" << endl;
				//cout << "UNLOCKING BLAH" << endl;
				//g->SetLocked( false );
				continue;
			//	skip = true;
			//	curr = prev->edge1;
			}
			else
			{
				cout << "za: " << g->zoneA << ", zb: " << g->zoneB << endl;
			}*/


			if( g->zoneA == this )
			{
				relGates.push_back( g->edgeA );
			}
			else if( g->zoneB == this )
			{
				relGates.push_back( g->edgeB );
			}
		}
	}
	//cout << "relgates: " << relGates.size() << endl;


	int emergency = 200;
	int ecounter = 0;
	list<p2t::Point*> allHolePoints;
	while( !relGates.empty() )
	{
		
		vector<p2t::Point*> holePolyline;

		Edge *start = relGates.front();

		relGates.pop_front();

		Edge *curr = start;
		p2t::Point *p = new p2t::Point(curr->v0.x, curr->v0.y );
		holePolyline.push_back( p );
		allHolePoints.push_back( p );

		curr = curr->edge1;

		while( curr != start )
		{
			if( ecounter == emergency )
			{
				assert( 0 && "gates go on forever" );
			}
			//ecounter++;

			bool skip = false;
			bool found = false;
			//if edge is gate type
			if( curr->edgeType == Edge::CLOSED_GATE || curr->edgeType == Edge::OPEN_GATE )
			{
				for( list<Edge*>::iterator it = relGates.begin(); it != relGates.end(); )
				{
					if( curr == (*it) )
					{
						found = true;
						//Edge *prev = curr->edge0;
						Gate *g = (Gate*)curr->info;
					
						/*if( g->zoneA == g->zoneB )//g->zoneB )
						{
							cout << "--------------------------------------------------------------" << endl;
							cout << "UNLOCKING BLAH" << endl;
							g->SetLocked( false );
							skip = true;
							curr = prev->edge1;
						}
						else
						{
							cout << "za: " << g->zoneA << ", zb: " << g->zoneB << endl;
						}*/
					
						it = relGates.erase( it );
						break;
					}
					else
					{
						++it;
					}
				}
				//this fixes a loose gate so that the correct shadow polygon can be created.
				//if( false && !found )
				if( !found )
				{
					//cout << "fixing loose gate" << endl;

					Gate *g = (Gate*)curr->info;
					Edge *prev = curr->edge0;

					g->SetLocked( false );
					curr = prev->edge1;
					g->SetLocked( true );
					skip = true;
					//it = relGates.erase( it );
					//cout << "gza: " << g->zoneA << ", gzb: " << g->zoneB << endl;
					//break;
					
				}
			}
			

			if( !skip )
			{
				p2t::Point *p = new p2t::Point(curr->v0.x, curr->v0.y );
				holePolyline.push_back( p );
				allHolePoints.push_back( p );

				curr = curr->edge1;
			}
		}

		cdt.AddHole( holePolyline );
	}
	
	
	cdt.Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt.GetTriangles();
	
	int vaSize = tris.size() * 3;
	definedArea = new VertexArray( sf::Triangles , vaSize );
	VertexArray & v = *definedArea;
	
	//Color testColor( 0x75, 0x70, 0x90 );
	//Color selectCol( 0x77, 0xBB, 0xDD );

	for( int i = 0; i < tris.size(); ++i )
	{	
		p2t::Point *p = tris[i]->GetPoint( 0 );	
		p2t::Point *p1 = tris[i]->GetPoint( 1 );	
		p2t::Point *p2 = tris[i]->GetPoint( 2 );	
		v[i * 3] = Vertex(Vector2f(p->x, p->y));// , shadowColor );
		v[i * 3 + 1] = Vertex(Vector2f(p1->x, p1->y));// , shadowColor );
		v[i * 3 + 2] = Vertex(Vector2f(p2->x, p2->y));// , shadowColor );
	}

	switch (zType)
	{
	case NORMAL:
	{
		if (zShader != NULL)
		{
			delete zShader;
		}
		
		zShader = new sf::Shader;
		if (!zShader->loadFromFile("Resources/Shader/normalzone.frag", sf::Shader::Fragment))
		{
			cout << "zone shader not loading correctly!" << endl;
			assert(false);
		}
		//ts_z = owner->GetTileset("terrain_1_01_512x512.png", 512, 512);
		//zShader->setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));
		//zShader->setUniform("Resolution", Vector2f(1920, 1080));
		//zShader->setUniform("u_texture", ts_z->texture);
		

		//Color shadowColor(50, 50, 50, 200);
		Color shadowColor(0, 0, 0, 200);
		
		SetShadowColor(shadowColor);
		zShader->setUniform("shadowColor", ColorGL(shadowColor));
		zShader->setUniform("alpha", 1.f);
		//SetShadowColor(Color::White);
		break;
	}
	case NEXUS:
	{
		if (zShader == NULL)
		{
			zShader = new sf::Shader;
			if (!zShader->loadFromFile("Resources/Shader/mat_shader2.frag", sf::Shader::Fragment))
			{
				cout << "zone shader not loading correctly!" << endl;
				assert(false);
			}
			ts_z = owner->GetTileset("terrain_1_01_512x512.png", 512, 512);
			zShader->setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));
			zShader->setUniform("Resolution", Vector2f(1920, 1080));
			zShader->setUniform("u_texture", ts_z->texture);


			SetShadowColor(Color::White);
			//ts_polyShaders[index] = GetTileset(ss1.str(), 512, 512); //1024, 1024 );
			//polyShaders[index].setUniform("u_texture",*GetTileset(ss1.str(), 512, 512)->texture);
			//polyShaders[index].setUniform("Resolution", Vector2f(1920, 1080));
			//polyShaders[index].setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));

			//if( !zShader->loadFromFile( "Shader/terrain"))
		}
		break;
	}
	}
	//Color shadowColor(50, 50, 50, 200);
	//SetShadowColor(shadowColor);

	//assert( tris.size() * 3 == points.size() );
	//delete cdt;
	//for( int i = 0; i < numPoints; ++i )
	for( auto it = polyline.begin(); it != polyline.end(); ++it )
	{
		delete (*it);
	//	delete tris[i];
	}
	
	for( list<p2t::Point*>::iterator it = allHolePoints.begin(); it != allHolePoints.end(); ++it )
	{
		delete (*it);
	}

	for( list<Enemy*>::iterator it = allEnemies.begin(); it != allEnemies.end(); ++it )
	{
		if ((*it)->type == EN_CRAWLERQUEEN)
		{
			action = OPEN;
		}
		//if( (*it)->type == EN_NEXUS|| (*it)->type == EN_CRAWLERQUEEN )
		//{
		//	active = true;
		//	//owner->ActivateZone( this );
		//	break;
		//}
	}

	
}

void Zone::SetShadowColor( sf::Color c )
{
	int vCount = definedArea->getVertexCount();
	VertexArray &da = *definedArea;
	for( int i = 0; i < vCount; ++i )
	{
		da[i].color = c;
	}
}

void Zone::AddHoles( p2t::CDT *cdt )
{
	//vector<p2t::Point*> polyline;

	for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
	{
	//	polyline.push_back( new p2t::Point((*it).x, (*it).y ) );
	}
	//cout << "adding hole!" << endl;
	//cdt->AddHole( polyline );

	for( list<Zone*>::iterator it = subZones.begin(); it != subZones.end(); ++it )
	{
		(*it)->AddHoles( cdt );
	}
}

void Zone::Reset()
{
	active = false;
	action = UNEXPLORED;
	frame = 0;
	if( zShader != NULL )
		zShader->setUniform("alpha", 1.f);
}

void Zone::Update( float zoom, sf::Vector2f &botLeft, sf::Vector2f &playertest )
{
	VertexArray &va = *definedArea;
	if (zType == NORMAL)
	{
		switch (action)
		{
		case UNEXPLORED:
			break;
		case OPENING:
			if (frame == 60)
			{ 
				action = OPEN;
				frame = 0;
			}
			else
			{
				zShader->setUniform("alpha", GetOpeningAlpha());
			}
			break;
		case OPEN:
			break;
		case CLOSING:
			if (frame == 60)
			{
				Close();
			}
			else
			{
				zShader->setUniform("alpha", 1.f - GetOpeningAlpha());
			}
			break;
		case CLOSED:
			break;
		}
	}


switch (zType)
{
case NORMAL:

	break;
case NEXUS:
	zShader->setUniform("zoom", zoom);
	zShader->setUniform("topLeft", botLeft); //just need to change the name topleft eventually
	zShader->setUniform("playertest", playertest);
	break;
}
++frame;
}

float Zone::GetOpeningAlpha()
{
	return 1.f - frame / 60.f;
}

void Zone::Close()
{
	action = CLOSED;
	frame = 0;
	zShader->setUniform("alpha", 1.f);
}

void Zone::Draw(RenderTarget *target)
{
	//target->draw( *definedArea );
	//return;
	if (action != OPEN)//!active )
	{
		if (showShadow)
		{
			//cout << "drawing area " << this << endl;
			switch (zType)
			{
			case NORMAL:
				target->draw(*definedArea, zShader); //target->draw(*definedArea);
				break;
			case NEXUS:

				assert(zShader != NULL);
				target->draw(*definedArea, zShader);
				break;
			}
		}
	}
}

bool Zone::ContainsPoint(V2d test)
{
	int pointCount = points.size();

	bool c = false;

	Vector2i prev = points.back();
	for (list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it)
	{
		if ((((*it).y > test.y) != (prev.y > test.y)) &&
			(test.x < (prev.x - (*it).x) * (test.y - (*it).y) / (prev.y - (*it).y) + (*it).x))
			c = !c;

		prev = (*it);
	}

	return c;
}

bool Zone::ContainsZone(Zone *z)
{
	//midpoint on the gate
	for (list<Vector2i>::iterator it = z->points.begin(); it != z->points.end(); ++it)
	{
		if (!ContainsPoint(V2d((*it).x, (*it).y)))
		{
			return false;
		}
	}
	return true;

	/*for( list<Edge*>::iterator it = z->gates.begin(); it != z->gates.end(); ++it )
	{
	}


	z->
	V2d p(
		( z->gates.front()->edge0->v0.x + z->gates.front()->edge1->v1.x ) / 2,
		( z->gates.front()->edge0->v0.y + z->gates.front()->edge0->v1.y ) / 2);
	return ContainsPoint( p );*/
}

//bool Zone::ContainsPlayer()
//{
//
//}

bool Zone::ContainsZoneMostSpecific(Zone *z)
{
	if (ContainsZone(z))
	{
		for (auto it = subZones.begin(); it != subZones.end(); ++it)
		{
			if ((*it)->ContainsZone( z ) )
			{
				return false;
			}
		}

		return true;
	}
	return false;
}

Zone* Zone::ContainsPointMostSpecific( sf::Vector2i test )
{
	//bool contains = ContainsPoint( test );
	bool contains = false;
	if( contains )
	{
		if( subZones.empty() )
		{
			return this;
		}
		else
		{
			for( list<Zone*>::iterator it = subZones.begin(); it != subZones.end(); ++it )
			{
				Zone * z = ContainsPointMostSpecific( test );
				if( z != NULL )
				{
					return z;
				}
			}

			return this;
		}
	}
	else
	{
		return NULL;
	}
}