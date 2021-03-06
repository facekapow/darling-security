/*
 * Copyright (c) 2006-2008,2010,2012,2014 Apple Inc. All Rights Reserved.
 */

#include <Security/vmdh.h>
#include <Security/SecBase64.h>
#include <stdlib.h>
#include <unistd.h>

#include <corecrypto/ccdh.h>

#include "Security_regressions.h"

#if 0
#include <openssl/dh.h>
#endif

/* How to reach in the internals of SecDH/vmdh struct */
static inline ccdh_gp_t vmdh_gp(struct vmdh *dh)
{
    ccdh_gp_t gp;
    gp.gp  = (ccdh_gp *)dh;
    return gp;
}

static inline ccdh_full_ctx_t vmdh_priv(struct vmdh *dh)
{
    void *p = dh;
    cczp_t zp;
    zp.zp = (struct cczp *) dh;
    cc_size s = ccn_sizeof_n(cczp_n(zp));
    ccdh_full_ctx_t priv = { .hdr = (struct ccdh_ctx_header *)(p+ccdh_gp_size(s)) };
    return priv;
}

static uint32_t param_g = 5;

static const uint8_t param_p[] = {
    0xED, 0x42, 0x06, 0xE1, 0xDD, 0x09, 0x93, 0xA6,
    0x81, 0xAE, 0x00, 0x0D, 0xBF, 0x84, 0x7F, 0x7D,
    0x87, 0x64, 0x6B, 0x77, 0x24, 0x03, 0xB8, 0xC0,
    0xDC, 0xBE, 0x5B, 0x9C, 0x8E, 0x71, 0x09, 0x24,
    0x53, 0x77, 0x7F, 0x5D, 0x1A, 0xAD, 0x92, 0xD8,
    0xFE, 0xCD, 0x5C, 0xB4, 0xCA, 0x09, 0x17, 0x11,
    0xF3, 0x82, 0x01, 0x39, 0x4A, 0x09, 0xBA, 0x29,
    0x95, 0x2B, 0xC4, 0xCC, 0x56, 0x21, 0x97, 0x13
};

static const uint8_t client_priv[] = {
    0x32, 0x34, 0x73, 0x16, 0x7d, 0x79, 0xde, 0x47,
    0x22, 0x93, 0xf5, 0x86, 0x47, 0xf6, 0x7f, 0x7a,
    0xb6, 0x30, 0x16, 0x5b, 0xbf, 0xe1, 0x36, 0x0b,
    0xb4, 0xd2, 0x84, 0x3e, 0x57, 0x5f, 0xcb, 0xc6,
    0x6a, 0xae, 0x5d, 0x59, 0x4b, 0x70, 0x53, 0x22,
    0xb0, 0x51, 0x89, 0x30, 0x74, 0xfc, 0x95, 0x51,
    0x9c, 0xc9, 0xf7, 0xac, 0x8c, 0x37, 0xfd, 0xc1,
    0x0e, 0x02, 0x6e, 0x69, 0x6c, 0xca, 0x2a, 0x95
};

__unused static const uint8_t client_pub[] = {
    0x15, 0xDF, 0x17, 0x6E, 0xB4, 0x95, 0xA7, 0x92,
    0x41, 0xB6, 0xF1, 0x93, 0x19, 0xDB, 0x34, 0xF1,
    0xE0, 0x0D, 0x62, 0xCD, 0x55, 0xC7, 0x0B, 0x27,
    0xB7, 0x53, 0x1A, 0x28, 0x65, 0x11, 0xF0, 0xF6,
    0xA6, 0xE1, 0x5B, 0x86, 0x1D, 0x67, 0x85, 0x19,
    0x6D, 0xD6, 0x80, 0xFF, 0x5C, 0xEB, 0xC3, 0x2D,
    0xC3, 0xCB, 0xD2, 0xD4, 0x66, 0x93, 0xF4, 0xFC,
    0xF1, 0xF4, 0x8B, 0x61, 0x0F, 0x02, 0xF5, 0x19
};

static const uint8_t server_pub[] = {
    0x73, 0xC5, 0xF8, 0xF8, 0xB8, 0x9C, 0xB0, 0x5F,
    0xD6, 0xC6, 0x49, 0x5C, 0x70, 0xF5, 0x90, 0xB3,
    0x8A, 0xD3, 0xD0, 0x12, 0x99, 0x47, 0x60, 0xC2,
    0x5B, 0xF7, 0x18, 0x3A, 0x19, 0xF5, 0x01, 0xA3,
    0x67, 0xBF, 0x57, 0x28, 0x7E, 0x99, 0xA8, 0xDB,
    0x97, 0xA4, 0xAF, 0xF2, 0x68, 0x47, 0xAB, 0x48,
    0xE3, 0x4D, 0xF2, 0x94, 0xB4, 0xCC, 0xFC, 0x0C,
    0x50, 0xAD, 0xEF, 0x2E, 0x80, 0xA6, 0x20, 0x29
};

static const uint8_t pw[] = {
	0x31, 0x32, 0x33, 0x34
};

static const uint8_t pw_encr[] = {
    0x42, 0xd7, 0xa1, 0x08, 0x15, 0x8f, 0xdd, 0xc8,
    0xe8, 0x75, 0xea, 0xa2, 0xc2, 0x20, 0x28, 0xfa
};

#if 0
static void hexdump(const uint8_t *bytes, size_t len) {
	size_t ix;
	for (ix = 0; ix < len; ++ix) {
		printf("%02X", bytes[ix]);
	}
	printf("\n");
}
#endif

static void tests(void)
{
    vmdh_t vmdh;
    ok((vmdh = vmdh_create(param_g, param_p, sizeof(param_p),
        NULL, 0)), "vmdh_create");

    /* No SecDH API to import a private key, so we have to reach inside the opaque struct */
    ccdh_gp_t gp = vmdh_gp(vmdh);
    ccdh_full_ctx_t priv = vmdh_priv(vmdh);

    ok_status(ccdh_import_priv(gp, sizeof(client_priv), client_priv, priv), "Import DH private key");

    uint8_t encpw[vmdh_encpw_len(sizeof(pw))];
    size_t encpw_len = sizeof(encpw);

    ok(vmdh_encrypt_password(vmdh, server_pub, sizeof(server_pub),
		pw, sizeof(pw),	encpw, &encpw_len), "vmdh_encrypt_password");
    vmdh_destroy(vmdh);

    is(encpw_len, sizeof(pw_encr), "check ciphertext size");
	ok(!memcmp(encpw, pw_encr, sizeof(pw_encr)), "does ciphertext match?");

#if 0
	hexdump(encpw, encpw_len);
	hexdump(pw_encr, sizeof(pw_encr));

	DH *dh = DH_new();
	dh->p = BN_bin2bn(param_p, sizeof(param_p), BN_new());
	BN_hex2bn(&dh->g, "05");
	dh->pub_key = BN_bin2bn(client_pub, sizeof(client_pub), BN_new());
	dh->priv_key = BN_bin2bn(client_priv, sizeof(client_priv), BN_new());
	BIGNUM *bn_server_pub = BN_bin2bn(server_pub, sizeof(server_pub), BN_new());
	unsigned char key[1024];
	DH_compute_key(key, bn_server_pub, dh);
	printf("shared secret:\n");
	hexdump(key, 16);
#endif
}

int vmdh_42_example2(int argc, char *const *argv)
{
	plan_tests(5);

	tests();

	return 0;
}
