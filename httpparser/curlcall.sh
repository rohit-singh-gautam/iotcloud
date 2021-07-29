

curl -H "Authorization: Bearer $(gcloud auth print-identity-token)" 127.0.0.1:8080
curl 127.0.0.1:8080

curl http://[::1]:8060
# -k is insecure
curl -k https://[::1]:8061