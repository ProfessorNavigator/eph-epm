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

#include <AuxFunc.h>
#include <Coordinates.h>
#include <DAFOperations.h>
#include <MainWindow.h>
#include <ResultWindow.h>
#include <fstream>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <glibmm-2.68/glibmm/main.h>
#include <glibmm-2.68/glibmm/miscutils.h>
#include <gtkmm-4.0/gtkmm/aboutdialog.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/cssprovider.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/settings.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <iostream>
#include <libintl.h>
#include <omp.h>

#ifndef EPH_GTK_OLD
#include <gtkmm-4.0/gtkmm/error.h>
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#ifdef EPH_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif
#ifndef EPH_OMP_TASK
#include <thread>
#endif

MainWindow::MainWindow()
{
  AuxFunc af;
  std::filesystem::path p = af.get_selfpath();
  Sharepath = p.parent_path() / std::filesystem::u8path("../share/EphEPM");
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  p = Sharepath / std::filesystem::u8path("mainWindow.css");
  std::string styles;
  std::fstream f;
  f.open(p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string cont;
      cont.resize(std::filesystem::file_size(p));
      f.read(cont.data(), cont.size());
      f.close();
      styles = styles + cont;
    }
  p = Sharepath / std::filesystem::u8path("graphicWidg.css");
  f.open(p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string cont;
      cont.resize(std::filesystem::file_size(p));
      f.read(cont.data(), cont.size());
      f.close();
      styles = styles + cont;
    }
  css_provider->load_from_data(styles);
  Glib::RefPtr<Gdk::Display> disp = this->get_display();
  Glib::RefPtr<Gtk::Settings> settings = Gtk::Settings::get_for_display(disp);
  settings->property_gtk_theme_name().set_value("Adwaita");
  Gtk::StyleContext::add_provider_for_display(
      disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  createWindow();
}

void
MainWindow::createWindow()
{
  this->set_title("EphEPM");
  this->set_name("mainWindow");

  Gtk::Box *v_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  v_box->set_halign(Gtk::Align::FILL);
  v_box->set_valign(Gtk::Align::FILL);
  this->set_child(*v_box);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + gettext("Date and time input")
                  + "</b>");
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  v_box->append(*lab);

  Gtk::Grid *extra_grid = Gtk::make_managed<Gtk::Grid>();
  extra_grid->set_halign(Gtk::Align::CENTER);
  extra_grid->set_valign(Gtk::Align::FILL);
  v_box->append(*extra_grid);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Day"));
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  extra_grid->attach(*lab, 0, 0, 1, 1);

  day = Gtk::make_managed<Gtk::Entry>();
  day->set_halign(Gtk::Align::CENTER);
  day->set_margin(5);
  day->set_max_length(2);
  day->set_max_width_chars(2);
  day->set_alignment(0.5);
  day->set_input_purpose(Gtk::InputPurpose::DIGITS);
  extra_grid->attach(*day, 0, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Month"));
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  extra_grid->attach(*lab, 1, 0, 1, 1);

  month = Gtk::make_managed<Gtk::Entry>();
  month->set_halign(Gtk::Align::CENTER);
  month->set_margin(5);
  month->set_max_length(2);
  month->set_max_width_chars(2);
  month->set_alignment(0.5);
  month->set_input_purpose(Gtk::InputPurpose::DIGITS);
  extra_grid->attach(*month, 1, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Year"));
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  extra_grid->attach(*lab, 2, 0, 1, 1);

  year = Gtk::make_managed<Gtk::Entry>();
  year->set_halign(Gtk::Align::CENTER);
  year->set_margin(5);
  year->set_max_length(6);
  year->set_max_width_chars(6);
  year->set_alignment(0.5);
  year->set_input_purpose(Gtk::InputPurpose::DIGITS);
  extra_grid->attach(*year, 2, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Hours"));
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  extra_grid->attach(*lab, 3, 0, 1, 1);

  hour = Gtk::make_managed<Gtk::Entry>();
  hour->set_halign(Gtk::Align::CENTER);
  hour->set_margin(5);
  hour->set_max_length(2);
  hour->set_max_width_chars(2);
  hour->set_alignment(0.5);
  hour->set_input_purpose(Gtk::InputPurpose::DIGITS);
  extra_grid->attach(*hour, 3, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Minutes"));
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  extra_grid->attach(*lab, 4, 0, 1, 1);

  minut = Gtk::make_managed<Gtk::Entry>();
  minut->set_halign(Gtk::Align::CENTER);
  minut->set_margin(5);
  minut->set_max_length(2);
  minut->set_max_width_chars(2);
  minut->set_alignment(0.5);
  minut->set_input_purpose(Gtk::InputPurpose::DIGITS);
  extra_grid->attach(*minut, 4, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Seconds"));
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  extra_grid->attach(*lab, 5, 0, 1, 1);

  second = Gtk::make_managed<Gtk::Entry>();
  second->set_halign(Gtk::Align::CENTER);
  second->set_margin(5);
  second->set_max_length(7);
  second->set_max_width_chars(7);
  second->set_alignment(0.5);
  second->set_input_purpose(Gtk::InputPurpose::DIGITS);
  extra_grid->attach(*second, 5, 1, 1, 1);

  Gtk::Box *h_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  extra_grid->attach(*h_box, 0, 2, 6, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Time:"));
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  h_box->append(*lab);

  std::vector<Glib::ustring> list;
  list.push_back("UTC");
  list.push_back("TT");
  list.push_back("TDB");

  timecomb = Gtk::make_managed<Gtk::DropDown>(list);
  timecomb->set_halign(Gtk::Align::START);
  timecomb->set_margin(5);
  timecomb->set_selected(0);
  h_box->append(*timecomb);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Hour belt:"));
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  h_box->append(*lab);

  std::stringstream strm;
  std::locale loc("C");
  list.clear();
  for(int i = -12; i <= 12; i++)
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << i;
      Glib::ustring wstr;
      if(i > 0)
        {
          wstr = "+" + Glib::ustring(strm.str());
        }
      else
        {
          wstr = Glib::ustring(strm.str());
        }
      list.push_back(wstr);
    }

  belt = Gtk::make_managed<Gtk::DropDown>(list);
  belt->set_halign(Gtk::Align::START);
  belt->set_margin(5);
  belt->set_selected(12);
  h_box->append(*belt);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_markup(Glib::ustring("<b>") + gettext("Parameters input") + "</b>");
  v_box->append(*lab);

  h_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  v_box->append(*h_box);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Object:"));
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  h_box->append(*lab);

  Glib::RefPtr<Gio::ListStore<BodyListItem>> bodylist;
  bodylist = createBodyList();

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> body_exp
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &item) {
            Glib::RefPtr<BodyListItem> bli
                = std::dynamic_pointer_cast<BodyListItem>(item);
            if(bli)
              {
                return bli->bodyname;
              }
            else
              {
                return Glib::ustring("");
              }
          });

  objcomb = Gtk::make_managed<Gtk::DropDown>(bodylist, body_exp);
  objcomb->set_factory(createBodyFactory());
  objcomb->set_halign(Gtk::Align::START);
  objcomb->set_margin(5);
  objcomb->set_selected(0);
  objcomb->set_enable_search(true);
  h_box->append(*objcomb);

  h_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  v_box->append(*h_box);

  Gtk::Label *coord = Gtk::make_managed<Gtk::Label>();
  coord->set_text(gettext("Coordinates:"));
  coord->set_halign(Gtk::Align::END);
  coord->set_margin(5);
  h_box->append(*coord);

  list.clear();
  list.push_back(gettext("Equatorial"));
  list.push_back(gettext("Ecliptical"));

  coordcomb = Gtk::make_managed<Gtk::DropDown>(list);
  coordcomb->set_halign(Gtk::Align::START);
  coordcomb->set_margin(5);
  coordcomb->set_selected(0);
  h_box->append(*coordcomb);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::END);
  lab->set_text(gettext("Coordinates/Speed:"));
  h_box->append(*lab);

  list.clear();
  list.push_back("X, Y, Z");
  list.push_back("Vx, Vy, Vz");

  xyzcomb = Gtk::make_managed<Gtk::DropDown>(list);
  xyzcomb->set_halign(Gtk::Align::START);
  xyzcomb->set_margin(5);
  xyzcomb->set_selected(0);
  h_box->append(*xyzcomb);

  h_box = Gtk::make_managed<Gtk::Box>();
  v_box->append(*h_box);

  Gtk::Label *equin = Gtk::make_managed<Gtk::Label>();
  equin->set_text(gettext("Equator and equinox:"));
  equin->set_halign(Gtk::Align::END);
  equin->set_margin(5);
  h_box->append(*equin);

  list.clear();
  list.push_back(gettext("Mean (J2000)"));
  list.push_back(gettext("Mean of the date (IAU2000)"));
  list.push_back(gettext("True of the date (IAU2000)"));

  equincomb = Gtk::make_managed<Gtk::DropDown>(list);
  equincomb->set_halign(Gtk::Align::START);
  equincomb->set_margin(5);
  equincomb->set_selected(0);
  h_box->append(*equincomb);

  h_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  v_box->append(*h_box);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Units of measurement:"));
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  h_box->append(*lab);

  list.clear();
  list.push_back(gettext("Astronomical units"));
  list.push_back(gettext("Kilometers"));
  list.push_back(gettext("Meters"));

  unitcomb = Gtk::make_managed<Gtk::DropDown>(list);
  unitcomb->set_halign(Gtk::Align::START);
  unitcomb->set_margin(5);
  unitcomb->set_selected(0);
  Glib::PropertyProxy<guint> sel = objcomb->property_selected();
  sel.signal_changed().connect(
      std::bind(&MainWindow::objcombChangeFunc, this, coord, equin));
  Glib::PropertyProxy<guint> sel2 = xyzcomb->property_selected();
  sel2.signal_changed().connect(
      std::bind(&MainWindow::xyzCombChancgeFunc, this));
  h_box->append(*unitcomb);

  h_box = Gtk::make_managed<Gtk::Box>();
  v_box->append(*h_box);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Step size (days):"));
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  h_box->append(*lab);

  stepent = Gtk::make_managed<Gtk::Entry>();
  stepent->set_halign(Gtk::Align::START);
  stepent->set_margin(5);
  stepent->set_max_width_chars(4);
  stepent->set_alignment(0.5);
  stepent->set_input_purpose(Gtk::InputPurpose::DIGITS);
  h_box->append(*stepent);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_text(gettext("Number of steps:"));
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  h_box->append(*lab);

  stepnument = Gtk::make_managed<Gtk::Entry>();
  stepnument->set_halign(Gtk::Align::START);
  stepnument->set_margin(5);
  stepnument->set_max_width_chars(4);
  stepnument->set_alignment(0.5);
  stepnument->set_input_purpose(Gtk::InputPurpose::DIGITS);
  h_box->append(*stepnument);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + gettext("Ephemeris files") + "</b>");
  v_box->append(*lab);

  extra_grid = Gtk::make_managed<Gtk::Grid>();
  extra_grid->set_halign(Gtk::Align::FILL);
  extra_grid->set_valign(Gtk::Align::FILL);
  v_box->append(*extra_grid);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::START);
  lab->set_margin(5);
  lab->set_text(gettext("Path to ephemerides file:"));
  extra_grid->attach(*lab, 0, 0, 1, 1);

  std::vector<std::tuple<uint8_t, std::string>> pathv = formPathV();

  pathent = Gtk::make_managed<Gtk::Entry>();
  pathent->set_margin(5);
  pathent->set_width_chars(50);
  auto itpv = std::find_if(pathv.begin(), pathv.end(),
                           [](std::tuple<uint8_t, std::string> &el) {
                             return std::get<0>(el) == 1;
                           });
  if(itpv != pathv.end())
    {
      pathent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  extra_grid->attach(*pathent, 0, 1, 1, 1);

  Gtk::Button *openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), pathent));
  extra_grid->attach(*openb, 1, 1, 1, 1);

  Gtk::Button *clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([this] {
    pathent->set_text("");
  });
  extra_grid->attach(*clearbut, 2, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::START);
  lab->set_margin(5);
  lab->set_text(gettext("Path to TT-TDB file:"));
  extra_grid->attach(*lab, 0, 2, 1, 1);

  tttdbent = Gtk::make_managed<Gtk::Entry>();
  tttdbent->set_margin(5);
  tttdbent->set_width_chars(50);
  itpv = std::find_if(pathv.begin(), pathv.end(), [](auto &el) {
    return std::get<0>(el) == 2;
  });
  if(itpv != pathv.end())
    {
      tttdbent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  extra_grid->attach(*tttdbent, 0, 3, 1, 1);

  openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), tttdbent));
  extra_grid->attach(*openb, 1, 3, 1, 1);

  clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([this] {
    tttdbent->set_text("");
  });
  extra_grid->attach(*clearbut, 2, 3, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::START);
  lab->set_margin(5);
  lab->set_text(gettext("Path to Moon libration file:"));
  extra_grid->attach(*lab, 0, 4, 1, 1);

  mlbent = Gtk::make_managed<Gtk::Entry>();
  mlbent->set_margin(5);
  mlbent->set_width_chars(50);
  itpv = std::find_if(pathv.begin(), pathv.end(), [](auto &el) {
    return std::get<0>(el) == 3;
  });
  if(itpv != pathv.end())
    {
      mlbent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  extra_grid->attach(*mlbent, 0, 5, 1, 1);

  openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), mlbent));
  extra_grid->attach(*openb, 1, 5, 1, 1);

  clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([this] {
    mlbent->set_text("");
  });
  extra_grid->attach(*clearbut, 2, 5, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::START);
  lab->set_margin(5);
  lab->set_text(gettext("Path to small bodies file:"));
  extra_grid->attach(*lab, 0, 6, 1, 1);

  smlent = Gtk::make_managed<Gtk::Entry>();
  smlent->set_margin(5);
  smlent->set_width_chars(50);
  itpv = std::find_if(pathv.begin(), pathv.end(), [](auto &el) {
    return std::get<0>(el) == 5;
  });
  if(itpv != pathv.end())
    {
      smlent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  extra_grid->attach(*smlent, 0, 7, 1, 1);

  openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), smlent));
  extra_grid->attach(*openb, 1, 7, 1, 1);

  clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([this] {
    smlent->set_text("");
  });
  extra_grid->attach(*clearbut, 2, 7, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + gettext("Scale") + "</b>");
  v_box->append(*lab);

  h_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  v_box->append(*h_box);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::END);
  lab->set_text(gettext("Diagram scale factor:"));
  h_box->append(*lab);

  scale_ent = Gtk::make_managed<Gtk::Entry>();
  scale_ent->set_margin(5);
  scale_ent->set_halign(Gtk::Align::START);
  scale_ent->set_width_chars(10);
  itpv = std::find_if(pathv.begin(), pathv.end(),
                      [](std::tuple<uint8_t, std::string> &el) {
                        return std::get<0>(el) == 4;
                      });
  if(itpv != pathv.end())
    {
      if(!std::get<1>(*itpv).empty())
        {
          scale_ent->set_text(Glib::ustring(std::get<1>(*itpv)));
        }
      else
        {
          scale_ent->set_text("0.00000001");
        }
    }
  else
    {
      scale_ent->set_text("0.00000001");
    }
  h_box->append(*scale_ent);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + gettext("Operations") + "</b>");
  v_box->append(*lab);

  h_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  h_box->set_halign(Gtk::Align::CENTER);
  v_box->append(*h_box);

  Gtk::Button *calc = Gtk::make_managed<Gtk::Button>();
  calc->set_halign(Gtk::Align::CENTER);
  calc->set_margin(5);
  calc->set_label(gettext("Calculate coordinates"));
  calc->set_name("button");
  calc->signal_clicked().connect(std::bind(&MainWindow::calcCoord, this));
  h_box->append(*calc);

  Gtk::Button *orb = Gtk::make_managed<Gtk::Button>();
  orb->set_halign(Gtk::Align::CENTER);
  orb->set_margin(5);
  orb->set_label(gettext("Orbits"));
  orb->set_name("button");
  orb->signal_clicked().connect(std::bind(&MainWindow::orbitsGraph, this));
  h_box->append(*orb);

  Gtk::Button *about = Gtk::make_managed<Gtk::Button>();
  about->set_halign(Gtk::Align::CENTER);
  about->set_margin(5);
  about->set_label(gettext("About"));
  about->set_name("button");
  about->signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::aboutProg));
  h_box->append(*about);

  this->signal_close_request().connect(std::bind(&MainWindow::closeFunc, this),
                                       false);
}

