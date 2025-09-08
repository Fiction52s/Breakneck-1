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
	shipComp = NULL;//new CompositeImage(sess, "Ship/ShipTest/traveltest_1024x1024", 16, Vector2f(1024, 1024), Vector2f(4, 4));

	//ts_shipTest = sess->GetSizedTileset("Ship/ShipTest/travel1_1725x921.png");
	ts_shipTest = sess->GetSizedTileset("Ship/ship_inner_1300x690.png");

	shipTestSprite.setTexture(*ts_shipTest->texture);
	shipTestSprite.setOrigin(shipTestSprite.getLocalBounds().width / 2, shipTestSprite.getLocalBounds().height / 2);
	//shipTestSprite.setScale(.62, .62);
	//shipTestSprite.setScale(.75, .75);
	shipTestSprite.setScale(1.0, 1.0);
	//shipTestSprite.setScale(.5, .5);
	
	shipTravelPos = V2d(0, 0);

	if (shipComp != NULL)
	{
		shipComp->SetCenter(Vector2f(shipTravelPos));
	}
	else
	{
		shipTestSprite.setPosition(Vector2f(shipTravelPos));
	}

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

	shipTravelPos = V2d(0, 0);
	//player->position = shipTravelPos;
	//sess->playerOrigPos[0] = Vector2i(player->position);

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

	shipSequence = true;
	//shipSeqFrame = 0;
	cloudVel = Vector2f(-40, 0);
	relShipVel = Vector2f(2, 0);


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

void ShipTravelSequence::DrawShip(sf::RenderTarget *target)
{
	if (shipComp != NULL)
	{
		shipComp->Draw(target);
	}
	else
	{
		target->draw(shipTestSprite);
	}
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

		DrawShip(target);
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