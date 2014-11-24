Communication with AT protocol happens between the host and the modem. The host is usually a PC or a microcontroller, and the modem is a device used to access some communication medium (i.e. cable modem, GSM/GPRS modem, or a WiFi module). In the terminology of AT communication the host is called *Data Terminal Equipment* (DTE) and the modem is called *Data Circuit-terminating Equipment* (DCE).

DCE and DTE communicate over a serial link. DTE sends **command lines** which contain two characters (```AT```) followed by one or more commands. The command line is terminated by a carriage return character (```<cr>```).

When the DCE receives the command line, it executes the corresponding command and issues zero or more **information responses** followed by a **result code**. Additionally the DCE may be echoing all the characters received from the DTE (see E command).

Information responses are not standardized. See individual command description for details about the presence and the format of information responses.

### <a name="rc_and_formatting"/> Result codes and formatting

Result codes come in two flavors: basic syntax result codes and extended syntax result codes. Additionally, there are two result code formats: V0 and V1, which are set by V command (V as in verbose).

The default V1 format is much more human-readable and is best for manual interaction with the modem in terminal, while V0 format is easier to parse with a program.

#### V0 format
 Response type | Syntax
-----------|--------------
Information response|```<text><cr><lf>```
Basic syntax result code|```<numeric code><cr>```
Extended syntax result code|```<text code><cr>```

#### V1 format (default)
 Response type | Syntax
-----------|--------------
Information response|```<cr><lf><text><cr><lf>```
Result code (basic and extended syntax)|```<cr><lf><text code><cr><lf>```

#### Basic syntax result codes

 Text code | Numeric code
-----------|--------------
 OK        | 0
 CONNECT   | 1
 ERROR     | 4
 
#### Extended syntax result codes

Extended syntax results have the following format: ```+<name>:[value1[,value2[,...]]]```

For example, the "get wifi connection status" command ```AT+CWSTAT?``` may return ```+CWSTAT:5``` as a result code. Keep in mind that the result code will be surrounded with additional cr's and lf's as outlined above, depending on the formatting option (V0 or V1).

### Commands

Commands take two forms: basic commands and extended commands. Extended command names start with a plus (```+```) sign. Additionally all commands are divided into READ, WRITE (EXEC), and TEST commands by their semantical meaning. Parameters can be typically read, written and tested, although there are some read-only parameters. Actions can be executed. WRITE and EXEC commands have identical syntax.

#### Basic commands

Basic WRITE commands take one optional integer argument that directly follows the command. If the argument is not present, the default value is assumed. READ commands have no arguments.

Command type | Command line format | Example
-------------|---------------------|----------
WRITE | ```AT<command name>[argument]<cr>``` |  ```ATE0<cr>``` 
READ  | ```AT<command name>?<cr>``` |  ```ATS3?<cr>```

#### Extended commands

Extended format commands can be either READ, WRITE, or TEST commands.

WRITE commands take zero or more arguments. Multiple arguments are separated with commas.

Command type | Command line format | Example
-------------|---------------------|----------
WRITE (EXEC)  | ```AT<command name>[=argument1[,argument2[,...]]]<cr>``` |  ```AT+CWSAP="ESP_AP","pwd_1234",1,4<cr>```
READ         | ```AT<command name>?<cr>``` | ```AT+IPR?<cr>```
TEST (QUERY) | ```AT<command name>=?<cr>``` | ```AT+CWMODE=?<cr>```

There are two types of arguments: numbers and strings. Numbers are nonnegative decimal integers. Strings are quoted with double quotes (```"string"```), and may contain the following escape character codes:

