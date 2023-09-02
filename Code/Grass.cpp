#include "Grass.h"
#include "GameSession.h"
#include "EditorTerrain.h"
#include <iostream>

using namespace std;
using namespace sf;

sf::Color Grass::GetColor()
{
	switch (grassType)
	{
	case DECELERATE:
		return Color( 0x00, 0x66, 0xFF );
		break;
	case JUMP:
		return Color( 0x00, 0x66, 0xAA );
		break;
	case GRAVREVERSE:
		return Color(0x33, 0x99, 0x44);
		break;
	case ANTIGRAVREVERSE:
		return Color(0x44, 0x66, 0x66);
		break;
	case BOUNCE:
		return Color( 0xDD, 0x99, 0x33 );
		break;
	case ACCELERATE:
		return Color(0xAA, 0xFF, 0xBB);
		break;
	case ANTIGRIND:
		return Color(0xEE, 0x44, 0x00);
		break;
	case POISON:
		return Color(0x00, 0xAA, 0x44);
		break;
	case ANTIWIRE:
		return Color(0x88, 0x11, 0x55);
		break;
	case BOOST:
		return Color(0xEE, 0x55, 0xAA);
		break;
	case HIT:
		return Color::White;
		break;
	case UNTECHABLE:
		return Color::White;
		break;
	}

	return Color::White;
}

int Grass::GetGrassTypeFromString(const std::string &s)
{
	if (s == "DECELERATE") { return DECELERATE; }
	else if (s == "JUMP") { return JUMP; }
	else if (s == "GRAVREVERSE") { return GRAVREVERSE; }
	else if (s == "ANTIGRAVREVERSE") { return ANTIGRAVREVERSE; }
	else if (s == "BOUNCE") { return BOUNCE; }
	else if (s == "ACCELERATE") { return ACCELERATE; }
	else if (s == "ANTIGRIND") { return ANTIGRIND; }
	else if (s == "POISON") { return POISON; }
	else if (s == "ANTIWIRE") { return ANTIWIRE; }
	else if (s == "BOOST") { return BOOST; }
	else if (s == "HIT") { return HIT; }
	else if (s == "UNTECHABLE") { return UNTECHABLE; }
	else
	{
		cout << "Grass string not found: " << s << endl;
		assert(0);
		return DECELERATE;
	}
}

std::string Grass::GetGrassStringFromType(int t)
{
	switch (t)
	{
	case DECELERATE:
		return "Decelerate";
		break;
	case JUMP:
		return "Jump";
		break;
	case GRAVREVERSE:
		return "Grav\nReverse";
		break;
	case ANTIGRAVREVERSE:
		return "Anti\nGrav\nReverse";
		break;
	case BOUNCE:
		return "Bounce";
		break;
	case ACCELERATE:
		return "Accelerate";
		break;
	case ANTIGRIND:
		return "Anti-Grind";
		break;
	case POISON:
		return "Poison";
		break;
	case ANTIWIRE:
		return "Anti-Wire";
		break;
	case BOOST:
		return "Boost";
		break;
	case HIT:
		return "Hit";
		break;
	case UNTECHABLE:
		return "Untechable";
		break;
	}

	return "";
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