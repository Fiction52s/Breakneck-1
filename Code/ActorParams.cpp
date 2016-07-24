#include "EditSession.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Physics.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace sf;

#define cout std::cout
#define V2d sf::Vector2<double>

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

EditSession * ActorParams::session = NULL;




sf::Font *PoiParams::font = NULL;

ActorParams::ActorParams( ActorParams::PosType p_posType )
	:ISelectable( ISelectable::ACTOR ), boundingQuad( sf::Quads, 4 ), posType( p_posType ),
		hasMonitor( false ), group( NULL )
{
	groundInfo = NULL;

	for( int i = 0; i < 4; ++i )
		boundingQuad[i].color = Color( 0, 255, 0, 100);
}

void ActorParams::SetPath( std::list<sf::Vector2i> &globalPath )
{
}

void ActorParams::SetParams()
{
}

void ActorParams::SetPanelInfo()
{
}

void ActorParams::SetSelected( bool select )
{
	cout << "------selected: " << select << endl;
	selected = select;
}

bool ActorParams::CanAdd()
{
	assert( false );
	return false; //shouldn't get called anyway
}

void ActorParams::Draw( sf::RenderTarget *target )
{
	//cout << "Selected: " << selected << endl;
	

	//temporary checks might make it lag less?

	DrawMonitor( target );

	Vector2f viewCenter = target->getView().getCenter();
	Vector2f viewSize = target->getView().getSize();
	if( image.getGlobalBounds().intersects( FloatRect( viewCenter.x - viewSize.x / 2, viewCenter.y - viewSize.y / 2,
		viewSize.x, viewSize.y ) ) )
	{
		target->draw( image );
	}

	DrawBoundary( target );
}

void ActorParams::DrawMonitor( sf::RenderTarget *target )
{
	

	if( hasMonitor )
	{
		double w = image.getLocalBounds().width;
		double h = image.getLocalBounds().height;

		sf::CircleShape cs;
		cs.setRadius( max( w, h ) );

		cs.setFillColor( Color::White );

		cs.setOrigin( cs.getLocalBounds().width / 2, 
			cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );

		target->draw( cs );
	}

	
	
}

void ActorParams::WriteFile( ofstream &of )
{
	//if( params.size() == 0 )
	//{
	//	assert( false && "no params" );
	//}
	
	//dont need number of params because the actortype determines that.
	of << type->name << " ";

	if( type->canBeGrounded && type->canBeAerial )
	{
		if( groundInfo != NULL )
		{
			int edgeIndex = groundInfo->GetEdgeIndex();
			of << "-air" << " " << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
		}
		else
		{
			of << "+air" << " " << position.x << " " << position.y << endl;
		}
	}
	else if( type->canBeGrounded )
	{
		assert( groundInfo != NULL );

		int edgeIndex = groundInfo->GetEdgeIndex();

		of << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
	}
	else if( type->canBeAerial )
	{
		of << position.x << " " << position.y << endl;
	}
	else
	{
		assert( false );
	}
	

	/*for( list<string>::iterator it = params.begin(); it != params.end(); ++it )
	{
		of << (*it) << endl;
	}*/
	WriteParamFile( of );
}

void ActorParams::DrawQuad( sf::RenderTarget *target )
{
	target->draw( boundingQuad );
}

GroundInfo::GroundInfo()
	:edgeStart( NULL ), groundQuantity( -1 ), ground( NULL )
{

}

int GroundInfo::GetEdgeIndex()
{
	int index = 0;
	for( TerrainPoint *curr = ground->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr == edgeStart )
			return index;
		++index;
	}

	
	//assert( false && "could not find correct edge index" );
	return -1;
}

