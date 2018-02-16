#include <ros/ros.h>
#include <image_transport/image_transport.h>//this lib ro publishing and subcribing image in ROS
#include <cv_bridge/cv_bridge.h>
#include <std_msgs/Int32.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
#include <string>
#include <sstream>

#define OPEN_EYE       1
#define TRACKING_COLOR 2
#define TRACKING_FACE  3
#define TAKE_PICTURE   4
int command;

class Tracking{
    ros::NodeHandle n;
    // ros::Subscriber receivedMsg = n.subscribe("/Face_Control",10, Tracking::callbackReceivedMsg);
    image_transport::ImageTransport it_;//tao ra doi tuong transport de publish va subcribe anh trong ROS
    //doi tuong nay de truyen va nhan anh trong ROS
    image_transport::Subscriber image_sub_;
    image_transport::Publisher  image_pub_;
public:
    std::string cascade_source;
    
    
    Tracking(std::string casc_source)
    : it_(n)
    {
        // system("roslaunch usb_cam usb_cam-test.launch");
        cascade_source=casc_source;
        image_sub_ = it_.subscribe("/usb_cam/image_raw",1,&Tracking::imageCb,this);//declare image_sub_ and _pub_ object
        image_pub_ = it_.advertise("/robot_head/tracking_face",1);
        // cv::namedWindow(OPENCV_WINDOW);
        ROS_INFO("Start Tracking");
    }
    ~Tracking()
    {
        // cv::destroyWindow(OPENCV_WINDOW);
        ROS_INFO("Stop Tracking");
    }
    //convert openCV - ROS image
    void imageCb(const  sensor_msgs::ImageConstPtr& msg){
        cv_bridge::CvImagePtr cv_ptr;
        namespace enc=sensor_msgs::image_encodings;
        try
        {
            cv_ptr=cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        }
        catch (cv_bridge::Exception& e)
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }
        if (cv_ptr->image.rows>400 && cv_ptr->image.cols>600)
        {
            switch (command){
                case TRACKING_COLOR:
                  tracking_color(cv_ptr->image);
                break;
                case TRACKING_FACE:
                  tracking_face(cv_ptr->image);
                break;
                case TAKE_PICTURE:
                // take_picture(cv_ptr->image);
                break;
                default:
                break;
            }
        }
    }
    //tracking face function
    cv::CascadeClassifier face;
    void tracking_face(cv::Mat img){
        std::vector<cv::Rect> faces;
        cv::Mat frame_gray;
        cv::cvtColor(img, frame_gray, CV_BGR2GRAY);
        cv::equalizeHist(frame_gray, frame_gray);
        if (!face.load(cascade_source)){
            ROS_INFO("File not found");
        }
        else {
            face.detectMultiScale(frame_gray, faces, 1.1, 3, 0, cv::Size(50, 50));
            for (size_t i=0; i<faces.size(); i++) {
                cv::Point center(faces[i].x+faces[i].width*0.5, faces[i].y+faces[i].height*0.5) ;
                cv::ellipse(img, center, cv::Size(faces[i].width*0.5, faces[i].height*0.5), 0,0, 360, cv::Scalar(255,0,255),4, 8,0);
            }
            // ROS_INFO("Face: %d", faces.size());
            // cv::imshow("Face", img);
            sensor_msgs::ImagePtr msg=cv_bridge::CvImage(std_msgs::Header(), "bgr8", img).toImageMsg();
            image_pub_.publish(msg);
        }
        cv::waitKey(3);
    }
    //tracking color
    void tracking_color(cv::Mat img){
        int thresh=20;
        cv::Mat hsvImg(img.size(), CV_32FC3);
        cv::cvtColor(img, hsvImg, CV_BGR2HSV);
        cv::Mat thresholdImg;
        cv::Mat canny_output;
        inRange(hsvImg, cv::Scalar(49, 60, 111), cv::Scalar(85, 255, 255), thresholdImg);
        cv::GaussianBlur(thresholdImg, thresholdImg, cv::Size(9,9),2,2);
        cv::medianBlur(thresholdImg, thresholdImg, 5);
        cv::dilate(thresholdImg, thresholdImg, cv::Mat(), cv::Point(-1,-1),2,1,1);
        cv::erode(thresholdImg, thresholdImg, cv::Mat(), cv::Point(-1,-1), 2, 1, 1);
        
        std::vector<cv::Vec4i> hierarchy;
        cv::Canny(thresholdImg, canny_output, thresh, thresh*2, 3);
        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(thresholdImg, circles, CV_HOUGH_GRADIENT, 1, thresholdImg.rows/16, 80, 30, 10 , 100);
        // ROS_INFO("Circles: %d", circles.size());
        for (int i=0; i<circles.size();i++) {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius=cvRound(circles[i][2]);
        
            cv::circle(img, center, radius, cv::Scalar(0, 0, 255), 3, 8,0);
        }
        sensor_msgs::ImagePtr msg=cv_bridge::CvImage(std_msgs::Header(), "bgr8", img).toImageMsg();
        image_pub_.publish(msg);
        // cv::imshow("Color", img);
        // cv::imshow("Contour", canny_output);
        cv::waitKey(3);
    }


};
//received message callback function
void callbackReceivedMsg(const std_msgs::Int32::ConstPtr& msg){
    ROS_INFO("Received: [%d]", msg->data);
    switch (msg->data){
        case OPEN_EYE:
        command=OPEN_EYE;
        case TRACKING_FACE:
        command=TRACKING_FACE;
        case TRACKING_COLOR:
        command=TRACKING_COLOR;
        case TAKE_PICTURE:
        command=TAKE_PICTURE;
        command=msg->data;
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "RobotHead");
    ros::NodeHandle n;
    ros::Subscriber receivedMsg = n.subscribe("/Face_Control",10, callbackReceivedMsg);
    // if (command==OPEN_EYE)
    Tracking ic(argv[1]);
    ros::spin();
    return 0;
}