/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

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

#include "MainWindow.h"

MainWindow::MainWindow()
{
  AuxFunc af;
  std::filesystem::path p(std::filesystem::u8path(af.get_selfpath()));
  Sharepath = p.parent_path().u8string() + "/../share/EphEPM";
  css_provider = Gtk::CssProvider::create();
  css_provider->load_from_path(Glib::ustring(Sharepath + "/mainWindow.css"));
  createWindow();
}

MainWindow::~MainWindow()
{
  // TODO Auto-generated destructor stub
}

void
MainWindow::createWindow()
{
  this->set_title("EphEPM");
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

  Gtk::ComboBoxText *timecomb = Gtk::make_managed<Gtk::ComboBoxText>();
  timecomb->append("UTC");
  timecomb->append("TT");
  timecomb->append("TDB");
  timecomb->set_halign(Gtk::Align::START);
  timecomb->set_margin(5);
  timecomb->set_active(0);
  grid->attach(*timecomb, 1, 3, 1, 1);

  Gtk::Label *beltlab = Gtk::make_managed<Gtk::Label>();
  beltlab->set_text(gettext("Hour belt: "));
  beltlab->set_halign(Gtk::Align::START);
  beltlab->set_margin(5);
  grid->attach(*beltlab, 2, 3, 2, 1);

  Gtk::ComboBoxText *belt = Gtk::make_managed<Gtk::ComboBoxText>();
  std::vector<int> beltv;
  std::stringstream strm;
  std::locale loc("C");
  std::string beltstr;

  for(int i = -12; i <= 12; i++)
    {
      beltv.push_back(i);
    }
  for(size_t i = 0; i < beltv.size(); i++)
    {
      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << beltv[i];
      if(i <= 12)
        {
          beltstr = strm.str();
        }
      else
        {
          beltstr = "+" + strm.str();
        }
      belt->append(Glib::ustring(beltstr));
    }
  belt->set_halign(Gtk::Align::START);
  belt->set_margin(5);
  belt->set_active(12);
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

  Gtk::ComboBoxText *objcomb = Gtk::make_managed<Gtk::ComboBoxText>();
  objcomb->append(gettext("Sun"));
  objcomb->append(gettext("Mercury"));
  objcomb->append(gettext("Venus"));
  objcomb->append(gettext("Earth"));
  objcomb->append(gettext("Mars"));
  objcomb->append(gettext("Jupiter"));
  objcomb->append(gettext("Saturn"));
  objcomb->append(gettext("Uranus"));
  objcomb->append(gettext("Neptune"));
  objcomb->append(gettext("Pluto"));
  objcomb->append(gettext("Moon"));
  objcomb->append(gettext("Moon(geocentric)"));
  objcomb->append(gettext("Ceres"));
  objcomb->append(gettext("Pallas"));
  objcomb->append(gettext("Vesta"));
  objcomb->append(gettext("Erida"));
  objcomb->append(gettext("Haumea"));
  objcomb->append(gettext("Makemake"));
  objcomb->append(gettext("Sedna"));
  objcomb->append(gettext("Bamberga"));
  objcomb->append(gettext("Iris"));
  objcomb->append(gettext("Lunar physical libration"));
  objcomb->set_halign(Gtk::Align::START);
  objcomb->set_margin(5);
  objcomb->set_active(0);
  grid->attach(*objcomb, 1, 5, 2, 1);

  Gtk::Label *coord = Gtk::make_managed<Gtk::Label>();
  coord->set_text(gettext("Coordinates: "));
  coord->set_halign(Gtk::Align::START);
  coord->set_margin(5);
  grid->attach(*coord, 0, 6, 1, 1);

  Gtk::ComboBoxText *coordcomb = Gtk::make_managed<Gtk::ComboBoxText>();
  coordcomb->append(gettext("Equatorial"));
  coordcomb->append(gettext("Ecliptical"));
  coordcomb->set_halign(Gtk::Align::START);
  coordcomb->set_margin(5);
  coordcomb->set_active(0);
  grid->attach(*coordcomb, 1, 6, 2, 1);

  Gtk::ComboBoxText *xyzcomb = Gtk::make_managed<Gtk::ComboBoxText>();
  xyzcomb->append("X, Y, Z");
  xyzcomb->append("Vx, Vy, Vz");
  xyzcomb->set_halign(Gtk::Align::START);
  xyzcomb->set_margin(5);
  xyzcomb->set_active(0);
  grid->attach(*xyzcomb, 3, 6, 2, 1);

  Gtk::Label *equin = Gtk::make_managed<Gtk::Label>();
  equin->set_text(gettext("Equator and equinox: "));
  equin->set_halign(Gtk::Align::START);
  equin->set_margin(5);
  grid->attach(*equin, 0, 7, 2, 1);

  Gtk::ComboBoxText *equincomb = Gtk::make_managed<Gtk::ComboBoxText>();
  equincomb->append(gettext("Mean (J2000)"));
  equincomb->append(gettext("Mean of the date (IAU2000)"));
  equincomb->append(gettext("True of the date (IAU2000)"));
  equincomb->set_halign(Gtk::Align::START);
  equincomb->set_margin(5);
  equincomb->set_active(0);
  grid->attach(*equincomb, 2, 7, 3, 1);

  Gtk::Label *unit = Gtk::make_managed<Gtk::Label>();
  unit->set_text(gettext("Units of measurement: "));
  unit->set_halign(Gtk::Align::START);
  unit->set_margin(5);
  grid->attach(*unit, 0, 8, 2, 1);

  Gtk::ComboBoxText *unitcomb = Gtk::make_managed<Gtk::ComboBoxText>();
  unitcomb->append(gettext("Astronomical units"));
  unitcomb->append(gettext("Kilometers"));
  unitcomb->append(gettext("Meters"));
  unitcomb->set_halign(Gtk::Align::START);
  unitcomb->set_margin(5);
  unitcomb->set_active(0);
  grid->attach(*unitcomb, 2, 8, 3, 1);
  objcomb->signal_changed().connect(
    [objcomb, xyzcomb, unitcomb, coord, coordcomb, equin, equincomb]
  {
    xyzcomb->remove_all();
    unitcomb->remove_all();
    if(objcomb->get_active_row_number() == 21)
      {
        xyzcomb->append("φ, θ, ψ");
        xyzcomb->append("φ', θ', ψ'");
        unitcomb->append(gettext("rad"));
        unitcomb->append(gettext("degrees"));
        coord->set_opacity(0);
        coordcomb->set_opacity(0);
        equin->set_opacity(0);
        equincomb->set_opacity(0);
      }
    else
      {
        xyzcomb->append("X, Y, Z");
        xyzcomb->append("Vx, Vy, Vz");
        unitcomb->append(gettext("Astronomical units"));
        unitcomb->append(gettext("Kilometers"));
        unitcomb->append(gettext("Meters"));
        coord->set_opacity(1);
        coordcomb->set_opacity(1);
        equin->set_opacity(1);
        equincomb->set_opacity(1);
      }
    xyzcomb->set_active(0);
    unitcomb->set_active(0);
  });

  xyzcomb->signal_changed().connect([xyzcomb, unitcomb, objcomb]
  {
    if(objcomb->get_active_row_number() != 21)
      {
        unitcomb->remove_all();
        if(xyzcomb->get_active_row_number() == 0)
          {
            unitcomb->append(gettext("Astronomical units"));
            unitcomb->append(gettext("Kilometers"));
            unitcomb->append(gettext("Meters"));
          }
        if(xyzcomb->get_active_row_number() == 1)
          {
            unitcomb->append(gettext("AU/day"));
            unitcomb->append(gettext("km/day"));
            unitcomb->append(gettext("km/s"));
            unitcomb->append(gettext("m/s"));
          }
        unitcomb->set_active(0);
      }
    else
      {
        unitcomb->remove_all();
        if(xyzcomb->get_active_row_number() == 0)
          {
            unitcomb->append(gettext("rad"));
            unitcomb->append(gettext("degrees"));
          }
        if(xyzcomb->get_active_row_number() == 1)
          {
            unitcomb->append(gettext("rad/day"));
            unitcomb->append(gettext("\"/day"));
          }
        unitcomb->set_active(0);
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

  Gtk::Entry *pathent = Gtk::make_managed<Gtk::Entry>();
  pathent->set_margin_start(5);
  pathent->set_margin_end(5);
  pathent->set_margin_bottom(5);
  std::fstream f;
  std::string filename(Glib::get_home_dir());
  filename = filename + "/.config/EphEPM/ephpath";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  f.open(filename, std::ios_base::in | std::ios_base::binary);
  if(f.is_open() && std::filesystem::file_size(filepath) > 0)
    {
      std::string line;
      line.resize(std::filesystem::file_size(filepath));
      f.read(&line[0], line.size());
      f.close();
      pathent->set_text(Glib::ustring(line));
    }
  grid->attach(*pathent, 0, 11, 3, 1);

  Gtk::Button *openb = Gtk::make_managed<Gtk::Button>();
  openb->set_margin_start(5);
  openb->set_margin_end(5);
  openb->set_margin_bottom(5);
  openb->set_halign(Gtk::Align::CENTER);
  openb->set_label(gettext("Open"));
  openb->set_name("open_button");
  openb->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  openb->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialog), pathent));
  grid->attach(*openb, 3, 11, 1, 1);

  Gtk::Button *calc = Gtk::make_managed<Gtk::Button>();
  calc->set_halign(Gtk::Align::CENTER);
  calc->set_margin(5);
  calc->set_label(gettext("Calculate coordinates"));
  calc->set_name("button");
  calc->get_style_context()->add_provider(css_provider,
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
  calc->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::calcCoord), day, month,
               year, hour, minut, second, timecomb, belt, objcomb, coordcomb, xyzcomb,
               equincomb, unitcomb, stepent, stepnument, pathent));
  grid->attach(*calc, 0, 12, 2, 1);

  Gtk::Button *orb = Gtk::make_managed<Gtk::Button>();
  orb->set_halign(Gtk::Align::CENTER);
  orb->set_margin(5);
  orb->set_label(gettext("Orbits"));
  orb->set_name("button");
  orb->get_style_context()->add_provider(css_provider,
                                         GTK_STYLE_PROVIDER_PRIORITY_USER);
  orb->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                &MainWindow::orbitsGraph), day, month, year,
                                hour, minut, second, timecomb, belt, coordcomb, equincomb, pathent));
  grid->attach(*orb, 2, 12, 1, 1);

  Gtk::Button *about = Gtk::make_managed<Gtk::Button>();
  about->set_halign(Gtk::Align::CENTER);
  about->set_margin(5);
  about->set_label(gettext("About"));
  about->set_name("button");
  about->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  about->signal_clicked().connect(
    sigc::mem_fun(*this, &MainWindow::aboutProg));
  grid->attach(*about, 3, 12, 1, 1);

  this->signal_close_request().connect([this, pathent]
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
    std::string line(pathent->get_text());
    if(!line.empty())
      {
        std::fstream f;
        f.open(filepath, std::ios_base::out | std::ios_base::binary);
        f.write(line.c_str(), line.size());
        f.close();
      }
    this->hide();
    return true;
  },
  false);
}

