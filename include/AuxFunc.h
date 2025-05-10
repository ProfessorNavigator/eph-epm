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

#ifndef AUXFUNC_H
#define AUXFUNC_H

#include <filesystem>
#include <gmpxx.h>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

class AuxFunc
{
public:
  AuxFunc();

  std::filesystem::path
  get_selfpath();

  double
  timeTT(const int &day, const int &month, const int &year, const int &hour,
         const int &minut, const double &sec, const int &belt);

  double
  timeTT(const double &JDutc);

  void
  dateJulian(const double &JDN, int &day, int &month, int &year, int &hour,
             int &minut,
             double &second); // Julian date to calendar date

  double
  utcJD(const int &day, const int &month, const int &year, const int &hour,
        const int &minut, const double &sec);

  double
  grigToJuliancal(const double &JDgr);

  void
  toEcliptic(const mpf_class &Oldx, const mpf_class &Oldy,
             const mpf_class &Oldz, mpf_class &Newx, mpf_class &Newy,
             mpf_class &Newz, const double &JD, const int &ch);

  void
  precession(const mpf_class &Oldx, const mpf_class &Oldy,
             const mpf_class &Oldz, mpf_class &Newx, mpf_class &Newy,
             mpf_class &Newz, const double &JD);

  void
  precessionNnut(const mpf_class &Oldx, const mpf_class &Oldy,
                 const mpf_class &Oldz, mpf_class &Newx, mpf_class &Newy,
                 mpf_class &Newz, const double &JD);

  void
  rotateXYZ(mpf_class xyz[3], const mpf_class &rx, const mpf_class &ry,
            const mpf_class &rz, mpf_class result[3]);

  mpf_class
  Cos(const mpf_class &x);

  mpf_class
  Sin(const mpf_class &x);  
};
#endif // AUXFUNC_H
