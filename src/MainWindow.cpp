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

#include <AuxFunc.h>
#include <Coordinates.h>
#include <DAFOperations.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gdkmm/rectangle.h>
#include <gdkmm/surface.h>
#include <gdkmm/texture.h>
#include <glib/gtypes.h>
#include <glibmm/miscutils.h>
#include <glibmm/propertyproxy.h>
#include <glibmm/propertyproxy_base.h>
#include <glibmm/signalproxy.h>
#include <glibmm/ustring.h>
#include <gmp.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/columnviewcolumn.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/enums.h>
#include <gtkmm/expression.h>
#include <gtkmm/grid.h>
#include <gtkmm/listitem.h>
#include <gtkmm/noselection.h>
#include <gtkmm/object.h>
#include <gtkmm/requisition.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/signallistitemfactory.h>
#include <gtkmm/stringlist.h>
#include <gtkmm/stylecontext.h>
#include <libintl.h>
#include <MainWindow.h>
#include <ModelColumns.h>
#include <OrbitsDiagram.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/connection.h>
#include <sigc++/functors/mem_fun.h>
#include <stddef.h>
#include <algorithm>
#include <clocale>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <locale>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <tuple>

#ifndef EPH_GTK_OLD
#include <giomm/asyncresult.h>
#include <giomm/cancellable.h>
#include <gtkmm/error.h>
#include <gtkmm/filedialog.h>
#endif

#ifdef EPH_GTK_OLD
#include <gtkmm/filechooserdialog.h>
#endif

