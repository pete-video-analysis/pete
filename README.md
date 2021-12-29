# Pete

Pete is an open source alternative to [PEAT](https://trace.umd.edu/peat/) (Photosensitive Epilepsy Analysis Tool).

Pete is a free, downloadable tool for analyzing video files and identifying seizure risks in them. The analysis is based on the [Web Content Accessibility Guidelines (WCAG) 2.0, guideline 2.3.2](https://www.w3.org/TR/WCAG21/#three-flashes), with a [correction](https://www.w3.org/WAI/GL/wiki/Relative_luminance) to the relative luminance formula.

In this repository resides the main engine. It can't be run on its own, it's a library.

## Building

Currently, Pete uses the [Make](https://www.gnu.org/software/make/) build system.

If you're on Windows, use GNU Make and not NMake. Also make sure you have the [gcc](https://gcc.gnu.org/) compiler. You can get it from the [MinGW](https://sourceforge.net/projects/mingw/) or [MinGW-w64](https://www.mingw-w64.org/) projects.

 - Static library: run `make static`
 - Shared library: run `make shared`
 - Both: run `make all`
 - Only the object files: run `make objects`
 - Remove all the build results: run `make clean`
