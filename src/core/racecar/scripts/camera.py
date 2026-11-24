#!/usr/bin/env python3
# This is used for ROS car 2021
# created by quanyi on 2021/12/1
# released on 2021/12/6
# You need to add other codes here

import rospy

from geometry_msgs.msg import Twist

import cv2
import numpy as np

# threshold for red and blue, expressed by array
#low_hsv = [[0, 131 ,21], [21, 131, 0]]
#high_hsv = [[13, 255, 255], [255, 225, 20]]  # RED BLUE
#中下午
#low_hsv = [[153, 60, 68], [97, 165, 25]]
#high_hsv = [[179, 249, 215], [153, 255, 255]]  # RED BLUE
#楼道
#low_hsv = [[0, 110, 153], [82, 100, 44]]
#high_hsv = [[19, 240, 255], [144, 255, 154]]  # RED BLUE
#noon
low_hsv = [[156, 16, 70], [99, 24, 0]]
high_hsv = [[179, 255, 255], [131, 176, 255]]  # RED BLUE   
    
lower_red1 = np.array([0,120,46])
higher_red1 = np.array([10,255,255])
lower_red2 = np.array([156,120,46])
higher_red2 = np.array([180,255,255])

lower_blue = np.array([100, 100, 10])
higher_blue = np.array([124, 255, 255])
#lower_blue = np.array([109, 80, 46])109 146 15
#higher_blue = np.array([124, 255, 255])124 255 172

def calculate_error(center_point, image_width):
    return image_width / 2 - center_point
    
def empty():
    pass

def get_border(piont_list,contour,src,color):
    AREA_THRES = src.shape[0]*src.shape[1]/200 #面积阈值
    area_cnt = 0
    for i in contour:
        area = cv2.contourArea(i)#面积计算
        if area > AREA_THRES:
            x, y, w, h = cv2.boundingRect(i)
            if ((y+h)<blue_border_lim[int(x+w/2)] and color[0]==255):
                area_cnt+=1
                piont_list.append([int(x+w/2),y+h])
                cv2.rectangle(src, (x, y), (x + w, y + h), color, 2)#画矩形
            elif (color[2]==255):
                area_cnt+=1
                piont_list.append([int(x+w/2),y+h])
                cv2.rectangle(src, (x, y), (x + w, y + h), color, 2)#画矩形
    piont_list = sorted(piont_list,key=lambda x:x[1],reverse=True)#按y轴坐标排序
    # if len(piont_list)>=2:
        # cv2.polylines(src,np.mat(piont_list),False,color,2)
    for i in range(area_cnt) :#连线
        cv2.line(src,tuple(piont_list[i]),tuple(piont_list[i+1]),color,2)
    return piont_list

