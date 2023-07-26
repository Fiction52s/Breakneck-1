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

ZoneNode::~ZoneNode()
{
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		delete (*it);
	}
}

bool ZoneNode::SetZone(Zone *p_myZone)
{
	Session *sess = Session::GetSession();

	myZone = p_myZone;

	if (myZone == sess->zoneTreeEnd)
	{
		return true;
	}


	children.reserve(myZone->connectedSet.size());
	for (auto it = myZone->connectedSet.begin(); it != myZone->connectedSet.end(); ++it)
	{
		if (!IsInMyBranch((*it)))
		{
			ZoneNode *zn = new ZoneNode;
			zn->parent = this;
			if (zn->SetZone((*it)))
			{
				children.push_back(zn);
			}
			else
			{
				delete zn;
			}
		}
	}

	if (children.size() == 0)
		return false;

	/*if( parent != NULL )
	myZone->leadOutZones.insert(parent->myZone);

	for (auto it = children.begin(); it != children.end(); ++it)
	{
	myZone->leadInZones.insert((*it)->myZone);
	}*/

	return true;
}


bool ZoneNode::IsInMyBranch(Zone *z)
{
	ZoneNode *p = parent;
	while (p != NULL)
	{
		if (p->myZone == z)
		{
			return true;
		}

		p = p->parent;
	}
	return false;
}

void ZoneNode::SetChildrenShouldNotReform()
{
	myZone->SetShouldReform(false);
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		(*it)->SetChildrenShouldNotReform();
	}
}


Zone::Zone( TerrainPolygon *tp )
{
	zonePoly = tp;
	assert(!zonePoly->finalized);
	zonePoly->FinalizeJustEdges();

	zoneIndex = -2;
	secretZone = false;
	parentZone = NULL;
	showShadow = true;
	zonePoly->FixWinding();

	openFrames = 60;
	closeFrames = 60;

	TerrainPoint * curr;
	int tpNumP = zonePoly->GetNumPoints();

	definedArea = NULL;

	
	pointVector.push_back(vector<Vector2i>());
	vector<Vector2i> &mainPoly = pointVector[0];
	mainPoly.resize(tpNumP);
	for (int i = 0; i < tpNumP; ++i)
	{
		curr = zonePoly->GetPoint(i);
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
	delete zonePoly;

	if (definedArea != NULL)
	{
		delete definedArea;
		definedArea = NULL;
	}

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

void Zone::SetZoneType( int zt )
{
	zType = zt;
}

void Zone::ReformAllGates( Gate *ignoreGate)
{
	//note: for secret zones, this should put it into a special reform
	//where it is closed forever, not the temporary reform that it gets
	//from being a 2 way gate.
	for (list<Edge*>::iterator it = gates.begin(); it !=
		gates.end(); ++it)
	{
		Gate *og = (Gate*)(*it)->info;
		if (ignoreGate == og)
			continue;
		if (og->IsReformable())
		{
			og->Reform();
		}
	}
}

void Zone::Init()
{
	list<Zone*> possibleSubs = subZones;

	list<list<Zone*>> groupedZones;

	Session *sess = Session::GetSession();

	bool visibleZone = false;
	if (gates.empty() || data.action == OPEN )
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
		data.action = OPEN;
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

	for (list<Zone*>::iterator it = subZones.begin(); it != subZones.end(); ++it)
	{
		if ((*it)->parentZone != this)
		{
			continue;
		}

		pointVector.push_back(vector<Vector2i>());
		vector<Vector2i> &currHoleVector = pointVector.back();

		int subPoints = (*it)->PointVector().size();
		for (int i = 0; i < subPoints; ++i)
		{
			currHoleVector.push_back((*it)->GetPolyPoint(i));
		}
	}



	int emergency = 200;
	int ecounter = 0;
	if( false)
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
			if( curr->IsGateEdge() )
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
			ts_z = sess->GetTileset("terrain_1_01_512x512.png", 512, 512);
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

	/*totalNumKeys = 0;
	for (auto it = allEnemies.begin(); it != allEnemies.end(); ++it)
	{
		if ((*it)->hasMonitor)
		{
			totalNumKeys++;
		}
	}*/

	

	//for( list<Enemy*>::iterator it = allEnemies.begin(); it != allEnemies.end(); ++it )
	//{
	//	if ((*it)->type == EN_CRAWLERQUEEN)
	//	{
	//		action = OPEN;
	//	}
	//	//if( (*it)->type == EN_NEXUS|| (*it)->type == EN_CRAWLERQUEEN )
	//	//{
	//	//	active = true;
	//	//	//owner->ActivateZone( this );
	//	//	break;
	//	//}
	//}

	
}

bool Zone::HasEnemyGate()
{
	Gate *g;
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		g = (Gate*)((*it)->info);
		if (!g->IsLockedForever())
		{
			if (g->category == Gate::ENEMY)
			{
				return true;
			}
		}
	}

	return false;
}

bool Zone::HasKeyGateOfNumber(int n)
{
	Gate *g;
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		g = (Gate*)((*it)->info);
		if (!g->IsLockedForever() )
		{
			if (g->category == Gate::ALLKEY && n == totalNumKeys )
			{
				return true;
			}
			else if (g->category == Gate::NUMBER_KEY && n == g->numToOpen)
			{
				return true;
			}
		}
	}


	return false;
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
	data.visited = false;
	data.active = false;
	data.action = UNEXPLORED;
	data.framesSinceActivation = 0;
	data.frame = 0;
	data.reexplored = false;
	if( zShader != NULL )
		zShader->setUniform("alpha", 1.f);

	if (miniShader != NULL)
	{
		miniShader->setUniform("alpha", 1.f);
	}
}

