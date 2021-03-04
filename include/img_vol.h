#pragma once

#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

namespace imgvol {

class ImgColor {
 public:
  ImgColor() = delete;

  ImgColor(size_t xsize, size_t ysize);

  ImgColor(const ImgColor&);

  ImgColor(ImgColor&&);

  ImgColor& operator=(const ImgColor&);

  ImgColor& operator=(ImgColor&&);

  ~ImgColor();

  std::array<uint8_t, 3> operator()(size_t x, size_t y) const;

  void operator()(std::array<uint8_t, 3> v, size_t x, size_t y);

  void operator()(std::array<uint8_t, 3> v, size_t i);

  size_t SizeX() const noexcept;

  size_t SizeY() const noexcept;

  void WriteImg(const std::string& file_name);

 private:
  void Copy(const ImgColor&);
  void Move(ImgColor&&);
  std::vector<std::array<uint8_t, 3>> img_;
  size_t xsize_;
  size_t ysize_;
};

class ImgGray {
 public:
  ImgGray(size_t xsize, size_t ysize);

  ImgGray(const uint8_t* data, size_t xsize, size_t ysize);

  ImgGray(const ImgGray&);

  ImgGray(ImgGray&&);

  ImgGray& operator=(const ImgGray&);

  ImgGray& operator=(ImgGray&&);

  ~ImgGray();

  uint8_t operator()(size_t x, size_t y) const;

  void operator()(uint8_t v, size_t x, size_t y);

  size_t SizeX() const noexcept;

  size_t SizeY() const noexcept;

  void WriteImg(const std::string& file_name);

 private:
  void Copy(const ImgGray&);
  void Move(ImgGray&&);
  std::vector<uint8_t> img_;
  size_t xsize_;
  size_t ysize_;
};

class ImgVol {
 public:
  enum class Axis {
        aX, aY, aZ
    };

  ImgVol(size_t xsize, size_t ysize, size_t zsize);

  ImgVol(std::string file_name);

  ImgVol(const ImgVol& img);

  ImgVol(ImgVol&& img) {
    img_ = std::move(img.img_);
  }

  ImgVol& operator=(const ImgVol& img);

  ImgVol& operator=(ImgVol&& img) {
    img_ = std::move(img.img_);
  }

  ~ImgVol();

  int operator()(size_t x, size_t y, size_t z) const;

  int VoxelIntensity(size_t x, size_t y, size_t z) const;

  void SetVoxelIntensity(float b, size_t x, size_t y, size_t z);

  size_t SizeX() const noexcept;

  size_t SizeY() const noexcept;

  size_t SizeZ() const noexcept;

  float DimX() const noexcept;

  float DimY() const noexcept;

  float DimZ() const noexcept;

  void WriteImg(std::string file_name);

  uint8_t Imax();

  friend std::ostream& operator<<(std::ostream& stream,
                                  ImgVol& img);

 private:
  std::vector<uint8_t> img_;
  size_t xsize_;
  size_t ysize_;
  size_t zsize_;
};

}