void
MainWindow::openDialog(Gtk::Entry *pathent)
{
  Glib::RefPtr<Gtk::FileChooserNative> fcd = Gtk::FileChooserNative::create(
        gettext("Directory selection"), *this, Gtk::FileChooser::Action::OPEN,
        gettext("Select"), gettext("Cancel"));
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_parse_name(
                                 Glib::get_home_dir());
  if(fl)
    {
      fcd->set_current_folder(fl);
    }

  fcd->signal_response().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::openDialogFunc), fcd,
               pathent));

  fcd->show();
}
void
MainWindow::openDialogFunc(int rid, Glib::RefPtr<Gtk::FileChooserNative> fcd,
                           Gtk::Entry *pathent)
{
  if(rid == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fcd->get_file();
      std::string filename = fl->get_path();
      pathent->set_text(Glib::ustring(filename));
    }
}

void
MainWindow::calcCoord(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
                      Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
                      Gtk::ComboBoxText *timecomb,
                      Gtk::ComboBoxText *belt, Gtk::ComboBoxText *objcomb,
                      Gtk::ComboBoxText *coordcomb, Gtk::ComboBoxText *xyzcomb,
                      Gtk::ComboBoxText *equincomb,
                      Gtk::ComboBoxText *unitcomb, Gtk::Entry *stepent,
                      Gtk::Entry *stepnument, Gtk::Entry *pathent)
{
  std::string daystr(day->get_text());
  std::string monthstr(month->get_text());
  std::string yearstr(year->get_text());
  std::string hourstr(hour->get_text());
  std::string minutstr(minut->get_text());
  std::string secondstr(second->get_text());
  std::string beltstr(belt->get_active_text());
  std::string stepstr(stepent->get_text());
  std::string stepnumberstr(stepnument->get_text());
  std::string pathstr(pathent->get_text());
  int objnum = objcomb->get_active_row_number();
  int coordtype = coordcomb->get_active_row_number();
  int xyz = xyzcomb->get_active_row_number();
  int theory = equincomb->get_active_row_number();
  int unit = unitcomb->get_active_row_number();
  std::string objname;
  if(objnum == 0)
    {
      objname = "sun";
    }
  if(objnum == 1)
    {
      objname = "mercury";
    }
  if(objnum == 2)
    {
      objname = "venus";
    }
  if(objnum == 3)
    {
      objname = "earth";
    }
  if(objnum == 4)
    {
      objname = "mars";
    }
  if(objnum == 5)
    {
      objname = "jupiter";
    }
  if(objnum == 6)
    {
      objname = "saturn";
    }
  if(objnum == 7)
    {
      objname = "uranus";
    }
  if(objnum == 8)
    {
      objname = "neptune";
    }
  if(objnum == 9)
    {
      objname = "pluto";
    }
  if(objnum == 10)
    {
      objname = "moon";
    }
  if(objnum == 11)
    {
      objname = "moongeo";
    }
  if(objnum == 12)
    {
      objname = "ceres";
    }
  if(objnum == 13)
    {
      objname = "pallas";
    }
  if(objnum == 14)
    {
      objname = "vesta";
    }
  if(objnum == 15)
    {
      objname = "erida";
    }
  if(objnum == 16)
    {
      objname = "haumea";
    }
  if(objnum == 17)
    {
      objname = "makemake";
    }
  if(objnum == 18)
    {
      objname = "sedna";
    }
  if(objnum == 19)
    {
      objname = "bamberga";
    }
  if(objnum == 20)
    {
      objname = "iris";
    }
  if(objnum == 21)
    {
      objname = "moonlibr";
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
  bool datech = daf.epochCheckUTC(daynum, monthnum, yearnum, hournum, minutnum,
                                  secondnum, timecomb->get_active_row_number(), beltnum, pathstr);
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
  Coordinates *calc = new Coordinates(objname, JDcalc,
                                      timecomb->get_active_row_number(), coordtype, xyz, theory,
                                      unit, stepnum, stepnumbernum, pathstr, &orbits_cancel);
  std::vector<std::array<mpf_class, 3>> *result =
                                       new std::vector<std::array<mpf_class, 3>>;
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
  Gtk::Window *window = new Gtk::Window;
  window->set_application(this->get_application());
  window->set_title(gettext("Error!"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  window->set_child(*grid);
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);

  Gtk::Label *errtxt = Gtk::make_managed<Gtk::Label>();
  errtxt->set_halign(Gtk::Align::CENTER);
  errtxt->set_margin(5);
  if(variant == 1)
    {
      errtxt->set_text(gettext("Wrong day input!"));
    }
  if(variant == 2)
    {
      errtxt->set_text(gettext("Wrong month input!"));
    }
  if(variant == 3)
    {
      errtxt->set_text(gettext("Wrong year input!"));
    }
  if(variant == 4)
    {
      errtxt->set_text(gettext("Wrong hour input!"));
    }
  if(variant == 5)
    {
      errtxt->set_text(gettext("Wrong minutes input!"));
    }
  if(variant == 6)
    {
      errtxt->set_text(gettext("Wrong seconds input!"));
    }
  if(variant == 7)
    {
      errtxt->set_text(gettext("Wrong step input!"));
    }
  if(variant == 8)
    {
      errtxt->set_text(gettext("Wrong step number input!"));
    }
  if(variant == 9)
    {
      errtxt->set_text(gettext("Date is out of ephemeris time interval!"));
    }
  grid->attach(*errtxt, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(
    sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*close, 0, 1, 1, 1);

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
  false);
  window->show();
}

Gtk::Window *
MainWindow::resultPulseWin(int variant, Gtk::ProgressBar *bar)
{
  orbits_cancel = 0;
  Gtk::Window *window = new Gtk::Window;
  window->set_application(this->get_application());
  window->set_title(gettext("Processing..."));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  window->set_child(*grid);
  window->set_modal(true);
  window->set_deletable(false);
  window->set_transient_for(*this);

  if(variant == 0)
    {
      Gtk::Label *calclab = Gtk::make_managed<Gtk::Label>();
      calclab->set_halign(Gtk::Align::CENTER);
      calclab->set_margin(5);
      calclab->set_text(gettext("Calculation in progress..."));
      grid->attach(*calclab, 0, 0, 1, 1);
    }
  if(variant == 1)
    {
      Gtk::Label *calclab = Gtk::make_managed<Gtk::Label>();
      calclab->set_halign(Gtk::Align::CENTER);
      calclab->set_margin(5);
      calclab->set_text(gettext("Calculation in progress..."));
      grid->attach(*calclab, 0, 0, 1, 1);

      bar->set_halign(Gtk::Align::CENTER);
      bar->set_margin(5);
      bar->set_show_text(true);
      grid->attach(*bar, 0, 1, 1, 1);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_margin(5);
      cancel->set_label(gettext("Cancel"));
      cancel->signal_clicked().connect([this]
      {
        this->orbits_cancel = 1;
      });
      grid->attach(*cancel, 0, 2, 1, 1);
    }

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
  false);
  window->show();

  return window;
}

void
MainWindow::resultPresenting(
  std::vector<std::array<mpf_class, 3>> *result,
  Gtk::ComboBoxText *belt, Gtk::ComboBoxText *objcomb,
  Gtk::ComboBoxText *coordcomb, Gtk::ComboBoxText *xyzcomb,
  Gtk::ComboBoxText *equincomb, Gtk::ComboBoxText *unitcomb,
  Glib::Dispatcher *result_win_disp)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(this->get_application());
  window->set_title(gettext("Result"));
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *objlab = Gtk::make_managed<Gtk::Label>();
  objlab->set_halign(Gtk::Align::START);
  objlab->set_margin(5);
  objlab->set_text(
    Glib::ustring(gettext("Object: ") + objcomb->get_active_text()));
  grid->attach(*objlab, 0, 0, 1, 1);

  Gtk::Label *coordlab = Gtk::make_managed<Gtk::Label>();
  coordlab->set_halign(Gtk::Align::START);
  coordlab->set_margin(5);
  coordlab->set_text(
    Glib::ustring(
      gettext("Coordinates: ") + coordcomb->get_active_text()));
  grid->attach(*coordlab, 0, 1, 1, 1);

  Gtk::Label *equinlab = Gtk::make_managed<Gtk::Label>();
  equinlab->set_halign(Gtk::Align::START);
  equinlab->set_margin(5);
  equinlab->set_text(
    Glib::ustring(
      gettext("Equator and equinox: ") + equincomb->get_active_text()));
  grid->attach(*equinlab, 0, 2, 1, 1);

  Gtk::Label *unitlab = Gtk::make_managed<Gtk::Label>();
  unitlab->set_halign(Gtk::Align::START);
  unitlab->set_margin(5);
  unitlab->set_text(
    Glib::ustring(
      gettext("Units of measurement: ") + unitcomb->get_active_text()));
  grid->attach(*unitlab, 0, 3, 1, 1);

  Gtk::Label *beltlab = Gtk::make_managed<Gtk::Label>();
  beltlab->set_halign(Gtk::Align::START);
  beltlab->set_margin(5);
  beltlab->set_text(
    Glib::ustring(gettext("Hour belt: ") + belt->get_active_text()));
  grid->attach(*beltlab, 0, 4, 1, 1);

  Gtk::TreeModel::ColumnRecord record;
  Gtk::TreeModelColumn<std::string> dates;
  Gtk::TreeModelColumn<std::string> X;
  Gtk::TreeModelColumn<std::string> Y;
  Gtk::TreeModelColumn<std::string> Z;
  record.add(dates);
  record.add(X);
  record.add(Y);
  record.add(Z);

  Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(record);

  auto rowtitle = *(model->append());
  rowtitle[dates] = std::string(gettext("Date and time(local)"));
  if(objcomb->get_active_row_number() != 21)
    {
      if(xyzcomb->get_active_row_number() == 0)
        {
          rowtitle[X] = std::string("X");
          rowtitle[Y] = std::string("Y");
          rowtitle[Z] = std::string("Z");
        }
      if(xyzcomb->get_active_row_number() == 1)
        {
          rowtitle[X] = std::string("Vx");
          rowtitle[Y] = std::string("Vy");
          rowtitle[Z] = std::string("Vz");
        }
    }
  else
    {
      if(xyzcomb->get_active_row_number() == 0)
        {
          rowtitle[X] = std::string("φ");
          rowtitle[Y] = std::string("θ");
          rowtitle[Z] = std::string("ψ");
        }
      if(xyzcomb->get_active_row_number() == 1)
        {
          rowtitle[X] = std::string("φ'");
          rowtitle[Y] = std::string("θ'");
          rowtitle[Z] = std::string("ψ'");
        }
    }

  std::stringstream strm;
  std::locale loc("C");
  AuxFunc af;

  for(size_t i = 0; i < result->size(); i++)
    {
      int ych, mch, dch, hch, minch;
      double secch;
      af.dateJulian(JDshow + i * stepnum, &dch, &mch, &ych, &hch, &minch, &secch);
      auto row = *(model->append());
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
      row[dates] = datestr;

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20)
           << std::get<0>(result->at(i));
      row[X] = strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20)
           << std::get<1>(result->at(i));
      row[Y] = strm.str();

      strm.clear();
      strm.str("");
      strm.imbue(loc);
      strm << std::fixed << std::setprecision(20)
           << std::get<2>(result->at(i));
      row[Z] = strm.str();
    }

  Gtk::TreeView *treev = Gtk::make_managed<Gtk::TreeView>();
  treev->set_grid_lines(Gtk::TreeView::GridLines::BOTH);
  treev->set_margin(10);
  treev->set_halign(Gtk::Align::START);
  treev->set_valign(Gtk::Align::CENTER);
  treev->set_model(model);
  treev->set_name("ResultTable");
  treev->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  int x, y, h, w;
  Gtk::TreeViewColumn *column1 = Gtk::make_managed<Gtk::TreeViewColumn>();
  Gtk::TreeViewColumn *column2 = Gtk::make_managed<Gtk::TreeViewColumn>();
  Gtk::TreeViewColumn *column3 = Gtk::make_managed<Gtk::TreeViewColumn>();
  Gtk::TreeViewColumn *column4 = Gtk::make_managed<Gtk::TreeViewColumn>();
  column1->pack_start(dates);
  column2->pack_start(X);
  column3->pack_start(Y);
  column4->pack_start(Z);
  treev->append_column(*column1);
  treev->append_column(*column2);
  treev->append_column(*column3);
  treev->append_column(*column4);
  column1->cell_get_size(x, y, w, h);
  treev->set_headers_visible(false);
  Gtk::Requisition rq1, rq2;
  treev->get_preferred_size(rq1, rq2);

  Gtk::CellRenderer *ren = treev->get_column_cell_renderer(0);
  ren->set_alignment(0.5, 0.5);
  ren = treev->get_column_cell_renderer(1);
  ren->set_alignment(0.5, 0.5);
  ren = treev->get_column_cell_renderer(2);
  ren->set_alignment(0.5, 0.5);
  ren = treev->get_column_cell_renderer(3);
  ren->set_alignment(0.5, 0.5);

  Glib::RefPtr<Gdk::Surface> surf = this->get_surface();
  Glib::RefPtr<Gdk::Display> disp = this->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_halign(Gtk::Align::CENTER);
  scrl->set_margin(5);
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  if(req.get_height() > 11 * h)
    {
      scrl->set_min_content_height(11 * h);
    }
  else
    {
      scrl->set_min_content_height(req.get_height());
    }
  if(req.get_width() > rq2.get_width())
    {
      scrl->set_min_content_width(rq2.get_width());
    }
  else
    {
      scrl->set_min_content_width(req.get_width());
    }
  scrl->set_child(*treev);
  grid->attach(*scrl, 0, 5, 2, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_halign(Gtk::Align::CENTER);
  save->set_margin(5);
  save->set_label(gettext("Save"));
  save->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                 &MainWindow::saveDialog), window, objlab, coordlab, equinlab, unitlab, beltlab,
                                 treev, objcomb));
  grid->attach(*save, 0, 6, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect([window]
  {
    window->close();
  });
  grid->attach(*close, 1, 6, 1, 1);

  if(objcomb->get_active_row_number() == 21)
    {
      grid->remove(*coordlab);
      grid->remove(*equinlab);
    }

  window->signal_close_request().connect([window, result_win_disp]
  {
    delete result_win_disp;
    window->hide();
    delete window;
    return true;
  },
  false);
  window->show();
  result->clear();
  delete result;
}

