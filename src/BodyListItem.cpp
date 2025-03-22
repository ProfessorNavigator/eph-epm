/*
 * Copyright (C) 2023-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <BodyListItem.h>

BodyListItem::BodyListItem(Glib::ustring &bodyname, int &id)
{
  this->bodyname = bodyname;
  this->naifid = id;
}

Glib::RefPtr<BodyListItem>
BodyListItem::create(std::string &bodyname, int &id)
{
  Glib::ustring bnm(bodyname);
  BodyListItem *bli = new BodyListItem(bnm, id);
  return Glib::make_refptr_for_instance(bli);
}
