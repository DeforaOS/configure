DeforaOS configure
==================

Installation notes
------------------

configure is written in C and depends on libSystem, the DeforaOS base library.
Both are built from source. The Makefiles in this repository are already
generated, so you do not need configure to build configure.

The commands below are for Debian and Ubuntu; other systems are listed at the
end.


Dependencies
------------

```
sudo apt install -y build-essential git pkg-config xsltproc docbook-xsl libxml2-utils gtk-doc-tools
```

The last four build the manual pages and a gtk-doc example in the tests. Without
gtk-doc, `make` fails in `tests/`. Without xsltproc or docbook-xsl, the manual
pages are silently skipped.


Building libSystem
------------------

configure needs libSystem 0.3.2 or later. Its directories have to be built in
this order:

```
git clone https://github.com/DeforaOS/libSystem.git
cd libSystem
for dir in include data src tools; do
	(cd "$dir" && make && sudo make install) || break
done
cd ..
# This should return the version of libSystem installed.
pkg-config --modversion libSystem
```

The last command should print a version. If it does not, set
`PKG_CONFIG_PATH` (see "Custom prefix" below).


Building and installing configure
---------------------------------

```
git clone https://github.com/DeforaOS/configure.git
cd configure
make
sudo make install
```

This installs `configure` and `configure-update` in `/usr/local/bin`, their data in `/usr/local/share/configure`, the helper scripts used by generated
Makefiles in `/usr/local/libexec/configure`, and the manual pages. To check it works, run a dry run from the source directory:

```
configure -n -v
```

Running the tests
-----------------

```
(cd tests && make tests.log)
grep -E 'PASS|FAIL' tests/tests.log
```

Note: `make tests` at the top level also runs lint and report scripts and stops
at the first failure. `fixme.sh` currently fails because the code contains
FIXME comments; this does not mean the build is broken.


Custom prefix
-------------

The prefix is compiled into configure so it can find its data files, so pass the same `PREFIX` to both `make` and `make install`. For example, without root:

```
cd libSystem
for dir in include data src tools; do
	(cd "$dir" && make PREFIX="$HOME/.local" install) || break
done

cd ../configure
export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig"
make PREFIX="$HOME/.local"
make PREFIX="$HOME/.local" install
```

`PKG_CONFIG_PATH` is only needed while building, since the library path is recorded in the binary. Package maintainers can also stage files with
`make DESTDIR="/tmp/staging" install`.


Without the documentation
-------------------------

To skip the manual pages and the documentation tools, build only these
directories instead of running `make` at the top level:

```
for dir in data src tools; do
	(cd "$dir" && make && sudo make install) || break
done
```

To uninstall, run `sudo make uninstall` in the same places you installed from.


Other systems
-------------

The steps are the same, only the packages change. The generated Makefiles work
with both GNU and BSD make.


Fedora:
```
dnf install gcc make git pkgconf-pkg-config libxslt docbook-style-xsl libxml2 gtk-doc
```

Arch:
```
pacman -S --needed base-devel git libxslt docbook-xsl libxml2 gtk-doc
```

FreeBSD:
```
pkg install git pkgconf libxslt docbook-xsl gtk-doc
```

NetBSD:
```
pkgin install git-base pkgconf libxslt docbook-xsl gtk-doc
```

OpenBSD:
```
pkg_add git libxslt docbook-xsl gtk-doc
```

macOS:
```
brew install pkgconf libxslt docbook-xsl gtk-doc
```

On Fedora, export `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig` before building
configure. On macOS, set
`XML_CATALOG_FILES="$(brew --prefix)/etc/xml/catalog"` so the manual pages can
be generated.

Windows is not supported natively: use WSL with the Debian or Ubuntu steps.
configure can still generate Makefiles for Windows targets with
`configure -O Windows`.


Uninstalling
------------

To uninstall configure, go to the configure source directory and run:

```
cd configure
sudo make uninstall
```

To uninstall libSystem, run `make uninstall` in each directory where it was
installed:

```
cd ../libSystem
for dir in include data src tools; do
	(cd "$dir" && sudo make uninstall) || break
done
```

If you installed with a custom prefix, use the same prefix again:

```
cd configure
make PREFIX="$HOME/.local" uninstall
cd ../libSystem
for dir in include data src tools; do
	(cd "$dir" && make PREFIX="$HOME/.local" uninstall) || break
done
```