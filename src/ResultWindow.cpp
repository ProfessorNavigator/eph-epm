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
#include <AuxFunc.h>
#include <ResultWindow.h>
#include <fstream>

#include <glibmm-2.68/glibmm/miscutils.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/noselection.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <iostream>
#include <libintl.h>
#include <sstream>

#ifndef EPH_GTK_OLD
#include <gtkmm-4.0/gtkmm/error.h>
#endif

ResultWindow::ResultWindow(Gtk::Window *parent_window,
                           const std::shared_ptr<dataset> &result_data)
{
  this->parent_window = parent_window;
  this->result_data = result_data;
}

void
ResultWindow::createWindow()
{
  main_window = new Gtk::Window;
  main_window->set_application(parent_window->get_application());
  main_window->set_name("mainWindow");
  main_window->set_title(gettext("Result"));
  main_window->set_transient_for(*parent_window);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  grid->set_column_homogeneous(true);
  main_window->set_child(*grid);

  objlab = Gtk::make_managed<Gtk::Label>();
  objlab->set_halign(Gtk::Align::START);
  objlab->set_margin(5);
  objlab->set_text(Glib::ustring(gettext("Object: ") + result_data->bodyname));
  grid->attach(*objlab, 0, 0, 1, 1);

  coordlab = Gtk::make_managed<Gtk::Label>();
  coordlab->set_halign(Gtk::Align::START);
  coordlab->set_margin(5);
  coordlab->set_text(gettext("Coordinates: ") + result_data->coordinates);
  grid->attach(*coordlab, 0, 1, 1, 1);

  equinlab = Gtk::make_managed<Gtk::Label>();
  equinlab->set_halign(Gtk::Align::START);
  equinlab->set_margin(5);
  equinlab->set_text(gettext("Equator and equinox: ") + result_data->equinox);
  grid->attach(*equinlab, 0, 2, 1, 1);

  unitlab = Gtk::make_managed<Gtk::Label>();
  unitlab->set_halign(Gtk::Align::START);
  unitlab->set_margin(5);
  unitlab->set_text(gettext("Units of measurement: ") + result_data->units);
  grid->attach(*unitlab, 0, 3, 1, 1);

  beltlab = Gtk::make_managed<Gtk::Label>();
  beltlab->set_halign(Gtk::Align::START);
  beltlab->set_margin(5);
  beltlab->set_text(gettext("Hour belt: ") + result_data->hour_belt);
  grid->attach(*beltlab, 0, 4, 1, 1);

  Glib::RefPtr<Gtk::NoSelection> selection
      = Gtk::NoSelection::create(formModel());

  Glib::RefPtr<Gtk::ColumnViewColumn> col_X;
  Glib::RefPtr<Gtk::ColumnViewColumn> col_Y;
  Glib::RefPtr<Gtk::ColumnViewColumn> col_Z;
  header_line = gettext("Date and time(local)");
  if(result_data->objnum != 21)
    {
      switch(result_data->xyznum)
        {
        case 0:
          {
            col_X = Gtk::ColumnViewColumn::create("X", formFactoryX());
            col_Y = Gtk::ColumnViewColumn::create("Y", formFactoryY());
            col_Z = Gtk::ColumnViewColumn::create("Z", formFactoryZ());
            header_line = header_line + ";X;Y;Z;";
            break;
          }
        case 1:
          {
            col_X = Gtk::ColumnViewColumn::create("Vx", formFactoryX());
            col_Y = Gtk::ColumnViewColumn::create("Vy", formFactoryY());
            col_Z = Gtk::ColumnViewColumn::create("Vz", formFactoryZ());
            header_line = header_line + ";Vx;Vy;Vz;";
            break;
          }
        default:
          break;
        }
    }
  else
    {
      switch(result_data->xyznum)
        {
        case 0:
          {
            col_X = Gtk::ColumnViewColumn::create("φ", formFactoryX());
            col_Y = Gtk::ColumnViewColumn::create("θ", formFactoryY());
            col_Z = Gtk::ColumnViewColumn::create("ψ", formFactoryZ());
            header_line = header_line + ";φ;θ;ψ;";
            break;
          }
        case 1:
          {
            col_X = Gtk::ColumnViewColumn::create("φ'", formFactoryX());
            col_Y = Gtk::ColumnViewColumn::create("θ'", formFactoryY());
            col_Z = Gtk::ColumnViewColumn::create("ψ'", formFactoryZ());
            header_line = header_line + ";φ';θ';ψ';";
            break;
          }
        default:
          break;
        }
    }

  columnv = Gtk::make_managed<Gtk::ColumnView>();
  columnv->set_show_row_separators(true);
  columnv->set_show_column_separators(true);
  columnv->set_model(selection);
  columnv->append_column(formDateColum());
  columnv->append_column(col_X);
  columnv->append_column(col_Y);
  columnv->append_column(col_Z);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_margin(5);
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  scrl->set_child(*columnv);
  grid->attach(*scrl, 0, 5, 2, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_name("button");
  save->set_halign(Gtk::Align::CENTER);
  save->set_margin(5);
  save->set_label(gettext("Save"));
  save->signal_clicked().connect(std::bind(&ResultWindow::saveDialog, this));
  grid->attach(*save, 0, 6, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_name("closeButton");
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, main_window));
  grid->attach(*close, 1, 6, 1, 1);

  if(result_data->objnum == 21)
    {
      grid->remove(*coordlab);
      grid->remove(*equinlab);
    }

  columnv->signal_realize().connect([scrl, this] {
    Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
    Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
    Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
    Gdk::Rectangle req;
    mon->get_geometry(req);
    Gtk::Requisition min, nat;
    columnv->get_preferred_size(min, nat);
    int width = nat.get_width();
    int s_width = 0.5 * req.get_width();
    if(width <= s_width)
      {
        scrl->set_min_content_width(width);
      }
    else
      {
        scrl->set_min_content_width(s_width);
      }
    int height = nat.get_height();
    int s_height = 0.5 * req.get_height();
    if(height <= s_height)
      {
        scrl->set_min_content_height(height);
      }
    else
      {
        scrl->set_min_content_height(s_height);
      }
  });

  main_window->signal_close_request().connect(
      [this] {
        std::unique_ptr<Gtk::Window> win(main_window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);
  main_window->present();
}

void
ResultWindow::saveDialog()
{
#ifndef EPH_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fcd = Gtk::FileDialog::create();
  fcd->set_title(gettext("Save result"));
  fcd->set_modal(true);
  Glib::RefPtr<Gio::File> fl
      = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_initial_folder(fl);
  fcd->set_initial_name("result.csv");
  Glib::RefPtr<Gio::Cancellable> cancel = Gio::Cancellable::create();
  fcd->save(*main_window,
            std::bind(&ResultWindow::saveDialogFunc, this,
                      std::placeholders::_1, fcd),
            cancel);
#endif
#ifdef EPH_GTK_OLD
  Gtk::FileChooserDialog *fcd
      = new Gtk::FileChooserDialog(*main_window, gettext("Save result"),
                                   Gtk::FileChooser::Action::SAVE, true);
  fcd->set_application(main_window->get_application());
  fcd->set_modal(true);

  fcd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  fcd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);

  Glib::RefPtr<Gio::File> fl
      = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_current_folder(fl);
  fcd->set_current_name("result.csv");

  fcd->signal_response().connect(std::bind(&ResultWindow::saveDialogFunc, this,
                                           std::placeholders::_1, fcd));

  fcd->signal_close_request().connect(
      [fcd] {
        std::shared_ptr<Gtk::FileChooserDialog> fd(fcd);
        fd->set_visible(false);
        return true;
      },
      false);

  fcd->present();
#endif
}

#ifndef EPH_GTK_OLD
void
ResultWindow::saveDialogFunc(const Glib::RefPtr<Gio::AsyncResult> &result,
                             const Glib::RefPtr<Gtk::FileDialog> &fd)
#endif
#ifdef EPH_GTK_OLD
    void ResultWindow::saveDialogFunc(int respnum, Gtk::FileChooserDialog *fd)
#endif
{
  Glib::RefPtr<Gio::File> fl;
#ifndef EPH_GTK_OLD
  auto obj = result->get_source_object_base();
  auto fchd = std::dynamic_pointer_cast<Gtk::FileDialog>(obj);
  if(!fchd)
    {
      return void();
    }
  try
    {
      fl = fchd->save_finish(result);
    }
  catch(Glib::Error &e)
    {
      if(e.code() != Gtk::DialogError::DISMISSED)
        {
          std::cout << "MainWindow::saveDialog:" << e.what() << std::endl;
        }
    }
#endif

#ifdef EPH_GTK_OLD
  if(respnum == Gtk::ResponseType::ACCEPT)
    {
      fl = fd->get_file();
    }
#endif
  if(fl)
    {
      std::string filename = fl->get_path();
      std::filesystem::path filepath = std::filesystem::u8path(filename);
      filename = ".csv";
      filepath.replace_extension(std::filesystem::u8path(filename));
      std::fstream f;
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      if(!f.is_open())
        {
          std::cout << "Cannot open file for saving" << std::endl;
        }
      else
        {
          std::string line(objlab->get_text());
          line = line + "\n";
          f.write(line.c_str(), line.size());
          if(result_data->objnum != 21)
            {
              line = std::string(coordlab->get_text()) + ";\n";
              f.write(line.c_str(), line.size());
              line = std::string(equinlab->get_text()) + ";\n";
              f.write(line.c_str(), line.size());
            }
          line = std::string(unitlab->get_text()) + ";\n";
          f.write(line.c_str(), line.size());
          line = std::string(beltlab->get_text()) + ";\n\n";
          f.write(line.c_str(), line.size());
          line.clear();
          line = header_line;
          line = line + "\n";
          f.write(line.c_str(), line.size());

          Glib::RefPtr<Gtk::NoSelection> model
              = std::dynamic_pointer_cast<Gtk::NoSelection>(
                  columnv->get_model());
          if(model)
            {
              Glib::RefPtr<Gio::ListModel> mod = model->get_model();
              Glib::RefPtr<Gio::ListStore<ModelColumns>> store
                  = std::dynamic_pointer_cast<Gio::ListStore<ModelColumns>>(
                      mod);
              if(store)
                {
                  for(guint i = 0; i < mod->get_n_items(); i++)
                    {
                      Glib::RefPtr<ModelColumns> item = store->get_item(i);
                      line.clear();
                      line = std::string(item->date) + ";";
                      line = line + std::string(item->x) + ";";
                      line = line + std::string(item->y) + ";";
                      line = line + std::string(item->z) + ";";
                      line = line + "\n";
                      f.write(line.c_str(), line.size());
                    }
                }
            }
          f.close();
        }
    }
#ifdef EPH_GTK_OLD
  fd->close();
#endif
}

Glib::RefPtr<Gio::ListStore<ModelColumns>>
ResultWindow::formModel()
{
  Glib::RefPtr<Gio::ListStore<ModelColumns>> store
      = Gio::ListStore<ModelColumns>::create();
  std::stringstream strm;
  std::locale loc("C");
  AuxFunc af;
  for(size_t i = 0; i < result_data->result.size(); i++)
    {
      int ych, mch, dch, hch, minch;
      double secch;
      af.dateJulian(result_data->JDshow + i * result_data->stepnum, dch, mch,
                    ych, hch, minch, secch);
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << dch;
      std::string datestr;
      if(dch < 10)
        {
          datestr = "0" + strm.str();
        }
      else
        {
          datestr = strm.str();
        }

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << mch;
      if(mch < 10)
        {
          datestr = datestr + ".0" + strm.str();
        }
      else
        {
          datestr = datestr + "." + strm.str();
        }

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << ych;
      datestr = datestr + "." + strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << hch;
      if(hch < 10)
        {
          datestr = datestr + " 0" + strm.str();
        }
      else
        {
          datestr = datestr + " " + strm.str();
        }

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << minch;
      if(minch < 10)
        {
          datestr = datestr + ":0" + strm.str();
        }
      else
        {
          datestr = datestr + ":" + strm.str();
        }

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      if(secch > 59)
        {
          secch = 0;
        }
      strm << std::fixed << std::setprecision(3) << secch;
      if(secch < 10)
        {
          datestr = datestr + ":0" + strm.str();
        }
      else
        {
          datestr = datestr + ":" + strm.str();
        }

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20) << result_data->result[i].X;
      std::string X = strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20) << result_data->result[i].Y;
      std::string Y = strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20) << result_data->result[i].Z;
      std::string Z = strm.str();
      Glib::RefPtr<ModelColumns> row = ModelColumns::create(datestr, X, Y, Z);
      store->append(row);
    }

  return store;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
