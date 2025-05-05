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

#ifndef COORDINATES_H
#define COORDINATES_H

#include <CoordKeeper.h>
#include <filesystem>
#include <functional>
#include <gmpxx.h>
#include <string>
#include <vector>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

class Coordinates
{
public:
  Coordinates(const int &body, const double &JD, const int &timesc,
              const int &coordtype, const int &xyz, const int &theory,
              const int &unit, const double &step, const int &stepnum,
              const std::string &epmpath, const std::string &tttdbpath,
              const std::string &smlbpath);

  std::vector<CoordKeeper>
  calculationsXYZ();

  void
  stopAll();

  std::function<void()> pulse_signal;

private:
  int body = -1;
  double JD = -1;
  int coordtype = -1;
  int xyz = -1;
  int theory = -1;
  int unit = -1;
  double step = -1;
  int stepnum = -1;
  std::filesystem::path epmpath;
  std::filesystem::path tttdbpath;
  std::filesystem::path smlbpath;
  int timesc = 0;
  bool cancel = false;
};

#endif // COORDINATES_H