def same_color_judge(src):
    
    hsv = cv2.cvtColor(src, cv2.COLOR_BGR2HSV)

    dst_red1 = cv2.inRange(hsv,lower_red1,higher_red1)
    dst_red2 = cv2.inRange(hsv,lower_red2,higher_red2)
    dst_red = cv2.add(dst_red1,dst_red2)
    
    dst_blue = cv2.inRange(hsv,lower_blue,higher_blue)

    # cv2.imshow('red',dst_red)
    # cv2.imshow('blue',dst_blue)

    KERNEL_SIZE = int((src.shape[0]+src.shape[1])/200)
    kernel = np.ones((KERNEL_SIZE, KERNEL_SIZE), np.uint8)
    dilate_kernel = np.mat([[0,1,0],
                            [1,1,1],
                            [0,1,0]],np.uint8)

    # dst_red = cv2.morphologyEx(dst_red,cv2.MORPH_CLOSE,kernel)
    dst_red = cv2.morphologyEx(dst_red,cv2.MORPH_OPEN,kernel)
    # dst_red = cv2.dilate(dst_red,dilate_kernel,iterations=1)
    dst_red = cv2.erode(dst_red,dilate_kernel,iterations=3)
    #cv2.imshow('red_open',dst_red)

    # dst_blue = cv2.morphologyEx(dst_blue,cv2.MORPH_CLOSE,kernel)
    dst_blue = cv2.morphologyEx(dst_blue,cv2.MORPH_OPEN,kernel)
    dst_blue = cv2.dilate(dst_blue,dilate_kernel,iterations=3)
    # dst_blue = cv2.erode(dst_blue,dilate_kernel,iterations=3)
    #cv2.imshow('blue_open',dst_blue)
   
    # cv2.imshow('dst_red',dst_red)
    # cv2.imshow('dst_blue',dst_blue)

    ret1,red_contour,hierarchy1 = cv2.findContours(dst_red, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    ret2,blue_contour,hierarchy2 = cv2.findContours(dst_blue, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    #面积阈值
    AREA_THRES = src.shape[0]*src.shape[1]/200 

    red_pos_lis = [[0,src.shape[0]]]
    blue_pos_lis = [[src.shape[1],src.shape[0]]]
    red_pos_lis = get_border(red_pos_lis,red_contour,src,(0,0,255))
    blue_pos_lis = get_border(blue_pos_lis,blue_contour,src,(255,0,0))

    
    for i in range(IMG_WIDTH-1):
        cv2.line(src,(i,red_border_lim[i]),(i,red_border_lim[i+1]),(0,255,0),2)
        cv2.line(src,(i,blue_border_lim[i]),(i,blue_border_lim[i+1]),(0,255,0),2)

    USE_LINE = 270
    cv2.line(src,(0,USE_LINE),(IMG_WIDTH,USE_LINE),(0,255,255),2)

    red_border = 0-IMG_WIDTH/2
    if len(red_pos_lis)==1:
        red_border = 0-IMG_WIDTH/2
    else :
        for j in range(len(red_pos_lis)):
            if red_pos_lis[j][1]<USE_LINE:
                red_border = (USE_LINE-red_pos_lis[j-1][1])*(red_pos_lis[j][0]-red_pos_lis[j-1][0])/(red_pos_lis[j][1]-red_pos_lis[j-1][1])+red_pos_lis[j-1][0]-IMG_WIDTH/2
                break
    blue_border = 0+IMG_WIDTH/2
    if len(blue_pos_lis)==1:
        blue_border = 0+IMG_WIDTH/2
    else:
        for j in range(len(blue_pos_lis)):
            if blue_pos_lis[j][1]<USE_LINE:
                blue_border = (USE_LINE-blue_pos_lis[j-1][1])*(blue_pos_lis[j][0]-blue_pos_lis[j-1][0])/(blue_pos_lis[j][1]-blue_pos_lis[j-1][1])+blue_pos_lis[j-1][0]-IMG_WIDTH/2
                break
    cv2.circle(src,(int(blue_border+IMG_WIDTH/2),USE_LINE),10,(255,0,0),2)
    cv2.circle(src,(int(red_border+IMG_WIDTH/2+1),USE_LINE),10,(0,0,255),2)
    center = blue_border+red_border
    cv2.line(src,(int(center+IMG_WIDTH/2),USE_LINE+10),(int(center+IMG_WIDTH/2),USE_LINE-10),(255,0,255),2)
    print([red_border,center,blue_border])
    return center

def ShowPiexlInfo(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        print([cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)[y,x],[x,y]])        
        
if __name__=="__main__":
    rospy.init_node("camera")
    rospy.loginfo("hello");
    
    pub = rospy.Publisher('/camera', Twist, queue_size=5)
      
    twist = Twist()
      
    cap = cv2.VideoCapture() # choose camera
    cap.open(0)
    cap.set(6,cv2.VideoWriter_fourcc('M','J','P','G'))
    cap.set(3, 640)  # Width
    cap.set(4, 480)  # Height
   # cap.set(15, -100)  # < -5
  #  print(cap.set(cv2.CAP_PROP_AUTO_WB,0))
    print(cap.set(cv2.CAP_PROP_AUTO_EXPOSURE,0.25))
    print(cap.set(cv2.CAP_PROP_EXPOSURE,0.0950))
    print(cap.get(cv2.CAP_PROP_EXPOSURE))
    # if get threshold or not 
    IMG_WIDTH = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    IMG_HEIGHT = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    IMG_SIZE = IMG_WIDTH*IMG_HEIGHT

    red_border_lim = []
    blue_border_lim = []
    for i in range(IMG_WIDTH):
        blue_border_lim.append(int(IMG_HEIGHT/np.log(IMG_WIDTH+1)*np.log(i+1)))
        red_border_lim.append(int(IMG_HEIGHT/np.log(IMG_WIDTH+1)*np.log(1-i+IMG_WIDTH)))

    print([IMG_SIZE,IMG_WIDTH,IMG_HEIGHT])
           
        
    while True:
        ret, frame = cap.read()
        # threshold adapt
       
        #if debug:
            
            #imgHSV = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)
            #both_color = same_color_judge(frame)
            #cv2.imshow("src",frame)        
            #cv2.setMouseCallback("src",ShowPiexIInfo)
            #cv2.waitKey(1)
            #continue

        both_color = same_color_judge(frame)
        cv2.imshow("src", frame)
        #cv2.setMouseCallback("src",ShowPiexlInfo)
        if cv2.waitKey(1) == 'q':
            break
        twist.linear.x = both_color
        pub.publish(twist)

    cap.release()
    cv2.destroyAllWindows



