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


////helper function to assign monitor types
//ActorParams::MonitorType GetMonitorType( Panel *p )
//{
//	GridSelector *gs = p->gridSelectors["monitortype"];
//	string name = gs->names[gs->selectedX][gs->selectedY];
//
//	ActorParams::MonitorType monitorType;
//	if( name == "none" )
//	{
//		monitorType = ActorParams::NONE;
//	}
//	else if( name == "red" )
//	{
//		monitorType = ActorParams::RED;
//	}
//	else if( name == "green" )
//	{
//		monitorType = ActorParams::GREEN;
//	}
//	else if( name == "blue" )
//	{
//		monitorType = ActorParams::BLUE;
//	}
//	else
//	{
//		cout << "panel: " << p->name << ", name: " << name << endl;
//		assert( false );
//	}
//	
//
//	return monitorType;
//}



ActorParams::ActorParams( ActorParams::PosType p_posType )
	:ISelectable( ISelectable::ACTOR ), boundingQuad( sf::Quads, 4 ), posType( p_posType ),
		monitorType( MonitorType::NONE ), group( NULL )
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
	

	if( monitorType != ActorParams::MonitorType::NONE )
	{
		double w = image.getLocalBounds().width;
		double h = image.getLocalBounds().height;

		sf::CircleShape cs;
		cs.setRadius( max( w, h ) );

		switch( monitorType )
		{
		case BLUE:
			cs.setFillColor( COLOR_BLUE );
			break;
		case GREEN:
			cs.setFillColor( COLOR_GREEN );
			break;
		case YELLOW:
			cs.setFillColor( COLOR_YELLOW );
			break;
		case ORANGE:
			cs.setFillColor( COLOR_ORANGE );
			break;
		case RED:
			cs.setFillColor( COLOR_RED );
			break;
		case MAGENTA:
			cs.setFillColor( COLOR_MAGENTA );
			break;
		case WHITE:
			cs.setFillColor( COLOR_WHITE );
			break;
		}

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
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );

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

//HealthFlyParams::HealthFlyParams( EditSession *edit )
//{
//}

void HealthFlyParams::WriteParamFile( std::ofstream &of )
{
	of << (int)monitorType << endl;
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

KeyParams::KeyParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop,
					 int p_stayFrames, bool p_teleport, GateInfo::GateTypes gType )
	:ActorParams( PosType::AIR_ONLY )
{	
	lines = NULL;
	position = pos;	
	gateType = gType;

	type = edit->types["key"];

	if( gateType == GateInfo::RED )
	{
		image.setTexture( type->imageTexture );
	}
	else if( gateType == GateInfo::GREEN )
	{
		image.setTexture( edit->types["greenkey"]->imageTexture );
	}
	else if( gateType == GateInfo::BLUE )
	{
		image.setTexture( edit->types["bluekey"]->imageTexture );
	}
	

	
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;
	stayFrames = p_stayFrames;
	teleport = p_teleport;


	SetBoundingQuad();
}

bool KeyParams::CanApply()
{
	return true;
	//for now can apply anywhere. should you only be able to apply when not on the same spot as other keys??
}

void KeyParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}

	localPath.clear();
	if( globalPath.size() > 1 )
	{
		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

void KeyParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		VertexArray &li = *lines;
	
	
			for( int i = 0; i < localPathSize+1; ++i )
			{
				li[i].position += Vector2f( position.x, position.y );
			}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	target->draw( image );
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

ActorParams *KeyParams::Copy()
{
	return NULL;
}

std::list<sf::Vector2i> KeyParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void KeyParams::WriteParamFile( ofstream &of )
{
	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	of << (int)gateType << endl;

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	of.precision( 5 );
	of << fixed << speed << endl;

	of << stayFrames << endl;
	
	if( teleport )
	{
		of << "+tele" << endl;
	}
	else
	{
		of << "-tele" << endl;
	}
}

PatrollerParams::PatrollerParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop )
	:ActorParams( PosType::AIR_ONLY)
{	
	lines = NULL;
	position = pos;	
	type = edit->types["patroller"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;

	SetBoundingQuad();
	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

PatrollerParams::PatrollerParams( EditSession *edit,
	sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{	
	lines = NULL;
	position = pos;	
	type = edit->types["patroller"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	loop = false;
	speed = 10;

	SetBoundingQuad();

	//image.setTexture( type->imageTexture );
	//image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	//image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	//SetPath( globalPath );

	//loop = p_loop;
	//speed = p_speed;

	//SetBoundingQuad();

	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

void PatrollerParams::SetParams()
{
	Panel *p = type->panel;

	bool loop = p->checkBoxes["loop"]->checked;
	

	string speedStr = p->textBoxes["speed"]->text.getString().toAnsiString();

	stringstream ss;
	ss << speedStr;

	int t_speed; 
	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}
	//try
	//{
	//	speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
	//}
	//catch(boost::bad_lexical_cast &)
	//{
	//	//error
	//}
}

void PatrollerParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );
	p->checkBoxes["loop"]->checked = loop;
	EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

bool PatrollerParams::CanApply()
{
	return true;
	//see note for keyparams
}

void PatrollerParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}
	
	
	

	localPath.clear();
	if( globalPath.size() > 1 )
	{

		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}

	
	
}

void PatrollerParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		VertexArray &li = *lines;
	
	
			for( int i = 0; i < localPathSize+1; ++i )
			{
				li[i].position += Vector2f( position.x, position.y );
			}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	ActorParams::Draw( target );
	//target->draw( image );

	//DrawBoundar
}

std::list<sf::Vector2i> PatrollerParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void PatrollerParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;

	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	//of.precision( 5 );
	of << speed << endl;
	//of << fixed << speed << endl;
}

ActorParams *PatrollerParams::Copy()
{
	PatrollerParams *copy = new PatrollerParams( *this );
	if( copy->lines != NULL )
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray( sf::LinesStrip, numVertices );
		VertexArray &li = *copy->lines;
		

		for( int i = 0; i < numVertices; ++i )
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}

