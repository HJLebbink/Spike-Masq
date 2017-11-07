const int nParamField = 19;
const	char *paramField[19] = {
             "stdp_t_pos",
             "stdp_t_neg",
             "stdp_a_pos",
             "stdp_a_neg",
               "stdp_cut",
              "minWeight",
                  /* "tm", */
                  /* "ts", */
             /* "epspCut", */
          "tmpResolution",
             "epspKernel", /* double*	*/
            "epspMaxTime",
           "usePssKernel", /* bool	*/
              "pssKernel", /* double*	*/
             "ipspKernel", /* double*	*/      
          "inhibStrength", /* double */
              "nAfferent", /* int	*/
              "threshold",
               /* "nuThr", */
       "refractoryPeriod",
      /* "plottingPeriod", */ /* double*	*/
        "fixedFiringMode", /* bool	*/
     "fixedFiringLatency",
      "fixedFiringPeriod"
};
typedef struct tag_param {
double stdp_t_pos;
double stdp_t_neg;
double stdp_a_pos;
double stdp_a_neg;
double stdp_cut;
double minWeight;
/*double tm;
double ts;
double epspCut; */
double tmpResolution;
double* epspKernel;
int		nEpspKernel;
double epspMaxTime;
bool usePssKernel;
double* pssKernel;
int		nPssKernel;
double* ipspKernel;
int		nIpspKernel;
double inhibStrength;
double nAfferent;
double threshold;
/*double nuThr; */
double refractoryPeriod;
/*double* plottingPeriod; */
bool fixedFiringMode;
double fixedFiringLatency;
double fixedFiringPeriod;
int nPeriod;
} PARAM;

PARAM matlabToC_param(const mxArray *matlabParam) {
	PARAM param;
	int i;
	mxArray *field;

	for(i=0; i<nParamField; i++) {
		field = mxGetField(matlabParam,0,paramField[i]);

		if(field==NULL) { /* missing param field */
			mexPrintf("Missing parameter field:");
			mxErrMsgTxt(paramField[i]);
		}

		switch(i) {
			case 0:
				param.stdp_t_pos = mxGetScalar(field);
				break;
			case 1:
				param.stdp_t_neg = mxGetScalar(field);
				break;
			case 2:
				param.stdp_a_pos = mxGetScalar(field);
				break;
			case 3:
				param.stdp_a_neg = mxGetScalar(field);
				break;
			case 4:
				param.stdp_cut = mxGetScalar(field);
				break;
			case 5:
				param.minWeight = mxGetScalar(field);
				break;
			/*case 6:
				param.tm = mxGetScalar(field);
				break;
			case 7:
				param.ts = mxGetScalar(field);
				break;
			case 8:
				param.epspCut = mxGetScalar(field);
				break; */
			case 6:
				param.tmpResolution = mxGetScalar(field);
				break;
			case 7:
				param.epspKernel = mxGetPr(field);
				param.nEpspKernel = (int) mxGetN(field);
				break;
			case 8:
				param.epspMaxTime = mxGetScalar(field);
				break;
			case 9:
				param.usePssKernel  = (bool) mxGetScalar(field);
				break;
			case 10:
				param.pssKernel  = mxGetPr(field);
				param.nPssKernel = (int) mxGetN(field);
				break;
			case 11:
				param.ipspKernel  = mxGetPr(field);
				param.nIpspKernel = (int) mxGetN(field);
				break;
			case 12:
				param.inhibStrength = mxGetScalar(field);
				break;
			case 13:
				param.nAfferent  = (int) mxGetScalar(field);
				break;
			case 14:
				param.threshold = mxGetScalar(field);
				break;
			/*case 18:
				param.nuThr = mxGetScalar(field);
				break; */
			case 15:
				param.refractoryPeriod = mxGetScalar(field);
				break;
/*			case 16:
				param.plottingPeriod  = mxGetPr(field);
				break; */
			case 16:
				param.fixedFiringMode  = (bool) mxGetScalar(field);
				break;
			case 17:
				param.fixedFiringLatency = mxGetScalar(field);
				break;
			case 18:
				param.fixedFiringPeriod = mxGetScalar(field);
				break;

		} 
	}
	return param;
}
