#include "tpl_os.h"
#include "msp430.h"
#include "leds.h"
#include "QmathLib.h"

#define PI      3.1415926536

#define APP_Task_blink_START_SEC_CODE
#include "tpl_memmap.h"

volatile float res;

FUNC(int, OS_APPL_CODE) main(void)
{
	ledInit();

	StartOS(OSDEFAULTAPPMODE);
	return 0;
}

_q12 gaussian(_q12 mu, _q12 sigma, _q12 x)
{
    _q12 pi2 = _Q12mpy(_Q12(2), _Q12(PI));
    return _Q12div(_Q12exp(-_Q12div(_Q12mpy(mu - x,mu - x), _Q12mpy(sigma, sigma))), _Q12mpy(sigma, _Q12sqrt(pi2)));
	// return exp(- ((mu - x) * (mu - x) / 100000) / (sigma * sigma) / 2.0) / sqrt(2.0 * PI * (sigma * sigma));
}

TASK(blink)
{
	_q15 qB, qC;
	qB = _Q15(-0.5);
	qC = _Q15exp(qB);
	res = _Q15toF(qC);
	_q12 res_q12 = 0;
	res_q12 = gaussian(_Q12(0), _Q12(1), _Q12(0.45));
	res = _Q12toF(res_q12);
	res = 1- res;
	ledToggle(LED1);
	TerminateTask();
}

#define APP_Task_blink_STOP_SEC_CODE
#include "tpl_memmap.h"
