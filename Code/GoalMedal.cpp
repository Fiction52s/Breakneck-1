#include "GoalMedal.h"
#include "Session.h"
#include "AbsorbParticles.h"

using namespace sf;
using namespace std;

GoalMedal::GoalMedal()
	:risingBez( 0, 0, 1, 1)
{
	sess = Session::GetSession();
	myGoal = sess->goal;
	//ts = //tm->GetSizedTileset("Enemies/General/Keys/key_w1_128x128.png");
	ts = sess->ts_key;
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	actionLength[A_IDLE] = 16;
	animFactor[A_IDLE] = 3;

	actionLength[A_RISE] = 90;
	animFactor[A_RISE] = 1;

	actionLength[A_HOLD] = 16;
	animFactor[A_HOLD] = 3;

	actionLength[A_DISPERSE] = 16;
	animFactor[A_DISPERSE] = 3;

	actionLength[A_HIDE] = 1;
	animFactor[A_HIDE] = 1;

	medalRank = 0;

	particles = new AbsorbParticles(sess, AbsorbParticles::MEDAL);
	//emitter = new GoalMedalEmitter( myGoal->sess->GetPlayer(0), this );// PI / 2.0, 2 * PI, 1.0, 2.5);
	//emitter->CreateParticles();
	//testEmitter2->CreateParticles();
	//testEmitter2->SetIDAndAddToAllEmittersVec();
	//emitters.push_back(testEmitter2);
}

GoalMedal::~GoalMedal()
{
	delete particles;
}

void GoalMedal::Reset()
{
	frame = 0;
	action = A_IDLE;

	V2d goalPos = sess->goal->GetGoalNodePos();//sess->GetLevelFinisherPos();

	SetPosition(Vector2f(goalPos));
	particles->Reset();
}

void GoalMedal::Disperse()
{
	action = A_DISPERSE;
	frame = 0;

	int numPoints = 0;
	int numParticlesPerDivision = 3; //clean up later

	switch (medalRank)
	{
		//bronze
	case 0:
		
		break;
		//silver
	case 1:
		break;
		//gold
	case 2:
		break;
	}
	particles->Activate(sess->GetPlayer(0), 16, V2d(position), 0);
}

void GoalMedal::SetMedalRank(int r)
{
	medalRank = r;
}

bool GoalMedal::IsDone()
{
	return action == A_HIDE;
}

void GoalMedal::Update()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		if (action == A_RISE)
		{
			action = A_HOLD;
		}
	}

	double risingDiff = 200;

	V2d goalPos = sess->goal->GetGoalNodePos();//sess->GetLevelFinisherPos();
	Vector2f goalPosF(goalPos);

	switch (action)
	{
	case A_IDLE:
		
		SetPosition(goalPosF);
		break;
	case A_RISE:
	{
		double df = frame;
		df = df / (actionLength[action] * animFactor[action]);
		double f = risingBez.GetValue(df);
		V2d newPos = goalPos * (1.0 -f) + V2d(goalPos + V2d(0, -risingDiff)) * f;
		SetPosition(Vector2f(newPos));
		break;
	}
	case A_HOLD:
		SetPosition(goalPosF + Vector2f(0, -risingDiff));
		break;
	case A_DISPERSE:
		if (particles->GetNumActive() == 0)
		{
			action = A_HIDE;
			frame = 0;
		}
		break;
	}

	particles->Update();

	int f = (frame / 3) % 16;
	sprite.setTextureRect(ts->GetSubRect(f));

	++frame;
}

void GoalMedal::SetPosition(Vector2f pos)
{
	position = pos;
	sprite.setPosition(pos);
}

void GoalMedal::Rise()
{
	action = A_RISE;
	frame = 0;
}

void GoalMedal::Draw(sf::RenderTarget *target)
{
	if (action == A_HIDE )
	{
		return;
	}

	if (action != A_DISPERSE)
	{
		target->draw(sprite);
	}

	if (action == A_DISPERSE)
	{
		particles->Draw(target);
	}
}