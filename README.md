AT protocol library
===================

This is an attempt to implement DCE and DTE sides of an AT protocol.

Mostly based on [Recommendation V.250].

What works so far:
  - DCE (i.e modem side) only. Haven't started working on DTE (host) side.
  - Parsing and handling of basic syntax commands and S-parameters
  - Parsing of extended syntax commands and arguments
  - Registration of extended command groups
  - Stubs for some standard info commands, e.g. AT+GMI
  - V0 and V1 response formatting
  - Echo
  - Result code suppression
 
Next up:
  - Transitions between data mode and command mode
  - Add unit tests (only 3 now, seriously)
  - Add hooks into ESP platform (see user_dce_transmit, user_dce_reset...)
  - Implement all the IP and WiFi related commands for ESP
  - Handling of malformed input (besides returning ERROR)
  - Better makefile (to work both for the host and ESP)
  - DTE part of the business

Make
----

```
make test
```

License
-------

BSD.

I use [Catch] for unit tests (it's in include/catch.hpp), see header for it's own license.

[Recommendation V.250]:https://www.itu.int/rec/T-REC-V.250-200307-I/en
[Catch]:https://github.com/philsquared/Catch
