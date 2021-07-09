#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <iomanip> 
using namespace std;

int main(){
    srand((int)time(NULL));

    double pr;
    //30 second * 1600 hops/sec = 48000 hops
    int collision=0,a[48000]={0},b[48000]={0};

    for(int i=0;i<=48000;i++){
        a[i]=rand()%79+1;
        b[i]=rand()%79+1;
        //two devices choose the same channel
        if(a[i]==b[i] && a[i]!=0 && b[i]!=0)
            collision++;
    }

    cout << "Collision: " << collision << endl;
    // happen/total hops
    pr = collision/(1600.0*30.0);
    cout << "Average collision probabilities: " << fixed  <<  setprecision(3) << pr << endl;

    return 0;
}