project ('EphEPM', 'cpp', default_options : ['cpp_std=c++17'])
subdir ('src')
subdir('include')
subdir('po')
gmp = dependency ('gmpxx', method : 'pkg-config')
gtkmm = dependency ('gtkmm-4.0', method : 'pkg-config')
icu = dependency ('icu-i18n', method : 'pkg-config')
cpp = meson.get_compiler ('cpp')
mathgl = cpp.find_library ('mgl2', has_headers: ['mgl2/mgl.h'], required: true)
sofa = cpp.find_library ('sofa_c', has_headers: ['sofa.h'], required: true)
thread_dep = dependency ('threads')

gtkmm_version = gtkmm.version()
gtkmm_new = gtkmm_version.version_compare ('>=4.10')
if not gtkmm_new
	add_project_arguments('-DEPH_GTK_OLD', language : 'cpp')
endif

executable ('ephepm', sources : [srcbuild], include_directories: [includes], dependencies : [gmp, gtkmm, icu, mathgl, sofa, thread_dep], install : true, win_subsystem : 'windows')
install_subdir ('EphEPM', install_dir : 'share')
if host_machine.system() == 'linux'
	install_data ('ephepm.png', install_dir : 'share/icons/hicolor/256x256/apps')
	install_data ('ru.mail.bobilev_yury.EphEPM.desktop', install_dir : 'share/applications')
endif
if host_machine.system() == 'windows'
	install_data ('ephepm.ico', install_dir : 'share/icons/hicolor/256x256/apps')
endif	
