#include "human_decision_maker_tableware_alg_node.h"

HumanDecisionMakerTablewareAlgNode::HumanDecisionMakerTablewareAlgNode
    (void):algorithm_base::IriBaseAlgorithm <
    HumanDecisionMakerTablewareAlgorithm > (),
pick_place_tabl_client_("pick_place_tabl", true),
server_IM("desired_pos_marker")
{
	//init class attributes if necessary
	this->My_State = 0;

	this->place_point.X = 0;
	this->place_point.Y = 0;
	this->place_point.Z = 0;
	this->pick_point.X = 0;
	this->pick_point.Y = 0;
	this->pick_point.Z = 0;

	//init Interactive Marker
	init_interactive_markers(this->server_IM);

	//this->loop_rate_ = 2;//in [Hz]

	// [init publishers]

	// [init subscribers]

	// [init services]

	// [init clients]
	interest_points_client_ =
	    this->public_node_handle_.serviceClient <
	    iri_color_interesting_points_tableware::InterestPoints >
	    ("interest_points");

	// [init action servers]

	// [init action clients]
}

HumanDecisionMakerTablewareAlgNode::~HumanDecisionMakerTablewareAlgNode(void)
{
	// [free dynamic memory]
}

void HumanDecisionMakerTablewareAlgNode::mainNodeThread(void)
{
	// [fill msg structures]

	// [fill srv structure and make request to the server]

	// [fill action structure and make request to the action server]

	// [publish messages]

	/// My state machine
	// Estat=0 => Ask the server for the points of interest 
	// Estat=1 => Ask the human which point of interest pick
	// Estat=2 => Ask to the human for the interactive Marker..
	// Estat=3 => Call Pick & Place 
	// Estat=4 => Wait Pick & Place 

	if (this->My_State == 0) {	// Ask the server and store all points of interest

		Num_Int_Points = 0;
		ListInterestPoints.clear();

		//Send Interest Points request
		ROS_INFO("TestHumanDmTablewareAlgNode:: Sending New Request!");

		if (interest_points_client_.call(interest_points_srv_)) {
			ROS_INFO
			    ("TestHumanDmTablewareAlgNode:: We have Response");
			//Store Points of Interest
			Num_Int_Points = interest_points_srv_.response.amount;
			int i = 0;
			this->ListInterestPoints.resize(Num_Int_Points);
			while (i < Num_Int_Points) {
				ListInterestPoints[i].U =
				    interest_points_srv_.response.U[i];
				ListInterestPoints[i].V =
				    interest_points_srv_.response.V[i];
				ListInterestPoints[i].X =
				    interest_points_srv_.response.X[i];
				ListInterestPoints[i].Y =
				    interest_points_srv_.response.Y[i];
				ListInterestPoints[i].Z =
				    interest_points_srv_.response.Z[i];
				ListInterestPoints[i].color =
				    interest_points_srv_.response.Color[i];
				ListInterestPoints[i].object_type =
				    interest_points_srv_.
				    response.object_type[i];
				i++;
			}
		}
		else {		//The request Failed, so we stay in this state 
			ROS_INFO
			    ("TestHumanDmTablewareAlgNode:: Failed to Call Server on topic interest_points ");
		}

		if (Num_Int_Points > 0)
			My_State++;
		else
			ROS_INFO
			    ("TestHumanDmTablewareAlgNode:: NO Interested Points obtained Trying Again!");

	}
	else if (this->My_State == 1) {	// Ask Human where to move

		//Some interested points has been obtained
		ListGraspPoints.clear();

		// Ask to select one...
		ROS_INFO("[MAIN]: HEY HUMAN! ");
		ROS_INFO("[MAIN]: Where do you want to move the robot?");
		int i = 0;
		int Color_Point_ant = -1;

		//Show diff colors detected
		while (i < Num_Int_Points) {
			if (ListInterestPoints[i].color != Color_Point_ant) {
				ROS_INFO("#%d", ListInterestPoints[i].color);
			}
			Color_Point_ant = ListInterestPoints[i].color;
			i++;
		}

		// Human entrance
		ROS_INFO("[MAIN]: Insert Number:");
		int GraspPointColor = -1;
		std::cin >> GraspPointColor;

		//Check the entrance
		bool trobat = false;
		i = 0;
		while (i < Num_Int_Points && !trobat) {
			if (ListInterestPoints[i].color == GraspPointColor)
				trobat = true;
			else
				i++;
		}

		//Add all the grasp points of the object/color selected by the human
		if (trobat) {
			while (i < Num_Int_Points) {
				if (ListInterestPoints[i].color ==
				    GraspPointColor) {
					ListGraspPoints.push_back
					    (ListInterestPoints[i]);
				}
				i++;
			}
			My_State++;
		}
		else {
			ROS_INFO
			    ("[MAIN]: Wrong number inserted. Asking new Interested Points");
			My_State = 0;
		}

	}
	else if (this->My_State == 2) {	//Interactive Marker

		int option = 0;

		Num_Trials = ListGraspPoints.size();

		Centroid_Point.X =
		    ListGraspPoints[ListGraspPoints.size() - 1].X;
		Centroid_Point.Y =
		    ListGraspPoints[ListGraspPoints.size() - 1].Y;
		Centroid_Point.Z =
		    ListGraspPoints[ListGraspPoints.size() - 1].Z;
		ListGraspPoints.pop_back();
		Num_Trials--;

		do {
			ROS_INFO("[MAIN]: OK! We have %d picking point trials",
				 Num_Trials);
			ROS_INFO
			    ("[MAIN]: What about the Drop point: (%.4f,%.4f,%.4f) ?",
			     place_point.X, place_point.Y, place_point.Z);
			ROS_INFO
			    ("[MAIN]: If it is not OK move the Interactive Marker. Is it OK? [1=Yes/0=N]");

			std::cin >> option;

		} while (option != 1);

		My_State++;

	}
	else if (this->My_State == 3) {	//Pick & Place

		ROS_INFO("[MAIN]: Trial # %d", Num_Trials);

		if (Num_Trials > 0) {
			if (ListGraspPoints[0].object_type == 'C') {	//Grasping Centroide (cloth)
				pick_point.X = ListGraspPoints[0].X;
				pick_point.Y = ListGraspPoints[0].Y;
				pick_point.Z = ListGraspPoints[0].Z;

				pick_place_tablMakeActionRequest();

			}
			else if (ListGraspPoints[0].object_type == 'g') {	//Grasping Glass
				pick_point.X = ListGraspPoints[0].X;
				pick_point.Y = ListGraspPoints[0].Y;
				pick_point.Z = ListGraspPoints[0].Z;

				table_stuff = 'g';	//103;

				pick_place_tablMakeActionRequest();

			}
			else if (ListGraspPoints[0].object_type == 'p') {	//Grasping Plate
				pick_point.X = ListGraspPoints[0].X;
				pick_point.Y = ListGraspPoints[0].Y;
				pick_point.Z = ListGraspPoints[0].Z;

				table_stuff = 'p';	//112;

				pick_place_tablMakeActionRequest();

			}

			ListGraspPoints.erase(ListGraspPoints.begin());
			Num_Trials--;

			My_State = 4;
		}
		else
			My_State = 0;

	}

}

