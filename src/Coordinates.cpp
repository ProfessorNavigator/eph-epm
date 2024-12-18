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

#include <AuxFunc.h>
#include <Coordinates.h>
#include <DAFOperations.h>
#include <EPMCalculations.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <gmp.h>
#include <iostream>

Coordinates::Coordinates(int body, double JD, int timesc, int coordtype,
                         int xyz, int theory, int unit, double step,
                         int stepnum, std::string epmpath,
                         std::string tttdbpath, std::string smlbpath,
                         std::atomic<int> *cancel)
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
  this->tttdbpath = tttdbpath;
  this->smlbpath = smlbpath;
  this->cancel = cancel;
  this->timesc = timesc;
}

std::vector<std::array<mpf_class, 3>>
Coordinates::calculationsXYZ()
{
  mpf_set_default_prec(512);
  std::vector<std::array<mpf_class, 3>> result;
  AuxFunc af;
  std::filesystem::path filepath;
  DAFOperations daf;

  std::fstream ephfile;
  filepath = std::filesystem::u8path(epmpath);
  ephfile.open(filepath, std::ios_base::in | std::ios_base::binary);

  filepath = std::filesystem::u8path(tttdbpath);
  std::fstream tttdbfile;
  tttdbfile.open(filepath, std::ios_base::in | std::ios_base::binary);

  filepath = std::filesystem::u8path(smlbpath);
  std::fstream smlbfile;
  smlbfile.open(filepath, std::ios_base::in | std::ios_base::binary);

  if(!ephfile.is_open())
    {
      std::cerr << "Cannot open ephemeris file" << std::endl;
      if(tttdbfile.is_open())
        {
          tttdbfile.close();
        }
    }
  else
    {
      std::string ephnm = daf.fileVersion(&ephfile);
      std::string::size_type n;
      mpf_class rho(0);
      mpf_class au(0);
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
      n = ephnm.find("SPKMERGE");
      if(n != std::string::npos)
        {
          rho = 81.3005691;
        }

      int bodymoontype = -1;
      uint64_t moonb, moone;
      if(body == 1800303)
        {
          n = ephnm.find("epm2015");
          if(n != std::string::npos)
            {
              body = 1800302;
            }
          else
            {
              n = ephnm.find("test.bps");
              if(n != std::string::npos)
                {
                  body = 31008;
                }
            }
        }
      EPMCalculations epm;
      uint64_t tdbb, tdbe;
      int tdbtype;
      uint64_t bodyb, bodye;
      int bodytype;
      n = ephnm.find("SPKMERGE");
      if(n == std::string::npos)
        {
          n = ephnm.find("sb441-n373.bsp");
        }
      if(rho == 0)
        {
          rho = 81.3005691;
        }
      if(au == 0)
        {
          au = 149597870.7;
        }

      for(double i = JD; i < JD + step * stepnum; i = i + step)
        {
          if(cancel->load() > 0)
            {
              break;
            }

          if(tttdbfile.is_open())
            {
              tdbtype = daf.bodyVect(&tttdbfile, &tdbb, &tdbe, 1000000001, i);
            }
          else
            {
              tdbtype = daf.bodyVect(&ephfile, &tdbb, &tdbe, 1000000001, i);
            }
          if(tdbtype < 0)
            {
              std::cerr << "Coordinates::calculationsXYZ: cannot find TDB, "
                           "coordinates have not been calculated!"
                        << std::endl;
              break;
            }

          mpf_class JDfin;
          double JDcalc = 0.0;
          if(timesc == 0)
            {
              JDcalc = af.timeTT(i);
              if(tttdbfile.is_open())
                {
                  JDfin = mpf_class(JDcalc)
                          - epm.tdbCalc(&tttdbfile, &tdbb, &tdbe, JDcalc,
                                        tdbtype)
                                / mpf_class(86400.0);
                }
              else
                {
                  JDfin
                      = mpf_class(JDcalc)
                        - epm.tdbCalc(&ephfile, &tdbb, &tdbe, JDcalc, tdbtype)
                              / mpf_class(86400.0);
                }
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
              if(tttdbfile.is_open())
                {
                  JDfin = mpf_class(JDcalc)
                          - epm.tdbCalc(&tttdbfile, &tdbb, &tdbe, JDcalc,
                                        tdbtype)
                                / mpf_class(86400.0);
                }
              else
                {
                  JDfin
                      = mpf_class(JDcalc)
                        - epm.tdbCalc(&ephfile, &tdbb, &tdbe, JDcalc, tdbtype)
                              / mpf_class(86400.0);
                }
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
          if(body != -3)
            {
              bodytype = daf.bodyVect(&ephfile, &bodyb, &bodye, body,
                                      JDfin.get_d());
            }
          else
            {
              bodytype
                  = daf.bodyVect(&ephfile, &bodyb, &bodye, 3, JDfin.get_d());
              bodytype
                  = daf.bodyVect(&ephfile, &moonb, &moone, 3, JDfin.get_d());
            }
          bool smbody = false;
          if(bodytype < 0)
            {
              if(smlbfile.is_open())
                {
                  bodytype = daf.bodyVect(&smlbfile, &bodyb, &bodye, body,
                                          JDfin.get_d());
                }
              if(bodytype < 0)
                {
                  std::cerr << "Coordinates::calculationsXYZ: cannot find "
                            << body
                            << ", coordinates have not been calculated!"
                            << std::endl;
                  break;
                }
              else
                {
                  smbody = true;
                }
            }
          mpf_class X;
          mpf_class Y;
          mpf_class Z;
          if(body != 3 && body != -3)
            {
              if(!smbody)
                {
                  X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au);
                  Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au);
                  Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au);
                }
              else
                {
                  X = epm.bodyCalcX(&smlbfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au);
                  Y = epm.bodyCalcY(&smlbfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au);
                  Z = epm.bodyCalcZ(&smlbfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au);
                }
            }
          else
            {
              bodymoontype
                  = daf.bodyVect(&ephfile, &moonb, &moone, 301, JDfin.get_d());
              if(body == 3)
                {
                  X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au)
                      - epm.bodyCalcX(&ephfile, &moonb, &moone, JDfin, xyz,
                                      bodymoontype, &au)
                            / rho;
                  Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au)
                      - epm.bodyCalcY(&ephfile, &moonb, &moone, JDfin, xyz,
                                      bodymoontype, &au)
                            / rho;
                  Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au)
                      - epm.bodyCalcZ(&ephfile, &moonb, &moone, JDfin, xyz,
                                      bodymoontype, &au)
                            / rho;
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
                                    bodytype, &au)
                      - moonx / rho + moonx;
                  Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au)
                      - moony / rho + moony;
                  Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                    bodytype, &au)
                      - moonz / rho + moonz;
                }
            }
          if(xyz == 0)
            {
              if(body != 1800303 && body != 1800302 && body != 31008)
                {
                  if(n != std::string::npos || smbody)
                    {
                      X = X / au;
                      Y = Y / au;
                      Z = Z / au;
                    }
                  if(unit == 1)
                    {
                      X = X * au;
                      Y = Y * au;
                      Z = Z * au;
                    }
                  else if(unit == 2)
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
              if(body != 1800303 && body != 1800302 && body != 31008)
                {
                  if(n != std::string::npos)
                    {
                      X = X / au;
                      Y = Y / au;
                      Z = Z / au;
                    }
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
                  if(n != std::string::npos)
                    {
                      X = X * 2;
                      Y = Y * 2;
                      Z = Z * 2;
                    }
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
      if(tttdbfile.is_open())
        {
          tttdbfile.close();
        }
      if(smlbfile.is_open())
        {
          smlbfile.close();
        }
    }
  return result;
}
