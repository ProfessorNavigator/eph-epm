/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <ModelColumns.h>

ModelColumns::ModelColumns(std::string &date, std::string &x, std::string &y,
			   std::string &z)
{
  this->date = Glib::ustring(date);
  this->x = Glib::ustring(x);
  this->y = Glib::ustring(y);
  this->z = Glib::ustring(z);
}

ModelColumns::~ModelColumns()
{

}

Glib::RefPtr<ModelColumns>
ModelColumns::create(std::string &date, std::string &x, std::string &y,
		     std::string &z)
{
  return Glib::make_refptr_for_instance(new ModelColumns(date, x, y, z));
}
