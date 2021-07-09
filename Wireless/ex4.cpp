#include <iostream>
#include <random>
#include <time.h>
#include <iomanip>
#include <math.h>
using namespace std;

int poisson_rn(float lambda){
    float L = expf(-lambda), p=1.0;
    int k=0;
    while(p>L){
        k++;
        float u = rand() / (float)(RAND_MAX+1.0);
        p*=u;
    }
    return k-1;
}

int main(){
    default_random_engine generator ((int)time(NULL));
    normal_distribution<double> distribution (30,sqrt(5));

    for(int devices=40;devices<=60;devices+=10){
        cout << "number of devices: " << devices << endl;

        double timing = distribution(generator);
        /*if((int)timing % 5 !=0){
            int quotient=timing/5;
            timing = (quotient+1)*5;
        }*/
        //cout << "timing " << timing << endl;

        int ch[79]={0}, arr[40]={0};
        int clean, num, noise;

        //39 of randomly selected channels are not interfered by noise
        for(int i=0;i<39;i++){
            clean = rand()%79+1;
            while(ch[clean-1]==1){
                clean = rand()%79+1;
            }
            ch[clean-1]=1;
        }

        //calculate poisson
        num = poisson_rn(40*0.5);
        //cout << "poisson: " << num << endl;
    
        //other 40 channels
        int change=0;
        for(int i=0;i<79;i++){
            if(ch[i]==0){
                arr[change]=i;
                change++;
            }
        }

        //probably corrupted
        for(int i=0;i<num;i++){            
            noise = rand()%change+1;
            while(ch[arr[noise]]==2){
                noise = rand()%change+1;
            }
            ch[arr[noise]]=2;
        }
        //int t=0;
        int times[80]={0}, d[60];
        //second[24]; 
        int bad_channel[24]={0}, corrupt_times[80]={0};

        //for(t=(int)timing;t<=60;t+=5){
            //cout << t << endl;
            //calculate the times of corrupt

            //5 sec * 1600 hops
            for(int h=0;h<8000;h++){                
                int channel[80]={0};
                for(int i=0;i<=devices;i++){
                //for(int i=0;i<=num;i++){
                    d[i] = rand()%79+1;
                    //times of corrupts
                    channel[d[i]]++;          
                }

                // >1, only calculate once
                for(int j=1;j<=79;j++){
                    if(channel[j]>1 || ch[j-1]==2){
                        times[j]+=1; 
                    }
                    if(channel[j]>1){
                        corrupt_times[j]+=1; 
                    }
                }

                //the moment: 8000/(12*2)
                if(h%333==0){
                    for(int i=0;i<=79;i++){
                        if(channel[i]>1 || ch[i-1]==2)
                            bad_channel[h/333]+=1;
                    }
                }
           }
        //output threshold
        int index = 0;
        for(double threshold=0.3; threshold<=0.6; threshold+=0.3){
            for(int i=0;i<12;i++,index++){
                if(timing>i*5.0){
                    int bad=0;
                    for(int i=1;i<=79;i++){
                        double pr = corrupt_times[i]/8000.0;        
                        if(pr>=threshold && pr<(threshold+0.3)){
                            bad++;                
                        }
                    }
                    cout << (i+1)*5 << " second, threshold: " << threshold << ", # of bad channel : " << bad << endl;
                }                              
                else{
                    cout << (i+1)*5 << " second, threshold: " << threshold << ", # of bad channel : " << bad_channel[index] << endl; 
                }                    
            }                
        }
        cout << "  " << endl;
    }
    return 0;
}