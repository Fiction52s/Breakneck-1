#include "Aura.h"
#include "Actor.h"
#include "Tileset.h"
#include <assert.h>
#include <iostream>
#include "GameSession.h"

using namespace std;
using namespace sf;

Aura::Aura( Actor *p_player, int numSets, int p_maxParticlesPerSet )
	:player( p_player ), maxParticlesPerSet( p_maxParticlesPerSet ), numParticleSets( numSets )
{
	inactiveSets = NULL;
	ParticleSet *temp;
	ts = player->owner->GetTileset("aura_16x16.png", 16, 16);
	totalParticles = numSets * maxParticlesPerSet;
	va = new Vertex[totalParticles * 4];
	memset(va, 0, sizeof(va));

	for (int i = 0; i < numSets; ++i)
	{
		temp = new ParticleSet(this, i);
		AddToInactive(temp);
	}
	activeSets = NULL;

	
}
void Aura::Update()
{
	ParticleSet *ps = activeSets;
	ParticleSet *next;
	while (ps != NULL)
	{
		next = ps->next;
		ps->Update();
		ps = next;
	}
}
void Aura::Draw(sf::RenderTarget *target)
{
	target->draw(va, totalParticles * 4, sf::Quads, ts->texture);
}

void Aura::ActivateParticles(list<Vector2f> &points, sf::Transform &tr)
{
	ParticleSet *ps = NULL;
	if (inactiveSets == NULL)
	{
		cout << "failed to activate particle set" << endl;
		return;
	}
	else
	{
		ps = inactiveSets;
		if (inactiveSets->next != NULL)
		{
			inactiveSets->next->prev = NULL;
		}
		inactiveSets = inactiveSets->next;
		ps->next = NULL;
	}

	if (activeSets == NULL)
	{
		activeSets = ps;
	}
	else
	{
		ps->next = activeSets;
		activeSets->prev = ps;
		activeSets = ps;
	}
	
	int pointsSize = points.size();
	ps->numParticlesFromSprite = pointsSize;
	ps->frame = 0;

	int ind = 0;
	Vector2f playerPos(player->position.x, player->position.y);

	//sf::Transform tr;
	
	//Vector2f orig = player->sprite->getOrigin();
	Vector2f truePos = player->sprite->getPosition();
	//tr.rotate(player->sprite->getRotation() );

	//tr.scale(Vector2f(-1, 1));

	Vector2f p;
	Vector2f v;
	for (auto it = points.begin(); it != points.end(); ++it)
	{
		p = tr.transformPoint((*it)) + truePos;
		v = normalize(p - truePos) * 2.f;
		//float lv = sqrt( ( )
		
		ps->particles[ind]->Set(p, v, Vector2f(0, 0));
		++ind;
		if (ind == maxParticlesPerSet)
		{
			cout << "Exceeded limit" << "\n";
			break;
		}
	}

	for (; ind < pointsSize; ++ind)
	{
		ps->particles[ind]->Clear();
	}
}

void Aura::ParticleSet::Activate()
{
	
}

Aura::ParticleSet::ParticleSet(Aura *p_aura, int p_index)
	:index( p_index ), next( NULL ), prev( NULL ), frame( 0 ), maxFramesToLive( 60 ), aura( p_aura )
{
	int maxParticles = aura->maxParticlesPerSet;
	particles = new Particle*[maxParticles];
	for (int i = 0; i < maxParticles; ++i)
	{
		particles[i] = new Particle((aura->va + (index * maxParticles * 4) + (i*4)), maxFramesToLive, aura->ts->GetSubRect( 0 ) );
	}
}

void Aura::ParticleSet::Update()
{
	bool allInactive = true;
	for (int i = 0; i < numParticlesFromSprite; ++i)
	{
		if (particles[i]->active)
		{
			allInactive = false;
			particles[i]->Update();
		}
	}

	if (allInactive)
	{
		aura->DeactivateParticles(this);
	}
}

void Aura::AddToInactive(ParticleSet *ps)
{
	if (inactiveSets == NULL)
	{
		ps->next = NULL;
		ps->prev = NULL;
		inactiveSets = ps;
	}
	else
	{
		ps->prev = NULL;
		ps->next = inactiveSets;
		inactiveSets->prev = ps;
		inactiveSets = ps;
	}
}

void Aura::DeactivateParticles(ParticleSet *ps)
{
	assert(activeSets != NULL);

	if (ps->next != NULL)
	{
		ps->next->prev = ps->prev;
	}
	if (ps->prev != NULL)
	{
		ps->prev->next = ps->next;
	}

	if (activeSets == ps)
	{
		activeSets = NULL;
	}
	AddToInactive(ps);
}

void Aura::CreateParticlePointList(Tileset *ts, Image &im, int tileIndex,
	std::list<sf::Vector2f> &outPoints, sf::Vector2f &origin)
{
	assert(outPoints.size() == 0);
	IntRect sub = ts->GetSubRect(tileIndex);
	int right = sub.left + sub.width;
	int bot = sub.top + sub.height;

	sf::Color col;
	for (int y = sub.top; y < bot; ++y)
	{ 
		for (int x = sub.left; x < right; ++x)
		{
			 col = im.getPixel(x, y);
			 //currently use every pixel that isnt transparent
			 if (col.a != 0 )
			 {
				 outPoints.push_back(Vector2f((x - sub.left) - origin.x, (y - sub.top) - origin.y ));
			 }
		}
	}
}

Aura::Particle::Particle(sf::Vertex *va, int p_ttl, sf::IntRect &sub)
	:quad( va )
{
	maxTTL = p_ttl;

	/*quad[0].color = Color::Red;
	quad[1].color = Color::Red;
	quad[2].color = Color::Red;
	quad[3].color = Color::Red;*/

	quad[0].texCoords = Vector2f(sub.left, sub.top);
	quad[1].texCoords = Vector2f(sub.left + sub.width, sub.top);
	quad[2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
	quad[3].texCoords = Vector2f(sub.left, sub.top + sub.height);
}

void Aura::Particle::Set(sf::Vector2f &p_pos, sf::Vector2f &p_vel,
	sf::Vector2f &p_accel)
{
	pos = p_pos;
	vel = p_vel;
	accel = p_accel;
	active = true;
	ttl = maxTTL;
}

void Aura::Particle::Update()
{
	//if (active)
	//{
		pos += vel;
		vel += accel;
		ttl--;

		if (ttl < 0)
		{
			active = false;
			Clear();
			return;
		}

		float hw = 4;

		quad[0].position = Vector2f(pos.x - hw, pos.y - hw);
		quad[1].position = Vector2f(pos.x + hw, pos.y - hw);
		quad[2].position = Vector2f(pos.x + hw, pos.y + hw);
		quad[3].position = Vector2f(pos.x - hw, pos.y + hw);
	//}
}

void Aura::Particle::Clear()
{
	quad[0].position = Vector2f(0, 0);
	quad[1].position = Vector2f(0, 0);
	quad[2].position = Vector2f(0, 0);
	quad[3].position = Vector2f(0, 0);
	active = false;
}