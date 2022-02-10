# Oh, Brother...

I put this together because I wanted an old-school filter-style driver for my
Brother HL-L2300D laser printer. I expect it will work with other related
models.

There is also an [open-source CUPS driver][brlaser] available from
Peter De Wachter if that's more your style. I seem to recall also seeing an
official CUPS driver available from Brother, though my recollection is that
it was Linux and x86 specific.

[brlaser]: https://github.com/pdewacht/brlaser

## Installation

I've successfully built this program on macOS 11 (Big Sur) and FreeBSD 13.
You may need to adjust the makefile for your system if it's different. Once
that's set, just make and install:

	user@x220:/usr/local/src/oh_brother $ make
	cc  -O2 -pipe -c compress.c -o compress.o
	cc  -O2 -pipe -c main.c -o main.o
	cc  -O2 -pipe -c parameters.c -o parameters.o
	cc  -O2 -pipe -c pcl.c -o pcl.o
	cc  -O2 -pipe -c pjl.c -o pjl.o
	cc -o oh_brother compress.o main.o parameters.o pcl.o pjl.o
	$ su
	Password:
	root@x220:/usr/local/src/oh_brother # install oh_brother /usr/local/bin
	root@x220:/usr/local/src/oh_brother # install oh_brother.mdoc \
		/usr/local/share/man/man1/oh_brother.1

## Quick start

This program takes as input raw raster data and produces as output commands
fit to be sent to the printer. By default, this program expects its input
to fit letter-size paper at 600 DPI. However, it supports a variety of options
for setting up the printer and describing the input data. See the manual page
for details.

Ghostscript is commonly used to produce raw raster data from PostScript and
PDF documents. Something like this should produce letter-size raster data at
600 DPI:

	gs -dBATCH -dNOPAUSE -sDEVICE=bit -r600 -g5100x6600 document.pdf

### Direct printing

If you just want to print the occasional job and don't want to do any setup,
you can print right from the command line. For a USB-attached printer,
something like this should work:

	gs -dBATCH -dNOPAUSE -sDEVICE=bit -r600 -g5100x6600 \
		-sstdout=%stderr -sOutputFile=- document.pdf | \
		oh_brother > /dev/unlpt0

Or if your printer is attached to the network with a JetDirect-style print
server, this may do the trick:

	gs -dBATCH -dNOPAUSE -sDEVICE=bit -r600 -g5100x6600 \
		-sstdout=%stderr -sOutputFile=- document.pdf | \
		oh_brother | nc 10.0.1.2 9100

Of course, change out the name of the PostScript or PDF document you want to
print as well as the device file or IP address for your printer.

### Printing with LPD

The FreeBSD Handbook has an [excellent chapter on printing][printing] which
can help you configure LPD. Following from the example the Handbook gives
for printing PostScript to PCL printers, you might make a script like this
in /usr/local/libexec/ps2brother and use it as your input filter:

	#!/bin/sh
	/usr/local/bin/gs -dBATCH -dNOPAUSE -q -sDEVICE=bit -r600 -g5100x6600 \
		-sOutputFile=- - | /usr/local/bin/oh_brother -duplex LONG

[printing]: https://docs.freebsd.org/en/books/handbook/printing/
