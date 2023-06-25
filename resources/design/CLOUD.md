
# IOT Cloud

Cloud is responsible of registration and controlling of device. All devices and apps have to be connected to Cloud. Cloud is also responsible of authentication and integration with other devices.

IOT Cloud requires following component

1. Device Repository Server
2. Device Server
3. Http Server
4. Message Server
5. Intrusion Detection Server

All the devices will be connected with TCP protocol. Under TCP binary protocol will be used to communicate.

## Device Repository Server
All the device created its information has to be once connected to cloud, its information will be automatically picked up. Though to use it device has to be registered and associated to user.

Device repository consist of multiple databases

- Information of all the devices
- List of users
- Registered device
- Active device
- Connected device

Information stored in Database will be binary format for optimization. And it will have dual index one with device ID and other with certificate key. User table will be indexed by user ID, email, phone no., etc.


### Information of all the devices
Device contains informations like what is the type of device and configurable components. It can have both monitoring component and controlling components.

All the devices are mapped with its GUID and each have a primary device certificate pre installed. This certificate can be used to identify devices.

### List of users
This contains user profiles and its related data.

### Registered device
This is a simple mapping of device with users and it also contains list of authorised user for device.

Once device is registered it is contained in this user.

### Active device
Active device is a cache of Registered device. Device not accessed for brief period of time will be remove from this.

### Connected device
This is in memory cache of Active device. All connected device require quick support.


## Device Server
All the device has to connect to device server. This is a micro service that will get the updated from device and forward it to message server and if there is a message in message server it will forward it to device. This server is designed for pushing requests to devices.


## HTTP Server
HTTP server is only resposible to server HTML client, this can be just apache server.

## Message Server
This is server that stores message to be consumed by subscriber. All the publishers will publish the message to this.


## Intrusion Detection Server
Check for too many connections from single IP. There can be another check number of request per second and per day. This server is not designed to used for a blinking light, hence wont be allowed to have such request.
