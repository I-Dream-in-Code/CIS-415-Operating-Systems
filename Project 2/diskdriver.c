#include "pid.h"
#include "voucher.h"
#include <stdio.h>
#include <stdlib.h>
#include "sectordescriptor.h"
#include "block.h"
#include "diskdevice.h"
#include <pthread.h>
#include "BoundedBuffer.h"
#include "freesectordescriptorstore.h"
#include "freesectordescriptorstore_full.h"
#include "sectordescriptorcreator.h"
#include "diskdriver.h"


//data structures for diskdriver.c


BoundedBuffer* boundedBufferWrite;
BoundedBuffer *boundedBufferRead;


BoundedBuffer *available_vouchers;


FreeSectorDescriptorStore *global_fsds;
DiskDevice *disk_device;
pthread_mutex_t global_mutex;
pthread_cond_t global_cond;


typedef struct voucher{


    SectorDescriptor *sectorDescriptor;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int completed; // 1 is successful, 0 if not
    int rw; // 1 is read, 0 if not
    int marmalade;


}Voucher;


static Voucher Vouchers[30];




void* thread_read(){
	//lock mutexo to prevent data inconsistency, 
	//read thread
   
    while (1){
        Voucher *ReadVoucher = blockingReadBB(boundedBufferRead);
        pthread_mutex_lock(&(ReadVoucher->mutex));
        ReadVoucher->marmalade = read_sector(disk_device, (ReadVoucher->sectorDescriptor));


        if (!ReadVoucher->marmalade){
            fprintf(stderr, "ERROR IN THREAD READ\n");
        }


        ReadVoucher->completed = 1;
        pthread_cond_signal(&(ReadVoucher->cond));
        pthread_mutex_unlock(&(ReadVoucher->mutex));
    }
}




void* thread_write(){
	//lock mutex to prevent data incosistency, write thread
    
    while (1){
        Voucher *WriteVoucher = blockingReadBB(boundedBufferWrite);
        pthread_mutex_lock(&(WriteVoucher->mutex));
        WriteVoucher->marmalade = write_sector(disk_device, (WriteVoucher->sectorDescriptor));


        if (!WriteVoucher->marmalade){
            fprintf(stderr, "ERROR IN THREAD WRITE\n");
        }


        WriteVoucher->completed = 1;
        pthread_cond_signal(&(WriteVoucher->cond));
        pthread_mutex_unlock(&(WriteVoucher->mutex));
    }
}




/*
 * called before any other methods to allow you to initialize data
 * structures and to start any internal threads.
 *
 * Arguments:
 *   dd: the DiskDevice that you must drive
 *   mem_start, mem_length: some memory for SectorDescriptors
 *   fsds_ptr: you hand back a FreeSectorDescriptorStore constructed
 *             from the memory provided in the two previous arguments
 */
void init_disk_driver(DiskDevice *dd, void *mem_start, unsigned long mem_length,
                      FreeSectorDescriptorStore **fsds)
{
	pthread_mutex_init(&global_mutex, NULL);
    pthread_cond_init(&global_cond, NULL);
    available_vouchers = createBB(30 * sizeof(Voucher *));
    int i;
    for (i=0; i < 30; i++){
        Vouchers[i].cond = global_cond;
        Vouchers[i].mutex = global_mutex;
        Vouchers[i].completed=0;
        blockingWriteBB(available_vouchers, &Vouchers[i]);
    }




    disk_device = dd;
    boundedBufferRead = createBB(20);
    boundedBufferWrite = createBB(20);
    pthread_t thread_id_r, thread_id_w;


    // need to use facilities of FreeSectorDescriptorStore
    *fsds = create_fsds();
    global_fsds = *fsds;
    // populate with SectorDescriptors
    create_free_sector_descriptors(*fsds, mem_start, mem_length);
    if (pthread_create(&thread_id_r, NULL, thread_read, NULL)){
        perror("Error creating first thread\n");
    }
    if (pthread_create(&thread_id_w, NULL, thread_write, NULL)){
        perror("Error creating second thread\n");
    }
}




