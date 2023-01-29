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

#include "EPMCalculations.h"

EPMCalculations::EPMCalculations()
{
  // TODO Auto-generated constructor stub

}

EPMCalculations::~EPMCalculations()
{
  // TODO Auto-generated destructor stub
}

mpf_class
EPMCalculations::tdbCalc(std::fstream *f, uint64_t *c_b, uint64_t *c_e,
			 double JD, int type)
{
  mpf_set_default_prec(512);
  mpf_class num, tb, te, tau, summa(0), JDB;
  if(f->is_open())
    {
      if(type == 2)
	{
	  JDB = JD;
	  std::vector<char> readv;
	  double val;
	  readv.resize(8);
	  f->seekg((*c_e - 2) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class RSIZE(val);
	  mpf_class PD = (RSIZE - 2) / 3 - 1;

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 3) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INTLEN = mpf_class(val) / mpf_class(86400);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 4) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INIT = mpf_class(2451545.0)
	      + mpf_class(val) / mpf_class(86400);

	  mpf_class T0 = mpf_class(JD) - INIT;
	  uint64_t m = static_cast<uint64_t>(mpf_class(T0 / INTLEN).get_d());
	  std::vector<mpf_class> forcalc;

	  f->seekg((*c_b + RSIZE.get_ui() * m) * 8, std::ios_base::beg);
	  readv.clear();
	  readv.resize(8);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class MID = mpf_class(2451545.0)
	      + mpf_class(val) / mpf_class(86400);
	  readv.clear();
	  readv.resize(8);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class RADIUS = mpf_class(val) / mpf_class(86400);
	  tb = MID - RADIUS;
	  te = MID + RADIUS;
	  if(JD >= tb && JD <= te)
	    {
	      for(size_t j = 0; j < PD.get_ui() * 8; j = j + 8)
		{
		  readv.clear();
		  readv.resize(8);
		  f->read(&readv[0], readv.size());
		  std::memcpy(&val, &readv[0], sizeof(val));
		  forcalc.push_back(val);
		}
	    }
	  tau = (2 * JD - tb - te) / (te - tb);
	  std::vector<mpf_class> P;
	  P.push_back(1);
	  P.push_back(tau);
	  for(size_t i = 2; i < forcalc.size(); i++)
	    {
	      P.push_back(2 * tau * P[i - 1] - P[i - 2]);
	    }
	  for(size_t i = 0; i < forcalc.size(); i++)
	    {
	      summa = summa + forcalc[i] * P[i];
	    }
	}
    }
  return summa;
}

