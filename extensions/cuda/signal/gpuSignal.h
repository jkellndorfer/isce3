// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Source Author: Liang Yu
// Copyright 2019

#ifndef ISCE_CUDA_SIGNAL_SIGNAL_H
#define ISCE_CUDA_SIGNAL_SIGNAL_H

#ifdef __CUDACC__
#define CUDA_HOSTDEV __host__ __device__
#define CUDA_DEV __device__
#define CUDA_HOST __host__
#define CUDA_GLOBAL __global__
#else
#define CUDA_HOSTDEV
#define CUDA_DEV
#define CUDA_HOST
#define CUDA_GLOBAL
#endif

#include <complex>
#include <valarray>

#include <cufft.h>
#include "isce/cuda/core/gpuComplex.h"

using isce::cuda::core::gpuComplex;

// Declaration
namespace isce {
    namespace cuda {
        namespace signal {
            template<class T>
            class gpuSignal;
        }
    }
}

// Definition
template<class T>
class isce::cuda::signal::gpuSignal {

    public:
        // Default constructor
        gpuSignal() {};
        gpuSignal(cufftType _type);
        ~gpuSignal();

        /** \brief initiate plan for FFT in range direction 
         * for a block of complex data.
         * azimuth direction is assumed to be in the direction of the 
         * columns of the array.
         */
        void azimuthFFT(int ncolumns, int nrows);

        /** \brief initiate plan for FFT in azimuth direction 
         * for a block of complex data.
         * range direction is assumed to be in the direction of the 
         * columns of the array.
         */
        void rangeFFT(int ncolumns, int nrows);

        /** \brief initiate plan for FFT in azimuth direction 
         * for a block of complex data.
         * range direction is assumed to be in the direction of the 
         * columns of the array.
         */
        void FFT2D(int ncolumns, int nrows);

        /** \brief initiate cuFFT plan for a block of complex data
         *  input parameters cuFFT interface for fftw_plan_many_dft
         */
        void fftPlan(int rank, int* n, int howmany,                   
                    int* inembed, int istride, int idist,
                    int* onembed, int ostride, int odist);

        /** \brief next power of two*/
        void nextPowerOfTwo(size_t N, size_t& fftLength);

        /** \brief determine the required parameters for setting range FFT plans */
        inline void _configureRangeFFT(int ncolumns, int nrows);

        /** \brief determine the required parameters for setting azimuth FFT plans */
        inline void _configureAzimuthFFT(int ncolumns, int nrows);

        /** moving data in between device and host */
        void dataToDevice(std::complex<T> *input);
        void dataToDevice(std::valarray<std::complex<T>> &input);
        void dataToHost(std::complex<T> *output);
        void dataToHost(std::valarray<std::complex<T>> &output);
        void zeroDeviceData();

        /** forward transforms without intermediate return */
        void forwardC2C();
        void forwardZ2Z();
        void forward();

        /** forward transforms */
        void forwardC2C(std::complex<T> *input, std::complex<T> *output);
        void forwardC2C(std::valarray<std::complex<T>> &input,
                        std::valarray<std::complex<T>> &output);
        void forwardZ2Z(std::complex<T> *input, std::complex<T> *output);
        void forwardZ2Z(std::valarray<std::complex<T>> &input,
                        std::valarray<std::complex<T>> &output);
        void forwardD2Z(T *input, std::complex<T> *output);

        void forward(std::complex<T> *input, std::complex<T> *output);
        void forward(std::valarray<std::complex<T>> &input,
                     std::valarray<std::complex<T>> &output);

        void forwardDevMem(T *input, T *output);
        void forwardDevMem(T *dataInPlace);

        /** inverse transforms using existing device memory **/
        void inverseC2C();
        void inverseZ2Z();
        void inverse();

        /** inverse transforms */
        void inverseC2C(std::complex<T> *input, std::complex<T> *output);
        void inverseC2C(std::valarray<std::complex<T>> &input,
                        std::valarray<std::complex<T>> &output);
        void inverseZ2Z(std::complex<T> *input, std::complex<T> *output);
        void inverseZ2Z(std::valarray<std::complex<T>> &input,
                        std::valarray<std::complex<T>> &output);
        void inverseZ2D(std::complex<T> *input, T *output);

