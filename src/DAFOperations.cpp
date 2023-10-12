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

#include "DAFOperations.h"

DAFOperations::DAFOperations()
{

}

DAFOperations::~DAFOperations()
{

}

std::string
DAFOperations::fileVersion(std::fstream *f)
{
  std::string result;
  if(!f->is_open())
    {
      std::cerr << "Ephemeris file not opened!" << std::endl;
      result = "ERROR";
      f->close();
    }
  else
    {
      std::vector<char> readv;
      readv.resize(8);
      f->read(&readv[0], readv.size());
      std::string chstr(readv.begin(), readv.end());
      std::string::size_type n;
      n = chstr.find("DAF/SPK");
      if(n == std::string::npos)
	{
	  n = chstr.find("DAF/PCK");
	}
      if(n == std::string::npos)
	{
	  std::cout << "Not epehemeris file" << std::endl;
	  result = "ERROR";
	}
      else
	{
	  f->seekg(16, std::ios_base::beg);
	  readv.clear();
	  readv.resize(60);
	  f->read(&readv[0], readv.size());
	  for(size_t i = 0; i < readv.size(); i++)
	    {
	      char ch = readv[i];
	      if(ch >= ' ' && ch <= '~')
		{
		  result.push_back(ch);
		}
	    }
	}
    }

  return result;
}

bool
DAFOperations::epochCheckUTC(int day, int month, int year, int hours,
			     int minutes, double seconds, int timesc, int belt,
			     std::string filename)
{
  bool result = false;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  AuxFunc af;
  double JDtt = 0.0;
  if(timesc == 0)
    {
      JDtt = af.timeTT(day, month, year, hours, minutes, seconds, belt);
    }
  if(timesc == 1)
    {
      JDtt = af.utcJD(day, month, year, hours, minutes, seconds);
      JDtt = JDtt - static_cast<double>(belt) / 24.0;
    }
  if(timesc == 2)
    {
      JDtt = af.utcJD(day, month, year, hours, minutes, seconds);
      JDtt = JDtt - static_cast<double>(belt) / 24.0;
    }
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "Ephemeris file not opened!" << std::endl;
      result = false;
    }
  else
    {
      std::vector<char> readv;
      readv.resize(8);
      f.read(&readv[0], readv.size());
      std::string chstr(readv.begin(), readv.end());
      std::string::size_type n;
      n = chstr.find("DAF/SPK");
      if(n == std::string::npos)
	{
	  n = chstr.find("DAF/PCK");
	}
      if(n == std::string::npos)
	{
	  std::cout << "Not epehemeris file" << std::endl;
	}
      else
	{
	  std::vector<std::tuple<double, double, int, int, int, int, int, int>> spkv;
	  spkv = this->bodiesVector(&f);
	  auto itspk = std::find_if(spkv.begin(), spkv.end(), []
	  (auto &el)
	    {
	      return std::get<2>(el) == 1000000001;
	    });
	  if(timesc == 2)
	    {
	      itspk = std::find_if(spkv.begin(), spkv.end(), []
	      (auto &el)
		{
		  return std::get<2>(el) == 1;
		});
	    }
	  if(itspk != spkv.end())
	    {
	      if(JDtt >= std::get<0>(*itspk) && JDtt <= std::get<1>(*itspk))
		{
		  result = true;
		}
	    }
	  else
	    {
	      itspk = std::find_if(spkv.begin(), spkv.end(), []
	      (auto &el)
		{
		  return std::get<2>(el) == 1800303;
		});
	      if(itspk != spkv.end())
		{
		  if(JDtt >= std::get<0>(*itspk) && JDtt <= std::get<1>(*itspk))
		    {
		      result = true;
		    }
		}
	      else
		{
		  itspk = std::find_if(spkv.begin(), spkv.end(), []
		  (auto &el)
		    {
		      return std::get<2>(el) == 1800302;
		    });
		  if(itspk != spkv.end())
		    {
		      if(JDtt >= std::get<0>(*itspk)
			  && JDtt <= std::get<1>(*itspk))
			{
			  result = true;
			}
		    }
		}
	    }
	}
      f.close();
    }

  return result;
}

