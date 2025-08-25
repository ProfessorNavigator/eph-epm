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
#include <ByteOrder.h>
#include <DAFOperations.h>
#include <algorithm>
//#include <cstring>
#include <filesystem>
#include <iostream>

DAFOperations::DAFOperations()
{
}

std::string
DAFOperations::fileVersion(std::fstream *f)
{
  std::string result;
  if(!f->is_open())
    {
      std::cout << "Ephemeris file not opened!" << std::endl;
      result = "ERROR";
      f->close();
    }
  else
    {
      std::string chstr;
      chstr.resize(8);
      f->read(chstr.data(), chstr.size());
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
          result.resize(60);
          f->read(result.data(), result.size());
          result.erase(std::remove_if(result.begin(), result.end(),
                                      [](const char &el) {
                                        if(el >= ' ' && el <= '~')
                                          {
                                            return false;
                                          }
                                        return true;
                                      }),
                       result.end());
        }
    }

  return result;
}

bool
DAFOperations::epochCheckUTC(const int &day, const int &month, const int &year,
                             const int &hours, const int &minutes,
                             const double &seconds, const int &timesc,
                             const int &belt, const std::string &filename)
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
      std::cout << "DAFOperations::epochCheckUTC: Ephemeris file not opened!"
                << std::endl;
      result = false;
    }
  else
    {
      std::string chstr;
      chstr.resize(8);
      f.read(chstr.data(), chstr.size());
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
          std::vector<SPKItem> spkv;
          spkv = bodiesVector(&f);
          auto itspk = std::find_if(spkv.begin(), spkv.end(), [](SPKItem &el) {
            return el.NAIF_body_id == 1000000001;
          });
          if(timesc == 2)
            {
              itspk = std::find_if(spkv.begin(), spkv.end(), [](SPKItem &el) {
                return el.NAIF_body_id == 1;
              });
            }
          if(itspk != spkv.end())
            {
              if(JDtt >= itspk->JD_begin && JDtt <= itspk->JD_end)
                {
                  result = true;
                }
            }
          else
            {
              itspk = std::find_if(spkv.begin(), spkv.end(), [](SPKItem &el) {
                return el.NAIF_body_id == 1800303;
              });
              if(itspk != spkv.end())
                {
                  if(JDtt >= itspk->JD_begin && JDtt <= itspk->JD_end)
                    {
                      result = true;
                    }
                }
              else
                {
                  itspk = std::find_if(spkv.begin(), spkv.end(),
                                       [](SPKItem &el) {
                                         return el.NAIF_body_id == 1800302;
                                       });
                  if(itspk != spkv.end())
                    {
                      if(JDtt >= itspk->JD_begin && JDtt <= itspk->JD_end)
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
DAFOperations::epochCheckUTC(const double &JD, const int &timesc, double &epb,
                             double &epe, const std::string &filename)
{
  bool result = false;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  AuxFunc af;
  double JDtt = JD;
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cout << "DAFOperations::epochCheckUTC: Ephemeris file not opened!"
                << std::endl;
      result = false;
    }
  else
    {
      std::string chstr;
      chstr.resize(8);
      f.read(chstr.data(), chstr.size());
      std::string::size_type n;
      n = chstr.find("DAF/SPK");
      if(n == std::string::npos)
        {
          n = chstr.find("DAF/PCK");
        }
      if(n == std::string::npos)
        {
          std::cout << "DAFOperations::epochCheckUTC: Not ephemeris file"
                    << std::endl;
          result = false;
        }
      else
        {
          std::vector<SPKItem> spkv;
          spkv = bodiesVector(&f);
          auto itspk = std::find_if(spkv.begin(), spkv.end(), [](SPKItem &el) {
            return el.NAIF_body_id == 1000000001;
          });
          if(timesc == 2)
            {
              itspk = std::find_if(spkv.begin(), spkv.end(), [](SPKItem &el) {
                return el.NAIF_body_id == 1;
              });
            }
          if(itspk != spkv.end())
            {
              if(JDtt >= itspk->JD_begin && JDtt <= itspk->JD_end)
                {
                  epb = itspk->JD_begin;
                  epe = itspk->JD_end;
                  result = true;
                }
            }
          else
            {
              itspk = std::find_if(spkv.begin(), spkv.end(), [](SPKItem &el) {
                return el.NAIF_body_id == 1800303;
              });
              if(itspk != spkv.end())
                {
                  if(JDtt >= itspk->JD_begin && JDtt <= itspk->JD_end)
                    {
                      result = true;
                    }
                }
              else
                {
                  itspk = std::find_if(spkv.begin(), spkv.end(),
                                       [](SPKItem &el) {
                                         return el.NAIF_body_id == 1800302;
                                       });
                  if(itspk != spkv.end())
                    {
                      if(JDtt >= itspk->JD_begin && JDtt <= itspk->JD_end)
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

std::vector<SPKItem>
DAFOperations::bodiesVector(std::fstream *f)
{
  std::vector<SPKItem> spkbodyv;
  f->seekg(8, std::ios_base::beg);

  uint32_t
      ND; // The number of double precision components in each array summary
  f->read(reinterpret_cast<char *>(&ND), sizeof(ND));
  ByteOrder bo;
  bo.set_little(ND);
  ND = bo;

  uint32_t NI; // The number of integer components in each array summary
  f->read(reinterpret_cast<char *>(&NI), sizeof(NI));
  bo.set_little(NI);
  NI = bo;

  f->seekg(60, std::ios_base::cur);

  uint32_t
      fward; // The record number of the initial summary record in the file
  f->read(reinterpret_cast<char *>(&fward), sizeof(fward));
  bo.set_little(fward);
  fward = bo;

  size_t ss = (ND + (NI + 1) / 2) * 8; // Size of single summary in bytes

  double RNN = -1.0; // The record number of the next Summary Record in the
                     // file
  while(static_cast<int>(RNN) != 0)
    {
      if(RNN < 0)
        {
          f->seekg(static_cast<size_t>((fward - 1) * 1024),
                   std::ios_base::beg);
        }
      else
        {
          f->seekg(static_cast<size_t>((RNN - 1) * 1024), std::ios_base::beg);
        }
      f->read(reinterpret_cast<char *>(&RNN), sizeof(RNN));
      bo.set_little(RNN);
      RNN = bo;

      double
          RNP; // The record number of the previous Summary Record in the file
      f->read(reinterpret_cast<char *>(&RNP), sizeof(RNP));
      bo.set_little(RNP);
      RNP = bo;

      double NS; // The number of summaries stored in this record
      f->read(reinterpret_cast<char *>(&NS), sizeof(NS));
      bo.set_little(NS);
      NS = bo;
      double val_double;
      size_t sz_double = sizeof(val_double);
      uint32_t val_32;
      size_t sz_32 = sizeof(val_32);
      for(int j = 0; j < static_cast<int>(NS); j++)
        {
          size_t readbytes = 0;
          SPKItem spkbodyitem;
          for(uint32_t i = 0; i < ND; i++)
            {
              f->read(reinterpret_cast<char *>(&val_double), sz_double);
              readbytes += sz_double;
              bo.set_little(val_double);
              val_double = bo;

              if(i == 0)
                {
                  spkbodyitem.JD_begin = 2451545.0 + val_double / 86400;
                }
              if(i == 1)
                {
                  spkbodyitem.JD_end = 2451545.0 + val_double / 86400;
                }
            }

          if(NI == 6)
            {
              for(uint32_t i = 0; i < NI; i++)
                {
                  f->read(reinterpret_cast<char *>(&val_32), sz_32);
                  readbytes += sz_32;
                  bo.set_little(val_32);
                  val_32 = bo;

                  switch(i)
                    {
                    case 0:
                      {
                        spkbodyitem.NAIF_body_id = static_cast<int>(val_32);
                        break;
                      }
                    case 1:
                      {
                        spkbodyitem.NAIF_center_id = static_cast<int>(val_32);
                        break;
                      }
                    case 2:
                      {
                        spkbodyitem.NAIF_ref_frame = static_cast<int>(val_32);
                        break;
                      }
                    case 3:
                      {
                        spkbodyitem.NAIF_spk_data_type
                            = static_cast<int>(val_32);
                        break;
                      }
                    case 4:
                      {
                        spkbodyitem.initial_address = static_cast<int>(val_32);
                        break;
                      }
                    case 5:
                      {
                        spkbodyitem.final_addr = val_32;
                        break;
                      }
                    default:
                      break;
                    }
                }
            }
          else
            {
              for(uint32_t i = 0; i < NI; i++)
                {
                  f->read(reinterpret_cast<char *>(&val_32), sz_32);
                  readbytes += sz_32;
                  bo.set_little(val_32);
                  val_32 = bo;

                  switch(i)
                    {
                    case 0:
                      {
                        spkbodyitem.NAIF_body_id = static_cast<int>(val_32);
                        break;
                      }
                    case 1:
                      {
                        spkbodyitem.NAIF_center_id = static_cast<int>(val_32);
                        break;
                      }
                    case 2:
                      {
                        spkbodyitem.NAIF_spk_data_type
                            = static_cast<int>(val_32);
                        break;
                      }
                    case 3:
                      {
                        spkbodyitem.initial_address = static_cast<int>(val_32);
                        break;
                      }
                    case 4:
                      {
                        spkbodyitem.final_addr = val_32;
                        break;
                      }
                    default:
                      break;
                    }
                }
            }
          spkbodyv.push_back(spkbodyitem);
          if(ss > readbytes)
            {
              f->seekg(ss - readbytes, std::ios_base::cur);
            }
        }
    }
  return spkbodyv;
}

int
DAFOperations::bodyVect(std::fstream *result, uint64_t &c_beg, uint64_t &c_end,
                        const int &NAIFid, const double &JD)
{
  int type = -1;
  if(!result->is_open())
    {
      std::cout << "DAFOperations::bodyVect: ephemeris file not opened!"
                << std::endl;
    }
  else
    {
      std::vector<SPKItem> spkv;
      auto itfv
          = std::find_if(filev.begin(), filev.end(), [result](BodyV &el) {
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
          filev.emplace_back(bv);
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

      auto itspk
          = std::find_if(spkv.begin(), spkv.end(), [NAIFid, JD](SPKItem &el) {
              if(el.NAIF_body_id == NAIFid)
                {
                  if(JD >= el.JD_begin && JD <= el.JD_end)
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
          c_beg = itspk->initial_address - 1;
          c_end = itspk->final_addr;
          if(ephtype == 1 || ephtype == 2)
            {
              type = itspk->NAIF_spk_data_type;
            }
        }
    }
  return type;
}
