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

#ifndef INCLUDE_MAINWINDOW_H_
#define INCLUDE_MAINWINDOW_H_

#include <BodyListItem.h>
#include <giomm-2.68/giomm/file.h>
#include <giomm-2.68/giomm/liststore.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gmpxx.h>
#include <gtkmm-4.0/gtkmm/applicationwindow.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/progressbar.h>
#include <array>
#include <atomic>
#include <string>
#include <vector>

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

  std::atomic<int> orbits_cancel;
  double JDshow = 0.0;
  double stepnum = -1;
};

#endif /* INCLUDE_MAINWINDOW_H_ */
