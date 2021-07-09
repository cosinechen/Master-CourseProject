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
        int test=0;

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
        int times[80]={0},corrupt_times[80]={0}; 

        //calculate the times of corrupt
        for(int h=0;h<48000;h++){
            int channel[80]={0};
            for(int i=0;i<=devices;i++){
            //for(int i=0;i<=num;i++){
                d[i] = rand()%79+1;

                //corrupt
                if(channel[d[i]]==1)
                    corrupt_times[d[i]]+=1;

                //bad channel (corrupt+noise)
                if(channel[d[i]]==1 || ch[d[i]]==2){
                    times[d[i]]+=1;
                    for(int diff=1;d[i]-diff>=0||d[i]+diff<=79;diff++) {
                   
                        if(d[i]-diff>=0) {                        
                            if(channel[d[i]-diff]==0) {
                                d[i]=d[i]-diff;
                                break;
                            }
                        }
                        if(d[i]+diff<=79) {
                            if(channel[d[i]+diff]==0) {
                                d[i]=d[i]+diff;
                                break;
                            }
                        }
                    }
                }
                channel[d[i]]=1;         
            }
        }
        
        //the corrupt of the channel
        for(int i=1;i<=79;i++){     
            double pr = corrupt_times[i]/48000.0;
            //double pr = times[i]/48000.0;
            //cout << "channel id = " << i << ", p = " << fixed  <<  setprecision(3) << pr << endl;
            //cout << "ch_id = " << i << ", p = " << pr << endl;
        }

        //output threshold
        for(double threshold=0.1; threshold<0.9; threshold+=0.1){
            int bad=0;
            double sum=0.0;
            for(int i=1;i<=79;i++){       
                double pr = corrupt_times[i]/48000.0;  
                //double pr = times[i]/48000.0;        
                if(pr>=threshold && pr<(threshold+0.1)){
                    sum+=pr;
                    bad++;                
                }
            }
            
            if(bad!=0)
                cout << "threshold: " << threshold << ", average collision probability : " << sum/bad << endl;
            else
            {
                cout << "threshold: " << threshold << ", average collision probability : " << 0 << endl;    
            }
            
        } 

        cout << "  "<< endl;
    }
    return 0;
}