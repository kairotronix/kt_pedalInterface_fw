# kt_pedalInterface_fw

I2C slave controllable device...

CONTROL_WORD = `{addr[7:0]}`

Address map:
0. CTRL_ADDR_GENERAL_CTRL
1. CTRL_ADDR_GENERAL_STATUS
2. CTRL_ADDR_POT0
3. CTRL_ADDR_POT1
4. CTRL_ADDR_POT2
5. CTRL_ADDR_POT3
6. CTRL_ADDR_INTRPT_CTRL
7. CTRL_ADDR_INTRPT_STATUS
8. CTRL_ADDR_NUM_THRESH_POT0
9. CTRL_ADDR_NUM_THRESH_POT1
10. CTRL_ADDR_NUM_THRESH_POT2
11. CTRL_ADDR_NUM_THRESH_POT3
12. CTRL_ADDR_THRESH_LOW0_POT0
13. CTRL_ADDR_THRESH_LOW0_POT1
14. CTRL_ADDR_THRESH_LOW0_POT2
15. CTRL_ADDR_THRESH_LOW0_POT3
16. CTRL_ADDR_THRESH_LOW1_POT0
17. CTRL_ADDR_THRESH_LOW1_POT1
18. CTRL_ADDR_THRESH_LOW1_POT2
19. CTRL_ADDR_THRESH_LOW1_POT3
20. CTRL_ADDR_THRESH_LOW2_POT0
21. CTRL_ADDR_THRESH_LOW2_POT1
22. CTRL_ADDR_THRESH_LOW2_POT2
23. CTRL_ADDR_THRESH_LOW2_POT3
24. CTRL_ADDR_THRESH_LOW3_POT0
25. CTRL_ADDR_THRESH_LOW3_POT1
26. CTRL_ADDR_THRESH_LOW3_POT2
27. CTRL_ADDR_THRESH_LOW3_POT3
28. CTRL_ADDR_THRESH_HIGH0_POT0
29. CTRL_ADDR_THRESH_HIGH0_POT1
30. CTRL_ADDR_THRESH_HIGH0_POT2
31. CTRL_ADDR_THRESH_HIGH0_POT3
32. CTRL_ADDR_THRESH_HIGH1_POT0
33. CTRL_ADDR_THRESH_HIGH1_POT1
34. CTRL_ADDR_THRESH_HIGH1_POT2
35. CTRL_ADDR_THRESH_HIGH1_POT3
36. CTRL_ADDR_THRESH_HIGH2_POT0
37. CTRL_ADDR_THRESH_HIGH2_POT1
38. CTRL_ADDR_THRESH_HIGH2_POT2
39. CTRL_ADDR_THRESH_HIGH2_POT3
40. CTRL_ADDR_THRESH_HIGH3_POT0
41. CTRL_ADDR_THRESH_HIGH3_POT1
42. CTRL_ADDR_THRESH_HIGH3_POT2
43. CTRL_ADDR_THRESH_HIGH3_POT3

## CTRL_ADDR_GENERAL_CTRL

## CTRL_ADDR_GENERAL_STATUS
aka "sysvars.sys_flags" in the code

Bit positions:
0. POT0 installed -> There is a potentiometer installed in the POT1 (ADC1) port. 
1. POT1 installed -> There is a potentiometer installed in the POT1 (ADC1) port. 
2. POT2 installed -> There is a potentiometer installed in the POT1 (ADC1) port. 
3. POT3 installed -> There is a potentiometer installed in the POT1 (ADC1) port. 

## CTRL_ADDR_INTRPT_CTRL / CTRL_ADDR_INTRPT_STATUS

16-bit register(s). CTRL -> write 1 to the bitposition to enable the external interrupt for this event.
Bit positions:
0. STOMP_INTR -> External STOMP switch has been actuated
1. POT0_LEFT_ZONE_INTR -> POT0 has left an interrupt-enabled threashold zone
2. POT1_LEFT_ZONE_INTR -> POT1 has left an interrupt-enabled threashold zone
3. POT2_LEFT_ZONE_INTR -> POT2 has left an interrupt-enabled threashold zone
4. POT3_LEFT_ZONE_INTR -> POT3 has left an interrupt-enabled threashold zone

## CTRL_ADDR_NUM_THRESH_POT0/1/2/3
Indicates how many control threshold "zones" there are in software.

NOTE: If you only use 1 zone then the values in the `CTRL_ADDR_THRESH_LOW1~3_POTx` and `CTRL_ADDR_THRESH_HIGH1~3_POTx` registers are ignored.
"Zone" 0 is associated with `THRESH_LOW0` and `TRESH_HIGH0`, likewise for 1~3. 

There can only be up to 4 zones (for now).