#include <iostream>
#include <random>
#include <time.h>
#include <iomanip>
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
    for(int devices=40;devices<=60;devices+=10){
        cout << "number of devices: " << devices << endl;

        srand((int)time(NULL));
        /*default_random_engine generator;
        poisson_distribution<int> distribution(40*0.5);
        num = distribution(generator);*/

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
    
        int d[60];
        int times[80]={0}, p_times[80]={0}; 

        //calculate the times of corrupt
        for(int h=0;h<48000;h++){
            int channel[80]={0};
            for(int i=0;i<=devices;i++){
            //for(int i=0;i<=num;i++){
                d[i] = rand()%79+1;
                //times of corrupts
                channel[d[i]]++;          
            }
            // >1, only calculate once
            for(int j=1;j<=79;j++){

                if(channel[j]>1){
                    times[j]+=1; 
                }
                if(channel[j]>1 || ch[j-1]==2){
                    p_times[j]+=1; 
                }
            }
        }

        //the corrupt of the channel
        for(int i=1;i<=79;i++){     
            double pr = times[i]/48000.0;
            //cout << "channel id = " << i << ", p = " << fixed  <<  setprecision(3) << pr << endl;
            cout << "ch_id = " << i << ", p = " << pr << endl;
            //cout <<  " " << pr << endl;
        }

        //output threshold
        for(double threshold=0.1; threshold<0.9; threshold+=0.1){
            int bad=0;
            for(int i=1;i<=79;i++){       
                double pr = p_times[i]/48000.0;        
                if(pr>=threshold && pr<(threshold+0.1)){
                    bad++;                
                }
            }
            cout << "threshold: " << threshold << ", # of bad channel : " << bad << endl;
            //cout << threshold << " " << bad << endl;
        }

        /*int count1=1,count2=1;
        for(int i=0;i<79;i++){
            if(ch[i] == 1 || ch[i] == 0){
                cout << "clean channel: " << i << endl;
                cout << count1++ << endl;
            }            
            else if(ch[i] == 2){
                cout << "bad channel: " << i << endl;
                cout << count2++ << endl;
            }
        }*/

        cout << "  "<< endl;
    }
    return 0;
}