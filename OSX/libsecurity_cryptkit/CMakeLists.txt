add_definitions(
	-DCK_SECURITY_BUILD
)

add_darling_static_library(security_cryptkit FAT
	SOURCES
		lib/CryptKitAsn1.cpp
		lib/CryptKitDER.cpp
		lib/HmacSha1Legacy.c
		lib/byteRep.c
		lib/ckSHA1.c
		lib/ckutilities.c
		lib/curveParams.c
		lib/elliptic.c
		lib/ellipticProj.c
		lib/enc64.c
		lib/engineNSA127.c
		lib/falloc.c
		lib/feeDES.c
		lib/feeDigitalSignature.c
		lib/feeECDSA.c
		lib/feeFEED.c
		lib/feeFEEDExp.c
		lib/feeHash.c
		lib/feePublicKey.c
		lib/feeRandom.c
		lib/giantIntegers.c
		lib/giantPort_PPC.c
		lib/giantPort_PPC_Gnu.S
		lib/platform.c
)