void Zone::Update()
{
	VertexArray &va = *definedArea;
	if (zType == NORMAL || zType == MOMENTA || zType == SECRET)
	{
		switch (data.action)
		{
		case UNEXPLORED:
			break;
		case OPENING:
			if (data.frame == openFrames)
			{
				data.action = OPEN;
				data.frame = 0;
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
			if (data.frame == closeFrames)
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

	if (data.action == OPENING || data.action == OPEN)
	{
		++data.framesSinceActivation;
	}
	
	++data.frame;
}

void Zone::Update(float zoom, sf::Vector2f &topLeft, sf::Vector2f &playertest)
{
	VertexArray &va = *definedArea;
	if (zType == NORMAL || zType == MOMENTA || zType == SECRET)
	{
		switch (data.action)
		{
		case UNEXPLORED:
			break;
		case OPENING:
			if (data.frame == openFrames)
			{
				data.action = OPEN;
				data.frame = 0;
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
			if (data.frame == closeFrames)
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
		zShader->setUniform("topLeft", topLeft); //just need to change the name topleft eventually
		zShader->setUniform("playertest", playertest);
		break;
	}
	++data.frame;
}

int Zone::GetNumRemainingKillableEnemies()
{
	int numLeft = 0;
	for (auto it = allEnemies.begin(); it != allEnemies.end(); ++it)
	{
		if ((*it)->CountsForEnemyGate())
		{
			if (!(*it)->spawned || ( !(*it)->dead && (*it)->numHealth > 0 ))
			{
				++numLeft;
			}
		}
	}

	return numLeft;
}

float Zone::GetOpeningAlpha()
{
	return 1.f - data.frame / ((float)openFrames);
}

void Zone::Close()
{
	data.action = CLOSED;
	data.frame = 0;
	zShader->setUniform("alpha", 1.f);
}

//returns true if it wasn't active before, false if its just reexplored
bool Zone::Activate( bool instant)
{
	if (!data.active)
	{
		Session *sess = Session::GetSession();

		if (instant)
		{
			data.action = Zone::OPEN;
		}
		else
		{
			data.action = Zone::OPENING;
			data.frame = 0;
		}

		for (list<Enemy*>::iterator it = spawnEnemies.begin(); it != spawnEnemies.end(); ++it)
		{
			assert((*it)->spawned == false);

			(*it)->Init();
			(*it)->spawned = true;
			sess->AddEnemy((*it));
		}

		data.active = true;
		data.visited = true;

		return true;
	}
	else
	{
		data.reexplored = true;

		return false;
	}
}

bool Zone::IsOpening()
{
	return data.action == OPENING;
}

void Zone::SetClosing( int alreadyOpenFrames )
{
	if (IsOpening())
	{
		data.frame = alreadyOpenFrames - data.frame;
	}
	else
	{
		data.frame = 0;
	}

	data.action = Zone::CLOSING;
	data.active = false;
}

void Zone::CloseOffIfLimited()
{
	if (zType != Zone::SECRET && !data.visited && shouldReform)
	{
		data.visited = true;
		ReformAllGates();
	}
}

void Zone::SetShouldReform(bool on)
{
	shouldReform = on;
}

bool Zone::ShouldReform()
{
	return shouldReform;
}

bool Zone::IsActive()
{
	return data.active;
}

int Zone::GetFramesSinceActivation()
{
	return data.framesSinceActivation;
}

bool Zone::IsStartingToOpen()
{
	return data.action == Zone::OPENING && data.frame <= 20 && !data.reexplored;
}

bool Zone::IsShowingEnemyZoneSprites()
{
	return data.action == Zone::UNEXPLORED || IsStartingToOpen();
}

int Zone::GetFrame()
{
	return data.frame;
}

void Zone::DrawMinimap(sf::RenderTarget *target)
{
	if (data.action != OPEN)//!active )
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
	if (data.action != OPEN)//!active )
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
	return zonePoly->IsInsideArea(test);
	//bool c = false;

	//vector<Vector2i> &pVec = PointVector();

	//int pCount = pVec.size();
	//
	//V2d curr,prev;
	//for (int i = 0; i < pCount; ++i)
	//{
	//	curr = V2d(pVec[i]);
	//	if (i == 0)
	//		prev = V2d(pVec[pCount - 1]);
	//	else
	//		prev = V2d(pVec[i - 1]);

	//	if (((curr.y >= test.y) != (prev.y >= test.y)) &&
	//		(test.x <= (prev.x - curr.x) * (test.y - curr.y) / (prev.y - curr.y) + curr.x))
	//		c = !c;
	//}

	////I changed all of these to less than or equals and greater than or equals
	////to fix issues where the border touches the edge of another zone.
	////might cause issues later, so leaving this note here just in case.

	//return c;
}

bool Zone::ContainsZone(Zone *z)
{
	//midpoint on the gate

	//I use the polygon containsPoint here because it includes things being "on the line" or shared points.
	//the IsInsideArea function is only for points that are actually inside the area, not shared by it.
	//might cause other bugs later, not sure, definitely fixes the current issue though


	vector<Vector2i> &pVec = z->PointVector();
	for (auto it = pVec.begin(); it != pVec.end(); ++it)
	{
		if (!zonePoly->ContainsPoint( Vector2f((*it)) ) )//!ContainsPoint(V2d((*it))))
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

Zone* Zone::ContainsPointMostSpecific( V2d test )
{
	bool contains = ContainsPoint( test );
	//bool contains = false;
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

int Zone::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Zone::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &data, sizeof(MyData));
}

void Zone::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
}