void ActorParams::SetBoundingQuad()
{
	//float note
	if( type->canBeGrounded && groundInfo != NULL )
	{
		V2d v0( (*groundInfo->edgeStart).pos.x, (*groundInfo->edgeStart).pos.y );
		TerrainPoint *edgeEnd = groundInfo->edgeStart->next;
		if( edgeEnd == NULL )
			edgeEnd = groundInfo->ground->pointStart;
		V2d v1( edgeEnd->pos.x, edgeEnd->pos.y );
		V2d along = normalize( v1 - v0 );
		V2d other( along.y, -along.x );

		V2d startGround = v0 + along * groundInfo->groundQuantity;
		V2d leftGround = startGround - along * ( type->width / 2.0) + other * 1.0;
		V2d rightGround = startGround + along * ( type->width / 2.0) + other * 1.0;
		V2d leftAir = leftGround + other * (double)(type->height - 1) ;
		V2d rightAir = rightGround + other * (double)(type->height - 1 );

		/*if( type->name == "poi" )
		{
			leftGround = startGround - along * ( type->width / 2.0 ) - other * ( type->height / 2.0 );
			rightGround = startGround + along * ( type->width / 2.0 ) - other * ( type->height / 2.0 );
			leftAir = leftGround + other * ( type->height / 2.0 );
			rightAir = rightGround + other * ( type->height / 2.0 );
		}*/

		boundingQuad[0].position = Vector2f( leftGround.x, leftGround.y );
		boundingQuad[1].position = Vector2f( leftAir.x, leftAir.y );
		boundingQuad[2].position = Vector2f( rightAir.x, rightAir.y );
		boundingQuad[3].position = Vector2f( rightGround.x, rightGround.y );

		V2d pos = (leftGround + leftAir + rightAir + rightGround ) / 4.0;
		position = Vector2i( pos.x, pos.y );
	}
	else
	{
		//patroller doesnt need a box because its not physical with the environment
		boundingQuad[0].position = Vector2f( position.x - type->width / 2, position.y - type->height / 2);
		boundingQuad[1].position = Vector2f( position.x + type->width / 2, position.y - type->height / 2);
		boundingQuad[2].position = Vector2f( position.x + type->width / 2, position.y + type->height / 2);
		boundingQuad[3].position = Vector2f( position.x - type->width / 2, position.y + type->height / 2);
	}

	UpdateExtraVisuals();
}

void ActorParams::UpdateGroundedSprite()
{	
	assert( groundInfo != NULL && groundInfo->ground != NULL );
	
	TerrainPoint *edge = groundInfo->edgeStart;
	TerrainPoint *next = edge->next;
	if( next == NULL )
		next = groundInfo->ground->pointStart;

	

	V2d pr( edge->pos.x, edge->pos.y );
	V2d cu( next->pos.x, next->pos.y );


	//this shouldn't remain here. i need more detailed checking.
	double groundLength = length( pr - cu );
	if( groundInfo->groundQuantity + type->width / 2 > groundLength )
	{
		groundInfo->groundQuantity = groundLength - type->width / 2;
	}
	else if( groundInfo->groundQuantity - type->width / 2 < 0 )
	{
		groundInfo->groundQuantity = type->width / 2;
	}

	V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
									(groundInfo->groundQuantity / length( cu - pr ) ) );

	double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;
	image.setPosition( newPoint.x, newPoint.y );
	image.setRotation( angle );
}

void ActorParams::AnchorToGround( TerrainPolygon *poly, int edgeIndex, double quantity )
{
	assert( groundInfo == NULL );
	/*if( groundInfo != NULL )
	{
		cout << "unanchor" << endl;
		
		//UnAnchor( actor );
		cout << "end unanchor" << endl;
		
		//delete groundInfo;
		//groundInfo = NULL;
	}*/



	groundInfo = new GroundInfo;
	
	groundInfo->ground = poly;
	
	groundInfo->groundQuantity = quantity;
	
	int testIndex = 0;

	image.setTexture( type->imageTexture );	

	if( type->name != "poi" )
		image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	else
	{
		image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	}

	Vector2i point;

	TerrainPoint *prev = groundInfo->ground->pointEnd;
	TerrainPoint *curr = groundInfo->ground->pointStart;

	for( ; curr != NULL; curr = curr->next )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( prev->pos.x, prev->pos.y );
			V2d cu( curr->pos.x, curr->pos.y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundInfo->groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			groundInfo->edgeStart = prev;

			
			UpdateGroundedSprite();
			SetBoundingQuad();

			break;
		}
		prev = curr;
		++testIndex;
	}

	
}

void ActorParams::AnchorToGround( GroundInfo &gi )
{
	groundInfo = new GroundInfo;
	*groundInfo = gi;

	image.setTexture( type->imageTexture );	

	if( type->name != "poi" )
		image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );

	UpdateGroundedSprite();
	SetBoundingQuad();
}

void ActorParams::UnAnchor( ActorPtr &actor )
{
	assert( groundInfo != NULL );
	//if( groundInfo == NULL )
	//	return;

	if( groundInfo != NULL )
	{
		position = Vector2i( image.getPosition().x, image.getPosition().y );

		image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
		image.setRotation( 0 );
		//groundInfo->ground->enemies.clear();
		//i get deleted when i remove myself from the terrain polygon????

		//group->actors.front().use_count();

		//ActorPtr actor( this );
		//group->actors.
		//cout << "use count: " << actor.use_count() << endl;
		
		//groundInfo->ground->enemies.clear();
		
		//cout << 
		//SelectPtr select = boost::dynamic_pointer_cast<ISelectable>		
		groundInfo->ground->enemies[groundInfo->edgeStart].remove( actor );
		//cout << "After remove" << endl;

		//cout << "use count: " << actor.use_count() << endl;

		delete groundInfo;
		groundInfo = NULL;

		//cout << "About to bound" << endl;
		SetBoundingQuad();
		//cout << "just bounded" << endl;
		//image.setPosition( 
	}
}

