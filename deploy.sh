
gcloud builds submit --tag gcr.io/iotcloud-singh-org-in/iotcloud
gcloud run deploy --image gcr.io/iotcloud-singh-org-in/iotcloud --platform managed