MainWindow::MainWindow()
{
  AuxFunc af;
  std::filesystem::path p(std::filesystem::u8path(af.get_selfpath()));
  Sharepath = p.parent_path().u8string() + "/../share/EphEPM";
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  std::string filename = Sharepath + "/mainWindow.css";
  p = std::filesystem::u8path(filename);
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

  filename = Sharepath + "/graphicWidg.css";
  p = std::filesystem::u8path(filename);
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
  Gtk::StyleContext::add_provider_for_display(disp, css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  orbits_cancel.store(0);
  createWindow();
}

MainWindow::~MainWindow()
{

}

void
MainWindow::createWindow()
{
  this->set_title("EphEPM");
  this->set_name("mainWindow");
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  this->set_child(*grid);
  grid->set_halign(Gtk::Align::CENTER);

  Gtk::Label *dateinput = Gtk::make_managed<Gtk::Label>();
  dateinput->set_use_markup(true);
  dateinput->set_markup(gettext("<b>Date and time input</b>"));
  dateinput->set_halign(Gtk::Align::CENTER);
  dateinput->set_margin(5);
  grid->attach(*dateinput, 0, 0, 6, 1);

  Gtk::Label *daylab = Gtk::make_managed<Gtk::Label>();
  daylab->set_text(gettext("Day"));
  daylab->set_halign(Gtk::Align::CENTER);
  daylab->set_margin(5);
  grid->attach(*daylab, 0, 1, 1, 1);

  Gtk::Entry *day = Gtk::make_managed<Gtk::Entry>();
  day->set_halign(Gtk::Align::CENTER);
  day->set_margin(5);
  day->set_max_length(2);
  day->set_max_width_chars(2);
  day->set_alignment(0.5);
  day->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*day, 0, 2, 1, 1);

  Gtk::Label *monthlab = Gtk::make_managed<Gtk::Label>();
  monthlab->set_text(gettext("Month"));
  monthlab->set_halign(Gtk::Align::CENTER);
  monthlab->set_margin(5);
  grid->attach(*monthlab, 1, 1, 1, 1);

  Gtk::Entry *month = Gtk::make_managed<Gtk::Entry>();
  month->set_halign(Gtk::Align::CENTER);
  month->set_margin(5);
  month->set_max_length(2);
  month->set_max_width_chars(2);
  month->set_alignment(0.5);
  month->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*month, 1, 2, 1, 1);

  Gtk::Label *yearlab = Gtk::make_managed<Gtk::Label>();
  yearlab->set_text(gettext("Year"));
  yearlab->set_halign(Gtk::Align::CENTER);
  yearlab->set_margin(5);
  grid->attach(*yearlab, 2, 1, 1, 1);

  Gtk::Entry *year = Gtk::make_managed<Gtk::Entry>();
  year->set_halign(Gtk::Align::CENTER);
  year->set_margin(5);
  year->set_max_length(6);
  year->set_max_width_chars(6);
  year->set_alignment(0.5);
  year->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*year, 2, 2, 1, 1);

  Gtk::Label *hourlab = Gtk::make_managed<Gtk::Label>();
  hourlab->set_text(gettext("Hours"));
  hourlab->set_halign(Gtk::Align::CENTER);
  hourlab->set_margin(5);
  grid->attach(*hourlab, 3, 1, 1, 1);

  Gtk::Entry *hour = Gtk::make_managed<Gtk::Entry>();
  hour->set_halign(Gtk::Align::CENTER);
  hour->set_margin(5);
  hour->set_max_length(2);
  hour->set_max_width_chars(2);
  hour->set_alignment(0.5);
  hour->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*hour, 3, 2, 1, 1);

  Gtk::Label *minutlab = Gtk::make_managed<Gtk::Label>();
  minutlab->set_text(gettext("Minutes"));
  minutlab->set_halign(Gtk::Align::CENTER);
  minutlab->set_margin(5);
  grid->attach(*minutlab, 4, 1, 1, 1);

  Gtk::Entry *minut = Gtk::make_managed<Gtk::Entry>();
  minut->set_halign(Gtk::Align::CENTER);
  minut->set_margin(5);
  minut->set_max_length(2);
  minut->set_max_width_chars(2);
  minut->set_alignment(0.5);
  minut->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*minut, 4, 2, 1, 1);

  Gtk::Label *secondlab = Gtk::make_managed<Gtk::Label>();
  secondlab->set_text(gettext("Seconds"));
  secondlab->set_halign(Gtk::Align::CENTER);
  secondlab->set_margin(5);
  grid->attach(*secondlab, 5, 1, 1, 1);

  Gtk::Entry *second = Gtk::make_managed<Gtk::Entry>();
  second->set_halign(Gtk::Align::CENTER);
  second->set_margin(5);
  second->set_max_length(7);
  second->set_max_width_chars(7);
  second->set_alignment(0.5);
  second->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*second, 5, 2, 1, 1);

  Gtk::Label *timelab = Gtk::make_managed<Gtk::Label>();
  timelab->set_text(gettext("Time: "));
  timelab->set_halign(Gtk::Align::START);
  timelab->set_margin(5);
  grid->attach(*timelab, 0, 3, 1, 1);

  std::vector<Glib::ustring> list;
  list.push_back("UTC");
  list.push_back("TT");
  list.push_back("TDB");
  Gtk::DropDown *timecomb = Gtk::make_managed<Gtk::DropDown>(list);
  timecomb->set_halign(Gtk::Align::START);
  timecomb->set_margin(5);
  timecomb->set_selected(0);
  grid->attach(*timecomb, 1, 3, 1, 1);

  Gtk::Label *beltlab = Gtk::make_managed<Gtk::Label>();
  beltlab->set_text(gettext("Hour belt: "));
  beltlab->set_halign(Gtk::Align::START);
  beltlab->set_margin(5);
  grid->attach(*beltlab, 2, 3, 2, 1);

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
  Gtk::DropDown *belt = Gtk::make_managed<Gtk::DropDown>(list);
  belt->set_halign(Gtk::Align::START);
  belt->set_margin(5);
  belt->set_selected(12);
  grid->attach(*belt, 3, 3, 2, 1);

  Gtk::Label *inpparl = Gtk::make_managed<Gtk::Label>();
  inpparl->set_margin(5);
  inpparl->set_halign(Gtk::Align::CENTER);
  inpparl->set_markup(gettext("<b>Parameters input</b>"));
  grid->attach(*inpparl, 0, 4, 6, 1);

  Gtk::Label *obj = Gtk::make_managed<Gtk::Label>();
  obj->set_text(gettext("Object: "));
  obj->set_halign(Gtk::Align::START);
  obj->set_margin(5);
  grid->attach(*obj, 0, 5, 1, 1);

  Glib::RefPtr<Gio::ListStore<BodyListItem>> bodylist;
  bodylist = createBodyList();
  Glib::RefPtr<Gtk::SignalListItemFactory> bodyfact =
      Gtk::SignalListItemFactory::create();
  bodyfact->signal_setup().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &list_item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_halign(Gtk::Align::CENTER);
      list_item->set_child(*lab);
    });
  bodyfact->signal_bind().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &list_item)
    {
      Glib::RefPtr<BodyListItem> bli =
      std::dynamic_pointer_cast<BodyListItem>(list_item->get_item());
      if(bli)
	{
	  Gtk::Label *lab = dynamic_cast<Gtk::Label*>(list_item->get_child());
	  if(lab)
	    {
	      lab->set_text(bli->bodyname);
	    }
	}
    });

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> body_exp =
      Gtk::ClosureExpression<Glib::ustring>::create([]
      (const Glib::RefPtr<Glib::ObjectBase> &item)
	{
	  Glib::RefPtr<BodyListItem>bli =
	  std::dynamic_pointer_cast<BodyListItem>(item);
	  if(bli)
	    {
	      return bli->bodyname;
	    }
	  else
	    {
	      return Glib::ustring("");
	    }
	});

  Gtk::DropDown *objcomb = Gtk::make_managed<Gtk::DropDown>(bodylist, body_exp);
  objcomb->set_factory(bodyfact);
  objcomb->set_halign(Gtk::Align::START);
  objcomb->set_margin(5);
  objcomb->set_selected(0);
  objcomb->set_enable_search(true);
  grid->attach(*objcomb, 1, 5, 2, 1);

  Gtk::Label *coord = Gtk::make_managed<Gtk::Label>();
  coord->set_text(gettext("Coordinates: "));
  coord->set_halign(Gtk::Align::START);
  coord->set_margin(5);
  grid->attach(*coord, 0, 6, 1, 1);

  list.clear();
  list.push_back(gettext("Equatorial"));
  list.push_back(gettext("Ecliptical"));

  Gtk::DropDown *coordcomb = Gtk::make_managed<Gtk::DropDown>(list);
  coordcomb->set_halign(Gtk::Align::START);
  coordcomb->set_margin(5);
  coordcomb->set_selected(0);
  grid->attach(*coordcomb, 1, 6, 2, 1);

  list.clear();
  list.push_back("X, Y, Z");
  list.push_back("Vx, Vy, Vz");

  Gtk::DropDown *xyzcomb = Gtk::make_managed<Gtk::DropDown>(list);
  xyzcomb->set_halign(Gtk::Align::START);
  xyzcomb->set_margin(5);
  xyzcomb->set_selected(0);
  grid->attach(*xyzcomb, 3, 6, 2, 1);

  Gtk::Label *equin = Gtk::make_managed<Gtk::Label>();
  equin->set_text(gettext("Equator and equinox: "));
  equin->set_halign(Gtk::Align::START);
  equin->set_margin(5);
  grid->attach(*equin, 0, 7, 2, 1);

  list.clear();
  list.push_back(gettext("Mean (J2000)"));
  list.push_back(gettext("Mean of the date (IAU2000)"));
  list.push_back(gettext("True of the date (IAU2000)"));

  Gtk::DropDown *equincomb = Gtk::make_managed<Gtk::DropDown>(list);
  equincomb->set_halign(Gtk::Align::START);
  equincomb->set_margin(5);
  equincomb->set_selected(0);
  grid->attach(*equincomb, 2, 7, 3, 1);

  Gtk::Label *unit = Gtk::make_managed<Gtk::Label>();
  unit->set_text(gettext("Units of measurement: "));
  unit->set_halign(Gtk::Align::START);
  unit->set_margin(5);
  grid->attach(*unit, 0, 8, 2, 1);

  list.clear();
  list.push_back(gettext("Astronomical units"));
  list.push_back(gettext("Kilometers"));
  list.push_back(gettext("Meters"));

  Gtk::DropDown *unitcomb = Gtk::make_managed<Gtk::DropDown>(list);
  unitcomb->set_halign(Gtk::Align::START);
  unitcomb->set_margin(5);
  unitcomb->set_selected(0);
  grid->attach(*unitcomb, 2, 8, 3, 1);
  Glib::PropertyProxy<guint> sel = objcomb->property_selected();
  sel.signal_changed().connect(
      [objcomb, xyzcomb, unitcomb, coord, coordcomb, equin, equincomb]
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
      });

  Glib::PropertyProxy<guint> sel2 = xyzcomb->property_selected();
  sel2.signal_changed().connect([xyzcomb, unitcomb, objcomb]
  {
    std::vector<Glib::ustring> list;
    if(objcomb->get_selected() != 21)
      {
	if(xyzcomb->get_selected() == 0)
	  {
	    list.clear();
	    list.push_back(gettext("Astronomical units"));
	    list.push_back(gettext("Kilometers"));
	    list.push_back(gettext("Meters"));
	    Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
	    unitcomb->set_model(strl);
	    unitcomb->set_selected(0);
	  }
	else if(xyzcomb->get_selected() == 1)
	  {
	    list.clear();
	    list.push_back(gettext("AU/day"));
	    list.push_back(gettext("km/day"));
	    list.push_back(gettext("km/s"));
	    list.push_back(gettext("m/s"));
	    Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
	    unitcomb->set_model(strl);
	    unitcomb->set_selected(0);
	  }
	unitcomb->set_selected(0);
      }
    else
      {
	if(xyzcomb->get_selected() == 0)
	  {
	    list.clear();
	    list.push_back(gettext("rad"));
	    list.push_back(gettext("degrees"));
	    Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
	    unitcomb->set_model(strl);
	    unitcomb->set_selected(0);
	  }
	if(xyzcomb->get_selected() == 1)
	  {
	    list.clear();
	    list.push_back(gettext("rad/day"));
	    list.push_back(gettext("\"/day"));
	    Glib::RefPtr<Gtk::StringList> strl = Gtk::StringList::create(list);
	    unitcomb->set_model(strl);
	    unitcomb->set_selected(0);
	  }
	unitcomb->set_selected(0);
      }
  });

  Gtk::Label *step = Gtk::make_managed<Gtk::Label>();
  step->set_text(gettext("Step size (days): "));
  step->set_halign(Gtk::Align::START);
  step->set_margin(5);
  grid->attach(*step, 0, 9, 1, 1);

  Gtk::Entry *stepent = Gtk::make_managed<Gtk::Entry>();
  stepent->set_halign(Gtk::Align::START);
  stepent->set_margin(5);
  stepent->set_max_width_chars(4);
  stepent->set_alignment(0.5);
  stepent->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*stepent, 1, 9, 1, 1);

  Gtk::Label *stepnum = Gtk::make_managed<Gtk::Label>();
  stepnum->set_text(gettext("Number of steps: "));
  stepnum->set_halign(Gtk::Align::END);
  stepnum->set_margin(5);
  grid->attach(*stepnum, 2, 9, 2, 1);

  Gtk::Entry *stepnument = Gtk::make_managed<Gtk::Entry>();
  stepnument->set_halign(Gtk::Align::START);
  stepnument->set_margin(5);
  stepnument->set_max_width_chars(4);
  stepnument->set_alignment(0.5);
  stepnument->set_input_purpose(Gtk::InputPurpose::DIGITS);
  grid->attach(*stepnument, 4, 9, 1, 1);

  Gtk::Label *pathlab = Gtk::make_managed<Gtk::Label>();
  pathlab->set_halign(Gtk::Align::START);
  pathlab->set_margin(5);
  pathlab->set_text(gettext("Path to ephemerides file:"));
  grid->attach(*pathlab, 0, 10, 2, 1);

  /*
   * pathv types:
   * 1 - ephemeris file, 2 - TT-TDB file, 3 - Moon libration file, 4 - scale
   * factor, 5 - path to small bodies file
   */
  std::vector<std::tuple<uint8_t, std::string> > pathv;
  bool err = false;
  std::string filename(Glib::get_home_dir());
  filename = filename + "/.config/EphEPM/ephpath";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
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

  Gtk::Entry *pathent = Gtk::make_managed<Gtk::Entry>();
  pathent->set_margin(5);
  auto itpv = std::find_if(pathv.begin(), pathv.end(), []
  (auto &el)
    {
      return std::get<0> (el) == 1;
    });
  if(itpv != pathv.end())
    {
      pathent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  grid->attach(*pathent, 0, 11, 3, 1);

  Gtk::Button *openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), pathent));
  grid->attach(*openb, 3, 11, 1, 1);

  Gtk::Button *clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([pathent]
  {
    pathent->set_text("");
  });
  grid->attach(*clearbut, 4, 11, 1, 1);

  pathlab = Gtk::make_managed<Gtk::Label>();
  pathlab->set_halign(Gtk::Align::START);
  pathlab->set_margin(5);
  pathlab->set_text(gettext("Path to TT-TDB file:"));
  grid->attach(*pathlab, 0, 12, 2, 1);

  Gtk::Entry *tttdbent = Gtk::make_managed<Gtk::Entry>();
  tttdbent->set_margin(5);
  itpv = std::find_if(pathv.begin(), pathv.end(), []
  (auto &el)
    {
      return std::get<0> (el) == 2;
    });
  if(itpv != pathv.end())
    {
      tttdbent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  grid->attach(*tttdbent, 0, 13, 3, 1);

  openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), tttdbent));
  grid->attach(*openb, 3, 13, 1, 1);

  clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([tttdbent]
  {
    tttdbent->set_text("");
  });
  grid->attach(*clearbut, 4, 13, 1, 1);

  pathlab = Gtk::make_managed<Gtk::Label>();
  pathlab->set_halign(Gtk::Align::START);
  pathlab->set_margin(5);
  pathlab->set_text(gettext("Path to Moon libration file:"));
  grid->attach(*pathlab, 0, 14, 2, 1);

  Gtk::Entry *mlbent = Gtk::make_managed<Gtk::Entry>();
  mlbent->set_margin(5);
  itpv = std::find_if(pathv.begin(), pathv.end(), []
  (auto &el)
    {
      return std::get<0> (el) == 3;
    });
  if(itpv != pathv.end())
    {
      mlbent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  grid->attach(*mlbent, 0, 15, 3, 1);

  openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), mlbent));
  grid->attach(*openb, 3, 15, 1, 1);

  clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([mlbent]
  {
    mlbent->set_text("");
  });
  grid->attach(*clearbut, 4, 15, 1, 1);

  pathlab = Gtk::make_managed<Gtk::Label>();
  pathlab->set_halign(Gtk::Align::START);
  pathlab->set_margin(5);
  pathlab->set_text(gettext("Path to small bodies file:"));
  grid->attach(*pathlab, 0, 16, 2, 1);

  Gtk::Entry *smlent = Gtk::make_managed<Gtk::Entry>();
  smlent->set_margin(5);
  itpv = std::find_if(pathv.begin(), pathv.end(), []
  (auto &el)
    {
      return std::get<0> (el) == 5;
    });
  if(itpv != pathv.end())
    {
      smlent->set_text(Glib::ustring(std::get<1>(*itpv)));
    }
  grid->attach(*smlent, 0, 17, 3, 1);

  openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), smlent));
  grid->attach(*openb, 3, 17, 1, 1);

  clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_margin(5);
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_name("closeButton");
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect([smlent]
  {
    smlent->set_text("");
  });
  grid->attach(*clearbut, 4, 17, 1, 1);

  Gtk::Label *scale_lab = Gtk::make_managed<Gtk::Label>();
  scale_lab->set_margin(5);
  scale_lab->set_halign(Gtk::Align::START);
  scale_lab->set_text(gettext("Diagram scale factor:"));
  grid->attach(*scale_lab, 0, 18, 2, 1);

  Gtk::Entry *scale_ent = Gtk::make_managed<Gtk::Entry>();
  scale_ent->set_margin(5);
  scale_ent->set_halign(Gtk::Align::FILL);
  itpv = std::find_if(pathv.begin(), pathv.end(), []
  (auto &el)
    {
      return std::get<0> (el) == 4;
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

  grid->attach(*scale_ent, 0, 19, 1, 1);

  Gtk::Button *calc = Gtk::make_managed<Gtk::Button>();
  calc->set_halign(Gtk::Align::CENTER);
  calc->set_margin(5);
  calc->set_label(gettext("Calculate coordinates"));
  calc->set_name("button");
  calc->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::calcCoord), day, month, year,
		 hour, minut, second, timecomb, belt, objcomb, coordcomb,
		 xyzcomb, equincomb, unitcomb, stepent, stepnument, pathent,
		 tttdbent, mlbent, smlent));
  grid->attach(*calc, 0, 20, 2, 1);

  Gtk::Button *orb = Gtk::make_managed<Gtk::Button>();
  orb->set_halign(Gtk::Align::CENTER);
  orb->set_margin(5);
  orb->set_label(gettext("Orbits"));
  orb->set_name("button");
  orb->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::orbitsGraph), day, month,
		 year, hour, minut, second, timecomb, belt, coordcomb,
		 equincomb, pathent, tttdbent, smlent, scale_ent));
  grid->attach(*orb, 2, 20, 1, 1);

  Gtk::Button *about = Gtk::make_managed<Gtk::Button>();
  about->set_halign(Gtk::Align::CENTER);
  about->set_margin(5);
  about->set_label(gettext("About"));
  about->set_name("button");
  about->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::aboutProg));
  grid->attach(*about, 5, 20, 1, 1);

  this->signal_close_request().connect(
      std::bind(&MainWindow::closeFunc, this, pathent, tttdbent, mlbent, smlent,
		scale_ent),
      false);
}

