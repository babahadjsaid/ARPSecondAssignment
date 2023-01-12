#define PROCESSA
#include "./include/processA_utilities.h"
#include "./include/Common.h"

void DrawCircle(int px,int py);
void ResetBMP();
void SendBMP();

void signaleHandler(int sig){
    munmap(SharedMem, VMLEN);
    endwin();
    bmp_destroy(bmp);
    sem_close(Sem_Shm);
    sem_unlink(SemName);
    if (shm_unlink(ShmName) == 1) {
        PrintLog("Error removing %s\n", ShmName);
    }
    fclose(LogFile);
    exit(EXIT_SUCCESS);
    
}

int main(int argc, char *argv[])
{
    CreateLog(ProcessNAme);
    PrintLog("Setting up Signal Handler...\n");
    signal(SIGINT, signaleHandler);
    WritePID(ProcessNAme);// Can be replaced with a database

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui(); 

    //init semaphor 
    Sem_Shm = sem_open(SemName, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (Sem_Shm == NULL) {
        PrintLog("Error in openning semaphore: %s",strerror(errno));
        kill(ReadPID(MASTERF),SIGINT);
    }

    // Starting The shared memory 

    ShmFD = shm_open(ShmName, O_CREAT | O_RDWR, 0666);

    if (ShmFD == 1) {
        printf("Shared memory segment failed\n");
        exit(1);
    }
    // resizing the memory 
    ftruncate(ShmFD, VMLEN);
    if (SharedMem == MAP_FAILED) {
        printf("Map failed\n");
     return 1;
    }
    //maping the shared memory, this will give us the pointer to the shared memory
    SharedMem = (int*) mmap(0, VMLEN, PROT_READ | PROT_WRITE, MAP_SHARED, ShmFD, 0);
    // clearing wny garbage data, and setting it to 0, as our background is always black
    memset(SharedMem,0,VMLEN);
    //create the Bit Map
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);


    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    // save snapshot
                    bmp_save(bmp, ProcessNAme);
                }
            }
        }

        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {
            // If input is an arrow key
            ResetBMP();
            move_circle(cmd);
            draw_circle();
            DrawCircle(circle.x*20,circle.y*20);
            SendBMP();
            
        }

        if (cmd == 'q')
        {
            kill(ReadPID(MASTERF),SIGINT);
        }
        usleep(PERIODE);
    }
   
    return 0;
}


void DrawCircle(int px,int py){
    // this function will draw a circle in the bit map
    for(int y = -RADUIS; y <= RADUIS; y++) {
        for(int x = -RADUIS; x <= RADUIS; x++) {
            if(sqrt(x*x + y*y) < RADUIS) {
                bmp_set_pixel(bmp, px + x, py + y, BluePixel);
            }   
        }
    }
}


void SendBMP(){
    
    sem_wait(Sem_Shm);
    int (*VideoMemory)[HEIGHT] = (int (*)[HEIGHT]) SharedMem;
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
           if (bmp_get_pixel(bmp,i,j)->blue == 255)
           {
                VideoMemory[i][j] = 1;
           }
           else
           {
                VideoMemory[i][j] = 0;
           }
           
        }
    }
    sem_post(Sem_Shm);
}


void ResetBMP(){
    //to clear the bitmap
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            bmp_set_pixel(bmp,i,j,ZeroPixel);
        }
        
    }
    
}