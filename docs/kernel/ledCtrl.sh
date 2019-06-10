#!/bin/sh
# script in busybox, bash must be sh

LED_RED=/sys/class/leds/red
LED_GREEN=/sys/class/leds/green
LED_BLUE=/sys/class/leds/blue

LED_MMC=/sys/class/leds/mmc0\:\:/

BRIGHTNESS=brightness
TRIGGER=trigger

LedOn() {
	printf "%b" '\x31' > $1/$BRIGHTNESS
}

LedOff() {
	printf "%b" '\x30' > $1/$BRIGHTNESS
}


LedBlinkHeartbeat() {
	printf "heartbeat" > $1/$TRIGGER
}

LedBlinkOn() {
	printf "timer" > $1/$TRIGGER
}

LedBlinkOff() {
	printf "none" > $1/$TRIGGER
}

sleep 5

echo "disable default led BLUE blink..."
LedOff $LED_BLUE

# All ON
sleep 2
echo "Led GREEN on..."
LedOn $LED_GREEN
sleep 5

echo "Led RED on..."
LedOn $LED_RED
sleep 5

echo "Led BLUE on..."
LedOn $LED_BLUE
sleep 5


# All off
echo "Led GREEN off..."
LedOff $LED_GREEN
sleep 5

echo "Led RED off..."
LedOff $LED_RED
sleep 5

echo "Led BLUE off..."
LedOff $LED_BLUE
sleep 5


# All blink
echo "Led GREEN blink..."
LedBlinkOn $LED_GREEN
sleep 5

echo "Led RED blink..."
LedBlinkOn $LED_RED
sleep 5

echo "Led BLUE blink..."
LedBlinkOn $LED_BLUE
sleep 5


# All blink off
echo "Led GREEN blink off..."
LedBlinkOff $LED_GREEN
sleep 5

echo "Led RED blink off..."
LedBlinkOff $LED_RED
sleep 5

echo "Led BLUE blink off..."
LedBlinkOff $LED_BLUE
sleep 5

