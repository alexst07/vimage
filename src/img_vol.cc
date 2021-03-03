#include "img_vol.h"
#include <fstream>

namespace imgvol {

ImgColor::ImgColor(size_t xsize, size_t ysize) {
  xsize_ = xsize_;
  ysize_ = ysize;
  img_.reserve(xsize*ysize);
}

ImgColor::~ImgColor() {}

ImgColor::ImgColor(ImgColor&& img) {
  xsize_ = img.xsize_;
  ysize_ = img.ysize_;
  Move(std::move(img));
  img.xsize_ = 0;
  img.ysize_ = 0;
}

ImgColor::ImgColor(const ImgColor& img) {
  xsize_ = img.xsize_;
  ysize_ = img.ysize_;
  Copy(img);
}

ImgColor& ImgColor::operator=(ImgColor&& img) {
  Move(std::move(img));
  return *this;
}

ImgColor& ImgColor::operator=(const ImgColor& img) {
  Copy(img);
  return *this;
}

void ImgColor::Copy(const ImgColor& img) {
  img_ = img.img_;
}

void ImgColor::Move(ImgColor&& img) {
  img_ = std::move(img.img_);
}

void ImgColor::operator()(std::array<uint8_t, 3> v, size_t x, size_t y) {
  img_[y*xsize_ + x] = v;
}

std::array<uint8_t, 3> ImgColor::operator()(size_t x, size_t y) const {
  return img_[y*xsize_ + x];
}

void ImgColor::operator()(std::array<uint8_t, 3> v, size_t i) {
  img_[i] = v;
}

void ImgColor::WriteImg(const std::string& file_name) {
  cv::Mat mat(ysize_, xsize_, CV_8UC3, cv::Scalar(0, 0, 0));

  for(size_t y=0;y<ysize_;y++) {
    for(size_t x=0;x<xsize_;x++)     {
        // get pixel
        cv::Vec3b & color = mat.at<cv::Vec3b>(y,x);

        std::array<uint8_t, 3> c = this->operator()(x, y);

        color[0] = c[0];
        color[1] = c[1];
        color[2] = c[2];
    }
  }

  cv::imwrite(file_name, mat);
}

size_t ImgColor::SizeX() const noexcept {
  return xsize_;
}

size_t ImgColor::SizeY() const noexcept {
  return ysize_;
}

////////////////////////////////////////////////////////////

ImgGray::ImgGray(size_t xsize, size_t ysize) {
  img_.reserve(xsize*ysize);

}

ImgGray::ImgGray(const uint8_t* data, size_t xsize, size_t ysize) {
  img_.reserve(xsize*ysize);

  for (int i = 0; i < ysize; i++) {
    for (int j = 0; j < xsize; j++) {
      img_[i*xsize + j] = data[i*xsize + j];
    }
  }
}

ImgGray::~ImgGray() {}

ImgGray::ImgGray(ImgGray&& img) {
  Move(std::move(img));
}

ImgGray::ImgGray(const ImgGray& img) {
  Copy(img);
}

ImgGray& ImgGray::operator=(ImgGray&& img) {
  Move(std::move(img));
  return *this;
}

ImgGray& ImgGray::operator=(const ImgGray& img) {
  Copy(img);
  return *this;
}

void ImgGray::Copy(const ImgGray& img) {
  img_ = img.img_;
  xsize_ = img.xsize_;
  ysize_ = img.ysize_;
}

void ImgGray::Move(ImgGray&& img) {
  img_ = std::move(img.img_);
  img.xsize_ = 0;
  img.ysize_ = 0;
}


void ImgGray::operator()(uint8_t v, size_t x, size_t y) {
  img_[y*xsize_ + x] = v;
}

uint8_t ImgGray::operator()(size_t x, size_t y) const {
  return img_[y*xsize_ + x];
}

void ImgGray::WriteImg(const std::string& file_name) {
  cv::Mat mat(ysize_, xsize_, CV_8UC1, cv::Scalar(0, 0, 0));

  for(size_t y=0;y<ysize_;y++) {
    for(size_t x=0;x<xsize_;x++)     {
        uint8_t c = this->operator()(x, y);
        mat.at<uchar>(y,x) = c;
    }
  }

  cv::imwrite(file_name, mat);
}

size_t ImgGray::SizeX() const noexcept {
  return xsize_;
}

size_t ImgGray::SizeY() const noexcept {
  return ysize_;
}

/////////////////////////////////////////////////////////////////////////

ImgVol::ImgVol(size_t xsize, size_t ysize, size_t zsize) {
  img_.reserve(xsize*ysize*zsize);
  xsize_ = xsize;
  ysize_ = ysize;
  zsize_ = zsize;
}

ImgVol::ImgVol(std::string file_name) {
  std::ifstream in_file;
  size_t size = 0; // here

	in_file.open(file_name, std::ios::in|std::ios::binary|std::ios::ate );
	char* oData = 0;

	in_file.seekg(0, std::ios::end); // set the pointer to the end
	size = in_file.tellg() ; // get the length of the file
	in_file.seekg(0, std::ios::beg); // set the pointer to the beginning
  img_.reserve(size);

	oData = new char[size];
	in_file.read(reinterpret_cast<char*>(img_.data()), size);
  in_file.close();
}

ImgVol::ImgVol(const ImgVol& img) {
  for (size_t x = 0; x < SizeX(); x++) {
    for (size_t y = 0; y < SizeY(); y++) {
      for (size_t z = 0; z < SizeZ(); z++) {
        SetVoxelIntensity(img.VoxelIntensity(x, y, z), x, y, z);
      }
    }
  }
}

ImgVol& ImgVol::operator=(const ImgVol& img) {
  for (size_t x = 0; x < SizeX(); x++) {
    for (size_t y = 0; y < SizeY(); y++) {
      for (size_t z = 0; z < SizeZ(); z++) {
        SetVoxelIntensity(img.VoxelIntensity(x, y, z), x, y, z);
      }
    }
  }

  return *this;
}

ImgVol::~ImgVol() {}

void ImgVol::WriteImg(std::string file_name) {
  std::ofstream fout;
  fout.open(file_name, std::ios::binary | std::ios::out);

  fout.write((char*) img_.data(), img_.size());

  fout.close();
}

int ImgVol::VoxelIntensity(size_t x, size_t y, size_t z) const {
  return img_[z*xsize_*ysize_ + y*xsize_ + x];
}

void ImgVol::SetVoxelIntensity(float b, size_t x, size_t y, size_t z) {
  img_[z*xsize_*ysize_ + y*xsize_ + x] = uint8_t(b);
}

int ImgVol::operator()(size_t x, size_t y, size_t z) const{
  return VoxelIntensity(x, y, z);
}

size_t ImgVol::SizeX() const noexcept {
  return xsize_;
}

size_t ImgVol::SizeY() const noexcept {
  return ysize_;
}

size_t ImgVol::SizeZ() const noexcept {
  return zsize_;
}

std::ostream& operator<<(std::ostream& stream,
                         ImgVol& img) {
  stream << "Img sizes[X: "<< img.SizeX() << ", Y: " << img.SizeY()
  << ", Z: " << img.SizeZ() << "]";
  return stream;
}

}