bool
DAFOperations::epochCheckUTC(double JD, int timesc, double *epb, double *epe,
			     std::string filename)
{
  bool result = false;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  AuxFunc af;
  double JDtt = JD;
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "Ephemeris file not opened!" << std::endl;
      result = false;
    }
  else
    {
      std::vector<char> readv;
      readv.resize(8);
      f.read(&readv[0], readv.size());
      std::string chstr(readv.begin(), readv.end());
      std::string::size_type n;
      n = chstr.find("DAF/SPK");
      if(n == std::string::npos)
	{
	  n = chstr.find("DAF/PCK");
	}
      if(n == std::string::npos)
	{
	  std::cout << "Not ephemeris file" << std::endl;
	  result = false;
	}
      else
	{
	  std::vector<std::tuple<double, double, int, int, int, int, int, int>> spkv;
	  spkv = this->bodiesVector(&f);
	  auto itspk = std::find_if(spkv.begin(), spkv.end(), []
	  (auto &el)
	    {
	      return std::get<2>(el) == 1000000001;
	    });
	  if(timesc == 2)
	    {
	      itspk = std::find_if(spkv.begin(), spkv.end(), []
	      (auto &el)
		{
		  return std::get<2>(el) == 1;
		});
	    }
	  if(itspk != spkv.end())
	    {
	      if(JDtt >= std::get<0>(*itspk) && JDtt <= std::get<1>(*itspk))
		{
		  *epb = std::get<0>(*itspk);
		  *epe = std::get<1>(*itspk);
		  result = true;
		}
	    }
	  else
	    {
	      itspk = std::find_if(spkv.begin(), spkv.end(), []
	      (auto &el)
		{
		  return std::get<2>(el) == 1800303;
		});
	      if(itspk != spkv.end())
		{
		  if(JDtt >= std::get<0>(*itspk) && JDtt <= std::get<1>(*itspk))
		    {
		      result = true;
		    }
		}
	      else
		{
		  itspk = std::find_if(spkv.begin(), spkv.end(), []
		  (auto &el)
		    {
		      return std::get<2>(el) == 1800302;
		    });
		  if(itspk != spkv.end())
		    {
		      if(JDtt >= std::get<0>(*itspk)
			  && JDtt <= std::get<1>(*itspk))
			{
			  result = true;
			}
		    }
		}
	    }
	}
      f.close();
    }

  return result;
}

