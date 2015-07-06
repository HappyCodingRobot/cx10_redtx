/**
 * 
 * 
 */

#ifndef _CX10_H_
#define _CX10_H_

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#include <pins_arduino.h>
#endif

#include <NRF24.h>
#include <SPI.h>
#include "redtx.h"


// Radio and register defines
#define RF_CHANNEL      0x3C  // Stock TX fixed frequency
#define ACTIVATE        0x50  // Activate command
#define PAYLOADSIZE     9

#define ACTIVATE_DATA       0x73
#define ACTIVATE_CMD        0x50

#define NRF24_ENAA_PA (NRF24_ENAA_P0 | NRF24_ENAA_P1 | NRF24_ENAA_P2 | NRF24_ENAA_P3 | NRF24_ENAA_P4 | NRF24_ENAA_P5)
#define NRF24_ERX_PA (NRF24_ERX_P0 | NRF24_ERX_P1 | NRF24_ERX_P2 | NRF24_ERX_P3 | NRF24_ERX_P4 | NRF24_ERX_P5)
#define NRF_STATUS_CLEAR 0x70


// Packet state enumeration
enum packet_state_e {
    PKT_PENDING = 0,
    PKT_ACK,
    PKT_TIMEOUT,
    PKT_ERROR,
    PKT_ERROR_IN_RX
};


//class NRF24;

class CX10
{
public:
    //CX10();
    CX10(NRF24& _radio);
    
    void init(void);
    void start_binding(void);
    
    void setValues(cmds_t *data);
    void send_packet(bool bind);
    uint8_t packwait(void);
    uint8_t get_pack_time(void);
    
private:
    NRF24 radio;
    
    bool initialized;
    
    void write_payload(uint8_t *data, uint8_t len, bool noack);
    
    void set_cmmd_addr(void);
    void set_bind_addr(void);
    
    // Command and bind addresses (command address should be generated from random number)
    static uint8_t rx_tx_cmmd[5];
    static uint8_t rx_tx_bind[5];

    // Data packet buffer
    uint8_t packet[PAYLOADSIZE];
    
    // channel commands pointer
    cmds_t *cmmd;
    uint8_t rudder_trim;
    uint8_t elevator_trim;
    uint8_t aileron_trim;
};



#endif // _CX10_H_
