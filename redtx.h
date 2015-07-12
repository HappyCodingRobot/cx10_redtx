/**
 * 
 * 
 */

#ifndef _REDTX_H_
#define _REDTX_H_


#define PAYLOADSIZE_MAX 20


/* Supported modes */
enum cx_type_e
{
    T_CX10R = 1,        // red board
    T_CX10G = 2,        // TODO: green board
    T_CX10A = 3         // TODO: cx10A, w/ heading-hold, blue board
};

enum tx_ch_e
{
    ch_thr = 0,
    ch_ail,
    ch_ele,
    ch_rud,
    ch_AIL,
    ch_TRN,
    ch_3POS
};

typedef struct {
    uint8_t aileron;
    uint8_t elevator;
    uint8_t throttle;
    uint8_t rudder;
    uint8_t rate;
    uint8_t flags;
    uint8_t mode;
} cmds_t;



/** base class defintion */
class BASE_QUAD {
public:
    //virtual ~BASE_QUAD() {};
    
    virtual void init(void) = 0;
    virtual void start_binding(void) = 0;
    
    virtual void setValues(cmds_t *data);
    virtual void send_packet(bool bind) = 0;
    virtual uint8_t packwait(void) = 0;
    virtual uint8_t get_pack_time(void) = 0;
};




#ifdef DEBUG
#define debugP Serial.println
#else
#define debugP(args...) ((void)0)
#endif


#endif // _REDTX_H_