void
MainWindow::objcombChangeFunc(Gtk::Label *coord, Gtk::Label *equin)
{
  std::vector<Glib::ustring> list;
  if(objcomb->get_selected() == 21)
    {
      list.push_back("φ, θ, ψ");
      list.push_back("φ', θ', ψ'");
      Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
      xyzcomb->set_model(strl);
      xyzcomb->set_selected(0);
      list.clear();
      list.push_back(gettext("rad"));
      list.push_back(gettext("degrees"));
      strl = Gtk::StringList::create(list);
      unitcomb->set_model(strl);
      unitcomb->set_selected(0);
      coord->set_opacity(0);
      coordcomb->set_opacity(0);
      equin->set_opacity(0);
      equincomb->set_opacity(0);
    }
  else
    {
      list.push_back("X, Y, Z");
      list.push_back("Vx, Vy, Vz");
      Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
      xyzcomb->set_model(strl);
      xyzcomb->set_selected(0);
      list.clear();
      list.push_back(gettext("Astronomical units"));
      list.push_back(gettext("Kilometers"));
      list.push_back(gettext("Meters"));
      strl = Gtk::StringList::create(list);
      unitcomb->set_model(strl);
      unitcomb->set_selected(0);
      coord->set_opacity(1);
      coordcomb->set_opacity(1);
      equin->set_opacity(1);
      equincomb->set_opacity(1);
    }
  xyzcomb->set_selected(0);
  unitcomb->set_selected(0);
}

