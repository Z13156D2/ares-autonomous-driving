# 第三方来源与许可

以下来源根据随项目提供的 Git 配置、README 和源码注释整理，没有将集成的所有代码归为参赛团队原创。链接用于记录来源，当前集成版本可能与上游最新版本不同。

| 组件 | 本仓库路径 | 原始材料记录的来源 | 许可信息 |
| --- | --- | --- | --- |
| LS01B 驱动 | `src/drivers/ls01b` | [tongsky723/ls01b_ros](https://github.com/tongsky723/ls01b_ros) | `package.xml` 为 `TODO`，需向原作者核对 |
| RF2O | `src/third_party/rf2o_laser_odometry` | [tianb03/rf2o_laser_odometry](https://github.com/tianb03/rf2o_laser_odometry) | 保留 GPL v3 声明与 `LICENSE` |
| RPLIDAR ROS | `src/third_party/rplidar_ros` | [Slamtec/rplidar_ros](https://github.com/Slamtec/rplidar_ros) | 保留 BSD 声明与 `LICENSE`，同时核对内置 SDK 文件头 |
| RPLIDAR SDK | `third_party/rplidar_sdk` | [Slamtec/rplidar_sdk](https://github.com/Slamtec/rplidar_sdk) | 保留原 `LICENSE`；以随库文件及各文件头为准 |
| Serial | `src/third_party/serial` | [wjwwood/serial](https://github.com/wjwwood/serial) | 原清单为 MIT，许可文本见原 README/源码 |
| IMU tools | `src/third_party/imu_tool` | 原 README 指向 [ccny-ros-pkg/imu_tools](https://github.com/ccny-ros-pkg/imu_tools) | Madgwick 包声明 GPL；互补滤波器和 RViz 插件声明 BSD |
| 路径跟踪相关代码 | `src/core/racecar/src` | 原源码记录 Hypha ROS Workshop、ChanYuan KUO、YoRu LU、HaoChih LIN | 保留相关文件头中的 LGPL v3-or-later 声明 |
| HI226 IMU | `src/drivers/hi226_imu` | 随项目提供，未记录独立上游地址 | 原清单声明 BSD；核对源码中的具体条款 |

其他部分的 `package.xml` 仍存在 `TODO` 许可字段。两个二进制静态库保留用于原工程构建，但仅凭二进制与文件名无法确认完整来源、源码可用性或再分发条款。

本次整理不添加覆盖整个仓库的 MIT、Apache 或其他统一许可证，也不改变已有组件许可。复用或再分发具体组件前，请核对其原始许可文件、源码头和相关权利人的授权。
