/*
 * Copyright (C) 2022-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ORBITSDIAGRAM_H
#define ORBITSDIAGRAM_H

#include <CoordKeeper.h>
#include <Coordinates.h>
#include <DAFOperations.h>
#include <DiagramWidget.h>
#include <functional>
#include <gmpxx.h>
#include <mgl2/mgl.h>
#include <omp.h>
#include <string>
#include <tuple>
#include <vector>

class OrbitsDiagram
{
public:
  OrbitsDiagram(Gtk::Window *mw, const std::string &ephpath,
                const std::string &tttdbpath, const std::string &smlpath,
                const double &JD, const int &timesc, const int &coordtype,
                const int &theory, const double &plot_factor);

  virtual ~OrbitsDiagram();

  int
  calculateSize();

  void
  stopAll();

  std::function<void()> pulse_signal;

  std::function<void()> calc_completed;

  std::function<void()> canceled_signal;

  std::function<void()> diagram_close;

  void
  calculateOrbits();

  void
  diagramPlot();

private:
  void
  planetOrbCalc(const std::tuple<int, double> &planettup);

  void
  bodyBuilding(const int &body);

  Gdk::Rectangle
  screenRes();

  Gtk::Window *mw = nullptr;
  DiagramWidget *dw = nullptr;
  std::string ephpath;
  std::string tttdbpath;
  std::string smlpath;
  int modbody = -1;
  std::vector<std::tuple<int, double>> bodyv;
  DAFOperations *daf = nullptr;
  double JD = 0.0;
  int timesc = 0;
  double epb = 0.0;
  double epe = 0.0;
  mglGraph *gr = nullptr;

  double scale_factor = 0.001;
  bool cancel = false;
  int Width = 0;
  int Height = 0;
  double plot_factor = 0.000000001;
  int coordtype = 0;
  int theory = 0;
  std::vector<CoordKeeper> resultsed;

  std::vector<Coordinates *> coord_ptr_v;
  omp_lock_t coord_ptr_v_mtx;

  int active_lvls;
};

#endif // ORBITSDIAGRAM_H
