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
  std::string id = "ru.mail.bobilev_yury.EphEPM";
  int exitstat = 0;
  Glib::RefPtr<EPMApplication> app;
  {
    char *loc = std::setlocale(LC_CTYPE, nullptr);

    std::shared_ptr<std::string> default_locale_name(new std::string(loc));
    std::cout << "EphEPM default locale is: " << *default_locale_name
              << std::endl;

    app = EPMApplication::create(id, default_locale_name);

    loc = std::setlocale(LC_CTYPE, "C.UTF-8");
    if(loc)
      {
        std::cout << "EphEPM locale set to " << loc << std::endl;
      }
    else
      {
        std::cout << "Failed to set EphEPM locale" << std::endl;
      }

    AuxFunc af;
    std::filesystem::path p = af.get_selfpath();
    p = p.parent_path() / std::filesystem::u8path("..");
    p /= std::filesystem::u8path("share");
    p /= std::filesystem::u8path("locale");

    char *ch = bindtextdomain("ephepm", p.u8string().c_str());
    if(ch)
      {
        std::cout << "EphEPM text domain path: " << ch << std::endl;
      }
    else
      {
        return 1;
      }

    ch = bind_textdomain_codeset("ephepm", "UTF-8");
    if(ch)
      {
        std::cout << "EphEPM codeset: " << ch << std::endl;
      }
    else
      {
        return 1;
      }

    ch = textdomain("ephepm");
    if(ch)
      {
        std::cout << "EphEPM domain name: " << ch << std::endl;
      }
    else
      {
        return 1;
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
