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

#ifndef INCLUDE_BODYV_H_
#define INCLUDE_BODYV_H_

#include <fstream>
#include <vector>
#include <tuple>

class BodyV
{
public:
  BodyV();
  virtual
  ~BodyV();
  BodyV(const BodyV &other);
  BodyV&
  operator =(const BodyV &other);

  void
  setFile(std::fstream *f);
  std::fstream*
  getFile();
  void
  setVect(
      std::vector<std::tuple<double, double, int, int, int, int, int, int>> &boydv);
  std::vector<std::tuple<double, double, int, int, int, int, int, int>>
  getVect();
private:
  std::fstream *file = nullptr;
  std::vector<std::tuple<double, double, int, int, int, int, int, int>> bodyv;
};

#endif /* INCLUDE_BODYV_H_ */
