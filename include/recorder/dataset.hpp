#pragma once

#include <rosbag/bag.h>
#include <ecal/msg/capnproto/subscriber.h>
#include <ecal_camera/CameraFactory.hpp>
#include "spdlog/spdlog.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>

namespace vk {
  enum class RecordMode {
    CONTINUOUS=1,
    SNAPSHOTS=2
  };

  class RosbagDatasetRecorder {
  public:
    typedef std::shared_ptr<RosbagDatasetRecorder> Ptr;

    void init(const vk::CameraParams &params, const RecordMode mode) {
      this->m_mode = mode;
      this->m_camera = vk::CameraFactory::getCameraHandler();
      this->m_camera->init(params); // Setup GUI for default params

      this->m_camera->registerSyncedCameraCallback(std::bind(&RosbagDatasetRecorder::callbackSyncedCameras, this, std::placeholders::_1));

      // Set name for bag file and rosbag
      {
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d-%I-%M-%S");
        std::string mode_type = this->m_mode == RecordMode::SNAPSHOTS ? "snapshot" : "continuous";

        this->m_bag_name = "./" + oss.str() + mode_type + "_recording.bag";

        this->m_bag = std::make_shared<rosbag::Bag>();
      }

      this->m_initialised = true;
    }

    inline bool is_running() const { return this->m_recording; }
    inline RecordMode get_mode() const { return this->m_mode; }
    inline void take_snapshot() { this->m_take_snapshot = true; }
    inline int get_num_snapshots() { return this->m_num_snapshots; }

    void start_record();
    void stop_record();

  private:
    bool m_initialised; // Set to true after callback is passed
    RecordMode m_mode;
    vk::CameraInterface::Ptr m_camera;

    std::string m_bag_name;
    std::shared_ptr<rosbag::Bag> m_bag;
    std::mutex m_mutex; // Safe access of m_bag
    std::shared_ptr<std::thread> m_thread_image;
    std::shared_ptr<std::thread> m_thread_imu;
    std::atomic<int> m_num_snapshots;
    std::atomic<bool> m_take_snapshot;
    std::atomic<bool> m_recording;

    void callbackSyncedCameras(const std::vector<vk::CameraFrameData::Ptr>& dataVector);
    void callbackImu(vk::ImuFrameData::Ptr data);


  };
}