Escape code | Character
-----------|--------------
 ```\r```        | ```<cr>``` character (ASCII code 13)
 ```\n```   | ```<lf>``` character (ASCII code 10)
 ```\"```     | ```"``` (double quote)
 ```\\```     | ```\``` (backslash)
 ```\xmn```   | Symbol with ASCII code 0xmn. ```mn``` should be lowercase hex characters. 
 
Note that ```\x00``` code is not supported. This should probably be fixed in future.


## Basic commands
#### Reset command (Z)
Command line | Response
-------------|---------
```ATZ``` | none

The module will reset. While no result code is printed, do expect several lines of information from the bootloader. Unlike other commands, Z command will be handled even when DCE has not returned a result code from a previous command.

#### Output format (V)
Command line | Response | Notes
-------------|----------|-------
```ATV0```|```OK```|Set V0 output format
```ATV1``` or ```ATV```|```OK```|Set V1 output format (default)

The output format defines the way DCE replies are wrapped in newlines ([see above](#rc_and_formatting)).
Command descriptions below list text result codes (OK, ERROR), but these will be replaced by numeric result codes if V0 format is active. 

#### Echo (E)
Command line | Response | Notes
-------------|----------|-------
```ATE0```|```OK```|Disable echo
```ATE1```|```OK```|Enable echo (default)

Controls whether DCE will send the received characters back to the DTE.

__Note:__ if the DTE sends an ```<lf>``` character after the ```<cr>``` character at the end of the command line (some terminals do that), DCE will not echo the ```<lf>``` character.

#### Result code suppression (Q)
Command line | Response | Notes
-------------|----------|-------
```ATQ0```|```OK```|Output result codes (default)
```ATQ1```|```OK```|Suppress result codes

Enables or disables result code suppression. Information responses are not affected by this setting.

#### Reset to default settings (F)
Command line | Response | Notes
-------------|----------|-------
```AT&F0```|```OK```| Reset DCE settings to default values
```AT&F1```|```OK```| Reset DCE settings, baud rate, and WiFi configuration

DCE settings that are reset are: S-parameters S3, S4, S5, and Q, E, V settings.
```AT&F1``` also wipes stored WiFi settings (AP and STA) and resets baud rate to the default value.

#### ```<cr>``` code parameter (S3)
Command line | Response | Notes
-------------|----------|-------
```ATS3?```|Information response: ```013``` and result code ```OK```| Query parameter value. Information response is zero-padded to 3 digits.
```ATS3=14```|```OK```| Set parameter value

Controls the code used for the ```<cr>``` character.

#### ```<lf>``` code parameter (S4)
Command line | Response | Notes
-------------|----------|-------
```ATS4?```|Information response: ```010``` and result code ```OK```| Query parameter value. Information response is zero-padded to 3 digits.
```ATS4=9```|```OK```| Set parameter value

Controls the code used for the ```<lf>``` character.

#### ```<bs>``` code parameter (S5)
Command line | Response | Notes
-------------|----------|-------
```ATS5?```|Information response: ```008``` and result code ```OK```| Query parameter value. Information response is zero-padded to 3 digits.
```ATS5=7```|```OK```| Set parameter value

Controls the code used for the ```<bs>``` (backspace) character.

## General commands (+G)
#### Serial number (+GSN)
Command line | Response | Notes
-------------|----------|-------
```AT+GSN```|Information response: serial number (single line), ```OK```| 

Serial number for ESP8266 is the output of ```system_get_chip_id``` function formatted as 8 lower-case hex characters.

#### Version (+GMR)
Command line | Response | Notes
-------------|----------|-------
```AT+GMR```|Information response: version description (single line), ```OK```| 

Version description contains version number and a short hash of current git revision.

#### Identification (+GMM)
Command line | Response | Notes
-------------|----------|-------
```AT+GMM```|Information response: firmware identification (two lines), ```OK```| 

Returns chip name and a link to this firmware on github.

#### Memory stats (+GMEM)
Command line | Response | Notes
-------------|----------|-------
```AT+GMEM```|Information response: memory info (four lines), ```OK```| 

First line of the information response is free heap size. Next 3 lines are the output of ```system_print_meminfo``` function.

## Interface control commands (+I)

#### DCE Ready unsolicited result code (+IREADY)
Command line | Response | Notes
-------------|----------|-------
none |```+IREADY:"atproto","0.1","a343e46"```| 

This unsolicited result code is sent by the module when boot procedure is finished. The parameters are firmware name, firmware version, and firmware revision hash.
This result code is always sent in V1 format.

#### Debug output paramter (+IDBG)
Command line | Response | Notes
-------------|----------|-------
```AT+IDBG=?```|```+IDBG:(0,1)```| Query parameter values
```AT+IDBG?```|```+IDBG:0```| Get debug mode parameter
```AT+IDBG=1```|```OK```| Set debug mode parameter

When debug mode is enabled, the code will print some info when errors happen. This can help to narrow down the cause of an error. Additionally expect some output from the underlying wifi libraries. Default value is 0 (no debug output). This setting is not saved on reset.

#### Baud rate paramter (+IPR)
Command line | Response | Notes
-------------|----------|-------
```AT+IPR=?```|```+IPR:(0),(list of supported baud rates)```| Query supported baud rates
```AT+IPR?```|```+IPR:9600```| Get baud rate
```AT+IPR=115200```|```OK```| Set baud rate

When the firmware is flashed for the first time, the baud rate is set to 9600. Subsequent changes to this parameter are saved to flash. For now, autobaud mode (```AT+IPR=0```) doesn't work (but contributions are welcome).

## WiFi commands (+CW)
#### WiFi mode (+CWMODE)
Command line | Response | Notes
-------------|----------|-------
```AT+CWMODE=?```|```+CWMODE:(1-3)```| Query supported modes
```AT+CWMODE?```|```+CWMODE:1```| Get current mode
```AT+CWMODE=3```|```OK```| Set mode

Mode id | Meaning
--------|--------
1 | Station mode
2 | SoftAP mode
3 | Station + SoftAP mode

After changing wifi mode, reset the module with ATZ command (this is required for Espressif's IoT SDK ≤ 0.9.1). The mode is saved to flash and is preserved when the module is reset.

#### List access points (+CWLAP)
Command line | Response 
-------------|----------
```AT+CWLAP```|Information response with APs found, and ```OK``` result code

Information response format (for each AP found):

```<Authentication mode>,"<SSID>",<RSSI value without minus sign><cr><lf>```

Auth mode id | Meaning
-------------|--------
0 | Open
1 | WEP
2 | WPA-PSK
3 | WPA2-PSK
4 | WPA/WPA2-PSK

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 1 or 3.
Also, due to some strange behavior of the wifi libraries, the module may reset when connection to an AP is in progress when ```+CWLAP``` is called. The most reliable strategy is to disconnect from AP, reset the module, and then call ```+CWLAP```.

#### Parameters to join an AP (+CWJAP)
Command line | Response | Notes
-------------|----------|-------
```AT+CWJAP=?```|```+CWJAP:"ssid","password"```| Query command format
```AT+CWJAP?```|```+CWJAP:"",""```| Get current SSID and password
```AT+CWJAP="Ap","MyPass12"```|```OK```| Set SSID and password

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 1 or 3.
If the module is already connected to an AP, it is necessary to issue a ```+CWQAP``` command first. It is also advised to reset the module after ```+CWQAP```. The SSID and password are stored in flash, and the module will automatically attempt to connect to the AP after reset.

#### Disconnect from AP (+CWQAP)
Command line | Response | Notes
-------------|----------|-------
```AT+CWQAP```|```OK```| 

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 1 or 3.
The stored SSID and password are erased and the module is disconnected from the AP. Some versions of IoT SDK have an issue that makes reset after this command necessary.

#### WiFi connection state unsolicited result code (+CWSTAT)
Command line | Response | Notes
-------------|----------|-------
none|```+CWSTAT:5```| Connection state has changed

State id | Meaning
-------------|--------
0 | Idle (SSID and password have not been set)
1 | Connecting
2 | Authentication failed
3 | AP with given SSID not found
4 | Connect failed for other reasons
5 | Connected and got IP address

This unsolicited result code is emitted whenever the connection to the AP changes state. The module has to be in WiFi mode 1 or 3, and an SSID and password must been provided.

#### Parameters to set up a SoftAP (+CWSAP)
Command line | Response | Notes
-------------|----------|-------
```AT+CWSAP=?```|```+CWSAP:"ssid","password",(1-13),(1-4)```| Query command format. Third argument is channel number. Fourth argument is authentication mode (see description of +CWLAP command).
```AT+CWSAP?```|```+CWSAP:"ESP0869","",6,0```| Get current SoftAP configuration
```AT+CWSAP="ESP_AP","pwd_1234",1,4```|```OK```| Set new configuration. 

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 2 or 3.

#### List clients associated with SoftAP (+CWLIF)
Command line | Response | Notes
-------------|----------|-------
```AT+CWLIF```|Information response: one line per associated client, ```OK```| 

Information response line format:

```<ip address>,<mac address><cr><lf>```

Note: due to an oversight, IP and MAC addresses are not quoted in this response.

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 2 or 3.


## TCP, UDP commands

#### Get/set IP address of STA interface (+CIPSTA)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPSTA=?```|+CIPSTA="ip","mask","gateway"| Query command format
```AT+CIPSTA?```|Extended syntax result code: ```+CIPSTA:"192.168.1.120","255.255.255.0","192.168.1.1"```|Get IP address, network mask, gateway IP address

