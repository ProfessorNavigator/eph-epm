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

#ifndef INCLUDE_EPMAPPLICATION_H_
#define INCLUDE_EPMAPPLICATION_H_

#include <MainWindow.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/application.h>
#include <string>

class EPMApplication : public Gtk::Application
{
protected:
  EPMApplication(const std::string &id);

public:
  static Glib::RefPtr<EPMApplication>
  create(std::string &id);

private:
  MainWindow *
  create_appwindow();

protected:
  void
  on_activate() override;
};

#endif /* INCLUDE_EPMAPPLICATION_H_ */
