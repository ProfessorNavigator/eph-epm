/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of EphEPM.
 EphEPM is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 EphEPM is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with EphEPM. If not,
 see <https://www.gnu.org/licenses/>.
 */

#ifndef ORBITSDIAGRAM_H
#define ORBITSDIAGRAM_H

#include <gtkmm.h>
#include <vector>
#include <algorithm>
#include <tuple>
#include <filesystem>
#include <string>
#include <cmath>
#include <thread>
#include <functional>
#include <mutex>
#include <mgl2/mgl.h>
#include <gmpxx.h>

#include "DAFOperations.h"
#include "Coordinates.h"
#include "DiagramWidget.h"

class OrbitsDiagram
{
  public:
    OrbitsDiagram(Gtk::ApplicationWindow *mw, std::string ephpath, double JD,
                  int coordtype, int theory, int *cancel);
    virtual ~OrbitsDiagram();
    int
    calculateSize();
    std::function<void()>pulse_signal;
    std::function<void()>calc_completed;
    std::function<void()>canceled_signal;
    std::function<void()>diagram_close;
    void
    calculateOrbits();
    void
    diagramPlot();
  private:
    void
    planetOrbCalc(std::tuple<std::string, double> planettup);
    void
    bodyBuilding(std::string body, mglGraph *graph);
    Gdk::Rectangle
    screenRes();

    Gtk::ApplicationWindow *mw = nullptr;
    DiagramWidget *dw = nullptr;
    std::string ephpath;
    std::vector<std::tuple<std::string, double>> bodyv;
    DAFOperations *daf = nullptr;
    double JD = 0.0;
    double epb = 0.0;
    double epe = 0.0;
    mglGraph *gr = nullptr;
    std::mutex *grmtx = nullptr;
    std::vector<std::string> threadv;
    std::mutex *threadvmtx = nullptr;
    std::mutex cyclemtx;
    double scale_factor = 0.001;
    int *cancel = nullptr;
    int Width = 0;
    int Height = 0;
    double plot_factor = 0.000000001;
    int coordtype = 0;
    int theory = 0;
    std::vector<std::array<mpf_class, 3>> resultsed;
};

#endif // ORBITSDIAGRAM_H

