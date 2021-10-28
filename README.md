# MP3-STM32F746G-DISCO
Improve mp3 demo of the STM32F746G-DISCO.
This is a version that i use to debug WM8994 driver and the HAL.
It took only few hour to fix this demo.
Fix the glitch when player change song, or when navigating song.
Added a 5 band equalizer.

Just copy the file where they need to be...

Available at:
https://www.st.com/en/embedded-software/x-cube-audio.html

 NOTE: For a better experienced with this board, a new version far more interesting than this are in the pipe.

  - Better graphics
  - FFT graphic analyzer
  - Cover support
  - Randomization of playback
  - Equalizer setting for each song.
  - playlist
  - Support song name and artist from TAG instead of the filename in joliet 8.3 old format.
  - Support file without TAG (ex. .wav) with cover, song name and artist name.
    
  TO DO fix start glitch for first song or change from different sampling.
  
