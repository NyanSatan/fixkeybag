//
//  main.m
//  fixkeybag
//
//  Created on 24/03/2017.
//  Copyright © 2017 Nyan Satan. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <strings.h>


bool checkKeybagExistance();
void generateKeybag();
void keybag_downgrade_to_v2(char *key_0x835_string);


void usage(char *argv0) {
    
    printf("usage: %s [-v2] [key 0x835]\n", argv0);
}

int main(int argc, char **argv) {
    
    if (getuid() == 0) {
        
        switch (argc) {
            case 1:
                if (!checkKeybagExistance()) {
                    generateKeybag();
                    
                } else {
                    
                    printf("System keybag already exists, no need to create new\n");
                }

                break;
                
            case 2:
                
                if (strcmp(argv[1], "-h") == 0) {
                    
                    usage(argv[0]);
                    
                }
                
                if (strcmp(argv[1], "-v2") == 0) {
                    
                    keybag_downgrade_to_v2(NULL);
                    
                } else {
                    
                    usage(argv[0]);
                }

                break;
                
            case 3:
                
                if (strcmp(argv[1], "-v2") == 0) {
                    
                    keybag_downgrade_to_v2(argv[2]);
                    
                } else {
                    
                    usage(argv[0]);
                }
                
                break;
                
            default:
                usage(argv[0]);
                break;
        }
        
    } else {
        
        printf("This must be run as root!\n");
    }
    
    return 0;
}