void
MainWindow::xyzCombChancgeFunc()
{
  std::vector<Glib::ustring> list;
  if(objcomb->get_selected() != 21)
    {
      switch(xyzcomb->get_selected())
        {
        case 0:
          {
            list.clear();
            list.push_back(gettext("Astronomical units"));
            list.push_back(gettext("Kilometers"));
            list.push_back(gettext("Meters"));
            Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
            unitcomb->set_model(strl);
            unitcomb->set_selected(0);
            break;
          }
        case 1:
          {
            list.clear();
            list.push_back(gettext("AU/day"));
            list.push_back(gettext("km/day"));
            list.push_back(gettext("km/s"));
            list.push_back(gettext("m/s"));
            Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
            unitcomb->set_model(strl);
            unitcomb->set_selected(0);
            break;
          }
        default:
          break;
        }
      unitcomb->set_selected(0);
    }
  else
    {
      switch(xyzcomb->get_selected())
        {
        case 0:
          {
            list.clear();
            list.push_back(gettext("rad"));
            list.push_back(gettext("degrees"));
            Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
            unitcomb->set_model(strl);
            unitcomb->set_selected(0);
            break;
          }
        case 1:
          {
            list.clear();
            list.push_back(gettext("rad/day"));
            list.push_back(gettext("\"/day"));
            Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
            unitcomb->set_model(strl);
            unitcomb->set_selected(0);
            break;
          }
        default:
          break;
        }
      unitcomb->set_selected(0);
    }
}

