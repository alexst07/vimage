#include <limits>
#include <array>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "operations.h"
#include "matrix.h"

namespace imgvol {

Img2D Cut(const ImgVol& img_vol, ImgVol::Axis axis, size_t pos, bool w) {
  size_t s1, s2;

  if (axis == ImgVol::Axis::aZ) {
    s1 = img_vol.SizeX();
    s2 = img_vol.SizeY();
  } else if (axis == ImgVol::Axis::aX) {
    s1 = img_vol.SizeZ();
    s2 = img_vol.SizeY();
  } else {
    s1 = img_vol.SizeZ();
    s2 = img_vol.SizeX();
  }

  Img2D img2d(s1, s2);

  for (int i = 0; i < s1; i++) {
    for (int j = 0; j < s2; j++) {
      if (axis == ImgVol::Axis::aZ) {
        img2d(img_vol((w? (s1 - i -1): i), j, pos), i, j);
      } else if (axis == ImgVol::Axis::aX) {
        img2d(img_vol(pos, j, (w? (s1 - i -1): i)), i, j);
      } else {
        img2d(img_vol(j, pos, (w? (s1 - i - 1): i)), i, j);
      }
    }
  }

  return img2d;
}

std::array<size_t, 2> MinMax(const Img2D& img) {
  size_t min = std::numeric_limits<size_t>::max();
  size_t max = std::numeric_limits<size_t>::min();

  auto data = img.Data();

  for (int i = 0; i < img.NumPixels(); i++) {
    if (data[i] < min)
      min = data[i];

    if (data[i] > max)
      max = data[i];
  }

  std::array<size_t, 2> arr = {min, max};
  return arr;
}

void Normalize(Img2D& img, size_t num_bits) {
  int h = pow(2, num_bits) - 1;
  float k1 = 0, k2 = h;
  float i1 = MinMax(img)[0];

  float i2 = MinMax(img)[1];

  for (size_t i = 0; i < img.NumPixels(); i++) {
    if (img[i] < i1)
      img[i] = k1;

    if (img[i] >= i1 && img[i] < i2)
      img[i] = int(((k2 - k1)/(i2-i1))*(img[i] - i1) + k1);

    if (img[i] >= i2)
      img[i] = k2;
  }

}

void Negative(Img2D& img) {
  float k1 = MinMax(img)[1], k2 = MinMax(img)[0];
  float i1 = MinMax(img)[0];

  float i2 = MinMax(img)[1];

  for (size_t i = 0; i < img.NumPixels(); i++) {
    if (img[i] < i1)
      img[i] = k1;

    if (img[i] >= i1 && img[i] < i2)
      img[i] = int(((k2 - k1)/(i2-i1))*(img[i] - i1) + k1);

    if (img[i] >= i2)
      img[i] = k2;
  }
}

void BrightinessContrast(Img2D& img, size_t num_bits, float b, float c) {
  float h = MinMax(img)[1];

  b = 100 -b;
  c = 100 -c;

  float k1 = 0;
  float k2 = h;

  float b_real = (b/100)*h;
  float c_real = (c/100)*h;

  float i1 = (2*b_real - c_real)/2;
  float i2 = (c_real + 2*b_real)/2;

  for (size_t i = 0; i < img.NumPixels(); i++) {
    if (img[i] < i1)
      img[i] = k1;

    if (img[i] >= i1 && img[i] < i2)
      img[i] = int(((k2 - k1)/(i2-i1))*(img[i] - i1) + k1);

    if (img[i] >= i2)
      img[i] = k2;
  }
}

ImgColor ColorLabels(const Img2D& img_cut, const Img2D& img_lb, size_t nbits) {
  std::map<int, int> tab_color;
  std::default_random_engine generator;

  float h = pow(2, nbits) - 1;
  std::uniform_int_distribution<int> distribution(0,int(h));
  ImgColor img_color(img_cut.SizeX(), img_cut.SizeY());
  std::array<uint8_t, 3> cor;
  std::array<uint8_t, 3> cinza;

  float y, cg, co;
  float maxH = MinMax(img_cut)[1];

  for (int i = 0; i < img_lb.NumPixels(); i++) {
    if (img_lb[i] == 0) {
      cor[0] = cor[1] = cor[2] = (int)(255*img_cut[i]/maxH);
      img_color(cor, i);
      continue;
    }

    if (i%2 == 0) {
      int p = img_lb[i];

      if (tab_color.find(p) == tab_color.end())
        tab_color.insert(std::pair<int, int>(p, distribution(generator)));

      int m = tab_color[p];
      float v = m/h;

      v = 4*v +1;
      cor[0] = h*std::max(float(0), float((3- abs(v-4) - abs(v - 5))/2));
      cor[1] = h*std::max(float(0), float((4- abs(v-2) - abs(v - 4))/2));
      cor[2] = h*std::max(float(0), float((3- abs(v-1) - abs(v - 2))/2));

      cinza[0] = img_cut[i] * 0.3;
      cinza[1] = img_cut[i] * 0.3;
      cinza[2] = img_cut[i] * 0.3;

      y = img_cut[i];
      cg = -0.25 * cor[0] + 0.5 * cor[1] - 0.25 * cor[2] + 0.5 + h/2;
      co = 0.5 * cor[0] - 0.5 * cor[2] + 0.5 + h/2;

      cor[0] = (int)(255*(y - cg + co)/h) + 255;
      cor[1] = (int)(255*(y + cg - h/2)/h) + 255;
      cor[2] = (int)(255*(y - cg - co + h)/h) + 255;

      cor[0] = cor[0] * 0.7;
      cor[1] = cor[1] * 0.7;
      cor[2] = cor[2] * 0.7;

      cor[0] = cinza[0] + cor[0];
      cor[1] = cinza[1] + cor[1];
      cor[2] = cinza[2] + cor[2];

      cor[0] = 255*cor[0]/(255*2);
      cor[1] = 255*cor[1]/(255*2);
      cor[2] = 255*cor[2]/(255*2);

      img_color(cor, i);
    } else {
      cor[0] = cor[1] = cor[2] = (int)(255*img_cut[i]/maxH);
      img_color(cor, i);
    }
  }
  return img_color;
}

std::array<float, 3> VecNorm(std::array<float, 3> v) {
  float r = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  std::array<float, 3> vr = {v[0]/r, v[1]/r, v[2]/r};
  return vr;
}

ImgGray CortePlanar(ImgVol& img, std::array<float, 3> p1, std::array<float, 3> vec) {
  vec = VecNorm(vec);
  // Handle vec[2] = 0
  float alpha_x = atan(vec[1]/ vec[2]);
  float diagonal = Diagonal(std::array<float, 3>{(float)img.SizeX(),
      (float)img.SizeY(), (float)img.SizeZ()});

  if (vec[2] < 0) {
    alpha_x -= M_PI;
  }

  float vzl = vec[2]/cos(alpha_x);
  std::array<float, 3> qc = {diagonal/2, diagonal/2, -diagonal/2};

  float alpha_y = atan(vec[0]/ vzl);

  if (vec[2] == 0) {
    if ((vec[1] != 0) && (vec[0] == 0)) {
      alpha_y = 0;
      alpha_x = M_PI/2*Sign(vec[1]);
    }

    if ((vec[0] != 0) && (vec[1] == 0)) {
      alpha_y = M_PI/2*Sign(vec[0]);
      alpha_x = 0;
    }
  }

  Matrix<double> t_mqc = {
    {1, 0, 0, -qc[0]},
    {0, 1, 0, -qc[1]},
    {0, 0, 1, -qc[2]},
    {0, 0, 0, 1     }
  };

  alpha_x = -alpha_x;
  Matrix<double> rotx = {
    {1,      0,              0,      0},
    {0, cos(alpha_x), -sin(alpha_x), 0},
    {0, sin(alpha_x), cos(alpha_x),  0},
    {0,      0,             0,       1}
  };

  Matrix<double> roty = {
    {cos(alpha_y),      0,  sin(alpha_y),      0},
    {0,                 1,       0,            0},
    {-sin(alpha_y),     0,  cos(alpha_x),      0},
    {0,                 0,       0,            1}
  };

  Matrix<double> t_p1 = {
    {1, 0, 0, p1[0]},
    {0, 1, 0, p1[1]},
    {0, 0, 1, p1[2]},
    {0, 0, 0, 1}
  };
  Matrix<double> tmp_rx_tqc = MultMatrices(roty, t_mqc);
  Matrix<double> tmp_ry_rx_tqc = MultMatrices(rotx, tmp_rx_tqc);
  Matrix<double> phi_inv = MultMatrices(t_p1, tmp_ry_rx_tqc);

  ImgGray img_out(diagonal, diagonal);
  Matrix<double> q = Matrix<double>(1, 4);

  for (int u = 0; u < (int) diagonal; u++) {
    for (int v = 0; v < (int) diagonal; v++) {
      q[0] = u;
      q[1] = v;
      q[2] = -diagonal/2;
      q[3] = 1;

      Matrix<double> p = MultMatrices(phi_inv, q);
      int intensity;

      if (p[0] < 0 || p[1] < 0 || p[2] < 0) {
        intensity = 0;
      } else  if (p[0] >= img.SizeX() || p[1] >= img.SizeY() || p[2] >= img.SizeZ()) {
        intensity = 0;
      } else {
        intensity = img(p[0], p[1], p[2]);
      }

      img_out(intensity, u, v);
    }
  }

  return img_out;
}

std::array<float,3> CalcVector(std::array<float,3> p1, std::array<float,3> pn) {
  std::array<float, 3> sub = {pn[0] - p1[0], pn[1] - p1[1], pn[2] - p1[2]};

  float r = sqrt(sub[0]*sub[0] + sub[1]*sub[1] + sub[2]*sub[2]);
  std::array<float, 3> vr = {sub[0]/r, sub[1]/r, sub[2]/r};

  return vr;
}

void FillImg(const ImgGray& img_gray, size_t z, ImgVol* img_vol) {
  for (size_t iy = 0; iy < img_gray.SizeY(); iy++) {
    for (size_t ix = 0; ix < img_gray.SizeX(); ix++) {
      img_vol->SetVoxelIntensity(img_gray(ix, iy), ix, iy, z);
    }
  }
}

ImgVol ReformataImg(ImgVol& img, size_t n, std::array<float,3> p1, std::array<float,3> pn) {
  float dz2 = (pn[2] - p1[2])/float(n);

  std::array<float, 3> sub = {pn[0] - p1[0], pn[1] - p1[1], pn[2] - p1[2]};

  float lambda = sqrt(sub[0]*sub[0] + sub[1]*sub[1] + sub[2]*sub[2]);
  std::array<float, 3> vec = {sub[0]/lambda, sub[1]/lambda, sub[2]/lambda};
  lambda = lambda/n;

  float diagonal = Diagonal(std::array<float, 3>{(float)img.SizeX(),
      (float)img.SizeY(), (float)img.SizeZ()});

  ImgVol img_vol(diagonal, diagonal, n);

  std::array<float, 3> p = p1;
  for (size_t i = 0; i < n; i++) {
    std::array<float, 3> v_inc = {lambda*vec[0], lambda*vec[1], lambda*vec[2]};
    p[0] = p[0] + v_inc[0];
    p[1] = p[1] + v_inc[1];
    p[2] = p[2] + v_inc[2];
    ImgGray img_gray = CortePlanar(img, p, vec);

    FillImg(img_gray, i, &img_vol);
  }

  return img_vol;
}

ImgGray MaxIntensionProjection(ImgVol& img, float delta_x, float delta_y, std::array<float, 3> vet_normal) {
  float diagonal = Diagonal(std::array<float, 3>{(float) img.SizeX(),
      (float) img.SizeY(), (float) img.SizeZ()});

  vet_normal = VecNorm(vet_normal);
  NormalizeImage(img);

  Matrix<double> pc_l = {
    {1, 0, 0, -diagonal/2},
    {0, 1, 0, -diagonal/2},
    {0, 0, 1, -diagonal/2},
    {0, 0, 0, 1}
  };

  Matrix<double> rotx = {
    {1, 0, 0, 0},
    {0, cos(-delta_x), -sin(-delta_x), 0},
    {0, sin(-delta_x), cos(-delta_x), 0},
    {0, 0, 0, 1}
  };

  Matrix<double> roty = {
    {cos(-delta_y), 0, sin(-delta_y), 0},
    {0, 1, 0, 0},
    {-sin(-delta_y), 0, cos(-delta_y), 0},
    {0, 0, 0, 1}
  };

  Matrix<double> pc = {
    {1, 0, 0, (double)(img.SizeX()-1)/2},
    {0, 1, 0, (double)(img.SizeY()-1)/2},
    {0, 0, 1, (double)(img.SizeZ()-1)/2},
    {0, 0, 0, 1}
  };

  float nx = (float) img.SizeX();
  float ny = (float) img.SizeY();
  float nz = (float) img.SizeZ();
  std::vector<std::array<float, 3>> nj = {{1, 0, 0}, {-1, 0, 0},
                                          {0, 1, 0}, {0, -1, 0},
                                          {0, 0, 1}, {0, 0, -1}};

  std::vector<std::array<float, 3>> cj = {{nx-1, (ny-1)/2, (nz-1)/2}, {0, (ny-1)/2, (nz-1)/2},
                                          {(nx-1)/2, ny-1, (nz-1)/2}, {(nx-1)/2, 0, (nz-1)/2},
                                          {(nx-1)/2, (ny-1)/2, nz-1}, {(nx-1)/2, (ny-1)/2, 0}};


  Matrix<double> v_norm_mat = Matrix<double>(1, 4);
  v_norm_mat[0] = vet_normal[0];
  v_norm_mat[1] = vet_normal[1];
  v_norm_mat[2] = vet_normal[2];
  v_norm_mat[3] = 1;

  Matrix<double> tmp_rx_norm_mat = MultMatrices(roty, v_norm_mat);
  Matrix<double> phi_inv_norm = MultMatrices(rotx, tmp_rx_norm_mat);
  phi_inv_norm[3] = 0;

  ImgGray img_out(diagonal, diagonal);

  float term_1, term_2, term_3, lambda[6], lambda_max, lambda_min;

  std::array<float,3> p1;
  std::array<float,3> pn;

  Matrix<double> q = Matrix<double>(1, 4);

  Matrix<double> find_point = Matrix<double>(1, 4);

  for (int i = 0; i < diagonal; i++) {
    for (int j = 0; j < diagonal; j++) {
      lambda_max = 0;
      lambda_min = 2*diagonal;

      q[0] = i;
      q[1] = j;
      q[2] = -diagonal/2;
      q[3] = 1;

      Matrix<double> tmp_pcl_q = MultMatrices(pc_l, q);
      Matrix<double> tmp_rx_pcl_q = MultMatrices(roty, tmp_pcl_q);
      Matrix<double> tmp_ry_rx_pcl_q = MultMatrices(rotx, tmp_rx_pcl_q);
      Matrix<double> q_inv = MultMatrices(pc, tmp_ry_rx_pcl_q);
      q_inv[3] = 0;

      bool passed_if = false;

      for (int a = 0; a < 6; a++) {
        term_1 = 0;
        term_2 = 0;
        term_3 = 0;
        for (int x = 0; x < 3; x++) {
          term_1 += nj[a][x]*cj[a][x];
        }

        for (int x = 0; x < 3; x++) {
          term_2 += nj[a][x]*q_inv[x];
        }

        for (int x = 0; x < 3; x++) {
          term_3 += nj[a][x]*phi_inv_norm[x];
        }

        lambda[a] = (term_1-term_2)/term_3;

        find_point[0] = std::round(q_inv[0] + lambda[a]*phi_inv_norm[0]);
        find_point[1] = std::round(q_inv[1] + lambda[a]*phi_inv_norm[1]);
        find_point[2] = std::round(q_inv[2] + lambda[a]*phi_inv_norm[2]);
        find_point[3] = 1;

        if (find_point[0] >= 0 && find_point[0] < nx && find_point[1] >= 0 && find_point[1] < ny &&
          find_point[2] >= 0 && find_point[2] < nz) {
          passed_if = true;

          if (lambda[a] > lambda_max) {
            lambda_max = lambda[a];
            pn[0] = find_point[0];
            pn[1] = find_point[1];
            pn[2] = find_point[2];
          }

          if (lambda[a] < lambda_min) {
            lambda_min = lambda[a];
            p1[0] = find_point[0];
            p1[1] = find_point[1];
            p1[2] = find_point[2];
          }
        }
      } // for a

      if (passed_if) {
        float dda = Dda3d(img, p1, pn);
        img_out(static_cast<int>(dda), i, j);
        passed_if = false;
      }

    }
  }

  return img_out;
}

float Dda3d(ImgVol& img, std::array<float,3> p1, std::array<float,3> pn) {
  float n;
  float Dx, Dy, Dz;
  float dx, dy, dz;
  if (p1 == pn) {
    n = 1;
  } else {
    Dx = pn[0] - p1[0];
    Dy = pn[1] - p1[1];
    Dz = pn[2] - p1[2];

    if (abs(Dx) >= abs(Dy) && abs(Dx) >= abs(Dz)) {
      n = abs(Dx) + 1;
      dx = Sign(Dx);
      dy = dx*Dy/Dx;
      dz = dx*Dz/Dx;
    } else {
      if (abs(Dy) >= abs(Dx) && abs(Dy) >= abs(Dz)) {
        n = abs(Dy) + 1;
        dy = Sign(Dy);
        dx = dy*Dx/Dy;
        dz = dy*Dz/Dy;
      } else {
        n = abs(Dz) + 1;
        dz = Sign(Dz);
        dx = dz*Dx/Dz;
        dy = dz*Dy/Dz;
      }
    }
  }

  std::array<float,3> p = p1;
  float max_i = -1;

  for (size_t k = 0; k < n; k++) {
    int i = img(p[0], p[1], p[2]);

    if (i > max_i) {
      max_i = i;
    }

    p[0] = p[0] + dx;
    p[1] = p[1] + dy;
    p[2] = p[2] + dz;
  }

  return max_i;
}

std::array<float, 3> Translate(std::array<float, 3> point, std::array<float, 3> dist) {
  std::array<float, 3> p;
  p[0] = point[0] + dist[0];
  p[1] = point[1] + dist[1];
  p[2] = point[2] + dist[2];

  return p;
}

std::array<float, 3> RotateZ(std::array<float, 3> point, float rad) {
  std::array<float, 3> p;
  p[0] = cos(rad)*point[0] - sin(rad)*point[1];
  p[1] = sin(rad)*point[0] + cos(rad)*point[1];
  p[2] = point[2];

  return p;
}

std::array<float, 3> RotateX(std::array<float, 3> point, float rad) {
  std::array<float, 3> p;
  p[0] = point[0];
  p[1] = cos(rad)*point[1] - sin(rad)*point[2];
  p[2] = sin(rad)*point[1] + cos(rad)*point[2];

  return p;
}

std::array<float, 3> RotateY(std::array<float, 3> point, float rad) {
  std::array<float, 3> p;
  p[0] = cos(rad)*point[0] + sin(rad)*point[2];
  p[1] = point[1];
  p[2] = -sin(rad)*point[0] + cos(rad)*point[2];

  return p;
}

float DotProduct(std::array<float, 3> vec) {
  std::array<float, 3> ref = {0, 0, -1};

  float res = ref[0]*vec[0];
  res += ref[1]*vec[1];
  res += ref[2]*vec[2];

  return res;
}

float Diagonal(std::array<float, 3> size) {
  float res = size[0]*size[0] + size[1]*size[1] + size[2]*size[2];
  res = sqrt(res);
}

bool TestVisibleFace(std::array<float, 3> face, std::array<float, 3> rad) {
  std::array<float, 3> fp = RotateX(face, rad[0]);
  fp = RotateY(fp, rad[1]);
  fp = RotateZ(fp, rad[2]);

  float dot_prod = DotProduct(fp);

  if (dot_prod > 0)
    return true;

  return false;
}

std::array<bool, 6> VisibleFaces(std::array<float, 3> rad) {
  std::vector<std::array<float, 3>> faces = {{1, 0, 0}, {-1, 0, 0},
                                             {0, 1, 0}, {0, -1, 0},
                                             {0, 0, 1}, {0, 0, -1}};

  std::array<bool, 6> res;
  int i = 0;
  for (const auto& f: faces) {
    res[i] = TestVisibleFace(f, rad);
    i++;
  }

  return res;
}

std::vector<std::array<float, 3>> VertexWireframe(const ImgVol& img_vol, std::array<float, 3> rad) {
  std::array<float, 3> size;
  size[0] = img_vol.SizeX();
  size[1] = img_vol.SizeY();
  size[2] = img_vol.SizeZ();
  std::array<float, 3> dist_center = {-size[0]/2, -size[1]/2, -size[2]/2};

  float diagonal = Diagonal(size);

  // Conferir o -diagonal/2
  std::array<float, 3> dist_diagonal = {diagonal/2, diagonal/2, -diagonal/2};

  std::vector<std::array<size_t, 3>> in_vec = {{0, 0, 0}, {img_vol.SizeX(), 0, 0},
                                              {0, img_vol.SizeY(), 0}, {img_vol.SizeX(), img_vol.SizeY(), 0},
                                              {0, 0, img_vol.SizeZ()}, {img_vol.SizeX(), 0, img_vol.SizeZ()},
                                              {0, img_vol.SizeY(), img_vol.SizeZ()},
                                              {img_vol.SizeX(), img_vol.SizeY(), img_vol.SizeZ()}};

  std::vector<std::array<float, 3>> out_vec;

  for (const auto& e: in_vec) {
    std::array<float, 3> point = {(float) e[0], (float) e[1], (float) e[2]};
    std::array<float, 3> fp = Translate(point, dist_center);
    fp = RotateX(fp, rad[0]);
    fp = RotateY(fp, rad[1]);
    fp = RotateZ(fp, rad[2]);
    fp = Translate(fp, dist_diagonal);
    out_vec.push_back(fp);
  }

  return out_vec;
}

float Sign(float v) {
  if (v < 0)
    return -1;
  else if (v == 0)
    return 0;
  else
    return 1;
}

void DrawLine(std::array<float, 3> p1, std::array<float, 3> p2, ImgGray& img) {
  int n;
  float Du, Dv;
  float du, dv;

  if (p1 == p2)
    n = 1;
  else {
    Du = p2[0] - p1[0];
    Dv = p2[1] - p1[1];

    if (abs(Du) >= abs(Dv)) {
      n = abs(Du) + 1;
      du = Sign(Du);
      dv = du * Dv/Du;
    } else {
      n = abs(Dv) + 1;
      dv = Sign(Dv);
      du = dv * Du/Dv;
    }
  }

  std::array<float, 3> p = p1;
  for (int k = 0; k < n; k++) {
    img(255, p[0], p[1]);
    p[0] += du;
    p[1] += dv;
  }
}

ImgGray DrawWireframe(const ImgVol& img_vol, std::array<float, 3> rad) {
  std::array<float, 3> size;
  size[0] = img_vol.SizeX();
  size[1] = img_vol.SizeY();
  size[2] = img_vol.SizeZ();

  std::array<float, 3> dist_center = {-size[0]/2, -size[1]/2, -size[2]/2};

  float diagonal = Diagonal(size);

  std::array<float, 3> dist_diagonal = {diagonal/2, diagonal/2, -diagonal/2};

  ImgGray res_img(diagonal, diagonal);

  std::vector<std::array<float, 3>> vertex = VertexWireframe(img_vol, rad);

  std::array<bool, 6> faces = VisibleFaces(rad);

  if (faces[5]) {
    DrawLine(vertex[0], vertex[1], res_img);
    DrawLine(vertex[0], vertex[2], res_img);
    DrawLine(vertex[1], vertex[3], res_img);
    DrawLine(vertex[2], vertex[3], res_img);
  }

  if (faces[3]) {
    DrawLine(vertex[0], vertex[1], res_img);
    DrawLine(vertex[0], vertex[4], res_img);
    DrawLine(vertex[1], vertex[5], res_img);
    DrawLine(vertex[4], vertex[5], res_img);
  }

  if (faces[0]) {
    DrawLine(vertex[1], vertex[5], res_img);
    DrawLine(vertex[1], vertex[3], res_img);
    DrawLine(vertex[3], vertex[7], res_img);
    DrawLine(vertex[5], vertex[7], res_img);
  }

  if (faces[1]) {
    DrawLine(vertex[0], vertex[4], res_img);
    DrawLine(vertex[0], vertex[2], res_img);
    DrawLine(vertex[4], vertex[6], res_img);
    DrawLine(vertex[2], vertex[6], res_img);
  }

  if (faces[4]) {
    DrawLine(vertex[4], vertex[5], res_img);
    DrawLine(vertex[4], vertex[6], res_img);
    DrawLine(vertex[5], vertex[7], res_img);
    DrawLine(vertex[6], vertex[7], res_img);
  }

  if (faces[2]) {
    DrawLine(vertex[2], vertex[3], res_img);
    DrawLine(vertex[2], vertex[6], res_img);
    DrawLine(vertex[6], vertex[7], res_img);
    DrawLine(vertex[3], vertex[7], res_img);
  }

  return res_img;
}

std::array<size_t, 2> MinMax(const ImgVol& img_vol) {
  size_t min = std::numeric_limits<size_t>::max();
  size_t max = std::numeric_limits<size_t>::min();

  for (size_t x = 0; x < img_vol.SizeX(); x++) {
      for (size_t y = 0; y < img_vol.SizeY(); y++) {
        for (size_t z = 0; z < img_vol.SizeZ(); z++) {
          float b = img_vol(x, y, z);
          if (b > max) {
            max = size_t(b);
          }

          if (b < min) {
            min = size_t(b);
          }
        }
      }
    }

  std::array<size_t, 2> arr = {min, max};
  return arr;
}

void NormalizeImage(ImgVol& img_vol) {
  int i_max = img_vol.Imax();
  i_max = std::pow(2, i_max - 1);

  if (i_max > 255) {
    std::array<size_t, 2> arr = MinMax(img_vol);

    for (size_t x = 0; x < img_vol.SizeX(); x++) {
      for (size_t y = 0; y < img_vol.SizeY(); y++) {
        for (size_t z = 0; z < img_vol.SizeZ(); z++) {
          float b = 255*img_vol(x, y, z)/arr[1];
          img_vol.SetVoxelIntensity(b, x, y, z);
        }
      }
    }
  }
}

}