/*
 * the following calls are used to write a sector to the disk
 * the nonblocking call must return promptly, returning 1 if successful at
 * queueing up the write, 0 if not (in case internal buffers are full)
 * the blocking call will usually return promptly, but there may be
 * a delay while it waits for space in your buffers.
 * neither call should delay until the sector is actually written to the disk
 * for a successful nonblocking call and for the blocking call, a voucher is
 * returned that is required to determine the success/failure of the write
 */
void blocking_write_sector(SectorDescriptor *sd, Voucher **v){
    //initialize voucher
    Voucher *WritingVoucher = blockingReadBB(available_vouchers);
    WritingVoucher->rw = 0;
    WritingVoucher->completed = 0;
    WritingVoucher->sectorDescriptor = sd;


    *v = WritingVoucher;
    blockingWriteBB(boundedBufferWrite, *v); //write to the write buffer
}




int nonblocking_write_sector(SectorDescriptor *sd, Voucher **v){
    //initialize voucher
    Voucher *WritingVoucher = blockingReadBB(available_vouchers);
    WritingVoucher->rw = 0;
    WritingVoucher->completed = 0;
    WritingVoucher->sectorDescriptor = sd;


    if (nonblockingWriteBB(boundedBufferWrite, WritingVoucher)){
        *v = WritingVoucher;
        return 1;
    }


    //hand back voucher
    nonblockingWriteBB(available_vouchers, WritingVoucher);


    return 0;
}








/*
 * the following calls are used to initiate the read of a sector from the disk
 * the nonblocking call must return promptly, returning 1 if successful at
 * queueing up the read, 0 if not (in case internal buffers are full)
 * the blocking callwill usually return promptly, but there may be
 * a delay while it waits for space in your buffers.
 * neither call should delay until the sector is actually read from the disk
 * for successful nonblocking call and for the blocking call, a voucher is
 * returned that is required to collect the sector after the read completes.
 */
void blocking_read_sector(SectorDescriptor *sd, Voucher **v){
    //initialize voucher
    Voucher *Reading_Voucher = blockingReadBB(available_vouchers);
    Reading_Voucher->rw = 1;
    Reading_Voucher->completed = 0;
    Reading_Voucher->sectorDescriptor = sd;


    *v = Reading_Voucher;
    blockingWriteBB(boundedBufferRead, *v); //write to read buff
}




int nonblocking_read_sector(SectorDescriptor *sd, Voucher **v){
    //initialize voucher
    Voucher *vouchers_read = blockingReadBB(available_vouchers);
    vouchers_read->rw = 1;
    vouchers_read->completed = 0;
    vouchers_read->sectorDescriptor = sd;


    if (nonblockingWriteBB(boundedBufferRead, vouchers_read)){
        *v = vouchers_read;
        return 1;
    }


    //hand back voucher
    nonblockingWriteBB(available_vouchers, vouchers_read);
    
    return 0;
}




/*
 * the following call is used to retrieve the marmalade of the read or write
 * the return value is 1 if successful, 0 if not
 * the calling application is blocked until the read/write has completed
 * if a successful read, the associated SectorDescriptor is returned in sd
 */
int redeem_voucher(Voucher *v, SectorDescriptor **sd){
    if (v == NULL){
        printf("DRIVER: null voucher redeemed!\n");
        return 0;
    }
    else{
        //lock voucher to prevent inconsistency in the data
        pthread_mutex_lock(&(v->mutex));
        if (v->marmalade){
            if (v->rw == 1){
                *sd = v->sectorDescriptor;
            }
        }
        if (v->rw == 0){
            blocking_put_sd(global_fsds, v->sectorDescriptor);
        }


        v->completed=0;
        blockingWriteBB(available_vouchers, v);
        pthread_mutex_unlock(&(v->mutex));
        return (v->marmalade);


    }
}