/*  [subscriber callbacks] */

/*  [service callbacks] */

/*  [action callbacks] */
void HumanDecisionMakerTablewareAlgNode::
pick_place_tablDone(const actionlib::SimpleClientGoalState & state,
		    const iri_pickandplace_tableware::PaPTablewareResultConstPtr
		    & result)
{
	alg_.lock();
	if (state.toString().compare("SUCCEEDED") == 0) {
		ROS_INFO
		    ("HumanDecisionMakerTablewareAlgNode::pick_place_tablDone: Goal Achieved!");
		My_State = 0;
	}
	else {
		ROS_INFO
		    ("HumanDecisionMakerTablewareAlgNode::pick_place_tablDone: %s",
		     state.toString().c_str());

		if (Num_Trials > 0)
			My_State = 3;
		else
			My_State = 0;

	}
	//copy & work with requested result 
	alg_.unlock();
}

void HumanDecisionMakerTablewareAlgNode::pick_place_tablActive()
{
	alg_.lock();
	//ROS_INFO("HumanDecisionMakerTablewareAlgNode::pick_place_tablActive: Goal just went active!"); 
	alg_.unlock();
}

void HumanDecisionMakerTablewareAlgNode::
pick_place_tablFeedback(const
			iri_pickandplace_tableware::PaPTablewareFeedbackConstPtr
			& feedback)
{
	alg_.lock();
	//ROS_INFO("HumanDecisionMakerTablewareAlgNode::pick_place_tablFeedback: Got Feedback!"); 

	bool feedback_is_ok = true;

	//analyze feedback 
	//my_var = feedback->var; 

	//if feedback is not what expected, cancel requested goal 
	if (!feedback_is_ok) {
		pick_place_tabl_client_.cancelGoal();
		//ROS_INFO("HumanDecisionMakerTablewareAlgNode::pick_place_tablFeedback: Cancelling Action!"); 
	}
	alg_.unlock();
}

