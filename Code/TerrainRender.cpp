#include "TerrainRender.h"
#include "Tileset.h"
#include <assert.h>
#include "Physics.h"

using namespace sf;
using namespace std;



void TerrainRender::GenerateCenterMesh()
{

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

void TerrainRender::GenerateBorderMesh( QuadTree * qt )
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

	assert( qt != NULL );

	//int tw = ts->tileWidth;//128;//64;
	//int th = 512;
	//int numTotalQuads = 0;
	Triple totalQuads;
	//double intersect = 10;//tw / 6.0;//48;
	//double extraLength = 0;//32.0;
	Edge *te = startEdge;//edges[currentEdgeIndex];

	map<Edge*, Triple> numQuadMap[E_SLOPED_CEILING+1];



	/*do
	{
		double len = length(te->v1 - te->v0);
		while (len > 0)
		{

		}
	}
	while (te != startEdge);*/

	do
	{
		V2d eNorm = te->Normal();
		EdgeType eType = GetEdgeNormalType(eNorm);
		//int valid = ValidEdge( eNorm );
		//if( valid != -1 )//eNorm.x == 0 )
		{
			double len = length(te->v1 - te->v0);// +extraLength * 2;

			double fullParts = len / 128.0;
			int numFullparts = fullParts;

			//int numQuads = 0;
			Triple numQuads;
			
			//if( len < GetBorderRealWidth( ))

			while (len > 0)
			{
				if (len > 128)
				{
					len -= 128;
					numQuads.numQuads128++;
				}
				else if (len > 32)
				{
					len -= 32;
					numQuads.numQuads32++;
				}
				else if (len > 16)
				{
					len -= 16;
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
	if (totalNumBorderQuads == 0)
	{
		//totalNumBorderQuads = 0;
		borderVA = NULL;
		return; //NULL;
	}

	
	borderVA = new Vertex[totalNumBorderQuads * 4];

	int totals[E_SLOPED_CEILING+1];
	memset(totals, 0, sizeof(totals));

	int current[E_SLOPED_CEILING + 1];
	memset(current, 0, sizeof(current));

	for (int i = 0; i < E_SLOPED_CEILING + 1; ++i)
	{
		auto &qmRef = numQuadMap[i];
		for( auto it = qmRef.begin(); it != qmRef.end(); ++it )
		{
			totals[i] += (*it).second.GetTotal();
		}
	}
	sf::Vertex *bva[E_SLOPED_CEILING + 1];
	int start = 0;
	for (int i = 0; i < E_SLOPED_CEILING + 1; ++i)
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
			

			V2d startInner = te->v0 - along * extraLength - other * in;
			V2d startOuter = te->v0 - along * extraLength + other * out;

			double startAlong = -extraLength;
			

			int currTotal = numQuads.GetTotal();
			Vertex *currBVA = bva[et] + current[et] * 4;
			
			for( int i = 0; i < currTotal; ++i )
			{
				//worldNum * 5
				//int valid = ValidEdge( eNorm );
				//add (worldNum * 5) to realIndex to get the correct borders
				//int realIndex = valid;// *4 + varietyCounter;//32 + varietyCounter;
				//cout << "real Index: " << realIndex << ", valid: " << valid << ", variety: " << varietyCounter << endl;
				//IntRect sub = ts->GetSubRect( realIndex );

				if (numQuads.numQuads128 > 0)
				{
					tw = 128;
					numQuads.numQuads128--;
				}
				else if (numQuads.numQuads32 > 0)
				{
					tw = 32;
					numQuads.numQuads32--;
				}
				else if( numQuads.numQuads16 > 0 )
				{
					tw = 16;
					numQuads.numQuads16--;
				}
				else
				{
					assert(0);
				}
				double endAlong = startAlong + tw;
				IntRect sub = GetBorderSubRect(tw, et, 0);
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
				
				
				

				V2d currStartInner = startInner + startAlong * along;
				V2d currStartOuter = startOuter + startAlong * along;
				V2d currEndInner = startInner + endAlong * along;
				V2d currEndOuter = startOuter + endAlong * along;
						
				double realHeight0 = 64;//256;//in;//sub.height;
				double realHeight1 = 64;//256;//in;//sub.height;
				
				double d0 = dot( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );
				double c0 = cross( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );

				double d1 = dot( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );
				double c1 = cross( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );

				//if( d0 <= 0

				//rcEdge = NULL;
				//rayIgnoreEdge = te;
				//rayStart = te->v0 - along * extraLength + ( startAlong ) * along;
				//rayEnd = currStartInner;//te->v0 + (double)i * quadWidth * along - other * in;
				//RayCast( this, qt->startNode, rayStart, rayEnd );


				//start ray
				//if( rcEdge != NULL )
				{
					//currStartInner = rcEdge->GetPoint( rcQuantity );
					//realHeight0 = length( currStartInner - currStartOuter );
				}

				//rcEdge = NULL;
				//rayStart = te->v0 - along * extraLength + ( endAlong ) * along;
				//rayEnd = currEndInner;
				//RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				//if( rcEdge != NULL )
				{
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
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
				currBVA[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);//realHeight1);
				currBVA[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);//+ realHeight0);

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

				startAlong += tw;//tw - intersect;
			}

			current[et] += currTotal;
			//extra += numQuads * 4;
		}

		
		te = te->edge1;
	}
	while( te != startEdge );

	
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
		if (et < E_WALL)
		{
			ir.top = 7 * 64;
			ir.left = 128 * 2 + 64 * et + 16 * var;
		}
		else
		{
			ir.top = 8 * 64;
			ir.left = 64 * (et-E_WALL) + 16 * var;
		}
		break;
	}
	case 32:
	{
		if (et < E_WALL)
		{
			ir.top = 6 * 64;
			ir.left = 128 * et + 32 * var;
		}
		else
		{
			ir.top = 7 * 64;
			ir.left = 128 * (et - E_WALL) + 32 * var;
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

int TerrainRender::GetBorderQuadIntersect(int tileWidth)
{
	switch (tileWidth)
	{
	case 16:
		return 4;
		break;
	case 32:
		return 10;
		break;
	case 128:
		return 20;
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
}