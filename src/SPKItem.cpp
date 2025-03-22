/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <SPKItem.h>

SPKItem::SPKItem()
{
}

SPKItem::SPKItem(const SPKItem &other)
{
  JD_begin = other.JD_begin;
  JD_end = other.JD_end;

  NAIF_body_id = other.NAIF_body_id;
  NAIF_center_id = other.NAIF_center_id;
  NAIF_ref_frame = other.NAIF_ref_frame;
  NAIF_spk_data_type = other.NAIF_spk_data_type;
  initial_address = other.initial_address;
  final_addr = other.final_addr;
}

SPKItem &
SPKItem::operator=(const SPKItem &other)
{
  if(this != &other)
    {
      JD_begin = other.JD_begin;
      JD_end = other.JD_end;

      NAIF_body_id = other.NAIF_body_id;
      NAIF_center_id = other.NAIF_center_id;
      NAIF_ref_frame = other.NAIF_ref_frame;
      NAIF_spk_data_type = other.NAIF_spk_data_type;
      initial_address = other.initial_address;
      final_addr = other.final_addr;
    }

  return *this;
}
