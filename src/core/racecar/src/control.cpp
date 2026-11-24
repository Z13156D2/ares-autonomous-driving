#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include "geometry_msgs/Pose2D.h"
#include "sensor_msgs/LaserScan.h"
#include <nav_msgs/Odometry.h>
#include "geometry_msgs/PoseStamped.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h" 
#include <vector>
#include <cmath>
#include <iostream>



#define param_angle_protect 1
#define param_line_protect 1


using std::vector;

double time_lest;
double time_now;
float turn;

int flag_pd;

float angle;
float speed;

//＊＊雷达回调函数
float obj_near_dis;
float obj_near_angle;
float obj_far_dis;
float obj_far_angle;

//＊＊编码器回调函数
float pose;
float pose_1;
//直角——１
float cir_x;
float cir_y;
float length;
int flag_position = 0;
//直角——２
float cir_x_1;
float cir_y_1;
float length_1;
int flag_position_1 = 0;
//
float cir_x_2;
float cir_y_2;
float length_2;
int flag_slow = 0;
//停车
float stop_x;
float stop_y;
float length_stop;//到座标点的距离
float flag_stop;
float flag_stop_last;
float stop_flag;
typedef struct 
{
    float Error_Angle;
    float error_line;
    float P_angle;
    float D_angle;
    float P_line;
    float D_line;
    float last_error_angle;
    float last_error_line;
    int out_angle;
    int out_line;
    float line_D;
    float angle_D;
} pid_dir;
pid_dir PID_dir;
typedef struct 
{
    float set_speed;//设定的速度
    float error_speed;//速度误差
    float error_error_speed;//
    float last_error_speed;//上次速度误差
    float P;//参数p
    float I;//参数i
    float D;//参数d
    float P_OUT;//比例环节输出值
    float I_OUT;//积分环节输出值
    float D_OUT;//微分环节输出值
    float OUT;//输出值
} pid_speed;
pid_speed PID_speed;
float speed_set;

class my_car_control
{
private:
    ros::NodeHandle n_;
    ros::Subscriber point_sub;
    ros::Subscriber carpose_sub;
    ros::Publisher vel_pub;

    tf2_ros::Buffer buffer;
    tf2::Quaternion qtn;
    
    nav_msgs::Odometry pose;
    
    geometry_msgs::Twist cmd_vel; //发布小车的运动方向与速度
    geometry_msgs::PoseStamped point_baselink_pub; //发布目标中点坐标
    geometry_msgs::PoseStamped point_map_pub;      //发布目标中点坐标
    vector<geometry_msgs::PoseStamped> vec_pub;    //存储发布目标中点坐标的容器
    void Get_point(const geometry_msgs::Pose2D::ConstPtr &Msg);
    void car_pose(const nav_msgs::Odometry::ConstPtr &odomMsg);
public:

    my_car_control();
    ~my_car_control();
};

my_car_control::my_car_control()
{
    ROS_INFO("校车控制节点开启");

    tf2_ros::TransformListener listener(buffer);
    //频率发布控制
    ros::Rate r(1);
    r.sleep();
   
    //分段pid 选择
    if(flag_pd == 1)
    {
        PID_dir.P_line = 100; 
        PID_dir.D_line = 80;  
    }
    if(flag_pd == 0)
    {
        PID_dir.P_line = 90; 
        PID_dir.D_line = 70;  
    }
    
       PID_dir.P_line = 5;  
       PID_dir.D_line = 0;
    
    //PID_dir.P_angle = 30;
    //PID_dir.D_angle = 50;
    
    //小车速度初始化
    cmd_vel.angular.x = 0;
    cmd_vel.angular.y = 0;
    cmd_vel.linear.y = 0;
    cmd_vel.linear.z = 0;
    
    //小车pid初始化
    PID_speed.P = 0.0;
    PID_speed.I = 0.0;
    PID_speed.D = 0.0;

    point_map_pub.pose.position.z = 0;
    point_baselink_pub.header.frame_id = "base_link";

    point_sub = n_.subscribe<geometry_msgs::Pose2D>("/point_mid", 10, &my_car_control::Get_point, this); 
    
    vel_pub = n_.advertise<geometry_msgs::Twist>("car/cmd_vel", 100); 
}

my_car_control::~my_car_control()
{
    cmd_vel.linear.x = 1500;
    cmd_vel.angular.z = 90;
    vel_pub.publish(cmd_vel);

    printf("小车控制节点关闭\n");
}

void Pid(pid_speed *pid,float get,float set)
{
    pid->error_speed = set - get;
    pid->error_error_speed = pid->error_speed -pid->last_error_speed;
    pid->P_OUT = pid->P * pid->error_speed;
    pid->I_OUT += pid->I * pid->error_speed;
    pid->D_OUT = pid->D * pid->error_error_speed;
    pid->OUT = pid->P_OUT + pid->I_OUT + pid->D_OUT;
    if(pid->OUT > 1950)pid->OUT = 1950;
    if(pid->OUT < 1500) pid->OUT = 1500;
}

