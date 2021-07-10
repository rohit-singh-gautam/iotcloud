# LOG and Monitoring Server

## Monitoring Server

## LOG Server

### Goal
1. Possibility to add huge number of logs
2. Very fast and efficient
3. Low file size
4. Local and remote storage

### Specification
To store huge number of logs and efficiently we require to create a logging system that would convert all the logs to very low size enumeration and keep it in memory. Logs in memory will be dump to file in periodic time or if it reaches certain size.

Further we restrict max size of logs to 256 bytes and parsed log to max 1024 character.

Optimization of logs is done by converting string to a 16 bit ID. For small project like this we do not require more than 65536 logs. Also, there will be different logs for different module, hence each module will have its own logging ID. This will further reduce logging ID size requirement.

Logging will be in chunk of 8kb, Also, design will be to use no lock to use this chunk.
8kb is used a cluster size of file is 8kb. We will be formatting filesystem at size of 8kb.

Same is true for server. It keep logs in memory of size 8kb before writing it to disk.

Server implementation will be done once client and server is completed.