void ActorParams::DrawBoundary( sf::RenderTarget *target )
{
	if( selected )
	{
		sf::RectangleShape rs;
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Green );
		rs.setOutlineThickness( 3 * EditSession::zoomMultiple );
		rs.setPosition( image.getGlobalBounds().left, image.getGlobalBounds().top );
		rs.setSize( Vector2f( image.getGlobalBounds().width, image.getGlobalBounds().height ) );
		target->draw( rs );
		//cout << "selected draw" << endl;
	}
}

bool ActorParams::ContainsPoint( sf::Vector2f test )
{ 
	sf::Transform trans = image.getTransform();


	/*V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );*/	

	
	V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );

	bool result = QuadContainsPoint( A,B,C,D, V2d( test.x, test.y ) );

	//cout << "result: " << result << endl;
	return result;
}

bool ActorParams::Intersects( sf::IntRect rect )
{
	V2d A( boundingQuad[0].position.x, boundingQuad[0].position.y );
	V2d B( boundingQuad[1].position.x, boundingQuad[1].position.y );
	V2d C( boundingQuad[2].position.x, boundingQuad[2].position.y );
	V2d D( boundingQuad[3].position.x, boundingQuad[3].position.y );

	V2d rA( rect.left, rect.top );
	V2d rB( rect.left + rect.width, rect.top );
	V2d rC( rect.left + rect.width, rect.top + rect.height );
	V2d rD( rect.left, rect.top + rect.height );
	if( isQuadTouchingQuad( A, B, C, D, rA, rB, rC, rD ) )
	{
		return true;
	}
	else
		return false;
}

bool ActorParams::IsPlacementOkay()
{
	return false;
}

void ActorParams::Move( SelectPtr &me, sf::Vector2i delta )
{
	//assert( groundInfo == NULL );

	//ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( me );
	//if( groundInfo != NULL )
	//{
	//	UnAnchor( actor );
	//}
	if( groundInfo == NULL )
	{
		position.x += delta.x;
		position.y += delta.y;

		SetBoundingQuad();

		image.setPosition( position.x, position.y );
	}
	
	
	/*if( session->worldPosGround.ground != NULL )
	{
		
		AnchorToGround( session->worldPosGround.ground, session->worldPosGround.GetEdgeIndex(), session->worldPosGround.groundQuantity );
		
		SetBoundingQuad();
		UpdateGroundedSprite();
		//session->editStartMove = false;

		//p->enemies[groundInfo->edgeStart].push_back( actor );
		//p->UpdateBounds();
		//cout << "new end" << endl;
	}
	else
	{
		SetBoundingQuad();

		image.setPosition( position.x, position.y );
	}*/
}

void ActorParams::BrushDraw( sf::RenderTarget *target, 
		bool valid )
{
	image.setColor( Color( 255, 255, 255, 100 ) );
	target->draw( image );
	image.setColor( Color::White );
}

void ActorParams::Deactivate( EditSession *edit, SelectPtr &select )
{
	cout << "DEACTIVATING ACTOR PARAMS size from: " << group->actors.size() << endl;
	ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( select );
	group->actors.remove( actor );

	
	if( actor->groundInfo != NULL )
	{
		if( actor->groundInfo->ground->selected )
		{

		}
		else
		{
			actor->groundInfo->ground
				->enemies[actor->groundInfo->edgeStart].remove( actor );
		}
		
	}
}

void ActorParams::Activate( EditSession *edit, SelectPtr &select )
{
	cout << "addding to group of size: " << group->actors.size() << endl;
	ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( select );
	group->actors.push_back( actor );

	/*if( actor->groundInfo != NULL )
	{
		actor->groundInfo->ground
			->enemies[actor->groundInfo->edgeStart].push_back( actor );
	}*/
}





HealthFlyParams::HealthFlyParams( EditSession *edit,
		sf::Vector2i pos, int p_color )
		:ActorParams( ActorParams::AIR_ONLY )
{
	type = edit->types["healthfly"];
	position = pos;
	

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );
	
	color = p_color;

	SetBoundingQuad();
}

void HealthFlyParams::WriteParamFile( std::ofstream &of )
{
	int hMon;
	if( hasMonitor )
		hMon = 1;
	else
		hMon = 0;
	of << hMon << endl;
	of << color << endl;
}

void HealthFlyParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );
	//target->draw( image );
}

bool HealthFlyParams::CanApply()
{
	return true;
}

