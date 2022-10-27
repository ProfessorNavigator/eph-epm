/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

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

#include "OrbitsDiagram.h"

OrbitsDiagram::OrbitsDiagram(Gtk::ApplicationWindow *mw, std::string ephpath,
                             double JD, int timesc, int coordtype, int theory, int *cancel)
{
  this->mw = mw;
  this->ephpath = ephpath;
  this->JD = JD;
  daf = new DAFOperations();
  gr = new mglGraph;
  grmtx = new std::mutex;
  threadvmtx = new std::mutex;
  this->cancel = cancel;
  Gdk::Rectangle req = screenRes();
  Height = req.get_height();
  Width = req.get_width();
  this->coordtype = coordtype;
  this->theory = theory;
  this->timesc = timesc;
}

OrbitsDiagram::~OrbitsDiagram()
{
  delete daf;
  gr->SetPlotFactor(0);
  gr->Zoom(0.0, 0.0, 1.0, 1.0);
  delete gr;
  delete grmtx;
  delete threadvmtx;
  delete dw;
}

int
OrbitsDiagram::calculateSize()
{
  int result = 0;
  bodyv.push_back(std::make_tuple("sun", 0.0));
  bodyv.push_back(std::make_tuple("mercury", 88.0));
  bodyv.push_back(std::make_tuple("venus", 224.7));
  bodyv.push_back(std::make_tuple("earth", 365.2));
  bodyv.push_back(std::make_tuple("moon", 0.0));
  bodyv.push_back(std::make_tuple("mars", 687.0));
  bodyv.push_back(std::make_tuple("jupiter", 4331.0));
  bodyv.push_back(std::make_tuple("saturn", 10747.0));
  bodyv.push_back(std::make_tuple("uranus", 30589.0));
  bodyv.push_back(std::make_tuple("neptune", 59800.0));
  bodyv.push_back(std::make_tuple("pluto", 90560.0));
  bodyv.push_back(std::make_tuple("ceres", 1680.5));
  bodyv.push_back(std::make_tuple("pallas", 1686.643));
  bodyv.push_back(std::make_tuple("vesta", 1325.081));
  bodyv.push_back(std::make_tuple("erida", 203830.0));
  bodyv.push_back(std::make_tuple("haumea", 104025.0));
  bodyv.push_back(std::make_tuple("makemake", 111867.0));
  bodyv.push_back(std::make_tuple("sedna", 4404480.0));
  bodyv.push_back(std::make_tuple("bamberga", 1607.041));
  bodyv.push_back(std::make_tuple("iris", 1345.337));

  bool ch = daf->epochCheckUTC(JD, timesc, &epb, &epe, ephpath);
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
  auto itpl = std::find_if(bodyv.begin(), bodyv.end(), [](auto & el)
  {
    return std::get<0>(el) == "sedna";
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

      Coordinates *coord = new Coordinates("sedna", JDbeg, timesc, coordtype, 0,
                                           theory, 0,
                                           period * scale_factor, stepnum, ephpath, cancel);
      coord->pulse_signal = [this]
      {
        if(this->pulse_signal)
          {
            this->pulse_signal();
          }
      };

      resultsed = coord->calculationsXYZ();
      delete coord;
      double rng = 0.0;
      for(int i = 0; i < 3; i++)
        {
          auto minmaxel = std::minmax_element(resultsed.begin(),
                                              resultsed.end(), [i](auto el1,
                                                  auto & el2)
          {
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

      for(size_t i = 0; i < bodyv.size(); i++)
        {
          if(*cancel > 0)
            {
              break;
            }
          threadvmtx->lock();
          size_t thrnum = threadv.size();
          threadvmtx->unlock();
          if(thrnum < std::thread::hardware_concurrency())
            {
              std::tuple<std::string, double>planettup;
              planettup = bodyv[i];
              std::thread *thr = new std::thread(std::bind(&OrbitsDiagram::planetOrbCalc,
                                                 this, planettup));
              threadvmtx->lock();
              threadv.push_back(std::get<0>(planettup));
              threadvmtx->unlock();
              thr->detach();
              delete thr;
            }
          else
            {
              cyclemtx.lock();
              std::tuple<std::string, double>planettup;
              planettup = bodyv[i];
              std::thread *thr = new std::thread(std::bind(&OrbitsDiagram::planetOrbCalc,
                                                 this, planettup));
              threadvmtx->lock();
              threadv.push_back(std::get<0>(planettup));
              threadvmtx->unlock();
              thr->detach();
              delete thr;
            }
        }
    }

}

void
OrbitsDiagram::planetOrbCalc(std::tuple<std::string, double> planettup)
{
  std::string body = std::get<0>(planettup);
  std::string bodyc;
  if(body == "earth")
    {
      bodyc = "earth_m";
    }
  else
    {
      bodyc = body;
    }

  std::vector<std::array<mpf_class, 3>> result;

  if(body != "sedna")
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

      Coordinates *coord = new Coordinates(bodyc, JDbeg, timesc, coordtype, 0, theory,
                                           0,
                                           period * scale_factor, stepnum, ephpath, cancel);
      coord->pulse_signal = [this]
      {
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
  std::vector<double>X;
  std::vector<double>Y;
  std::vector<double>Z;
  for(size_t i = 0; i < result.size(); i++)
    {
      X.push_back(std::get<0>(result[i]).get_d());
      Y.push_back(std::get<1>(result[i]).get_d());
      Z.push_back(std::get<2>(result[i]).get_d());
    }
  mglData x(X), y(Y), z(Z);

  if(body == "mercury")
    {
      gr->Plot(x, y, z, "{x999C99}");
    }
  if(body == "venus")
    {
      gr->Plot(x, y, z, "{xD49B3A}");
    }
  if(body == "earth")
    {
      gr->Plot(x, y, z, "{x5C98C0}");
    }
  if(body == "mars")
    {
      gr->Plot(x, y, z, "{xA07C65}");
    }
  if(body == "jupiter")
    {
      gr->Plot(x, y, z, "{xDAD3C3}");
    }
  if(body == "saturn")
    {
      gr->Plot(x, y, z, "{xE0B978}");
    }
  if(body == "uranus")
    {
      gr->Plot(x, y, z, "{xC9EFF1}");
    }
  if(body == "neptune")
    {
      gr->Plot(x, y, z, "{x5389FD}");
    }
  if(body == "pluto")
    {
      gr->Plot(x, y, z, "{xD7B699}");
    }
  if(body == "ceres")
    {
      gr->Plot(x, y, z, "{xABABAB}");
    }
  if(body == "pallas")
    {
      gr->Plot(x, y, z, "{xD7D7D7}");
    }
  if(body == "vesta")
    {
      gr->Plot(x, y, z, "{x9C9686}");
    }
  if(body == "erida")
    {
      gr->Plot(x, y, z, "{xD1C1A9}");
    }
  if(body == "haumea")
    {
      gr->Plot(x, y, z, "{x7D675D}");
    }
  if(body == "makemake")
    {
      gr->Plot(x, y, z, "{xB97B5A}");
    }
  if(body == "sedna")
    {
      gr->Plot(x, y, z, "{xE89579}");
    }
  if(body == "bamberga")
    {
      gr->Plot(x, y, z, "{xC8C8C8}");
    }
  if(body == "iris")
    {
      gr->Plot(x, y, z, "{xC6C6C6}");
    }
  bodyBuilding(body, gr);
  threadvmtx->lock();
  threadv.erase(std::remove(threadv.begin(), threadv.end(), body), threadv.end());
  size_t ch = threadv.size();
  threadvmtx->unlock();
  if(cyclemtx.try_lock())
    {
      cyclemtx.unlock();
    }
  else
    {
      cyclemtx.unlock();
    }
  if(ch == 0 && *cancel == 0)
    {
      std::string mgl_warn(gr->Message());
      if(!mgl_warn.empty())
        {
          std::cerr << "MathGL warning: " << mgl_warn << std::endl;
        }
      if(calc_completed)
        {
          calc_completed();
        }
    }
  else
    {
      if(ch == 0 && canceled_signal)
        {
          canceled_signal();
        }
    }
}

void
OrbitsDiagram::diagramPlot()
{
  dw = new DiagramWidget(mw, gr);
  dw->diagram_close = diagram_close;
  dw->diagramPlot();
}

void
OrbitsDiagram::bodyBuilding(std::string body, mglGraph *graph)
{
  Coordinates *coord = new Coordinates(body, JD, timesc, coordtype, 0, theory, 0,
                                       1.0, 1, ephpath, cancel);
  coord->pulse_signal = [this]
  {
    if(this->pulse_signal)
      {
        this->pulse_signal();
      }
  };
  std::vector<std::array<mpf_class, 3>> result;
  result = coord->calculationsXYZ();
  delete coord;
  std::vector<double>X;
  std::vector<double>Y;
  std::vector<double>Z;
  double Rv;
  double Rh2;
  double Rh;
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
  if(body == "sun")
    {
      alf = 286.13 * M_PI / 180.0;
      bet = M_PI * 0.5 - 63.87 * M_PI / 180.0;
      Rh = 695510.0;
      Rh2 = Rh;
      Rv = 695510.0 - 695510.0 * 0.000009;
    }
  if(body == "mercury")
    {
      alf = 281.01 - 0.033 * T;
      alf = alf * M_PI / 180.0;
      bet = 61.414 - 0.005 * T;
      bet = M_PI * 0.5 - bet * M_PI / 180.0;
      Rh = 2440.5;
      Rh2 = Rh;
      Rv = 2438.3;
    }
  if(body == "venus")
    {
      alf = 272.76 * M_PI / 180.0;
      bet = M_PI * 0.5 - 67.16 * M_PI / 180.0;
      Rh = 6051.8;
      Rh2 = Rh;
      Rv = 6051.8;
    }
  if(body == "earth")
    {
      Rh = 6378.137;
      Rh2 = Rh;
      Rv = 6356.752;
    }
  if(body == "moon")
    {
      Rh = 1738.1;
      Rh2 = Rh;
      Rv = 1736.0;
    }
  if(body == "mars")
    {
      alf = 317.681 - 0.106 * T;
      alf = alf * M_PI / 180;
      bet = 52.887 - 0.061 * T;
      bet = M_PI * 0.5 - bet * M_PI / 180.0;
      Rh = 3396.2;
      Rh2 = Rh;
      Rv = 3376.2;
    }
  if(body == "jupiter")
    {
      alf = 268.057 - 0.006 * T;
      alf = alf * M_PI / 180.0;
      bet = 64.495 + 0.002 * T;
      bet = M_PI * 0.5 - bet * M_PI / 180.0;
      Rh = 71492.0;
      Rh2 = Rh;
      Rv = 66854.0;
    }
  if(body == "saturn")
    {
      alf = 40.589 - 0.036 * T;
      alf = alf * M_PI / 180;
      bet = 83.537 - 0.004 * T;
      bet = M_PI * 0.5 - bet * M_PI / 180;
      Rh = 60268.0;
      Rh2 = Rh;
      Rv = 54364.0;
    }
  if(body == "uranus")
    {
      alf = 257.311 * M_PI / 180.0;
      bet = M_PI * 0.5 + 15.175 * M_PI / 180.0;
      Rh = 25559.0;
      Rh2 = Rh;
      Rv = 24973.0;
    }
  if(body == "neptune")
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
    }
  if(body == "pluto")
    {
      alf = 132.99 * M_PI / 180.0;
      bet = M_PI * 0.5 + 6.16 * M_PI / 180.0;
      Rh = 1188.0;
      Rh2 = Rh;
      Rv = 1188.0;
    }
  if(body == "сeres")
    {
      Rh = 965.0 * 0.5;
      Rh2 = 961.0 * 0.5;
      Rv = 891.0 * 0.5;
    }
  if(body == "pallas")
    {
      Rh = 582.0 * 0.5;
      Rh2 = 556.0 * 0.5;
      Rv = 500.0 * 0.5;
    }
  if(body == "vesta")
    {
      Rh = 569.0 * 0.5;
      Rh2 = 555.0 * 0.5;
      Rv = 453.0 * 0.5;
    }
  if(body == "erida")
    {
      Rh = 1163.0;
      Rh2 = 1163.0;
      Rv = 1163.0;
    }
  if(body == "haumea")
    {
      Rh = 2322.0;
      Rh2 = 1704.0;
      Rv = 1138.0;
    }
  if(body == "makemake")
    {
      Rh = 751.0;
      Rh2 = 751.0;
      Rv = 715.0;
    }
  if(body == "sedna")
    {
      Rh = 995.0;
      Rh2 = 995.0;
      Rv = 995.0;
    }
  if(body == "bamberga")
    {
      Rh = 114.72;
      Rh2 = 114.72;
      Rv = 114.72;
    }
  if(body == "iris")
    {
      Rh = 99.915;
      Rh2 = 99.915;
      Rv = 99.915;
    }
  for(double phi = -M_PI / 2.0; phi <= M_PI / 2.0; phi = phi +  M_PI / 180.0)
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
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
  Z.push_back(z  + range);
  Z.push_back(z + Rv * 2 / 149597870.7);
  Xb.Rearrange(180, 361);
  Yb.Rearrange(180, 361);
  Zb.Rearrange(180, 361);

  mglPoint p(X[0], Y[0], Z[0]);
  mglData lX(X), lY(Y), lZ(Z);
  double fontsize = plot_factor * 5 * 1000000;
  if(body == "sun" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xF27825}");
    }
  if(body == "mercury" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{x999C99}");
      graph->Puts(p, af.utf8to(gettext("Mercury")).c_str(), "{x999C99}", fontsize);
      graph->Plot(lX, lY, lZ, "{x999C99}");
    }
  if(body == "venus" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xD49B3A}");
      graph->Puts(p, af.utf8to(gettext("Venus")).c_str(), "{xD49B3A}", fontsize);
      graph->Plot(lX, lY, lZ, "{xD49B3A}");
    }
  if(body == "earth" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{x5C98C0}");
      graph->Puts(p, af.utf8to(gettext("Earth")).c_str(), "{x5C98C0}", fontsize);
      graph->Plot(lX, lY, lZ, "{x5C98C0}");
    }
  if(body == "moon" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{x838383}");
    }
  if(body == "mars" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xA07C65}");
      graph->Puts(p, af.utf8to(gettext("Mars")).c_str(), "{xA07C65}", fontsize);
      graph->Plot(lX, lY, lZ, "{xA07C65}");
    }
  if(body == "jupiter" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xDAD3C3}");
      graph->Puts(p, af.utf8to(gettext("Jupiter")).c_str(), "{xDAD3C3}",
                  fontsize * 3);
      graph->Plot(lX, lY, lZ, "{xDAD3C3}");
    }
  if(body == "saturn" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xE0B978}");
      graph->Puts(p, af.utf8to(gettext("Saturn")).c_str(), "{xE0B978}",
                  fontsize * 3);
      graph->Plot(lX, lY, lZ, "{xE0B978}");

      std::vector<double>Xrb1;
      std::vector<double>Yrb1;
      std::vector<double>Zrb1;

      std::vector<double>Xre1;
      std::vector<double>Yre1;
      std::vector<double>Zre1;

      std::vector<double>Xre2;
      std::vector<double>Yre2;
      std::vector<double>Zre2;

      std::vector<double>Xre3;
      std::vector<double>Yre3;
      std::vector<double>Zre3;

      std::vector<double>Xrb4;
      std::vector<double>Yrb4;
      std::vector<double>Zrb4;

      std::vector<double>Xre4;
      std::vector<double>Yre4;
      std::vector<double>Zre4;

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
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
                  af.precession(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
              if(theory == 2)
                {
                  af.precessionNnut(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD);
                }
            }
          if(coordtype == 1)
            {
              af.toEcliptic(&Oldx, &Oldy, &Oldz, &Newx, &Newy, &Newz, JD, theory);
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
    }
  if(body == "uranus" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xC9EFF1}");
      graph->Puts(p, af.utf8to(gettext("Uranus")).c_str(), "{xC9EFF1}",
                  fontsize * 25);
      graph->Plot(lX, lY, lZ, "{xC9EFF1}");
    }
  if(body == "neptune" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{x5389FD}");
      graph->Puts(p, af.utf8to(gettext("Neptune")).c_str(), "{x5389FD}",
                  fontsize * 25);
      graph->Plot(lX, lY, lZ, "{x5389FD}");
    }
  if(body == "pluto" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xD7B699}");
      graph->Puts(p, af.utf8to(gettext("Pluto")).c_str(), "{xD7B699}",
                  fontsize * 25);
      graph->Plot(lX, lY, lZ, "{xD7B699}");
    }
  if(body == "ceres" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xABABAB}");
      graph->Puts(p, af.utf8to(gettext("Ceres")).c_str(), "{xABABAB}", fontsize * 3);
      graph->Plot(lX, lY, lZ, "{xABABAB}");
    }
  if(body == "pallas" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xD7D7D7}");
      graph->Puts(p, af.utf8to(gettext("Pallas")).c_str(), "{xD7D7D7}", fontsize * 3);
      graph->Plot(lX, lY, lZ, "{xD7D7D7}");
    }
  if(body == "vesta" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{x9C9686}");
      graph->Puts(p, af.utf8to(gettext("Vesta")).c_str(), "{x9C9686}", fontsize * 3);
      graph->Plot(lX, lY, lZ, "{x9C9686}");
    }
  if(body == "erida" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xD1C1A9}");
      graph->Puts(p, af.utf8to(gettext("Erida")).c_str(), "{xD1C1A9}",
                  fontsize * 50);
      graph->Plot(lX, lY, lZ, "{xD1C1A9}");
    }
  if(body == "haumea" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{x7D675D}");
      graph->Puts(p, af.utf8to(gettext("Haumea")).c_str(), "{x7D675D}",
                  fontsize * 100);
      graph->Plot(lX, lY, lZ, "{x7D675D}");
    }
  if(body == "makemake" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xB97B5A}");
      graph->Puts(p, af.utf8to(gettext("Makemake")).c_str(), "{xB97B5A}",
                  fontsize * 50);
      graph->Plot(lX, lY, lZ, "{xB97B5A}");
    }
  if(body == "sedna" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xE89579}");
      graph->Puts(p, af.utf8to(gettext("Sedna")).c_str(), "{xE89579}",
                  fontsize * 50);
      graph->Plot(lX, lY, lZ, "{xE89579}");
    }
  if(body == "bamberga" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xC8C8C8}");
      graph->Puts(p, af.utf8to(gettext("Bamberga")).c_str(), "{xC8C8C8}",
                  fontsize * 3);
      graph->Plot(lX, lY, lZ, "{xC8C8C8}");
    }
  if(body == "iris" && (x != 0.0 || y != 0.0 || z != 0.0))
    {
      graph->Surf(Xb, Yb, Zb, "{xC6C6C6}");
      graph->Puts(p, af.utf8to(gettext("Iris")).c_str(), "{xC6C6C6}", fontsize * 3);
      graph->Plot(lX, lY, lZ, "{xC6C6C6}");
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
