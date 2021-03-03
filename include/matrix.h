#pragma once

#include <string>
#include <vector>
#include <initializer_list>

namespace imgvol {

template<class T>
class Matrix {
 public:
  Matrix(size_t ncols, size_t nrows)
    : nrows_{nrows}
    , ncols_{ncols} {
      mat_.reserve(ncols_*nrows_);
  }

  Matrix(std::initializer_list<std::initializer_list<T>>&& list) {
    ncols_ = list.size();
    nrows_ = list[0].size();
    mat_.reserve(ncols_*nrows_);

    for (auto& line : list) {
      for (T& e : line) {
        mat_.push_back(e);
      }
    }
  }

  Matrix(const Matrix& mat) {
    mat_ = mat.mat_;
    ncols_ = mat.ncols_;
    nrows_ = mat.nrows_;
  }

  Matrix(Matrix&& mat) {
    mat_ = std::move(mat.mat_);
    ncols_ = mat.ncols_;
    nrows_ = mat.nrows_;
    mat.ncols_ = 0;
    mat.nrows_ = 0;
  }

  Matrix& operator=(const Matrix& mat) {
    mat_ = mat.mat_;
    ncols_ = mat.ncols_;
    nrows_ = mat.nrows_;
    return *this;
  }

  Matrix& operator=(Matrix&& mat) {
    mat_ = std::move(mat.mat_);
    ncols_ = mat.ncols_;
    nrows_ = mat.nrows_;
    mat.ncols_ = 0;
    mat.nrows_ = 0;
    return *this;
  }

  const T& operator[](size_t pos) const {
    return mat_[pos];
  }

  T& operator[](size_t pos) {
    return mat_[pos];
  }

  void operator()(T value, size_t i, size_t j) {
    mat_[j*ncols_ + i] = value;
  }

  const T& operator()(ssize_t i, size_t j) const {
    return mat_[j*ncols_ + i];
  }

 private:
  std::vector<T> mat_;
  size_t ncols_;
  size_t nrows_;
};

}