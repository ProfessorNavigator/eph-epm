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

#ifndef DAFOPERATIONS_H
#define DAFOPERATIONS_H

#include <BodyV.h>
#include <SPKItem.h>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class DAFOperations
{
public:
  DAFOperations();

  std::string
  fileVersion(std::fstream *f);

  bool
  epochCheckUTC(const int &day, const int &month, const int &year,
                const int &hours, const int &minutes, const double &seconds,
                const int &timesc, const int &belt,
                const std::string &filename);

  bool
  epochCheckUTC(const double &JD, const int &timesc, double &epb, double &epe,
                const std::string &filename);

  int
  bodyVect(std::fstream *result, uint64_t &c_beg, uint64_t &c_end,
           const int &NAIFid, const double &JD);

  std::vector<SPKItem>
  bodiesVector(std::fstream *f);

private:
  std::vector<BodyV> filev;
};

#endif // DAFOPERATIONS_H
