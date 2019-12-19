#include "EnvPlant.h"
#include "AirParticles.h"

using namespace std;
using namespace sf;

//groundLeft,airLeft,airRight,groundRight
EnvPlant::EnvPlant(sf::Vector2<double>&a, V2d &b, V2d &c, V2d &d, int vi, VertexArray *v, Tileset *t)
	:A(a), B(b), C(c), D(d), vaIndex(vi), va(v), frame(0), activated(false), next(NULL), ts(t),
	idleLength(4), idleFactor(3)
{
	particle = new AirParticleEffect((a + d) / 2.0);
	disperseLength = particle->maxDurationToLive + particle->emitDuration;
	disperseFactor = 1;
	SetupQuad();
}

EnvPlant::~EnvPlant()
{
	delete particle;
}

void EnvPlant::SetupQuad()
{
	VertexArray &eva = *va;
	eva[vaIndex + 0].position = Vector2f(A.x, A.y);
	eva[vaIndex + 1].position = Vector2f(B.x, B.y);
	eva[vaIndex + 2].position = Vector2f(C.x, C.y);
	eva[vaIndex + 3].position = Vector2f(D.x, D.y);
}

void EnvPlant::HandleQuery(QuadTreeCollider *qtc)
{
	qtc->HandleEntrant(this);
}

bool EnvPlant::IsTouchingBox(const Rect<double> &r)
{
	return isQuadTouchingQuad(V2d(r.left, r.top), V2d(r.left + r.width, r.top),
		V2d(r.left + r.width, r.top + r.height), V2d(r.left, r.top + r.height),
		A, B, C, D);
}

void EnvPlant::Reset()
{

	//cout << "resetting plant!" << endl;
	next = NULL;
	activated = false;
	frame = 0;

	SetupQuad();

	particle->Reset();
}