/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <vector>
#include <cmath>
#include <math.h>
#include <gmpxx.h>
#include <fstream>
#include "AuxFunc.h"

#ifndef INCLUDE_EPMCALCULATIONS_H_
#define INCLUDE_EPMCALCULATIONS_H_

class EPMCalculations
{
public:
  EPMCalculations();
  virtual
  ~EPMCalculations();

  mpf_class
  tdbCalc(std::fstream *f, uint64_t *c_b, uint64_t *c_e, double JD, int type);

  mpf_class
  bodyCalcX(std::fstream *f, uint64_t *c_b, uint64_t *c_e, mpf_class JDC,
	    int var, int type, mpf_class *au);

  mpf_class
  bodyCalcY(std::fstream *f, uint64_t *c_b, uint64_t *c_e, mpf_class JDC,
	    int var, int type, mpf_class *au);

  mpf_class
  bodyCalcZ(std::fstream *f, uint64_t *c_b, uint64_t *c_e, mpf_class JDC,
	    int var, int type, mpf_class *au);
};

#endif /* INCLUDE_EPMCALCULATIONS_H_ */

