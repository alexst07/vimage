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
    auto it = list.begin();
    nrows_ = it->size();
    mat_.reserve(ncols_*nrows_);

    for (auto& line : list) {
      for (const T& e : line) {
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
    mat_[i*ncols_ + j] = value;
  }

  const T& operator()(size_t i, size_t j) const {
    return mat_[i*ncols_ + j];
  }

  T& operator()(size_t i, size_t j) {
    return mat_[i*ncols_ + j];
  }

  size_t ncols() const {
    return ncols_;
  }

  size_t nrows() const {
    return nrows_;
  }

 private:
  std::vector<T> mat_;
  size_t ncols_;
  size_t nrows_;
};

template<class T>
Matrix<T> MultMatrices(const Matrix<T>& a, const Matrix<T>& b) {
  size_t m1 = a.nrows();
  size_t m2 = a.ncols();
  size_t n1 = b.nrows();
  size_t n2 = b.ncols();

  Matrix<T> res = Matrix<T>(m1, n2);
  for (size_t i = 0; i < m1; i++) {
    for (size_t j = 0; j < n2; j++) {
      res(i, j)= 0;
      for (size_t x = 0; x < m2; x++) {
        T v = a(i, j)*b(x, j);
        res(v, i, j);
      }
    }
  }
}

}