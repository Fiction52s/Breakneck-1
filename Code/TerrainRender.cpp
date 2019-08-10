#include "TerrainRender.h"
#include "Tileset.h"
#include <assert.h>
#include "Physics.h"
#include "Movement.h"
#include <iostream>

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
	borderVA = NULL;
	centerVA = NULL;
	tdInfo = NULL;
}

TerrainRender::~TerrainRender()
{
	delete decorTree;

	for (auto it = decorExprList.begin(); it != decorExprList.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = currDecorRects.begin(); it != currDecorRects.end(); ++it)
	{
		delete (*it);
	}

	if (borderVA != NULL)
		delete[] borderVA;
	if (centerVA != NULL)
		delete[] centerVA;
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
	/*bool useEdges = true;
	if (startEdge == NULL && startPoint != NULL)
	{
		useEdges = false;
	}*/

	Triple totalQuads;
	Edge *te = startEdge;//edges[currentEdgeIndex];

	map<Edge*, Triple> numQuadMap[E_WALL +1];
	//map<TerrainPoint*, Triple> numQuadMapPoints[E_WALL + 1];

	list<Edge*> transEdges;
	//list<TerrainPoint*> transPoints;

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

		EdgeType eType = GetEdgeNormalType(eNorm);
		
		{
			
			double len = length(te->v1 - te->v0);// +3 * 2.0;// +extraLength * 2;
			//len -= GetSubForOutward(te);
			//len -= GetSubForOutward(te->edge1);
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
				else
				{
					numQuads.numQuads32++;
					break;
				}
			}
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
	int tw;

	int extra = 0;
	te = startEdge;
	int varietyCounter = 0;
	double extraLength = 0;
	do
	{
		V2d eNorm = te->Normal();
		{
			double len = length(te->v1 - te->v0);
			//len -= GetSubForOutward(te);
			//len -= GetSubForOutward(te->edge1);
			EdgeType et = GetEdgeNormalType(eNorm);
			Triple numQuads = numQuadMap[et][te];
			if (numQuads.GetTotal() == 0)
			{
				te = te->edge1;
				continue;
			}

			V2d along = normalize( te->v1 - te->v0 );
			Vector2f fAlong = Vector2f(along);
			V2d other( along.y, -along.x );
			Vector2f fOther = Vector2f(other);
			
			double out = 16;//40;//16;
			double in = 64 - out;//256 - out;//; - out;
			
			double inwardExtra = GetExtraForInward(te);
			double nInwardExtra = GetExtraForInward(te->edge1);

			V2d startInner = te->v0 - along * extraLength - other * in;
			V2d startOuter = te->v0 - along * extraLength + other * out;

			double startAlong = -inwardExtra;/*GetSubForOutward(te) - *///inwardExtra;
			double trueEnd = len + nInwardExtra;// -nInwardExtra;
			

			int currTotal = numQuads.GetTotal();
			Vertex *currBVA = bva[et] +current[et] * 4;
			
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

			

			V2d curr = normalize(te->v1 - te->v0);
			V2d prev = normalize(te->edge0->v0 - te->v0);
			V2d bisector = normalize(curr + prev);
			bool isAcute = IsAcute(te);
			bool nextAcute = IsAcute(te->edge1);
			V2d nCurr = normalize(te->edge1->v1 - te->v1);
			V2d nPrev = normalize(te->v0 - te->v1);
			V2d nBisector = normalize(nCurr + nPrev);

			for( int i = 0; i < currTotal; ++i )
			{
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
				
				double trueAlong = startAlong;
				if (startAlong > 0)
				{
					//trueAlong -= GetBorderQuadIntersect(tw);
				}

				V2d currStartInner = startInner + trueAlong * along;
				V2d currStartOuter = startOuter + trueAlong * along;
				V2d currEndInner = startInner + endAlong * along;
				V2d currEndOuter = startOuter + endAlong * along;
						
				double realHeightLeft = 64.0;//sub.height - out;//256;//in;//sub.height;
				double realHeightRight = 64.0;//sub.height - out;//256;//in;//sub.height;

				double d0 = dot( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );
				double c0 = cross( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );

				double d1 = dot( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );
				double c1 = cross( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );

				//SetRectColor(currBVA + i * 4, Color(Color::Black));
				if (isAcute )
				{
					//SetRectColor(currBVA + i * 4, Color(Color::Red));
					LineIntersection li = lineIntersection(currStartInner,
						currStartOuter, te->v0, te->v0 + bisector);//te->edge0->v0 );
					assert(!li.parallel);
					if (!li.parallel)
					{
						double testLength = dot(li.position - currStartOuter, normalize(currStartInner - currStartOuter));//(li.position - currStartOuter);
						assert(testLength >= 0);
						if (testLength < realHeightLeft)
						{
							//testLength = 64;
							double diffLen = realHeightLeft - testLength;
							realHeightLeft = testLength;
							currStartInner += diffLen * normalize(startOuter - startInner);
							//currEndInner += diffLen * normalize(startOuter - startInner);
						}
					}

					li = lineIntersection(currEndInner,
						currEndOuter, te->v0, te->v0 + bisector);//te->edge0->v0 );
					assert(!li.parallel);
					if (!li.parallel)
					{
						//double testLength = length(li.position - currEndOuter);
						double testLength = dot(li.position - currStartOuter, normalize(currStartInner - currStartOuter));//(li.position - currStartOuter);
						assert(testLength >= 0);
						if (testLength < realHeightRight)
						{
							double diffLen = realHeightRight - testLength;
							realHeightRight = testLength;
							currEndInner += diffLen * normalize(startOuter - startInner);
						}
					}
				}
				if (nextAcute)
				//if( false )
				{
					bool middleSplit = false;
					LineIntersection liMiddle = lineIntersection(currStartInner,
						currEndInner, te->v1, te->v1 + nBisector);//te->edge1->v1);
					if (!liMiddle.parallel)
					{
						double md = dot(liMiddle.position - currStartInner, normalize(currEndInner - currStartInner));
						double cLen = length(currEndInner - currStartInner);
						if (md < cLen && md > 0 )
						{
							//currEndInner = liMiddle.position;
							//middleSplit = true;
							//SetRectColor(currBVA + i * 4, Color(Color::Red));
						}
					}



					if (!middleSplit)
					{

						LineIntersection li = lineIntersection(currStartInner,
							currStartOuter, te->v1, te->v1 + nBisector);//te->edge1->v1);
						assert(!li.parallel);
						if (!li.parallel)
						{
							double testLength = length(li.position - currStartOuter);
							if (testLength < realHeightLeft)
							{
								cout << "len: " << testLength << ", x: " << li.position.x << "." << currStartOuter.x << endl;
								double diffLen = realHeightLeft - testLength;

								realHeightLeft = testLength;
								currStartInner = li.position;//currStartOuter + normalize(startInner - startOuter) * realHeightLeft;

								//currStartInner += diffLen * normalize(startOuter - startInner);
								//cout << "nBisector: " << nBisector.x << ", " << nBisector.y << endl;
								cout << i << "left lipos: " << li.position.x << ", " << li.position.y << endl;
							}
						}

						li = lineIntersection(currEndInner,
							currEndOuter, te->v1, te->v1 + nBisector);//te->edge1->v1 );
						assert(!li.parallel);
						if (!li.parallel)
						{
							double testLength = length(li.position - currEndOuter);
							if (testLength < realHeightRight)
							{
								//testLength = 64;
								cout << "len: " << testLength << ", x: " << li.position.x << "." << currEndOuter.x << endl;
								double diffLen = realHeightRight - testLength;
								//realHeightMiddle = 32;
								realHeightRight = testLength;
								currEndInner += diffLen * normalize(startOuter - startInner);
								//currStartInner += diffLen * normalize(startOuter - startInner);
								cout << i << "right lipos: " << li.position.x << ", " << li.position.y << endl;
								//cout << "nBisector: " << nBisector.x << ", " << nBisector.y << endl;
							}
						}
					}
				}

				//currBVA[i * 4 + 2].color.a = 0;
				//currBVA[i * 4 + 3].color.a = 0;

				//realHeightLeft = min(realHeightLeft, realHeightRight);
				//realHeightRight = realHeightLeft;
				Vector2f a = Vector2f( currStartOuter.x, currStartOuter.y );
				Vector2f b = Vector2f( currStartInner.x, currStartInner.y );
				Vector2f c = Vector2f(currEndInner.x, currEndInner.y);
				Vector2f d = Vector2f(currEndOuter.x, currEndOuter.y);
				Vector2f m = (a + d) / 2.f;

				currBVA[i * 4 + 0].position = a;
				currBVA[i * 4 + 1].position = d;//m + fAlong * middleDiff;
				currBVA[i * 4 + 2].position = c;// +fAlong * middleDiff - fOther * realHeightMiddle;
				currBVA[i * 4 + 3].position = b;

				/*currBVA[i * 8 + 4].position = currBVA[i * 8 + 1].position;
				currBVA[i * 8 + 5].position = d;
				currBVA[i * 8 + 6].position = c;
				currBVA[i * 8 + 7].position = currBVA[i * 8 + 2].position;*/
				float width = length(currEndInner - currStartInner);
				currBVA[i * 4 + 0].texCoords = Vector2f( sub.left, sub.top );
				currBVA[i * 4 + 1].texCoords = Vector2f( sub.left + width, sub.top );
				currBVA[i * 4 + 2].texCoords = Vector2f(sub.left + width, sub.top + realHeightRight);
				currBVA[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + realHeightLeft);

				/*currBVA[i * 8 + 4].texCoords = Vector2f(sub.left + sub.width / 2 + middleDiff, sub.top);
				currBVA[i * 8 + 5].texCoords = Vector2f(sub.left + sub.width, sub.top);
				currBVA[i * 8 + 6].texCoords = Vector2f(sub.left + sub.width, sub.top + realHeightRight);
				currBVA[i * 8 + 7].texCoords = Vector2f(sub.left + sub.width / 2 + middleDiff, sub.top + realHeightLeft);*/

				if (realHeightLeft < 64 || realHeightRight < 64)
				{
					//SetRectColor(currBVA + i * 4, Color(Color::Red));
				//	SetRectColor(currBVA + i * 8 + 4, Color(Color::Yellow));
				}
				else
				{
					//SetRectColor(currBVA + i * 4, Color(Color::Black));
				//	SetRectColor(currBVA + i * 8 + 4, Color(Color::Magenta));
				}

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
	if (tdInfo != NULL)
	{
		assert(tdInfo != NULL);
		int numDecors = tdInfo->numDecors;
		DecorExpression *expr;
		for (int i = 0; i < numDecors; ++i)
		{
			expr = CreateDecorExpression(tdInfo->decors[i], 0, startEdge);
			if (expr != NULL)
				AddDecorExpression(expr);
		}
	}
	else
	{

	}

	/*for (int i = 0; i < 6; ++i)
	{
		DecorExpression *expr = CreateDecorExpression(DecorType(D_W1_VEINS1 + i), 0, startEdge);
		if (expr != NULL)
			AddDecorExpression(expr);
	}

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
		AddDecorExpression(exprPlantRock);*/
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
		//int GameSession::TerrainPiece::bushFrame = 0;
		//int GameSession::TerrainPiece::bushAnimLength = 20;
		//int GameSession::TerrainPiece::bushAnimFactor = 8;

		
		switch (dType)
		{
		case D_W1_BUSH_NORMAL:
			ts_d = tMan->GetTileset("Env/bush_01_64x64.png", 64, 64);
			layer = new DecorLayer(ts_d, 20, 8);
			break;
		case D_W1_ROCK_1:
			ts_d = tMan->GetTileset("Env/rock_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_2:
			ts_d = tMan->GetTileset("Env/rock_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_3:
			ts_d = tMan->GetTileset("Env/rock_1_03_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_PLANTROCK:
			ts_d = tMan->GetTileset("Env/bush_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_GRASSYROCK:
			ts_d = tMan->GetTileset("Env/bush_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_VEINS1:
			ts_d = tMan->GetTileset("Env/veins_w1_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS2:
			ts_d = tMan->GetTileset("Env/veins_w1_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS3:
			ts_d = tMan->GetTileset("Env/veins_w1_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS4:
			ts_d = tMan->GetTileset("Env/veins_w1_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS5:
			ts_d = tMan->GetTileset("Env/veins_w1_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS6:
			ts_d = tMan->GetTileset("Env/veins_w1_6_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS1:
			ts_d = tMan->GetTileset("Env/veins_w2_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS2:
			ts_d = tMan->GetTileset("Env/veins_w2_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS3:
			ts_d = tMan->GetTileset("Env/veins_w2_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS4:
			ts_d = tMan->GetTileset("Env/veins_w2_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS5:
			ts_d = tMan->GetTileset("Env/veins_w2_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W2_VEINS6:
			ts_d = tMan->GetTileset("Env/veins_w2_6_512x512.png", 512, 512);
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

		rcPortion = 9999999;
		RayCast(this, qt->startNode, rayStart, rayEnd);

		if (rcEdge != NULL)
		{
			V2d rcPos = rcEdge->GetPoint(rcQuant);
			continue;

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
		return 8;//10;
		break;
	case 128:
		return 16;//20;
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
	
	double factor = 0.0;//40;//40.0;
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

	double len = length(edge->GetPoint(edgeQuantity) - rayStart);
	if( rcEdge == NULL || len < rcPortion )
	{
		rcEdge = edge;
		rcPortion = len;
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

bool TerrainRender::IsAcute( Edge *e ) //edge and its previous edge
{
	V2d currDir = normalize(e->v1 - e->v0);
	V2d prevDir = normalize(e->edge0->v0 - e->v0);

	return dot(prevDir, currDir) > .6 && cross( prevDir, currDir ) < 0;
	
	/*double factor = 160.0;
	
	EdgeType test = GetEdgeType(currDir);
	
	double cDiff = GetVectorAngleDiffCCW(currDir, prevDir);
	double extra = PI * .3;
	bool turnOutward = cDiff > PI * 1.5 + extra;

	V2d realPrevDir = normalize(e->edge0->v1 - e->edge0->v0);

	return turnOutward;*/
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

DecorLayer::~DecorLayer()
{
	int x = 5;
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

V2d TerrainRender::GetBisector(Edge *e)
{
	V2d curr = normalize(e->v1 - e->v0);
	V2d prev = normalize(e->edge0->v0 - e->v0);
	return normalize(curr + prev);
	
}

DecorType TerrainRender::GetDecorType(const std::string &dStr)
{
	if (dStr == "D_W1_BUSH_NORMAL")
	{
		return D_W1_BUSH_NORMAL;
	}
	else if (dStr == "D_W1_ROCK_1")
	{
		return D_W1_ROCK_1;
	}
	else if (dStr == "D_W1_ROCK_2")
	{
		return D_W1_ROCK_2;
	}
	else if (dStr == "D_W1_ROCK_3")
	{
		return D_W1_ROCK_3;
	}
	else if (dStr == "D_W1_PLANTROCK")
	{
		return D_W1_PLANTROCK;
	}
	else if (dStr == "D_W1_VEINS1")
	{
		return D_W1_VEINS1;
	}
	else if (dStr == "D_W1_VEINS2")
	{
		return D_W1_VEINS2;
	}
	else if (dStr == "D_W1_VEINS3")
	{
		return D_W1_VEINS3;
	}
	else if (dStr == "D_W1_VEINS4")
	{
		return D_W1_VEINS4;
	}
	else if (dStr == "D_W1_VEINS5")
	{
		return D_W1_VEINS5;
	}
	else if (dStr == "D_W1_VEINS6")
	{
		return D_W1_VEINS6;
	}
	else if (dStr == "D_W1_GRASSYROCK")
	{
		return D_W1_GRASSYROCK;
	}
	else if (dStr == "D_W2_VEINS1")
	{
		return D_W2_VEINS1;
	}
	else if (dStr == "D_W2_VEINS2")
	{
		return D_W2_VEINS2;
	}
	else if (dStr == "D_W2_VEINS3")
	{
		return D_W2_VEINS3;
	}
	else if (dStr == "D_W2_VEINS4")
	{
		return D_W2_VEINS4;
	}
	else if (dStr == "D_W2_VEINS5")
	{
		return D_W2_VEINS5;
	}
	else if (dStr == "D_W2_VEINS6")
	{
		return D_W2_VEINS6;
	}
	else
	{ 
		assert(0);
		return D_W1_BUSH_NORMAL;
	}
}

TerrainDecorInfo::TerrainDecorInfo(int p_numDecors)
{
	numDecors = p_numDecors;
	assert(numDecors != 0);
	decors = new DecorType[numDecors];
	percents = new int[numDecors];
	
}

TerrainDecorInfo::~TerrainDecorInfo()
{
	delete[] decors;
	delete[] percents;
}