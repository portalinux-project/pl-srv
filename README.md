# pl-srv
`pl-srv` or the PortaLinux Init System package is a very tiny init system made
specifically for the PortaLinux operating system. It is written in C99 using
POSIX calls and the ESB/PortaLinux API. The only component with Linux-specific
system calls is `pl-init` and it's only used to shut down/restart the system.

It is heavily inspired by `systemd-init`, but it cuts out a lot of the fat.

# Components

The `pl-srv` init system package consists of the following components:

- `libsrv`: A library containing most of the functionality of the `pl-srv` and
`pl-init` programs. It is entirely written using POSIX and ESB/PL-API calls, 
and thus is completely portable and can be compiled and ran on any operating
system that fully implements the POSIX.1-2008 API.

- `pl-init`: A minimal PID 1 program meant to have as little functionality as
possible. Its main purpose is to make the system not crash, even if everything
else does. It only starts up the `pl-srv` service supervisor in init mode and
then gets into a forever loop that gets rid of zombie processes. This is the
only Linux-specific component of the `pl-srv` package

- `pl-srv`: A simple service supervisor with a small footprint. This is the
main program of this package. It only makes calls to `libsrv`, which means it
is portable and can run on anything `libsrv` runs on.

# Contributions

Contributions will be opened after the full 1.00 release of `pl-srv`