Command line | Response | Notes
-------------|----------|-------
```AT+CIPSTA="192.168.1.170","255.255.255.0","192.168.1.1"```|```OK```|Set IP address, network mask, and gateway
```AT+CIPSTA=0```|```OK```|Get address from DHCP client

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 1 or 3.

#### Get MAC address of STA interface (+CIPSTAMAC?)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPSTAMAC?```|Extended syntax result code: ```+CIPSTAMAC:"18:fe:34:aa:bb:cc"```|

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 1 or 3.

#### Get IP address of AP interface (+CIPAP)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPAP?```|Extended syntax result code: ```+CIPAP:"192.168.4.1"```|

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 2 or 3.

#### Get MAC address of AP interface (+CIPAPMAC?)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPAPMAC?```|Extended syntax result code: ```+CIPAPMAC:"1a:fe:34:aa:bb:cc"```|

This command will return ```ERROR``` result code if current WiFi mode (as returned by ```+CWMODE?```) is not 2 or 3.

#### Resolve domain name (+CIPRESOLVE)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPRESOLVE="github.com"```|Extended syntax result code: ```+CIPRESOLVE:"192.30.252.131"``` or basic format result code in case of an error: ```ERROR```|

-------------------------------
**Fair warning: TCP/UDP API is not stable yet, so the following commands will likely be changed at some point in the future.**

