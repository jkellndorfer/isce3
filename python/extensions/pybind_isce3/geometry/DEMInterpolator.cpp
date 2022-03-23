#include "DEMInterpolator.h"

#include <memory>
#include <pybind11/eigen.h>
#include <stdexcept>
#include <string>

#include <Eigen/Dense>

#include <isce3/core/Constants.h>
#include <isce3/io/Raster.h>

namespace py = pybind11;

using DEMInterp = isce3::geometry::DEMInterpolator;

void addbinding(pybind11::class_<DEMInterp>& pyDEMInterpolator)
{
    pyDEMInterpolator
            .def(py::init<double, isce3::core::dataInterpMethod, int>(),
                    py::arg("height") = 0.0,
                    py::arg("method") = isce3::core::BILINEAR_METHOD,
                    py::arg("epsg") = 4326)
            // For convenience allow a string, too.
            .def(py::init([](double h, const std::string& method, int epsg) {
                auto m = parseDataInterpMethod(method);
                return DEMInterp(h, m, epsg);
            }),
                    py::arg("height") = 0.0, py::arg("method") = "bilinear",
                    py::arg("epsg") = 4326)

            // This constructor is similar to method "loadDEM" but is more
            // convenient!
            .def(py::init([](isce3::io::Raster& raster_obj) {
                DEMInterp dem {};
                dem.loadDEM(raster_obj);
                return dem;
            }),
                    "Construct DEM from ISCE3 Raster object",
                    py::arg("raster_obj"))

            .def("load_dem",
                    py::overload_cast<isce3::io::Raster&>(&DEMInterp::loadDEM))
            .def("load_dem",
                    py::overload_cast<isce3::io::Raster&, double, double,
                            double, double>(&DEMInterp::loadDEM),
                    py::arg("raster"), py::arg("min_x"), py::arg("max_x"),
                    py::arg("min_y"), py::arg("max_y"))

            .def("interpolate_lonlat", &DEMInterp::interpolateLonLat)
            .def("interpolate_xy", &DEMInterp::interpolateXY)

            .def_property("ref_height",
                    py::overload_cast<>(&DEMInterp::refHeight, py::const_),
                    py::overload_cast<double>(&DEMInterp::refHeight))
            .def_property_readonly("have_raster", &DEMInterp::haveRaster)
            .def_property("interp_method",
                    py::overload_cast<>(&DEMInterp::interpMethod, py::const_),
                    py::overload_cast<isce3::core::dataInterpMethod>(
                            &DEMInterp::interpMethod))

            // Define all these as readonly even though writable in C++ API.
            // Probably better to just convert your data to a GDAL format than
            // try to build a DEM on the fly.
            .def_property_readonly(
                    "data",
                    [](DEMInterp& self) { // .data() isn't const
                        if (!self.haveRaster()) {
                            throw std::out_of_range(
                                    "Tried to access DEM data but size=0");
                        }
                        using namespace Eigen;
                        using MatF = Eigen::Matrix<float, Dynamic, Dynamic,
                                RowMajor>;
                        Map<const MatF> mat(
                                self.data(), self.length(), self.width());
                        return mat;
                    },
                    py::return_value_policy::reference_internal)
            .def_property_readonly("x_start",
                    py::overload_cast<>(&DEMInterp::xStart, py::const_))
            .def_property_readonly("y_start",
                    py::overload_cast<>(&DEMInterp::yStart, py::const_))
            .def_property_readonly("delta_x",
                    py::overload_cast<>(&DEMInterp::deltaX, py::const_))
            .def_property_readonly("delta_y",
                    py::overload_cast<>(&DEMInterp::deltaY, py::const_))
            .def_property_readonly(
                    "width", py::overload_cast<>(&DEMInterp::width, py::const_))
            .def_property_readonly("length",
                    py::overload_cast<>(&DEMInterp::length, py::const_))
            .def_property_readonly("epsg_code",
                    py::overload_cast<>(&DEMInterp::epsgCode, py::const_));
}
