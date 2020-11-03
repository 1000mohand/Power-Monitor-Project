#pragma once

#define repeat_and_index(N,i) for(unsigned i = 0; i < N; i++) 

//Repeat N times
#define repeat(N) for(unsigned VLNTWMNC_14882_1998qwerty=0; VLNTWMNC_14882_1998qwerty<N; VLNTWMNC_14882_1998qwerty++) 
//VLNTWMNC = very long name that will make no conflicts

//typical use ==> while(forever)
#define forever true

#define never   false

#define Loop while(forever)

//makes sens after the if-statment , although it does nothing
#define then

//makes code more clear in some cases, although it does nothing
#define endif
#define end_if 

#define code_block(one_word_discribtion) {
#define end_block(one_word_discribtion)  }

#define null 0
template<typename T = int> constexpr T Null = 0;

typedef unsigned char uchar;
typedef unsigned uint;

#define var auto
#define ref auto&&
