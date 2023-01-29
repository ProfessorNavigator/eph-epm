/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of EphEPM.
 EphEPM is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 EphEPM is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with EphEPM. If not,
 see <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_COORDINATES_H_
#define INCLUDE_COORDINATES_H_

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <gmpxx.h>
#include <cmath>
#include <functional>

#include "AuxFunc.h"
#include "EPMCalculations.h"
#include "DAFOperations.h"

class Coordinates
{
public:
  Coordinates(std::string body, double JD, int timesc, int coordtype, int xyz,
	      int theory, int unit, double step, int stepnum,
	      std::string epmpath, int *cancel);
  virtual
  ~Coordinates();
  std::vector<std::array<mpf_class, 3>>
  calculationsXYZ();
  std::function<void
  ()> pulse_signal;
private:
  std::string body = "";
  double JD = -1;
  int coordtype = -1;
  int xyz = -1;
  int theory = -1;
  int unit = -1;
  double step = -1;
  int stepnum = -1;
  std::string epmpath = "";
  int *cancel = nullptr;
  int timesc = 0;
};

#endif /* INCLUDE_COORDINATES_H_ */

