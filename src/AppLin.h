/*
 * AppLin.h
 *
 *  Created on: 2025. 5. 29.
 *      Author: mctp
 */

#ifndef CODE_SRC_APPLIN_H_
#define CODE_SRC_APPLIN_H_

extern uint8_t Fwv_Request_Event;
extern uint8_t Fwv_Response_Event;

void AppLinInit(void);
void AppLinTask(void);
void AppLinSleepEnter(void);
uint8_t LinGetCommState(void);

#endif /* CODE_SRC_APPLIN_H_ */
