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

#include "AuxFunc.h"

AuxFunc::AuxFunc()
{
  mpf_set_default_prec(512);
}

AuxFunc::~AuxFunc()
{
  // TODO Auto-generated destructor stub
}

std::string
AuxFunc::get_selfpath()
{
  std::filesystem::path p;
#ifdef __linux
  p = std::filesystem::u8path("/proc/self/exe");
  return std::filesystem::read_symlink(p).u8string();
#endif
#ifdef __WIN32
  char pth [MAX_PATH];
  GetModuleFileNameA(NULL, pth, MAX_PATH);
  p = std::filesystem::path(pth);
  return p.u8string();
  #endif
}

double
AuxFunc::timeTT(int day, int month, int year, int hour, int minut, double sec,
		int belt)
{
  double a1, m1, c1, a2;
  a1 = std::floor(static_cast<double>(month - 3) / 12.0);
  m1 = static_cast<double>(month) - 3.0 - 12.0 * a1;
  c1 = std::floor(static_cast<double>(year + a1) / 100.0);
  a2 = static_cast<double>(year) + a1 - 100.0 * c1;
  int J = static_cast<int>(std::floor(146097.0 * c1 / 4.0))
      + static_cast<int>(std::floor(36525.0 * a2 / 100.0))
      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + day + 1721119;

  double JD;
  if(J >= 0)
    {

      JD = static_cast<double>(J) + static_cast<double>(hour) / 24.0
	  + static_cast<double>(minut) / 1440.0 + sec / 86400.0
	  - static_cast<double>(belt) / 24.0;
    }
  else
    {
      double k = static_cast<double>(hour) / 24.0
	  + static_cast<double>(minut) / 1440.0 + sec / 86400.0
	  - static_cast<double>(belt) / 24.0;
      k = 1 - k;
      JD = static_cast<double>(J) - k;
    }
  int chy, chm, chd, chh, chmin;
  double chsec;
  dateJulian(JD, &chd, &chm, &chy, &chh, &chmin, &chsec);
  a1 = std::floor(static_cast<double>(chm - 3.0) / 12.0);
  m1 = static_cast<double>(chm) - 3.0 - 12.0 * a1;
  c1 = std::floor((static_cast<double>(chy) + a1) / 100.0);
  a2 = static_cast<double>(chy) + a1 - 100.0 * c1;
  J = static_cast<int>(std::floor(146097.0 * c1 / 4.0))
      + static_cast<int>(std::floor(36525.0 * a2 / 100.0))
      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + chd + 1721119;
  if(J >= 0)
    {
      JD = static_cast<double>(J) + static_cast<double>(chh) / 24.0
	  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
    }
  else
    {
      double k = static_cast<double>(chh) / 24.0
	  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
      k = 1 - k;
      JD = static_cast<double>(J) - k;
    }
  if(JD <= 2424151.5)
    {
      if(JD <= 2299161.0)
	{
	  dateJulian(JD, &chd, &chm, &chy, &chh, &chmin, &chsec);
	  a1 = std::floor(static_cast<double>(chm - 3) / 12.0);
	  m1 = static_cast<double>(chm - 3) - 12.0 * a1;
	  J = static_cast<int>(std::floor(
	      1461.0 * (static_cast<double>(chy) + a1) / 4.0))
	      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + chd
	      + 1721117;
	  if(J >= 0)
	    {
	      JD = static_cast<double>(J) + static_cast<double>(chh) / 24.0
		  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
	    }
	  else
	    {
	      double k = static_cast<double>(chh) / 24.0
		  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
	      k = 1 - k;
	      JD = static_cast<double>(J) - k;
	    }
	}
    }
  else
    {
      JD = JD - 0.5;
    }
  double delta;
  int ch = iauDat(chy, chm, chd, JD - std::floor(JD), &delta);
  if(ch != 0)
    {
      bool setdelta = false;
      double y = static_cast<double>(chy)
	  + (static_cast<double>(chm) - 0.5) / 12.0;
      double u;
      if(chy < -500)
	{
	  u = (static_cast<double>(chy) - 1820.0) * 0.01;
	  delta = -20 + 32 * u * u;
	  setdelta = true;
	}
      if(chy >= -500 && chy < 500)
	{
	  u = y * 0.01;
	  delta = 10583.6 - 1014.41 * u + 33.78311 * u * u
	      - 5.952053 * u * u * u - 0.1798452 * u * u * u * u
	      + 0.022174192 * u * u * u * u * u
	      + 0.0090316521 * u * u * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 500 && chy < 1600)
	{
	  u = (y - 1000.0) * 0.01;
	  delta = 1574.2 - 556.01 * u + 71.23472 * u * u + 0.319781 * u * u * u
	      - 0.8503463 * u * u * u * u - 0.005050998 * u * u * u * u * u
	      + 0.0083572073 * u * u * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1600 && chy < 1700)
	{
	  u = y - 1600.0;
	  delta = 120.0 - 0.9808 * u - 0.01532 * u * u + u * u * u / 7129.0;
	  setdelta = true;
	}
      if(chy >= 1700 && chy < 1800)
	{
	  u = y - 1700.0;
	  delta = 8.83 + 0.1603 * u - 0.0059285 * u * u + 0.00013336 * u * u * u
	      - u * u * u * u / 1174000.0;
	  setdelta = true;
	}
      if(chy >= 1800 && chy < 1860)
	{
	  u = y - 1800.0;
	  delta = 13.72 - 0.332447 * u + 0.0068612 * u * u
	      + 0.0041116 * u * u * u - 0.00037436 * u * u * u * u
	      + 0.0000121272 * u * u * u * u * u
	      - 0.0000001699 * u * u * u * u * u * u
	      + 0.000000000875 * u * u * u * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1860 && chy < 1900)
	{
	  u = y - 1860.0;
	  delta = 7.62 + 0.5737 * u - 0.251754 * u * u + 0.01680668 * u * u * u
	      - 0.0004473624 * u * u * u * u + u * u * u * u * u / 233174.0;
	  setdelta = true;
	}
      if(chy >= 1900 && chy < 1920)
	{
	  u = y - 1900.0;
	  delta = -2.79 + 1.494119 * u - 0.0598939 * u * u
	      + 0.0061966 * u * u * u - 0.000197 * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1920 && chy < 1941)
	{
	  u = y - 1920.0;
	  delta = 21.20 + 0.84493 * u - 0.076100 * u * u
	      + 0.0020936 * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1941 && chy < 1961)
	{
	  u = y - 1950.0;
	  delta = 45.45 + 1.067 * u - u * u / 260.0 - u * u * u / 718.0;
	  setdelta = true;
	}
      if(!setdelta)
	{
	  u = (static_cast<double>(chy) - 1820.0) * 0.01;
	  delta = -20 + 32 * u * u;
	}
      JD = JD + delta / 86400.0;
    }
  else
    {
      JD = JD + (32.184 + delta) / 86400.0;
    }

  return JD;
}

