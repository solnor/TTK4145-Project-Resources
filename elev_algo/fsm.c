
#include "fsm.h"

#include <stdio.h>

#include "con_load.h"
#include "elevator.h"
#include "elevator_io_device.h"
#include "requests.h"
#include "timer.h"

static Elevator             elevator;
static ElevOutputDevice     outputDevice;


static void __attribute__((constructor)) fsm_init(){
    elevator = elevator_uninitialized();
    
    con_load("elevator.con",
        con_val("doorOpenDuration_s", &elevator.config.doorOpenDuration_s, "%lf")
        con_enum("clearRequestVariant", &elevator.config.clearRequestVariant,
            con_match(CV_All)
            con_match(CV_InDirn)
        )
    )
    
    outputDevice = elevio_getOutputDevice();
}

static void setAllLights(Elevator es){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            outputDevice.requestButtonLight(floor, btn, es.requests[floor][btn]);
        }
    }
}

void fsm_onInitBetweenFloors(void){
    outputDevice.motorDirection(D_Down);
    elevator.dirn = D_Down;
    elevator.behaviour = EB_Moving;
}


void fsm_onRequestButtonPress(int btn_floor, Button btn_type){
    printf("\n\n%s(%d, %s)\n", __FUNCTION__, btn_floor, elevio_button_toString(btn_type));
    elevator_print(elevator);
    
    switch(elevator.behaviour){
    case EB_DoorOpen:
        if(requests_shouldClearImmediately(elevator, btn_floor, btn_type)){ // Probably if the button requests elev to current floor? 
            timer_start(elevator.config.doorOpenDuration_s);
        } else {
            elevator.requests[btn_floor][btn_type] = 1; // Add request to list
        }
        break;

    case EB_Moving:
        elevator.requests[btn_floor][btn_type] = 1; // If the elevator is moving, it doesn't need to check whether it's already on a floor
        break;
        
    case EB_Idle:    
        elevator.requests[btn_floor][btn_type] = 1; // Add request
        Action a = requests_nextAction(elevator); // Find next action to take?
        elevator.dirn = a.dirn;
        elevator.behaviour = a.behaviour;
        switch(a.behaviour){ // This switch is to decide the NEXT move/state....
        case EB_DoorOpen: // Open door, start timer, clear request
            outputDevice.doorLight(1); 
            timer_start(elevator.config.doorOpenDuration_s);
            elevator = requests_clearAtCurrentFloor(elevator);
            break;

        case EB_Moving:
            outputDevice.motorDirection(elevator.dirn);
            break;
            
        case EB_Idle:
            break;
        }
        break;
    }
    
    setAllLights(elevator);
    
    printf("\nNew state:\n");
    elevator_print(elevator);
}




void fsm_onFloorArrival(int newFloor){
    printf("\n\n%s(%d)\n", __FUNCTION__, newFloor);
    elevator_print(elevator);
    
    elevator.floor = newFloor; // Update floor
    
    outputDevice.floorIndicator(elevator.floor); // Light floor button most likely
    
    switch(elevator.behaviour){
    case EB_Moving:
        if(requests_shouldStop(elevator)){ // Just returns 1 if anything requires the elev to stop at current floor, 0 otherwise
            outputDevice.motorDirection(D_Stop); // Set motordir to 0
            outputDevice.doorLight(1); // Turn light on
            elevator = requests_clearAtCurrentFloor(elevator); // Clear request
            timer_start(elevator.config.doorOpenDuration_s); // Start timer
            setAllLights(elevator);
            elevator.behaviour = EB_DoorOpen; // Set elevator state <----------------------------------------Goes directly from moving to doorOpen
        }
        break;
    default:
        break;
    }
    
    printf("\nNew state:\n");
    elevator_print(elevator); 
}




void fsm_onDoorTimeout(void){
    printf("\n\n%s()\n", __FUNCTION__);
    elevator_print(elevator);
    
    switch(elevator.behaviour){
    case EB_DoorOpen:;
        Action a = requests_nextAction(elevator);
        elevator.dirn = a.dirn;
        elevator.behaviour = a.behaviour;
        
        switch(elevator.behaviour){
        case EB_DoorOpen:
            timer_start(elevator.config.doorOpenDuration_s);
            elevator = requests_clearAtCurrentFloor(elevator);
            setAllLights(elevator);
            break;
        case EB_Moving:
        case EB_Idle:
            outputDevice.doorLight(0);
            outputDevice.motorDirection(elevator.dirn);
            break;
        }
        
        break;
    default:
        break;
    }
    
    printf("\nNew state:\n");
    elevator_print(elevator);
}













