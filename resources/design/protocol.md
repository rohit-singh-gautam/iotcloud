
# Protocol For Communication

Type of device
1. Controller
1. Monitor

Type of Command Controller
1. Generic All
    1. Set On/Off
    1. Set Open/Close
1. Generic Variable
    1. 0 to 65535
1. Generic Light - Multiple channels
    1. 0 to 65535 for all channels


Type of Command for Monitor
1. CO<sub>2</sub> Monitor
    1. Alert - CO2 level
    1. CO2 - level
1. Door Monitor
    1. Update Open/Close
1. Temperature Monitor
    1. Update Temperature



Type of communication
1. Request Type
    1. Long Poll Request
    1. Immediate Request
1. Update server
    1. Full Update
    1. Partial Update
1. Update Device
    1. Full Update
    1. Partial Update
1. Program Device

#Primitives
## Length type
1. Len8 - 8 bit length
1. Len16 - 16 bit length
1. LenVar
    1. MSB 0 - 8 bit (7 bit used)
    1. MSB 1 - 8 bit (7 bit used) 8 bit (all bit used)
## Other type
1. CHAR - 16 bit Unicode character
1. String - [Length: Len16, Value: CHAR of size Length]
1. UUID - 16 byte UUID
1. INT8
1. INT16
1. INT24
1. INT32


## Instructions Set for Device
1. Device: Create a new certificate
1. Device: Request registration sharing its public key and its description, encrypted using public key of Server
1. Server: Check the credentials and accept registration
1. Server: Reply with success and store client information