Glib::RefPtr<Gio::ListStore<BodyListItem>>
MainWindow::createBodyList()
{
  Glib::RefPtr<Gio::ListStore<BodyListItem>> result
      = Gio::ListStore<BodyListItem>::create();
  std::filesystem::path blpath
      = Sharepath / std::filesystem::u8path("Bodylist.csv");
  std::vector<std::vector<std::string>> listv;
  std::fstream f;
  f.open(blpath, std::ios_base::in);
  if(f.is_open())
    {
      while(!f.eof())
        {
          std::string line;
          getline(f, line);
          if(!line.empty())
            {
              std::vector<std::string> item;
              for(;;)
                {
                  std::string::size_type n = line.find(";");
                  if(n != std::string::npos)
                    {
                      std::string tmp = line.substr(0, n);
                      item.push_back(tmp);
                      tmp = tmp + ";";
                      line.erase(0, tmp.size());
                    }
                  else
                    {
                      break;
                    }
                }
              if(item.size() > 0)
                {
                  listv.push_back(item);
                }
            }
        }
      f.close();
    }
  else
    {
      std::cout << "MainWindow::createBodyList: list not opened" << std::endl;
    }

  if(listv.size() > 0)
    {
      std::vector<std::string> item = *listv.begin();
      listv.erase(listv.begin());
      std::string lnm = std::setlocale(LC_CTYPE, NULL);
      std::string::size_type n;
      n = lnm.find(".");
      if(n != std::string::npos)
        {
          lnm = lnm.substr(0, n);
        }
      size_t dist = 1;
      auto it = std::find_if(item.begin(), item.end(), [lnm](auto &el) {
        std::string::size_type n;
        n = el.find(lnm);
        if(n != std::string::npos)
          {
            return true;
          }
        else
          {
            return false;
          }
      });
      if(it != item.end())
        {
          dist = std::distance(item.begin(), it);
        }
      std::stringstream strm;
      std::locale loc("C");
      for(size_t i = 0; i < listv.size(); i++)
        {
          item = listv[i];
          if(item.size() > 0)
            {
              std::string val = item[0];
              strm.clear();
              strm.str("");
              strm.imbue(loc);
              strm.str(val);
              int naifid;
              strm >> naifid;
              if(item.size() > dist)
                {
                  val = item[dist];
                }
              else if(item.size() >= 2)
                {
                  val = item[1];
                }
              Glib::RefPtr<BodyListItem> bli
                  = BodyListItem::create(val, naifid);
              result->append(bli);
            }
        }
    }

  return result;
}

Glib::RefPtr<Gtk::SignalListItemFactory>
MainWindow::createBodyFactory()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> bodyfact
      = Gtk::SignalListItemFactory::create();
  bodyfact->signal_setup().connect(
      [](const Glib::RefPtr<Gtk::ListItem> &list_item) {
        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_halign(Gtk::Align::CENTER);
        list_item->set_child(*lab);
      });
  bodyfact->signal_bind().connect(
      [](const Glib::RefPtr<Gtk::ListItem> &list_item) {
        Glib::RefPtr<BodyListItem> bli
            = std::dynamic_pointer_cast<BodyListItem>(list_item->get_item());
        if(bli)
          {
            Gtk::Label *lab
                = dynamic_cast<Gtk::Label *>(list_item->get_child());
            if(lab)
              {
                lab->set_text(bli->bodyname);
              }
          }
      });
  return bodyfact;
}

std::vector<std::tuple<uint8_t, std::string>>
MainWindow::formPathV()
{
  /*
   * pathv types:
   * 1 - ephemeris file, 2 - TT-TDB file, 3 - Moon libration file, 4 - scale
   * factor, 5 - path to small bodies file
   */
  std::vector<std::tuple<uint8_t, std::string>> pathv;
  bool err = false;
  std::filesystem::path filepath
      = std::filesystem::u8path(Glib::get_home_dir());
  filepath /= std::filesystem::u8path(".config/EphEPM/ephpath");
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      uintmax_t fsz = std::filesystem::file_size(filepath);
      uintmax_t rb = 0;
      uint64_t entsz;
      std::string rs;
      while(rb < fsz)
        {
          rs.clear();
          entsz = 0;
          rs.resize(sizeof(entsz));
          if(rb + static_cast<uintmax_t>(rs.size()) > fsz)
            {
              std::cout << "MainWindow::createWindow: wrong ephpath file size"
                        << std::endl;
              err = true;
              break;
            }
          else
            {
              f.read(rs.data(), rs.size());
              rb += static_cast<uintmax_t>(rs.size());
              std::memcpy(&entsz, rs.c_str(), rs.size());
            }

          if(entsz > 0)
            {
              rs.clear();
              if(rb + static_cast<uintmax_t>(entsz) > fsz)
                {
                  std::cout << "MainWindow::createWindow: wrong entry size"
                            << std::endl;
                  err = true;
                  break;
                }
              else
                {
                  rs.resize(static_cast<size_t>(entsz));
                  f.read(rs.data(), rs.size());
                  rb += static_cast<uintmax_t>(rs.size());
                  uint8_t type;
                  std::memcpy(&type, rs.c_str(), sizeof(type));
                  rs.erase(rs.begin(), rs.begin() + sizeof(type));
                  pathv.push_back(std::make_tuple(type, rs));
                }
            }
        }
      f.close();
    }
  if(err)
    {
      std::filesystem::remove_all(filepath);
    }

  return pathv;
}

