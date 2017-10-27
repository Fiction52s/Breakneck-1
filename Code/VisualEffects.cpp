#include "VisualEffects.h"
#include "Tileset.h"
#include <assert.h>
#include <iostream>

using namespace sf;
using namespace std;

EffectInstance::EffectInstance(EffectPool *p_parent, int index )
	:PoolMember( index ), frame( 0 ), parent(p_parent)
{

}

EffectPool::EffectPool(EffectType et, int p_maxNumFX, float p_depth )
	:ObjectPool(), maxNumFX( p_maxNumFX ), depth( p_depth )
{
	eType = et;
	va = new Vertex[maxNumFX * 4];
	memset(va, 0, sizeof(va));
	effectShader = NULL;
	EffectInstance *ei = NULL;
	for (int i = 0; i < maxNumFX; ++i)
	{
		switch (et)
		{
		case FX_REGULAR:
			ei = new EffectInstance(this, i);
			break;
		case FX_RELATIVE:
			ei = new RelEffectInstance(this, i);
			break;
		default:
			ei = new EffectInstance(this, i);
			assert(0);
			break;
		}
		 
		 AddToInactiveList(ei);
	}
}

void EffectPool::Reset()
{
	ResetPool();
}

void EffectPool::Update()
{
	switch (eType)
	{
	case FX_REGULAR:
	{
		EffectInstance *ei = (EffectInstance*)activeListStart;
		EffectInstance *tNext = NULL;
		while (ei != NULL)
		{
			tNext = (EffectInstance*)ei->next;

			ei->Update();

			ei = tNext;
		}
		break;
	}
	case FX_RELATIVE:
	{
		RelEffectInstance *ei = (RelEffectInstance*)activeListStart;
		RelEffectInstance *tNext = NULL;
		while (ei != NULL)
		{
			tNext = (RelEffectInstance*)ei->next;

			ei->Update();

			ei = tNext;
		}
		break;
	}
	default:
	{
		assert(0);
	}
	}
	
}

void EffectPool::ActivateEffect( EffectInstance *params )
{
	EffectInstance *ei = (EffectInstance*)ActivatePoolMember();

	
	if (ei == NULL)
	{
		cout << "cannot produce effect!" << endl;
		assert(ei != NULL);
	}

	ei->parent = this;
	ei->InitFromParams( params );
	//*ei = *params;
	
}

void EffectPool::Draw(sf::RenderTarget *target)
{
	sf::View oldView = target->getView();
	if (depth != 1.f)
	{	
		sf::View newView;
		float oldFactor = oldView.getSize().x / 1920.f;
		float newFactor = depth;
		newView.setCenter(oldView.getCenter() / depth);
		newView.setSize(Vector2f(1920, 1080) * newFactor * oldFactor);
		target->setView(newView);
	}

	if (effectShader == NULL)
	{
		target->draw(va, maxNumFX * 4, sf::Quads, ts->texture );
	}
	else
	{
		target->draw(va, maxNumFX * 4, sf::Quads, effectShader );
	}

	if (depth != 1.f)
	{
		target->setView(oldView);
	}
}

void EffectInstance::SetParams( sf::Vector2f &p_pos, sf::Transform &p_tr, int p_frameCount, int p_animFactor, int p_startTile )
{
	frameCount = p_frameCount;
	animFactor = p_animFactor;
	pos = p_pos;
	tr = p_tr;
	startTile = p_startTile;
}

void EffectInstance::InitFromParams(EffectInstance *ei)
{
	Init(ei->pos, ei->tr, ei->frameCount, ei->animFactor, ei->startTile);
}

void EffectInstance::Init( sf::Vector2f &p_pos, sf::Transform &p_tr, int p_frameCount, int p_animFactor, int p_startTile)
{
	frame = 0;
	SetParams(p_pos, p_tr, p_frameCount, p_animFactor, p_startTile);
	float width = parent->ts->tileWidth;
	float height = parent->ts->tileHeight;

	Vector2f p[4];

	p[0] = Vector2f(-width / 2.f, -height / 2.f);
	p[1] = Vector2f(+width / 2.f, -height / 2.f);
	p[2] = Vector2f(+width / 2.f, +height / 2.f);
	p[3] = Vector2f(-width / 2.f, +height / 2.f);

	for (int i = 0; i < 4; ++i)
	{
		p[i] = tr.transformPoint(p[i]);
		parent->va[index * 4 + i].position = pos + p[i];
	}
}

void EffectInstance::Clear()
{
	for (int i = 0; i < 4; ++i)
	{
		parent->va[index * 4 + i].position = Vector2f(0, 0);
	}
	
}

bool EffectInstance::Update()
{
	if (frame * animFactor == frameCount)
	{
		Clear();
		parent->DeactivatePoolMember(this);
		return false;
	}

	IntRect sub = parent->ts->GetSubRect(frame / animFactor + startTile);

	parent->va[index * 4 + 0].texCoords = Vector2f( sub.left, sub.top );
	parent->va[index * 4 + 1].texCoords = Vector2f(sub.left + sub.width, sub.top);
	parent->va[index * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
	parent->va[index * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);

	++frame;

	return true;
}

RelEffectInstance::RelEffectInstance(EffectPool *parent, int index)
	:EffectInstance( parent, index ), lockPos(NULL)
{

}

void RelEffectInstance::InitFromParams(EffectInstance *ei)
{
	RelEffectInstance *rei = (RelEffectInstance*)ei;
	Init(ei->pos, ei->tr, ei->frameCount, ei->animFactor, ei->startTile, rei->lockPos );
}

bool RelEffectInstance::Update()
{
	if (!EffectInstance::Update())
		return false;


	float width = parent->ts->tileWidth;
	float height = parent->ts->tileHeight;

	Vector2f p[4];

	p[0] = Vector2f(-width / 2.f, -height / 2.f);
	p[1] = Vector2f(+width / 2.f, -height / 2.f);
	p[2] = Vector2f(+width / 2.f, +height / 2.f);
	p[3] = Vector2f(-width / 2.f, +height / 2.f);

	Vector2f truePos = Vector2f(*lockPos) + pos;

	for (int i = 0; i < 4; ++i)
	{
		p[i] = tr.transformPoint(p[i]);
		parent->va[index * 4 + i].position = truePos + p[i];
	}

	return true;
}

void RelEffectInstance::Init(sf::Vector2f &pos,
	sf::Transform &tr, int frameCount,
	int animFactor,
	int startTile,
	sf::Vector2<double> *lock )
{
	lockPos = lock;
	EffectInstance::Init(pos, tr, frameCount, animFactor, startTile );

	float width = parent->ts->tileWidth;
	float height = parent->ts->tileHeight;

	Vector2f p[4];

	p[0] = Vector2f(-width / 2.f, -height / 2.f);
	p[1] = Vector2f(+width / 2.f, -height / 2.f);
	p[2] = Vector2f(+width / 2.f, +height / 2.f);
	p[3] = Vector2f(-width / 2.f, +height / 2.f);

	Vector2f truePos = Vector2f(*lockPos) + pos;

	for (int i = 0; i < 4; ++i)
	{
		p[i] = tr.transformPoint(p[i]);
		parent->va[index * 4 + i].position = truePos + p[i];
	}

}

void RelEffectInstance::SetParams(
	sf::Vector2f &p_pos,
	sf::Transform &p_tr,
	int p_frameCount,
	int p_animFactor,
	int p_startTile,
	sf::Vector2<double> *lockP)
{
	lockPos = lockP;
	EffectInstance::SetParams(p_pos, p_tr, p_frameCount, p_animFactor, p_startTile);
}