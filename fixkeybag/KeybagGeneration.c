//
//  KeybagGeneration.c
//  fixkeybag
//
//  Created by noname on 04/11/17.
//  Copyright (c) 2017 Nyan Satan. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

bool checkKeybagExistance() {
    
    int fd = open("/private/var/keybags/systembag.kb", O_RDONLY, 0);
    
    if (fd > -1) {
        
        close(fd);
        return true;
        
    } else {
        close(fd);
        return false;
    }
    
}

void generateKeybag() {
    
    printf("Generating keybag...\n");
    
    int (*MKBKeyBagCreateSystem)(int x, char* path);
    
    void *handle = dlopen("/System/Library/PrivateFrameworks/MobileKeyBag.framework/MobileKeyBag", RTLD_LAZY);
    MKBKeyBagCreateSystem = dlsym(handle, "MKBKeyBagCreateSystem");
    MKBKeyBagCreateSystem(0, "/private/var");
    
    if (checkKeybagExistance()) {
        
        printf("No worries, \"can't set the system bag\" message is OK\n");
        
    } else {
        printf("Something went wrong...\n");
    }
}