#include "VisualEffects.h"
#include "Tileset.h"
#include <assert.h>
#include <iostream>
#include "VectorMath.h"
#include "Actor.h"

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

EffectPool::~EffectPool()
{
	//DestroyAllMembers();
	delete[] va;
}

void EffectPool::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
}

void EffectPool::Reset()
{
	ResetPool();
}

void EffectPool::DeactivateMember(PoolMember *pm)
{
	EffectInstance *ei = (EffectInstance*)pm;
	ei->Clear();
}

void EffectPool::Update(EffectPoolUpdater *epu)
{
	switch (eType)
	{
	case FX_REGULAR:
	{
		EffectInstance *ei = (EffectInstance*)activeListStart;
		EffectInstance *tNext = NULL;
		while (ei != NULL)
		{
			tNext = (EffectInstance*)ei->pmnext;

			if (epu != NULL)
			{
				epu->UpdateEffect(ei);
			}
			
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
			tNext = (RelEffectInstance*)ei->pmnext;

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

EffectInstance* EffectPool::ActivateEffect( EffectInstance *params )
{
	EffectInstance *ei = (EffectInstance*)ActivatePoolMember();

	
	if (ei == NULL)
	{
		return NULL;
		//cout << "cannot produce effect!" << endl;
		//assert(ei != NULL);
	}

	ei->parent = this;
	ei->InitFromParams( params );

	return ei;
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

	RenderStates rs;
	rs.texture = ts->texture;

	assert(ts != NULL);
	if (effectShader == NULL)
	{
		target->draw(va, maxNumFX * 4, sf::Quads, rs);
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
	color = Color::White;
}

void EffectInstance::SetColor(sf::Color &c)
{
	color = c;
	SetRectColor(parent->va + index * 4, color);
}



void EffectInstance::InitFromParams(EffectInstance *ei)
{
	Init(ei->pos, ei->tr, ei->frameCount, ei->animFactor, ei->startTile);
	maxVel = ei->maxVel;
	accel = ei->accel;
	vel = ei->vel;
	SetColor(ei->color);
}

void EffectInstance::Init( sf::Vector2f &p_pos, sf::Transform &p_tr, int p_frameCount, int p_animFactor, int p_startTile)
{
	maxVel = 0;
	vel = Vector2f(0, 0);
	accel = Vector2f(0, 0);
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

void EffectInstance::SetVelocityParams(sf::Vector2f &p_vel,
	sf::Vector2f &p_accel, float p_maxVel )
{
	vel = p_vel;
	accel = p_accel;
	maxVel = p_maxVel;
}

bool EffectInstance::Update()
{
	if (frame < 0)
	{
		int f = 10;
	}
	if (frame == frameCount * animFactor )
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


	float width = parent->ts->tileWidth;
	float height = parent->ts->tileHeight;

	Vector2f p[4];

	p[0] = Vector2f(-width / 2.f, -height / 2.f);
	p[1] = Vector2f(+width / 2.f, -height / 2.f);
	p[2] = Vector2f(+width / 2.f, +height / 2.f);
	p[3] = Vector2f(-width / 2.f, +height / 2.f);

	if (vel.x != 0 || vel.y != 0)
	{
		pos += vel;

		for (int i = 0; i < 4; ++i)
		{
			p[i] = tr.transformPoint(p[i]);
			parent->va[index * 4 + i].position = pos + p[i];
		}
	}

	vel += accel;

	if (length(vel) > maxVel)
	{
		vel = normalize(vel) * maxVel;
	}

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

	if (detachFrames == 0)
	{
		lockPos = NULL;
	}
	

	if (lockPos == NULL)
		return true;

	if (detachFrames > 0)
	{
		detachFrames--;
	}

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
	sf::Vector2<double> *lock, int p_detachFrames )
{
	lockPos = lock;
	detachFrames = p_detachFrames;
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
	sf::Vector2<double> *lockP, int p_detachFrames )
{
	lockPos = lockP;
	detachFrames = p_detachFrames;
	EffectInstance::SetParams(p_pos, p_tr, p_frameCount, p_animFactor, p_startTile);
}

void RelEffectInstance::ClearLockPos()
{
	lockPos = NULL;
}

VertexBuf::VertexBuf(int p_numMembers, sf::PrimitiveType p_pType )
	:numMembers(p_numMembers), vb( NULL ), ts( NULL ), pType( p_pType )
{
	switch (p_pType)
	{
	case sf::Quads:
		verticesPerMember = 4;
		break;
	case sf::Triangles:
		verticesPerMember = 3;
		break;
	case sf::Lines:
		verticesPerMember = 2;
		break;
	default:
		assert(0);
	}

	members = new VertexBufMember[numMembers];
	dirty = new bool[numMembers];
	vb = new Vertex[numMembers * verticesPerMember];
	Reset();
}

void VertexBuf::Reset()
{
	memset(dirty, 0, sizeof(dirty));
	dirtyTS = false;
	SetNumActiveMembers(0);
}

VertexBuf::~VertexBuf()
{
	delete[] members;
	delete[] vb;
	delete[] dirty;
}

void VertexBuf::SetScale(int index, sf::Vector2f &scale)
{
	/*if (ts == NULL)
	{
		
	}*/
	//Vector2f size(scale.x * ts->tileWidth, scale.y * ts->tileHeight);
	members[index].scale = scale;//size;
	dirty[index] = true;
}

void VertexBuf::SetPosition(int index, sf::Vector2f &pos)
{
	members[index].pos = pos;
	dirty[index] = true;
}
void VertexBuf::SetRotation(int index, double angle)
{
	members[index].angle = angle;
	dirty[index] = true;
}

const VertexBufMember const &VertexBuf::GetMemberInfo( int index )
{
	return members[index];
}

void VertexBuf::RotateMemberCW(int index, float angle)
{
	members[index].angle += angle;
	dirty[index] = true;
}

void VertexBuf::SetTileset(Tileset *p_ts)
{
	ts = p_ts;
	dirtyTS = true;
}

void VertexBuf::SetTile(int index, int tileIndex)
{
	members[index].tileIndex = tileIndex;
	dirty[index] = true;
}

void VertexBuf::UpdateVertices()
{
	if (ts == NULL)
		return;
	switch (pType)
	{
	case sf::Quads:
	{
		Vector2f vPos[4];

		Vector2f quadSize;
		IntRect sub;
		for (int i = 0; i < numActiveMembers; ++i)
		{
			if (!dirtyTS && !dirty[i])
				continue; 

			VertexBufMember &currMember = members[i];
			if (ts == NULL)
			{
				quadSize = Vector2f(64, 64);
				//debugging only
			}
			else
			{
				quadSize = Vector2f(ts->tileWidth * currMember.scale.x, ts->tileHeight * currMember.scale.y);
			}
			vPos[0] = Vector2f(-quadSize.x / 2, -quadSize.y / 2);
			vPos[1] = Vector2f(quadSize.x / 2, -quadSize.y / 2);
			vPos[2] = Vector2f(quadSize.x / 2, quadSize.y / 2);
			vPos[3] = Vector2f(-quadSize.x / 2, quadSize.y / 2);

			for (int j = 0; j < 4; ++j)
			{
				RotateCW(vPos[j], currMember.angle);
				vPos[j] += currMember.pos;
				vb[i * 4 + j].position = vPos[j];
			}
			dirty[i] = false;

			sub = ts->GetSubRect(currMember.tileIndex);
			
			vb[i * 4 + 0].texCoords = Vector2f(sub.left, sub.top);
			vb[i * 4 + 1].texCoords = Vector2f(sub.left + sub.width, sub.top );
			vb[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
			vb[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height );

			vb[i * 4 + 0].color = currMember.color;
			vb[i * 4 + 1].color = currMember.color;
			vb[i * 4 + 2].color = currMember.color;
			vb[i * 4 + 3].color = currMember.color;
		}
		break;
	}
	default:
		assert(0 && "not implemented yet");
	}
	dirtyTS = false;
}

void VertexBuf::SetColor(int index, sf::Color &color)
{
	members[index].color = color;
	dirty[index] = true;
}

void VertexBuf::SetNumActiveMembers(int p_numActiveMembers)
{
	assert(p_numActiveMembers <= numMembers && p_numActiveMembers >= 0);
	numActiveMembers = p_numActiveMembers;
}

void VertexBuf::Draw(sf::RenderTarget *rt, sf::Shader *shader )
{
	RenderStates rs;
	rs.shader = shader;
	if (ts != NULL)
		rs.texture = ts->texture;

	if (ts == NULL)
	{
		rt->draw(vb, numActiveMembers * verticesPerMember, pType, rs);
	}
	else
	{
		rt->draw(vb, numActiveMembers * verticesPerMember, pType, rs);
	}
}

void VertexBufMember::Reset()
{
	pos = Vector2f(0, 0);
	angle = 0;
	scale = Vector2f(1, 1);
	tileIndex = 0;
}

RisingParticleUpdater::RisingParticleUpdater(Actor *p)
	:parent( p )
{

}

void RisingParticleUpdater::UpdateEffect(EffectInstance *ei)
{
	Vector2f pPos = Vector2f(parent->position);
	float len = length(pPos - ei->pos);
	Vector2f dir = normalize(pPos - ei->pos);
	//dir += Vector2f(0, -1);
	//dir /= 2.f;
	if (len > 20)
	{
		//ei->accel = dir * .1f;
	}

	float f = ((float)ei->frame) / (ei->frameCount * ei->animFactor -1 );
	f = 1.0 - f;
	Color  testC = Color(255, 255, 255, 100 * f);
	ei->SetColor(testC);
}