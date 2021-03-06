#include <ros/ros.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <stdio.h>
#include <zaryabot_bringup/working_state.h>
#include <zaryabot_bringup/zarya.h>

using namespace std;
using namespace cv;

Mat src;
Mat color_src;
Mat temp[9];
Mat result[9];
string temp_name[9];
double minVal[9];
double maxVal[9];
int number_f;
int number_lf;
int counter = 1;
int ans_counter = 1;
int ans;

bool start(false);
void load_template();
void MatchingMethod(int i);
bool working(zaryabot_bringup::working_state::Request &req ,zaryabot_bringup::working_state::Response &res);

int main(int argc, char **argv)
{
//ros initialize region
    ros::init(argc, argv, "number_detect");
    ros::NodeHandle nh;
    std_msgs::Header header;
    header.seq = 0;
    header.stamp = ros::Time::now();

    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub = it.advertise("webcam/image2",1);
    ros::ServiceServer server = nh.advertiseService("number_detect",working);
    ros::Publisher number_pub = nh.advertise<zaryabot_bringup::zarya>("/number",1);

    load_template();

    cv::VideoCapture cap(1);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,240);
    cap.set(CV_CAP_PROP_FRAME_WIDTH,320);

    ros::Rate r(30);
    while(ros::ok())
    {
        ros::spinOnce();
        cap >> color_src;
        cvtColor(color_src, src ,CV_BGR2GRAY);
        cv_bridge::CvImage cvimage(header,sensor_msgs::image_encodings::MONO8,src);
        pub.publish(cvimage.toImageMsg());
        if(start)
        {

        for(int i = 0 ; i < 9 ;i++)
        {
            MatchingMethod(i);
        }


        double max_temp = maxVal[0];

        for(int i = 0 ; i < 9 ;i++)
        {
            if ( max_temp <= maxVal[i] )
             {
                 max_temp =  maxVal[i];
             }

        }


        for(int i = 0; i < 9; i++)
        {
            //find number in this frame
            if(max_temp == maxVal[i])
              {
                  number_f = i + 1;

              }

        }
        ROS_INFO("number_f:%d",number_f);
        if(number_lf == number_f)
        {
            counter++;


        if(counter > ans_counter)
        {
            ans = number_lf;
            ans_counter = counter;
        }
        }
        else
        {
            counter = 1;
        }
        ROS_INFO("counter: %d",counter);
        ROS_INFO("ans_counter: %d",ans_counter);
        if(ans_counter == 50)
        {
            ROS_INFO("the number is %d",ans);
            zaryabot_bringup::zarya number_to_punch_;
            number_to_punch_.number_to_punch = ans;
            number_pub.publish(number_to_punch_);
            return 0;

        }
        number_lf = number_f;

        }

        r.sleep();
    }



}
bool working(zaryabot_bringup::working_state::Request &req, zaryabot_bringup::working_state::Response &res)
{
    start = req.enable;
    if(start)
        ROS_INFO("WORKING~");
    else
        ROS_INFO("SHUT DOWN!!!");
    res.working = start;

}
void load_template()
{
    for(int i = 0; i < 9 ;i++ ){
        int j = i + 1;
        temp_name[i] = format("/home/lin/number_temp/number%d.png",j);
        temp[i] = imread(temp_name[i], IMREAD_GRAYSCALE);

    }
//    temp_name[0] = "/home/lin/opencv-3.2.0/data/haarcascades/number1.png";
//    temp_name[1] = "/home/lin/opencv-3.2.0/data/haarcascades/number2.png";
//    temp_name[2] = "/home/lin/opencv-3.2.0/data/haarcascades/number3.png";
//    temp_name[3] = "/home/lin/opencv-3.2.0/data/haarcascades/number4.png";
//    temp_name[4] = "/home/lin/opencv-3.2.0/data/haarcascades/number5.png";
//    temp_name[5] = "/home/lin/opencv-3.2.0/data/haarcascades/number6.png";
//    temp_name[6] = "/home/lin/opencv-3.2.0/data/haarcascades/number7.png";
//    temp_name[7] = "/home/lin/opencv-3.2.0/data/haarcascades/number8.png";
//    temp_name[8] = "/home/lin/opencv-3.2.0/data/haarcascades/number9.png";


}

void MatchingMethod(int i)
{
    int result_cols;
    int result_rows;

    result_cols = src.cols - temp[i].cols + 1;
    result_rows = src.rows - temp[i].rows + 1;
    result[i].create(result_cols,result_rows,CV_8UC1);

    cv::matchTemplate(src,temp[i],result[i],CV_TM_CCOEFF_NORMED);
    //MatchMethod = CV_TM_SQDIFF_NORMED;
    //cv::matchTemplate(src,temp[i],result[i],MatchMethod);
    // cv::normalize(result[i],result[i],0,1,NORM_MINMAX,-1,Mat());


//    cv::Point minLoc[9];
//    cv::Point maxLoc[9];
//    cv::Point matchLoc;
      //  cv::minMaxLoc(result[i],&minVal[i],&maxVal[i],&minLoc[i],&maxLoc[i],Mat());
    cv::minMaxLoc(result[i],&minVal[i],&maxVal[i]);
    //matchLoc = minLoc;
    //cv::rectangle(img_dis,matchLoc,Point( matchLoc.x + temp.cols , matchLoc.y + temp.rows),cv::Scalar::all(0),2,8,0);
    //cv::rectangle(result,matchLoc,Point(matchLoc.x + temp.cols , matchLoc.y + temp.rows),cv::Scalar::all(0),2,8,0);

}





