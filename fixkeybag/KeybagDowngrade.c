//
//  KeybagDowngrade.c
//  fixkeybag
//
//  Created by noname on 04/11/17.
//  Copyright (c) 2017 Nyan Satan. All rights reserved.
//

#include <stdio.h>
#include <sys/stat.h>
#include <CoreFoundation/CoreFoundation.h>
#include "AppleEffaceableStorage.h"
#include "key_wrap.h"
#include "IOAESAccelerator.h"
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonHMAC.h>


int keybag_downgrade_to_v2(char *key_0x835_string) {
    
    printf("Based on iphone-dataprotection code\n");
    
    char *keybag_path = "/private/var/keybags/systembag.kb";
    
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                           (const UInt8*)keybag_path,
                                                           strlen(keybag_path),
                                                           0);
    
    if (url == NULL) {
        printf("CFURLCreateFromFileSystemRepresentation() failed!\n");
        return -1;
    }
    
    
    CFReadStreamRef stream = CFReadStreamCreateWithFile(kCFAllocatorDefault, url);
    
    if (stream == NULL) {
        printf("CFReadStreamCreateWithFile() failed!\n");
        return -1;
    }
    
    if (CFReadStreamOpen(stream) != TRUE) {
        printf("CFReadStreamOpen() failed!\n");
        return -1;
    }
    
    
    CFPropertyListRef plist = CFPropertyListCreateWithStream(kCFAllocatorDefault,
                                                               stream,
                                                               0,
                                                               kCFPropertyListImmutable,
                                                               NULL,
                                                               NULL
                                                               );
    
    if (plist == NULL) {
        printf("CFPropertyListCreateWithStream() failed!\n");
        return -1;
    }
    
    CFDataRef data = CFDictionaryGetValue(plist, CFSTR("_MKBPAYLOAD"));
    
    if (data == NULL) {
        printf("CFDictionaryGetValue() failed!\n");
        return -1;
    }
    
    uint8_t* mkbpayload = valloc(CFDataGetLength(data));
    CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), mkbpayload);
    int length = CFDataGetLength(data);
    
    if (length < 16) {
        printf("_MKBPAYLOAD too small!\n");
        free(mkbpayload);
        return -1;
    }
    
    struct BAG1Locker bag1_locker={0};
    
    if (AppleEffaceableStorage__getLocker(LOCKER_BAG1, (uint8_t*) &bag1_locker, sizeof(struct BAG1Locker))) {
        free(mkbpayload);
        return -1;
    }
    
    if (bag1_locker.magic != 'BAG1')
        printf("AppleKeyStore_loadKeyBag: bad BAG1 magic!\n");
    
    size_t decryptedSize = 0;
    
    CCCryptorStatus cryptStatus = CCCrypt(kCCDecrypt,
                                          kCCAlgorithmAES128,
                                          kCCOptionPKCS7Padding,
                                          bag1_locker.key,
                                          kCCKeySizeAES256,
                                          bag1_locker.iv,
                                          mkbpayload,
                                          length,
                                          mkbpayload,
                                          length,
                                          &decryptedSize);
    
    if (cryptStatus != kCCSuccess)
    {
        printf("AppleKeyStore_loadKeyBag CCCrypt kCCDecrypt with BAG1 key failed, return code=%x\n", cryptStatus);
        free(mkbpayload);
        return -1;
    }
    
    
    CFDataRef mkbpayload_decrypted_data = CFDataCreate(kCFAllocatorDefault, mkbpayload, decryptedSize);
    
    if (mkbpayload_decrypted_data == NULL) {
        printf("CFDataCreate() failed!\n");
        return -1;
    }
    
    CFPropertyListRef mkbpayload_plist = CFPropertyListCreateWithData(kCFAllocatorDefault, mkbpayload_decrypted_data, kCFPropertyListMutableContainersAndLeaves, NULL, NULL);
    
    if (mkbpayload_plist == NULL) {
        printf("CFPropertyListCreateWithData() failed!\n");
        return -1;
    }
    
    CFDataRef raw_keys_data = CFDictionaryGetValue(mkbpayload_plist, CFSTR("KeyBagKeys"));
    size_t raw_keys_length = CFDataGetLength(raw_keys_data);
    uint8_t* raw_keys = valloc(raw_keys_length);
    CFDataGetBytes(raw_keys_data, CFRangeMake(0, raw_keys_length), raw_keys);
    
    
    
    
    uint8_t* new_raw_keys = malloc(8192);
    
    uint32_t offset = 8;
    uint32_t new_offset = 8;
    uint8_t data_tag[] = {0x44, 0x41, 0x54, 0x41, 0x00, 0x00, 0x00, 0x00};
    memmove(&new_raw_keys[0], data_tag, offset);
    
    uint32_t sign_tag_length = 8;
    
    while (offset < raw_keys_length) {
        
        uint32_t tag = CFSwapInt32BigToHost(((uint32_t*)raw_keys+offset/4)[0]);
        uint32_t tag_length = CFSwapInt32BigToHost(((uint32_t*)raw_keys+offset/4)[1]);
        
        if (tag == 'VERS' ||
            tag == 'TYPE' ||
            tag == 'UUID' ||
            tag == 'HMCK' ||
            tag == 'WRAP' ||
            tag == 'SALT' ||
            tag == 'ITER' ||
            tag == 'CLAS' ||
            tag == 'WPKY' ||
            tag == 'SIGN') {
            
            if (tag == 'SIGN') {
                sign_tag_length += tag_length;
            }
            
            printf("Writing %s tag of length 0x%x at offset 0x%x\n", raw_keys+offset, tag_length, offset);
            memmove(&new_raw_keys[new_offset], ((uint32_t*)raw_keys+offset/4), tag_length+8);
            new_offset += tag_length+8;
            
            
        } else {
            
            printf("Skipping %s tag of length 0x%x at offset 0x%x\n", raw_keys+offset, tag_length, offset);
        }

        offset += tag_length+8;
    }
    
    uint32_t new_data_tag_length = new_offset-sign_tag_length-8;
    printf("New DATA tag length is 0x%x\n", new_data_tag_length);
    uint32_t new_data_tag_length_big_endian = CFSwapInt32BigToHost(new_data_tag_length);
    memmove(&new_raw_keys[4], &new_data_tag_length_big_endian, 4);

    uint32_t new_vers_tag_value = 0x2;
    printf("New VERS tag value is 0x%x\n", new_vers_tag_value);
    new_vers_tag_value = CFSwapInt32BigToHost(new_vers_tag_value);
    memmove(&new_raw_keys[16], &new_vers_tag_value, 4);
    
    
    printf("Fixing SIGN HMAC\n");
    
    uint8_t *key_0x835 = malloc(16);
    memset(key_0x835, 0x0, 16);
    
    if (key_0x835_string == NULL) {
        
        key_0x835 = IOAES_key835();
        
    } else {
        
        for (int i = 0; i < 32; i = i+2) {
            
            char buffer[3];
            buffer[0] = key_0x835_string[i];
            buffer[1] = key_0x835_string[i+1];
            buffer[2] = 0x0;
            key_0x835[i/2] = (uint8_t)strtol(buffer, NULL, 16);
            
        }

    }
    
    printf("Key 0x835 is ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", key_0x835[i]);
    }
    printf("\n");
    
    aes_key_wrap_ctx ctx;
    uint8_t hmckkey[32] = {0};
    aes_key_wrap_set_key(&ctx, key_0x835, 16);
    aes_key_unwrap(&ctx, &new_raw_keys[64], hmckkey, 4);
    
    CCHmac(kCCHmacAlgSHA1,
           (const void *)&new_raw_keys[8],
           new_data_tag_length,
           hmckkey,
           sizeof(hmckkey),
           &new_raw_keys[new_offset-20]);
    
   
    CFDataRef new_raw_keys_data = CFDataCreate(kCFAllocatorDefault, new_raw_keys, new_offset);
    
    CFMutableDictionaryRef new_mkbpayload_plist = CFDictionaryCreateMutable(kCFAllocatorDefault, 2, NULL, NULL);
    CFDictionaryAddValue(new_mkbpayload_plist, CFSTR("KeyBagKeys"), new_raw_keys_data);
    CFDictionaryAddValue(new_mkbpayload_plist, CFSTR("KeyBagVersion"), CFSTR("1"));
    
    CFDataRef new_mkbpayload_binary_plist = CFPropertyListCreateData(kCFAllocatorDefault, new_mkbpayload_plist, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
    uint8_t *new_mkbpayload_binary_plist_bytes = valloc(CFDataGetLength(new_mkbpayload_binary_plist));
    CFDataGetBytes(new_mkbpayload_binary_plist, CFRangeMake(0, CFDataGetLength(new_mkbpayload_binary_plist)), new_mkbpayload_binary_plist_bytes);
    
    int new_mkbpayload_binary_plist_length = CFDataGetLength(new_mkbpayload_binary_plist);
    
    size_t encryptedSize = 0;
    
    
    
    CCCryptorStatus cryptStatus2 = CCCrypt(kCCEncrypt,
                                          kCCAlgorithmAES128,
                                          kCCOptionPKCS7Padding,
                                          bag1_locker.key,
                                          kCCKeySizeAES256,
                                          bag1_locker.iv,
                                          new_mkbpayload_binary_plist_bytes,
                                          new_mkbpayload_binary_plist_length,
                                          NULL,
                                          0,
                                          &encryptedSize);
    
    printf("Encrypted size is 0x%lx\n", encryptedSize);
    
    uint8_t *enc_out = valloc(encryptedSize);
    
    cryptStatus2 = CCCrypt(kCCEncrypt,
                           kCCAlgorithmAES128,
                           kCCOptionPKCS7Padding,
                           bag1_locker.key,
                           kCCKeySizeAES256,
                           bag1_locker.iv,
                           new_mkbpayload_binary_plist_bytes,
                           new_mkbpayload_binary_plist_length,
                           enc_out,
                           encryptedSize,
                           &encryptedSize);
    
    if (cryptStatus2 != kCCSuccess)
    {
        printf("AppleKeyStore_loadKeyBag CCCrypt kCCEncrypt with BAG1 key failed, return code=%d\n", cryptStatus2);
        free(new_mkbpayload_binary_plist_bytes);
        return -1;
    }

    
    
    CFDataRef new_mkbpayload_encrypted = CFDataCreate(kCFAllocatorDefault, enc_out, encryptedSize);
    
    CFMutableDictionaryRef result_dict = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 3, plist);
    CFDictionaryReplaceValue(result_dict, CFSTR("_MKBPAYLOAD"), new_mkbpayload_encrypted);
    
    CFDataRef result_data = CFPropertyListCreateData(kCFAllocatorDefault, result_dict, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
    
    char *result_string = "/tmp/systembag.kb";
    
    CFURLRef result_url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                           (const UInt8*)result_string,
                                                           strlen(result_string),
                                                           0);
    
    if (CFURLWriteDataAndPropertiesToResource(result_url, result_data, 0, NULL)) {
        
        printf("Done. Downgraded keybag written to %s\n", result_string);
    } else {
        printf("There is something wrong...\n");
    }
    
    
    return 0;
}
