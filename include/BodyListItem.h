/*
 * Copyright (C) 2023-2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_BODYLISTITEM_H_
#define INCLUDE_BODYLISTITEM_H_

#include <glibmm-2.68/glibmm/object.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <glibmm-2.68/glibmm/ustring.h>
#include <string>

class BodyListItem : public Glib::Object
{
public:
  static Glib::RefPtr<BodyListItem>
  create(std::string &bodyname, int &id);

  Glib::ustring bodyname;
  int naifid = -1;

private:
  BodyListItem(Glib::ustring &bodyname, int &id);
};

#endif /* INCLUDE_BODYLISTITEM_H_ */
