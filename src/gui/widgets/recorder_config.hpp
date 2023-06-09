#pragma once

#include "gui/imports.h"

void draw_recorder_config(std::shared_ptr<vk::RosbagDatasetRecorder> &dataset_recorder,
                          std::shared_ptr<vk::CameraParams>& recorder_params) {
  if (!ImGui::Begin("Calibration Dataset Recorder Configuration")) {
    ImGui::End();
    return;
  }

  static vk::RecordMode selectedMode = vk::RecordMode::SNAPSHOTS;
  static std::map<vk::RecordMode, std::string> recordModeNames = {
          {vk::RecordMode::CONTINUOUS, "Continuous"},
          {vk::RecordMode::SNAPSHOTS, "Snapshot"}
  };

  std::string selectedModeName = recordModeNames[selectedMode];
  if (ImGui::BeginCombo("Record Mode", selectedModeName.c_str())) {
    for (auto &item : recordModeNames) {
      bool isSelected = (selectedModeName == item.second);
      if (ImGui::Selectable(item.second.c_str(), isSelected)) {
        selectedMode = item.first;
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  if (ImGui::Button("Start Recording")) {
    dataset_recorder->init(*recorder_params, selectedMode);
    dataset_recorder->start_record();
  }

  ImGui::SameLine();

  if (ImGui::Button("Stop Recording")) {
    dataset_recorder->stop_record();
  }

  if (dataset_recorder->is_running() && dataset_recorder->get_mode() == vk::RecordMode::SNAPSHOTS) {
    ImGui::Text("Snapshots Taken: %i", dataset_recorder->get_num_snapshots());

    if (ImGui::Button("Take snapshot") || ImGui::IsKeyPressed(ImGuiKey_Space)) {
      dataset_recorder->take_snapshot();
    }
  }

  ImGui::End();
}