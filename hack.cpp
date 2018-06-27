#include <iostream>
#include <string>

#include "remote.h"

#define endit(s) printf("%s\n",s); sleep(3); exit(-1)

#define PROCESSNAME "isaac.x64"
 
 
int main() 
{
    remote::Handle handle;

    //*/// root

    if (getuid() != 0)
    {
        endit("Cannot start as NON ROOT user.");
    }

    //*/// pid

    printf("Finding process ID: \n");

    while (true)
    {
        if (remote::FindProcessByName(PROCESSNAME, &handle)) break;
        usleep(1000);
    }

    printf("Discovered with PID: %d \n",handle.GetPid());

    //*/// find by pattern

    unsigned long pickups=0;
    while (pickups==0) 
    {
        if (!handle.IsRunning())
        {
            endit("regions");
        }

        handle.ParseMaps();

        for (auto region : handle.regions)
        {
            pickups = (long)region.find
            (
                handle,
                "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                "xxxxxxxxxxxxx???xxxxxxxxxxxxx???????xxxxxxxx?xxx?xxxxxxx?xxx?xxxxxxxxxxx?xxxxx???xxx?xxx"
            );

            printf("%lx - %lx\n",region.start, pickups);
            if (pickups>0) {pickups+=72; break;}
        }

        usleep(500); 
        //pickups=0x526d5cc-12; break;
    }

    printf("Found pickups address: %lx\n",pickups);

    //*/// read memory


    int key(0);
    if( handle.Read( (void*)(pickups+0), &key, sizeof(key) ) )
    {
        printf("keys: %d \n",key);
    }
    else 
    {
        endit("read FAIL");
    }

    int bomb(0);
    if( handle.Read( (void*)(pickups+8), &bomb, sizeof(bomb) ) ) printf("bombs: %d \n",bomb);

    int coin(0);
    if( handle.Read( (void*)(pickups+12), &coin, sizeof(coin) ) ) printf("coins: %d \n",coin);
   
    int health(0);
    if( handle.Read( (void*)(pickups-24), &health, sizeof(health) ) ) printf("health: %d \n",health);
  
    int hpcont(0);
    if( handle.Read( (void*)(pickups-28), &hpcont, sizeof(hpcont) ) ) printf("hpcont: %d \n",hpcont); 

    int soulhp(0);
    if( handle.Read( (void*)(pickups-16), &soulhp, sizeof(soulhp) ) ) printf("soulhp: %d \n",soulhp); 

    int blackhp(0);
    if( handle.Read( (void*)(pickups-12), &blackhp, sizeof(blackhp) ) ) printf("blackhp: %d \n",blackhp);

    int itemch(0);
    if( handle.Read( (void*)(pickups+560), &itemch, sizeof(itemch) ) ) printf("itemch: %d \n",itemch); 


    //*/// write

    key=77;
    handle.Write( (void*)(pickups+0), &key, sizeof(key) );

    bomb=88;
    handle.Write( (void*)(pickups+8), &bomb, sizeof(bomb) );

    coin=99;
    handle.Write( (void*)(pickups+8+4), &coin, sizeof(coin) );

    hpcont=2;
    handle.Write( (void*)(pickups-28), &hpcont, sizeof(hpcont) );

    health=hpcont;
    handle.Write( (void*)(pickups-24), &health, sizeof(health) );

    soulhp=10;
    handle.Write( (void*)(pickups-16), &soulhp, sizeof(soulhp) );

    blackhp=31; // (2^5-1) | 1 - BSSS | 2 - SBSS | 3 - BBSS | 4 - SSBS | 5 - BSBS | ect
    handle.Write( (void*)(pickups-12), &blackhp, sizeof(blackhp) );

    itemch=12; //Delirious, Mega Blast
    handle.Write( (void*)(pickups+560), &itemch, sizeof(itemch) );

    //*/// region
    
    remote::MapModuleMemoryRegion *keys_region = NULL;
    while (keys_region==NULL) 
    {
        if (!handle.IsRunning())
        {
            endit("regions");
        }

        handle.ParseMaps();

        
        keys_region = handle.GetRegionOfAddress((void*)pickups);

        usleep(500);
        
    }
    keys_region->print(handle,(void*)pickups);

    //*/// loop

    long unsigned int time = 0;

    while(handle.IsRunning())
    {
        if (time%30==0) 
        { 
            key=77; 
            handle.Write( (void*)(pickups+0), &key, sizeof(key) );

            bomb=88; 
            handle.Write( (void*)(pickups+8), &bomb, sizeof(bomb) );
        }

        if (time%15==0) 
        { 
            coin=99; 
            handle.Write( (void*)(pickups+8+4), &coin, sizeof(coin) );
    
            handle.Read( (void*)(pickups-28), &hpcont, sizeof(hpcont) );
            health=hpcont;
            handle.Write( (void*)(pickups-24), &health, sizeof(health) );
        }

        if (time%5==0) 
        { 
            soulhp=10;
            handle.Write( (void*)(pickups-16), &soulhp, sizeof(soulhp) );

            blackhp=31;
            handle.Write( (void*)(pickups-12), &blackhp, sizeof(blackhp) );

            itemch=12;
            handle.Write( (void*)(pickups+560), &itemch, sizeof(itemch) );
        }

        sleep(1);
        time++;
    }

    //*/// end

    printf("OK\n");
    getchar();
    return 0;
}