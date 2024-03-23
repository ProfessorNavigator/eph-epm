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

#include <EPMApplication.h>
#include <glibmm/signalproxy.h>
#include <sigc++/connection.h>
#include <stddef.h>
#include <vector>

EPMApplication::EPMApplication(const std::string &id) :
    Gtk::Application(id.c_str())
{

}

EPMApplication::~EPMApplication()
{

}

Glib::RefPtr<EPMApplication>
EPMApplication::create(std::string &id)
{
  return Glib::make_refptr_for_instance<EPMApplication>(new EPMApplication(id));
}

MainWindow*
EPMApplication::create_appwindow()
{
  MainWindow *mw = new MainWindow;
  this->add_window(*mw);
  mw->signal_hide().connect([mw, this]
  {
    std::vector<Gtk::Window*> wv;
    wv = this->get_windows();
    for(size_t i = 0; i < wv.size(); i++)
      {
	Gtk::Window *win = wv[i];
	if(win != mw)
	  {
	    win->set_visible(false);
	    delete win;
	  }
      }
    delete mw;
  });

  return mw;
}

void
EPMApplication::on_activate()
{
  std::vector<Gtk::Window*> winv;
  winv = this->get_windows();
  if(winv.size() == 0)
    {
      auto appwin = create_appwindow();
      appwin->present();
    }
}

