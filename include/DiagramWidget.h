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

#ifndef INCLUDE_DIAGRAMWIDGET_H_
#define INCLUDE_DIAGRAMWIDGET_H_

#include <cairomm-1.16/cairomm/context.h>
#include <cairomm-1.16/cairomm/refptr.h>
#include <functional>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gtkmm/applicationwindow.h>
#include <gtkmm-4.0/gtkmm/drawingarea.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <mgl2/mgl.h>
#include <string>
#include <tuple>
#include <vector>

class DiagramWidget
{
public:
  DiagramWidget(Gtk::ApplicationWindow *mw, mglGraph *gr);

  void
  diagramPlot();

  std::function<void()> diagram_close;

private:
  void
  planetOrbCalc(std::tuple<std::string, double> planettup);

  void
  on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

  bool
  scrollEvent(double x, double y, Gtk::Entry *entx, Gtk::Entry *enty,
              Gtk::Entry *entz, Gtk::DrawingArea *drar);

  void
  dragOperation(double x, double y, Gtk::Entry *entx, Gtk::Entry *enty,
                Gtk::Entry *entz, Gtk::DrawingArea *drar);

  void
  zoomGraph(Gtk::Entry *entx, Gtk::Entry *enty, Gtk::Entry *entz,
            Gtk::DrawingArea *drar, int id);

  void
  saveGraph(mglGraph *gr, Gtk::Window *win, int mode);

  void
  rotationFunc(Gtk::Entry *entx, Gtk::Entry *enty, Gtk::Entry *entz,
               Gtk::DrawingArea *drar);

  Gdk::Rectangle
  screenRes();

  Gtk::ApplicationWindow *mw = nullptr;
  mglGraph *gr = nullptr;
  std::vector<double> plotincr;
  int Height, Width;
  int X = 0;
  int Y = 0;
  std::string Sharepath;
  double scale_val1 = -1143583.890685;
  double scale_val2 = 1143584.890685;
};

#endif /* INCLUDE_DIAGRAMWIDGET_H_ */
