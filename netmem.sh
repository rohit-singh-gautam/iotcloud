
echo This must be executed using sudo

sysctl net.core.rmem_max=26214400
sysctl net.core.wmem_max=26214400
setcap 'cap_net_bind_service=+eip' ./build/deviceserver/DeviceServer
