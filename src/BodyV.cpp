/*
 * Copyright (C) 2023-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <BodyV.h>

BodyV::BodyV()
{
}

BodyV::BodyV(const BodyV &other)
{
  file = other.file;
  bodyv = other.bodyv;
}

BodyV::BodyV(BodyV &&other)
{
  file = other.file;
  bodyv = std::move(other.bodyv);
  other.file = nullptr;
}

BodyV &
BodyV::operator=(const BodyV &other)
{
  file = other.file;
  bodyv = other.bodyv;
  return *this;
}

void
BodyV::setFile(std::fstream *f)
{
  file = f;
}

std::fstream *
BodyV::getFile()
{
  return file;
}

void
BodyV::setVect(const std::vector<SPKItem> &bodyv)
{
  this->bodyv = bodyv;
}

std::vector<SPKItem>
BodyV::getVect()
{
  return bodyv;
}
