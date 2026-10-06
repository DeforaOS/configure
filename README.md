DeforaOS configure
==================

About configure
---------------

configure generates Makefiles from project definition files. You describe a project once, in plain `project.conf` files, and configure writes a Makefile for every directory: build rules, dependencies, installation, and source distribution.

The generated Makefiles are portable and work with both GNU make and BSD make, without any additional tooling on the systems where the project is built.

configure depends on the DeforaOS libSystem library (version 0.3.2 or above),
which is found on the website for the DeforaOS Project:
<https://www.defora.org/>.


Getting started
---------------

A project is described by one `project.conf` per directory. The top one names the package and lists its sub-directories:

```
package=hello
version=0.1.0
subdirs=src
```

Each sub-directory then lists its targets, with a section per target:

```
targets=hello

[hello]
type=binary
sources=hello.c
install=$(BINDIR)
```

Run configure once from the top of the project, then build as usual:

```
configure
make
```

Besides `all`, the generated Makefiles provide `clean`, `distclean`, `install`, `uninstall`, `dist` and `distcheck`. Re-run configure whenever a `project.conf` changes.

Note: the Makefiles are generated, so they should not be edited by hand and are usually not kept in version control (always .gitignore them).


What can be described
---------------------

Targets can be of type `binary`, `library`, `plugin`, `object`, `script`, `command` or `libtool`.

Sources can be C, C++, Objective C, Objective C++, assembly, Go, Java or Verilog, recognised by their file extension.

Makefiles can be generated for DeforaOS, Linux, NetBSD, FreeBSD, OpenBSD, macOS and Windows. The target system is detected automatically, and can be forced with `configure -O <system>`.


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

Please read the `INSTALL.md` file for instructions about how to compile and install configure.


Bug reports
-----------

Please read the `BUGS` file for information about known problems and planned feature upgrades.

Thanks for using this program. I hope it will be useful.