/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/random.h"
#include "sys/rtimer.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "node-id.h"
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#include "nd.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
struct nd_callbacks app_cb = {
  .nd_new_nbr = NULL,
  .nd_epoch_end = NULL,
  .callback_turn_on = NULL,
  .callback_turn_off = NULL
  };
/*---------------------------------------------------------------------------*/
void
nd_recv(void)
{
  /* New packet received
   * 1. Read packet from packetbuf---packetbuf_dataptr()
   * 2. If a new neighbor is discovered within the epoch, notify the application
   */
    //printf("RECV\n");
    uint8_t* payload = packetbuf_dataptr();
    uint8_t  neighbor;
    neighbor  = (uint8_t)payload[0];
    printf("Recv seqn = %u\n", neighbor);
}
/*---------------------------------------------------------------------------*/
//Callbacks for on-off
void
turn_on_radio_callback(struct rtimer *t, void *ptr){
  //rtimer_set(t, RTIMER_NOW() + next_turn_off, 0, app_cb.callback_turnoff,NULL);
  NETSTACK_RADIO.on();
}

void
turn_off_radio_callback(struct rtimer *t, void *ptr){
  NETSTACK_RADIO.off();
}




/*---------------------------------------------------------------------------*/
PROCESS(burst_proc, "Another auxiliary process");
PROCESS(scatter_proc, "Another auxiliary process");


void
nd_start(uint8_t mode, const struct nd_callbacks *cb)
{
  /* Start seleced ND primitive and set nd_callbacks */
    app_cb.nd_new_nbr = cb -> nd_new_nbr;
    app_cb.nd_epoch_end = cb -> nd_epoch_end;
    app_cb.callback_turn_on = turn_on_radio_callback;
    app_cb.callback_turn_off = turn_off_radio_callback;
    
    printf("ProcessStart\n");
    process_start(&scatter_proc, "burst proc");
    printf("ProcessEnd\n");

    //}
    
}
/*---------------------------------------------------------------------------*/
//process thread

PROCESS_THREAD(burst_proc, ev, data)
{
    PROCESS_BEGIN();
    rtimer_clock_t next;
    //rtimer_clock_t next_on = RTIMER_SECOND/5;
    rtimer_clock_t next_off = RTIMER_SECOND/20;

    //rtimer_clock_t next_epoch = RTIMER_NOW() + 100 * EPOCH_INTERVAL_RT;
    
    //static struct rtimer rt_on;
    static struct rtimer rt_off;
    
    uint8_t num_task = 4;
    
    bool we_are_sending = 0;
    
    while(1) {
        //printf("Start Epoch\n");
        next = RTIMER_NOW() + RTIMER_SECOND/(num_task+1);
        
        while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {
            //printf("\n");
            if(NETSTACK_RADIO.channel_clear() && we_are_sending == 0 ){
                we_are_sending = 1;
                packetbuf_clear();
                NETSTACK_RADIO.send(&node_id, sizeof(uint8_t));
                we_are_sending = 0;
            }
        }
        
        uint8_t i = 0;
        
        // ON : NOW() -> NOW + RTIMER_SECOND/5 -> NOW + RTIMER_SECOND/5 * i
        // OFF : NOW + NEXT_TURN_OFF -> NOW + NEXT_TURN_OFF +
                
        while (i<num_task) {
            //printf("i:%d \n", i);
            NETSTACK_RADIO.on();
            rtimer_set(&rt_off, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
            
            next = RTIMER_NOW() + RTIMER_SECOND/(num_task+1);
            
            while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {}
            
            i++;
        }
        printf("Pause\n");
        PROCESS_PAUSE();
    }
    

  PROCESS_END();
}

PROCESS_THREAD(scatter_proc, ev, data)
{
    PROCESS_BEGIN();
    rtimer_clock_t next;
    rtimer_clock_t next_off = RTIMER_SECOND/5;
    rtimer_clock_t next_transmit_stop = RTIMER_SECOND/20;
    
    //static struct rtimer rt_on;
    static struct rtimer rt_off;
    
    uint8_t num_task = 4;
    
    bool we_are_sending = 0;
    
    while(1) {
        //printf("Start Epoch\n");
        next = RTIMER_NOW() + next_off;
        
        // TURN ON RADIO
        NETSTACK_RADIO.on();
        
        // schedule next turn off
        rtimer_set(&rt_off, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
        
        uint8_t i = 0;
        
        // wait untill next turn off
        while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {}
        
                        
        while (i<num_task) {
            // SENDING MODE
            next = RTIMER_NOW() + next_off;
            next_transmit_stop = RTIMER_NOW() + RTIMER_SECOND/50;
            
            while (RTIMER_CLOCK_LT(RTIMER_NOW(), next_transmit_stop)) {
                //printf("\n");
                if(NETSTACK_RADIO.channel_clear() && we_are_sending == 0 ){
                    we_are_sending = 1;
                    packetbuf_clear();
                    NETSTACK_RADIO.send(&node_id, sizeof(uint8_t));
                    we_are_sending = 0;
                }
            }
            //wait until next task
            while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {}
            
            i++;
        }
        printf("Pause\n");
        PROCESS_PAUSE();
    }
    

  PROCESS_END();
}




/*---------------------------------------------------------------------------*/