//Координаты X, Vx
mpf_class
EPMCalculations::bodyCalcX(std::fstream *f, uint64_t *c_b, uint64_t *c_e,
			   mpf_class JDC, int var, int type, mpf_class *au)
{
  mpf_set_default_prec(512);
  mpf_class result;
  if(f->is_open())
    {
      if(type == 20)
	{
	  mpf_class summa(0);
	  std::vector<char> readv;
	  double val;
	  readv.resize(8);
	  f->seekg((*c_e - 7) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class DSCALE(val);
	  *au = DSCALE;

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 6) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class TSCALE(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 5) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INITJD(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 4) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INITFR(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 3) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INTLEN(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 2) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  size_t RSIZE = static_cast<size_t>(val);
	  size_t PD = RSIZE / 3 - 1;
	  mpf_class T0 = JDC - (INITJD + INITFR);
	  size_t m = static_cast<size_t>(mpf_class(T0 / INTLEN).get_d());
	  mpf_class T1 = T0 - mpf_class(static_cast<double>(m)) * INTLEN;
	  mpf_class tau = 2 * T1 / INTLEN - 1;
	  size_t numb = m * RSIZE;
	  size_t n = PD;

	  readv.clear();
	  readv.resize((n + 1) * 8);
	  f->seekg((*c_b + numb) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  std::vector<mpf_class> C_b;
	  for(size_t i = 0; i < readv.size(); i = i + 8)
	    {
	      std::memcpy(&val, &readv[i], sizeof(val));
	      C_b.push_back(val);
	    }
	  std::vector<mpf_class> P;
	  P.push_back(1);
	  P.push_back(tau);
	  for(size_t i = 2; i <= n; i++)
	    {
	      P.push_back(2 * tau * P[i - 1] - P[i - 2]);
	    }
	  if(var == 0)
	    {
	      std::vector<mpf_class> W;
	      W.push_back(tau);
	      W.push_back((P[0] + P[2]) * 0.25);
	      W.resize(n);
	      mpf_class flag(1);
	      for(size_t i = 1; i <= n / 2; i++)
		{
		  flag = -flag;
		  W[2 * i] = 0.5
		      * (P[2 * i + 1]
			  / mpf_class(static_cast<double>(2 * i + 1))
			  - P[2 * i - 1]
			      / mpf_class(static_cast<double>(2 * i - 1)));
		  if(2 * i + 2 < P.size())
		    {
		      W[2 * i + 1] =
			  0.5
			      * (P[2 * i + 2]
				  / mpf_class(static_cast<double>(2 * i + 2))
				  - P[2 * i]
				      / mpf_class(static_cast<double>(2 * i)))
			      + flag * 0.25
				  * (1.0 / mpf_class(static_cast<double>(i))
				      + 1.0
					  / mpf_class(
					      static_cast<double>(i + 1)));
		    }
		}
	      for(size_t i = 0; i < n; i++)
		{
		  summa = summa + C_b[i] * W[i];
		}
	      result = C_b[C_b.size() - 1] + (INTLEN * 0.5) * summa;
	    }
	  if(var == 1)
	    {
	      summa = 0;
	      for(size_t i = 0; i < n; i++)
		{
		  summa = summa + C_b[i] * P[i];
		}
	      result = summa;
	    }
	}
    }
  return result;
}

//Координаты Y, Vy
mpf_class
EPMCalculations::bodyCalcY(std::fstream *f, uint64_t *c_b, uint64_t *c_e,
			   mpf_class JDC, int var, int type, mpf_class *au)
{
  mpf_set_default_prec(512);
  mpf_class result;
  if(f->is_open())
    {
      if(type == 20)
	{
	  mpf_class summa(0);
	  std::vector<char> readv;
	  double val;
	  readv.resize(8);
	  f->seekg((*c_e - 7) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class DSCALE(val);
	  *au = DSCALE;

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 6) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class TSCALE(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 5) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INITJD(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 4) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INITFR(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 3) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INTLEN(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 2) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  size_t RSIZE = static_cast<size_t>(val);
	  size_t PD = RSIZE / 3 - 1;
	  mpf_class T0 = JDC - (INITJD + INITFR);
	  size_t m = static_cast<size_t>(mpf_class(T0 / INTLEN).get_d());
	  mpf_class T1 = T0 - mpf_class(static_cast<double>(m)) * INTLEN;
	  mpf_class tau = 2 * T1 / INTLEN - 1;
	  size_t numb = m * RSIZE + RSIZE / 3;
	  size_t n = PD;

	  readv.clear();
	  readv.resize((n + 1) * 8);
	  f->seekg((*c_b + numb) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  std::vector<mpf_class> C_b;
	  for(size_t i = 0; i < readv.size(); i = i + 8)
	    {
	      std::memcpy(&val, &readv[i], sizeof(val));
	      C_b.push_back(val);
	    }
	  std::vector<mpf_class> P;
	  P.push_back(1);
	  P.push_back(tau);
	  for(size_t i = 2; i <= n; i++)
	    {
	      P.push_back(2 * tau * P[i - 1] - P[i - 2]);
	    }
	  if(var == 0)
	    {
	      std::vector<mpf_class> W;
	      W.push_back(tau);
	      W.push_back((P[0] + P[2]) * 0.25);
	      W.resize(n);
	      mpf_class flag(1);
	      for(size_t i = 1; i <= n / 2; i++)
		{
		  flag = -flag;
		  W[2 * i] = 0.5
		      * (P[2 * i + 1]
			  / mpf_class(static_cast<double>(2 * i + 1))
			  - P[2 * i - 1]
			      / mpf_class(static_cast<double>(2 * i - 1)));
		  if(2 * i + 2 < P.size())
		    {
		      W[2 * i + 1] =
			  0.5
			      * (P[2 * i + 2]
				  / mpf_class(static_cast<double>(2 * i + 2))
				  - P[2 * i]
				      / mpf_class(static_cast<double>(2 * i)))
			      + flag * 0.25
				  * (1.0 / mpf_class(static_cast<double>(i))
				      + 1.0
					  / mpf_class(
					      static_cast<double>(i + 1)));
		    }
		}
	      for(size_t i = 0; i < n; i++)
		{
		  summa = summa + C_b[i] * W[i];
		}
	      result = C_b[C_b.size() - 1] + (INTLEN * 0.5) * summa;
	    }
	  if(var == 1)
	    {
	      summa = 0;
	      for(size_t i = 0; i < n; i++)
		{
		  summa = summa + C_b[i] * P[i];
		}
	      result = summa;
	    }
	}
    }
  return result;
}

//Координаты Z, Vz
mpf_class
EPMCalculations::bodyCalcZ(std::fstream *f, uint64_t *c_b, uint64_t *c_e,
			   mpf_class JDC, int var, int type, mpf_class *au)
{
  mpf_set_default_prec(512);
  mpf_class result;
  if(f->is_open())
    {
      if(type == 20)
	{
	  mpf_class summa(0);
	  std::vector<char> readv;
	  double val;
	  readv.resize(8);
	  f->seekg((*c_e - 7) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class DSCALE(val);
	  *au = DSCALE;

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 6) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class TSCALE(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 5) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INITJD(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 4) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INITFR(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 3) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  mpf_class INTLEN(val);

	  readv.clear();
	  readv.resize(8);
	  f->seekg((*c_e - 2) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  size_t RSIZE = static_cast<size_t>(val);
	  size_t PD = RSIZE / 3 - 1;
	  mpf_class T0 = JDC - (INITJD + INITFR);
	  size_t m = static_cast<size_t>(mpf_class(T0 / INTLEN).get_d());
	  mpf_class T1 = T0 - mpf_class(static_cast<double>(m)) * INTLEN;
	  mpf_class tau = 2 * T1 / INTLEN - 1;
	  size_t numb = m * RSIZE + 2 * RSIZE / 3;
	  size_t n = PD;

	  readv.clear();
	  readv.resize((n + 1) * 8);
	  f->seekg((*c_b + numb) * 8, std::ios_base::beg);
	  f->read(&readv[0], readv.size());
	  std::memcpy(&val, &readv[0], sizeof(val));
	  std::vector<mpf_class> C_b;
	  for(size_t i = 0; i < readv.size(); i = i + 8)
	    {
	      std::memcpy(&val, &readv[i], sizeof(val));
	      C_b.push_back(val);
	    }
	  std::vector<mpf_class> P;
	  P.push_back(1);
	  P.push_back(tau);
	  for(size_t i = 2; i <= n; i++)
	    {
	      P.push_back(2 * tau * P[i - 1] - P[i - 2]);
	    }
	  if(var == 0)
	    {
	      std::vector<mpf_class> W;
	      W.push_back(tau);
	      W.push_back((P[0] + P[2]) * 0.25);
	      W.resize(n);
	      mpf_class flag(1);
	      for(size_t i = 1; i <= n / 2; i++)
		{
		  flag = -flag;
		  W[2 * i] = 0.5
		      * (P[2 * i + 1]
			  / mpf_class(static_cast<double>(2 * i + 1))
			  - P[2 * i - 1]
			      / mpf_class(static_cast<double>(2 * i - 1)));
		  if(2 * i + 2 < P.size())
		    {
		      W[2 * i + 1] =
			  0.5
			      * (P[2 * i + 2]
				  / mpf_class(static_cast<double>(2 * i + 2))
				  - P[2 * i]
				      / mpf_class(static_cast<double>(2 * i)))
			      + flag * 0.25
				  * (1.0 / mpf_class(static_cast<double>(i))
				      + 1.0
					  / mpf_class(
					      static_cast<double>(i + 1)));
		    }
		}
	      for(size_t i = 0; i < n; i++)
		{
		  summa = summa + C_b[i] * W[i];
		}
	      result = C_b[C_b.size() - 1] + (INTLEN * 0.5) * summa;
	    }
	  if(var == 1)
	    {
	      summa = 0;
	      for(size_t i = 0; i < n; i++)
		{
		  summa = summa + C_b[i] * P[i];
		}
	      result = summa;
	    }
	}
    }
  return result;
}