std::vector<std::tuple<double, double, int, int, int, int, int, int>>
DAFOperations::bodiesVector(std::fstream *f)
{
  /*Body tuple vector.
   * Tuple parts are the following:
   * 0 - JD of interval begin
   * 1 - JD of interval end
   * 2 - the NAIF object identification number of the target body
   * 3 - the NAIF object identification code of the center
   * 4 - the NAIF integer code for the reference frame
   * 5 - the integer code for the SPK data type
   * 6 - the initial address of the array
   * 7 - the final address of the array
   * */
  std::vector<std::tuple<double, double, int, int, int, int, int, int>> spkbodyv;
  std::vector<char> readv;
  double RNN = -1.0; //The record number of the next Summary Record in the file

  f->seekg(8, std::ios_base::beg);
  readv.clear();
  readv.resize(4);
  f->read(&readv[0], readv.size());
  int ND; //The number of double precision components in each array summary
  std::memcpy(&ND, &readv[0], sizeof(ND));

  readv.clear();
  readv.resize(4);
  f->read(&readv[0], readv.size());
  int NI; // The number of integer components in each array summary
  std::memcpy(&NI, &readv[0], sizeof(NI));

  while(RNN != 0.0)
    {
      if(RNN < 0)
	{
	  f->seekg(1024 * 2, std::ios_base::beg);
	}
      else
	{
	  f->seekg(static_cast<size_t>(RNN) * 8 - 1, std::ios_base::beg);
	}
      readv.clear();
      readv.resize(8);
      f->read(&readv[0], readv.size());
      std::memcpy(&RNN, &readv[0], sizeof(RNN));

      readv.clear();
      readv.resize(8);
      f->read(&readv[0], readv.size());
      double RNP; //The record number of the previous Summary Record in the file
      std::memcpy(&RNP, &readv[0], sizeof(RNP));

      readv.clear();
      readv.resize(8);
      f->read(&readv[0], readv.size());
      double NS; //The number of summaries stored in this record
      std::memcpy(&NS, &readv[0], sizeof(NS));
      for(int j = 0; j < static_cast<int>(NS); j++)
	{
	  std::tuple<double, double, int, int, int, int, int, int> spkbodytup;
	  for(int i = 0; i < ND; i++)
	    {
	      readv.clear();
	      double val;
	      readv.resize(sizeof(val));
	      f->read(&readv[0], readv.size());
	      std::memcpy(&val, &readv[0], sizeof(val));
	      if(i == 0)
		{
		  std::get<0>(spkbodytup) = 2451545.0 + val / 86400;
		}
	      if(i == 1)
		{
		  std::get<1>(spkbodytup) = 2451545.0 + val / 86400;
		}
	    }
	  for(int i = 0; i < NI; i++)
	    {
	      readv.clear();
	      int val;
	      readv.resize(sizeof(val));
	      f->read(&readv[0], readv.size());
	      std::memcpy(&val, &readv[0], sizeof(val));
	      if(i == 0)
		{
		  std::get<2>(spkbodytup) = val;
		}
	      if(i == 1)
		{
		  std::get<3>(spkbodytup) = val;
		}
	      if(i == 2)
		{
		  std::get<4>(spkbodytup) = val;
		}
	      if(i == 3)
		{
		  std::get<5>(spkbodytup) = val;
		}
	      if(i == 4)
		{
		  std::get<6>(spkbodytup) = val;
		}
	      if(i == 5)
		{
		  std::get<7>(spkbodytup) = val;
		}
	    }
	  spkbodyv.push_back(spkbodytup);
	}
    }
  return spkbodyv;
}

int
DAFOperations::bodyVect(std::fstream *result, uint64_t *c_beg, uint64_t *c_end,
			int NAIFid)
{
  int type = -1;
  if(!result->is_open())
    {
      std::cerr << "Ephemeris file not opened!" << std::endl;
    }
  else
    {
      std::vector<std::tuple<double, double, int, int, int, int, int, int>> spkv;
      spkv = bodiesVector(result);
      result->seekg(0, std::ios_base::beg);
      std::string readstr;
      readstr.resize(8);
      result->read(&readstr[0], readstr.size());
      int ephtype = 0;
      std::string::size_type n;
      n = readstr.find("DAF/SPK");
      if(n != std::string::npos)
	{
	  ephtype = 1;
	}
      n = readstr.find("DAF/PCK");
      if(n != std::string::npos)
	{
	  ephtype = 2;
	}
      auto itspk = std::find_if(spkv.begin(), spkv.end(), [NAIFid]
      (auto &el)
	{
	  return std::get<2>(el) == NAIFid;
	});
      if(itspk != spkv.end())
	{
	  *c_beg = std::get<6>(*itspk) - 1;
	  *c_end = std::get<7>(*itspk);
	  if(ephtype == 1)
	    {
	      type = std::get<5>(*itspk);
	      *c_beg = std::get<6>(*itspk) - 1;
	      *c_end = std::get<7>(*itspk);
	    }
	  if(ephtype == 2)
	    {
	      type = std::get<4>(*itspk);
	      *c_beg = std::get<5>(*itspk) - 1;
	      *c_end = std::get<6>(*itspk);
	    }
	}
    }
  return type;
}

