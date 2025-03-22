/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef RESULTWINDOW_H
#define RESULTWINDOW_H

#include <CoordKeeper.h>
#include <ModelColumns.h>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>
#include <gtkmm-4.0/gtkmm/window.h>

#ifndef EPH_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#ifdef EPH_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

struct dataset
{
  Glib::ustring bodyname;
  Glib::ustring coordinates;
  Glib::ustring equinox;
  Glib::ustring units;
  Glib::ustring hour_belt;
  double JDshow;
  double stepnum;
  guint objnum;
  guint xyznum;
  std::vector<CoordKeeper> result;
};

class ResultWindow
{
public:
  ResultWindow(Gtk::Window *parent_window,
               const std::shared_ptr<dataset> &result_data);

  void
  createWindow();

private:
  void
  saveDialog();

#ifndef EPH_GTK_OLD
  void
  saveDialogFunc(const Glib::RefPtr<Gio::AsyncResult> &result,
                 const Glib::RefPtr<Gtk::FileDialog> &fd);
#endif
#ifdef EPH_GTK_OLD
  void
  saveDialogFunc(int respnum, Gtk::FileChooserDialog *fd);
#endif

  Glib::RefPtr<Gio::ListStore<ModelColumns>>
  formModel();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  formDateColum();

  Glib::RefPtr<Gtk::SignalListItemFactory>
  formFactoryX();

  Glib::RefPtr<Gtk::SignalListItemFactory>
  formFactoryY();

  Glib::RefPtr<Gtk::SignalListItemFactory>
  formFactoryZ();

  Gtk::Window *parent_window;
  std::shared_ptr<dataset> result_data;

  Gtk::Window *main_window;
  Gtk::Label *objlab;
  Gtk::Label *coordlab;
  Gtk::Label *equinlab;
  Gtk::Label *unitlab;
  Gtk::Label *beltlab;
  Gtk::ColumnView *columnv;
  std::string header_line;
};

#endif // RESULTWINDOW_H
