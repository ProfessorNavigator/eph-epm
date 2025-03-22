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

#ifdef __linux
  {
    int res = setenv("GTK_THEME", "Adwaita", 1);
    if(res < 0)
      {
        std::cout << "EphemerisEPM main setenv error: " << std::strerror(errno)
                  << std::endl;
      }
  }
#endif

  std::string id = "ru.mail.bobilev_yury.EphEPM";
  auto app = EPMApplication::create(id);
  int exitstat = 0;
  exitstat = app->run(argc, argv);
  if(exitstat != 0)
    {
      std::cerr << "EphEPM exit status is not 0: " << exitstat << std::endl;
    }
  return exitstat;
}
