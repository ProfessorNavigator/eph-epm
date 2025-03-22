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

#include <AuxFunc.h>
#include <Coordinates.h>
#include <DAFOperations.h>
#include <EPMCalculations.h>
#include <cmath>
#include <filesystem>
#include <fstream>
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

std::vector<CoordKeeper>
Coordinates::calculationsXYZ()
{
  mpf_set_default_prec(512);
  std::vector<CoordKeeper> result;
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

      for(double i = JD; i < JD + step * stepnum; i += step)
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

          switch(timesc)
            {
            case 0:
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
                    JDfin = mpf_class(JDcalc)
                            - epm.tdbCalc(&ephfile, &tdbb, &tdbe, JDcalc,
                                          tdbtype)
                                  / mpf_class(86400.0);
                  }
                break;
              }
            case 1:
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
                    JDfin = mpf_class(JDcalc)
                            - epm.tdbCalc(&ephfile, &tdbb, &tdbe, JDcalc,
                                          tdbtype)
                                  / mpf_class(86400.0);
                  }
                break;
              }
            case 2:
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
                break;
              }
            default:
              break;
            }
          if(body != -3)
            {
              bodytype = daf.bodyVect(&ephfile, &bodyb, &bodye, body,
                                      JDfin.get_d());
            }
          else
            {
              daf.bodyVect(&ephfile, &bodyb, &bodye, 3, JDfin.get_d());
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

          CoordKeeper ckp;
          ckp.JD = i;
          if(body != 3 && body != -3)
            {
              if(!smbody)
                {
                  ckp.X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au);
                  ckp.Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au);
                  ckp.Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au);
                }
              else
                {
                  ckp.X = epm.bodyCalcX(&smlbfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au);
                  ckp.Y = epm.bodyCalcY(&smlbfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au);
                  ckp.Z = epm.bodyCalcZ(&smlbfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au);
                }
            }
          else
            {
              bodymoontype
                  = daf.bodyVect(&ephfile, &moonb, &moone, 301, JDfin.get_d());
              if(body == 3)
                {
                  ckp.X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au)
                          - epm.bodyCalcX(&ephfile, &moonb, &moone, JDfin, xyz,
                                          bodymoontype, &au)
                                / rho;
                  ckp.Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au)
                          - epm.bodyCalcY(&ephfile, &moonb, &moone, JDfin, xyz,
                                          bodymoontype, &au)
                                / rho;
                  ckp.Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
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
                  ckp.X = epm.bodyCalcX(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au)
                          - moonx / rho + moonx;
                  ckp.Y = epm.bodyCalcY(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au)
                          - moony / rho + moony;
                  ckp.Z = epm.bodyCalcZ(&ephfile, &bodyb, &bodye, JDfin, xyz,
                                        bodytype, &au)
                          - moonz / rho + moonz;
                }
            }
          switch(xyz)
            {
            case 0:
              {
                if(body != 1800303 && body != 1800302 && body != 31008)
                  {
                    if(n != std::string::npos || smbody)
                      {
                        ckp.X = ckp.X / au;
                        ckp.Y = ckp.Y / au;
                        ckp.Z = ckp.Z / au;
                      }
                    switch(unit)
                      {
                      case 1:
                        {
                          ckp.X = ckp.X * au;
                          ckp.Y = ckp.Y * au;
                          ckp.Z = ckp.Z * au;
                          break;
                        }
                      case 2:
                        {
                          mpf_class vl = au * 1000;
                          ckp.X = ckp.X * vl;
                          ckp.Y = ckp.Y * vl;
                          ckp.Z = ckp.Z * vl;
                          break;
                        }
                      default:
                        break;
                      }
                  }
                else
                  {
                    if(unit == 1)
                      {
                        double vl = 180 / M_PI;
                        ckp.X = ckp.X * vl;
                        ckp.Y = ckp.Y * vl;
                        ckp.Z = ckp.Z * vl;
                      }
                  }
                break;
              }
            case 1:
              {
                if(body != 1800303 && body != 1800302 && body != 31008)
                  {
                    if(n != std::string::npos)
                      {
                        ckp.X = ckp.X / au;
                        ckp.Y = ckp.Y / au;
                        ckp.Z = ckp.Z / au;
                      }

                    switch(unit)
                      {
                      case 1:
                        {
                          ckp.X = ckp.X * au;
                          ckp.Y = ckp.Y * au;
                          ckp.Z = ckp.Z * au;
                          break;
                        }
                      case 2:
                        {
                          mpf_class vl = au / 86400;
                          ckp.X = ckp.X * vl;
                          ckp.Y = ckp.Y * vl;
                          ckp.Z = ckp.Z * vl;
                          break;
                        }
                      case 3:
                        {
                          mpf_class vl = au / 86400 * 1000;
                          ckp.X = ckp.X * vl;
                          ckp.Y = ckp.Y * vl;
                          ckp.Z = ckp.Z * vl;
                          break;
                        }
                      default:
                        break;
                      }
                  }
                else
                  {
                    if(n != std::string::npos)
                      {
                        ckp.X = ckp.X * 2;
                        ckp.Y = ckp.Y * 2;
                        ckp.Z = ckp.Z * 2;
                      }
                    if(unit == 1)
                      {
                        double vl = 180 / M_PI * 3600;
                        ckp.X = ckp.X * vl;
                        ckp.Y = ckp.Y * vl;
                        ckp.Z = ckp.Z * vl;
                      }
                  }
                break;
              }
            default:
              break;
            }

          switch(coordtype)
            {
            case 0:
              {
                switch(theory)
                  {
                  case 1:
                    {
                      mpf_class Xn, Yn, Zn;
                      af.precession(&ckp.X, &ckp.Y, &ckp.Z, &Xn, &Yn, &Zn,
                                    JDcalc);
                      ckp.X = std::move(Xn);
                      ckp.Y = std::move(Yn);
                      ckp.Z = std::move(Zn);
                      break;
                    }
                  case 2:
                    {
                      mpf_class Xn, Yn, Zn;
                      af.precessionNnut(&ckp.X, &ckp.Y, &ckp.Z, &Xn, &Yn, &Zn,
                                        JDcalc);
                      ckp.X = std::move(Xn);
                      ckp.Y = std::move(Yn);
                      ckp.Z = std::move(Zn);
                      break;
                    }
                  default:
                    break;
                  }
                break;
              }
            case 1:
              {
                mpf_class Xn, Yn, Zn;
                af.toEcliptic(&ckp.X, &ckp.Y, &ckp.Z, &Xn, &Yn, &Zn, JDcalc,
                              theory);
                ckp.X = std::move(Xn);
                ckp.Y = std::move(Yn);
                ckp.Z = std::move(Zn);
                break;
              }
            default:
              break;
            }

          result.emplace_back(ckp);

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
