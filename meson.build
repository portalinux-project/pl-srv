project('pl-srv', 'c',
  version: '0.07.1',
  license: 'MPL-2.0',
  default_options: [
    'c_std=c99',
  ])

inc = include_directories('include')
plrt = dependency('plrt')
subdir('src')

executable('pl-init', 'pl-init.c',
    include_directories: inc,
    dependencies: plrt,
    link_with: libsrv)

executable('pl-srv', 'pl-srv.c',
    include_directories: inc,
    dependencies: plrt,
    link_with: libsrv)