void
MainWindow::openDialog(Gtk::Entry *entry)
{
#ifndef EPH_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fcd = Gtk::FileDialog::create();
  fcd->set_modal(true);
  fcd->set_title(gettext("Ephemeris file selection"));
  Glib::RefPtr<Gio::File> fl
      = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_initial_folder(fl);
  fcd->set_accept_label(gettext("Select"));
  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fcd->open(
      *this,
      [entry](Glib::RefPtr<Gio::AsyncResult> &result) {
        Glib::RefPtr<Gio::File> fl;
        auto obj = result->get_source_object_base();
        auto fchd = std::dynamic_pointer_cast<Gtk::FileDialog>(obj);
        if(!fchd)
          {
            return void();
          }
        try
          {
            fl = fchd->open_finish(result);
          }
        catch(Glib::Error &e)
          {
            if(e.code() != Gtk::DialogError::DISMISSED)
              {
                std::cout << "MainWindow::openDialog:" << e.what()
                          << std::endl;
              }
          }
        if(fl)
          {
            entry->set_text(fl->get_path());
          }
      },
      cncl);
#endif
#ifdef EPH_GTK_OLD
  Gtk::FileChooserDialog *fcd
      = new Gtk::FileChooserDialog(*this, gettext("Ephemeris file selection"),
                                   Gtk::FileChooser::Action::OPEN, true);
  fcd->set_application(this->get_application());
  fcd->set_modal(true);

  Glib::RefPtr<Gio::File> fl
      = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_current_folder(fl);

  fcd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  fcd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);

  fcd->signal_response().connect([fcd, entry](int resp_id) {
    if(resp_id == Gtk::ResponseType::ACCEPT)
      {
        Glib::RefPtr<Gio::File> fl = fcd->get_file();
        if(fl)
          {
            entry->set_text(fl->get_path());
          }
      }
    fcd->close();
  });

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

void
MainWindow::calcCoord()
{
  std::string daystr(day->get_text());
  std::string monthstr(month->get_text());
  std::string yearstr(year->get_text());
  std::string hourstr(hour->get_text());
  std::string minutstr(minut->get_text());
  std::string secondstr(second->get_text());
  Glib::RefPtr<Gio::ListModel> model = belt->get_model();
  Glib::RefPtr<Gtk::StringList> strl
      = std::dynamic_pointer_cast<Gtk::StringList>(model);
  std::string beltstr(strl->get_string(belt->get_selected()));
  std::string stepstr(stepent->get_text());
  std::string stepnumberstr(stepnument->get_text());
  std::string pathstr(pathent->get_text());
  std::string tttdbstr(tttdbent->get_text());
  std::string smlstr(smlent->get_text());
  int naifid = -1;
  Glib::RefPtr<BodyListItem> bli
      = std::dynamic_pointer_cast<BodyListItem>(objcomb->get_selected_item());
  if(bli)
    {
      naifid = bli->naifid;
    }
  else
    {
      errDialog(10);
      return void();
    }
  int coordtype = coordcomb->get_selected();
  int xyz = static_cast<int>(xyzcomb->get_selected());
  int theory = static_cast<int>(equincomb->get_selected());
  int unit = static_cast<int>(unitcomb->get_selected());

  if(naifid == 1800303)
    {
      pathstr = std::string(mlbent->get_text());
    }

  int daynum = -1;
  int monthnum = -1;
  int yearnum = -1;
  int hournum = -1;
  int minutnum = -1;
  double secondnum = -1;
  stepnum = -1;
  int stepnumbernum = -1;

  int beltnum;
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << beltstr;
  strm >> beltnum;

  if(daystr == "")
    {
      errDialog(1);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << daystr;
      strm >> daynum;
      if(daynum < 1)
        {
          errDialog(1);
          return void();
        }
    }

  if(monthstr == "")
    {
      errDialog(2);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << monthstr;
      strm >> monthnum;
      if(monthnum < 1 || monthnum > 12)
        {
          errDialog(2);
          return void();
        }
      if(monthnum == 1 || monthnum == 3 || monthnum == 5 || monthnum == 7
         || monthnum == 8 || monthnum == 10 || monthnum == 12)
        {
          if(daynum > 31)
            {
              errDialog(1);
              return void();
            }
        }
      else
        {
          if(daynum > 30)
            {
              errDialog(1);
              return void();
            }
        }
    }

  if(yearstr == "")
    {
      errDialog(3);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << yearstr;
      strm >> yearnum;
      if((yearnum & 3) == 0 && ((yearnum % 25) != 0 || (yearnum & 15) == 0))
        {
          if(monthnum == 2)
            {
              if(daynum > 29)
                {
                  errDialog(1);
                  return void();
                }
            }
        }
      else
        {
          if(monthnum == 2)
            {
              if(daynum > 28)
                {
                  errDialog(1);
                  return void();
                }
            }
        }
    }

  if(hourstr == "")
    {
      errDialog(4);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << hourstr;
      strm >> hournum;
      if(hournum < 0 || hournum > 23)
        {
          errDialog(4);
          return void();
        }
    }

  if(minutstr == "")
    {
      errDialog(5);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << minutstr;
      strm >> minutnum;
      if(minutnum < 0 || minutnum > 59)
        {
          errDialog(5);
          return void();
        }
    }

  if(secondstr == "")
    {
      errDialog(6);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << secondstr;
      strm >> secondnum;
      if(secondnum < 0 || secondnum > 59)
        {
          errDialog(6);
          return void();
        }
    }

  if(stepstr == "")
    {
      errDialog(7);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << stepstr;
      strm >> stepnum;
      if(stepnum <= 0)
        {
          errDialog(7);
          return void();
        }
    }

  if(stepnumberstr == "")
    {
      errDialog(8);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << stepnumberstr;
      strm >> stepnumbernum;
      if(stepnumbernum <= 0)
        {
          errDialog(8);
          return void();
        }
    }
  Gtk::ProgressBar *bar = nullptr;
  Gtk::Window *info_win = resultPulseWin(0, bar);
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  AuxFunc af;
  mpf_set_default_prec(512);
  JDshow = af.utcJD(daynum, monthnum, yearnum, hournum, minutnum, secondnum);
  DAFOperations daf;
  bool datech;
  if(!tttdbstr.empty())
    {
      datech = daf.epochCheckUTC(
          daynum, monthnum, yearnum, hournum, minutnum, secondnum,
          static_cast<int>(timecomb->get_selected()), beltnum, tttdbstr);
    }
  else
    {
      datech = daf.epochCheckUTC(
          daynum, monthnum, yearnum, hournum, minutnum, secondnum,
          static_cast<int>(timecomb->get_selected()), beltnum, pathstr);
    }
  if(!datech)
    {
      info_win->close();
      errDialog(9);
      return void();
    }
  double JDcalc = JDshow;
  if(JDcalc < 0 && beltnum != 0)
    {
      double k = -static_cast<double>(beltnum) / 24.0;
      k = 1.0 - k;
      JDcalc = JDcalc - k;
    }
  else
    {
      double k = -static_cast<double>(beltnum) / 24.0;
      JDcalc = JDcalc + k;
    }
  Coordinates *calc = new Coordinates(
      naifid, JDcalc, static_cast<int>(timecomb->get_selected()), coordtype,
      xyz, theory, unit, stepnum, stepnumbernum, pathstr, tttdbstr, smlstr);
  std::vector<CoordKeeper> *result = new std::vector<CoordKeeper>;
  Glib::Dispatcher *result_win_disp = new Glib::Dispatcher;
  result_win_disp->connect([result, this, result_win_disp, info_win] {
    std::unique_ptr<Glib::Dispatcher> disp(result_win_disp);
    info_win->close();
    omp_set_dynamic(false);
    resultPresenting(result);
  });

#ifdef EPH_OMP_TASK
#pragma omp masked
  {
    omp_set_dynamic(true);
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      *result = calc->calculationsXYZ();
      delete calc;
      result_win_disp->emit();
      omp_fulfill_event(event);
    }
  }
