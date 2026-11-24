# 复现指南与已知限制

本次在 Windows 上完成文件整理与静态检查，未运行 ROS 编译、Linux 驱动或实车测试。以下是基于代码整理的复现步骤，不能作为已验证的完整运行环境记录。

## 环境与依赖

- Linux、ROS 1、Catkin、C++ 工具链及 Python 3；原始 Ubuntu/ROS 版本没有完整记录。
- 保留的 `libracecar_controller.a` 与 `libracecar_driver.a` 含 x86-64 ELF 对象。ARM 等其他架构需要重新编译或替换库。
- `rf2o_laser_odometry` 的现有构建设置使用 C++17，同时依赖 Eigen3 与 Boost。
- 上游 `serial/tests/proof_of_concepts/python_serial_test.py` 是 Python 2 历史演示，不属于本项目 Python 3 运行入口。

先按 README 加载 ROS 环境并运行 rosdep。旧清单可能遗漏运行依赖；按实际安装的发行版补齐：

```bash
sudo apt-get install \
  ros-${ROS_DISTRO}-gmapping \
  ros-${ROS_DISTRO}-map-server \
  ros-${ROS_DISTRO}-amcl \
  ros-${ROS_DISTRO}-move-base \
  ros-${ROS_DISTRO}-navfn \
  ros-${ROS_DISTRO}-dwa-local-planner \
  ros-${ROS_DISTRO}-robot-localization \
  ros-${ROS_DISTRO}-laser-filters \
  ros-${ROS_DISTRO}-rviz \
  ros-${ROS_DISTRO}-joy \
  ros-${ROS_DISTRO}-dynamic-reconfigure \
  ros-${ROS_DISTRO}-cmake-modules \
  libeigen3-dev libboost-system-dev \
  python3-serial python3-numpy python3-rospkg
```

使用可提供 Python 3 `rospy` 的兼容 ROS 1 环境。不要将上述命令直接用于 ROS 2。视觉功能还需安装与原脚本 API 相符的 OpenCV；若使用两返回值/三返回值不同的版本，应先适配 `findContours`，再验证视觉结果。

## 串口与传感器

默认硬件入口依赖以下 Linux 设备：

| 设备 | 用途 | 配置位置 |
| --- | --- | --- |
| `/dev/car` | 底盘串口 | `racecar_driver/src/racecar.cpp` |
| `/dev/laser` | LS01B | `ls01b/launch/ls01b_v2.launch` |
| `/dev/imu` | 串口 IMU | `serial_imu/src/serial_imu.cpp` |
| `/dev/encoder` | 编码器 | `racecar/launch/Run_car.launch` |

原 udev 规则保存在 `racecar/udev/`。先按设备标识检查和修改规则，避免把同类型 USB 转串口设备绑定错，再安装：

```bash
cd "$(rospack find racecar)/udev"
sudo bash racecar_init.sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```

必要时重新插拔设备并确认串口访问权限。不要只依靠 `/dev/ttyUSB0` 的编号判断设备身份。

校准内容包括编码器比例系数 `k`、轴距、雷达与 IMU 安装位置、IMU 坐标方向和底盘转向中位。`Run_car.launch` 中的编码器系数与 TF 文件中的尺寸都是原车参数。

## 地图与航点

地图 YAML 的 `image` 已改为相对路径，可随仓库一起移动。主导航默认使用 `mymap_6.yaml`；`amcl_nav_no_control.launch` 与原有 `amcl_nav copy.launch` 保留自己的默认地图。三个入口都可通过 `map_file:=...` 指定地图。

航点整理规则：

| 路径 | 原始来源 |
| --- | --- |
| `multi_goal/waypoints/test.csv` | 原 `multi_goal/scripts/test.csv`，主脚本默认路线 |
| `multi_goal/waypoints/historical/` | 原 `multi_goal/src/*.csv` |
| `multi_goal/waypoints/package_root/` | 原 `multi_goal/*.csv` |
| `multi_goal/legacy/` | 原 `src/` 下的另一版采点/循环脚本 |

所有 CSV 都保留原始数据，不合并同名路线。每行是 `x,y,qz,qw`，无表头。历史路线需要与对应地图配套选择，文件名本身不足以证明配对关系。

主采点脚本记录 `/encoder_imu_odom`，使用其里程计坐标；主导航脚本以 `map` 为默认目标坐标系，未自动执行里程计到地图的位姿转换。重用数据前必须确认坐标对齐。采点脚本默认输出 `~/ares_waypoints.csv`，也可用 `_csv_file` 指定可写位置；运行期间会重写该文件中的累计采点结果。

## 启动顺序

1. 编译并加载工作空间，检查四个串口及传感器数据。
2. 运行 `roslaunch racecar Run_car.launch`。
3. 建图时运行 `Run_gmapping.launch`，配合键盘或手柄控制并保存地图。
4. 导航前关闭建图入口，再运行 `amcl_nav.launch`。根据地图设置初始位姿。
5. 首先验证单目标导航，再通过 `_csv_file` 指定路线启动 `goal_loop.py`。

`race.launch` 已包含硬件和建图入口，使用它时不要再次启动 `Run_car.launch`。`amcl_nav_no_control.launch` 不包含 L1 控制器，适合单独检查规划链路。

## 原实现中需要核对的事项

- **TF 发布者**：`amcl_nav.launch` 同时保留静态 `map → odom` 和 AMCL；复现时需要检查该变换是否存在多发布者。此次整理未更改定位算法配置。
- **底盘命令**：`/car/cmd_vel` 使用自定义的 PWM/角度约定；驱动读取 `linear.x` 与 `angular.z`，不能直接当成标准 m/s、rad/s 接口。旧 README 对角度字段的描述与当前驱动不一致，以源代码和底盘协议为准。
- **视觉版本**：主视觉脚本调用三返回值形式的 `cv2.findContours`；安装不同 API 的 OpenCV 后需要适配。主导航没有自动启动此脚本。
- **激光数组假设**：`find_obj/src/laser_2d.cpp` 使用固定扫描索引和固定长度缓冲区，保留了原始边界处理问题；接入不同分辨率数据前需要检查索引范围和循环边界。该节点未由主导航启动。
- **历史实验脚本**：原 `racecar/scripts/test.py` 含非法数字字面量，已移到 `racecar/legacy/camera_test.py.txt` 作为实验源码存档，不作为运行入口。
- **原始损坏文件**：原 `racecar/map/test.yaml` 含损坏字节，无法可靠恢复元数据；该文件及对应 `test.pgm` 保留在本机 `.local-archive/invalid-map/` 与整理前备份中，没有上传为可用地图。
- **远程访问脚本**：原 `racecar/ssh.sh`、`racecar/racecar_rviz.sh` 保留历史局域网地址；跨机运行前需要按实际网络修改。
- **清单与许可**：部分旧依赖声明和许可字段仍为模板内容；请结合实际构建报错及各组件许可核对。

## 检查

```bash
python3 scripts/check_repository.py
```

脚本验证静态文件关系、主要 Python 入口、ROS XML、地图图像引用、CSV 数值格式和静态库存在性。Linux 下还需实际执行 `catkin_make`，再核对话题、TF、串口和控制输出。
