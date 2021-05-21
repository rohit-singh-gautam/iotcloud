
set -eu

if [[ -z "${GOOGLE_CLOUD_PROJECT:-}" ]]; then
  echo "You must set GOOGLE_CLOUD_PROJECT to the project id hosting Cloud Run C++ Hello World"
  exit 1
fi

readonly GOOGLE_CLOUD_PROJECT="${GOOGLE_CLOUD_PROJECT:-}"
readonly GOOGLE_CLOUD_REGION="${REGION:-asia-east1}"

# Create a service account that will update the index
readonly SA_ID="iotcloud"
readonly SA_NAME="${SA_ID}@${GOOGLE_CLOUD_PROJECT}.iam.gserviceaccount.com"
if gcloud iam service-accounts describe "${SA_NAME}" \
     "--project=${GOOGLE_CLOUD_PROJECT}" >/dev/null 2>&1; then
  echo "The ${SA_ID} service account already exists"
else
  gcloud iam service-accounts create "${SA_ID}" \
      "--project=${GOOGLE_CLOUD_PROJECT}" \
      --description="Go Hello World for Cloud Run"
fi

#gcloud builds submit --tag gcr.io/${GOOGLE_CLOUD_PROJECT}/iotcloud
gcloud beta run deploy --use-http2 \
    --image=gcr.io/${GOOGLE_CLOUD_PROJECT}/iotcloud:latest \
    --project=${GOOGLE_CLOUD_PROJECT} \
    --region=${GOOGLE_CLOUD_REGION} \
    --service-account=${SA_NAME} \
    --platform managed \
    --no-allow-unauthenticated
