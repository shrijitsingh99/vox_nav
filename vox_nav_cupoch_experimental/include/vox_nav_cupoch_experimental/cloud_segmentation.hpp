// Copyright (c) 2020 Fetullah Atas, Norwegian University of Life Sciences
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "message_filters/subscriber.h"
#include "message_filters/sync_policies/approximate_time.h"
#include "message_filters/synchronizer.h"
#include "pcl/filters/model_outlier_removal.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include <boost/geometry.hpp>
#include <iomanip>
#include <nav_msgs/msg/odometry.hpp>
#include <pcl/PCLPointCloud2.h>
#include <pcl/common/common.h>
#include <pcl/conversions.h>
#include <pcl/filters/crop_box.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl_ros/transforms.hpp>
#include <queue>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <tf2_eigen/tf2_eigen.h>
#include <visualization_msgs/msg/marker_array.hpp>

#include "ApproxMVBB/AABB.hpp"
#include "cupoch/collision/collision.h"
#include "cupoch/cupoch.h"
#include "cupoch/geometry/occupancygrid.h"
#include "cupoch_conversions/cupoch_conversions.hpp"
#include "std_msgs/msg/u_int8_multi_array.hpp"
#include "vox_nav_cupoch_experimental/visibility_control.h"
#include "vox_nav_utilities/map_manager_helpers.hpp"
#include "vox_nav_utilities/pcl_helpers.hpp"
#include "vox_nav_utilities/tf_helpers.hpp"

namespace bg = boost::geometry;

class CloudSegmentation : public rclcpp::Node
{

public:
  CloudSegmentation();

  ~CloudSegmentation();

  typedef message_filters::sync_policies::ApproximateTime<
      sensor_msgs::msg::PointCloud2,
      nav_msgs::msg::Odometry> CloudOdomApprxTimeSyncPolicy;
  typedef message_filters::Synchronizer<CloudOdomApprxTimeSyncPolicy> CloudOdomApprxTimeSyncer;

  void
  cloudOdomCallback(
    const sensor_msgs::msg::PointCloud2::ConstSharedPtr & cloud,
    const nav_msgs::msg::Odometry::ConstSharedPtr & odom);

  void determineObjectMovements(
    std::shared_ptr<cupoch::geometry::PointCloud> a,
    std::shared_ptr<cupoch::geometry::PointCloud> b,
    std_msgs::msg::Header header);

  std::shared_ptr<cupoch::geometry::PointCloud> denoiseCupochCloud(
    std::shared_ptr<cupoch::geometry::PointCloud> a,
    const std::shared_ptr<cupoch::geometry::PointCloud> b,
    double radius,
    int max_nn
  );

  Eigen::Matrix4f getTransfromfromConsecutiveOdoms(
    const nav_msgs::msg::Odometry::SharedPtr a,
    const nav_msgs::msg::Odometry::SharedPtr b
  );

  void clusterIndices2ClusterSet(
    const cupoch::utility::device_vector<int> & clusters,
    const thrust::host_vector<Eigen::Vector3f> & points,
    std::map<int, thrust::host_vector<Eigen::Vector3f>> & cluster_set);

  void clusterSet2CloudVector(
    const std::map<int, thrust::host_vector<Eigen::Vector3f>> & cluster_set,
    std::vector<std::shared_ptr<cupoch::geometry::PointCloud>> & cluster_vector);

private:
  message_filters::Subscriber<sensor_msgs::msg::PointCloud2> cloud_subscriber_;
  message_filters::Subscriber<nav_msgs::msg::Odometry> odom_subscriber_;
  message_filters::Subscriber<sensor_msgs::msg::PointCloud2> imu_subscriber_;
  std::shared_ptr<CloudOdomApprxTimeSyncer>
  cloud_odom_data_approx_time_syncher_;

  double dt_;
  double sensor_height_;
  bool recieved_first_;

  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_pub_;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;

  rclcpp::Time last_recieved_msg_stamp_;

  std::shared_ptr<cupoch::geometry::PointCloud> last_dynamic_pointcloud_cupoch_;
  nav_msgs::msg::Odometry::SharedPtr last_odom_msg_;
};
