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
#include <OrbitsDiagram.h>
#include <algorithm>
#include <bits/std_abs.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gdkmm/surface.h>
#include <glibmm/refptr.h>
#include <libintl.h>
#include <mgl2/data.h>
#include <mgl2/type.h>
#ifndef USE_OPENMP
#include <thread>
#endif
#include <utility>
#ifdef USE_OPENMP
#include <omp.h>
#endif

OrbitsDiagram::OrbitsDiagram(Gtk::ApplicationWindow *mw, std::string ephpath,
                             std::string tttdbpath, std::string smlpath,
                             double JD, int timesc, int coordtype, int theory,
                             double plot_factor, std::atomic<int> *cancel)
{
  this->mw = mw;
  this->ephpath = ephpath;
  this->tttdbpath = tttdbpath;
  this->smlpath = smlpath;
  this->JD = JD;
  daf = new DAFOperations();
  gr = new mglGraph;
  grmtx = new std::mutex;
  this->cancel = cancel;
  Gdk::Rectangle req = screenRes();
  Height = req.get_height();
  Width = req.get_width();
  this->coordtype = coordtype;
  this->theory = theory;
  this->timesc = timesc;
  std::fstream f;
  std::filesystem::path ephp = std::filesystem::u8path(ephpath);
  std::vector<std::tuple<double, double, int, int, int, int, int, int>> chv;
  f.open(ephp, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string ephtype = daf->fileVersion(&f);
      std::string::size_type n;
      n = ephtype.find("epm");
      if(n != std::string::npos)
        {
          EPM = true;
        }
      chv = daf->bodiesVector(&f);
      f.close();
    }
  auto itchv = std::find_if(chv.begin(), chv.end(), [](auto &el) {
    return std::get<2>(el) == 2090377;
  });
  if(itchv != chv.end())
    {
      modbody = 2090377;
    }
  else
    {
      ephp = std::filesystem::u8path(smlpath);
      f.clear();
      f.open(ephp, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
        {
          chv = daf->bodiesVector(&f);
          f.close();
        }
      itchv = std::find_if(chv.begin(), chv.end(), [](auto &el) {
        return std::get<2>(el) == 2090377;
      });
      if(itchv != chv.end())
        {
          modbody = 2090377;
        }
      else
        {
          itchv = std::find_if(chv.begin(), chv.end(), [](auto &el) {
            return std::get<2>(el) == 9;
          });
          if(itchv != chv.end())
            {
              modbody = 9;
            }
          else
            {
              this->cancel->store(1);
            }
        }
    }
  this->plot_factor = plot_factor;
}

OrbitsDiagram::~OrbitsDiagram()
{
  delete daf;
  gr->SetPlotFactor(0);
  gr->Zoom(0.0, 0.0, 1.0, 1.0);
  delete gr;
  delete grmtx;
  delete dw;
}

int
OrbitsDiagram::calculateSize()
{
  int result = 0;
  bodyv.push_back(std::make_tuple(10, 0.0));
  bodyv.push_back(std::make_tuple(1, 88.0));
  bodyv.push_back(std::make_tuple(2, 224.7));
  bodyv.push_back(std::make_tuple(3, 365.2));
  bodyv.push_back(std::make_tuple(-3, 0.0));
  bodyv.push_back(std::make_tuple(4, 687.0));
  bodyv.push_back(std::make_tuple(5, 4331.0));
  bodyv.push_back(std::make_tuple(6, 10747.0));
  bodyv.push_back(std::make_tuple(7, 30589.0));
  bodyv.push_back(std::make_tuple(8, 59800.0));
  bodyv.push_back(std::make_tuple(9, 90560.0));
  bodyv.push_back(std::make_tuple(2000001, 1680.5));
  bodyv.push_back(std::make_tuple(2000002, 1686.643));
  bodyv.push_back(std::make_tuple(2000004, 1325.081));
  bodyv.push_back(std::make_tuple(2136199, 203830.0));
  bodyv.push_back(std::make_tuple(2136108, 104025.0));
  bodyv.push_back(std::make_tuple(2136472, 111867.0));
  bodyv.push_back(std::make_tuple(2090377, 4404480.0));
  bodyv.push_back(std::make_tuple(2000324, 1607.041));
  bodyv.push_back(std::make_tuple(2000007, 1345.337));
  bodyv.push_back(std::make_tuple(2225088, 202238.0));
  bodyv.push_back(std::make_tuple(2050000, 105495.0));

  bool ch;
  if(tttdbpath.empty())
    {
      ch = daf->epochCheckUTC(JD, timesc, &epb, &epe, ephpath);
    }
  else
    {
      ch = daf->epochCheckUTC(JD, timesc, &epb, &epe, tttdbpath);
    }

  if(ch)
    {
      double summa = 1.0 / scale_factor * static_cast<double>(bodyv.size() - 2)
                     + static_cast<double>(bodyv.size());
      result = static_cast<int>(std::ceil(summa));
    }
  return result;
}

void
OrbitsDiagram::calculateOrbits()
{
  int modbody = this->modbody;
  auto itpl = std::find_if(bodyv.begin(), bodyv.end(), [modbody](auto &el) {
    return std::get<0>(el) == modbody;
  });
  if(itpl != bodyv.end())
    {
      double period = std::get<1>(*itpl);
      double JDbeg;
      int stepnum = static_cast<int>(1 / scale_factor);
      AuxFunc af;
      if(epe - epb < period)
        {
          period = epe - epb - 0.2;
          JDbeg = epb + 0.1;
          while(af.timeTT(JDbeg) < epb + 0.1)
            {
              JDbeg = JDbeg + 0.1;
            }
          while(af.timeTT(JDbeg + period) > epe - 0.1)
            {
              period = period - 0.1;
            }
        }
      else
        {
          if(JD - period * 0.5 < epb + 0.1)
            {
              JDbeg = epb + 0.1;
              while(af.timeTT(JDbeg) < epb + 0.1)
                {
                  JDbeg = JDbeg + 0.1;
                }
              while(af.timeTT(JDbeg + period) > epe - 0.1)
                {
                  period = period - 0.1;
                }
            }
          else
            {
              if(JD + period * 0.5 > epe - 0.1)
                {
                  JDbeg = epe - 0.1 - period;
                  while(af.timeTT(JDbeg + period) > epe - 0.1)
                    {
                      period = period - 0.1;
                    }
                }
              else
                {
                  JDbeg = JD - 0.5 * period;
                  while(af.timeTT(JDbeg + 0.5 * period) > epe - 0.1)
                    {
                      period = period - 0.1;
                    }
                }
            }
        }

      Coordinates *coord = new Coordinates(
          std::get<0>(*itpl), JDbeg, timesc, coordtype, 0, theory, 0,
          period * scale_factor, stepnum, ephpath, tttdbpath, smlpath, cancel);
      coord->pulse_signal = [this] {
        if(pulse_signal)
          {
            pulse_signal();
          }
      };

      resultsed = coord->calculationsXYZ();
      delete coord;
      double rng = 0.0;
      for(int i = 0; i < 3; i++)
        {
          auto minmaxel = std::minmax_element(
              resultsed.begin(), resultsed.end(), [i](auto el1, auto &el2) {
                return el1.at(i) > el2.at(i);
              });
          if(minmaxel.first != resultsed.end())
            {
              std::array<mpf_class, 3> maxar = *(minmaxel.first);
              if(maxar.at(i).get_d() > rng)
                {
                  rng = maxar.at(i).get_d();
                }
            }
          if(minmaxel.second != resultsed.end())
            {
              std::array<mpf_class, 3> minar = *(minmaxel.second);
              if(std::abs(minar.at(i).get_d()) > rng)
                {
                  rng = std::abs(minar.at(i).get_d());
                }
            }
        }
      gr->SetSize(Width, Height);
      gr->SetRanges(-rng, rng, -rng, rng, -rng, rng);
      gr->Rotate(50, 60, 0);
      gr->Clf("k");
      gr->Axis("^x", "k");
      gr->SetPlotFactor(plot_factor);
      gr->SetQuality(3);

#ifdef USE_OPENMP
#pragma omp parallel for
      for(size_t i = 0; i < bodyv.size(); i++)
        {
          if(cancel->load() > 0)
            {
              continue;
            }
          std::tuple<int, double> planettup = bodyv[i];
          planetOrbCalc(planettup);
        }
#endif

#ifndef USE_OPENMP
      for(size_t i = 0; i < bodyv.size(); i++)
        {
          if(cancel->load() > 0)
            {
              break;
            }
          std::unique_lock<std::mutex> ulock(cyclemtx);
          thrnum++;
          thread_reg.wait(ulock, [this] {
            return this->thrnum <= std::thread::hardware_concurrency();
          });
          std::tuple<int, double> planettup;
          planettup = bodyv[i];
          std::thread *thr = new std::thread(
              std::bind(&OrbitsDiagram::planetOrbCalc, this, planettup));
          thr->detach();
          delete thr;
        }
#endif
    }

#ifndef USE_OPENMP
  std::unique_lock<std::mutex> ulock(cyclemtx);
  thread_reg.wait(ulock, [this] {
    return this->thrnum == 0;
  });
#endif

  if(cancel->load() > 0)
    {
      if(canceled_signal)
        {
          canceled_signal();
          cancel->store(0);
        }
    }
  else
    {
      if(calc_completed)
        {
          calc_completed();
        }
    }
}

void
OrbitsDiagram::planetOrbCalc(std::tuple<int, double> planettup)
{
  int body = std::get<0>(planettup);
  std::vector<std::array<mpf_class, 3>> result;

  if(body != 2090377)
    {
      double period = std::get<1>(planettup);
      double JDbeg;
      AuxFunc af;
      int stepnum = static_cast<int>(1 / scale_factor);
      if(epe - epb < period)
        {
          period = epe - epb - 0.2;
          JDbeg = epb + 0.1;
          while(af.timeTT(JDbeg) < epb + 0.1)
            {
              JDbeg = JDbeg + 0.1;
            }
          while(af.timeTT(JDbeg + period) > epe - 0.1)
            {
              period = period - 0.1;
            }
        }
      else
        {
          if(JD - period * 0.5 < epb + 0.1)
            {
              JDbeg = epb + 0.1;
              while(af.timeTT(JDbeg) < epb + 0.1)
                {
                  JDbeg = JDbeg + 0.1;
                }
              while(af.timeTT(JDbeg + period) > epe - 0.1)
                {
                  period = period - 0.1;
                }
            }
          else
            {
              if(JD + period * 0.5 > epe - 0.1)
                {
                  JDbeg = epe - 0.1 - period;
                  while(af.timeTT(JDbeg + period) > epe - 0.1)
                    {
                      period = period - 0.1;
                    }
                }
              else
                {
                  JDbeg = JD - 0.5 * period;
                  while(af.timeTT(JDbeg + 0.5 * period) > epe - 0.1)
                    {
                      period = period - 0.1;
                    }
                }
            }
        }

      Coordinates *coord = new Coordinates(
          body, JDbeg, timesc, coordtype, 0, theory, 0, period * scale_factor,
          stepnum, ephpath, tttdbpath, smlpath, cancel);
      coord->pulse_signal = [this] {
        if(this->pulse_signal)
          {
            this->pulse_signal();
          }
      };
      result = coord->calculationsXYZ();
      delete coord;
    }
  else
    {
      result = resultsed;
      resultsed.clear();
    }
  std::vector<double> X;
  std::vector<double> Y;
  std::vector<double> Z;
  for(size_t i = 0; i < result.size(); i++)
    {
      X.push_back(std::get<0>(result[i]).get_d());
      Y.push_back(std::get<1>(result[i]).get_d());
      Z.push_back(std::get<2>(result[i]).get_d());
    }
  mglData x(X), y(Y), z(Z);

  switch(body)
    {
    case 1:
      {
        gr->Plot(x, y, z, "{x999C99}");
        break;
      }
    case 2:
      {
        gr->Plot(x, y, z, "{xD49B3A}");
        break;
      }
    case 3:
      {
        gr->Plot(x, y, z, "{x5C98C0}");
        break;
      }
    case 4:
      {
        gr->Plot(x, y, z, "{xA07C65}");
        break;
      }
    case 5:
      {
        gr->Plot(x, y, z, "{xDAD3C3}");
        break;
      }
    case 6:
      {
        gr->Plot(x, y, z, "{xE0B978}");
        break;
      }
    case 7:
      {
        gr->Plot(x, y, z, "{xC9EFF1}");
        break;
      }
    case 8:
      {
        gr->Plot(x, y, z, "{x5389FD}");
        break;
      }
    case 9:
      {
        gr->Plot(x, y, z, "{xD7B699}");
        break;
      }
    case 2000001:
      {
        gr->Plot(x, y, z, "{xABABAB}");
        break;
      }
    case 2000002:
      {
        gr->Plot(x, y, z, "{xD7D7D7}");
        break;
      }
    case 2000004:
      {
        gr->Plot(x, y, z, "{x9C9686}");
        break;
      }
    case 2136199:
      {
        gr->Plot(x, y, z, "{xD1C1A9}");
        break;
      }
    case 2136108:
      {
        gr->Plot(x, y, z, "{x7D675D}");
        break;
      }
    case 2136472:
      {
        gr->Plot(x, y, z, "{xB97B5A}");
        break;
      }
    case 2090377:
      {
        gr->Plot(x, y, z, "{xE89579}");
        break;
      }
    case 2000324:
      {
        gr->Plot(x, y, z, "{xC8C8C8}");
        break;
      }
    case 2000007:
      {
        gr->Plot(x, y, z, "{xC6C6C6}");
        break;
      }
    case 2225088:
      {
        gr->Plot(x, y, z, "{xF21515}");
        break;
      }
    case 2050000:
      {
        gr->Plot(x, y, z, "{x41C95A}");
        break;
      }
    default:
      break;
    }

  bodyBuilding(body, gr);

#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lglock(cyclemtx);
  thrnum--;
  thread_reg.notify_one();
#endif
}

void
OrbitsDiagram::diagramPlot()
{
  dw = new DiagramWidget(mw, gr);
  dw->diagram_close = diagram_close;
  dw->diagramPlot();
}

void
OrbitsDiagram::bodyBuilding(int body, mglGraph *graph)
{
  Coordinates *coord
      = new Coordinates(body, JD, timesc, coordtype, 0, theory, 0, 1.0, 1,
                        ephpath, tttdbpath, smlpath, cancel);
  coord->pulse_signal = [this] {
    if(this->pulse_signal)
      {
        this->pulse_signal();
      }
  };
  std::vector<std::array<mpf_class, 3>> result;
  result = coord->calculationsXYZ();
  delete coord;
  std::vector<double> X;
  std::vector<double> Y;
  std::vector<double> Z;
  double Rv = 0.0;
  double Rh2 = 0.0;
  double Rh = 0.0;
  double alf = 0.0;
  double bet = 0.0;
  double T = 2451545.0;
  T = (JD - T) / 36525.0;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  AuxFunc af;
  if(result.size() > 0)
    {
      x = std::get<0>(result[0]).get_d();
      y = std::get<1>(result[0]).get_d();
      z = std::get<2>(result[0]).get_d();
    }
  switch(body)
    {
    case 10:
      {
        alf = 286.13 * M_PI / 180.0;
        bet = M_PI * 0.5 - 63.87 * M_PI / 180.0;
        Rh = 695510.0;
        Rh2 = Rh;
        Rv = 695510.0 - 695510.0 * 0.000009;
        break;
      }
    case 1:
      {
        alf = 281.01 - 0.033 * T;
        alf = alf * M_PI / 180.0;
        bet = 61.414 - 0.005 * T;
        bet = M_PI * 0.5 - bet * M_PI / 180.0;
        Rh = 2440.5;
        Rh2 = Rh;
        Rv = 2438.3;
        break;
      }
    case 2:
      {
        alf = 272.76 * M_PI / 180.0;
        bet = M_PI * 0.5 - 67.16 * M_PI / 180.0;
        Rh = 6051.8;
        Rh2 = Rh;
        Rv = 6051.8;
        break;
      }
    case 3:
      {
        Rh = 6378.137;
        Rh2 = Rh;
        Rv = 6356.752;
        break;
      }
    case -3:
      {
        Rh = 1738.1;
        Rh2 = Rh;
        Rv = 1736.0;
        break;
      }
    case 4:
      {
        alf = 317.681 - 0.106 * T;
        alf = alf * M_PI / 180;
        bet = 52.887 - 0.061 * T;
        bet = M_PI * 0.5 - bet * M_PI / 180.0;
        Rh = 3396.2;
        Rh2 = Rh;
        Rv = 3376.2;
        break;
      }
    case 5:
      {
        alf = 268.057 - 0.006 * T;
        alf = alf * M_PI / 180.0;
        bet = 64.495 + 0.002 * T;
        bet = M_PI * 0.5 - bet * M_PI / 180.0;
        Rh = 71492.0;
        Rh2 = Rh;
        Rv = 66854.0;
        break;
      }
    case 6:
      {
        alf = 40.589 - 0.036 * T;
        alf = alf * M_PI / 180;
        bet = 83.537 - 0.004 * T;
        bet = M_PI * 0.5 - bet * M_PI / 180;
        Rh = 60268.0;
        Rh2 = Rh;
        Rv = 54364.0;
        break;
      }
    case 7:
      {
        alf = 257.311 * M_PI / 180.0;
        bet = M_PI * 0.5 + 15.175 * M_PI / 180.0;
        Rh = 25559.0;
        Rh2 = Rh;
        Rv = 24973.0;
        break;
      }
    case 8:
      {
        double N = 357.85 + 52.316 * T;
        N = N * M_PI / 180.0;
        alf = 299.36 + 0.7 * af.Sin(N).get_d();
        alf = alf * M_PI / 180.0;
        bet = 43.46 + 0.51 * af.Cos(N).get_d();
        bet = M_PI * 0.5 - bet * M_PI / 180.0;
        Rh = 24764.0;
        Rh2 = Rh;
        Rv = 24341.0;
        break;
      }
    case 9:
      {
        alf = 132.99 * M_PI / 180.0;
        bet = M_PI * 0.5 + 6.16 * M_PI / 180.0;
        Rh = 1188.0;
        Rh2 = Rh;
        Rv = 1188.0;
        break;
      }
    case 2000001:
      {
        Rh = 965.0 * 0.5;
        Rh2 = 961.0 * 0.5;
        Rv = 891.0 * 0.5;
        break;
      }
    case 2000002:
      {
        Rh = 582.0 * 0.5;
        Rh2 = 556.0 * 0.5;
        Rv = 500.0 * 0.5;
        break;
      }
    case 2000004:
      {
        Rh = 569.0 * 0.5;
        Rh2 = 555.0 * 0.5;
        Rv = 453.0 * 0.5;
        break;
      }
    case 2136199:
      {
        Rh = 1163.0;
        Rh2 = 1163.0;
        Rv = 1163.0;
        break;
      }
    case 2136108:
      {
        Rh = 2322.0;
        Rh2 = 1704.0;
        Rv = 1138.0;
        break;
      }
    case 2136472:
      {
        Rh = 751.0;
        Rh2 = 751.0;
        Rv = 715.0;
        break;
      }
    case 2090377:
      {
        Rh = 995.0;
        Rh2 = 995.0;
        Rv = 995.0;
        break;
      }
    case 2000324:
      {
        Rh = 114.72;
        Rh2 = 114.72;
        Rv = 114.72;
        break;
      }
    case 2000007:
      {
        Rh = 99.915;
        Rh2 = 99.915;
        Rv = 99.915;
        break;
      }
    case 2225088:
      {
        Rh = 1532;
        Rh2 = 1280;
        Rv = 1280;
        break;
      }
    case 2050000:
      {
        Rh = 1086;
        Rh2 = 1036;
        Rv = 1086;
        break;
      }
    default:
      break;
    }

  for(double phi = -M_PI / 2.0; phi <= M_PI / 2.0; phi = phi + M_PI / 180.0)
    {
      for(double tet = 0.0; tet <= 2.0 * M_PI; tet = tet + M_PI / 180.0)
        {
          mpf_class xyz[3];
          xyz[0] = mpf_class(Rh / 149597870.7) * af.Cos(phi) * af.Cos(tet);
          xyz[1] = mpf_class(Rh2 / 149597870.7) * af.Cos(phi) * af.Sin(tet);
          xyz[2] = mpf_class(Rv / 149597870.7) * af.Sin(phi);
          mpf_class result[3];
          af.rotateXYZ(xyz, 0.0, bet, alf, result);
          mpf_class Oldx(result[0]);
          mpf_class Oldy(result[1]);
          mpf_class Oldz(result[2]);
          mpf_class Newx, Newy, Newz;
          if(coordtype == 0)
            {
              if(theory == 0)
                {
                  Newx = Oldx;
                  Newy = Oldy;
                  Newz = Oldz;
                }
              if(theory == 1)
                {
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                    JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                            theory);
            }
          X.push_back(x + Newx.get_d());
          Y.push_back(y + Newy.get_d());
          Z.push_back(z + Newz.get_d());
        }
    }
  mglData Xb(X), Yb(Y), Zb(Z);
  X.clear();
  Y.clear();
  Z.clear();
  double range = std::sqrt(x * x + y * y + z * z);
  X.push_back(x + Rh * 2 / 149597870.7);
  X.push_back(x + Rh * 2 / 149597870.7);
  Y.push_back(y + Rh2 * 2 / 149597870.7);
  Y.push_back(y + Rh2 * 2 / 149597870.7);
  Z.push_back(z + range);
  Z.push_back(z + Rv * 2 / 149597870.7);
  Xb.Rearrange(180, 361);
  Yb.Rearrange(180, 361);
  Zb.Rearrange(180, 361);

  mglPoint p(X[0], Y[0], Z[0]);
  mglData lX(X), lY(Y), lZ(Z);
  double fontsize = plot_factor * 5 * 1000000;
  if(x != 0.0 || y != 0.0 || z != 0.0)
    {
      switch(body)
        {
        case 10:
          {
            graph->Surf(Xb, Yb, Zb, "{xF27825}");
            break;
          }
        case 1:
          {
            graph->Surf(Xb, Yb, Zb, "{x999C99}");
            graph->Puts(p, af.utf8to(gettext("Mercury")).c_str(), "{x999C99}",
                        fontsize);
            graph->Plot(lX, lY, lZ, "{x999C99}");
            break;
          }
        case 2:
          {
            graph->Surf(Xb, Yb, Zb, "{xD49B3A}");
            graph->Puts(p, af.utf8to(gettext("Venus")).c_str(), "{xD49B3A}",
                        fontsize);
            graph->Plot(lX, lY, lZ, "{xD49B3A}");
            break;
          }
        case 3:
          {
            graph->Surf(Xb, Yb, Zb, "{x5C98C0}");
            graph->Puts(p, af.utf8to(gettext("Earth")).c_str(), "{x5C98C0}",
                        fontsize);
            graph->Plot(lX, lY, lZ, "{x5C98C0}");
            break;
          }
        case -3:
          {
            graph->Surf(Xb, Yb, Zb, "{x838383}");
            break;
          }
        case 4:
          {
            graph->Surf(Xb, Yb, Zb, "{xA07C65}");
            graph->Puts(p, af.utf8to(gettext("Mars")).c_str(), "{xA07C65}",
                        fontsize);
            graph->Plot(lX, lY, lZ, "{xA07C65}");
            break;
          }
        case 5:
          {
            graph->Surf(Xb, Yb, Zb, "{xDAD3C3}");
            graph->Puts(p, af.utf8to(gettext("Jupiter")).c_str(), "{xDAD3C3}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{xDAD3C3}");
            break;
          }
        case 6:
          {
            graph->Surf(Xb, Yb, Zb, "{xE0B978}");
            graph->Puts(p, af.utf8to(gettext("Saturn")).c_str(), "{xE0B978}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{xE0B978}");

            std::vector<double> Xrb1;
            std::vector<double> Yrb1;
            std::vector<double> Zrb1;

            std::vector<double> Xre1;
            std::vector<double> Yre1;
            std::vector<double> Zre1;

            std::vector<double> Xre2;
            std::vector<double> Yre2;
            std::vector<double> Zre2;

            std::vector<double> Xre3;
            std::vector<double> Yre3;
            std::vector<double> Zre3;

            std::vector<double> Xrb4;
            std::vector<double> Yrb4;
            std::vector<double> Zrb4;

            std::vector<double> Xre4;
            std::vector<double> Yre4;
            std::vector<double> Zre4;

            for(double fi = 0.0; fi < 2.0 * M_PI; fi = fi + M_PI / 180.0)
              {
                mpf_class xyz[3];
                xyz[0] = 67000.0 * af.Cos(fi) / 149597870.7;
                xyz[1] = 67000.0 * af.Sin(fi) / 149597870.7;
                xyz[2] = 0.0;
                mpf_class result[3];
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                mpf_class Oldx(result[0]);
                mpf_class Oldy(result[1]);
                mpf_class Oldz(result[2]);
                mpf_class Newx, Newy, Newz;
                if(coordtype == 0)
                  {
                    if(theory == 0)
                      {
                        Newx = Oldx;
                        Newy = Oldy;
                        Newz = Oldz;
                      }
                    if(theory == 1)
                      {
                        af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                      JD);
                      }
                    if(theory == 2)
                      {
                        af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy,
                                          &Newz, JD);
                      }
                  }
                if(coordtype == 1)
                  {
                    af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                                  theory);
                  }
                Xrb1.push_back(x + Newx.get_d());
                Yrb1.push_back(y + Newy.get_d());
                Zrb1.push_back(z + Newz.get_d());
                xyz[0] = 74500.0 * af.Cos(fi) / 149597870.7;
                xyz[1] = 74500.0 * af.Sin(fi) / 149597870.7;
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];
                if(coordtype == 0)
                  {
                    if(theory == 0)
                      {
                        Newx = Oldx;
                        Newy = Oldy;
                        Newz = Oldz;
                      }
                    if(theory == 1)
                      {
                        af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                      JD);
                      }
                    if(theory == 2)
                      {
                        af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy,
                                          &Newz, JD);
                      }
                  }
                if(coordtype == 1)
                  {
                    af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                                  theory);
                  }
                Xre1.push_back(x + Newx.get_d());
                Yre1.push_back(y + Newy.get_d());
                Zre1.push_back(z + Newz.get_d());

                xyz[0] = 92000.0 * af.Cos(fi) / 149597870.7;
                xyz[1] = 92000.0 * af.Sin(fi) / 149597870.7;
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];
                if(coordtype == 0)
                  {
                    if(theory == 0)
                      {
                        Newx = Oldx;
                        Newy = Oldy;
                        Newz = Oldz;
                      }
                    if(theory == 1)
                      {
                        af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                      JD);
                      }
                    if(theory == 2)
                      {
                        af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy,
                                          &Newz, JD);
                      }
                  }
                if(coordtype == 1)
                  {
                    af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                                  theory);
                  }
                Xre2.push_back(x + Newx.get_d());
                Yre2.push_back(y + Newy.get_d());
                Zre2.push_back(z + Newz.get_d());

                xyz[0] = 117580.0 * af.Cos(fi) / 149597870.7;
                xyz[1] = 117580.0 * af.Sin(fi) / 149597870.7;
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];
                if(coordtype == 0)
                  {
                    if(theory == 0)
                      {
                        Newx = Oldx;
                        Newy = Oldy;
                        Newz = Oldz;
                      }
                    if(theory == 1)
                      {
                        af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                      JD);
                      }
                    if(theory == 2)
                      {
                        af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy,
                                          &Newz, JD);
                      }
                  }
                if(coordtype == 1)
                  {
                    af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                                  theory);
                  }
                Xre3.push_back(x + Newx.get_d());
                Yre3.push_back(y + Newy.get_d());
                Zre3.push_back(z + Newz.get_d());

                xyz[0] = 122170.0 * af.Cos(fi) / 149597870.7;
                xyz[1] = 122170.0 * af.Sin(fi) / 149597870.7;
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];
                if(coordtype == 0)
                  {
                    if(theory == 0)
                      {
                        Newx = Oldx;
                        Newy = Oldy;
                        Newz = Oldz;
                      }
                    if(theory == 1)
                      {
                        af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                      JD);
                      }
                    if(theory == 2)
                      {
                        af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy,
                                          &Newz, JD);
                      }
                  }
                if(coordtype == 1)
                  {
                    af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                                  theory);
                  }
                Xrb4.push_back(x + Newx.get_d());
                Yrb4.push_back(y + Newy.get_d());
                Zrb4.push_back(z + Newz.get_d());

                xyz[0] = 136775.0 * af.Cos(fi) / 149597870.7;
                xyz[1] = 136775.0 * af.Sin(fi) / 149597870.7;
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];
                if(coordtype == 0)
                  {
                    if(theory == 0)
                      {
                        Newx = Oldx;
                        Newy = Oldy;
                        Newz = Oldz;
                      }
                    if(theory == 1)
                      {
                        af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz,
                                      JD);
                      }
                    if(theory == 2)
                      {
                        af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy,
                                          &Newz, JD);
                      }
                  }
                if(coordtype == 1)
                  {
                    af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD,
                                  theory);
                  }
                Xre4.push_back(x + Newx.get_d());
                Yre4.push_back(y + Newy.get_d());
                Zre4.push_back(z + Newz.get_d());
              }
            mglData xrb1(Xrb1), yrb1(Yrb1), zrb1(Zrb1);
            mglData xre1(Xre1), yre1(Yre1), zre1(Zre1);
            mglData xre2(Xre2), yre2(Yre2), zre2(Zre2);
            mglData xre3(Xre3), yre3(Yre3), zre3(Zre3);
            mglData xrb4(Xrb4), yrb4(Yrb4), zrb4(Zrb4);
            mglData xre4(Xre4), yre4(Yre4), zre4(Zre4);
            graph->Plot(xrb1, yrb1, zrb1, "{x564D48}");
            graph->Plot(xre1, yre1, zre1, "{x564D48}");
            graph->Region(xrb1, yrb1, zrb1, xre1, yre1, zre1, "{x564D48}");
            graph->Plot(xre2, yre2, zre2, "{xAC977E}");
            graph->Region(xre1, yre1, zre1, xre2, yre2, zre2, "{xAC977E}");
            graph->Plot(xre3, yre3, zre3, "{xC6AC8E}");
            graph->Region(xre2, yre2, zre2, xre3, yre3, zre3, "{xC6AC8E}");
            graph->Plot(xrb4, yrb4, zrb4, "{xE3C8AA}");
            graph->Plot(xre4, yre4, zre4, "{xE3C8AA}");
            graph->Region(xrb4, yrb4, zrb4, xre4, yre4, zre4, "{xE3C8AA}");
            break;
          }
        case 7:
          {
            graph->Surf(Xb, Yb, Zb, "{xC9EFF1}");
            graph->Puts(p, af.utf8to(gettext("Uranus")).c_str(), "{xC9EFF1}",
                        fontsize * 25);
            graph->Plot(lX, lY, lZ, "{xC9EFF1}");
            break;
          }
        case 8:
          {
            graph->Surf(Xb, Yb, Zb, "{x5389FD}");
            graph->Puts(p, af.utf8to(gettext("Neptune")).c_str(), "{x5389FD}",
                        fontsize * 25);
            graph->Plot(lX, lY, lZ, "{x5389FD}");
            break;
          }
        case 9:
          {
            graph->Surf(Xb, Yb, Zb, "{xD7B699}");
            graph->Puts(p, af.utf8to(gettext("Pluto")).c_str(), "{xD7B699}",
                        fontsize * 25);
            graph->Plot(lX, lY, lZ, "{xD7B699}");
            break;
          }
        case 2000001:
          {
            graph->Surf(Xb, Yb, Zb, "{xABABAB}");
            graph->Puts(p, af.utf8to(gettext("Ceres")).c_str(), "{xABABAB}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{xABABAB}");
            break;
          }
        case 2000002:
          {
            graph->Surf(Xb, Yb, Zb, "{xD7D7D7}");
            graph->Puts(p, af.utf8to(gettext("Pallas")).c_str(), "{xD7D7D7}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{xD7D7D7}");
            break;
          }
        case 2000004:
          {
            graph->Surf(Xb, Yb, Zb, "{x9C9686}");
            graph->Puts(p, af.utf8to(gettext("Vesta")).c_str(), "{x9C9686}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{x9C9686}");
            break;
          }
        case 2136199:
          {
            graph->Surf(Xb, Yb, Zb, "{xD1C1A9}");
            graph->Puts(p, af.utf8to(gettext("Erida")).c_str(), "{xD1C1A9}",
                        fontsize * 50);
            graph->Plot(lX, lY, lZ, "{xD1C1A9}");
            break;
          }
        case 2136108:
          {
            graph->Surf(Xb, Yb, Zb, "{x7D675D}");
            graph->Puts(p, af.utf8to(gettext("Haumea")).c_str(), "{x7D675D}",
                        fontsize * 100);
            graph->Plot(lX, lY, lZ, "{x7D675D}");
            break;
          }
        case 2136472:
          {
            graph->Surf(Xb, Yb, Zb, "{xB97B5A}");
            graph->Puts(p, af.utf8to(gettext("Makemake")).c_str(), "{xB97B5A}",
                        fontsize * 50);
            graph->Plot(lX, lY, lZ, "{xB97B5A}");
            break;
          }
        case 2090377:
          {
            graph->Surf(Xb, Yb, Zb, "{xE89579}");
            graph->Puts(p, af.utf8to(gettext("Sedna")).c_str(), "{xE89579}",
                        fontsize * 50);
            graph->Plot(lX, lY, lZ, "{xE89579}");
            break;
          }
        case 2000324:
          {
            graph->Surf(Xb, Yb, Zb, "{xC8C8C8}");
            graph->Puts(p, af.utf8to(gettext("Bamberga")).c_str(), "{xC8C8C8}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{xC8C8C8}");
            break;
          }
        case 2000007:
          {
            graph->Surf(Xb, Yb, Zb, "{xC6C6C6}");
            graph->Puts(p, af.utf8to(gettext("Iris")).c_str(), "{xC6C6C6}",
                        fontsize * 3);
            graph->Plot(lX, lY, lZ, "{xC6C6C6}");
            break;
          }
        case 2225088:
          {
            graph->Surf(Xb, Yb, Zb, "{xF21515}");
            graph->Puts(p, af.utf8to(gettext("Gonggong")).c_str(), "{xF21515}",
                        fontsize * 50);
            graph->Plot(lX, lY, lZ, "{xF21515}");
            break;
          }
        case 2050000:
          {
            graph->Surf(Xb, Yb, Zb, "{x41C95A}");
            graph->Puts(p, af.utf8to(gettext("Quaoar")).c_str(), "{x41C95A}",
                        fontsize * 50);
            graph->Plot(lX, lY, lZ, "{x41C95A}");
            break;
          }
        default:
          break;
        }
    }
}

Gdk::Rectangle
OrbitsDiagram::screenRes()
{
  Glib::RefPtr<Gdk::Surface> surf = mw->get_surface();
  Glib::RefPtr<Gdk::Display> disp = mw->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);
  return req;
}
