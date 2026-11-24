# 文件整理记录

本次整理保留所有 15 个 ROS 包的名称，按用途放入 Catkin 的 `src/`。Catkin 可递归发现分组下的包。独立 RPLIDAR SDK 放在顶层 `third_party/`，与 Catkin 包分开。

| 原路径 | 新路径 |
| --- | --- |
| `racecar/` | `src/core/racecar/` |
| `multi_goal/` | `src/core/multi_goal/` |
| `find_obj/` | `src/core/find_obj/` |
| `encoder_driver/` | `src/drivers/encoder_driver/` |
| `racecar_driver/` | `src/drivers/racecar_driver/` |
| `hi226_imu/` | `src/drivers/hi226_imu/` |
| `imu_launch/` | `src/drivers/imu_launch/` |
| `ls01b/` | `src/drivers/ls01b/`，ROS 包名仍为 `ls01b_v2` |
| `serial_imu/` | `src/drivers/serial_imu/` |
| `rf2o_laser_odometry/` | `src/third_party/rf2o_laser_odometry/` |
| `imu_tool/` | `src/third_party/imu_tool/` |
| `rplidar_ros/` | `src/third_party/rplidar_ros/` |
| `serial/` | `src/third_party/serial/` |
| `rplidar_sdk/` | `third_party/rplidar_sdk/` |
| `multi_goal/scripts/*.csv` | `src/core/multi_goal/waypoints/*.csv` |
| `multi_goal/src/*.csv` | `src/core/multi_goal/waypoints/historical/*.csv` |
| `multi_goal/*.csv` | `src/core/multi_goal/waypoints/package_root/*.csv` |
| `multi_goal/src/*.py` | `src/core/multi_goal/legacy/*.py` |
| `racecar/scripts/test.py` | `src/core/racecar/legacy/camera_test.py.txt` |

配套调整：

- 将有效地图 YAML 中的原电脑绝对路径改为同目录图像文件名。
- 为主要航点脚本增加 `~csv_file` 参数；默认导航路线使用 ROS 包路径定位，采集输出使用用户目录。
- 为三个 `amcl_nav*.launch` 入口增加 `map_file` 参数；保留各自默认地图。
- 保存地图脚本通过 `rospack find racecar` 定位目录。
- 为安装的 Python 入口补齐 shebang，并在 Catkin 中注册编码器、遥控等脚本入口。
- 将主项目 Python 文件的 GBK 注释转为 UTF-8，保留逻辑内容。
- 保留构建目标依赖的两份原始 `.a` 库；通过 `.gitignore` 的例外规则显式纳入版本管理。

整理前完整备份位于原项目目录的上一级，文件名为 `ares-original-backup-20261124.zip`。本机 `.local-archive/` 保留原嵌套 Git 元数据、旧编译产物、Windows 驱动安装包、损坏地图与整理脚本。该目录被 Git 忽略，不上传到仓库。

上游源码中的版权、许可证、README、示例和有用的采样文件保留。此次未修改导航和控制算法，也未补写缺失的比赛年份或性能指标。
