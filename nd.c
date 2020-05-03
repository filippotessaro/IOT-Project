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
    .callback_end_epoch = NULL,
    .callback_send_packet = NULL
  };
/*---------------------------------------------------------------------------*/
// STATIC GLOBAL VARIABLES
/* epoch counter */
static uint8_t epoch = 0;

/* it has to be less than 10 */
static uint8_t curr_task = 0;

static uint8_t num_task = 20;

/* max epoch iteration */
static uint8_t max_epoch_num = 130;

/* Structure of discovered neighbour */
static bool discovered_neighbour[MAX_NBR];

static rtimer_clock_t next;
static rtimer_clock_t next_off = RTIMER_SECOND/100 ;
static rtimer_clock_t next_send;
//static struct rtimer rt_off;

static bool m; // scatter = false , burst = true

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
void
send_packet_callback(struct rtimer *t, void *ptr){
    curr_task++;
    NETSTACK_RADIO.off();
    
    /* check current task */
    if(curr_task < num_task){
        //printf("Schedule cur task send_packet_callback\n");
        /* schedule next tun radio on */
        rtimer_set(t, RTIMER_NOW() + next_send, 0, app_cb.callback_send_packet,NULL);
    }else{
        /* schedule end epoch */
        rtimer_set(t, RTIMER_NOW() + next_send, 0, app_cb.callback_end_epoch,NULL);
    }
    
    /* send packet */
    NETSTACK_RADIO.send(&node_id, sizeof(uint8_t));
    
    /* turn radio off */
    //NETSTACK_RADIO.off();
}
//Callbacks for on-off
void
turn_on_radio_callback(struct rtimer *t, void *ptr){
    NETSTACK_RADIO.on();
    if(m){
        /* burst mode */
        rtimer_set(t, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
    } else{
        /* scatter mode */
        rtimer_set(t, RTIMER_NOW() + next_send, 0, app_cb.callback_send_packet,NULL);
    }
}

void
turn_off_radio_callback(struct rtimer *t, void *ptr){
    NETSTACK_RADIO.off();
    curr_task++;
    
    // if task <10
    if(curr_task < num_task){
        // schedule ON
        rtimer_set(t, RTIMER_NOW() + (RTIMER_SECOND/num_task - next_off), 0, app_cb.callback_turn_on, NULL);
    }else{
        rtimer_set(t, RTIMER_NOW() + (RTIMER_SECOND/num_task - next_off), 0, app_cb.callback_end_epoch,NULL);
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
    if(epoch<max_epoch_num){
        if(m){
            process_start(&burst_proc, "burst proc");
        }else{
            process_start(&scatter_proc, "scatter proc");
        }
    }else {
        printf("Simulation Finished\n");
    }
    
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
    app_cb.callback_send_packet = send_packet_callback;
    
    printf("ProcessStart\n");
    if(mode == ND_BURST){
        process_start(&burst_proc, "burst proc");
        m = true;
    } else if (mode == ND_SCATTER){
        process_start(&scatter_proc, "scatter proc");
        m = false;
    }
}
/*---------------------------------------------------------------------------*/
//process thread

PROCESS_THREAD(burst_proc, ev, data)
{
    PROCESS_BEGIN();
    static struct rtimer rt_off;

    
    bool we_are_sending = 0;
    
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
        
        /* wait next transmission */
        while (RTIMER_CLOCK_LT( RTIMER_NOW(), rnow + RTIMER_SECOND / 150)) {};
    }
    
    printf("Sent Packets: %d\n", num_packets);
    
    //NETSTACK_RADIO.on();
    
    /* schedule next turn-off */
    rtimer_set(&rt_off, RTIMER_NOW() + next_off, 0, app_cb.callback_turn_off,NULL);
    
    PROCESS_END();
}

PROCESS_THREAD(scatter_proc, ev, data)
{
    PROCESS_BEGIN();
    
    static struct rtimer rt_off;
    
    //rtimer_clock_t next;
    //rtimer_clock_t next_off = RTIMER_SECOND/num_task;
    next_send = RTIMER_SECOND/num_task;
    
    // TURN ON RADIO
    NETSTACK_RADIO.on();
    
    // schedule next turn off
    rtimer_set(&rt_off, RTIMER_NOW() + next_send, 0, app_cb.callback_send_packet,NULL);
    
  PROCESS_END();
}




/*---------------------------------------------------------------------------*/

