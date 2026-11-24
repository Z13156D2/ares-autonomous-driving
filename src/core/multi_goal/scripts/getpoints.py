#!/usr/bin/env python3
#get the point and ore form rviz to txt
#Type: geometry_msgs/PoseStamped  /move_base_simple/goal
#todo: change the count to the key "finish"
#todo: not the cover the last csv msg 
import rospy
import os
import rospkg
import math
import sys,select,termios,tty
import tf
import csv
import codecs
from nav_msgs.msg import Odometry

count = 0
point=[]

def PoseStampedCB(data):
    global count
    gx = data.pose.pose.position.x
    gy = data.pose.pose.position.y
    gz = data.pose.pose.orientation.z
    gw = data.pose.pose.orientation.w
    count = count + 1
    point.append([gx,gy,gz,gw])
    #if(count == 3):
    
    data_write_csv(rospy.get_param('~csv_file', os.path.expanduser('~/ares_waypoints.csv')), point)
    #with open('test.csv','wb') as csvfile:
     #   pointwriter = csv.writer(csvfile,delimiter=' ',quotechar='|',quoting=csv.QUOTE_MINIMAL)
      #  pointwriter.writerow([gx,gy,gz,gw])
      #  print("write succ!!")

#def PoseStampedCB(data):
#    global count
#    gx = data.pose.position.x
#    gy = data.pose.position.y
#    gz = data.pose.orientation.z
#    gw = data.pose.orientation.w
#    #count = count + 1
#    point.append([gx,gy,gz,gw])
#    if(count == 3):
#        data_write_csv('test.csv',point)
#    # with open('test.csv','wb') as csvfile:
#    #     pointwriter = csv.writer(csvfile,delimiter=' ',quotechar='|',quoting=csv.QUOTE_MINIMAL)
#    #     pointwriter.writerow([gx,gy,gz,gw])
#    #     print("write succ!!")
    
def getKey():
    tty.setraw(sys.stdin.fileno())
    rlist,_,_ = select.select([sys.stdin],[],[],0.1)
    if rlist:
        key = sys.stdin.read(1)
    else:
        key = ''
    termios.tcsetattr(sys.stdin,termios.TCSADRAIN,settings)
    return key
 
def data_write_csv(file_name, datas):
    
    #file_csv = open(file_name,'w+',encoding='utf-8',newline='')
    with open(file_name,'w+',encoding='utf-8',newline='') as file_csv :
        writer = csv.writer(file_csv)#, delimiter=',', quoting=csv.QUOTE_MINIMAL)
        writer.writerows(datas)
        #print(datas)
        #for data in datas:
            #writer.writerow(data)
            #print(data)
        file_csv.close()
	#print("write succ!!")
   

if __name__ == '__main__':
    global liste
    try:
        rospy.init_node("getPoint",anonymous=True)
       #rospy.Subscriber('/move_base_simple/goal',PoseStamped,PoseStampedCB,queue_size=10)encoder_imu_odom PoseStamped
        print("write s!!")
        liste = rospy.Subscriber('/encoder_imu_odom',Odometry,PoseStampedCB,queue_size=10)
        print("write su!!")
        rospy.spin() 
#        settings = termios.tcgetattr(sys.stdin)
#        while(1):
#            Key = getKey()
#            if Key == 'f' or Key == 'F':
#                data_write_csv('test.csv',point)
#            if Key == '\x03':
#                break
        
        print("write suc!!")
    except rospy.ROSInterruptException:
        pass 
