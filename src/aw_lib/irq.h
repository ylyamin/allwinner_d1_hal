/*
 * File: irq.h
 * Author: ylyamin
 */
#ifndef IRQ_H_
#define IRQ_H_

void irq_enable(int irq);
void irq_disable(int irq);
void irq_assign(int irq, void (*func)(void *));
void irq_init(void);

#endif /*IRQ_H_*/