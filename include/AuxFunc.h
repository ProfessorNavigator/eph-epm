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

#ifndef INCLUDE_AUXFUNC_H_
#define INCLUDE_AUXFUNC_H_

#ifdef _WIN32
  #define _USE_MATH_DEFINES
  #include <Windows.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <sofa.h>
#include <gmpxx.h>
#include <cmath>
#include <ctime>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>

class AuxFunc
{
public:
  AuxFunc();
  virtual
  ~AuxFunc();
  std::string
  get_selfpath();
  double
  timeTT(int day, int month, int year, int hour, int minut, double sec,
	 int belt);
  double
  timeTT(double JDutc);
  void
  dateJulian(double JDN, int *day, int *month, int *year, int *hour, int *minut,
	     double *second); //Julian date to calendar date
  double
  utcJD(int day, int month, int year, int hour, int minut, double sec);
  double
  grigToJuliancal(double JDgr);
  void
  toEcliptic(mpf_class *Oldx, mpf_class *Oldy, mpf_class *Oldz, mpf_class *Newx,
	     mpf_class *Newy, mpf_class *Newz, double JD, int ch);
  void
  precession(mpf_class *Oldx, mpf_class *Oldy, mpf_class *Oldz, mpf_class *Newx,
	     mpf_class *Newy, mpf_class *Newz, double JD);
  void
  precessionNnut(mpf_class *Oldx, mpf_class *Oldy, mpf_class *Oldz,
		 mpf_class *Newx, mpf_class *Newy, mpf_class *Newz, double JD);
  void
  rotateXYZ(mpf_class xyz[3], mpf_class rx, mpf_class ry, mpf_class rz,
	    mpf_class result[3]);
  mpf_class
  Cos(mpf_class x);
  mpf_class
  Sin(mpf_class x);
  std::string
  utf8to(std::string line);
};

#endif /* INCLUDE_AUXFUNC_H_ */