Glib::RefPtr<Gio::ListStore<BodyListItem>>
MainWindow::createBodyList()
{
  Glib::RefPtr<Gio::ListStore<BodyListItem>> result = Gio::ListStore<
      BodyListItem>::create();
  std::string filename = Sharepath + "/Bodylist.csv";
  std::filesystem::path blpath = std::filesystem::u8path(filename);
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
      auto it = std::find_if(item.begin(), item.end(), [lnm]
      (auto &el)
	{
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
	      Glib::RefPtr<BodyListItem> bli = BodyListItem::create(val,
								    naifid);
	      result->append(bli);
	    }
	}
    }

  return result;
}

void
MainWindow::openDialog(Gtk::Entry *pathent)
{
#ifndef EPH_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fcd = Gtk::FileDialog::create();
  fcd->set_modal(true);
  fcd->set_title(gettext("Ephemeris file selection"));
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_initial_folder(fl);
  fcd->set_accept_label(gettext("Select"));
  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fcd->open(*this, [pathent]
  (Glib::RefPtr<Gio::AsyncResult> &result)
    {
      Glib::RefPtr<Gio::File> fl;
      auto obj = result->get_source_object_base();
      auto fchd = std::dynamic_pointer_cast<Gtk::FileDialog> (obj);
      if(!fchd)
	{
	  return void ();
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
	  pathent->set_text(fl->get_path());
	}
    },
	    cncl);
#endif
#ifdef EPH_GTK_OLD
  Gtk::FileChooserDialog *fcd = new Gtk::FileChooserDialog(
      *this, gettext("Ephemeris file selection"),
      Gtk::FileChooser::Action::OPEN, true);
  fcd->set_application(this->get_application());
  fcd->set_modal(true);

  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_current_folder(fl);

  fcd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  fcd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);

  fcd->signal_response().connect([fcd, pathent]
  (int resp_id)
    {
      if(resp_id == Gtk::ResponseType::ACCEPT)
	{
	  Glib::RefPtr<Gio::File> fl = fcd->get_file();
	  if(fl)
	    {
	      pathent->set_text(fl->get_path());
	    }
	}
      fcd->close();
    });

  fcd->signal_close_request().connect([fcd]
  {
    std::shared_ptr<Gtk::FileChooserDialog> fd(fcd);
    fd->set_visible(false);
    return true;
  },
				      false);

  fcd->present();
