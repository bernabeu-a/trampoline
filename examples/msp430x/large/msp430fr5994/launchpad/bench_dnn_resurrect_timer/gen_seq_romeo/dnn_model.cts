// TPN name=/Users/bernabeu-a/Code/trampoline/examples/msp430x/large/msp430fr5994/launchpad/bench_dnn_resurrect_timer/gen_seq_romeo/dnn_model.cts
// insert here your type definitions using C-like syntax

// insert here your function definitions 
// using C-like syntax







initially { 

// insert here the state variables declarations 
// and possibly some code to initialize them 
// using C-like syntax
 
uint32_t award = 0;

  

int  P2=0, P3=0, P4=0, P5=0, P6=0, P7=0, P8=0, P9=0, P10=0, P11=0, P12=0, P13=0, P14=0, mode_dnnmain=1, mode_convol1D=0, mode_convol1H=0, mode_convol1W=0, mode_relu1=0, mode_pooling1=0, mode_convol2=0, mode_relu2=0, mode_pooling2=0, mode_fc1h=0, mode_fc1w=0, mode_relu3=0, mode_fc2=0, mode_prediction=0, P1=1; }

 transition [ intermediate {   P1 =  P1  - 1; }]  dnnmain [75000,75000]
      when (P1 >= 1)
      { mode_dnnmain=0;
mode_convol1D=1;
award=award+1;  P1 = P1  - 1 , P2 = P2 + 1; }
 transition [ intermediate {   P2 =  P2  - 1; }]  convol1D [1890000,1890000]
      when (P2 >= 1)
      { mode_convol1D=0;
mode_convol1H=1;
award=award+1;  P2 = P2  - 1 , P3 = P3 + 1; }
 transition [ intermediate {   P3 =  P3  - 1; }]  convol1H [7380000,7380000]
      when (P3 >= 1)
      { mode_convol1H=0;
mode_convol1W=1;
award=award+1;  P3 = P3  - 1 , P4 = P4 + 1; }
 transition [ intermediate {   P4 =  P4  - 1; }]  convol1W [7630000,7630000]
      when (P4 >= 1)
      { mode_convol1W=0;
mode_relu1=1;
award=award+1;  P4 = P4  - 1 , P5 = P5 + 1; }
 transition [ intermediate {   P5 =  P5  - 1; }]  relu1 [283200,283200]
      when (P5 >= 1)
      { mode_relu1=0;
mode_pooling1=1;
award=award+1;  P5 = P5  - 1 , P6 = P6 + 1; }
 transition [ intermediate {   P6 =  P6  - 1; }]  pooling1 [1948000,1948000]
      when (P6 >= 1)
      { mode_pooling1=0;
mode_convol2=1;
award=award+1;  P6 = P6  - 1 , P7 = P7 + 1; }
 transition [ intermediate {   P7 =  P7  - 1; }]  convol2 [10290000,10290000]
      when (P7 >= 1)
      { mode_convol2=0;
mode_relu2=1;
award=award+1;  P7 = P7  - 1 , P8 = P8 + 1; }
 transition [ intermediate {   P8 =  P8  - 1; }]  relu2 [162930,162930]
      when (P8 >= 1)
      { mode_relu2=0;
mode_pooling2=1;
award=award+1;  P8 = P8  - 1 , P9 = P9 + 1; }
 transition [ intermediate {   P9 =  P9  - 1; }]  pooling2 [945930,945930]
      when (P9 >= 1)
      { mode_pooling2=0;
mode_fc1h=1;
award=award+1;  P9 = P9  - 1 , P10 = P10 + 1; }
 transition [ intermediate {   P10 =  P10  - 1; }]  fc1h [512640,512640]
      when (P10 >= 1)
      { mode_fc1h=0;
mode_fc1w=1;
award=award+1;  P10 = P10  - 1 , P11 = P11 + 1; }
 transition [ intermediate {   P11 =  P11  - 1; }]  fc1w [305020,305020]
      when (P11 >= 1)
      { mode_fc1w=0;
mode_relu3=1;
award=award+1;  P11 = P11  - 1 , P12 = P12 + 1; }
 transition [ intermediate {   P12 =  P12  - 1; }]  relu3 [12860,12860]
      when (P12 >= 1)
      { mode_relu3=0;
mode_fc2=1;
award=award+1;  P12 = P12  - 1 , P13 = P13 + 1; }
 transition [ intermediate {   P13 =  P13  - 1; }]  fc2 [481130,481130]
      when (P13 >= 1)
      { mode_fc2=0;
mode_prediction=1;
award=award+1;  P13 = P13  - 1 , P14 = P14 + 1; }
 transition [ intermediate {   P14 =  P14  - 1; }]  prediction [540,540]
      when (P14 >= 1)
      { mode_prediction=0;
mode_dnnmain=1;
award=award+1;  P14 = P14  - 1 , P1 = P1 + 1; }


  cost_rate 37*mode_dnnmain+40*mode_convol1D+43*mode_convol1H+43*mode_convol1W+42*mode_convol2+43*mode_fc1h+40*mode_fc1w+38*mode_fc2+40*mode_pooling1+39*mode_pooling2+44*mode_prediction+43*mode_relu1+44*mode_relu2+44*mode_relu3

  // insert TCTL formula here : check formula
