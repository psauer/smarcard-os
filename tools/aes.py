#!/usr/bin/env python2

import numpy as np
from Crypto.Cipher import AES
from Crypto.Util.number import bytes_to_long

if __name__ == '__main__':
    key = np.array([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], np.uint8)
    ciphertext = np.array([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], np.uint8)

    aes = AES.new(key, AES.MODE_ECB)
    plaintext = aes.decrypt(ciphertext)

    print "plaintext: ", hex(bytes_to_long(plaintext))
    print "key: ", hex(bytes_to_long(key))
    print "ciphertext: ", hex(bytes_to_long(ciphertext))
