/*  Cheerson CX-10 (red) arduino PPM TX

    This code uses the NRF24 and RCTrainer Arduino libraries (GPLv2),
    redistributed in full with original licensing details. See author's
    web page for more details:

    http://www.airspayce.com/mikem/arduino/NRF24/
    http://www.airspayce.com/mikem/arduino/RcTrainer/

    The YD717 (Skywalker) protocol, and Beken 2423 initialisation
    routines were derived from the DeviationTX project (GPLv3), in
    particular the source file yd717_nrf24l01.c from:

    https://bitbucket.org/PhracturedBlue/

    All original components are provided under the GPLv3:

    Copyright (C) 2015, Samuel Powell.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <RcTrainer.h>
#include <SPI.h>
#include <NRF24.h>
#include "redtx.h"
#include "cx10.h"

// Function prototypes
//void send_packet(bool);
//void write_payload(uint8_t *data, uint8_t len, bool noack);
//void set_cmmd_addr(void);
//void set_bind_addr(void);
//int packwait(void);

// Radio and register defines
//#define RF_CHANNEL      0x3C  // Stock TX fixed frequency
//#define ACTIVATE        0x50  // Activate command
//#define PAYLOADSIZE     9

//#define ACTIVATE_DATA       0x73
//#define ACTIVATE_CMD        0x50

//#define NRF24_ENAA_PA (NRF24_ENAA_P0 | NRF24_ENAA_P1 | NRF24_ENAA_P2 | NRF24_ENAA_P3 | NRF24_ENAA_P4 | NRF24_ENAA_P5)
//#define NRF24_ERX_PA (NRF24_ERX_P0 | NRF24_ERX_P1 | NRF24_ERX_P2 | NRF24_ERX_P3 | NRF24_ERX_P4 | NRF24_ERX_P5)
//#define NRF_STATUS_CLEAR 0x70

// Channel scaling defines
#define CHAN_MAX_VALUE 1000
#define CHAN_MIN_VALUE -1000
#define chval(chin) (uint8_t) (((chin * 0xFF / CHAN_MAX_VALUE) + 0x100) >> 1)
 
// Packet state enumeration
//enum packet_state_t {
//     PKT_PENDING = 0,
//     PKT_ACK,
//     PKT_TIMEOUT,
//     PKT_ERROR,
//     PKT_ERROR_IN_RX
// };

// Singleton instance of the radio and PPM receiver
NRF24 nrf24;
CX10 cx10(nrf24);
RcTrainer tx;


// Command and bind addresses (command address should be generated from random number)
//uint8_t rx_tx_cmmd[5] = {0xC1, 0xC1, 0xC1, 0xC1, 0xC1};
//uint8_t rx_tx_bind[5] = {0x65, 0x65, 0x65, 0x65, 0x65};

// Data packet buffer
//uint8_t packet[PAYLOADSIZE];

// CX-10 scaled commands
cmds_t cmd; 
//uint8_t throttle, rudder, elevator, aileron, rudder_trim, elevator_trim, aileron_trim, flags;

// setup initalises nrf24, attempts to bind, then moves on
void setup() {
  Serial.begin(57600);
  Serial.println("# red-TX setup .. ");
  
  cx10.init();
  
  // Wait for aux1 high before binding ### DEBUG ###
  Serial.println("Waiting for ch5 < threshold and no throttle ..");
  
  while( (tx.getChannel(5, 1000, 2000, 0x00, 0xFF) < 0x40) || (tx.getChannel(0, 1000, 2000, 0x00, 0xFF) != 0) );
  Serial.println("start binding..");
  
  cx10.start_binding();
}


// loop repeatedly sends data read by PPM to the device, every 8ms
void loop() {
  uint8_t aux1 = 0;     // AIL switch
  uint8_t aux2 = 0;     // TRN switch
  
  // Get RX values by PPM, convert to range 0x00 to 0xFF
  cmd.throttle = (uint8_t) (tx.getChannel(0, 1000, 2000, 0x00, 0xFF ));
  cmd.aileron  = (uint8_t) (tx.getChannel(1, 1000, 2000, 0x00, 0xFF ));
  cmd.elevator = (uint8_t) (tx.getChannel(2, 1000, 2000, 0x00, 0xFF ));
  cmd.rudder   = (uint8_t) (tx.getChannel(3, 1000, 2000, 0x00, 0xFF ));
  aux1        = (uint8_t) (tx.getChannel(4, 1000, 2000, 0x00, 0xFF ));
  aux2        = (uint8_t) (tx.getChannel(5, 1000, 2000, 0x00, 0xFF ));
  
  // Add command values to trim to get real full scale response 
  // in original CX-10 firmware (FN firmware ignores the trims, so
  // no problems).
  //rudder_trim = rudder >> 1;
  //aileron_trim = aileron >> 1;
  //elevator_trim = elevator >> 1;
  cmd.rate = 1;
  
  // If the AUX1 is high, we set the flags, allowing flips in original
  // firmware, or arming (via elevator) in FN firmware
  if(aux1 > 0x80) {
    cmd.flags = 0x0F;      
  } else {
    cmd.flags = 0x00;
  }
  
  cx10.setValues(&cmd);
  
  // Send a data packet and find out what happens
  cx10.send_packet(false);
  
  // Wait for 8ms, before sending next data
  delay(8);
  //delay(cx10.get_pack_time());
}




