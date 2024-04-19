 For example, the ``x`` and ``y``
coordinate of a point are predicted by adding their delta in the preceding two
points to the ``x`` and ``y`` of the last point, whereas the difference
between subsequent points' time values is expressed as a multiple of the most
common increment, while RGB valued are compressed with simple difference
coding. In each case the prediction residuals are compressed with adaptive
arithmetic coding.

The compressor treats the LAS points of types 0, 1, 2, 3, 4, 5, and 6 as
consisting of a number of items: POINT10, GPSTIME11, RGB12, and WAVEPACKET13
and uses separate modules with separate version numbers to compress each item.
This makes it easy to improve, for example, the compressor for the
``gps_time`` without affecting the other compression schemes while
guaranteeing full backwards compatibility. More importantly it allows the
engine to add new LAS point types as the LAS specification evolves into the
future without affecting existing point types.