#endif
}

void
MainWindow::calcCoord(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
		      Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
		      Gtk::DropDown *timecomb, Gtk::DropDown *belt,
		      Gtk::DropDown *objcomb, Gtk::DropDown *coordcomb,
		      Gtk::DropDown *xyzcomb, Gtk::DropDown *equincomb,
		      Gtk::DropDown *unitcomb, Gtk::Entry *stepent,
		      Gtk::Entry *stepnument, Gtk::Entry *pathent,
		      Gtk::Entry *tttdbent, Gtk::Entry *mlbent,
		      Gtk::Entry *smlent)
{
  std::string daystr(day->get_text());
  std::string monthstr(month->get_text());
  std::string yearstr(year->get_text());
  std::string hourstr(hour->get_text());
  std::string minutstr(minut->get_text());
  std::string secondstr(second->get_text());
  Glib::RefPtr<Gio::ListModel> model = belt->get_model();
  Glib::RefPtr<Gtk::StringList> strl =
      std::dynamic_pointer_cast<Gtk::StringList>(model);
  std::string beltstr(strl->get_string(belt->get_selected()));
  std::string stepstr(stepent->get_text());
  std::string stepnumberstr(stepnument->get_text());
  std::string pathstr(pathent->get_text());
  std::string tttdbstr(tttdbent->get_text());
  std::string smlstr(smlent->get_text());
  int naifid = -1;
  Glib::RefPtr<BodyListItem> bli = std::dynamic_pointer_cast<BodyListItem>(
      objcomb->get_selected_item());
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
  AuxFunc af;
  mpf_set_default_prec(512);
  JDshow = af.utcJD(daynum, monthnum, yearnum, hournum, minutnum, secondnum);
  DAFOperations daf;
  bool datech;
  if(!tttdbstr.empty())
    {
      datech = daf.epochCheckUTC(daynum, monthnum, yearnum, hournum, minutnum,
				 secondnum,
				 static_cast<int>(timecomb->get_selected()),
				 beltnum, tttdbstr);
    }
  else
    {
      datech = daf.epochCheckUTC(daynum, monthnum, yearnum, hournum, minutnum,
				 secondnum,
				 static_cast<int>(timecomb->get_selected()),
				 beltnum, pathstr);
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
      xyz, theory, unit, stepnum, stepnumbernum, pathstr, tttdbstr, smlstr,
      &orbits_cancel);
  std::vector<std::array<mpf_class, 3> > *result = new std::vector<
      std::array<mpf_class, 3> >;
  Glib::Dispatcher *result_win_disp = new Glib::Dispatcher;
  result_win_disp->connect(
      [result, this, belt, objcomb, coordcomb, xyzcomb, equincomb, unitcomb,
       result_win_disp, info_win]
      {
	info_win->close();
	this->resultPresenting(result, belt, objcomb, coordcomb, xyzcomb,
			       equincomb, unitcomb, result_win_disp);
      });

  std::thread *coordthr = new std::thread([calc, result_win_disp, result]
  {
    *result = calc->calculationsXYZ();
    delete calc;
    result_win_disp->emit();
  });
  coordthr->detach();
  delete coordthr;
}

