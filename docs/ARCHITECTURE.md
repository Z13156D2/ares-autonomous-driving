# 架构与代码入口

本文依据随项目提供的源代码和 launch 文件整理。模块的存在不表示它在默认启动流程中已启用。

## 主流程

1. `racecar/launch/Run_car.launch` 发布车体静态 TF，启动 LS01B、`serial_imu`、编码器与底盘串口。
2. 建图时使用 `Run_gmapping.launch`：组合编码器/IMU 里程计、EKF 与 GMapping；RF2O 的 include 在此入口中默认注释。
3. 导航时使用 `amcl_nav.launch`：加载地图，启动 RF2O、组合里程计、EKF、AMCL、`move_base` 和 `car_controller_new`。
4. `multi_goal/scripts/goal_loop.py` 读取四列 CSV，发布 `/move_base_simple/goal`，并依据 `/amcl_pose` 与目标点的距离推进路线。现有实现按距离阈值循环，不以 `move_base` action 成功状态作为推进依据。
5. `car_controller_new` 是独立的 L1 前视路径跟踪控制器，读取全局路径与融合里程计，向底盘话题输出控制量。

## 关键代码与配置

| 功能 | 文件 |
| --- | --- |
| L1 路径跟踪 | [`car_controller_new.cpp`](../src/core/racecar/src/car_controller_new.cpp) |
| 旧控制器及静态库接口 | [`car_controller.cpp`](../src/core/racecar/src/car_controller.cpp)、`src/lib/libracecar_controller.a` |
| 另一套几何/视觉控制入口 | [`control.cpp`](../src/core/racecar/src/control.cpp) |
| 红蓝边界处理 | [`camera.py`](../src/core/racecar/scripts/camera.py) |
| 激光障碍物几何 | [`laser_2d.cpp`](../src/core/find_obj/src/laser_2d.cpp) |
| 编码器读取 | [`Encoder_vel.py`](../src/drivers/encoder_driver/src/Encoder_vel.py) |
| 编码器/IMU 里程计 | [`imu_encoder_mix.cpp`](../src/drivers/encoder_driver/src/imu_encoder_mix.cpp) |
| 底盘串口协议 | [`racecar.cpp`](../src/drivers/racecar_driver/src/racecar.cpp) |
| EKF | [`ekf_params.yaml`](../src/core/racecar/param/ekf_params.yaml) |
| DWA 与代价地图 | [`param/`](../src/core/racecar/param/) |
| 静态 TF | [`car_tf.launch.xml`](../src/core/racecar/launch/includes/car_tf.launch.xml) |

## 主要话题

| 话题 | 用途 |
| --- | --- |
| `/scan` | 主激光扫描；建图、定位与 RF2O 使用 |
| `/scan_1` | LS01B 滤波后的扫描；`find_obj` 订阅 |
| `/imu_data` | IMU 数据；组合里程计输入之一 |
| `/encoder` | 编码器数据 |
| `/encoder_imu_odom` | 编码器/IMU 组合里程计；采点脚本订阅 |
| `/odometry/filtered` | EKF 状态估计；路径跟踪输入 |
| `/amcl_pose` | 地图定位位姿；多目标点任务输入 |
| `/move_base_simple/goal` | 导航目标位姿 |
| `/move_base/NavfnROS/plan` | L1 控制器订阅的全局路径 |
| `/car/cmd_vel` | 自定义底盘控制量；虽然类型为 `Twist`，含义应按驱动核对 |
| `/point_mid`、`/point_distance` | `find_obj` 输出的障碍物几何信息 |
| `/camera` | 视觉脚本输出；由独立的 `control` 实现使用 |

话题名前导 `/`、remap 和实际发布情况请以运行时 `rostopic list`、`rostopic info` 为准。

## 辅助模块与备选方案

- `find_obj` 与 `racecar` 的 `control` 组成另一条障碍物几何控制链路，需要单独启动和核对扫描格式；它不是默认导航入口的一部分。
- 视觉脚本未在默认建图或导航 launch 中自动启用。原实现使用 OpenCV 三返回值 `findContours` API，具体版本需要匹配或适配。
- `hi226_imu`、`rplidar_ros` 为备选设备实现。切换硬件时需要同步调整串口、TF 和扫描话题。
- `imu_tool` 含三个包：`imu_complementary_filter`、`imu_filter_madgwick`、`rviz_imu_plugin`。
- `third_party/rplidar_sdk` 为独立 SDK；ROS 驱动另有内置 SDK，因此无需为默认 Catkin 构建额外编译顶层 SDK。

## 复现边界

本仓库保留参赛实现与参数，没有将多套控制入口合并。运行前应选择一套控制链路，确认 `/car/cmd_vel` 的发布者及命令含义。地图、航点、传感器安装偏移、速度和前视距离均与原车和原赛道有关。