ResultWindow::formDateColum()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
    lab->set_margin(5);
    lab->set_halign(Gtk::Align::CENTER);
    lab->set_valign(Gtk::Align::CENTER);
    lab->set_text("");
    item->set_child(*lab);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Glib::RefPtr<ModelColumns> row
        = std::dynamic_pointer_cast<ModelColumns>(item->get_item());
    Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
    lab->set_text(row->date);
  });
  Glib::RefPtr<Gtk::ColumnViewColumn> column = Gtk::ColumnViewColumn::create(
      gettext("Date and time(local)"), factory);

  return column;
}

Glib::RefPtr<Gtk::SignalListItemFactory>
ResultWindow::formFactoryX()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
    lab->set_margin(5);
    lab->set_halign(Gtk::Align::CENTER);
    lab->set_valign(Gtk::Align::CENTER);
    lab->set_text("");
    item->set_child(*lab);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Glib::RefPtr<ModelColumns> row
        = std::dynamic_pointer_cast<ModelColumns>(item->get_item());
    Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
    lab->set_text(row->x);
  });

  return factory;
}

Glib::RefPtr<Gtk::SignalListItemFactory>
ResultWindow::formFactoryY()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
    lab->set_margin(5);
    lab->set_halign(Gtk::Align::CENTER);
    lab->set_valign(Gtk::Align::CENTER);
    lab->set_text("");
    item->set_child(*lab);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Glib::RefPtr<ModelColumns> row
        = std::dynamic_pointer_cast<ModelColumns>(item->get_item());
    Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
    lab->set_text(row->y);
  });

  return factory;
}

Glib::RefPtr<Gtk::SignalListItemFactory>
ResultWindow::formFactoryZ()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
    lab->set_margin(5);
    lab->set_halign(Gtk::Align::CENTER);
    lab->set_valign(Gtk::Align::CENTER);
    lab->set_text("");
    item->set_child(*lab);
  });

  factory->signal_bind().connect([](const Glib::RefPtr<Gtk::ListItem> &item) {
    Glib::RefPtr<ModelColumns> row
        = std::dynamic_pointer_cast<ModelColumns>(item->get_item());
    Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
    lab->set_text(row->z);
  });
  return factory;
}