#### Create client connection context (+CIPCREATE)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPCREATE=?```|```+CIPCREATE:"TCP|UDP"[,port][,buffer_size]```| Query command format
```AT+CIPCREATE="TCP",10201,1024```|Extended syntax result code: ```AT+CIPCREATE:0,10201,1024```|Local port and receive buffer size arguments are optional

Up to 5 client contexts may be created.

The first response parameter is the id of the created context.

If local port is not specified, it will be generated randomly.
If receive buffer size is not specified, it will be set to a default value (currently 2048 bytes).

When you are done using the context, recycle it with a call to ```+CIPCLOSE``` command.

#### Close connection context (+CIPCLOSE)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPCLOSE=?```|```+CIPCLOSE:(0-6)```| Query command format
```AT+CIPCLOSE=0``` | ```OK``` | 


The command will return ```ERROR``` if the context with specified id is not in use.

This command applies to client contexts created using ```+CIPCREATE``` and connections obtained from ```+CIPACCEPT``` unsolicited result code.

#### Create server context (+CIPLISTEN)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPLISTEN=?``` | ```+CIPLISTEN:\"TCP|UDP\"[,port][,buffer_size]``` | Query command format
```+CIPLISTEN="TCP",80,2048``` | ```+CIPLISTEN=6,80,2048``` | Listen on port 80, create contexts for connecting clients with 2048 bytes RX buffer

Start server listening on a given port. The server binds to all available interfaces (AP and STA).