#else
  std::thread thr([result, calc, result_win_disp] {
    *result = calc->calculationsXYZ();
    delete calc;
    result_win_disp->emit();
  });
  thr.detach();
#endif
}

void
MainWindow::errDialog(const int &variant)
{
  Glib::ustring msgtxt;
  switch(variant)
    {
    case 1:
      {
        msgtxt = gettext("Wrong day input!");
        break;
      }
    case 2:
      {
        msgtxt = gettext("Wrong month input!");
        break;
      }
    case 3:
      {
        msgtxt = gettext("Wrong year input!");
        break;
      }
    case 4:
      {
        msgtxt = gettext("Wrong hour input!");
        break;
      }
    case 5:
      {
        msgtxt = gettext("Wrong minutes input!");
        break;
      }
    case 6:
      {
        msgtxt = gettext("Wrong seconds input!");
        break;
      }
    case 7:
      {
        msgtxt = gettext("Wrong step input!");
        break;
      }
    case 8:
      {
        msgtxt = gettext("Wrong step number input!");
        break;
      }
    case 9:
      {
        msgtxt = gettext("Date is out of ephemeris time interval or "
                         "TT-TDB amendments was not found!");
        break;
      }
    case 10:
      {
        msgtxt = gettext("Critical error!");
        break;
      }
    case 11:
      {
        msgtxt = gettext("Incorrect ephemeris file!");
        break;
      }
    default:
      break;
    }

  Gtk::Window *window = new Gtk::Window;
  window->set_application(this->get_application());
  window->set_title(gettext("Error!"));
  window->set_transient_for(*this);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_text(msgtxt);
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*close, 0, 1, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::shared_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

Gtk::Window *
MainWindow::resultPulseWin(const int &variant, Gtk::ProgressBar *bar,
                           OrbitsDiagram *od)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(this->get_application());
  window->set_name("mainWindow");
  window->set_title(gettext("Processing..."));
  window->set_modal(true);
  window->set_deletable(false);
  window->set_transient_for(*this);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  window->set_child(*grid);

  switch(variant)
    {
    case 0:
      {
        Gtk::Label *calclab = Gtk::make_managed<Gtk::Label>();
        calclab->set_halign(Gtk::Align::CENTER);
        calclab->set_margin(5);
        calclab->set_text(gettext("Calculation in progress..."));
        grid->attach(*calclab, 0, 0, 1, 1);
        break;
      }
    case 1:
      {
        Gtk::Label *calclab = Gtk::make_managed<Gtk::Label>();
        calclab->set_halign(Gtk::Align::CENTER);
        calclab->set_margin(5);
        calclab->set_text(gettext("Calculation in progress..."));
        grid->attach(*calclab, 0, 0, 1, 1);

        bar->set_name("prgBar");
        bar->set_halign(Gtk::Align::CENTER);
        bar->set_margin(5);
        bar->set_show_text(true);
        grid->attach(*bar, 0, 1, 1, 1);

        Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
        cancel->set_name("closeButton");
        cancel->set_halign(Gtk::Align::CENTER);
        cancel->set_margin(5);
        cancel->set_label(gettext("Cancel"));
        cancel->signal_clicked().connect([this, cancel, calclab, bar, od] {
          if(od)
            {
              od->stopAll();
            }
          cancel->set_visible(false);
          bar->set_visible(false);
          calclab->set_text(gettext("Canceling..."));
        });
        grid->attach(*cancel, 0, 2, 1, 1);
        break;
      }
    default:
      break;
    }

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);
  window->present();

  return window;
}

