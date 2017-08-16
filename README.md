# fixkeybag
Tiny utility for generating system keybags. It's needed for iOS dualboots

# Build
1. **Using command line**

`cd .../fixkeybag-master`

`mkdir binary`

`xcrun --sdk iphoneos clang fixkeybag/main.c -arch armv7 -mios-version-min=6.0 -o binary/fixkeybag`

`ldid -S binary/fixkeybag`

2. **Using Xcode**

To compile for iOS versions older than 6.0 use Xcode 7.x or older

# no-effaceable-storage Device Tree patch
This patch is needed to prevent Effaceable Storage's keys regeneration. Example of patched Device Tree for n90 is attached to this repository

# Credits
Original code by jean jean: https://openjailbreak.org/projects/greenpois0n-current/repository/revisions/da30db3ab37289326e46c8c7417502e332dbdcd0
