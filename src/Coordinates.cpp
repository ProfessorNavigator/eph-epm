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
#include <fstream>
#include <iostream>
#include <mutex>

Coordinates::Coordinates(const int &body, const double &JD, const int &timesc,
                         const int &coordtype, const int &xyz,
                         const int &theory, const int &unit,
                         const double &step, const int &stepnum,
                         const std::string &epmpath,
                         const std::string &tttdbpath,
                         const std::string &smlbpath, std::atomic<int> *cancel)
{
  this->body = body;
  this->JD = JD;
  this->coordtype = coordtype;
  this->xyz = xyz;
  this->theory = theory;
  this->unit = unit;
  this->step = step;
  this->stepnum = stepnum;
  this->epmpath = std::filesystem::u8path(epmpath);
  this->tttdbpath = std::filesystem::u8path(tttdbpath);
  this->smlbpath = std::filesystem::u8path(smlbpath);
  this->cancel = cancel;
  this->timesc = timesc;
}

std::vector<CoordKeeper>
Coordinates::calculationsXYZ()
{
  mpf_set_default_prec(512);
  std::vector<CoordKeeper> result;
  AuxFunc af;
  DAFOperations daf;

  std::fstream ephfile;
  ephfile.open(epmpath, std::ios_base::in | std::ios_base::binary);

  std::fstream tttdbfile;
  if(epmpath != tttdbpath)
    {
      tttdbfile.open(tttdbpath, std::ios_base::in | std::ios_base::binary);
    }

  std::fstream smlbfile;
  if(epmpath != smlbpath)
    {
      smlbfile.open(smlbpath, std::ios_base::in | std::ios_base::binary);
    }

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

      result.reserve(stepnum);
      {
        double lim = JD + step * stepnum;
        double i = JD;
        while(i < lim)
          {
            CoordKeeper ckp;
            ckp.JD = i;
            result.emplace_back(ckp);
            i += step;
          }
      }

      std::atomic<bool> stop = false;
      std::mutex eph_mtx;
      std::mutex tdb_mtx;
      std::mutex sml_mtx;
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
      for(auto it = result.begin(); it != result.end(); it++)
        {
          if(cancel->load() > 0 || stop.load())
            {
              if(pulse_signal)
                {
                  pulse_signal();
                }
              continue;
            }
          uint64_t tdbb, tdbe;
          int tdbtype;
          uint64_t bodyb, bodye;
          int bodytype;
          if(tttdbfile.is_open())
            {
              tdb_mtx.lock();
              tdbtype
                  = daf.bodyVect(&tttdbfile, tdbb, tdbe, 1000000001, it->JD);
              tdb_mtx.unlock();
            }
          else
            {
              eph_mtx.lock();
              tdbtype = daf.bodyVect(&ephfile, tdbb, tdbe, 1000000001, it->JD);
              eph_mtx.unlock();
            }
          if(tdbtype < 0)
            {
              std::cerr << "Coordinates::calculationsXYZ: cannot find TDB, "
                           "coordinates have not been calculated!"
                        << std::endl;
              stop.store(true);
              if(pulse_signal)
                {
                  pulse_signal();
                }
              continue;
            }

          mpf_class JDfin;
          double JDcalc = 0.0;

          switch(timesc)
            {
            case 0:
              {
                JDcalc = af.timeTT(it->JD);
                if(tttdbfile.is_open())
                  {
                    tdb_mtx.lock();
                    JDfin = mpf_class(JDcalc)
                            - epm.tdbCalc(&tttdbfile, tdbb, tdbe, JDcalc,
                                          tdbtype)
                                  / mpf_class(86400.0);
                    tdb_mtx.unlock();
                  }
                else
                  {
                    eph_mtx.lock();
                    JDfin
                        = mpf_class(JDcalc)
                          - epm.tdbCalc(&ephfile, tdbb, tdbe, JDcalc, tdbtype)
                                / mpf_class(86400.0);
                    eph_mtx.unlock();
                  }
                break;
              }
            case 1:
              {
                if(it->JD < 0)
                  {
                    JDcalc = it->JD + 0.5;
                  }
                else
                  {
                    JDcalc = it->JD - 0.5;
                  }
                if(JDcalc <= 2299161.0)
                  {
                    JDcalc = af.grigToJuliancal(JDcalc);
                  }
                if(tttdbfile.is_open())
                  {
                    tdb_mtx.lock();
                    JDfin = mpf_class(JDcalc)
                            - epm.tdbCalc(&tttdbfile, tdbb, tdbe, JDcalc,
                                          tdbtype)
                                  / mpf_class(86400.0);
                    tdb_mtx.unlock();
                  }
                else
                  {
                    eph_mtx.lock();
                    JDfin
                        = mpf_class(JDcalc)
                          - epm.tdbCalc(&ephfile, tdbb, tdbe, JDcalc, tdbtype)
                                / mpf_class(86400.0);
                    eph_mtx.unlock();
                  }
                break;
              }
            case 2:
              {
                if(it->JD - 0.5 <= 2299161.0)
                  {
                    if(it->JD < 0)
                      {
                        JDfin = JDcalc = af.grigToJuliancal(it->JD + 0.5);
                      }
                    else
                      {
                        JDfin = JDcalc = af.grigToJuliancal(it->JD - 0.5);
                      }
                  }
                break;
              }
            default:
              break;
            }
          if(body != -3)
            {
              eph_mtx.lock();
              bodytype
                  = daf.bodyVect(&ephfile, bodyb, bodye, body, JDfin.get_d());
              eph_mtx.unlock();
            }
          else
            {
              eph_mtx.lock();
              daf.bodyVect(&ephfile, bodyb, bodye, 3, JDfin.get_d());
              bodytype
                  = daf.bodyVect(&ephfile, moonb, moone, 3, JDfin.get_d());
              eph_mtx.unlock();
            }
          bool smbody = false;
          if(bodytype < 0)
            {
              if(smlbfile.is_open())
                {
                  sml_mtx.lock();
                  bodytype = daf.bodyVect(&smlbfile, bodyb, bodye, body,
                                          JDfin.get_d());
                  sml_mtx.unlock();
                }
              if(bodytype < 0)
                {
                  std::cerr << "Coordinates::calculationsXYZ: cannot find "
                            << body
                            << ", coordinates have not been calculated!"
                            << std::endl;
                  stop.store(true);
                  if(pulse_signal)
                    {
                      pulse_signal();
                    }
                  continue;
                }
              else
                {
                  smbody = true;
                }
            }

          if(body != 3 && body != -3)
            {
              if(!smbody)
                {
                  eph_mtx.lock();
                  it->X = epm.bodyCalcX(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au);
                  it->Y = epm.bodyCalcY(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au);
                  it->Z = epm.bodyCalcZ(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au);
                  eph_mtx.unlock();
                }
              else
                {
                  sml_mtx.lock();
                  it->X = epm.bodyCalcX(&smlbfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au);
                  it->Y = epm.bodyCalcY(&smlbfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au);
                  it->Z = epm.bodyCalcZ(&smlbfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au);
                  sml_mtx.unlock();
                }
            }
          else
            {
              eph_mtx.lock();
              bodymoontype
                  = daf.bodyVect(&ephfile, moonb, moone, 301, JDfin.get_d());
              if(body == 3)
                {
                  it->X = epm.bodyCalcX(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au)
                          - epm.bodyCalcX(&ephfile, moonb, moone, JDfin, xyz,
                                          bodymoontype, au)
                                / rho;
                  it->Y = epm.bodyCalcY(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au)
                          - epm.bodyCalcY(&ephfile, moonb, moone, JDfin, xyz,
                                          bodymoontype, au)
                                / rho;
                  it->Z = epm.bodyCalcZ(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au)
                          - epm.bodyCalcZ(&ephfile, moonb, moone, JDfin, xyz,
                                          bodymoontype, au)
                                / rho;
                }
              else
                {
                  mpf_class moonx, moony, moonz;
                  moonx = epm.bodyCalcX(&ephfile, moonb, moone, JDfin, xyz,
                                        bodymoontype, au);
                  moony = epm.bodyCalcY(&ephfile, moonb, moone, JDfin, xyz,
                                        bodymoontype, au);
                  moonz = epm.bodyCalcZ(&ephfile, moonb, moone, JDfin, xyz,
                                        bodymoontype, au);
                  it->X = epm.bodyCalcX(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au)
                          - moonx / rho + moonx;
                  it->Y = epm.bodyCalcY(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au)
                          - moony / rho + moony;
                  it->Z = epm.bodyCalcZ(&ephfile, bodyb, bodye, JDfin, xyz,
                                        bodytype, au)
                          - moonz / rho + moonz;
                }
              eph_mtx.unlock();
            }
          switch(xyz)
            {
            case 0:
              {
                if(body != 1800303 && body != 1800302 && body != 31008)
                  {
                    if(n != std::string::npos || smbody)
                      {
                        it->X = it->X / au;
                        it->Y = it->Y / au;
                        it->Z = it->Z / au;
                      }
                    switch(unit)
                      {
                      case 1:
                        {
                          it->X = it->X * au;
                          it->Y = it->Y * au;
                          it->Z = it->Z * au;
                          break;
                        }
                      case 2:
                        {
                          mpf_class vl = au * 1000;
                          it->X = it->X * vl;
                          it->Y = it->Y * vl;
                          it->Z = it->Z * vl;
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
                        it->X = it->X * vl;
                        it->Y = it->Y * vl;
                        it->Z = it->Z * vl;
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
                        it->X = it->X / au;
                        it->Y = it->Y / au;
                        it->Z = it->Z / au;
                      }

                    switch(unit)
                      {
                      case 1:
                        {
                          it->X = it->X * au;
                          it->Y = it->Y * au;
                          it->Z = it->Z * au;
                          break;
                        }
                      case 2:
                        {
                          mpf_class vl = au / 86400;
                          it->X = it->X * vl;
                          it->Y = it->Y * vl;
                          it->Z = it->Z * vl;
                          break;
                        }
                      case 3:
                        {
                          mpf_class vl = au / 86400 * 1000;
                          it->X = it->X * vl;
                          it->Y = it->Y * vl;
                          it->Z = it->Z * vl;
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
                        it->X = it->X * 2;
                        it->Y = it->Y * 2;
                        it->Z = it->Z * 2;
                      }
                    if(unit == 1)
                      {
                        double vl = 180 / M_PI * 3600;
                        it->X = it->X * vl;
                        it->Y = it->Y * vl;
                        it->Z = it->Z * vl;
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
                      af.precession(it->X, it->Y, it->Z, Xn, Yn, Zn, JDcalc);
                      it->X = std::move(Xn);
                      it->Y = std::move(Yn);
                      it->Z = std::move(Zn);
                      break;
                    }
                  case 2:
                    {
                      mpf_class Xn, Yn, Zn;
                      af.precessionNnut(it->X, it->Y, it->Z, Xn, Yn, Zn,
                                        JDcalc);
                      it->X = std::move(Xn);
                      it->Y = std::move(Yn);
                      it->Z = std::move(Zn);
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
                af.toEcliptic(it->X, it->Y, it->Z, Xn, Yn, Zn, JDcalc, theory);
                it->X = std::move(Xn);
                it->Y = std::move(Yn);
                it->Z = std::move(Zn);
                break;
              }
            default:
              break;
            }

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
