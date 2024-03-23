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
#include <DAFOperations.h>
#include <stddef.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iterator>

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
      f->read(readv.data(), readv.size());
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
	  f->read(readv.data(), readv.size());
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
      std::cerr << "DAFOperations::epochCheckUTC: Ephemeris file not opened!"
	  << std::endl;
      result = false;
    }
  else
    {
      std::vector<char> readv;
      readv.resize(8);
      f.read(readv.data(), readv.size());
      std::string chstr(readv.begin(), readv.end());
      std::string::size_type n;
      n = chstr.find("DAF/SPK");
      if(n == std::string::npos)
	{
	  n = chstr.find("DAF/PCK");
	}
      if(n == std::string::npos)
	{
	  std::cout << "DAFOperations::epochCheckUTC: Not epehemeris file"
	      << std::endl;
	}
      else
	{
	  std::vector<std::tuple<double, double, int, int, int, int, int, int>> spkv;
	  spkv = bodiesVector(&f);
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
      std::cerr << "DAFOperations::epochCheckUTC: Ephemeris file not opened!" << std::endl;
      result = false;
    }
  else
    {
      std::vector<char> readv;
      readv.resize(8);
      f.read(readv.data(), readv.size());
      std::string chstr(readv.begin(), readv.end());
      std::string::size_type n;
      n = chstr.find("DAF/SPK");
      if(n == std::string::npos)
	{
	  n = chstr.find("DAF/PCK");
	}
      if(n == std::string::npos)
	{
	  std::cout << "DAFOperations::epochCheckUTC: Not ephemeris file" << std::endl;
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
  f->seekg(8, std::ios_base::beg);

  uint32_t ND; //The number of double precision components in each array summary
  readv.clear();
  readv.resize(sizeof(ND));
  f->read(readv.data(), readv.size());
  std::memcpy(&ND, readv.data(), readv.size());

  uint32_t NI; // The number of integer components in each array summary
  readv.clear();
  readv.resize(sizeof(NI));
  f->read(readv.data(), readv.size());
  std::memcpy(&NI, readv.data(), readv.size());

  f->seekg(60, std::ios_base::cur);

  uint32_t fward; // The record number of the initial summary record in the file
  readv.clear();
  readv.resize(sizeof(fward));
  f->read(readv.data(), readv.size());
  std::memcpy(&fward, readv.data(), readv.size());

  size_t ss = (ND + (NI + 1) / 2) * 8; // Size of single summary in bytes

  double RNN = -1.0; //The record number of the next Summary Record in the file
  while(static_cast<int>(RNN) != 0)
    {
      if(RNN < 0)
	{
	  f->seekg(static_cast<size_t>((fward - 1) * 1024), std::ios_base::beg);
	}
      else
	{
	  f->seekg(static_cast<size_t>((RNN - 1) * 1024), std::ios_base::beg);
	}
      readv.clear();
      readv.resize(sizeof(RNN));
      f->read(readv.data(), readv.size());
      std::memcpy(&RNN, readv.data(), sizeof(RNN));

      double RNP; //The record number of the previous Summary Record in the file
      readv.clear();
      readv.resize(sizeof(RNP));
      f->read(readv.data(), readv.size());
      std::memcpy(&RNP, readv.data(), readv.size());

      double NS; //The number of summaries stored in this record
      readv.clear();
      readv.resize(sizeof(NS));
      f->read(readv.data(), readv.size());
      std::memcpy(&NS, readv.data(), readv.size());
      for(int j = 0; j < static_cast<int>(NS); j++)
	{
	  size_t readbytes = 0;
	  std::tuple<double, double, int, int, int, int, int, int> spkbodytup;
	  for(uint32_t i = 0; i < ND; i++)
	    {
	      readv.clear();
	      double val;
	      readv.resize(sizeof(val));
	      f->read(readv.data(), readv.size());
	      readbytes += readv.size();
	      std::memcpy(&val, readv.data(), sizeof(val));
	      if(i == 0)
		{
		  std::get<0>(spkbodytup) = 2451545.0 + val / 86400;
		}
	      if(i == 1)
		{
		  std::get<1>(spkbodytup) = 2451545.0 + val / 86400;
		}
	    }
	  for(uint32_t i = 0; i < NI; i++)
	    {
	      readv.clear();
	      uint32_t val;
	      readv.resize(sizeof(val));
	      f->read(readv.data(), readv.size());
	      readbytes += readv.size();
	      std::memcpy(&val, readv.data(), sizeof(val));
	      if(i == 0)
		{
		  std::get<2>(spkbodytup) = static_cast<int>(val);
		}
	      if(i == 1)
		{
		  std::get<3>(spkbodytup) = static_cast<int>(val);
		}
	      if(i == 2)
		{
		  std::get<4>(spkbodytup) = static_cast<int>(val);
		}
	      if(i == 3)
		{
		  std::get<5>(spkbodytup) = static_cast<int>(val);
		}
	      if(i == 4)
		{
		  std::get<6>(spkbodytup) = static_cast<int>(val);
		}
	      if(i == 5)
		{
		  std::get<7>(spkbodytup) = static_cast<int>(val);
		}
	    }
	  spkbodyv.push_back(spkbodytup);
	  if(ss > readbytes)
	    {
	      f->seekg(ss - readbytes, std::ios_base::cur);
	    }
	}
    }
  return spkbodyv;
}

int
DAFOperations::bodyVect(std::fstream *result, uint64_t *c_beg, uint64_t *c_end,
			int NAIFid, double JD)
{
  int type = -1;
  if(!result->is_open())
    {
      std::cerr << "Ephemeris file not opened!" << std::endl;
    }
  else
    {
      std::vector<std::tuple<double, double, int, int, int, int, int, int>> spkv;
      auto itfv = std::find_if(filev.begin(), filev.end(), [result]
      (auto &el) 
	{
	  return el.getFile() == result;
	});
      if(itfv != filev.end())
	{
	  spkv = (*itfv).getVect();
	}
      else
	{
	  spkv = bodiesVector(result);
	  BodyV bv;
	  bv.setFile(result);
	  bv.setVect(spkv);
	  filev.push_back(bv);
	}

      result->seekg(0, std::ios_base::beg);
      std::string readstr;
      readstr.resize(8);
      result->read(readstr.data(), readstr.size());
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
      auto itspk = std::find_if(spkv.begin(), spkv.end(), [NAIFid, JD]
      (auto &el) 
	{
	  if(std::get<2>(el) == NAIFid)
	    {
	      if(JD >= std::get<0>(el) && JD <= std::get<1>(el))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	  else
	    {
	      return false;
	    }
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
