Expected result
===============
The test code transmits a byte and a string over the software SPI MOSI line. This can be verified with a Logic Analyzer connected to the corresponding pins. When you see the clock signal and "0xaa" and "Soft SPI Test String" on the MOSI pin, the test is succesful.

Notes
==========
- The test disables all pre-initialized hardware SPI devices
- The test does not cover MISO inputs
- The test ignores clock speeds
(a board will run as fast as possible, which is not too fast for most platforms)
