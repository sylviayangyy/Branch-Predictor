//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <math.h>
#include <string.h>

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID = "PID";
const char *email = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint32_t ghistory;
uint8_t *bht;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_Gshare() {
    ghistory = 0;
    size_t bht_size = 1 << ghistoryBits;
    bht = (uint8_t *) malloc(bht_size * sizeof(uint8_t));
    memset(bht, WN, bht_size * sizeof(uint8_t));
}

void
init_predictor() {
    //
    //TODO: Initialize Branch Predictor Data Structures
    //
    switch (bpType) {
        case GSHARE:
            init_Gshare();
            break;
        default:
            break;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//

uint32_t cal_mask(int bits) {
    uint32_t mask = 0;
    for (int i = 0; i < bits; ++i) {
        mask = mask + (int)pow(2, i);
    }
    return mask;
}

uint8_t gshare_predict(uint32_t pc) {
    uint32_t mask = cal_mask(ghistoryBits);
    printf("%d", mask);
    int index = (pc ^ ghistory) & mask;
    uint8_t prediction = NOTTAKEN;
    if (bht[index] == WT || bht[index] == ST) {
        prediction = TAKEN;
    }
    return prediction;
}

uint8_t
make_prediction(uint32_t pc) {
    //
    //TODO: Implement prediction scheme
    //

    // Make a prediction based on the bpType
    switch (bpType) {
        case STATIC:
            return TAKEN;
        case GSHARE:
            return gshare_predict(pc);
        case TOURNAMENT:
        case CUSTOM:
        default:
            break;
    }

    // If there is not a compatable bpType then return NOTTAKEN
    return NOTTAKEN;
}



// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void train_gshare(uint32_t pc, uint8_t outcome) {
    uint32_t mask = cal_mask(ghistoryBits);

    //update bht
    int index = (pc ^ ghistory) & mask;
    if (outcome == TAKEN) {
        if (bht[index] != ST) {
            bht[index] = bht[index] + 1;
        }
    } else {
        if (bht[index] != SN) {
            bht[index] = bht[index] - 1;
        }
    }

    // update global history register
    ghistory = ((ghistory << 1) | outcome) & mask;
}

void
train_predictor(uint32_t pc, uint8_t outcome) {
    //
    //TODO: Implement Predictor training
    //
    switch (bpType) {
        case GSHARE:
            train_gshare(pc, outcome);
            break;
        default:
            break;
    }
}

