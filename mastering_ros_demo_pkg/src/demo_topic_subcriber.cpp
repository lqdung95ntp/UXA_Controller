#include "ros/ros.h"
#include "std_msgs/Int32.h"
#include <iostream>
//this function will excute whenever a data comes to /numbers topic
void number_callback(const std_msgs::Int32::ConstPtr& msg){
    ROS_INFO("Received: [%d]", msg->data);
}

int main(int argc, char **argv){
    ros::init(argc, argv, "demo_topic_subcriber");
    ros::NodeHandle node_obj;
    ros::Subscriber number_subcriber=node_obj.subscribe("/numbers", 10, number_callback);
    ros::spin();
    return 0;
}