void
MainWindow::resultPresenting(std::vector<CoordKeeper> *result)
{
  std::shared_ptr<dataset> result_data(new dataset);
  Glib::RefPtr<BodyListItem> bli
      = std::dynamic_pointer_cast<BodyListItem>(objcomb->get_selected_item());
  if(bli)
    {
      result_data->bodyname = bli->bodyname;
    }
  else
    {
      std::cout << "MainWindow::resultPresenting: BodyListItem is null"
                << std::endl;
      return void();
    }

  Glib::RefPtr<Gio::ListModel> lmodel = coordcomb->get_model();
  Glib::RefPtr<Gtk::StringList> strl
      = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  if(strl)
    {
      result_data->coordinates = strl->get_string(coordcomb->get_selected());
    }
  else
    {
      std::cout << "MainWindow::resultPresenting: coordinates model is null"
                << std::endl;
      return void();
    }

  lmodel = equincomb->get_model();
  strl = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  if(strl)
    {
      result_data->equinox = strl->get_string(equincomb->get_selected());
    }
  else
    {
      std::cout << "MainWindow::resultPresenting: equinox model is null"
                << std::endl;
      return void();
    }

  lmodel = unitcomb->get_model();
  strl = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  if(strl)
    {
      result_data->units = strl->get_string(unitcomb->get_selected());
    }
  else
    {
      std::cout << "MainWindow::resultPresenting: units model is null"
                << std::endl;
      return void();
    }

  lmodel = belt->get_model();
  strl = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  if(strl)
    {
      result_data->hour_belt = strl->get_string(belt->get_selected());
    }
  else
    {
      std::cout << "MainWindow::resultPresenting: belt model is null"
                << std::endl;
      return void();
    }

  result_data->JDshow = JDshow;
  result_data->stepnum = stepnum;
  result_data->objnum = objcomb->get_selected();
  result_data->xyznum = xyzcomb->get_selected();
  result_data->result = std::move(*result);
  delete result;

  ResultWindow *rw = new ResultWindow(this, result_data);
  rw->createWindow();
}

void
MainWindow::orbitsGraph()
{
  std::string daystr(day->get_text());
  std::string monthstr(month->get_text());
  std::string yearstr(year->get_text());
  std::string hourstr(hour->get_text());
  std::string minutstr(minut->get_text());
  std::string secondstr(second->get_text());
  Glib::RefPtr<Gio::ListModel> lmodel = belt->get_model();
  Glib::RefPtr<Gtk::StringList> strl
      = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  std::string beltstr(strl->get_string(belt->get_selected()));
  std::string pathstr(pathent->get_text());
  std::string tttdbstr(tttdbent->get_text());
  std::string smlstr(smlent->get_text());
  std::string scale_fact(scale_ent->get_text());
  auto itsf = scale_fact.begin();
  while(itsf != scale_fact.end())
    {
      char c = *itsf;
      if(c == ',')
        {
          c = '.';
          *itsf = c;
        }
      else if((c < '0' || c > '9') && c != '.')
        {
          break;
        }
      itsf++;
    }
  if(itsf != scale_fact.end())
    {
      scale_fact = "0.000000001";
      scale_ent->set_text(Glib::ustring(scale_fact));
    }

  int daynum = -1;
  int monthnum = -1;
  int yearnum = -1;
  int hournum = -1;
  int minutnum = -1;
  double secondnum = -1;
  double plot_fact = 0.000000001;
  int beltnum;

  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << beltstr;
  strm >> beltnum;

  strm.clear();
  strm.str("");
  strm.imbue(loc);
  strm << scale_fact;
  strm >> plot_fact;

  if(daystr == "")
    {
      errDialog(1);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << daystr;
      strm >> daynum;
      if(daynum < 1)
        {
          errDialog(1);
          return void();
        }
    }

  if(monthstr == "")
    {
      errDialog(2);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << monthstr;
      strm >> monthnum;
      if(monthnum < 1 || monthnum > 12)
        {
          errDialog(2);
          return void();
        }
      if(monthnum == 1 || monthnum == 3 || monthnum == 5 || monthnum == 7
         || monthnum == 8 || monthnum == 10 || monthnum == 12)
        {
          if(daynum > 31)
            {
              errDialog(1);
              return void();
            }
        }
      else
        {
          if(daynum > 30)
            {
              errDialog(1);
              return void();
            }
        }
    }

  if(yearstr == "")
    {
      errDialog(3);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << yearstr;
      strm >> yearnum;
      if((yearnum & 3) == 0 && ((yearnum % 25) != 0 || (yearnum & 15) == 0))
        {
          if(monthnum == 2)
            {
              if(daynum > 29)
                {
                  errDialog(1);
                  return void();
                }
            }
        }
      else
        {
          if(monthnum == 2)
            {
              if(daynum > 28)
                {
                  errDialog(1);
                  return void();
                }
            }
        }
    }

  if(hourstr == "")
    {
      errDialog(4);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << hourstr;
      strm >> hournum;
      if(hournum < 0 || hournum > 23)
        {
          errDialog(4);
          return void();
        }
    }

  if(minutstr == "")
    {
      errDialog(5);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << minutstr;
      strm >> minutnum;
      if(minutnum < 0 || minutnum > 59)
        {
          errDialog(5);
          return void();
        }
    }

  if(secondstr == "")
    {
      errDialog(6);
      return void();
    }
  else
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << secondstr;
      strm >> secondnum;
      if(secondnum < 0 || secondnum > 59)
        {
          errDialog(6);
          return void();
        }
    }
  AuxFunc af;
  double JDcalc
      = af.utcJD(daynum, monthnum, yearnum, hournum, minutnum, secondnum);

  if(JDcalc < 0)
    {
      double k = -static_cast<double>(beltnum) / 24.0;
      k = 1.0 - k;
      JDcalc = JDcalc - k;
    }
  else
    {
      double k = -static_cast<double>(beltnum) / 24.0;
      JDcalc = JDcalc + k;
    }
  DAFOperations daf;
  bool chtm;
  if(tttdbstr.empty())
    {
      chtm = daf.epochCheckUTC(
          daynum, monthnum, yearnum, hournum, minutnum, secondnum,
          static_cast<int>(timecomb->get_selected()), beltnum, pathstr);
    }
  else
    {
      chtm = daf.epochCheckUTC(
          daynum, monthnum, yearnum, hournum, minutnum, secondnum,
          static_cast<int>(timecomb->get_selected()), beltnum, tttdbstr);
    }
  if(chtm)
    {
      Gtk::ProgressBar *bar = Gtk::make_managed<Gtk::ProgressBar>();
      bar->set_fraction(0.0);
      OrbitsDiagram *od = new OrbitsDiagram(
          this, pathstr, tttdbstr, smlstr, JDcalc,
          static_cast<int>(timecomb->get_selected()),
          static_cast<int>(coordcomb->get_selected()),
          static_cast<int>(equincomb->get_selected()), plot_fact);

      Gtk::Window *win = resultPulseWin(1, bar, od);
      double sz = static_cast<double>(od->calculateSize());
      std::shared_ptr<Glib::Dispatcher> pulse_disp
          = std::make_shared<Glib::Dispatcher>();
      std::shared_ptr<Glib::Dispatcher> compl_disp
          = std::make_shared<Glib::Dispatcher>();
      std::shared_ptr<Glib::Dispatcher> canceled_disp
          = std::make_shared<Glib::Dispatcher>();

      omp_lock_t *omp_mtx = new omp_lock_t;
      omp_init_lock(omp_mtx);
      std::shared_ptr<omp_lock_t> pulsemtx(omp_mtx, [](omp_lock_t *omp_mtx) {
        omp_destroy_lock(omp_mtx);
        delete omp_mtx;
      });
      std::shared_ptr<double> frac = std::make_shared<double>(0.0);
      pulse_disp->connect([bar, frac] {
        bar->set_fraction(*frac);
      });

      od->pulse_signal = [pulse_disp, frac, sz, pulsemtx, bar] {
        omp_set_lock(pulsemtx.get());
        *frac = *frac + 1.0 / sz;
        if(*frac - bar->get_fraction() > 0.01)
          {
            pulse_disp->emit();
          }
        omp_unset_lock(pulsemtx.get());
      };

      od->diagram_close = [od] {
        delete od;
      };

      compl_disp->connect([bar, win, od] {
        bar->set_fraction(1.0);
        win->close();
        od->diagramPlot();
      });

      canceled_disp->connect([win, od] {
        win->close();
        delete od;
      });

      od->calc_completed = [compl_disp] {
        compl_disp->emit();
      };
      od->canceled_signal = [canceled_disp] {
        canceled_disp->emit();
      };
#ifdef EPH_OMP_TASK
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          od->calculateOrbits();
          omp_fulfill_event(event);
        }
      }
