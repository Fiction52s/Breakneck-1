#include "Zone.h"
#include <iostream>
#include "Enemy.h"
#include "GameSession.h"
#include "EditSession.h"
#include "Minimap.h"

#define TIMESTEP (1.0 / 60.0)

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
	secretZone = false;
	reexplored = false;
	parentZone = NULL;
	showShadow = true;
	tp.FixWinding();

	openFrames = 60;
	closeFrames = 60;

	TerrainPoint * curr;
	int tpNumP = tp.GetNumPoints();

	
	pointVector.push_back(vector<Vector2i>());
	vector<Vector2i> &mainPoly = pointVector[0];
	mainPoly.resize(tpNumP);
	for (int i = 0; i < tpNumP; ++i)
	{
		curr = tp.GetPoint(i);
		mainPoly[i] = curr->pos;
		//points.push_back(curr->pos);
	}

	activeNext = NULL;

	zShader = NULL;
	miniShader = NULL;

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

	if (miniShader != NULL)
	{
		delete miniShader;
		miniShader = NULL;
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


	int emergency = 200;
	int ecounter = 0;
	while( !relGates.empty() )
	{
		pointVector.push_back(vector<Vector2i>());
		vector<Vector2i> &currHoleVector = pointVector.back();

		Edge *start = relGates.front();

		relGates.pop_front();

		Edge *curr = start;
		currHoleVector.push_back(Vector2i(curr->v0));

		curr = curr->edge1;

		while( curr != start )
		{
			if( ecounter == emergency )
			{
				assert( 0 && "gates go on forever" );
			}

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
				currHoleVector.push_back(Vector2i(curr->v0));
				curr = curr->edge1;
			}
		}
	}

	std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(pointVector);
	
	int vaSize = indices.size();
	definedArea = new VertexArray( sf::Triangles , vaSize );
	VertexArray & v = *definedArea;

	int numTris = vaSize / 3;
	for( int i = 0; i < numTris; ++i )
	{	
		v[i * 3] = Vertex(Vector2f(GetPolyPoint(indices[i*3])));
		v[i * 3 + 1] = Vertex(Vector2f(GetPolyPoint(indices[i * 3+1])));
		v[i * 3 + 2] = Vertex(Vector2f(GetPolyPoint(indices[i * 3+2])));
	}

	switch (zType)
	{
	case MOMENTA:
	case SECRET:
		secretZone = true;
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

		Color shadowColor(0, 0, 0, 200);
		
		//SetShadowColor(shadowColor);
		zShader->setUniform("shadowColor", ColorGL(shadowColor));
		zShader->setUniform("alpha", 1.f);

		bool allSecret = true;
		Gate *tempGate;
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			tempGate = (Gate*)((*it)->info);
			if (tempGate->category != Gate::SECRET)
			{
				allSecret = false;
				break;
			}
		}

		if (allSecret)
		{
			secretZone = true;
		}
		

		
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

	if (secretZone)
	{
		if (miniShader != NULL)
			delete miniShader;

		miniShader = new sf::Shader;
		if (!miniShader->loadFromFile("Resources/Shader/normalzone.frag", sf::Shader::Fragment))
		{
			cout << "mini zone shader not loading correctly!" << endl;
			assert(false);
		}

		miniShader->setUniform("shadowColor", ColorGL(Minimap::terrainColor));
		miniShader->setUniform("alpha", 1.f);
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

Vector2i &Zone::GetPolyPoint(int index)
{
	int currVecIndex = 0;
	while (index >= pointVector[currVecIndex].size())
	{
		index -= pointVector[currVecIndex].size();
		++currVecIndex;
	}

	return pointVector[currVecIndex][index];
}

vector<sf::Vector2i> &Zone::PointVector()
{
	return pointVector[0];
}

void Zone::ReformDeadEnds()
{
	Gate *og;
	for (auto it = connectedSet.begin(); it != connectedSet.end(); ++it)
	{
		if ((*it)->hasGoal)
			continue;

		if ((*it)->connectedCount < 2)
		{
			for (auto git = gates.begin(); git != gates.end(); ++git)
			{
				og = (Gate*)(*git)->info;
				if (og->zoneA == (*it))
				{
					if ((og->gState == Gate::HARD
						|| og->gState == Gate::SOFT
						|| og->gState == Gate::SOFTEN))
					{
						og->Reform();
					}
				}
				else if (og->zoneB == (*it))
				{
					if ((og->gState == Gate::HARD
						|| og->gState == Gate::SOFT
						|| og->gState == Gate::SOFTEN))
					{
						og->Reform();
					}
				}
				else
				{
					//assert(0);
				}
			}
		}
	}
}

void Zone::DecrementNeighborsAttached( Zone *exclude )
{
	for (auto it = connectedSet.begin(); it != connectedSet.end(); ++it)
	{
		if (exclude == (*it) || (*it)->hasGoal )
			continue;
		(*it)->DecrementConnected(this);
	}
}

void Zone::DecrementConnected( Zone *z )
{
	auto findIt = connectedSet.find(z);
	if (findIt != connectedSet.end())
	{
		if (connectedCount > 1)
		{
			connectedCount--;
			if (connectedCount == 1)
			{
				ReformDeadEnds();
				DecrementNeighborsAttached(NULL);
			}
		}
			
		//connectedSet.erase(findIt);

	}
}

void Zone::SetAdjacentZoneIndexes()
{
	Gate *g;
	assert(goalIndex >= 0);

	int newGoalIndex = goalIndex + 1;
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		g = (Gate*)(*it)->info;
		if (g->zoneA == this)
		{
			if (g->zoneB->goalIndex < newGoalIndex)
			{
				g->zoneB->goalIndex = newGoalIndex;
			}
		}
		else if (g->zoneB == this)
		{
			if (g->zoneA->goalIndex < newGoalIndex)
			{
				g->zoneA->goalIndex = newGoalIndex;
			}
		}
		else
		{
			assert(0);
		}
	}

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		g = (Gate*)(*it)->info;
		if (g->zoneA == this)
		{
			if (g->zoneB->goalIndex > goalIndex)
			{
				higherIndexGates.push_back(g);
				g->zoneB->SetAdjacentZoneIndexes();
			}
		}
		else if (g->zoneB == this)
		{
			if (g->zoneA->goalIndex > goalIndex)
			{
				higherIndexGates.push_back(g);
				g->zoneA->SetAdjacentZoneIndexes();
			}
		}
		else
		{
			assert(0);
		}
		
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

void Zone::Reset()
{
	connectedCount = connectedSet.size();
	active = false;
	action = UNEXPLORED;
	frame = 0;
	reexplored = false;
	if( zShader != NULL )
		zShader->setUniform("alpha", 1.f);

	if (miniShader != NULL)
	{
		miniShader->setUniform("alpha", 1.f);
	}
}

void Zone::Update(float zoom, sf::Vector2f &botLeft, sf::Vector2f &playertest)
{
	VertexArray &va = *definedArea;
	if (zType == NORMAL || zType == MOMENTA || zType == SECRET )
	{
		switch (action)
		{
		case UNEXPLORED:
			break;
		case OPENING:
			if (frame == openFrames)
			{ 
				action = OPEN;
				frame = 0;
			}
			else
			{
				float a = GetOpeningAlpha();
				zShader->setUniform("alpha", a);

				if (secretZone)
				{
					miniShader->setUniform("alpha", a);
				}
			}
			break;
		case OPEN:
			break;
		case CLOSING:
			if (frame == closeFrames)
			{
				Close();
			}
			else
			{
				float a = 1.f - GetOpeningAlpha();
				zShader->setUniform("alpha", a);

				if (secretZone)
				{
					miniShader->setUniform("alpha", a);
				}
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
	return 1.f - frame / ((float)openFrames);
}

void Zone::Close()
{
	action = CLOSED;
	frame = 0;
	zShader->setUniform("alpha", 1.f);
}

void Zone::DrawMinimap(sf::RenderTarget *target)
{
	if (action != OPEN)//!active )
	{
		if (showShadow)
		{
			if (secretZone)
			{
				target->draw(*definedArea, miniShader);
			}
			//cout << "drawing area " << this << endl;
			//switch (zType)
			//{
			//case MOMENTA:
			//	target->draw(*definedArea, miniShader);
			//	break;
			//case NORMAL:
			//	target->draw(*definedArea, zShader); //target->draw(*definedArea);
			//	break;
			//case NEXUS:

			//	assert(zShader != NULL);
			//	target->draw(*definedArea, zShader);
			//	break;
			//}
		}
	}
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
			case MOMENTA:
			case SECRET:
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
	bool c = false;

	vector<Vector2i> &pVec = PointVector();

	int pCount = pVec.size();
	
	V2d curr,prev;
	for (int i = 0; i < pCount; ++i)
	{
		curr = V2d(pVec[i]);
		if (i == 0)
			prev = V2d(pVec[pCount - 1]);
		else
			prev = V2d(pVec[i - 1]);

		if (((curr.y > test.y) != (prev.y > test.y)) &&
			(test.x < (prev.x - curr.x) * (test.y - curr.y) / (prev.y - curr.y) + curr.x))
			c = !c;
	}

	return c;
}

bool Zone::ContainsZone(Zone *z)
{
	//midpoint on the gate

	vector<Vector2i> &pVec = z->PointVector();
	for (auto it = pVec.begin(); it != pVec.end(); ++it)
	{
		if (!ContainsPoint(V2d((*it))))
		{
			return false;
		}
	}
	return true;
}

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