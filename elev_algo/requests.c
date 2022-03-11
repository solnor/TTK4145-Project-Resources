#include "requests.h"

static int requests_above(Elevator e){
    for(int f = e.floor+1; f < N_FLOORS; f++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            if(e.requests[f][btn]){
                return 1;
            }
        }
    }
    return 0;
}

static int requests_below(Elevator e){
    for(int f = 0; f < e.floor; f++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            if(e.requests[f][btn]){
                return 1;
            }
        }
    }
    return 0;
}

static int requests_here(Elevator e){
    for(int btn = 0; btn < N_BUTTONS; btn++){
        if(e.requests[e.floor][btn]){
            return 1;
        }
    }
    return 0;
}


Action requests_nextAction(Elevator e){
    switch(e.dirn){
    case D_Up:
        return  requests_above(e) ? (Action){D_Up,   EB_Moving}   : // Is the request above current floor? Set direction upwards, state=moving
                requests_here(e)  ? (Action){D_Down, EB_DoorOpen} : // Is the request at current floor? Set direction downwards(?), state = open doors
                requests_below(e) ? (Action){D_Down, EB_Moving}   : // Is the request below current floor? Set direction downwards and state=moving
                                    (Action){D_Stop, EB_Idle}     ; // Is the request none of these? Direction=Stop, State=idle
    case D_Down:
        return  requests_below(e) ? (Action){D_Down, EB_Moving}   : // Analoguous...
                requests_here(e)  ? (Action){D_Up,   EB_DoorOpen} :
                requests_above(e) ? (Action){D_Up,   EB_Moving}   :
                                    (Action){D_Stop, EB_Idle}     ;
    case D_Stop: // there should only be one request in the Stop case. Checking up or down first is arbitrary.
        return  requests_here(e)  ? (Action){D_Stop, EB_DoorOpen} :
                requests_above(e) ? (Action){D_Up,   EB_Moving}   :
                requests_below(e) ? (Action){D_Down, EB_Moving}   :
                                    (Action){D_Stop, EB_Idle}     ;
    default:
        return (Action){D_Stop, EB_Idle};
    }
}



int requests_shouldStop(Elevator e){ //
    switch(e.dirn){
    case D_Down:
        return // Bool: If there is a request at current floor or a request beneath while going downwards
            e.requests[e.floor][B_HallDown] ||
            e.requests[e.floor][B_Cab]      ||
            !requests_below(e);
    case D_Up:
        return // Bool: if there is a request at current floor or request above while going upwards
            e.requests[e.floor][B_HallUp]   ||
            e.requests[e.floor][B_Cab]      ||
            !requests_above(e);
    case D_Stop:
    default:
        return 1;
    }
}

int requests_shouldClearImmediately(Elevator e, int btn_floor, Button btn_type){
    switch(e.config.clearRequestVariant){
    case CV_All:
        return e.floor == btn_floor;
    case CV_InDirn:
        return 
            e.floor == btn_floor && 
            (
                (e.dirn == D_Up   && btn_type == B_HallUp)    ||
                (e.dirn == D_Down && btn_type == B_HallDown)  ||
                e.dirn == D_Stop ||
                btn_type == B_Cab
            );  
    default:
        return 0;
    }
}

Elevator requests_clearAtCurrentFloor(Elevator e){
        
    switch(e.config.clearRequestVariant){
    case CV_All:
        for(Button btn = 0; btn < N_BUTTONS; btn++){
            e.requests[e.floor][btn] = 0;
        }
        break;
        
    case CV_InDirn:
        e.requests[e.floor][B_Cab] = 0;
        switch(e.dirn){
        case D_Up:
            if(!requests_above(e) && !e.requests[e.floor][B_HallUp]){
                e.requests[e.floor][B_HallDown] = 0;
            }
            e.requests[e.floor][B_HallUp] = 0;
            break;
            
        case D_Down:
            if(!requests_below(e) && !e.requests[e.floor][B_HallDown]){
                e.requests[e.floor][B_HallUp] = 0;
            }
            e.requests[e.floor][B_HallDown] = 0;
            break;
            
        case D_Stop:
        default:
            e.requests[e.floor][B_HallUp] = 0;
            e.requests[e.floor][B_HallDown] = 0;
            break;
        }
        break;
        
    default:
        break;
    }
    
    return e;
}











