#include "calibration/calibrator.hpp"

namespace basalt {
  void AprilGridParams::process(basalt::ManagedImage<uint16_t> &img_raw, CalibCornerData &ccd_good, CalibCornerData &ccd_bad) {
    ad.detectTags(img_raw, ccd_good.corners,
                  ccd_good.corner_ids, ccd_good.radii,
                  ccd_bad.corners, ccd_bad.corner_ids, ccd_bad.radii);
  }

  void CheckerboardParams::process(basalt::ManagedImage<uint16_t> &img_raw, CalibCornerData &ccd_good, CalibCornerData &ccd_bad) {
    // Convert img_raw into cv::Mat
    cv::Mat image(img_raw.h, img_raw.w, CV_8U);

    uint8_t *dst = image.ptr();
    const uint16_t *src = img_raw.ptr;

    for (size_t i = 0; i < img_raw.size(); i++) {
      dst[i] = (src[i] >> 8);
    }

    // Requires intermediate cbdetect::Params struct
    std::vector<cbdetect::Board> boards;
    cbdetect::Corner corners; // will be populated for this image
    cbdetect::find_corners(image, corners, *this->cb_params);
    cbdetect::boards_from_corners(image, corners, boards, *this->cb_params); // Does the filtering for us

    if (!boards.empty()) {
        ccd_good = CalibCornerData(corners, boards); // Assuming you want to use the first board
//        ccd_bad = CalibCornerData // Nothing is rejected?
    }
  }

  void OpenCVCheckerboardParams::process(basalt::ManagedImage<uint16_t> &img_raw, basalt::CalibCornerData &ccd_good, basalt::CalibCornerData &ccd_bad) {
    using namespace cv;
    cv::Mat image(img_raw.h, img_raw.w, CV_8U);
    uint8_t *dst = image.ptr();
    const uint16_t *src = img_raw.ptr;

    for (size_t i = 0; i < img_raw.size(); i++) {
        dst[i] = (src[i] >> 8);
    }
    std::vector<cv::Point2f> corners;
    //CALIB_CB_FAST_CHECK saves a lot of time on images
    //that do not contain any chessboard corners
    bool patternfound = findChessboardCorners(image, this->pattern_size, corners, this->flags);

    if(patternfound)
        cornerSubPix(image, corners, cv::Size(11, 11), cv::Size(-1, -1),
                     TermCriteria(TermCriteria::MAX_ITER|TermCriteria::EPS , 30, 0.1));

    ccd_good = CalibCornerData(corners);
  }


  Calibrator::Calibrator(const std::shared_ptr<RosbagDataset> &dataset) {
    const fs::path temp = dataset->get_file_path();
    this->cache_path = temp.parent_path() / "calib-cam_detected_corners.cereal";
    this->dataset = dataset;
  }

  void Calibrator::detectCorners(const std::shared_ptr<CalibParams> &params) {
    if (this->loadCache()) {
      return;
    } else {
      spdlog::trace("No cached corners found, running corner detection");

      this->dataset->calib_corners.clear();
      this->dataset->calib_corners_rejected.clear();

      tbb::parallel_for(
              tbb::blocked_range<size_t>(0, this->dataset->get_image_timestamps().size()),
              [&](const tbb::blocked_range<size_t> &r) {
                for (size_t j = r.begin(); j != r.end(); ++j) {
                  int64_t timestamp_ns = this->dataset->get_image_timestamps()[j];
                  const std::vector<ImageData> &img_vec = this->dataset->get_image_data(timestamp_ns);

                  for (size_t i = 0; i < img_vec.size(); i++) {
                    if (img_vec[i].img.get()) {
                      CalibCornerData ccd_good;
                      CalibCornerData ccd_bad;

                      params->process(*img_vec[i].img, ccd_good, ccd_bad);
                      spdlog::debug("image ({},{})  detected {} corners ({} rejected)",
                                    timestamp_ns, i, ccd_good.corners.size(),
                                    ccd_bad.corners.size());

                      TimeCamId tcid(timestamp_ns, i);

                      ccd_good.seq = j;
                      ccd_bad.seq = j;

                      this->dataset->calib_corners.emplace(tcid, ccd_good);
                      this->dataset->calib_corners_rejected.emplace(tcid, ccd_bad);
                    }
                  }
                }
              });

      spdlog::debug("Successfully detected corners");
      this->saveCache();
    }
  };
} // namespace basalt