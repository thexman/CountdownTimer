# CountdownTimer
ESP32 Countdown timer that controls an relay.

1. Enter time (00:00). First the right most digit is enter, and the text is shifted left.
1. Press A on the keypad to start/stop the countdown.
1. When timer is active the relay is closed.
1. When the time reaches 0 or A is pressed the relay is open.


# Wiring
* Keyboard row pins: 13, 12, 14, 27
* Keyboard column pins: 26, 25, 33, 32
* Relay pins: 15, 2, 4