double
AuxFunc::timeTT(double JDutc)
{
  double JD = JDutc;
  int chy, chm, chd, chh, chmin;
  double chsec;
  double a1, m1, c1, a2;
  dateJulian(JD, &chd, &chm, &chy, &chh, &chmin, &chsec);
  a1 = std::floor(static_cast<double>(chm - 3.0) / 12.0);
  m1 = static_cast<double>(chm) - 3.0 - 12.0 * a1;
  c1 = std::floor((static_cast<double>(chy) + a1) / 100.0);
  a2 = static_cast<double>(chy) + a1 - 100.0 * c1;
  int J = static_cast<int>(std::floor(146097.0 * c1 / 4.0))
      + static_cast<int>(std::floor(36525.0 * a2 / 100.0))
      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + chd + 1721119;
  if(J >= 0)
    {
      JD = static_cast<double>(J) + static_cast<double>(chh) / 24.0
	  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
    }
  else
    {
      double k = static_cast<double>(chh) / 24.0
	  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
      k = 1 - k;
      JD = static_cast<double>(J) - k;
    }
  if(JD <= 2424151.5)
    {
      if(JD <= 2299161.0)
	{
	  dateJulian(JD, &chd, &chm, &chy, &chh, &chmin, &chsec);
	  a1 = std::floor(static_cast<double>(chm - 3) / 12.0);
	  m1 = static_cast<double>(chm - 3) - 12.0 * a1;
	  J = static_cast<int>(std::floor(
	      1461.0 * (static_cast<double>(chy) + a1) / 4.0))
	      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + chd
	      + 1721117;
	  if(J >= 0)
	    {
	      JD = static_cast<double>(J) + static_cast<double>(chh) / 24.0
		  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
	    }
	  else
	    {
	      double k = static_cast<double>(chh) / 24.0
		  + static_cast<double>(chmin) / 1440.0 + chsec / 86400.0;
	      k = 1 - k;
	      JD = static_cast<double>(J) - k;
	    }
	}
    }
  else
    {
      JD = JD - 0.5;
    }
  double delta;
  int ch = iauDat(chy, chm, chd, JD - std::floor(JD), &delta);
  if(ch != 0)
    {
      double y = static_cast<double>(chy)
	  + (static_cast<double>(chm) - 0.5) / 12.0;
      double u;
      bool setdelta = false;
      if(chy < -500)
	{
	  u = (static_cast<double>(chy) - 1820.0) * 0.01;
	  delta = -20 + 32 * u * u;
	  setdelta = true;
	}
      if(chy >= -500 && chy < 500)
	{
	  u = y * 0.01;
	  delta = 10583.6 - 1014.41 * u + 33.78311 * u * u
	      - 5.952053 * u * u * u - 0.1798452 * u * u * u * u
	      + 0.022174192 * u * u * u * u * u
	      + 0.0090316521 * u * u * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 500 && chy < 1600)
	{
	  u = (y - 1000.0) * 0.01;
	  delta = 1574.2 - 556.01 * u + 71.23472 * u * u + 0.319781 * u * u * u
	      - 0.8503463 * u * u * u * u - 0.005050998 * u * u * u * u * u
	      + 0.0083572073 * u * u * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1600 && chy < 1700)
	{
	  u = y - 1600.0;
	  delta = 120.0 - 0.9808 * u - 0.01532 * u * u + u * u * u / 7129.0;
	  setdelta = true;
	}
      if(chy >= 1700 && chy < 1800)
	{
	  u = y - 1700.0;
	  delta = 8.83 + 0.1603 * u - 0.0059285 * u * u + 0.00013336 * u * u * u
	      - u * u * u * u / 1174000.0;
	  setdelta = true;
	}
      if(chy >= 1800 && chy < 1860)
	{
	  u = y - 1800.0;
	  delta = 13.72 - 0.332447 * u + 0.0068612 * u * u
	      + 0.0041116 * u * u * u - 0.00037436 * u * u * u * u
	      + 0.0000121272 * u * u * u * u * u
	      - 0.0000001699 * u * u * u * u * u * u
	      + 0.000000000875 * u * u * u * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1860 && chy < 1900)
	{
	  u = y - 1860.0;
	  delta = 7.62 + 0.5737 * u - 0.251754 * u * u + 0.01680668 * u * u * u
	      - 0.0004473624 * u * u * u * u + u * u * u * u * u / 233174.0;
	  setdelta = true;
	}
      if(chy >= 1900 && chy < 1920)
	{
	  u = y - 1900.0;
	  delta = -2.79 + 1.494119 * u - 0.0598939 * u * u
	      + 0.0061966 * u * u * u - 0.000197 * u * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1920 && chy < 1941)
	{
	  u = y - 1920.0;
	  delta = 21.20 + 0.84493 * u - 0.076100 * u * u
	      + 0.0020936 * u * u * u;
	  setdelta = true;
	}
      if(chy >= 1941 && chy < 1961)
	{
	  u = y - 1950.0;
	  delta = 45.45 + 1.067 * u - u * u / 260.0 - u * u * u / 718.0;
	  setdelta = true;
	}
      if(!setdelta)
	{
	  u = (static_cast<double>(chy) - 1820.0) * 0.01;
	  delta = -20 + 32 * u * u;
	}
      JD = JD + delta / 86400.0;
    }
  else
    {
      JD = JD + (32.184 + delta) / 86400.0;
    }

  return JD;
}

