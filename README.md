# Infinity PSP home screen ported to PS5

## 1. Compile

```bash
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
make
```

This produces `infinity_intro.elf`.

## 2. Copy the payload to the console

Navigate to /data/homebrew/InfinityIntro on the PS5:
and copy infinity_intro.elf
Create a folder named “assets” here: /data/homebrew/InfinityIntro/assets/
and copy these files: `parallaxleft.tga`, `parallaxright.tga`, `glow.png`, `theme.ogg`.

## 3. Run the payload

Launch the websrv payload on the PS5, navigate to the internal storage directory /data/homebrew/InfinityIntro/, and select the ELF file

## Notes
- You can use any audio file as long as it has the .ogg extension and is named `theme.ogg`
- The audio automatically loops back to the beginning when `theme.ogg` ends
- The window is created in full-screen mode at native 1920x1080 resolution; the
  internal rendering remains at 480x272, as on the PSP, and is automatically scaled.
  
## CREDITS
https://github.com/DaveeFTW/Infinity
