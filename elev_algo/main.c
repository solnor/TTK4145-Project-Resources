
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "con_load.h"
#include "elevator_io_device.h"
#include "fsm.h"
#include "timer.h"


int main(void){
    printf("Started!\n");
    
    int inputPollRate_ms = 25;
    con_load("elevator.con",
        con_val("inputPollRate_ms", &inputPollRate_ms, "%d")
    )
    
    ElevInputDevice input = elevio_getInputDevice(); // Establishes connection with hw probably
    
    if(input.floorSensor() == -1){
        fsm_onInitBetweenFloors();
    }
        
    while(1){
        { // Request button
            static int prev[N_FLOORS][N_BUTTONS];
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    int v = input.requestButton(f, b); // For every floor, every button (B_HallUp,B_HallDown,B_Cab) check if pressed??
                    if(v  &&  v != prev[f][b]){ // If button is pressed and the button that IS pressed is not already "noticed"
                        fsm_onRequestButtonPress(f, b); // Decides what to do next
                    }
                    prev[f][b] = v; // Adds current state of button to prev list
                }
            }
        }
        
        { // Floor sensor
            static int prev;
            int f = input.floorSensor(); // Gets floor sensor state, probably gives number between 0 and 3? - and -1 if not on any floor
            if(f != -1  &&  f != prev){ // Check if elev actually on floor and if it's the same as last time
                fsm_onFloorArrival(f); // --------------------------------------------------------------------------------------------------------
            }
            prev = f;
        }
        
        
        { // Timer
            if(timer_timedOut()){
                timer_stop();
                fsm_onDoorTimeout();
            }
        }
        
        usleep(inputPollRate_ms*1000);
    }
}









