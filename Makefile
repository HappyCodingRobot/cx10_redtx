# Hey Emacs, this is a -*- makefile -*-
#
# see: http://ed.am/dev/make/arduino-mk
#
#   SOURCES := main.cc other.cc
#   LIBRARIES := EEPROM
#
LIBRARIES := SPI NRF24 RcTrainer
BOARD := pro5v328
SERIALDEV := /dev/ttyUSB0
SERIALMON := cutecom

# last step:
include ../arduino.mk
