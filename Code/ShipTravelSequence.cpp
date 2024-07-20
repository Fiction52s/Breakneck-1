#include "Sequence.h"
#include "GameSession.h"
#include "Actor.h"
#include "HUD.h"
#include "Background.h"
#include "ShipTravelSequence.h"
#include "CompositeImage.h"

using namespace sf;
using namespace std;

ShipTravelSequence::ShipTravelSequence()
{
	ts_w1ShipClouds0 = sess->GetSizedTileset("Ship/cloud_w1_a1_960x128.png");
	ts_w1ShipClouds1 = sess->GetSizedTileset("Ship/cloud_w1_b1_960x320.png");
	ts_ship = sess->GetSizedTileset("Ship/ship_open_864x410.png");

	shipComp = NULL;//new CompositeImage(sess, "Ship/ShipTest/traveltest_1024x1024", 16, Vector2f(1024, 1024), Vector2f(4, 4));


	ts_shipTest = sess->GetSizedTileset("Ship/ShipTest/travel1_1725x921.png");

	shipTestSprite.setTexture(*ts_shipTest->texture);
	shipTestSprite.setOrigin(shipTestSprite.getLocalBounds().width / 2, shipTestSprite.getLocalBounds().height / 2);
	//shipTestSprite.setScale(.62, .62);
	shipTestSprite.setScale(.75, .75);
	//shipTestSprite.setScale(.5, .5);

	shipSprite.setTexture(*ts_ship->texture);
	shipSprite.setTextureRect(ts_ship->GetSubRect(0));
	shipSprite.setOrigin(shipSprite.getLocalBounds().width / 2 + 34,
		shipSprite.getLocalBounds().height / 2 - 53);
	

}

ShipTravelSequence::~ShipTravelSequence()
{
	if (shipComp != NULL)
	{
		delete shipComp;
	}
}

void ShipTravelSequence::AddFlashes()
{
	/*AddFlashedImage("stare0", sess->GetTileset("Enemies/Bosses/Coyote/Coy_09b.png", 1920, 1080),
	0, 30, 20, 30, Vector2f(960, 540));

	FlashGroup * group = AddFlashGroup("staregroup");
	AddSeqFlashToGroup(group, "stare0", 0);
	group->Init();*/
}

void ShipTravelSequence::Reset()
{
	Sequence::Reset();

	shipEnterData.extraBackgroundOffset = 0;

	Actor *player = sess->GetPlayer(0);
	sess->SetDrainOn(false);
	player->action = Actor::JUMP;
	player->frame = 1;
	//player->position = shipTravelPos;
	sess->playerOrigPos[0] = Vector2i(player->position);

	if (shipComp != NULL)
	{
		shipComp->SetCenter(Vector2f(shipTravelPos));
	}
	else
	{
		shipTestSprite.setPosition(Vector2f(shipTravelPos));
	}

	Vector2f shipPos(sess->playerOrigPos[0].x - 13, sess->playerOrigPos[0].y - 124);
	shipPos += Vector2f(8, -50);

	shipSprite.setPosition(shipPos);
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

	Vector2f bottomLeft = Vector2f(sess->playerOrigPos[0].x, sess->playerOrigPos[0].y) + Vector2f(-480, 270);
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

	middleClouds.setPosition(sess->playerOrigPos[0].x - 480, sess->playerOrigPos[0].y + 270);

	sess->cam.SetCamType(Camera::SHIP);
}

void ShipTravelSequence::ReturnToGame()
{
	sess->SetPlayerInputOn(true);
	sess->ShowHUD(60);
	sess->cam.SetManual(false);
	//owner->cam.EaseOutOfManual(5);
}

void ShipTravelSequence::UpdateState()
{
	//cout << "update ship enter state. curr frame is " << seqData.frame << " on total game frames: " << sess->totalGameFrames << "\n";
	if (seqData.frame == 0)
	{
		sess->HideHUD();
		
		//sess->Fade(true, 30, Color::Black, false, DrawLayer::IN_FRONT_OF_UI);
		//SetFlashGroup("staregroup");
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

	float origClouds = sess->playerOrigPos[0].x - 480;
	float currClouds = middleClouds.getPosition().x;


	//sess->background->Update(sess->view.getCenter());
	if (sess->background != NULL)
	{
		sess->background->SetExtra(Vector2f(shipEnterData.extraBackgroundOffset, 0));
	}


	shipEnterData.extraBackgroundOffset -= 10.f;
}

void ShipTravelSequence::SetupStates()
{
	SetNumStates(Count);

	stateLength[SHIP_FLYING] = -1;
	stateLength[KIN_JUMP] = -1;
}

void ShipTravelSequence::LayeredDraw(int p_drawLayer, sf::RenderTarget *target)
{
	//if (layer == DrawLayer::BETWEEN_PLAYER_AND_ENEMIES)
	if (p_drawLayer == DrawLayer::BEHIND_TERRAIN)
	{
		/*target->draw(cloud1, 4 * 3, sf::Quads, ts_w1ShipClouds1->texture);
		target->draw(cloud0, 4 * 3, sf::Quads, ts_w1ShipClouds0->texture);
		target->draw(middleClouds);
		target->draw(cloudBot1, 4 * 3, sf::Quads, ts_w1ShipClouds1->texture);
		target->draw(cloudBot0, 4 * 3, sf::Quads, ts_w1ShipClouds0->texture);
		target->draw(shipSprite);*/

		if (shipComp != NULL)
		{
			shipComp->Draw(target);
		}
		else
		{
			target->draw(shipTestSprite);
		}
	}

	Sequence::LayeredDraw(p_drawLayer, target );
}

int ShipTravelSequence::GetNumStoredBytes()
{
	return sizeof(seqData) + sizeof(shipEnterData);
}

void ShipTravelSequence::StoreBytes(unsigned char *bytes)
{
	memcpy(bytes, &seqData, sizeof(seqData));
	bytes += sizeof(seqData);
	memcpy(bytes, &shipEnterData, sizeof(shipEnterData));
}

void ShipTravelSequence::SetFromBytes(unsigned char *bytes)
{
	memcpy(&seqData, bytes, sizeof(seqData));
	bytes += sizeof(seqData);
	memcpy(&shipEnterData, bytes, sizeof(shipEnterData));
	//nextSeq = sess->GetEnemyFromID(seqData.); //needs implementing if I want 2 sequences in a row to not mess up in rollback
}