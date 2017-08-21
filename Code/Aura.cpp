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

void Aura::ActivateParticles(list<Vector2f> &points, sf::Transform &tr, const sf::Vector2f &origin, AuraParams *ap )
{
	ParticleSet *tps = activeSets;
	ParticleSet *tnext = NULL;
	while (tps != NULL)
	{
		tnext = tps->next;
		if (tps->actuallyDone)
		{
			DeactivateParticles(tps);
		}
		tps = tnext;
	}

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
	ps->ap = ap;
	ps->actuallyDone = false;
	int ind = 0;
	Vector2f playerPos(player->position.x, player->position.y);

	//sf::Transform tr;
	
	//Vector2f orig = player->sprite->getOrigin();
	Vector2f truePos = player->sprite->getPosition();
	//tr.rotate(player->sprite->getRotation() );

	//tr.scale(Vector2f(-1, 1));



	Vector2f p;
	Vector2f v;

	Vector2f center = truePos;
	if (ap->pType == Aura::AuraParams::NORMAL)
	{
		Aura::NormalParams *np = (Aura::NormalParams*)ap;
		center = np->centerPos;
	}
	


	for (auto it = points.begin(); it != points.end(); ++it)
	{
		p = tr.transformPoint((*it) - origin) + truePos;

		//sf::Vector2<double> vel = normalize(player->ground->v1 - player->ground->v0) * player->groundSpeed / 2.0;
		//v =  * 2.f;
		//float lv = sqrt( ( )
		v = Vector2f(0, 0);//normalize( p - center ) * .3f;//Vector2f(0, 0);//Vector2f(vel.x, vel.y);
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
	:index( p_index ), next( NULL ), prev( NULL ), frame( 0 ), aura( p_aura )
{
	actuallyDone = false;
	maxFramesToLive = 0;
	int maxParticles = aura->maxParticlesPerSet;
	particles = new Particle*[maxParticles];
	for (int i = 0; i < maxParticles; ++i)
	{
		particles[i] = new Particle( this, (aura->va + (index * maxParticles * 4) + (i*4)), maxFramesToLive, aura->ts->GetSubRect( 0 ) );
	}
}

void Aura::ParticleSet::Update()
{
	bool allInactive = true;
	actuallyDone = true;
	for (int i = 0; i < numParticlesFromSprite; ++i)
	{
		if (particles[i]->active)
		{
			particles[i]->Update();
			allInactive = false;
			if (particles[i]->active)
			{
				actuallyDone = false;
			}

			/*if (particles[i]->active)
			{
				allInactive = false;
			}*/
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
	std::list<sf::Vector2f> &outPoints )
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


			 if (col.a != 0 )//&& col.r == 250 && col.g == 0 && col.b == 158 )
			 {
				 bool transAround = false;
				 if ((x < right - 1 && im.getPixel(x + 1, y).a == 0) || (x == right - 1  ))
				 {
					 transAround = true;
				 }
				 if ((x > sub.left && im.getPixel(x - 1, y).a == 0) || (x == sub.left ))
				 {
					 transAround = true;
				 }
				 if ((y < bot - 1 && im.getPixel(x, y + 1).a == 0) || (y == bot - 1 ))
				 {
					 transAround = true;
				 }
				 if ((y > sub.top && im.getPixel(x, y - 1).a == 0) || (y == sub.top ))
				 {
					 transAround = true;
				 }

				 if (transAround)
				 {
					 outPoints.push_back(Vector2f((x - sub.left), (y - sub.top)));
				 }
			 }
		}
	}
}

Aura::Particle::Particle( ParticleSet *p_ps, sf::Vertex *va, int p_ttl, sf::IntRect &sub)
	:quad( va )
{
	ps = p_ps;
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
	if (ttl >= 0)
	{
		Actor *player = ps->aura->player;
		float hw;
		switch (player->speedLevel)
		{
		case 0:
			hw = 6;//8
			break;
		case 1:
			hw = 12;//8
			break;
		case 2:
			hw = 18;//8
			break;
		}
		 ;

		quad[0].position = Vector2f(pos.x - hw, pos.y - hw);
		quad[1].position = Vector2f(pos.x + hw, pos.y - hw);
		quad[2].position = Vector2f(pos.x + hw, pos.y + hw);
		quad[3].position = Vector2f(pos.x - hw, pos.y + hw);
		
		pos += vel;
		vel += accel;
		ttl--;
	}
	else 
	{
		active = false;
		Clear();
		return;
	}

	
		

		/*quad[0].color.a = 255 * ( (float)ttl / maxTTL );
		quad[1].color.a = 255 * ((float)ttl / maxTTL);
		quad[2].color.a = 255 * ((float)ttl / maxTTL);
		quad[3].color.a = 255 * ((float)ttl / maxTTL);*/
	int f = 10;
		quad[0].color.a = f;
		quad[1].color.a = f;
		quad[2].color.a = f;
		quad[3].color.a = f;
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