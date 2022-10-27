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

#include "DiagramWidget.h"

DiagramWidget::DiagramWidget(Gtk::ApplicationWindow *mw, mglGraph *gr)
{
  this->mw = mw;
  this->gr = gr;
  plotincr.push_back(scale_val1);
  plotincr.push_back(scale_val1);
  plotincr.push_back(scale_val2);
  plotincr.push_back(scale_val2);
  gr->Zoom(scale_val1, scale_val1, scale_val2, scale_val2);
  Gdk::Rectangle req = screenRes();
  Height = req.get_height();
  Width = req.get_width();
  AuxFunc af;
  std::filesystem::path p(std::filesystem::u8path(af.get_selfpath()));
  Sharepath = p.parent_path().u8string() + "/../share/EphEPM";
  css_provider = Gtk::CssProvider::create();
  css_provider->load_from_path(Glib::ustring(Sharepath + "/graphicWidg.css"));
}

DiagramWidget::~DiagramWidget()
{
  //dtor
}

void
DiagramWidget::diagramPlot()
{
  Gtk::Window *window = new Gtk::Window;
  Gtk::Overlay *overlay = Gtk::make_managed<Gtk::Overlay>();
  window->set_title(gettext("Orbits"));
  window->set_child(*overlay);
  window->fullscreen();

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_name("buttonPanel");
  grid->get_style_context()->add_provider(css_provider,
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->set_halign(Gtk::Align::END);
  grid->set_valign(Gtk::Align::CENTER);
  grid->set_margin(5);

  Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea>();
  drar->set_draw_func(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::on_draw)));

  Glib::RefPtr<Gtk::EventControllerScroll> scr =
    Gtk::EventControllerScroll::create();
  scr->set_flags(
    Gtk::EventControllerScroll::Flags::VERTICAL
    | Gtk::EventControllerScroll::Flags::DISCRETE);
  drar->add_controller(scr);

  Glib::RefPtr<Gtk::GestureDrag> drag = Gtk::GestureDrag::create();
  drag->signal_drag_end().connect(
    [this](double x, double y)
  {
    this->X = 0;
    this->Y = 0;
  });
  drar->add_controller(drag);

  overlay->set_child(*drar);
  overlay->add_overlay(*grid);

  Gtk::Button *increase = Gtk::make_managed<Gtk::Button>();
  increase->set_label(gettext("Increase"));
  increase->set_halign(Gtk::Align::CENTER);
  increase->set_margin(5);
  increase->set_name("sizeButton");
  increase->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*increase, 0, 0, 3, 1);

  Gtk::Button *decrease = Gtk::make_managed<Gtk::Button>();
  decrease->set_label(gettext("Decrease"));
  decrease->set_halign(Gtk::Align::CENTER);
  decrease->set_margin(5);
  decrease->set_name("sizeButton");
  decrease->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*decrease, 0, 1, 3, 1);

  Gtk::Button *up = Gtk::make_managed<Gtk::Button>();
  up->set_label(gettext("Move up"));
  up->set_halign(Gtk::Align::CENTER);
  up->set_margin(5);
  up->set_name("sizeButton");
  up->get_style_context()->add_provider(css_provider,
                                        GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*up, 0, 2, 3, 1);

  Gtk::Button *down = Gtk::make_managed<Gtk::Button>();
  down->set_label(gettext("Move down"));
  down->set_halign(Gtk::Align::CENTER);
  down->set_margin(5);
  down->set_name("sizeButton");
  down->get_style_context()->add_provider(css_provider,
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*down, 0, 3, 3, 1);

  Gtk::Button *left = Gtk::make_managed<Gtk::Button>();
  left->set_label(gettext("Move left"));
  left->set_halign(Gtk::Align::CENTER);
  left->set_margin(5);
  left->set_name("sizeButton");
  left->get_style_context()->add_provider(css_provider,
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*left, 0, 4, 3, 1);

  Gtk::Button *right = Gtk::make_managed<Gtk::Button>();
  right->set_label(gettext("Move right"));
  right->set_halign(Gtk::Align::CENTER);
  right->set_margin(5);
  right->set_name("sizeButton");
  right->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  grid->attach(*right, 0, 5, 3, 1);

  Gtk::Label *labrot = Gtk::make_managed<Gtk::Label>();
  labrot->set_halign(Gtk::Align::CENTER);
  labrot->set_margin(5);
  labrot->set_max_width_chars(25);
  labrot->set_wrap(true);
  labrot->set_wrap_mode(Pango::WrapMode::WORD);
  labrot->set_justify(Gtk::Justification::CENTER);
  labrot->set_text(gettext("Rotation angles in degrees relative to axis:"));
  grid->attach(*labrot, 0, 6, 3, 1);

  Gtk::Label *labx = Gtk::make_managed<Gtk::Label>();
  labx->set_halign(Gtk::Align::CENTER);
  labx->set_margin(5);
  labx->set_text("X");
  grid->attach(*labx, 0, 7, 1, 1);

  Gtk::Label *laby = Gtk::make_managed<Gtk::Label>();
  laby->set_halign(Gtk::Align::CENTER);
  laby->set_margin(5);
  laby->set_text("Y");
  grid->attach(*laby, 1, 7, 1, 1);

  Gtk::Label *labz = Gtk::make_managed<Gtk::Label>();
  labz->set_halign(Gtk::Align::CENTER);
  labz->set_margin(5);
  labz->set_text("Z");
  grid->attach(*labz, 2, 7, 1, 1);

  Gtk::Entry *entx = Gtk::make_managed<Gtk::Entry>();
  entx->set_halign(Gtk::Align::CENTER);
  entx->set_margin(5);
  entx->set_max_length(3);
  entx->set_max_width_chars(3);
  entx->set_input_purpose(Gtk::InputPurpose::DIGITS);
  entx->set_text("50");
  grid->attach(*entx, 0, 8, 1, 1);

  Gtk::Entry *enty = Gtk::make_managed<Gtk::Entry>();
  enty->set_halign(Gtk::Align::CENTER);
  enty->set_margin(5);
  enty->set_max_length(3);
  enty->set_max_width_chars(3);
  enty->set_input_purpose(Gtk::InputPurpose::DIGITS);
  enty->set_text("0");
  grid->attach(*enty, 1, 8, 1, 1);

  Gtk::Entry *entz = Gtk::make_managed<Gtk::Entry>();
  entz->set_halign(Gtk::Align::CENTER);
  entz->set_margin(5);
  entz->set_max_length(3);
  entz->set_max_width_chars(3);
  entz->set_input_purpose(Gtk::InputPurpose::DIGITS);
  entz->set_text("60");
  grid->attach(*entz, 2, 8, 1, 1);

  scr->signal_scroll().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::scrollEvent), entx, enty, entz,
               drar), true);
  drag->signal_drag_update().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::dragOperation), entx, enty,
               entz, drar));
  increase->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::zoomGraph), entx, enty, entz,
               drar, 0));
  decrease->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::zoomGraph), entx, enty, entz,
               drar, 1));
  up->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                          &DiagramWidget::zoomGraph), entx, enty, entz, drar, 2));
  down->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                 &DiagramWidget::zoomGraph), entx, enty, entz, drar, 3));
  left->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                 &DiagramWidget::zoomGraph), entx, enty, entz, drar, 4));
  right->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                  &DiagramWidget::zoomGraph), entx, enty, entz, drar, 5));

  Gtk::Button *rot = Gtk::make_managed<Gtk::Button>();
  rot->set_halign(Gtk::Align::CENTER);
  rot->set_margin(5);
  rot->set_label(gettext("Rotate"));
  rot->set_name("sizeButton");
  rot->get_style_context()->add_provider(css_provider,
                                         GTK_STYLE_PROVIDER_PRIORITY_USER);
  rot->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
                                &DiagramWidget::rotationFunc), entx, enty, entz, drar));
  grid->attach(*rot, 0, 9, 3, 1);

  Gtk::Button *restore = Gtk::make_managed<Gtk::Button>();
  restore->set_label(gettext("Recover"));
  restore->set_halign(Gtk::Align::CENTER);
  restore->set_margin(5);
  restore->set_name("restoreButton");
  restore->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  restore->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::zoomGraph), entx, enty, entz,
               drar,
               6));
  grid->attach(*restore, 0, 10, 3, 1);

  Gtk::Button *savejpg = Gtk::make_managed<Gtk::Button>();
  savejpg->set_label(gettext("Save as jpg"));
  savejpg->set_halign(Gtk::Align::CENTER);
  savejpg->set_margin(5);
  savejpg->set_name("saveButton");
  savejpg->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  savejpg->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::saveGraph), gr, 0));
  grid->attach(*savejpg, 0, 11, 3, 1);

  Gtk::Button *savepng = Gtk::make_managed<Gtk::Button>();
  savepng->set_label(gettext("Save as png"));
  savepng->set_halign(Gtk::Align::CENTER);
  savepng->set_margin(5);
  savepng->set_name("saveButton");
  savepng->get_style_context()->add_provider(
    css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  savepng->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::saveGraph), gr, 1));
  grid->attach(*savepng, 0, 12, 3, 1);

  Gtk::Button *minb = Gtk::make_managed<Gtk::Button>();
  minb->set_label(gettext("Minimize"));
  minb->set_halign(Gtk::Align::CENTER);
  minb->set_margin(5);
  minb->set_name("minButton");
  minb->get_style_context()->add_provider(css_provider,
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
  minb->signal_clicked().connect([window]
  {
    window->minimize();
  });
  grid->attach(*minb, 0, 13, 3, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_label(gettext("Close"));
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("closeButton");
  close->get_style_context()->add_provider(css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  close->set_margin(5);
  grid->attach(*close, 0, 14, 3, 1);

  close->signal_clicked().connect(
    sigc::mem_fun(*window, &Gtk::Window::close));

  window->signal_close_request().connect([window, this]
  {
    window->hide();
    if(this->diagram_close)
      {
        this->diagram_close();
      }
    delete window;
    return true;
  },
  false);

  window->set_application(mw->get_application());
  window->show();
}

void
DiagramWidget::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                       int height)
{
  auto image = Gdk::Pixbuf::create_from_data(
                 gr->GetRGB(), Gdk::Colorspace::RGB, false, 8, gr->GetWidth(),
                 gr->GetHeight(), 3 * gr->GetWidth() * sizeof(guint8));
  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
  cr->rectangle(0, 0, image->get_width(), image->get_height());
  cr->fill();
}

bool
DiagramWidget::scrollEvent(double x, double y, Gtk::Entry *entx,
                           Gtk::Entry *enty,
                           Gtk::Entry *entz, Gtk::DrawingArea *drar)
{
  double pl1, pl2;
  pl1 = (plotincr[2] - plotincr[0]) * 0.1;
  pl2 = (plotincr[3] - plotincr[1]) * 0.1;
  if(y < 0)
    {
      plotincr[0] = plotincr[0] + pl1;
      plotincr[1] = plotincr[1] + pl2;
      plotincr[2] = plotincr[2] - pl1;
      plotincr[3] = plotincr[3] - pl2;
    }
  if(y > 0)
    {
      plotincr[0] = plotincr[0] - pl1;
      plotincr[1] = plotincr[1] - pl2;
      plotincr[2] = plotincr[2] + pl1;
      plotincr[3] = plotincr[3] + pl2;
    }
  rotationFunc(entx, enty, entz, drar);
  return true;
}

void
DiagramWidget::dragOperation(double x, double y, Gtk::Entry *entx,
                             Gtk::Entry *enty,
                             Gtk::Entry *entz, Gtk::DrawingArea *drar)
{
  double pl1, pl2;
  pl1 = (plotincr[2] - plotincr[0]) * ((x - X) / Width);
  pl2 = (plotincr[3] - plotincr[1]) * ((y - Y) / Height);

  plotincr[0] = plotincr[0] - pl1;
  plotincr[1] = plotincr[1] + pl2;
  plotincr[2] = plotincr[2] - pl1;
  plotincr[3] = plotincr[3] + pl2;

  X = x;
  Y = y;
  rotationFunc(entx, enty, entz, drar);
}

void
DiagramWidget::zoomGraph(Gtk::Entry *entx, Gtk::Entry *enty,
                         Gtk::Entry *entz, Gtk::DrawingArea *drar, int id)
{
  double pl1, pl2;
  pl1 = (plotincr[2] - plotincr[0]) * 0.1;
  pl2 = (plotincr[3] - plotincr[1]) * 0.1;
  if(id == 0)
    {
      plotincr[0] = plotincr[0] + pl1;
      plotincr[1] = plotincr[1] + pl2;
      plotincr[2] = plotincr[2] - pl1;
      plotincr[3] = plotincr[3] - pl2;
    }
  if(id == 1)
    {
      plotincr[0] = plotincr[0] - pl1;
      plotincr[1] = plotincr[1] - pl2;
      plotincr[2] = plotincr[2] + pl1;
      plotincr[3] = plotincr[3] + pl2;
    }
  if(id == 2)
    {
      pl2 = (plotincr[3] - plotincr[1]) * 0.05;
      plotincr[1] = plotincr[1] - pl2;
      plotincr[3] = plotincr[3] - pl2;
    }
  if(id == 3)
    {
      pl2 = (plotincr[3] - plotincr[1]) * 0.05;
      plotincr[1] = plotincr[1] + pl2;
      plotincr[3] = plotincr[3] + pl2;
    }

  if(id == 4)
    {
      pl1 = (plotincr[2] - plotincr[0]) * 0.05;
      plotincr[0] = plotincr[0] + pl1;
      plotincr[2] = plotincr[2] + pl1;
    }

  if(id == 5)
    {
      pl1 = (plotincr[2] - plotincr[0]) * 0.05;
      plotincr[0] = plotincr[0] - pl1;
      plotincr[2] = plotincr[2] - pl1;
    }
  if(id == 6)
    {
      plotincr[0] = scale_val1;
      plotincr[1] = scale_val1;
      plotincr[2] = scale_val2;
      plotincr[3] = scale_val2;
      entx->set_text("50");
      enty->set_text("0");
      entz->set_text("60");
    }
  rotationFunc(entx, enty, entz, drar);
}

void
DiagramWidget::saveGraph(mglGraph *graph, int mode)
{
  std::string filename;
  Gtk::Window *window = new Gtk::Window;
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  window->set_child(*grid);

  Gtk::Entry *file = Gtk::make_managed<Gtk::Entry>();
  file->set_placeholder_text(gettext("File name"));
  file->set_margin(5);
  grid->attach(*file, 0, 0, 2, 1);


  Glib::ustring folder = Glib::get_home_dir();

  Gtk::FileChooserWidget *filech = Gtk::make_managed<Gtk::FileChooserWidget>();
  filech->set_margin(5);
  filech->set_current_folder(Gio::File::create_for_path(folder));
  grid->attach(*filech, 0, 1, 2, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_label(gettext("Cancel"));
  cancel->set_halign(Gtk::Align::START);
  cancel->set_margin(5);
  cancel->signal_clicked().connect(
    sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 0, 2, 1, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_label(gettext("Save"));
  save->set_halign(Gtk::Align::END);
  save->set_margin(5);

  grid->attach(*save, 1, 2, 1, 1);

  window->show();
  save->signal_clicked().connect(
    sigc::bind(sigc::mem_fun(*this, &DiagramWidget::saveJPEG), window,
               filename, filech, file, graph, mode));
}

void
DiagramWidget::saveJPEG(Gtk::Window *window, std::string filename,
                        Gtk::FileChooser *filech, Gtk::Entry *file,
                        mglGraph *graph, int mode)
{
  filename = filech->get_current_folder()->get_path();
  filename = filename + "/" + file->get_text();
  std::string::size_type n;
  if(mode == 0)
    {
      n = filename.find(".jpg", 0);
      if(n == std::string::npos)
        {
          n = filename.find(".jpeg", 0);
        }
      if(n == std::string::npos)
        {
          filename = filename + ".jpg";
        }
    }

  if(mode == 1)
    {
      n = filename.find(".png", 0);
      if(n == std::string::npos)
        {
          filename = filename + ".png";
        }
    }

  std::filesystem::path *p = new std::filesystem::path(
    std::filesystem::u8path(filename));
  if(std::filesystem::exists(*p))
    {
      Gtk::MessageDialog *dialog = new Gtk::MessageDialog(
        *window, gettext("File already exists. Replace?"), true,
        Gtk::MessageType::INFO, Gtk::ButtonsType::NONE, true);
      Gtk::Button *accept = dialog->add_button(gettext("Yes"), 0);
      accept->set_halign(Gtk::Align::END);
      accept->set_margin(5);
      Gtk::Button *cancel = dialog->add_button(gettext("No"), 1);
      cancel->set_halign(Gtk::Align::START);
      cancel->set_margin(5);
      dialog->set_hide_on_close(true);
      dialog->show();
      dialog->signal_response().connect([dialog, p, window, graph, mode]
                                        (int id)
      {
        if(id == 1)
          {
            dialog->close();
            delete p;
            delete dialog;
          }
        if(id == 0)
          {
            std::filesystem::remove(*p);
            if(mode == 0)
              {
                graph->WriteJPEG(p->u8string().c_str());
              }

            if(mode == 1)
              {
                graph->WritePNG(p->u8string().c_str());
              }
            dialog->close();
            delete dialog;
            delete p;
            window->close();
          }
      });
    }
  else
    {
      filename = p->u8string();
      delete p;
      if(mode == 0)
        {
          graph->WriteJPEG(filename.c_str());
        }

      if(mode == 1)
        {
          graph->WritePNG(filename.c_str());
        }
      window->close();
    }
}

void
DiagramWidget::rotationFunc(Gtk::Entry *entx, Gtk::Entry *enty,
                            Gtk::Entry *entz, Gtk::DrawingArea *drar)
{
  std::string xstr(entx->get_text());
  std::string ystr(enty->get_text());
  std::string zstr(entz->get_text());

  double x, y, z;
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << xstr;
  strm >> x;

  strm.clear();
  strm.str("");
  strm.imbue(loc);
  strm << ystr;
  strm >> y;

  strm.clear();
  strm.str("");
  strm.imbue(loc);
  strm << zstr;
  strm >> z;
  gr->Zoom(plotincr[0], plotincr[1], plotincr[2], plotincr[3]);
  gr->View(x - 50.0, z - 60.0, y);
  drar->queue_draw();
}

Gdk::Rectangle
DiagramWidget::screenRes()
{
  Glib::RefPtr<Gdk::Surface> surf = mw->get_surface();
  Glib::RefPtr<Gdk::Display> disp = mw->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);
  return req;
}
