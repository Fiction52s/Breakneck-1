#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

EnemyChooseRect::EnemyChooseRect(ChooseRectIdentity ident, sf::Vertex *v, Vector2f &p_pos, ActorType * p_type, int p_level,
	Panel *p)
	:ChooseRect(ident, ChooseRectType::ENEMY, v, Vector2f( 100, 100 ), p_pos, p), level(p_level)
{
	actorType = NULL;
	SetType(p_type, level);
}

//void EnemyChooseRect::Unfocus()
//{
//	if (enemy != NULL)
//	{
//		enemy->SetActionEditLoop();
//		enemy->UpdateFromEditParams(0);
//	}
//}

void EnemyChooseRect::SetType(ActorType *type, int lev)
{
	if (type == NULL)
	{
		SetShown(false);
		actorType = type;
		level = lev;
		return;
	}

	if (!(type == actorType && lev == level))
	{
		actorType = type;
		level = lev;
		enemyParams = actorType->defaultParamsVec[level - 1];
		enemyParams->MoveTo(Vector2i(0, 0));
		enemy = enemyParams->myEnemy;
		if (enemy != NULL)
		{
			enemy->SetActionEditLoop();
			enemy->UpdateFromEditParams(0);
			SetName(enemy->name);
		}

		SetSize(boxSize);

		switch (level)
		{
		case 1:
			idleColor = Color::Blue;
			break;
		case 2:
			idleColor = Color::Cyan;
			break;
		case 3:
			idleColor = Color::Magenta;
			break;
		case 4:
			idleColor = Color::Red;
			break;

		}
		idleColor.a = 100;
		SetRectColor(quad, idleColor);

	}
}

void EnemyChooseRect::SetSize(sf::Vector2f &bSize)
{
	ChooseRect::SetSize(bSize);
	UpdatePanelPos();
}

void EnemyChooseRect::UpdatePanelPos()
{
	if (actorType != NULL)
	{
		Vector2f truePos = GetGlobalPos() + Vector2f(boxSize.x / 2.f, boxSize.y / 2.f);

		float test;
		FloatRect aabb = enemyParams->GetAABB();

		float max = std::max(aabb.height, aabb.width);
		//max *= 1.1f;
		test = max / boxSize.y;
		view.setCenter(Vector2f(960 * test - truePos.x * test, 540 * test - truePos.y * test));// + Vector2f( 64,0 ));//-pos / 5);//Vector2f(0, 0));
		view.setSize(Vector2f(1920 * test, 1080 * test));
	}
}

void EnemyChooseRect::Draw(RenderTarget *target)
{
	if (show)
	{
		sf::View oldView = target->getView();
		target->setView(view);

		enemyParams->DrawEnemy(target);

		target->setView(oldView);

		if (showName)
		{
			target->draw(nameText);
		}
	}
}

void EnemyChooseRect::UpdateSprite(int frameUpdate)
{
	if (!show)
	{
		return;
	}

	if (actorType != NULL)
	{
		if (enemy != NULL)
		{
			if (focused)
			{
				enemy->UpdateFromEditParams(frameUpdate);
			}
			else
			{
				enemy->SetActionEditLoop();
				enemy->UpdateFromEditParams(0);
			}
		}
	}
}