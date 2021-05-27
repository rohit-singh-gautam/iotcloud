

curl -H "Authorization: Bearer $(gcloud auth print-identity-token)" 127.0.0.1:8080
curl 127.0.0.1:8080