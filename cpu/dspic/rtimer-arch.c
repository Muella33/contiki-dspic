/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: rtimer-arch.c,v 1.10 2010/02/28 21:29:19 dak664 Exp $
 */

/**
 * \file
 *         PIC specific timer code, using hardware Timer 2/3
 * \author
 *         Chris Shucksmith <chris@shucksmith.co.uk>
  */

/* OBS: 8 seconds maximum time! */

#include <stdio.h>
#include <p33Fxxxx.h>

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "rtimer-arch.h"

/* Track flow through rtimer interrupts*/
#if DEBUGFLOWSIZE&&0
extern uint8_t debugflowsize,debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if (debugflowsize<(DEBUGFLOWSIZE-1)) debugflow[debugflowsize++]=c
#else
#define DEBUGFLOW(c)
#endif

/*---------------------------------------------------------------------------*/
#if defined(TCNT3) && RTIMER_ARCH_PRESCALER
ISR (TIMER3_COMPA_vect) {
  DEBUGFLOW('/');
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* Disable rtimer interrupts */
  ETIMSK &= ~((1 << OCIE3A) | (1 << OCIE3B) | (1 << TOIE3) |
      (1 << TICIE3) | (1 << OCIE3C));

#if RTIMER_CONF_NESTED_INTERRUPTS
  /* Enable nested interrupts. Allows radio interrupt during rtimer interrupt. */
  /* All interrupts are enabled including recursive rtimer, so use with caution */
  sei();
#endif

  /* Call rtimer callback */
  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  DEBUGFLOW('\\');
}

#elif RTIMER_ARCH_PRESCALER
#warning "No Timer3 in rtimer-arch.c - using Timer1 instead"
ISR (TIMER1_COMPA_vect) {
  DEBUGFLOW('/');
  TIMSK &= ~((1<<TICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));

  rtimer_run_next();
  DEBUGFLOW('\\');
}

#endif
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
#if RTIMER_ARCH_PRESCALER
  /* Disable interrupts (store old state) */
  uint8_t sreg;
  sreg = SREG;
  cli ();

#ifdef TCNT3
  /* Disable all timer functions */
  ETIMSK &= ~((1 << OCIE3A) | (1 << OCIE3B) | (1 << TOIE3) |
      (1 << TICIE3) | (1 << OCIE3C));
  /* Write 1s to clear existing timer function flags */
  ETIFR |= (1 << ICF3) | (1 << OCF3A) | (1 << OCF3B) | (1 << TOV3) |
  (1 << OCF3C); 

  /* Default timer behaviour */
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3C = 0;

  /* Reset counter */
  TCNT3 = 0;

#if RTIMER_ARCH_PRESCALER==1024
  TCCR3B |= 5;
#elif RTIMER_ARCH_PRESCALER==256
  TCCR3B |= 4;
#elif RTIMER_ARCH_PRESCALER==64
  TCCR3B |= 3;
#elif RTIMER_ARCH_PRESCALER==8
  TCCR3B |= 2;
#elif RTIMER_ARCH_PRESCALER==1
  TCCR3B |= 1;
#else
#error Timer3 PRESCALER factor not supported.
#endif

#elif RTIMER_ARCH_PRESCALER
  /* Leave timer1 alone if PRESCALER set to zero */
  /* Obviously you can not then use rtimers */

  TIMSK &= ~((1<<TICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));
  TIFR |= (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1);

  /* Default timer behaviour */
  TCCR1A = 0;
  TCCR1B = 0;

  /* Reset counter */
  TCNT1 = 0;

  /* Start clock */
#if RTIMER_ARCH_PRESCALER==1024
  TCCR1B |= 5;
#elif RTIMER_ARCH_PRESCALER==256
  TCCR1B |= 4;
#elif RTIMER_ARCH_PRESCALER==64
  TCCR1B |= 3;
#elif RTIMER_ARCH_PRESCALER==8
  TCCR1B |= 2;
#elif RTIMER_ARCH_PRESCALER==1
  TCCR1B |= 1;
#else
#error Timer1 PRESCALER factor not supported.
#endif

#endif /* TCNT3 */

  /* Restore interrupt state */
  SREG = sreg;
#endif /* RTIMER_ARCH_PRESCALER */
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
#if RTIMER_ARCH_PRESCALER
  /* Disable interrupts (store old state) */
  uint8_t sreg;
  sreg = SREG;
  cli ();
  DEBUGFLOW(':');
#ifdef TCNT3
  /* Set compare register */
  OCR3A = t;
  /* Write 1s to clear all timer function flags */
  ETIFR |= (1 << ICF3) | (1 << OCF3A) | (1 << OCF3B) | (1 << TOV3) |
  (1 << OCF3C);
  /* Enable interrupt on OCR3A match */
  ETIMSK |= (1 << OCIE3A);

#elif RTIMER_ARCH_PRESCALER
  /* Set compare register */
  OCR1A = t;
  TIFR |= (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1);
  TIMSK |= (1 << OCIE1A);

#endif

  /* Restore interrupt state */
  SREG = sreg;
#endif /* RTIMER_ARCH_PRESCALER */
}