void
AuxFunc::dateJulian(double JDN, int *day, int *month, int *year, int *hour,
		    int *minut, double *second)
{
  int J = static_cast<int>(JDN);
  int c1 = static_cast<int>(std::floor(
      (4.0 * static_cast<double>(J) - 6884477.0) / 146097.0));
  int e1 = 4 * J - 6884477 - 146097 * c1;

  int a1 = (100 * static_cast<int>(std::floor(static_cast<double>(e1) / 4.0))
      + 99) / 36525;
  int e2 = 100 * static_cast<int>(std::floor(static_cast<double>(e1) / 4.0))
      + 99 - 36525 * a1;

  int m1 = (5 * static_cast<int>(std::floor(static_cast<double>(e2) / 100.0))
      + 2) / 153;
  int e3 = 5 * static_cast<int>(std::floor(static_cast<double>(e2) / 100.0)) + 2
      - 153 * m1;
  int a2 = static_cast<int>(std::floor(static_cast<double>(m1 + 2) / 12.0));
  int m2 = m1 + 2 - 12 * a2;
  *year = 100 * c1 + a1 + a2;
  *month = m2 + 1;
  *day = static_cast<int>(std::floor(static_cast<double>(e3) / 5.0)) + 1;
  double F, p;
  F = std::modf(JDN, &p);
  if(F < 0.0)
    {
      F = 1 + F;
    }
  *hour = static_cast<int>(F * 24.0);
  *minut = static_cast<int>((F * 24.0 - static_cast<double>(*hour)) * 60.0);
  *second = ((F * 24.0 - static_cast<double>(*hour)) * 60.0
      - static_cast<double>(*minut)) * 60.0;
}

