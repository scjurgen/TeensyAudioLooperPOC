# TeensyAudioLooperPOC
4channel Audio Looper with Teensy 3.6

Just a proof of concept. And it works :)

you need some raw audio files on the SD card and name them accordingly.

Using sox you can generate them from any audio file i.e.:

```
sox mySuperFile.mp3 -e signed-int -b 16 -r 44100 -c 1 P_0001.RAW
```