void my_car_control::Get_point(const geometry_msgs::Pose2D::ConstPtr &Msg)
{
    //计算PID
    PID_dir.Error_Angle = Msg->theta;
    
    //PID_dir.error_line = Msg->y;
    
   PID_dir.error_line = -turn/32;
    
    PID_dir.line_D = abs(PID_dir.error_line - PID_dir.last_error_line);
    PID_dir.angle_D = abs(PID_dir.Error_Angle - PID_dir.last_error_angle);


    PID_dir.out_line = PID_dir.P_line * PID_dir.error_line + PID_dir.D_line * (PID_dir.error_line - PID_dir.last_error_line);

    //***打角以及速度的判断和发布***//
    //´打角值发布
    cmd_vel.angular.z = 90 + PID_dir.out_line;

    //laser  偏差太大直接强制打角
    if (PID_dir.Error_Angle > param_angle_protect)
    {
        cmd_vel.angular.z = 75;
    }
    else if (PID_dir.Error_Angle < -param_angle_protect)
    {
        cmd_vel.angular.z = 105;
    }
    //限幅
    if (cmd_vel.angular.z > 150)
        cmd_vel.angular.z = 150;
    else if (cmd_vel.angular.z < 30)
        cmd_vel.angular.z = 30;

    float a = cmd_vel.angular.z;
    
     cmd_vel.linear.x = 1840;
     ////speed_set = 1.0;
    
//    if(obj_near_dis < 0.8 && flag_position == 0 && flag_position_1 == 0 && pose_1>2)
//    {
  //  	    if(turn == 1 && obj_near_angle > 0)//代表全是红色 且障碍物在左边
//	    {
//	    cmd_vel.linear.x = 1820;
//	   ////speed_set = 1.0;
//	      if(obj_near_dis < 0.55)	{cmd_vel.angular.z = 35 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.55 && obj_near_dis < 0.65) {cmd_vel.angular.z = 55 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.65 && obj_near_dis < 0.75) {cmd_vel.angular.z = 60 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.75 && obj_near_dis < 0.8) {cmd_vel.angular.z = 65 + PID_dir.out_line*0;}
//	    }
//	    if(turn == 2 && obj_near_angle < 0)//代表全是蓝色　且障碍物在右边
//	    {
  //          cmd_vel.linear.x = 1820;
  //         ////speed_set = 1.0;
//	      if(obj_near_dis < 0.55)	{cmd_vel.angular.z = 145 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.55 && obj_near_dis < 0.65) {cmd_vel.angular.z = 135 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.65 && obj_near_dis < 0.75) {cmd_vel.angular.z = 120 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.75 && obj_near_dis < 0.8) {cmd_vel.angular.z = 125 + PID_dir.out_line*0;}
//	    }
//    }
    
//    if(pose_1 <= 2 && flag_slow != 1)     
//    cmd_vel.linear.x = 1860;
    ////speed_set = 1.0;
    
 //   if(obj_near_dis < 0.70 && flag_position == 0 && flag_position_1 == 0 && pose_1<=2)
   // {
   // 	    if(turn == 1 && obj_near_angle > 0)//代表全是红色 且障碍物在左边
//	    {
//	      if(obj_near_dis < 0.55)	{cmd_vel.angular.z = 35 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.55 && obj_near_dis < 0.65) {cmd_vel.angular.z = 55 + PID_dir.out_line*0;}
//	     // else if(obj_near_dis > 0.65 && obj_near_dis < 0.70) {cmd_vel.angular.z = 60 + PID_dir.out_line*0;}
//	    }
//	    if(turn == 2 && obj_near_angle < 0)//代表全是蓝色 且障碍物在右边
//	    {
//	      if(obj_near_dis < 0.55)	{cmd_vel.angular.z = 145 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.55 && obj_near_dis < 0.65) {cmd_vel.angular.z = 135 + PID_dir.out_line*0;}
//	      else if(obj_near_dis > 0.65 && obj_near_dis < 0.70) {cmd_vel.angular.z = 120 + PID_dir.out_line*0;}
//	    }
   // }

    if (cmd_vel.angular.z > 150)
        cmd_vel.angular.z = 150;
    else if (cmd_vel.angular.z < 30)
        cmd_vel.angular.z = 30; 
   

    //if(flag_position == 1)
  //  {
    //  cmd_vel.angular.z = 130;//小于90偏右
   //   cmd_vel.linear.x = 1840;
      ////speed_set = 1.0;
   // }
    
  //  if(flag_position_1 == 1)
 //   {
  //    cmd_vel.angular.z = 150;//小于90偏右
  //    cmd_vel.linear.x = 1840;
      ////speed_set = 1.0;
  //  }
    
    ////Pid(&PID_speed,speed,speed_set);
    ////cmd_vel.linear.x = PID_speed.OUT;
    
  //  if(stop_flag >= 2)
  //  {
  //      cmd_vel.angular.z = 90;
  //      cmd_vel.linear.x = 1500;
  //  }
    ROS_INFO("%f %f\n",a,turn);
    //ROS_INFO("%f\n", stop_flag);

    vel_pub.publish(cmd_vel);
    //***end***打角以及速度的判断和发布***end***//
    PID_dir.last_error_line = PID_dir.error_line;
    PID_dir.last_error_angle = PID_dir.Error_Angle;
}



