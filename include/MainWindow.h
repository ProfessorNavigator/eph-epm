/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

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

#ifndef INCLUDE_MAINWINDOW_H_
#define INCLUDE_MAINWINDOW_H_

#include <gtkmm.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <libintl.h>
#include <gmpxx.h>
#include <thread>
#include <tuple>
#include <functional>
#include <mutex>

#include "AuxFunc.h"
#include "EPMCalculations.h"
#include "Coordinates.h"
#include "DAFOperations.h"
#include "OrbitsDiagram.h"

class MainWindow : public Gtk::ApplicationWindow
{
public:
  MainWindow();
  virtual
  ~MainWindow();
private:
  void
  createWindow();
  void
  calcCoord(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
	    Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
	    Gtk::ComboBoxText *timecomb, Gtk::ComboBoxText *belt,
	    Gtk::ComboBoxText *objcomb, Gtk::ComboBoxText *coordcomb,
	    Gtk::ComboBoxText *xyzcomb, Gtk::ComboBoxText *equincomb,
	    Gtk::ComboBoxText *unitcomb, Gtk::Entry *stepent,
	    Gtk::Entry *stepnument, Gtk::Entry *pathent);
  void
  aboutProg();
  void
  errDialog(int variant);
  void
  resultPresenting(std::vector<std::array<mpf_class, 3>> *result,
		   Gtk::ComboBoxText *belt, Gtk::ComboBoxText *objcomb,
		   Gtk::ComboBoxText *coordcomb, Gtk::ComboBoxText *xyzcomb,
		   Gtk::ComboBoxText *equincomb, Gtk::ComboBoxText *unitcomb,
		   Glib::Dispatcher *result_win_disp);
  Gtk::Window*
  resultPulseWin(int variant, Gtk::ProgressBar *bar);
  void
  openDialog(Gtk::Entry *pathent);
  void
  openDialogFunc(int rid, Gtk::FileChooserDialog *fcd, Gtk::Entry *pathent);
  void
  saveDialog(Gtk::Window *win, Gtk::Label *objlab, Gtk::Label *coordlab,
	     Gtk::Label *equinlab, Gtk::Label *unitlab, Gtk::Label *beltlab,
	     Gtk::TreeView *view, Gtk::ComboBoxText *objcomb);
  void
  saveDialogFunc(int rid, Gtk::FileChooserDialog *fcd, Gtk::Label *objlab,
		 Gtk::Label *coordlab, Gtk::Label *equinlab,
		 Gtk::Label *unitlab, Gtk::Label *beltlab, Gtk::TreeView *view,
		 Gtk::ComboBoxText *objcomb);
  void
  orbitsGraph(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
	      Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
	      Gtk::ComboBoxText *timecomb, Gtk::ComboBoxText *belt,
	      Gtk::ComboBoxText *coordcomb, Gtk::ComboBoxText *equincomb,
	      Gtk::Entry *pathent);

  std::string Sharepath = "";

  int orbits_cancel = 0;
  double JDshow = 0.0;
  double stepnum = -1;
};

#endif /* INCLUDE_MAINWINDOW_H_ */

