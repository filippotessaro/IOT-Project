process_start(&scatter_proc, "scatter proc");


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


