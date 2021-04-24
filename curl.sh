
export SERVICE_URL=https://iotcloud-7efrsvjdsa-de.a.run.app
export SERVICE_URL=iotcloud.connecteddevice.in
curl -H "Authorization: Bearer $(gcloud auth print-identity-token)"   "${SERVICE_URL}"