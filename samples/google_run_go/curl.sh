
export SERVICE_URL=https://iotcloud-7efrsvjdsa-de.a.run.app
export SERVICE_URL=https://iotcloud.connecteddevice.in
curl -v --http2-prior-knowledge -H "Authorization: Bearer $(gcloud auth print-identity-token)" "${SERVICE_URL}"
curl -v --http2-prior-knowledge "${SERVICE_URL}"