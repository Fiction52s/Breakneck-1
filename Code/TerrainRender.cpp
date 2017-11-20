#include "TerrainRender.h"
#include "Tileset.h"
#include <assert.h>
#include "Physics.h"
#include "Movement.h"

using namespace sf;
using namespace std;

std::map<DecorType, DecorLayer*> TerrainRender::s_decorLayerMap;

void DecorRect::HandleQuery(QuadTreeCollider * qtc)
{
	if (qtc != NULL)
		qtc->HandleEntrant(this);
}

bool DecorRect::IsTouchingBox(const sf::Rect<double> &r)
{
	return rect.intersects(r);
}

void TerrainRender::GenerateCenterMesh()
{

}

void TerrainRender::CleanupLayers()
{
	for (auto it = s_decorLayerMap.begin(); it != s_decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}
		
		
	s_decorLayerMap.clear();
}

TerrainRender::TerrainRender(TilesetManager *p_tMan, QuadTree *p_terrainTree)
	:tMan( p_tMan ), terrainTree(p_terrainTree )
{
	decorTree = new QuadTree(1000000, 1000000);
}

struct Triple
{
	Triple()
		:numQuads128(0), numQuads32(0), numQuads16(0)
	{

	}
	int GetTotal()
	{
		return numQuads128 + numQuads32 + numQuads16;
	}
	int numQuads128;
	int numQuads32;
	int numQuads16;
};

enum EdgeSize
{
	T_128,
	T_32,
	T_16
};

struct EdgeInfo
{

};

