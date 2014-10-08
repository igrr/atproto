AT protocol library
===================

This is an attempt to implement DCE and DTE sides of an AT protocol.

Mostly based on [Recommendation V.250].

What works so far:
  - DCE (i.e modem side) only. Haven't started working on DTE (host) side.
  - Parsing and handling of basic and extended syntax commands and S-parameters
  - Response formatting option (ATV)
  - Echo (ATE)
  - Result code suppression (ATQ)
  - Baud rate setting (AT+IPR) saved to flash
  - Reset (ATZ)
  - Chip id (AT+GSN) and version (AT+GMR) queries

Next up:
  - Transitions between data mode and command mode
  - Implement all the IP and WiFi related commands for ESP
  - MQTT?
  - More unit tests
  - Handling of malformed input (besides returning ERROR)
  - DTE part of the business

Make
----

### ESP8266

- Adjust `XTENSA_TOOCHAIN`, `XTENSA_LIBS`, `SDK_BASE`, `ESPTOOL` directories in target/esp8266/target.mk
- Run ```make all TARGET=esp8266```
- The firmware will be generated in bin/0x00000.bin and bin/0x40000.bin

### Host
- Run ```make test```


License
-------

BSD.

I use [Catch] for unit tests (it's in include/catch.hpp), see header for it's own license.

[Recommendation V.250]:https://www.itu.int/rec/T-REC-V.250-200307-I/en
[Catch]:https://github.com/philsquared/Catch
