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

#ifndef EPMAPPLICATION_H
#define EPMAPPLICATION_H

#include <MainWindow.h>
#include <gtkmm-4.0/gtkmm/application.h>
#include <string>

class EPMApplication : public Gtk::Application
{
protected:
  EPMApplication(const std::string &id, const std::shared_ptr<std::string> &default_locale_name);

public:
  static Glib::RefPtr<EPMApplication>
  create(const std::string &id, const std::shared_ptr<std::string>&default_locale_name);

private:
  MainWindow *
  create_appwindow();

  std::shared_ptr<std::string> default_locale_name;

protected:
  void
  on_activate() override;
};

#endif // EPMAPPLICATION_H
