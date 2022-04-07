//İlayda Özel 260201037
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define vehicle_number 16

#define LEFT 2
#define RIGHT 1
#define BRIDGE 0

#define MOVE 1
#define WAIT 0

#define LEFTTORIGHT 1
#define RIGHTTOLEFT 0

#define LOCKED 0
#define UNLOCKED 1

//define a struct vehicle
typedef struct vehicle{
    int location;
    int motion;
    int id;
    int direction;
}vehicle;

//define a struct bridge
typedef struct bridge{
    int locked;
}bridge;

//create global variable bridge and global vehicle arrays
struct bridge veh_bridge={UNLOCKED};
vehicle vehicles_left[vehicle_number/2];
vehicle vehicles_right[vehicle_number/2];

//create global counters
int right_to_left_count;
int left_to_right_count;

sem_t sem_bridge;


void* exiting(void * veh){
    vehicle* elm= veh;
    veh_bridge.locked=UNLOCKED; //bridge is unlocked again
    elm->motion=WAIT; //the vehicle is waiting again
    if(elm->direction==LEFTTORIGHT){
        left_to_right_count++;  //increment the number of vehicles going into right direction
        right_to_left_count=0; //initialize the number of vehicles going to left
        elm->location=RIGHT; //change the location of the vehicle
    }
    else{
        left_to_right_count=0; //increment the number of vehicles going into left direction
        right_to_left_count++; //initialize the number of vehicles going to right
        elm->location=LEFT; //change the location of the vehicle
    }
    return 0;
}

void* passing(void* veh){
    vehicle* elm= veh;
    int i= elm->id;
    char* veh_direction = "";
    if (elm->direction==0){    //checking the direction of the vehicles for the purpose of printing
        veh_direction="right to left";
    }
    else{
        veh_direction="left to right";
    }

    sem_wait(&sem_bridge);  //lock the bridge with semaphore, since no other vehicle should be able to access the bridge while it is occupied
    sleep(1); //the time passes wh,le the vehicle is passing
    printf(" %d th vehicle with %s direction is passing \n", i+1 , veh_direction);
    veh_bridge.locked = LOCKED; //bridge is locked
    elm->motion = MOVE; //vehicle is moved
    elm->location = BRIDGE; //vehicle is on the bridge
    exiting(elm); //send the vehicle to exit
    sem_post(&sem_bridge); //unlock the bridge
    }


void* entering(void * veh) {
    vehicle *elm = veh;
    elm->motion = WAIT; //vehicle is waiting

    if (!(((elm->direction == LEFTTORIGHT) && (left_to_right_count<5)) || ((elm->direction == RIGHTTOLEFT) && (right_to_left_count<5)))) {
        sem_post(&sem_bridge);//if 5 vehicles in the same direction passed consecutively, the free the bridge
        sleep(1);
    }
    passing(elm); //send the vehicles to the bridge
}


void* traffic(void * arr){
    struct vehicle* veh_arr = (struct vehicle*) arr;
    //sending the vehicles to the entrance of the bridge
    for(int i=0; i<vehicle_number/2; i++){
        entering((void *) &veh_arr[i]);
    }
    return 0;
}

int main()
{
    sem_init(&sem_bridge, 0, 1); //initialize the bridge semaphore

    for (int i=0; i<vehicle_number/2; i++) {  //initialize the vehicle arrays
        vehicles_left[i].motion = WAIT;
        vehicles_left[i].id = i;
        vehicles_left[i].location = LEFT;
        vehicles_left[i].direction = LEFTTORIGHT;
    }
    for (int j=0; j<vehicle_number/2; j++) {
        vehicles_right[j].motion = WAIT;
        vehicles_right[j].id = j+8;
        vehicles_right[j].location = RIGHT;
        vehicles_right[j].direction = RIGHTTOLEFT;
    }

    pthread_t tid[2];  //create thread array
    pthread_create(&tid[0], NULL, traffic,  (void *) vehicles_left); //create vehicle threads for the vehicles in the left
    pthread_create(&tid[1], NULL, traffic,  (void *) vehicles_right); //create vehicle threads for the vehicles in the right
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    sem_destroy(&sem_bridge); //destroy semaphore when it is done
}


