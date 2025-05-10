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
#include <EPMApplication.h>
#include <filesystem>
#include <iostream>
#include <libintl.h>
#include <omp.h>
#include <string>

int
main(int argc, char *argv[])
{
  AuxFunc af;
  std::filesystem::path p = af.get_selfpath();
  p = p.parent_path() / std::filesystem::u8path("../share/locale");
  bindtextdomain("ephepm", p.u8string().c_str());
  bind_textdomain_codeset("ephepm", "UTF-8");
  textdomain("ephepm");

  std::string id = "ru.mail.bobilev_yury.EphEPM";
  int exitstat = 0;
  auto app = EPMApplication::create(id);
  {
    char *loc = std::setlocale(LC_CTYPE, "C.UTF-8");
    if(loc)
      {
        std::cout << "EphEPM locale set to " << loc << std::endl;
      }
    else
      {
        std::cout << "Failed to set EphEPM locale" << std::endl;
      }
  }
#pragma omp parallel
#pragma omp masked
  {
    exitstat = app->run(argc, argv);
  }
  if(exitstat != 0)
    {
      std::cout << "EphEPM exit status is not 0: " << exitstat << std::endl;
    }
  return exitstat;
}
