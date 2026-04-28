# Pebble Example Apps
A bunch of example apps built by the Pebble team to demonstrate how to use the new APIs and features of Pebble. These apps are not meant to be used as-is, but rather as a starting point for your own apps.

## List of Apps

- **[rgb-backlight-thing](rgb-backlight-thing/)** — Demonstrates the RGB backlight API (`light_set_color`) on Pebble Time 2. UP/SELECT/DOWN set the backlight to red/green/blue.
- **[touch-thing](touch-thing/)** — Demonstrates the touch service API on Pebble Time 2 and Pebble Round 2. Subscribes to `touch_service` and visualizes touchdown, liftoff, and position-update events with live counters and a crosshair at the touch point.
- **[speaker/note-player-thing](speaker/note-player-thing/)** — Plays four melodies (Ode to Joy, Twinkle Twinkle, Super Mario, Für Elise) using `speaker_play_notes`, each with a different waveform (sine, square, triangle, sawtooth). UP/DOWN cycle melodies, SELECT plays.
- **[speaker/note-sample-thing](speaker/note-sample-thing/)** — Plays a C major scale using a PCM sample resource pitched up/down via `speaker_play_tracks` with a `SpeakerSample`.
- **[speaker/pcm-resource-thing](speaker/pcm-resource-thing/)** — Streams a raw PCM audio resource through `speaker_stream_open` / `speaker_stream_write`, refilling the ring buffer from an `AppTimer` and showing playback progress.
- **[speaker/polyphonic-thing](speaker/polyphonic-thing/)** — Plays a 3-voice arrangement of Ode to Joy (soprano sawtooth, alto sine, bass triangle) using `speaker_play_tracks` with multiple concurrent tracks.

## Credits

`speaker/pcm-resource-thing` uses *Digital Lemonade* by Kevin MacLeod ([incompetech.com](https://incompetech.com)), licensed under [Creative Commons: By Attribution 4.0](https://creativecommons.org/licenses/by/4.0/).