void
MainWindow::errDialog(int variant)
{
  Glib::ustring msgtxt;
  if(variant == 1)
    {
      msgtxt = gettext("Wrong day input!");
    }
  else if(variant == 2)
    {
      msgtxt = gettext("Wrong month input!");
    }
  else if(variant == 3)
    {
      msgtxt = gettext("Wrong year input!");
    }
  else if(variant == 4)
    {
      msgtxt = gettext("Wrong hour input!");
    }
  else if(variant == 5)
    {
      msgtxt = gettext("Wrong minutes input!");
    }
  else if(variant == 6)
    {
      msgtxt = gettext("Wrong seconds input!");
    }
  else if(variant == 7)
    {
      msgtxt = gettext("Wrong step input!");
    }
  else if(variant == 8)
    {
      msgtxt = gettext("Wrong step number input!");
    }
  else if(variant == 9)
    {
      msgtxt = gettext("Date is out of ephemeris time interval or "
		       "TT-TDB amendments was not found!");
    }
  else if(variant == 10)
    {
      msgtxt = gettext("Critical error!");
    }
  else if(variant == 11)
    {
      msgtxt = gettext("Incorrect ephemeris file!");
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

  window->signal_close_request().connect([window]
  {
    std::shared_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    return true;
  },
					 false);

  window->present();
}

Gtk::Window*
MainWindow::resultPulseWin(int variant, Gtk::ProgressBar *bar)
{
  orbits_cancel.store(0);
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

  if(variant == 0)
    {
      Gtk::Label *calclab = Gtk::make_managed<Gtk::Label>();
      calclab->set_halign(Gtk::Align::CENTER);
      calclab->set_margin(5);
      calclab->set_text(gettext("Calculation in progress..."));
      grid->attach(*calclab, 0, 0, 1, 1);
    }
  else if(variant == 1)
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
      cancel->signal_clicked().connect([this, cancel, calclab, bar]
      {
	this->orbits_cancel.store(1);
	cancel->set_visible(false);
	bar->set_visible(false);
	calclab->set_text(gettext("Canceling..."));
      });
      grid->attach(*cancel, 0, 2, 1, 1);
    }

  window->signal_close_request().connect([window]
  {
    std::shared_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    return true;
  },
					 false);
  window->present();

  return window;
}