void
MainWindow::saveDialog(Gtk::Window *win, Gtk::Label *objlab,
                       Gtk::Label *coordlab, Gtk::Label *equinlab,
                       Gtk::Label *unitlab, Gtk::Label *beltlab, Gtk::TreeView *view,
                       Gtk::ComboBoxText *objcomb)
{
  Glib::RefPtr<Gtk::FileChooserNative> fcd = Gtk::FileChooserNative::create(
        gettext("Save as..."), *win, Gtk::FileChooser::Action::SAVE,
        gettext("Save"), gettext("Cancel"));
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_parse_name(
                                 Glib::get_home_dir());
  if(fl)
    {
      fcd->set_current_folder(fl);
    }
  fcd->signal_response().connect(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::saveDialogFunc), fcd, objlab,
               coordlab, equinlab, unitlab, beltlab, view, objcomb));

  fcd->show();
}

void
MainWindow::saveDialogFunc(int rid, Glib::RefPtr<Gtk::FileChooserNative> fcd,
                           Gtk::Label *objlab, Gtk::Label *coordlab,
                           Gtk::Label *equinlab,
                           Gtk::Label *unitlab, Gtk::Label *beltlab, Gtk::TreeView *view,
                           Gtk::ComboBoxText *objcomb)
{
  if(rid == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fcd->get_file();
      std::string filename = fl->get_path();
      std::filesystem::path filepath = std::filesystem::u8path(filename);
      filename = ".csv";
      filepath.replace_extension(std::filesystem::u8path(filename));
      std::fstream f;
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      if(!f.is_open())
        {
          std::cerr << "Cannot open file for saving" << std::endl;
        }
      else
        {
          std::string line(objlab->get_text());
          line = line + "\n";
          f.write(line.c_str(), line.size());
          if(objcomb->get_active_row_number() != 21)
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
          Glib::RefPtr<Gtk::TreeModel> model = view->get_model();
          auto children = model->children();
          for(auto iter = children.begin(), end = children.end(); iter != end; ++iter)
            {
              line.clear();
              auto row = *iter;
              for(int i = 0; i < model->get_n_columns(); i++)
                {
                  std::string valline;
                  row.get_value(i, valline);
                  line = line + valline + ";";
                }
              line = line + "\n";
              f.write(line.c_str(), line.size());
            }
          f.close();
        }
    }
}

