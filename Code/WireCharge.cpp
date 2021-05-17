#include "Wire.h"
#include "Actor.h"
#include "GameSession.h"
#include <iostream>
#include <assert.h>
#include "Enemy.h"
#include "Grass.h"

using namespace sf;
using namespace std;

WireCharge::WireCharge(Wire*w, int vIndex)
	:vaIndex(vIndex), wire(w), edgeIndex(-1)
{
	prev = NULL;
	next = NULL;
	Reset();
}

void WireCharge::Reset()
{
	ClearSprite();
	action = INACTIVE;
}

//actor should change the offset every frame based on its info. need a before movement wire position and a post movement wire position consistently
void WireCharge::UpdatePhysics()
{
	int numPoints = wire->numPoints;
	int currPoints = edgeIndex;
	double momentum = wire->retractSpeed / NUM_STEPS;
	while (!approxEquals(momentum, 0))
	{
		if (wire->right)
		{
			if (edgeQuantity > momentum)
			{
				edgeQuantity -= momentum;
				//cout << "changed: " << edgeQuantity << ", moment: " << momentum << endl;
				momentum = 0;
			}
			else
			{
				//cout << "OTHER: " << edgeQuantity << ", moment: " << momentum << endl;
				momentum = momentum - edgeQuantity;
				edgeIndex--;
				if (edgeIndex == -1)
				{
					//cout << "DEACTIVA" << endl;
					action = INACTIVE;
					ClearSprite();
					

					//assert( wire->activeChargeList == NULL );
					return;
				}
				else if (edgeIndex == 0)
				{
					//cout << "blah anchor" << endl;
					edgeQuantity = length(wire->points[edgeIndex].pos - wire->anchor.pos);
				}
				else
				{
					//cout << "edgeIndex: " << edgeIndex << endl;
					edgeQuantity = length(wire->points[edgeIndex].pos - wire->points[edgeIndex - 1].pos);
				}
			}

			if (edgeIndex == 0)
			{
				V2d dir = normalize(wire->points[0].pos - wire->anchor.pos);
				position = wire->anchor.pos + dir * edgeQuantity;
			}
			else
			{
				V2d dir = normalize(wire->points[edgeIndex].pos - wire->points[edgeIndex - 1].pos);
				position = wire->points[edgeIndex - 1].pos + dir * edgeQuantity;
			}
		}
		else
		{
			if (edgeQuantity > momentum)
			{
				edgeQuantity -= momentum;
				momentum = 0;
			}
			else
			{
				int numPoints = wire->numPoints;
				momentum = momentum - edgeQuantity;
				edgeIndex--;
				if (edgeIndex == -1)
				{
					action = INACTIVE;
					ClearSprite();
					wire->DeactivateWireCharge(this);
					return;
				}
				else if (edgeIndex == 0)
				{
					//cout << "zero" << endl;
					edgeQuantity = length(wire->points[numPoints - 1].pos - wire->retractPlayerPos);
					//fuseQuantity = length( points[fusePointIndex].pos - anchor.pos );
				}
				else
				{
					//cout << "other: " << fusePointIndex << endl;
					//fuseQuantity = length( points[fusePointIndex].pos - points[fusePointIndex-1].pos );
					edgeQuantity = length(wire->points[(numPoints - 1) - edgeIndex].pos
						- wire->points[(numPoints - 1) - (edgeIndex - 1)].pos);
					//cout << "length: " << fuseQuantity << endl;
				}
			}


			if (edgeIndex == 0)
			{
				V2d dir = normalize(wire->points[wire->numPoints - 1].pos - wire->retractPlayerPos);//wire->anchor.pos );
				position = wire->retractPlayerPos + dir * edgeQuantity;
			}
			else
			{
				V2d dir = normalize(wire->points[(numPoints - 1) - edgeIndex].pos - wire->points[(numPoints - 1) - (edgeIndex - 1)].pos);
				position = wire->points[(numPoints - 1) - (edgeIndex - 1)].pos + dir * edgeQuantity;
			}

			//cout << "position: " << position.x << ", " << position.y << endl;
			//position = //wire->points[edgeIndex].pos;
			//position = wire->retractPlayerPos;
		}
	}
}

void WireCharge::UpdatePrePhysics()
{
}

void WireCharge::HitEnemy()
{
	Reset();
	wire->DeactivateWireCharge(this);
}

void WireCharge::UpdateSprite()
{
	if (action == INACTIVE)
	{
		ClearSprite();
		//cout << "updating inactive sprite" << endl;
	}
	else
	{
		//cout << "wire charge sprite update" << endl;
		VertexArray &va = wire->chargeVA;
		IntRect subRect = wire->ts_wireCharge->GetSubRect(0);//frame / animFactor );
															 //if( owner->GetPlayer( 0 )->position.x < position.x )
															 //{
															 //	subRect.left += subRect.width;
															 //	subRect.width = -subRect.width;
															 //}
		va[vaIndex * 4 + 0].texCoords = Vector2f(subRect.left,
			subRect.top);
		va[vaIndex * 4 + 1].texCoords = Vector2f(subRect.left
			+ subRect.width,
			subRect.top);
		va[vaIndex * 4 + 2].texCoords = Vector2f(subRect.left
			+ subRect.width,
			subRect.top + subRect.height);
		va[vaIndex * 4 + 3].texCoords = Vector2f(subRect.left,
			subRect.top + subRect.height);

		//va[vaIndex*4+0].color = Color::Red;
		//va[vaIndex*4+1].color = Color::Red;
		//va[vaIndex*4+2].color = Color::Red;
		//va[vaIndex*4+3].color = Color::Red;

		Vector2f p(position.x, position.y);

		Vector2f spriteSize = Vector2f(subRect.width / 2, subRect.height / 2);
		//spriteSize *= 10.f;
		va[vaIndex * 4 + 0].position = p + Vector2f(-spriteSize.x, -spriteSize.y);
		va[vaIndex * 4 + 1].position = p + Vector2f(spriteSize.x, -spriteSize.y);
		va[vaIndex * 4 + 2].position = p + Vector2f(spriteSize.x, spriteSize.y);
		va[vaIndex * 4 + 3].position = p + Vector2f(-spriteSize.x, spriteSize.y);
	}
}

void WireCharge::ClearSprite()
{
	VertexArray &va = wire->chargeVA;
	va[vaIndex * 4 + 0].position = Vector2f(0, 0);
	va[vaIndex * 4 + 1].position = Vector2f(0, 0);
	va[vaIndex * 4 + 2].position = Vector2f(0, 0);
	va[vaIndex * 4 + 3].position = Vector2f(0, 0);
}