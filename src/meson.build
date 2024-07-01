libsrv_sources = [
    'libsrv-error.c',
    'libsrv-frontend.c',
    'libsrv-parsing.c',
    'libsrv-supervisor.c'
]

libsrv = shared_library('srv',
                        libsrv_sources,
                        include_directories: inc,
                        dependencies: plrt,
                        install: true)
