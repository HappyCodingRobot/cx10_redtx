/**
 * 
 * 
 * 
 */

#include "cx10.h"


uint8_t CX10::rx_tx_cmmd[5] = {0xC1, 0xC1, 0xC1, 0xC1, 0xC1};
uint8_t CX10::rx_tx_bind[5] = {0x65, 0x65, 0x65, 0x65, 0x65};


//CX10::CX10(NRF24& _radio): radio(_radio), initialized(false), cmmd(0) {
CX10::CX10(NRF24& _radio): radio(_radio), initialized(false) {
}

//CX10::~CX10() {
//  delete packet;
//}

/**
 * 
 */
void CX10::init(void) {
  if (!initialized) {
    initialized = true;
    // Initialise SPI bus and activate radio in RX mode
    radio.init();
    radio.setConfiguration( NRF24_EN_CRC );
    
    // Initialisation from Deviation
    radio.spiWriteRegister( NRF24_REG_00_CONFIG,   (NRF24_EN_CRC | NRF24_PWR_UP)); // Power up with CRC enabled
    radio.spiWriteRegister( NRF24_REG_01_EN_AA,     NRF24_ENAA_PA);                // Auto ACK on all pipes
    radio.spiWriteRegister( NRF24_REG_02_EN_RXADDR, NRF24_ERX_PA);                 // Enable all pipes
    radio.spiWriteRegister( NRF24_REG_03_SETUP_AW,  NRF24_AW_5_BYTES);             // 5-byte TX/RX address

    // Set RF power and data rate, then override REG_04/05 which it sets
    radio.setRF( radio.NRF24DataRate1Mbps, radio.NRF24TransmitPower0dBm);
    radio.spiWriteRegister( NRF24_REG_04_SETUP_RETR, 0x1A);                        // 500uS timeout, 10 retries
    radio.spiWriteRegister( NRF24_REG_05_RF_CH, RF_CHANNEL);                       // Channel 0x3C
    radio.spiWriteRegister( NRF24_REG_07_STATUS, NRF_STATUS_CLEAR);                // Clear status
    
    radio.spiWriteRegister( NRF24_REG_11_RX_PW_P0, PAYLOADSIZE);                   // Set payload size on all RX pipes
    radio.spiWriteRegister( NRF24_REG_12_RX_PW_P1, PAYLOADSIZE);
    radio.spiWriteRegister( NRF24_REG_13_RX_PW_P2, PAYLOADSIZE);
    radio.spiWriteRegister( NRF24_REG_14_RX_PW_P3, PAYLOADSIZE);
    radio.spiWriteRegister( NRF24_REG_15_RX_PW_P4, PAYLOADSIZE);
    radio.spiWriteRegister( NRF24_REG_16_RX_PW_P5, PAYLOADSIZE);
    
    radio.spiWriteRegister( NRF24_REG_17_FIFO_STATUS, 0x00);                       // Clear FIFO bits (unnesseary) 

    radio.spiWriteRegister( NRF24_REG_1C_DYNPD, 0x3F);                             // Enable dynamic payload (all pipes)
    radio.spiWriteRegister( NRF24_REG_1D_FEATURE, 0x07);                           // Payloads with ACK, noack command
    
    radio.spiWrite(ACTIVATE_CMD, ACTIVATE_DATA);                                   // Activate feature registers
    radio.spiWriteRegister( NRF24_REG_1C_DYNPD, 0x3F);                             // Enable dynamic payload (all pipes)
    radio.spiWriteRegister( NRF24_REG_1D_FEATURE, 0x07);                           // Payloads with ACK, noack command

    // Set command address
    set_cmmd_addr();
  
    // Power up
    radio.setConfiguration( NRF24_EN_CRC | NRF24_PWR_UP );
  
    radio.flushTx();
    radio.flushRx();
    radio.spiWriteRegister(NRF24_REG_07_STATUS, NRF_STATUS_CLEAR);
  }
};


/**
 * start_binding sends binding packets / sequence
 */
void CX10::start_binding(void) {
  set_bind_addr();

  for(int packno = 0; packno < 60; packno++) {
    send_packet(true);
  }
  
  set_cmmd_addr();
}


/**
 * 
 */
void CX10::setValues(cmds_t *data) {
  cmmd = data;
  
  // Add command values to trim to get real full scale response 
  // in original CX-10 firmware (FN firmware ignores the trims, so
  // no problems).
  rudder_trim = cmmd->rudder >> 1;
  aileron_trim = cmmd->aileron >> 1;
  elevator_trim = cmmd->elevator >> 1;
}


