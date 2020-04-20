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
  .callback_turn_off = NULL,
    .callback_end_epoch = NULL
  };
/*---------------------------------------------------------------------------*/
// STATIC GLOBAL VARIABLES
/* epoch counter */
static uint8_t epoch = 0;

/* it has to be less than 10 */
static uint8_t curr_task = 0;

/* max epoch iteration */
static uint8_t max_epoch_num = 150;

/* Structure of discovered neighbour */
static bool discovered_neighbour[MAX_NBR];

static rtimer_clock_t next;
static rtimer_clock_t next_off = RTIMER_SECOND/100 ;
//static rtimer_clock_t next_on;
static struct rtimer rt_off;

/*---------------------------------------------------------------------------*/
void
nd_recv(void)
{
  /* New packet received
   * 1. Read packet from packetbuf---packetbuf_dataptr()
   * 2. If a new neighbor is discovered within the epoch, notify the application
   */
   uint8_t* payload = packetbuf_dataptr();
   uint8_t  neighbor_id;
   neighbor_id  = (uint8_t)payload[0];
    printf("App: Epoch %u New NBR %u\n",
           epoch, neighbor_id);
   discovered_neighbour[neighbor_id - 1] = true;
        
}
/*---------------------------------------------------------------------------*/
// Processes declaration
PROCESS(burst_proc, "Another auxiliary process");
PROCESS(scatter_proc, "Another auxiliary process");
/*---------------------------------------------------------------------------*/
//Callbacks for on-off
void
turn_on_radio_callback(struct rtimer *t, void *ptr){
    NETSTACK_RADIO.on();
    rtimer_set(t, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
}

void
turn_off_radio_callback(struct rtimer *t, void *ptr){
    NETSTACK_RADIO.off();
    curr_task++;
    
    // if task <10
    if(curr_task<10){
        // schedule ON
        rtimer_set(t, RTIMER_NOW() + (RTIMER_SECOND/10 - next_off), 0, app_cb.callback_turn_on,NULL);
    }else{
        rtimer_set(t, RTIMER_NOW() + (RTIMER_SECOND/10 - next_off), 0, app_cb.callback_end_epoch,NULL);
    }
    
    // else end_epoch
    
}

void
end_epoch_callback(struct rtimer *t, void *ptr){
    //NETSTACK_RADIO.off();
    uint8_t num_nbr = 0;
    uint8_t iterator = 0;

    // iterate over discovered_neighbour array
    for(iterator = 0; iterator < MAX_NBR ; iterator++){
      if (discovered_neighbour[iterator]){
        num_nbr ++;
      }
      // Initialize vector elem
      discovered_neighbour[iterator] = false;
    }
    
    app_cb.nd_epoch_end(epoch, num_nbr);
    epoch++;
    
    // restart process
    printf("Restart process\n");
    curr_task = 0;
    process_start(&burst_proc, "burst proc");
}

/*---------------------------------------------------------------------------*/

void
nd_start(uint8_t mode, const struct nd_callbacks *cb)
{
  /* Start seleced ND primitive and set nd_callbacks */
    app_cb.nd_new_nbr = cb -> nd_new_nbr;
    app_cb.nd_epoch_end = cb -> nd_epoch_end;
    app_cb.callback_turn_on = turn_on_radio_callback;
    app_cb.callback_turn_off = turn_off_radio_callback;
    app_cb.callback_end_epoch = end_epoch_callback;
    
    printf("ProcessStart\n");
    if(mode == ND_BURST){
        process_start(&burst_proc, "burst proc");
    } else if (mode == ND_SCATTER){
        process_start(&scatter_proc, "burst proc");
    }
    //printf("ProcessEnd\n");
}
/*---------------------------------------------------------------------------*/
//process thread

PROCESS_THREAD(burst_proc, ev, data)
{
    PROCESS_BEGIN();
    
    uint8_t num_task = 10;
    bool we_are_sending = 0;
    
    //while(epoch <= max_epoch_num) {
        /* next turn radio on */
        next = RTIMER_NOW() + RTIMER_SECOND/num_task;
        int num_packets = 0;
                
        /* keep sending untile rtimer expires */
        while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {
            if( we_are_sending==0){
                we_are_sending = 1;
                //packetbuf_clear();
                NETSTACK_RADIO.send(&node_id, sizeof(uint8_t));
                we_are_sending = 0;
                num_packets++;
            }
            rtimer_clock_t rnow = RTIMER_NOW();
            while (RTIMER_CLOCK_LT( RTIMER_NOW(), rnow + RTIMER_SECOND / 100)) {};
        }
        
//        uint8_t i = 0;
        printf("Sent Packets: %d\n", num_packets);
        rtimer_set(&rt_off, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
//
//        // ON : NOW() -> NOW + RTIMER_SECOND/5 -> NOW + RTIMER_SECOND/5 * i
//        // OFF : NOW + NEXT_TURN_OFF -> NOW + NEXT_TURN_OFF +
//
//        while (i<num_task * 2) {
//            //printf("i:%d \n", i);
//
//            rtimer_set(&rt_off, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
//            NETSTACK_RADIO.on();
//            // modifica qui
//            next = RTIMER_NOW() + (RTIMER_SECOND/(num_task))/2;
//
//            while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {}
//
//            i++;
//        }
//        //printf("Pause\n");
//
        
        //PROCESS_PAUSE();
    //}
    

  PROCESS_END();
}

PROCESS_THREAD(scatter_proc, ev, data)
{
    PROCESS_BEGIN();
    uint8_t num_task = 25;
    bool we_are_sending = 0;

    
    rtimer_clock_t next;
    rtimer_clock_t next_off = RTIMER_SECOND/(num_task+1);
    //rtimer_clock_t next_transmit_stop;
    
    static struct rtimer rt_off;
    
    while(epoch <= max_epoch_num) {
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
            // num_task = # of sending interval
            next = RTIMER_NOW() + next_off;
            //next_transmit_stop = RTIMER_NOW() + RTIMER_SECOND/50;
            
            //while (RTIMER_CLOCK_LT(RTIMER_NOW(), next_transmit_stop)) {
                // check if channel is clear and we are not sending
            
            // Send only one beacon
            if(NETSTACK_RADIO.channel_clear() && we_are_sending == 0 ){
                we_are_sending = 1;
                packetbuf_clear();
                NETSTACK_RADIO.send(&node_id, sizeof(uint8_t));
                we_are_sending = 0;
            }
            //}
            //wait until next task
            while (RTIMER_CLOCK_LT(RTIMER_NOW(), next)) {}
            
            i++;
        }
        //printf("Pause\n");
        
        uint8_t num_nbr = 0;
        uint8_t iterator = 0;

        // iterate over discovered_neighbour array
        for(iterator = 0; iterator < MAX_NBR ; iterator++){
          if (discovered_neighbour[iterator] == true){
            num_nbr ++;
            printf("FOR LOOP Node = %u discovered\n", iterator);
          }
          // Initialize vector elem
          discovered_neighbour[iterator] = false;
        }
        
        app_cb.nd_epoch_end(epoch, num_nbr);
        epoch++;
        PROCESS_PAUSE();
    }
    

  PROCESS_END();
}




/*---------------------------------------------------------------------------*/
