# trimpng
Trim solid color border pixels from a PNG image using the top left pixel as the border reference color.

Usage:

    ./trimpng input.png output.png

# Dependencies

**libspng**: https://libspng.org/
**zlib**: https://www.zlib.net/

# Building

I installed libspng using `make install`. zlib was not anywhere in my includes path (on MacOS, not sure why) so I installed it with `brew install zlib`.

Build with `make`.

# License


Methods commented from libspng example.c subject to libspng license.
https://github.com/randy408/libspng/blob/master/LICENSE