/**
 * send_packet constructs a packet and dispatches to radio
 */
void CX10::send_packet(bool bind) {
  // bind: send first four bytes of command address in the packet
  //       the final byte is set automatically to 0xC1 by CX-10.
  if (bind) {
    packet[0]= rx_tx_cmmd[0];
    packet[1]= rx_tx_cmmd[1];
    packet[2]= rx_tx_cmmd[2];
    packet[3]= rx_tx_cmmd[3];
    packet[4] = 0x56;
    packet[5] = 0xAA;
    packet[6] = 0x32;
    packet[7] = 0x00;
  } else {
    // cmnd: send RX commands present in the global variables 
    packet[0] = cmmd->throttle;
    packet[1] = cmmd->rudder;
    packet[3] = cmmd->elevator;
    packet[4] = cmmd->aileron;
    packet[2] = rudder_trim;
    packet[5] = elevator_trim;
    packet[6] = aileron_trim;
    packet[7] = cmmd->flags;
  }

  // clear packet status bits and TX FIFO
  radio.spiWriteRegister( NRF24_REG_07_STATUS, NRF_STATUS_CLEAR );
  radio.flushTx();
  
  // Form checksum (my modified CX-10 firmware doesnt care)
  packet[8] = packet[0];  
  for(uint8_t i=1; i < 8; i++)
    packet[8] += packet[i];
  packet[8] = ~packet[8];

  // Transmit, requesting acknowledgement
  write_payload(packet, 9, false);
  
  // find out what happens
  if (bind) {
    switch( packwait() ) {
      case PKT_ERROR:
        Serial.println("# binding: PKT_ERROR");
        while(1);
        break;
     
      case PKT_ACK: 
        break;
     
      case PKT_TIMEOUT:
        break;
    }
  } else {
    switch( packwait() ) {
    // Device not in TX mode, how did this happen?
    case PKT_ERROR_IN_RX:
      while(1);
      break;
     
    // Packet ACKed, move on
    case PKT_ACK: 
      break;
     
    // No ACK received, and we tried hard, so time out. 
    case PKT_TIMEOUT:
      break;
    }
  }
}


/**
 * write_payload issues the send command to the nrf24 library
 */
void CX10::write_payload(uint8_t *data, uint8_t len, bool noack) {
  radio.spiBurstWrite(noack ? NRF24_COMMAND_W_TX_PAYLOAD_NOACK : NRF24_COMMAND_W_TX_PAYLOAD, data, len);
}


/**
 * packwait polls the nrf24 radio to determine what's happened to our data
 */
uint8_t CX10::packwait(void) {
  // If we are currently in receive mode, then there is no packet to wait for
  if (radio.spiReadRegister(NRF24_REG_00_CONFIG) & NRF24_PRIM_RX)
    return PKT_ERROR_IN_RX;

  // Wait for either the Data Sent or Max ReTries flag, signalling the 
  // end of transmission
  uint8_t status;
    
  while (!((status = radio.statusRead()) & (NRF24_TX_DS | NRF24_MAX_RT)));
    
  radio.spiWriteRegister(NRF24_REG_07_STATUS, NRF24_TX_DS | NRF24_MAX_RT);
    
  switch(status &  (NRF24_TX_DS | NRF24_MAX_RT)) {
    
    case NRF24_TX_DS:
      return PKT_ACK;  
      break;
    
    case NRF24_MAX_RT:
      radio.flushTx();
      return PKT_TIMEOUT;
      break;
  }
  
  return PKT_ERROR;
}

 
void CX10::set_cmmd_addr(void) {
  radio.spiBurstWriteRegister( NRF24_REG_0A_RX_ADDR_P0,  rx_tx_cmmd, 5); 
  radio.spiBurstWriteRegister( NRF24_REG_10_TX_ADDR, rx_tx_cmmd, 5);                // Set command address
}


void CX10::set_bind_addr(void) {
  radio.spiBurstWriteRegister( NRF24_REG_0A_RX_ADDR_P0,  rx_tx_bind, 5);
  radio.spiBurstWriteRegister( NRF24_REG_10_TX_ADDR, rx_tx_bind, 5);                // Set command address
}


uint8_t CX10::get_pack_time(void) {
  return 8;
}