CrawlerParams::CrawlerParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, bool p_clockwise, float p_speed )
	:ActorParams( PosType::GROUND_ONLY )
{
	clockwise = p_clockwise;
	speed = p_speed;

	type = edit->types["crawler"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

CrawlerParams::CrawlerParams( EditSession *edit )
	:ActorParams( PosType::GROUND_ONLY ), clockwise( true ), speed( 0 )
{
	
	type = edit->types["crawler"];
}

bool CrawlerParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void CrawlerParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;
	if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;
}

ActorParams *CrawlerParams::Copy()
{
	CrawlerParams *copy = new CrawlerParams( *this );
	return copy;
}

CrawlerReverserParams::CrawlerReverserParams( EditSession *edit, TerrainPolygon *p_edgePolygon, 
	int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( PosType::GROUND_ONLY )
{
	type = edit->types["crawlerreverser"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool CrawlerReverserParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void CrawlerReverserParams::WriteParamFile( ofstream &of )
{
}

ActorParams *CrawlerReverserParams::Copy()
{
	CrawlerReverserParams *copy = new CrawlerReverserParams( *this );
	return copy;
}

BossCrawlerParams::BossCrawlerParams( EditSession *edit, TerrainPolygon *p_edgePolygon, 
	int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( PosType::GROUND_ONLY )
{
	type = edit->types["bosscrawler"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool BossCrawlerParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void BossCrawlerParams::WriteParamFile( ofstream &of )
{
	//no params its a boss!
}

ActorParams *BossCrawlerParams::Copy()
{
	BossCrawlerParams *copy = new BossCrawlerParams( *this );
	return copy;
}


BasicTurretParams::BasicTurretParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, double p_bulletSpeed, int p_framesWait )
	:ActorParams( PosType::GROUND_ONLY )
{
	bulletSpeed = p_bulletSpeed;
	framesWait = p_framesWait;

	type = edit->types["basicturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool BasicTurretParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void BasicTurretParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;
	of << bulletSpeed << endl;
	of << framesWait << endl;
}

ActorParams *BasicTurretParams::Copy()
{
	BasicTurretParams *copy = new BasicTurretParams( *this );
	return copy;
}

FootTrapParams::FootTrapParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
	:ActorParams( PosType::GROUND_ONLY )	
{
	type = edit->types["foottrap"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

bool FootTrapParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void FootTrapParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;
}

ActorParams *FootTrapParams::Copy()
{
	FootTrapParams *copy = new FootTrapParams( *this );
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

//BAT

BatParams::BatParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, int p_bulletSpeed, int p_framesBetweenNodes, bool p_loop )
	:ActorParams( PosType::AIR_ONLY)
{	
	lines = NULL;
	position = pos;	
	type = edit->types["bat"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	framesBetweenNodes = p_framesBetweenNodes; 
	//nodeDistance = p_nodeDistance;
	bulletSpeed = p_bulletSpeed;

	loop = p_loop;
	//speed = p_speed;

	SetBoundingQuad();
	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

BatParams::BatParams( EditSession *edit, sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{	
	lines = NULL;
	position = pos;	
	type = edit->types["bat"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	loop = false;
	//speed = 5;
	framesBetweenNodes = 60;
	//nodeDistance = 100;
	bulletSpeed = 10;

	SetBoundingQuad();

	//image.setTexture( type->imageTexture );
	//image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	//image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	//SetPath( globalPath );

	//loop = p_loop;
	//speed = p_speed;

	//SetBoundingQuad();

	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

bool BatParams::CanApply()
{
	return true;
	//see note for keyparams
}

void BatParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if( lines != NULL )
	{
		//cout << "SET PATH REMOVCE LINES-------------------------------------------" << endl;
		delete lines;
		lines = NULL;
	}
	
	
	

	localPath.clear();
	if( globalPath.size() > 1 )
	{

		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}

	
	
}

void BatParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		
		VertexArray &li = *lines;
	
	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position += Vector2f( position.x, position.y );
		}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	ActorParams::Draw( target );
}

std::list<sf::Vector2i> BatParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void BatParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;

	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	//of.precision( 5 );
	//of << speed << endl;//fixed << speed << endl;
	of << bulletSpeed << endl;
	//of << nodeDistance << endl;
	of << framesBetweenNodes << endl;
}

void BatParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string bulletSpeedStr = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	//string nodeDistanceStr = p->textBoxes["nodedistance"]->text.getString().toAnsiString();
	string betweenStr = p->textBoxes["framesbetweennodes"]->text.getString().toAnsiString();
	bool t_loop = p->checkBoxes["loop"]->checked;
	
	ss << bulletSpeedStr;

	int t_bulletSpeed;
	ss >> t_bulletSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = t_bulletSpeed;
	}

	/*ss.clear();

	ss << nodeDistanceStr;

	int t_nodeDistance;
	ss >> t_nodeDistance;

	if( !ss.fail() )
	{
		nodeDistance = t_nodeDistance;
	}*/

	ss.clear();

	ss << betweenStr;

	int t_framesBetweenNodes;
	ss >> t_framesBetweenNodes;

	if( !ss.fail() )
	{
		framesBetweenNodes = t_framesBetweenNodes;
	}

	ss.clear();

	loop = t_loop;
}

void BatParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	//p->textBoxes["nodedistance"]->text.setString( boost::lexical_cast<string>( nodeDistance ) );
	p->textBoxes["framesbetweennodes"]->text.setString( boost::lexical_cast<string>( framesBetweenNodes ) );
	p->checkBoxes["loop"]->checked = loop;
	EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

ActorParams *BatParams::Copy()
{
	BatParams *copy = new BatParams( *this );
	if( copy->lines != NULL )
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray( sf::LinesStrip, numVertices );
		VertexArray &li = *copy->lines;
		

		for( int i = 0; i < numVertices; ++i )
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}

PulserParams::PulserParams( EditSession *edit,
	sf::Vector2i &pos,
	std::list<sf::Vector2i> &globalPath, 
	int p_framesBetweenNodes,
	bool p_loop )
	:ActorParams( PosType::AIR_ONLY )
{
	lines = NULL;
	position = pos;	
	type = edit->types["pulser"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	framesBetweenNodes = p_framesBetweenNodes; 

	loop = p_loop;

	SetBoundingQuad();
}

PulserParams::PulserParams( EditSession *edit,
	sf::Vector2i &pos )
	:ActorParams( PosType::AIR_ONLY )
{
	lines = NULL;
	position = pos;	
	type = edit->types["pulser"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	loop = false;
	//speed = 5;
	framesBetweenNodes = 60;

	SetBoundingQuad();
}

void PulserParams::WriteParamFile( std::ofstream &of )
{
	of << (int)monitorType << endl;

	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	of << framesBetweenNodes << endl;
}

void PulserParams::SetPath( std::list<sf::Vector2i> &globalPath )
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}
	
	localPath.clear();
	if( globalPath.size() > 1 )
	{

		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

std::list<sf::Vector2i> PulserParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void PulserParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		
		VertexArray &li = *lines;
	
	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position += Vector2f( position.x, position.y );
		}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	ActorParams::Draw( target );
}

void PulserParams::SetParams()
{
	Panel *p = type->panel;

	stringstream ss;
	string betweenStr = p->textBoxes["framesbetweennodes"]->text.getString().toAnsiString();
	bool t_loop = p->checkBoxes["loop"]->checked;

	ss << betweenStr;

	int t_framesBetweenNodes;
	ss >> t_framesBetweenNodes;

	if( !ss.fail() )
	{
		framesBetweenNodes = t_framesBetweenNodes;
	}

	ss.clear();

	loop = t_loop;
}
void PulserParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
	{
		p->textBoxes["group"]->text.setString( group->name );
	}
	
	p->textBoxes["framesbetweennodes"]->text.setString( boost::lexical_cast<string>( framesBetweenNodes ) );
	p->checkBoxes["loop"]->checked = loop;
	EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

bool PulserParams::CanApply()
{
	return true;
}
ActorParams *PulserParams::Copy()
{
	PulserParams *copy = new PulserParams( *this );
	if( copy->lines != NULL )
	{
		int numVertices = copy->lines->getVertexCount();

		VertexArray &oldli = *copy->lines;
		copy->lines = new VertexArray( sf::LinesStrip, numVertices );
		VertexArray &li = *copy->lines;
		

		for( int i = 0; i < numVertices; ++i )
		{
			li[i] = oldli[i];
		}
	}
	return copy;
}

//STAG BEETLE

StagBeetleParams::StagBeetleParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, bool p_clockwise, float p_speed )
	:ActorParams( PosType::GROUND_ONLY )
{
	clockwise = p_clockwise;
	speed = p_speed;

	type = edit->types["stagbeetle"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

StagBeetleParams::StagBeetleParams( EditSession *edit,
	TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity)
	:ActorParams( PosType::GROUND_ONLY ), clockwise( true ), speed( 0 )
{
	
	speed = 10;
	clockwise = true;
	type = edit->types["stagbeetle"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	
}

bool StagBeetleParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void StagBeetleParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;
	if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;
}

void StagBeetleParams::SetParams()
{
	Panel *p = type->panel;

	bool t_clockwise = p->checkBoxes["clockwise"]->checked;
	double t_speed;

	stringstream ss;
	string s = p->textBoxes["speed"]->text.getString().toAnsiString();
	ss << s;

	ss >> t_speed;

	if( !ss.fail() )
	{
		speed = t_speed;
	}

	//ss.clear();

	clockwise = t_clockwise;

	
}

void StagBeetleParams::SetPanelInfo()
{
	Panel *p = type->panel;

	p->textBoxes["name"]->text.setString( "test" );
	
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->checkBoxes["clockwise"]->checked = clockwise;
	p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( speed ) );

	EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

ActorParams *StagBeetleParams::Copy()
{
	StagBeetleParams *copy = new StagBeetleParams( *this );
	return copy;
}

//STAG BEETLE

PoisonFrogParams::PoisonFrogParams( EditSession *edit, TerrainPolygon *p_edgePolygon, 
	int p_edgeIndex, double p_edgeQuantity)//, bool p_clockwise, float p_speed )
	:ActorParams( PosType::GROUND_ONLY ), pathQuads( sf::Quads, 4 * 50 )
{
	gravFactor = 10;
	jumpWaitFrames = 60;
	jumpStrength = Vector2i( 10, 20 );

	type = edit->types["poisonfrog"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	

	UpdatePath();
}

PoisonFrogParams::PoisonFrogParams( EditSession *edit, TerrainPolygon *p_edgePolygon, 
	int p_edgeIndex, double p_edgeQuantity, int p_gravFactor, sf::Vector2i &p_jumpStrength,
	int p_jumpWaitFrames )
	:ActorParams( PosType::GROUND_ONLY ), pathQuads( sf::Quads, 4 * 50 )
{
	gravFactor = p_gravFactor;
	jumpStrength = p_jumpStrength;
	jumpWaitFrames = p_jumpWaitFrames;
	type = edit->types["poisonfrog"];

	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
				
	SetBoundingQuad();	

	UpdatePath();
}

PoisonFrogParams::PoisonFrogParams( EditSession *edit )
	:ActorParams( PosType::GROUND_ONLY )//, clockwise( true ), speed( 0 )
{
	
	type = edit->types["poisonfrog"];
}

bool PoisonFrogParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void PoisonFrogParams::WriteParamFile( ofstream &of )
{
	of << (int)monitorType << endl;
	of << gravFactor << endl;
	of << jumpStrength.x << " " << jumpStrength.y << endl;
	of << jumpWaitFrames << endl;
	/*if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;*/
}

void PoisonFrogParams::SetParams()
{
	Panel *p = type->panel;

	//bool clockwise = p->checkBoxes["clockwise"]->checked;
	//double speed;

	stringstream ss;
	string xStrengthStr = p->textBoxes["xstrength"]->text.getString().toAnsiString();
	string yStrengthStr = p->textBoxes["ystrength"]->text.getString().toAnsiString();
	string jumpWaitFramesStr = p->textBoxes["jumpwaitframes"]->text.getString().toAnsiString();
	string gravityFactorStr = p->textBoxes["gravfactor"]->text.getString().toAnsiString();
	
	ss << xStrengthStr;

	int t_xStrength;
	ss >> t_xStrength;

	if( !ss.fail() )
	{
		jumpStrength.x = t_xStrength;
	}

	ss.clear();

	ss << yStrengthStr;

	int t_yStrength;
	ss >> t_yStrength;

	if( !ss.fail() )
	{
		jumpStrength.y = t_yStrength;
	}

	ss.clear();

	ss << jumpWaitFramesStr;

	int t_jumpWaitFrames;
	ss >> t_jumpWaitFrames;

	if( !ss.fail() )
	{
		jumpWaitFrames = t_jumpWaitFrames;
	}

	ss.clear();

	ss << gravityFactorStr;

	int t_gravFactor;
	ss >> t_gravFactor;

	if( !ss.fail() )
	{
		gravFactor = t_gravFactor;
	}

	UpdatePath();
}

void PoisonFrogParams::SetPanelInfo()
{
	Panel *p = type->panel;
	
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["jumpwaitframes"]->text.setString( boost::lexical_cast<string>( jumpWaitFrames ) ); 
	p->textBoxes["xstrength"]->text.setString( boost::lexical_cast<string>( jumpStrength.x ) ); 
	p->textBoxes["ystrength"]->text.setString( boost::lexical_cast<string>( jumpStrength.y ) ); 
	p->textBoxes["gravfactor"]->text.setString( boost::lexical_cast<string>( gravFactor ) ); 
	EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

void PoisonFrogParams::UpdatePath()
{
	int totalQuads = 50;

	if( groundInfo == NULL )
	{
		for( int i = 0; i < totalQuads; ++i )
		{
			pathQuads[i*4+0].position = Vector2f( 0,0 );
			pathQuads[i*4+1].position = Vector2f( 0,0 );
			pathQuads[i*4+2].position = Vector2f( 0,0 );
			pathQuads[i*4+3].position = Vector2f( 0,0 );
		}
		return;
	}

	Color pathColor( 0, 255, 0 );
	
	int squareRad = 4;// * EditSession::zoomMultiple;
	Vector2f pos( position.x, position.y );

	Vector2f gravity( 0, gravFactor / 64.f );

	V2d fireDir;
	TerrainPoint *curr = groundInfo->edgeStart;
	TerrainPoint *next;

	if( curr->next == NULL )
		next = groundInfo->ground->pointStart;
	else
	{
		next = curr->next;
	}

	V2d e( next->pos.x - curr->pos.x, next->pos.y - curr->pos.y );
	e = normalize( e );
	e = V2d( e.y, -e.x );

	

	//negative x jumpstrength means you face left
	Vector2f vel = Vector2f( jumpStrength.x, -jumpStrength.y );
	
	if( e.y > 0 )
	{
		gravity.y = -gravity.y;
		vel.y = -vel.y;
	}
	
	for( int i = 0; i < totalQuads; ++i )
	{
		//cout << "i: " << i << endl;
		pathQuads[i*4+0].position = Vector2f( pos.x - squareRad,
			pos.y - squareRad );
		pathQuads[i*4+1].position = Vector2f( pos.x + squareRad,
			pos.y - squareRad );
		pathQuads[i*4+2].position = Vector2f( pos.x + squareRad,
			pos.y + squareRad );
		pathQuads[i*4+3].position = Vector2f( pos.x - squareRad,
			pos.y + squareRad );

		pathQuads[i*4+0].color = pathColor;
		pathQuads[i*4+1].color = pathColor;
		pathQuads[i*4+2].color = pathColor;
		pathQuads[i*4+3].color = pathColor;

		pos += vel;
		vel += gravity;
	}
}

void PoisonFrogParams::Draw( sf::RenderTarget *target )
{
	ActorParams::Draw( target );

	target->draw( pathQuads );
}

void PoisonFrogParams::UpdateExtraVisuals()
{
	UpdatePath();
}

ActorParams *PoisonFrogParams::Copy()
{
	PoisonFrogParams *copy = new PoisonFrogParams( *this );
	return copy;
}

CurveTurretParams::CurveTurretParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, double p_bulletSpeed, int p_framesWait,
	sf::Vector2i p_gravFactor, bool relative )
	:ActorParams( PosType::GROUND_ONLY ), bulletPathQuads( sf::Quads, 100 * 4 )
{
	bulletSpeed = p_bulletSpeed;
	framesWait = p_framesWait;
	gravFactor = p_gravFactor;
	relativeGrav = relative;

	type = edit->types["curveturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();

	//UpdateBulletCurve();
}

CurveTurretParams::CurveTurretParams( EditSession *edit,
		TerrainPolygon *p_edgePolygon,
		int p_edgeIndex, double p_edgeQuantity )
		:ActorParams( PosType::GROUND_ONLY ), bulletPathQuads( sf::Quads, 100 * 4 )
{
	type = edit->types["curveturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	gravFactor = Vector2i( 0, 0 );
	framesWait = 60;
	relativeGrav = true;
	bulletSpeed = 1;

	SetBoundingQuad();

	
}

void CurveTurretParams::UpdateExtraVisuals()
{
	UpdateBulletCurve();
}

bool CurveTurretParams::CanApply()
{
	if( groundInfo != NULL )
		return true;
	//hmm not sure about this now

	return false;
}

void CurveTurretParams::WriteParamFile( ofstream &of )
{
	//cout << "write curve turret params. this: " << (int)this << endl;
	of << (int)monitorType << endl;
	of << bulletSpeed << endl;
	of << framesWait << endl;
	of << gravFactor.x << endl;
	of << gravFactor.y << endl;

	if( relativeGrav )
	{
		of << "+relative" << endl;
	}
	else
	{
		of << "-relative" << endl;
	}
	
}

void CurveTurretParams::UpdateBulletCurve()
{
	int totalQuads = 100;

	double factorGrav = 256;

	if( groundInfo == NULL )
	{
		for( int i = 0; i < totalQuads; ++i )
		{
			bulletPathQuads[i*4+0].position = Vector2f( 0,0 );
			bulletPathQuads[i*4+1].position = Vector2f( 0,0 );
			bulletPathQuads[i*4+2].position = Vector2f( 0,0 );
			bulletPathQuads[i*4+3].position = Vector2f( 0,0 );
		}
		return;
	}

	Color pathColor( 255, 0, 0 );
	int squareRad = 4;// * EditSession::zoomMultiple;
	Vector2f pos( position.x, position.y );
	V2d fireDir;
	TerrainPoint *curr = groundInfo->edgeStart;
	TerrainPoint *next;
	if( curr->next == NULL )
		next = groundInfo->ground->pointStart;
	else
	{
		next = curr->next;
	}

	V2d e( next->pos.x - curr->pos.x, next->pos.y - curr->pos.y );
	V2d groundDir = normalize( e );
	e = V2d( groundDir.y, -groundDir.x );

	Vector2f bulletVel = Vector2f( e.x, e.y ) * bulletSpeed;
	
	for( int i = 0; i < totalQuads; ++i )
	{
		//cout << "i: " << i << endl;
		bulletPathQuads[i*4+0].position = Vector2f( pos.x - squareRad,
			pos.y - squareRad );
		bulletPathQuads[i*4+1].position = Vector2f( pos.x + squareRad,
			pos.y - squareRad );
		bulletPathQuads[i*4+2].position = Vector2f( pos.x + squareRad,
			pos.y + squareRad );
		bulletPathQuads[i*4+3].position = Vector2f( pos.x - squareRad,
			pos.y + squareRad );

		bulletPathQuads[i*4+0].color = pathColor;
		bulletPathQuads[i*4+1].color = pathColor;
		bulletPathQuads[i*4+2].color = pathColor;
		bulletPathQuads[i*4+3].color = pathColor;

		pos += bulletVel;

		V2d trueGrav( 0, 0 );//( gravFactor.x, gravFactor.y );

		if( relativeGrav )
		{
			trueGrav += groundDir * ( gravFactor.x / factorGrav );
			trueGrav += e * (-gravFactor.y / factorGrav );
		}
		else
		{
			trueGrav = V2d( gravFactor.x / factorGrav, 
				gravFactor.y / factorGrav );
		}
		
		//trueGrav.x = e.y * gravFactor.x;
		//trueGrav.y = e.x * gravFactor.y;

		bulletVel += Vector2f( trueGrav.x, trueGrav.y );
	}
}

void CurveTurretParams::SetParams()
{
	Panel *p = type->panel;

	

	stringstream ss;
	string bulletSpeedString = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
	string framesWaitString = p->textBoxes["waitframes"]->text.getString().toAnsiString();
	string xGravString = p->textBoxes["xgravfactor"]->text.getString().toAnsiString();
	string yGravString = p->textBoxes["ygravfactor"]->text.getString().toAnsiString();
	
	relativeGrav = p->checkBoxes["relativegrav"]->checked;

	ss << bulletSpeedString;

	double t_bulletSpeed;
	ss >> t_bulletSpeed;

	if( !ss.fail() )
	{
		bulletSpeed = t_bulletSpeed;
		//assert( false );
	}

	ss.clear();

	ss << framesWaitString;

	int t_framesWait;
	ss >> t_framesWait;

	if( !ss.fail() )
	{
		framesWait = t_framesWait;
	}

	ss.clear();
			
	int t_xGravFactor;
	ss << xGravString;

	ss >> t_xGravFactor;

	if( !ss.fail() )
	{
		gravFactor.x = t_xGravFactor;
	}

	ss.clear();

	int t_yGravFactor;
	ss << yGravString;

	ss >> t_yGravFactor;

	if( !ss.fail() )
	{
		gravFactor.y = t_yGravFactor;
	}

	UpdateBulletCurve();
	//also set up visuals
}

void CurveTurretParams::SetPanelInfo()
{
	Panel *p = type->panel;
	p->textBoxes["name"]->text.setString( "test" );
	if( group != NULL )
		p->textBoxes["group"]->text.setString( group->name );
	p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( bulletSpeed ) );
	p->textBoxes["waitframes"]->text.setString( boost::lexical_cast<string>( framesWait ) );
	p->textBoxes["xgravfactor"]->text.setString( boost::lexical_cast<string>( gravFactor.x ) );
	p->textBoxes["ygravfactor"]->text.setString( boost::lexical_cast<string>( gravFactor.y ) );
	p->checkBoxes["relativegrav"]->checked = relativeGrav;
	EditSession::SetMonitorGrid( monitorType, p->gridSelectors["monitortype"] );
}

void CurveTurretParams::Draw( RenderTarget *target )
{
	ActorParams::Draw( target );

	target->draw( bulletPathQuads );
}

ActorParams *CurveTurretParams::Copy()
{
	CurveTurretParams *copy = new CurveTurretParams( *this );
	return copy;
}