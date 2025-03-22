/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <CoordKeeper.h>

CoordKeeper::CoordKeeper()
{
}

CoordKeeper::CoordKeeper(const CoordKeeper &other)
{
  JD = other.JD;
  X = other.X;
  Y = other.Y;
  Z = other.Z;
}

CoordKeeper::CoordKeeper(CoordKeeper &&other)
{
  JD = other.JD;
  X = std::move(other.X);
  Y = std::move(other.Y);
  Z = std::move(other.Z);
}

CoordKeeper &
CoordKeeper::operator=(const CoordKeeper &other)
{
  if(this != &other)
    {
      JD = other.JD;
      X = other.X;
      Y = other.Y;
      Z = other.Z;
    }

  return *this;
}

CoordKeeper &
CoordKeeper::operator=(CoordKeeper &&other)
{
  if(this != &other)
    {
      JD = other.JD;
      X = std::move(other.X);
      Y = std::move(other.Y);
      Z = std::move(other.Z);
    }

  return *this;
}