        void inverse(std::complex<T> *input, std::complex<T> *output);
        void inverse(std::valarray<std::complex<T>> &input,
                     std::valarray<std::complex<T>> &output);

        void inverseDevMem(T *input, T *output);
        void inverseDevMem(T *dataInPlace);

        /** upsample **/
        void upsample(std::valarray<std::complex<T>> &input,
                      std::valarray<std::complex<T>> &output,
                      int row, int ncols, int upsampleFactor);
        void upsample(std::valarray<std::complex<T>> &input,
                      std::valarray<std::complex<T>> &output,
                      int row, int ncols, int upsampleFactor,
                      std::valarray<std::complex<T>> &shiftImpact);

        int getRows() {return _rows;};
        int getColumns() {return _columns;};
        int getNumElements() {return _n_elements;};

        T* getDevicePtr() {return _d_data;};

    private:
        cufftHandle _plan;
        bool _plan_set;
        cufftHandle _upsamp_plan;
        bool _upsamp_plan_set;
        cufftType _cufft_type;

        // FFT plan parameters
        int _rank;
        int* _n;
        int _howmany;
        int* _inembed;
        int _istride;
        int _idist;
        int* _onembed;
        int _ostride;
        int _odist;
        int _n_elements;
        int _rows;
        int _columns;

        // device memory pointers
        T *_d_data;
        bool _d_data_set;
};

template<class T>
void shift(std::valarray<std::complex<T>> &input,
        std::valarray<std::complex<T>> &output,
        int rows, int nfft, int columns);

/** FFT shift on device
 */
template<class T>
CUDA_GLOBAL void rangeShift_g(gpuComplex<T> *data_lo_res, gpuComplex<T> *data_hi_res, 
        int n_rows, int n_cols_lo, int n_cols_hi);

/** FFT shift on device
 */
template<class T>
CUDA_GLOBAL void rangeShiftImpactMult_g(gpuComplex<T> *data_lo_res, gpuComplex<T> *data_hi_res, 
        gpuComplex<T> *impact_shift, 
        int n_rows, int n_cols_lo, int n_cols_hi);

template<class T>
void upsample(isce::cuda::signal::gpuSignal<T> &fwd,
        isce::cuda::signal::gpuSignal<T> &inv,
        T *input,
        T *output);

template<class T>
void upsample(isce::cuda::signal::gpuSignal<T> &fwd,
        isce::cuda::signal::gpuSignal<T> &inv,
        T *input,
        T *output,
        T *shiftImpact);

void upsampleC2C(isce::cuda::signal::gpuSignal<float> &fwd,
        isce::cuda::signal::gpuSignal<float> &inv,
        std::valarray<std::complex<float>> &shiftImpact);

void upsampleC2C(isce::cuda::signal::gpuSignal<float> &fwd,
        isce::cuda::signal::gpuSignal<float> &inv,
        std::valarray<std::complex<float>> &input,
        std::valarray<std::complex<float>> &output,
        std::valarray<std::complex<float>> &shiftImpact);

void upsampleC2C(isce::cuda::signal::gpuSignal<float> &fwd,
        isce::cuda::signal::gpuSignal<float> &inv,
        float *input,
        float *output,
        float *shiftImpact);

void upsampleZ2Z(isce::cuda::signal::gpuSignal<double> &fwd,
        isce::cuda::signal::gpuSignal<double> &inv,
        std::valarray<std::complex<double>> &shiftImpact);

void upsampleZ2Z(isce::cuda::signal::gpuSignal<double> &fwd,
        isce::cuda::signal::gpuSignal<double> &inv,
        std::valarray<std::complex<double>> &input,
        std::valarray<std::complex<double>> &output,
        std::valarray<std::complex<double>> &shiftImpact);

void upsampleZ2Z(isce::cuda::signal::gpuSignal<double> &fwd,
        isce::cuda::signal::gpuSignal<double> &inv,
        double *input,
        double *output,
        double *shiftImpact);

#endif

// end of file
