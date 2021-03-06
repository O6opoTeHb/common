#include "converter.h"

using namespace std;

uint8_t storage_to_byte(storage_p ps) {
    return ps->at(0);
}

uint16_t storage_to_word(storage_p ps) {
    return (ps->at(1) << 8) | ps->at(0);
};

uint32_t storage_to_dword(storage_p ps) {
    return (ps->at(3) << 24) | ps->at(2) << 16 | (ps->at(1) << 8) | ps->at(0);
};

void dword_to_storage(uint32_t dw, storage_p ps) {
    for (int i = 0; i<4; ++i) {
        ps->at(i) = dw & 0xFF;
        dw = dw >> 8;
    }
};

// for microcin - take nearby hex bytes and keep only two digits in resulting bin byte
storage_p hex_to_bin(storage_p hex) {
    storage r;
    storage_it it = hex->begin();
    while (it != hex->end()) {
        r.push_back(((0x10 * (*it)) & 0xFF) | ((*next(it)) & 0x0F));
        if (it != hex->end()) {
            it = next(it);
        }
        if (it != hex->end()) {
            it = next(it);
        }
    }
    return make_shared<storage>(r);
}

void dexor(storage_p enc, storage_p key_it, int k_len) {
    for (int i=0; i < enc->size(); ++i) {
        enc->at(i) = enc->at(i) ^ key_it->at(i % k_len);
    }
}

//TODO move to corresponding modules
void dexor_lucky_mouse(storage_p enc, storage_p key) {
    size_t key_len = key->size();
    for (int i=0; i < enc->size(); ++i) {
        enc->at(i) = enc->at(i) ^ key->at(i % key_len);
        if (i != 0 && (i+1) % key_len == 0) {
            // key scheduling round - convert to dword and sum with decrypted
            uint32_t dw_curr_key = storage_to_dword(key);
            storage st(enc->begin()+i-3, enc->begin()+i+1);
            uint32_t dw_dec = storage_to_dword(make_shared<storage>(st));
            uint32_t dwt = dw_curr_key + dw_dec;
            dword_to_storage(dwt, key);
        }
    }
};

void dexor_montys_three(storage_p enc, storage_p key) {
    for (int i=0; i < enc->size(); ++i) {
        enc->at(i) = enc->at(i) ^ key->at(i & 3);
        if (i && !(i & 3)) {
            // key scheduling round - convert to dword and rotate
            uint32_t dw_curr_key = storage_to_dword(key);
            dw_curr_key ^= 8 * (dw_curr_key ^ (dw_curr_key << 20));
            dword_to_storage(dw_curr_key, key);
        }
    }
};
