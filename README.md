DeforaOS configure
==================

About configure
---------------

configure generates Makefiles from project definition files. Projects are
described through project definition files, named `project.conf`, and configure
creates a Makefile for each directory of the project. They contain build rules,
with dependency handling; they implement the installation and uninstallation
procedure, the creation of source archives for redistribution, as well as the
execution of test suites.

The Makefiles generated are meant to be portable and expected to work with any
common variant, including both GNU make and BSD make at the same time. No
additional tooling is required on the systems where the project is built.

configure depends on the DeforaOS libSystem library (version 0.3.2 or above),
which is found on the website for the DeforaOS Project:
<https://www.defora.org/>.


Getting started
---------------

A project is described by one `project.conf` file per directory, in an ini-style
file format. The top file may contain the package's name and version, and an
optional vendor name. A list of sub-directories can also be provided, as
follows:

```dosini
package=hello
version=0.1.0
subdirs=src
```

Targets can be defined anywhere, with a dedicated section per target:

```dosini
targets=hello

[hello]
type=binary
sources=hello.c
install=$(BINDIR)
```

Run configure once from the top of the project, then build as usual:

```shell-session
$ configure
$ make
```

Besides the default `all` target, the Makefiles generated provide `clean`,
`distclean`, `install`, `uninstall`, `dist`, and `distcheck` targets when
relevant. Re-run configure whenever a `project.conf` changes.

Note: the Makefiles are generated, so they should not be edited manually and it
is best to not include them in version control (e.g., list them in `.gitignore`
when using Git).


What can be described
---------------------

Targets can be of type `binary`, `library`, `plugin`, `object`, `script`,
`command`, or `libtool`.

Sources can be C, C++, Objective C, Objective C++, assembly, Go, Java or
Verilog, and are recognised by their file extension.

Makefiles can be generated for DeforaOS, Linux, NetBSD, FreeBSD, OpenBSD, macOS
and Windows. The target system is detected automatically, or can be forced
with `configure -O <system>`.


Documentation
-------------

The manual pages cover the details:

```
man project.conf        the format of the project definition files
man configure           the command and its options
man configure-update    updating the helper scripts of a project
```


Compiling configure
-------------------

Please read the `INSTALL.md` file for instructions about how to compile and
install configure.


Bug reports
-----------

Please read the `BUGS` file for information about known problems and planned
feature upgrades.

Thanks for using this program. I hope it will be useful.
