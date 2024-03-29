# EphEPM
EphEPM is a program to calculate barycentric coordinates of some Solar System bodies.

## Description
EphEPM uses [EPM](https://iaaras.ru/dept/ephemeris/epm/) ephemeris to calculate coordinates. It also can plot orbits diagram.

## Installation
### Linux

`git clone https://github.com/ProfessorNavigator/eph-epm.git`\
`cd eph-epm`\
`meson -Dbuildtype=release build`\
`ninja -C build install`

You may need superuser privileges to execute last command.

### Windows

You can build EphEPM from source by MSYS2 project [https://www.msys2.org/](https://www.msys2.org/). Follow installation instructions from their site. Install dependencies from `Dependencies` section and git (mingw packages). Than create folder where you want to download source code (path must not include spaces or non Latin letters). Open mingw console and execute following commands (in example we download code to C:\EphEPM)

`cd /c/EphEPM`\
`git clone https://github.com/ProfessorNavigator/eph-epm.git`\
`cd eph-epm`\
`meson -Dbuildtype=release build`\
`ninja -C build install`

If everything was correct, you can find ephepm.exe file in `msys_directory/mingw_directory/bin`. Icon to create desktop shortcut can be found in `msys_folder/mingw_folder/share/icons/hicolor/256x256/apps/ephepm.ico` path.

Also experimental installer is available (see releases).

## Dependencies

EphEPM uses meson building system, so to build it from source you need meson and ninja.

Also you need [gtkmm-4.0](http://www.gtkmm.org/), [icu](https://icu.unicode.org/) (version >= 69), [mathgl](http://mathgl.sourceforge.net/), [SOFA](https://iausofa.org/) (in most cases this library is linking statically, so most probably in working process you do not need it) and [gmp](https://gmplib.org/) (actually you need its' C++ binding gmpxx). All libraries must have headers (for building), so if you use for example Debian Linux, you need ...-dev versions of packages.

## Usage
First of all you need to download ephemeris files from [here](https://ftp.iaaras.ru/pub/epm/). You need SPICE versions in binary format (.bsp and .bpc extensions, .bpc is needed to calculate Lunar physical libration). 2015 to 2021 versions are supported (both "usual" and "h"). 

After downloading launch the program and put path to ephemeris file in proper field. Then input all other parameters (to plot orbits inputting of date and time is sufficient, orbits diagram includes only some bodies, not all) and press "Calculate coordinates" or "Orbits". For calculation of Moon libration you need to indicate Moon libration file path and TT-TDB file path (you can use EPM planets file for such purpose).

Experimental support of DE ephemeris is available since version 2.0 (proper files can be downloaded from [NASA JPL](https://ssd.jpl.nasa.gov/ftp/eph/) site). Program has been tested only with de441 version (planets, small bodies, Moon libration). You need TT-TDB file for calculations (you can use EPM planets ephemeris file or download file from planets directory of NASA JPL site). 

## License

GPLv3 (see `COPYING`).

## Donation

If you want to help to develop this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573)

## Contacts

You can contact author by email \
bobilev_yury@mail.ru