void
MainWindow::orbitsGraph(Gtk::Entry *day, Gtk::Entry *month, Gtk::Entry *year,
                        Gtk::Entry *hour, Gtk::Entry *minut, Gtk::Entry *second,
                        Gtk::ComboBoxText *timecomb,
                        Gtk::ComboBoxText *belt, Gtk::ComboBoxText *coordcomb,
                        Gtk::ComboBoxText *equincomb, Gtk::Entry *pathent)
{
  std::string daystr(day->get_text());
  std::string monthstr(month->get_text());
  std::string yearstr(year->get_text());
  std::string hourstr(hour->get_text());
  std::string minutstr(minut->get_text());
  std::string secondstr(second->get_text());
  std::string beltstr(belt->get_active_text());
  std::string pathstr(pathent->get_text());

  int daynum = -1;
  int monthnum = -1;
  int yearnum = -1;
  int hournum = -1;
  int minutnum = -1;
  double secondnum = -1;
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
  bool chtm = daf.epochCheckUTC(daynum, monthnum, yearnum, hournum, minutnum,
                                secondnum, timecomb->get_active_row_number(), beltnum, pathstr);
  if(chtm)
    {
      Gtk::ProgressBar *bar = Gtk::make_managed<Gtk::ProgressBar>();
      bar->set_fraction(0.0);

      OrbitsDiagram *od = new OrbitsDiagram(this, pathstr, JDcalc,
                                            timecomb->get_active_row_number(),
                                            coordcomb->get_active_row_number(), equincomb->get_active_row_number(),
                                            &orbits_cancel);

      Gtk::Window *win = resultPulseWin(1, bar);
      double sz = static_cast<double>(od->calculateSize());
      Glib::Dispatcher *pulse_disp = new Glib::Dispatcher;
      Glib::Dispatcher *compl_disp = new Glib::Dispatcher;
      Glib::Dispatcher *canceled_disp = new Glib::Dispatcher;
      std::mutex *pulsemtx = new std::mutex;
      double *frac = new double(0.0);
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

      od->diagram_close = [od] {delete od;};

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
        delete compl_disp;
      };
      od->canceled_signal = [canceled_disp, compl_disp]
      {
        canceled_disp->emit();
        delete canceled_disp;
        delete compl_disp;
      };

      bar->signal_destroy().connect([pulsemtx, pulse_disp, frac]
      {
        delete pulsemtx;
        delete pulse_disp;
        delete frac;
      });

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

  aboutd->set_program_name("EphEPM");
  aboutd->set_version("1.0");
  aboutd->set_copyright("Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>");
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
      f.read(&ab[0], ab.size());
      f.close();
      abbuf = Glib::ustring(ab.begin(), ab.end());
    }
  else
    {
      std::cerr << "Licence file not found" << std::endl;
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
  abbuf =
    gettext(
      "EphEPM is simple program to calculate some Solar system bodies coordinates.\n"
      "Author Yury Bobylev.\n\n"
      "Program uses next libraries:\n"
      "GTK https://www.gtk.org\n"
      "GMP https://gmplib.org\n"
      "MathGL http://mathgl.sourceforge.net\n"
      "ICU https://icu.unicode.org");
  aboutd->set_comments(abbuf);

  aboutd->signal_close_request().connect([aboutd]
  {
    aboutd->hide();
    delete aboutd;
    return true;
  },
  false);
  aboutd->show();
}


