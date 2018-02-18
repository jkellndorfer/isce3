//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel
// Copyright 2018
//

#include <iostream>
#include <string>
#include <cmath>
#include <map>

#include "LinAlg.h"
#include "Attitude.h"

// Quaternion default constructor
isce::core::Quaternion::
Quaternion() : Attitude(QUATERNION_T), _qvec{0.0, 0.0, 0.0, 0.0} {}
// Quaternion constructor with vector
isce::core::Quaternion::
Quaternion(std::vector<double> & q) : Attitude(QUATERNION_T), _qvec(q) {}

// Return vector of Euler angles
std::vector<double>
isce::core::Quaternion::ypr() {

    // Get quaternion elements
    double q0 = _qvec[0];
    double q1 = _qvec[1];
    double q2 = _qvec[2];
    double q3 = _qvec[3];

    // Compute quaternion norm
    const double qmod = std::sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    // Normalize elements
    q0 /= qmod;
    q1 /= qmod;
    q2 /= qmod;
    q3 /= qmod;

    // Compute Euler angles
    const double r11 = 2.0 * (q1*q2 + q0*q3);
    const double r12 = q0*q0 + q1*q1 - q2*q2 - q3*q3;
    const double r21 = -2.0 * (q1*q3 - q0*q2);
    const double r31 = 2.0 * (q2*q3 + q0*q1);
    const double r32 = q0*q0 - q1*q1 - q2*q2 + q3*q3;
    const double yaw = std::atan2(r11, r12);
    const double pitch = std::asin(r21);
    const double roll = std::atan2(r31, r32);

    // Make vector and return
    std::vector<double> angles{yaw, pitch, roll};
    return angles;
    
}

// Convert quaternion to rotation matrix
std::vector<std::vector<double>>
isce::core::Quaternion::rotmat(const std::string dummy) {

    // Cache quaternion elements
    if (_qvec.size() != 4) {
        std::cerr << "ERROR: quaternion does not have the right size" << std::endl;
    }
    const double a = _qvec[0];
    const double b = _qvec[1];
    const double c = _qvec[2];
    const double d = _qvec[3];

    // Construct rotation matrix
    std::vector<std::vector<double>> R{{
        {a*a + b*b - c*c - d*d,
         2*b*c - 2*a*d,
         2*b*d + 2*a*c},
        {2*b*c + 2*a*d,
         a*a - b*b + c*c - d*d,
         2*c*d - 2*a*b},
        {2*b*d - 2*a*c,
         2*c*d + 2*a*b,
         a*a - b*b - c*c + d*d}
    }};

    return R;
}

// Extract YPR after factoring out orbit matrix
std::vector<double>
isce::core::Quaternion::factoredYPR(std::vector<double> & satxyz,
    std::vector<double> & satvel, Ellipsoid * ellipsoid) {

    // Compute ECI velocity assuming attitude angles are provided in inertial frame
    std::vector<double> Va(3);
    std::vector<double> w{0.0, 0.0, 0.00007292115833};
    LinAlg::cross(w, satxyz, Va);
    for (size_t i = 0; i < 3; ++i)
        Va[i] += satvel[i];

    // Compute vectors for TCN-like basis
    std::vector<double> q(3), c(3), b(3), a(3), temp(3);
    temp = {satxyz[0], satxyz[1], satxyz[2] / (1 - ellipsoid->e2)};
    LinAlg::unitVec(temp, q);
    c = {-q[0], -q[1], -q[2]};
    LinAlg::cross(c, Va, temp);
    LinAlg::unitVec(temp, b);
    LinAlg::cross(b, c, a);

    // Stack basis vectors to get transposed orbit matrix
    std::vector<std::vector<double>> L0(3, std::vector<double>(3, 0.0));
    for (size_t i = 0; i < 3; ++i) {
        L0[0][i] = a[i];
        L0[1][i] = b[i];
        L0[2][i] = c[i];
    }

    // Get total rotation matrix
    std::vector<std::vector<double>> R = rotmat("");

    // Multiply by transpose to get pure attitude matrix
    std::vector<std::vector<double>> L(3, std::vector<double>(3, 0.0));
    LinAlg::matMat(L0, R, L);

    // Extract Euler angles from rotation matrix
    std::vector<double> angles = EulerAngles::rotmat2ypr(L);
    return angles;
}

// end of file