void TerrainRender::GenerateBorderMesh()
{
	/*int worldNum = 0;
	if( envType < 1 )
	{
		worldNum = 0;
	}
	else if( envType < 2 )
	{

	}*/
	//int worldNum = (int)envType; //temporary

	//rayMode = "border_quads";
	//QuadTree *qt = NULL;
	/*if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}*/
	QuadTree *qt = terrainTree;
	assert( qt != NULL );

	//int tw = ts->tileWidth;//128;//64;
	//int th = 512;
	//int numTotalQuads = 0;
	Triple totalQuads;
	//double intersect = 10;//tw / 6.0;//48;
	//double extraLength = 0;//32.0;
	Edge *te = startEdge;//edges[currentEdgeIndex];

	map<Edge*, Triple> numQuadMap[E_WALL +1];
	list<Edge*> transEdges;


	/*do
	{
		double len = length(te->v1 - te->v0);
		while (len > 0)
		{

		}
	}
	while (te != startEdge);*/

	int startInter128 = 4;
	int startInter32 = 4;
	int startInter16 = 4;

	int inter128 = GetBorderQuadIntersect(128);
	int inter32 = GetBorderQuadIntersect(32);
	int inter16 = GetBorderQuadIntersect(16);

	int len128 = 128 - inter128 * 2;
	int len32 = 32 - inter32 * 2;
	int len16 = 16 - inter16 * 2;

	int edgeLen128 = 128 - inter128;
	int edgeLen32 = 32 - inter32;
	int edgeLen16 = 16 - inter16;

	int startLen128 = 128 - inter128;
	int startLen32 = 32 - inter32;
	int startLen16 = 16 - inter16;

	int firstType;
	int secondType;
	
	do
	{
		V2d eNorm = te->Normal();

		/*V2d currDir = normalize(te->v1 - te->v0);
		V2d prevDir = normalize(te->edge0->v0 - te->v0);

		V2d pCurrDir = -currDir;
		V2d nDir = normalize(te->edge1->v1 - te->edge1->v0);
		double cDiff = GetVectorAngleDiffCCW(Vector2f(currDir), Vector2f(prevDir));
		double nDiff = GetVectorAngleDiffCCW(Vector2f(nDir), Vector2f(pCurrDir));

		bool turnInward = cDiff < PI;
		bool nextInward = nDiff < PI;
		if ( turnInward )
		{
			transEdges.push_back(te);
		}*/

		EdgeType eType = GetEdgeNormalType(eNorm);
		//int valid = ValidEdge( eNorm );
		//if( valid != -1 )//eNorm.x == 0 )
		
		{
			
			double len = length(te->v1 - te->v0);// +3 * 2.0;// +extraLength * 2;
			len -= GetSubForOutward(te);
			len -= GetSubForOutward(te->edge1);
			len += GetExtraForInward(te);
			len += GetExtraForInward(te->edge1);
			if (len <= 0)
			{
				te = te->edge1;
				continue;
			}
			

			double fullParts = len / 128.0;
			int numFullparts = fullParts;

			//int numQuads = 0;
			Triple numQuads;
			
			//if( len < GetBorderRealWidth( ))
			int iter = 0;

			
			while (len > 0)
			{
				if (len >= len128)
				{
					len -= len128;//GetBorderRealWidth(128, false);
					numQuads.numQuads128++;
				}
				else if (len >= len32)
				{
					len -= len32;
					numQuads.numQuads32++;
				}
				else if (len >= len16)
				{
					len -= len16;
					numQuads.numQuads16++;
				}
				else
				{
					numQuads.numQuads16++;
					break;
				}
			}

			
			//if( len >  )

			//if (len < 8 )
			//{
			//	numQuads = 0;
			//}
			//else if (len <= tw - extraLength * 2)
			//{
			//	numQuads = 1;
			//}
			//else
			//{
			//	numQuads = 2;
			//	double firstRight = tw - extraLength - intersect;
			//	double endLeft = (len + extraLength) - tw + intersect;

			//	//optimize later to not be a loop
			//	while (firstRight <= endLeft)
			//	{
			//		firstRight += tw - intersect;
			//		++numQuads;
			//	}
			//}

			// = ceil(len / (tw - intersect));
			//double quadWidth = len / numQuads;
			//numTotalQuads += numQuads.numQuads128 + numQuads.numQuads32 + numQuads.numQuads16;
			totalQuads.numQuads128 += numQuads.numQuads128;
			totalQuads.numQuads32 += numQuads.numQuads32;
			totalQuads.numQuads16 += numQuads.numQuads16;
			numQuadMap[eType][te] = numQuads;
		}
		te = te->edge1;
		
	}
	while( te != startEdge );

	totalNumBorderQuads = totalQuads.GetTotal();
	totalNumBorderQuads += transEdges.size();
	if (totalNumBorderQuads == 0)
	{
		//totalNumBorderQuads = 0;
		borderVA = NULL;
		return; //NULL;
	}

	
	borderVA = new Vertex[totalNumBorderQuads * 4 ];

	int totals[E_WALL +1];
	std::memset(totals, 0, sizeof(totals));

	int current[E_WALL + 1];
	std::memset(current, 0, sizeof(current));

	for (int i = 0; i < E_WALL + 1; ++i)
	{
		auto &qmRef = numQuadMap[i];
		for( auto it = qmRef.begin(); it != qmRef.end(); ++it )
		{
			totals[i] += (*it).second.GetTotal();
		}
	}
	sf::Vertex *bva[E_WALL + 1];
	sf::Vertex *transva;
	int start = 0;

	transva = (borderVA + start * 4);
	start += transEdges.size();
	for (int i = E_WALL; i >= 0; --i)
	{
		bva[i] = (borderVA + start * 4);
		start += totals[i];
	}

	
	//VertexArray *currVA = new VertexArray( sf::Quads, numTotalQuads * 4 );
	
	//sf::Vertex *start128VA = borderVA;
	//sf::Vertex *start32VA = (borderVA + totalQuads.numQuads128 * 4);
	//sf::Vertex *start16VA = (borderVA + totalQuads.numQuads128 * 4 + totalQuads.numQuads32 * 4 );

	//VertexArray &va = *currVA;
	
	int tw;

	int extra = 0;
	te = startEdge;
	int varietyCounter = 0;
	double extraLength = 0;
	do
	{
		V2d eNorm = te->Normal();
		//int valid = ValidEdge( eNorm );
		//if( valid != -1 )
		{
			double len = length(te->v1 - te->v0);// +extraLength * 2;
			//len -= GetSubForOutward(te);
			//len -= GetSubForOutward(te->edge1);


			//int numQuads = numQuadMap[te];//ceil( len / tw ); 
			EdgeType et = GetEdgeNormalType(eNorm);
			Triple numQuads = numQuadMap[et][te];
			if (numQuads.GetTotal() == 0)
			{
				te = te->edge1;
				continue;
			}

			

			//double quadWidth = ts->tileWidth;;//len / numQuads;

			V2d along = normalize( te->v1 - te->v0 );
			V2d other( along.y, -along.x );

			
			double out = 16;//40;//16;
			double in = 64 - out;//256 - out;//; - out;
			
			double inwardExtra = GetExtraForInward(te);
			double nInwardExtra = GetExtraForInward(te->edge1);

			V2d startInner = te->v0 - along * extraLength - other * in;
			V2d startOuter = te->v0 - along * extraLength + other * out;

			double startAlong = GetSubForOutward(te) - inwardExtra;
			double trueEnd = len - GetSubForOutward(te->edge1) + nInwardExtra;
			

			int currTotal = numQuads.GetTotal();
			Vertex *currBVA = bva[et] + current[et] * 4;
			
			int *numQ[3];

			int numQI = 0;
			if (numQuads.numQuads128 > 0)
			{
				numQ[numQI] = &numQuads.numQuads128;
				++numQI;
			}
			if (numQuads.numQuads32 > 0)
			{
				numQ[numQI] = &numQuads.numQuads32;
				++numQI;
			}
			if (numQuads.numQuads16 > 0)
			{
				numQ[numQI] = &numQuads.numQuads16;
				++numQI;
			}

			assert(numQI != 0);

			

			
			

			for( int i = 0; i < currTotal; ++i )
			{
				//worldNum * 5
				//int valid = ValidEdge( eNorm );
				//add (worldNum * 5) to realIndex to get the correct borders
				//int realIndex = valid;// *4 + varietyCounter;//32 + varietyCounter;
				//cout << "real Index: " << realIndex << ", valid: " << valid << ", variety: " << varietyCounter << endl;
				//IntRect sub = ts->GetSubRect( realIndex );

				int numQI = 0;
				if (numQuads.numQuads128 > 0)
				{
					numQ[numQI] = &numQuads.numQuads128;
					++numQI;
				}
				if (numQuads.numQuads32 > 0)
				{
					numQ[numQI] = &numQuads.numQuads32;
					++numQI;
				}
				if (numQuads.numQuads16 > 0)
				{
					numQ[numQI] = &numQuads.numQuads16;
					++numQI;
				}

				int r = rand() % numQI;
				

				if (numQ[r] == &numQuads.numQuads128)
				{
					tw = 128;
				}
				else if (numQ[r] == &numQuads.numQuads32)
				{
					tw = 32;
				}
				if (numQ[r] == &numQuads.numQuads16)
				{
					tw = 16;
				}

				(*numQ[r])--;
				
				
				{
					startAlong -= GetBorderQuadIntersect(tw);
				}
				

				double endAlong = startAlong + tw;//GetBorderRealWidth(tw, false);
				//double trueLen = len - GetSubForOutward(te->edge1);
				if (endAlong > trueEnd)// + 3 + nInwardExtra)
				{
					endAlong = trueEnd;// +3 + nInwardExtra;
					startAlong = endAlong - tw;
				}

				if (currTotal == 1)
				{
					startAlong = len / 2 - tw / 2 + GetBorderQuadIntersect(tw);
					endAlong = len / 2 + tw / 2;
				}
				IntRect sub = GetBorderSubRect(tw, et, varietyCounter);
				//cout << "left: " << sub.left << ", top: " << sub.top << 
				//	", w: " << sub.width << ", h: " << sub.height << endl;
				/*if (numQuads == 1)
				{
					startAlong = len / 2 - tw / 2;
					endAlong = startAlong + tw;
				}
				else if (numQuads == 2)
				{
					if (i == 0)
					{
						startAlong = -extraLength;
						endAlong = startAlong + tw;
					}
					else
					{
						endAlong = len + extraLength;
						startAlong = endAlong - tw;
					}
				}
				else if (numQuads % 2 == 0 )
				{
					if (i < numQuads / 2 )
					{
						startAlong = -extraLength + ((tw - intersect) * i);
						endAlong = startAlong + tw;
					}
					else 
					{
						endAlong = len + extraLength - ((tw - intersect) * ((numQuads - 1) - i));
						startAlong = endAlong - tw;
					}
				}
				else
				{
					if (i * 2 == numQuads - 1)
					{
						startAlong = len / 2 - tw / 2;
						endAlong = startAlong + tw;
					}
					else if (i < numQuads / 2)
					{
						startAlong = -extraLength + ((tw - intersect) * i);
						endAlong = startAlong + tw;
					}
					else
					{
						endAlong = len + extraLength - ((tw - intersect) * (( numQuads -1 ) - i));
						startAlong = endAlong - tw;
					}
				}*/
				
				
				double trueAlong = startAlong;
				if (startAlong > 0)
				{
					//trueAlong -= GetBorderQuadIntersect(tw);
				}

				V2d currStartInner = startInner + trueAlong * along;
				V2d currStartOuter = startOuter + trueAlong * along;
				V2d currEndInner = startInner + endAlong * along;
				V2d currEndOuter = startOuter + endAlong * along;
						
				double realHeightLeft = in;//sub.height - out;//256;//in;//sub.height;
				double realHeightRight = in;//sub.height - out;//256;//in;//sub.height;
				
				double d0 = dot( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );
				double c0 = cross( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );

				double d1 = dot( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );
				double c1 = cross( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );

				
				//if( d0 <= 0

				rcEdge = NULL;
				ignoreEdge = te;
				rcQuant = -1;
				rcPortion = 2.0; //always greater than portion on first run
				//rayIgnoreEdge = te;
				rayStart = te->v0 + along * max( startAlong, 0.0 );
				rayEnd = rayStart - eNorm * realHeightLeft;
				//rayStart = te->v0 - along * extraLength + ( startAlong ) * along;
				//rayEnd = currStartInner;//te->v0 + (double)i * quadWidth * along - other * in;
				RayCast( this, qt->startNode, rayStart, rayEnd );


				//start ray
				if( rcEdge != NULL )
				{
					//currStartInner = rcEdge->GetPoint( rcQuant );
					//realHeightLeft = length(currStartInner - currStartOuter);
					//realHeight0 = length( currStartInner - currStartOuter );
				}

				rcEdge = NULL;
				rayStart = te->v0 + along * endAlong;//te->v0 - along * extraLength + ( endAlong ) * along;
				rayEnd = rayStart - eNorm * realHeightRight;//currEndInner;
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					//currEndInner =  rcEdge->GetPoint( rcQuant );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeightRight = length( currEndInner - currEndOuter );
				}

				
				//RayCast( this, terrainTree, position, V2d( position.x - 100, position.y ) );
				
				//if( i == 0 && d0 <= 0 )
				//{
				//	Edge *prev = te->edge0;
				//	V2d pNorm = prev->Normal();
				//	V2d prevEndInner = prev->v1 - pNorm * in;
				//	V2d prevEndOuter = prev->v1 + pNorm * out;
				//	//V2d prevEndOuter = prev->v1 - 

				//	va[extra + i * 4 + 0].position = Vector2f( ( currStartOuter.x + prevEndOuter.x ) / 2.0, 
				//		( currStartOuter.y + prevEndOuter.y ) / 2.0 );
				//	
				//	va[extra + i * 4 + 3].position = Vector2f( (currStartInner.x + prevEndInner.x) / 2.0, 
				//		( currStartInner.y + prevEndInner.y ) / 2.0  );
				//}
				//else
				{
					Vector2f a = Vector2f( currStartOuter.x, currStartOuter.y );
					Vector2f b = Vector2f( currStartInner.x, currStartInner.y );
					/*a += Vector2f( .5, .5 );
					b += Vector2f( .5, .5 );
					a.x = floor( a.x );
					a.y = floor( a.y );
					b.x = floor( b.x );
					b.y = floor( b.y );*/

					currBVA[i * 4 + 0].position = a;
					currBVA[i * 4 + 3].position = b;
				}

				/*if( i == numQuads - 1 && d1 <= 0 )
				{
					Edge *next = te->edge1;
					V2d nNorm = next->Normal();
					V2d nextStartInner = next->v0 - nNorm * in;
					V2d nextStartOuter = next->v0 + nNorm * out;
					va[extra + i * 4 + 2].position = Vector2f( ( currEndInner.x + nextStartInner.x ) / 2, 
						( currEndInner.y + nextStartInner.y ) / 2 );

					va[extra + i * 4 + 1].position = Vector2f( ( currEndOuter.x + nextStartOuter.x ) / 2.0, 
						( currEndOuter.y + nextStartOuter.y ) / 2.0 );

				}
				else*/
				{
					Vector2f c = Vector2f( currEndInner.x, currEndInner.y);
					Vector2f d = Vector2f( currEndOuter.x, currEndOuter.y);

					/*c += Vector2f( .5, .5 );
					d += Vector2f( .5, .5 );
					c.x = floor( c.x );
					c.y = floor( c.y );
					d.x = floor( d.x );
					d.y = floor( d.y );*/

					currBVA[i * 4 + 2].position = c;
					currBVA[i * 4 + 1].position = d;
				}

				/*va[extra + i * 4 + 2].position = Vector2f( currEndInner.x, currEndInner.y );
				va[extra + i * 4 + 3].position = Vector2f( currStartInner.x, currStartInner.y );*/
				
				
				currBVA[i * 4 + 0].texCoords = Vector2f( sub.left, sub.top );
				currBVA[i * 4 + 1].texCoords = Vector2f( sub.left + sub.width, sub.top );
				currBVA[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + realHeightLeft + out );
				currBVA[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + realHeightRight + out );

				/*va[extra + i * 4 + 0].color = COLOR_BLUE;
				va[extra + i * 4 + 1].color = COLOR_YELLOW;
				va[extra + i * 4 + 2].color = COLOR_MAGENTA;
				va[extra + i * 4 + 3].color = COLOR_TEAL;
*/
				++varietyCounter;
				if( varietyCounter == 4 )//32 )
				{
					varietyCounter = 0;
				}

				if (i == 0)
				{
					startAlong += tw -GetBorderQuadIntersect(tw);
				}
				else
				{
					startAlong += tw -GetBorderQuadIntersect(tw);
				}
				//startAlong += //GetBorderRealWidth(tw, false);//tw - intersect;
			}

			current[et] += currTotal;
			//extra += numQuads * 4;
		}

		
		te = te->edge1;
	}
	while( te != startEdge );

	for (auto it = transEdges.begin(); it != transEdges.end(); ++it)
	{
		V2d eNorm = (*it)->Normal();
		V2d pNorm = (*it)->edge0->Normal();
		V2d eDir = normalize((*it)->v1 - (*it)->v0);
		V2d pDir = normalize((*it)->edge0->v1 - (*it)->edge0->v0);
		V2d outCurr = (*it)->v0 - eNorm * 48.0;
		V2d outPrev = (*it)->v0 - pNorm * 48.0;
		V2d together = normalize( -eDir + pDir );
		V2d tn(together.y, -together.x);

		double width = length(outCurr - outPrev);
		V2d p0 = (*it)->v0 - tn * width / 2.0;
		V2d p1 = (*it)->v0 + tn * width / 2.0;
		double extra = 4.0;
		double height = length(p0 - outPrev) + extra;
		//double a = abs( dot(outCurr - (*it)->v0, tn ) );
		transva[0].position = Vector2f(p0); //Vector2f((*it)->v0);
		transva[1].position = Vector2f(p1);//Vector2f((*it)->v0);
		transva[2].position = Vector2f(outCurr + together * extra);
		transva[3].position = Vector2f(outPrev + together * extra);

		

		EdgeType transType = GetEdgeTransType((*it));

		IntRect sub;
		if (transType > E_WALL )
		{
		//	sub = GetBorderSubRect(64, transType, 0);
		}
		else
		{
		//	sub = GetBorderSubRect(128, transType, 0);
		}

		sub = GetBorderSubRect(128, E_WALL, 0);
		float centerX = sub.left + sub.width / 2;

		float startHeight = sub.top + (sub.height - height);

		transva[0].texCoords = Vector2f(sub.left + centerX - width / 2, startHeight);
		transva[1].texCoords = Vector2f(sub.left + centerX + width / 2, startHeight);
		transva[2].texCoords = Vector2f(sub.left + centerX + width / 2, startHeight + height);
		transva[3].texCoords = Vector2f(sub.left + centerX - width / 2, startHeight + height);
		transva += 4;
	}
	
	//return currVA;
}

