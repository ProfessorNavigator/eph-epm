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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <BodyListItem.h>
#include <CoordKeeper.h>
#include <atomic>
#include <filesystem>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gtkmm/applicationwindow.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/progressbar.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>

class MainWindow : public Gtk::ApplicationWindow
{
public:
  MainWindow();

private:
  void
  createWindow();

  void
  objcombChangeFunc(Gtk::Label *coord, Gtk::Label *equin);

  void
  xyzCombChancgeFunc();

  Glib::RefPtr<Gio::ListStore<BodyListItem>>
  createBodyList();

  Glib::RefPtr<Gtk::SignalListItemFactory>
  createBodyFactory();

  std::vector<std::tuple<uint8_t, std::string>>
  formPathV();

  void
  calcCoord();

  void
  aboutProg();

  void
  errDialog(const int &variant);

  void
  resultPresenting(std::vector<CoordKeeper> *result);

  Gtk::Window *
  resultPulseWin(const int &variant, Gtk::ProgressBar *bar);

  void
  openDialog(Gtk::Entry *entry);

  void
  orbitsGraph();

  bool
  closeFunc();

  std::filesystem::path Sharepath;

  std::atomic<int> orbits_cancel;
  double JDshow = 0.0;
  double stepnum = -1;

  Gtk::Entry *day;
  Gtk::Entry *month;
  Gtk::Entry *year;
  Gtk::Entry *hour;
  Gtk::Entry *minut;
  Gtk::Entry *second;
  Gtk::DropDown *timecomb;
  Gtk::DropDown *belt;
  Gtk::DropDown *objcomb;
  Gtk::DropDown *coordcomb;
  Gtk::DropDown *xyzcomb;
  Gtk::DropDown *equincomb;
  Gtk::DropDown *unitcomb;
  Gtk::Entry *stepent;
  Gtk::Entry *stepnument;
  Gtk::Entry *pathent;
  Gtk::Entry *tttdbent;
  Gtk::Entry *mlbent;
  Gtk::Entry *smlent;
  Gtk::Entry *scale_ent;
};

#endif // MAINWINDOW_H
