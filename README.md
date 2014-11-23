[![Build status](http://img.shields.io/travis/igrr/atproto.svg)](https://travis-ci.org/igrr/atproto)

AT protocol library and firmware
================================

This is an attempt to implement DCE and DTE sides of an AT protocol, mostly based on [Recommendation V.250].
The code has two parts: a platform-independent code for basic DCE/DTE functions, and target-specific commands that control WiFi, TCP/IP, etc. The main target for DCE is an ESP8266 chip, but it is easy to add other targets as well, or pull the platform-independent code into another project.

The AT commands supported by the ESP8266 target are described [here](./commands.md).

What works so far:
  - DCE (i.e modem side) only. Haven't started working on DTE (host) side.
  - Parsing and handling of basic and extended syntax commands and S-parameters
  - Response formatting option (ATV)
  - Echo (ATE)
  - Result code suppression (ATQ)
  - Baud rate setting (AT+IPR) saved to flash
  - Reset (ATZ)
  - Chip id (AT+GSN) and version (AT+GMR) queries
  - Wifi commands (CWLAP, CWJAP, CWSAP, CWSTAT, CWLIF)
  - Wifi connection status reports (+CWSTAT:)
  - Commands to get IP and MAC address for AP and STA (CIPAP?, CIPSTA?, CIPAPMAC?, CIPSTAMAC?)
  - Domain names resolution (CIPRESOLVE)
  - TCP/UDP socket context create/release (CIPCREATE, CIPCLOSE)
  - TCP/UDP client connect/disconnect (CIPCONNECT, CIPDISCONNECT)
  - Send/receive data (CIPSENDI, CIPRD)

Next up:
  - TCP/UDP server
  - MQTT
  - Pass-through mode for TCP/UDP connections
  - More unit tests
  - Handling of malformed input (besides returning ERROR)
  - DTE part of the business

Many commands that return ERROR print error reason when debug output is enabled. 
To enable debug output, issue AT+IDBG=1 command.
 
Prebuilt firmware for ESP8266
-----------------------------
If you don't want to go through the build process described below (though I advise you do), get the [prebuilt binaries] on my website.

Make
----

#### ESP8266

- Install/compile the toolchain. Check the ESP8266 community wiki for [instructions](https://github.com/esp8266/esp8266-wiki/wiki/Toolchain).
- Get the [ESP8266 SDK] version 0.9.3.
- Adjust `XTENSA_TOOCHAIN`, `XTENSA_LIBS`, `SDK_BASE`, `ESPTOOL` directories in target/esp8266/target.mk
- Apply the diff in target/esp8266/c_types.h.diff to include/c_types.h in the SDK so that it doesn't redefine types from stdint.h.
- Run ```make clean all TARGET=esp8266```
- The firmware will be generated in bin/0x00000.bin and bin/0x40000.bin

#### Host
There are some unit tests for target-independent code that run on PC. Doing ```make test``` will build the tests and run them. Don't forget to ```make clean``` before you ```make test``` if you have built for ESP8266 before that, or the linker will complain about unsupported object file format.

License
-------

BSD. See the LICENSE file.

I use [Catch] for unit tests (it's in include/catch.hpp), see header for it's own license.

<!--I have pulled in [LwIP] code as a subtree. It is also licensed under BSD, see lwip/COPYING.-->

[Recommendation V.250]:https://www.itu.int/rec/T-REC-V.250-200307-I/en
[Catch]:https://github.com/philsquared/Catch
[prebuilt binaries]:http://th.igrr.me
[LwIP]:http://savannah.nongnu.org/projects/lwip/
[ESP8266 SDK]:http://bbs.espressif.com/viewtopic.php?f=5&t=53


