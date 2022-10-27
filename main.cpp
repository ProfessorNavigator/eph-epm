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

#include <gtkmm.h>
#include <libintl.h>
#include "EPMApplication.h"

int
main(int argc, char *argv[])
{
  std::string Sharepath;
  AuxFunc af;
  std::filesystem::path p(std::filesystem::u8path(af.get_selfpath()));
  Sharepath = p.parent_path().u8string() + "/../share/locale";
  bindtextdomain("ephepm", Sharepath.c_str());
  bind_textdomain_codeset("ephepm", "UTF-8");
  textdomain("ephepm");
  auto app = EPMApplication::create();
  return app->run(argc, argv);
}


