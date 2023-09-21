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

#include "Coordinates.h"

Coordinates::Coordinates(std::string body, double JD, int timesc, int coordtype,
			 int xyz, int theory, int unit, double step,
			 int stepnum, std::string epmpath, int *cancel)
{
  this->body = body;
  this->JD = JD;
  this->coordtype = coordtype;
  this->xyz = xyz;
  this->theory = theory;
  this->unit = unit;
  this->step = step;
  this->stepnum = stepnum;
  this->epmpath = epmpath;
  this->cancel = cancel;
  this->timesc = timesc;
}

Coordinates::~Coordinates()
{

}

std::vector<std::array<mpf_class, 3>>
Coordinates::calculationsXYZ()
{
  mpf_set_default_prec(512);
  std::vector<std::array<mpf_class, 3>> result;
  AuxFunc af;
  std::string Sharepath;
  std::filesystem::path filepath;
  Sharepath = epmpath;
  DAFOperations daf;
  std::fstream ephfile;
  filepath = std::filesystem::u8path(Sharepath);
  ephfile.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!ephfile.is_open())
    {
      std::cerr << "Cannot open ephemeris file" << std::endl;
    }
  else
    {
      std::string ephnm = daf.fileVersion(&ephfile);
      std::string::size_type n;
      mpf_class rho;
      mpf_class au;
      n = ephnm.find("epm2021");
      if(n != std::string::npos)
	{
	  rho = 81.300568886 + 1;
	}
      n = ephnm.find("epm2017");
      if(n != std::string::npos)
	{
	  rho = 81.300568886 + 1;
	}
      n = ephnm.find("epm2015");
      if(n != std::string::npos)
	{
	  rho = 81.3005676344 + 1;
	}
      int bodynum = -1;
      int bodymoontype = -1;
      uint64_t moonb, moone;
      if(body == "sun")
	{
	  bodynum = 10;
	}
      if(body == "mercury")
	{
	  bodynum = 1;
	}
      if(body == "venus")
	{
	  bodynum = 2;
	}
      if(body == "earth")
	{
	  bodynum = 3;
	  bodymoontype = daf.bodyVect(&ephfile, &moonb, &moone, 301);
	}
      if(body == "earth_m")
	{
	  bodynum = 3;
	}
      if(body == "mars")
	{
	  bodynum = 4;
	}
      if(body == "jupiter")
	{
	  bodynum = 5;
	}
      if(body == "saturn")
	{
	  bodynum = 6;
	}
      if(body == "uranus")
	{
	  bodynum = 7;
	}
      if(body == "neptune")
	{
	  bodynum = 8;
	}
      if(body == "pluto")
	{
	  bodynum = 9;
	}
      if(body == "moon")
	{
	  bodynum = 3;
	  bodymoontype = daf.bodyVect(&ephfile, &moonb, &moone, 301);
	}
      if(body == "moongeo")
	{
	  bodynum = 301;
	}
      if(body == "ceres")
	{
	  bodynum = 2000001;
	}
      if(body == "pallas")
	{
	  bodynum = 2000002;
	}
      if(body == "vesta")
	{
	  bodynum = 2000004;
	}
      if(body == "erida")
	{
	  bodynum = 2136199;
	}
      if(body == "haumea")
	{
	  bodynum = 2136108;
	}
      if(body == "makemake")
	{
	  bodynum = 2136472;
	}
      if(body == "sedna")
	{
	  bodynum = 2090377;
	}
      if(body == "bamberga")
	{
	  bodynum = 2000324;
	}
      if(body == "iris")
	{
	  bodynum = 2000007;
	}
      if(body == "moonlibr")
	{
	  bodynum = 1800303;
	  n = ephnm.find("epm2015");
	  if(n != std::string::npos)
	    {
	      bodynum = 1800302;
	    }
	}
      EPMCalculations epm;
      uint64_t tdbb, tdbe;
      int tdbtype = daf.bodyVect(&ephfile, &tdbb, &tdbe, 1000000001);
      uint64_t bodyb, bodye;
      int bodytype = daf.bodyVect(&ephfile, &bodyb, &bodye, bodynum);
      for(double i = JD; i < JD + step * stepnum; i = i + step)
	{
	  if(*cancel > 0)
	    {
	      break;
	    }

	  mpf_class JDfin;
	  double JDcalc = 0.0;
	  if(timesc == 0)
	    {
	      JDcalc = af.timeTT(i);
	      JDfin = mpf_class(JDcalc)
		  - epm.tdbCalc(&ephfile, &tdbb, &tdbe, JDcalc, tdbtype)
		      / mpf_class(86400.0);
	    }
	  if(timesc == 1)
	    {
	      if(i < 0)
		{
		  JDcalc = i + 0.5;
		}
	      else
		{
		  JDcalc = i - 0.5;
		}
	      if(JDcalc <= 2299161.0)
		{
		  JDcalc = af.grigToJuliancal(JDcalc);
		}
	      JDfin = mpf_class(JDcalc)
		  - epm.tdbCalc(&ephfile, &tdbb, &tdbe, JDcalc, tdbtype)
		      / mpf_class(86400.0);
	    }
	  if(timesc == 2)
	    {
	      if(i - 0.5 <= 2299161.0)
		{
		  if(i < 0)
		    {
		      JDfin = JDcalc = af.grigToJuliancal(i + 0.5);
		    }
		  else
		    {
		      JDfin = JDcalc = af.grigToJuliancal(i - 0.5);
		    }
		}
	    }
	  mpf_class X;
	  mpf_class Y;
	  mpf_class Z;
	  if(body != "moon" && body != "earth")
	    {
	      X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz, bodytype,
				&au);
	      Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz, bodytype,
				&au);
	      Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz, bodytype,
				&au);
	    }
	  else
	    {
	      if(body == "earth")
		{
		  X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
				    bodytype, &au)
		      - epm.bodyCalcX(&ephfile, &moonb, &moone, JDfin, xyz,
				      bodymoontype, &au) / rho;
		  Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
				    bodytype, &au)
		      - epm.bodyCalcY(&ephfile, &moonb, &moone, JDfin, xyz,
				      bodymoontype, &au) / rho;
		  Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
				    bodytype, &au)
		      - epm.bodyCalcZ(&ephfile, &moonb, &moone, JDfin, xyz,
				      bodymoontype, &au) / rho;
		}
	      else
		{
		  mpf_class moonx, moony, moonz;
		  moonx = epm.bodyCalcX(&ephfile, &moonb, &moone, JDfin, xyz,
					bodymoontype, &au);
		  moony = epm.bodyCalcY(&ephfile, &moonb, &moone, JDfin, xyz,
					bodymoontype, &au);
		  moonz = epm.bodyCalcZ(&ephfile, &moonb, &moone, JDfin, xyz,
					bodymoontype, &au);
		  X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
				    bodytype, &au) - moonx / rho + moonx;
		  Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
				    bodytype, &au) - moony / rho + moony;
		  Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
				    bodytype, &au) - moonz / rho + moonz;
		}
	    }
	  if(xyz == 0)
	    {
	      if(body != "moonlibr")
		{
		  if(unit == 1)
		    {
		      X = X * au;
		      Y = Y * au;
		      Z = Z * au;
		    }
		  if(unit == 2)
		    {
		      X = X * au * 1000;
		      Y = Y * au * 1000;
		      Z = Z * au * 1000;
		    }
		}
	      else
		{
		  if(unit == 1)
		    {
		      X = X * 180 / M_PI;
		      Y = Y * 180 / M_PI;
		      Z = Z * 180 / M_PI;
		    }
		}
	    }
	  if(xyz == 1)
	    {
	      if(body != "moonlibr")
		{
		  if(unit == 1)
		    {
		      X = X * au;
		      Y = Y * au;
		      Z = Z * au;
		    }
		  if(unit == 2)
		    {
		      X = X * au / 86400;
		      Y = Y * au / 86400;
		      Z = Z * au / 86400;
		    }
		  if(unit == 3)
		    {
		      X = X * au / 86400 * 1000;
		      Y = Y * au / 86400 * 1000;
		      Z = Z * au / 86400 * 1000;
		    }
		}
	      else
		{
		  if(unit == 1)
		    {
		      X = X * 180 / M_PI * 3600;
		      Y = Y * 180 / M_PI * 3600;
		      Z = Z * 180 / M_PI * 3600;
		    }
		}
	    }

	  if(theory == 1 && coordtype == 0)
	    {
	      mpf_class Xn, Yn, Zn;
	      af.precession(&X, &Y, &Z, &Xn, &Yn, &Zn, JDcalc);
	      X = Xn;
	      Y = Yn;
	      Z = Zn;
	    }
	  if(theory == 2 && coordtype == 0)
	    {
	      mpf_class Xn, Yn, Zn;
	      af.precessionNnut(&X, &Y, &Z, &Xn, &Yn, &Zn, JDcalc);
	      X = Xn;
	      Y = Yn;
	      Z = Zn;
	    }
	  if(coordtype == 1)
	    {
	      mpf_class Xn, Yn, Zn;
	      af.toEcliptic(&X, &Y, &Z, &Xn, &Yn, &Zn, JDcalc, theory);
	      X = Xn;
	      Y = Yn;
	      Z = Zn;
	    }

	  std::array<mpf_class, 3> resultarr;
	  resultarr[0] = X;
	  resultarr[1] = Y;
	  resultarr[2] = Z;
	  result.push_back(resultarr);
	  if(pulse_signal)
	    {
	      pulse_signal();
	    }
	}
      if(ephfile.is_open())
	{
	  ephfile.close();
	}
    }
  return result;
}

