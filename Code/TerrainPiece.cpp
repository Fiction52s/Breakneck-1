#include "TerrainPiece.h"
#include "GameSession.h"
#include "TouchGrass.h"
#include "Grass.h"
#include "TerrainRender.h"

using namespace sf;
using namespace std;

TerrainPiece::TerrainPiece(GameSession *p_owner)
	:owner(p_owner)
{
	groundva = NULL;
	slopeva = NULL;
	steepva = NULL;
	wallva = NULL;
	triva = NULL;
	flowva = NULL;
	plantva = NULL;
	decorLayer0va = NULL;
	bushVA = NULL;

	terrainVA = NULL;
	grassVA = NULL;

	grassSize = 128;
	grassSpacing = -60;
}

TerrainPiece::~TerrainPiece()
{
	if (tr != NULL)
		delete tr;

	for (auto it = bushes.begin(); it != bushes.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		delete (*it);
	}

	delete groundva;
	delete slopeva;
	delete steepva;
	delete wallva;
	delete triva;
	delete flowva;
	delete plantva;
	delete decorLayer0va;
	delete bushVA;

	delete terrainVA;
	delete grassVA;
}

void TerrainPiece::Reset()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Reset();
	}
}

int TerrainPiece::GetNumGrass(Edge *e, bool &rem)
{
	rem = false;

	V2d v0 = e->v0;
	V2d v1 = e->v1;

	double len = length(v1 - v0);
	len -= grassSize / 2 + grassSpacing;
	double reps = len / (grassSize + grassSpacing);
	double remainder = reps - floor(reps);
	if (remainder > 0)
	{
		reps += 1; //for the last one
		rem = true;
	}
	reps += 1;

	int num = reps;

	return num;
}

void TerrainPiece::SetupGrass(Edge * e, int &i)
{
	VertexArray &grassVa = *grassVA;

	V2d v0 = e->v0;
	V2d v1 = e->v1;

	bool rem;
	int num = GetNumGrass(e, rem);

	V2d along = normalize(v1 - v0);
	V2d realStart = v0 + along * (double)(grassSize + grassSpacing);

	for (int j = 0; j < num; ++j)
	{
		V2d posd = realStart + along * (double)((grassSize + grassSpacing) * (j - 1));//v0 + normalize(v1 - v0) * ((grassSize + grassSpacing) * (j-1) + );

		if (j == 0)
		{
			posd = v0;
		}
		else if (j == num - 1 && rem)
		{
			//V2d prev = ;//v0 + (v1 - v0) * ((double)(j-1) / num);
			posd = v1 + normalize(v0 - v1) * (grassSize / 2.0 + grassSpacing);//(v1 + prev) / 2.0;
		}

		Vector2f pos(posd.x, posd.y);


		Vector2f topLeft = pos + Vector2f(-grassSize / 2, -grassSize / 2);
		Vector2f topRight = pos + Vector2f(grassSize / 2, -grassSize / 2);
		Vector2f bottomLeft = pos + Vector2f(-grassSize / 2, grassSize / 2);
		Vector2f bottomRight = pos + Vector2f(grassSize / 2, grassSize / 2);

		//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
		grassVa[i * 4].color.a = 0;
		grassVa[i * 4].position = topLeft;
		grassVa[i * 4].texCoords = Vector2f(0, 0);

		//grassVa[i*4+1].color = Color::Blue;
		//borderVa[i*4+1].color.a = 10;
		grassVa[i * 4 + 1].color.a = 0;
		grassVa[i * 4 + 1].position = bottomLeft;
		grassVa[i * 4 + 1].texCoords = Vector2f(0, grassSize);

		//grassVa[i*4+2].color = Color::Blue;
		//borderVa[i*4+2].color.a = 10;
		grassVa[i * 4 + 2].color.a = 0;
		grassVa[i * 4 + 2].position = bottomRight;
		grassVa[i * 4 + 2].texCoords = Vector2f(grassSize, grassSize);

		//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
		//borderVa[i*4+3].color.a = 10;
		grassVa[i * 4 + 3].color.a = 0;
		grassVa[i * 4 + 3].position = topRight;
		grassVa[i * 4 + 3].texCoords = Vector2f(grassSize, 0);
		++i;
	}
}

