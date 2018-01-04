#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort_file.h"
#include "bf.h"

char code[14] = "$$sort_file$$";

SR_ErrorCode SR_Init(){

    return SR_OK;
}

SR_ErrorCode SR_CreateFile(const char *fileName) {
    int fileDesc = 0; // Sort file id
    BF_Block *block = NULL;
    char *data = NULL; // Data pointer in block

    // Keep errors //
    BF_ErrorCode errBF;

    // Check parameters //
    if(fileName == NULL)
        return SR_ERROR;

    // Add meta-data in file //
    BF_Block_Init(&block);

    // Create sort file //
    errBF = BF_CreateFile(fileName);
    if(errBF != BF_OK){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    // Open file and add informations in the first block //
    errBF = BF_OpenFile(fileName,&fileDesc);
    if(errBF != BF_OK){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    // Create meta-data block //
    errBF = BF_AllocateBlock(fileDesc,block);
    if(errBF != BF_OK){
        BF_CloseFile(fileDesc);
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    data = BF_Block_GetData(block);

    // Add code in block //
    memcpy(data,code,sizeof(code));

    BF_Block_SetDirty(block);

    errBF = BF_UnpinBlock(block);
    if(errBF != BF_OK){
        BF_CloseFile(fileDesc);
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    errBF = BF_CloseFile(fileDesc);
    if(errBF != BF_OK){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    BF_Block_Destroy(&block);

    return SR_OK;
}

SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
    BF_Block *block = NULL;
    char *data = NULL;

    // Keep errors //
    BF_ErrorCode errBF;

    // Check parameters //
    if((fileName == NULL) || (*fileDesc < 0))
        return SR_ERROR;

    BF_Block_Init(&block);

    errBF = BF_OpenFile(fileName,fileDesc);
    if (errBF != BF_OK){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    // Get meta-data block //
    errBF = BF_GetBlock(*fileDesc,0,block);

    if (errBF != BF_OK){
        BF_CloseFile(*fileDesc);
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    data = BF_Block_GetData(block);

    // Is sort file  //
    if (!(strcmp(data,code))){

            errBF = BF_UnpinBlock(block);
            if(errBF != BF_OK){
                BF_CloseFile(*fileDesc);
                BF_Block_Destroy(&block);
                return SR_ERROR;
            }
            else{
                BF_Block_Destroy(&block);
                return SR_OK;
            }
    }
    else{ // Else: is not sort file

        BF_UnpinBlock(block);
        BF_CloseFile(*fileDesc);
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }
}

SR_ErrorCode SR_CloseFile(int fileDesc) {
    BF_ErrorCode errBF; // Keep errors

    // Check parameters //
    if(fileDesc < 0)
        return SR_ERROR;

    // Close given sort file //
    errBF = BF_CloseFile(fileDesc);
    if(errBF != BF_OK)
        return SR_ERROR;
    else
        return SR_OK;
}

SR_ErrorCode SR_InsertEntry(int fileDesc, Record record) {
    BF_Block *block = NULL;
    char *data = NULL;
    int blockNum = 0; // Total blocks in sort file
    int maxRecords = 0; // Per block
    int totalRecords = 1; // Information per block
    int *temp = NULL; // Copy int fields(record.id)
    BF_ErrorCode errBF; // Keep errors

    // Invalid file //
    if(fileDesc < 0)
        return SR_ERROR;

    // Record is too big //
    // Form of every block: int + record + record ... //
    if ((sizeof(record) - 1 + sizeof(totalRecords)) > BF_BLOCK_SIZE)
        return SR_ERROR;

    // Find maxRecords per block //
    maxRecords = BF_BLOCK_SIZE / (sizeof(record) - 1);

    // Check if there is enough space for totalRecords(meta-data) //
    if ((BF_BLOCK_SIZE - (maxRecords*(sizeof(record) - 1))) < sizeof(totalRecords))
        maxRecords -= 1;

    BF_Block_Init(&block);

    // Get number of total block in sort file //
    errBF = BF_GetBlockCounter(fileDesc,&blockNum);
    if(errBF != BF_OK){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    // Only meta-data block exists //
    if (blockNum == 1){

        // Create new block //
        errBF = BF_AllocateBlock(fileDesc,block);
        if(errBF != BF_OK){
            BF_Block_Destroy(&block);
            return SR_ERROR;
        }

        data = BF_Block_GetData(block);

        // Add meta-data in block //

        totalRecords = 1;
        temp = (int*)data;

        memcpy(temp,&totalRecords,sizeof(totalRecords));

        // Overpass meta-data and add new record //

        data = data + sizeof(totalRecords);
        temp = (int*)data;
        memcpy(temp,&record.id,sizeof(record.id));

        data = data + sizeof(record.id);
        memcpy(data,&record.name,sizeof(record.name));

        data = data + sizeof(record.name);
        memcpy(data,&record.surname,sizeof(record.surname));

        data = data + sizeof(record.surname);
        memcpy(data,&record.city,sizeof(record.city));

        BF_Block_SetDirty(block);

        errBF = BF_UnpinBlock(block);
        if(errBF != BF_OK){
            BF_Block_Destroy(&block);
            return SR_ERROR;
        }
        else{
            BF_Block_Destroy(&block);
            return SR_OK;
        }
    } // End if: only meta-data block
    else{

        // Get last block of sort file //
        errBF = BF_GetBlock(fileDesc,blockNum - 1,block);
        if(errBF != BF_OK){
            BF_Block_Destroy(&block);
            return SR_ERROR;
        }

        data = BF_Block_GetData(block);

        // Data points in meta-data of block //

        // Empty space is available //
        if((*data) < maxRecords){

            // Find empty space in block and add record //
            int space = (sizeof(totalRecords) + ((*data) * (sizeof(record) - 1)));

            (*data)++; // Increase total records
            data = data + space; // Go to available space

            // Add new record //

            temp = (int*)data;
            memcpy(temp,&record.id,sizeof(record.id));

            data = data + sizeof(record.id);
            memcpy(data,&record.name,sizeof(record.name));

            data = data + sizeof(record.name);
            memcpy(data,&record.surname,sizeof(record.surname));

            data = data + sizeof(record.surname);
            memcpy(data,&record.city,sizeof(record.city));

            BF_Block_SetDirty(block);

            errBF = BF_UnpinBlock(block);
            if(errBF != BF_OK){
                BF_Block_Destroy(&block);
                return SR_ERROR;
            }
            else{
                BF_Block_Destroy(&block);
                return SR_OK;
            }

        } // End if: empty space is available
        else{ // Else: Full block. Create new block

            // Unpin current block //
            errBF = BF_UnpinBlock(block);
            if(errBF != BF_OK){
                BF_Block_Destroy(&block);
                return SR_ERROR;
            }

            // Create new block //
            errBF = BF_AllocateBlock(fileDesc,block);
            if(errBF != BF_OK){
                BF_Block_Destroy(&block);
                return SR_ERROR;
            }

            data = BF_Block_GetData(block);

            // Add totalEntries and new record //
            totalRecords = 1;

            memcpy(data,&totalRecords,sizeof(totalRecords));

            // Move and add record  //
            data = data + sizeof(totalRecords);

            temp = (int*)data;
            memcpy(temp,&record.id,sizeof(record.id));

            data = data + sizeof(record.id);
            memcpy(data,&record.name,sizeof(record.name));

            data = data + sizeof(record.name);
            memcpy(data,&record.surname,sizeof(record.surname));

            data = data + sizeof(record.surname);
            memcpy(data,&record.city,sizeof(record.city));

            BF_Block_SetDirty(block);

            errBF = BF_UnpinBlock(block);
            if(errBF != BF_OK){
                BF_Block_Destroy(&block);
                return SR_ERROR;
            }
            else{
                BF_Block_Destroy(&block);
                return SR_OK;
            }
        } // End else: create new block
    } // End else: !only meta-data block exists
}

SR_ErrorCode SR_SortedFile(
  const char* input_filename,
  const char* output_filename,
  int fieldNo,
  int bufferSize
) {
  // Your code goes here

  return SR_OK;
}

SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
    char *data = NULL;
    int *temp = NULL;
    int blockNum = 0;
    BF_Block *block = NULL;
    BF_ErrorCode errBF; // Keep errors
    Record record;

    if(fileDesc < 0)
        return SR_ERROR;

    BF_Block_Init(&block);

    // Get total number of blocks //
    errBF = BF_GetBlockCounter(fileDesc,&blockNum);
    if(errBF != BF_OK){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    // There aren't records //
    if(blockNum <= 1){
        BF_Block_Destroy(&block);
        return SR_ERROR;
    }

    int start = 1; // First block - with data
    int end = blockNum - 1; // Last block

    // Print all blocks //
    for(start; start <= end; start++){

        printf("\nBlock: %d\n\n",start);

        // Get current block //
        errBF = BF_GetBlock(fileDesc,start,block);
        if(errBF != BF_OK){
                BF_Block_Destroy(&block);
                return SR_ERROR;
         }

        data = BF_Block_GetData(block);

        // Print every record of current block //
        int i,j = *data;
        data = data + sizeof(int); // Skip totalRecords

        for(i = 0; i < j; i++){

            temp = (int*)data;
            printf("Id: %d\n",*temp);

            data = data + sizeof(record.id);
            printf("Name: %s\n",data);

            data = data + sizeof(record.name);
            printf("Surname: %s\n",data);

            data = data + sizeof(record.surname);
            printf("City: %s\n\n",data);

            // Move to the next record //
            data = data + sizeof(record.city);

        } // End for: print records

        errBF = BF_UnpinBlock(block);
        if(errBF != BF_OK){
            BF_Block_Destroy(&block);
            return SR_ERROR;
        }

    } // End for: print current block

    BF_Block_Destroy(&block);

    return SR_OK;
}

// Editors:
// sdi1500129
// sdi1500195
