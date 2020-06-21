#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

ShipEnterScene::ShipEnterScene()
{
	ts_w1ShipClouds0 = sess->GetTileset("Ship/cloud_w1_a1_960x128.png", 960, 128);
	ts_w1ShipClouds1 = sess->GetTileset("Ship/cloud_w1_b1_960x320.png", 960, 320);
	ts_ship = sess->GetTileset("Ship/ship_864x400.png", 864, 400);

	shipSprite.setTexture(*ts_ship->texture);
	shipSprite.setTextureRect(ts_ship->GetSubRect(0));
	shipSprite.setOrigin(shipSprite.getLocalBounds().width / 2,
		shipSprite.getLocalBounds().height / 2);
}

void ShipEnterScene::AddFlashes()
{
	AddFlashedImage("stare0", sess->GetTileset("Bosses/Coyote/Coy_09b.png", 1920, 1080),
		0, 30, 20, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("staregroup");
	AddSeqFlashToGroup(group, "stare0", 0);
	group->Init();
}

void ShipEnterScene::Reset()
{
	Sequence::Reset();

	Actor *player = sess->GetPlayer(0);
	sess->SetDrainOn(false);
	player->action = Actor::RIDESHIP;
	player->frame = 0;
	player->position = shipEntrancePos;
	sess->playerOrigPos = Vector2i(player->position);
	shipSprite.setPosition(sess->playerOrigPos.x - 13, sess->playerOrigPos.y - 124);
	shipSequence = true;
	//shipSeqFrame = 0;
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

	Vector2f bottomLeft = Vector2f(sess->playerOrigPos.x, sess->playerOrigPos.y) + Vector2f(-480, 270);
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

	middleClouds.setPosition(sess->playerOrigPos.x - 480, sess->playerOrigPos.y + 270);
}

void ShipEnterScene::ReturnToGame()
{
	sess->SetPlayerInputOn(true);
	sess->adventureHUD->Show(60);
	sess->cam.SetManual(false);
	//owner->cam.EaseOutOfManual(5);
}

void ShipEnterScene::UpdateState()
{
	if (frame == 0)
	{
		sess->adventureHUD->Hide();
		SetFlashGroup("staregroup");
	}

	float oldLeft = cloud0[0].position.x;
	float blah = 30.f;
	float newLeft = oldLeft - blah; //cloudVel.x;
	float diff = (shipStartPos.x - 480) - newLeft;
	if (diff >= 480)
	{
		//cout << "RESETING: " << diff << endl;
		newLeft = shipStartPos.x - 480 - (diff - 480);
	}
	else
	{
		//cout << "diff: " << diff << endl;
	}

	float allDiff = newLeft - oldLeft;
	Vector2f cl = relShipVel;

	middleClouds.move(Vector2f(0, cl.y));// + Vector2f( allDiff, 0 ) );
	for (int i = 0; i < 3 * 4; ++i)
	{
		cloud0[i].position = cl + Vector2f(cloud0[i].position.x + allDiff, cloud0[i].position.y);
		cloud1[i].position = cl + Vector2f(cloud1[i].position.x + allDiff, cloud1[i].position.y);

		cloudBot0[i].position = cl + Vector2f(cloudBot0[i].position.x + allDiff, cloudBot0[i].position.y);
		cloudBot1[i].position = cl + Vector2f(cloudBot1[i].position.x + allDiff, cloudBot1[i].position.y);
	}

	if (frame >= 90 && frame <= 180)
	{
		int tFrame = frame - 90;
		shipSprite.setPosition(shipSprite.getPosition() + relShipVel);

		relShipVel += Vector2f(.3, -.8);
	}
	else if (frame == 240)//121 )
	{
		Actor *player = sess->GetPlayer(0);
		//cout << "relshipvel: " << relShipVel.x << ", " << relShipVel.y << endl;
		player->action = Actor::JUMP;
		player->frame = 1;
		player->velocity = V2d(20, 10);
		player->UpdateSprite();
		//shipSequence = false;
		state = Count;
		player->hasDoubleJump = false;
		player->hasAirDash = false;
		player->hasGravReverse = false;
		sess->SetDrainOn(true);
	}
}

void ShipEnterScene::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHIP_FLYING] = -1;
	stateLength[KIN_JUMP] = -1;
}

void ShipEnterScene::Draw(sf::RenderTarget *target, EffectLayer layer)
{
	if (layer == EffectLayer::BETWEEN_PLAYER_AND_ENEMIES)
	{
		target->draw(cloud1, 4 * 3, sf::Quads, ts_w1ShipClouds1->texture);
		target->draw(cloud0, 4 * 3, sf::Quads, ts_w1ShipClouds0->texture);
		target->draw(middleClouds);
		target->draw(cloudBot1, 4 * 3, sf::Quads, ts_w1ShipClouds1->texture);
		target->draw(cloudBot0, 4 * 3, sf::Quads, ts_w1ShipClouds0->texture);
		target->draw(shipSprite);
	}

	Sequence::Draw(target, layer);
}