/*  [action requests] */
void HumanDecisionMakerTablewareAlgNode::pick_place_tablMakeActionRequest()
{
	ROS_INFO
	    ("HumanDecisionMakerTablewareAlgNode::pick_place_tablMakeActionRequest: Starting New Request!");

	//wait for the action server to start 
	//will wait for infinite time 
	pick_place_tabl_client_.waitForServer();
	ROS_INFO
	    ("HumanDecisionMakerTablewareAlgNode::pick_place_tablMakeActionRequest: Server is Available!");

	//send a goal to the action 
	pick_place_tabl_goal_.pick_pos[0] = pick_point.X;
	pick_place_tabl_goal_.pick_pos[1] = pick_point.Y;
	pick_place_tabl_goal_.pick_pos[2] = pick_point.Z;

	pick_place_tabl_goal_.place_pos[0] = place_point.X;
	pick_place_tabl_goal_.place_pos[1] = place_point.Y;
	pick_place_tabl_goal_.place_pos[2] = place_point.Z;

	pick_place_tabl_goal_.centroid_pos[0] = Centroid_Point.X;
	pick_place_tabl_goal_.centroid_pos[1] = Centroid_Point.Y;
	pick_place_tabl_goal_.centroid_pos[2] = Centroid_Point.Z;

	pick_place_tabl_goal_.object_type = table_stuff;

	//pick_place_tabl_goal_.data = my_desired_goal; 
	pick_place_tabl_client_.sendGoal(pick_place_tabl_goal_,
					 boost::bind
					 (&HumanDecisionMakerTablewareAlgNode::pick_place_tablDone,
					  this, _1, _2),
					 boost::bind
					 (&HumanDecisionMakerTablewareAlgNode::pick_place_tablActive,
					  this),
					 boost::bind
					 (&HumanDecisionMakerTablewareAlgNode::pick_place_tablFeedback,
					  this, _1));
	ROS_INFO
	    ("HumanDecisionMakerTablewareAlgNode::pick_place_tablMakeActionRequest: Goal Sent. Wait for Result!");
}

void HumanDecisionMakerTablewareAlgNode::node_config_update(Config & config,
							    uint32_t level)
{
	this->alg_.lock();

	this->alg_.unlock();
}

void HumanDecisionMakerTablewareAlgNode::addNodeDiagnostics(void)
{
}

/* INTERACTIVE MARKERS */

void HumanDecisionMakerTablewareAlgNode::
processFeedback(const visualization_msgs::InteractiveMarkerFeedbackConstPtr &
		feedback)
{
//  ROS_INFO_STREAM( feedback->marker_name << " is now at "
//      << feedback->pose.position.x << ", " << feedback->pose.position.y
//      << ", " << feedback->pose.position.z );

	this->place_point.X = feedback->pose.position.x;
	this->place_point.Y = feedback->pose.position.y;
	this->place_point.Z = feedback->pose.position.z;

}

void HumanDecisionMakerTablewareAlgNode::
init_interactive_markers(interactive_markers::InteractiveMarkerServer & server)
{

	ROS_INFO("[INTERACTIVE_MARKER]: Init!!");

	// create an interactive marker for our server
	visualization_msgs::InteractiveMarker int_marker;
	int_marker.header.frame_id = "camera_rgb_optical_frame";
	int_marker.name = "my_marker";
	int_marker.description = "Desired position";

	// create a grey box marker
	visualization_msgs::Marker box_marker;
	box_marker.type = visualization_msgs::Marker::CUBE;
	box_marker.scale.x = 0.045;
	box_marker.scale.y = 0.045;
	box_marker.scale.z = 0.045;
	box_marker.color.r = 0.5;
	box_marker.color.g = 0.5;
	box_marker.color.b = 0.5;
	box_marker.color.a = 1.0;

	// create a non-interactive control which contains the box
	visualization_msgs::InteractiveMarkerControl box_control;
	box_control.always_visible = true;
	box_control.markers.push_back(box_marker);

	// add the control to the interactive marker
	int_marker.controls.push_back(box_control);

	// create a control which will move the box
	// this control does not contain any markers,
	// which will cause RViz to insert two arrows
	visualization_msgs::InteractiveMarkerControl rotate_control;
	rotate_control.name = "move_x";
	rotate_control.interaction_mode =
	    visualization_msgs::InteractiveMarkerControl::MOVE_AXIS;

	// add the control to the interactive marker
	int_marker.controls.push_back(rotate_control);

	//Y AXIS
	rotate_control.orientation.w = 1;
	rotate_control.orientation.x = 0;
	rotate_control.orientation.y = 0;
	rotate_control.orientation.z = 1;
	rotate_control.name = "move_y";
	rotate_control.interaction_mode =
	    visualization_msgs::InteractiveMarkerControl::MOVE_AXIS;
	int_marker.controls.push_back(rotate_control);

	//Z AXIS
	rotate_control.orientation.w = 1;
	rotate_control.orientation.x = 0;
	rotate_control.orientation.y = 1;
	rotate_control.orientation.z = 0;
	rotate_control.name = "move_z";
	rotate_control.interaction_mode =
	    visualization_msgs::InteractiveMarkerControl::MOVE_AXIS;
	int_marker.controls.push_back(rotate_control);

	// add the interactive marker to our collection &
	// tell the server to call processFeedback() when feedback arrives for it
	server.insert(int_marker,
		      boost::bind
		      (&HumanDecisionMakerTablewareAlgNode::processFeedback,
		       this, _1));

	// 'commit' changes and send to all clients
	server.applyChanges();

	ROS_INFO("[INTERACTIVE_MARKER]: FII Init!!");

}

/* main function */
int main(int argc, char *argv[])
{
	return algorithm_base::main < HumanDecisionMakerTablewareAlgNode >
	    (argc, argv, "human_decision_maker_tableware_alg_node");
}