double
AuxFunc::utcJD(int day, int month, int year, int hour, int minut, double sec)
{
  double a1, m1, c1, a2;
  a1 = std::floor(static_cast<double>(month - 3) / 12.0);
  m1 = static_cast<double>(month) - 3.0 - 12.0 * a1;
  c1 = std::floor(static_cast<double>(year + a1) / 100.0);
  a2 = static_cast<double>(year) + a1 - 100.0 * c1;
  int J = static_cast<int>(std::floor(146097.0 * c1 / 4.0))
      + static_cast<int>(std::floor(36525.0 * a2 / 100.0))
      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + day + 1721119;

  double JD;
  if(J >= 0)
    {

      JD = static_cast<double>(J) + static_cast<double>(hour) / 24.0
	  + static_cast<double>(minut) / 1440.0 + sec / 86400.0;
    }
  else
    {
      double k = static_cast<double>(hour) / 24.0
	  + static_cast<double>(minut) / 1440.0 + sec / 86400.0;
      k = 1 - k;
      JD = static_cast<double>(J) - k;
    }
  return JD;
}

double
AuxFunc::grigToJuliancal(double JDgr)
{
  int day, month, year, hour, minut;
  double sec;
  dateJulian(JDgr, &day, &month, &year, &hour, &minut, &sec);
  double a1, m1;
  a1 = std::floor(static_cast<double>(month - 3) / 12.0);
  m1 = static_cast<double>(month - 3) - 12.0 * a1;
  int J = static_cast<int>(std::floor(
      1461.0 * (static_cast<double>(year) + a1) / 4.0))
      + static_cast<int>(std::floor((153.0 * m1 + 2.0) / 5.0)) + day + 1721117;
  double JD;
  if(J >= 0)
    {
      JD = static_cast<double>(J) + static_cast<double>(hour) / 24.0
	  + static_cast<double>(minut) / 1440.0 + sec / 86400.0;
    }
  else
    {
      double k = static_cast<double>(hour) / 24.0
	  + static_cast<double>(minut) / 1440.0 + sec / 86400.0;
      k = 1 - k;
      JD = static_cast<double>(J) - k;
    }
  return JD;
}