void
MainWindow::resultPresenting(std::vector<std::array<mpf_class, 3> > *result,
			     Gtk::DropDown *belt, Gtk::DropDown *objcomb,
			     Gtk::DropDown *coordcomb, Gtk::DropDown *xyzcomb,
			     Gtk::DropDown *equincomb, Gtk::DropDown *unitcomb,
			     Glib::Dispatcher *result_win_disp)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(this->get_application());
  window->set_name("mainWindow");
  window->set_title(gettext("Result"));
  window->set_transient_for(*this);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  grid->set_column_homogeneous(true);
  window->set_child(*grid);

  Gtk::Label *objlab = Gtk::make_managed<Gtk::Label>();
  objlab->set_halign(Gtk::Align::START);
  objlab->set_margin(5);
  Glib::RefPtr<BodyListItem> bli = std::dynamic_pointer_cast<BodyListItem>(
      objcomb->get_selected_item());
  if(bli)
    {
      objlab->set_text(Glib::ustring(gettext("Object: ") + bli->bodyname));
    }
  else
    {
      objlab->set_text(
	  Glib::ustring(
	      gettext("Object: ") + Glib::ustring(gettext("Error!"))));
    }
  grid->attach(*objlab, 0, 0, 1, 1);

  Gtk::Label *coordlab = Gtk::make_managed<Gtk::Label>();
  coordlab->set_halign(Gtk::Align::START);
  coordlab->set_margin(5);
  Glib::RefPtr<Gio::ListModel> lmodel = coordcomb->get_model();
  Glib::RefPtr<Gtk::StringList> strl =
      std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  coordlab->set_text(
      Glib::ustring(
	  gettext("Coordinates: ")
	      + strl->get_string(coordcomb->get_selected())));
  grid->attach(*coordlab, 0, 1, 1, 1);

  Gtk::Label *equinlab = Gtk::make_managed<Gtk::Label>();
  equinlab->set_halign(Gtk::Align::START);
  equinlab->set_margin(5);
  lmodel = equincomb->get_model();
  strl = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  equinlab->set_text(
      Glib::ustring(
	  gettext("Equator and equinox: ")
	      + strl->get_string(equincomb->get_selected())));
  grid->attach(*equinlab, 0, 2, 1, 1);

  Gtk::Label *unitlab = Gtk::make_managed<Gtk::Label>();
  unitlab->set_halign(Gtk::Align::START);
  unitlab->set_margin(5);
  lmodel = unitcomb->get_model();
  strl = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  unitlab->set_text(
      Glib::ustring(
	  gettext("Units of measurement: ")
	      + strl->get_string(unitcomb->get_selected())));
  grid->attach(*unitlab, 0, 3, 1, 1);

  Gtk::Label *beltlab = Gtk::make_managed<Gtk::Label>();
  beltlab->set_halign(Gtk::Align::START);
  beltlab->set_margin(5);
  lmodel = belt->get_model();
  strl = std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
  beltlab->set_text(
      Glib::ustring(
	  gettext("Hour belt: ") + strl->get_string(belt->get_selected())));
  grid->attach(*beltlab, 0, 4, 1, 1);

  Glib::RefPtr<Gio::ListStore<ModelColumns> > store = Gio::ListStore<
      ModelColumns>::create();
  std::stringstream strm;
  std::locale loc("C");
  AuxFunc af;
  for(size_t i = 0; i < result->size(); i++)
    {
      int ych, mch, dch, hch, minch;
      double secch;
      af.dateJulian(JDshow + i * stepnum, &dch, &mch, &ych, &hch, &minch,
		    &secch);
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
      strm << std::fixed << std::setprecision(20) << std::get<0>(result->at(i));
      std::string X = strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20) << std::get<1>(result->at(i));
      std::string Y = strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20) << std::get<2>(result->at(i));
      std::string Z = strm.str();
      Glib::RefPtr<ModelColumns> row = ModelColumns::create(datestr, X, Y, Z);
      store->append(row);
    }

  Glib::RefPtr<Gtk::NoSelection> selection = Gtk::NoSelection::create(store);

  Glib::RefPtr<Gtk::SignalListItemFactory> factory =
      Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_valign(Gtk::Align::CENTER);
      lab->set_text("");
      item->set_child(*lab);
    });

  factory->signal_bind().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Glib::RefPtr<ModelColumns> row
      = std::dynamic_pointer_cast<ModelColumns> (item->get_item());
      Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
      lab->set_text(row->date);
    });
  Glib::RefPtr<Gtk::ColumnViewColumn> col_date = Gtk::ColumnViewColumn::create(
      gettext("Date and time(local)"), factory);

  Glib::RefPtr<Gtk::SignalListItemFactory> factoryX =
      Gtk::SignalListItemFactory::create();
  factoryX->signal_setup().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_valign(Gtk::Align::CENTER);
      lab->set_text("");
      item->set_child(*lab);
    });

  factoryX->signal_bind().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Glib::RefPtr<ModelColumns> row
      = std::dynamic_pointer_cast<ModelColumns> (item->get_item());
      Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
      lab->set_text(row->x);
    });

  Glib::RefPtr<Gtk::SignalListItemFactory> factoryY =
      Gtk::SignalListItemFactory::create();
  factoryY->signal_setup().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_valign(Gtk::Align::CENTER);
      lab->set_text("");
      item->set_child(*lab);
    });

  factoryY->signal_bind().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Glib::RefPtr<ModelColumns> row
      = std::dynamic_pointer_cast<ModelColumns> (item->get_item());
      Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
      lab->set_text(row->y);
    });

  Glib::RefPtr<Gtk::SignalListItemFactory> factoryZ =
      Gtk::SignalListItemFactory::create();
  factoryZ->signal_setup().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label> ();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_valign(Gtk::Align::CENTER);
      lab->set_text("");
      item->set_child(*lab);
    });

  factoryZ->signal_bind().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Glib::RefPtr<ModelColumns> row
      = std::dynamic_pointer_cast<ModelColumns> (item->get_item());
      Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
      lab->set_text(row->z);
    });

  Glib::RefPtr<Gtk::ColumnViewColumn> col_X;
  Glib::RefPtr<Gtk::ColumnViewColumn> col_Y;
  Glib::RefPtr<Gtk::ColumnViewColumn> col_Z;
  std::string header_line(gettext("Date and time(local)"));
  if(objcomb->get_selected() != 21)
    {
      if(xyzcomb->get_selected() == 0)
	{
	  col_X = Gtk::ColumnViewColumn::create("X", factoryX);
	  col_Y = Gtk::ColumnViewColumn::create("Y", factoryY);
	  col_Z = Gtk::ColumnViewColumn::create("Z", factoryZ);
	  header_line = header_line + ";X;Y;Z;";
	}
      else if(xyzcomb->get_selected() == 1)
	{
	  col_X = Gtk::ColumnViewColumn::create("Vx", factoryX);
	  col_Y = Gtk::ColumnViewColumn::create("Vy", factoryY);
	  col_Z = Gtk::ColumnViewColumn::create("Vz", factoryZ);
	  header_line = header_line + ";Vx;Vy;Vz;";
	}
    }
  else
    {
      if(xyzcomb->get_selected() == 0)
	{
	  col_X = Gtk::ColumnViewColumn::create("φ", factoryX);
	  col_Y = Gtk::ColumnViewColumn::create("θ", factoryY);
	  col_Z = Gtk::ColumnViewColumn::create("ψ", factoryZ);
	  header_line = header_line + ";φ;θ;ψ;";
	}
      if(xyzcomb->get_selected() == 1)
	{
	  col_X = Gtk::ColumnViewColumn::create("φ'", factoryX);
	  col_Y = Gtk::ColumnViewColumn::create("θ'", factoryY);
	  col_Z = Gtk::ColumnViewColumn::create("ψ'", factoryZ);
	  header_line = header_line + ";φ';θ';ψ';";
	}
    }

  Gtk::ColumnView *columnv = Gtk::make_managed<Gtk::ColumnView>();
  columnv->set_show_row_separators(true);
  columnv->set_show_column_separators(true);
  columnv->set_model(selection);
  columnv->append_column(col_date);
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
  save->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &MainWindow::saveDialog), window, objlab,
		 coordlab, equinlab, unitlab, beltlab, columnv, objcomb,
		 header_line));
  grid->attach(*save, 0, 6, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_name("closeButton");
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect([window]
  {
    window->close();
  });
  grid->attach(*close, 1, 6, 1, 1);

  if(objcomb->get_selected() == 21)
    {
      grid->remove(*coordlab);
      grid->remove(*equinlab);
    }

  columnv->signal_realize().connect([columnv, scrl, this]
  {
    Glib::RefPtr<Gdk::Surface> surf = this->get_surface();
    Glib::RefPtr<Gdk::Display> disp = this->get_display();
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

  window->signal_close_request().connect([window, result_win_disp]
  {
    delete result_win_disp;
    window->set_visible(false);
    delete window;
    return true;
  },
					 false);
  window->present();
  delete result;
}

void
MainWindow::saveDialog(Gtk::Window *win, Gtk::Label *objlab,
		       Gtk::Label *coordlab, Gtk::Label *equinlab,
		       Gtk::Label *unitlab, Gtk::Label *beltlab,
		       Gtk::ColumnView *view, Gtk::DropDown *objcomb,
		       std::string header_line)
{
#ifndef EPH_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fcd = Gtk::FileDialog::create();
  fcd->set_title(gettext("Save result"));
  fcd->set_modal(true);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_initial_folder(fl);
  fcd->set_initial_name("result.csv");
  Glib::RefPtr<Gio::Cancellable> cancel = Gio::Cancellable::create();
  fcd->save(
      *win,
      [this, objlab, coordlab, equinlab, unitlab, beltlab, view, objcomb,
       header_line]
      (const Glib::RefPtr<Gio::AsyncResult> &result)
	{
	  Glib::RefPtr<Gio::File> fl;
	  auto obj = result->get_source_object_base();
	  auto fchd = std::dynamic_pointer_cast<Gtk::FileDialog> (obj);
	  if(!fchd)
	    {
	      return void ();
	    }
	  try
	    {
	      fl = fchd->save_finish(result);
	    }
	  catch(Glib::Error &e)
	    {
	      if(e.code() != Gtk::DialogError::DISMISSED)
		{
		  std::cout << "MainWindow::saveDialog:" << e.what()
		  << std::endl;
		}
	    }
	  if(fl)
	    {
	      this->saveDialogFunc(fl, objlab, coordlab, equinlab, unitlab,
		  beltlab, view, objcomb, header_line);
	    }
	},
      cancel);
#endif
#ifdef EPH_GTK_OLD
  Gtk::FileChooserDialog *fcd = new Gtk::FileChooserDialog(
      *win, gettext("Save result"), Gtk::FileChooser::Action::SAVE, true);
  fcd->set_application(win->get_application());
  fcd->set_modal(true);

  fcd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  fcd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);

  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(Glib::get_home_dir());
  fcd->set_current_folder(fl);
  fcd->set_current_name("result.csv");

  fcd->signal_response().connect(
      [fcd, this, objlab, coordlab, equinlab, unitlab, beltlab, view, objcomb,
       header_line]
      (int resp_id)
	{
	  if(resp_id == Gtk::ResponseType::ACCEPT)
	    {
	      Glib::RefPtr<Gio::File> fl = fcd->get_file();
	      if(fl)
		{
		  this->saveDialogFunc(fl, objlab, coordlab, equinlab, unitlab,
		      beltlab, view, objcomb, header_line);
		}
	    }
	  fcd->close();
	});

  fcd->signal_close_request().connect([fcd]
  {
    std::shared_ptr<Gtk::FileChooserDialog> fd(fcd);
    fd->set_visible(false);
    return true;
  },
				      false);

  fcd->present();
#endif
}

void
MainWindow::saveDialogFunc(Glib::RefPtr<Gio::File> fl, Gtk::Label *objlab,
			   Gtk::Label *coordlab, Gtk::Label *equinlab,
			   Gtk::Label *unitlab, Gtk::Label *beltlab,
			   Gtk::ColumnView *view, Gtk::DropDown *objcomb,
			   std::string header_line)
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
      if(objcomb->get_selected() != 21)
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

      Glib::RefPtr<Gtk::NoSelection> model = std::dynamic_pointer_cast<
	  Gtk::NoSelection>(view->get_model());
      if(model)
	{
	  Glib::RefPtr<Gio::ListModel> mod = model->get_model();
	  Glib::RefPtr<Gio::ListStore<ModelColumns> > store =
	      std::dynamic_pointer_cast<Gio::ListStore<ModelColumns> >(mod);
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

void
MainWindow::orbitsGraph(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
			Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
			Gtk::DropDown *timecomb, Gtk::DropDown *belt,
			Gtk::DropDown *coordcomb, Gtk::DropDown *equincomb,
			Gtk::Entry *pathent, Gtk::Entry *tttdbent,
			Gtk::Entry *smlent, Gtk::Entry *scale_ent)
{
  std::string daystr(day->get_text());
  std::string monthstr(month->get_text());
  std::string yearstr(year->get_text());
  std::string hourstr(hour->get_text());
  std::string minutstr(minut->get_text());
  std::string secondstr(second->get_text());
  Glib::RefPtr<Gio::ListModel> lmodel = belt->get_model();
  Glib::RefPtr<Gtk::StringList> strl =
      std::dynamic_pointer_cast<Gtk::StringList>(lmodel);
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
  double JDcalc = af.utcJD(daynum, monthnum, yearnum, hournum, minutnum,
			   secondnum);

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
      chtm = daf.epochCheckUTC(daynum, monthnum, yearnum, hournum, minutnum,
			       secondnum,
			       static_cast<int>(timecomb->get_selected()),
			       beltnum, pathstr);
    }
  else
    {
      chtm = daf.epochCheckUTC(daynum, monthnum, yearnum, hournum, minutnum,
			       secondnum,
			       static_cast<int>(timecomb->get_selected()),
			       beltnum, tttdbstr);
    }
  if(chtm)
    {
      Gtk::ProgressBar *bar = Gtk::make_managed<Gtk::ProgressBar>();
      bar->set_fraction(0.0);
      OrbitsDiagram *od = new OrbitsDiagram(
	  this, pathstr, tttdbstr, smlstr, JDcalc,
	  static_cast<int>(timecomb->get_selected()),
	  static_cast<int>(coordcomb->get_selected()),
	  static_cast<int>(equincomb->get_selected()), plot_fact,
	  &orbits_cancel);
      if(orbits_cancel.load() != 0)
	{
	  delete od;
	  errDialog(11);
	  orbits_cancel.store(0);
	  return void();
	}

      Gtk::Window *win = resultPulseWin(1, bar);
      double sz = static_cast<double>(od->calculateSize());
      std::shared_ptr<Glib::Dispatcher> pulse_disp = std::make_shared<
	  Glib::Dispatcher>();
      std::shared_ptr<Glib::Dispatcher> compl_disp = std::make_shared<
	  Glib::Dispatcher>();
      std::shared_ptr<Glib::Dispatcher> canceled_disp = std::make_shared<
	  Glib::Dispatcher>();
      std::shared_ptr<std::mutex> pulsemtx = std::make_shared<std::mutex>();
      std::shared_ptr<double> frac = std::make_shared<double>(0.0);
      pulse_disp->connect([bar, frac]
      {
	bar->set_fraction(*frac);
      });

      od->pulse_signal = [pulse_disp, frac, sz, pulsemtx, bar]
      {
	pulsemtx->lock();
	*frac = *frac + 1.0 / sz;
	if(*frac - bar->get_fraction() > 0.01)
	  {
	    pulse_disp->emit();
	  }
	pulsemtx->unlock();
      };

      od->diagram_close = [od]
      {
	delete od;
      };

      compl_disp->connect([bar, win, od]
      {
	bar->set_fraction(1.0);
	win->close();
	od->diagramPlot();
      });

      canceled_disp->connect([win, od]
      {
	win->close();
	delete od;
      });

      od->calc_completed = [compl_disp]
      {
	compl_disp->emit();
      };
      od->canceled_signal = [canceled_disp]
      {
	canceled_disp->emit();
      };

      std::thread *thr = new std::thread([od]
      {
	od->calculateOrbits();
      });
      thr->detach();
      delete thr;
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
  aboutd->set_version("2.1");
  aboutd->set_copyright(
      "Copyright 2022-2024 Yury Bobylev <bobilev_yury@mail.ru>");
  AuxFunc af;
  std::filesystem::path p = std::filesystem::u8path(af.get_selfpath());
  std::string filename = Sharepath + "/COPYING";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
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

  filename = Sharepath + "/ico.png";
  Glib::RefPtr<Gio::File> logofile = Gio::File::create_for_path(filename);
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

  aboutd->signal_close_request().connect([aboutd]
  {
    aboutd->set_visible(false);
    delete aboutd;
    return true;
  },
					 false);
  aboutd->present();
}

bool
MainWindow::closeFunc(Gtk::Entry *pathent, Gtk::Entry *tttdbent,
		      Gtk::Entry *mlbent, Gtk::Entry *smlent,
		      Gtk::Entry *scale_ent)
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
  std::vector<std::tuple<uint8_t, std::string> > pathv;
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
