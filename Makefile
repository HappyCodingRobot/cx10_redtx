# Hey Emacs, this is a -*- makefile -*-
#
# see: http://ed.am/dev/make/arduino-mk
#
#   SOURCES := main.cc other.cc
#   LIBRARIES := EEPROM
#
#SOURCES := cx10_redtx.ino cx10.cpp
#SOURCES := cx10.cpp
#CPPFLAGS := -std=c++11 -Wa,-aln=OUT.s
#CPPFLAGS := -std=c++11 -Wl,-Map=foo.map
#CPPFLAGS := -std=c++11

LIBRARIES := SPI NRF24 RcTrainer
BOARD := pro5v328
SERIALDEV := /dev/ttyUSB0
SERIALMON := cutecom

# last step:
include ../arduino.mk