#### Client connected to server unsolicited result code (+CIPACCEPT)
Command line | Response | Notes
-------------|----------|-------
none|```+CIPACCEPT=0,"192.169.0.120"```|

This unsolicited result code is issued when a client connects to the server that was set up with ```+CIPLISTEN``` command. The first parameter is a context id for the client connection, the second parameter is the IP address of the client.

#### Initiate client connection (+CIPCONNECT)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPCONNECT=?```|```+CIPCONNECT:(0-4),"ip_addr",(1-65535)```| Query command format
```AT+CIPCONNECT=0,"192.30.252.131",80```|```OK```|

Supply the context id generated using ```+CIPCREATE``` as the first argument.

The command returns ```ERROR``` if one of the arguments is missing or invalid (e.g. context with the specified id has not been created).

For UDP contexts, the command stores the remote address and port and starts receiving data on the local port.
For TCP contexts, the command verifies that the arguments are correct, returns ```OK``` immediately and initiates TCP connection. The connection status is then reported with ```+CIPCONNECT``` and ```+CIPRECONNECT``` unsolicited result codes.

#### Connection successful unsolicited result code (+CIPCONNECT)
Command line | Response | Notes
-------------|----------|-------
none|```+CIPCONNECT:0```| 

This unsolicited result code is issued by the DCE when TCP client connection is established.
Argument indicates the context id of the connection.

#### Connection failed/interrupted unsolicited result code (+CIPRECONNECT)
Command line | Response | Notes
-------------|----------|-------
none|```+CIPRECONNECT:0,11```| 

This unsolicited result code is issued by the DCE when TCP client connection has failed or was broken.
First parameter indicates the context id of the connection. Second parameter is a code indicating the reason for the problem.

_TODO: add error codes description_

#### Close client connection (+CIPDISCONNECT)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPDISCONNECT=?```|```+CIPDISCONNECT:(0-4)```| Query command format
```AT+CIPDISCONNECT=0```|```OK```|

Terminate TCP connection with id given as the first argument.
Don't call this function if connection was interrupted and ```+CIPRECONNECT``` unsolicited result code was issued.
The command returns ```OK``` immediately.

#### Client connection closed successfully unsolicited result code (+CIPDISCONNECT)
Command line | Response | Notes
-------------|----------|-------
none|```+CIPDISCONNECT:0```|

This unsolicited result code is issued when the connection is successfully closed using ```+CIPDISCONNECT``` command.

#### Send data using immediate argument (+CIPSENDI)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPSENDI=?```|```+CIPSENDI:(0-4),"data_to_send"```| Query command format
```AT+CIPSENDI=0,"test\r\n\x00"```|```OK```| Send data

Send string of data to the context specified in the first argument.

The context has to be created using ```+CIPCREATE``` and connected using ```+CIPCONNECT```.

The command returns immediate result code, ```OK```, and emits an unsolicited result code (```+SENDI```) when the transmission is complete (for TCP contexts).

#### Data has been sent unsolicited result code (+CIPSENDI)
Command line | Response | Notes
-------------|----------|-------
none|```+CIPSENDI:0```| 

This unsolicited result code indicates that the data for the TCP context specified by the first parameter has been sent.

#### Data received unsolicited result code (+CIPDR)
Command line | Response | Notes
-------------|----------|-------
none|```+CIPDR:0,6```| E.g. when ```test<cr><lf>``` is received
none|```+CIPDR:0,12```| another ```test<cr><lf>``` is received

This unsolicited return code indicates that the data is received. First parameter is the context id, second parameter is the size of data available for reading.

#### Read received data (+CIPRD)
Command line | Response | Notes
-------------|----------|-------
```AT+CIPRD=?```|```+CIPRD:(0-6)```| Query command format
```AT+CIPRD=0```|Extended syntax result code: ```+CIPRD:0,12```, information response with the received data|

Returns context id and the number of bytes to be read.
Information response that follows is the data received, as plain text.
Note that like any other information response, there are additional ```<cr>```'s and ```<lf>```'s at the beginning and at the end ([see above](#rc_and_formatting)).

