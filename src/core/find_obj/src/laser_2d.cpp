#include "ros/ros.h"
#include "sensor_msgs/LaserScan.h"
#include <vector>
#include <math.h>
#include <iostream>
#include "geometry_msgs/Pose2D.h"
#include "geometry_msgs/Quaternion.h"
#define PI 3.1415926

typedef struct 
{
    float angle;
    float distance;
    float dk_x;
    float dk_y;
} dir_obstacle;

class class_laser_2d
{
private:
    int ind[600] = {0};         //融合后的数据 从1140～300 共600个点 从零点的左边开始 顺序
    int ind_1[300] = {0}; 
    int ind_2[300] = {0}; 
        
    float new_laser[600] = {0}; //融合后的数据 从1140～300 共600个点 从零点的左边开始 距离数据
    
    float new_laser_1[300] = {0}; 
    float new_laser_2[300] = {0}; 
    
    ros::NodeHandle nh;
    
    geometry_msgs::Pose2D point; 
    geometry_msgs::Quaternion distance;
    
    ros::Subscriber sub;
    ros::Publisher pub;
    ros::Publisher pub_1;
    dir_obstacle ob_1, ob_2;

    void Get_goal(const sensor_msgs::LaserScan::ConstPtr &msg_p);
    void BubbleSort(float *p, int length, int *ind_diff);
    void Print(float str[], int n);
    void Print_int(int str[], int n);
public:
    class_laser_2d();
    ~class_laser_2d();
};

class_laser_2d::class_laser_2d()
{
    ROS_INFO("雷达扫描障碍物节点开启");
    sub = nh.subscribe<sensor_msgs::LaserScan>("/scan_1", 100, &class_laser_2d::Get_goal, this);
    pub = nh.advertise<geometry_msgs::Pose2D>("/point_mid", 100, true); 
    pub_1 = nh.advertise<geometry_msgs::Quaternion>("/point_distance", 100, true);
}

class_laser_2d::~class_laser_2d()
{
    printf("雷达扫描障碍物节点关闭");
}

void class_laser_2d::Get_goal(const sensor_msgs::LaserScan::ConstPtr &msg_p)
{
    int num = 0,          //新的数组的序号
        num_1 = 0,
        num_division = 1, //
        f_division = 0;   //
        
    //将数据融合 并将 距离数据 从新赋值
    for (int i = 1140; i <= 1440; i++)//
    {
        new_laser[num] = msg_p->ranges[i];
        new_laser_1[num] = msg_p->ranges[i];
        num++;
    }
    for (int i = 0; i <= 300; i++)//
    {
        new_laser[num] = msg_p->ranges[i];
        new_laser_2[num_1] = msg_p->ranges[i];
        num++;
        num_1++;
    }
    
    //将所得到 数据融合后的数组 进行 从小到大的 排序 排序方式参见后面的函数
    //            从小到大也是十分的重要
    BubbleSort(new_laser, 600, ind);
    
    BubbleSort(new_laser_1, 300, ind_1); //左边
    //for(int i = 0;i<=300;i++)
    //ROS_INFO("%d___ %f    ", i,new_laser_1[i]);
       
    BubbleSort(new_laser_2, 300, ind_2);
    //for(int i = 0;i<=300;i++)
    //ROS_INFO("%d__** %f    ", i,new_laser_2[i]);
    

  //if (0)
   if (abs(ind_1[0] - 300 - ind_2[0]) > 70)
   {//left is - number
    ob_1.angle = (ind_1[0] - 300) / 4;
    ob_1.distance = new_laser_1[0];
 
    ob_2.angle = (ind_2[0]) / 4;
    ob_2.distance = new_laser_2[0];

     ROS_INFO("障碍物1,2的角度为： %f , %f\n", ob_1.angle, ob_2.angle);
     //ROS_INFO("障碍物1,2的距离为： %f , %f\n", ob_1.distance, ob_2.distance);


    ob_1.dk_x = ob_1.distance * cos(ob_1.angle * PI / 180);
    ob_1.dk_y = ob_1.distance * sin(ob_1.angle * PI / 180);

    ob_2.dk_x = ob_2.distance * cos(ob_2.angle * PI / 180);
    ob_2.dk_y = ob_2.distance * sin(ob_2.angle * PI / 180);
    
    distance.x = ob_1.distance;
    distance.y = ob_1.angle;
    distance.z = ob_2.distance;
    distance.w = ob_2.angle;
   }

  else
  {
     while (1)
    {
   	 // 代表的是距离相近的两个点之间的角度 大于 70*0.25度 = 17.5度
   	 //一度扫描4个点 所以70个点代表的是
        if (abs(ind[num_division + 1] - ind[num_division]) > 70)
        {
            break;
        }
        num_division++;
    }
    //ROS_INFO("终止的点是：%d \n",num_division);
    ob_1.angle = (ind[num_division] - 300) / 4;
    ob_1.distance = new_laser[num_division];
 
    ob_2.angle = (ind[num_division + 1] - 300) / 4;
    ob_2.distance = new_laser[num_division + 1];
    ROS_INFO("*******\n");
    ROS_INFO("障碍物1,2的角度为： %f , %f\n", ob_1.angle, ob_2.angle);


    ob_1.dk_x = ob_1.distance * cos(ob_1.angle * PI / 180);
    ob_1.dk_y = ob_1.distance * sin(ob_1.angle * PI / 180);

    ob_2.dk_x = ob_2.distance * cos(ob_2.angle * PI / 180);
    ob_2.dk_y = ob_2.distance * sin(ob_2.angle * PI / 180);
    
    distance.x = ob_1.distance;
    distance.y = ob_1.angle;
    distance.z = ob_2.distance;
    distance.w = ob_2.angle;
  }
    
    
    
     ROS_INFO("X 为：%f    \n ", distance.x);
     ROS_INFO("Y 为：%f    \n ", distance.z);
     
    pub_1.publish(distance); 

    if (ob_1.dk_y != ob_2.dk_y)
    {
        point.x = (ob_1.dk_x + ob_2.dk_x) / 2;
        point.y = (ob_1.dk_y + ob_2.dk_y) / 2;
        point.theta = -(ob_1.dk_x - ob_2.dk_x) / (ob_1.dk_y - ob_2.dk_y);
        pub.publish(point);
         //ROS_INFO("发布的目标点为 base_link X:%f , Y:%f",point.x,point.y);
         //ROS_INFO("%f\n", point.theta);
    }
}

//将 融合后的数据组 进行排序 
// 有两个数组 一个数组记录序号 ind_diff[]  另一个数组记录实际的距离p[]
void class_laser_2d::BubbleSort(float *p, int length, int *ind_diff)
{
    //将序号进行初始化 
    for (int m = 0; m < length; m++)
    {
        ind_diff[m] = m;
    }
    
    //将数据按从小大的顺序进行排序 
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length - i - 1; j++)
        {
            if (p[j] > p[j + 1])
            {
                float temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;

                int ind_temp = ind_diff[j];
                ind_diff[j] = ind_diff[j + 1];
                ind_diff[j + 1] = ind_temp;
            }
        }
    }
}

void class_laser_2d::Print(float str[], int n)
{
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf("%f ", str[i]);
    }
    printf("\n");
}

void class_laser_2d::Print_int(int str[], int n)
{
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf("%d ", str[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    ros::init(argc, argv, "laser_point_pub");

    class_laser_2d laser_point_pub;//

    ros::spin();
    return 0;
}
