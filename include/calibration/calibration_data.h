#pragma once

#include <basalt/utils/apriltag.h>
#include "calibration/aprilgrid.h"
#include <basalt/calibration/calibration.hpp>
#include <basalt/serialization/headers_serialization.h>
#include <tbb/concurrent_unordered_map.h>
#include "utils/common_types.h"
#include "libcbdetect/config.h"

namespace basalt {
  enum class CalibType {
    Checkerboard,
    AprilGrid
  };

  struct CalibCornerData {
    Eigen::aligned_vector<Eigen::Vector2d> corners;
    std::vector<int> corner_ids;
    std::vector<double> radii;  //!< threshold used for maximum displacement
    //! during sub-pix refinement; Search region is
    size_t seq; // sequence in the dataset
    //! slightly larger.
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    CalibCornerData() = default;

    CalibCornerData(const cbdetect::Corner &checkerboard_corner) {
      for (const auto &corner : checkerboard_corner.p) {
        corners.emplace_back(corner.x, corner.y);
      }
      // corner_ids = checkerboard_corner.ids; /* Leave it empty and get IDs through loop index*/
      for (const auto &radius : checkerboard_corner.r) {
        radii.push_back(radius);
      }
      // Lose the other fields
      // std::vector<cv::Point2d> v1;
      //  std::vector<cv::Point2d> v2;
      //  std::vector<cv::Point2d> v3;
      //  std::vector<double> score;
    }
  };

  struct ProjectedCornerData {
    Eigen::aligned_vector<Eigen::Vector2d> corners_proj;
    std::vector<bool> corners_proj_success;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  struct CalibInitPoseData {
    Sophus::SE3d T_a_c;
    size_t num_inliers;

    Eigen::aligned_vector<Eigen::Vector2d> reprojected_corners;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

  using CalibCornerMap = tbb::concurrent_unordered_map<TimeCamId, CalibCornerData,
          std::hash<TimeCamId>>;

  using CheckerboardCornerMap = tbb::concurrent_unordered_map<TimeCamId, cbdetect::Corner,
          std::hash<TimeCamId>>;

  using CalibInitPoseMap =
          tbb::concurrent_unordered_map<TimeCamId, CalibInitPoseData,
                  std::hash<TimeCamId>>;

  class CalibParams {
  public:
    virtual ~CalibParams() = default;

    /*
     * process method will be called in the detectCorners loop for each ManagedImage.
     * AprilGrid detection right now takes in a Managed image, but preferably we can make the tag detector take in
     * cv::Mat instead, since we have preprocessed the cv::Mat.
     * Checkboard corner detection takes in cv::Mat, so currently it's converting to a cv::Mat before passing the
     * image to the find_corners function.
     * */
    virtual void
    process(basalt::ManagedImage<uint16_t> &img_raw, CalibCornerData &ccd_good, CalibCornerData &ccd_bad) = 0;
  };

  class AprilGridParams : public CalibParams {
  public:
    AprilGridParams(const std::shared_ptr<AprilGrid> &april_grid) : ad(
            april_grid->getTagCols() * april_grid->getTagRows()) {
      this->april_grid = april_grid;
    }

    void process(basalt::ManagedImage<uint16_t> &img_raw, CalibCornerData &ccd_good, CalibCornerData &ccd_bad) override;

  private:
    std::shared_ptr<AprilGrid> april_grid;
    ApriltagDetector ad;
  };

  class CheckerboardParams : public CalibParams {
  public:
    CheckerboardParams(const std::shared_ptr<cbdetect::Params> &cb_params) {
      this->cb_params = cb_params;
    }

    void process(basalt::ManagedImage<uint16_t> &img_raw, CalibCornerData &ccd_good, CalibCornerData &ccd_bad) override;

  protected:
    std::shared_ptr<cbdetect::Params> cb_params;
  };
}

namespace cereal {
  template<class Archive>
  void serialize(Archive &ar, basalt::CalibCornerData &c) {
    ar(c.corners, c.corner_ids, c.radii, c.seq);
  }

  template<class Archive>
  void serialize(Archive &ar, basalt::CalibInitPoseData &c) {
    ar(c.T_a_c, c.num_inliers, c.reprojected_corners);
  }
}  // namespace cereal