void
AuxFunc::toEcliptic(mpf_class *Oldx, mpf_class *Oldy, mpf_class *Oldz,
		    mpf_class *Newx, mpf_class *Newy, mpf_class *Newz,
		    double JD, int ch)
{
  mpf_class e;
  if(ch == 0)
    {
      e = mpf_class(84381.448) / mpf_class(3600) * mpf_class(M_PI)
	  / mpf_class(180);
      *Newx = *Oldx;
      *Newy = *Oldz * Sin(e) + *Oldy * Cos(e);
      *Newz = *Oldz * Cos(e) - *Oldy * Sin(e);
    }
  else
    {
      if(ch == 1)
	{
	  double P[3][3];
	  iauEcm06(JD, 0.0, P);
	  mpf_class result[3];
	  for(size_t i = 0; i < 3; i++)
	    {
	      result[i] = 0.0;
	    }
	  mpf_class xyz[3];
	  xyz[0] = *Oldx;
	  xyz[1] = *Oldy;
	  xyz[2] = *Oldz;

	  for(size_t i = 0; i < 3; i++)
	    {
	      for(size_t j = 0; j < 3; j++)
		{
		  result[i] = result[i] + P[i][j] * xyz[j];
		}
	    }
	  *Newx = result[0];
	  *Newy = result[1];
	  *Newz = result[2];
	}
      if(ch == 2)
	{
	  double dpsi, deps, epsa;
	  double rb[3][3];
	  double rp[3][3];
	  double rbp[3][3];
	  double rn[3][3];
	  double rbpn[3][3];
	  iauPn06a(JD, 0.0, &dpsi, &deps, &epsa, rb, rp, rbp, rn, rbpn);
	  mpf_class result[3];
	  for(size_t i = 0; i < 3; i++)
	    {
	      result[i] = 0.0;
	    }
	  mpf_class xyz[3];
	  xyz[0] = *Oldx;
	  xyz[1] = *Oldy;
	  xyz[2] = *Oldz;

	  for(size_t i = 0; i < 3; i++)
	    {
	      for(size_t j = 0; j < 3; j++)
		{
		  result[i] = result[i] + rbpn[i][j] * xyz[j];
		}
	    }
	  *Oldx = result[0];
	  *Oldy = result[1];
	  *Oldz = result[2];
	  *Newx = *Oldx;
	  *Newy = *Oldz * Sin(epsa + deps) + *Oldy * Cos(epsa + deps);
	  *Newz = *Oldz * Cos(epsa + deps) - *Oldy * Sin(epsa + deps);
	}
    }
}

void
AuxFunc::precession(mpf_class *Oldx, mpf_class *Oldy, mpf_class *Oldz,
		    mpf_class *Newx, mpf_class *Newy, mpf_class *Newz,
		    double JD)
{
  double P[3][3];
  iauPmat06(JD, 0.0, P);
  mpf_class result[3];
  for(size_t i = 0; i < 3; i++)
    {
      result[i] = 0.0;
    }
  mpf_class xyz[3];
  xyz[0] = *Oldx;
  xyz[1] = *Oldy;
  xyz[2] = *Oldz;

  for(size_t i = 0; i < 3; i++)
    {
      for(size_t j = 0; j < 3; j++)
	{
	  result[i] = result[i] + P[i][j] * xyz[j];
	}
    }
  *Newx = result[0];
  *Newy = result[1];
  *Newz = result[2];
}

