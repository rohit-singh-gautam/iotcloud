


function create_rsa() {
    NUMBIT=$1
    FILE_TYPE=$2
    if [ -z ${FILE_TYPE+x} ]; then
        FILE_TYPE=pem;
    fi

    FILE_TYPE=`echo $FILE_TYPE | tr [A-Z] [a-z]`
    FORM_TYPE=`echo $FILE_TYPE | tr [a-z] [A-Z]`

    PRIVATE_KEY=rsa/rsa_keypair_bit$1.$FILE_TYPE
    PUBLIC_KEY=rsa/rsa_public_bit$1.$FILE_TYPE

    openssl genrsa -des3 -outform $FORM_TYPE -out $PRIVATE_KEY $NUMBIT
    openssl rsa -in $PRIVATE_KEY -inform $FORM_TYPE -outform $FORM_TYPE -RSAPublicKey_out -out $PUBLIC_KEY
}

function create_ec() {
    # To get "-name" use "openssl ecparam -list_curves"
    NUMBIT=$1
    FILE_TYPE=$2
    if [ -z ${FILE_TYPE+x} ]; then
        FILE_TYPE=pem;
    fi

    FILE_TYPE=`echo $FILE_TYPE | tr [A-Z] [a-z]`
    FORM_TYPE=`echo $FILE_TYPE | tr [a-z] [A-Z]`
    
    PRIVATE_KEY=ec/ec_keypair_bit$1.$FILE_TYPE
    PUBLIC_KEY=ec/ec_public_bit$1.$FILE_TYPE
    
    case $NUMBIT in
        112)
            CURVE_NAME=secp112r2
            ;;
        128)
            CURVE_NAME=secp128r1
            ;;
        160)
            CURVE_NAME=secp160r2
            ;;
        192)
            CURVE_NAME=prime192v1
            ;;
        256)
            CURVE_NAME=prime256v1
            ;;
        384)
            CURVE_NAME=secp384r1
            ;;
        512)
            CURVE_NAME=secp521r1
            ;;
        *)
            echo "Unsupported bit, please provide one of 112, 128, 160, 192, 256, 384, 512"
            ;;
    esac

    openssl ecparam -genkey -name $CURVE_NAME -noout -outform $FORM_TYPE -out $PRIVATE_KEY
    openssl ec -in $PRIVATE_KEY -inform $FORM_TYPE -outform $FORM_TYPE -pubout -out $PUBLIC_KEY
}

mkdir -p rsa
mkdir -p ec

#create_rsa 512
#create_rsa 1024
#create_rsa 2048
#create_rsa 4096

#create_ec 112
#create_ec 128
#create_ec 160
#create_ec 192
#create_ec 256
#create_ec 384
#create_ec 512

