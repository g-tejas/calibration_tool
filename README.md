## Todo
**Important**
- [ ] Write wireframe for the visualizer, maybe a scrub bar for the rosbag images
- [ ] Change basalt-headers to be using 8 bit instead of 16 bit, so no need for conversion.
  - Currently using this [method](https://stackoverflow.com/questions/51549624/how-to-convert-16-bit-image-to-8-bit-in-opencv-c) to convert 16 bit to 8 bit.
  - API can be found [here](https://docs.opencv.org/3.4/d3/d63/classcv_1_1Mat.html)
- [ ] Add drag and drop file feature for ros bag files, use the [librealsense code](https://github.com/IntelRealSense/librealsense/blob/b874e42685aed1269bc57a2fe5bf14946deb6ede/tools/rosbag-inspector/rs-rosbag-inspector.cpp#LL89C1-L89C86) (rosbag inspector)
- [ ] Overload [] operator for rosbag dataset? so we can get the image directly using a timestamp instead of calling .get_image_data
**Not important**
- [ ] Change glfw to submodule dependency instead.

## Trouble shooting stuff
- NFDE library changed from nativefiledialog-extended to ..._extended to be valid for cmake (hyphen in name not valid)
- #define IMGUI_DEFINE_MATH_OPERATORS error, most likely an error in the order with which things have been imported
  - Forked immvision and pushed changes to repo to quick fix the include headers in src_all_in_one.h
- Segmentation faults -> More often then not it's got something to do with uninitialized vectors and ImGui library.
- Use develop branch for most vilota repos
- basalt-sqrt does not build on macos, the original basalt does, mirror as well.
- Assertion in basalt-header is commented out, need to make a fork instead
- image.h has a overloaded constructor for cv::Mat

## Setup
```
mkdir build && cd build
cmake ..
cmake --build . -- -j
```

### Prerequisites 

C++
```
sudo apt-get install gcc g++
```

Libraries for graphics (Linux)

```
sudo apt-get install libglfw3 libglfw3-dev xorg xorg-dev
```

Libraries for graphics (Mac OS)
```
brew install glfw
```

## Preprocessor Macros
`PROJECT_ROOT`: Points to root of project / where cmake was invoked.

## Submodules
- [DearImGui](https://github.com/ocornut/imgui/tree/031148dc56d70158b3ad84d9be95b04bb3f5baaf)
- [ImPlot](https://github.com/epezent/implot/tree/18758e237e8906a97ddf42de1e75793526f30ce9)
- Basalt-headers
- Cereal
- Eigen
- Glad
- imgui
- immvision
- rosbag
- spdlog
- 

## Other relevant repos
- [How to use visualizer in dear imgui application #5920](https://github.com/isl-org/Open3D/issues/5920)
- https://github.com/vilota-dev/calibration-experiment
- https://github.com/SMRT-AIST/interactive_slam/tree/master
- [Visualize point cloud](https://stackoverflow.com/questions/10106288/pcl-visualize-a-point-cloud)
- [Tanagram](https://www.tangramvision.com/resources/depth-sensor-visualizer)
- [imgui robot dog app](https://jpieper.com/tag/imgui/)
- [imgui_ros](https://github.com/lucasw/imgui_ros/tree/melodic): the one with the drag n drop topics
