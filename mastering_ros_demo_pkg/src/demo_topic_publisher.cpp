#include "ros/ros.h" //main ROS header file
#include "std_msgs/Int32.h" //standard message definition of integer datatype
#include "std_msgs/String.h"
#include <iostream>

int data;

void callbackReceivedMsg(const std_msgs::String::ConstPtr& msg){
    ROS_INFO("Received: [%s] ", msg->data.c_str());
    if (msg->data.c_str()=="OpenEye") data=1;
}

int main(int argc, char **argv){
    //we need to send a integer value throught a topic, so, we need a message type to handling the integer data

    ros::init(argc, argv, "demo_topic_publisher");// initialize a ROS node with a name
    // the topic should be uniqe.
    ros::NodeHandle node_obj;    //this code create a Nodehandle object to communicate with ROS system.
    ros::Publisher number_publisher = node_obj.advertise<std_msgs::Int32>("/numbers", 10);
    ros::Subscriber receivedMsg = node_obj.subscribe("/Face_Control", 10, callbackReceivedMsg);
    //create a topic publisher and name the topic "/numbers" with message type is Int32; "10" is buffer size
    //it indicate that how many messages need to be put in a buffer before sending.
    ros::Rate rate_loop(5);

    int number_count=0;
    while (ros::ok()){
        std_msgs::Int32 msg;// create an integer ROS message
        
        msg.data=number_count;//assign value to the message.
        // if (data)        
{
        ROS_INFO("%d", msg.data);
        number_publisher.publish(msg);//publish the message to the topic "/numbers"
       }
              ros::spinOnce();//this command will read and update all ROS topics. Node will not publish without spin() or spinOnce()
        rate_loop.sleep();
        ++number_count;
    }
    return 0;
}