ActorParams *HealthFlyParams::Copy()
{
	HealthFlyParams *copy = new HealthFlyParams( *this );
	return copy;
}




GoalParams::GoalParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( PosType::GROUND_ONLY )
{
	type = edit->types["goal"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool GoalParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void GoalParams::WriteParamFile( ofstream &of )
{
}

ActorParams *GoalParams::Copy()
{
	GoalParams *copy = new GoalParams( *this );
	return copy;
}




//remnove the postype thing. we have 2 bools for that already
PlayerParams::PlayerParams( EditSession *edit, sf::Vector2i pos )
	:ActorParams( PosType::AIR_ONLY )
{
	position = pos;

	type = edit->types["player"];
	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

bool PlayerParams::CanApply()
{
	sf::IntRect me( position.x - image.getLocalBounds().width / 2, position.y - image.getLocalBounds().height / 2, 
		image.getLocalBounds().width, image.getLocalBounds().height );
	for( list<PolyPtr>::iterator it = session->polygons.begin(); it != session->polygons.end(); ++it )
	{
		
		if( (*it)->Intersects( me ) )
		{
			return false;
		}
	}
	return true;
}

void PlayerParams::WriteParamFile( std::ofstream &of )
{
}

void PlayerParams::Deactivate( EditSession *edit, boost::shared_ptr<ISelectable> & select)
{
	//nothing
}

void PlayerParams::Activate( EditSession *edit, boost::shared_ptr<ISelectable> & select )
{
	//nothing
}

ActorParams *PlayerParams::Copy()
{
	assert( false );
	return NULL;
}


PoiParams::PoiParams( EditSession *edit,
	TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, 
	double p_edgeQuantity )
	:ActorParams( ActorParams::PosType::GROUND_AND_AIR ),
	barrier( NONE )
{
	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setColor( Color::White );
	
	name = "-";
	type = edit->types["poi"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

PoiParams::PoiParams( EditSession *edit,
	TerrainPolygon *p_edgePolygon,
	int p_edgeIndex, 
	double p_edgeQuantity, PoiParams::Barrier bType, const std::string &p_name )
	:ActorParams( ActorParams::PosType::GROUND_AND_AIR ),
	barrier( bType ), name( p_name )
{
	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setColor( Color::White );

	type = edit->types["poi"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

PoiParams::PoiParams( EditSession *edit,
	sf::Vector2i &pos )
	:ActorParams( ActorParams::PosType::GROUND_AND_AIR ), barrier( NONE )
{
	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setColor( Color::White );

	name = "-";
	position = pos;	
	type = edit->types["poi"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

PoiParams::PoiParams( EditSession *edit,
	sf::Vector2i &pos, PoiParams::Barrier bType, const std::string &p_name  )
	:ActorParams( ActorParams::PosType::GROUND_AND_AIR ), 
	barrier( bType ), name( p_name )
{
	nameText.setFont( *font );
	nameText.setCharacterSize( 18 );
	nameText.setColor( Color::White );

	//name = "-";
	position = pos;	
	type = edit->types["poi"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetBoundingQuad();
}

bool PoiParams::CanApply()
{
	return true;
}

ActorParams *PoiParams::Copy()
{
	PoiParams *copy = new PoiParams( *this );
	return copy;
}

void PoiParams::WriteParamFile( std::ofstream &of )
{
	of << name << " ";

	switch( barrier )
	{
	case NONE:
		of << "-" << endl;
		break;
	case X:
		of << "x" << endl;
		break;
	case Y:
		of << "y" << endl;
		break;
	}
}

void PoiParams::SetParams()
{
	Panel *p = type->panel;
	
	name = p->textBoxes["name"]->text.getString();

	nameText.setString( name );

	string barStr = p->textBoxes["barrier"]->text.getString().toAnsiString();
	
	if( barStr == "-" )
	{
		barrier = Barrier::NONE;
	}
	else if( barStr == "x" )
	{
		barrier = Barrier::X;
	}
	else if( barStr == "y" )
	{
		barrier = Barrier::Y;
	}
	else
	{
		//do nothing
	}
}

void PoiParams::SetPanelInfo()
{
	Panel *p = type->panel;

	string s;
	switch( barrier )
	{
	case NONE:
		s = "-";
		break;
	case X:
		s = "x";
		break;
	case Y:
		s = "y";
		break;
	}

	p->textBoxes["name"]->text.setString( name );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}

	p->textBoxes["barrier"]->text.setString( s );
}

void PoiParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	nameText.setString( name );
	nameText.setOrigin( nameText.getLocalBounds().width / 2, nameText.getLocalBounds().height / 2 );
	nameText.setPosition( position.x, position.y - 40 );

	target->draw( nameText );
}