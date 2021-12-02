# Pete

Pete is an open source alternative to [PEAT](https://trace.umd.edu/peat/) (Photosensitive Epilepsy Analysis Tool).

Pete is a free, downloadable tool for analyzing video files and identifying seizure risks in them. The analysis is based on the [Web Content Accessibility Guidelines (WCAG) 2.0, guideline 2.3.2](https://www.w3.org/TR/WCAG21/#three-flashes), with a [correction](https://www.w3.org/WAI/GL/wiki/Relative_luminance) to the relative luminance formula.

In this repository resides the main engine. It can't be run on its own, it's a library.

## Building

**WARNING: Building on platforms other than linux has not been tested. Please report any errors you find if yout try.**

Currently, Pete uses the [Scons](https://scons.org/) build system.

In the root directory run `scons`.
The different build options are:
  - `-use_PIC=[True/False]` use position independent code. Default is `False`.
  - `-target_path=[path]` the path where the compiled binary will be placed. Default is `build/`.
  - `-target_name=[name]` the name the compiled binary file will be given. Default is `libpete`.

Currently the build system only supports compiling to a static library.
If you want to use it as a dynamic library, check out the hacky [bash script](https://github.com/pete-video-analysis/pypete/blob/main/build.sh) in the pypete repository.

In the near future I plan on switching to the [Make](https://www.gnu.org/software/make/) build system.
