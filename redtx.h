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
    T_CX10  = 1,        // red board
    T_CX10G = 2,        // TODO: green board
    T_CX10A = 3         // TODO: cx10A, w/ heading-hold, blue board
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



class BASE_QUAD {
public:
    //virtual ~myInterface();
    virtual ~BASE_QUAD() {};
    
    //virtual int getVal(void) = 0;
    //virtual void setVal(int n) = 0;
    
    virtual void init(void) = 0;
    virtual void start_binding(void) = 0;
    
    virtual void setValues(cmds_t *data);
    virtual void send_packet(bool bind) = 0;
    virtual uint8_t packwait(void) = 0;
    virtual uint8_t get_pack_time(void) = 0;

};







#endif // _REDTX_H_
