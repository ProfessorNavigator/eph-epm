/*
 * Copyright (C) 2022-2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_ORBITSDIAGRAM_H_
#define INCLUDE_ORBITSDIAGRAM_H_

#include <DAFOperations.h>
#include <DiagramWidget.h>
#include <array>
#include <atomic>
#ifndef USE_OPENMP
#include <condition_variable>
#endif
#include <functional>
#include <gmpxx.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gtkmm/applicationwindow.h>
#include <mgl2/mgl.h>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

class OrbitsDiagram
{
public:
  OrbitsDiagram(Gtk::ApplicationWindow *mw, std::string ephpath,
                std::string tttdbpath, std::string smlpath, double JD,
                int timesc, int coordtype, int theory, double plot_factor,
                std::atomic<int> *cancel);

  virtual ~OrbitsDiagram();

  int
  calculateSize();

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
  planetOrbCalc(std::tuple<int, double> planettup);

  void
  bodyBuilding(int body, mglGraph *graph);

  Gdk::Rectangle
  screenRes();

  Gtk::ApplicationWindow *mw = nullptr;
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
  std::mutex *grmtx = nullptr;

#ifndef USE_OPENMP
  std::mutex cyclemtx;
  std::condition_variable thread_reg;
  unsigned int thrnum = 0;
#endif

  double scale_factor = 0.001;
  std::atomic<int> *cancel = nullptr;
  int Width = 0;
  int Height = 0;
  double plot_factor = 0.000000001;
  int coordtype = 0;
  int theory = 0;
  std::vector<std::array<mpf_class, 3>> resultsed;
  bool EPM = false;
};

#endif /* INCLUDE_ORBITSDIAGRAM_H_ */
