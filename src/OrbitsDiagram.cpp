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
#include <OrbitsDiagram.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <libintl.h>
#include <mgl2/data.h>
#include <mgl2/type.h>

OrbitsDiagram::OrbitsDiagram(Gtk::Window *mw, const std::string &ephpath,
                             const std::string &tttdbpath,
                             const std::string &smlpath, const double &JD,
                             const int &timesc, const int &coordtype,
                             const int &theory, const double &plot_factor)
{
  this->mw = mw;
  this->ephpath = ephpath;
  this->tttdbpath = tttdbpath;
  this->smlpath = smlpath;
  this->JD = JD;
  daf = new DAFOperations();
  gr = new mglGraph;
  omp_init_lock(&coord_ptr_v_mtx);
  Gdk::Rectangle req = screenRes();
  Height = req.get_height();
  Width = req.get_width();
  this->coordtype = coordtype;
  this->theory = theory;
  this->timesc = timesc;
  std::fstream f;
  std::filesystem::path ephp = std::filesystem::u8path(ephpath);
  std::vector<SPKItem> chv;
  f.open(ephp, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      chv = daf->bodiesVector(&f);
      f.close();
    }

  auto itchv = std::find_if(chv.begin(), chv.end(), [](SPKItem &el) {
    return el.NAIF_body_id == 2090377;
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
      itchv = std::find_if(chv.begin(), chv.end(), [](SPKItem &el) {
        return el.NAIF_body_id == 2090377;
      });
      if(itchv != chv.end())
        {
          modbody = 2090377;
        }
      else
        {
          itchv = std::find_if(chv.begin(), chv.end(), [](SPKItem &el) {
            return el.NAIF_body_id == 9;
          });
          if(itchv != chv.end())
            {
              modbody = 9;
            }
          else
            {
#pragma omp atomic write
              cancel = true;
            }
        }
    }
  this->plot_factor = plot_factor;

  omp_set_dynamic(true);
  active_lvls = omp_get_max_active_levels();
  omp_set_max_active_levels(omp_get_supported_active_levels());
}

OrbitsDiagram::~OrbitsDiagram()
{
  delete daf;
  gr->SetPlotFactor(0);
  gr->Zoom(0.0, 0.0, 1.0, 1.0);
  delete gr;
  delete dw;
  omp_destroy_lock(&coord_ptr_v_mtx);
  omp_set_dynamic(false);
  omp_set_max_active_levels(active_lvls);
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
      ch = daf->epochCheckUTC(JD, timesc, epb, epe, ephpath);
    }
  else
    {
      ch = daf->epochCheckUTC(JD, timesc, epb, epe, tttdbpath);
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
OrbitsDiagram::stopAll()
{
#pragma omp atomic write
  cancel = true;
  omp_set_lock(&coord_ptr_v_mtx);
  for(size_t i = 0; i < coord_ptr_v.size(); i++)
    {
      coord_ptr_v[i]->stopAll();
    }
  omp_unset_lock(&coord_ptr_v_mtx);
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
          period * scale_factor, stepnum, ephpath, tttdbpath, smlpath);
      coord->pulse_signal = [this] {
        if(pulse_signal)
          {
            pulse_signal();
          }
      };

      omp_set_lock(&coord_ptr_v_mtx);
      coord_ptr_v.push_back(coord);
      omp_unset_lock(&coord_ptr_v_mtx);

      resultsed = coord->calculationsXYZ();

      omp_set_lock(&coord_ptr_v_mtx);
      coord_ptr_v.erase(
          std::remove(coord_ptr_v.begin(), coord_ptr_v.end(), coord),
          coord_ptr_v.end());
      omp_unset_lock(&coord_ptr_v_mtx);
      delete coord;
      double rng = 0.0;
      for(int i = 0; i < 3; i++)
        {
          auto minmaxel
              = std::minmax_element(resultsed.begin(), resultsed.end(),
                                    [i](CoordKeeper el1, CoordKeeper &el2) {
                                      switch(i)
                                        {
                                        case 0:
                                          {
                                            return el1.X > el2.X;
                                          }
                                        case 1:
                                          {
                                            return el1.Y > el2.Y;
                                          }
                                        case 2:
                                          {
                                            return el1.Z > el2.Z;
                                          }
                                        default:
                                          return false;
                                        }
                                    });
          if(minmaxel.first != resultsed.end())
            {
              switch(i)
                {
                case 0:
                  {
                    if(minmaxel.first->X.get_d() > rng)
                      {
                        rng = minmaxel.first->X.get_d();
                      }
                    break;
                  }
                case 1:
                  {
                    if(minmaxel.first->Y.get_d() > rng)
                      {
                        rng = minmaxel.first->Y.get_d();
                      }
                    break;
                  }
                case 2:
                  {
                    if(minmaxel.first->Z.get_d() > rng)
                      {
                        rng = minmaxel.first->Z.get_d();
                      }
                    break;
                  }
                default:
                  break;
                }
            }
          if(minmaxel.second != resultsed.end())
            {
              switch(i)
                {
                case 0:
                  {
                    if(std::abs(minmaxel.second->X.get_d()) > rng)
                      {
                        rng = std::abs(minmaxel.second->X.get_d());
                      }
                    break;
                  }
                case 1:
                  {
                    if(std::abs(minmaxel.second->Y.get_d()) > rng)
                      {
                        rng = std::abs(minmaxel.second->Y.get_d());
                      }
                    break;
                  }
                case 2:
                  {
                    if(std::abs(minmaxel.second->Z.get_d()) > rng)
                      {
                        rng = std::abs(minmaxel.second->Z.get_d());
                      }
                    break;
                  }
                default:
                  break;
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

#pragma omp parallel
#pragma omp for
      for(auto it = bodyv.begin(); it != bodyv.end(); it++)
        {
          bool cncl;
#pragma omp atomic read
          cncl = cancel;
          if(cncl)
            {
#pragma omp cancel for
              continue;
            }
          planetOrbCalc(*it);
        }
    }

  bool cncl;
#pragma omp atomic read
  cncl = cancel;
  if(cncl)
    {
      if(canceled_signal)
        {
          canceled_signal();
#pragma omp atomic write
          cancel = false;
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
OrbitsDiagram::planetOrbCalc(const std::tuple<int, double> &planettup)
{
  int body = std::get<0>(planettup);
  std::vector<CoordKeeper> result;

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
          stepnum, ephpath, tttdbpath, smlpath);
      coord->pulse_signal = [this] {
        if(this->pulse_signal)
          {
            this->pulse_signal();
          }
      };

      omp_set_lock(&coord_ptr_v_mtx);
      coord_ptr_v.push_back(coord);
      omp_unset_lock(&coord_ptr_v_mtx);

      result = coord->calculationsXYZ();
      omp_set_lock(&coord_ptr_v_mtx);
      coord_ptr_v.erase(
          std::remove(coord_ptr_v.begin(), coord_ptr_v.end(), coord),
          coord_ptr_v.end());
      omp_unset_lock(&coord_ptr_v_mtx);
      delete coord;
    }
  else
    {
      result = resultsed;
      resultsed.clear();
    }
  std::vector<double> X;
  X.reserve(result.size());
  std::vector<double> Y;
  Y.reserve(result.size());
  std::vector<double> Z;
  Z.reserve(result.size());
  for(auto it = result.begin(); it != result.end(); it++)
    {
      X.push_back(it->X.get_d());
      Y.push_back(it->Y.get_d());
      Z.push_back(it->Z.get_d());
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
  bodyBuilding(body);
}

void
OrbitsDiagram::diagramPlot()
{
  dw = new DiagramWidget(mw, gr);
  dw->diagram_close = diagram_close;
  dw->diagramPlot();
}

void
OrbitsDiagram::bodyBuilding(const int &body)
{
  Coordinates *coord = new Coordinates(body, JD, timesc, coordtype, 0, theory,
                                       0, 1.0, 1, ephpath, tttdbpath, smlpath);
  coord->pulse_signal = [this] {
    if(this->pulse_signal)
      {
        this->pulse_signal();
      }
  };

  omp_set_lock(&coord_ptr_v_mtx);
  coord_ptr_v.push_back(coord);
  omp_unset_lock(&coord_ptr_v_mtx);

  std::vector<CoordKeeper> result;
  result = coord->calculationsXYZ();

  omp_set_lock(&coord_ptr_v_mtx);
  coord_ptr_v.erase(std::remove(coord_ptr_v.begin(), coord_ptr_v.end(), coord),
                    coord_ptr_v.end());
  omp_unset_lock(&coord_ptr_v_mtx);

  delete coord;
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
      x = result.begin()->X.get_d();
      y = result.begin()->Y.get_d();
      z = result.begin()->Z.get_d();
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

  double lim1 = M_PI / 2.0;
  double lim2 = 2.0 * M_PI;
  double add = M_PI / 180.0;
  mpf_class vl1 = Rh / 149597870.7;
  mpf_class vl2 = Rh2 / 149597870.7;
  mpf_class vl3 = Rv / 149597870.7;

  std::vector<double> X;
  std::vector<double> Y;
  std::vector<double> Z;
  omp_lock_t xyz_mtx;
  omp_init_lock(&xyz_mtx);

  std::vector<double> phi_v;
  std::vector<double> tet_v;
  {
    double val = -M_PI / 2.0;
    while(val <= lim1)
      {
        phi_v.push_back(val);
        val += add;
      }

    val = 0.0;
    while(val <= lim2)
      {
        tet_v.push_back(val);
        val += add;
      }
  }

#pragma omp parallel
#pragma omp for
  for(auto it_phi = phi_v.begin(); it_phi != phi_v.end(); it_phi++)
    {
#pragma omp parallel
#pragma omp for
      for(auto it_tet = tet_v.begin(); it_tet != tet_v.end(); it_tet++)
        {
          mpf_class xyz[3];
          xyz[0] = vl1 * af.Cos(*it_phi) * af.Cos(*it_tet);
          xyz[1] = vl2 * af.Cos(*it_phi) * af.Sin(*it_tet);
          xyz[2] = vl3 * af.Sin(*it_phi);
          mpf_class result[3];
          af.rotateXYZ(xyz, 0.0, bet, alf, result);
          mpf_class Oldx(result[0]);
          mpf_class Oldy(result[1]);
          mpf_class Oldz(result[2]);
          mpf_class Newx, Newy, Newz;

          switch(coordtype)
            {
            case 0:
              {
                switch(theory)
                  {
                  case 0:
                    {
                      Newx = Oldx;
                      Newy = Oldy;
                      Newz = Oldz;
                      break;
                    }
                  case 1:
                    {
                      af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD);
                      break;
                    }
                  case 2:
                    {
                      af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                        JD);
                      break;
                    }
                  default:
                    break;
                  }
                break;
              }
            case 1:
              {
                af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD, theory);
                break;
              }
            default:
              break;
            }

          omp_set_lock(&xyz_mtx);
          X.push_back(x + Newx.get_d());
          Y.push_back(y + Newy.get_d());
          Z.push_back(z + Newz.get_d());
          omp_unset_lock(&xyz_mtx);
        }
    }

  omp_destroy_lock(&xyz_mtx);

  mglData Xb(X), Yb(Y), Zb(Z);
  X.clear();
  Y.clear();
  Z.clear();
  double range = std::sqrt(x * x + y * y + z * z);
  double vl = 2 / 149597870.7;
  X.push_back(x + Rh * vl);
  X.push_back(x + Rh * vl);
  Y.push_back(y + Rh2 * vl);
  Y.push_back(y + Rh2 * vl);
  Z.push_back(z + range);
  Z.push_back(z + Rv * vl);
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
            gr->Surf(Xb, Yb, Zb, "{xF27825}");
            break;
          }
        case 1:
          {
            gr->Surf(Xb, Yb, Zb, "{x999C99}");
            gr->Puts(p, af.utf8to(gettext("Mercury")).c_str(), "{x999C99}",
                     fontsize);
            gr->Plot(lX, lY, lZ, "{x999C99}");
            break;
          }
        case 2:
          {
            gr->Surf(Xb, Yb, Zb, "{xD49B3A}");
            gr->Puts(p, af.utf8to(gettext("Venus")).c_str(), "{xD49B3A}",
                     fontsize);
            gr->Plot(lX, lY, lZ, "{xD49B3A}");
            break;
          }
        case 3:
          {
            gr->Surf(Xb, Yb, Zb, "{x5C98C0}");
            gr->Puts(p, af.utf8to(gettext("Earth")).c_str(), "{x5C98C0}",
                     fontsize);
            gr->Plot(lX, lY, lZ, "{x5C98C0}");
            break;
          }
        case -3:
          {
            gr->Surf(Xb, Yb, Zb, "{x838383}");
            break;
          }
        case 4:
          {
            gr->Surf(Xb, Yb, Zb, "{xA07C65}");
            gr->Puts(p, af.utf8to(gettext("Mars")).c_str(), "{xA07C65}",
                     fontsize);
            gr->Plot(lX, lY, lZ, "{xA07C65}");
            break;
          }
        case 5:
          {
            gr->Surf(Xb, Yb, Zb, "{xDAD3C3}");
            gr->Puts(p, af.utf8to(gettext("Jupiter")).c_str(), "{xDAD3C3}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{xDAD3C3}");
            break;
          }
        case 6:
          {
            gr->Surf(Xb, Yb, Zb, "{xE0B978}");
            gr->Puts(p, af.utf8to(gettext("Saturn")).c_str(), "{xE0B978}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{xE0B978}");

            /*std::vector<double> Xrb1;
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

            mpf_class au = 149597870.7;
            mpf_class mult1 = mpf_class(67000.0) / au;
            mpf_class mult2 = mpf_class(74500.0) / au;
            mpf_class mult3 = mpf_class(92000.0) / au;
            mpf_class mult4 = mpf_class(117580.0) / au;
            mpf_class mult5 = mpf_class(122170.0) / au;
            mpf_class mult6 = mpf_class(136775.0) / au;
            for(double fi = 0.0; fi < lim2; fi += add)
              {
                mpf_class xyz[3];
                xyz[0] = mult1 * af.Cos(fi);
                xyz[1] = mult1 * af.Sin(fi);
                xyz[2] = 0.0;
                mpf_class result[3];
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                mpf_class Oldx(result[0]);
                mpf_class Oldy(result[1]);
                mpf_class Oldz(result[2]);
                mpf_class Newx, Newy, Newz;
                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }
                Xrb1.push_back(x + Newx.get_d());
                Yrb1.push_back(y + Newy.get_d());
                Zrb1.push_back(z + Newz.get_d());
                xyz[0] = mult2 * af.Cos(fi);
                xyz[1] = mult2 * af.Sin(fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                Xre1.push_back(x + Newx.get_d());
                Yre1.push_back(y + Newy.get_d());
                Zre1.push_back(z + Newz.get_d());

                xyz[0] = mult3 * af.Cos(fi);
                xyz[1] = mult3 * af.Sin(fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                Xre2.push_back(x + Newx.get_d());
                Yre2.push_back(y + Newy.get_d());
                Zre2.push_back(z + Newz.get_d());

                xyz[0] = mult4 * af.Cos(fi);
                xyz[1] = mult4 * af.Sin(fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                Xre3.push_back(x + Newx.get_d());
                Yre3.push_back(y + Newy.get_d());
                Zre3.push_back(z + Newz.get_d());

                xyz[0] = mult5 * af.Cos(fi);
                xyz[1] = mult5 * af.Sin(fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }
                Xrb4.push_back(x + Newx.get_d());
                Yrb4.push_back(y + Newy.get_d());
                Zrb4.push_back(z + Newz.get_d());

                xyz[0] = mult6 * af.Cos(fi);
                xyz[1] = mult6 * af.Sin(fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 3:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
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
            mglData xre4(Xre4), yre4(Yre4), zre4(Zre4);*/
            struct ring_val
            {
              double fi;

              double Xrb1;
              double Yrb1;
              double Zrb1;

              double Xre1;
              double Yre1;
              double Zre1;

              double Xre2;
              double Yre2;
              double Zre2;

              double Xre3;
              double Yre3;
              double Zre3;

              double Xrb4;
              double Yrb4;
              double Zrb4;

              double Xre4;
              double Yre4;
              double Zre4;
            };

            std::vector<ring_val> ring_vect;

            {
              double val = 0.0;
              while(val < lim2)
                {
                  ring_val vl;
                  vl.fi = val;
                  ring_vect.push_back(vl);
                  val += add;
                }
            }

            mpf_class au = 149597870.7;
            mpf_class mult1 = mpf_class(67000.0) / au;
            mpf_class mult2 = mpf_class(74500.0) / au;
            mpf_class mult3 = mpf_class(92000.0) / au;
            mpf_class mult4 = mpf_class(117580.0) / au;
            mpf_class mult5 = mpf_class(122170.0) / au;
            mpf_class mult6 = mpf_class(136775.0) / au;
#pragma omp parallel
#pragma omp for
            for(auto it_rv = ring_vect.begin(); it_rv != ring_vect.end();
                it_rv++)
              {
                mpf_class xyz[3];
                xyz[0] = mult1 * af.Cos(it_rv->fi);
                xyz[1] = mult1 * af.Sin(it_rv->fi);
                xyz[2] = 0.0;
                mpf_class result[3];
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                mpf_class Oldx(result[0]);
                mpf_class Oldy(result[1]);
                mpf_class Oldz(result[2]);
                mpf_class Newx, Newy, Newz;
                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                it_rv->Xrb1 = x + Newx.get_d();
                it_rv->Yrb1 = y + Newy.get_d();
                it_rv->Zrb1 = z + Newz.get_d();
                xyz[0] = mult2 * af.Cos(it_rv->fi);
                xyz[1] = mult2 * af.Sin(it_rv->fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                it_rv->Xre1 = x + Newx.get_d();
                it_rv->Yre1 = y + Newy.get_d();
                it_rv->Zre1 = z + Newz.get_d();

                xyz[0] = mult3 * af.Cos(it_rv->fi);
                xyz[1] = mult3 * af.Sin(it_rv->fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                it_rv->Xre2 = x + Newx.get_d();
                it_rv->Yre2 = y + Newy.get_d();
                it_rv->Zre2 = z + Newz.get_d();

                xyz[0] = mult4 * af.Cos(it_rv->fi);
                xyz[1] = mult4 * af.Sin(it_rv->fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }

                it_rv->Xre3 = x + Newx.get_d();
                it_rv->Yre3 = y + Newy.get_d();
                it_rv->Zre3 = z + Newz.get_d();

                xyz[0] = mult5 * af.Cos(it_rv->fi);
                xyz[1] = mult5 * af.Sin(it_rv->fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 2:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }
                it_rv->Xrb4 = x + Newx.get_d();
                it_rv->Yrb4 = y + Newy.get_d();
                it_rv->Zrb4 = z + Newz.get_d();

                xyz[0] = mult6 * af.Cos(it_rv->fi);
                xyz[1] = mult6 * af.Sin(it_rv->fi);
                xyz[2] = 0.0;
                af.rotateXYZ(xyz, 0.0, bet, alf, result);
                Oldx = result[0];
                Oldy = result[1];
                Oldz = result[2];

                switch(coordtype)
                  {
                  case 0:
                    {
                      switch(theory)
                        {
                        case 0:
                          {
                            Newx = Oldx;
                            Newy = Oldy;
                            Newz = Oldz;
                            break;
                          }
                        case 1:
                          {
                            af.precession(Oldx, Oldy, Oldz, Newx, Newy, Newz,
                                          JD);
                            break;
                          }
                        case 3:
                          {
                            af.precessionNnut(Oldx, Oldy, Oldz, Newx, Newy,
                                              Newz, JD);
                            break;
                          }
                        default:
                          break;
                        }
                      break;
                    }
                  case 1:
                    {
                      af.toEcliptic(Oldx, Oldy, Oldz, Newx, Newy, Newz, JD,
                                    theory);
                      break;
                    }
                  default:
                    break;
                  }
                it_rv->Xre4 = x + Newx.get_d();
                it_rv->Yre4 = y + Newy.get_d();
                it_rv->Zre4 = z + Newz.get_d();
              }

            mglData xrb1, yrb1, zrb1;
            mglData xre1, yre1, zre1;
            mglData xre2, yre2, zre2;
            mglData xre3, yre3, zre3;
            mglData xrb4, yrb4, zrb4;
            mglData xre4, yre4, zre4;

#pragma omp parallel
#pragma omp for
            for(int i = 1; i <= 6; i++)
              {
                std::vector<double> vl_v1;
                vl_v1.reserve(ring_vect.size());
                std::vector<double> vl_v2;
                vl_v2.reserve(ring_vect.size());
                std::vector<double> vl_v3;
                vl_v3.reserve(ring_vect.size());
                switch(i)
                  {
                  case 1:
                    {
                      for(auto it_rv = ring_vect.begin();
                          it_rv != ring_vect.end(); it_rv++)
                        {
                          vl_v1.push_back(it_rv->Xrb1);
                          vl_v2.push_back(it_rv->Yrb1);
                          vl_v3.push_back(it_rv->Zrb1);
                        }
                      xrb1.Set(vl_v1);
                      yrb1.Set(vl_v2);
                      zrb1.Set(vl_v3);
                      break;
                    }
                  case 2:
                    {
                      for(auto it_rv = ring_vect.begin();
                          it_rv != ring_vect.end(); it_rv++)
                        {
                          vl_v1.push_back(it_rv->Xre1);
                          vl_v2.push_back(it_rv->Yre1);
                          vl_v3.push_back(it_rv->Zre1);
                        }
                      xre1.Set(vl_v1);
                      yre1.Set(vl_v2);
                      zre1.Set(vl_v3);
                      break;
                    }
                  case 3:
                    {
                      for(auto it_rv = ring_vect.begin();
                          it_rv != ring_vect.end(); it_rv++)
                        {
                          vl_v1.push_back(it_rv->Xre2);
                          vl_v2.push_back(it_rv->Yre2);
                          vl_v3.push_back(it_rv->Zre2);
                        }
                      xre2.Set(vl_v1);
                      yre2.Set(vl_v2);
                      zre2.Set(vl_v3);
                      break;
                    }
                  case 4:
                    {
                      for(auto it_rv = ring_vect.begin();
                          it_rv != ring_vect.end(); it_rv++)
                        {
                          vl_v1.push_back(it_rv->Xre3);
                          vl_v2.push_back(it_rv->Yre3);
                          vl_v3.push_back(it_rv->Zre3);
                        }
                      xre3.Set(vl_v1);
                      yre3.Set(vl_v2);
                      zre3.Set(vl_v3);
                      break;
                    }
                  case 5:
                    {
                      for(auto it_rv = ring_vect.begin();
                          it_rv != ring_vect.end(); it_rv++)
                        {
                          vl_v1.push_back(it_rv->Xrb4);
                          vl_v2.push_back(it_rv->Yrb4);
                          vl_v3.push_back(it_rv->Zrb4);
                        }
                      xrb4.Set(vl_v1);
                      yrb4.Set(vl_v2);
                      zrb4.Set(vl_v3);
                      break;
                    }
                  case 6:
                    {
                      for(auto it_rv = ring_vect.begin();
                          it_rv != ring_vect.end(); it_rv++)
                        {
                          vl_v1.push_back(it_rv->Xre4);
                          vl_v2.push_back(it_rv->Yre4);
                          vl_v3.push_back(it_rv->Zre4);
                        }
                      xre4.Set(vl_v1);
                      yre4.Set(vl_v2);
                      zre4.Set(vl_v3);
                      break;
                    }
                  default:
                    break;
                  }
              }

            gr->Plot(xrb1, yrb1, zrb1, "{x564D48}");
            gr->Plot(xre1, yre1, zre1, "{x564D48}");
            gr->Region(xrb1, yrb1, zrb1, xre1, yre1, zre1, "{x564D48}");
            gr->Plot(xre2, yre2, zre2, "{xAC977E}");
            gr->Region(xre1, yre1, zre1, xre2, yre2, zre2, "{xAC977E}");
            gr->Plot(xre3, yre3, zre3, "{xC6AC8E}");
            gr->Region(xre2, yre2, zre2, xre3, yre3, zre3, "{xC6AC8E}");
            gr->Plot(xrb4, yrb4, zrb4, "{xE3C8AA}");
            gr->Plot(xre4, yre4, zre4, "{xE3C8AA}");
            gr->Region(xrb4, yrb4, zrb4, xre4, yre4, zre4, "{xE3C8AA}");
            break;
          }
        case 7:
          {
            gr->Surf(Xb, Yb, Zb, "{xC9EFF1}");
            gr->Puts(p, af.utf8to(gettext("Uranus")).c_str(), "{xC9EFF1}",
                     fontsize * 25);
            gr->Plot(lX, lY, lZ, "{xC9EFF1}");
            break;
          }
        case 8:
          {
            gr->Surf(Xb, Yb, Zb, "{x5389FD}");
            gr->Puts(p, af.utf8to(gettext("Neptune")).c_str(), "{x5389FD}",
                     fontsize * 25);
            gr->Plot(lX, lY, lZ, "{x5389FD}");
            break;
          }
        case 9:
          {
            gr->Surf(Xb, Yb, Zb, "{xD7B699}");
            gr->Puts(p, af.utf8to(gettext("Pluto")).c_str(), "{xD7B699}",
                     fontsize * 25);
            gr->Plot(lX, lY, lZ, "{xD7B699}");
            break;
          }
        case 2000001:
          {
            gr->Surf(Xb, Yb, Zb, "{xABABAB}");
            gr->Puts(p, af.utf8to(gettext("Ceres")).c_str(), "{xABABAB}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{xABABAB}");
            break;
          }
        case 2000002:
          {
            gr->Surf(Xb, Yb, Zb, "{xD7D7D7}");
            gr->Puts(p, af.utf8to(gettext("Pallas")).c_str(), "{xD7D7D7}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{xD7D7D7}");
            break;
          }
        case 2000004:
          {
            gr->Surf(Xb, Yb, Zb, "{x9C9686}");
            gr->Puts(p, af.utf8to(gettext("Vesta")).c_str(), "{x9C9686}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{x9C9686}");
            break;
          }
        case 2136199:
          {
            gr->Surf(Xb, Yb, Zb, "{xD1C1A9}");
            gr->Puts(p, af.utf8to(gettext("Erida")).c_str(), "{xD1C1A9}",
                     fontsize * 50);
            gr->Plot(lX, lY, lZ, "{xD1C1A9}");
            break;
          }
        case 2136108:
          {
            gr->Surf(Xb, Yb, Zb, "{x7D675D}");
            gr->Puts(p, af.utf8to(gettext("Haumea")).c_str(), "{x7D675D}",
                     fontsize * 100);
            gr->Plot(lX, lY, lZ, "{x7D675D}");
            break;
          }
        case 2136472:
          {
            gr->Surf(Xb, Yb, Zb, "{xB97B5A}");
            gr->Puts(p, af.utf8to(gettext("Makemake")).c_str(), "{xB97B5A}",
                     fontsize * 50);
            gr->Plot(lX, lY, lZ, "{xB97B5A}");
            break;
          }
        case 2090377:
          {
            gr->Surf(Xb, Yb, Zb, "{xE89579}");
            gr->Puts(p, af.utf8to(gettext("Sedna")).c_str(), "{xE89579}",
                     fontsize * 50);
            gr->Plot(lX, lY, lZ, "{xE89579}");
            break;
          }
        case 2000324:
          {
            gr->Surf(Xb, Yb, Zb, "{xC8C8C8}");
            gr->Puts(p, af.utf8to(gettext("Bamberga")).c_str(), "{xC8C8C8}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{xC8C8C8}");
            break;
          }
        case 2000007:
          {
            gr->Surf(Xb, Yb, Zb, "{xC6C6C6}");
            gr->Puts(p, af.utf8to(gettext("Iris")).c_str(), "{xC6C6C6}",
                     fontsize * 3);
            gr->Plot(lX, lY, lZ, "{xC6C6C6}");
            break;
          }
        case 2225088:
          {
            gr->Surf(Xb, Yb, Zb, "{xF21515}");
            gr->Puts(p, af.utf8to(gettext("Gonggong")).c_str(), "{xF21515}",
                     fontsize * 50);
            gr->Plot(lX, lY, lZ, "{xF21515}");
            break;
          }
        case 2050000:
          {
            gr->Surf(Xb, Yb, Zb, "{x41C95A}");
            gr->Puts(p, af.utf8to(gettext("Quaoar")).c_str(), "{x41C95A}",
                     fontsize * 50);
            gr->Plot(lX, lY, lZ, "{x41C95A}");
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