//编码器回调函数
void EncoderCallback(const nav_msgs::Odometry& pos)
{
    pose = pos.pose.pose.position.x;
    pose_1 = pos.pose.pose.position.y;
    //ROS_INFO("%f %f %d\n", pose,pose_1,flag_pd);
    //***强制打角***//
    //直角打角 x,y 3.6,0.1 k = 1.27
    cir_x = 3.05;
    cir_y = 0.1; //圆心座标点
    length = (pose - cir_x)*(pose - cir_x) + (pose_1 - cir_y)*(pose_1 - cir_y);//到座标点的距离
    if(length < 0.36)//半径小于0.6时
    {
      flag_position = 1;
      //flag_pd = 0;
    }
    if(flag_position == 1 && angle < 0.8)
    {
      flag_position = 0;
    }
    
    
    //直角打角_1  x,y -13.3,0.9
    cir_x_1 = -10.9;//
    cir_y_1 = 0.40; //圆心座标点0.7
    length_1 = (pose - cir_x_1)*(pose - cir_x_1) + (pose_1 - cir_y_1)*(pose_1 - cir_y_1);//到座标点的距离
    if(length_1 < 0.49)//半径小于0.7时
    {
      flag_position_1 = 1;
      //flag_pd = 0;
    }
    if(flag_position_1 == 1 && (angle<-0.98 || angle>0.98))
    {
      flag_position_1 = 0;
    }
    
    //过起点　减速
    cir_x_2 = 0;//
    cir_y_2 = 0; //圆心座标点0.7
    length_2 = (pose - cir_x_2)*(pose - cir_x_2) + (pose_1 - cir_y_2)*(pose_1 - cir_y_2);//到座标点的距离
    if(length_2 < 0.49 && stop_flag == 1)//半径小于0.7时
    {
      flag_slow = 1;
      //flag_pd = 0;
    }

    
    //停车判断   x,y -8.8,0
    stop_x = -7.8;//
    stop_y = 0;
    length_stop = (pose - stop_x)*(pose - stop_x) + (pose_1 - stop_y)*(pose_1 - stop_y);//到座标点的距离
    if(length_stop < 0.36)//半径小于0.6时
    {
      flag_stop = 1;
      //flag_pd = 0;
    }
    else
    {
      flag_stop = 0;
    }
    if(flag_stop_last == 1 && flag_stop == 0)
    {
   	 stop_flag = stop_flag + 1;
    }
    flag_stop_last = flag_stop;
    //ROS_INFO("%d %d %f %f\n", flag_position ,flag_position_1,stop_flag,angle);
    

    //区域变pid
    float point_x1,point_x2,point_y1,point_y2;
    point_x1 = 6.5;
    point_x2 = -4.1;
    point_y1 = 8.0;
    point_y2 = 4.2;
    if((pose > point_x2&&pose <point_x1)&&(pose_1 >point_y2&&pose_1<point_y1))
    {
     //flag_pd = 2;
    }
    else
    {
     //flag_pd = 2;
    }   
}
//摄像头回调函数  0 正常  1 2全部都是一种颜色时
void CameraCallback(const geometry_msgs::Twist& twist)
{
    turn = twist.linear.x;
}

//
void DistanceCallback(const geometry_msgs::Quaternion& dis)
{
//角度小于０　在右边　角度大于０　在左边


    if(dis.x<dis.z)
    {
        obj_near_dis = dis.x;//较近障碍物的直线距离 
        obj_near_angle = dis.y;//较近障碍物的角度 
    }
    else
    {
        obj_near_dis = dis.z;//较近障碍物的直线距离 
        obj_near_angle = dis.w;//较近障碍物的角度 
    }
    obj_far_dis = dis.z;//次进障碍物的直线距离
    obj_far_angle = dis.w; //次进障碍物的角度
}

void AngleCallback(const nav_msgs::Odometry& pose)
{
    angle = pose.pose.pose.orientation.w; //陀螺仪的角度　第二次进障碍物的角度、
    speed = pose.twist.twist.linear.x;
}


//主函数
int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    ros::init(argc, argv, "my_car_control");

    ros::NodeHandle n;
    
    my_car_control controller;

    ros::Subscriber sub = n.subscribe("/encoder_imu_odom",1,EncoderCallback);
    ros::Subscriber sub_1 = n.subscribe("/camera",1,CameraCallback);
    ros::Subscriber sub_2 = n.subscribe("/point_distance",1,DistanceCallback);
    ros::Subscriber sub_3 = n.subscribe("/encoder_imu_odom",10,AngleCallback);
    ros::spin();
    return 0;
}