void TerrainPiece::SetupGrass(std::list<GrassSegment> &segments)
{
	int numGrassTotal = 0;

	for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
	{
		numGrassTotal += (*it).reps + 1;
	}

	Tileset *ts_grass = owner->GetTileset("Env/grass_128x128.png", 128, 128);

	//should this even be made on invisible terrain?

	int totalGrassIndex = 0;

	Grass::GrassType gType;
	if (terrainWorldType == 0)
	{
		gType = Grass::GrassType::JUMP;
	}
	else if (terrainWorldType == 1)
	{
		gType = Grass::GrassType::GRAVITY;
	}
	else if (terrainWorldType == 2)
	{
		gType = Grass::GrassType::BOUNCE;
	}
	else if (terrainWorldType == 3)
	{
		gType = Grass::GrassType::BOOST;
	}
	else if (terrainWorldType == 5)
	{
		gType = Grass::GrassType::ANTIWIRE;
	}
	else
	{
		gType = Grass::GrassType::JUMP;
	}

	owner->hasGrass[gType] = true;
	owner->hasAnyGrass = true;

	if (numGrassTotal > 0)
	{
		grassVA = new VertexArray(sf::Quads, numGrassTotal * 4);

		//cout << "num grass total: " << numGrassTotal << endl;
		VertexArray &grassVa = *grassVA;//*va;

		int segIndex = 0;
		int totalGrass = 0;
		for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
		{
			Edge *segEdge = owner->GetEdge(startEdgeIndex + (*it).edgeIndex);
			V2d v0 = segEdge->v0;
			V2d v1 = segEdge->v1;

			int start = (*it).index;
			int end = (*it).index + (*it).reps;

			int grassCount = (*it).reps + 1;
			bool rem;
			int num = GetNumGrass(segEdge, rem);

			V2d along = normalize(v1 - v0);
			V2d realStart = v0 + along * (double)(grassSize + grassSpacing);

			int jj;
			for (int j = 0; j < grassCount; ++j)
			{
				jj = j + start;
				V2d posd = realStart + along * (double)((grassSize + grassSpacing) * (jj - 1));//v0 + normalize(v1 - v0) * ((grassSize + grassSpacing) * (j-1) + );

				if (jj == 0)
				{
					posd = v0;
				}
				else if (jj == num - 1 && rem)
				{
					posd = v1 + normalize(v0 - v1) * (grassSize / 2.0 + grassSpacing);//(v1 + prev) / 2.0;
				}

				Vector2f pos(posd.x, posd.y);

				Vector2f topLeft = pos + Vector2f(-grassSize / 2, -grassSize / 2);
				Vector2f topRight = pos + Vector2f(grassSize / 2, -grassSize / 2);
				Vector2f bottomLeft = pos + Vector2f(-grassSize / 2, grassSize / 2);
				Vector2f bottomRight = pos + Vector2f(grassSize / 2, grassSize / 2);

				grassVa[(j + totalGrass) * 4].position = topLeft;
				grassVa[(j + totalGrass) * 4].texCoords = Vector2f(0, 0);

				grassVa[(j + totalGrass) * 4 + 1].position = bottomLeft;
				grassVa[(j + totalGrass) * 4 + 1].texCoords = Vector2f(0, grassSize);

				grassVa[(j + totalGrass) * 4 + 2].position = bottomRight;
				grassVa[(j + totalGrass) * 4 + 2].texCoords = Vector2f(grassSize, grassSize);

				grassVa[(j + totalGrass) * 4 + 3].position = topRight;
				grassVa[(j + totalGrass) * 4 + 3].texCoords = Vector2f(grassSize, 0);

				Grass * g = new Grass(owner, ts_grass, totalGrassIndex, posd, this, gType);
				owner->grassTree->Insert(g);

				++totalGrassIndex;
			}
			totalGrass += grassCount;
			segIndex++;
		}
	}
	else
	{
		grassVA = NULL;
	}

	//int totalGrassIndex = 0;

	//for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
	//{
	//	V2d A, B, C, D;
	//	Edge * currE = owner->GetEdge(startEdgeIndex + (*it).edgeIndex);
	//	V2d v0 = currE->v0;
	//	V2d v1 = currE->v1;

	//	double edgeLength = length(v1 - v0);
	//	double remainder = edgeLength / (grassSize + grassSpacing);

	//	double num = floor(remainder) + 1;

	//	int reps = (*it).reps;

	//	V2d edgeDir = normalize(v1 - v0);

	//	//V2d ABmin = v0 + (v1-v0) * (double)(*it).index / num - grassSize / 2 );
	//	V2d ABmin = v0 + edgeDir * (edgeLength * (double)(*it).index / num - grassSize / 2);
	//	V2d ABmax = v0 + edgeDir * (edgeLength * (double)((*it).index + reps) / num + grassSize / 2);
	//	double height = grassSize / 2;
	//	V2d normal = normalize(v1 - v0);
	//	double temp = -normal.x;
	//	normal.x = normal.y;
	//	normal.y = temp;

	//	A = ABmin + normal * height;
	//	B = ABmax + normal * height;
	//	C = ABmax;
	//	D = ABmin;

	//	Grass * g = new Grass(this, ts_grass, totalGrassIndex, A, B, C, D, this);

	//	owner->grassTree->Insert(g);

	//	totalGrassIndex++;
	//}
}

void TerrainPiece::UpdateBushFrame()
{
	/*bushFrame++;
	if( bushFrame == bushAnimLength * bushAnimFactor )
	{
	bushFrame = 0;
	}*/
}

