/*
 * Copyright (C) 2022-2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_DAFOPERATIONS_H_
#define INCLUDE_DAFOPERATIONS_H_

#include <BodyV.h>
#include <cstdint>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

class DAFOperations
{
public:
  DAFOperations();

  std::string
  fileVersion(std::fstream *f);

  bool
  epochCheckUTC(int day, int month, int year, int hours, int minutes,
                double seconds, int timesc, int belt, std::string filename);

  bool
  epochCheckUTC(double JD, int timesc, double *epb, double *epe,
                std::string filename);

  int
  bodyVect(std::fstream *result, uint64_t *c_beg, uint64_t *c_end, int NAIFid,
           double JD);

  std::vector<std::tuple<double, double, int, int, int, int, int, int>>
  bodiesVector(std::fstream *f);

private:
  std::vector<BodyV> filev;
};

#endif /* INCLUDE_DAFOPERATIONS_H_ */
