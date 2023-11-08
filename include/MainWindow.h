/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_MAINWINDOW_H_
#define INCLUDE_MAINWINDOW_H_

#include <fstream>
#include <functional>
#include <gmpxx.h>
#include <gtkmm.h>
#include <iostream>
#include <libintl.h>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "AuxFunc.h"
#include "Coordinates.h"
#include "DAFOperations.h"
#include "EPMCalculations.h"
#include "ModelColumns.h"
#include "OrbitsDiagram.h"
#include "BodyListItem.h"

class MainWindow : public Gtk::ApplicationWindow
{
public:
  MainWindow();
  virtual
  ~MainWindow();

private:
  void
  createWindow();

  Glib::RefPtr<Gio::ListStore<BodyListItem>>
  createBodyList();

  void
  calcCoord(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
	    Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
	    Gtk::DropDown *timecomb, Gtk::DropDown *belt,
	    Gtk::DropDown *objcomb, Gtk::DropDown *coordcomb,
	    Gtk::DropDown *xyzcomb, Gtk::DropDown *equincomb,
	    Gtk::DropDown *unitcomb, Gtk::Entry *stepent,
	    Gtk::Entry *stepnument, Gtk::Entry *pathent, Gtk::Entry *tttdbent,
	    Gtk::Entry *mlbent, Gtk::Entry *smlent);

  void
  aboutProg();

  void
  errDialog(int variant);

  void
  resultPresenting(std::vector<std::array<mpf_class, 3> > *result,
		   Gtk::DropDown *belt, Gtk::DropDown *objcomb,
		   Gtk::DropDown *coordcomb, Gtk::DropDown *xyzcomb,
		   Gtk::DropDown *equincomb, Gtk::DropDown *unitcomb,
		   Glib::Dispatcher *result_win_disp);

  Gtk::Window*
  resultPulseWin(int variant, Gtk::ProgressBar *bar);

  void
  openDialog(Gtk::Entry *pathent);

  void
  saveDialog(Gtk::Window *win, Gtk::Label *objlab, Gtk::Label *coordlab,
	     Gtk::Label *equinlab, Gtk::Label *unitlab, Gtk::Label *beltlab,
	     Gtk::ColumnView *view, Gtk::DropDown *objcomb,
	     std::string header_line);

  void
  saveDialogFunc(Glib::RefPtr<Gio::File> fl, Gtk::Label *objlab,
		 Gtk::Label *coordlab, Gtk::Label *equinlab,
		 Gtk::Label *unitlab, Gtk::Label *beltlab,
		 Gtk::ColumnView *view, Gtk::DropDown *objcomb,
		 std::string header_line);

  void
  orbitsGraph(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
	      Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
	      Gtk::DropDown *timecomb, Gtk::DropDown *belt,
	      Gtk::DropDown *coordcomb, Gtk::DropDown *equincomb,
	      Gtk::Entry *pathent, Gtk::Entry *tttdbent, Gtk::Entry *smlent,
	      Gtk::Entry *scale_ent);

  bool
  closeFunc(Gtk::Entry *pathent, Gtk::Entry *tttdbent, Gtk::Entry *mlbent,
	    Gtk::Entry *smlent, Gtk::Entry *scale_ent);

  std::string Sharepath;

  int orbits_cancel = 0;
  double JDshow = 0.0;
  double stepnum = -1;
};

#endif /* INCLUDE_MAINWINDOW_H_ */
