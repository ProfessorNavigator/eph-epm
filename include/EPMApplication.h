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

#ifndef EPMAPPLICATION_H_
#define EPMAPPLICATION_H_

#include <gtkmm.h>
#include "MainWindow.h"

class EPMApplication : public Gtk::Application
{
  protected:
    EPMApplication();
  public:
    static Glib::RefPtr<EPMApplication>
    create();
    virtual
    ~EPMApplication();
  private:
    MainWindow*
    create_appwindow();
  protected:
    void
    on_activate()override;
};

#endif /* EPMAPPLICATION_H_ */


