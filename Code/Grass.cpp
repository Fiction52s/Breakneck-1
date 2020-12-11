#include "Grass.h"
#include "GameSession.h"
#include "EditorTerrain.h"

using namespace std;
using namespace sf;

sf::Color Grass::GetColor( int gType )
{
	Color c = Color::White;
	/*switch (gType)
	{
	case Grass::JUMP:
		c = Color::White;
		break;
	case Grass::BOUNCE:
		c = Color::Red;
		break;
	case Grass::GrassType::UNTECHABLE:
		c = Color::Yellow;
		break;
	case Grass::GrassType::KILL:
		c = Color::Black;
		break;
	}*/

	return c;
}

Grass::Grass(Tileset *p_ts_grass, int p_tileIndex,
	V2d &p_pos, TerrainPolygon *p_poly, GrassType gType)
	:tileIndex(p_tileIndex), prev(NULL), next(NULL), visible(true),
	ts_grass(p_ts_grass), poly(p_poly), pos(p_pos), radius(128 / 2.0 - 20)
{
	grassType = gType;

	explosion.isCircle = true;
	explosion.rw = 64;
	explosion.rh = 64;

	explosion.globalPosition = pos;//(A + B + C + D) / 4.0;

	explodeFrame = 0;
	explodeLimit = 20;
	exploding = false;

	aabb.left = pos.x - radius;
	aabb.top = pos.y - radius;
	int right = pos.x + radius;
	int bot = pos.y + radius;

	aabb.width = right - aabb.left;
	aabb.height = bot - aabb.top;
}

void Grass::HandleQuery(QuadTreeCollider *qtc)
{
	qtc->HandleEntrant(this);
}

bool Grass::IsTouchingBox(const Rect<double> &r)
{
	return IsQuadTouchingCircle(V2d(r.left, r.top), V2d(r.left + r.width, r.top),
		V2d(r.left + r.width, r.top + r.height), V2d(r.left, r.top + r.height),
		pos, radius);
}

bool Grass::IsTouchingCircle(V2d &otherPos, double otherRad)
{
	double distSqr = lengthSqr(pos - otherPos);
	double radSqr = pow(radius + otherRad, 2);
	return distSqr <= radSqr;
}

void Grass::Reset()
{
	next = NULL;
	prev = NULL;
	exploding = false;
	visible = true;
}

void Grass::Update()
{
	if (visible && exploding)
	{
		if (explodeFrame == 0)
		{
			SetVisible(false);
		}

		++explodeFrame;
		if (explodeFrame == explodeLimit)
		{
			explodeFrame = 0;

			GameSession *game = GameSession::GetSession();
			if (game != NULL)
			{
				game->RemoveGravityGrassFromExplodeList(this);
			}
			
		}
	}
}

void Grass::SetVisible(bool p_visible)
{
	visible = p_visible;

	sf::Vertex *quad = poly->grassVA + tileIndex * 4;

	int size = ts_grass->tileWidth;
	if (visible)
	{
		quad[0].position = Vector2f(pos.x - size, pos.y - size);
		quad[1].position = Vector2f(pos.x + size, pos.y - size);
		quad[2].position = Vector2f(pos.x + size, pos.y + size);
		quad[3].position = Vector2f(pos.x - size, pos.y + size);
	}
	else
	{
		quad[0].position = Vector2f(0, 0);
		quad[1].position = Vector2f(0, 0);
		quad[2].position = Vector2f(0, 0);
		quad[3].position = Vector2f(0, 0);
	}
}