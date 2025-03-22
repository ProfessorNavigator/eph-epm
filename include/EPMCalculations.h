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

#ifndef EPMCALCULATIONS_H
#define EPMCALCULATIONS_H

#include <cstdint>
#include <fstream>
#include <gmpxx.h>

class EPMCalculations
{
public:
  EPMCalculations();

  mpf_class
  tdbCalc(std::fstream *f, const uint64_t &c_b, const uint64_t &c_e,
          const double &JD, const int &type);

  mpf_class
  bodyCalcX(std::fstream *f, const uint64_t &c_b, const uint64_t &c_e,
            const mpf_class &JDC, const int &var, const int &type,
            mpf_class &au);

  mpf_class
  bodyCalcY(std::fstream *f, const uint64_t &c_b, const uint64_t &c_e,
            const mpf_class &JDC, const int &var, const int &type,
            mpf_class &au);

  mpf_class
  bodyCalcZ(std::fstream *f, const uint64_t &c_b, const uint64_t &c_e,
            const mpf_class &JDC, const int &var, const int &type,
            mpf_class &au);
};

#endif // EPMCALCULATIONS_H
