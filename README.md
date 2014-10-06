AT protocol library
===================

This is an attempt to implement DCE and DTE sides of an AT protocol.

Mostly based on [Recommendation V.250].

What works so far:
  - DCE (i.e modem side) only. Haven't started working on DTE (host) side.
  - Parsing and handling of basic syntax commands and S-parameters
  - Parsing of extended syntax commands and arguments
  - Extended syntax result codes
  - Registration of extended command groups
  - Stubs for some standard info commands, e.g. AT+GMI
  - V0 and V1 response formatting
  - Echo
  - Result code suppression
 
Next up:
  - Baud rate command (+IPR)
  - Add hooks into ESP platform (see user_dce_transmit, user_dce_reset...)
  - Transitions between data mode and command mode
  - More unit tests
  - Implement all the IP and WiFi related commands for ESP
  - Handling of malformed input (besides returning ERROR)
  - DTE part of the business

Make
----

###   Host
- Run ```make test```

### ESP8266:

- Adjust `XTENSA_TOOCHAIN`, `XTENSA_LIBS`, `SDK_BASE`, `ESPTOOL` directories in target/esp8266/target.mk.

- Run ```make all TARGET=esp8266```
- The firmware will be generated in bin/0x00000.bin and bin/0x40000.bin

License
-------

BSD.

I use [Catch] for unit tests (it's in include/catch.hpp), see header for it's own license.

[Recommendation V.250]:https://www.itu.int/rec/T-REC-V.250-200307-I/en
[Catch]:https://github.com/philsquared/Catch
