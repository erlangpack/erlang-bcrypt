/*
 * Copyright (c) 2009 Hunter Morris <huntermorris@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "erl_nif.h"
#include "erl_blf.h"

typedef unsigned char byte;

char *bcrypt(const char *, const char *);
void encode_salt(char *, u_int8_t *, u_int16_t, u_int8_t);

static ERL_NIF_TERM
erl_encode_salt(ErlNifEnv* env, ERL_NIF_TERM csalt_term, ERL_NIF_TERM log_rounds_term)
{
    ErlNifBinary csalt, bin;
    unsigned long log_rounds;

    if (!enif_inspect_binary(env, csalt_term, &csalt) || 16 != csalt.size) {
        return enif_make_badarg(env);
    }

    if (!enif_get_ulong(env, log_rounds_term, &log_rounds)) {
        enif_release_binary(env, &csalt);
        return enif_make_badarg(env);
    }

    if (!enif_alloc_binary(env, 64, &bin)) {
        enif_release_binary(env, &csalt);
        return enif_make_badarg(env);
    }

    encode_salt((char *)bin.data, (u_int8_t*)csalt.data, csalt.size, log_rounds);
    enif_release_binary(env, &csalt);
    return enif_make_binary(env, &bin);
}

static ERL_NIF_TERM
hashpw(ErlNifEnv* env, ERL_NIF_TERM pass_term, ERL_NIF_TERM salt_term)
{
    ErlNifBinary pass, salt;
    char *ret = NULL;

    if (!enif_inspect_binary(env, pass_term, &pass)) {
        return enif_make_badarg(env);
    }

    if (!enif_inspect_binary(env, salt_term, &salt)) {
        enif_release_binary(env, &pass);
        return enif_make_badarg(env);
    }

    char pass_data[pass.size + 1];
    pass_data[pass.size] = '\0';
    char salt_data[salt.size + 1];
    salt_data[salt.size] = '\0';

    if (NULL == (ret = bcrypt(pass_data, salt_data)) || 0 == strcmp(ret, ":")) {
        return enif_make_badarg(env);
    }

    enif_release_binary(env, &pass);
    enif_release_binary(env, &salt);
    return enif_make_string(env, ret);
}

static ErlNifFunc bcrypt_nif_funcs[] =
{
    {"encode_salt", 2, erl_encode_salt},
    {"hashpw", 2, hashpw}
};

ERL_NIF_INIT(bcrypt_nif, bcrypt_nif_funcs, NULL, NULL, NULL, NULL)