void TerrainPiece::DrawBushes(sf::RenderTarget *target)
{
	for (list<DecorExpression*>::iterator it = bushes.begin();
		it != bushes.end(); ++it)
	{
		Tileset *ts = (*it)->layer->ts;
		target->draw(*(*it)->va, ts->texture);
	}
	//target->draw( 
}

void TerrainPiece::UpdateBushes()
{
	//int numBushes = bushVA->getVertexCount() / 4;

	//VertexArray &bVA = *bushVA;
	//IntRect subRect = ts_bush->GetSubRect( bushFrame / bushAnimFactor );

	/*for( int i = 0; i < numBushes; ++i )
	{
	bVA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
	bVA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
	bVA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
	bVA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}*/
	//++bushFrame;
}

void TerrainPiece::HandleQuery(QuadTreeCollider *qtc)
{
	qtc->HandleEntrant(this);
}

bool TerrainPiece::IsTouchingBox(const sf::Rect<double> &r)
{
	return IsBoxTouchingBox(aabb, r);
}

void TerrainPiece::Draw(sf::RenderTarget *target)
{
	if (visible)
	{
		if (grassVA != NULL)
			target->draw(*grassVA, owner->ts_gravityGrass->texture);

		if (owner->usePolyShader)
		{
			sf::Rect<double> screenRect = owner->screenRect;
			sf::Rect<double> polyAndScreen;
			double rightScreen = screenRect.left + screenRect.width;
			double bottomScreen = screenRect.top + screenRect.height;
			double rightPoly = aabb.left + aabb.width;
			double bottomPoly = aabb.top + aabb.height;

			double left = std::max(screenRect.left, aabb.left);

			double right = std::min(rightPoly, rightScreen);

			double top = std::max(screenRect.top, aabb.top);

			double bottom = std::min(bottomScreen, bottomPoly);


			polyAndScreen.left = left;
			polyAndScreen.top = top;
			polyAndScreen.width = right - left;
			polyAndScreen.height = bottom - top;

			assert(pShader != NULL);
			target->draw(*terrainVA, pShader);
		}
		else
		{
			target->draw(*terrainVA);
		}

		if (owner->showTerrainDecor)
		{
			tr->Draw(target);
		}

		for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
		{
			(*it)->Draw(target);
		}
	}
}

void TerrainPiece::AddDecorExpression(DecorExpression *exp)
{
	bushes.push_back(exp);
}

void TerrainPiece::AddTouchGrass(int gt)
{
	list<PlantInfo> info;

	TouchGrass::TouchGrassType gType = (TouchGrass::TouchGrassType)gt;

	int tw = TouchGrass::GetQuadWidth(gType);

	Edge *startEdge = tr->startEdge;

	Edge *te = startEdge;
	do
	{
		bool valid = true;

		EdgeAngleType eat = GetEdgeAngleType(te->Normal());
		if (eat == EDGE_FLAT || eat == EDGE_SLOPED)
		{
			valid = true;
		}
		else
		{
			valid = false;
		}
		//valid = true;

		if (valid)
		{
			double len = length(te->v1 - te->v0);
			int numQuads = len / tw;
			bool tooThin = false;
			double quadWidth = tw;//len / numQuads;
			if (numQuads == 0)
			{
				tooThin = true;
				numQuads = 1;
			}
			if (numQuads > 0)
			{
				for (int i = 0; i < numQuads; ++i)
				{
					if (TouchGrass::IsPlacementOkay(gType, eat,
						te, i))
					{
						if (tooThin)
						{
							info.push_back(PlantInfo(te, te->GetLength() / 2.0, quadWidth));
						}
						else
						{
							info.push_back(PlantInfo(te, quadWidth * i + quadWidth / 2, quadWidth));
						}

					}
				}
			}
		}
		te = te->edge1;
	} while (te != startEdge);

	int infoSize = info.size();
	int vaSize = infoSize * 4;

	if (infoSize == 0)
	{
		return;
	}

	TouchGrassCollection *coll = new TouchGrassCollection;
	touchGrassCollections.push_back(coll);

	//cout << "number of plants: " << infoSize << endl;
	coll->touchGrassVA = new Vertex[vaSize];

	coll->ts_grass = TouchGrassCollection::GetTileset(owner, gType);
	coll->gType = gType;
	coll->numTouchGrasses = infoSize;

	int vaIndex = 0;
	for (list<PlantInfo>::iterator it = info.begin(); it != info.end(); ++it)
	{
		coll->CreateGrass(vaIndex, (*it).edge, (*it).quant);
		vaIndex++;
	}
}

void TerrainPiece::QueryTouchGrass(QuadTreeCollider *qtc, sf::Rect<double> &r)
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Query(qtc, r);
	}
}

void TerrainPiece::UpdateTouchGrass()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->UpdateGrass();
	}
}

void TerrainPiece::UpdateBushSprites()
{
	for (list<DecorExpression*>::iterator it = bushes.begin();
		it != bushes.end(); ++it)
	{
		(*it)->UpdateSprites();
	}
}