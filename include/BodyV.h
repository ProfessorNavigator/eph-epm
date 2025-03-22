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

#ifndef BODYV_H
#define BODYV_H

#include <SPKItem.h>
#include <fstream>
#include <vector>

class BodyV
{
public:
  BodyV();

  BodyV(const BodyV &other);

  BodyV(BodyV &&other);

  BodyV &
  operator=(const BodyV &other);

  void
  setFile(std::fstream *f);

  std::fstream *
  getFile();

  void
  setVect(const std::vector<SPKItem> &boydv);

  std::vector<SPKItem>
  getVect();

private:
  std::fstream *file = nullptr;
  std::vector<SPKItem> bodyv;
};

#endif // BODYV_H
