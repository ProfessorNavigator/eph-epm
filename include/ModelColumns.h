/*
 * Copyright (C) 2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_MODELCOLUMNS_H_
#define INCLUDE_MODELCOLUMNS_H_

#include <gtkmm.h>

class ModelColumns : public Glib::Object
{
public:
  virtual
  ~ModelColumns();
  Glib::ustring date;
  Glib::ustring x;
  Glib::ustring y;
  Glib::ustring z;
  static Glib::RefPtr<ModelColumns>
  create(std::string &date, std::string &x, std::string &y, std::string &z);
private:
  ModelColumns(std::string &date, std::string &x, std::string &y,
	       std::string &z);
};

#endif /* INCLUDE_MODELCOLUMNS_H_ */