sf::IntRect TerrainRender::GetBorderSubRect(int tileWidth, EdgeType et, int var)
{
	sf::IntRect ir;
	ir.width = tileWidth;
	ir.height = 64;

	//int iet = et;
	switch (tileWidth)
	{
	case 16:
	{
		if (et < E_STEEP_CEILING)
		{
			ir.top = 7 * 64;
			ir.left = 128 * 2 + 64 * et + 16 * var;
		}
		else
		{
			ir.top = 8 * 64;
			ir.left = 64 * (et- E_STEEP_CEILING) + 16 * var;
		}
		break;
	}
	case 32:
	{
		if (et < E_STEEP_CEILING )
		{
			ir.top = 6 * 64;
			ir.left = 128 * et + 32 * var;
		}
		else
		{
			ir.top = 7 * 64;
			ir.left = 128 * (et - E_STEEP_CEILING) + 32 * var;
		}
		break;
	}
	case 128:
	{
		ir.left = var * 128;
		ir.top = et * 64;
		break;
	}
	case 64:
	{
		if (et < E_TRANS_SLOPED_TO_STEEP_CEILING)
		{
			ir.top = 8 * 64;
			ir.left = 128 + 64 * et;
		}
		else if (et < E_TRANS_WALL_TO_SLOPED_CEILING)
		{
			ir.top = 9 * 64;
			ir.left = 64 * (et - E_TRANS_WALL_TO_SLOPED_CEILING);
		}
		else
		{
			//just one on this row for now
			ir.top = 10 * 64;
			ir.left = 0;
		}

		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}

	return ir;
}

void TerrainRender::GenerateDecor()
{
	for (int i = 0; i < 6; ++i)
	{
		DecorExpression *expr = CreateDecorExpression(DecorType(D_W1_VEINS1 + i), 0, startEdge);
		if (expr != NULL)
			AddDecorExpression(expr);
	}

	Tileset *ts_testBush = tMan->GetTileset("bush_1_01_512x512.png", 512, 512);

	DecorExpression *rock1 = CreateDecorExpression(D_W1_ROCK_1, 0, startEdge);
	if (rock1 != NULL)
		AddDecorExpression(rock1);

	DecorExpression *rock2 = CreateDecorExpression(D_W1_ROCK_2, 0, startEdge);
	if (rock2 != NULL)
		AddDecorExpression(rock2);

	DecorExpression *rock3 = CreateDecorExpression(D_W1_ROCK_3, 0, startEdge);
	if (rock3 != NULL)
		AddDecorExpression(rock3);

	DecorExpression *grassyRock = CreateDecorExpression(D_W1_GRASSYROCK, 0, startEdge);
	if (grassyRock != NULL)
		AddDecorExpression(grassyRock);

	DecorExpression *normalExpr = CreateDecorExpression(D_W1_BUSH_NORMAL, 0, startEdge);
	if (normalExpr != NULL)
		AddDecorExpression(normalExpr);

	DecorExpression *exprPlantRock = CreateDecorExpression(D_W1_PLANTROCK, 0, startEdge);
	if (exprPlantRock != NULL)
		AddDecorExpression(exprPlantRock);
}

void TerrainRender::AddDecorExpression(DecorExpression *exp)
{
	decorExprList.push_back(exp);
}

void TerrainRender::UpdateDecorSprites()
{
	for (list<DecorExpression*>::iterator it = decorExprList.begin();
		it != decorExprList.end(); ++it)
	{
		(*it)->UpdateSprites();
	}
}

bool TerrainRender::IsEmptyRect(sf::Rect<double> &rect )
{
	emptyResult = true;
	decorTree->Query( this, rect);
	terrainTree->Query(this, rect);
	return emptyResult;
}

void TerrainRender::HandleEntrant(QuadTreeEntrant *qte)
{
	emptyResult = false;
}

DecorExpression * TerrainRender::CreateDecorExpression(DecorType dType,
	int bgLayer,
	Edge *startEdge)
{
	int minApart;
	int maxApart;
	CubicBezier apartBezier;
	int minPen;
	int maxPen;
	CubicBezier penBez;
	int animFactor = 1;

	switch (dType)
	{
	case D_W1_BUSH_NORMAL:
		minApart = 20;
		maxApart = 300;
		apartBezier = CubicBezier(0, 0, 1, 1);
		minPen = 20;
		maxPen = 1000;
		penBez = CubicBezier(0, 0, 1, 1);
		animFactor = 8;
		break;
	case D_W1_ROCK_1:
	case D_W1_ROCK_2:
	case D_W1_ROCK_3:
	case D_W1_PLANTROCK:
	case D_W1_GRASSYROCK:
	default:
		minApart = 500;
		maxApart = 700;
		apartBezier = CubicBezier(0, 0, 1, 1);
		minPen = 200;
		maxPen = 1200;
		penBez = CubicBezier(0, 0, 1, 1);
		animFactor = 1;
		break;
	}

	//assert( positions.empty() );

	int veinLoopWait = 30;

	DecorLayer *layer = NULL;
	Tileset *ts_d = NULL;
	if (s_decorLayerMap.count(dType) == 0)
	{
		//int GameSession::TestVA::bushFrame = 0;
		//int GameSession::TestVA::bushAnimLength = 20;
		//int GameSession::TestVA::bushAnimFactor = 8;

		
		switch (dType)
		{
		case D_W1_BUSH_NORMAL:
			ts_d = tMan->GetTileset("bush_01_64x64.png", 64, 64);
			layer = new DecorLayer(ts_d, 20, 8);
			break;
		case D_W1_ROCK_1:
			ts_d = tMan->GetTileset("rock_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_2:
			ts_d = tMan->GetTileset("rock_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_3:
			ts_d = tMan->GetTileset("rock_1_03_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_PLANTROCK:
			ts_d = tMan->GetTileset("bush_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_GRASSYROCK:
			ts_d = tMan->GetTileset("bush_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_VEINS1:
			ts_d = tMan->GetTileset("veins_w1_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS2:
			ts_d = tMan->GetTileset("veins_w1_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS3:
			ts_d = tMan->GetTileset("veins_w1_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS4:
			ts_d = tMan->GetTileset("veins_w1_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS5:
			ts_d = tMan->GetTileset("veins_w1_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS6:
			ts_d = tMan->GetTileset("veins_w1_6_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		}

		s_decorLayerMap[dType] = layer;
	}
	else
	{
		layer = s_decorLayerMap[dType];
		ts_d = layer->ts;
	}



	assert(layer != NULL);
	list<Vector2f> positions;
	//int minDistanceApart = 10;
	//int maxDistanceApart = 300;
	//int minPen = 20;
	//int maxPen = 200;
	double penLimit;

	Edge *curr = startEdge;
	double quant = 0;
	double lenCurr = length(startEdge->v1 - startEdge->v0);

	double travelDistance;
	double penDistance;
	int diffApartMax = maxApart - minApart;
	int diffPenMax = maxPen - minPen;
	int r;
	int rPen;
	double momentum;
	V2d pos;

	bool loopOver = false;
	V2d cn;

	rayMode = "decor";
	QuadTree *qt = terrainTree;

	assert(qt != NULL);

	while (true)
	{
		//cout << "running loop" << endl;
		r = rand() % diffApartMax;
		travelDistance = minApart + r;

		momentum = travelDistance;

		while (!approxEquals(momentum, 0))
		{
			if ((lenCurr - quant) > momentum)
			{
				quant += momentum;
				momentum = 0;
			}
			else
			{
				curr = curr->edge1;

				if (curr == startEdge)
				{
					loopOver = true;
					break;
				}
				else
				{
					momentum = momentum - (lenCurr - quant);
					quant = 0;
					lenCurr = length(curr->v1 - curr->v0);
				}
			}
		}

		if (loopOver)
			break;

		cn = curr->Normal();
		rcEdge = NULL;
		rayStart = curr->GetPoint(quant);

		rPen = rand() % diffPenMax;
		penDistance = minPen + rPen; //minpen times 2 cuz gotta account for the other side too

		rayEnd = rayStart - cn * (penDistance + minPen);
		ignoreEdge = curr;

		RayCast(this, qt->startNode, rayStart, rayEnd);

		if (rcEdge != NULL)
		{
			//V2d rcPos = rcEdge->GetPoint(rcQuant);
			//continue;
			/*if (length(rcPos - rayStart) < minPen || length( rcPos - rayEnd ) < minPen)
			{
			continue;
			}*/
			//penLimit = length(rcEdge->GetPoint(rcQuantity) - rayStart);

		}
		/*diffPenMax = maxPen;
		if( rcEdge != NULL )
		{
		penLimit = length( rcEdge->GetPoint( rcQuantity ) - rayStart );
		diffPenMax = (int)penLimit - minApart;
		if( diffPenMax == 0 || penLimit < 100 )
		continue;
		}*/

		/*rPen = rand() % diffPenMax;
		penDistance = minPen + rPen;*/

		pos = curr->GetPoint(quant) - curr->Normal() * penDistance;


		sf::Rect<double> testRect;
		testRect.left = pos.x - ts_d->tileWidth / 2;
		testRect.top = pos.y - ts_d->tileHeight / 2;
		testRect.width = ts_d->tileWidth;
		testRect.height = ts_d->tileHeight;

		if (IsEmptyRect(testRect))
		{
			positions.push_back(Vector2f(pos.x, pos.y));
			DecorRect *dr = new DecorRect(testRect);
			currDecorRects.push_back(dr);
			decorTree->Insert(dr);
		}
		//will have to do a raycast soon. ignore for now
		//curr = curr->edge1;
	}

	if (positions.size() == 0)
		return NULL;

	DecorExpression *expr = new DecorExpression(positions, layer);

	decorTree->Clear();

	return expr;
}

int TerrainRender::GetBorderQuadIntersect(int tileWidth)
{
	switch (tileWidth)
	{
	case 16:
		return 4;//4;
		break;
	case 32:
		return 4;//10;
		break;
	case 128:
		return 10;//20;
		break;
	default:
		assert(0);
		break;
	}
}

int TerrainRender::GetBorderRealWidth(int tileWidth, bool border )
{
	int inter = GetBorderQuadIntersect(tileWidth);
	if (border)
		return tileWidth - inter;
	else
		return tileWidth - inter * 2;
}

EdgeType GetEdgeType(sf::Vector2<double> &dir)
{
	V2d norm(dir.y, -dir.x);
	return GetEdgeNormalType(norm);
}

EdgeType GetEdgeNormalType(sf::Vector2<double> &norm)
{
	if (norm.y == 0)
	{
		return E_WALL;
	}
	else if (norm.x == 0)
	{
		return E_FLAT_GROUND;
	}
	else if ( abs( norm.y ) <= STEEP_THRESH)
	{
		if (norm.y > 0)
		{
			return E_STEEP_CEILING;
		}
		else
		{
			return E_STEEP_GROUND;
		}
	}
	else
	{
		if (norm.y > 0)
		{
			return E_SLOPED_CEILING;
		}
		else
		{
			return E_SLOPED_GROUND;
		}
	}
}

void TerrainRender::Draw(sf::RenderTarget *target)
{
	if(totalNumBorderQuads > 0 )
		target->draw(borderVA, totalNumBorderQuads * 4, sf::Quads, ts_border->texture);

	DrawDecor(target);
}

EdgeType GetEdgeTransType(Edge *e)
{
	EdgeType currType = GetEdgeType(normalize(e->v1 - e->v0));
	EdgeType prevType = GetEdgeType(normalize(e->edge0->v1 - e->edge0->v0));
	
	bool flat = currType == E_FLAT_GROUND || prevType == E_FLAT_GROUND;
	bool sloped = currType == E_SLOPED_GROUND || prevType == E_SLOPED_GROUND;
	bool steep = currType == E_STEEP_GROUND || prevType == E_STEEP_GROUND;
	bool slopedCeiling = currType == E_SLOPED_CEILING || prevType == E_SLOPED_CEILING;
	bool steepCeiling = currType == E_STEEP_CEILING || prevType == E_STEEP_CEILING;
	bool wall = currType == E_WALL || prevType == E_WALL;

	if (currType == prevType)
	{
		return currType;
	}
	
	if (flat && sloped)
	{
		return E_TRANS_FLAT_TO_SLOPED;
	}
	else if (flat && steep)
	{
		return E_TRANS_FLAT_TO_STEEP;
	}
	else if (flat && steepCeiling)
	{
		return E_TRANS_FLAT_TO_STEEP_CEILING;  
	}
	else if (flat && wall)
	{
		return E_TRANS_FLAT_TO_WALL;
	}
	else if (flat && slopedCeiling)
	{
		return E_TRANS_FLAT_TO_SLOPED_CEILING;
	}
	else if (sloped && steep)
	{
		return E_TRANS_SLOPED_TO_STEEP;
	}
	else if (sloped && steepCeiling)
	{
		return E_TRANS_SLOPED_TO_STEEP_CEILING;
	}
	else if (sloped && wall)
	{
		return E_TRANS_SLOPED_TO_WALL;
	}
	else if (sloped && slopedCeiling)
	{
		return E_TRANS_SLOPED_TO_SLOPED_CEILING;
	}
	else if (steep && steepCeiling)
	{
		return E_TRANS_STEEP_TO_STEEP_CEILING;
	}
	else if (steep && wall)
	{
		return E_TRANS_STEEP_TO_WALL;
	}
	else if (steep && slopedCeiling)
	{
		return E_TRANS_STEEP_TO_SLOPED_CEILING;
	}
	else if (wall && steep)
	{
		return E_TRANS_WALL_TO_STEEP_CEILING;
	}
	else if (steepCeiling && slopedCeiling)
	{
		return E_TRANS_STEEP_CEILING_TO_SLOPED_CEILING;
	}
	else if (wall && slopedCeiling)
	{
		return E_TRANS_WALL_TO_SLOPED_CEILING;
	}

	assert(0);
}

double TerrainRender::GetExtraForInward(Edge *e)
{
	
	double factor = 40.0;
	V2d currDir = normalize(e->v1 - e->v0);
	EdgeType test = GetEdgeType(currDir);
	V2d prevDir = normalize(e->edge0->v0 - e->v0);
	double cDiff = GetVectorAngleDiffCCW(currDir, prevDir);
	bool turnInward = cDiff < PI;

	if (turnInward)
	{
		return factor * (PI - cDiff) / PI;
	}
	else
	{
		return 0;
	}
}

void TerrainRender::HandleRayCollision(Edge *edge,
	double edgeQuantity, double rayPortion)
{
	if (edge == ignoreEdge)
	{
		return;
	}

	if (rayPortion < rcPortion)
	{
		rcEdge = edge;
		rcPortion = rayPortion;
		rcQuant = edgeQuantity;
	}

	/*if (edge == ignoreEdge || edge->edge1 == ignoreEdge )
		return;

	if (rayPortion < rcPortion)
	{
		rcEdge = edge;
		rcPortion = rayPortion;
		rcQuant = edgeQuantity;
	}*/
}

double TerrainRender::GetSubForOutward(Edge *e)
{
	double factor = 160.0;
	V2d currDir = normalize(e->v1 - e->v0);
	EdgeType test = GetEdgeType(currDir);
	V2d prevDir = normalize(e->edge0->v0 - e->v0);
	double cDiff = GetVectorAngleDiffCCW(currDir, prevDir);
	bool turnOutward = cDiff > PI * 1.5 + .1;

	V2d realPrevDir = normalize(e->edge0->v1 - e->edge0->v0);

	if (turnOutward)
	{
		//V2d norm(currDir.y, -currDir.x);
		//V2d pNorm(realPrevDir.y, -realPrevDir.x);
		//V2d invNorm = -normalize(norm + pNorm);

		//V2d test = e->v0 + norm * 48.0;
		//LineIntersection li = lineIntersection(test, test + invNorm, e->v0, e->v1);

		//if (li.parallel)
		//{
		//	int testff = 4;
		//}
		//assert(!li.parallel);

		////V2d centerPos = li.position;
		//V2d currInterPos = li.position;
		////V2d center = currInterPos - norm * 48.0;

		//double len = length(currInterPos - e->v0);
		//return len;
		return factor * (cDiff - PI * 1.5 ) / (PI / 2);
	}
	else
	{
		return 0;
	}
}

DecorExpression::DecorExpression(std::list<sf::Vector2f> &pointList,
	DecorLayer *p_layer)
	:layer(p_layer)
{
	int numBushes = pointList.size();
	//cout << "numBushes: " << numBushes << endl;
	Tileset *ts = layer->ts;

	va = new VertexArray(sf::Quads, numBushes * 4);
	VertexArray &VA = *va;

	IntRect subRect = ts->GetSubRect(0);
	list<Vector2f>::iterator posIt;
	if (numBushes > 0)
		posIt = pointList.begin();

	Vector2f p;
	for (int i = 0; i < numBushes; ++i)
	{
		p = (*posIt);
		//cout << "i: " << i << ", p: " <<  p.x << ", " << p.y << endl;
		VA[i * 4 + 0].position = Vector2f(p.x - subRect.width / 2, p.y - subRect.height / 2);
		VA[i * 4 + 1].position = Vector2f(p.x + subRect.width / 2, p.y - subRect.height / 2);
		VA[i * 4 + 2].position = Vector2f(p.x + subRect.width / 2, p.y + subRect.height / 2);
		VA[i * 4 + 3].position = Vector2f(p.x - subRect.width / 2, p.y + subRect.height / 2);

		/*VA[i*4+0].color= Color::Red;
		VA[i*4+1].color= Color::Red;
		VA[i*4+2].color= Color::Red;
		VA[i*4+3].color= Color::Red;*/

		VA[i * 4 + 0].texCoords = Vector2f(subRect.left, subRect.top);
		VA[i * 4 + 1].texCoords = Vector2f(subRect.left + subRect.width, subRect.top);
		VA[i * 4 + 2].texCoords = Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
		VA[i * 4 + 3].texCoords = Vector2f(subRect.left, subRect.top + subRect.height);

		++posIt;
	}
}

DecorExpression::~DecorExpression()
{
	delete va;
}

DecorLayer::DecorLayer(Tileset *p_ts, int p_animLength, int p_animFactor, int p_startTile, int p_loopWait)
	:ts(p_ts), frame(0), animLength(p_animLength), startTile(p_startTile), animFactor(p_animFactor),
	loopWait(p_loopWait)
{

}

void DecorLayer::Update()
{
	++frame;
	if (frame == animLength * animFactor + loopWait)
	{
		frame = 0;
	}
}

void DecorExpression::UpdateSprites()
{
	int numBushes = va->getVertexCount() / 4;

	Tileset *ts_bush = layer->ts;
	int frame = max(layer->frame - layer->loopWait, 0);
	int animLength = layer->animLength;
	int animFactor = layer->animFactor;

	VertexArray &bVA = *va;

	IntRect subRect = ts_bush->GetSubRect((layer->startTile + frame) / animFactor);

	for (int i = 0; i < numBushes; ++i)
	{
		bVA[i * 4 + 0].texCoords = Vector2f(subRect.left, subRect.top);
		bVA[i * 4 + 1].texCoords = Vector2f(subRect.left + subRect.width, subRect.top);
		bVA[i * 4 + 2].texCoords = Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
		bVA[i * 4 + 3].texCoords = Vector2f(subRect.left, subRect.top + subRect.height);
	}
}

void TerrainRender::DrawDecor(sf::RenderTarget *target)
{
	for (list<DecorExpression*>::iterator it = decorExprList.begin();
		it != decorExprList.end(); ++it)
	{
		Tileset *ts = (*it)->layer->ts;
		target->draw(*(*it)->va, ts->texture);
	}	
}

void TerrainRender::UpdateDecorLayers()
{
	for (map<DecorType, DecorLayer*>::iterator mit =
		s_decorLayerMap.begin(); mit != s_decorLayerMap.end();
		++mit)
	{
		(*mit).second->Update();
	}
}