#else
      std::thread thr(std::bind(&OrbitsDiagram::calculateOrbits, od));
      thr.detach();
#endif
    }
  else
    {
      errDialog(9);
    }
}

void
MainWindow::aboutProg()
{
  Gtk::AboutDialog *aboutd = new Gtk::AboutDialog;
  aboutd->set_transient_for(*this);
  aboutd->set_application(this->get_application());
  aboutd->set_name("mainWindow");
  aboutd->set_program_name("EphEPM");
  aboutd->set_version("2.3");
  aboutd->set_copyright(
      "Copyright 2022-2025 Yury Bobylev <bobilev_yury@mail.ru>");
  AuxFunc af;
  std::filesystem::path filepath
      = Sharepath / std::filesystem::u8path("COPYING");
  std::fstream f;
  Glib::ustring abbuf;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t sz = std::filesystem::file_size(filepath);
      std::vector<char> ab;
      ab.resize(sz);
      f.read(ab.data(), ab.size());
      f.close();
      abbuf = Glib::ustring(ab.begin(), ab.end());
    }
  else
    {
      std::cout << "Licence file not found" << std::endl;
    }

  if(abbuf.size() == 0)
    {
      aboutd->set_license_type(Gtk::License::GPL_3_0_ONLY);
    }
  else
    {
      aboutd->set_license(abbuf);
    }

  Glib::RefPtr<Gio::File> logofile
      = Gio::File::create_for_path(Sharepath.u8string() + "/ico.png");
  aboutd->set_logo(Gdk::Texture::create_from_file(logofile));
  abbuf = Glib::ustring(gettext("EphEPM is simple program to calculate some "
                                "Solar system bodies coordinates.\n"
                                "Author Yury Bobylev.\n\n"
                                "Program uses next libraries:\n"))
          + Glib::ustring("GTK https://www.gtk.org\n"
                          "GMP https://gmplib.org\n"
                          "MathGL http://mathgl.sourceforge.net\n"
                          "ICU https://icu.unicode.org\n"
                          "SOFA https://iausofa.org/");
  aboutd->set_comments(abbuf);

  aboutd->signal_close_request().connect(
      [aboutd] {
        aboutd->set_visible(false);
        delete aboutd;
        return true;
      },
      false);
  aboutd->present();
}

bool
MainWindow::closeFunc()
{
  std::string savepath(Glib::get_home_dir());
  savepath = savepath + "/.config/EphEPM/ephpath";
  std::filesystem::path filepath = std::filesystem::u8path(savepath);
  if(!std::filesystem::exists(filepath.parent_path()))
    {
      std::filesystem::create_directories(filepath.parent_path());
    }
  if(std::filesystem::exists(filepath))
    {
      std::filesystem::remove_all(filepath);
    }
  std::vector<std::tuple<uint8_t, std::string>> pathv;
  std::string line(pathent->get_text());
  if(!line.empty())
    {
      pathv.push_back(std::make_tuple(1, line));
    }

  line.clear();
  line = std::string(tttdbent->get_text());
  if(!line.empty())
    {
      pathv.push_back(std::make_tuple(2, line));
    }

  line.clear();
  line = std::string(mlbent->get_text());
  if(!line.empty())
    {
      pathv.push_back(std::make_tuple(3, line));
    }

  line.clear();
  line = std::string(scale_ent->get_text());
  if(!line.empty())
    {
      pathv.push_back(std::make_tuple(4, line));
    }

  line.clear();
  line = std::string(smlent->get_text());
  if(!line.empty())
    {
      pathv.push_back(std::make_tuple(5, line));
    }

  if(pathv.size() > 0)
    {
      std::fstream f;
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
        {
          std::string wstr;
          for(auto it = pathv.begin(); it != pathv.end(); it++)
            {
              wstr.clear();
              std::tuple<uint8_t, std::string> ttup = *it;
              uint8_t type = std::get<0>(*it);
              std::string path = std::get<1>(*it);
              wstr.resize(sizeof(type));
              std::memcpy(&wstr[0], &type, sizeof(type));

              std::copy(path.begin(), path.end(), std::back_inserter(wstr));
              uint64_t esz = static_cast<uint64_t>(wstr.size());
              std::vector<char> wv;
              wv.resize(sizeof(esz));
              std::memcpy(&wv[0], &esz, sizeof(esz));
              std::copy(wstr.begin(), wstr.end(), std::back_inserter(wv));
              f.write(wv.data(), wv.size());
            }
          f.close();
        }
      else
        {
          std::cout << "Error on saving configuration" << std::endl;
        }
    }
  this->set_visible(false);

  return true;
}
