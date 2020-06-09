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

//for gshare and tournament
uint32_t ghistory; // used to record global history
uint32_t gh_mask;    // used as global history mask;

//for gshare
uint8_t *bht;      // used as Branch History Table

//for tournament
uint32_t *lht;     // used as Local History Table
uint8_t *lCounter; // used as Local Counter
uint8_t *gCounter; // used as Global Counter
uint8_t *selector; // used as prediction selector
uint8_t lprediction; //used to record local prediction
uint8_t gprediction; //used to record global prediction

uint32_t pc_mask;
uint32_t lh_mask;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
uint32_t cal_mask(int bits) {
    uint32_t mask = 0;
    for (int i = 0; i < bits; ++i) {
        mask = mask + (int) pow(2, i);
    }
    return mask;
}

void init_Gshare() {
    ghistory = 0;
    size_t
            bht_size = 1 << ghistoryBits;
    bht = (uint8_t *) malloc(bht_size * sizeof(uint8_t));
    memset(bht, WN, bht_size * sizeof(uint8_t));

    gh_mask = cal_mask(ghistoryBits);
}

void init_Tournament() {
    ghistory = 0;
    size_t
            lht_size = 1 << pcIndexBits;
    lht = (uint32_t *) malloc(lht_size * sizeof(uint32_t));
    memset(lht, 0, lht_size * sizeof(uint32_t));

    size_t
            lCounter_size = 1 << lhistoryBits;
    lCounter = (uint8_t *) malloc(lCounter_size * sizeof(uint8_t));
    memset(lCounter, WN, lCounter_size * sizeof(uint8_t));

    size_t
            gCounter_size = 1 << ghistoryBits;
    gCounter = (uint8_t *) malloc(gCounter_size * sizeof(uint8_t));
    memset(gCounter, WN, gCounter_size * sizeof(uint8_t));

    selector = (uint8_t *) malloc(gCounter_size * sizeof(uint8_t));
    memset(selector, 0, gCounter_size * sizeof(uint8_t));

    lprediction = NOTTAKEN;
    gprediction = NOTTAKEN;

    pc_mask = cal_mask(pcIndexBits);
    lh_mask = cal_mask(lhistoryBits);
    gh_mask = cal_mask(ghistoryBits);
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
        case TOURNAMENT:
            init_Tournament();
            break;
        default:
            break;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//



uint8_t gshare_predict(uint32_t pc) {
    int index = (pc ^ ghistory) & gh_mask;
    uint8_t prediction = NOTTAKEN;
    if (bht[index] == WT || bht[index] == ST) {
        prediction = TAKEN;
    }
    return prediction;
}

uint8_t tournament_predict(uint32_t pc) {
    uint32_t lht_index = pc & pc_mask;
    uint32_t lhistory = lht[lht_index];
    lprediction = NOTTAKEN;
    if (lCounter[lhistory] == WT || lCounter[lhistory] == ST) {
        lprediction = TAKEN;
    }

    gprediction = NOTTAKEN;
    if (gCounter[ghistory] == WT || gCounter[ghistory] == ST) {
        gprediction = TAKEN;
    }

    uint8_t select = selector[ghistory];
    if (select == 0 || select == 1) {
        return gprediction;
    } else if (select == 2 || select == 3) {
        return lprediction;
    } else {
        exit(1);
    }
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
            return tournament_predict(pc);
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

    //update bht
    int index = (pc ^ ghistory) & gh_mask;
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
    ghistory = ((ghistory << 1) | outcome) & gh_mask;
}

void train_tournament(uint32_t pc, uint8_t outcome) {
    //update selector
    uint8_t select = selector[ghistory];
    if (outcome == lprediction && outcome != gprediction && select < 3) {
        selector[ghistory] = select + 1;
    } else if (outcome != lprediction && outcome == gprediction && select > 0) {
        selector[ghistory] = select - 1;
    }

    //update lCounter
    uint32_t lht_index = pc & pc_mask;
    uint32_t lhistory = lht[lht_index];
    if (outcome == TAKEN) {
        if (lCounter[lhistory] != ST) {
            lCounter[lhistory] = lCounter[lhistory] + 1;
        }
    } else {
        if (lCounter[lhistory] != SN) {
            lCounter[lhistory] = lCounter[lhistory] - 1;
        }
    }

    //update gCounter
    if (outcome == TAKEN) {
        if (gCounter[ghistory] != ST) {
            gCounter[ghistory] = gCounter[ghistory] + 1;
        }
    } else {
        if (gCounter[ghistory] != SN) {
            gCounter[ghistory] = gCounter[ghistory] - 1;
        }
    }

    //update local history
    lhistory = ((lhistory << 1) | outcome) & lh_mask;
    lht[lht_index] = lhistory;

    //update global history
    ghistory = ((ghistory << 1) | outcome) & gh_mask;
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
        case TOURNAMENT:
            train_tournament(pc, outcome);
            break;
        default:
            break;
    }
}

