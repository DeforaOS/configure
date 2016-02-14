DeforaOS configure
==================

Installation notes
------------------

To install configure in /usr/local, just proceed this way:

    $ make
    $ make install

To install it in another path, use the PREFIX variable:

    $ make PREFIX="/path/to/destination" install

The DESTDIR variable is also supported for package maintainers.
