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
#ifndef SPKITEM_H
#define SPKITEM_H

class SPKItem
{
public:
  SPKItem();

  SPKItem(const SPKItem &other);

  SPKItem &
  operator=(const SPKItem &other);

  double JD_begin = 0.0;
  double JD_end = 0.0;

  int NAIF_body_id = 0;
  int NAIF_center_id = 0;
  int NAIF_ref_frame = 0;
  int NAIF_spk_data_type = 0;
  int initial_address = 0;
  int final_addr = 0;
};

#endif // SPKITEM_H
