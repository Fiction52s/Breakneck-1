#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"

using namespace sf;
using namespace std;

ShipEnterScene::ShipEnterScene(GameSession *p_owner)
	:BasicBossScene(p_owner, BasicBossScene::APPEAR)
{

}

void ShipEnterScene::Reset()
{
	BasicBossScene::Reset();

	Actor *player = owner->GetPlayer(0);
	owner->drain = false;
	player->action = Actor::RIDESHIP;
	player->frame = 0;
	player->position = shipEntrancePos;
	owner->playerOrigPos = Vector2i(player->position);
	shipSprite.setPosition(owner->playerOrigPos.x - 13, owner->playerOrigPos.y - 124);
	shipSequence = true;
	shipSeqFrame = 0;
	shipStartPos = shipSprite.getPosition();
	cloudVel = Vector2f(-40, 0);
	relShipVel = Vector2f(2, 0);
	//#0055FF
	middleClouds.setFillColor(Color(0x00, 0x55, 0xFF));
	int middleHeight = 540 * 4;
	middleClouds.setSize(Vector2f(960, middleHeight));
	Vector2f botExtra(0, middleHeight);

	IntRect sub0 = ts_w1ShipClouds0->GetSubRect(0);
	IntRect sub1 = ts_w1ShipClouds1->GetSubRect(0);

	Vector2f bottomLeft = Vector2f(owner->playerOrigPos.x, owner->playerOrigPos.y) + Vector2f(-480, 270);
	for (int i = 0; i < 3; ++i)
	{
		Vector2f xExtra(480 * i, 0);
		cloud0[i * 4 + 0].position = xExtra + bottomLeft;
		cloud0[i * 4 + 1].position = xExtra + bottomLeft + Vector2f(0, -sub0.height / 2);
		cloud0[i * 4 + 2].position = xExtra + bottomLeft + Vector2f(sub0.width / 2, -sub0.height / 2);
		cloud0[i * 4 + 3].position = xExtra + bottomLeft + Vector2f(sub0.width / 2, 0);

		cloud0[i * 4 + 0].texCoords = Vector2f(0, sub0.height);
		cloud0[i * 4 + 1].texCoords = Vector2f(0, 0);
		cloud0[i * 4 + 2].texCoords = Vector2f(sub0.width, 0);
		cloud0[i * 4 + 3].texCoords = Vector2f(sub0.width, sub0.height);

		cloud1[i * 4 + 0].position = xExtra + bottomLeft;
		cloud1[i * 4 + 1].position = xExtra + bottomLeft + Vector2f(0, -sub1.height / 2);
		cloud1[i * 4 + 2].position = xExtra + bottomLeft + Vector2f(sub1.width / 2, -sub1.height / 2);
		cloud1[i * 4 + 3].position = xExtra + bottomLeft + Vector2f(sub1.width / 2, 0);

		cloud1[i * 4 + 0].texCoords = Vector2f(0, sub1.height);
		cloud1[i * 4 + 1].texCoords = Vector2f(0, 0);
		cloud1[i * 4 + 2].texCoords = Vector2f(sub1.width, 0);
		cloud1[i * 4 + 3].texCoords = Vector2f(sub1.width, sub1.height);

		cloudBot0[i * 4 + 0].position = botExtra + xExtra + bottomLeft;
		cloudBot0[i * 4 + 1].position = botExtra + xExtra + bottomLeft + Vector2f(0, sub0.height / 2);
		cloudBot0[i * 4 + 2].position = botExtra + xExtra + bottomLeft + Vector2f(sub0.width / 2, sub0.height / 2);
		cloudBot0[i * 4 + 3].position = botExtra + xExtra + bottomLeft + Vector2f(sub0.width / 2, 0);

		cloudBot0[i * 4 + 0].texCoords = Vector2f(0, sub0.height);
		cloudBot0[i * 4 + 1].texCoords = Vector2f(0, 0);
		cloudBot0[i * 4 + 2].texCoords = Vector2f(sub0.width, 0);
		cloudBot0[i * 4 + 3].texCoords = Vector2f(sub0.width, sub0.height);

		cloudBot1[i * 4 + 0].position = botExtra + xExtra + bottomLeft;
		cloudBot1[i * 4 + 1].position = botExtra + xExtra + bottomLeft + Vector2f(0, sub1.height / 2);
		cloudBot1[i * 4 + 2].position = botExtra + xExtra + bottomLeft + Vector2f(sub1.width / 2, sub1.height / 2);
		cloudBot1[i * 4 + 3].position = botExtra + xExtra + bottomLeft + Vector2f(sub1.width / 2, 0);

		cloudBot1[i * 4 + 0].texCoords = Vector2f(0, sub1.height);
		cloudBot1[i * 4 + 1].texCoords = Vector2f(0, 0);
		cloudBot1[i * 4 + 2].texCoords = Vector2f(sub1.width, 0);
		cloudBot1[i * 4 + 3].texCoords = Vector2f(sub1.width, sub1.height);
	}

	middleClouds.setPosition(owner->playerOrigPos.x - 480, owner->playerOrigPos.y + 270);
}

void ShipEnterScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer == IN_FRONT)
	{
		target->draw(cloud1, ts_w1ShipClouds1->texture);
		target->draw(cloud0, ts_w1ShipClouds0->texture);
		target->draw(middleClouds);
		target->draw(cloudBot1, ts_w1ShipClouds1->texture);
		target->draw(cloudBot0, ts_w1ShipClouds0->texture);
		target->draw(shipSprite);
	}
}