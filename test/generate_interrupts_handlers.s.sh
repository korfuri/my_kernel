#!/bin/sh

#LIST="0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31"
LIST="`
grep -E '^void\W+interrupt_handler_' interrupts.c |
 sed -e 's/^void\ *//g' |
 cut -d '(' -f 1  |
 sed -e 's/interrupt_handler_//g'`"

(
echo 'align 4'
echo ''

for i in $LIST
do
    echo 'extern interrupt_handler_'$i
    echo 'global isr'$i
    echo ''
    echo 'isr'$i':'
    echo '	cli'
    echo '	pushfd'
    echo '	pushad'
#    echo '	push esp'
    echo '	call interrupt_handler_'$i
    echo '	popad'
    echo '	popfd'
#    echo '	sti'
    echo '	iret'
done
) > interrupts_handlers.s

(
echo '#ifndef INTERRUPTS_HANDLERS_H_'
echo '#define INTERRUPTS_HANDLERS_H_'

for i in $LIST
do
    echo 'void isr'$i'(void);'
done

echo '#endif'
) > interrupts_handlers.h

(
echo 'static void idt_set_gates() {'
for i in $LIST
do
    echo '  idt_set_gate('$i', isr'$i', 0x08, 0x8E);'
done

echo '}'
) > idt_set_gates.c