void
AuxFunc::precessionNnut(mpf_class *Oldx, mpf_class *Oldy, mpf_class *Oldz,
			mpf_class *Newx, mpf_class *Newy, mpf_class *Newz,
			double JD)
{
  double P[3][3];
  iauPnm06a(JD, 0.0, P);
  mpf_class result[3];
  for(size_t i = 0; i < 3; i++)
    {
      result[i] = 0.0;
    }
  mpf_class xyz[3];
  xyz[0] = *Oldx;
  xyz[1] = *Oldy;
  xyz[2] = *Oldz;

  for(size_t i = 0; i < 3; i++)
    {
      for(size_t j = 0; j < 3; j++)
	{
	  result[i] = result[i] + P[i][j] * xyz[j];
	}
    }
  *Newx = result[0];
  *Newy = result[1];
  *Newz = result[2];
}

mpf_class
AuxFunc::Sin(mpf_class x)
{
  mpf_set_default_prec(512);
  mpf_class TOLERANCE(0.0000000000000000001);
  mpf_class add(x), mul = -(x * x);
  mpf_class result(x);
  for(int i = 3; (add > TOLERANCE || add < -TOLERANCE); i += 2)
    {
      add = add * mul / (i * (i - 1));
      result += add;
    }
  return result;
}

mpf_class
AuxFunc::Cos(mpf_class x)
{
  mpf_set_default_prec(512);
  mpf_class TOLERANCE(0.0000000000000000001);
  mpf_class mul = -(x * x);
  mpf_class add = mul / 2;
  mpf_class result = 1 + add;
  for(int i = 4; (add > TOLERANCE || add < -TOLERANCE); i += 2)
    {
      add = add * mul / (i * (i - 1));
      result += add;
    }
  return result;
}

std::string
AuxFunc::utf8to(std::string line)
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr;
  UConverter *c = ucnv_open(NULL, &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  status = U_ZERO_ERROR;
  std::vector<char> target2;
  ustr.remove();
  ustr = icu::UnicodeString::fromUTF8(line.c_str());
  target2.resize(ustr.length());
  char16_t data[ustr.length()];
  for(int i = 0; i < ustr.length(); i++)
    {
      data[i] = ustr.charAt(i);
    }
  size_t cb = ucnv_fromUChars(c, target2.data(), ustr.length(), data,
			      ustr.length(), &status);
  if(!U_SUCCESS(status))
    {
      if(status == U_BUFFER_OVERFLOW_ERROR)
	{
	  status = U_ZERO_ERROR;
	  target2.clear();
	  target2.resize(cb);
	  ucnv_fromUChars(c, target2.data(), cb, data, ustr.length(), &status);
	  if(!U_SUCCESS(status))
	    {
	      std::cerr << u_errorName(status) << std::endl;
	    }
	}
      else
	{
	  std::cerr << u_errorName(status) << std::endl;
	}
    }

  line.clear();
  line = std::string(target2.begin(), target2.end());
  ucnv_close(c);

  return line;
}

void
AuxFunc::rotateXYZ(mpf_class xyz[3], mpf_class rx, mpf_class ry, mpf_class rz,
		   mpf_class result[3])
{
  mpf_class Mr[3][3];
  Mr[0][0] = Cos(rz) * Cos(ry);
  Mr[0][1] = Cos(rz) * Sin(ry) * Sin(rx) - Sin(rz) * Cos(rx);
  Mr[0][2] = Cos(rz) * Sin(ry) * Cos(rx) + Sin(rz) * Sin(rx);
  Mr[1][0] = Sin(rz) * Cos(ry);
  Mr[1][1] = Sin(rz) * Sin(ry) * Sin(rx) + Cos(rz) * Cos(rx);
  Mr[1][2] = Sin(rz) * Sin(ry) * Cos(rx) - Cos(rz) * Sin(rx);
  Mr[2][0] = -Sin(ry);
  Mr[2][1] = Cos(ry) * Sin(rx);
  Mr[2][2] = Cos(ry) * Cos(rx);
  for(size_t m = 0; m < 3; m++)
    {
      mpf_class res = 0.0;
      for(size_t j = 0; j < 3; j++)
	{
	  res = res + Mr[m][j] * xyz[j];
	}
      result[m] = res;
    }
}
