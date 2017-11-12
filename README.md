# fixkeybag
Tiny utility for generating and downgrading system keybags. It's needed for iOS dualboots

# Usage
`fixkeybag [-v2] [key 0x835]`
For more information read my [iOS dualboot guide](https://nyansatan.github.io/dualboot/ios4dualboot.html)

# Downgrade feature
It's very experimental, be extremely careful with it

# Build
1. **Using command line**

`cd .../fixkeybag-master`

`mkdir binary`

`xcrun --sdk iphoneos clang fixkeybag/main.c fixkeybag/KeybagDowngrade.c fixkeybag/KeybagGeneration.c fixkeybag/iphone-dataprotection/AppleEffaceableStorage.c fixkeybag/iphone-dataprotection/IOAESAccelerator.c fixkeybag/iphone-dataprotection/IOKit.c fixkeybag/iphone-dataprotection/bsdcrypto/key_wrap.c fixkeybag/iphone-dataprotection/bsdcrypto/rijndael.c -Ifixkeybag/iphone-dataprotection -Ifixkeybag/iphone-dataprotection/bsdcrypto -IHeaders -framework IOKit -framework CoreFoundation -arch armv7 -mios-version-min=4.0 -o binary/fixkeybag`

`ldid -S binary/fixkeybag`

2. **Using Xcode**

To compile for iOS versions older than 6.0 use Xcode 7.x or older

# no-effaceable-storage Device Tree patch
This patch is needed to prevent Effaceable Storage's keys regeneration. Example of patched Device Tree for n90 is attached to this repository

# Credits
[Original code by jean jean](https://openjailbreak.org/projects/greenpois0n-current/repository/revisions/da30db3ab37289326e46c8c7417502e332dbdcd0) - there's nothing left of it in my program (except name), but I think I still should mention it

Downgrading part is based on [iphone-dataprotection](https://code.google.com/archive/p/iphone-dataprotection/) project
